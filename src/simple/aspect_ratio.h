#ifndef ASPECT_RATIO_H_
#define ASPECT_RATIO_H_
#include <osg/Camera>
#include <osg/RenderInfo>
#include <osg/Uniform>
#include <osg/ref_ptr>

class CameraAspectRatioCBack : public osg::Camera::DrawCallback
{
public:
	CameraAspectRatioCBack( osg::Uniform* vportU = 0 ) : vportU_( vportU ) {}
	virtual void operator () ( osg::RenderInfo& renderInfo ) const
	{
		osg::Camera* c = renderInfo.getCurrentCamera();
		if( !c ) return;
		osg::Viewport* v = c->getViewport();
		if( !v || v->height() == 0 ) return;
		ar = double( v->width() ) / v->height();
		c->getProjectionMatrixAsFrustum( l, r, b, t, n, f );
		if( abs( t - b ) < 0.000001 ) return;
		a = ( r - l ) / ( t - b );
		//if( abs( ar - a ) < 0.00001 ) return;
		dx = ar * (1. / a );
		c->setProjectionMatrixAsFrustum( l * dx, r * dx, b, t, n, f );
		if( vportU_ != 0 ) vportU_->set( osg::Vec4( v->x(), v->y(), v->width(), v->height() ) );
	}
private:
	mutable double ar;
	mutable double l, r, b, t, n, f;
	mutable double a;
	mutable double dx;
	osg::ref_ptr< osg::Uniform > vportU_;
};


//------------------------------------------------------------------------------
// Synchronize

// sync viewer's camera with pre-render camera
class SyncCameraNode : public osg::Camera::DrawCallback //osg::NodeCallback
{
public:
	SyncCameraNode( const osg::Camera* observedCamera, osg::Camera* cameraToUpdate, osg::Uniform* vp )
		: observedCamera_( observedCamera ), cameraToUpdate_( cameraToUpdate ), uniform_( vp ), init_( true ) {}
	void operator()( osg::Node* n, osg::NodeVisitor* )
    {
        SyncCameras();
    }
	void operator() ( osg::RenderInfo& /*renderInfo*/ ) const
    {
        SyncCameras();
	}
    void SyncCameras() const
    {

		osg::ref_ptr< osg::Camera > sc = cameraToUpdate_; 
		sc->setProjectionMatrix( observedCamera_->getProjectionMatrix() );
		sc->setViewMatrix( observedCamera_->getViewMatrix() );
		osg::Viewport* v =  const_cast< osg::Camera* >( osg::get_pointer( observedCamera_ ) )->getViewport();

		if( init_ )
		{
			// useful when FBO attached since FBO is not resized and needs to be
			// set to the desired maximum size in advance
			sc->setViewport( 0, 0, 2048, 2048 );
			init_=false;
		}

		else if( v )
		{
			sc->setViewport( v );
			uniform_->set( osg::Vec4( v->x(), v->y(), v->width(), v->height() ) );
		}
    }
private:
	osg::ref_ptr< const osg::Camera > observedCamera_;
    osg::ref_ptr< osg::Camera > cameraToUpdate_;
	osg::ref_ptr< osg::Uniform > uniform_;
	mutable int init_;
};



#endif // ASPECT_RATIO_H_