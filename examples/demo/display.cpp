/****************************************************************************
** $Id: //depot/qt/main/examples/demo/display.cpp#5 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "display.h"

#include <qpainter.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qdial.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qspinbox.h>

#include <math.h>

Screen::Screen(  QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    setLineWidth( FrameWidth );
    setFrameStyle( Panel | Sunken );
    setBackgroundMode( PaletteBase );
    setFixedSize( MaxSamples+2*FrameWidth, Range+2*FrameWidth );
    QPalette pal = palette();
    QColorGroup grp = pal.active();
    grp.setColor( QColorGroup::Foreground, QColor( blue ));
    grp.setColor( QColorGroup::Base, QColor( black ));
    pal.setActive( grp );
    pal.setInactive( grp );
    setPalette( pal );

    memset( yval, 0, sizeof( yval ));
    pos0 = 0;
    t0 = 0;
    step = 0;
}

void Screen::animate()
{
    if ( step == 0 )
	return;

    int t = t0;
    int p = pos0;
    if ( step < 0 ) {
	t += MaxSamples + step;
    } else {
	t -= step;
	p -= step;
	if ( p < 0 )
	    p += MaxSamples;
   }

    for ( int i = 0; i < QABS( step ); i++ ) {
	int y = (int)((Range-FrameWidth)/2 * sin( 3.1415*(double)t/180.0 ));
	yval[ p ] = y;
	++t;
	t %= 360;
	++p;
	p %= MaxSamples;
    }
    t0 -= step;
    if ( t0 < 0 )
	t0 += 360;
    pos0 = (pos0 - step) % MaxSamples;
    if ( pos0 < 0 )
	pos0 += MaxSamples;

    scroll( step, 0, QRect( FrameWidth, FrameWidth, MaxSamples, Range ));
}

void Screen::setStep( int s )
{
    step = s;
}

void Screen::drawContents( QPainter *p )
{
    QRect r = p->hasClipping() ?
	      p->clipRegion().boundingRect() : contentsRect();

    int vp = ( r.left() - FrameWidth + pos0 ) % MaxSamples;
    int y0 = FrameWidth + Range/2;

    for ( int x = r.left(); x <= r.right(); x++ ) {
	p->drawLine( x, y0 + yval[ vp ], x, r.bottom());
	++vp;
	vp %= MaxSamples;
    }
}

/***********************************************************************/

Curve::Curve( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    setLineWidth( FrameWidth );
    setFrameStyle( Panel | Sunken );
    setBackgroundMode( PaletteBase );
    QPalette pal = palette();
    QColorGroup grp = pal.active();
    grp.setColor( QColorGroup::Foreground, QColor( red ));
    grp.setColor( QColorGroup::Base, QColor( black ));
    pal.setActive( grp );
    pal.setInactive( grp );
    setPalette( pal );

    setFixedSize( 2*(Radius+FrameWidth), 2*(Radius+FrameWidth) );

    shift = 0;
    n = 1;
}

void Curve::drawContents( QPainter *p )
{
    p->moveTo( 100, 100 + (int)(90.0*sin( double(shift)*3.1415/180.0)));

    for ( double a = 0.0; a < 360.0; a += 1.0 ) {
	double rad = 3.1415 / 180.0 * a;
	double x = 103.0 + 90.0 * sin(rad);
	double y = 103.0 + 90.0 * sin(n * rad + double(shift)*3.1415/180.0);
	p->lineTo( int(x), int(y) );
    }
}

void Curve::animate()
{
    shift = (shift + 1) % 360;
    update( FrameWidth, FrameWidth, 2*Radius, 2*Radius );
}

void Curve::setFactor( int f )
{
    n = f;
}

/***********************************************************************/

DisplayWidget::DisplayWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    timer = 0;

    QVBoxLayout *vbox = new QVBoxLayout( this, 10 );

    QHBoxLayout *hbox = new QHBoxLayout( vbox );
    screen = new Screen( this );
    dial = new QDial( this );
    dial->setFixedSize( screen->height(), screen->height() );
    dial->setNotchesVisible( TRUE );
    dial->setRange( -10, 10 );
    dial->setValue( 1 );
    screen->setStep( dial->value() );
    connect( dial, SIGNAL( valueChanged( int )),
	     screen, SLOT( setStep( int )));
    hbox->addWidget( screen );
    hbox->addWidget( dial );

    curve = new Curve( this );
    spin = new QSpinBox( 1, 10, 1, this );
    spin->setFixedWidth( curve->width() );
    connect( spin, SIGNAL( valueChanged( int )), curve, SLOT( setFactor( int )));
    spin->setValue( 2 );

    lcd = new QLCDNumber( 2, this );
    lcd->setFixedSize( 100, 100 );
    lcdval = 0;

    bar = new QProgressBar( 10, this );
    tbar = 0;

    vbox->addWidget( curve );
    vbox->addWidget( spin );
    vbox->addWidget( lcd );
    vbox->addWidget( bar );
}

void DisplayWidget::run()
{
    if ( !timer ) {
	timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), SLOT( tick() ) );
    }

    timer->start( 5 );
}

void DisplayWidget::stop()
{
    timer->stop();
}

void DisplayWidget::tick()
{
    // sine
    screen->animate();
    // Lissajous
    curve->animate();
    // lcd display
    lcd->display( ++lcdval % 100 );
    // progress bar
    bar->setProgress( 5 + (int)(5*sin( 3.1415 * (double)tbar / 180.0 )));
    ++tbar;
    tbar %= 360;
}

void DisplayWidget::showEvent( QShowEvent * )
{
    run();
    screen->repaint();
}

void DisplayWidget::hideEvent( QHideEvent * )
{
    stop();
}
