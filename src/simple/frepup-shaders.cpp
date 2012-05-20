#include "frepup-shaders.h"
#include "aspect_ratio.h"

#include <iostream>

#include <cassert>

#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/PolygonMode>
#include <osg/ComputeBoundsVisitor>
#include <osg/LineStipple>

#include <osgManipulator/TranslateAxisDragger> 
#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabPlaneTrackballDragger>
#include <osgManipulator/RotateCylinderDragger>
#include <osgManipulator/RotateSphereDragger>
#include <osgManipulator/Scale1DDragger>
#include <osgManipulator/Scale2DDragger>
#include <osgManipulator/ScaleAxisDragger>
#include <osgManipulator/TabBoxTrackballDragger>
#include <osgManipulator/TabPlaneDragger>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/TranslatePlaneDragger>



static const std::string ISOFUN  = "IsoFunction";
static const std::string ISOGRAD = "IsoGradient";
static const std::string INTERSECT = "Intersect";
static const std::string COLOR   = "ComputeColor";


osg::Program* CreatePassThroughProgram()
{
	static const char PASSTHROUGH_VERT[] =
	"varying vec4 color;"
	"void main(void)\n"
	"{\n"
	"  color = gl_FrontMaterial.diffuse;\n"
	"  gl_Position = ftransform();\n"
	"}\n";

	static const char PASSTHROUGH_FRAG[] =
	"varying vec4 color;"
	"void main(void)\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n";


	osg::ref_ptr< osg::Program > p = new osg::Program;
    p->addShader( new osg::Shader( osg::Shader::FRAGMENT, PASSTHROUGH_FRAG ) );
	p->addShader( new osg::Shader( osg::Shader::VERTEX,   PASSTHROUGH_VERT ) );
	return p.release();
}


osgManipulator::Dragger* SetupDraggerProperties( osgManipulator::Dragger* d ) { return d; }
/// Creates manipulator from text.
osgManipulator::Dragger* CreateDragger( const std::string& type )
{
    osg::ref_ptr< osgManipulator::Dragger > d;
    if( type == "TranslateAxisDragger" ) d = SetupDraggerProperties( new osgManipulator::TranslateAxisDragger );
    else if( type == "TabBoxDragger" ) d = SetupDraggerProperties( new osgManipulator::TabBoxDragger );
    else if( type == "TabPlaneTrackballDragger" ) d = SetupDraggerProperties( new osgManipulator::TabPlaneTrackballDragger );
    else if( type == "RotateCylinderDragger" ) d = SetupDraggerProperties( new osgManipulator::RotateCylinderDragger );
    else if( type == "RotateSphereDragger" ) d = SetupDraggerProperties( new osgManipulator::RotateSphereDragger );
    else if( type == "Scale1DDragger" ) d = SetupDraggerProperties( new osgManipulator::Scale1DDragger );
    else if( type == "Scale2DDragger" ) d = SetupDraggerProperties( new osgManipulator::Scale2DDragger );
    else if( type == "ScaleAxisDragger" ) d = SetupDraggerProperties( new osgManipulator::ScaleAxisDragger );
    else if( type == "TabBoxTrackballDragger" ) d = SetupDraggerProperties( new osgManipulator::TabBoxTrackballDragger );
    else if( type == "TabPlaneDragger" ) d = SetupDraggerProperties( new osgManipulator::TabPlaneDragger );
    else if( type == "TrackballDragger" ) d = SetupDraggerProperties( new osgManipulator::TrackballDragger );
    else if( type == "TranslatePlaneDragger" ) d = SetupDraggerProperties( new osgManipulator::TranslatePlaneDragger );
    if( !d ) 
    {
        throw std::logic_error( "Unknown dragger: " + type );
        return 0; //in case exceptions not enabled
    }
    return d.release();
}

static const float MANIPULATOR_SCALING_FACTOR = 1.3f;

