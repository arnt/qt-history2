#include "qsqlresultinfo.h"

////////////////////////////////////////////////////////////////////////////////

/*! \class QSqlResultInfo qsqlresultinfo.h
  
  \brief QSqlResultInfo provides result-specific SQL information

  \module database

  This class provides information about a QSqlResult.  The default implementation
  does not do anything useful.  Derived classes should provide their own
  database-specific information.

*/

/*! Creates an empty QSqlResultInfo object.

*/

QSqlResultInfo::QSqlResultInfo()
: sz(-1), affRows(-1)
{
    qDebug("QSqlResultInfo()");
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlResultInfo::~QSqlResultInfo()
{
    qDebug("~QSqlResultInfo()");
}

/*! Constructs a copy of \a s.

*/

QSqlResultInfo::QSqlResultInfo( const QSqlResultInfo &s )
: sz(s.sz), affRows(s.affRows), fieldList(s.fieldList)
{
}

/*! Assigns \a s to this result info and returns a reference
    to this result info.

*/

QSqlResultInfo& QSqlResultInfo::operator=( const QSqlResultInfo &s )
{
    sz = s.sz;
    affRows = s.affRows;
    fieldList = s.fieldList;
    return *this;
}

/*! Returns the size of the result, or -1 if it cannot be determined.
    Note that for non-SELECT statements, size() will return -1.  To
    determine the number of rows affected by a non-SELECT statement,
    use affectedRows().

    \sa affectedRows()

*/

int QSqlResultInfo::size() const
{
    return sz;
}


/*!  Returns a list of fields used in the result.

     \sa QSqlFieldInfo
*/

QSqlFieldInfoList QSqlResultInfo::fields() const
{
    return fieldList;
}


/*! Returns the number of rows affected by the result's SQL statement, or
    -1 if it cannot be determined.  Note that for SELECT statements, this
    value will be the same as size(),
    \sa size()

*/

int QSqlResultInfo::affectedRows() const
{
    return affRows;
}

/*! Protected method which appends field \a field to the list of
    result fields.  This method returns the position (zero-based) of the added field.

*/

int QSqlResultInfo::appendField( const QSqlFieldInfo& field )
{
    fieldList.append( field );
    return ( fieldList.count() - 1 );
}

/*! Protected method which allows derived classes to set the internal size to \a size.

    \sa size()
*/

void QSqlResultInfo::setSize( int size )
{
    sz = size;
}

/*! Protected method which allows derived classes to set the number of rows affected
    by the SQL statement.

    \sa affectedRows()

*/

void QSqlResultInfo::setAffectedRows( int rows )
{
    affRows = rows;
}
