/*
$Id$
*/  

#include "productlist.h"
#include <qspinbox.h>
#include <qstring.h>


struct {
    QString product;
    double price;
    int quantity;
} winelist[] = {
    { "Wynns Coonawarra Shiraz 1998", 15.00, 0 },
    { "Meißner Kapitelberg Riesling Kabinett trocken 1999", 8.94, 0 },
    { "Perdera Monica di Sardegna 1997", 7.69, 0 }
};    

const int numwines = sizeof( winelist ) / sizeof( winelist[0] );  

ProductList::ProductList()
        :QTable( numwines + 2, 4, 0, "productlist" ) 
{
    horizontalHeader()->setLabel( 0, "Quantity" );
    horizontalHeader()->setLabel( 1, "Product" );
    horizontalHeader()->setLabel( 2, "Price/bottle (EUR)" );
    horizontalHeader()->setLabel( 3, "Sum (EUR)" );    

    for ( int i = 0; i < numwines; i++ ){
	createEditor( i, 0, FALSE );
	setText( i, 1, winelist[i].product );
	setText( i, 2, QString::number( winelist[i].price ) );
	setText( i, 3, QString::number( winelist[i].quantity ) );
    }

    setText( numRows() - 2, 1, "Discount" );
    QTableItem * discount = new QTableItem( this, QTableItem::Always, "-0.00" ); 
    setItem( numRows() - 2, 3, discount );

    processValueChanged( 0, 0 );

    setColumnReadOnly( 1, TRUE );
    setColumnReadOnly( 2, TRUE );
    setColumnReadOnly( 3, TRUE );    

    connect( this, SIGNAL( valueChanged( int, int ) ),
             this, SLOT( processValueChanged( int, int ) ) );  

    adjustColumn( 1 );
    adjustColumn( 2 );
}

QWidget * ProductList::createEditor( int row, int col, bool initFromCell ) const 
{
    QTableItem * i = item( row, col );

    if ( ( initFromCell || i && !i->isReplaceable() ) &&  
         ( col != 0 || row >= numwines ) ){
	return QTable::createEditor( row, col, initFromCell );
    } else if ( initFromCell ){
        ;
    } 	
    return createMyEditor( row, col );
}

QWidget * ProductList::createMyEditor( int row, int col ) const
{
	QSpinBox * quantities = new QSpinBox( (QTable * ) this, "quantities" );
	quantities->setSuffix( " btls" );
	quantities->setMaxValue( 250 );
	quantities->setValue( winelist[row].quantity );
        ( (QTable * ) this )->setCellWidget( row, col, quantities );

        connect( quantities, SIGNAL( valueChanged( int ) ),
                 this, SLOT( changeQuantity( int ) ) );
        return quantities;
}

void ProductList::changeQuantity( int )
{
    for ( int i = 0; i < numwines; i++ ){
	setCellContentFromEditor( i, 0 );
        emit valueChanged( i, 0 );
    }	
}

void ProductList::setCellContentFromEditor( int row, int col )
{
    QWidget * editor = cellWidget( row, col );	
    if ( editor->inherits( "QSpinBox" ) ){ 
        winelist[row].quantity = ((QSpinBox *) editor)->value();
    } else {
	QTable::setCellContentFromEditor( row, col ); 
    }	
}

 
void ProductList::processValueChanged( int row, int col )
{
    double total = calcPrice( row );
    setText( row, 3, QString::number( total ) ); 
   
    if ( col == 0 ){
	total = sumUp( col );
	setText( numRows() - 1, col, QString::number( total, 'f', 0 ) + " btls");
    } 
    total = sumUp( 3 );
    setText( numRows() - 1, 3, QString::number( total ) ); 
}

double ProductList::calcPrice( int row )
{
    return winelist[row].quantity * winelist[row].price;
}

double ProductList::sumUp( int col )
{
    double sum = 0;

    if ( col == 3 ){
	for ( int i = 0; i <= numwines; i++ )
	    sum += text( i, col ).toDouble();
    } else if ( col == 0 ){
	for ( int i = 0; i <= numwines; i++ )
	    sum += winelist[i].quantity;
    }
    
    return sum;
}
