/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwindowsstyle.cpp#1 $
**
** Implementation of Windows-like style class
**
** Created : 981231
**
** Copyright (C) 1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qwindowsstyle.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now
#include "qpixmap.h" // for now
#include "qpalette.h" // for now
#include "qwidget.h"
#include "qlabel.h"
#include "qimage.h"
#include "qpushbutton.h"
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#define INCLUDE_MENUITEM_DEF
#include "qpopupmenu.h"
#include <limits.h>

/*!
  \class QWindowsStyle qwindowsstyle.h
  \brief Windows Look and Feel

  This class implements the look and feel known from the Windows
  plattform. Naturally it is also Qt's default GUI style on Windows.
*/


/*!
    Constructs a QWindowsStyle
*/
QWindowsStyle::QWindowsStyle() : QCommonStyle(WindowsStyle)
{
}


/*! \reimp */

void QWindowsStyle::drawIndicator( QPainter* p,
				   int x, int y, int w, int h, const QColorGroup &g,
				   int s, bool down, bool /*enabled*/ )
{
    QBrush fill;
    if ( s == QButton::NoChange ) {
	QBrush b = p->brush();	
	QColor c = p->backgroundColor();
	p->setBackgroundMode( TransparentMode );
	p->setBackgroundColor( green );
	fill = QBrush(g.base(), Dense4Pattern);
	p->setBackgroundColor( c );
	p->setBrush( b );
    } else if ( down )
	fill = g.brush( QColorGroup::Button );
    else
	fill = g.brush( QColorGroup::Base );
    qDrawWinPanel( p, x, y, w, h, g, TRUE, &fill );
    if ( s != QButton::Off ) {
	QPointArray a( 7*2 );
	int i, xx, yy;
	xx = x+3;
	yy = y+5;
	for ( i=0; i<3; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy++;
	}
	yy -= 2;
	for ( i=3; i<7; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy--;
	}
	if ( s == QButton::NoChange ) {
	    p->setPen( g.dark() );
	} else {
	    p->setPen( g.foreground() );
	}
	p->drawLineSegments( a );
    }
}



/*! \reimp */

void QWindowsStyle::drawFocusRect( QPainter* p,
			      const QRect& r, const QColorGroup &, const QColor* bg, bool)
{
    if (!bg)
	p->drawWinFocusRect( r );
    else
	p->drawWinFocusRect( r, *bg );
}



/*!
  This function draws a rectangle with two pixel line width.
  It is called from qDrawWinButton() and qDrawWinPanel().

  c1..c4 and fill are used:

    1 1 1 1 1 2
    1 3 3 3 4 2
    1 3 F F 4 2
    1 3 F F 4 2
    1 4 4 4 4 2
    2 2 2 2 2 2
*/
void QWindowsStyle::drawWinShades( QPainter *p,
				   int x, int y, int w, int h,
				   const QColor &c1, const QColor &c2,
				   const QColor &c3, const QColor &c4,
				   const QBrush *fill )
{
    if ( w < 2 || h < 2 )			// nothing to draw
	return;
    QPen oldPen = p->pen();
    QPointArray a( 3 );
    a.setPoint( 0, x, y+h-2 );
    a.setPoint( 1, x, y );
    a.setPoint( 2, x+w-2, y );
    p->setPen( c1 );
    p->drawPolyline( a );
    a.setPoint( 0, x, y+h-1 );
    a.setPoint( 1, x+w-1, y+h-1 );
    a.setPoint( 2, x+w-1, y );
    p->setPen( c2 );
    p->drawPolyline( a );
    if ( w > 4 && h > 4 ) {
	a.setPoint( 0, x+1, y+h-3 );
	a.setPoint( 1, x+1, y+1 );
	a.setPoint( 2, x+w-3, y+1 );
	p->setPen( c3 );
	p->drawPolyline( a );
	a.setPoint( 0, x+1, y+h-2 );
	a.setPoint( 1, x+w-2, y+h-2 );
	a.setPoint( 2, x+w-2, y+1 );
	p->setPen( c4 );
	p->drawPolyline( a );
	if ( fill ) {
	    QBrush oldBrush = p->brush();
	    p->setBrush( *fill );
	    p->setPen( NoPen );
	    p->drawRect( x+2, y+2, w-4, h-4 );
	    p->setBrush( oldBrush );
	}
    }
    p->setPen( oldPen );
}


