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
#include <qsettings.h>

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
	     c == '\"' || c == ' ' ||
	     c == '-' || c == '*' ) )
	    return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

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

    QString base( qInstallPathDocs() );
    documentationPath = base + "/html";
    mime->addFilePath( documentationPath );
    mime->addFilePath( base + "/../gif/" );

    editIndex->installEventFilter( this );
    listBookmarks->header()->hide();
    listBookmarks->header()->setStretchEnabled( TRUE );
    listContents->header()->hide();
    listContents->header()->setStretchEnabled( TRUE );
    framePrepare->hide();
    connect( qApp, SIGNAL(lastWindowClosed()), SLOT(lastWinClosed()) );

    termsEdit->setValidator( new SearchValidator( termsEdit ) );

    contentList.setAutoDelete( TRUE );
    contentList.clear();

    initDoneMsgShown = FALSE;
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
    contentList.clear();
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
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QStringList list = settings.readListEntry( "/Qt Assistant/3.1/CategoriesSelected" );
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
    showInitDoneMessage();
    setCursor( arrowCursor );
}

Q_UINT32 HelpDialog::getFileAges()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QStringList addDocuFiles = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles" );
    QStringList::iterator i = addDocuFiles.begin();

    Q_UINT32 fileAges = 0;
    for( ; i != addDocuFiles.end(); i++ ) {
	QFileInfo fi( *i );
	if ( fi.exists() )
	    fileAges += fi.lastModified().toTime_t();
    }

    return fileAges;
}

void HelpDialog::buildKeywordDB()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
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
	if ( !file.exists() ) {
	    QMessageBox::warning( this, tr( "Warning" ),
		tr( "Documentation file %1 does not exist!\n"
		    "Skipping file." ).arg( QFileInfo( file ).absFilePath() ) );
	    HelpDialog::removeDocFile( *i );
	    continue;
        }
	fileAges += QFileInfo( file ).lastModified().toTime_t();
	QXmlInputSource source( file );
	QXmlSimpleReader reader;
	reader.setContentHandler( &handler );
	reader.setErrorHandler( &handler );
	bool ok = reader.parse( source );
	file.close();
	if( !ok ){
	    QString msg = QString( "In file %1:\n%2" )
			  .arg( QFileInfo( file ).absFilePath() )
			  .arg( handler.errorProtocol() );
	    QMessageBox::critical( this, tr( "Parse Error" ), tr( msg ) );
	    continue;
	}
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
    if ( !lst.isEmpty() )
	qHeapSort( lst );

    QFile indexout( QDir::homeDirPath() + "/.indexdb" );
    if ( indexout.open( IO_WriteOnly ) ) {
	QDataStream s( &indexout );
	s << fileAges;
	s << lst;
	indexout.close();
    }
}

void HelpDialog::setupTitleMap()
{
    if ( contentsDone )
	return;
    if ( contentList.isEmpty() )
	getAllContents();

    QDictIterator<ContentList> lstIt( contentList );
    for ( ; lstIt.current(); ++lstIt ) {
	QValueList<ContentItem> &lst = *(lstIt.current());
	QValueList<ContentItem>::iterator it;
	QFileInfo finfo( lstIt.currentKey() );
	QString dir = finfo.dirPath( TRUE ) + "/";
	for ( it = lst.begin(); it != lst.end(); ++it ) {
	    QFileInfo link( dir + (*it).reference.simplifyWhiteSpace() );
	    titleMap[ link.absFilePath() ] = (*it).title.stripWhiteSpace();
	}
    }
    qApp->processEvents();
}

void HelpDialog::getAllContents()
{
    QFile contentFile( QDir::homeDirPath() + "/.contentdb" );
    if ( !contentFile.open( IO_ReadOnly ) ) {
	buildContentDict();
	return;
    }

    QDataStream ds( &contentFile );
    Q_UINT32 fileAges;
    ds >> fileAges;
    if ( fileAges != getFileAges() ) {
	contentFile.close();
	buildContentDict();
	return;
    }
    QString key;
    QValueList<ContentItem> lst;
    while ( !ds.atEnd() ) {
	ds >> key;
	ds >> lst;
	contentList.insert( key, new QValueList<ContentItem>( lst ) );
    }
    contentFile.close();
    qApp->processEvents();

}

void HelpDialog::buildContentDict()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QStringList docuFiles = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles" );

    qApp->processEvents();
    Q_UINT32 fileAges = 0;
    for( QStringList::iterator it = docuFiles.begin(); it != docuFiles.end(); it++ ) {
	DocuParser handler;
	QFile file( *it );
	if ( !file.exists() ) {
	    QMessageBox::warning( this, tr( "Warning" ),
	    tr( "Documentation file %1 does not exist!\n"
	        "Skipping file." ).arg( QFileInfo( file ).absFilePath() ) );
	    HelpDialog::removeDocFile( *it );
	    continue;
        }
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
	    contentList.insert( *it, new QValueList<ContentItem>( handler.getContentItems() ) );
	} else {
	    QString msg = QString( "In file %1:\n%2" )
			  .arg( QFileInfo( file ).absFilePath() )
			  .arg( handler.errorProtocol() );
	    QMessageBox::critical( this, tr( "Parse Error" ), tr( msg ) );
	    continue;
	}
    }

    QFile contentOut( QDir::homeDirPath() + "/.contentdb" );
    if ( contentOut.open( IO_WriteOnly ) ) {
	QDataStream s( &contentOut );
	s << fileAges;
	QDictIterator<ContentList> it( contentList );
	for ( ; it.current(); ++it ) {
	    s << it.currentKey();
	    s << *(it.current());
	}
	contentOut.close();
    }
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

