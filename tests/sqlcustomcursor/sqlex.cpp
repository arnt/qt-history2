#include <qsplitter.h>
#include <qlistview.h>
#include <qdatatable.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>
#include <qsqlrecord.h>
#include <qsqlcursor.h>
#include <qsqldriver.h>
#include "sqlex.h"

class QCustomSqlCursor: public QSqlCursor
{
public:
    QCustomSqlCursor( const QString & query = QString::null, bool autopopulate = TRUE, QSqlDatabase* db = 0 ): QSqlCursor( QString::null, autopopulate, db )
    {
	exec( query );
	if ( autopopulate )
	    *(QSqlRecord*)this = ((QSqlQuery*)this)->driver()->record( *(QSqlQuery*)this );
	setMode( QSqlCursor::ReadOnly );
    }
    QCustomSqlCursor( const QCustomSqlCursor & other ): QSqlCursor( other ) {}
    QCustomSqlCursor( const QSqlQuery & query, bool autopopulate = TRUE ): QSqlCursor( QString::null, autopopulate )
    {
	*(QSqlQuery*)this = query;
	if ( autopopulate )
	    *(QSqlRecord*)this = query.driver()->record( query );
	setMode( QSqlCursor::ReadOnly );
    }
    bool select( const QString & /*filter*/, const QSqlIndex & /*sort*/ = QSqlIndex() ) { return exec( lastQuery() ); }
    QSqlIndex primaryIndex( bool /*prime*/ = TRUE ) const { return QSqlIndex(); }
    int insert( bool /*invalidate*/ = TRUE ) { return FALSE; }
    int update( bool /*invalidate*/ = TRUE ) { return FALSE; }
    int del( bool /*invalidate*/ = TRUE ) { return FALSE; }
    void setName( const QString& /*name*/, bool /*autopopulate*/ = TRUE ) {}


//    QSqlIndex index( const QStringList& fieldNames ) const { return QSqlIndex(); }

//    bool exec( const QString & /*sql*/ ) { return TRUE; }
};

SqlEx::SqlEx( QWidget* parent = 0, const char* name = 0, WFlags f = 0 ): QWidget( parent, name, f )
{
    QHBoxLayout* hbl = new QHBoxLayout( this );
    QSplitter* hsplit = new QSplitter( Qt::Horizontal, this, "HSplitter" );
    hbl->addWidget( hsplit );
    lv = new QListView( hsplit, "TableList" );
    lv->addColumn( "Table" );
    lv->setRootIsDecorated( TRUE );
    lv->resize( lv->sizeHint() );

    QVBox* vb = new QVBox( hsplit, "VBox1" );
    QSplitter* vsplit = new QSplitter( Qt::Vertical, vb, "VSplitter" );
    dt = new QDataTable( vsplit, "DataTable" );

    QHBox* hb = new QHBox( vsplit, "HBox1" );
    te = new QTextEdit( hb, "SQLEdit" );
    QVBox* vb2 = new QVBox( hb, "VBox2" );
    QPushButton* submitBtn = new QPushButton( "Submit", vb2, "SubmitButton" );
    QPushButton* clearBtn = new QPushButton( "Clear", vb2, "ClearButton" );

    connect( clearBtn, SIGNAL(clicked()), te, SLOT(clear()) );
    connect( submitBtn, SIGNAL(clicked()), SLOT(execQuery()) );
    connect( lv, SIGNAL(doubleClicked(QListViewItem*)), SLOT(showTable(QListViewItem*)) );
    connect( lv, SIGNAL(returnPressed(QListViewItem*)), SLOT(showTable(QListViewItem*)) );

    QSqlDatabase* dcon = QSqlDatabase::addDatabase( "QPSQL7" );
    dcon->setHostName( "iceblink.troll.no" );
    dcon->setDatabaseName( "qttest" );
    dcon->setPort( 5433 );
    dcon->open( "troll", "trond" );
    setConnection( QSqlDatabase::defaultConnection );

    hsplit->setResizeMode( lv, QSplitter::KeepSize );
    vsplit->setResizeMode( hb, QSplitter::KeepSize );
}

SqlEx::~SqlEx()
{

}

bool SqlEx::setConnection( QString conName )
{
    con = conName;
    QSqlDatabase* db = QSqlDatabase::database( con );
    if ( !db )
	return FALSE;

    lv->clear();
    QStringList tables = db->tables();
    for ( QStringList::Iterator it = tables.begin(); it != tables.end(); ++it ) {
	QListViewItem* lvi = new QListViewItem( lv, *it );
	lv->insertItem( lvi );
	QSqlRecordInfo ri = db->recordInfo ( *it );
	for ( QSqlRecordInfo::Iterator it = ri.begin(); it != ri.end(); ++it ) {
	    QListViewItem* fi = new QListViewItem( lvi, (*it).name() );
	    lvi->insertItem( fi );
	}
    }

    return TRUE;
}

void SqlEx::showTable( QListViewItem * item )
{
    // get the table name
    QListViewItem* i = item->parent();
    if ( !i ) {
	i = item;
    }

    // get the database connection
    QSqlDatabase* db = QSqlDatabase::database( con );
    if ( !db ) {
	qDebug( QString( "SqlEx::showTable: Database %1 not found!" ).arg( con ) );
	return;
    }

    qDebug( QString( "SqlEx::showTable: Populating %1" ).arg( i->text( 0 ) ) );
    // populate the data table
    QSqlCursor* cursor = new QSqlCursor( i->text( 0 ), TRUE, db );
    dt->setSqlCursor( cursor, TRUE, TRUE );
    dt->refresh( QDataTable::RefreshAll );
}

void SqlEx::execQuery()
{
    // get the database connection
    QSqlDatabase* db = QSqlDatabase::database( con );
    if ( !db ) {
	qDebug( QString( "SqlEx::execQuery: Database %1 not found!" ).arg( con ) );
	return;
    }

    // use a custom cursor to populate the data table
    QCustomSqlCursor* cursor = new QCustomSqlCursor( te->text(), TRUE, db );
    dt->setSqlCursor( cursor, TRUE, TRUE );
    dt->refresh( QDataTable::RefreshAll );
}

