#include "qsqlview.h"
#include "qsqldriver.h"
#include "qsqlresult.h"

#ifndef QT_NO_SQL

QString qOrderByClause( const QSqlIndex & i, const QString& prefix = QString::null )
{
    QString str;
    int k = i.count();
    if( k == 0 ) return QString::null;
    str = " order by " + i.toString( prefix );
    return str;
}

/*!
  Constructs a view on database \a db.  The \a name of the view
  must correspond to an existing table or view name in the database.

*/

QSqlView::QSqlView( const QString & name, const QString& databaseName )
    : QSqlFieldList(), QSql( QSqlConnection::database( databaseName )->driver()->createResult() ), lastAt( QSqlResult::BeforeFirst ), nm( name ), srt( name )
{
    if ( !nm.isNull() )
	setName( nm );
}

/*!
  Constructs a copy of view \a s.

*/

QSqlView::QSqlView( const QSqlView & s )
    : QSqlFieldList( s ), QSql( s ), lastAt( s.lastAt ), nm( s.nm )    
{

}

QSqlView::~QSqlView()
{

}

/*!
  Sets the view equal to \a s.

*/

QSqlView& QSqlView::operator=( const QSqlView& s )
{
    QSqlFieldList::operator=( s );
    QSql::operator=( s );
    lastAt = s.lastAt;
    nm = s.nm;
    srt = s.srt;
    ftr = s.ftr;
    return *this;
}

/*!
  Sets the name of the view to \a name, which must correspond to a valid table or
  view name in the database.

*/
void QSqlView::setName( const QString& name )
{
    nm = name;
    *this = driver()->fields( name );
}

/*!
  \internal

*/
QSqlFieldList & QSqlView::operator=( const QSqlFieldList & list )
{
    return QSqlFieldList::operator=( list );
}

/*!
  Returns the primary index associated with the view, or an empty
  index if there is no primary index.

*/

QSqlIndex QSqlView::primaryIndex() const
{
    return driver()->primaryIndex( name() );
}

/*!
  Selects all fields in the rowset.  The order in which the data is returned
  is database-specific.

*/

bool QSqlView::select()
{
    return select( "*", QSqlIndex() );
}

/*!
  Selects all fields in the rowset.  The data is returned in the order specified
  by the index \a sort.

*/

bool QSqlView::select( const QSqlIndex& sort )
{
    return select( "*", sort );
}

/*!
  Selects all fields in the rowset matching the filter criteria \a filter.  The
  data is returned in the order specified by the index \a sort.  Note that the
  \a filter string will be placed in the generated WHERE clause, but should not
  include the 'WHERE' keyword.  As a special case, using "*" as the filter string
  will retrieve all records.  For example:

  \code
  QSqlView myRowset(db, "MyTable");
  myRowset.select("deptID=10"); // select everything in department 10
  ...
  myRowset.select("*"); // select all records in rowset
  ...
  myRowset.select("deptID>10");  // select other departments
  ...
  myRowset.select(); // select all records again
  \endcode

*/

bool QSqlView::select( const QString & filter, const QSqlIndex & sort )
{
    QString str= "select " + toString( nm );
    str += " from " + nm;
    if ( !filter.isNull() && filter != "*" ) {
	ftr = filter;
	str += " where " + filter;
    } else
	ftr = QString::null;
    if ( sort.count() > 0 )
	str += " order by " + sort.toString( nm );
    str += ";";
    srt = sort;
    return setQuery( str );
}

/*!
  Selects all fields in the rowset matching the filter index \a filter.  The
  data is returned in the order specified by the index \a sort.  Note that the
  \a filter index fields that are in the rowset will use the current value of the rowset
  data fields when generating the WHERE clause.  This method is useful, for example,
  for retrieving data based upon a table's primary index:

  \code
  QSqlView myRowset(db, "MyTable");
  QSqlIndex pk = db->primaryIndex("MyTable");
  myRowset["id"] = 10;
  myRowset.select( pk ); // generates "select ... from MyTable where id=10;"
  ...
  \endcode

*/
bool QSqlView::select( const QSqlIndex & filter, const QSqlIndex & sort )
{
    return select( fieldEqualsValue( nm, "and", filter ), sort );
}

QString qMakeFieldValue( const QSqlDriver* driver, const QString& prefix, QSqlField* field, const QString& op = "=" )
{
    QString f = ( prefix.length() > 0 ? prefix + QString(".") : QString::null ) + field->name();
    if ( !field->isNull() ) {
	if( (field->type() == QVariant::String) || (field->type() == QVariant::CString) )
	    f += " " + op + " '" + field->value().toString() + "'";
	else
	    f += " " + op + " " + field->value().toString();
    } else {
	f += " " + op + " " + driver->nullText();
    }
    return f;
}

/*!
  \internal

*/
QString QSqlView::fieldEqualsValue( const QString& prefix, const QString& fieldSep, const QSqlIndex & i )
{
    QString filter;
    int k = i.count();

    if ( k ) { // use index
	for( int j = 0; j < k; ++j ){
	    if( j > 0 )
		filter += " " + fieldSep + " " ;
	    QString fn = i.field(j)->name();
	    QSqlField* f = field( fn );
	    filter += qMakeFieldValue( driver(), prefix, f );
	}
    } else { // use all fields
 	for ( uint j = 0; j < count(); ++j ) {
	    if ( j > 0 )
		filter += " " + fieldSep + " " ;
	    filter += qMakeFieldValue( driver(), prefix, field( j ) );
	}
    }
    return filter;
}