template < class M >
M* CreateManipulator( osg::MatrixTransform* mt )
{
	if( !mt ) throw std::runtime_error( "NULL matrix transform" );
	if( !mt ) return 0; // in case exceptions not enabled
	osg::ref_ptr< M > dragger = new M;
	const float scale = mt->getBound().radius() * MANIPULATOR_SCALING_FACTOR;
    dragger->setMatrix( osg::Matrix::scale( scale, scale, scale ) *
                        osg::Matrix::translate( mt->getBound().center() ) );
    dragger->setHandleEvents( true );
    dragger->addTransformUpdating( mt );
	dragger->setupDefaultGeometry();
	//dragger->getOrCreateStateSet()->setAttributeAndModes( CreatePassThroughProgram(), osg::StateAttribute::OVERRIDE );
	return dragger.release();
}


//------------------------------------------------------------------------------
std::string BuildFragShaderPrefix( const std::string& frepname,
								   const std::string& gradname )
{
	std::string prefix;
	prefix += "#define IsoFunction ";
	prefix += frepname;
	prefix += '\n';
	prefix += "#define IsoGradient ";
	prefix += gradname;
	prefix += '\n';
	return prefix;
}

//------------------------------------------------------------------------------
std::string ReplaceAllOccurrences( std::string text,
								   const std::string& replaced,
								   const std::string& replacement )
{
	std::string::size_type s = text.find( replaced );
	while( s != std::string::npos )
	{
		text.replace( s, replaced.size(), replacement ); 
		s = text.find( replaced );
	}
	return text;
}

//------------------------------------------------------------------------------
osg::Group* CreatePrimitiveSceneGraph( PrimitiveEntry& p,
									   const std::string& vertexShaderText, 
									   const std::string& fragmentShaderText )
{
	// manipulator -> matrix transform -> unit box geode
	// shaders -> unit box geode
	osg::ref_ptr< osg::Group > root = new osg::Group;
	osg::ref_ptr< osg::MatrixTransform > t = new osg::MatrixTransform;
	p.manipTransform = t;
	osg::ref_ptr< osg::Geode > geode = new osg::Geode;
	geode->addDrawable( new osg::ShapeDrawable( 
							new osg::Box( 
									osg::Vec3( 0.f, 0.f, 0.f ),
									p.boxSizeX, p.boxSizeY, p.boxSizeZ ) ) );
	t->addChild( osg::get_pointer( geode ) );
	p.node = t;

	osg::ref_ptr< osg::StateSet > set = geode->getOrCreateStateSet();
	osg::ref_ptr< osg::Program > aprogram = new osg::Program;
	aprogram->setName( "frep " + p.objname );
	aprogram->addShader( new osg::Shader( osg::Shader::VERTEX, vertexShaderText ) );
	std::string fragShader = ReplaceAllOccurrences( 
								ReplaceAllOccurrences( 
									ReplaceAllOccurrences( fragmentShaderText, INTERSECT, p.intersectname ), 
									ISOFUN, p.frepname ),
								ISOGRAD, p.gradname );
	fragShader = ReplaceAllOccurrences( fragShader, COLOR, p.colorname );
	std::clog << "=======================\n" << "Primitive " << p.objname << '\n';
	std::clog << "Shader: \n" << fragShader << '\n';
	// implementation of frep and gradient functions
	aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, p.frepCode ) );
	// material shader: ComputeColor invoked by main fragment shader
	if( !p.matCode.empty() ) aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, p.matCode ) );
	// the main fragment shader invokes IsoFuntion and IsoGradient redefined by the prefix
	// and implemented in separate shader
	std::clog << "Material: \n" << p.matCode << '\n';
	aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragShader  ) );
	set->setAttributeAndModes( osg::get_pointer( aprogram ), osg::StateAttribute::ON );
	osg::ref_ptr< osgManipulator::TabBoxDragger > m = CreateManipulator< osgManipulator::TabBoxDragger >( osg::get_pointer( t ) );
	//osg::ref_ptr< osgManipulator::TabBoxTrackballDragger > m = CreateManipulator< osgManipulator::TabBoxTrackballDragger >( osg::get_pointer( t ) );
	//osg::ref_ptr< osgManipulator::TabPlaneTrackballDragger > m = CreateManipulator< osgManipulator::TabPlaneTrackballDragger >( osg::get_pointer( t ) );	
	root->addChild( osg::get_pointer( m ) );
	root->addChild( osg::get_pointer( t ) );
	set->addUniform( new osg::Uniform( "halfBoxSize", osg::Vec3( p.boxSizeX, p.boxSizeY, p.boxSizeZ ) * .5 ) );
	return root.release();
}

