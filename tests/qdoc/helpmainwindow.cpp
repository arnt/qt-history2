#include "helpmainwindow.h"
#include "helpnavigation.h"
#include "helpfinddialog.h"
#include "helpview.h"
#include <qsplitter.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qurl.h>
#include <qprogressbar.h>
#include <qstatusbar.h>

#include <stdlib.h>

static const char * pix_back[]={
    "16 16 5 1",
    "# c #000000",
    "a c #ffffff",
    "c c #808080",
    "b c #c0c0c0",
    ". c None",
    "................",
    ".......#........",
    "......##........",
    ".....#a#........",
    "....#aa########.",
    "...#aabaaaaaaa#.",
    "..#aabbbbbbbbb#.",
    "...#abbbbbbbbb#.",
    "...c#ab########.",
    "....c#a#ccccccc.",
    ".....c##c.......",
    "......c#c.......",
    ".......cc.......",
    "........c.......",
    "................",
    "......................"};

static const char * pix_forward[]={
    "16 16 5 1",
    "# c #000000",
    "a c #ffffff",
    "c c #808080",
    "b c #c0c0c0",
    ". c None",
    "................",
    "................",
    ".........#......",
    ".........##.....",
    ".........#a#....",
    "..########aa#...",
    "..#aaaaaaabaa#..",
    "..#bbbbbbbbbaa#.",
    "..#bbbbbbbbba#..",
    "..########ba#c..",
    "..ccccccc#a#c...",
    "........c##c....",
    "........c#c.....",
    "........cc......",
    "........c.......",
    "................",
    "................"};

static const char * pix_home[]={
    "16 16 4 1",
    "# c #000000",
    "a c #ffffff",
    "b c #c0c0c0",
    ". c None",
    "........... ....",
    "   ....##.......",
    "..#...####......",
    "..#..#aabb#.....",
    "..#.#aaaabb#....",
    "..##aaaaaabb#...",
    "..#aaaaaaaabb#..",
    ".#aaaaaaaaabbb#.",
    "###aaaaaaaabb###",
    "..#aaaaaaaabb#..",
    "..#aaa###aabb#..",
    "..#aaa#.#aabb#..",
    "..#aaa#.#aabb#..",
    "..#aaa#.#aabb#..",
    "..#aaa#.#aabb#..",
    "..#####.######..",
    "................"};

static const char * pix_print[] = {
    "    16    14        6            1",
    ". c #000000",
    "# c #848284",
    "a c #c6c3c6",
    "b c #ffff00",
    "c c #ffffff",
    "d c None",
    "ddddd.........dd",
    "dddd.cccccccc.dd",
    "dddd.c.....c.ddd",
    "ddd.cccccccc.ddd",
    "ddd.c.....c....d",
    "dd.cccccccc.a.a.",
    "d..........a.a..",
    ".aaaaaaaaaa.a.a.",
    ".............aa.",
    ".aaaaaa###aa.a.d",
    ".aaaaaabbbaa...d",
    ".............a.d",
    "d.aaaaaaaaa.a.dd",
    "dd...........ddd"
};

