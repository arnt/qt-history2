/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qvariant.h> // HP-UX compiler need this here

#include "orderindicator.h"
#include "formwindow.h"

#include <qpainter.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qevent.h>

OrderIndicator::OrderIndicator( int i, QWidget* w, FormWindow *fw )
    : QWidget( fw, "order_indicator", WMouseNoMask ), formWindow( fw )
{
    order = -1;
    widget = w;
    setBackgroundMode( NoBackground );
    setAutoMask( TRUE );
    setOrder( i, w );
}


OrderIndicator::~OrderIndicator()
{
}


void OrderIndicator::setOrder( int i, QWidget* wid )
{
    if ( widget != wid )
	return;
    if ( !wid->isVisibleTo( formWindow ) ) {
	hide();
	return;
    }

    if ( order == i ) {
	show();
	raise();
	return;
    }
    order = i;
    int w = fontMetrics().width( QString::number( i ) ) + 10;
    int h = fontMetrics().lineSpacing() * 3 / 2;
    QFont f( font() );
    f.setBold( TRUE );
    setFont( f );
    resize( qMax( w, h ), h );
    update(); // in case the size didn't change
    reposition();
    show();
    raise();
}

void OrderIndicator::reposition()
{
    QPoint p =parentWidget()->mapFromGlobal( widget->mapToGlobal( widget->rect().topLeft() ) );
    move( p - QPoint( width()/3, height()/3 ) );
}


void OrderIndicator::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    p.setPen( white );
    p.setBrush( blue );
    p.drawEllipse( rect() );
    p.drawText( rect(), AlignCenter, QString::number( order ) );
}


void OrderIndicator::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );
    {
	QPainter p( &bm, this );
	p.setPen( color1 );
	p.setBrush( color1 );
	p.drawEllipse( rect() );
    }
    setMask( bm );
}

void OrderIndicator::mousePressEvent( QMouseEvent *e )
{
    QApplication::sendEvent( widget, e );
}
