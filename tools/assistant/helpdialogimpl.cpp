/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
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
#include "topicchooserimpl.h"
#include "docuparser.h"
#include "mainwindow.h"

#include <stdlib.h>

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
//#ifdef QT_PALMTOPCENTER_DOCS
#include <qsettings.h>
//#endif

static QPixmap *bookPixmap = 0;

class SortableString : public QString
{
public:
    SortableString() {}
    SortableString( const QString& other )
	: QString( other ), key( other )
    {
	// "Chapter 1" becomes "chapter01"; "Chapter 12" becomes
	// "chapter102", which is wrong but good enough
	key.replace( QRegExp("(?=\\b[0-9]\\b)"), "0" );
	key.replace( QRegExp("\\W"), " " );
	key = key.stripWhiteSpace().lower();

	// use original string as second sort criterion
	key += QChar( ' ' ) + other;
    }
    QString key;
};

struct Entry
{
    QString link;
    QString title;
    int depth;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const Entry& ) const { return FALSE; }
#endif
};

bool operator<=( const SortableString &s1, const SortableString &s2 )
{ return s1.key <= s2.key; }
bool operator<( const SortableString &s1, const SortableString &s2 )
{ return s1.key < s2.key; }
bool operator>( const SortableString &s1, const SortableString &s2 )
{ return s1.key > s2.key; }

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





HelpDialog::HelpDialog( QWidget *parent, MainWindow *h, QTextBrowser *v )
    : HelpDialogBase( parent, 0, FALSE ), help( h ), viewer( v ), lwClosed( FALSE )
{
    bookPixmap = new QPixmap( book_xpm );
    QMimeSourceFactory *mime = QMimeSourceFactory::defaultFactory();
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
    mime->setExtensionType("html","text/html;charset=UTF-8");
#else
    QString qtdir = getenv( "QTDIR" );
    documentationPath = qtdir + "/doc/html";
    mime->addFilePath( documentationPath );
    mime->addFilePath( qtdir + "/gif/" );
#endif
    indexDone = FALSE;
    contentsDone = FALSE;
    contentsInserted = FALSE;
    bookmarksInserted = FALSE;
    editIndex->installEventFilter( this );
    listBookmarks->header()->hide();
    listBookmarks->header()->setStretchEnabled( TRUE );
    listContents->header()->hide();
    listContents->header()->setStretchEnabled( TRUE );
    framePrepare->hide();
    setupTitleMap();
    connect( qApp, SIGNAL(lastWindowClosed()), SLOT(lastWinClosed()) );
#ifdef QT_PALMTOPCENTER_DOCS
    tabWidget->removePage( contentPage );
#endif
}

void HelpDialog::lastWinClosed()
{
    lwClosed = TRUE;
}

void HelpDialog::generateNewDoc()
{
    QString d( QDir::homeDirPath() );
    QDir dir( d );    
    QStringList list = dir.entryList( ".indexdb*; .titlemapdb*", QDir::Files | QDir::Hidden );
    QStringList::iterator it = list.begin();
    for ( ; it != list.end(); ++it ) {
	if( QFile::exists( QDir::homeDirPath() + "/" + *it ) ){
	    QFile f( QDir::homeDirPath() + "/" + *it );
	    f.remove();
	}
    }
    showCatDoc(); 
}

void HelpDialog::showCatDoc()
{
    indexDone = FALSE;
    contentsDone = FALSE;
    contentsInserted = FALSE;
    QTimer::singleShot( 100, this, SLOT( loadIndexFile() ) );
    setupTitleMap();
    insertContents(); 
}

QString HelpDialog::generateFileNumber()
{
    QSettings settings;
    QStringList list = settings.readListEntry( "/Qt Assistant/categories/available/" );
    QStringList::iterator it = list.begin();
    uint i = 2;
    uint j = 0;    
    
    categoryMap.clear();    
    for ( ; it != list.end(); ++it ) {
	if ( *it == "all" ) 
	    categoryMap["all"] = 1;
	else {
	    categoryMap[*it] = i;
	    i *= 2;
	}
    }	

    list = settings.readListEntry( "/Qt Assistant/categories/selected/" );
    for ( it = list.begin(); it != list.end(); ++it ){
	if ( *it == "all" )
	    return "";
	j += categoryMap[*it];
    }
    return QString("%1").arg( j );
}

