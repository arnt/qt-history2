/****************************************************************************
** Implementation of LCDRange class, Qt tutorial 13
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
    repaint( cannonRect(), FALSE );
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

void CannonField::setGameOver()
{
    if ( noGame )
	return;
    if ( shooting )
	stopShooting();
    noGame = TRUE;
    repaint();
}

void CannonField::restartGame()
{    
    if ( shooting )
	stopShooting();
    newTarget();
    noGame = FALSE;
    repaint();
}

void CannonField::timerEvent( QTimerEvent * )
{
    QRect shotR   = shotRect();
    QRect targetR = targetRect();

    if ( shotR.intersects( targetR ) ) {
	erase( targetR );
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
    erase( shotR );
    timerCount++;
    repaint( shotRect(), FALSE );
}

void CannonField::paintEvent( QPaintEvent *e )
{
    QRect updateR = e->rect();
    QPainter p;
    p.begin( this );

    if ( updateR.intersects( cannonRect() ) )
	paintCannon( &p );
    if ( noGame ) {
	p.setPen( black );
	p.setFont( QFont( "Courier", 48, QFont::Bold ) );
	p.drawText( rect(), AlignCenter, "Game Over" );
    } else {
	if ( shooting &&  updateR.intersects( shotRect() ) )
	    paintShot( &p );
	if ( updateR.intersects( targetRect() ) )
	    paintTarget( &p );
    }
    p.end();
}

void CannonField::stopShooting()
{
    erase( shotRect() );
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
    tmp.drawRect( QRect( 33, -4, 15, 8 ) );
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
