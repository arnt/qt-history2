/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

static const char * book_xpm[]={
    "22 22 4 1",
    "# c #000000",
    "a c #808080",
    "b c #ffffff",
    ". c None",
    "......................",
    "......................",
    "......................",
    "......................",
    ".....##...............",
    "....#ab#....###.......",
    "....#abb#.##bb#.......",
    "....#abbb#abbb###.....",
    "....#abbb#bbbb#a#.....",
    "....#abbb#abbb#a#.....",
    "....#abbb#bbbb#a#.....",
    "....#abbb#abbb#a#.....",
    "....#abbb#bbbb#a#.....",
    ".....#abb#abb##a#.....",
    "......#ab#b##bba#.....",
    ".......#a##aaaaa#.....",
    ".......##a#######.....",
    "........##............",
    "......................",
    "......................",
    "......................",
    "......................"};


#include "helpdialogimpl.h"
#include "helpwindow.h"
#include "topicchooserimpl.h"
#include "docuparser.h"
#include "mainwindow.h"

#include <stdlib.h>
#include <limits.h>

#include <qprogressbar.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qtabwidget.h>
#include <qurl.h>
#include <qheader.h>
#include <qtimer.h>
#include <qlineedit.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtextbrowser.h>
#include <qaccel.h>
#include <qregexp.h>
#include <qpixmap.h>
#include <qmime.h>
#include <qptrstack.h>
#include <qptrlist.h>
#include <qcursor.h>
#include <qstatusbar.h>
#include <qvalidator.h>
//#ifdef QT_PALMTOPCENTER_DOCS
#include <qsettings.h>
//#endif

static QPixmap *bookPixmap = 0;

struct IndexKeyword {
    IndexKeyword( const QString &kw, const QString &l )
	: keyword( kw ), link( l ) {}
    IndexKeyword() : keyword( QString::null ), link( QString::null ) {}
    bool operator<( const IndexKeyword &ik ) const {
	return keyword.lower() < ik.keyword.lower();
    }
    bool operator<=( const IndexKeyword &ik ) const {
	return keyword.lower() <= ik.keyword.lower();
    }
    bool operator>( const IndexKeyword &ik ) const {
	return keyword.lower() > ik.keyword.lower();
    }
    QString keyword;
    QString link;
};

QDataStream &operator>>( QDataStream &s, IndexKeyword &ik )
{
    s >> ik.keyword;
    s >> ik.link;
    return s;
}

QDataStream &operator<<( QDataStream &s, const IndexKeyword &ik )
{
    s << ik.keyword;
    s << ik.link;
    return s;
}

