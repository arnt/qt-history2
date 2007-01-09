#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    createMenus();
    createToolBars();
    createDockWidgets();
    //setMenuWidget(new QPushButton("Hello"));
}

void MainWindow::createMenus()
{
    //setMenuWidget(new QPushButton("Hello"));
    QMenu *menu = new QMenu("File");
    menu->addAction("Save &As");
    
    QMenuBar *bar = new QMenuBar;
    bar->addMenu(menu);
    
    setMenuWidget(new QWidget());
}

void MainWindow::createToolBars()
{
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    QToolBar *t1 = new QToolBar;
    t1->addAction(new QAction("t1", this));

    QToolBar *t2 = new QToolBar;
    t2->addAction(new QAction("t2", this));

    addToolBar(Qt::LeftToolBarArea, t1);
    addToolBar(Qt::LeftToolBarArea, t2);
}

void MainWindow::createDockWidgets()
{
    QWidget *dockWidgetContents = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QPushButton("My Button."));
    dockWidgetContents->setLayout(layout);

    QDockWidget *dockWidget = new QDockWidget(tr("Dock Widget"), this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | 
				Qt::RightDockWidgetArea);
    dockWidget->setWidget(dockWidgetContents);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget); 
}
