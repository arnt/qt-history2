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

CannonField::CannonField( QWidget *parent=0, const char *name=0 )
        : QWidget( parent, name )
{
    ang      = 0;
    f        = 0;
    shooting = FALSE;
    newTarget();
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
    QRect shotR   = shotRect();
    QRect targetR = targetRect();

    if ( shotR.intersects( targetR ) ) {
	erase( targetR );
	stopShooting();
	newTarget();
	paintTarget();
	emit hit();	
	return;
    }
    if ( shotR.x() > width() || shotR.y() > height() ) {
	stopShooting();
	emit missed();
	return;
    }	
    erase( shotR );
    timerCount++;
    paintShot();
}

void CannonField::paintEvent( QPaintEvent *e )
{
    paintCannon();
    if ( shooting )
	paintShot();
    paintTarget();
}

void CannonField::stopShooting()
{
    erase( shotRect() );
    shooting = FALSE;
    killTimers();
}


void CannonField::paintShot()
{
    QPainter p;
    p.begin( this );
    p.setBrush( black );
    p.drawRect( shotRect() );
    p.end();
    
}

void CannonField::paintTarget()
{
    QPainter p;
    p.begin( this );
    p.setBrush( red );
    p.drawRect( targetRect() );
    p.end();
    
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

QRect CannonField::shotRect() const
{
    const double gravity = 4;

    double time      = timerCount / 3.0;
    double forceFrac = shoot_f/0.7; 
    double radians   = shoot_ang*3.14159265/180;
    
    double vely      = forceFrac*sin( radians );
    double velx      = forceFrac*cos( radians );
    int    y         = (int) (55*sin(radians) + vely*time - gravity*time*time);
    int    x         = (int) (55*cos(radians) + velx*time);

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
