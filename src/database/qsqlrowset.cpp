#include <stdio.h>
#include "qsqlrowset.h"
#include "qsqldatabase.h"
#include "qsqlresult.h"
#include "qsqldriver.h"

#ifndef QT_NO_SQL

QString qOrderByClause( const QSqlIndex & i )
{
    QString str;
    int k = i.fields().count();
    if( k == 0 ) return QString::null;
    str = " order by " + i.toString();
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
    QString str= "select " + toString();
    str += " from " + tableName;
    if ( !filter.isNull() && filter != "*" )
	str += " where " + filter;
    if ( sort.count() )
	str += " order by " + sort.toString();
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
    QString str= "select " + toString();
    str += " from " + tableName;
    if ( filter.count() )
	str += whereClause( filter );
    if ( sort.count() )
	str += qOrderByClause( sort );
    str += ";";
    return query( str );
}

/*!
  Generate the SQL 'where' clause based on an index.

*/
QString QSqlRowset::whereClause( const QSqlIndex & i )
{
    QString filter = " where ";
    int k = i.fields().count();

    if( k == 0 ) return QString::null;

    // Build a filter based on the current field values
    for( int j = 0; j < k; j++ ){
	QVariant::Type type = i.fields().field(j).type();
	QString        fn   = i.fields().field(j).name();
	QVariant       val  = QSqlFieldList::operator[]( fn );

	if( (type == QVariant::Invalid) || (val == QString::null) )
	    continue;

	if( j > 0 )
	    filter += " and " ;

	filter += fn;
	if( (type == QVariant::String) || (type == QVariant::CString) )
	    filter += " = '" + val.asString() + "'";
	else
	    filter += " = " + val.asString();
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
    updateFieldValues();
    return QSqlFieldList::operator[]( i );
}

QVariant& QSqlRowset::operator[]( const QString& name )
{
    updateFieldValues();    
    return QSqlFieldList::operator[]( name );
}

void QSqlRowset::updateFieldValues()
{
    if ( lastAt != at() ) {
	lastAt = at();
	// ### This one have to be optimized somehow..
	for ( unsigned int i = 0; i < count(); i++ ){
	    QSqlFieldList::operator[](i) = QSql::operator[](i);
	}
    }
}
// Debugging only
void QSqlRowset::dumpRecords()
{
    static int n = 0;

    //    if( affectedRows() > 0 ){
	QString out;
	for(uint i = 0; i < count(); i++){
	    out += "* " +
	      QString().sprintf("%-15s", (const char *)
				QSql::fields().field(i).name()) + " ";
	}
	qDebug( QString().fill('*',count()*18) + "*");
	qDebug( out + "*" );
	qDebug( QString().fill('*',count()*18) + "*");
	while(next()){
	    out = "";
	    for(uint i = 0; i < count(); i++){
		out +=
		    QString().sprintf("%-15s", (const char *)
				      QSql::operator[](i).asString()) + " * ";
	    }
	    qDebug("* " + out, n);
	}
	qDebug( QString().fill('*',count()*18) + "*");
	n++;
	//    } else {
	//	qDebug("No matching records.");
	//    }
}

#endif // QT_NO_SQL


