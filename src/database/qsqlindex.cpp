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
    : table(tablename), nm(name)
{

}

/*!  Constructs a copy of \a other.

*/

QSqlIndex::QSqlIndex( const QSqlIndex& other )
    : flist(other.flist), table(other.table), fieldList(other.fieldList), nm(other.nm)
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
  Clears the index.

*/

void QSqlIndex::clear()
{
    flist = "";
    fieldList.clear();
}

/*!
  Returns an ordered list of fields used in the index.

*/

QSqlFieldList QSqlIndex::fields() const
{
    return fieldList;
}

/*!
  Appends \a field to the list of indexed fields.

*/

void QSqlIndex::append( QSqlField field )
{
    for( unsigned int i = 0; i < fieldList.count(); i++ )
    {
	if( fieldList.field(i).name() == field.name() )
	    return;
    }

    if( !flist.isEmpty() )
	flist += ", " + field.name();
    else
	flist = field.name();

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

/*!
  Returns a comma separated list of the fields in the index.  If no prefix
  is specified, tableName() is prefixed to all field names.

*/

QString QSqlIndex::toString( const QString& prefix ) const
{
    QString pfix = ( prefix.isNull() ? tableName() : prefix ) + ".";
    QString pflist = flist;
    pflist = pfix + pflist.replace( QRegExp(", "), QString(", ") + pfix );
    return pflist;
}

uint QSqlIndex::count() const
{
    return fieldList.count();
}

/////////////

/*!
    \class QSqlRelation qsqlindex.h
    \brief Class used for relating SQL tables or views

    \module database
*/

/*!  Constructs a SQL relation as a link between \a parentIndex and \a childIndex.
  The number of fields in \a parentIndex must be less than or equal to the number of 
  fields in \a childIndex.  If no \name is given, the relation name defaults to:
  
  \code
  parentIndex.tableName() + " to " + childIndex.tableName()
  \endcode
  
*/

QSqlRelation::QSqlRelation( const QSqlIndex& parentIndex, const QSqlIndex& childIndex, const QString& name )
    : pIdx( parentIndex ), cIdx( childIndex ), nm( name )
{
    if ( nm.isNull() )
	nm = parentIndex.tableName() + " to " + childIndex.tableName();
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlRelation::~QSqlRelation()
{
    
}

/*! \fn QString name() const
  Returns the name of the relation.
*/

#endif