std::string AddColorPrimitivesDeclarations( const std::string& shader, const Primitives& p )
{
	std::string header;
	for( Primitives::const_iterator i = p.begin(); i != p.end(); ++i )
	{
		if( i->colorname.empty() ) continue;
		const std::string computecolordec = "vec4 " + i->colorname + "( vec3, vec3 );\n";
		header += computecolordec;
	}
	return header + shader;
}


//------------------------------------------------------------------------------
osg::Node* CreateCompositeBBox( osg::Box* b )
{
	osg::ref_ptr< osg::Geode > box = new osg::Geode;
	osg::ref_ptr< osg::ShapeDrawable > drawable = new osg::ShapeDrawable( b );
	drawable->setUseDisplayList( false );
	box->addDrawable( osg::get_pointer( drawable ) );
    osg::ref_ptr< osg::PolygonMode > polyModeObj = new osg::PolygonMode;
    polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
	osg::ref_ptr< osg::LineStipple > stipplePattern = new osg::LineStipple( 1, 0xF0 );
    box->getOrCreateStateSet()->setAttributeAndModes( osg::get_pointer( polyModeObj ) );
	box->getOrCreateStateSet()->setAttributeAndModes( osg::get_pointer( stipplePattern ) );
	box->getOrCreateStateSet()->setAttributeAndModes( CreatePassThroughProgram(), osg::StateAttribute::OVERRIDE );
	//disable box
	box->setNodeMask( 0 );
	
	return box.release();
}

//------------------------------------------------------------------------------
osg::Group* CreateCompositeSceneGraph( Primitives& primitives,
									   const std::string& compositeCode,
									   const std::string& compositeMaterial,
									   const std::string& vertexShaderText,
									   const std::string& fragmentShaderText,
									   float compositeBoxSize,
									   osg::ref_ptr< osg::Box >& compositeBox,
									   osg::ref_ptr< osg::Uniform >& halfBoxU,
									   osg::ref_ptr< osg::Uniform >& boxCenterU )
{
	osg::ref_ptr< osg::Group > root = new osg::Group;

	osg::ref_ptr< osg::Geode > geode = new osg::Geode;
	compositeBox = new osg::Box( osg::Vec3( 0.f, 0.f, 0.f ), compositeBoxSize,
								 compositeBoxSize, compositeBoxSize );

	osg::ref_ptr< osg::ShapeDrawable > drawable = new osg::ShapeDrawable( osg::get_pointer( compositeBox ) );
	drawable->setUseDisplayList( false );
	geode->addDrawable( osg::get_pointer( drawable ) );
	root->addChild( osg::get_pointer( geode ) );

	osg::ref_ptr< osg::StateSet > set = geode->getOrCreateStateSet();
	osg::ref_ptr< osg::Program > aprogram = new osg::Program;
    aprogram->setName( "frep composite" );
	aprogram->addShader( new osg::Shader( osg::Shader::VERTEX, vertexShaderText ) );
	// main shader invokes IsoFunction and IsoGradient which need to be subsituted with
	// the name of the functions defined in the composite shader code
	std::string fragShader = ReplaceAllOccurrences( 
								ReplaceAllOccurrences( 
									ReplaceAllOccurrences( fragmentShaderText, INTERSECT, "composite_I" ),
									ISOFUN, "composite_F" ),
								ISOGRAD, "composite_G" );
	std::clog << "///////////////////////////////\n" << "Composite shader:\n";
	std::clog << fragShader << '\n';
	// add one shader per primitive: composite shader will call into each primitive shader
	for( Primitives::iterator i = primitives.begin(); i != primitives.end(); ++i )
	{
		aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, i->frepCode ) );
		i->invMatrixUniform = new osg::Uniform( i->invMatrixName.c_str(), osg::Matrix::identity() );
		i->invNormalMatrixUniform = new osg::Uniform( i->invNormalMatrixName.c_str(), osg::Matrix::identity() );
		i->matrixUniform = new osg::Uniform( i->matrixName.c_str(), osg::Matrix::identity() );
		i->normalMatrixUniform = new osg::Uniform( i->normalMatrixName.c_str(), osg::Matrix::identity() );
		set->addUniform( osg::get_pointer( i->invMatrixUniform ) );
		set->addUniform( osg::get_pointer( i->invNormalMatrixUniform ) );
		set->addUniform( osg::get_pointer( i->matrixUniform ) );
		set->addUniform( osg::get_pointer( i->normalMatrixUniform ) );
	}
	// composite shader
	const std::string compositeShader = compositeCode;
	aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, compositeShader ) );
	// main shader
	aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragShader ) );
	// material
	if( !compositeMaterial.empty() )
	{
		// add one shader per primitive: composite shader will call into each primitive shader
		for( Primitives::iterator i = primitives.begin(); i != primitives.end(); ++i )
		{
			aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, i->matCode ) );
		}
