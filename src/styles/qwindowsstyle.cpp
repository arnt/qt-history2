/****************************************************************************
** $Id: //depot/qt/main/src/styles/qwindowsstyle.cpp#53 $
**
** Implementation of Windows-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwindowsstyle.h"

#ifndef QT_NO_STYLE_WINDOWS

#define INCLUDE_MENUITEM_DEF
#include "qpopupmenu.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now
#include "qpixmap.h" // for now
#include "qpalette.h" // for now
#include "qwidget.h"
#include "qlabel.h"
#include "qimage.h"
#include "qpushbutton.h"
#include "qcombobox.h"
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qlistview.h"
#include "qbitmap.h"
#include "qcleanuphandler.h"

#include <limits.h>


class QWindowsStylePrivate
{
public:
    QWindowsStylePrivate() : hotWidget( 0 )
    {
    }

    QWidget *hotWidget;
};


// NOT REVISED
/*!
  \class QWindowsStyle qwindowsstyle.h
  \brief The QWindowsStyle class provides Microsoft Windows look and feel.
  \ingroup appearance

  This class implements the look and feel known from the Windows
  platform. Naturally it is also Qt's default GUI style on Windows.
*/


/*!
    Constructs a QWindowsStyle
*/
QWindowsStyle::QWindowsStyle() : QCommonStyle(WindowsStyle)
{
    d = new QWindowsStylePrivate;
}


/*!\reimp
*/
QWindowsStyle::~QWindowsStyle()
{
    delete d;
}


/*!\reimp */
void QWindowsStyle::polish( QWidget *widget )
{
    if ( widget->inherits( "QToolButton" ) )
	widget->installEventFilter( this );
}


/*!\reimp */
void QWindowsStyle::unPolish( QWidget *widget )
{
    widget->removeEventFilter( this );
}


/*!
  \reimp
*/
void QWindowsStyle::drawPrimitive( PrimitiveOperation op,
				   QPainter *p,
				   const QRect &r,
				   const QColorGroup &cg,
				   PFlags flags,
				   void *data ) const
{
    switch (op) {
    case PO_ButtonCommand:
    case PO_ButtonBevel:
	qDrawWinButton(p, r, cg, flags & PStyle_Sunken, &cg.brush(QColorGroup::Button));
	break;

    case PO_FocusRect:
	p->drawWinFocusRect(r);
	break;

    default:
	if (op >= PO_ArrowUp && op <= PO_ArrowLeft) {
	    QPointArray a;

	    switch ( op ) {
	    case PO_ArrowUp:
		a.setPoints( 7, -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2 );
		break;

	    case PO_ArrowDown:
		a.setPoints( 7, -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1 );
		break;

	    case PO_ArrowRight:
		a.setPoints( 7, -1,-3, -1,3, 0,-2, 0,2, 1,-1, 1,1, 2,0 );
		break;

	    case PO_ArrowLeft:
		a.setPoints( 7, 1,-3, 1,3, 0,-2, 0,2, -1,-1, -1,1, -2,0 );
		break;

	    default:
		break;
	    }

	    if (a.isNull())
		return;

	    QPen savePen = p->pen();                    // save current pen
	    if ( flags & PStyle_Enabled ) {
		a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
		p->setPen( cg.buttonText() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    } else {
		a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
		p->setPen( cg.light() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
		a.translate( -1, -1 );
		p->setPen( cg.mid() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    }
	    p->setPen( savePen );                       // restore pen
	} else
	    QCommonStyle::drawPrimitive(op, p, r, cg, flags, data);
    }
}


/*!
  \reimp
*/
void QWindowsStyle::drawControl( ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 CFlags how,
				 void *data ) const
{
    switch (element) {
    case CE_PushButton: {
	QPushButton *button = (QPushButton *) widget;
	QRect br = r;
	int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);

	PFlags flags = PStyle_Default;
	if (button->isEnabled())
	    flags |= PStyle_Enabled;
	if (button->isDown())
	    flags |= PStyle_Sunken;

	if (button->isDefault() || button->autoDefault()) {
	    if ( button->isDefault()) {
		p->setPen(cg.shadow());
		p->drawRect(br);
	    }

	    br.setCoords(br.left()   + dbi,
			 br.top()    + dbi,
			 br.right()  - dbi,
			 br.bottom() - dbi);
	}

	if (button->isDown() && button->isDefault()) {
	    p->setPen(cg.dark());
	    p->drawRect(br);
	} else
	    drawPrimitive(PO_ButtonCommand, p, br, cg, flags);

	if (button->hasFocus())
	    drawPrimitive(PO_FocusRect, p, subRect(SR_PushButtonFocusRect, widget),
			  cg, flags);
	break; }

    default:
	QCommonStyle::drawControl(element, p, widget, r, cg, how, data);
    }
}


/*!
  \reimp
*/
int QWindowsStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 1;
	break;

    case PM_ScrollBarMaximumDragDistance:
	ret = 20;
	break;

    default:
	ret = QCommonStyle::pixelMetric(metric, widget);
	break;
    }

    return ret;
}


/*!
  \reimp
*/
QSize QWindowsStyle::sizeFromContents( ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       void *data ) const
{
    QSize sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, data);

    if (contents == CT_PushButton) {
	QPushButton *button = (QPushButton *) widget;
	int w = sz.width(), h = sz.height();

	if (button->isDefault() || button->autoDefault()) {
	    if (w < 85 && ! button->pixmap())
		w = 80;
	    if (h < 25)
		h = 25;
	} else {
	    if (h < 23)
		h = 23;
	}

	sz = QSize(w, h);
    }

    return sz;
}


/*! \reimp */

void QWindowsStyle::drawIndicator( QPainter* p,
                                   int x, int y, int w, int h, const QColorGroup &g,
                                   int s, bool down, bool enabled )
{
#ifndef QT_NO_BUTTON
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
        fill = g.brush( enabled ? QColorGroup::Base : QColorGroup::Background );
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
            p->setPen( g.text() );
        }
        p->drawLineSegments( a );
    }
#endif
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


/*! \reimp */

void
QWindowsStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
                const QColorGroup &g, bool sunken,
                   int lineWidth, const QBrush* fill)
{
    if ( lineWidth == 2 )
	qDrawWinPanel(p, x, y, w, h, g, sunken);
    else
        QStyle::drawPanel( p, x, y, w, h, g, sunken, lineWidth, fill );
}


/*! \reimp */
void
QWindowsStyle::drawPopupPanel( QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g,  int /* lineWidth */,
                               const QBrush *fill )
{
    qDrawWinPanel( p, x, y,  w, h, g, FALSE, fill );
}


