/****************************************************************
**
** Implementation Cannonfield class, Qt tutorial 9
**
****************************************************************/

#include "cannon.h"
#include <qpainter.h>

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