QValidator::State SearchValidator::validate( QString &str, int & ) const
{
    for ( int i = 0; i < (int) str.length(); ++i ) {
	uchar r = str[i].row();
	uchar c = str[i].cell();

	if ( r != 0 || !( ( c >= '0' && c <= '9' ) ||
	     ( c >= 'a' && c <= 'z' ) ||
	     ( c >= 'A' && c <= 'Z' ) ||
	     c == '\'' || c == '`' ||
	     c == '\"' || c == ' ' ) )
	    return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

struct Entry
{
    QString link;
    QString title;
    int depth;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const Entry& ) const { return FALSE; }
#endif
};

HelpNavigationListItem::HelpNavigationListItem( QListBox *ls, const QString &txt )
    : QListBoxText( ls, txt )
{
}

void HelpNavigationListItem::addLink( const QString &link )
{
    int hash = link.find( '#' );
    if ( hash == -1 ) {
	linkList << link;
	return;
    }

    QString preHash = link.left( hash );
    if ( linkList.grep( preHash, FALSE ).count() > 0 )
	return;
    linkList << link;
}

HelpNavigationContentsItem::HelpNavigationContentsItem( QListView *v, QListViewItem *after )
    : QListViewItem( v, after )
{
}

HelpNavigationContentsItem::HelpNavigationContentsItem( QListViewItem *v, QListViewItem *after )
    : QListViewItem( v, after )
{
}

void HelpNavigationContentsItem::setLink( const QString &lnk )
{
    theLink = lnk;
}

QString HelpNavigationContentsItem::link() const
{
    return theLink;
}





HelpDialog::HelpDialog( QWidget *parent, MainWindow *h, HelpWindow *v )
    : HelpDialogBase( parent, 0, FALSE ), help( h ), viewer( v ), lwClosed( FALSE )
{
}

void HelpDialog::initialize()
{
    connect( tabWidget, SIGNAL( selected(const QString&) ),
	     this, SLOT( currentTabChanged(const QString&) ) );
    connect( listContents, SIGNAL( clicked(QListViewItem*) ),
	     this, SLOT( showTopic() ) );
    connect( listContents, SIGNAL( currentChanged(QListViewItem*) ),
	     this, SLOT( currentContentsChanged(QListViewItem*) ) );
    connect( listContents, SIGNAL( selectionChanged(QListViewItem*) ),
	     this, SLOT( currentContentsChanged(QListViewItem*) ) );
    connect( listContents, SIGNAL( doubleClicked(QListViewItem*) ),
	     this, SLOT( showTopic() ) );
    connect( listContents, SIGNAL( returnPressed(QListViewItem*) ),
	     this, SLOT( showTopic() ) );
    connect( editIndex, SIGNAL( returnPressed() ),
	     this, SLOT( showTopic() ) );
    connect( editIndex, SIGNAL( textChanged(const QString&) ),
	     this, SLOT( searchInIndex(const QString&) ) );
    connect( listIndex, SIGNAL( selectionChanged(QListBoxItem*) ),
	     this, SLOT( currentIndexChanged(QListBoxItem*) ) );
    connect( listIndex, SIGNAL( returnPressed(QListBoxItem*) ),
	     this, SLOT( showTopic() ) );
    connect( listIndex, SIGNAL( clicked(QListBoxItem*) ),
	     this, SLOT( showTopic() ) );
    connect( listIndex, SIGNAL( currentChanged(QListBoxItem*) ),
	     this, SLOT( currentIndexChanged(QListBoxItem*) ) );
    connect( listBookmarks, SIGNAL( clicked(QListViewItem*) ),
	     this, SLOT( showTopic() ) );
    connect( listBookmarks, SIGNAL( returnPressed(QListViewItem*) ),
	     this, SLOT( showTopic() ) );
    connect( listBookmarks, SIGNAL( selectionChanged(QListViewItem*) ),
	     this, SLOT( currentBookmarkChanged(QListViewItem*) ) );
    connect( listBookmarks, SIGNAL( currentChanged(QListViewItem*) ),
	     this, SLOT( currentBookmarkChanged(QListViewItem*) ) );

    bookPixmap = new QPixmap( book_xpm );
    QMimeSourceFactory *mime = QMimeSourceFactory::defaultFactory();
    mime->setExtensionType( "html", "text/html;charset=UTF-8" );
#ifdef QT_PALMTOPCENTER_DOCS
    QSettings settings;
    settings.insertSearchPath( QSettings::Unix,
			   QDir::homeDirPath() + "/.palmtopcenter/" );

    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QString basePath;
    basePath = settings.readEntry( "/palmtopcenter/qtopiadir" );
    if ( basePath.isEmpty() )
	basePath = getenv( "PALMTOPCENTERDIR" );
    QString lang = settings.readEntry( "/palmtopcenter/language" );
    if ( lang.isEmpty() )
	lang = getenv( "LANG" );
    documentationPath = basePath + "/doc/" + lang;

    mime->addFilePath( basePath + "/pics" );
    mime->addFilePath( basePath + "/pics/inline" );
    mime->addFilePath( basePath + "/pics/large" );
    mime->addFilePath( basePath + "/pics/small" );
    mime->addFilePath( documentationPath );
    mime->addFilePath( basePath + "/doc/en" );
#else
    QString base( qInstallPathDocs() );
    documentationPath = base + "/html";
    mime->addFilePath( documentationPath );
    mime->addFilePath( base + "/../gif/" );
#endif
    editIndex->installEventFilter( this );
    listBookmarks->header()->hide();
    listBookmarks->header()->setStretchEnabled( TRUE );
    listContents->header()->hide();
    listContents->header()->setStretchEnabled( TRUE );
    framePrepare->hide();
    connect( qApp, SIGNAL(lastWindowClosed()), SLOT(lastWinClosed()) );
#ifdef QT_PALMTOPCENTER_DOCS
    tabWidget->removePage( contentPage );
#endif

    termsEdit->setValidator( new SearchValidator( termsEdit ) );

    fullTextIndex = 0;
    indexDone = FALSE;
    contentsDone = FALSE;
    contentsInserted = FALSE;
    bookmarksInserted = FALSE;
    setupTitleMap();
}

void HelpDialog::lastWinClosed()
{
    lwClosed = TRUE;
}

void HelpDialog::generateNewDocu()
{
    QString d( QDir::homeDirPath() );
    QDir dir( d );
    QStringList list = dir.entryList( ".indexdb*; .titlemapdb", QDir::Files | QDir::Hidden );
    QStringList::iterator it = list.begin();
    for ( ; it != list.end(); ++it ) {
	if( QFile::exists( QDir::homeDirPath() + "/" + *it ) ){
	    QFile f( QDir::homeDirPath() + "/" + *it );
	    f.remove();
	}
    }
    indexDone = FALSE;
    contentsDone = FALSE;
    contentsInserted = FALSE;
    if ( fullTextIndex ) {
	delete fullTextIndex;
	fullTextIndex = 0;
    }
    setupTitleMap();
    if ( tabWidget->tabLabel( tabWidget->currentPage() ).contains( tr( "Con&tents" ) ) ) {
	QTimer::singleShot( 0, this, SLOT( insertContents() ) );
    }
    else if ( tabWidget->tabLabel( tabWidget->currentPage() ).contains( tr( "&Index" ) ) ) {
	QTimer::singleShot( 0, this, SLOT( loadIndexFile() ) );
    }
    else if ( tabWidget->tabLabel( tabWidget->currentPage() ).contains( tr( "&Search" ) ) ) {
	QTimer::singleShot( 0, this, SLOT( setupFullTextIndex() ) );
    }
}

bool HelpDialog::isValidCategory( QString category )
{
    QSettings settings;
    QStringList list = settings.readListEntry( "/Qt Assistant/3.1/CategoriesSelected/" );
    QStringList::iterator it = list.begin();
    for( ; it != list.end(); ++it ){
	if( ((*it).lower() == category.lower()) || ( *it == "all" ) )
	    return TRUE;
    }
    return FALSE;
}

void HelpDialog::loadIndexFile()
{
    if ( indexDone )
	return;

    setCursor( waitCursor );
    indexDone = TRUE;
    labelPrepare->setText( tr( "Prepare..." ) );
    framePrepare->show();
    qApp->processEvents();

    QProgressBar *bar = progressPrepare;
    bar->setTotalSteps( 100 );
    bar->setProgress( 0 );


    QValueList<IndexKeyword> lst;

    QFile indexFile( QDir::homeDirPath() + "/.indexdb" );
    if ( !indexFile.open( IO_ReadOnly ) ) {
	buildKeywordDB();
	indexFile.open( IO_ReadOnly );
    }

    QDataStream ds( &indexFile );
    Q_UINT32 fileAges;
    ds >> fileAges;
    if ( fileAges != getFileAges() ) {
	indexFile.close();
	buildKeywordDB();
	if ( !indexFile.open( IO_ReadOnly ) ) {
	    QMessageBox::warning( help, tr( "Qt Assistant" ),
		tr( "Cannot open the index file %1" ).arg( QFileInfo( indexFile ).absFilePath() ) );
	    return;
	}
	ds.setDevice( &indexFile );
	ds >> fileAges;
    }
    ds >> lst;
    indexFile.close();

    bar->setProgress( bar->totalSteps() );
    qApp->processEvents();

    listIndex->clear();
    HelpNavigationListItem *lastItem = 0;
    QString lastKeyword = QString::null;
    QValueList<IndexKeyword>::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it ) {
	if ( lastKeyword.lower() != (*it).keyword.lower() )
	    lastItem = new HelpNavigationListItem( listIndex, (*it).keyword );
	lastItem->addLink( (*it).link );
	lastKeyword = (*it).keyword;
    }
    framePrepare->hide();
    setCursor( arrowCursor );
}