/*! \reimp */

void
QWindowsStyle::drawArrow( QPainter *p, ArrowType type, bool down,
                 int x, int y, int w, int h,
                 const QColorGroup &g, bool enabled, const QBrush *fill )
{
    QPointArray a;                              // arrow polygon
    switch ( type ) {
    case UpArrow:
        a.setPoints( 7, -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2 );
        break;
    case DownArrow:
        a.setPoints( 7, -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1 );
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
        x += pixelMetric(PM_ButtonShiftHorizontal, 0),
        y += pixelMetric(PM_ButtonShiftVertical, 0);
    }

    QPen savePen = p->pen();                    // save current pen
    if (down)
        p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
    if ( fill )
        p->fillRect( x, y, w, h, *fill );
    if (down)
        p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
    if ( enabled ) {
        a.translate( x+w/2, y+h/2 );
        p->setPen( g.buttonText() );
        p->drawLineSegments( a, 0, 3 );         // draw arrow
        p->drawPoint( a[6] );
    } else {
        a.translate( x+w/2+1, y+h/2+1 );
        p->setPen( g.light() );
        p->drawLineSegments( a, 0, 3 );         // draw arrow
        p->drawPoint( a[6] );
        a.translate( -1, -1 );
        p->setPen( g.mid() );
        p->drawLineSegments( a, 0, 3 );         // draw arrow
        p->drawPoint( a[6] );
    }
    p->setPen( savePen );                       // restore pen

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
                                   bool on, bool down, bool enabled )
{

    static const QCOORD pts1[] = {              // dark lines
        1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
    static const QCOORD pts2[] = {              // black lines
        2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
    static const QCOORD pts3[] = {              // background lines
        2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
    static const QCOORD pts4[] = {              // white lines
        2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
        11,4, 10,3, 10,2 };
    static const QCOORD pts5[] = {              // inner fill
        4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
    // in right to left mode, we need it reversed
    static const QCOORD rpts1[] = {             // dark lines
        11-1,9, 11-1,8, 11-0,7, 11-0,4, 11-1,3, 11-1,2, 11-2,1, 11-3,1, 11-4,0, 11-7,0, 11-8,1, 11-9,1 };
    static const QCOORD rpts2[] = {             // black lines
        11-2,8, 11-1,7, 11-1,4, 11-2,3, 11-2,2, 11-3,2, 11-4,1, 11-7,1, 11-8,2, 11-9,2 };
    static const QCOORD rpts3[] = {             // background lines
        11-2,9, 11-3,9, 11-4,10, 11-7,10, 11-8,9, 11-9,9, 11-9,8, 11-10,7, 11-10,4, 11-9,3 };
    static const QCOORD rpts4[] = {             // white lines
        11-2,10, 11-3,10, 11-4,11, 11-7,11, 11-8,10, 11-9,10, 11-10,9, 11-10,8, 11-11,7,
            11-11,4, 11-10,3, 11-10,2 };
    static const QCOORD rpts5[] = {             // inner fill
        11-4,2, 11-7,2, 11-9,4, 11-9,7, 11-7,9, 11-4,9, 11-2,7, 11-2,4 };

    p->eraseRect( x, y, w, h );
    bool reverse = QApplication::reverseLayout();
    QPointArray a;
    if( reverse )
        a.setPoints( QCOORDARRLEN(rpts1), rpts1 );
    else
        a.setPoints( QCOORDARRLEN(pts1), pts1 );
    a.translate( x, y );
    p->setPen( g.dark() );
    p->drawPolyline( a );
    if( reverse )
        a.setPoints( QCOORDARRLEN(rpts2), rpts2 );
    else
        a.setPoints( QCOORDARRLEN(pts2), pts2 );
    a.translate( x, y );
    p->setPen( g.shadow() );
    p->drawPolyline( a );
    if( reverse )
        a.setPoints( QCOORDARRLEN(rpts3), rpts3 );
    else
        a.setPoints( QCOORDARRLEN(pts3), pts3 );
    a.translate( x, y );
    p->setPen( g.midlight() );
    p->drawPolyline( a );
    if( reverse )
        a.setPoints( QCOORDARRLEN(rpts4), rpts4 );
    else
        a.setPoints( QCOORDARRLEN(pts4), pts4 );
    a.translate( x, y );
    p->setPen( g.light() );
    p->drawPolyline( a );
    if( reverse )
        a.setPoints( QCOORDARRLEN(rpts5), rpts5 );
    else
        a.setPoints( QCOORDARRLEN(pts5), pts5 );
    a.translate( x, y );
    QColor fillColor = ( down || !enabled ) ? g.button() : g.base();
    p->setPen( fillColor );
    p->setBrush( fillColor  ) ;
    p->drawPolygon( a );
    if ( on ) {
        p->setPen( NoPen );
        p->setBrush( g.text() );
        p->drawRect( x+5, y+4, 2, 4 );
        p->drawRect( x+4, y+5, 4, 2 );
    }

}


/*!\reimp
*/
void
QWindowsStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool /* on */)
{
    QColorGroup g(color1, color1, color1, color1, color1, color1, color1, color1, color0);
    drawExclusiveIndicator(p , x, y, w, h, g, FALSE, FALSE, FALSE );
}



/*!\reimp
 */
QSize
QWindowsStyle::exclusiveIndicatorSize() const
{
    return QSize(12,12);
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
    //qDrawWinPanel(p, w-2-16,2,16,h-4, g, sunken);
    // #### DO SUNKEN!
    int xpos = x;
    if( !QApplication::reverseLayout() )
        xpos += w - 2 - 16;

    qDrawWinPanel( p, xpos, y+2, 16, h-4, g, sunken,
		   fill ? fill : &g.brush( QColorGroup::Button ) );

    drawArrow( p, QStyle::DownArrow, sunken,
               xpos + 2, y+2+ 2, 16- 4, h-4- 4, g, enabled,
               fill ? fill : &g.brush( QColorGroup::Button ) );

}

/*!\reimp
 */
QRect QWindowsStyle::comboButtonRect( int x, int y, int w, int h ) const
{
    QRect r(x+2, y+2, w-4-16, h-4);
    if( QApplication::reverseLayout() )
        r.moveBy( 2 + 16, 0 );
    return r;
}


/*!\reimp
 */
QRect QWindowsStyle::comboButtonFocusRect( int x, int y, int w, int h ) const
{
    QRect r(x+3, y+3, w-6-16, h-6);
    if( QApplication::reverseLayout() )
        r.moveBy( 2 + 16, 0 );
    return r;
}


/*! \reimp */
void QWindowsStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe,
                                   int& overlap ) const
{
    QCommonStyle::tabbarMetrics( t, hframe, vframe, overlap );
}

/*! \reimp */
void QWindowsStyle::drawTab( QPainter* p,  const QTabBar* tb, QTab* t , bool selected )
{
#ifndef QT_NO_TABBAR
    QRect r( t->rect() );
    bool reverse = QApplication::reverseLayout();
    if ( tb->shape()  == QTabBar::RoundedAbove ) {
        p->setPen( tb->colorGroup().midlight() );
        p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
        p->setPen( tb->colorGroup().light() );
        p->drawLine( r.left(), r.bottom()-1, r.right(), r.bottom()-1 );
        if ( r.left() == 0 )
            p->drawPoint( tb->rect().bottomLeft() );
//      else {
//          p->setPen( tb->colorGroup().midlight() );
//          p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
//      }

        if ( selected ) {
            p->fillRect( QRect( r.left()+1, r.bottom()-1, r.width()-3, 2),
                         tb->colorGroup().brush( QColorGroup::Background ));
            p->setPen( tb->colorGroup().background() );
            p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top()+2 );
            p->setPen( tb->colorGroup().light() );
        } else {
            p->setPen( tb->colorGroup().light() );
            r.setRect( r.left() + 2, r.top() + 2,
                       r.width() - 4, r.height() - 2 );
        }

        int x1, x2;
        if( reverse ) {
            x2 = r.left() + 2;
            x1 = r.right();
        } else {
            x1 = r.left();
            x2 = r.right() - 2;
        }
        p->drawLine( x1, r.bottom()-1, x1, r.top() + 2 );
        if ( reverse )
            x1--;
        else
            x1++;
        p->drawPoint( x1, r.top() + 1 );
        if ( reverse )
            x1--;
        else
            x1++;
        p->drawLine( x1, r.top(), x2, r.top() );
        if ( r.left() > 0 ) {
            p->setPen( tb->colorGroup().midlight() );
        }
        if ( reverse )
            x1 = r.right();
        else
            x1 = r.left();
        p->drawPoint( x1, r.bottom());

        p->setPen( tb->colorGroup().midlight() );
        if ( reverse )
            x1--;
        else
            x1++;
        p->drawLine( x1, r.bottom(), x1, r.top() + 2 );
        if ( reverse )
            x1--;
        else
            x1++;
        p->drawLine( x1, r.top()+1, x2, r.top()+1 );

        p->setPen( tb->colorGroup().dark() );
        if( reverse ) {
            x2 = r.left() + 1;
        } else {
            x2 = r.right() - 1;
        }
        p->drawLine( x2, r.top() + 2, x2, r.bottom() - 1 + (selected?1:-1));
        p->setPen( tb->colorGroup().shadow() );
        p->drawPoint( x2, r.top() + 1 );
        p->drawPoint( x2, r.top() + 1 );
        if ( reverse )
            x2--;
        else
            x2++;
        p->drawLine( x2, r.top() + 2, x2, r.bottom() - (selected?1:2));
    } else if ( tb->shape() == QTabBar::RoundedBelow ) {
        if ( selected ) {
            // ### add right to left support here!!!!
            p->fillRect( QRect( r.left()+1, r.top(), r.width()-3, 1),
                         tb->palette().active().brush( QColorGroup::Background ));
            p->setPen( tb->colorGroup().background() );
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
#endif
}

/*!\reimp
 */
int QWindowsStyle::sliderLength() const
{
    return 11;
}

enum  SliderDir { SlUp, SlDown, SlLeft, SlRight };

/*!\reimp
 */
void QWindowsStyle::drawSlider( QPainter *p,
                             int x, int y, int w, int h,
                             const QColorGroup &g,
                             Orientation orient, bool tickAbove, bool tickBelow )
{
#ifndef QT_NO_SLIDER
    // 4444440
    // 4333310
    // 4322210
    // 4322210
    // 4322210
    // 4322210
    // *43210*
    // **410**
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

    bool reverse = QApplication::reverseLayout();

    p->fillRect( x, y, w, h, g.brush( QColorGroup::Background ) );

    if ( (tickAbove && tickBelow) || (!tickAbove && !tickBelow) ) {
        qDrawWinButton( p, QRect(x,y,w,h), g, FALSE,
                        &g.brush( QColorGroup::Button ) );
        return;
    }

    SliderDir dir;

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

    QPointArray a;

    int d = 0;
    switch ( dir ) {
    case SlUp:
        y1 = y1 + w/2;
        d =  (w + 1) / 2 - 1;
        a.setPoints(5, x1,y1, x1,y2, x2,y2, x2,y1, x1+d,y1-d );
        break;
    case SlDown:
        y2 = y2 - w/2;
        d =  (w + 1) / 2 - 1;
        a.setPoints(5, x1,y1, x1,y2, x1+d,y2+d, x2,y2, x2,y1 );
        break;
    case SlLeft:
        d =  (h + 1) / 2 - 1;
        x1 = x1 + h/2;
        a.setPoints(5, x1,y1, x1-d,y1+d, x1,y2, x2,y2, x2,y1);
        break;
    case SlRight:
        d =  (h + 1) / 2 - 1;
        x2 = x2 - h/2;
        a.setPoints(5, x1,y1, x1,y2, x2,y2, x2+d,y1+d, x2,y1 );
        break;
    }


    QBrush oldBrush = p->brush();
    p->setBrush( g.brush( QColorGroup::Button ) );
    p->setPen( NoPen );
    p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
    p->drawPolygon( a );
    p->setBrush( oldBrush );



    if ( dir != SlUp ) {
        p->setPen( c4 );
        p->drawLine( x1, y1, x2, y1 );
        p->setPen( c3 );
        p->drawLine( x1, y1+1, x2, y1+1 );
    }
    if ( dir != SlLeft ) {
        if ( reverse )
            p->setPen( c1 );
        else
            p->setPen( c3 );
        p->drawLine( x1+1, y1+1, x1+1, y2 );
        if ( reverse )
            p->setPen( c0 );
        else
            p->setPen( c4 );
        p->drawLine( x1, y1, x1, y2 );
    }
    if ( dir != SlRight ) {
        if ( reverse )
            p->setPen( c4 );
        else
            p->setPen( c0 );
        p->drawLine( x2, y1, x2, y2 );
        if ( reverse )
            p->setPen( c3 );
        else
            p->setPen( c1 );
        p->drawLine( x2-1, y1+1, x2-1, y2-1 );
    }
    if ( dir != SlDown ) {
        p->setPen( c0 );
        p->drawLine( x1, y2, x2, y2 );
        p->setPen( c1 );
        p->drawLine( x1+1, y2-1, x2-1, y2-1 );
    }

    switch ( dir ) {
        case SlUp:
            if ( reverse )
                p->setPen( c0 );
            else
                p->setPen( c4 );
            p->drawLine( x1, y1, x1+d, y1-d);
            if ( reverse )
                p->setPen( c4 );
            else
                p->setPen( c0 );
            d = w - d - 1;
            p->drawLine( x2, y1, x2-d, y1-d);
            d--;
            if ( reverse )
                p->setPen( c1 );
            else
                p->setPen( c3 );
            p->drawLine( x1+1, y1, x1+1+d, y1-d );
            if ( reverse )
                p->setPen( c3 );
            else
                p->setPen( c1 );
            p->drawLine( x2-1, y1, x2-1-d, y1-d);
            break;
        case SlDown:
            if ( reverse )
                p->setPen( c0 );
            else
                p->setPen( c4 );
            p->drawLine( x1, y2, x1+d, y2+d);
            if ( reverse )
                p->setPen( c4 );
            else
                p->setPen( c0 );
            d = w - d - 1;
            p->drawLine( x2, y2, x2-d, y2+d);
            d--;
            if ( reverse )
                p->setPen( c1 );
            else
                p->setPen( c3 );
            p->drawLine( x1+1, y2, x1+1+d, y2+d );
            if ( reverse )
                p->setPen( c3 );
            else
                p->setPen( c1 );
            p->drawLine( x2-1, y2, x2-1-d, y2+d);
            break;
        case SlLeft:
            p->setPen( c4 );
            p->drawLine( x1, y1, x1-d, y1+d);
            p->setPen( c0 );
            d = h - d - 1;
            p->drawLine( x1, y2, x1-d, y2-d);
            d--;
            p->setPen( c3 );
            p->drawLine( x1, y1+1, x1-d, y1+1+d );
            p->setPen( c1 );
            p->drawLine( x1, y2-1, x1-d, y2-1-d);
            break;
        case SlRight:
            p->setPen( c4 );
            p->drawLine( x2, y1, x2+d, y1+d);
            p->setPen( c0 );
            d = h - d - 1;
            p->drawLine( x2, y2, x2+d, y2-d);
            d--;
            p->setPen( c3 );
            p->drawLine(  x2, y1+1, x2+d, y1+1+d );
            p->setPen( c1 );
            p->drawLine( x2, y2-1, x2+d, y2-1-d);
            break;
    }
#endif
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
    return QMAX( 6, QApplication::globalStrut().width() );
}


/*! \reimp
*/

void QWindowsStyle::drawSplitter( QPainter *p,  int x, int y, int w, int h,
                                  const QColorGroup &g,  Orientation)
{
        qDrawWinPanel( p, x, y, w, h, g );
}



static const int windowsItemFrame               = 2;    // menu item frame width
static const int windowsSepHeight               = 2;    // separator item height
static const int windowsItemHMargin     = 3;    // menu item hor text margin
static const int windowsItemVMargin     = 2;    // menu item ver text margin
static const int windowsArrowHMargin    = 6;    // arrow horizontal margin
static const int windowsTabSpacing      = 12;   // space between text and tab
static const int windowsCheckMarkHMargin        = 2;    // horiz. margins of check mark
static const int windowsRightBorder     = 12;       // right border on windows
static const int windowsCheckMarkWidth = 12;       // checkmarks width on windows

/*! \reimp
*/
void QWindowsStyle::polishPopupMenu( QPopupMenu* p)
{
#ifndef QT_NO_POPUPMENU
    p->setMouseTracking( TRUE );
    if ( !p->testWState( WState_Polished ) )
        p->setCheckable( TRUE );
    p->setLineWidth( 2 );
#endif
}



/*! \reimp
*/
void QWindowsStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
                                   const QColorGroup &g,
                                   bool act, bool dis )
{
    const int markW = w > 7 ? 7 : w;
    const int markH = markW;
    int posX = x + ( w - markW )/2 - 1;
    int posY = y + ( h - markH )/2;

    // Could do with some optimizing/caching...
    QPointArray a( markH*2 );
    int i, xx, yy;
    xx = posX;
    yy = 3 + posY;
    for ( i=0; i<markW/2; i++ ) {
        a.setPoint( 2*i,   xx, yy );
        a.setPoint( 2*i+1, xx, yy+2 );
        xx++; yy++;
    }
    yy -= 2;
    for ( ; i<markH; i++ ) {
        a.setPoint( 2*i,   xx, yy );
        a.setPoint( 2*i+1, xx, yy+2 );
        xx++; yy--;
    }
    if ( dis && !act ) {
        int pnt;
        p->setPen( g.highlightedText() );
        QPoint offset(1,1);
        for ( pnt = 0; pnt < (int)a.size(); pnt++ )
            a[pnt] += offset;
        p->drawLineSegments( a );
        for ( pnt = 0; pnt < (int)a.size(); pnt++ )
            a[pnt] -= offset;
    }
    p->setPen( g.text() );
    p->drawLineSegments( a );
}


/*! \reimp
*/
int QWindowsStyle::extraPopupMenuItemWidth( bool checkable, int maxpmw,
                                            QMenuItem* mi,
                                            const QFontMetrics& /*fm*/ ) const
{
#ifndef QT_NO_POPUPMENU
    int w = 2*windowsItemHMargin + 2*windowsItemFrame; // a little bit of border can never harm

    if ( mi->isSeparator() )
        return 10; // arbitrary
    else if ( mi->pixmap() )
        w += mi->pixmap()->width();     // pixmap only

    if ( !mi->text().isNull() ) {
        if ( mi->text().find('\t') >= 0 )       // string contains tab
            w += windowsTabSpacing;
    }

    if ( maxpmw ) { // we have iconsets
        w += maxpmw;
        w += 6; // add a little extra border around the iconset
    }

    if ( checkable && maxpmw < windowsCheckMarkWidth ) {
        w += windowsCheckMarkWidth - maxpmw; // space for the checkmarks
    }

    if ( maxpmw > 0 || checkable ) // we have a check-column ( iconsets or checkmarks)
        w += windowsCheckMarkHMargin; // add space to separate the columns

    w += windowsRightBorder; // windows has a strange wide border on the right side

    return w;
#endif
}

/*! \reimp
*/
int QWindowsStyle::popupMenuItemHeight( bool /*checkable*/, QMenuItem* mi,
                                        const QFontMetrics& fm ) const
{
#ifndef QT_NO_POPUPMENU
    int h = 0;
    if ( mi->isSeparator() )                    // separator height
        h = windowsSepHeight;
    else if ( mi->pixmap() )            // pixmap height
        h = mi->pixmap()->height() + 2*windowsItemFrame;
    else                                        // text height
        h = fm.height() + 2*windowsItemVMargin + 2*windowsItemFrame;

    if ( !mi->isSeparator() && mi->iconSet() != 0 ) {
        h = QMAX( h, mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height() + 2*windowsItemFrame );
    }
    if ( mi->custom() )
        h = QMAX( h, mi->custom()->sizeHint().height() + 2*windowsItemVMargin + 2*windowsItemFrame );
    return h;
#endif
}

/*! \reimp
*/
void QWindowsStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                       int tab, QMenuItem* mi,
                                       const QPalette& pal, bool act,
                                       bool enabled,
                                       int x, int y, int w, int h)
{
#ifndef QT_NO_POPUPMENU
    const QColorGroup & g = pal.active();
    bool dis = !enabled;
    QColorGroup itemg = dis ? pal.disabled() : pal.active();

    if ( checkable ) {
#if defined(Q_WS_WIN)
        if ( qWinVersion() == Qt::WV_2000 ||
	     qWinVersion() == Qt::WV_98 ||
	     qWinVersion() == Qt::WV_XP )

            maxpmw = QMAX( maxpmw, 16 );
#endif
        maxpmw = QMAX( maxpmw, 12 ); // space for the checkmarks
    }

    int checkcol = maxpmw;

    if ( mi && mi->isSeparator() ) {                    // draw separator
        p->setPen( g.dark() );
        p->drawLine( x, y, x+w, y );
        p->setPen( g.light() );
        p->drawLine( x, y+1, x+w, y+1 );
        return;
    }

    QBrush fill = act? g.brush( QColorGroup::Highlight ) :
                            g.brush( QColorGroup::Button );
    p->fillRect( x, y, w, h, fill);

    if ( !mi )
        return;

    bool reverse = QApplication::reverseLayout();

    int xpos = x;
    if ( reverse )
        xpos += w - checkcol;
    if ( mi->isChecked() ) {
        if ( act && !dis ) {
            qDrawShadePanel( p, xpos, y, checkcol, h,
                             g, TRUE, 1, &g.brush( QColorGroup::Button ) );
        } else {
            qDrawShadePanel( p, xpos, y, checkcol, h,
                             g, TRUE, 1, &g.brush( QColorGroup::Midlight ) );
        }
    } else if ( !act ) {
        p->fillRect(xpos, y, checkcol , h,
                    g.brush( QColorGroup::Button ));
    }

    if ( mi->iconSet() ) {              // draw iconset
        QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
        if (act && !dis )
            mode = QIconSet::Active;
	QPixmap pixmap;
	if ( checkable && mi->isChecked() )
	    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
	else
	    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        if ( act && !dis ) {
            if ( !mi->isChecked() )
                qDrawShadePanel( p, xpos, y, checkcol, h, g, FALSE, 1,
                                 &g.brush( QColorGroup::Button ) );
        }
        QRect cr( xpos, y, checkcol, h );
        QRect pmr( 0, 0, pixw, pixh );
        pmr.moveCenter( cr.center() );
        p->setPen( itemg.text() );
        p->drawPixmap( pmr.topLeft(), pixmap );

        QBrush fill = act? g.brush( QColorGroup::Highlight ) :
                              g.brush( QColorGroup::Button );
        int xp;
        if ( reverse )
            xp = x;
        else
            xp = xpos + checkcol + 1;
        p->fillRect( xp, y, w - checkcol - 1, h, fill);
    } else  if ( checkable ) {  // just "checking"...
        int mw = checkcol + windowsItemFrame;
        int mh = h - 2*windowsItemFrame;
        if ( mi->isChecked() ) {
            int xp = xpos;
            if( reverse )
                xp -= windowsItemFrame;
            else
                xp += windowsItemFrame;
            drawCheckMark( p, xp, y+windowsItemFrame, mw, mh, itemg, act, dis );
        }
    }

    p->setPen( act ? g.highlightedText() : g.buttonText() );

    QColor discol;
    if ( dis ) {
        discol = itemg.text();
        p->setPen( discol );
    }

    int xm = windowsItemFrame + checkcol + windowsItemHMargin;
    if ( reverse )
        xpos = windowsItemFrame + tab;
    else
        xpos += xm;

    if ( mi->custom() ) {
        int m = windowsItemVMargin;
        p->save();
        if ( dis && !act ) {
            p->setPen( g.light() );
            mi->custom()->paint( p, itemg, act, enabled,
                                 xpos+1, y+m+1, w-xm-tab+1, h-2*m );
            p->setPen( discol );
        }
        mi->custom()->paint( p, itemg, act, enabled,
                             x+xm, y+m, w-xm-tab+1, h-2*m );
        p->restore();
    }
    QString s = mi->text();
    if ( !s.isNull() ) {                        // draw text
        int t = s.find( '\t' );
        int m = windowsItemVMargin;
        const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
        if ( t >= 0 ) {                         // draw tab text
            int xp;
            if( reverse )
                xp = x + windowsRightBorder+windowsItemHMargin+windowsItemFrame - 1;
            else
                xp = x + w - tab - windowsRightBorder-windowsItemHMargin-windowsItemFrame+1;
            if ( dis && !act ) {
                p->setPen( g.light() );
                p->drawText( xp, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
                p->setPen( discol );
            }
            p->drawText( xp, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
            s = s.left( t );
        }
        if ( dis && !act ) {
            p->setPen( g.light() );
            p->drawText( xpos+1, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t );
            p->setPen( discol );
        }
        p->drawText( xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
    } else if ( mi->pixmap() ) {                        // draw pixmap
        QPixmap *pixmap = mi->pixmap();
        if ( pixmap->depth() == 1 )
            p->setBackgroundMode( OpaqueMode );
        p->drawPixmap( xpos, y+windowsItemFrame, *pixmap );
        if ( pixmap->depth() == 1 )
            p->setBackgroundMode( TransparentMode );
    }
    if ( mi->popup() ) {                        // draw sub menu arrow
        int dim = (h-2*windowsItemFrame) / 2;
        ArrowType arrow;
        if ( reverse ) {
            arrow = LeftArrow;
            xpos = x + windowsArrowHMargin + windowsItemFrame;
        } else {
            arrow = RightArrow;
            xpos = x+w - windowsArrowHMargin - windowsItemFrame - dim;
        }
        if ( act ) {
            if ( !dis )
                discol = white;
            QColorGroup g2( discol, g.highlight(),
                            white, white,
                            dis ? discol : white,
                            discol, white );
            drawArrow( p, arrow, FALSE,
                               xpos,  y+h/2-dim/2,
                               dim, dim, g2, TRUE );
        } else {
            drawArrow( p, arrow, FALSE,
                               xpos,  y+h/2-dim/2,
                               dim, dim, g, mi->isEnabled() );
        }
    }
#endif
}

/*! \reimp */
void QWindowsStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
                                     QMenuItem* mi, QColorGroup& g,
                                     bool active, bool down, bool hasFocus )
{
    QRect r( x, y, w, h );
    p->fillRect( r, g.brush( QColorGroup::Button ) );

    if ( active || hasFocus ) {
        QBrush b = g.brush( QColorGroup::Button );
        if ( active && down )
            p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
        if ( active && hasFocus )
            qDrawShadeRect( p, x, y, w, h, g, active && down, 1, 0, &b );
        if ( active && down ) {
            r.setRect( x+2, y+2, w-2, h-2 );
            p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
        }
    }
    QCommonStyle::drawMenuBarItem( p, r.left(), r.top(), r.width(), r.height(),
                                   mi, g, active, down, hasFocus );
}

/*!
  \reimpl
 */
int QWindowsStyle::menuBarFrameWidth() const
{
    return 0;
}

/*!
 \reimp
*/
int QWindowsStyle::spinBoxFrameWidth() const
{
    return 2;
}

/*!
 \reimp
 */
int QWindowsStyle::progressChunkWidth() const
{
    return 9;
}

/*!
 \reimp
*/
void QWindowsStyle::drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    qDrawShadePanel( p, x, y, w, h, g, TRUE, 1 );
}

/*!
 \reimp
 */
void QWindowsStyle::drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    p->fillRect( x + 1, y + 1, w - 2, h - 2,
	g.brush( QColorGroup::Highlight ) );
}

/*!
 \reimp
*/
void QWindowsStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool on, bool down, bool enabled, bool autoRaised,
		 const QBrush *fill )
{
    QBrush onfill( g.light(), Dense4Pattern );

    const QBrush *thefill = fill;
#if defined(Q_WS_WIN)
    if ( !thefill && on && enabled &&
	( qWinVersion() == WV_2000 ||
	  qWinVersion() == WV_98 ||
	  qWinVersion() == WV_XP ) )
	thefill = &onfill;
#else
    Q_UNUSED(enabled)
    if ( !thefill && on )
	thefill = &onfill;
#endif

    if ( !autoRaised )
	QCommonStyle::drawToolButton( p, x, y, w, h, g, on || down, thefill );
    else
	drawPanel( p, x, y, w, h, g, on || down, 1, thefill );

#if defined(Q_WS_WIN)
    if ( on && enabled &&
	( qWinVersion() == WV_2000 ||
	  qWinVersion() == WV_98 ||
	  qWinVersion() == WV_XP ) ) {
        p->setPen( g.button() );
        p->drawRect( x + 1, y + 1, w - 2, h - 2 );
    }
#endif
}

