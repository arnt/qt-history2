/****************************************************************************
** Form implementation generated from reading ui file 'tableinfo.ui'
**
** Created: la 8. des 19:32:36 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include <qvariant.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtable.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qsqlquery.h>
#include <qlayout.h>
#include <qdragobject.h>

#include "tableinfo.h"
#include "distributionwidget.h"

static QPixmap uic_load_pixmap_TableInfo( const QString &name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );
    if ( !m )
	return QPixmap();
    QPixmap pix;
    QImageDrag::decode( m, pix );
    return pix;
}

/* 
 *  Constructs a TableInfo which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
TableInfo::TableInfo( QWidget* parent,  const char* name, WFlags fl )
    : QTabWidget( parent, name, fl )
{
    if ( !name )
	setName( "TableInfo" );
    setCaption( "" );

    columnsTab = new QWidget( this, "columnsTab" );
    QGridLayout* columnsTabLayout = new QGridLayout( columnsTab, 1, 1, 6, 6, "columnsTabLayout"); 

    QVBoxLayout* Layout2 = new QVBoxLayout( 0, 0, 6, "Layout2"); 
    QHBoxLayout* Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 

    QLabel* TextLabel4 = new QLabel( columnsTab, "TextLabel4" );
    TextLabel4->setText( "Tablespace" );
    Layout1->addWidget( TextLabel4 );

    tableTableSpace = new QLineEdit( columnsTab, "tableTableSpace" );
    tableTableSpace->setReadOnly( TRUE );
    Layout1->addWidget( tableTableSpace );
    Layout2->addLayout( Layout1 );

    columnsTable = new QTable( 0, 7, columnsTab, "fieldsTable" );
    columnsTable->horizontalHeader()->setLabel( 0, "Name" );
    columnsTable->horizontalHeader()->setLabel( 1, "Type" );
    columnsTable->horizontalHeader()->setLabel( 2, "Length" );
    columnsTable->horizontalHeader()->setLabel( 3, "Precision" );
    columnsTable->horizontalHeader()->setLabel( 4, "Scale" );
    columnsTable->horizontalHeader()->setLabel( 5, "Nullable" );
    columnsTable->horizontalHeader()->setLabel( 6, "Default" );
    Layout2->addWidget( columnsTable );

    columnsTabLayout->addLayout( Layout2, 0, 0 );
    insertTab( columnsTab, "Columns" );

    constraintsTab = new QWidget( this, "constraintsTab" );
    QGridLayout* constraintsTabLayout = new QGridLayout( constraintsTab, 1, 1, 6, 6, "constraintsTabLayout"); 

    constraintsTable = new QTable( 0, 5, constraintsTab, "constraintsTable" );
    constraintsTable->horizontalHeader()->setLabel( 0, "Name" );
    constraintsTable->horizontalHeader()->setLabel( 1, "Type" );
    constraintsTable->horizontalHeader()->setLabel( 2, "Search condition" );
    constraintsTable->horizontalHeader()->setLabel( 3, "Status" );
    constraintsTable->horizontalHeader()->setLabel( 4, "Validated" );

    constraintsTabLayout->addWidget( constraintsTable, 0, 0 );
    insertTab( constraintsTab, "Constraints" );

    indexTab = new QWidget( this, "indexTab" );
    QGridLayout* indexTabLayout = new QGridLayout( indexTab, 1, 1, 6, 6, "indexTabLayout"); 

    indexTable = new QTable( 0, 5, indexTab, "indexTable" );
    indexTable->horizontalHeader()->setLabel( 0, "Name" );
    indexTable->horizontalHeader()->setLabel( 1, "Type" );
    indexTable->horizontalHeader()->setLabel( 2, "Unique" );
    indexTable->horizontalHeader()->setLabel( 3, "Tablespace" );
    indexTable->horizontalHeader()->setLabel( 4, "Status" );

    indexTabLayout->addWidget( indexTable, 0, 0 );
    insertTab( indexTab, "Indexes" );

    statsTab = new QWidget( this, "statsTab" );

    QVBoxLayout* Layout3 = new QVBoxLayout( statsTab, 0, 6, "Layout3" ); 

    dataStatsBox = new QGroupBox( statsTab, "dataStatsBox" );
    dataStatsBox->setTitle( "Data statistics" );
    dataStatsBox->setColumnLayout( 0, Qt::Vertical );
    dataStatsBox->layout()->setSpacing( 6 );
    dataStatsBox->layout()->setMargin( 6 );
    QGridLayout* dataStatsLayout = new QGridLayout( dataStatsBox->layout() );
    dataStatsLayout->setAlignment( Qt::AlignTop );

    QLabel* TextLabel1 = new QLabel( dataStatsBox, "TextLabel1" );
    TextLabel1->setText( "Total data size" );
    dataStatsLayout->addWidget( TextLabel1, 0, 0 );

    QLabel* TextLabel2 = new QLabel( dataStatsBox, "TextLabel2" );
    TextLabel2->setText( "Total data extents" );
    dataStatsLayout->addWidget( TextLabel2, 1, 0 );

    QLabel* TextLabel3 = new QLabel( dataStatsBox, "TextLabel3" );
    TextLabel3->setText( "Total data blocks" );
    dataStatsLayout->addWidget( TextLabel3, 2, 0 );

    QLabel* TextLabel9 = new QLabel( dataStatsBox, "TextLabel9" );
    TextLabel9->setText( "Number of rows" );
    dataStatsLayout->addWidget( TextLabel9, 3, 0 );

    dataSize = new QLineEdit( dataStatsBox, "dataSize" );
    dataSize->setReadOnly( TRUE );
    dataStatsLayout->addWidget( dataSize, 0, 1 );
    numDataExtents = new QLineEdit( dataStatsBox, "numDataExtents" );
    numDataExtents->setReadOnly( TRUE );
    dataStatsLayout->addWidget( numDataExtents, 1, 1 );
    numDataBlocks = new QLineEdit( dataStatsBox, "numDataBlocks" );
    numDataBlocks->setReadOnly( TRUE );
    dataStatsLayout->addWidget( numDataBlocks, 2, 1 );
    numRows = new QLineEdit( dataStatsBox, "numRows" );
    numRows->setReadOnly( TRUE );
    dataStatsLayout->addWidget( numRows, 3, 1 );

    Layout3->addWidget( dataStatsBox );

    indexStatsBox = new QGroupBox( statsTab, "GroupBox2" );
    indexStatsBox->setTitle( "Index statistics" );
    indexStatsBox->setColumnLayout( 0, Qt::Vertical );
    indexStatsBox->layout()->setSpacing( 6 );
    indexStatsBox->layout()->setMargin( 6 );
    QGridLayout* indexStatsLayout = new QGridLayout( indexStatsBox->layout() );
    indexStatsLayout->setAlignment( Qt::AlignTop );

    QLabel* TextLabel8 = new QLabel( indexStatsBox, "TextLabel8" );
    TextLabel8->setText( "Number of indexes" );
    indexStatsLayout->addWidget( TextLabel8, 0, 0 );
    QLabel* TextLabel5 = new QLabel( indexStatsBox, "TextLabel5" );
    TextLabel5->setText( "Total index size" );
    indexStatsLayout->addWidget( TextLabel5, 1, 0 );
    QLabel* TextLabel6 = new QLabel( indexStatsBox, "TextLabel6" );
    TextLabel6->setText( "Total index extents" );
    indexStatsLayout->addWidget( TextLabel6, 2, 0 );
    QLabel* TextLabel7 = new QLabel( indexStatsBox, "TextLabel7" );
    TextLabel7->setText( "Total index blocks" );
    indexStatsLayout->addWidget( TextLabel7, 3, 0 );

    numIndexes = new QLineEdit( indexStatsBox, "numIndexes" );
    numIndexes->setReadOnly( TRUE );
    indexStatsLayout->addWidget( numIndexes, 0, 1 );
    indexSize = new QLineEdit( indexStatsBox, "indexSize" );
    indexSize->setReadOnly( TRUE );
    indexStatsLayout->addWidget( indexSize, 1, 1 );
    numIndexExtents = new QLineEdit( indexStatsBox, "numIndexExtents" );
    numIndexExtents->setReadOnly( TRUE );
    indexStatsLayout->addWidget( numIndexExtents, 2, 1 );
    numIndexBlocks = new QLineEdit( indexStatsBox, "numIndexBlocks" );
    numIndexBlocks->setReadOnly( TRUE );
    indexStatsLayout->addWidget( numIndexBlocks, 3, 1 );

    Layout3->addWidget( indexStatsBox );

    totalStatsBox = new QGroupBox( statsTab, "totalStatsBox" );
    totalStatsBox->setTitle( "Totals" );
    totalStatsBox->setColumnLayout( 0, Qt::Vertical );
    totalStatsBox->layout()->setSpacing( 6 );
    totalStatsBox->layout()->setMargin( 6 );
    QGridLayout* totalStatsLayout = new QGridLayout( totalStatsBox->layout() );
    totalStatsLayout->setAlignment( Qt::AlignTop );

    QLabel* TextLabel11 = new QLabel( totalStatsBox, "TextLabel1" );
    TextLabel11->setText( "Total data size" );
    totalStatsLayout->addWidget( TextLabel11, 0, 0 );

    QLabel* TextLabel12 = new QLabel( totalStatsBox, "TextLabel2" );
    TextLabel12->setText( "Total data extents" );
    totalStatsLayout->addWidget( TextLabel12, 1, 0 );

    QLabel* TextLabel13 = new QLabel( totalStatsBox, "TextLabel3" );
    TextLabel13->setText( "Total data blocks" );
    totalStatsLayout->addWidget( TextLabel13, 2, 0 );

    totalSize = new QLineEdit( totalStatsBox, "totalSize" );
    totalSize->setReadOnly( TRUE );
    totalStatsLayout->addWidget( totalSize, 0, 1 );
    numTotalExtents = new QLineEdit( totalStatsBox, "numTotalExtents" );
    numTotalExtents->setReadOnly( TRUE );
    totalStatsLayout->addWidget( numTotalExtents, 1, 1 );
    numTotalBlocks = new QLineEdit( totalStatsBox, "numTotalBlocks" );
    numTotalBlocks->setReadOnly( TRUE );
    totalStatsLayout->addWidget( numTotalBlocks, 2, 1 );

    Layout3->addWidget( totalStatsBox );

    insertTab( statsTab, "Statistics" );

    dataTab = new QWidget( this, "dataTab" );
    QGridLayout* dataTabLayout = new QGridLayout( dataTab, 1, 1, 6, 6, "dataTabLayout"); 
    insertTab( dataTab, "Data" );
    dataTable = new QTable( 0, 0, dataTab, "dataTable" );

    dataTabLayout->addWidget( dataTable, 0, 0 );

    storageTab = new QWidget( this, "storageTab" );
    QVBoxLayout* Layout4 = new QVBoxLayout( storageTab, 0, 6, "Layout4" );

    dataDistBox = new QGroupBox( storageTab, "dataDistBox" );
    dataDistBox->setTitle( "Data distribution" );
    dataDistBox->setColumnLayout( 0, Qt::Vertical );
    dataDistBox->layout()->setSpacing( 6 );
    dataDistBox->layout()->setMargin( 6 );
    QVBoxLayout* dataDistLayout = new QVBoxLayout( dataDistBox->layout() );
    dataDistLayout->setAlignment( Qt::AlignTop );
    
    dataDist = new DistributionWidget( dataDistBox, "dataDist" );
    dataDist->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    dataDistLayout->addWidget( dataDist );

    Layout4->addWidget( dataDistBox );

    indexDistBox = new QGroupBox( storageTab, "indexDistBox" );
    indexDistBox->setTitle( "Index distribution" );
    indexDistBox->setColumnLayout( 0, Qt::Vertical );
    indexDistBox->layout()->setSpacing( 6 );
    indexDistBox->layout()->setMargin( 6 );
    QVBoxLayout* indexDistLayout = new QVBoxLayout( indexDistBox->layout() );
    indexDistLayout->setAlignment( Qt::AlignTop );
    
    indexDist = new DistributionWidget( indexDistBox, "indexDist" );
    indexDist->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
    indexDistLayout->addWidget( indexDist );

    Layout4->addWidget( indexDistBox );

    insertTab( storageTab, "Storage analysis" );

    connect( this, SIGNAL( currentChanged( QWidget* ) ), this, SLOT( showPage( QWidget* ) ) );

}

/*  
 *  Destroys the object and frees any allocated resources
 */
