/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
#include <qpushbutton.h>
#include <qcombobox.h>

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

QToolBar *myTb;

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window", WDestructiveClose ),
      cb( 0 ), pb( 0 )
{
    // Toolbars

    QFile f( ".tbconfig" );
    bool tbconfig = f.open( IO_ReadOnly );
    QMap< QString, int > docks;
    QMap< QString, int > indices;
    QMap< QString, int > nls;
    QMap< QString, int > eos;
    int w = 450, h = 600;
    if ( tbconfig ) {
	QDataStream s( &f );
	s >> docks;
	s >> indices;
	s >> nls;
	s >> eos;
	s >> w >> h;
    }

    QString toolBars[] = {
	"file operations",
	"file2 operations",
	"file3 operations",
	"file4 operations"
    };

    if ( tbconfig ) {
	QMap< QString, int >::Iterator dit, iit;
	QMap< QString, int >::Iterator nit, eit;
	dit = docks.begin();
	iit = indices.begin();
	nit = nls.begin();
	eit = eos.begin();
	for ( ; dit != docks.end(); ++dit, ++iit, ++nit, ++eit ) {
	    QToolBar *tb = createToolbar( dit.key().mid( 1, 0xFFFFFF ), FALSE );
	    moveToolBar( tb, (ToolBarDock)*dit, (bool)*nit, *iit, *eit );
	}
    } else {
	for ( unsigned int i = 0; i < 4; ++i )
	    createToolbar( toolBars[ i ], TRUE );
    }

    // Menus:

    QPixmap openIcon, saveIcon, printIcon;
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


    QPopupMenu* appMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&App", appMenu );

    appMenu->setCheckable( TRUE );

    justId = appMenu->insertItem( "Right &Justify", this,
				  SLOT(toggleJust()), CTRL+Key_J );
    bigpixId = appMenu->insertItem( "&Big Pixmaps", this,
				    SLOT(toggleBigpix()), CTRL+Key_B );
    textlabelid = appMenu->insertItem( "&Text Labels", this,
				    SLOT(toggleTextLabel()), CTRL+Key_T );
    opaqueId = appMenu->insertItem( "&Opaque Toolbar moving", this,
				    SLOT(toggleOpaque()), ALT+Key_O );
    appMenu->insertItem( "&Show/Hide first toolbar", this,
			 SLOT(hideToolbar()), ALT+Key_H );

    appMenu->insertSeparator();
    fullScreenId = appMenu->insertItem( "&Full Screen", this,
					  SLOT(toggleFullScreen()), CTRL+Key_F );

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
    resize( w, h );
}


QToolBar* ApplicationWindow::createToolbar( const QString &name, bool nl )
{
    QPixmap openIcon, saveIcon, printIcon;
    QPixmap openIcon2, saveIcon2, printIcon2;

    if ( name == "file operations" ) {
	QToolBar* fileTools = new QToolBar( this, "file operations" );
	myTb = fileTools;
	
	openIcon = QPixmap( fileopen );

 	pb = new QPushButton( fileTools );
 	pb->setText("Hallo");
	QPopupMenu* popup = new QPopupMenu( this );
	popup->insertItem("Eins");
	popup->insertItem("Zwei");
	popup->insertItem("Drei");
	popup->insertItem("Vier");
 	pb->setPopup( popup );

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
	addToolBar( fileTools, "Toolbar 1", Top, FALSE );
	connect( fileTools, SIGNAL( orientationChanged( Orientation ) ),
		 this, SLOT( orientationChanged() ) );
	return fileTools;
    } else if ( name == "file2 operations" ) {
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

	cb = new QComboBox( TRUE, fileTools2 );
	fileTools2->setStretchableWidget( cb );
	addToolBar( fileTools2, "Toolbar 2", Top, FALSE );
	connect( fileTools2, SIGNAL( orientationChanged( Orientation ) ),
		 this, SLOT( orientationChanged() ) );
	return fileTools2;
    } else if ( name == "file3 operations" ) {
	QToolBar *fileTools3 = new QToolBar( this, "file3 operations" );

	openIcon2 = QPixmap( fileopen2 );
	(void)new QToolButton( openIcon2, "Open File2", QString::null,
			       this, SLOT(load2()), fileTools3, "open file2" );

	saveIcon2 = QPixmap( filesave2 );
	(void)new QToolButton( saveIcon2, "Save File2", QString::null,
			       this, SLOT(save2()), fileTools3, "save file2" );

	printIcon2 = QPixmap( fileprint2 );
	(void)new QToolButton( printIcon2, "Print File", QString::null,
			       this, SLOT(print2()), fileTools3, "print file2" );
	(void)new QToolButton( printIcon2, "Print File", QString::null,
			       this, SLOT(print2()), fileTools3, "print file2" );
	(void)new QToolButton( printIcon2, "Print File", QString::null,
			       this, SLOT(print2()), fileTools3, "print file2" );

	addToolBar( fileTools3, "Toolbar 3", Top, nl );
	return fileTools3;
    } else if ( name == "file4 operations" ) {
	QToolBar* fileTools4 = new QToolBar( this, "file4 operations" );

	openIcon = QPixmap( fileopen );
	(void)new QToolButton( openIcon, "Open File2", QString::null,
			       this, SLOT(load2()), fileTools4, "open file2" );

	saveIcon = QPixmap( filesave );
	(void)new QToolButton( saveIcon, "Save File2", QString::null,
			       this, SLOT(save2()), fileTools4, "save file2" );
	(void)new QToolButton( saveIcon, "Save File2", QString::null,
			       this, SLOT(save2()), fileTools4, "save file2" );

	printIcon2 = QPixmap( fileprint );
	(void)new QToolButton( printIcon2, "Print File", QString::null,
			       this, SLOT(print2()), fileTools4, "print file2" );

	fileTools4->setVerticalStretchable( TRUE );
	addToolBar( fileTools4, "Toolbar 4", Top, FALSE );
	return fileTools4;
    }

    return 0;
}


