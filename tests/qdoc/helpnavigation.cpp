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
#include <qalgorithms.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qcombobox.h>
#include <qdir.h>

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

HelpNavigation::HelpNavigation( QWidget *parent, const QString &dd )
    : QWidget( parent ), docDir( dd ), inSearch( FALSE )
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
    connect( contentsView, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SIGNAL( moveFocusToBrowser() ) );
    contentsView->setFocus();

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

    connect( indexList, SIGNAL( returnPressed( QListBoxItem * ) ),
	     this, SIGNAL( moveFocusToBrowser() ) );
    connect( indexEdit, SIGNAL( returnPressed() ),
	     this, SIGNAL( moveFocusToBrowser() ) );

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

    // -------------- bookmark tab -----------
    bookmarkTab = new QWidget( tabWidget );
    tabWidget->addTab( bookmarkTab, tr( "&Boorkmarks" ) );

    QVBoxLayout *bookmarkLayout = new QVBoxLayout( bookmarkTab );
    bookmarkLayout->setMargin( 5 );
    bookmarkLayout->setSpacing( 5 );

    l = new QLabel( tr( "&Topics:" ), bookmarkTab );
    bookmarkLayout->addWidget( l );

    bookmarkList = new QListView( bookmarkTab );
    bookmarkLayout->addWidget( bookmarkList );
    l->setBuddy( bookmarkList );

    QHBoxLayout *buttonLayout = new QHBoxLayout( bookmarkLayout );
    buttonLayout->setSpacing( 5 );
    QPushButton *pb = new QPushButton( tr( "&Add bookmark" ), bookmarkTab );
    buttonLayout->addWidget( pb );
    connect( pb, SIGNAL( clicked() ),
	     parent->parentWidget(), SLOT( addBookmark() ) );
    pb = new QPushButton( tr( "&Remove bookmark" ), bookmarkTab );
    buttonLayout->addWidget( pb );
    connect( pb, SIGNAL( clicked() ),
	     parent->parentWidget(), SLOT( removeBookmark() ) );
    bookmarkList->addColumn( "" );
    bookmarkList->header()->hide();

    connect( bookmarkList, SIGNAL( doubleClicked( QListViewItem * ) ),
	     this, SLOT( showContents( QListViewItem * ) ) );
    connect( bookmarkList, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SLOT( showContents( QListViewItem * ) ) );
    connect( bookmarkList, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SIGNAL( moveFocusToBrowser() ) );
    bookmarkList->setFocus();

    // ------------------ search tab ---------------
    searchTab = new QWidget( tabWidget );
    tabWidget->addTab( searchTab, tr( "&Search" ) );

    QVBoxLayout *searchLayout = new QVBoxLayout( searchTab );
    searchLayout->setMargin( 5 );
    searchLayout->setSpacing( 5 );

    l = new QLabel( tr( "Type in the &query:" ), searchTab );
    searchLayout->addWidget( l );

    searchCombo = new QComboBox( TRUE, searchTab );
    l->setBuddy( searchCombo );
    connect( searchCombo, SIGNAL( activated( int ) ),
	     this, SLOT( startSearch() ) );
    searchLayout->addWidget( searchCombo );

    searchButton = new QPushButton( tr( "&Start Search" ), searchTab );
    searchButton->setFixedSize( searchButton->sizeHint() );
    connect( searchButton, SIGNAL( clicked() ),
	     this, SLOT( startSearch() ) );
    searchLayout->addWidget( searchButton );

    searchList = new QListView( searchTab );
    searchLayout->addWidget( searchList );
    searchList->addColumn( "" );
    searchList->header()->hide();

    connect( searchList, SIGNAL( doubleClicked( QListViewItem * ) ),
	     this, SLOT( showContents( QListViewItem * ) ) );
    connect( searchList, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SLOT( showContents( QListViewItem * ) ) );
    connect( searchList, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SIGNAL( moveFocusToBrowser() ) );

    searchCombo->setFocus();
    searchCombo->setDuplicatesEnabled( FALSE );


    connect( tabWidget, SIGNAL( selected( const QString & ) ),
	     this, SIGNAL( tabChanged() ) );
}



class MyString : public QString
{
public:
    MyString() {}
    MyString( const QString& other )
	:QString( other ){
	    lower = other.lower();
    }
    QString lower;
};

bool operator<=( const MyString &s1, const MyString &s2 )
{ return s1.lower <= s2.lower; }
bool operator<( const MyString &s1, const MyString &s2 )
{ return s1.lower < s2.lower; }
bool operator>( const MyString &s1, const MyString &s2 )
{ return s1.lower > s2.lower; }

void HelpNavigation::loadIndexFile( QProgressBar *bar )
{
    QString indexFile = docDir + "/index";
    QString titleFile = docDir + "/titleindex";
    QFile f( indexFile );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    HelpNavigationListItem *lastItem = 0;


    //### if constructed on stack, it will crash on WindowsNT
    QValueList<MyString>* lst = new QValueList<MyString>;
    while ( !ts.atEnd() ) {
	qApp->processEvents();
	QString l = ts.readLine();
	lst->append( l );
	bar->setProgress( bar->progress() + l.length() );
    }
    qHeapSort( *lst );
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
	bar->setProgress( bar->progress() + s.length() );
	qApp->processEvents();
	int pipe = s.find( '|' );
	if ( pipe == -1 )
	    continue;
	QString title = s.left( pipe - 1 );
	QString link = s.mid( pipe + 1, 0xFFFFFF );
	link = link.simplifyWhiteSpace();
	titleMap[ link ] = title.stripWhiteSpace();
    }
    delete lst;
}