Q_UINT32 HelpDialog::getFileAges()
{
    QSettings settings;
    QStringList addDocuFiles = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles" );
    QStringList::iterator i = addDocuFiles.begin();

    Q_UINT32 fileAges = 0;
    for( ; i != addDocuFiles.end(); i++ )
	fileAges += QFileInfo( *i ).lastModified().toTime_t();

    return fileAges;
}

void HelpDialog::buildKeywordDB()
{
    QSettings settings;
    QStringList addDocuFiles = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles" );
    QStringList::iterator i = addDocuFiles.begin();

    int steps = 0;
    for( ; i != addDocuFiles.end(); i++ )
	steps += QFileInfo( *i ).size();

    labelPrepare->setText( tr( "Prepare..." ) );
    progressPrepare->setTotalSteps( steps );
    progressPrepare->setProgress( 0 );
    qApp->processEvents();

    QValueList<IndexKeyword> lst;
    Q_UINT32 fileAges = 0;
    for( i = addDocuFiles.begin(); i != addDocuFiles.end(); i++ ){
	DocuParser handler;
	QFile file( *i );
	fileAges += QFileInfo( file ).lastModified().toTime_t();
	QXmlInputSource source( file );
	QXmlSimpleReader reader;
	reader.setContentHandler( &handler );
	reader.setErrorHandler( &handler );
	bool ok = reader.parse( source );
	file.close();
	if( !ok ){
	    QMessageBox::critical( this, "Parse Error",
		    handler.errorProtocol()
		    + "\nSkipping file " + *i );
	}
	else{
	    if( !isValidCategory( handler.getCategory() ) )
		continue;

	    QFileInfo finfo( *i );
	    QString dir = finfo.dirPath( TRUE ) + "/";
	    QPtrList<IndexItem> indLst = handler.getIndexItems();
	    QPtrListIterator<IndexItem> it( indLst );
	    IndexItem *indItem;
	    while ( ( indItem = it.current() ) != 0 ) {
		QFileInfo fi( dir + indItem->reference );
		lst.append( IndexKeyword( indItem->keyword, fi.absFilePath() ) );
		if ( progressPrepare )
		    progressPrepare->setProgress( progressPrepare->progress() + fi.absFilePath().length() * 1.6 );
		++it;
	    }
	}
    }
    if ( !lst.isEmpty() )
	qHeapSort( lst );

    QFile indexout( QDir::homeDirPath() + "/.indexdb" );
    if ( indexout.open( IO_WriteOnly ) ) {
	QDataStream s( &indexout );
	s << fileAges;
	s << lst;
    }
    indexout.close();
}