/*! \reimp */

void
QWindowsStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
		const QColorGroup &g, bool sunken,
		   int lineWidth, const QBrush* fill)
{
    if ( lineWidth == 2 )
	QWindowsStyle::drawButton(p, x, y, w, h, g, sunken, fill);
    else
	QStyle::drawPanel( p, x, y, w, h, g, sunken, lineWidth, fill );
}

/*! \reimp */

void
QWindowsStyle::drawArrow( QPainter *p, ArrowType type, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool enabled, const QBrush *fill )
{
    QPointArray a;				// arrow polygon
    switch ( type ) {
    case UpArrow:
	a.setPoints( 7, -3,1, 3,1, -2,0, 2,0, -1,-1, 1,-1, 0,-2 );
	break;
    case DownArrow:
	a.setPoints( 7, -3,-1, 3,-1, -2,0, 2,0, -1,1, 1,1, 0,2 );
	break;
    case LeftArrow:
	a.setPoints( 7, 1,-3, 1,3, 0,-2, 0,2, -1,-1, -1,1, -2,0 );
	break;
    case RightArrow:
	a.setPoints( 7, -1,-3, -1,3, 0,-2, 0,2, 1,-1, 1,1, 2,0 );
	break;
    }
    if ( a.isNull() )
	return;

    if ( down ) {
	x++;
	y++;
    }

    QPen savePen = p->pen();			// save current pen
    if (down)
	p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
    p->fillRect( x, y, w, h, fill?*fill:g.brush( QColorGroup::Button ) );
    if (down)
	p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
    if ( enabled ) {
	a.translate( x+w/2, y+h/2 );
	p->setPen( g.foreground() );
	p->drawLineSegments( a, 0, 3 );		// draw arrow
	p->drawPoint( a[6] );
    } else {
	a.translate( x+w/2+1, y+h/2+1 );
	p->setPen( g.light() );
	p->drawLineSegments( a, 0, 3 );		// draw arrow
	p->drawPoint( a[6] );
	a.translate( -1, -1 );
	p->setPen( g.mid() );
	p->drawLineSegments( a, 0, 3 );		// draw arrow
	p->drawPoint( a[6] );
    }
    p->setPen( savePen );			// restore pen

}

/*! \reimp */

QSize
QWindowsStyle::indicatorSize() const
{
    return QSize(13,13);
}


#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
/*! \reimp */

void QWindowsStyle::drawExclusiveIndicator( QPainter* p,
				   int x, int y, int w, int h, const QColorGroup &g,
				   bool on, bool down, bool /* enabled */ )
{

    static QCOORD pts1[] = {		// dark lines
	1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
    static QCOORD pts2[] = {		// black lines
	2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
    static QCOORD pts3[] = {		// background lines
	2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
    static QCOORD pts4[] = {		// white lines
	2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
	11,4, 10,3, 10,2 };
    static QCOORD pts5[] = {		// inner fill
	4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
    p->eraseRect( x, y, w, h );
    QPointArray a( QCOORDARRLEN(pts1), pts1 );
    a.translate( x, y );
    p->setPen( g.dark() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts2), pts2 );
    a.translate( x, y );
    p->setPen( g.shadow() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts3), pts3 );
    a.translate( x, y );
    p->setPen( g.midlight() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts4), pts4 );
    a.translate( x, y );
    p->setPen( g.light() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts5), pts5 );
    a.translate( x, y );
    QColor fillColor = down ? g.button() : g.base();
    p->setPen( fillColor );
    p->setBrush( down ?  g.brush( QColorGroup::Button ) :
                         g.brush( QColorGroup::Base ) ) ;
    p->drawPolygon( a );
    if ( on ) {
	p->setPen( NoPen );
	p->setBrush( g.foreground() );
	p->drawRect( x+5, y+4, 2, 4 );
	p->drawRect( x+4, y+5, 4, 2 );
    }

}


