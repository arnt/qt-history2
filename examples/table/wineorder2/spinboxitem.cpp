/*
$Id$
*/

#include "spinboxitem.h"
#include <qspinbox.h>
#include <qregexp.h>

SpinBoxItem::SpinBoxItem( QTable * myTable, const int value, 
                          const QString & text )
           : QTableItem( table, WhenCurrent, "" )
{
   table = myTable;
   suffix = text;
   setText( QString::number( value ) + suffix );
}

QWidget * SpinBoxItem::createEditor() const
{
    QSpinBox * quantities = new QSpinBox( table, "quantities" );
    quantities->setSuffix( suffix );
    quantities->setMaxValue( 250 );
    quantities->setValue( getValue() );

    return quantities;
}

int SpinBoxItem::getValue() const
{
    QString value = text();
    value.replace( QRegExp( suffix ), "" );

    bool ok;
    int number;
    number = value.toInt( &ok, 10 );

    if ( ok )
	return number;

    return 0;
}

void SpinBoxItem::setContentFromEditor( QWidget * spinbox )
{
    setText( ( (QSpinBox *) spinbox )->text() ); 
}