void HelpDialog::setupTitleMap()
{
    if ( contentsDone )
	return;
    contentsDone = TRUE;
    newFullTextIndex = FALSE;
    QFile titleFile( QDir::homeDirPath() + "/.titlemapdb" );
    if ( !titleFile.open( IO_ReadOnly ) ) {
	newFullTextIndex = TRUE;
	buildTitlemapDB();
	titleFile.open( IO_ReadOnly );
    }

    QDataStream ds( &titleFile );
    Q_UINT32 fileAges;
    ds >> fileAges;
    if ( fileAges != getFileAges() ) {
	titleFile.close();
	newFullTextIndex = TRUE;
	buildTitlemapDB();
	if ( !titleFile.open( IO_ReadOnly ) ) {
	    QMessageBox::warning( help, tr( "Qt Assistant" ),
		tr( "Cannot open the title file %1" ).arg( QFileInfo( titleFile ).absFilePath() ) );
	    return;
	}
	ds.setDevice( &titleFile );
	ds >> fileAges;
    }
    ds >> titleMap;
    titleFile.close();
    qApp->processEvents();
}

void HelpDialog::buildTitlemapDB()
{
    QSettings settings;
    QStringList docuFiles = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles" );

    titleMap.clear();
    Q_UINT32 fileAges = 0;

    for( QStringList::iterator it = docuFiles.begin(); it != docuFiles.end(); it++ ) {
	DocuParser handler;
	QFile file( *it );
	fileAges += QFileInfo( file ).lastModified().toTime_t();
	QXmlInputSource source( file );
	QXmlSimpleReader reader;
	reader.setContentHandler( &handler );
	reader.setErrorHandler( &handler );
	bool ok = reader.parse( source );
	file.close();
	if( ok ){
	    if( !isValidCategory( handler.getCategory() ) )
		continue;

	    QFileInfo finfo( *it );
	    QString dir = finfo.dirPath( TRUE ) + "/";
	    QPtrList<ContentItem> contLst = handler.getContentItems();
	    QPtrListIterator<ContentItem> iter( contLst );
	    ContentItem *contItem;
	    while ( (contItem = iter.current()) != 0 ) {
		QFileInfo link( dir + contItem->reference.simplifyWhiteSpace() );
		titleMap[ link.absFilePath() ] = contItem->title.stripWhiteSpace();
		++iter;
	    }
	}
    }
    QFile titleFile( QDir::homeDirPath() + "/.titlemapdb" );
    if ( titleFile.open( IO_WriteOnly ) ) {
	QDataStream s( &titleFile );
	s << fileAges;
	s << titleMap;
    }
    titleFile.close();
}

