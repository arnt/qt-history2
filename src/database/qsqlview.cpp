#include "qsqlview.h"
#include "qsqldriver.h"

#ifndef QT_NO_SQL

/*!  
  Constructs a view on database \a db.  The \a name of the view
  must correspond to an existing table or view name in the database.
  
*/

QSqlView::QSqlView( QSqlDatabase * db, const QString & name )
    : QSqlRowset( db, name )
{
    
}

/*!  
  Constructs a copy of view \a s.
  
*/

QSqlView::QSqlView( const QSqlView & s )
    : QSqlRowset( s )
{
    
}

/*!
  Returns the primary index associated with the view, or an empty
  index if there is no primary index.

*/

QSqlIndex QSqlView::primaryIndex() const
{
    QSqlIndex i = driver()->primaryIndex( name() );
    return driver()->primaryIndex( name() );
}

/*!
  Inserts the current contents of the view record buffer into the database.  
  Returns the number of rows affected.  For error information, use lastError().

*/

int QSqlView::insert()
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
    query( str );
    return affectedRows();
}

/*!
  Updates the database with the current contents of the record buffer, using the
  specified \a filter.  Only records which meet the filter criteria are updated, 
  otherwise all records in the table are updated.  Returns the number of records
  which were updated.  For error information, use lastError().

*/

int QSqlView::update( const QString & filter )
{
    return 0;
}

/*!
  Updates the database with the current contents of the record buffer, using the
  filter index \a filter.  Only records which meet the filter criteria specified
  by the index are updated.  If no index is specified, the primary index of the underlying
  rowset is used.  Returns the number of records which were updated. For error information, 
  use lastError().  For example:
  
  \code
  QSqlDatabase* db;
  ...
  QSqlView empView ( db, "Employee" );
  empView["id"] = 10;  // set the primary index field
  empView["firstName"] = "Dave";
  empView.update();  // update an employee name using primary index
  \endcode

*/

int QSqlView::update( const QSqlIndex & filter )
{
    return 0;    
}

/*!
  Deletes the record from the view using the filter \a filter.  Only records which meet the 
  filter criteria specified by the index are deleted.  Returns the number of records which
  were updated. For error information, use lastError().

*/

int QSqlView::del( const QString & filter )
{
    return 0;    
}

/*!
  Deletes the record from the view using the filter index \a filter.  Only records which meet 
  the filter criteria specified by the index are updated.  If no index is specified, the primary 
  index of the underlying rowset is used.  Returns the number of records which were deleted. 
  For error information, use lastError().  For example:

*/

int QSqlView::del( const QSqlIndex & filter )
{
    return 0;    
}

#endif