HelpMainWindow::HelpMainWindow()
    : QMainWindow()
{
    setCaption( tr( "Qt Online Documentation" ) );
    QSplitter *splitter = new QSplitter( this );

    docDir = QString( getenv( "QTDIR" ) ) + "/html";
    if ( !QFile::exists( docDir ) )
	docDir = QString( getenv( "QTDIR" ) ) + "/doc/html";
    if ( !QFile::exists( docDir ) ) {
	QMessageBox::critical( this, tr( "Error" ), tr( "Couldn't find the Qt documentation directory!" ) );
	exit( 0 );
	return;
    }

    if ( !QFile::exists( docDir + "/index" ) ) {
	QMessageBox::critical( this, tr( "Error" ), tr( "Couldn't find the Qt documentation index file!\n"
							"Use mkindex to create it!" ) );
	exit( 0 );
	return;
    }

    if ( !QFile::exists( docDir + "/titleindex" ) ) {
	QMessageBox::critical( this, tr( "Error" ), tr( "Couldn't find the Qt documentation title index file!\n"
							"Use mktitleindex to create it!" ) );
	exit( 0 );
	return;
    }

    navigation = new HelpNavigation( splitter, docDir );
    viewer = new HelpView( splitter, docDir );
    splitter->setResizeMode( navigation, QSplitter::KeepSize );
    setCentralWidget( splitter );

    connect( navigation, SIGNAL( showLink( const QString &, const QString& ) ),
	     viewer, SLOT( showLink( const QString &, const QString& ) ) );

    QPopupMenu *file = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&File" ), file );
    file->insertItem( tr( "&Print" ), this, SLOT( slotFilePrint() ), CTRL + Key_P );
    file->insertSeparator();
    file->insertItem( tr( "&Quit" ), qApp, SLOT( quit() ), CTRL + Key_Q );

    QPopupMenu *edit = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&Edit" ), edit );

    edit->insertItem( tr( "&Copy" ), this, SLOT( slotEditCopy() ), CTRL + Key_C );
    edit->insertItem( tr( "Select &All" ), this, SLOT( slotEditSelectAll() ), CTRL + Key_A );
    edit->insertSeparator();
    edit->insertItem( tr( "&Find in this Topic..." ), this, SLOT( slotEditFind() ), CTRL + Key_F );

    view = new QPopupMenu( this );
    view->setCheckable( TRUE );
    menuBar()->insertItem( tr( "&View" ), view );

    contents_id = view->insertItem( tr( "&Contents" ), this, SLOT( slotViewContents() ), ALT + Key_C );
    index_id = view->insertItem( tr( "&Index" ), this, SLOT( slotViewIndex() ), ALT + Key_I );
    bookmarks_id = view->insertItem( tr( "&Bookmarks" ), this, SLOT( slotViewBookmarks() ), ALT + Key_B );
    search_id = view->insertItem( tr( "&Search" ), this, SLOT( slotViewSearch() ), ALT + Key_S );

    go = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&Go" ), go );

    backward_id = go->insertItem( tr( "&Back" ), this, SLOT( slotGoBack() ), CTRL + Key_Left );
    forward_id = go->insertItem( tr( "&Forward" ), this, SLOT( slotGoForward() ), CTRL + Key_Right );
    go->insertItem( tr( "&Home" ), this, SLOT( slotGoHome() ), CTRL + Key_Home );
    go->insertSeparator();
    history = new QPopupMenu( this );
    go->insertItem( tr( "History" ), history );

    QPopupMenu *help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr( "&Help" ), help );

    help->insertItem( tr( "&About" ), this, SLOT( slotHelpAbout() ), Key_F1 );
    help->insertItem( tr( "About &Qt" ), this, SLOT( slotHelpAboutQt() ) );

    QToolBar *tb = new QToolBar( this );

    backward = new QToolButton( QPixmap( pix_back ), tr( "Back" ), "", this,
				SLOT( slotGoBack() ), tb );
    forward = new QToolButton( QPixmap( pix_forward ), tr( "Forward" ), "", this,
			       SLOT( slotGoForward() ), tb );
    QToolButton *b = new QToolButton( QPixmap( pix_home ), tr( "Home" ), "", this,
				      SLOT( slotGoHome() ), tb );
    tb->addSeparator();
    b = new QToolButton( QPixmap( pix_print ), tr( "Print" ), "", this,
			 SLOT( slotFilePrint() ), tb );
    tb->setStretchableWidget( new QWidget( tb ) );
    tb->setStretchMode( QToolBar::FullWidth );
    setUsesTextLabel( TRUE );

    navigation->setViewMode( HelpNavigation::Index );
    viewer->blockSignals( TRUE );
    viewer->setSource( docDir + "/index.html" );
    viewer->blockSignals( FALSE );

    connect( viewer, SIGNAL( newSource( const QString & ) ),
	     this, SLOT( newSource( const QString & ) ) );
    connect( navigation, SIGNAL( moveFocusToBrowser() ),
	     this, SLOT( moveFocusToBrowser() ) );
    connect( history, SIGNAL( aboutToShow() ),
	     this, SLOT( setupHistoryMenu() ) );
    connect( history, SIGNAL( activated( int ) ),
	     this, SLOT( showFromHistory( int ) ) );
    indexCreated = FALSE;

    label = new QLabel(  statusBar() );
    statusBar()->addWidget( label, 2, TRUE );
    bar = new QProgressBar( statusBar() );
    statusBar()->addWidget( bar, 1, TRUE );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    findDialog = 0;
    
    connect( navigation, SIGNAL( preparePorgress( int ) ),
	     this, SLOT( preparePorgress( int ) ) );
    connect( navigation, SIGNAL( incProcess() ),
	     this, SLOT( incProcess() ) );
    connect( navigation, SIGNAL( finishProgress() ),
	     this, SLOT( finishProgress() ) );
    connect( viewer, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( forwardAvailable( bool ) ) );
    connect( viewer, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( backwardAvailable( bool ) ) );
    connect( navigation, SIGNAL( tabChanged() ),
	     this, SLOT( updateViewMenu() ) );
}

