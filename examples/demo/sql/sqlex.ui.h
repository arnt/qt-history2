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
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qsqlerror.h>
#include "connect.h"

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

static void showError( const QSqlError& err, QWidget* parent = 0 )
{
   QString errStr ( "The database reported an error\n" );
    if ( !err.databaseText().isEmpty() )
	errStr += err.databaseText();
    if ( !err.driverText().isEmpty() )
	errStr += err.driverText();
    QMessageBox::warning( parent, "Error", errStr );
}
void SqlEx::init()
{
    hsplit->setResizeMode( lv, QSplitter::KeepSize );
    vsplit->setResizeMode( gb, QSplitter::KeepSize );
    lv->setRootIsDecorated( TRUE );    
    Frame7->hide();
}

void SqlEx::dbConnect()
{ 
    ConnectDialog* conDiag = new ConnectDialog( this, "Connection Dialog", TRUE );
    if ( conDiag->exec() != QDialog::Accepted )
	return;
    QSqlDatabase* db = QSqlDatabase::addDatabase( conDiag->comboDriver->currentText(), "SqlEx" );
    if ( !db )
	return;
    db->setHostName( conDiag->editHostname->text() );
    db->setDatabaseName( conDiag->editDatabase->text() );
    db->setPort( conDiag->portSpinBox->value() );
    if ( !db->open( conDiag->editUsername->text(), conDiag->editPassword->text() ) )
	showError( db->lastError(), this );
    Frame7->show();
    lbl->setText( "Double-Click on a table-name to view the contents" );
    lv->clear();
    
    QStringList tables = db->tables();
    for ( QStringList::Iterator it = tables.begin(); it != tables.end(); ++it ) {
	QListViewItem* lvi = new QListViewItem( lv, *it );
	QSqlRecordInfo ri = db->recordInfo ( *it );
	for ( QSqlRecordInfo::Iterator it = ri.begin(); it != ri.end(); ++it ) {
	    QString req;
	    if ( (*it).isRequired() > 0 ) {
		req = "Yes";
	    } else if ( (*it).isRequired() == 0 ) {
		req = "No";
	    } else {
		req = "?";
	    }
	    QListViewItem* fi = new QListViewItem( lvi, (*it).name(),  + QVariant::typeToName( (*it).type() ), req );
	    lvi->insertItem( fi );
	}
	lv->insertItem( lvi );	
    }
}

void SqlEx::execQuery()
{
    // use a custom cursor to populate the data table
    QCustomSqlCursor* cursor = new QCustomSqlCursor( te->text(), TRUE, QSqlDatabase::database( "SqlEx", TRUE ) );
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
	    showError( cursor->lastError(), this );
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
    QSqlCursor* cursor = new QSqlCursor( i->text( 0 ), TRUE, QSqlDatabase::database( "SqlEx", TRUE ) );
    dt->setSqlCursor( cursor, TRUE, TRUE );
    dt->setSort( cursor->primaryIndex() );
    dt->refresh( QDataTable::RefreshAll );
    lbl->setText( "Displaying table " + i->text( 0 ) );
}