bool HelpDialog::isValidCategory( QString category )
{
    QSettings settings;
    QStringList list = settings.readListEntry( "/Qt Assistant/categories/selected/" );
    QStringList::iterator it = list.begin();
    for( ; it != list.end(); ++it ){ 
	QString cat(*it);
	cat = cat.lower();	
	if( (cat == category.lower()) || ( cat == "all" ) )
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
    framePrepare->show();
    qApp->processEvents();
    QString num = generateFileNumber();
    QString indexFile = documentationPath + "/index";

    QSettings settings;
    QStringList addDocuPath = settings.readListEntry( "/Qt Assistant/additionalDocu/Path" );
    QStringList::iterator i = addDocuPath.begin();
    
    int steps = QFileInfo( indexFile ).size();
    for( ; i != addDocuPath.end(); i++ )
	steps += QFileInfo( *i + "index.xml" ).size();
    
    QProgressBar *bar = progressPrepare;
    bar->setTotalSteps( steps );
    bar->setProgress( 0 );

    HelpNavigationListItem *lastItem = 0;
    
    //### if constructed on stack, it will crash on WindowsNT
    QValueList<SortableString>* lst = new QValueList<SortableString>;
    bool buildDb = TRUE;
    QFile f( indexFile );
    if ( QFile::exists( QDir::homeDirPath() + "/.indexdb" + num ) ) {
	QFile indexin( QDir::homeDirPath() + "/.indexdb" + num );
	if ( !indexin.open( IO_ReadOnly ) )
	    goto build_db;

	QDataStream ds( &indexin );
	QDateTime dt;
	uint size;
	ds >> dt;
	ds >> size;
	if ( size != f.size()  || dt != QFileInfo( f ).lastModified() )
	    goto build_db;

	ds >> *lst;
	indexin.close();
	bar->setProgress( bar->totalSteps() );
	qApp->processEvents();
	buildDb = FALSE;
    }

 build_db:
    if ( buildDb ) {
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    while ( !ts.atEnd() && !lastWindowClosed() ) {
		qApp->processEvents();
		if ( lastWindowClosed() )
		    break;
		QString l = ts.readLine();		
		if ( l.find( "::" ) != -1 ) {
		    int i = l.find( "\"" ) + 1;
		    l.remove( i, l.find( "::" ) + 2 - i );		    
		}		
		
		QString buf( l );
		l.remove( 0, l.find( "\"", 1 ) + 2 );
		buf.remove( buf.find( "\"", 1 ) + 1, buf.length() );
		l = buf + " " + documentationPath + "/" + l;		
		
		lst->append( l );
		
		if ( bar )
		    bar->setProgress( bar->progress() + l.length() * 1.3 );
	    }
	    
	    for( i = addDocuPath.begin(); i != addDocuPath.end(); i++ ){	
		DocuIndexParser handler;		
		QFile file( *i + "index.xml" );
		QXmlInputSource source( file );
		QXmlSimpleReader reader;
		reader.setContentHandler( &handler );
		reader.setErrorHandler( &handler );
		bool ok = reader.parse( source );
		file.close();
		if( !ok ){
		    QMessageBox::critical( this, "Parse Error", 
				 handler.errorProtocol() 
				 + "\nSkipping file " + *i + "index.xml" );
		}
		else{
		    if( !isValidCategory( handler.getCategory() ))
			continue;
		    QStringList indexlist = handler.getIndices();
		    QStringList::Iterator j = indexlist.begin();
		    for( ; j != indexlist.end(); j++ ){
			QString l( *j );
			
			QString buf( l );
			l.remove( 0, l.find( "\"", 1 ) + 2 );
			buf.remove( buf.find( "\"", 1 ) + 1, buf.length() );
			l = buf + " " + *i + l;
			
			lst->append( l );
			bar->setProgress( bar->progress() + (*j).length() * 3 );    
		    }
		}
	    }
	    	    
	    if ( lastWindowClosed() ) {
		delete lst;
		return;
	    }
	}
	if ( !lst->isEmpty() ) {
	    qHeapSort( *lst );

	    QFile indexout( QDir::homeDirPath() + "/.indexdb" + num );
	    if ( indexout.open( IO_WriteOnly ) ) {
		QDataStream s( &indexout );
		s << QFileInfo( f ).lastModified();
		s << f.size();
		s << *lst;
	    }
	    indexout.close();
	} else {
	    indexDone = FALSE;
	}
    }

    listIndex->clear();
    QValueList<SortableString>::Iterator it = lst->begin();
    for ( ; it != lst->end(); ++it ) {
	QString s( *it );
	if ( s.find( "::" ) != -1 )
	    continue;
	if ( s[1] == '~' )
	    continue;
	if ( s.find( "http://" ) != -1 ||
	     s.find( "ftp://" ) != -1 ||
	     s.find( "mailto:" ) != -1 )
	    continue;
	int from = s.find( "\"" );
	if ( from == -1 )
	    continue;
	int to = s.findRev( "\"" );
	if ( to == -1 )
	    continue;
	QString link = s.mid( to + 2 );
	s = s.mid( from + 1, to - from - 1 );

	if ( s.isEmpty() )
	    continue;
	s = s.replace( QRegExp( "\\\\" ), "" );
	if ( !lastItem || lastItem->text() != s ){
	    lastItem = new HelpNavigationListItem( listIndex, s );
	}
	lastItem->addLink( link );
    }

    delete lst;
    f.close();

    framePrepare->hide();
    setCursor( arrowCursor );
}

