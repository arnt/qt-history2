#include "helpnavigation.h"
#include "helptopichcooser.h"
#include <qtabwidget.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qurl.h>
#include <qlistview.h>
#include <qheader.h>
#include <qapplication.h>

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

HelpNavigation::HelpNavigation( QWidget *parent, const QString &indexFile,
				const QString &titleFile, const char *name = 0 )
    : QWidget( parent, name )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 5 );
    layout->setSpacing( 5 );
    tabWidget = new QTabWidget( this );
    layout->addWidget( tabWidget );

    // --------------- contents tab ----------
    contentsTab = new QWidget( tabWidget );
    tabWidget->addTab( contentsTab, tr( "&Contents" ) );

    QVBoxLayout *contentsLayout = new QVBoxLayout( contentsTab );
    contentsLayout->setMargin( 5 );
    contentsLayout->setSpacing( 5 );

    contentsView = new QListView( contentsTab );
    contentsLayout->addWidget( contentsView );

    connect( contentsView, SIGNAL( doubleClicked( QListViewItem * ) ),
	     this, SLOT( showContents( QListViewItem * ) ) );
    connect( contentsView, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SLOT( showContents( QListViewItem * ) ) );

    // ----------- index tab -----------
    indexTab = new QWidget( tabWidget );
    tabWidget->addTab( indexTab, tr( "&Index" ) );

    QVBoxLayout *indexLayout = new QVBoxLayout( indexTab );
    indexLayout->setMargin( 5 );
    indexLayout->setSpacing( 5 );

    QLabel *l = new QLabel( tr( "Type in a &keyword:" ), indexTab );
    indexLayout->addWidget( l );

    indexEdit = new QLineEdit( indexTab );
    indexEdit->installEventFilter( this );
    indexLayout->addWidget( indexEdit );

    indexList = new QListBox( indexTab );
    indexLayout->addWidget( indexList );

    l->setBuddy( indexEdit );

    connect( indexEdit, SIGNAL( textChanged( const QString & ) ),
	     this, SLOT( searchInIndexLine( const QString & ) ) );

    connect( indexList, SIGNAL( doubleClicked( QListBoxItem * ) ),
	     this, SLOT( showTopic( QListBoxItem * ) ) );
    connect( indexList, SIGNAL( returnPressed( QListBoxItem * ) ),
	     this, SLOT( showTopic( QListBoxItem * ) ) );
    connect( indexList, SIGNAL( currentChanged( QListBoxItem * ) ),
	     this, SLOT( setIndexTopic( QListBoxItem * ) ) );

    indexEdit->setFocus();

    loadIndexFile( indexFile, titleFile );
    setupContentsView( titleFile );
}

void HelpNavigation::loadIndexFile( const QString &indexFile, const QString &titleFile )
{
    QFile f( indexFile );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    HelpNavigationListItem *lastItem = 0;
    while ( !ts.atEnd() ) {
	QString s( ts.readLine() );
	if ( s.find( "::" ) != -1 )
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
	    lastItem = new HelpNavigationListItem( indexList, s );
	lastItem->addLink( link );
    }
    f.close();
    QFile f2( titleFile );
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
	titleMap[ link ] = title;
    }
}

void HelpNavigation::searchInIndexLine( const QString &s )
{
    QListBoxItem *i = indexList->firstItem();
    while ( i ) {
	if ( i->text().lower().left( s.length() ) == s.lower() ) {
	    indexList->setCurrentItem( i );
	    break;
	}
	i = i->next();
    }
}

void HelpNavigation::showTopic( QListBoxItem *i )
{
    if ( !i )
	return;

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
	QString link = HelpTopicChooser::getLink( this, linkNames, linkList, i->text() );
	if ( !link.isEmpty() )
	    emit showLink( link, i->text() );
    }
}

void HelpNavigation::setIndexTopic( QListBoxItem *i )
{
    if ( !i || !indexList->hasFocus() )
	return;
    indexEdit->blockSignals( TRUE );
    indexEdit->setText( i->text() );
    indexEdit->blockSignals( FALSE );
}

QString HelpNavigation::titleOfLink( const QString &link )
{
    QUrl u( link );
    return titleMap[ u.fileName() ];
}

void HelpNavigation::setupContentsView( const QString &titleFile )
{
    QFile f( titleFile );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    HelpNavigationContentsItem *lastItem = 0;
    HelpNavigationContentsItem *lastGroup = 0;
    contentsView->addColumn( tr( "Topics" ) );
    contentsView->setRootIsDecorated( TRUE );
    contentsView->header()->hide();
    while ( !ts.atEnd() ) {
	QString s = ts.readLine();
	int i = s.find( " - " );
	if ( i == -1 ) {
	    QListViewItem *oldLast = lastItem;
	    lastItem = new HelpNavigationContentsItem( contentsView, lastItem );
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
	    if ( !lastGroup || lastGroup->text( 0 ) != preMinus ) {
		lastItem = new HelpNavigationContentsItem( contentsView, lastItem );
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
}

void HelpNavigation::showContents( QListViewItem *i )
{
    if ( !i || i->firstChild() )
	return;
    HelpNavigationContentsItem *item = (HelpNavigationContentsItem*)i;
    emit showLink( item->link(), item->text(0) );
}

bool HelpNavigation::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return TRUE;

    if ( o == indexEdit && e->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Up ) {
	    int i = indexList->currentItem();
	    if ( --i >= 0 ) {
		indexList->setCurrentItem( i );
		indexEdit->blockSignals( TRUE );
		indexEdit->setText( indexList->currentText() );
		indexEdit->blockSignals( FALSE );
	    }
	    return TRUE;
	} else if ( ke->key() == Key_Down ) {
	    int i = indexList->currentItem();
	    if ( ++i < indexList->count() ) {
		indexList->setCurrentItem( i );
		indexEdit->blockSignals( TRUE );
		indexEdit->setText( indexList->currentText() );
		indexEdit->blockSignals( FALSE );
	    }
	    return TRUE;
	} else if ( ke->key() == Key_Enter ||
		    ke->key() == Key_Return ) {
	    showTopic( indexList->item( indexList->currentItem() ) );
	    return TRUE;
	}
    }

    return QWidget::eventFilter( o, e );
}

void HelpNavigation::setViewMode( ViewMode m )
{
    switch ( m ) {
    case Contents:
	tabWidget->showPage( contentsTab );
	break;
    case Index:
	tabWidget->showPage( indexTab );
	break;
    }
}