#ifdef ADD_DECLARATIONS
		const std::string matShader = AddPrimitivesDeclarations( AddColorPrimitivesDeclarations( compositeMaterial, primitives ), primitives );
#else
		const std::string matShader = compositeMaterial;
#endif
		std::clog << "Material: \n" << matShader << '\n';
		aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, matShader ) );
	}
	set->setAttributeAndModes( osg::get_pointer( aprogram ), osg::StateAttribute::ON );
	halfBoxU = new osg::Uniform( "halfBoxSize", osg::Vec3( compositeBoxSize, compositeBoxSize, compositeBoxSize ) * .5 );
	boxCenterU = new osg::Uniform( "boxCenter", osg::Vec3() );
	set->addUniform( osg::get_pointer( halfBoxU ) );
	set->addUniform( osg::get_pointer( boxCenterU ) );
	root->addChild( CreateCompositeBBox( osg::get_pointer( compositeBox ) ) );
	return root.release();
}

//------------------------------------------------------------------------------
osg::Group* CreateOptimalCompositeSceneGraph( Primitives& primitives,
									   const std::string& compositeCode,
									   const std::string& compositeMaterial,
									   const std::string& vertexShaderText,
									   const std::string& fragmentShaderText,
									   float compositeBoxSize,
									   osg::ref_ptr< osg::Box >& compositeBox,
									   BoxVector& compositeBoxes, 
									   UniformVector& halfBoxU,
									   UniformVector& primitiveMatrices )
{
	osg::ref_ptr< osg::Group > root = new osg::Group;

	//add one box per primitive
	compositeBox = new osg::Box( osg::Vec3( 0.f, 0.f, 0.f ), compositeBoxSize,
								 compositeBoxSize, compositeBoxSize );
	for( Primitives::iterator i = primitives.begin(); i != primitives.end(); ++i )
	{
		osg::ref_ptr< osg::Geode > geode = new osg::Geode;
		osg::ref_ptr< osg::Box > box = new osg::Box;
		osg::ref_ptr< osg::ShapeDrawable > drawable = new osg::ShapeDrawable( osg::get_pointer( box ) );
		compositeBoxes.push_back( box );
		osg::ref_ptr< osg::Uniform > halfBoxSize =
			new osg::Uniform( "halfBoxSize", osg::Vec3( i->boxSizeX,
														i->boxSizeY,
														i->boxSizeZ ) * 0.5 );
		osg::ref_ptr< osg::Uniform > primitiveMatrix = new osg::Uniform( "primitiveModelviewInv", osg::Matrix::identity() );
		primitiveMatrices.push_back( primitiveMatrix );
		geode->getOrCreateStateSet()->addUniform( osg::get_pointer( halfBoxSize ) );
		geode->getOrCreateStateSet()->addUniform( osg::get_pointer( primitiveMatrix ) );
		halfBoxU.push_back( halfBoxSize );
		drawable->setUseDisplayList( false );
		geode->addDrawable( osg::get_pointer( drawable ) );
		root->addChild( osg::get_pointer( geode ) );
		root->addChild( CreateCompositeBBox( osg::get_pointer( box ) ) );
	}

	osg::ref_ptr< osg::StateSet > set = root->getOrCreateStateSet();
	osg::ref_ptr< osg::Program > aprogram = new osg::Program;
    aprogram->setName( "frep composite" );
	aprogram->addShader( new osg::Shader( osg::Shader::VERTEX, vertexShaderText ) );
	// main shader invokes IsoFunction and IsoGradient which need to be subsituted with
	// the name of the functions defined in the composite shader code
	std::string fragShader = ReplaceAllOccurrences( 
								ReplaceAllOccurrences( fragmentShaderText, ISOFUN, "composite_F" ),
								ISOGRAD, "composite_G" );
//	std::clog << "///////////////////////////////\n" << "Composite shader:\n";
//	std::clog << fragShader << '\n';
	// add one shader per primitive: composite shader will call into each primitive shader
	for( Primitives::iterator i = primitives.begin(); i != primitives.end(); ++i )
	{
		aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, i->frepCode ) );
		i->invMatrixUniform = new osg::Uniform( i->invMatrixName.c_str(), osg::Matrix::identity() );
		set->addUniform( osg::get_pointer( i->invMatrixUniform ) );
	}
	// composite shader