/*!
  Draws the mask of a mark indicating the state of an exclusive choice
*/
void
QWindowsStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool /* on */)
{
    QColorGroup g(color1, color1, color1, color1, color1, color1, color1, color1, color0);
    drawExclusiveIndicator(p , x, y, w, h, g, FALSE, FALSE, FALSE );
    //p->fillRect(x, y, w, h, color1);
}



/*!\reimp
 */
QSize
QWindowsStyle::exclusiveIndicatorSize() const
{
    return QSize(12,12);
}




/*!
  Draws a press-sensitive shape.
*/
void QWindowsStyle::drawButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    if (sunken)
	drawWinShades( p, x, y, w, h,
		       g.shadow(), g.light(), g.dark(), g.button(),
                       fill?fill: &g.brush( QColorGroup::Button ) );
    else
	drawWinShades( p, x, y, w, h,
		       g.light(), g.shadow(), g.midlight(), g.dark(),
		       fill?fill:&g.brush( QColorGroup::Button ) );

}

/*!\reimp
 */
void QWindowsStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    QWindowsStyle::drawButton(p, x, y, w, h, g, sunken, fill);
}


/*!\reimp
 */
void
QWindowsStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
    QColorGroup g = btn->colorGroup();
    int x1, y1, x2, y2;

    btn->rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates
    int w = x2 + 1;
    int h = y2 + 1;

    p->setPen( g.foreground() );
    p->setBrush( QBrush(g.button(),NoBrush) );

    bool clearButton = TRUE;
    if ( btn->isDown() ) {
	if ( btn->isDefault() ) {
	    p->setPen( g.shadow() );
	    p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
	    p->setPen( g.dark() );
	    p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	} else {
	    drawButton( p, x1, y1, w, h, g, TRUE,
			&g.brush( QColorGroup::Button ) );
	}
    } else {
	if ( btn->isDefault() ) {
	    p->setPen( g.shadow() );
	    p->drawRect( x1, y1, w, h );
	    x1++; y1++;
	    x2--; y2--;
	}
	if ( btn->isToggleButton() && btn->isOn() && btn->isEnabled() ) {
	    QBrush fill(g.light(), Dense4Pattern );
	    drawButton( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, &fill );
	    clearButton = FALSE;
	} else {
	    drawButton( p, x1, y1, x2-x1+1, y2-y1+1, g, btn->isOn(),
			&g.brush( QColorGroup::Button ) );
	}
    }
    if ( clearButton ) {
	if (btn->isDown())
	    p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
	p->fillRect( x1+2, y1+2, x2-x1-3, y2-y1-3,
		     g.brush( QColorGroup::Button ) );
	if (btn->isDown())
	    p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
    }
    if ( btn->isMenuButton() ) {
	int dx = (y2-y1) / 3;
	drawArrow( p, DownArrow, FALSE,
		    x2 - dx, y1, dx, y2 - y1,
		    g, btn->isEnabled() );
    }

    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );

}


/*!\reimp
 */
void QWindowsStyle::getButtonShift( int &x, int &y)
{
    x = 1;
    y = 1;
}

/*!\reimp
 */
void QWindowsStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
				     const QColorGroup &g, bool sunken ,
				     bool /* editable */,
				     bool enabled,
				     const QBrush *fill )
{
    qDrawWinPanel(p, x, y, w, h, g, TRUE,
                   fill?fill:(enabled?&g.brush( QColorGroup::Base ):
                                      &g.brush( QColorGroup::Background )));
    // the special reversed left shadow panel ( slightly different from drawPanel() )
    qDrawWinPanel(p, w-2-16,2,16,h-4, g, sunken);
    drawArrow( p, QStyle::DownArrow, sunken,
	       w-2-16+ 2, 2+ 2, 16- 4, h-4- 4, g, enabled, fill );

}

/*!\reimp
 */
QRect QWindowsStyle::comboButtonRect( int x, int y, int w, int h){
    return QRect(x+2, y+2, w-4-16, h-4);
}


/*!\reimp
 */
