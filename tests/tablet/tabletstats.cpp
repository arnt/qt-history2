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

#include "tabletstats.h"

void StatsCanvas::tabletEvent( QTabletEvent *e )
{
    QPainter p;
    
    
    QRect r( e->x() - e->pressure() / 2, e->y() - e->pressure() / 2, e->pressure(), e->pressure() );
    
    e->accept();
    p.begin( &buffer );
    p.setBrush( black );
    p.drawEllipse( r );
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
}

void StatsCanvas::paintEvent( QPaintEvent *e )
{
    QPainter p;
    QString str = tr("Test Your Tablet Pen Here");
    p.begin( &buffer );
    p.setPen( black );
    p.drawText( (width() / 2) - str.length() / 2, height() / 2, str );
    p.end();
    Canvas::paintEvent(e);
}


TabletStats::TabletStats( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    QGridLayout *layout = new QGridLayout( this, 4, 5 );
    int row;

    row = 0;
    QLabel *lbl;
    lbl = new QLabel( tr("X Pos:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblXPos = new QLabel( tr("Touch the canvas"), this );
    lblXPos->setMinimumSize( lblXPos->sizeHint() );
    layout->addWidget( lblXPos, row++, 1 );
    lbl = new QLabel( tr("Y Pos:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblYPos = new QLabel( tr("Touch the canvas"), this );
    lblYPos->setMinimumSize( lblYPos->sizeHint() );
    layout->addWidget( lblYPos, row++, 1 );
    lbl = new QLabel( tr("X Tilt:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblXTilt = new QLabel( tr("Touch the canvas"), this );
    lblXTilt->setMinimumSize( lblXTilt->sizeHint() );
    layout->addWidget( lblXTilt, row++, 1 );
    lbl = new QLabel( tr("Y Tilt:"), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblYTilt = new QLabel( tr("Touch the canvas"), this );
    lblYTilt->setMinimumSize( lblYTilt->sizeHint() );
    layout->addWidget( lblYTilt, row++, 1 );
    lbl = new QLabel( tr("Pressure: "), this );
    lbl->setMinimumSize( lbl->sizeHint() );
    layout->addWidget( lbl, row, 0 );
    lblPressure = new QLabel( tr("Touch the canvas"), this );
    lblPressure->setMinimumSize( lblPressure->sizeHint() );
    layout->addWidget( lblPressure, row++, 1 );
    lbl = new QLabel( tr("Device:"), this );
    layout->addWidget( lbl, row, 0 );
    lblDev = new QLabel( tr("Touch the canvas"), this );
    lblDev->setMinimumSize( lblDev->sizeHint() );
    layout->addWidget( lblDev, row++, 1 );
    statCan = new StatsCanvas( this );
    statCan->setMinimumSize( 100, 100 );
    layout->addMultiCellWidget(statCan, 0, row - 1, 2, 4 );
    layout->activate();

    QObject::connect( statCan, SIGNAL(signalNewTilt(int, int)),
	              this, SLOT(slotTiltChanged(int, int)) );
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
