#include "qsqlindex.h"

/*!  
  Constructs an empty SQL index using database \a database
  and table \a tablename.

*/

QSqlIndex::QSqlIndex( const QSqlDatabase* database, const QString& tablename )
    : table(tablename), db(database)
{
 
}

/*! 
  Destroys the object and frees any allocated resources.

*/

QSqlIndex::~QSqlIndex()
{
    
}

/*! 
  Returns an ordered list of fields used in the index.

*/

QSqlFieldInfoList QSqlIndex::fields()
{
    return fieldList;
}

/*! 
  Appends \a field to the list of indexed fields.

*/

void QSqlIndex::append( QSqlFieldInfo field )
{
    fieldList.append( field );
}