QRect QWindowsStyle::comboButtonFocusRect( int x, int y, int w, int h)
{
    return QRect(x+3, y+3, w-6-16, h-6);
}


/*! \reimp */
void QWindowsStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe, int& overlap)
{
    QCommonStyle::tabbarMetrics( t, hframe, vframe, overlap );
}

/*! \reimp */
void QWindowsStyle::drawTab( QPainter* p,  const QTabBar* tb, QTab* t , bool selected )
{
    QRect r( t->r );
    if ( tb->shape()  == QTabBar::RoundedAbove ) {
	p->setPen( tb->colorGroup().midlight() );
	p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
	p->setPen( tb->colorGroup().light() );
	p->drawLine( r.left(), r.bottom()-1, r.right(), r.bottom()-1 );
	if ( r.left() == 0 )
	    p->drawPoint( tb->rect().bottomLeft() );
	else {
	    p->setPen( tb->colorGroup().midlight() );
	    p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
	}

	if ( selected ) {
	    p->fillRect( QRect( r.left()+1, r.bottom()-1, r.width()-3, 2),
			 tb->palette().normal().brush( QColorGroup::Background ));
 	    p->setPen( tb->colorGroup().background() );
// 	    p->drawLine( r.left()+1, r.bottom(), r.right()-2, r.bottom() );
// 	    p->drawLine( r.left()+1, r.bottom()-1, r.right()-2, r.bottom()-1 );
	    p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top()+2 );
	    p->setPen( tb->colorGroup().light() );
	} else {
	    p->setPen( tb->colorGroup().light() );
	    r.setRect( r.left() + 2, r.top() + 2,
		       r.width() - 4, r.height() - 2 );
	}

	p->drawLine( r.left(), r.bottom()-1, r.left(), r.top() + 2 );
	p->drawPoint( r.left()+1, r.top() + 1 );
	p->drawLine( r.left()+2, r.top(),
		     r.right() - 2, r.top() );
	if ( r.left() > 0 ) {
 	    p->setPen( tb->colorGroup().midlight() );
	}
	p->drawPoint( r.left(), r.bottom());
	
	p->setPen( tb->colorGroup().midlight() );
	p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top() + 2 );
	p->drawLine( r.left()+2, r.top()+1,
		     r.right() - 2, r.top()+1 );

	p->setPen( tb->colorGroup().dark() );
	p->drawLine( r.right() - 1, r.top() + 2,
		     r.right() - 1, r.bottom() - 1 + (selected?1:-1));
	p->setPen( tb->colorGroup().shadow() );
	p->drawPoint( r.right() - 1, r.top() + 1 );
	p->drawLine( r.right(), r.top() + 2, r.right(), r.bottom() - (selected?1:2));
	p->drawPoint( r.right() - 1, r.top() + 1 );
    } else if ( tb->shape() == QTabBar::RoundedBelow ) {
        if ( selected ) {
	    p->fillRect( QRect( r.left()+1, r.top(), r.width()-3, 1),
			 tb->palette().normal().brush( QColorGroup::Background ));
 	    p->setPen( tb->colorGroup().background() );
// 	    p->drawLine( r.left()+1, r.top(), r.right()-2, r.top() );
	    p->drawLine( r.left()+1, r.top(), r.left()+1, r.bottom()-2 );
	    p->setPen( tb->colorGroup().dark() );
	} else {
	    p->setPen( tb->colorGroup().dark() );
	    p->drawLine( r.left(), r.top(), r.right(), r.top() );
	    r.setRect( r.left() + 2, r.top(),
		       r.width() - 4, r.height() - 2 );
	}

	p->drawLine( r.right() - 1, r.top(),
		     r.right() - 1, r.bottom() - 2 );
	p->drawPoint( r.right() - 2, r.bottom() - 2 );
	p->drawLine( r.right() - 2, r.bottom() - 1,
		     r.left() + 1, r.bottom() - 1 );
	p->drawPoint( r.left() + 1, r.bottom() - 2 );

	p->setPen( tb->colorGroup().shadow() );
	p->drawLine( r.right(), r.top(),
		     r.right(), r.bottom() - 1 );
	p->drawPoint( r.right() - 1, r.bottom() - 1 );
	p->drawLine( r.right() - 1, r.bottom(),
		     r.left() + 2, r.bottom() );

	p->setPen( tb->colorGroup().light() );
	p->drawLine( r.left(), r.top(),
		     r.left(), r.bottom() - 2 );
	
    } else {
	QCommonStyle::drawTab( p, tb, t, selected );
    }

}

