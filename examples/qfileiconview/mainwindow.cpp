/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/mainwindow.cpp#7 $
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
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qdir.h>
#include <qfileinfo.h>

static const char* cdtoparent_xpm[]={
    "15 13 3 1",
    ". c None",
    "* c #000000",
    "a c #ffff99",
    "..*****........",
    ".*aaaaa*.......",
    "***************",
    "*aaaaaaaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaa***aaaaaaa*",
    "*aa*****aaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa******aaa*",
    "*aaaaaaaaaaaaa*",
    "*aaaaaaaaaaaaa*",
    "***************"};

static const char* newfolder_xpm[] = {
    "15 14 4 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFF00",
    "@	c #FFFFFF",
    "          .    ",
    "               ",
    "          .    ",
    "       .     . ",
    "  ....  . . .  ",
    " .+@+@.  . .   ",
    "..........  . .",
    ".@+@+@+@+@..   ",
    ".+@+@+@+@+. .  ",
    ".@+@+@+@+@.  . ",
    ".+@+@+@+@+.    ",
    ".@+@+@+@+@.    ",
    ".+@+@+@+@+.    ",
    "...........    "};

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

    dirlist = new DirectoryView( splitter, "dirlist", TRUE );
    dirlist->addColumn( "Name" );
    dirlist->addColumn( "Type" );
    Directory *root = new Directory( dirlist, "/" );
    root->setOpen( TRUE );
    splitter->setResizeMode( dirlist, QSplitter::KeepSize );

    fileview = new QtFileIconView( "/", splitter );
    fileview->setSelectionMode( QIconView::StrictMulti );
    fileview->setViewMode( QIconSet::Large );

    setCentralWidget( splitter );

    QToolBar *toolbar = new QToolBar( this, "toolbar" );
    setRightJustification( TRUE );

    (void)new QLabel( tr( " Path: " ), toolbar );

    pathCombo = new QComboBox( TRUE, toolbar );
    toolbar->setStretchableWidget( pathCombo );
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
             this, SLOT ( changePath( const QString & ) ) );

    toolbar->addSeparator();

    QPixmap pix;

    pix = QPixmap( cdtoparent_xpm );
	(void)new QToolButton( pix, "One directory up", QString::null,
                           this, SLOT( cdUp() ), toolbar, "cd up" );

    pix = QPixmap( newfolder_xpm );
	(void)new QToolButton( pix, "New Folder", QString::null,
                           this, SLOT( newFolder() ), toolbar, "new folder" );

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

void FileMainWindow::setPathCombo()
{
    QString dir = caption();
    int i = 0;
    bool found = FALSE;
    for ( i = 0; i < pathCombo->count(); ++i ) {
        if ( pathCombo->text( i ) == dir) {
            found = TRUE;
            break;
        }
    }

    if ( found )
        pathCombo->setCurrentItem( i );
    else {
        pathCombo->insertItem( dir );
        pathCombo->setCurrentItem( pathCombo->count() - 1 );
    }

}

void FileMainWindow::directoryChanged( const QString &dir )
{
    setCaption( dir );
    setPathCombo();
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

void FileMainWindow::cdUp()
{
    QDir dir = fileview->currentDir();
    dir.cd( ".." );
    fileview->setDirectory( dir );
}

void FileMainWindow::newFolder()
{
    fileview->newDirectory();
}

void FileMainWindow::changePath( const QString &path )
{
    if ( QFileInfo( path ).exists() )
        fileview->setDirectory( path );
    else
        setPathCombo();
}
