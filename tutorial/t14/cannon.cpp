/****************************************************************
**
** Implementation Cannonfield class, Qt tutorial 14
**
****************************************************************/

#include "cannon.h"
#include <qpainter.h>
#include <qpixmap.h>
#include <qwmatrix.h>
#include <qfont.h>

#include <math.h>
#include <stdlib.h>

CannonField::CannonField( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    ang           = 45;
    f             = 0;
    shooting      = FALSE;
    timerCount    = 0;
    shoot_ang	  = 0;
    shoot_f	  = 0;
    gameEnded     = FALSE;
    barrelPressed = FALSE;
    newTarget();
}

void CannonField::setAngle( int degrees )
{
    if ( degrees < 5 )
	degrees = 5;
    if ( degrees > 70 )
	degrees = 70;
    if ( ang == degrees )
	return;
    ang = degrees;
    repaint( cannonRect(), FALSE );
    emit angleChanged( ang );
}

void CannonField::setForce( int newton )
{
    if ( newton < 0 )
	newton = 0;
    if ( newton > 50 )
	newton = 50;
    if ( f == newton )
	return;
    f = newton;
    emit forceChanged( f );
    const bool cheat = TRUE;
    if ( cheat ) {
	update();
	QRect r;
	QPainter p;
	p.begin( this );
	timerCount = 0;
	shoot_ang = ang;
	shoot_f = f;
	p.setPen( red );
	do {
	    r = shotRect();
	    p.drawPoint( r.center() );
	    timerCount++;
	} while ( r.x() < width() );
	timerCount = 0;
	p.end();
    }
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
    if ( gameEnded )
	return;
    if ( shooting )
	stopShooting();
    gameEnded = TRUE;
    repaint();
}

void CannonField::restartGame()
{    
    if ( shooting )
	stopShooting();
    newTarget();
    gameEnded = FALSE;
    repaint();
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
    if ( shotR.intersects( barrierRect() ) ) {
	stopShooting();
	repaint( barrierRect(), FALSE );
	emit missed();
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
    if ( updateR.intersects( barrierRect() ) )
	paintBarrier( &p );
    if ( gameEnded ) {
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

void CannonField::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    if ( barrelHit( e->pos() ) )
	barrelPressed = TRUE;
}

const QRect barrel_rect(33, -4, 15, 8);

bool CannonField::barrelHit( const QPoint &p ) const
{
    QWMatrix mtx;
    mtx.translate( 0, height() - 1 );
    mtx.rotate( -ang );
    QRect r = mtx.map( barrel_rect );
    return r.contains( p );
}

void CannonField::mouseMoveEvent( QMouseEvent *e )
{
    if ( !barrelPressed )
	return;
    QPoint pnt = e->pos();
    if ( pnt.x() <= 0 )
	pnt.setX( 1 );
    if ( pnt.y() >= height() )
	pnt.setY( height() - 1 );
    double rad = atan( ((double) rect().bottom() - pnt.y()) /  pnt.x() );
    setAngle ( qRound ( rad*180/3.14159265 ) );
}

void CannonField::mouseReleaseEvent( QMouseEvent *e )
{
    mouseMoveEvent( e );
    barrelPressed = FALSE;
}

void CannonField::mouseDoubleEvent( QMouseEvent *e )
{
    mousePressEvent( e );
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

void CannonField::paintBarrier( QPainter *p )
{
    p->setBrush( yellow );
    p->setPen( black );
    p->drawRect( barrierRect() );
}

void CannonField::paintCannon( QPainter *p )
{
    QPixmap  pix( cannonRect().size() );
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

QRect CannonField::barrierRect() const
{
    return QRect( 145, height() - 100, 15, 100 );
}

void  CannonField::newTarget()
{
    target = QPoint( 200 + rand() % ( width() - 200 - 10), 
                      35 + rand() % (height() - 35  - 10) );
}
