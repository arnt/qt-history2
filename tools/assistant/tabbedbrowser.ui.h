/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qfileinfo.h>

#include "config.h"

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
    win->setMimeSourceFactory( tabMimeFactory );

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
    tabMimeFactory = new QMimeSourceFactory();
    lastCurrentTab = 0;
    while( tab->count() )
	tab->removePage( tab->page(0) );
    //    newTab( QString::null );
    connect( tab, SIGNAL( currentChanged( QWidget* ) ),
	     this, SLOT( transferFocus() ) );
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

    //QString base( qInstallPathDocs() );

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

    //tabMimeFactory->addFilePath( base + "/html" );
    setupMimeSource();
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

QMimeSourceFactory *TabbedBrowser::mimeSourceFactory()
{
    return tabMimeFactory;
}

void TabbedBrowser::setupMimeSource()
{
    Config *config = Config::configuration();
    QStringList docList = config->docFiles();
    QValueListConstIterator<QString> it = docList.begin();
    for ( ; it != docList.end(); ++it )
    {
	QFileInfo fi( *it );
	tabMimeFactory->addFilePath( fi.dirPath( TRUE ) );
	if ( !config->docImageDir( *it ).isEmpty() )
	    tabMimeFactory->addFilePath( config->docImageDir( *it ) );
    }
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
