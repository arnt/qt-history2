#include <stdio.h>
#include "qsqlrowset.h"
#include "qsqldatabase.h"
#include "qsqlresult.h"
#include "qsqldriver.h"

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
  Constructs a QSqlRowset object. The database \a dbase must properly initialized
  before a QSqlRowset object can operate properly.

*/
QSqlRowset::QSqlRowset( QSqlDatabase * dbase, const QString & table )
    : QSqlFieldList(), QSql( dbase->driver()->createResult() ), lastAt( QSqlResult::BeforeFirst ), tableName( table )
{
    *this = driver()->fields( table );
}

/*!
  Copy constuctor.

*/
QSqlRowset::QSqlRowset( const QSqlRowset & s )
    : QSqlFieldList( s ), QSql( s ), lastAt( s.lastAt ), tableName( s.tableName )
{
}

/*!
  \internal

*/
QSqlFieldList & QSqlRowset::operator=( const QSqlFieldList & list )
{
    return QSqlFieldList::operator=( list );
}

/*!
  Selects all fields in the rowset.  The order in which the data is returned
  is database-specific.

*/

bool QSqlRowset::select()
{
    return select( "*", QSqlIndex() );
}

/*!
  Selects all fields in the rowset.  The data is returned in the order specified
  by the index \a sort.

*/

bool QSqlRowset::select( const QSqlIndex& sort )
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
  QSqlRowset myRowset(db, "MyTable");
  myRowset.select("deptID=10"); // select everything in department 10
  ...
  myRowset.select("*"); // select all records in rowset
  ...
  myRowset.select("deptID>10");  // select other departments
  ...
  myRowset.select(); // select all records again
  \endcode

*/

bool QSqlRowset::select( const QString & filter, const QSqlIndex & sort )
{
    QString str= "select " + toString( tableName );
    str += " from " + tableName;
    if ( !filter.isNull() && filter != "*" )
	str += " where " + filter;
    if ( sort.count() )
	str += " order by " + sort.toString( tableName );
    str += ";";
    return query( str );
}

/*!
  Selects all fields in the rowset matching the filter index \a filter.  The
  data is returned in the order specified by the index \a sort.  Note that the
  \a filter index fields that are in the rowset will use the current value of the rowset
  data fields when generating the WHERE clause.  This method is useful, for example,
  for retrieving data based upon a table's primary index:

  \code
  QSqlRowset myRowset(db, "MyTable");
  QSqlIndex pk = db->primaryIndex("MyTable");
  myRowset["id"] = 10;
  myRowset.select( pk ); // generates "select ... from MyTable where id=10;"
  ...
  \endcode

*/
bool QSqlRowset::select( const QSqlIndex & filter, const QSqlIndex & sort )
{
    return select( fieldEqualsValue( "and", filter ), sort );
}

QString qMakeFieldValue( const QString& prefix, QSqlField& field, const QString& op = "=" )
{
    QString f = prefix + "." + field.name();
    if( (field.type() == QVariant::String) || (field.type() == QVariant::CString) )
	f += " " + op + " '" + field.value().toString() + "'";
    else
	f += " " + op + " " + field.value().toString();
    return f;
}

/*!
  \internal

*/
QString QSqlRowset::fieldEqualsValue( const QString& fieldSep, const QSqlIndex & i )
{
    QString filter;
    int k = i.count();

    if ( k ) { // use index
	for( int j = 0; j < k; ++j ){
	    if( j > 0 )
		filter += " " + fieldSep + " " ;
	    QString fn = i.field(j).name();
	    QSqlField f = field( fn );
	    filter += qMakeFieldValue( tableName, f );
	}
    } else { // use all fields
 	for ( uint j = 0; j < count(); ++j ) {
	    if ( j > 0 )
		filter += " " + fieldSep + " " ;
	    filter += qMakeFieldValue( tableName, field( j ) );
	}
    }
    return filter;
}

/*!
  Executes the SQL query \a str.  Returns TRUE of the rowset is active,
  otherwise returns FALSE.

*/
bool QSqlRowset::query( const QString & str )
{
    qDebug( "\n### SQL: " + str );
    *this << str;
    return isActive();
}

QVariant& QSqlRowset::operator[]( int i )
{
    sync();
    return QSqlFieldList::operator[]( i );
}

QVariant& QSqlRowset::operator[]( const QString& name )
{
    sync();
    return QSqlFieldList::operator[]( name );
}

void QSqlRowset::sync()
{
    if ( lastAt != at() ) {
	lastAt = at();
	for ( unsigned int i = 0; i < count(); ++i ){
	    QSqlFieldList::operator[](i) = QSql::operator[](i);
	}
    }
}

#endif // QT_NO_SQL
