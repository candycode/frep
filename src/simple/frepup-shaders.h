#ifndef FREPUP_SHADERS_H_
#define FREPUP_SHADERS_H_

#include <string>
#include <vector>
#include <osg/Group>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Uniform>


//------------------------------------------------------------------------------
struct PrimitiveEntry
{
	std::string objname;
	std::string frepname;
	std::string gradname;
	std::string intersectname;
	std::string colorname;
	std::string invMatrixName;
	std::string matrixName;
	std::string invNormalMatrixName;
	std::string normalMatrixName;
	std::string frepCode;
	std::string matCode;
	osg::ref_ptr< osg::MatrixTransform > manipTransform; //filled by function that creates the primitive scenegraph
	float boxSizeX;
	float boxSizeY;
	float boxSizeZ;
	osg::ref_ptr< osg::Uniform > matrixUniform; //filled by function that creates the primitive scenegraph
	osg::ref_ptr< osg::Uniform > normalMatrixUniform; //filled by function that creates the primitive scenegraph 
	osg::ref_ptr< osg::Uniform > invMatrixUniform; //filled by function that creates the primitive scenegraph
	osg::ref_ptr< osg::Uniform > invNormalMatrixUniform; //filled by function that creates the primitive scenegraph
	osg::ref_ptr< osg::Node > node; //filled by function that creates the primitive scenegraph
	PrimitiveEntry() {}
	PrimitiveEntry( const std::string& obj,
					const std::string& frep,
					const std::string& grad,
					const std::string& isect,
					const std::string& compColorName,
					const std::string& invmatname,
					const std::string& matname,
					const std::string& invNormMatName,
					const std::string& normMatName,
					const std::string& code,
					const std::string& mCode ) : 
					objname( obj ), frepname( frep ),
					gradname( grad ), intersectname( isect ), colorname( compColorName ), invMatrixName( invmatname ),
					matrixName( matname ),
					invNormalMatrixName( invNormMatName ), normalMatrixName( normMatName ), frepCode( code ), matCode( mCode ),
					boxSizeX( 1.0f ), boxSizeY( 1.0f ),
					boxSizeZ( 1.0f ) {}
};

typedef std::vector< PrimitiveEntry > Primitives;

osg::Group* CreatePrimitiveSceneGraph( PrimitiveEntry& p,
									   const std::string& vertexShaderText, 
									   const std::string& fragmentShaderText );

osg::Group* CreateCompositeSceneGraph( Primitives& primitives,
									   const std::string& compositeCode,
									   const std::string& compositeMaterial,
									   const std::string& vertexShaderText,
									   const std::string& fragmentShaderText,
									   float compositeBoxHalfSize,
									   /*out*/ osg::ref_ptr< osg::Box >& compositeBox,
									   /*out*/ osg::ref_ptr< osg::Uniform >& halfBoxSizes,
									   /*out*/ osg::ref_ptr< osg::Uniform >& boxCenter );

osg::Camera::DrawCallback*  CreateCompositeCameraCallback( Primitives& p,
														   osg::Group* primitiveRoot,
														   osg::Group* compositeRoot,
														   osg::Box* compositeBox,
														   osg::Uniform* halfBoxSize,
														   osg::Uniform* boxCenter,
														   osg::Uniform* viewportUniform = 0 );

typedef std::vector< osg::ref_ptr< osg::Box > > BoxVector;
typedef std::vector< osg::ref_ptr< osg::Uniform > > UniformVector;
osg::Group* CreateOptimalCompositeSceneGraph( Primitives& primitives,
									   const std::string& compositeCode,
									   const std::string& compositeMaterial,
									   const std::string& vertexShaderText,
									   const std::string& fragmentShaderText,
									   float compositeBoxHalfSize,
									   /*out*/ osg::ref_ptr< osg::Box >& compositeBox,
									   /*out*/ BoxVector& compositeBoxes,
									   /*out*/ UniformVector& halfBoxSizes,
									   /*out*/ UniformVector& primitiveMatrices );

osg::Camera::DrawCallback*  CreateOptimalCompositeCameraCallback( Primitives& p,
																  std::vector< osg::ref_ptr< osg::Node > >& primitiveNodes,
																  BoxVector& compositeBoxes,
																  osg::Box* compositeBox,
																  UniformVector& halfBoxSizes,
																  UniformVector& primitiveMatrices,
																  osg::Uniform* viewportUniform = 0 );

#endif //FREPUP_HADERS_H_