#ifdef ADD_DECLARATIONS
	const std::string compositeShader = AddPrimitivesDeclarations( compositeCode, primitives );
#else
	const std::string compositeShader = compositeCode;
#endif
	aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, compositeShader ) );
	// main shader
	aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragShader ) );
	// material
	if( !compositeMaterial.empty() )
	{
		// add one shader per primitive: composite shader will call into each primitive shader
		for( Primitives::iterator i = primitives.begin(); i != primitives.end(); ++i )
		{
			aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, i->matCode ) );
		}
#ifdef ADD_DECLARATIONS
		const std::string matShader = AddPrimitivesDeclarations( AddColorPrimitivesDeclarations( compositeMaterial, primitives ), primitives );
#else
		const std::string matShader = compositeMaterial;
#endif
		std::clog << "Material: \n" << matShader << '\n';
		aprogram->addShader( new osg::Shader( osg::Shader::FRAGMENT, matShader ) );
	}
	set->setAttributeAndModes( osg::get_pointer( aprogram ), osg::StateAttribute::ON );
	return root.release();
}

//------------------------------------------------------------------------------
/** Draw callback for custom operations.*/
//struct OSG_EXPORT DrawCallback : virtual public Object
//{
//    DrawCallback() {}
//
//    DrawCallback(const DrawCallback&,const CopyOp&) {}
//
//    META_Object(osg, DrawCallback);
//
//    /** Functor method called by rendering thread. Users will typically override this method to carry tasks such as screen capture.*/
//    virtual void operator () (osg::RenderInfo& renderInfo) const;
//
//    /** Functor method, provided for backwards compatibility, called by operator() (osg::RenderInfo& renderInfo) method.*/
//    virtual void operator () (const osg::Camera& /*camera*/) const {}
//};
class CompositeCameraCBack : public osg::Camera::DrawCallback
{
public:
	CompositeCameraCBack( Primitives& p,
						  osg::Group* primitiveRoot,
						  osg::Group* compositeRoot,
						  osg::Box* compositeBox,
						  osg::Uniform* halfBoxSize,
						  osg::Uniform* boxCenter,
						  osg::Uniform* vportU,
						  bool computeNormalMatrix = true ) : // set to true when using external intersection routines
							primitives_( p ),
							primitiveRoot_( primitiveRoot ),
							compositeRoot_( compositeRoot ),
							compositeBox_( compositeBox ),
							fixAspectRatio_( vportU ),
							halfBoxSizeU_( halfBoxSize ),
							boxCenterU_( boxCenter ),
							computeNormalMatrix_( computeNormalMatrix ),
							compositeBoxMin_( compositeBox_ ) //record original size and ensure it's never smaller 
	{}
	void operator()( osg::RenderInfo& ri ) const
	{
		
		cbbv_.reset();
		for( Primitives::const_iterator i = primitives_.begin(); i != primitives_.end(); ++i )
		{
			i->node->accept(cbbv_);
			inv_ = i->manipTransform->getInverseMatrix();
			i->invMatrixUniform->set( inv_ );
			if( computeNormalMatrix_ ) // means we are using the shaders with external intersection
			{
				i->matrixUniform->set( i->manipTransform->getMatrix() );
				invTranspose_ = inv_; Transpose( invTranspose_ );
				invTransposeInv_ = invTranspose_; 
				i->normalMatrixUniform->set( invTransposeInv_ );
				osg::Matrix::inverse( invTransposeInv_ );
				i->invNormalMatrixUniform->set( invTransposeInv_ );
			}
		}
		//cbbv_.reset();
		//primitiveRoot_->accept(cbbv_);
		const osg::BoundingBox& bb = cbbv_.getBoundingBox();
		{
			hbs_ = osg::Vec3( abs( bb.xMax() - bb.xMin() ),
				   abs( bb.yMax() - bb.yMin() ),
				   abs( bb.zMax() - bb.zMin() ) ) * 0.5f;
			// min bbox is centered in <0,0,0> and the *Max() methods
			// return the actual half lengths
			hbs_.x() = std::max( hbs_.x(), compositeBoxMin_->getHalfLengths().x() );
			hbs_.y() = std::max( hbs_.y(), compositeBoxMin_->getHalfLengths().y() );
			hbs_.z() = std::max( hbs_.z(), compositeBoxMin_->getHalfLengths().z() );
			compositeBox_->setHalfLengths( hbs_ );
			compositeBox_->setCenter( bb.center() );
		    halfBoxSizeU_->set( hbs_ );
			boxCenterU_->set( bb.center() );
		}
		fixAspectRatio_( ri );
		if( !ri.getCurrentCamera() ) return;
		
		
	}
private:
	void Transpose( osg::Matrix& m ) const // operates on mutables only
	{
		std::swap( m( 0, 1 ), m( 1, 0 ) );
		std::swap( m( 0, 2 ), m( 2, 0 ) );
		std::swap( m( 0, 3 ), m( 3, 0 ) );
		std::swap( m( 1, 2 ), m( 2, 1 ) );
		std::swap( m( 1, 3 ), m( 3, 1 ) );
		std::swap( m( 2, 3 ), m( 3, 2 ) );
	}
	Primitives primitives_;
	osg::ref_ptr< osg::Group > primitiveRoot_;
	osg::ref_ptr< osg::Group > compositeRoot_;
	osg::ref_ptr< osg::Box >   compositeBox_;
	CameraAspectRatioCBack fixAspectRatio_;
	osg::ref_ptr< osg::Uniform > halfBoxSizeU_;
	osg::ref_ptr< osg::Uniform > boxCenterU_;
	bool computeNormalMatrix_;
	osg::ref_ptr< osg::Box >   compositeBoxMin_;
	mutable osg::ComputeBoundsVisitor cbbv_;
	mutable osg::Vec3 hbs_;
	mutable osg::Matrix inv_, invTranspose_, invTransposeInv_;
	mutable double l_, r_, b_, t_, n_, f_;
};

