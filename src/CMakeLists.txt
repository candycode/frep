project(frepup)

cmake_minimum_required(VERSION 2.6)

#if( COMMAND cmake_policy )
#  cmake_policy( SET CMP0003 NEW )
#endif( COMMAND cmake_policy )

find_package(Qt4)
if(NOT QT4_FOUND)
  message(FATAL_ERROR "Please set path to qmake manually")
endif(NOT QT4_FOUND)
include(${QT_USE_FILE}) 

set( OSG_INCLUDE_DIR "/usr/local/include" CACHE PATH "OpenSceneGraph include path" )
set( OSG_LIB_DIR "/usr/local/lib" CACHE PATH "OpenSceneGraph library path" )

include_directories( ${OSG_INCLUDE_DIR} )
link_directories( ${OSG_LIB_DIR} )

#user interface compilation
set(SOURCES_UI MainWindow.ui)
QT4_WRAP_UI( SOURCES_UI_H ${SOURCES_UI})


set( MOC_HEADERS AdapterWidget.h mainwindow.h dialogs.h toolbars.h)
QT4_WRAP_CPP( MOC_SRCS ${MOC_HEADERS} )
#QT4_ADD_RESOURCES( SAMPLE_RC_SRCS application.qrc )

SET( SRCS 
)
#SOURCE_GROUP("Resources" application.qrc)
#SOURCE_GROUP("Shaders" FILES ${SHADERS})


ADD_EXECUTABLE( frepup 
                #WIN32
                #${SAMPLE_RC_SRCS}
                ${MOC_SRCS}
                ${SOURCES_UI}
                ${SRCS} )

target_link_libraries( frepup
debug OpenThreadsd optimized OpenThreads
debug osgd optimized osg
debug osgGAd optimized osgGA
debug osgDBd optimized osgDB
debug osgUtild optimized osgUtil
debug osgViewerd optimized osgViewer
debug osgTextd optimized osgText
${QT_LIBRARIES} 
${QT_QTOPENGL_LIBRARY_RELEASE})

LINK_DIRECTORIES(${QT_LIBRARY_DIR})
