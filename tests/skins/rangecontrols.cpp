/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "rangecontrols.h"

#include <qlayout.h>
#include <qdial.h>
#include <qvbox.h>
#include <qsplitter.h>
#include <qlcdnumber.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qstyle.h>
#include "qskin.h"

#include <limits.h>

RangeControls::RangeControls( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QVBoxLayout *cell2 = new QVBoxLayout( this, 10, 6 );

    QLabel *label1 = new QLabel( QString( "Enter a value between\n%1 and %2:" ).arg( -INT_MAX ).arg( INT_MAX ), this );
    label1->setMaximumHeight( label1->sizeHint().height() );
    cell2->addWidget(label1);
    QSpinBox *sb1 = new QSpinBox( -INT_MAX, INT_MAX, 1, this );
    sb1->setValue( 0 );
    cell2->addWidget(sb1);

    QLabel *label2 = new QLabel( "Enter a zoom value:", this );
    label2->setMaximumHeight( label2->sizeHint().height() );
    cell2->addWidget(label2);
    QSpinBox *sb2 = new QSpinBox( 0, 1000, 10, this );
    sb2->setSuffix( " %" );
    sb2->setSpecialValueText( "Automatic" );
    cell2->addWidget(sb2);

    QLabel *label3 = new QLabel( "Enter a price:", this );
    label3->setMaximumHeight( label3->sizeHint().height() );
    cell2->addWidget(label3);
    QSpinBox *sb3 = new QSpinBox( 0, INT_MAX, 1, this );
    sb3->setPrefix( "$" );
    sb3->setValue( 355 );
    cell2->addWidget(sb3);
    cell2->addWidget(new QWidget(this));

    /* start very custom stuff */
    QWidget *wid = new QWidget(this, "rangecontrols");
    cell2->addWidget(wid);

    QSkinDial *dial = new QSkinDial(wid, "dial");
    QLCDNumber *lcd4 = new QLCDNumber( 2, wid, "lcd_dial" );
    lcd4->setSegmentStyle(QLCDNumber::Filled);
    lcd4->setFrameStyle(QFrame::NoFrame);
    connect( dial, SIGNAL( valueChanged( int ) ), lcd4, SLOT( display( int ) ) );


    QSlider *hslider = new QSlider( 0, 64, 1, 33, Qt::Horizontal, wid, "slider1" );
    QLCDNumber *lcd2 = new QLCDNumber( 2, wid, "lcd_slider1" );
    lcd2->display( 33 );
    lcd2->setSegmentStyle( QLCDNumber::Filled );
    connect( hslider, SIGNAL( valueChanged( int ) ), lcd2, SLOT( display( int ) ) );

    QSlider *vslider = new QSlider( 0, 64, 1, 8, Qt::Vertical, wid, "slider2" );
    QLCDNumber *lcd3 = new QLCDNumber( 3, wid , "lcd_slider2");
    lcd3->display( 8 );
    connect( vslider, SIGNAL( valueChanged( int ) ), lcd3, SLOT( display( int ) ) );

    if (style().inherits("QSkinStyle")) {
	QSkinLayout *s = new QSkinLayout(wid, "rangelayout");
	s->add(dial);
	s->add(lcd4);
	s->add(hslider);
	s->add(lcd2);
	s->add(vslider);
	s->add(lcd3);
    } else {
	QHBoxLayout *row2 = new QHBoxLayout( wid, 6 );
	QVBoxLayout *ccell1 = new QVBoxLayout( row2, 6 );

	ccell1->addWidget(dial);
	ccell1->addWidget(lcd4);

	QVBoxLayout *cell3 = new QVBoxLayout( row2, 6 );

	cell3->addWidget(hslider);
	cell3->addWidget(lcd2);

	row2->addWidget(vslider);
	row2->addWidget(lcd3);
    }
}
