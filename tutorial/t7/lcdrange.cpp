/****************************************************************************
** Implementation of LCDRange class, Qt tutorial 11
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "lcdrange.h"

#include <qscrbar.h>
#include <qlcdnum.h>

LCDRange::LCDRange( QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
    lcd  = new QLCDNumber( 2, this, "lcd"  );
    lcd->move( 0, 0 );
    sBar = new QScrollBar( 0, 99, 1, 10, 0, QScrollBar::Horizontal, 
                           this, "scrollbar" );
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

void LCDRange::resizeEvent( QResizeEvent *e )
{
    lcd->resize( width(), height() - 16 - 5 );
    sBar->setGeometry( 0, lcd->height() + 5, width(), 16 );
}