osg::Camera::DrawCallback*  CreateCompositeCameraCallback( Primitives& p,
														   osg::Group* primitiveRoot,
														   osg::Group* compositeRoot,
														   osg::Box* compositeBox,
														   osg::Uniform* halfBoxSizeU,
														   osg::Uniform* boxCenterU,
														   osg::Uniform* vp )
{
	assert( primitiveRoot );
	assert( compositeBox );
	return new CompositeCameraCBack( p, primitiveRoot, compositeRoot, compositeBox, halfBoxSizeU, boxCenterU, vp );
}

//------------------------------------------------------------------------------
/** Draw callback for custom operations.*/
//struct OSG_EXPORT DrawCallback : virtual public Object
//{
//    DrawCallback() {}
//
//    DrawCallback(const DrawCallback&,const CopyOp&) {}
//
//    META_Object(osg, DrawCallback);
//
//    /** Functor method called by rendering thread. Users will typically override this method to carry tasks such as screen capture.*/
//    virtual void operator () (osg::RenderInfo& renderInfo) const;
//
//    /** Functor method, provided for backwards compatibility, called by operator() (osg::RenderInfo& renderInfo) method.*/
//    virtual void operator () (const osg::Camera& /*camera*/) const {}
//};
typedef std::vector< osg::ref_ptr< osg::Node > > NodeArray;
class OptimalCompositeCameraCBack : public osg::Camera::DrawCallback
{
public:
	OptimalCompositeCameraCBack( Primitives& p,
								 NodeArray& primitiveNodes,
								 BoxVector& compositeBoxes,
								 osg::Box* compositeBox,
								 UniformVector& halfBoxSize,
								 UniformVector& primitiveMatrices,
								 osg::Uniform* vportU = 0 ) :
								 primitives_( p ),
								 primitiveNodes_( primitiveNodes ),
								 compositeBoxes_( compositeBoxes ),
								 compositeBox_( compositeBox ),
								 halfBoxSizeU_( halfBoxSize ),
								 fixAspectRatio_( vportU ),
								 primitiveMatrices_( primitiveMatrices )
								