void HelpDialog::currentTabChanged( const QString &s )
{
    if ( s.contains( tr( "&Index" ) ) ) {
	if ( !indexDone )
	    QTimer::singleShot( 0, this, SLOT( loadIndexFile() ) );
    } else if ( s.contains( tr( "&Bookmarks" ) ) ) {
	if ( !bookmarksInserted )
	    insertBookmarks();
    } else if ( s.contains( tr( "Con&tents" ) ) ) {
	if ( !contentsInserted )
	    QTimer::singleShot( 0, this, SLOT( insertContents() ) );
    } else if ( s.contains( tr( "&Search" ) ) ) {
	    QTimer::singleShot( 0, this, SLOT( setupFullTextIndex() ) );
    }
}

void HelpDialog::currentIndexChanged( QListBoxItem * )
{
}

void HelpDialog::showTopic()
{
    if ( tabWidget->tabLabel( tabWidget->currentPage() ).contains( tr( "Index" ) ) )
	showIndexTopic();
    else if ( tabWidget->tabLabel( tabWidget->currentPage() ).contains( tr( "Bookmarks" ) ) )
	showBookmarkTopic();
    else if ( tabWidget->tabLabel( tabWidget->currentPage() ).contains( tr( "Con&tents" ) ) )
	showContentsTopic();
}

void HelpDialog::showIndexTopic()
{
    QListBoxItem *i = listIndex->item( listIndex->currentItem() );
    if ( !i )
	return;

    editIndex->blockSignals( TRUE );
    editIndex->setText( i->text() );
    editIndex->blockSignals( FALSE );

    HelpNavigationListItem *item = (HelpNavigationListItem*)i;

    QStringList links = item->links();
    if ( links.count() == 1 ) {
	emit showLink( links.first() );
    } else {
	QStringList::Iterator it = links.begin();
	QStringList linkList;
	QStringList linkNames;
	for ( ; it != links.end(); ++it ) {
	    linkList << *it;
	    linkNames << titleOfLink( *it );
	}
	QString link = TopicChooser::getLink( this, linkNames, linkList, i->text() );
	if ( !link.isEmpty() )
	    emit showLink( link );
    }
}

void HelpDialog::searchInIndex( const QString &s )
{
    QListBoxItem *i = listIndex->firstItem();
    QString sl = s.lower();
    while ( i ) {
	QString t = i->text();
	if ( t.length() >= sl.length() &&
	     i->text().left(s.length()).lower() == sl ) {
	    listIndex->setCurrentItem( i );
	    break;
	}
	i = i->next();
    }
}

QString HelpDialog::titleOfLink( const QString &link )
{
    QString s( link );
    s.remove( s.find( '#' ), s.length() );
    s = titleMap[ s ];
    if ( s.isEmpty() )
	return link;
    return s;
}

bool HelpDialog::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return TRUE;

    if ( o == editIndex && e->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Up ) {
	    int i = listIndex->currentItem();
	    if ( --i >= 0 ) {
		listIndex->setCurrentItem( i );
		editIndex->blockSignals( TRUE );
		editIndex->setText( listIndex->currentText() );
		editIndex->blockSignals( FALSE );
	    }
	    return TRUE;
	} else if ( ke->key() == Key_Down ) {
	    int i = listIndex->currentItem();
	    if ( ++i < int(listIndex->count()) ) {
		listIndex->setCurrentItem( i );
		editIndex->blockSignals( TRUE );
		editIndex->setText( listIndex->currentText() );
		editIndex->blockSignals( FALSE );
	    }
	    return TRUE;
	} else if ( ke->key() == Key_Next || ke->key() == Key_Prior ) {
	    QApplication::sendEvent( listIndex, e);
	    editIndex->blockSignals( TRUE );
	    editIndex->setText( listIndex->currentText() );
	    editIndex->blockSignals( FALSE );
	}
    }

    return QWidget::eventFilter( o, e );
}

void HelpDialog::addBookmark()
{
    if ( !bookmarksInserted )
	insertBookmarks();
    QString link = QUrl( viewer->context(), viewer->source() ).path();
    QString title = viewer->documentTitle();
    if ( title.isEmpty() )
	title = titleOfLink( link );
    HelpNavigationContentsItem *i = new HelpNavigationContentsItem( listBookmarks, 0 );
    i->setText( 0, title );
    i->setLink( link );
    saveBookmarks();
    help->updateBookmarkMenu();
}