/*! \reimp */
void QWindowsStyle::drawTabMask( QPainter* p,  const QTabBar* tb, QTab* t, bool selected )
{
    QCommonStyle::drawTabMask(p, tb, t, selected );
}

#define HORIZONTAL	(sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2
#define SLIDER_MIN	9 // ### wtf does this have to do with motif?

/*!\reimp
 */
void QWindowsStyle::scrollBarMetrics( const QScrollBar* sb, int &sliderMin, int &sliderMax, int &sliderLength, int&buttonDim )
{
    int maxLength;
    int b = 0;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2  )
	buttonDim = extent - b*2;
    else
	buttonDim = ( length - b*2 )/2 - 1;

    sliderMin = b + buttonDim;
    maxLength  = length - b*2 - buttonDim*2;

    if ( sb->maxValue() == sb->minValue() ) {
	sliderLength = maxLength;
    } else {
	sliderLength = (sb->pageStep()*maxLength)/
			(sb->maxValue()-sb->minValue()+sb->pageStep());
	uint range = sb->maxValue()-sb->minValue();
 	if ( sliderLength < SLIDER_MIN || range > INT_MAX/2 )
	    sliderLength = SLIDER_MIN;
	if ( sliderLength > maxLength )
	    sliderLength = maxLength;
    }
    sliderMax = sliderMin + maxLength - sliderLength;

}


/*!\reimp
 */
void QWindowsStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb, int sliderStart, uint controls, uint activeControl )
{
#define ADD_LINE_ACTIVE ( activeControl == ADD_LINE )
#define SUB_LINE_ACTIVE ( activeControl == SUB_LINE )
    QColorGroup g  = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax) { // sanity check
	sliderStart = sliderMax;
    }

    int b = 0;
    int dimB = buttonDim;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( HORIZONTAL ) {
	subY = addY = ( extent - dimB ) / 2;
	subX = b;
	addX = length - dimB - b;
    } else {
	subX = addX = ( extent - dimB ) / 2;
	subY = b;
	addY = length - dimB - b;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
	subPageR.setRect( subB.right() + 1, b,
			  sliderStart - subB.right() - 1 , sliderW );
	addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
	sliderR .setRect( sliderStart, b, sliderLength, sliderW );
    } else {
	subPageR.setRect( b, subB.bottom() + 1, sliderW,
			  sliderStart - subB.bottom() - 1 );
	addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
	sliderR .setRect( b, sliderStart, sliderW, sliderLength );
    }

    bool maxedOut = (sb->maxValue() == sb->minValue());
    if ( controls & ADD_LINE ) {
	drawBevelButton( p, addB.x(), addB.y(),
			 addB.width(), addB.height(), g,
			 ADD_LINE_ACTIVE, &g.brush( QColorGroup::Button ) );
	drawArrow( p, VERTICAL ? DownArrow : RightArrow,
		   ADD_LINE_ACTIVE, addB.x()+2, addB.y()+2,
		   addB.width()-4, addB.height()-4, g, !maxedOut );
    }
    if ( controls & SUB_LINE ) {
	drawBevelButton( p, subB.x(), subB.y(),
			 subB.width(), subB.height(), g,
			 SUB_LINE_ACTIVE );
	drawArrow( p, VERTICAL ? UpArrow : LeftArrow,
		   SUB_LINE_ACTIVE, subB.x()+2, subB.y()+2,
		   subB.width()-4, subB.height()-4, g, !maxedOut );
    }
    p->setBrush( g.brush( QColorGroup::Light ).pixmap() ?
                 g.brush( QColorGroup::Light )     :
                 QBrush(g.light(), Dense4Pattern) );
    p->setPen( NoPen );
    p->setBackgroundMode( OpaqueMode );
    if ( maxedOut ) {
	p->drawRect( sliderR );
    } else {
	if ( (controls & SUB_PAGE && SUB_PAGE == activeControl) ||
	     (controls  & ADD_PAGE && ADD_PAGE == activeControl) ) {
	    QBrush b = p->brush();	
	    QColor c = p->backgroundColor();
// 	    p->fillRect( ADD_PAGE == activeControl? addPageR : subPageR, g.fillDark() );
	    p->setBackgroundColor( g.dark() );
 	    p->setBrush( QBrush(g.shadow(), Dense4Pattern) );
 	    p->drawRect( ADD_PAGE == activeControl? addPageR : subPageR );
	    p->setBackgroundColor( c );
	    p->setBrush( b );
	}
	if ( controls & SUB_PAGE && SUB_PAGE != activeControl)
	    p->drawRect( subPageR );
	if ( controls & ADD_PAGE && ADD_PAGE != activeControl)
	    p->drawRect( addPageR );
	if ( controls & SLIDER ) {
	    if ( !maxedOut ) {
		QPoint bo = p->brushOrigin();
		p->setBrushOrigin(sliderR.topLeft());
		drawBevelButton( p, sliderR.x(), sliderR.y(),
				 sliderR.width(), sliderR.height(), g,
				 FALSE, &g.brush( QColorGroup::Button ) );
		p->setBrushOrigin(bo);
	    }
	}
    }
    // ### perhaps this should not be able to accept focus if maxedOut?
    if ( sb->hasFocus() && (controls & SLIDER) )
	drawFocusRect(p, QRect(sliderR.x()+2, sliderR.y()+2,
			       sliderR.width()-5, sliderR.height()-5), g,
		      &sb->backgroundColor());

}

