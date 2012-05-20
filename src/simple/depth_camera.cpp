#include <osg/Camera>
#include <osg/Texture>
#include <osg/TextureRectangle>
#include <osg/StateSet>
#include <osg/Uniform>
#include <cassert>
#include "aspect_ratio.h"

static const int MAX_FBO_WIDTH = 2048;
static const int MAX_FBO_HEIGHT = 2048;

//------------------------------------------------------------------------------
osg::TextureRectangle* GenerateDepthTextureRectangle()
{
    osg::ref_ptr< osg::TextureRectangle > tr = new osg::TextureRectangle;
	tr->setInternalFormat( GL_DEPTH_COMPONENT );
  	tr->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
	tr->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
	tr->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
	tr->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
	return tr.release();
}

//------------------------------------------------------------------------------
osg::TextureRectangle* GenerateColorTextureRectangle()
{
    osg::ref_ptr< osg::TextureRectangle > tr = new osg::TextureRectangle;
    tr->setSourceFormat( GL_RGBA );
    tr->setSourceType( GL_FLOAT );
    tr->setInternalFormat( GL_RGBA32F_ARB );
	tr->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
	tr->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
	tr->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
	tr->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
	return tr.release();
}

//------------------------------------------------------------------------------
// Attach depth or positions & normals textures to pre-render camera
osg::Camera* CreatePreRenderCamera( osg::Texture* depth,
                                    osg::Texture* positions,
                                    osg::Texture* normals )
{
	osg::ref_ptr< osg::Camera > camera = new osg::Camera;
	camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
	camera->setRenderOrder( osg::Camera::PRE_RENDER);//, 1 );
	camera->setClearMask( GL_DEPTH_BUFFER_BIT );
	camera->setAllowEventFocus( false );
	// ATTACH DEPTH TEXTURE TO CAMERA
	if( depth != 0 ) camera->attach( osg::Camera::DEPTH_BUFFER, depth ); 
  
	camera->setViewport( 0, 0, MAX_FBO_WIDTH, MAX_FBO_HEIGHT );
	// ATTACH TEXTURE TO STORE WORLD SPACE POSITIONS AND NORMALS WITH OPTIONAL DEPTH
    // STORED AS W COMPONENT
    if( positions ) camera->attach( osg::Camera::BufferComponent( osg::Camera::COLOR_BUFFER0 ), positions );
    if( normals   ) camera->attach( osg::Camera::BufferComponent( osg::Camera::COLOR_BUFFER0 + 1 ), normals );
    if( positions || normals )
    {
        camera->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        // ATTACH SHADERS TO CAMERA
        osg::ref_ptr< osg::StateSet > set = camera->getOrCreateStateSet();
        assert( osg::get_pointer( set ) );
        osg::ref_ptr< osg::Program > program = new osg::Program;
	    program->setName( "Positions and Normals" );
		//program->addShader( new osg::Shader( osg::Shader::FRAGMENT, POSNORMALSDEPTH_FRAG_MRT ) );
		//program->addShader( new osg::Shader( osg::Shader::VERTEX,   POSNORMALS_VERT_MRT ) );
        set->setAttributeAndModes( program.get(), osg::StateAttribute::ON );
	}
    return camera.release();
}





void SetCameraCallback( osg::Camera* cameraToUpdate, osg::Camera* observedCamera, osg::Uniform* vp )
{
    
	cameraToUpdate->setPreDrawCallback(  new SyncCameraNode( observedCamera, cameraToUpdate, vp ) );
	//cameraToUpdate->setPostDrawCallback( new NodeMaskCBack( dragger, 0xffffffff ) );
    //camera->setPostDrawCallback(  new SyncCameraNode( mc, camera, vp ) );
    //camera->setUpdateCallback( new SyncCameraNode( mc, vp ) );
}