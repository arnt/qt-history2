#include "datagrid.h"
#include <qsqlindex.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include <qsqleditor.h>
#include <qsqleditorfactory.h>

DataGrid::DataGrid ( QWidget * parent = 0, const char * name = 0 )
    : QTable(parent,name), nullTxt("<null>")
{
    setMinimumWidth(300);
    setMinimumHeight(300);
}

DataGrid::~DataGrid()
{
}

void DataGrid::take( QSqlRowset* r, bool autoCreate )
{
    setUpdatesEnabled( FALSE );
    setNumRows(0);
    setNumCols(0);
    colIndex.clear();
    rset = r;
    if ( autoCreate ) {
	for ( uint j = 0; j < rset->count(); ++j )
	    addColumn( rset->field(j) );
    }
    int rows = rset->size();
    if (rows > 0)
	setNumRows( rows );
    setUpdatesEnabled( TRUE );
}

void DataGrid::addColumn( const QSqlField& field )
{
    if ( field.isVisible() && !field.isPrimaryIndex() ) {
	setNumCols( numCols() + 1 );
	colIndex.append( field.fieldNumber() );
	QHeader* h = horizontalHeader();
	h->setLabel( numCols()-1, field.name() );
    }
}

void DataGrid::removeColumn( uint col )
{
    if ( col >= (uint)numCols() )
	return;
    QHeader* h = horizontalHeader();    
    for ( uint i = col; i < (uint)numCols()-1; ++i )
	h->setLabel( i, h->label(i+1) );
    setNumCols( numCols()-1 );
    ColIndex::Iterator it = colIndex.at( col );
    if ( it != colIndex.end() )
	colIndex.remove( it );
}

void DataGrid::setColumn( uint col, const QSqlField& field )
{
    if ( col >= (uint)numCols() )
	return;
    if ( field.isVisible() && !field.isPrimaryIndex() ) {
	colIndex[ col ] = field.fieldNumber();
	QHeader* h = horizontalHeader();
	h->setLabel( col, field.name() );
    } else {
	removeColumn( col );
    }
}

void DataGrid::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    QTable::paintCell(p,row,col,cr,selected);
    if ( rset && rset->seek(row) ) {
	QString text = rset->value(colIndex[col]).toString();
	if ( rset->isNull(colIndex[col]) )
	    text = nullTxt;
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter,
		     text );
    }
}

QWidget * DataGrid::createEditor( int row, int col, bool initFromCell ) const
{
    return 0;
    Q_UNUSED( row );
    Q_UNUSED( col );
    Q_UNUSED( initFromCell );
}