/*!
 \reimp
*/
void QWindowsStyle::drawDropDownButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool down, bool /*enabled*/, bool autoRaised, const QBrush *fill )
{
    if ( !autoRaised )
	QCommonStyle::drawToolButton( p, x, y, w, h, g, down, fill );
    else
	drawPanel( p, x, y, w, h, g, down, 1, fill );

#if defined(Q_WS_WIN)
    if ( down &&
	( qWinVersion() == WV_2000 ||
	  qWinVersion() == WV_98 ||
	  qWinVersion() == WV_XP ) ) {
        p->setPen( g.button() );
        p->drawRect( x + 1, y + 1, w - 2, h - 2 );
    }
#endif
}

/*!
  \reimp
*/
bool QWindowsStyle::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() != QEvent::Enter && e->type() != QEvent::Leave )
	return QCommonStyle::eventFilter( o, e );
    if ( !o || !o->inherits( "QToolButton" ) )
	return QCommonStyle::eventFilter( o, e );

    switch ( e->type() )
    {
    case QEvent::Enter:
	d->hotWidget = (QWidget*)o;
	d->hotWidget->update();
	break;
    case QEvent::Leave:
	{
	    QWidget *old = d->hotWidget;
	    d->hotWidget = 0;
	    if ( old )
		old->update();
	}
	break;
    default:
	break;
    }

    return QCommonStyle::eventFilter( o, e );
}

