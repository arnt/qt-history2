/****************************************************************
**
** Implementation of LCDRange class, Qt tutorial 7
**
****************************************************************/

#include "lcdrange.h"

#include <qscrollbar.h>
#include <qlcdnumber.h>
#include <qlayout.h>

LCDRange::LCDRange( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    QLCDNumber *lcd  = new QLCDNumber( 2, this, "lcd"  );
    sBar = new QScrollBar( 0, 99, 	// range
			   1, 10,	// line/page steps
			   0,	// inital value
			   QScrollBar::Horizontal, 	// orientation
			   this, "scrollbar" );
    QVBoxLayout *vbox = new QVBoxLayout( this, 5 );
    vbox->addWidget( lcd );
    vbox->addWidget( sBar );

    connect( sBar, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)) );
    connect( sBar, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)) );

}

int LCDRange::value() const
{
    return sBar->value();
}

void LCDRange::setValue( int value )
{
    sBar->setValue( value );
}
