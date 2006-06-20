/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mainwindow.h"
#include "colorswatch.h"
#include "toolbar.h"

#include <QAction>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>

static const char * const message =
    "<p><b>Qt Main Window Demo</b></p>"

    "<p>This is a demonstration of the QMainWindow, QToolBar and "
    "QDockWidget classes.</p>"

    "<p>The tool bar and dock widgets can be dragged around and rearranged "
    "using the mouse or via the menu.</p>"

    "<p>Each dock widget contains a colored frame and a context "
    "(right-click) menu.</p>"

#ifdef Q_WS_MAC
    "<p>On Mac OS X, the \"Black\" dock widget has been created as a "
    "<em>Drawer</em>, which is a special kind of QDockWidget.</p>"
#endif
    ;

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    setObjectName("MainWindow");
    setWindowTitle("Qt Main Window Demo");

    setupToolBar();
    setupMenuBar();
    setupDockWidgets();

    QTextEdit *center = new QTextEdit(this);
    center->setReadOnly(true);
    center->setHtml(tr(message));
    center->setMinimumSize(400, 205);
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
    dockWidgetMenu = menuBar()->addMenu(tr("&Dock Widgets"));
}

void MainWindow::setupDockWidgets()
{
    QAction *action = dockWidgetMenu->addAction(tr("Animation"));
    action->setCheckable(true);
    action->setChecked(isAnimationEnabled());
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setAnimationEnabled(bool)));

    action = dockWidgetMenu->addAction(tr("Nesting"));
    action->setCheckable(true);
    action->setChecked(isDockNestingEnabled());
    connect(action, SIGNAL(toggled(bool)), this, SLOT(setDockNestingEnabled(bool)));

    static const struct Set {
        const char * name;
        uint flags;
        Qt::DockWidgetArea area;
    } sets [] = {
#ifndef Q_WS_MAC
        { "Black", 0, Qt::LeftDockWidgetArea },
#else
        { "Black", Qt::Drawer, Qt::LeftDockWidgetArea },
#endif
        { "White", 0, Qt::LeftDockWidgetArea },
        { "LightGrey", 0, Qt::RightDockWidgetArea },
        { "DarkGrey", 0, Qt::RightDockWidgetArea },
        { "Red", 0, Qt::TopDockWidgetArea },
        { "Green", 0, Qt::TopDockWidgetArea },
        { "Blue", 0, Qt::BottomDockWidgetArea },
        { "Yellow", 0, Qt::BottomDockWidgetArea }
    };
    const int setCount = sizeof(sets) / sizeof(Set);

    for (int i = 0; i < setCount; ++i) {
        ColorSwatch *swatch = new ColorSwatch(tr(sets[i].name), this, Qt::WFlags(sets[i].flags));
        if (i%2)
            swatch->setWindowIcon(QIcon(QPixmap(":/res/qt.png")));
        addDockWidget(sets[i].area, swatch);
        dockWidgetMenu->addMenu(swatch->menu);
    }
}
