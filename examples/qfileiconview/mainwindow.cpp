/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/mainwindow.cpp#4 $
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
#include <qprogressbar.h>
#include <qlabel.h>
#include <qstatusbar.h>

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
    connect( fileview, SIGNAL( startReadDir( int ) ),
             this, SLOT( slotStartReadDir( int ) ) );
    connect( fileview, SIGNAL( readNextDir() ),
             this, SLOT( slotReadNextDir() ) );
    connect( fileview, SIGNAL( readDirDone() ),
             this, SLOT( slotReadDirDone() ) );
    connect( fileview, SIGNAL( selectionChanged( int ) ),
             this, SLOT( slotNumItemsSelected( int ) ) );

    progress = new QProgressBar( statusBar() );
    statusBar()->addWidget( progress, TRUE );
    label = new QLabel( statusBar() );
    statusBar()->addWidget( label, TRUE );
}

void FileMainWindow::directoryChanged( const QString &dir )
{
    setCaption( dir );
}

void FileMainWindow::slotStartReadDir( int dirs )
{
    label->setText( tr( " Reading Directory..." ) );
    progress->reset();
    progress->setTotalSteps( dirs );
}

void FileMainWindow::slotReadNextDir()
{
    int p = progress->progress();
    progress->setProgress( ++p );
}

void FileMainWindow::slotReadDirDone()
{
    label->setText( tr( " Reading Directory Done." ) );
    progress->setProgress( progress->totalSteps() );
}

void FileMainWindow::slotNumItemsSelected( int num )
{
    if ( num == 1 )
        label->setText( tr( " %1 Item Selected" ).arg( num ) );
    else
        label->setText( tr( " %1 Items Selected" ).arg( num ) );
}
