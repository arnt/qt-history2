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

#include "tabletstats.h"

TabletStats::TabletStats( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	QHBoxLayout *layout = new QHBoxLayout( this );
	
	QLabel *lbl = new QLabel( "X Tilt:", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl );
	lblXTilt = new QLabel( "Don't Know", this );
	lblXTilt->setMinimumSize( lblXTilt->sizeHint() );
	layout->addWidget( lblXTilt );
	lbl = new QLabel( "Y Tilt:", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl );
	lblYTilt = new QLabel( "Don't Know", this );
	lblYTilt->setMinimumSize( lblYTilt->sizeHint() );
	layout->addWidget( lblYTilt );
	lbl = new QLabel( "Pressure: ", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl );
	lblPressure = new QLabel( "Don't know", this );
	lblPressure->setMinimumSize( lblPressure->sizeHint() );
	layout->addWidget( lblPressure );
	lbl = new QLabel( "Device:", this );
	layout->addWidget( lbl );
	lblDev = new QLabel( "Don't know", this );
	lblDev->setMinimumSize( lblDev->sizeHint() );
	layout->addWidget( lblDev );
	layout->activate();
}

TabletStats::~TabletStats()
{
}

void TabletStats::tabletEvent( QTabletEvent *e )
{
	lblXTilt->setNum( e->xTilt() );
	lblYTilt->setNum( e->yTilt() );
	lblPressure->setNum( e->pressure() );
	switch( e->device() ) {
	case QTabletEvent::STYLUS:
		lblDev->setText( "Stylus" );
		break;
	case QTabletEvent::ERASER:
		lblDev->setText( "Eraser" );
		break;
	default:
		lblDev->setText("something");
		break;
	}
}
