#include "qsqlviewtable.h"

#ifndef QT_NO_SQL

QSqlViewTable::QSqlViewTable ( QWidget * parent, const char * name )
    : QSqlSortedTable( parent, name )
{
}

QSqlViewTable::~QSqlViewTable()
{
    
}

void QSqlViewTable::setView( const QString& name, const QString& databaseName, bool autoPopulate )
{
    QSqlView v( name, databaseName );
    setView( v, autoPopulate );
}

void QSqlViewTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    QSqlTableBase::paintCell(p,row,col,cr,selected);
    if ( vw.seek(row) ) {
	QString text = vw.value( indexOf(col) ).toString();
	if ( vw.isNull( indexOf(col) ) )
	    text = nullText();
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter,
		     text );
    }
}



void QSqlViewTable::setView( const QSqlView& view, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    reset();
    vw = view;
    vw.select();    
    if ( autoPopulate ) {
	for ( uint j = 0; j < vw.count(); ++j ) {
	    addColumn( vw.field(j) );
	}
    }
    loadNextPage();
    setUpdatesEnabled( TRUE );
}

QSqlIndex QSqlViewTable::currentSort()
{
    return vw.sort();
}

QSqlField QSqlViewTable::field( int i )
{
    return vw.field( i );
}

void QSqlViewTable::setSort( QSqlIndex i )
{
    vw.select( i );
}

QSqlView QSqlViewTable::view() const
{
    return vw;
}

bool QSqlViewTable::rowExists( int r )
{
    return vw.seek( r );
}

#endif
