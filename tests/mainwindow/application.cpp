/****************************************************************************
** $Id: //depot/qt/main/tests/mainwindow/application.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"

#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qkeycode.h>
#include <qmultilineedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qobjectlist.h>
#include <qmap.h>

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"
#include "filesave2.xpm"
#include "fileopen2.xpm"
#include "fileprint2.xpm"

const char * fileOpenText = "<img source=\"fileopen\"> "
"Click this button to open a <em>new file</em>. <br><br>"
"You can also select the <b>Open command</b> from the File menu.";
const char * fileSaveText = "Click this button to save the file you are "
"editing.  You will be prompted for a file name.\n\n"
"You can also select the Save command from the File menu.\n\n"
"Note that implementing this function is left as an exercise for the reader.";
const char * filePrintText = "Click this button to print the file you "
"are editing.\n\n"
"You can also select the Print command from the File menu.";

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window", WDestructiveClose )
{

    QFile f( ".tbconfig" );
    bool tbconfig = f.open( IO_ReadOnly );
    QMap< QString, int > docks;
    QMap< QString, int > indices;
    if ( tbconfig ) {
	QDataStream s( &f );
	s >> docks;
	s >> indices;
    }
    
    // Tool bar 1:

    QPixmap openIcon, saveIcon, printIcon;

    QToolBar* fileTools = new QToolBar( this, "file operations" );

    openIcon = QPixmap( fileopen );
    QToolButton * fileOpen
	= new QToolButton( openIcon, "Open File", QString::null,
			   this, SLOT(load()), fileTools, "open file" );

    saveIcon = QPixmap( filesave );
    QToolButton * fileSave
	= new QToolButton( saveIcon, "Save File", QString::null,
			   this, SLOT(save()), fileTools, "save file" );

    printIcon = QPixmap( fileprint );
    QToolButton * filePrint
	= new QToolButton( printIcon, "Print File", QString::null,
			   this, SLOT(print()), fileTools, "print file" );

    (void)QWhatsThis::whatsThisButton( fileTools );

    QWhatsThis::add( fileOpen, fileOpenText );
    QMimeSourceFactory::defaultFactory()->setPixmap( "fileopen", openIcon );
    QWhatsThis::add( fileSave, fileSaveText );
    QWhatsThis::add( filePrint, filePrintText );

    if ( docks.contains( fileTools->name() ) &&
	 indices.contains( fileTools->name() ) ) 
	moveToolBar( fileTools, (ToolBarDock)docks[ fileTools->name() ],
		     indices[ fileTools->name() ] );
        
    // Tool bar 2:

    QPixmap openIcon2, saveIcon2, printIcon2;

    QToolBar* fileTools2 = new QToolBar( this, "file2 operations" );

    openIcon2 = QPixmap( fileopen2 );
    (void)new QToolButton( openIcon2, "Open File2", QString::null,
			   this, SLOT(load2()), fileTools2, "open file2" );

    saveIcon2 = QPixmap( filesave2 );
    (void)new QToolButton( saveIcon2, "Save File2", QString::null,
			   this, SLOT(save2()), fileTools2, "save file2" );

    printIcon2 = QPixmap( fileprint2 );
    (void)new QToolButton( printIcon2, "Print File", QString::null,
			   this, SLOT(print2()), fileTools2, "print file2" );

    addToolBar( fileTools2, "FILETOOLS2", Top, FALSE );

    if ( docks.contains( fileTools2->name() ) &&
	 indices.contains( fileTools2->name() ) ) 
	moveToolBar( fileTools2, (ToolBarDock)docks[ fileTools2->name() ],
		     indices[ fileTools2->name() ] );

    // Tool bar 3:

    fileTools2 = new QToolBar( this, "file3 operations" );

    openIcon2 = QPixmap( fileopen2 );
    (void)new QToolButton( openIcon2, "Open File2", QString::null,
			   this, SLOT(load2()), fileTools2, "open file2" );

    saveIcon2 = QPixmap( filesave2 );
    (void)new QToolButton( saveIcon2, "Save File2", QString::null,
			   this, SLOT(save2()), fileTools2, "save file2" );

    printIcon2 = QPixmap( fileprint2 );
    (void)new QToolButton( printIcon2, "Print File", QString::null,
			   this, SLOT(print2()), fileTools2, "print file2" );
    (void)new QToolButton( printIcon2, "Print File", QString::null,
			   this, SLOT(print2()), fileTools2, "print file2" );
    (void)new QToolButton( printIcon2, "Print File", QString::null,
			   this, SLOT(print2()), fileTools2, "print file2" );

    addToolBar( fileTools2, "FILETOOLS2", Top, FALSE );

    if ( docks.contains( fileTools2->name() ) &&
	 indices.contains( fileTools2->name() ) ) 
	moveToolBar( fileTools2, (ToolBarDock)docks[ fileTools2->name() ],
		     indices[ fileTools2->name() ] );

    // Tool bar 4:

    fileTools2 = new QToolBar( this, "file4 operations" );

    openIcon2 = QPixmap( fileopen2 );
    (void)new QToolButton( openIcon, "Open File2", QString::null,
			   this, SLOT(load2()), fileTools2, "open file2" );

    saveIcon2 = QPixmap( filesave );
    (void)new QToolButton( saveIcon, "Save File2", QString::null,
			   this, SLOT(save2()), fileTools2, "save file2" );
    (void)new QToolButton( saveIcon, "Save File2", QString::null,
			   this, SLOT(save2()), fileTools2, "save file2" );

    printIcon2 = QPixmap( fileprint );
    (void)new QToolButton( printIcon2, "Print File", QString::null,
			   this, SLOT(print2()), fileTools2, "print file2" );

    addToolBar( fileTools2, "FILETOOLS2", Top, FALSE );

    if ( docks.contains( fileTools2->name() ) &&
	 indices.contains( fileTools2->name() ) ) 
	moveToolBar( fileTools2, (ToolBarDock)docks[ fileTools2->name() ],
		     indices[ fileTools2->name() ] );

    // Menus:

    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );

    file->insertItem( "&New", this, SLOT(newDoc()), CTRL+Key_N );

    int id = file->insertItem( openIcon, "&Open",
			   this, SLOT(load()), CTRL+Key_O );
    file->setWhatsThis( id, fileOpenText );

    id = file->insertItem( saveIcon, "&Save",
			   this, SLOT(save()), CTRL+Key_S );
    file->setWhatsThis( id, fileSaveText );
    id = file->insertItem( "Save &as...", this, SLOT(saveAs()) );
    file->setWhatsThis( id, fileSaveText );
    file->insertSeparator();
    id = file->insertItem( printIcon, "&Print",
			   this, SLOT(print()), CTRL+Key_P );
    file->setWhatsThis( id, filePrintText );
    file->insertSeparator();
    file->insertItem( "&Close", this, SLOT(close()), CTRL+Key_W );
    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );


    appMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&App", appMenu );

    appMenu->setCheckable( TRUE );

    justId = appMenu->insertItem( "&RightJustify", this,
				  SLOT(toggleJust()), CTRL+Key_J );
    just = FALSE;

    bigpixId = appMenu->insertItem( "&Big Pixmaps", this,
				    SLOT(toggleBigpix()), CTRL+Key_B );
    bigpix = FALSE;

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About&Qt", this, SLOT(aboutQt()));
    help->insertSeparator();
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1);



    // Central widget:

    e = new QMultiLineEdit( this, "editor" );
    e->setFocus();
    setCentralWidget( e );


    // Start:

    statusBar()->message( "Ready", 2000 );
    resize( 450, 600 );
}


ApplicationWindow::~ApplicationWindow()
{
    QObjectList *l = queryList( "QToolBar" );
    if ( l && l->first() ) {
	QToolBar *tb = 0;
	QMap< QString, int > docks;
	QMap< QString, int > indices;
	QMainWindow::ToolBarDock dock;
	int index;
	for ( tb = (QToolBar*)l->first(); tb; tb = (QToolBar*)l->next() ) {
	    if ( findDockAndIndexOfToolbar( tb, dock, index ) ) {
		docks[ tb->name() ] = dock;
		indices[ tb->name() ] = index;
	    }
	}
	QFile file( ".tbconfig" );
	file.open( IO_WriteOnly );
	QDataStream s( &file );
	s << docks;
	s << indices;
    }
}



void ApplicationWindow::newDoc()
{
    ApplicationWindow *ed = new ApplicationWindow;
    ed->show();
}

void ApplicationWindow::load()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null,
					       this);
    if ( !fn.isEmpty() )
	load( fn );
    else
	statusBar()->message( "Loading aborted", 2000 );
}


void ApplicationWindow::load( const char *fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
	return;

    e->setAutoUpdate( FALSE );
    e->clear();

    QTextStream t(&f);
    while ( !t.eof() ) {
	QString s = t.readLine();
	e->append( s );
    }
    f.close();

    e->setAutoUpdate( TRUE );
    e->repaint();
    e->setEdited( FALSE );
    setCaption( fileName );
    QString s;
    s.sprintf( "Loaded document %s", fileName );
    statusBar()->message( s, 2000 );
}


void ApplicationWindow::save()
{
    if ( filename.isEmpty() ) {
	saveAs();
	return;
    }

    QString text = e->text();
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
	statusBar()->message( QString("Could not write to %1").arg(filename),
			      2000 );
	return;
    }

    QTextStream t( &f );
    t << text;
    f.close();

    e->setEdited( FALSE );

    setCaption( filename );

    statusBar()->message( QString( "File %1 saved" ).arg( filename ), 2000 );
}


void ApplicationWindow::saveAs()
{
    QString fn = QFileDialog::getSaveFileName( QString::null, QString::null,
					       this );
    if ( !fn.isEmpty() ) {
	filename = fn;
	save();
    } else {
	statusBar()->message( "Saving aborted", 2000 );
    }
}


void ApplicationWindow::print()
{
    statusBar()->message( "Would have done printing now...", 2000 );
}


void ApplicationWindow::about()
{
    QMessageBox::about( this, "Qt Application Example",
			"This example demonstrates simple use of "
			"QMainWindow,\nQMenuBar and QToolBar.");
}


void ApplicationWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Application Example" );
}


void ApplicationWindow::load2()
{
    debug( "load2" );
}

void ApplicationWindow::save2()
{
    debug( "save2" );
}

void ApplicationWindow::print2()
{
    debug( "print2" );
}


void ApplicationWindow::toggleJust()
{
    debug( "toggleJust" );
    just = !just;
    appMenu->setItemChecked( justId, just );
    setRightJustification( just );
}


void ApplicationWindow::toggleBigpix()
{
    debug( "toggleBigpix" );
    bigpix = !bigpix;
    appMenu->setItemChecked( bigpixId, bigpix );
    setUsesBigPixmaps( bigpix );
}