/*!
 \reimp
 */
void QWindowsStyle::drawListViewItemBranch( QPainter *p, int y, int w, int h, const QColorGroup &cg, QListViewItem *item )
{
#ifndef QT_NO_LISTVIEW
    QListViewItem *child = item->firstChild();
    int linetop = 0, linebot = 0;
    // each branch needs at most two lines, ie. four end points
    int dotoffset = (item->itemPos() + item->height() - y) %2;
    QPointArray dotlines( item->childCount() * 4 );
    int c = 0;

    // skip the stuff above the exposed rectangle
    while ( child && y + child->height() <= 0 ) {
	y += child->totalHeight();
	child = child->nextSibling();
    }

    int bx = w / 2;

    // paint stuff in the magical area
    while ( child && y < h ) {
	linebot = y + child->height()/2;
	if ( (child->isExpandable() || child->childCount()) &&
	     (child->height() > 0) ) {
	    // needs a box
	    p->setPen( cg.text() );
	    p->drawRect( bx-4, linebot-4, 9, 9 );
	    // plus or minus
	    p->drawLine( bx - 2, linebot, bx + 2, linebot );
	    if ( !child->isOpen() )
		p->drawLine( bx, linebot - 2, bx, linebot + 2 );
	    // dotlinery
	    dotlines[c++] = QPoint( bx, linetop );
	    dotlines[c++] = QPoint( bx, linebot - 5 );
	    dotlines[c++] = QPoint( bx + 5, linebot );
	    dotlines[c++] = QPoint( w, linebot );
	    linetop = linebot + 5;
	} else {
	    // just dotlinery
	    dotlines[c++] = QPoint( bx+1, linebot );
	    dotlines[c++] = QPoint( w, linebot );
	}

	y += child->totalHeight();
	child = child->nextSibling();
    }

    if ( child ) // there's a child, so move linebot to edge of rectangle
	linebot = h;

    if ( linetop < linebot ) {
	dotlines[c++] = QPoint( bx, linetop );
	dotlines[c++] = QPoint( bx, linebot );
    }

    p->setPen( cg.dark() );

    static QBitmap *verticalLine = 0, *horizontalLine = 0;
    static QCleanupHandler<QBitmap> qlv_cleanup_bitmap;
    if ( !verticalLine ) {
	// make 128*1 and 1*128 bitmaps that can be used for
	// drawing the right sort of lines.
	verticalLine = new QBitmap( 1, 129, TRUE );
	horizontalLine = new QBitmap( 128, 1, TRUE );
	QPointArray a( 64 );
	QPainter p;
	p.begin( verticalLine );
	int i;
	for( i=0; i<64; i++ )
	    a.setPoint( i, 0, i*2+1 );
	p.setPen( color1 );
	p.drawPoints( a );
	p.end();
	QApplication::flushX();
	verticalLine->setMask( *verticalLine );
	p.begin( horizontalLine );
	for( i=0; i<64; i++ )
	    a.setPoint( i, i*2+1, 0 );
	p.setPen( color1 );
	p.drawPoints( a );
	p.end();
	QApplication::flushX();
	horizontalLine->setMask( *horizontalLine );
	qlv_cleanup_bitmap.add( verticalLine );
	qlv_cleanup_bitmap.add( horizontalLine );
    }

    int line; // index into dotlines
    for( line = 0; line < c; line += 2 ) {
	// assumptions here: lines are horizontal or vertical.
	// lines always start with the numerically lowest
	// coordinate.

	// point ... relevant coordinate of current point
	// end ..... same coordinate of the end of the current line
	// other ... the other coordinate of the current point/line
	if ( dotlines[line].y() == dotlines[line+1].y() ) {
	    int end = dotlines[line+1].x();
	    int point = dotlines[line].x();
	    int other = dotlines[line].y();
	    while( point < end ) {
		int i = 128;
		if ( i+point > end )
		    i = end-point;
		p->drawPixmap( point, other, *horizontalLine,
			       0, 0, i, 1 );
		point += i;
	    }
	} else {
	    int end = dotlines[line+1].y();
	    int point = dotlines[line].y();
	    int other = dotlines[line].x();
	    int pixmapoffset = ((point & 1) != dotoffset ) ? 1 : 0;
	    while( point < end ) {
		int i = 128;
		if ( i+point > end )
		    i = end-point;
		p->drawPixmap( other, point, *verticalLine,
			       0, pixmapoffset, 1, i );
		point += i;
	    }
	}
    }
#endif //QT_NO_LISTVIEW
}

