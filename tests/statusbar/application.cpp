/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"

#include <qlabel.h>
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
#include "qpushbutton.h"

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"

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

QPopupMenu* sbmenu;
QPopupMenu* msg;

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window", WDestructiveClose )
{
    int id;

    printer = new QPrinter;
    QPixmap openIcon, saveIcon, printIcon;

    fileTools = new QToolBar( this, "file operations" );

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

    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );

    file->insertItem( "&New", this, SLOT(newDoc()), CTRL+Key_N );

    id = file->insertItem( openIcon, "&Open",
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

    sbmenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Status Bar", sbmenu );
    sbmenu->setCheckable( TRUE );
    sbmenu->insertItem( "Normal Item 0", this, SLOT(doItem(int)), 0, 0 );
    sbmenu->insertItem( "Normal Item 1", this, SLOT(doItem(int)), 0, 1 );
    sbmenu->insertItem( "Normal Item 2", this, SLOT(doItem(int)), 0, 2 );
    sbmenu->insertItem( "Permanent Item 10", this, SLOT(doItem(int)), 0, 10 );
    sbmenu->insertItem( "Permanent Item 11", this, SLOT(doItem(int)), 0, 11 );
    sbmenu->insertItem( "Permanent Item 12", this, SLOT(doItem(int)), 0, 12 );
    sbmenu->insertItem( "Push Button", this, SLOT(doItem(int)), 0, 3 );

    msg = new QPopupMenu( this );
    menuBar()->insertItem( "&Messages", msg );
    msg->setCheckable( TRUE );
    msg->insertItem( "temp message", this, SLOT(doMsg(int)), 0, 0 );
    msg->insertItem( "perm message", this, SLOT(doMsg(int)), 0, 1 );
    msg->insertItem( "clear message", this, SLOT(doMsg(int)), 0, 2 );
    

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About&Qt", this, SLOT(aboutQt()), CTRL + Key_U);
    help->insertSeparator();
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1);

    e = new QMultiLineEdit( this, "editor" );
    e->setFocus();
    setCentralWidget( e );

    statusBar()->message( "Ready", 2000 );

    resize( 450, 600 );
}


ApplicationWindow::~ApplicationWindow()
{
    delete printer;
}


void ApplicationWindow::doMsg( int i )
{
    qDebug( "DoMsg: %i", i );
    
    switch( i ) {
    case 0:
	statusBar()->message( "This is a temporary message", 1000 );
	break;
    case 1:
	statusBar()->message( "This is a permanent message" );
	break;
    case 2:
	statusBar()->clear();
	break;
    default:
	qDebug( "Error in test program!" );
    }	
}


void ApplicationWindow::doItem( int i )
{
    qDebug( "DoItem: %i", i );
    static QWidget* sbItems[20];
    if ( !sbmenu->isItemChecked( i ) ) {
	QString lbs = QString( "Real Long Item " ) + QString().setNum( i );
	QWidget* lb;
	if ( i != 3 )
	    lb = new QLabel( lbs, statusBar(), "auto-label" );
	else
	    lb = new QPushButton( "Push me!", statusBar(), "auto-buton" );
	lb->show();
	statusBar()->addWidget( lb, 0, (i >= 10) );
	
	sbItems[i] = lb;
	sbmenu->setItemChecked( i, TRUE );
    }
    else {
	statusBar()->removeWidget( sbItems[i] );
	delete sbItems[i];
	sbItems[i] = 0;
	sbmenu->setItemChecked( i, FALSE );
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
    const int Margin = 10;
    int pageNo = 1;

    if ( printer->setup(this) ) {		// printer dialog
	statusBar()->message( "Printing..." );
	QPainter p;
	p.begin( printer );			// paint on printer
	p.setFont( e->font() );
	int yPos	= 0;			// y position for each line
	QFontMetrics fm = p.fontMetrics();
	QPaintDeviceMetrics metrics( printer ); // need width/height
						// of printer surface
	for( int i = 0 ; i < e->numLines() ; i++ ) {
	    if ( Margin + yPos > metrics.height() - Margin ) {
		QString msg( "Printing (page " );
		msg += QString::number( ++pageNo );
		msg += ")...";
		statusBar()->message( msg );
		printer->newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( Margin, Margin + yPos,
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			e->textLine( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
	statusBar()->message( "Printing completed", 2000 );
    } else {
	statusBar()->message( "Printing aborted", 2000 );
    }

}

void ApplicationWindow::closeEvent( QCloseEvent* ce )
{
    if ( !e->edited() ) {
	ce->accept();
	return;
    }

    switch( QMessageBox::information( this, "Qt Application Example",
				      "The document has been changed since "
				      "the last save.",
				      "Save Now", "Cancel", "Leave Anyway",
				      0, 1 ) ) {
    case 0:
	save();
	ce->accept();
	break;
    case 1:
    default: // just for sanity
	ce->ignore();
	break;
    case 2:
	ce->accept();
	break;
    }
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
