#include "qsqlindex.h"

#ifndef QT_NO_SQL

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
    : QSqlFieldList(), table(tablename), nm(name)
{

}

/*!  Constructs a copy of \a other.

*/

QSqlIndex::QSqlIndex( const QSqlIndex& other )
    : QSqlFieldList(other), table(other.table), nm(other.nm), sorts(other.sorts)
{
}

QSqlIndex& QSqlIndex::operator=( const QSqlIndex& other )
{
    table = other.table;
    nm = other.nm;
    sorts = other.sorts;
    QSqlFieldList::operator=( other );
    return *this;
}

/*!
  Destroys the object and frees any allocated resources.

*/

QSqlIndex::~QSqlIndex()
{

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

/*!
  Appends the field \a field to the list of indexed fields.  The field
  is added in an ascending sort order.

*/

void QSqlIndex::append( const QSqlField& field )
{
    append( field, FALSE );
}

/*!
  Appends the field \a field to the list of indexed fields.  The field
  is added in an ascending sort order, unless \a desc is TRUE.

*/

void QSqlIndex::append( const QSqlField& field, bool desc )
{
    sorts.append( desc );
    QSqlFieldList::append( field );
}


/*!
  
  Returns true if field \a i in the index is sorted in descending
  order, otherwise FALSE is returned.
 
*/

bool QSqlIndex::isDescending( int i ) const
{
    if ( sorts.at( i ) != sorts.end() )
	return sorts[i];
    return FALSE;
}

/*!
  
  If \a desc is TRUE, field \a i is sorted in descending order.
  Otherwise, field \a i is sorted in ascending order (the default).
  If the field does not exist, nothing happens.

*/

void QSqlIndex::setDescending( int i, bool desc )
{
    if ( sorts.at( i ) != sorts.end() )
	sorts[i] = desc;
}


QString QSqlIndex::toString( const QString& prefix = QString::null ) const
{
    QString s;
    for ( uint i = 0; i < count(); ++i ) {
	if ( !prefix.isNull() )
	    s += prefix + ".";
	s += field( i ).name();
	if ( isDescending( i ) )
	    s += " desc";
    }
    return s;
}

#endif
