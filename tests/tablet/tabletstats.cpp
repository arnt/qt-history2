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
	//QVBoxLayout *vLayout = new QVBoxLayout( this );
	QLabel *lbl = new QLabel( "X Tilt:", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl );
	lblXTilt = new QLabel( "Don't Know", this );
	lblXTilt->setMinimumSize( lblXTilt->sizeHint() );
	layout->addWidget( lblXTilt );
	//vLayout->addLayout( layout );
	//layout = new QHBoxLayout( this );
	lbl = new QLabel( "Y Tilt:", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl );
	lblYTilt = new QLabel( "Don't Know", this );
	lblYTilt->setMinimumSize( lblYTilt->sizeHint() );
	layout->addWidget( lblYTilt );
	//vLayout->addLayout( layout );
	//layout = new QHBoxLayout( this );
	lbl = new QLabel( "Pressure: ", this );
	lbl->setMinimumSize( lbl->sizeHint() );
	layout->addWidget( lbl );
	lblPressure = new QLabel( "Don't know", this );
	lblPressure->setMinimumSize( lblPressure->sizeHint() );
	layout->addWidget( lblPressure );
	//vLayout->addLayout( layout );
}

TabletStats::~TabletStats()
{
}

void TabletStats::tabletEvent( QTabletEvent *e )
{
	lblXTilt->setNum( e->xTilt() );
	lblYTilt->setNum( e->yTilt() );
	lblPressure->setNum( e->pressure() );
}