/*!\reimp
 */
int QWindowsStyle::sliderLength() const
{
    return 9;
}

/*!\reimp
 */
void QWindowsStyle::drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     Orientation orient, bool tickAbove, bool tickBelow )
{
    // 3333330
    // 3444410
    // 3422210
    // 3422210
    // 3422210
    // 3422210
    // *34210*
    // **340**
    // ***0***



    const QColor c0 = g.shadow();
    const QColor c1 = g.dark();
    //    const QColor c2 = g.button();
    const QColor c3 = g.midlight();
    const QColor c4 = g.light();


    int x1 = x;
    int x2 = x+w-1;
    int y1 = y;
    int y2 = y+h-1;

    if ( tickAbove && tickBelow || !tickAbove && !tickBelow ) {
	qDrawWinButton( p, QRect(x,y,w,h), g, FALSE,
			&g.brush( QColorGroup::Button ) );
	return;
    }

    QBrush oldBrush = p->brush();
    p->setBrush( g.brush( QColorGroup::Button ) );
    p->setPen( NoPen );
    p->drawRect( x,y,w,h );
    p->setBrush( oldBrush );


    enum  { SlUp, SlDown, SlLeft, SlRight } dir;

    if ( orient == Horizontal )
	if ( tickAbove )
	    dir = SlUp;
	else
	    dir = SlDown;
    else
	if ( tickAbove )
	    dir = SlLeft;
	else
	    dir = SlRight;

    switch ( dir ) {
    case SlUp:
	y1 = y1 + w/2;
	break;
    case SlDown:
	y2 = y2 - w/2;
	break;
    case SlLeft:
	x1 = x1 + h/2;
	break;
    case SlRight:
	x2 = x2 - h/2;
	break;
    }

    if ( dir != SlUp ) {
	p->setPen( c4 );
	p->drawLine( x1, y1, x2, y1 );
	p->setPen( c3 );
	p->drawLine( x1, y1+1, x2, y1+1 );
    }
    if ( dir != SlLeft ) {
	p->setPen( c3 );
	p->drawLine( x1+1, y1+1, x1+1, y2 );
	p->setPen( c4 );
	p->drawLine( x1, y1, x1, y2 );
    }
    if ( dir != SlRight ) {
	p->setPen( c0 );
	p->drawLine( x2, y1, x2, y2 );
	p->setPen( c1 );
	p->drawLine( x2-1, y1+1, x2-1, y2-1 );
    }
    if ( dir != SlDown ) {
	p->setPen( c0 );
	p->drawLine( x1, y2, x2, y2 );
	p->setPen( c1 );
	p->drawLine( x1+1, y2-1, x2-1, y2-1 );
    }

    int d;
    switch ( dir ) {
	case SlUp:
	    p->setPen( c4 );
	    d =  (w + 1) / 2 - 1;
	    p->drawLine( x1, y1, x1+d, y1-d);
	    p->setPen( c0 );
	    d = w - d - 1;
	    p->drawLine( x2, y1, x2-d, y1-d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x2-1, y1, x2-1-d, y1-d);
	    break;
	case SlDown:
	    p->setPen( c4 );
	    d =  (w + 1) / 2 - 1;
	    p->drawLine( x1, y2, x1+d, y2+d);
	    p->setPen( c0 );
	    d = w - d - 1;
	    p->drawLine( x2, y2, x2-d, y2+d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x2-1, y2, x2-1-d, y2+d);
	    break;
	case SlLeft:
	    p->setPen( c4 );
	    d =  (h + 1) / 2 - 1;
	    p->drawLine( x1, y1, x1-d, y1+d);
	    p->setPen( c0 );
	    d = h - d - 1;
	    p->drawLine( x1, y2, x1-d, y2-d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x1, y2-1, x1-d, y2-1-d);
	    break;
	case SlRight:
	    p->setPen( c4 );
	    d =  (h + 1) / 2 - 1;
	    p->drawLine( x2, y1, x2+d, y1+d);
	    p->setPen( c0 );
	    d = h - d - 1;
	    p->drawLine( x2, y2, x2+d, y2-d);
	    p->setPen( c1 );
	    d--;
	    p->drawLine( x2, y2-1, x2+d, y2-1-d);
	    break;
    }
}

