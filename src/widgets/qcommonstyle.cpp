/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcommonstyle.cpp#2 $
**
** Implementation of the QCommonStyle class
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

#include "qcommonstyle.h"
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
#include "qtabbar.h"
#include "qscrollbar.h"
#include <limits.h>


/*!
  \class QCommonStyle qcommonstyle.h
  \brief Encapsulates common Look and Feel of a GUI.

  This abstract class implements some of the widget's look and feel
  that is common to all GUI styles provided and shipped as part of Qt.
*/

/*!
  Constructs a QCommonStyle that provides the style \a s.  This determines
  the default behavior of the virtual functions.
*/
QCommonStyle::QCommonStyle(GUIStyle s) : QStyle(s)
{
}

 /*!
  Destructs the style.
*/
QCommonStyle::~QCommonStyle()
{
}


/*! \reimp
 */
void QCommonStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool sunken,
			      bool /* editable */,
			      bool /*enabled */,
			      const QBrush *fill )
{
    drawButton(p, x, y, w, h, g, sunken, fill);
}


/*! \reimp
 */
QRect QCommonStyle::comboButtonRect( int x, int y, int w, int h)
{
    return buttonRect(x+3, y+3, w-6-21, h-6);
}

/*! \reimp
 */
QRect QCommonStyle::comboButtonFocusRect( int x, int y, int w, int h)
{
    return buttonRect(x+4, y+4, w-8-21, h-8);
}

/*! \reimp
*/

void QCommonStyle::drawComboButtonMask( QPainter *p, int x, int y, int w, int h)
{
    drawButtonMask(p, x, y, w, h);
}


/*!\reimp
 */
void QCommonStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
{
    QRect r = btn->rect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );

    int x1, y1, x2, y2;
    btn->rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates
    int dx = 0;
    int dy = 0;
    if ( btn->isMenuButton() )
	dx = (y2-y1) / 3;
    if ( dx || dy )
	p->translate( dx, dy );

    if ( btn->isDown() || btn->isOn() ){
	int sx = 0;
	int sy = 0;
	getButtonShift(sx, sy);
	x+=sx;
	y+=sy;
    }
    x += 2;  y += 2;  w -= 4;  h -= 4;
    drawItem( p, x, y, w, h,
	       AlignCenter|ShowPrefix,
	       btn->colorGroup(), btn->isEnabled(),
	       btn->pixmap(), btn->text(), -1, &btn->colorGroup().buttonText() );

    if ( dx || dy )
	p->translate( -dx, -dy );
}



/*!\reimp
 */
void QCommonStyle::getButtonShift( int &x, int &y)
{
    x = 0;
    y = 0;
}



/*!\reimp
 */
int QCommonStyle::defaultFrameWidth() const
{
    return 2;
}



/*!\reimp
 */
void QCommonStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe, int& overlap)
{
    overlap = 3;
    hframe = 24;
    vframe = 0;
    if ( t->shape() == QTabBar::RoundedAbove || t->shape() == QTabBar::RoundedBelow )
	vframe += 10;
}

/*!\reimp
 */
void QCommonStyle::drawTab( QPainter* p,  const  QTabBar* tb, QTab* t , bool selected )
{
    if ( tb->shape() == QTabBar::TriangularAbove || tb->shape() == QTabBar::TriangularBelow ) {
	// triangular, above or below
	int y;
	int x;
	QPointArray a( 10 );
	a.setPoint( 0, 0, -1 );
	a.setPoint( 1, 0, 0 );
	y = t->r.height()-2;
	x = y/3;
	a.setPoint( 2, x++, y-1 );
	a.setPoint( 3, x++, y );
	a.setPoint( 3, x++, y++ );
	a.setPoint( 4, x, y );
	
	int i;
	int right = t->r.width() - 1;
	for ( i = 0; i < 5; i++ )
	    a.setPoint( 9-i, right - a.point( i ).x(), a.point( i ).y() );

	if ( tb->shape() == QTabBar::TriangularAbove )
	    for ( i = 0; i < 10; i++ )
		a.setPoint( i, a.point(i).x(),
			    t->r.height() - 1 - a.point( i ).y() );

	a.translate( t->r.left(), t->r.top() );

	if ( selected )
	    p->setBrush( tb->colorGroup().base() );
	else
	    p->setBrush( tb->colorGroup().background() );
	p->setPen( tb->colorGroup().foreground() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
    }
}

/*!\reimp
 */
void QCommonStyle::drawTabMask( QPainter* p,  const  QTabBar* /* tb*/ , QTab* t, bool /* selected */ )
{
    p->drawRect( t->r );
}

/*!\reimp
 */
QStyle::ScrollControl QCommonStyle::scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p)
{
        if ( !sb->rect().contains( p ) )
	return NONE;
    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );
    pos = (sb->orientation() == QScrollBar::Horizontal)? p.x() : p.y();
    if ( pos < sliderMin )
	return SUB_LINE;
    if ( pos < sliderStart )
	return SUB_PAGE;
    if ( pos < sliderStart + sliderLength )
	return SLIDER;
    if ( pos < sliderMax + sliderLength )
	return ADD_PAGE;
    return ADD_LINE;
}

/*!\reimp
 */
void
QCommonStyle::drawSliderMask( QPainter *p,
			int x, int y, int w, int h,
			Orientation, bool, bool )
{
    p->fillRect(x, y, w, h, color1);
}


/*!\reimp
 */
void
QCommonStyle::drawSliderGrooveMask( QPainter *p,
				   int x, int y, int w, int h,
				   QCOORD /* c */,
				   Orientation )
{
    p->fillRect(x, y, w, h, color1);
}


/*!\reimp
 */
int QCommonStyle::maximumSliderDragDistance() const
{
    return -1;
}



static const int motifArrowHMargin	= 6;	// arrow horizontal margin

/*! \reimp
 */
int QCommonStyle::popupSubmenuIndicatorWidth( const QFontMetrics& fm  )
{
    return fm.ascent() + motifArrowHMargin;
}
