#include "qsqlindex.h"

/*!
    \class QSqlIndex qsqlindex.h
    \brief Class used for describing SQL database indexes

    \module database

     This class is used to describe SQL database indexes.  An index
     can belong to only one table in a database.  Information about
     the fields that compromise the index can be obtained by calling
     the fields() method.
     
     Normally, QSqlIndex objects are created by QSqlDatabase.

     \sa QSqlDatabase
*/


/*!
  Constructs an empty SQL index using database \a database
  and table \a tablename.

*/

QSqlIndex::QSqlIndex( const QString& tablename, const QString& name )
    : table(tablename), nm(name)
{

}

/*!  Constructs a copy of \a other.

*/

QSqlIndex::QSqlIndex( const QSqlIndex& other )
    : table(other.table), fieldList(other.fieldList), nm(other.nm)
{

}

QSqlIndex& QSqlIndex::operator=( const QSqlIndex& other )
{
    table = other.table;
    fieldList = other.fieldList;
    nm = other.nm;
    return *this;
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

QSqlFieldInfoList QSqlIndex::fields() const
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

/*!
  Sets the name of the index to \a n.

*/

void QSqlIndex::setName( const QString& name )
{
    nm = name;
}


/*!
  Returns the name of the index, or QString::null if no
  name has been set.

*/

QString QSqlIndex::name() const
{
    return nm;
}
