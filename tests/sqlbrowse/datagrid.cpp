#include "datagrid.h"
#include <qsqlindex.h>

DataGrid::DataGrid ( QWidget * parent = 0, const char * name = 0 )
    : QTable(parent,name)
{
    setMinimumWidth(300);
    setMinimumHeight(300);
}
DataGrid::~DataGrid()
{
}

void DataGrid::free()
{
}

void DataGrid::take( const QSqlRowset& r )
{
    free();
    rset = new QSqlRowset( r );
    setNumCols( rset->count() );
    int rows = rset->size();
    if (rows > 0)
	setNumRows(rows);
    QHeader* h = horizontalHeader();
    for ( int j = 0; j < numCols(); ++j )
	h->setLabel( j, rset->field(j).name() );
}

void DataGrid::paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected )
{
    QTable::paintCell(p,row,col,cr,selected);
    if ( rset->seek(row) ) {
	QString text = (*rset)[col].toString();
	if ( rset->isNull(col) )
	    text = "<null>";
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter, text );
    }
}

void DataGrid::columnClicked ( int col )
{
    if ( rset ) {
	QSqlIndex newSort = QSqlIndex( rset->name() );
	newSort.append( rset->field( col ).name() );
	rset->select( newSort );
	viewport()->repaint( TRUE );
    }
}