ApplicationWindow::~ApplicationWindow()
{
    QPtrList<QToolBar> lst;
    ToolBarDock da[] = { Left, Right, Top, Bottom, Minimized };
    QMap< QString, int > docks;
    QMap< QString, int > indices;
    QMap< QString, int > nls;
    QMap< QString, int > eos;
    int j = 0;
    for ( unsigned int i = 0; i < 5; ++i ) {
	lst = toolBars( da[ i ] );
	QToolBar *tb = lst.first();
	while ( tb ) {
	    ToolBarDock dock;
	    int index;
	    bool nl;
	    int extraOffset;
	    if ( getLocation( tb, dock, index, nl, extraOffset ) ) {
		docks[ QString::number( j ) + tb->name() ] = dock;
		indices[ QString::number( j ) + tb->name() ] = index;
		nls[ QString::number( j ) + tb->name() ] = nl;
		eos[ QString::number( j ) + tb->name() ] = extraOffset;
		++j;
	    }
	    tb = lst.next();
	}
    }
    QFile file( ".tbconfig" );
    file.open( IO_WriteOnly );
    QDataStream s( &file );
    s << docks;
    s << indices;
    s << nls;
    s << eos;
    s << width() << height();
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
    setRightJustification( !rightJustification() );
    menuBar()->setItemChecked( justId, rightJustification() );
}


void ApplicationWindow::toggleBigpix()
{
    debug( "toggleBigpix" );
    setUsesBigPixmaps( !usesBigPixmaps() );
    menuBar()->setItemChecked( bigpixId, usesBigPixmaps() );
}

void ApplicationWindow::toggleTextLabel()
{
    debug( "toggleTextLabel" );
    setUsesTextLabel( !usesTextLabel() );
    menuBar()->setItemChecked( textlabelid, usesTextLabel() );
}

void ApplicationWindow::toggleOpaque()
{
    debug( "toggleOpaque" );
    setOpaqueMoving( !opaqueMoving() );
    menuBar()->setItemChecked( opaqueId, opaqueMoving() );
}

void ApplicationWindow::toggleFullScreen()
{
    debug( "toggleFullScreen" );
    bool full = !menuBar()->isItemChecked( fullScreenId );
    menuBar()->setItemChecked( fullScreenId, full );
    if ( full )
	showFullScreen();
    else
	showNormal();
}

void ApplicationWindow::hideToolbar()
{
    debug( "hide/show Toolbar" );
    if ( myTb->isVisible() )
	myTb->hide();
    else
	myTb->show();
}

void ApplicationWindow::orientationChanged()
{
    if ( sender() && sender()->inherits( "QToolBar" ) ) {
	QToolBar *tb = (QToolBar*)sender();
	if ( cb && tb == cb->parent() ) {
	    if ( tb->orientation() == Vertical )
		cb->hide();
	    else
		cb->show();
	}
	if ( pb && tb == pb->parent() ) {
	    if ( tb->orientation() == Vertical )
		pb->hide();
	    else
		pb->show();
	}
    }
}
