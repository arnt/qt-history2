/****************************************************************************
** $Id: $
**
** Copyright ( C ) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <cmath>

#include "tabletstats.h"

MyOrientation::MyOrientation( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    setFrameStyle( QFrame::Box | QFrame::Sunken );
}

MyOrientation::~MyOrientation()
{
}

void MyOrientation::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    
}

void MyOrientation::newOrient( int tiltX, int tiltY )
{
    
    /*
    const MY_Z = 50;	// a faux Z setting, to mess with calculations
    double PI = 3.14159265359;
    QPainter p(this);
    p.setPen( black );
    p.drawLine( width() / 2, 0, width() / 2, height() );
    p.drawLine(  0, height() / 2, width(), height() / 2 );
    p.setBrush( red );
    int tmpX = MY_Z * tan( tiltX * (PI / 180 ) );
    int tmpY = MY_Z * tan( tiltY * (PI / 180 ) );
    p.drawLine( width() / 2, height() / 2, tmpX, tmpY );  
    p.end();
    */
}


void StatsCanvas::tabletEvent( QTabletEvent *e )
{
    QPainter p;
    
    static QRect oldR( -1, -1, -1, -1 );
    QRect r( e->x() - e->pressure() / 2, e->y() - e->pressure() / 2, e->pressure(), e->pressure() );
    
    e->accept();
    p.begin( &buffer );
    
    if ( !oldR.isNull() ) {
	p.fillRect( oldR, colorGroup().base() );
	bitBlt( this, oldR.x(), oldR.y(), &buffer, oldR.x(), oldR.y(),
	    oldR.width(), oldR.height() );
    }
    p.setBrush( black );
    p.drawEllipse( r );
    oldR = r;
    p.end();
    
    bitBlt( this, r.x(), r.y(), &buffer, r.x(), r.y(), r.width(), r.height() );
    emit signalNewTilt( e->xTilt(), e->yTilt() );
    emit signalNewDev( e->device() );
    emit signalNewLoc( e->x(), e->y() );
    emit signalNewPressure( e->pressure() );
}

void StatsCanvas::mouseMoveEvent( QMouseEvent *e )
{
    // do nothing
    QWidget::mouseMoveEvent( e );
}

void StatsCanvas::mouseReleaseEvent( QMouseEvent *e )
{
    clearScreen();
}

void StatsCanvas::paintEvent( QPaintEvent *e )
{
    /*
    QPainter p;
    QString str = tr("Test Your Tablet Pen Here");
    p.begin( &buffer );
    int offset = p.fontMetrics().boundingRect(str).width() / 2;
    // remove the old one...
    p.setPen( black );
    p.drawText( (width() / 2) - offset, height() / 2, str );
    p.end();
    */
    Canvas::paintEvent( e );
}

TabletStats::TabletStats( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    QGridLayout *layout = new QGridLayout( this, 4, 5 );
    int row;
    QString strStart( tr("Touch The Canvas") );

    row = 0;
    QLabel *lbl;
    lbl = new QLabel( tr("X Pos:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblXPos = new QLabel( strStart, this );
    lblXPos->setMinimumSize( lblXPos->sizeHint() );
    layout->addWidget( lblXPos, row++, 1 );
    lbl = new QLabel( tr("Y Pos:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblYPos = new QLabel( strStart, this );
    lblYPos->setMinimumSize( lblYPos->sizeHint() );
    layout->addWidget( lblYPos, row++, 1 );
    lbl = new QLabel( tr("X Tilt:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblXTilt = new QLabel( strStart, this );
    lblXTilt->setMinimumSize( lblXTilt->sizeHint() );
    layout->addWidget( lblXTilt, row++, 1 );
    lbl = new QLabel( tr("Y Tilt:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblYTilt = new QLabel( strStart, this );
    lblYTilt->setMinimumSize( lblYTilt->sizeHint() );
    layout->addWidget( lblYTilt, row++, 1 );
    lbl = new QLabel( tr("Pressure: "), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblPressure = new QLabel( strStart, this );
    lblPressure->setMinimumSize( lblPressure->sizeHint() );
    layout->addWidget( lblPressure, row++, 1 );
    lbl = new QLabel( tr("Device:"), this );
    layout->addWidget( lbl, row, 0 );
    lblDev = new QLabel( strStart, this );
    lblDev->setMinimumSize( lblDev->sizeHint() );
    layout->addWidget( lblDev, row++, 1 );
    orient = new MyOrientation( this, "stuff" );
    layout->addWidget( orient, row++, 0 );
    statCan = new StatsCanvas( this );
    statCan->setMinimumSize( 100, 100 );
    layout->addMultiCellWidget(statCan, 0, row - 1, 2, 4 );
    layout->activate();

    QObject::connect( statCan, SIGNAL(signalNewTilt(int, int)),
	              this, SLOT(slotTiltChanged(int, int)) );
    QObject::connect( statCan, SIGNAL(signalNewTilt(int, int)),
	              orient, SLOT(newOrient(int, int)) );
    QObject::connect( statCan, SIGNAL(signalNewPressure(int)),
                      this, SLOT(slotPressureChanged(int)) );
    QObject::connect( statCan, SIGNAL(signalNewDev(int)),
                      this, SLOT(slotDevChanged(int)) );
    QObject::connect( statCan, SIGNAL(signalNewLoc(int,int)),
                      this, SLOT( slotLocationChanged(int,int)) );
}

TabletStats::~TabletStats()
{
}

void TabletStats::slotDevChanged( int newDev )
{
    if ( newDev == QTabletEvent::Stylus )
	lblDev->setText( tr("Stylus") );
    else if ( newDev == QTabletEvent::Eraser )
	lblDev->setText( tr("Eraser") );
}

void TabletStats::slotLocationChanged( int newX, int newY )
{
    lblXPos->setNum( newX );
    lblYPos->setNum( newY );
}

void TabletStats::slotTiltChanged( int newTiltX, int newTiltY )
{
    lblXTilt->setNum( newTiltX );
    lblYTilt->setNum( newTiltY );

}

void TabletStats::slotPressureChanged( int newP )
{
    lblPressure->setNum( newP );
}
