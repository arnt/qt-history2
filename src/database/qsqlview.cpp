#include "qsqlview.h"
#include "qsqldriver.h"

#ifndef QT_NO_SQL

/*!
  Constructs a view on database \a db.  The \a name of the view
  must correspond to an existing table or view name in the database.

*/

QSqlView::QSqlView( const QString & name, const QString& databaseName )
    : QSqlRowset( name, databaseName )
{

}

/*!
  Constructs a copy of view \a s.

*/

QSqlView::QSqlView( const QSqlView & s )
    : QSqlRowset( s )
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
    QSqlRowset::operator=( s );
    return *this;
}

/*!
  Returns the primary index associated with the view, or an empty
  index if there is no primary index.

*/

QSqlIndex QSqlView::primaryIndex() const
{
    return driver()->primaryIndex( name() );
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
	QVariant::Type type = field(j).type();
	QVariant       val  = field(j).value();
	if( j > 0 )
	    vals += "," ;
	if( (type == QVariant::String) || (type == QVariant::CString) )
	    vals += "'" + val.asString() + "'";
	else
	    vals += val.asString();
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
	query( q );
	ar = affectedRows();
    } else {
	QSql sql( driver()->createResult() );
	sql.setQuery( q );
	ar = sql.affectedRows();
    }
    return ar;
}
#endif