void HelpDialog::setupTitleMap()
{
    if ( contentsDone )
	return;
    contentsDone = TRUE;
    QString num = generateFileNumber();
    QString titleFile  = documentationPath + "/titleindex";
    QFile f2( titleFile );
    bool buildDb = TRUE;
    if ( QFile::exists( QDir::homeDirPath() + "/.titlemapdb" + num ) ) {
	QFile titlein( QDir::homeDirPath() + "/.titlemapdb" + num );
	if ( !titlein.open( IO_ReadOnly ) )
	    goto build_db2;

	QDataStream ds( &titlein );
	QDateTime dt;
	uint size;
	ds >> dt;
	ds >> size;
	if ( size != f2.size() || dt != QFileInfo( f2 ).lastModified() )
	    goto build_db2;
	ds >> titleMap;
	titlein.close();
	qApp->processEvents();
	buildDb = FALSE;
    }

 build_db2:

    if ( buildDb ) {
	if ( !f2.open( IO_ReadOnly ) )
	    return;
	QTextStream ts2( &f2 );
	while ( !ts2.atEnd() ) {
	    QString s = ts2.readLine();
	    int pipe = s.find( '|' );
	    if ( pipe == -1 )
		continue;
	    QString title = s.left( pipe - 1 );
	    QString link = s.mid( pipe + 1 );
	    link = link.simplifyWhiteSpace();
	    link = documentationPath + "/" + link;
	    titleMap[ link ] = title.stripWhiteSpace();
	}
			
	QSettings settings;
	QStringList docuPath = settings.readListEntry( "/Qt Assistant/additionalDocu/Path" );
	
	for( QStringList::iterator it=docuPath.begin(); it != docuPath.end(); it++ ){
	    DocuIndexParser handler;
	    QFile file( *it + "index.xml" );
	    QXmlInputSource source( file );
	    QXmlSimpleReader reader;
	    reader.setContentHandler( &handler );
	    reader.setErrorHandler( &handler );
	    bool ok = reader.parse( source );
	    file.close();
	    if( ok ){
		if( !isValidCategory( handler.getCategory() ))
		    continue;
		QStringList titellist = handler.getTitles();
		QStringList::Iterator i = titellist.begin();
		for( ; i != titellist.end(); i++ ){
		    QString s = *i;
		    int pipe = s.find( '|' );
		    QString link = s.left( pipe - 1 );
		    QString title = s.mid( pipe + 1 );
		    link = link.simplifyWhiteSpace();
		    link = *it + link;
		    titleMap[ link ] = title.stripWhiteSpace();
		}
	    }	    
	}
	
	QFile titleout( QDir::homeDirPath() + "/.titlemapdb" + num );
	if ( titleout.open( IO_WriteOnly ) ) {
	    QDataStream s( &titleout );
	    s << QFileInfo( f2 ).lastModified();
	    s << f2.size();
	    s << titleMap;
	}
	titleout.close();
    }
}

