/****************************************************************************
** Implementation of LCDRange class, Qt tutorial 12
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "cannon.h"
#include <qpainter.h>
#include <qpixmap.h>

#include <math.h>
#include <stdlib.h>

CannonField::CannonField( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    ang           = 0;
    f             = 0;
    shooting      = FALSE;
    timerCount    = 0;
    shoot_ang	  = 0;
    shoot_f	  = 0;
    newTarget();
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
    repaint( cannonRect(), FALSE );
}

void CannonField::setForce( int newton )
{
    if ( f == newton )
	return;
    if ( newton < 0 )
	newton = 0;
    if ( newton > 50 )
	newton = 50;
    f = newton;
}

void CannonField::shoot()
{
    if ( shooting )
	return;
    timerCount = 0;
    shoot_ang  = ang;
    shoot_f    = f;
    shooting   = TRUE;
    startTimer( 50 );
}

void CannonField::timerEvent( QTimerEvent * )
{
    erase( shotRect() );
    timerCount++;

    QRect shotR   = shotRect();

    if ( shotR.intersects( targetRect() ) ) {
	erase( targetRect() );
	stopShooting();
	newTarget();
	repaint( targetRect(), FALSE );
	emit hit();	
	return;
    }
    if ( shotR.x() > width() || shotR.y() > height() ) {
	stopShooting();
	emit missed();
	return;
    }	
    repaint( shotRect(), FALSE );
}

void CannonField::paintEvent( QPaintEvent *e )
{
    QRect updateR = e->rect();
    QPainter p;
    p.begin( this );

    if ( updateR.intersects( cannonRect() ) )
	paintCannon( &p );
    if ( shooting &&  updateR.intersects( shotRect() ) )
	paintShot( &p );
    if ( updateR.intersects( targetRect() ) )
	paintTarget( &p );
    p.end();
}

void CannonField::stopShooting()
{
    shooting = FALSE;
    killTimers();
}

void CannonField::paintShot( QPainter *p )
{
    p->setBrush( black );
    p->setPen( NoPen );
    p->drawRect( shotRect() );
}

void CannonField::paintTarget( QPainter *p )
{
    p->setBrush( red );
    p->setPen( black );
    p->drawRect( targetRect() );
}

const QRect barrel_rect(33, -4, 15, 8);

void CannonField::paintCannon( QPainter *p )
{
    QPixmap  pix( 50, 50 );
    QPainter tmp;

    pix.fill( backgroundColor() );

    tmp.begin( &pix );
    tmp.setBrush( blue );
    tmp.setPen( NoPen );

    tmp.translate( 0, pix.height() - 1 );
    tmp.drawPie( QRect( -35,-35, 70, 70 ), 0, 90*16 );
    tmp.rotate( -ang );
    tmp.drawRect( QRect( barrel_rect ) );
    tmp.end();

    p->drawPixmap( 0, rect().bottom() - (pix.height() - 1), pix );
}

QRect CannonField::cannonRect() const
{
    QRect r( 0, 0, 50, 50 );
    r.setBottomLeft( rect().bottomLeft() );
    return r;
}

QRect CannonField::shotRect() const
{
    const double gravity = 4;

    double time      = timerCount / 4.0;
    double forceFrac = shoot_f/0.7; 
    double radians   = shoot_ang*3.14159265/180;
    
    double velx      = forceFrac*cos( radians );
    double vely      = forceFrac*sin( radians );
    double x0        = ( barrel_rect.right()  + 5 )*cos(radians);
    double y0        = ( barrel_rect.right()  + 5 )*sin(radians);
    int    x         = (int) (x0 + velx*time);
    int    y         = (int) (y0 + vely*time - gravity*time*time);

    QRect r = QRect( 0, 0, 6, 6 );
    r.setCenter( QPoint( qRound(x), height() - 1 - qRound(y) ) );
    return r;
}

QRect CannonField::targetRect() const
{
    QRect r( 0, 0, 20, 10 );
    r.setCenter( target );
    return r;
}

void  CannonField::newTarget()
{
    target = QPoint( 200 + random() % ( width() - 200 - 10), 
                      35 + random() % (height() - 35  - 10) );
}