	{}
	void operator()( osg::RenderInfo& ri ) const
	{
		fixAspectRatio_( ri );
		//const osg::BoundingSphere& bs = primitiveRoot_->getBound();
		//const float factor = 1.f;
		//const osg::Vec3 hbs( factor * bs.radius(), factor * bs.radius(), factor * bs.radius() );
		if( primitives_.size() != primitiveNodes_.size() ||
			primitiveNodes_.size() != compositeBoxes_.size() ) return;
		osg::BoundingBox gbb;
		for( size_t i = 0; i != primitives_.size(); ++i )
		{
			cbbv_.reset();
			primitiveNodes_[ i ]->accept( cbbv_ );
			const osg::BoundingBox& bb = cbbv_.getBoundingBox();
			gbb.expandBy( bb );
			hbs_ = osg::Vec3( abs( bb.xMax() - bb.xMin() ),
							  abs( bb.yMax() - bb.yMin() ),
							  abs( bb.zMax() - bb.zMin() ) ) * 0.5f;
			compositeBoxes_[ i ]->setHalfLengths( hbs_ );
			compositeBoxes_[ i ]->setCenter( bb.center() );
			//halfBoxSizeU_[ i ]->set( hbs_);
			primitives_[ i ].invMatrixUniform->set( primitives_[ i ].manipTransform->getInverseMatrix() );
			primitiveMatrices_[ i ]->set( primitives_[ i ].manipTransform->getMatrix() ); 
			
		}
		hbs_ = osg::Vec3( abs( gbb.xMax() - gbb.xMin() ),
							  abs( gbb.yMax() - gbb.yMin() ),
							  abs( gbb.zMax() - gbb.zMin() ) );
		for( size_t i = 0; i != primitives_.size(); ++i )
		{
			halfBoxSizeU_[ i ]->set( hbs_);
		}
		compositeBox_->set( gbb.center(), hbs_ * 0.5f );
	}
private:
	Primitives primitives_;
	NodeArray primitiveNodes_;
	BoxVector compositeBoxes_;
	osg::ref_ptr< osg::Box >   compositeBox_;
	UniformVector halfBoxSizeU_;
	UniformVector primitiveMatrices_;
	CameraAspectRatioCBack fixAspectRatio_;
	mutable osg::ComputeBoundsVisitor cbbv_;
	mutable osg::Vec3 hbs_;
};

osg::Camera::DrawCallback*  CreateOptimalCompositeCameraCallback( Primitives& p,
														   std::vector< osg::ref_ptr< osg::Node > >& primitiveNodes,
														   BoxVector& compositeBoxes,
														   osg::Box* compositeBox,
														   UniformVector& halfBoxSizeU,
														   UniformVector& primitiveMatrices,
														   osg::Uniform* vp )
{
	assert( compositeBox );
	return new OptimalCompositeCameraCBack( p, primitiveNodes, compositeBoxes, compositeBox, halfBoxSizeU, primitiveMatrices, vp );
}