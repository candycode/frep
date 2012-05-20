#include <QtGui/QApplication>
#include <QtCore/QPointer>
#include "MainWindow.h"

int main( int argc, char **argv )

{

    QApplication app( argc, argv );

    app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );

    QPointer<testMainWin> mainWindow = new MainWindow;
    //osg::ref_ptr<CompositeViewerQOSG> compositeViewer = mainWindow

    //osg::ref_ptr<ViewQOSG> view1 = new ViewQOSG( myMainWindow->ui.graphicsView1 );
    //view1->setObjectName("ViewQOSG 1");
    //osg::ref_ptr<ViewQOSG> view2 = new ViewQOSG( myMainWindow->ui.graphicsView2 );
    //view2->setObjectName("ViewQOSG 2");
    //osg::ref_ptr<ViewQOSG> view3 = new ViewQOSG( myMainWindow->ui.graphicsView3 );
    //view3->setObjectName("ViewQOSG 3");
    //osg::ref_ptr<ViewQOSG> view4 = new ViewQOSG( myMainWindow->ui.graphicsView4 );
    //view4->setObjectName("ViewQOSG 4");

    //view1->setData( Cow );
    //view2->setData( Truck );
    //view3->setData( Spaceship );
    //view4->setData( Cessna );

    //compositeViewer->addView( view1.get() );
    //compositeViewer->addView( view2.get() );
    //compositeViewer->addView( view3.get() );
    //compositeViewer->addView( view4.get() );

    mainWindow->show();

	return app.exec();

}