TableInfo::~TableInfo()
{
    // no need to delete child widgets, Qt does it all for us
}

void TableInfo::clear()
{
    columns.clear();
    populated[ columnsTab ] = false;
    populated[ constraintsTab ] = false;
    populated[ indexTab ] = false;
    populated[ statsTab ] = false;
    populated[ dataTab ] = false;
    populated[ storageTab ] = false;
    columnsTable->setNumRows( 0 );
    constraintsTable->setNumRows( 0 );
    indexTable->setNumRows( 0 );
    dataTable->setNumRows( 0 );
    dataTable->setNumCols( 0 );
}

void TableInfo::populate( ObjectInfo info )
{
    clear();

    this->info = info;
    showPage( currentPage() );
}

void TableInfo::buildColumnList()
{
    QSqlQuery q( QString( "SELECT COLUMN_NAME FROM ALL_TAB_COLUMNS WHERE OWNER = '%1' AND TABLE_NAME = '%2' ORDER BY COLUMN_ID" ).arg( info.owner ).arg( info.name ) );
    while( q.next() )
	columns << q.value( 0 ).toString();
}

void TableInfo::showPage( QWidget* w )
{
    if( !populated[ w ] ) {
	if( w == columnsTab ) {
	    QSqlQuery q( QString( "SELECT TABLESPACE_NAME FROM DBA_TABLES WHERE OWNER = '%1' AND TABLE_NAME = '%2'" ).arg( info.owner ).arg( info.name ) );

	    q.next();
	    tableTableSpace->setText( q.value( 0 ).toString() );

	    if( columns.isEmpty() )
		buildColumnList();
	    q.exec( QString( "SELECT COLUMN_NAME, DATA_TYPE, DATA_LENGTH, DATA_PRECISION, DATA_SCALE, NULLABLE, DATA_DEFAULT FROM ALL_TAB_COLUMNS WHERE OWNER = '%1' AND TABLE_NAME = '%2' ORDER BY COLUMN_ID" ).arg( info.owner ).arg( info.name ) );
	    columnsTable->hide();
	    while( q.next() ) {
		int nRow = columnsTable->numRows();
		columnsTable->setNumRows( nRow + 1 );
		columnsTable->setItem( nRow, 0, new QTableItem( columnsTable, QTableItem::Never, columns[ nRow ] ) );
		columnsTable->setItem( nRow, 1, new QTableItem( columnsTable, QTableItem::Never, q.value( 1 ).toString() ) );
		columnsTable->setItem( nRow, 2, new QTableItem( columnsTable, QTableItem::Never, q.value( 2 ).toString() ) );
		columnsTable->setItem( nRow, 3, new QTableItem( columnsTable, QTableItem::Never, q.value( 3 ).toString() ) );
		columnsTable->setItem( nRow, 4, new QTableItem( columnsTable, QTableItem::Never, q.value( 4 ).toString() ) );
		if( q.value( 5 ).toString() == "Y" )
		    columnsTable->setItem( nRow, 5, new QTableItem( columnsTable, QTableItem::Never, "", uic_load_pixmap_TableInfo( "yes.png" ) ) );
		columnsTable->setItem( nRow, 6, new QTableItem( columnsTable, QTableItem::Never, q.value( 6 ).toString() ) );
	    }
	    columnsTable->show();
	}
	else if( w == constraintsTab ) {
	    QMap<QString,QString> constraint_types;
	    constraint_types[ "P" ] = "Primary key";
	    constraint_types[ "C" ] = "Check";

	    QSqlQuery q( QString( "SELECT CONSTRAINT_NAME, CONSTRAINT_TYPE, SEARCH_CONDITION, STATUS, VALIDATED FROM DBA_CONSTRAINTS "
				   "WHERE OWNER = '%1' AND TABLE_NAME = '%2'" ).arg( info.owner ).arg( info.name ) );
	    while( q.next() ) {
		int nRow = constraintsTable->numRows();
		constraintsTable->setNumRows( nRow + 1 );
		constraintsTable->setItem( nRow, 0, new QTableItem( constraintsTable, QTableItem::Never, q.value( 0 ).toString() ) );
		constraintsTable->setItem( nRow, 1, new QTableItem( constraintsTable, QTableItem::Never, constraint_types[ q.value( 1 ).toString() ] ) );
		constraintsTable->setItem( nRow, 2, new QTableItem( constraintsTable, QTableItem::Never, q.value( 2 ).toString() ) );
		constraintsTable->setItem( nRow, 3, new QTableItem( constraintsTable, QTableItem::Never, q.value( 3 ).toString() ) );
		if( q.value( 4 ).toString() == "VALIDATED" )
		    constraintsTable->setItem( nRow, 4, new QTableItem( constraintsTable, QTableItem::Never, "", uic_load_pixmap_TableInfo( "yes.png" ) ) );
	    }

	}
	else if( w == indexTab ) {
	    QSqlQuery q( QString( "SELECT INDEX_NAME, INDEX_TYPE, UNIQUENESS, TABLESPACE_NAME, STATUS FROM DBA_INDEXES "
				   "WHERE TABLE_OWNER = '%1' AND TABLE_NAME = '%2'" ).arg( info.owner ).arg( info.name ) );
	    while( q.next() ) {
		int nRow = indexTable->numRows();
		indexTable->setNumRows( nRow + 1 );
		indexTable->setItem( nRow, 0, new QTableItem( indexTable, QTableItem::Never, q.value( 0 ).toString() ) );
		indexTable->setItem( nRow, 1, new QTableItem( indexTable, QTableItem::Never, q.value( 1 ).toString() ) );
		if( q.value( 2 ).toString() == "UNIQUE" )
		    indexTable->setItem( nRow, 2, new QTableItem( indexTable, QTableItem::Never, "", uic_load_pixmap_TableInfo( "yes.png" ) ) );
		indexTable->setItem( nRow, 3, new QTableItem( indexTable, QTableItem::Never, q.value( 3 ).toString() ) );
		indexTable->setItem( nRow, 4, new QTableItem( indexTable, QTableItem::Never, q.value( 4 ).toString() ) );
	    }

	}
	else if( w == statsTab ) {
	    unsigned int dataExtents, dataBlocks, indexExtents, indexBlocks, dataBytes, indexBytes, indexes;

	    QSqlQuery q( QString( "SELECT COUNT( EXTENT_ID ), SUM( BLOCKS ), SUM( BYTES ) FROM DBA_EXTENTS WHERE OWNER = '%1' AND SEGMENT_NAME = '%2' AND SEGMENT_TYPE = 'TABLE'" ).arg( info.owner ).arg( info.name ) );
	    q.next();
	    dataExtents = q.value( 0 ).toUInt();
	    dataBlocks = q.value( 1 ).toUInt();
	    dataBytes = q.value( 2 ).toUInt();

	    q.exec( QString( "SELECT COUNT( DISTINCT I.INDEX_NAME ), COUNT( E.EXTENT_ID ), SUM( E.BLOCKS ), SUM( E.BYTES ) "
			       "FROM DBA_INDEXES I, DBA_EXTENTS E "
			      "WHERE I.TABLE_OWNER = '%1' AND I.TABLE_NAME = '%2' AND E.OWNER = I.TABLE_OWNER AND E.SEGMENT_NAME = I.INDEX_NAME AND E.SEGMENT_TYPE = 'INDEX'" ).arg( info.owner ).arg( info.name ) );
	    q.next();
	    indexes = q.value( 0 ).toUInt();
	    indexExtents = q.value( 1 ).toUInt();
	    indexBlocks = q.value( 2 ).toUInt();
	    indexBytes = q.value( 3 ).toUInt();

	    q.exec( QString( "SELECT NUM_ROWS FROM DBA_TABLES WHERE OWNER = '%1' AND TABLE_NAME = '%2'" ).arg( info.owner ).arg( info.name ) );
	    q.next();
	    numRows->setText( q.value( 0 ).toString() );

	    numDataExtents->setText( QString( "%1" ).arg( dataExtents ) );
	    numDataBlocks->setText( QString( "%1" ).arg( dataBlocks ) );
	    dataSize->setText( QString( "%1 bytes" ).arg( dataBytes ) );
	    numIndexes->setText( QString( "%1" ).arg( indexes ) );
	    numIndexExtents->setText( QString( "%1" ).arg( indexExtents ) );
	    numIndexBlocks->setText( QString( "%2" ).arg( indexBlocks ) );
	    indexSize->setText( QString( "%1 bytes" ).arg( indexBytes ) );

	    numTotalExtents->setText( QString( "%1" ).arg( dataExtents + indexExtents ) );
	    numTotalBlocks->setText( QString( "%1" ).arg( dataBlocks + indexBlocks ) );
	    totalSize->setText( QString( "%1 bytes" ).arg( dataBytes + indexBytes ) );
	}
	else if( w == dataTab ) {
	    if( columns.isEmpty() )
		buildColumnList();

	    QSqlQuery q( QString( "SELECT " + columns.join( ", " ) + " FROM " + info.owner + "." + info.name ) );
	    int numCols = columns.count();
	    dataTable->hide();
	    dataTable->setNumCols( numCols );
	    for( int col = 0; col < numCols; col++ )
	        dataTable->horizontalHeader()->setLabel( col, columns[ col ] );
	    
	    while( q.next() ) {
		int nRow = dataTable->numRows();
		dataTable->setNumRows( nRow + 1 );
		for( int i = 0; i < numCols; i++ )
		    dataTable->setItem( nRow, i, new QTableItem( dataTable, QTableItem::Never, q.value( i ).toString() ) );
	    }
	    dataTable->show();
	}
	else if( w == storageTab ) {
	    QSqlQuery q( QString( "SELECT TABLESPACE_NAME FROM DBA_TABLES WHERE OWNER = '%1' AND TABLE_NAME = '%2'" ).arg( info.owner ).arg( info.name ) );
	    q.next();
	    QString tableSpace = q.value( 0 ).toString();
	    if( tableSpace.length() ) {
		q.exec( QString( "SELECT SUM( BLOCKS ) FROM DBA_DATA_FILES WHERE TABLESPACE_NAME = '%1'" ).arg( tableTableSpace->text() ) );
		q.next();
		QString tmp = q.lastQuery();
		dataDist->setTotalBlocks( q.value( 0 ).toInt() );

		q.exec( QString( "SELECT OWNER, SEGMENT_NAME, SEGMENT_TYPE, BLOCK_ID, BLOCKS FROM DBA_EXTENTS WHERE TABLESPACE_NAME = '%1' ORDER BY BLOCK_ID" ).arg( tableTableSpace->text() ) );
		while( q.next() ) {
		    int status = DistributionWidget::BlockStatus_Other;
		    QString owner = q.value( 0 ).toString();
		    QString name = q.value( 1 ).toString();
		    QString type = q.value( 2 ).toString();
		    if( ( q.value( 0 ).toString() == info.owner ) && ( q.value( 1 ).toString() == info.name ) && ( q.value( 2 ).toString() == "TABLE" ) )
			status = DistributionWidget::BlockStatus_Used;
		    int startBlock = q.value( 3 ).toInt();
		    for( int i = 0; i < q.value( 4 ).toInt(); i++ )
			dataDist->setBlockStatus( startBlock + i, status );
		}
	    }
	}
	populated[ w ] = true;
    }
}
