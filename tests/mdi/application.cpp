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


const char * fileOpenText = "Click this button to open a <em>new file</em>. <br><br>"
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
    file->insertItem( "&Close", this, SLOT(closeClient()), CTRL+Key_W );
    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

    windowsMenu = new QPopupMenu( this );
    windowsMenu->setCheckable( TRUE );
    connect( windowsMenu, SIGNAL( aboutToShow() ),
	     this, SLOT( windowsMenuAboutToShow() ) );
    menuBar()->insertItem( "&Windows", windowsMenu );

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About&Qt", this, SLOT(aboutQt()));
    help->insertSeparator();
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1);

    QVBox* vb = new QVBox( this );
    vb->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    ws = new QWorkspace( vb );
    setCentralWidget( vb );
    
    statusBar()->message( "Ready", 2000 );
    resize( 450, 600 );
}


ApplicationWindow::~ApplicationWindow()
{
    delete printer;
}



MDIWindow* ApplicationWindow::newDoc()
{
    MDIWindow* w = new MDIWindow( ws, 0, WDestructiveClose );
    connect( w, SIGNAL( message(const QString&, int) ), statusBar(), SLOT( message(const QString&, int )) );
    w->setCaption("unnamed document");
    if ( ws->clientList().isEmpty() ) // show the very first window in maximized mode
	w->showMaximized();
    else
	w->show();
    return w;
}

void ApplicationWindow::load()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() ) {
	MDIWindow* w = newDoc();
	w->load( fn );
    }  else {
	statusBar()->message( "Loading aborted", 2000 );
    }
}

void ApplicationWindow::save()
{
    MDIWindow* m = (MDIWindow*)ws->activeClient();
    if ( m )
	m->save();
}


void ApplicationWindow::saveAs()
{
    MDIWindow* m = (MDIWindow*)ws->activeClient();
    if ( m )
	m->saveAs();
}


void ApplicationWindow::print()
{
    MDIWindow* m = (MDIWindow*)ws->activeClient();
    if ( m )
	m->print( printer );
}


void ApplicationWindow::closeClient()
{
    MDIWindow* m = (MDIWindow*)ws->activeClient();
    if ( m )
	m->close();
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
    QWidgetList windows = ws->clientList();
    for ( int i = 0; i < int(windows.count()); ++i ) {
	int id = windowsMenu->insertItem(windows.at(i)->caption(),
					 this, SLOT( windowsMenuActivated( int ) ) );
	windowsMenu->setItemParameter( id, i );
	windowsMenu->setItemChecked( id, ws->activeClient() == windows.at(i) );
    }
}

void ApplicationWindow::windowsMenuActivated( int id )
{
    QWidget* w = ws->clientList().at( id );
    if ( w )
	w->setFocus();
}

MDIWindow::MDIWindow( QWidget* parent, const char* name, int wflags )
    : QMainWindow( parent, name, wflags )
{
    medit = new QMultiLineEdit( this );
    setFocusProxy( medit );
    setCentralWidget( medit );
}
MDIWindow::~MDIWindow()
{
}


void MDIWindow::load( const QString& fn )
{
    filename  = fn;
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	return;

    medit->setAutoUpdate( FALSE );
    medit->clear();

    QTextStream t(&f);
    while ( !t.eof() ) {
	QString s = t.readLine();
	medit->append( s );
    }
    f.close();

    medit->setAutoUpdate( TRUE );
    medit->repaint();
    setCaption( filename );
    emit message( QString("Loaded document %1").arg(filename), 2000 );
}

void MDIWindow::save()
{
    if ( filename.isEmpty() ) {
        saveAs();
        return;
    }

    QString text = medit->text();
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
        emit message( QString("Could not write to %1").arg(filename),
		      2000 );
        return;
    }

    QTextStream t( &f );
    t << text;
    f.close();

    setCaption( filename );

    emit message( QString( "File %1 saved" ).arg( filename ), 2000 );
}

void MDIWindow::saveAs()
{
    QString fn = QFileDialog::getSaveFileName( filename, QString::null, this );
    if ( !fn.isEmpty() ) {
        filename = fn;
        save();
    } else {
        emit message( "Saving aborted", 2000 );
    }
}

void MDIWindow::print( QPrinter* printer)
{
    const int Margin = 10;
    int pageNo = 1;

    if ( printer->setup(this) ) {		// printer dialog
	emit message( "Printing...", 0 );
	QPainter p;
	p.begin( printer );			// paint on printer
	p.setFont( medit->font() );
	int yPos        = 0;			// y position for each line
	QFontMetrics fm = p.fontMetrics();
	QPaintDeviceMetrics metrics( printer ); // need width/height
	// of printer surface
	for( int i = 0 ; i < medit->numLines() ; i++ ) {
	    if ( Margin + yPos > metrics.height() - Margin ) {
		QString msg( "Printing (page " );
		msg += QString::number( ++pageNo );
		msg += ")...";
		emit message( msg, 0 );
		printer->newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( Margin, Margin + yPos,
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			medit->textLine( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
	emit message( "Printing completed", 2000 );
    } else {
	emit message( "Printing aborted", 2000 );
    }
}

 
