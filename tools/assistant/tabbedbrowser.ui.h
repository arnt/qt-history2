/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qstyle.h>

#include "config.h"

static const char * tab_widget_new_xpm[]={
"8 8 3 1",
". c None",
"# c #000000",
"a c #ffffff",
".####...",
".#aa##..",
".#aa###.",
".#aaaa#.",
".#aaaa#.",
".#aaaa#.",
".######.",
"........"};

static QString reduceLabelLength( const QString &s )
{
    int maxLength = 16;
    QString str = s;
    if ( str.length() < maxLength )
	return str;
    str = str.left( maxLength - 3 );
    str += "...";
    return str;
}

void TabbedBrowser::forward()
{
    currentBrowser()->forward();
}

void TabbedBrowser::backward()
{
    currentBrowser()->backward();
}

void TabbedBrowser::setSource( const QString &ref )
{
    HelpWindow * win = currentBrowser();
    win->setSource( ref );
}

void TabbedBrowser::reload()
{
    currentBrowser()->reload();
}

void TabbedBrowser::home()
{
    currentBrowser()->home();
}

HelpWindow * TabbedBrowser::currentBrowser()
{
    return (HelpWindow*) tab->currentPage();
}

void TabbedBrowser::nextTab()
{
    if( tab->currentPageIndex()<=tab->count()-1 )
	tab->setCurrentPage( tab->currentPageIndex()+1 );
}

void TabbedBrowser::previousTab()
{
    int idx = tab->currentPageIndex()-1;
    if( idx>=0 )
	tab->setCurrentPage( idx );
}

void TabbedBrowser::newTab( const QString &lnk )
{
    MainWindow *mainWin = mainWindow();
    QString link( lnk );
    if( link.isNull() ) {
	HelpWindow *w = currentBrowser();
	if( w )
	    link = w->source();
    }

    HelpWindow *win = new HelpWindow( mainWin, this, "qt_assistant_helpwin" );
    win->setFont( font() );
    win->setPalette( palette() );
    win->setLinkUnderline( tabLinkUnderline );
    win->setStyleSheet( tabStyleSheet );
    win->setMimeSourceFactory( mimeSourceFactory );

    tab->addTab( win, "..." );
    tab->showPage( win );
    connect( win, SIGNAL( highlighted( const QString & ) ),
	     (const QObject*) (mainWin->statusBar()), SLOT( message( const QString & ) ) );

    if( !link.isNull() ) {
	win->setSource( link );
    }
}

void TabbedBrowser::zoomIn()
{
    currentBrowser()->zoomIn();
}

void TabbedBrowser::zoomOut()
{
    currentBrowser()->zoomOut();
}

