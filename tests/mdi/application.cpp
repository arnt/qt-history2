/****************************************************************************
** $Id: //depot/qt/main/tests/mdi/application.cpp#5 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"
#include "qworkspace.h"

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
#include <qlabel.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qobjectlist.h>
#include <qvbox.h>

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"

#include <X11/Xlib.h>

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

    windowsMenu = new QPopupMenu( this );
    windowsMenu->setCheckable( TRUE );
    connect( windowsMenu, SIGNAL( aboutToShow() ),
	     this, SLOT( windowsMenuAboutToShow() ) );
    menuBar()->insertItem( "&Windows", windowsMenu );

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About&Qt", this, SLOT(aboutQt()));
    help->insertSeparator();
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1);

    QVBox* vb = new QVBox( this );
    vb->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    ws = new QWorkspace( vb );
    setCentralWidget( vb );
//     XSelectInput( qt_xdisplay(), w->winId(),
// 	    KeyPressMask | KeyReleaseMask |
// 	    ButtonPressMask | ButtonReleaseMask |
// 	    KeymapStateMask |
// 	    ButtonMotionMask |
// 	    EnterWindowMask | LeaveWindowMask |
// 	    FocusChangeMask |
// 	    ExposureMask |
// 	    PropertyChangeMask |
// 	    StructureNotifyMask );


    statusBar()->message( "Ready", 2000 );
    resize( 450, 600 );
}


ApplicationWindow::~ApplicationWindow()
{
    qDebug("application window deleted");
    delete printer;
}



void ApplicationWindow::newDoc()
{
    QWidget* w = new MDIWindow( ws, 0, WDestructiveClose );
    qDebug("layout = %p", w->layout() );
    qDebug("size hint = %d", w->sizeHint().height() );
    connect( w, SIGNAL( destroyed() ), this, SLOT( childDestroyed() ) );
    windows.append( w );
    w->setCaption("unnamed document");
    w->show();
    qDebug("size  = %d", w->height() );
}

void ApplicationWindow::load()
{
    MDIWindow* m = (MDIWindow*)ws->activeClient();
    if ( !m )
	return;
    qDebug("set caption to hallo ");
    m->setCaption("Hallo");
    return;
//     static bool nase = FALSE;
//     static QWidget* nw= 0;

//     if (!nw)
// 	nw = new QWidget;

//     if (!nase ) {
// 	nw->show();
//     }
//     else
// 	nw->close();
//     nase = !nase;
//     return;

    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null,
					       this);
    if ( !fn.isEmpty() )
	load( fn );
    else
	statusBar()->message( "Loading aborted", 2000 );
}


void ApplicationWindow::load( const char *fileName )
{
    /*
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
    setCaption( fileName );
    QString s;
    s.sprintf( "Loaded document %s", fileName );
    statusBar()->message( s, 2000 );
    */
}


void ApplicationWindow::save()
{
    /*
    QObjectList list = *fileTools->children();
    for (QObjectListIt it(list); it.current(); ++it) {
	if ( it.current()->isWidgetType() )
	    delete it.current();
    }

    return;
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

    setCaption( filename );

    statusBar()->message( QString( "File %1 saved" ).arg( filename ), 2000 );
    */
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
    /*
    const int Margin = 10;
    int pageNo = 1;

    if ( printer->setup(this) ) {		// printer dialog
	statusBar()->message( "Printing..." );
	QPainter p;
	p.begin( printer );			// paint on printer
	p.setFont( e->font() );
	int yPos        = 0;			// y position for each line
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

    */

}

void ApplicationWindow::closeEvent( QCloseEvent* e )
{
    e->accept();

    // Note: If we accepted the close event, we will be deleted
    // afterwards since we used the WDestructiveClose flag in the
    // constructor
}


void ApplicationWindow::about()
{
    QMessageBox::about( this, "Qt Application Example",
			"This example demonstrates simple use of\n "
			"Qt's Multiple Document Interface (MDI).");
}


void ApplicationWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Application Example" );
}


void ApplicationWindow::windowsMenuAboutToShow()
{
    windowsMenu->clear();
    for ( int i = 0; i < int(windows.count()); ++i ) {
	int id = windowsMenu->insertItem(windows.at(i)->caption(),
					 this, SLOT( windowsMenuActivated( int ) ) );
	windowsMenu->setItemParameter( id, i );
	windowsMenu->setItemChecked( id, ws->activeClient() == windows.at(i) );
    }
}

void ApplicationWindow::windowsMenuActivated( int id )
{
    QWidget* w = windows.at( id );
    if ( w )
	ws->activateClient( w );
}

void ApplicationWindow::childDestroyed()
{
    if ( sender() ) {
	windows.remove( (QWidget*)sender() );
    }
}

MDIWindow::MDIWindow( QWidget* parent, const char* name, int wflags )
    : QMainWindow( parent, name, wflags )
{
    medit = new QMultiLineEdit( this );
    setCentralWidget( medit );
    (void) statusBar();
}
MDIWindow::~MDIWindow()
{
    qDebug("MDI window deleted");
}

void MDIWindow::closeEvent( QCloseEvent* e)
{
    qDebug("MDI window close event");
    e->accept();
}
