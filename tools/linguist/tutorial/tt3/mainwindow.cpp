/****************************************************************
**
** Implementation of MainWindow class, translation tutorial 3
**
****************************************************************/

#include "mainwindow.h"
#include "printpanel.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qmenu.h>

MainWindow::MainWindow( QWidget *parent, const char *name )
    : QMainWindow( parent )
{
    setObjectName(name);
    setWindowTitle( tr("Troll Print 1.0") );

    PrintPanel *pp = new PrintPanel( this );
    setCenterWidget( pp );

    QMenu *file = new QMenu( this );
    file->addAction( tr("E&xit"), qApp, SLOT(quit()),
                      tr("Ctrl+Q", "Quit") );
    QMenu *help = new QMenu( this );
    help->addAction( tr("&About"), this, SLOT(about()), Key_F1 );
    help->addAction( tr("About &Qt"), this, SLOT(aboutQt()) );

    menuBar()->addMenu( tr("&File"), file );
    menuBar()->addMenu( tr("&Help"), help );
}

void MainWindow::about()
{
    QMessageBox::information( this, tr("About Troll Print 1.0"),
                   tr("Troll Print 1.0.\n\n"
                      "Copyright 1999 Macroshaft, Inc.") );
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt( this );
}
