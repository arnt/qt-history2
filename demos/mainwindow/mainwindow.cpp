#include "mainwindow.h"

#include "colorswatch.h"
#include "toolbar.h"

#include <qaction.h>
// #include <qlabel.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qstatusbar.h>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    setObjectName("MainWindow");

    setupActions();
    setupToolBar();
    setupMenuBar();
    setupDockWindows();

    QWidget *center = new QWidget(this);
    center->setBackgroundRole(QPalette::Mid);

    QFrame *center2 = new QFrame(center);
    center2->setFrameStyle(QFrame::Panel | QFrame::Raised);
    center2->setBackgroundRole(QPalette::Background);
    center2->setMinimumSize(300, 200);

    QHBoxLayout *hbox = new QHBoxLayout(center);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    hbox->setAlignment(Qt::AlignCenter);
    hbox->addWidget(center2);

    setCenterWidget(center);

    statusBar()->message(tr("Status Bar"));
}

void MainWindow::actionTriggered(QAction *action)
{
    qDebug("action '%s' triggered", action->text().local8Bit());
}


void MainWindow::setupActions()
{
    dockWindowActions = new QAction(tr("Dock Windows"), this);
}

void MainWindow::setupToolBar()
{
    toolbar = new ToolBar(this);
    toolbar->setAllowedAreas(Qt::ToolBarAreaTop | Qt::ToolBarAreaBottom);
}

void MainWindow::setupMenuBar()
{
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Close"), this, SLOT(close()));

    QMenu *dockWindowMenu = new QMenu(this);
    dockWindowActions->setMenu(dockWindowMenu);

    menuBar()->addMenu(tr("File"), menu);
    menuBar()->addMenu(tr("Tool Bar"), toolbar->menu);
    menuBar()->addAction(dockWindowActions);
}

void MainWindow::setupDockWindows()
{
    static const struct Set {
        const char * name;
        Qt::WFlags flags;
        Qt::DockWindowArea area;
        uint allowedAreas;
        uint features;
    } sets [] = {
        { "Black", Qt::WMacDrawer, Qt::DockWindowAreaLeft,
          Qt::DockWindowAreaLeft | Qt::DockWindowAreaRight,
          QDockWindow::DockWindowClosable },

        { "White", 0, Qt::DockWindowAreaRight,
          Qt::DockWindowAreaLeft | Qt::DockWindowAreaRight,
          QDockWindow::AllDockWindowFeatures },

        { "Red", 0, Qt::DockWindowAreaTop,
          Qt::AllDockWindowAreas,
          QDockWindow::DockWindowClosable | QDockWindow::DockWindowMovable },
        { "Green", 0, Qt::DockWindowAreaTop,
          Qt::AllDockWindowAreas,
          QDockWindow::DockWindowClosable | QDockWindow::DockWindowMovable },

        { "Blue", 0, Qt::DockWindowAreaBottom,
          Qt::AllDockWindowAreas,
          QDockWindow::DockWindowClosable | QDockWindow::DockWindowMovable },
        { "Yellow", 0, Qt::DockWindowAreaBottom,
          Qt::AllDockWindowAreas,
          QDockWindow::DockWindowClosable | QDockWindow::DockWindowMovable }
    };
    const int setCount = sizeof(sets) / sizeof(Set);

    for (int i = 0; i < setCount; ++i) {
        ColorSwatch *swatch = new ColorSwatch(tr(sets[i].name), this, sets[i].flags);
        swatch->setAllowedAreas(Qt::DockWindowAreas(sets[i].allowedAreas));
        swatch->setFeatures(QDockWindow::DockWindowFeatures(sets[i].features));
        swatch->setArea(sets[i].area);

        dockWindowActions->menu()->addMenu(tr(sets[i].name), swatch->menu);
    }
}
