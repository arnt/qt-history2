/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ab_mainwindow.h"
#include "ab_centralwidget.h"

#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qkeycode.h>
#include <qstatusbar.h>
#include <qapplication.h>
#include <qfiledialog.h>

ABMainWindow::ABMainWindow()
    : QMainWindow( 0L, "example addressbook application" ),
      filename( QString::null )
{
    setupMenuBar();
    setupFileTools();
    setupStatusBar();
    setupCentralWidget();
}


ABMainWindow::~ABMainWindow()
{
}

void ABMainWindow::setupMenuBar()
{
    QPopupMenu *file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );

    file->insertItem( tr( "New" ), this, SLOT( fileNew() ), CTRL + Key_N );
    file->insertItem( QPixmap( "fileopen.xpm" ), tr( "Open" ), this, SLOT( fileOpen() ), CTRL + Key_O );
    file->insertSeparator();
    file->insertItem( QPixmap( "filesave.xpm" ), tr( "Save" ), this, SLOT( fileSave() ), CTRL + Key_S );
    file->insertItem( tr( "Save As..." ), this, SLOT( fileSaveAs() ) );
    file->insertSeparator();
    file->insertItem( QPixmap( "fileprint.xpm" ), tr( "Print..." ), this, SLOT( filePrint() ), CTRL + Key_P );
    file->insertSeparator();
    file->insertItem( tr( "Close" ), this, SLOT( closeWindow() ), CTRL + Key_W );
    file->insertItem( tr( "Quit" ), qApp, SLOT( quit() ), CTRL + Key_Q );
}

void ABMainWindow::setupFileTools()
{
    //fileTools = new QToolBar( this, "file operations" );
}

void ABMainWindow::setupStatusBar()
{
    //statusBar()->message( "Ready", 2000 );
}

void ABMainWindow::setupCentralWidget()
{
    view = new ABCentralWidget( this );
    setCentralWidget( view );
}

void ABMainWindow::closeWindow()
{
    close();
}

void ABMainWindow::fileNew()
{
}

void ABMainWindow::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() ) {
        filename = fn;
        view->load( filename );
    }
}

void ABMainWindow::fileSave()
{
    if ( filename.isEmpty() ) {
        fileSaveAs();
        return;
    }

    view->save( filename );
}

void ABMainWindow::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() ) {
        filename = fn;
        fileSave();
    }
}

void ABMainWindow::filePrint()
{
}
