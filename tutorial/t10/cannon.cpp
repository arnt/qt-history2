/****************************************************************************
** Implementation of LCDRange class, Qt tutorial 10
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "cannon.h"
#include "qpainter.h"
#include "qpixmap.h"

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
    paintCannon();
}

void CannonField::setForce( int newton )
{
    if ( newton < 0 )
	newton = 0;
    if ( newton > 50 )
	newton = 50;
    f = newton;
}

void CannonField::paintEvent( QPaintEvent *e )
{
    paintCannon();
}

void CannonField::paintCannon()
{
    QPixmap pix( 50, 50 );
    QPainter p;
    QBrush   brush( blue );
    QPen     pen( blue );

    pix.fill( backgroundColor() );

    p.begin( &pix );
    p.setBrush( brush );
    p.setPen( pen );

    p.translate( 0, pix.height() - 1 );
    p.drawPie( QRect( -35,-35, 70, 70 ), 0, 90*16 );
    p.rotate( -ang );
    p.drawRect( QRect( 33, -4, 15, 8 ) );
    p.end();

    p.begin( this );
    p.drawPixmap( 0, rect().bottom() - (pix.height() - 1), pix );
    p.end();
}
