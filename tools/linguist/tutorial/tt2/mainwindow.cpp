/****************************************************************
**
** Implementation of MainWindow class, translation tutorial 2
**
****************************************************************/

#include "arrowpad.h"
#include "mainwindow.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qmenu.h>

MainWindow::MainWindow( QWidget *parent, const char *name )
    : QMainWindow( parent)
{
    setObjectName(name);
    ArrowPad *ap = new ArrowPad( this );
    setCenterWidget( ap );

    QMenu *file = new QMenu( this );
    QAction *exitAction = file->addAction( tr("E&xit"), qApp, SLOT(quit()));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    
    menuBar()->addMenu(file)->setText(tr("&File"));
}