static const char * const qt_close_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"..##....##..",
"...##..##...",
"....####....",
".....##.....",
"....####....",
"...##..##...",
"..##....##..",
"............",
"............",
"............"};

static const char * const qt_maximize_xpm[]={
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".##########.",
".##########.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".##########.",
"............"};


static const char * const qt_minimize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"...######...",
"...######...",
"............",
"............",
"............"};

#if 0 // ### not used???
static const char * const qt_normalize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"...#######..",
"...#######..",
"...#.....#..",
".#######.#..",
".#######.#..",
".#.....#.#..",
".#.....###..",
".#.....#....",
".#.....#....",
".#######....",
"............"};
#endif

static const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"...#######..",
"...#######..",
"...#.....#..",
".#######.#..",
".#######.#..",
".#.....#.#..",
".#.....###..",
".#.....#....",
".#.....#....",
".#######....",
"............"};


static const char * const qt_shade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
".....#......",
"....###.....",
"...#####....",
"..#######...",
"............",
"............",
"............"};

static const char * const qt_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"..#######...",
"...#####....",
"....###.....",
".....#......",
"............",
"............",
"............",
"............"};


/*!
 \reimp
 */
QPixmap QWindowsStyle::titleBarPixmap( const QTitleBar *, TitleControl ctrl)
{
    switch(ctrl) {
    case TitleLabel:
    case TitleNone:
    case TitleSysMenu:
	break;
    case TitleShadeButton:
	return QPixmap((const char **)qt_shade_xpm);
    case TitleUnshadeButton:
	return QPixmap((const char **)qt_unshade_xpm);
    case TitleNormalButton:
	return QPixmap((const char **)qt_normalizeup_xpm);
    case TitleMinButton:
	return QPixmap((const char **)qt_minimize_xpm);
    case TitleMaxButton:
	return QPixmap((const char **)qt_maximize_xpm);
    case TitleCloseButton:
	return QPixmap((const char **)qt_close_xpm);
    }
    return QPixmap();
}