/*!  Inserts the current contents of the view record buffer into the
  database.  If \a invalidate is TRUE, the current view can no longer
  be navigated (i.e., any prior select statements will no longer be
  active or valid).  Returns the number of rows affected by the
  insert.  For error information, use lastError().

*/

int QSqlView::insert( bool invalidate )
{
    int k = count();
    if( k == 0 ) return 0;
    QString str = "insert into " + name();
    str += " (" + QSqlFieldList::toString() + ")";
    str += " values (";
    QString vals;
    for( int j = 0; j < k; ++j ){
	QVariant::Type type = field(j)->type();
	QVariant       val  = field(j)->value();
	if( j > 0 )
	    vals += ",";
	if ( !field(j)->isNull() ) {
	    if( (type == QVariant::String) || (type == QVariant::CString) )
		vals += "'" + val.toString() + "'";
	    else
		vals += val.toString();
	} else {
	    vals += driver()->nullText();
	}
    }
    str += vals + ");";
    return apply( str, invalidate );
}

/*!  Updates the database with the current contents of the record
  buffer, using the specified \a filter.  Only records which meet the
  filter criteria are updated, otherwise all records in the table are
  updated.  If \a invalidate is TRUE, the current view can no longer
  be navigated (i.e., any prior select statements will no longer be
  active or valid). Returns the number of records which were updated.
  For error information, use lastError().

*/

int QSqlView::update( const QString & filter, bool invalidate )
{
    int k = count();
    if( k == 0 ) return 0;
    QString str = "update " + name();
    str += " set " + fieldEqualsValue( "", "," );
    if ( filter.length() )
 	str+= " where " + filter;
    str += ";";
    return apply( str, invalidate );
}

/*!  Updates the database with the current contents of the record
  buffer, using the filter index \a filter.  Only records which meet
  the filter criteria specified by the index are updated.  If no index
  is specified, the primary index of the underlying rowset is used.
  If \a invalidate is TRUE, the current view can no longer be
  navigated (i.e., any prior select statements will no longer be
  active or valid). Returns the number of records which were
  updated. For error information, use lastError().  For example:

  \code
  QSqlDatabase* db;
  ...
  QSqlView empView ( db, "Employee" );
  empView["id"] = 10;  // set the primary index field
  empView["firstName"] = "Dave";
  empView.update();  // update an employee name using primary index
  \endcode

*/

int QSqlView::update( const QSqlIndex & filter, bool invalidate )
{
    return update( fieldEqualsValue( "", "and", filter ), invalidate );
}

/*!  Deletes the record from the view using the filter \a filter.
  Only records which meet the filter criteria specified by the index
  are deleted.  Returns the number of records which were updated. If
  \a invalidate is TRUE, the current view can no longer be navigated
  (i.e., any prior select statements will no longer be active or
  valid). For error information, use lastError().

*/

int QSqlView::del( const QString & filter, bool invalidate )
{
    int k = count();
    if( k == 0 ) return 0;
    QString str = "delete from " + name();
    if ( filter.length() )
 	str+= " where " + filter;
    str += ";";
    return apply( str, invalidate );
}

/*!  Deletes the record from the view using the filter index \a
  filter.  Only records which meet the filter criteria specified by
  the index are updated.  If no index is specified, the primary index
  of the underlying rowset is used.  If \a invalidate is TRUE, the
  current view can no longer be navigated (i.e., any prior select
  statements will no longer be active or valid). Returns the number of
  records which were deleted.  For error information, use lastError().
  For example:

*/

int QSqlView::del( const QSqlIndex & filter, bool invalidate )
{
    return del( fieldEqualsValue( "", "and", filter ), invalidate );
}

/*
  \internal
*/

int QSqlView::apply( const QString& q, bool invalidate )
{
    int ar = 0;
    if ( invalidate ) {
	setQuery( q );
	ar = affectedRows();
    } else {
	QSql sql( driver()->createResult() );
	sql.setQuery( q );
	ar = sql.affectedRows();
    }
    return ar;
}

/*!
  Executes the SQL query \a str.  Returns TRUE of the rowset is active,
  otherwise returns FALSE.

*/
bool QSqlView::setQuery( const QString & str )
{
    QSql::setQuery( str );
    return isActive();
}

QVariant& QSqlView::operator[]( int i )
{
    sync();
    return QSqlFieldList::operator[]( i );
}

QVariant& QSqlView::operator[]( const QString& name )
{
    sync();
    return QSqlFieldList::operator[]( name );
}

QVariant QSqlView::value( int i )
{
    sync();
    return QSqlFieldList::value( i );
}

QVariant QSqlView::value( const QString& name )
{
    sync();
    return QSqlFieldList::value( name );
}

void QSqlView::setValue( int i, const QVariant& value )
{
    QSqlFieldList::operator[]( i ) = value ;
}

void QSqlView::setValue( const QString& name, const QVariant& value )
{
    QSqlFieldList::operator[]( name ) = value ;
}

QVariant QSqlView::calculateField( uint )
{
    return QVariant();
}

void QSqlView::sync()
{
    if ( lastAt != at() ) {
	lastAt = at();
	uint i = 0;
	for ( ; i < count(); ++i ){
	    if ( field(i)->isCalculated() )
		setValue( i, calculateField( i ) );
	    else {
		setValue( i, QSql::value(i) );
	    }
	}
    }
}


#endif

