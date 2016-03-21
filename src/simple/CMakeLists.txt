project(frepup-simple)

cmake_minimum_required(VERSION 3.3)
set(CMAKE_BUILD_TYPE Release)
set(OSG_INCLUDE_DIR /usr/local/osg/include CACHE PATH
        "OpenSceneGraph include path" FORCE)
set(OSG_LIB_DIR /usr/local/osg/lib CACHE PATH
        "OpenSceneGraph library path" FORCE)

include_directories(${OSG_INCLUDE_DIR})
link_directories(${OSG_LIB_DIR})

set( SRCS frepup-simple.cpp frepup-shaders.cpp depth_camera.cpp
          frepup-shaders.h aspect_ratio.h )

add_executable( frepup-simple ${SRCS} )

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