void HelpDialog::removeBookmark()
{
    if ( !listBookmarks->currentItem() )
	return;

    delete listBookmarks->currentItem();
    saveBookmarks();
    help->updateBookmarkMenu();
}

void HelpDialog::insertBookmarks()
{
    if ( bookmarksInserted )
	return;
    bookmarksInserted = TRUE;
    QFile f( QDir::homeDirPath() + "/.bookmarks" );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    while ( !ts.atEnd() ) {
	HelpNavigationContentsItem *i = new HelpNavigationContentsItem( listBookmarks, 0 );
	i->setText( 0, ts.readLine() );
	i->setLink( ts.readLine() );
    }
    help->updateBookmarkMenu();
}

void HelpDialog::currentBookmarkChanged( QListViewItem * )
{
}

void HelpDialog::showBookmarkTopic()
{
    if ( !listBookmarks->currentItem() )
	return;

    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)listBookmarks->currentItem();
    emit showLink( i->link() );

}

void HelpDialog::saveBookmarks()
{
    QFile f( QDir::homeDirPath() + "/.bookmarks" );
    if ( !f.open( IO_WriteOnly ) )
	return;
    QTextStream ts( &f );
    QListViewItemIterator it( listBookmarks );
    for ( ; it.current(); ++it ) {
	HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)it.current();
	ts << i->text( 0 ) << endl;
	ts << i->link() << endl;
    }
    f.close();
}

void HelpDialog::insertContents()
{
    if ( contentsInserted )
	return;
    contentsInserted = TRUE;
    listContents->clear();
    setCursor( waitCursor );
    if ( !contentsDone )
	setupTitleMap();

    listContents->setSorting( -1 );
#ifdef QT_PALMTOPCENTER_DOCS
    HelpNavigationContentsItem *qtDocu;
    qtDocu = new HelpNavigationContentsItem( listContents, 0 );
    qtDocu->setText( 0, tr( "Qtopia Desktop Documentation" ) );
    qtDocu->setLink( documentationPath + "/qtopiadesktop.html" );
    qtDocu->setPixmap( 0, *bookPixmap );
#endif

    QSettings settings;
    QStringList addDocuFiles = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles" );
    QStringList::iterator i = addDocuFiles.begin();
    for( ; i != addDocuFiles.end(); i++ ){
	HelpNavigationContentsItem *additionalDocu;
	QMimeSourceFactory *mime = QMimeSourceFactory::defaultFactory();
	QFileInfo fi( *i );
	mime->addFilePath( fi.dirPath( TRUE ) );
	mime->setExtensionType("html","text/html;charset=UTF-8");
	mime->setExtensionType("png", "image/png" );
	mime->setExtensionType("jpg", "image/jpeg" );
	mime->setExtensionType("jpeg", "image/jpeg" );
	additionalDocu = new HelpNavigationContentsItem( listContents, 0 );
	additionalDocu->setPixmap( 0, *bookPixmap );
	bool ok = insertContents( *i, additionalDocu );
	if( !ok )
	    delete additionalDocu;
    }
    setCursor( arrowCursor );

#ifdef QT_PALMTOPCENTER_DOCS
    settings.insertSearchPath( QSettings::Unix,
			       QDir::homeDirPath() + "/.palmtopcenter/" );
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QString manualdir = "qtopiadesktop.html";
    //insertContents( manualdir, tr( "Qtopia Desktop Manual" ), lastItem, handbook );
#endif
}

bool HelpDialog::insertContents( const QString &filename, HelpNavigationContentsItem *newEntry )
{
    QFileInfo fi( filename );
    QString dir = fi.dirPath() + "/";
    QFile xmlFile( filename );
    QXmlInputSource source( &xmlFile );
    QXmlSimpleReader reader;
    DocuParser handler;
    reader.setContentHandler( &handler );
    reader.setErrorHandler( &handler );
    bool ok = reader.parse( source );
    xmlFile.close();
    if( !ok ){
	QMessageBox::critical( this, "Parse Error", handler.errorProtocol() );
	return FALSE;
    }

    if( !isValidCategory( handler.getCategory() ) )
	return FALSE;

    HelpNavigationContentsItem *contentEntry;
    QPtrStack<HelpNavigationContentsItem> stack;
    stack.clear();
    int depth = 0;
    bool root = FALSE;
    QPtrList<ContentItem> contentList = handler.getContentItems();

    HelpNavigationContentsItem *lastItem[64];
    for( int j = 0; j < 64; ++j )
	lastItem[j] = 0;
    ContentItem *item;
    for( item = contentList.first(); item; item = contentList.next() ){
	if( item->depth == 0 ){
	    newEntry->setText( 0, item->title );
	    newEntry->setLink( dir + item->reference );
	    stack.push( newEntry );
	    depth = 1;
	    root = TRUE;
	}
	else{
	    if( (item->depth > depth) && (root) ) {
		depth = item->depth;
		stack.push( contentEntry );
	    }
	    if( item->depth == depth ) {
		contentEntry = new HelpNavigationContentsItem( stack.top(), lastItem[ depth ] );
		lastItem[ depth ] = contentEntry;
		contentEntry->setText( 0, item->title );
		contentEntry->setLink( dir + item->reference );
	    }
	    else if( item->depth < depth ) {
		stack.pop();
		depth--;
		item = contentList.prev();
	    }
	}
    }
    return TRUE;
}

