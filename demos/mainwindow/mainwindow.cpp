/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mainwindow.h"

#include "colorswatch.h"
#include "toolbar.h"

#include <qaction.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qtextedit.h>

static const char * const message =
    "<p><b>Qt Main Window Demo</b></p>"

    "<p>This is a demonstration of the QMainWindow, QToolBar and "
    "QDockWindow classes.</p>"

    "<p>The tool bar and dock windows can be dragged around and rearranged "
    "using the mouse or via the menu.</p>"

    "<p>The tool bar contains three different types of buttons:"
    "<ul><li>Normal button with menu (button 1).</li>"
    "<li>Normal buttons (buttons 2, 3, 4 and 5).</li>"
    "<li>Checkable buttons (buttons 6, 7 and 8).</li>"
    "</ul></p>"

    "<p>Each dock window contains a colored frame and a context "
    "(right-click) menu.</p>"

#ifdef Q_WS_MAC
    "<p>On Mac OS X, the \"Black\" dock window has been created as a "
    "<em>Drawer</em>, which is a special kind of QDockWindow.</p>"
#endif
    ;

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    setObjectName("MainWindow");
    setWindowTitle("Qt Main Window Demo");

    setupToolBar();
    setupMenuBar();
    setupDockWindows();

    QTextEdit *center = new QTextEdit(this);
    center->setReadOnly(true);
    center->setHtml(tr(message));
    center->setMinimumSize(400, 200);
    setCentralWidget(center);

    statusBar()->showMessage(tr("Status Bar"));
}

void MainWindow::actionTriggered(QAction *action)
{
    qDebug("action '%s' triggered", action->text().toLocal8Bit().data());
}

void MainWindow::setupToolBar()
{
    toolbar = new ToolBar(this);
    toolbar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBar(toolbar);
}

void MainWindow::setupMenuBar()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(tr("&Quit"), this, SLOT(close()));

    menuBar()->addMenu(toolbar->menu);
    dockWindowMenu = menuBar()->addMenu(tr("&Dock windows"));
}

void MainWindow::setupDockWindows()
{
    static const struct Set {
        const char * name;
        uint flags;
        Qt::DockWidgetArea area;
        uint allowedAreas;
        uint features;
    } sets [] = {
        { "Black", Qt::Drawer, Qt::LeftDockWidgetArea,
          Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea,
          QDockWidget::DockWidgetClosable },

        { "White", 0, Qt::RightDockWidgetArea,
          Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea,
          QDockWidget::AllDockWidgetFeatures },

        { "Red", 0, Qt::TopDockWidgetArea,
          Qt::AllDockWidgetAreas,
          QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable },
        { "Green", 0, Qt::TopDockWidgetArea,
          Qt::AllDockWidgetAreas,
          QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable },

        { "Blue", 0, Qt::BottomDockWidgetArea,
          Qt::AllDockWidgetAreas,
          QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable },
        { "Yellow", 0, Qt::BottomDockWidgetArea,
          Qt::AllDockWidgetAreas,
          QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable }
    };
    const int setCount = sizeof(sets) / sizeof(Set);

    for (int i = 0; i < setCount; ++i) {
        ColorSwatch *swatch = new ColorSwatch(tr(sets[i].name), this, Qt::WFlags(sets[i].flags));
        swatch->setAllowedAreas(Qt::DockWidgetAreas(sets[i].allowedAreas));
        swatch->setFeatures(QDockWidget::DockWidgetFeatures(sets[i].features));

        addDockWidget(sets[i].area, swatch);

        dockWindowMenu->addMenu(swatch->menu);
    }
}
