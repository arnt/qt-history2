/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qgroupbox.cpp#1 $
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

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qgroupbox.cpp#1 $";
#endif


QGroupBox::QGroupBox( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
    align = AlignCenter;
    setFrame( QFrame::Box | QFrame::Plain );
}

QGroupBox::QGroupBox( const char *title, QWidget *parent, const char *name )
	: QFrame( parent, name ), str(title)
{
    align = AlignCenter;
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
    int   tw  = 0;
    QRect cr  = clientRect();
    QRect r   = cr;
    int   len = str.length();
    QPainter paint;
    paint.begin( this );
    if ( len == 0 )				// no title
	setFrameRect( QRect(0,0,0,0) );
    else {					// set up region etc. for title
	QFontMetrics fm = paint.fontMetrics();
	int h = fm.height();
	while ( len ) {
	    tw = fm.width( str ) + 2*fm.width( ' ' );
	    if ( tw < cr.width() )
		break;
	    len--;
	}
	if ( len ) {
	    r.setTop( h/2 );
	    setFrameRect( r );
	    int x;
	    if ( align & AlignCenter )		// center alignment
		x = r.width()/2;
	    else if ( align & AlignRight )	// right alignment
		x = r.width() - tw - 5;
	    else				// |eft alignment
		x = 5;
	    r.setRect( x, 0, tw, h );
	    QRegion rgn_all( cr );
	    QRegion rgn_title( r );
	    rgn_all = rgn_all.subtract( rgn_title );
	    paint.setClipRegion( rgn_all );
	}
    }
    drawFrame( &paint );
    if ( tw ) {
	paint.setClipping( FALSE );
	paint.drawText( r, AlignCenter | AlignVCenter, str, len );
    }
    drawContents( &paint );
    paint.end();
}
