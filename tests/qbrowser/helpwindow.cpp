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

#include "helpwindow.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qnetworkprotocol.h>

#include <ctype.h>

HelpWindow::HelpWindow( const QString& home_, const QString& _path, QWidget* parent, const char *name )
    : QMainWindow( parent, name, WDestructiveClose ), pathCombo( 0 ), selectedURL(),
      path( QFileInfo( home_ ).dirPath( TRUE ), "*.html *.htm" )
{
    readHistory();
    readBookmarks();

    fileList = path.entryList();

    browser = new QTextBrowser( this );
    browser->setTextFormat( RichText );
    browser->mimeSourceFactory()->setFilePath( _path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( textChanged() ),
	     this, SLOT( textChanged() ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
	browser->setSource( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
	     statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( tr("&New Window"), this, SLOT( newWindow() ), ALT | Key_N );
    file->insertItem( tr("&Open File"), this, SLOT( openFile() ), ALT | Key_O );
    file->insertSeparator();
    file->insertItem( tr("&Close"), this, SLOT( close() ), ALT | Key_Q );
    file->insertItem( tr("E&xit"), qApp, SLOT( closeAllWindows() ), ALT | Key_X );

    QPopupMenu* go = new QPopupMenu( this );
    backwardId = go->insertItem( QPixmap("back.xpm"),
				 tr("&Backward"), browser, SLOT( backward() ),
				 ALT | Key_Left );
    forwardId = go->insertItem( QPixmap("forward.xpm"),
				tr("&Forward"), browser, SLOT( forward() ),
				ALT | Key_Right );
    go->insertItem( QPixmap("home.xpm"), tr("&Home"), browser, SLOT( home() ) );

    QPopupMenu* help = new QPopupMenu( this );
    help->insertItem( tr("&About ..."), this, SLOT( about() ) );
    help->insertItem( tr("About &Qt ..."), this, SLOT( aboutQt() ) );

    hist = new QPopupMenu( this );
    QStringList::Iterator it = history.begin();
    for ( ; it != history.end(); ++it )
	mHistory[ hist->insertItem( *it ) ] = *it;
    connect( hist, SIGNAL( activated( int ) ),
	     this, SLOT( histChosen( int ) ) );

    bookm = new QPopupMenu( this );
    bookm->insertItem( tr( "Add Bookmark" ), this, SLOT( addBookmark() ) );
    bookm->insertSeparator();

    QStringList::Iterator it2 = bookmarks.begin();
    for ( ; it2 != bookmarks.end(); ++it2 )
	mBookmarks[ bookm->insertItem( *it2 ) ] = *it2;
    connect( bookm, SIGNAL( activated( int ) ),
	     this, SLOT( bookmChosen( int ) ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Go"), go );
    menuBar()->insertItem( tr( "History" ), hist );
    menuBar()->insertItem( tr( "Bookmarks" ), bookm );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr("&Help"), help );

    menuBar()->setItemEnabled( forwardId, FALSE);
    menuBar()->setItemEnabled( backwardId, FALSE);
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( setForwardAvailable( bool ) ) );


    QToolBar* toolbar = new QToolBar( this );
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( QPixmap("back.xpm"), tr("Backward"), "", browser, SLOT(backward()), toolbar );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( QPixmap("forward.xpm"), tr("Forward"), "", browser, SLOT(forward()), toolbar );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( QPixmap("home.xpm"), tr("Home"), "", browser, SLOT(home()), toolbar );

    toolbar->addSeparator();

    pathCombo = new QComboBox( TRUE, toolbar );
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
	     this, SLOT( pathSelected( const QString & ) ) );
    toolbar->setStretchableWidget( pathCombo );
    setRightJustification( TRUE );

    pathCombo->insertItem( home_ );
    pathCombo->installEventFilter( this );
    QObjectList *l = queryList( "QLineEdit" );
    if ( l && l->first() )
	( (QLineEdit*)l->first() )->installEventFilter( this );

    browser->setFocus();

    connect( &url, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
	     this, SLOT( newData( const QByteArray &, QNetworkOperation * ) ) );
}


void HelpWindow::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void HelpWindow::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void HelpWindow::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( browser->context() );
    else
	setCaption( browser->documentTitle() ) ;

    selectedURL = caption();
    if ( !selectedURL.isEmpty() && pathCombo ) {
	path = QDir( QFileInfo( selectedURL ).dirPath( TRUE ), "*.html *.htm" );
	fileList = path.entryList();
	bool exists = FALSE;
	int i;
	for ( i = 0; i < pathCombo->count(); ++i ) {
	    if ( pathCombo->text( i ) == selectedURL ) {
		exists = TRUE;
		break;
	    }
	}
	if ( !exists ) {
	    pathCombo->insertItem( selectedURL, 0 );
	    pathCombo->setCurrentItem( 0 );
	    mHistory[ hist->insertItem( selectedURL ) ] = selectedURL;
	} else
	    pathCombo->setCurrentItem( i );
	selectedURL = QString::null;
    }
}

HelpWindow::~HelpWindow()
{
    history.clear();
    QMap<int, QString>::Iterator it = mHistory.begin();
    for ( ; it != mHistory.end(); ++it )
	history.append( *it );

    QFile f( QDir::currentDirPath() + "/.history" );
    f.open( IO_WriteOnly );
    QDataStream s( &f );
    s << history;
    f.close();

    bookmarks.clear();
    QMap<int, QString>::Iterator it2 = mBookmarks.begin();
    for ( ; it2 != mBookmarks.end(); ++it2 )
	bookmarks.append( *it2 );

    QFile f2( QDir::currentDirPath() + "/.bookmarks" );
    f2.open( IO_WriteOnly );
    QDataStream s2( &f2 );
    s2 << bookmarks;
    f2.close();
}

void HelpWindow::about()
{
    QMessageBox::about( this, "QBrowser Example",
			"<p>This example implements a simple HTML browser "
			"using Qt's rich text capabilities</p>"
			"<p>It's just about 100 lines of C++ code, so don't expect too much :-)</p>"
	);
}


void HelpWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "QBrowser" );
}