/*!
  Draws the mask of a slider
*/
void
QWindowsStyle::drawSliderMask( QPainter *p,
			int x, int y, int w, int h,
			Orientation orient, bool tickAbove, bool tickBelow )
{

    if ( tickAbove && tickBelow || !tickAbove && !tickBelow ) {
	p->fillRect(x, y, w, h, color1);
	return;
    }

    int x1 = x;
    int x2 = x+w-1;
    int y1 = y;
    int y2 = y+h-1;

    enum  { SlUp, SlDown, SlLeft, SlRight } dir;

    if ( orient == Horizontal )
	if ( tickAbove )
	    dir = SlUp;
	else
	    dir = SlDown;
    else
	if ( tickAbove )
	    dir = SlLeft;
	else
	    dir = SlRight;

    switch ( dir ) {
    case SlUp:
	y1 = y1 + w/2;
	break;
    case SlDown:
	y2 = y2 - w/2;
	break;
    case SlLeft:
	x1 = x1 + h/2;
	break;
    case SlRight:
	x2 = x2 - h/2;
	break;
    }

    QPointArray a;

    switch ( dir ) {
    case SlUp:
    a.setPoints(5, x1,y1, x1 + w/2, y1 - w/2, x2,y1, x2,y2, x1,y2);
	break;
    case SlDown:
    a.setPoints(5, x1,y1, x2,y1,  x2,y2,  x1 + w/2, y2 + w/2, x1,y2);
	break;
    case SlLeft:
    a.setPoints(5, x1,y1, x2,y1, x2,y2, x1,y2, x1 - h/2, y1 + h/2 );
	break;
    case SlRight:
    a.setPoints(5, x1,y1, x2,y1, x2 + h/2, y1 + h/2,  x2,y2, x1,y2);
	break;
    }


    p->setBrush(color1);
    p->setPen(color1);
    p->drawPolygon( a );

}


/*!\reimp
 */