void HelpDialog::showInitDoneMessage()
{
    if ( initDoneMsgShown )
	return;
    initDoneMsgShown = TRUE;
    help->statusBar()->message( tr( "done." ), 3000 );
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
    if ( listBookmarks->firstChild() ) {
	listBookmarks->setSelected( listBookmarks->firstChild(), TRUE );
    }
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
    showInitDoneMessage();
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

    if ( contentList.isEmpty() )
	getAllContents();

    contentsInserted = TRUE;
    listContents->clear();
    setCursor( waitCursor );
    if ( !contentsDone )
	setupTitleMap();

    listContents->setSorting( -1 );

    QPtrList<ContentItem> lst;
    QDictIterator<ContentList> lstIt( contentList );
    for ( ; lstIt.current(); ++lstIt ) {
	QMimeSourceFactory *mime = QMimeSourceFactory::defaultFactory();
	QFileInfo fi( lstIt.currentKey() );
	QString dir = fi.dirPath() + "/";
	mime->addFilePath( fi.dirPath( TRUE ) );
	mime->setExtensionType("html","text/html;charset=UTF-8");
	mime->setExtensionType("png", "image/png" );
	mime->setExtensionType("jpg", "image/jpeg" );
	mime->setExtensionType("jpeg", "image/jpeg" );

	HelpNavigationContentsItem *newEntry;
	newEntry = new HelpNavigationContentsItem( listContents, 0 );
	newEntry->setPixmap( 0, *bookPixmap );

	HelpNavigationContentsItem *contentEntry;
	QPtrStack<HelpNavigationContentsItem> stack;
	stack.clear();
	int depth = 0;
	bool root = FALSE;

	HelpNavigationContentsItem *lastItem[64];
	for( int j = 0; j < 64; ++j )
	    lastItem[j] = 0;


	QValueList<ContentItem> &lst = *(lstIt.current());
	QValueList<ContentItem>::iterator it;
	for( it = lst.begin(); it != lst.end(); ++it ){
	    ContentItem item = *it;
	    if( item.depth == 0 ){
		newEntry->setText( 0, item.title );
		newEntry->setLink( dir + item.reference );
		stack.push( newEntry );
		depth = 1;
		root = TRUE;
	    }
	    else{
		if( (item.depth > depth) && root ) {
		    depth = item.depth;
		    stack.push( contentEntry );
		}
		if( item.depth == depth ) {
		    contentEntry = new HelpNavigationContentsItem( stack.top(), lastItem[ depth ] );
		    lastItem[ depth ] = contentEntry;
		    contentEntry->setText( 0, item.title );
		    contentEntry->setLink( dir + item.reference );
		}
		else if( item.depth < depth ) {
		    stack.pop();
		    depth--;
		    item = *(--it);
		}
	    }
	}
    }
    setCursor( arrowCursor );
    showInitDoneMessage();
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

void HelpDialog::removeDocFile( const QString &fileName )
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QStringList lst = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocFiles" );
    QStringList titleLst = settings.readListEntry( "/Qt Assistant/3.1/AdditionalDocTitles" );
    QStringList::iterator it = titleLst.begin();
    QStringList::iterator i = lst.begin();
    for ( ; i != lst.end() && it != titleLst.end(); ++i, ++it ) {
	if ( *i == fileName ) {
	    titleLst.remove( it );
	    lst.remove( i );
	    break;
	}
    }
    settings.writeEntry( "/Qt Assistant/3.1/AdditionalDocFiles", lst );
    settings.writeEntry( "/Qt Assistant/3.1/AdditionalDocTitles" , titleLst );
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
    fullTextIndex->setDictionaryFile( QDir::homeDirPath() + "/.indexdb.dict" );
    fullTextIndex->setDocListFile( QDir::homeDirPath() + "/.indexdb.doc" );
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
	if ( fullTextIndex->makeIndex() == -1 )
	    return;
	fullTextIndex->writeDict();
	progressPrepare->setProgress( 100 );
	framePrepare->hide();
	setCursor( arrowCursor );
	showInitDoneMessage();
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
    QString buf = str;
    str = str.replace( "-", " " );
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
		if ( s.contains( '*' ) ) {
		    QMessageBox::warning( this, tr( "Full Text Search" ),
			tr( "Using a wildcard within phrases is not allowed." ) );
		    return;
		}
		seqWords += QStringList::split( ' ', s );
		termSeq << s;
		beg = str.find( '\"', end + 1);
	    }
	} else {
	    QMessageBox::warning( this, tr( "Full Text Search" ),
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

    terms.clear();
    bool isPhrase = FALSE;
    QString s = "";
    for ( int i = 0; i < buf.length(); ++i ) {
	if ( buf[i] == '\"' ) {
	    isPhrase = !isPhrase;
	    s = s.simplifyWhiteSpace();
	    if ( !s.isEmpty() )
		terms << s;
	    s = "";
	} else if ( buf[i] == ' ' && !isPhrase ) {
	    s = s.simplifyWhiteSpace();
	    if ( !s.isEmpty() )
		terms << s;
	    s = "";
	} else
	    s += buf[i];
    }
    if ( !s.isEmpty() )
	terms << s;

    setCursor( arrowCursor );
}

void HelpDialog::showSearchHelp()
{
    emit showLink( documentationPath + "/assistant-5.html" );
}

void HelpDialog::showResultPage( int page )
{
    viewer->blockScrolling( TRUE );
    viewer->setCursor( waitCursor );
    if ( viewer->source() == foundDocs[page] )
	viewer->reload();
    else
	help->showLink( foundDocs[page] );
    viewer->sync();
    viewer->setCursor( arrowCursor );

    viewer->viewport()->setUpdatesEnabled( FALSE );
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
    viewer->viewport()->setUpdatesEnabled( TRUE );
    viewer->setCursorPosition( minPar, minIndex );
    viewer->updateContents();
}
