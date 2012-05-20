#include "MainWindow.h"

MainWindow::MainWindow()
    : QMainWindow()
{
    ui.setupUi( this );
    connect ( ui.actionExit, SIGNAL(activated(void)), this, SLOT(close()) );

}


