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
    QPainter p( this );
    
    QRect r( e->x() - e->pressure() / 2, e->y() - e->pressure() / 2, e->pressure(), e->pressure() );
    
    e->accept();
    p.setBrush( black );
    p.drawEllipse( r );
    

    bitBlt( this, r.x(), r.y(), &buffer, r.x(), r.y(), r.width(), r.height() );
}

void StatsCanvas::mouseMoveEvent( QMouseEvent *e )
{
    // do nothing
}


TabletStats::TabletStats( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	
	
    
	QGridLayout *layout = new QGridLayout( this, 4, 5 );
	
	QLabel *lbl = new QLabel( "X Tilt:", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl, 0, 0 );
	lblXTilt = new QLabel( "Don't Know", this );
	lblXTilt->setMinimumSize( lblXTilt->sizeHint() );
	layout->addWidget( lblXTilt, 0, 1 );
	lbl = new QLabel( "Y Tilt:", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl, 1, 0 );
	lblYTilt = new QLabel( "Don't Know", this );
	lblYTilt->setMinimumSize( lblYTilt->sizeHint() );
	layout->addWidget( lblYTilt, 1, 2 );
	lbl = new QLabel( "Pressure: ", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl, 2, 0 );
	lblPressure = new QLabel( "Don't know", this );
	lblPressure->setMinimumSize( lblPressure->sizeHint() );
	layout->addWidget( lblPressure, 2, 1 );
	lbl = new QLabel( "Device:", this );
	layout->addWidget( lbl, 3, 0 );
	lblDev = new QLabel( "Don't know", this );
	lblDev->setMinimumSize( lblDev->sizeHint() );
	layout->addWidget( lblDev, 3, 1 );
	statCan = new StatsCanvas( this );
	statCan->setMinimumSize( 100, 100 );
	layout->addMultiCellWidget(statCan, 0, 3, 2, 4 );
	layout->activate();
}

TabletStats::~TabletStats()
{
}

void TabletStats::slotDevChanged( int newDev )
{
}

void TabletStats::slotLocationChanged( int newX, int newY )
{
}

void TabletStats::slotTiltChanged( int newTiltX, int newTiltY )
{
}

void TabletStats::slotPressureChanged( int newP )
{
}

/*
void TabletStats::tabletEvent( QTabletEvent *e )
{
	e->accept();
	lblXTilt->setNum( e->xTilt() );
	lblYTilt->setNum( e->yTilt() );
	lblPressure->setNum( e->pressure() );
	switch( e->device() ) {
	case QTabletEvent::Stylus:
		lblDev->setText( "Stylus" );
		break;
	case QTabletEvent::Eraser:
		lblDev->setText( "Eraser" );
		break;
	default:
		lblDev->setText("something");
		break;
	}
}
*/