void HelpDialog::insertContents( const QString &filename, const QString &titl,
				 HelpNavigationContentsItem *lastItem,
				 HelpNavigationContentsItem *handbook )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return;
    QFileInfo fi( filename );
    QString dir = fi.dirPath();
    QTextStream ts( &file );
    QString text = ts.read();
    text = text.simplifyWhiteSpace();
    QValueList<Entry> entries;

    int i = text.find( titl );

    QString link, title;
    int depth = 0;
    while ( i < (int)text.length() ) {
	QChar c = text[ i ];
	if ( c == '<' ) {
	    ++i;
	    c = text[ i ];
	    if ( c == 'a' || c == 'A' ) {
		int j = text.find( "\"", i );
		int k = text.find( "\"", j + 1 );
		link = text.mid( j + 1, k - j - 1 );
		k = text.find( ">", k ) + 1;
		j = text.find( "<", k );
		title = text.mid( k, j - k );
		title = title.simplifyWhiteSpace();
		if ( title == "Next" ) {
		    i = text.length();
		    continue;
		}
		if ( !title.isEmpty() && !link.startsWith( "http:" ) ) {
		    Entry e;
		    e.link = link;
		    e.title = title;
		    e.depth = depth;
		    entries.append( e );
		}
	    }
	    if ( c == 'd' || c == 'D' ) {
		++i;
		c = text[ i ];
		if ( c == 'l' || c == 'L' )
		    depth++;
	    }
	    if ( c == '/' ) {
		++i;
		c = text[ i ];
		if ( c == 'd' || c == 'D' ) {
		    ++i;
		    c = text[ i ];
		    if ( c == 'l' || c == 'L' )
			depth--;
		}
	    }
	}
	++i;
    }

    int oldDepth = -1;
    lastItem = (HelpNavigationContentsItem*)handbook;
    for ( QValueList<Entry>::Iterator it2 = entries.begin(); it2 != entries.end(); ++it2 ) {
	if ( (*it2).depth == oldDepth )
	    lastItem = new HelpNavigationContentsItem( lastItem->parent(), lastItem );
	else if ( (*it2).depth > oldDepth )
	    lastItem = new HelpNavigationContentsItem( lastItem, lastItem );
	else if ( (*it2).depth < oldDepth ) {
	    int diff = oldDepth - (*it2).depth;
	    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)lastItem->parent(), *i2 = lastItem;
	    while ( diff > 0 ) {
		i = (HelpNavigationContentsItem*)i->parent();
		i2 = (HelpNavigationContentsItem*)i2->parent();
		--diff;
	    }
	    lastItem = new HelpNavigationContentsItem( i, i2 );
	}
	oldDepth = (*it2).depth;
	lastItem->setText( 0, (*it2).title );
	lastItem->setLink( dir + "/" + (*it2).link );
    }

}

void HelpDialog::currentContentsChanged( QListViewItem * )
{
}

void HelpDialog::showContentsTopic()
{
    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)listContents->currentItem();
    emit showLink( i->link() );
}