void HelpDialog::currentTabChanged( const QString &s )
{
    if ( s.contains( tr( "Index" ) ) ) {
	if ( !indexDone )
	    QTimer::singleShot( 100, this, SLOT( loadIndexFile() ) );
    } else if ( s.contains( tr( "Bookmarks" ) ) ) {
	if ( !bookmarksInserted )
	    insertBookmarks();
    } else if ( s.contains( tr( "Con&tents" ) ) ) {
	if ( !contentsInserted )
	    insertContents();
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
	emit showLink( links.first(), item->text() );
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
	    emit showLink( link, i->text() );
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
    QString title = titleOfLink( link );
    HelpNavigationContentsItem *i = new HelpNavigationContentsItem( listBookmarks, 0 );
    i->setText( 0, title );
    i->setLink( QUrl( link ).fileName() );
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
    emit showLink( i->link(), i->text( 0 ) );

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
    if ( !contentsDone )
	setupTitleMap();

    listContents->setSorting( -1 );
    HelpNavigationContentsItem *qtDocu, *handbook, *linguistDocu, *assistantDocu;
#ifdef QT_PALMTOPCENTER_DOCS
    qtDocu = new HelpNavigationContentsItem( listContents, 0 );
    qtDocu->setText( 0, tr( "Qtopia Desktop Documentation" ) );
    qtDocu->setLink( documentationPath + "/qtopiadesktop.html" );
    qtDocu->setPixmap( 0, *bookPixmap );
#else    
    qtDocu = new HelpNavigationContentsItem( listContents, 0 );
    qtDocu->setText( 0, tr( "Qt Reference Documentation" ) );
    qtDocu->setLink( documentationPath + "/index.html" );
    qtDocu->setPixmap( 0, *bookPixmap );
    handbook = new HelpNavigationContentsItem( listContents, 0 );
    handbook->setText( 0, tr( "Qt Designer Manual" ) );
    handbook->setLink( documentationPath + "/designer-manual.html" );
    handbook->setPixmap( 0, *bookPixmap );
    linguistDocu = new HelpNavigationContentsItem( listContents, 0 );
    linguistDocu->setText( 0, tr( "Qt Linguist Manual" ) );
    linguistDocu->setLink( documentationPath + "/linguist-manual.html" );
    linguistDocu->setPixmap( 0, *bookPixmap );
    assistantDocu = new HelpNavigationContentsItem( listContents, 0 );
    assistantDocu->setText( 0, tr( "Qt Assistant Manual" ) );
    assistantDocu->setLink( documentationPath + "/assistant.html" );
    assistantDocu->setPixmap( 0, *bookPixmap );
    
    QSettings settings;  
    QStringList addDocuPath = settings.readListEntry( "/Qt Assistant/additionalDocu/Path" );
    QStringList::iterator i = addDocuPath.begin();
    for( ; i != addDocuPath.end(); i++ ){	
	HelpNavigationContentsItem *additionalDocu;	
	QMimeSourceFactory *mime = QMimeSourceFactory::defaultFactory();
	mime->addFilePath( *i );
	mime->setExtensionType("html","text/html;charset=UTF-8");
	mime->setExtensionType("png", "image/png" );
	mime->setExtensionType("jpg", "image/jpeg" );
	mime->setExtensionType("jpeg", "image/jpeg" );	
	additionalDocu = new HelpNavigationContentsItem( listContents, 0 );
	additionalDocu->setPixmap( 0, *bookPixmap );
	bool ok = insertContents( *i, "contents.xml", additionalDocu );
	if( !ok )
	    delete additionalDocu;
    }
    
#endif
    
    HelpNavigationContentsItem *lastItem = 0;
    HelpNavigationContentsItem *lastGroup = 0;

    QValueList<SortableString>* lst = new QValueList<SortableString>;
    for ( QMap<QString, QString>::Iterator it = titleMap.begin(); it != titleMap.end(); ++it ) {
	QString s = *it + " | " + it.key();
	s = s.stripWhiteSpace();
	if ( s.lower().find( "easter egg" ) != -1 ||
	     s.lower().find( "easteregg" ) != -1 )
	    continue;
	lst->append( s );
    }

    qHeapSort( *lst );

    for ( QValueList<SortableString>::Iterator sit = lst->begin(); sit != lst->end(); ++sit ) {
	QString s = *sit;
	s = s.stripWhiteSpace();
	int i = s.find( " - " );
	if ( i == -1 ) {
	    if ( lastGroup ) {
		lastItem = lastGroup;
		lastGroup = 0;
	    }

	    QListViewItem *oldLast = lastItem;
	    lastItem = new HelpNavigationContentsItem( qtDocu, lastItem );
	    QString txt = s;
	    QString title, link;
	    i = txt.find( " | " );
	    if ( i == -1 ) {
		title = txt;
	    } else {
		title = txt.left( i );
		link = txt.mid( i + 3 );
		if ( oldLast && oldLast->text( 0 ).contains( title ) ) {
		    QString s2 = link;
		    i = s2.find( '.' );
		    if ( i != -1 )
			s2 = s2.left( i );
		    s2[ 0 ] = s2[ 0 ].upper();
		    title += " (" + s2 + ")";
		}
	    }
	    lastItem->setText( 0, title );
	    lastItem->setLink( link );
	    lastGroup = 0;
	} else {
	    QString preMinus = s.left( i );
	    QListViewItemIterator lit( qtDocu );
	    if ( !lastGroup || lastGroup->text( 0 ).lower() != preMinus.lower() ) {
		lastItem = new HelpNavigationContentsItem( qtDocu, lastItem );
		lastItem->setText( 0, preMinus );
		lastGroup = lastItem;
	    }
	    QString txt = s.mid( i + 3 );
	    QString title, link;
	    i = txt.find( " | " );
	    if ( i == -1 ) {
		title = txt;
	    } else {
		title = txt.left( i );
		link = txt.mid( i + 3 );
		if ( lastItem && title == lastItem->text( 0 ) ) {
		    QString s2 = link;
		    i = s2.find( '.' );
		    if ( i != -1 )
			s2 = s2.left( i );
		    s2[ 0 ] = s2[ 0 ].upper();
		    title += " (" + s2 + ")";
		}
	    }

	    lastItem = new HelpNavigationContentsItem( lastGroup, lastItem );
	    lastItem->setText( 0, title );
	    lastItem->setLink( link );
	}
    }
    delete lst;
#ifdef QT_PALMTOPCENTER_DOCS	
    settings.insertSearchPath( QSettings::Unix,
			       QDir::homeDirPath() + "/.palmtopcenter/" );
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

//     QString basePath;
//     basePath = settings.readEntry( "/palmtopcenter/qtopiadir", QString::null, &okay );    
    QString manualdir = "qtopiadesktop.html";
    insertContents( manualdir, tr( "Qtopia Desktop Manual" ), lastItem, handbook );
#else    
    QString manualdir = QString( getenv( "QTDIR" ) ) + "/doc/html/designer-manual.html";
    insertContents( manualdir, tr( "Qt Designer Manual" ), lastItem, handbook );
    manualdir = QString( getenv( "QTDIR" ) ) + "/doc/html/linguist-manual.html";
    insertContents( manualdir, tr( "Qt Linguist Manual" ), lastItem, linguistDocu );
    manualdir = QString( getenv( "QTDIR" ) ) + "/doc/html/assistant.html";
    insertContents( manualdir, tr( "Qt Assistant" ), lastItem, assistantDocu );
#endif
}

bool HelpDialog::insertContents( QString additionalPath, const QString &filename, 
				 HelpNavigationContentsItem *newEntry )
{       
    QFile xmlFile( additionalPath + filename );
    QXmlInputSource source( &xmlFile );
    QXmlSimpleReader reader;
    DocuContentParser *handler = new DocuContentParser();
    reader.setContentHandler( handler );
    reader.setErrorHandler( handler );
    bool ok = reader.parse( source );    
    xmlFile.close();
    if( !ok ){
	QMessageBox::critical( this, "Parse Error", handler->errorProtocol() );
	return FALSE;
    }
    
    
    if( !isValidCategory( handler->getCategory() )) 	
	return FALSE;    

    HelpNavigationContentsItem *contentEntry;
    QPtrStack<HelpNavigationContentsItem> stack;
    stack.clear();
    int depth = 0;
    bool root = FALSE;    
    QPtrList<ContentItem> contentList = handler->getContentItems();
    ContentItem *item;
    for( item = contentList.first(); item; item = contentList.next() ){	
	if( item->getDepth() == 0 ){
	    newEntry->setText( 0, tr( item->getContentName() ));
	    newEntry->setLink( additionalPath + item->getContentRef() );
	    stack.push( newEntry );
	    depth = 1;
	    root = TRUE;
	}	
	else{
	    if( (item->getDepth() > depth) && (root) ){
		depth = item->getDepth();
		stack.push( contentEntry );
	    }	    
	    if( item->getDepth() == depth ){
		contentEntry = new HelpNavigationContentsItem( stack.top(), 0 );
		contentEntry->setText( 0, tr( item->getContentName() ));
		contentEntry->setLink( additionalPath + item->getContentRef() );
	    }	    
	    else if( item->getDepth() < depth ){
		stack.pop();
		depth--;
		item = contentList.prev();
	    }    
	}	
    }
    newEntry->sortChildItems( 1, FALSE );
    return TRUE;    
}

void HelpDialog::insertContents( const QString &filename, const QString &titl,
				 HelpNavigationContentsItem *lastItem,
				 HelpNavigationContentsItem *handbook )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return;
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
	lastItem->setLink( (*it2).link );
    }

}

void HelpDialog::currentContentsChanged( QListViewItem * )
{
}

void HelpDialog::showContentsTopic()
{
    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)listContents->currentItem();
    emit showLink( i->link(), i->text( 0 ) );
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