void HelpMainWindow::slotFilePrint()
{
    QPrinter printer;
    printer.setFullPage(TRUE);
    if ( printer.setup() ) {
	QPainter p( &printer );
	QPaintDeviceMetrics metrics(p.device());
	int dpix = metrics.logicalDpiX();
	int dpiy = metrics.logicalDpiY();
	const int margin = 72; // pt
	QRect body(margin*dpix/72, (30+margin)*dpiy/72,
		   metrics.width()-margin*dpix/72*2,
		   metrics.height()-(margin+30)*dpiy/72*2 );
	double scale = 0.75;
	p.scale(scale, scale );
	body = QRect( int(body.x()/scale), int(body.y()/scale),
		      int(body.width()/scale), int(body.height()/scale) );
	QFont font("times");
	QSimpleRichText richText( viewer->text(), font, viewer->context(), viewer->styleSheet(),
				  viewer->mimeSourceFactory(), body.height() );
	richText.setWidth( &p, body.width() );
	font.setItalic( TRUE );
	font.setPointSize( 10 );
	p.setFont( font );
	QRect view( body );
	int page = 1;
	int ls = p.fontMetrics().lineSpacing();

	statusBar()->addWidget( label, 2, TRUE );
	statusBar()->addWidget( bar, 1, TRUE );
	bar->setTotalSteps( richText.height() / body.height() );
	bar->setProgress( 0 );
	label->show();
	bar->show();
	label->setText( tr( "Printing...." ) );
	
	qApp->processEvents();
	do {
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    p.setFont( font );
	    p.drawText( view.right() - p.fontMetrics().width( viewer->caption() ),
			view.top() - 3*ls/2, viewer->caption() );
	    p.setPen(1);
	    p.drawLine( view.left(), view.top()-ls, view.right(), view.top()-ls );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + ls, QString::number(page) );
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    if ( view.top()  >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	    bar->setProgress( bar->progress() + 1 );
	    qApp->processEvents();
	} while (TRUE);
	statusBar()->removeWidget( bar );
	statusBar()->removeWidget( label );
	bar->hide();
	label->hide();
    }
}

void HelpMainWindow::slotEditCopy()
{
    viewer->copy();
}

void HelpMainWindow::slotEditSelectAll()
{
    viewer->selectAll();
}

void HelpMainWindow::slotEditFind()
{
    if ( findDialog ) {
	findDialog->hide();
	findDialog->show();
    } else {
	findDialog = new HelpFindDialog( this );
	findDialog->show();
	connect( findDialog, SIGNAL( closed() ),
		 this, SLOT( findDialogClosed() ) );
    }
}

void HelpMainWindow::slotViewContents()
{
    navigation->setViewMode( HelpNavigation::Contents );
}

void HelpMainWindow::slotViewIndex()
{
    navigation->setViewMode( HelpNavigation::Index );
}

void HelpMainWindow::slotViewSearch()
{
    navigation->setViewMode( HelpNavigation::Search );
}

void HelpMainWindow::slotViewBookmarks()
{
    navigation->setViewMode( HelpNavigation::Bookmarks );
}

void HelpMainWindow::slotGoBack()
{
    viewer->backward();
}

