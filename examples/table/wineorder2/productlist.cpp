/*
$Id$
*/

#include "productlist.h"
#include "spinboxitem.h"
#include <qstring.h>
#include <qregexp.h>


struct {
    QString product;
    double price;
} winelist[] = {
    { "Wynns Coonawarra Shiraz 1998", 15.00 },
    { "Meißner Kapitelberg Riesling Kabinett trocken 1999", 8.94 },
    { "Perdera Monica di Sardegna 1997", 7.69 }
};

const int numwines = sizeof( winelist ) / sizeof( winelist[0] );

ProductList::ProductList()
    : QTable( numwines + 2, 4, 0, "productlist" )
{
    horizontalHeader()->setLabel( 0, "Quantity" );
    horizontalHeader()->setLabel( 1, "Product" );
    horizontalHeader()->setLabel( 2, "Price/bottle (EUR)" );
    horizontalHeader()->setLabel( 3, "Sum (EUR)" );

    for ( int i = 0; i < numwines; i++ ){
	SpinBoxItem * quantity = new SpinBoxItem( this, 0, "btls" );
	setItem( i, 0, quantity );
	setText( i, 1, winelist[i].product );
	setText( i, 2, QString::number( winelist[i].price ) );
	setText( i, 3, "0");
    }

    setText( numRows() - 2, 1, "Discount" );
    QTableItem * discount = new QTableItem( this, QTableItem::Always,
					    "-0.00" );
    setItem( numRows() - 2, 3, discount );

    processValueChanged( 0, 0 );

    setColumnReadOnly( 1, TRUE );
    setColumnReadOnly( 2, TRUE );
    setColumnReadOnly( 3, TRUE );

    connect( this, SIGNAL( valueChanged( int, int ) ),
	     this, SLOT( processValueChanged( int, int ) ) );

    adjustColumn( 1 );
    adjustColumn( 2 );

    viewport()->setFocus();
}

void ProductList::processValueChanged( int row, int col )
{
    QString total = QString::number( calcPrice( row ) );
    setText( row, 3, total );

    total = QString::number( sumUp( 0 ) );
    setText( numRows() - 1, 0, total + " btls" );

    total = QString::number( sumUp( 3 ) );
    setText( numRows() - 1, 3, total );
}

double ProductList::calcPrice( int row )
{
    QString value = text( row, 0 );
    value.replace( QRegExp( " btls" ), "" );

    double price = value.toDouble();

    return price * text( row, 2 ).toDouble();
}

double ProductList::sumUp( int col )
{
    double sum = 0;

    for ( int i = 0; i <= numwines; i++ )
	sum += text( i, col ).toDouble();

    return sum;
}
