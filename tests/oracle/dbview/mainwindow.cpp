/****************************************************************************
** Form implementation generated from reading ui file 'mainwindow.ui'
**
** Created: fr 7. des 20:01:39 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "mainwindow.h"

#include <qvariant.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qsqldatabase.h>
#include <qsplitter.h>
#include <qwidgetstack.h>
#include <qiconview.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qprogressdialog.h>

#include "dbconnection.h"
#include "tableinfo.h"
#include "movetabledlg.h"

extern Q_EXPORT QApplication* qApp;

static QPixmap uic_load_pixmap_MainWindow( const QString &name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );
    if ( !m )
	return QPixmap();
    QPixmap pix;
    QImageDrag::decode( m, pix );
    return pix;
}
/* 
 *  Constructs a MainWindow which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
MainWindow::MainWindow( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "MainWindow" );

    dbConnection = new DBConnection();

    resize( 810, 827 ); 
    setCaption( trUtf8( "EHB Oracle tool" ) );

    QHBoxLayout* Layout1 = new QHBoxLayout( this, 6, 2, "Layout1"); 

    QSplitter* splitter = new QSplitter( this, "splitter" );

    DBView = new QListView( splitter, "DBView" );
    DBView->addColumn( "" );
    DBView->setRootIsDecorated( TRUE );
    DBView->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    detailsStack = new QWidgetStack( splitter, "detailsStack" );
    tableInfo = new TableInfo( detailsStack, "tableInfo" );
    tableInfo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    viewInfo = new QLabel( detailsStack, "viewInfo" );
    viewInfo->setText( "View details not implemented yet" );
    viewInfo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    
    indexInfo = new QLabel( detailsStack, "indexInfo" );
    indexInfo->setText( "Index details not implemented yet" );
    indexInfo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    
    userInfo = new QLabel( detailsStack, "userInfo" );
    userInfo->setText( "User details not implemented yet" );
    userInfo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    
    tableSpaceInfo = new QLabel( detailsStack, "tableSpaceInfo" );
    tableSpaceInfo->setText( "Tablespace details not implemented yet" );
    tableSpaceInfo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    iconView = new QIconView( detailsStack, "iconView" );
    iconView->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    detailsStack->raiseWidget( iconView );

    Layout1->addWidget( splitter );

    populate();

    connect( DBView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( displayObject( QListViewItem* ) ) );
    connect( DBView, SIGNAL( rightButtonClicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( dbContext( QListViewItem*, const QPoint&, int) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
MainWindow::~MainWindow()
{
    delete dbConnection;
}

void MainWindow::populate()
{
    dbItem = new QListViewItem( DBView, 0 );
    dbItem->setText( 0, dbConnection->database()->databaseName() );
    dbItem->setPixmap( 0, uic_load_pixmap_MainWindow( "database.png" ) );
    dbItem->setOpen( TRUE );
    tablesFolder = new QListViewItem( dbItem, 0 );
    tablesFolder->setText( 0, "Tables" );
    tablesFolder->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );
    viewsFolder = new QListViewItem( dbItem, 0 );
    viewsFolder->setText( 0, "Views" );
    viewsFolder->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );
    indexesFolder = new QListViewItem( dbItem, 0 );
    indexesFolder->setText( 0, "Indexes" );
    indexesFolder->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );
    tableSpacesFolder = new QListViewItem( dbItem, 0 );
    tableSpacesFolder->setText( 0, "Tablespaces" );
    tableSpacesFolder->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );
    usersFolder = new QListViewItem( dbItem, 0 );
    usersFolder->setText( 0, "Users" );
    usersFolder->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );

    QStringList& users = dbConnection->users();
    for( QStringList::Iterator it = users.begin(); it != users.end(); ++it ) {
	QListViewItem* usersItem = new QListViewItem( usersFolder, 0 );
	usersItem->setText( 0, (*it) );
	usersItem->setPixmap( 0, uic_load_pixmap_MainWindow( "user.png" ) );
        objectMap[ usersItem ] = ObjectInfo( (*it), ObjectInfo::OBJTYPE_USER );
	
	QListViewItem* tablesUserItem = new QListViewItem( tablesFolder, 0 );
	tablesUserItem->setText( 0, (*it ) );
	tablesUserItem->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );

	QListViewItem* viewsUserItem = new QListViewItem( viewsFolder, 0 );
	viewsUserItem->setText( 0, (*it ) );
	viewsUserItem->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );

	QListViewItem* indexesUserItem = new QListViewItem( indexesFolder, 0 );
	indexesUserItem->setText( 0, (*it ) );
	indexesUserItem->setPixmap( 0, uic_load_pixmap_MainWindow( "folder.png" ) );

	QStringList& tables = dbConnection->tables( (*it) );
	for( QStringList::Iterator it2 = tables.begin(); it2 != tables.end(); ++it2 ) {
	    QListViewItem* item = new QListViewItem( tablesUserItem, 0 );
	    item->setText( 0, (*it2) );
	    item->setPixmap( 0, uic_load_pixmap_MainWindow( "table.png" ) );
	    objectMap[ item ] = ObjectInfo( (*it2), ObjectInfo::OBJTYPE_TABLE, (*it) );
	}

	QStringList& views = dbConnection->views( (*it) );
	for( QStringList::Iterator it3 = views.begin(); it3 != views.end(); ++it3 ) {
	    QListViewItem* item = new QListViewItem( viewsUserItem, 0 );
	    item->setText( 0, (*it3) );
	    item->setPixmap( 0, uic_load_pixmap_MainWindow( "table.png" ) );
	    objectMap[ item ] = ObjectInfo( (*it3), ObjectInfo::OBJTYPE_VIEW, (*it) );
	}

	QStringList& indexes = dbConnection->indexes( (*it) );
	for( QStringList::Iterator it4 = indexes.begin(); it4 != indexes.end(); ++it4 ) {
	    QListViewItem* item = new QListViewItem( indexesUserItem, 0 );
	    item->setText( 0, (*it4) );
	    item->setPixmap( 0, uic_load_pixmap_MainWindow( "table.png" ) );
	    objectMap[ item ] = ObjectInfo( (*it4), ObjectInfo::OBJTYPE_INDEX, (*it) );
	}
    }
    QStringList& tableSpaces = dbConnection->tableSpaces();
    for( QStringList::Iterator it5 = tableSpaces.begin(); it5 != tableSpaces.end(); ++it5 ) {
	QListViewItem* item = new QListViewItem( tableSpacesFolder, 0 );
	item->setText( 0, (*it5) );
	item->setPixmap( 0, uic_load_pixmap_MainWindow( "tablespace.png" ) );
	objectMap[ item ] = ObjectInfo( (*it5), ObjectInfo::OBJTYPE_TABLESPACE );
    }
}

void MainWindow::displayObject( QListViewItem* item )
{
    ObjectInfo info = objectMap[ item ];
    switch( info.objectType ) {
    case ObjectInfo::OBJTYPE_TABLE:
	tableInfo->populate( info );
	detailsStack->raiseWidget( tableInfo );
	break;
    case ObjectInfo::OBJTYPE_VIEW:
	detailsStack->raiseWidget( viewInfo );
	break;
    case ObjectInfo::OBJTYPE_INDEX:
	detailsStack->raiseWidget( indexInfo );
	break;
    case ObjectInfo::OBJTYPE_USER:
	detailsStack->raiseWidget( userInfo );
	break;
    case ObjectInfo::OBJTYPE_TABLESPACE:
	detailsStack->raiseWidget( tableSpaceInfo );
	break;
    default:
	detailsStack->raiseWidget( iconView );
	break;
    }
}

void MainWindow::dbContext( QListViewItem* item, const QPoint& pt, int col )
{

    popupInfo = objectMap[ item ];
    popupItem = item;

    if( item ) {
	QPopupMenu* popup = new QPopupMenu( this, "popup" );

	if( popupInfo.owner ) {
	    popup->insertItem( "Defragment database", this, SLOT( defragDatabase() ) );
	    if( popupInfo.objectType == ObjectInfo::OBJTYPE_TABLE ) {
		popup->insertItem( "Move table", this, SLOT( moveTable() ) );
	    }
	}

	if( popup->count() )
	    popup->popup( pt );
    }
}

void MainWindow::moveTable()
{
    MoveTableDlg dlg;
    
    dlg.loadTablespaces( dbConnection->tableSpaces() );

    if( dlg.exec() ) {
	dbConnection->moveTable( popupInfo, dlg.tableSpaceName() );
	displayObject( popupItem );
    }
}

void MainWindow::defragDatabase()
{
    QProgressDialog* progressDlg = new QProgressDialog( this, "progressDlg" );

    QSqlQuery q( QString( "SELECT COUNT(*) FROM ALL_TABLES WHERE OWNER = '%1'" ).arg( popupInfo.owner ) );

    QSqlQuery qTables( QString( "SELECT TABLE_NAME, TABLESPACE_NAME FROM ALL_TABLES WHERE OWNER = '%1'" ).arg( popupInfo.owner ) );

    q.next();
    progressDlg->setTotalSteps( q.value( 0 ).toInt() );

    int progress( 0 );
    while( qTables.next() && !progressDlg->wasCancelled() ) {
	progressDlg->setProgress( ++progress );
	progressDlg->setLabelText( QString( "%1.%2" ).arg( popupInfo.owner ).arg( qTables.value( 0 ).toString() ) );
	qApp->processEvents();
	dbConnection->moveTable( ObjectInfo( qTables.value( 0 ).toString(), ObjectInfo::OBJTYPE_TABLE, popupInfo.owner ), qTables.value( 1 ).toString() );
    }
}
