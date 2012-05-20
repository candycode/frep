#ifndef TESTMAINWIN_HPP_
#define TESTMAINWIN_HPP_

#include "ui_mainWindow.h"

class MainWindow : public QMainWindow // QWidget
{
    Q_OBJECT

public:
    MainWindow();

public:  // for now
    Ui::mainWindow ui;
};


#endif // TESTMAINWIN_HPP_
