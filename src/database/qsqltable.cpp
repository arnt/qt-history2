#include "qsqltable.h"

#ifndef QT_NO_SQL

QSqlGrid::QSqlGrid ( QWidget * parent = 0, const char * name = 0 )
    : QSqlTableBase( parent, name )
{
}

QSqlGrid::~QSqlGrid()
{
}

void QSqlGrid::setQuery( const QString& query, const QString& databaseName, bool autoPopulate )
{
    QSql s( databaseName );
    s.setQuery( query );
    setQuery( s, autoPopulate );
}

void QSqlGrid::setQuery( const QSql& query, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    reset();
    sql = query;
    if ( autoPopulate ) {
	QSqlFieldList fl = sql.fields();
	for ( uint j = 0; j < fl.count(); ++j ) {
	    addColumn( fl.field(j) );
	}
    }
    //    int rows = sql.size();
    //    if (rows > 0)
    //	setNumRows( rows );
    setNumRows( 10 );
    setUpdatesEnabled( TRUE );
}

void QSqlGrid::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    QSqlTableBase::paintCell(p,row,col,cr,selected);
    if ( sql.seek(row) ) {
	QString text = sql.value( indexOf(col) ).toString();
	if ( sql.isNull( indexOf(col) ) )
	    text = nullText();
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter,
		     text );
    }
}

void QSqlGrid::setNumRows( int r )
{
    if ( !sql.seek( r ) )
	setNumRows( r - 1 );
    QSqlTableBase::setNumRows( r );
    
}

QSql QSqlGrid::query() const
{
    return sql;
}

#endif
