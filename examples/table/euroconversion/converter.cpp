/*
$Id$
*/

#include "converter.h"
#include <qstring.h>

static const struct {
    QString currency;
    double rate;
} rates[] = {
    { "ATS (A)",   13.7603 },
    { "BEF (B)",   40.3399 },
    { "DEM (D)",   1.95583 },
    { "ESP (E)",   166.386 },
    { "FIM (FIN)", 5.94573 },
    { "FRP (F)",   6.55957 },
    { "GRD (GR)",  340.750 },
    { "IEP (IR)",  0.78756 },
    { "ITL (IT)",  1936.27 },
    { "LUF (LUX)", 40.3399 },
    { "NLG (NL)",  2.20371 },
    { "PTE (PT)",  200.482 }
};

const int numcurrencies = sizeof( rates ) / sizeof( rates[0] );

EuroConverter::EuroConverter()
              : QTable( 1, 5, 0, "euroconverter" )
{
    setLeftMargin( 0 );
    verticalHeader()->hide();

    horizontalHeader()->setLabel( 0, "Value" );
    horizontalHeader()->setLabel( 1, "Currency (Country)" );
    horizontalHeader()->setLabel( 2, "" );
    horizontalHeader()->setLabel( 3, "Value in Euro" );
    horizontalHeader()->setLabel( 4, "" );

    QStringList currencylist;
    for ( int i=0; i < numcurrencies; i++ ) 
	currencylist << rates[i].currency;

    inputcurrency = 0;
    currencies = new QComboTableItem( this, currencylist );
    setItem( 0, 1, currencies );
    
    setText( 0, 2, "equals" );
    setText( 0, 4, "EUR" );

    setColumnReadOnly( 2, TRUE );
    setColumnReadOnly( 3, TRUE );
    setColumnReadOnly( 4, TRUE );

    adjustColumn( 1 );
    adjustColumn( 2 );
    adjustColumn( 3 );
    adjustColumn( 4 );

    connect( this, SIGNAL( valueChanged( int, int ) ), 
             this, SLOT( processValueChange( int, int ) ) );
    adjustSize();         
}

void EuroConverter::processValueChange( int, int col )
{
    if ( col == 0 ){
	bool ok;
	double value;

	value = text( 0, col ).toDouble( &ok );
	if ( ok ){
	    double euro = calculate( value );
	    setText( 0, 3, QString::number( euro ) ); 
	} else {
	    setText( 0, 0, "" ); 
	    setText( 0, 3, "" ); 
	}
    } else if ( col == 1 ){
	inputcurrency = currencies->currentItem();
	emit valueChanged( 0, 0 );
    } 
}

double EuroConverter::calculate( const double value )
{
    return ( value / rates[inputcurrency].rate );
}