void QWindowsStyle::drawSliderGroove( QPainter *p,
				      int x, int y, int w, int h,
				      const QColorGroup& g, QCOORD c,
				      Orientation orient )
{

    if ( orient == Horizontal ) {
	qDrawWinPanel( p, x, y + c - 2,  w, 4, g, TRUE );
	p->setPen( g.shadow() );
	p->drawLine( x+1, y + c - 1, x + w - 3, y + c - 1 );
    } else {
	qDrawWinPanel( p, x + c - 2, y, 4, h, g, TRUE );
	p->setPen( g.shadow() );
	p->drawLine( x + c - 1, y + 1, x + c - 1, y + h - 3 );
    }

}
/*!\reimp
 */
int QWindowsStyle::maximumSliderDragDistance() const
{
    return 20;
}


/*! \reimp
*/

int QWindowsStyle::splitterWidth() const
{
    return 6;
}


/*! \reimp
*/

void QWindowsStyle::drawSplitter( QPainter *p,  int x, int y, int w, int h,
				  const QColorGroup &g,  Orientation)
{
	qDrawWinPanel( p, x, y, w, h, g );
}



static const int motifItemFrame		= 2;	// menu item frame width
static const int motifSepHeight		= 2;	// separator item height
static const int motifItemHMargin	= 3;	// menu item hor text margin
static const int motifItemVMargin	= 2;	// menu item ver text margin
static const int motifArrowHMargin	= 6;	// arrow horizontal margin
static const int motifArrowVMargin	= 2;	// arrow vertical margin
static const int motifTabSpacing	= 12;	// space between text and tab
static const int motifCheckMarkHMargin	= 2;	// horiz. margins of check mark

/*! \reimp
*/
void QWindowsStyle::polishPopupMenu( QPopupMenu* p)
{
    p->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    p->setMouseTracking( TRUE );
    p->setCheckable( TRUE );

}



/*! \reimp
*/
void QWindowsStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
				   const QColorGroup &g,
				   bool act, bool dis )
{
    const int markW = 7;
    const int markH = 7;
    int posX = x + ( w - markW )/2 - 1;
    int posY = y + ( h - markH )/2;

    // Could do with some optimizing/caching...
    QPointArray a( 7*2 );
    int i, xx, yy;
    xx = posX;
    yy = 3 + posY;
    for ( i=0; i<3; i++ ) {
	a.setPoint( 2*i,   xx, yy );
	a.setPoint( 2*i+1, xx, yy+2 );
	xx++; yy++;
    }
    yy -= 2;
    for ( i=3; i<7; i++ ) {
	a.setPoint( 2*i,   xx, yy );
	a.setPoint( 2*i+1, xx, yy+2 );
	xx++; yy--;
    }
    if ( dis && !act ) {
	uint pnt;
	p->setPen( g.highlightedText() );
	QPoint offset(1,1);
	for ( pnt = 0; pnt < a.size(); pnt++ )
	    a[pnt] += offset;
	p->drawLineSegments( a );
	for ( pnt = 0; pnt < a.size(); pnt++ )
	    a[pnt] -= offset;
    }
    p->setPen( g.text() );
    p->drawLineSegments( a );
}

/*! \reimp
*/
int QWindowsStyle::widthOfPopupCheckColumn( int maxpm )
{
    int cmw = 12;   // check mark width
    int w = QMAX( maxpm, cmw );
    w += 4;
    return w;
}



int QWindowsStyle::extraPopupMenuItemWidth( bool checkable, QMenuItem* mi, const QFontMetrics& fm )
{
    return 0;
}

int QWindowsStyle::popupMenuItemHeight( bool checkable, QMenuItem* mi, const QFontMetrics& fm )
{
    int h = 0;
    if ( mi->isSeparator() ) {			// separator height
	h = motifSepHeight;
    } else if ( mi->pixmap() ) {		// pixmap height
	h = mi->pixmap()->height() + 2*motifItemFrame;
    } else {					// text height
	h = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    }
    if ( !mi->isSeparator() && mi->iconSet() != 0 ) {
	h = QMAX( h, mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height() + 2*motifItemFrame );
	int h2 = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
	if ( h2 > h )
	    h = h2;
    }
    return h;

}

void QWindowsStyle::drawPopupMenuItem( QPainter* p, bool checkable, int tab, QMenuItem* mi,
				       const QFontMetrics& fm,
				       bool act, int x, int y, int w, int h)
{
}
