#include "table.h"

#include <qheader.h>
#include <qdrawutil.h>

Table::Table( QHeader *b, int rows, QWidget *parent,
			  int flags, const char *name )
    :QTableView(parent,name)
{
    bar = b;

    connect( b, SIGNAL(sizeChange(int,int)), SLOT(rehash()) );
    connect( b, SIGNAL(moved(int,int)), SLOT(moveCol(int,int)) );

    if ( flags < 0 )
	setTableFlags( Tbl_clipCellPainting | Tbl_autoHScrollBar);
    else
	setTableFlags( flags );
    setNumRows( rows );
    setNumCols( bar->count() );
    numbers.resize( bar->count() );
    for ( int i = 0; i < bar->count(); i++ ) 
	numbers[i] = i;
    setCellWidth(0);
    setCellHeight( fontMetrics().lineSpacing() + 3 );
    setBackgroundColor( colorGroup().base() );
}


Table::~Table()
{
}

void Table::rehash()
{
    repaint();
}

int Table::cellWidth( int col )
{
    return bar->cellSize( col );
}

void Table::paintCell( QPainter *p, int row, int col ) 
{
    /*
      qDrawShadePanel( p, 0, 0,
      cellWidth(col), cellHeight(),
      colorGroup(), FALSE); // raised
      */
    p->setPen( colorGroup().text() );
    QString str;
    switch ( numbers[col] ) {
    case 0:
	str.sprintf( "Paul %d", row );
	break;
    case 1:
	str.sprintf( "Arnt %d", row );
	break;
    case 2:
	str.sprintf( "Haavard %d", row );
	break;
    case 3:
	str.sprintf( "Styreformann Eirik Eng %d", row );
	break;
    default:
	str.sprintf( "Cell %c%d", numbers[col]+'A', row );
	break;
    }
    //int yPos = fm.ascent() + fm.leading()/2 - 1;
    p->drawText( 3, 1, cellWidth(col)-6, cellHeight()-2,
		 AlignVCenter | AlignLeft , str );
}




/*!

*/

void Table::moveCol( int fromIdx, int toIdx )
{
    if ( fromIdx == toIdx )
	return;
    debug( "Table::moveCol() from %d to %d", fromIdx, toIdx );

    int s = numbers[fromIdx];
    if ( fromIdx < toIdx ) {
	for ( int i = fromIdx; i < toIdx - 1; i++ ) {
	    numbers[i] = numbers[i+1];
	}
	numbers[toIdx-1] = s;
    } else {
	for ( int i = fromIdx; i > toIdx ; i-- ) {
	    numbers[i] = numbers[i-1];
	}
	numbers[toIdx] = s;
    }

    rehash();
}
