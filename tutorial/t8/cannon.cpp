/****************************************************************************
** Implementation of LCDRange class, Qt tutorial 8
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "cannon.h"

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
    QString s;
    s.sprintf( "Angle = %i", ang );
    drawText( 200, 100, s );
}