void QWindowsStyle::drawComplexControl( ComplexControl ctrl, QPainter * p,
					const QWidget * w,
					const QRect & r,
					const QColorGroup & cg,
					CFlags flags,
					SCFlags sub,
					SCFlags subActive, void * data ) const
{
    switch (ctrl) {
    case CC_ScrollBar: {
	if (! w)
	    break;

	QScrollBar *scrollbar = (QScrollBar *) w;
	QRect addline, subline, addpage, subpage, slider;
	bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	subline = querySubControlMetrics(ctrl, w, SC_ScrollBarSubLine, data);
	addline = querySubControlMetrics(ctrl, w, SC_ScrollBarAddLine, data);
	subpage = querySubControlMetrics(ctrl, w, SC_ScrollBarSubPage, data);
	addpage = querySubControlMetrics(ctrl, w, SC_ScrollBarAddPage, data);
	slider  = querySubControlMetrics(ctrl, w, SC_ScrollBarSlider, data);

	if (sub & SC_ScrollBarSubLine) {
	    drawPrimitive(PO_ButtonBevel, p, subline, cg,
			  PStyle_Enabled | ((subActive == SC_ScrollBarSubLine) ?
					    PStyle_Sunken :
					    PStyle_Default));
	    drawPrimitive(PO_ArrowUp, p, subline, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarSubLine) ?
			   PStyle_Sunken :
			   PStyle_Default));
	}

	if (sub & SC_ScrollBarAddLine) {
	    drawPrimitive(PO_ButtonBevel, p, addline, cg,
			  PStyle_Enabled | ((subActive == SC_ScrollBarAddLine) ?
					    PStyle_Sunken :
					    PStyle_Default));
	    drawPrimitive(PO_ArrowDown, p, addline, cg,
			  ((maxedOut) ? PStyle_Default : PStyle_Enabled) |
			  ((subActive == SC_ScrollBarAddLine) ?
			   PStyle_Sunken :
			   PStyle_Default));
	}

	QBrush br = (cg.brush(QColorGroup::Light).pixmap() ?
		     cg.brush(QColorGroup::Light) :
		     QBrush(cg.light(), Dense4Pattern));

	p->setBrush(br);
	p->setPen(NoPen);
	p->setBackgroundMode(OpaqueMode);

	if (maxedOut) {
	    p->drawRect(r);
	} else {
	    if (((sub & SC_ScrollBarSubPage) && subActive == SC_ScrollBarSubPage) ||
		((sub & SC_ScrollBarAddPage) && subActive == SC_ScrollBarAddPage)) {
		QBrush b = p->brush();
		QColor c = p->backgroundColor();
		p->setBackgroundColor( cg.dark() );
		p->setBrush( QBrush(cg.shadow(), Dense4Pattern) );
		p->drawRect( subActive == SC_ScrollBarAddPage ? addpage : subpage );
		p->setBackgroundColor( c );
		p->setBrush( b );
	    }

	    if ( sub & SC_ScrollBarSubPage && subActive != SC_ScrollBarSubPage)
		p->drawRect( subpage );
	    if ( sub & SC_ScrollBarAddPage && subActive != SC_ScrollBarAddPage)
		p->drawRect( addpage );

	    if ( sub & SC_ScrollBarSlider ) {
		if ( !maxedOut ) {
		    QPoint bo = p->brushOrigin();
		    p->setBrushOrigin(slider.topLeft());
		    drawPrimitive(PO_ButtonBevel, p, slider, cg, FALSE);
		    p->setBrushOrigin(bo);
		}
	    }
	}

	// ### perhaps this should not be able to accept focus if maxedOut?
	if ( scrollbar->hasFocus() && (sub & SC_ScrollBarSlider) ) {
	    QRect fr(slider.x() + 2, slider.y() + 2,
		     slider.width() - 5, slider.height() - 5);
	    drawPrimitive(PO_FocusRect, p, fr, cg, PStyle_Default);
	}

	break; }

    case CC_SpinWidget: {
	if ( sub != SC_None ) {
	    // draw only specified component
	    drawSubControl( sub, p, w, r, cg, flags, subActive, data );
	} else {
	    // draw the whole thing
	    drawSubControl( SC_SpinWidgetUp, p, w, r, cg, flags,
			    subActive, data );
	    drawSubControl( SC_SpinWidgetDown, p, w, r, cg, flags,
			    subActive, data );
	    drawSubControl( SC_SpinWidgetFrame, p, w, r, cg, flags,
			    subActive, data );
	}
	break; }

       case CC_ComboBox: {
            if ( sub != SC_None ) {
                drawSubControl( sub, p, w, r, cg, flags, subActive, data );
            } else {
                drawSubControl( SC_ComboBoxButton, p, w, r, cg, flags,
                                subActive, data );
                drawSubControl( SC_ComboBoxArrow, p, w, r, cg, flags,
                                subActive, data );
                drawSubControl( SC_ComboBoxEditField, p, w, r, cg, flags,
                                subActive, data );
                drawSubControl( SC_ComboBoxFocusRect, p, w, r, cg, flags,
                                subActive, data );
            }
            break; }

    default:
	QCommonStyle::drawComplexControl( ctrl, p, w, r, cg, flags, sub,
					  subActive, data );
	break;
    }
}

