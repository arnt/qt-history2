/****************************************************************************
** Implementation of LCDRange class, Qt tutorial 9
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "cannon.h"
#include "qpainter.h"

CannonField::CannonField( QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
    ang = 0;
}

void CannonField::setAngle( int degrees )
{
    if ( degrees < 0 )
	degrees = 0;
    if ( degrees > 90 )
	degrees = 90;
    ang = degrees;
    repaint();
}

void CannonField::paintEvent( QPaintEvent *e )
{
    QPainter p;
    QBrush   brush( blue );
    QPen     pen( blue );

    p.begin( this );
    p.setBrush( brush );
    p.setPen( pen );

    p.translate( 0, rect().bottom() );
    p.drawPie( QRect( -35,-35, 70, 70 ), 0, 90*16 );
    p.rotate( -ang );
    p.drawRect( QRect( 33, -4, 15, 8 ) );

    p.end();
}