void HelpWindow::openFile()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
	browser->setSource( fn );
}

void HelpWindow::newWindow()
{
    ( new HelpWindow(browser->source(), "qbrowser") )->show();
}

void HelpWindow::pathSelected( const QString &_path )
{
    if ( QUrl( _path ).isLocalFile() ) {
	browser->setSource( _path );
	path = QDir( QFileInfo( _path ).dirPath( TRUE ), "*.html *.htm" );
	fileList = path.entryList();
	QMap<int, QString>::Iterator it = mHistory.begin();
	bool exists = FALSE;
	for ( ; it != mHistory.end(); ++it ) {
	    if ( *it == _path ) {
		exists = TRUE;
		break;
	    }
	}
	if ( !exists )
	    mHistory[ hist->insertItem( _path ) ] = _path;
    } else {
	url = QUrlOperator( _path );
	url.get();
	pageData = "";
    }
}

bool HelpWindow::eventFilter( QObject * o, QEvent * e )
{
    QObjectList *l = queryList( "QLineEdit" );
    if ( !l || !l->first() )
	return FALSE;

    QLineEdit *lined = (QLineEdit*)l->first();

    if ( ( o == pathCombo || o == lined ) &&
	 e->type() == QEvent::KeyPress ) {

//	   if ( isprint(((QKeyEvent *)e)->ascii()) ) {
//	       if ( lined->hasMarkedText() )
//		   lined->del();
//	       QString nt( lined->text() );
//	       nt.remove( 0, nt.findRev( '/' ) + 1 );
//	       nt.truncate( lined->cursorPosition() );
//	       nt += (char)(((QKeyEvent *)e)->ascii());

//	       QStringList::Iterator it = fileList.begin();
//	       while ( it != fileList.end() && (*it).left( nt.length() ) != nt )
//		   ++it;

//	       if ( !(*it).isEmpty() ) {
//		   nt = *it;
//		   int cp = lined->cursorPosition() + 1;
//		   int l = path.canonicalPath().length() + 1;
//		   lined->validateAndSet( path.canonicalPath() + "/" + nt, cp, cp, l + nt.length() );
//		   return TRUE;
//	       }
//	   }
    }

    return FALSE;
}

void HelpWindow::readHistory()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.history" ) ) {
	QFile f( QDir::currentDirPath() + "/.history" );
	f.open( IO_ReadOnly );
	QDataStream s( &f );
	s >> history;
	f.close();
	while ( history.count() > 20 )
	    history.remove( history.begin() );
    }
}

void HelpWindow::readBookmarks()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.bookmarks" ) ) {
	QFile f( QDir::currentDirPath() + "/.bookmarks" );
	f.open( IO_ReadOnly );
	QDataStream s( &f );
	s >> bookmarks;
	f.close();
    }
}

void HelpWindow::histChosen( int i )
{
    if ( mHistory.contains( i ) )
	browser->setSource( mHistory[ i ] );
}

void HelpWindow::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
	browser->setSource( mBookmarks[ i ] );
}

void HelpWindow::addBookmark()
{
    mBookmarks[ bookm->insertItem( caption() ) ] = caption();
}

void HelpWindow::newData( const QByteArray &data, QNetworkOperation *op )
{
    pageData += data;
    browser->setText( pageData );
}