void HelpMainWindow::slotGoForward()
{
    viewer->forward();
}

void HelpMainWindow::slotGoHome()
{
    viewer->home();
}

void HelpMainWindow::slotHelpAbout()
{
    QMessageBox::about( this, "About Qt Online Documentation",
                        "<p>Interactive documentation browser for the Qt Toolkit. </p>"
                        "<p>Copyright (C) 2000 Troll Tech AS</p>"
        );
}

void HelpMainWindow::slotHelpAboutQt()
{
    QMessageBox::aboutQt( this, "Qt Application Example" );
}

void HelpMainWindow::newSource( const QString &name )
{
    viewer->setCaption( navigation->titleOfLink( name ) );
}

void HelpMainWindow::moveFocusToBrowser()
{
    viewer->setFocus();
}

void HelpMainWindow::showEvent( QShowEvent *e )
{
    QMainWindow::showEvent( e );
    if ( !indexCreated )
	QTimer::singleShot( 1, this, SLOT( createDatabase() ) );
}

void HelpMainWindow::createDatabase()
{
    bar->setTotalSteps( QFileInfo( docDir + "/index" ).size() +
			QFileInfo( docDir + "/titleindex" ).size() );
    bar->setProgress( 0 );
    label->setText( tr( "Creating Index Database..." ) );
    navigation->loadIndexFile( bar );
    navigation->setupContentsView( bar );
    navigation->loadBookmarks();
    statusBar()->removeWidget( bar );
    statusBar()->removeWidget( label );
    bar->hide();
    label->hide();
    indexCreated = TRUE;
    viewer->setSource( viewer->source() );
}

void HelpMainWindow::addBookmark()
{
    QString link = QUrl( viewer->context(), viewer->source() ).path();
    QString title = viewer->caption();
    navigation->addBookmark( title, link );
}

void HelpMainWindow::removeBookmark()
{
    navigation->removeBookmark();
}

HelpMainWindow::~HelpMainWindow()
{
    navigation->saveBookmarks();
}

void HelpMainWindow::setupHistoryMenu()
{
    history->clear();
    QMap<QString, QString> hist = viewer->history();
    QMap<QString, QString>::Iterator it = hist.begin();
    for ( ; it != hist.end(); ++it ) {
	history->insertItem( *it );
    }
}

void HelpMainWindow::showFromHistory( int id )
{
    QMap<QString, QString> hist = viewer->history();
    QMap<QString, QString>::Iterator it = hist.begin();
    for ( ; it != hist.end(); ++it ) {
	if ( *it == history->text( id ) ) {
	    viewer->showLink( it.key(), *it );
	}
    }
}

void HelpMainWindow::preparePorgress( int total )
{
    statusBar()->addWidget( label, 2, TRUE );
    statusBar()->addWidget( bar, 1, TRUE );
    bar->setTotalSteps( total );
    bar->setProgress( 0 );
    label->show();
    bar->show();
    label->setText( tr( "Searching...." ) );
}

void HelpMainWindow::incProcess()
{
    bar->setProgress( bar->progress() + 1 );
}

void HelpMainWindow::finishProgress()
{
    statusBar()->removeWidget( bar );
    statusBar()->removeWidget( label );
    bar->hide();
    label->hide();
}

void HelpMainWindow::forwardAvailable( bool b )
{
    forward->setEnabled( b );
    go->setItemEnabled( forward_id, b );
}

void HelpMainWindow::backwardAvailable( bool b )
{
    backward->setEnabled( b );
    go->setItemEnabled( backward_id, b );
}

void HelpMainWindow::updateViewMenu()
{
    view->setItemChecked( contents_id,
			  navigation->viewMode() == HelpNavigation::Contents );
    view->setItemChecked( index_id,
			  navigation->viewMode() == HelpNavigation::Index );
    view->setItemChecked( bookmarks_id,
			  navigation->viewMode() == HelpNavigation::Bookmarks );
    view->setItemChecked( search_id,
			  navigation->viewMode() == HelpNavigation::Search );
}

void HelpMainWindow::findDialogClosed()
{
    delete findDialog;
    findDialog = 0;
}
