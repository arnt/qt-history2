/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/mainwindow.cpp#2 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "mainwindow.h"
#include "qfileiconview.h"
#include "../dirview/dirview.h"

#include <qsplitter.h>

FileMainWindow::FileMainWindow()
    : QMainWindow()
{
    resize( 640, 480 );
    setup();
}

void FileMainWindow::show()
{
    QMainWindow::show();
    fileview->setDirectory( "/" );
}

void FileMainWindow::setup()
{
    QSplitter *splitter = new QSplitter( this );

    dirlist = new DirectoryView( splitter, "dirlist", true );
    dirlist->addColumn( "Name" );
    dirlist->addColumn( "Type" );
    Directory *root = new Directory( dirlist, "/" );
    root->setOpen( TRUE );
    splitter->setResizeMode( dirlist, QSplitter::KeepSize );

    fileview = new QtFileIconView( "/", splitter );
    fileview->setSelectionMode( QtIconView::StrictMulti );
    fileview->setViewMode( QIconSet::Large );

    setCentralWidget( splitter );

    connect( dirlist, SIGNAL( folderSelected( const QString & ) ),
             fileview, SLOT ( setDirectory( const QString & ) ) );
    connect( fileview, SIGNAL( directoryChanged( const QString & ) ),
             this, SLOT( directoryChanged( const QString & ) ) );
}

void FileMainWindow::directoryChanged( const QString &dir )
{
    setCaption( dir );
}
