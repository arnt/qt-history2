#include <stdio.h>
#include "qsqlrowset.h"
#include "qsqldatabase.h"

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
  Constructs a QSqlRowset object. The database \a dbase msut properly initialized
  before a QSqlRowset object can operate properly.

*/
QSqlRowset::QSqlRowset( QSqlDatabase * dbase, const QString & table )
    : db(dbase), tableName( table ), r(0)
{
    if( db && db->isOpen() )
	*this = db->fields( table );
}

/*!
  Copy constuctor.

*/
QSqlRowset::QSqlRowset( const QSqlRowset & s )
    : QSqlFieldList( s ), db(s.db), tableName( s.tableName ), r(0)
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
	QVariant       val  = operator[]( fn );

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
  Executes the actual SQL query.

*/
bool QSqlRowset::query( const QString & str )
{
    qDebug( "\n### SQL: " + str );

    if( !r ){
	r = new QSql( db->query( str ) );
    } else {
	*r << str;
    }
    if( r->isActive() && r->next() && r->isValid() ){
	qDebug("Copying values..");
	for(uint k = 0; k < r->fields().count(); k++){
	    operator[](k) = (*r)[k];
	    qDebug("Value: " + operator[](k).asString());
	}
	return TRUE;
    }
    return FALSE;
}

bool QSqlRowset::seek( int i, bool relative )
{
    if( r && r->seek( i, relative ) ){
	updateFieldValues();
	return TRUE;
    }
    return FALSE;
}

bool QSqlRowset::next()
{
    if( r && r->next() ){
	updateFieldValues();
	return TRUE;
    }
    return FALSE;
}

bool QSqlRowset::previous()
{
    if( r && r->previous() ){
	updateFieldValues();
	return TRUE;
    }
    return FALSE;
}

bool QSqlRowset::first()
{
    if( r && r->first() ){
	updateFieldValues();
	return TRUE;
    }
    return FALSE;
}

bool QSqlRowset::last()
{
    if( r && r->last() ){
	updateFieldValues();
	return TRUE;
    }
    return FALSE;
}

void QSqlRowset::updateFieldValues()
{
    // ### This one have to be optimized somehow..
    for ( unsigned int i = 0; i < count(); i++ ){
	operator[](i) = (*r)[i];
    }
}

int QSqlRowset::affectedRows()
{
    if( r && r->isActive() )
	return r->affectedRows();
    else
	return 0;
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
				r->fields().field(i).name()) + " ";
	}
	qDebug( QString().fill('*',count()*18) + "*");
	qDebug( out + "*" );
	qDebug( QString().fill('*',count()*18) + "*");
	while(next()){
	    out = "";
	    for(uint i = 0; i < count(); i++){
		out +=
		    QString().sprintf("%-15s", (const char *)
				      operator[](i).asString()) + " * ";
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
