/****************************************************************
**
** Implementation Cannonfield class, Qt tutorial 8
**
****************************************************************/

#include "cannon.h"

CannonField::CannonField( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    ang = 0;
}

void CannonField::setAngle( int degrees )
{
    if ( ang == degrees )
	return;
    if ( degrees < 5 )
	degrees = 5;
    if ( degrees > 70 )
	degrees = 70;
    ang = degrees;
    repaint();
}

void CannonField::paintEvent( QPaintEvent *e )
{
    QString s;
    s.sprintf( "Angle = %i", ang );
    drawText( 200, 100, s );
}
