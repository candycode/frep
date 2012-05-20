project(frepup-simple)

cmake_minimum_required(VERSION 2.6)

set( OSG_INCLUDE_DIR "/usr/local/include" CACHE PATH "OpenSceneGraph include path" )
set( OSG_LIB_DIR "/usr/local/lib" CACHE PATH "OpenSceneGraph library path" )

include_directories( ${OSG_INCLUDE_DIR} )
link_directories( ${OSG_LIB_DIR} )

SET( SRCS frepup-simple.cpp frepup-shaders.cpp depth_camera.cpp frepup-shaders.h aspect_ratio.h )



ADD_EXECUTABLE( frepup-simple ${SRCS} )

target_link_libraries( frepup-simple
debug OpenThreadsd optimized OpenThreads
debug osgd optimized osg
debug osgGAd optimized osgGA
debug osgDBd optimized osgDB
debug osgUtild optimized osgUtil
debug osgViewerd optimized osgViewer
debug osgTextd optimized osgText
debug osgManipulatord osgManipulator 
)
