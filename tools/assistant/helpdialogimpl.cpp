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

#include "helpdialogimpl.h"
#include "topicchooserimpl.h"
#include "mainwindow.h"

#include <stdlib.h>

#include <qprogressbar.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qprogressbar.h>
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

class MyString : public QString
{
public:
    MyString() {}
    MyString( const QString& other )
	: QString( other ) {
	int from = other.find( "\"" );
	int to = other.findRev( "\"" );
	if ( from != -1 && to != -1 )
	    key = other.mid( from+1, to-from-1 ).lower();
	else
	    key = other.lower();
	// original string as 2nd sort criteria
	key += QChar::null + other;
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

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
bool operator==( const Entry&, const Entry& ) { return FALSE; }
#endif

bool operator<=( const MyString &s1, const MyString &s2 )
{ return s1.key <= s2.key; }
bool operator<( const MyString &s1, const MyString &s2 )
{ return s1.key < s2.key; }
bool operator>( const MyString &s1, const MyString &s2 )
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
    : HelpDialogBase( parent, 0, FALSE ), help( h ), viewer( v )
{
    documentationPath = QString( getenv( "QTDIR" ) ) + "/doc/html";
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
}

void HelpDialog::loadIndexFile()
{
    if ( indexDone )
	return;

    setCursor( waitCursor );
    indexDone = TRUE;
    framePrepare->show();
    qApp->processEvents();

    QString indexFile = documentationPath + "/index";

    QProgressBar *bar = progressPrepare;
    bar->setTotalSteps( QFileInfo( indexFile ).size()  );
    bar->setProgress( 0 );


    HelpNavigationListItem *lastItem = 0;

    //### if constructed on stack, it will crash on WindowsNT
    QValueList<MyString>* lst = new QValueList<MyString>;
    bool buildDb = TRUE;
    QFile f( indexFile );
    if ( QFile::exists( QDir::homeDirPath() + "/.indexdb" ) ) {
	QFile indexin( QDir::homeDirPath() + "/.indexdb" );
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
	    while ( !ts.atEnd() && isVisible() ) {
		qApp->processEvents();
		if ( !isVisible() )
		    break;
		QString l = ts.readLine();
		if ( l.find( "::" ) != -1 ) {
		    int i = l.find( "\"" ) + 1;
		    l.remove( i, l.find( "::" ) + 2 - i );
		}
		lst->append( l );
		if ( bar )
		    bar->setProgress( bar->progress() + l.length() );
	    }
	}
	if ( !lst->isEmpty() ) {
	    qHeapSort( *lst );

	    QFile indexout( QDir::homeDirPath() + "/.indexdb" );
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

    QValueList<MyString>::Iterator it = lst->begin();
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
	QString link = s.mid( to + 2, 0xFFFFFF );
	s = s.mid( from + 1, to - from - 1 );

	if ( s.isEmpty() )
	    continue;
	if ( !lastItem || lastItem->text() != s )
	    lastItem = new HelpNavigationListItem( listIndex, s );
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
    QString titleFile  = documentationPath + "/titleindex";
    QFile f2( titleFile );
    bool buildDb = TRUE;
    if ( QFile::exists( QDir::homeDirPath() + "/.titlemapdb" ) ) {
	QFile titlein( QDir::homeDirPath() + "/.titlemapdb" );
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
	    QString link = s.mid( pipe + 1, 0xFFFFFF );
	    link = link.simplifyWhiteSpace();
	    titleMap[ link ] = title.stripWhiteSpace();
	}

	QFile titleout( QDir::homeDirPath() + "/.titlemapdb" );
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
    QUrl u( link );
    QString s = titleMap[ u.fileName() ];
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
    help->setupBookmarkMenu();
}

void HelpDialog::removeBookmark()
{
    if ( !listBookmarks->currentItem() )
	return;

    delete listBookmarks->currentItem();
    saveBookmarks();
    help->setupBookmarkMenu();
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
    help->setupBookmarkMenu();
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
    if ( !contentsDone )
	setupTitleMap();

    listContents->setSorting( -1 );
    HelpNavigationContentsItem *qtDocu, *handbook, *linguistDocu;
    qtDocu = new HelpNavigationContentsItem( listContents, 0 );
    qtDocu->setText( 0, tr( "Qt Class Documentation" ) );
    qtDocu->setLink( "index.html" );
    //    qtDocu->setPixmap( 0, PixmapChooser::loadPixmap( "book.xpm", PixmapChooser::Small ) );
    handbook = new HelpNavigationContentsItem( listContents, 0 );
    handbook->setText( 0, tr( "Designer Handbook" ) );
    handbook->setLink( "book1.html" );
//     handbook->setPixmap( 0, PixmapChooser::loadPixmap( "book.xpm", PixmapChooser::Small ) );
    linguistDocu = new HelpNavigationContentsItem( listContents, 0 );
    linguistDocu->setText( 0, tr( "Qt Linguist Manual" ) );
    linguistDocu->setLink( "qt-translation-tools.html" );

    HelpNavigationContentsItem *lastItem = 0;
    HelpNavigationContentsItem *lastGroup = 0;

    QValueList<MyString>* lst = new QValueList<MyString>;
    for ( QMap<QString, QString>::Iterator it = titleMap.begin(); it != titleMap.end(); ++it ) {
	QString s = *it + " | " + it.key();
	s = s.stripWhiteSpace();
	if ( s.lower().find( "easter egg" ) != -1 ||
	     s.lower().find( "easteregg" ) != -1 )
	    continue;
	lst->append( s );
    }

    qHeapSort( *lst );

    for ( QValueList<MyString>::Iterator sit = lst->begin(); sit != lst->end(); ++sit ) {
	QString s = *sit;
	s = s.stripWhiteSpace();
	int i = s.find( " - " );
	if ( i == -1 ) {
	    QListViewItem *oldLast = lastItem;
	    lastItem = new HelpNavigationContentsItem( qtDocu, lastItem );
	    QString txt = s;
	    QString title, link;
	    i = txt.find( " | " );
	    if ( i == -1 ) {
		title = txt;
	    } else {
		title = txt.left( i );
		link = txt.mid( i + 3, 0xFFFFFF );
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
	    QString txt = s.mid( i + 3, 0xFFFFFF );
	    QString title, link;
	    i = txt.find( " | " );
	    if ( i == -1 ) {
		title = txt;
	    } else {
		title = txt.left( i );
		link = txt.mid( i + 3, 0xFFFFFF );
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


    QString manualdir = QString( getenv( "QTDIR" ) ) + "/tools/designer/manual/book1.html";
    if ( !QFile::exists( manualdir ) )
	manualdir = QString( getenv( "QTDIR" ) ) + "/doc/html/designer/book1.html";
    QFile file( manualdir );
    if ( !file.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &file );
    QString text = ts.read();
    text = text.simplifyWhiteSpace();
    QValueList<Entry> entries;

    int i = text.find( "Table of Contents" );

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
		if ( !title.isEmpty() ) {
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

    // Linguist contents
    // I do this manually, because titleMap is only used for the
    // Qt reference docs. This will be removed when the Assistant becomes
    // more generalised.
    lastItem = new HelpNavigationContentsItem( linguistDocu, 0 );
    lastItem->setText( 0, tr( "Chapter 3: Programmers" ) );
    lastItem->setLink( "linguist-programmer.html" );
    lastItem = new HelpNavigationContentsItem( linguistDocu, 0 );
    lastItem->setText( 0, tr( "Chapter 2: Translators" ) );
    lastItem->setLink( "linguist-translator.html" );
    lastItem = new HelpNavigationContentsItem( linguistDocu, 0 );
    lastItem->setText( 0, tr( "Chapter 1: Release Manager" ) );
    lastItem->setLink( "linguist-manager.html" );
    lastItem = new HelpNavigationContentsItem( linguistDocu, 0 );
    lastItem->setText( 0, tr( "Guide to the Qt Translation tools" ) );
    lastItem->setLink( "qt-translation-tools.html" );
}

void HelpDialog::currentContentsChanged( QListViewItem * )
{
}

void HelpDialog::showContentsTopic()
{
    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)listContents->currentItem();
    emit showLink( i->link(), i->text( 0 ) );
}
