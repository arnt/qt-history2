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
    setCentralWidget( pp );

    QMenu *file = new QMenu( this );
    QAction *exitAction = file->addAction( tr("E&xit"), qApp, SLOT(quit()));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    QMenu *help = new QMenu( this );
    QAction *aboutAction = help->addAction( tr("&About"), this, SLOT(about()));
    aboutAction->setShortcut(Qt::Key_F1);
    help->addAction( tr("About &Qt"), this, SLOT(aboutQt()) );

    menuBar()->addMenu(file)->setText(tr("&File"));
    menuBar()->addMenu(help)->setText(tr("&Help"));
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
