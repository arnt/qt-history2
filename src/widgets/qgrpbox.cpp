/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qgrpbox.cpp#10 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qgrpbox.cpp#10 $";
#endif


/*! \class QGroupBox qgrpbox.h

  \brief The QGroupBox widget provides a group box frame with a title.

  This class is only partly documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */


/*!
Constructs a group box widget with no title.

The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    initMetaObject();
    align = AlignHCenter;
    setFrameStyle( QFrame::Box | QFrame::Plain );
    setLineWidth( 1 );
}


/*!
Sets the group box title text to \e title.
*/

void QGroupBox::setTitle( const char *title )
{
    if ( str == title )				// no change
	return;
    str = title;
    repaint();
}


/*!
\fn int QGroupBox::alignment() const
Returns the alignment of the group box title.

The default alignment is \c AlignHCenter.

\sa setAlignment().
*/

/*!
Sets the alignment of the group box title.

The title is always placed on the upper frame line, however,
the horizontal alignment can be specified by the \e alignment parameter.

The \e alignment is the bitwise OR of the following flags:
<ul>
<li> \c AlignLeft aligns the title text to the left.
<li> \c AlignRight aligns the title text to the right.
<li> \c AlignHCenter aligns the title text centered.
</ul>

\sa alignment().
*/

void QGroupBox::setAlignment( int alignment )
{
    align = alignment;
    repaint();
}


/*!
Internal; paints the group box.
*/

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
	    tw = fm.width( str, len ) + 2*fm.width( ' ' );
	    if ( tw < cr.width() )
		break;
	    len--;
	}
	if ( len ) {
	    r.setTop( h/2 );			// frame rect should be
	    setFrameRect( r );			//   smaller than client rect
	    int x;
	    if ( align & AlignHCenter )		// center alignment
		x = r.width()/2 - tw/2;
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
	paint.drawText( r, AlignCenter, str, len );
    }
    drawContents( &paint );
    paint.end();
}
