/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qgroupbox.cpp#4 $
**
** Implementation of QGroupBox widget class
**
** Author  : Haavard Nord
** Created : 950203
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qgrpbox.h"
#include "qpainter.h"
#include "qpalette.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qgroupbox.cpp#4 $";
#endif


QGroupBox::QGroupBox( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
    initMetaObject();
    align = AlignCenter;
    setFrame( QFrame::Box | QFrame::Plain );
    setFrameWidth( 1 );
}


void QGroupBox::setTitle( const char *title )
{
    if ( str == title )				// no change
	return;
    str = title;
    repaint();
}


void QGroupBox::setAlignment( int alignment )
{
    align = alignment;
    repaint();
}


void QGroupBox::paintEvent( QPaintEvent * )	// overrides QFrame::paintEvent
{
    int   	tw  = 0;
    QRect 	cr  = rect();
    QRect 	r   = cr;
    int   	len = str.length();
    QColorGroup g = colorGroup();
    QPainter	paint;

    paint.begin( this );
    if ( len == 0 )				// no title
	setFrameRect( QRect(0,0,0,0) );		//  then use client rect
    else {					// set up region for title
	QFontMetrics fm = paint.fontMetrics();
	int h = fm.height();
	while ( len ) {
	    tw = fm.width( str ) + 2*fm.width( ' ' );
	    if ( tw < cr.width() )
		break;
	    len--;
	}
	if ( len ) {
	    r.setTop( h/2 );			// frame rect should be
	    setFrameRect( r );			//   smaller than client rect
	    int x;
	    if ( align & AlignCenter )		// center alignment
		x = r.width()/2;
	    else if ( align & AlignRight )	// right alignment
		x = r.width() - tw - 8;
	    else				// |eft alignment
		x = 8;
	    r.setRect( x, 0, tw, h );
	    QRegion rgn_all( cr );
	    QRegion rgn_title( r );
	    rgn_all = rgn_all.subtract( rgn_title );
	    paint.setClipRegion( rgn_all );	// clip everything but title
	}
    }
    drawFrame( &paint );			// draw the frame
    if ( tw ) {					// draw the title
	paint.setClipping( FALSE );
	paint.setPen( g.text() );
	paint.drawText( r, AlignCenter | AlignVCenter, str, len );
    }
    drawContents( &paint );
    paint.end();
}