void HelpDialog::toggleContents()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 0 ) {
	tabWidget->setCurrentPage( 0 );
	parentWidget()->show();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::toggleIndex()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 1 || !editIndex->hasFocus() ) {
	tabWidget->setCurrentPage( 1 );
	parentWidget()->show();
	editIndex->setFocus();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::toggleBookmarks()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 2 ) {
	tabWidget->setCurrentPage( 2 );
	parentWidget()->show();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::toggleSearch()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 3 ) {
	tabWidget->setCurrentPage( 3 );
	parentWidget()->show();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::setupFullTextIndex()
{
    if ( fullTextIndex )
	return;

    QMap<QString, QString>::ConstIterator it = titleMap.begin();
    QStringList documentList;
    for ( ; it != titleMap.end(); ++it )
	documentList << it.key();

    fullTextIndex = new Index( documentList, QDir::homeDirPath() );
    connect( fullTextIndex, SIGNAL( indexingProgress( int ) ),
	     this, SLOT( setIndexingProgress( int ) ) );
    QFile f( QDir::homeDirPath() + "/.indexdb.dict" );
    if ( !f.exists() || newFullTextIndex ) {
	help->statusBar()->clear();
	setCursor( waitCursor );
	labelPrepare->setText( tr( "indexing files..." ) );
	progressPrepare->setTotalSteps( 100 );
	progressPrepare->reset();
	progressPrepare->show();
	framePrepare->show();
	qApp->processEvents();
	fullTextIndex->makeIndex();
	fullTextIndex->writeDict();
	progressPrepare->setProgress( 100 );
	framePrepare->hide();
	setCursor( arrowCursor );
    } else {
	setCursor( waitCursor );
	help->statusBar()->message( tr( "reading dictionary..." ) );
	qApp->processEvents();
	fullTextIndex->readDict();
	help->statusBar()->message( tr( "done." ), 3000 );
	setCursor( arrowCursor );
    }
}

void HelpDialog::setIndexingProgress( int prog )
{
    progressPrepare->setProgress( prog );
    qApp->processEvents();
}

void HelpDialog::startSearch()
{
    QString str = termsEdit->text();
    str = str.replace( "\'", "\"" );
    str = str.replace( "`", "\"" );
    str = str.replace( QRegExp( "\\s[A-Za-z0-9]?\\s" ), " " );
    terms = QStringList::split( " ", str );
    QStringList termSeq;
    QStringList seqWords;
    QStringList::iterator it = terms.begin();
    for ( ; it != terms.end(); ++it ) {
	(*it) = (*it).simplifyWhiteSpace();
	(*it) = (*it).lower();
	(*it) = (*it).replace( "\"", "" );
    }
    if ( str.contains( '\"' ) ) {
	if ( (str.contains( '\"' ))%2 == 0 ) {
	    int beg = 0;
	    int end = 0;
	    QString s;
	    beg = str.find( '\"', beg );
	    while ( beg != -1 ) {
		beg++;
		end = str.find( '\"', beg );
		s = str.mid( beg, end - beg );
		s = s.lower();
		s = s.simplifyWhiteSpace();
		if ( s.contains( '*' ) )
		    return;
		seqWords += QStringList::split( ' ', s );
		termSeq << s;
		beg = str.find( '\"', end + 1);
	    }
	} else {
	    QMessageBox::warning( this, tr( "full text search" ),
		tr( "The closing quotation mark is missing." ) );
	    return;
	}
    }
    setCursor( waitCursor );
    foundDocs.clear();
    foundDocs = fullTextIndex->query( terms, termSeq, seqWords );
    QString msg( QString( "%1 documents found." ).arg( foundDocs.count() ) );
    help->statusBar()->message( tr( msg ), 3000 );
    resultBox->clear();
    for ( it = foundDocs.begin(); it != foundDocs.end(); ++it )
	resultBox->insertItem( fullTextIndex->getDocumentTitle( *it )  );
    setCursor( arrowCursor );
}

void HelpDialog::showSearchHelp()
{
    emit showLink( documentationPath + "/assistant-5.html" );
}

void HelpDialog::showResultPage( int page )
{
    viewer->blockScrolling( TRUE );
    help->showLink( foundDocs[page] );
    viewer->sync();

    int minPar = INT_MAX;
    int minIndex = INT_MAX;
    QStringList::ConstIterator it = terms.begin();
    for ( ; it != terms.end(); ++it ) {
	int para = 0;
	int index = 0;
	bool found = viewer->find( *it, FALSE, TRUE, TRUE, &para, &index );
	while ( found ) {
	    if ( para < minPar ) {
		minPar = para;
		minIndex = index;
	    }
	    viewer->setColor( QColor( "red" ) );
	    found = viewer->find( *it, FALSE, TRUE, TRUE );
	}
    }
    viewer->blockScrolling( FALSE );
    viewer->setCursorPosition( minPar, minIndex );
}
