#include "qsqlrowsettable.h"

#ifndef QT_NO_SQL

QSqlRowsetTable::QSqlRowsetTable ( QWidget * parent, const char * name )
    : QSqlSortedTable( parent, name )
{
}

QSqlRowsetTable::~QSqlRowsetTable()
{

}

void QSqlRowsetTable::setRowset( const QString& name, const QString& databaseName, bool autoPopulate )
{
    QSqlRowset r( name, databaseName );
    setRowset( r, autoPopulate );
}

void QSqlRowsetTable::setRowset( const QSqlRowset& rowset, bool autoPopulate )
{
    setUpdatesEnabled( FALSE );
    reset();
    rset = rowset;
    rset.select();
    if ( autoPopulate ) {
	for ( uint j = 0; j < rset.count(); ++j ) {
	    addColumn( rset.field(j) );
	}
    }
    loadNextPage();
    setUpdatesEnabled( TRUE );
}

void QSqlRowsetTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    QSqlTableBase::paintCell(p,row,col,cr,selected);
    if ( rset.seek(row) ) {
	QString text = rset.value( indexOf(col) ).toString();
	if ( rset.isNull( indexOf(col) ) )
	    text = nullText();
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter,
		     text );
    }
}

QSqlIndex QSqlRowsetTable::currentSort()
{
    return rset.sort();
}

QSqlField QSqlRowsetTable::field( int i )
{
    return rset.field( i );
}

void QSqlRowsetTable::setSort( QSqlIndex i )
{
    rset.select( i );
}

QSqlRowset QSqlRowsetTable::rowset() const
{
    return rset;
}

bool QSqlRowsetTable::rowExists( int r )
{
    return rset.seek( r );
}

#endif
