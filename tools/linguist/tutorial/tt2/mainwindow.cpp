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
    : QMainWindow( parent, name )
{
    ArrowPad *ap = new ArrowPad( this, "arrow pad" );
    setCentralWidget( ap );

    QMenu *file = new QMenu( this );
    file->addAction( tr("E&xit"), qApp, SLOT(quit()),
                      tr("Ctrl+Q", "Quit") );
    menuBar()->addMenu( tr("&File"), file );
}