void QWindowsStyle::drawSubControl( SCFlags subCtrl, QPainter * p,
				    const QWidget * w,
				    const QRect & r, const QColorGroup & cg,
				    CFlags flags,
				    SCFlags subActive, void * data ) const
{
    switch( subCtrl ) {
    case SC_SpinWidgetUp: {
	QSpinWidget * sw = (QSpinWidget *) w;
	PFlags flags = PStyle_Default;
	PrimitiveOperation op = PO_SpinWidgetUp;

	flags |= PStyle_Enabled;
	if (subActive == subCtrl) {
	    flags |= PStyle_On;
	    flags |= PStyle_Sunken;
	}
	if ( sw->buttonSymbols() == QSpinWidget::PlusMinus )
	    op = PO_SpinWidgetPlus;

	drawPrimitive(PO_ButtonBevel, p, r, cg, flags);
	drawPrimitive(op, p, r, cg, flags);
    	break; }

    case SC_SpinWidgetDown: {
	QSpinWidget * sw = (QSpinWidget *) w;
	PFlags flags = PStyle_Default;
	PrimitiveOperation op = PO_SpinWidgetDown;

	flags |= PStyle_Enabled;
	if (subActive == subCtrl) {
	    flags |= PStyle_On;
	    flags |= PStyle_Sunken;
	}
	if ( sw->buttonSymbols() == QSpinWidget::PlusMinus )
	    op = PO_SpinWidgetMinus;

	drawPrimitive(PO_ButtonBevel, p, r, cg, flags);
	drawPrimitive(op, p, r, cg, flags);
	break; }

    case SC_SpinWidgetFrame:
	qDrawWinPanel( p, r, cg, TRUE ); //cstyle == Sunken );
	break;

    case SC_ComboBoxButton: {
	qDrawWinPanel( p, r, cg, TRUE, w->isEnabled() ?
		       &cg.brush( QColorGroup::Base ):
		       &cg.brush( QColorGroup::Background ) );
	break; }
	
    case SC_ComboBoxArrow: {
	int xpos = r.x();
	if( !QApplication::reverseLayout() )
	    xpos += r.width() - 2 - 16;
	QRect ar = querySubControlMetrics( CC_ComboBox, w,
					   SC_ComboBoxArrow );
	qDrawWinPanel( p, ar, cg, FALSE, // SUNKEN
		       &cg.brush( QColorGroup::Button ) );

	QRect ra( ar.x()+2, ar.y()+2, ar.width()-4, ar.width()-4 );
	if ( w->isEnabled() )
	    flags |= PStyle_Enabled;
	drawPrimitive( PO_ArrowDown, p, ra, cg, flags );
	break; }
	
    case SC_ComboBoxEditField: {
	QComboBox * cb = (QComboBox *) w;
	QRect re = querySubControlMetrics( CC_ComboBox, w, 
					   SC_ComboBoxEditField );
	if ( cb->hasFocus() && !cb->editable() )
	    p->fillRect( re.x(), re.y(), re.width(), re.height(),
			 cg.brush( QColorGroup::Highlight ) );	
        break; }
	
    case SC_ComboBoxFocusRect: {
	QComboBox * cb = (QComboBox *) w;
	if ( cb->hasFocus() && !cb->editable() ) {
	    QRect re = querySubControlMetrics( CC_ComboBox, w, 
					       SC_ComboBoxFocusRect );
	    drawPrimitive(PO_FocusRect, p, re, cg );
	}
	break;}
	
    default:
	break;
    }
}
#endif


