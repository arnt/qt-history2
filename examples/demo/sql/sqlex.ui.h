/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <qsqldriver.h>
#include <qmessagebox.h>
#include <qsqldatabase.h>

class QCustomSqlCursor: public QSqlCursor
{
public:
    QCustomSqlCursor( const QString & query = QString::null, bool autopopulate = TRUE, QSqlDatabase* db = 0 ) :
		QSqlCursor( QString::null, autopopulate, db )
    {
	exec( query );
	if ( autopopulate ) {
	    QSqlRecordInfo inf = ((QSqlQuery*)this)->driver()->recordInfo( *(QSqlQuery*)this );
	    for ( QSqlRecordInfo::iterator it = inf.begin(); it != inf.end(); ++it ) {
		append( *it );
	    }	    
	}
	setMode( QSqlCursor::ReadOnly );
    }
    QCustomSqlCursor( const QCustomSqlCursor & other ): QSqlCursor( other ) {}
    bool select( const QString & /*filter*/, const QSqlIndex & /*sort*/ = QSqlIndex() )
	{ return exec( lastQuery() ); }
    QSqlIndex primaryIndex( bool /*prime*/ = TRUE ) const
	{ return QSqlIndex(); }
    int insert( bool /*invalidate*/ = TRUE )
	{ return FALSE; }
    int update( bool /*invalidate*/ = TRUE )
	{ return FALSE; }
    int del( bool /*invalidate*/ = TRUE )
	{ return FALSE; }
    void setName( const QString& /*name*/, bool /*autopopulate*/ = TRUE ) {}
};

void SqlEx::init()
{
    hsplit->setResizeMode( lv, QSplitter::KeepSize );
    vsplit->setResizeMode( gb, QSplitter::KeepSize );
    lv->setRootIsDecorated( TRUE );    
    Frame7->hide();
}

void SqlEx::dbConnect()
{ 
    // ### TODO: Connection-Dialog    
    QSqlDatabase* db = QSqlDatabase::addDatabase( "QPSQL7" );
    if ( !db )
	return;
    db->setHostName( "iceblink.troll.no" );
    db->setDatabaseName( "qttest" );
    db->setPort( 5433 );
    db->open( "troll", "trond" );
    
    Frame7->show();
    lbl->setText( "Double-Click on a table-name to view the contents" );
    lv->clear();
    
    QStringList tables = db->tables();
    for ( QStringList::Iterator it = tables.begin(); it != tables.end(); ++it ) {
	QListViewItem* lvi = new QListViewItem( lv, *it );
	QSqlRecordInfo ri = db->recordInfo ( *it );
	for ( QSqlRecordInfo::Iterator it = ri.begin(); it != ri.end(); ++it ) {
	    QString req = "?";
	    QString defVal = (*it).defaultValue().toString();
	    QString len = (*it).length() >= 0 ? QString::number( (*it).length() ) : "?";
	    QString prec = (*it).precision() >= 0 ? QString::number( (*it).precision() ) : "?";
	    if ( (*it).isRequired() > 0 ) {
		req = "Yes";
	    } else if ( (*it).isRequired() == 0 ) {
		req = "No";
	    }
	    QListViewItem* fi = new QListViewItem( lvi, (*it).name(),  + QVariant::typeToName( (*it).type() ), req, defVal, len, prec );
	    lvi->insertItem( fi );
	}
	lv->insertItem( lvi );	
    }
}

void SqlEx::execQuery()
{
    // use a custom cursor to populate the data table
    QCustomSqlCursor* cursor = new QCustomSqlCursor( te->text(), TRUE );
    if ( cursor->isSelect() ) {
	dt->setSqlCursor( cursor, TRUE, TRUE );
	dt->refresh( QDataTable::RefreshAll );
	QString txt( "Query OK" );
	if ( cursor->size() >= 0 )
	    txt += ", returned rows: " + QString::number( cursor->size() );
	lbl->setText( txt );
    } else {
	// an error occured if the cursor is not active
	if ( !cursor->isActive() ) {
	    QSqlError err = cursor->lastError();
	    QString errStr ( "Error while executing Query\n" );
	    if ( !err.databaseText().isEmpty() )
		errStr += err.databaseText();
	    if ( !err.driverText().isEmpty() )
		errStr += err.driverText();
	    QMessageBox::warning( this, "Error", errStr );
	} else {
	    lbl->setText( QString("Query OK, affected rows: %1").arg( cursor->numRowsAffected() ) );
	}
    }
}

void SqlEx::showTable( QListViewItem * item )
{
    // get the table name
    QListViewItem* i = item->parent();
    if ( !i ) {
	i = item;
    }

    // populate the data table
    QSqlCursor* cursor = new QSqlCursor( i->text( 0 ), TRUE );
    dt->setSqlCursor( cursor, TRUE, TRUE );
    dt->setSort( cursor->primaryIndex() );
    dt->refresh( QDataTable::RefreshAll );
    lbl->setText( "Displaying table " + i->text( 0 ) );
}
