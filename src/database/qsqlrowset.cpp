#include <stdio.h>
#include "qsqlrowset.h"
#include "qsqldatabase.h"

#ifndef QT_NO_SQL

/*!
  Constructs a QSqlRowset object. The database this QSqlRowset object
  should be associated with have to be properly initialized before
  a QSqlRowset object can operate properly.
  
*/
QSqlRowset::QSqlRowset( QSqlDatabase * dbase, const QString & table )
    : tableName( table )
{
    r  = NULL;
    db = dbase;
    if( db && db->isOpen() ){
	*this = db->fields( table );
    }
}

/*!
  Copy constuctor.
  
*/
QSqlRowset::QSqlRowset( const QSqlRowset & s )
    : QSqlFieldList( s ), tableName( s.tableName )
{
}

/*!
  Hide this operation from the outsiders!
  
*/
QSqlFieldList & QSqlRowset::operator=( const QSqlFieldList & list )
{
    return QSqlFieldList::operator=( list );
}

/*!
  Executes the actual SQL query
  
*/
bool QSqlRowset::insert()
{
    return TRUE;
}

/*!
  Executes the actual SQL query
  
*/
bool QSqlRowset::update( const QSqlIndex & i )
{
    return TRUE;
}

/*!
  Executes the actual SQL query
  
*/
bool QSqlRowset::del( const QSqlIndex & i )
{
    return TRUE;
}

/*!
  Executes the actual SQL query
  
*/
bool QSqlRowset::select()
{
    return query( "select * from " + tableName );
}

/*!
  Do a select and sort any results according to the sort index.
  
*/
bool QSqlRowset::select( const QSqlIndex & i )
{
    QString str;
  
    str = fieldOrderClause( i );
    str += " from " + tableName;    
    str += orderByClause( i );

    return query( str );
}

/*!
  Do a select with a sort index and a filter index.
  
*/
bool QSqlRowset::select( const QSqlIndex & i, const QSqlIndex & j )
{
    QString str;
  
    str = fieldOrderClause( i );
    str += " from " + tableName;    
    str += whereClause( j );
    str += orderByClause( i );

    return query( str );
}

/*!
  Do a select with a sort index and a freeform filter clause.
 
*/
bool QSqlRowset::select( const QSqlIndex & i, const QString & filter )
{
    QString str;
  
    str = fieldOrderClause( i );
    str += " from " + tableName;    
    str += " " + filter;
    str += orderByClause( i );

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
  Generate the SQL 'order by' clause based on an index.
  
*/
QString QSqlRowset::orderByClause( const QSqlIndex & i )
{
    QString str;
    int k = i.fields().count();
    
    if( k == 0 ) return QString::null;
    
    str = " order by " + i.toString();
	
/*	for( int j = 0; j < k; j++ ){
	    str += i.fields().field(j).name();
	    if( (j + 1) < k ) 
		str += ", ";
	}	*/
    return str;
}

/*!
  Generates a QString that contains the fields in the current table.
  If the index contains any fields, these fields are added first in the
  order they were appended to the index.
  
*/
QString QSqlRowset::fieldOrderClause( const QSqlIndex & i )
{
    QString str;
    int k = i.fields().count();

    str = "select ";
    if( k != 0 ){
/*	for( int j = 0; j < k; j++ ){
	    
	    str += i.fields().field(j).name();
	    if( (j + 1) < k ) 
		str += ", ";
	}*/
	str += i.toString();
	// Check that all fields in the table are present
	for( int j = 0; j < count(); j++ )
	{
	    if( !str.contains( field(j).name() ) )
		str += ", " + field(j).name();
	}
	
    } else {
	str += "*";
    }
    return str;
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
    if( r->isActive() && r->isValid() ){
	qDebug("Copying values..");
	for(int k = 0; k < r->fields().count(); k++){
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

int QSqlRowset::numFields()
{
    if( r && r->isActive() )
	return count();
    else
	return 0;
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
    
    if( affectedRows() > 0 ){
	QString out;
	for(int i = 0; i < numFields(); i++){
	    out += "* " +
	      QString().sprintf("%-15s", (const char *) 
				r->fields().field(i).name()) + " ";
	}
	qDebug( QString().fill('*',numFields()*18) + "*");
	qDebug( out + "*" );
	qDebug( QString().fill('*',numFields()*18) + "*");
	while(next()){
	    out = "";
	    for(int i = 0; i < numFields(); i++){
		out += 
		    QString().sprintf("%-15s", (const char *)
				      operator[](i).asString()) + " * ";
	    }
	    qDebug("* " + out, n);
	}
	qDebug( QString().fill('*',numFields()*18) + "*");
	n++;
    } else {
	qDebug("No matching records.");
    }
}

#endif // QT_NO_SQL