void HelpNavigation::searchInIndexLine( const QString &s )
{
    QListBoxItem *i = indexList->firstItem();
    QString sl = s.lower();
    while ( i ) {
	QString t = i->text();
	if ( t.length() >= sl.length() && 
	     i->text().left(s.length()).lower() == sl )
	{
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

    indexEdit->blockSignals( TRUE );
    indexEdit->setText( i->text() );
    indexEdit->blockSignals( FALSE );

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
    QString s = titleMap[ u.fileName() ];
    if ( s.isEmpty() )
	return link;
    return s;
}

void HelpNavigation::setupContentsView( QProgressBar * )
{
    QString titleFile = docDir + "/titleindex";
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
	s = s.stripWhiteSpace();
	if ( s.lower().find( "easter egg" ) != -1 ||
	     s.lower().find( "easteregg" ) != -1 )
	    continue;
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
	    if ( ++i < int(indexList->count()) ) {
		indexList->setCurrentItem( i );
		indexEdit->blockSignals( TRUE );
		indexEdit->setText( indexList->currentText() );
		indexEdit->blockSignals( FALSE );
	    }
	    return TRUE;
	} else if ( ke->key() == Key_Next || ke->key() == Key_Prior ) {
	    QApplication::sendEvent( indexList, e);
	    indexEdit->blockSignals( TRUE );
	    indexEdit->setText( indexList->currentText() );
	    indexEdit->blockSignals( FALSE );
	} else if ( ke->key() == Key_Enter ||
		    ke->key() == Key_Return ) {
	    showTopic( indexList->item( indexList->currentItem() ) );
	    return FALSE;
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
    case Bookmarks:
	tabWidget->showPage( bookmarkTab );
	break;
    case Search:
	tabWidget->showPage( searchTab );
	break;
    }
}

void HelpNavigation::addBookmark( const QString &title, const QString &link )
{
    HelpNavigationContentsItem *i = new HelpNavigationContentsItem( bookmarkList, 0 );
    i->setText( 0, title );
    i->setLink( QUrl( link ).fileName() );
}

void HelpNavigation::removeBookmark()
{
    QListViewItem *i = bookmarkList->currentItem();
    if ( !i || !i->isSelected() )
	return;
    delete i;
}

void HelpNavigation::saveBookmarks()
{
    QString fn( QDir::home().absPath() + "/.qdoc-bookmarks" );
    QFile f( fn );
    if ( !f.open( IO_WriteOnly ) )
	return;
    QTextStream ts( &f );
    QListViewItemIterator it( bookmarkList );
    for ( ; it.current(); ++it ) {
	HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)it.current();
	ts << i->text( 0 ) << endl;
	ts << i->link() << endl;
    }
    f.close();
}

void HelpNavigation::loadBookmarks()
{
    QString fn( QDir::home().absPath() + "/.qdoc-bookmarks" );
    QFile f( fn );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    while ( !ts.atEnd() ) {
	HelpNavigationContentsItem *i = new HelpNavigationContentsItem( bookmarkList, 0 );
	i->setText( 0, ts.readLine() );
	i->setLink( ts.readLine() );
    }
}

void HelpNavigation::startSearch()
{
    if ( inSearch ) {
	inSearch = FALSE;
	searchButton->setText( tr( "Start Search" ) );
	searchCombo->setEnabled( TRUE );
	searchCombo->setFocus();
	return;
    }

    inSearch = TRUE;
    searchButton->setText( tr( "Stop Search" ) );
    searchCombo->setEnabled( FALSE );
    searchList->clear();
    searchList->setSorting( -1 );

    QDir d( docDir );
    QStringList files = d.entryList( "*.html", QDir::Files );

    emit preparePorgress( files.count() );

    QString s = searchCombo->currentText();
    QStringList query = QStringList::split( '+', s );
    QStringList::Iterator it = files.begin();
    QListViewItem *after = 0;
    for ( ; it != files.end(); ++it ) {
	qApp->processEvents();
	emit incProcess();
	after = doSearch( docDir + "/" + *it, query, after );
	if ( !inSearch ) {
	    emit finishProgress();
	    return;
	}
    }

    emit finishProgress();
    startSearch();
}

QListViewItem *HelpNavigation::doSearch( const QString &fn, const QStringList &query, QListViewItem *after )
{
    QFile f( fn );
    if ( !f.open( IO_ReadOnly ) )
	return after;
    QTextStream ts( &f );
    QString contents = ts.read();
    f.close();

    QStringList::ConstIterator it = query.begin();
    for ( ; it != query.end(); ++it ) {
	if ( contents.find( *it, 0, FALSE ) == -1 )
	    return after;
    }

    HelpNavigationContentsItem *i = new HelpNavigationContentsItem( searchList, after );
    i->setText( 0, titleOfLink( fn ) );
    i->setLink( QUrl( fn ).fileName() );
    return i;
}

HelpNavigation::ViewMode HelpNavigation::viewMode() const
{
    if ( tabWidget->currentPage() == contentsTab )
	return Contents;
    else if ( tabWidget->currentPage() == indexTab )
	return Index;
    else if ( tabWidget->currentPage() == bookmarkTab )
	return Bookmarks;
    else
	return Search;
}