void TabbedBrowser::init()
{
    tabLinkUnderline = FALSE;
    tabStyleSheet = new QStyleSheet( QStyleSheet::defaultSheet() );
    lastCurrentTab = 0;
    while( tab->count() )
	tab->removePage( tab->page(0) );

    mimeSourceFactory = new QMimeSourceFactory();
    mimeSourceFactory->setExtensionType("html","text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType("htm","text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType("png", "image/png" );
    mimeSourceFactory->setExtensionType("jpg", "image/jpeg" );
    mimeSourceFactory->setExtensionType("jpeg", "image/jpeg" );

    connect( tab, SIGNAL( currentChanged( QWidget* ) ),
	     this, SLOT( transferFocus() ) );

    QToolButton *newTabButton = new QToolButton( this );
    tab->setCornerWidget( newTabButton, Qt::TopLeft );
    newTabButton->setCursor( arrowCursor );
    newTabButton->setPixmap( QPixmap( tab_widget_new_xpm ) );
    newTabButton->setFixedSize( 12, 12 );
    QObject::connect( newTabButton, SIGNAL( clicked() ), this, SLOT( newTab() ) );

    QToolButton *closeTabButton = new QToolButton( this );
    tab->setCornerWidget( closeTabButton, Qt::TopRight );
    closeTabButton->setCursor( arrowCursor );
    closeTabButton->setPixmap( style().stylePixmap( QStyle::SP_DockWindowCloseButton, closeTabButton ) );
    closeTabButton->setFixedSize( 12, 12 );
    QObject::connect( closeTabButton, SIGNAL( clicked() ), this, SLOT( closeTab() ) );
}

void TabbedBrowser::setMimePath( QStringList lst )
{
    mimeSourceFactory->setFilePath( lst );
}

void TabbedBrowser::setMimeExtension( const QString &ext )
{
    mimeSourceFactory->setExtensionType( "html", ext );
    mimeSourceFactory->setExtensionType( "htm", ext );
}

void TabbedBrowser::updateTitle( const QString &title )
{
    tab->changeTab( currentBrowser(), title );
}

MainWindow* TabbedBrowser::mainWindow()
{
    return (MainWindow*) parent();
}

void TabbedBrowser::newTab()
{
    newTab( QString::null );
}

void TabbedBrowser::transferFocus()
{
    if( currentBrowser() ) {
	currentBrowser()->setFocus();
    }
}

void TabbedBrowser::initHelpWindow( HelpWindow * /*win*/ )
{
}

void TabbedBrowser::setup()
{
    Config *config = Config::configuration();

    QFont fnt( font() );
    QFontInfo fntInfo( fnt );
    fnt.setFamily( config->fontFamily() );
    fnt.setPointSize( config->fontSize() );
    setFont( fnt );

    QPalette pal = palette();
    QColor lc( config->linkColor() );
    pal.setColor( QPalette::Active, QColorGroup::Link, lc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, lc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, lc );
    setPalette( pal );

    tabLinkUnderline = config->isLinkUnderline();

    QString family = config->fontFixedFamily();
    tabStyleSheet->item( "pre" )->setFontFamily( family );
    tabStyleSheet->item( "code" )->setFontFamily( family );
    tabStyleSheet->item( "tt" )->setFontFamily( family );

    newTab( QString::null );
}

void TabbedBrowser::setLinkUnderline( bool uline )
{
    if( uline==tabLinkUnderline )
	return;
    tabLinkUnderline = uline;
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ )
	( (QTextBrowser*) tab->page( i ) )->setLinkUnderline( tabLinkUnderline );
}

void TabbedBrowser::setFont( const QFont &fnt )
{
    if( font()==fnt )
	return;
    QWidget::setFont( fnt );
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ )
	( (QTextBrowser*) tab->page( i ) )->setFont( fnt );
}

void TabbedBrowser::setPalette( const QPalette &pal )
{
    if( palette()==pal )
	return;
    QWidget::setPalette( pal );
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ )
	( (QTextBrowser*) tab->page( i ) )->setPalette( pal );
}

QStyleSheet* TabbedBrowser::styleSheet()
{
    return tabStyleSheet;
}

bool TabbedBrowser::linkUnderline()
{
    return tabLinkUnderline;
}

void TabbedBrowser::copy()
{
    currentBrowser()->copy();
}

void TabbedBrowser::closeTab()
{
    if( tab->count()==1 )
	return;
    HelpWindow *win = currentBrowser();
    tab->removePage( win );
    delete win;
}

QStringList TabbedBrowser::sources()
{
    QStringList lst;
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ ) {
	lst.append( ( (QTextBrowser*) tab->page(i) )->source() );
    }
    return lst;
}

void TabbedBrowser::displayEntireLabel( QWidget *w )
{
    HelpWindow *win = (HelpWindow*)lastCurrentTab;
    if ( win )
	tab->changeTab( lastCurrentTab, reduceLabelLength( win->documentTitle() ) );
    win = (HelpWindow*)(tab->currentPage());
    tab->changeTab( w, win->documentTitle() );
    lastCurrentTab = w;
}
