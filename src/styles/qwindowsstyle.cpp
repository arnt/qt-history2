/****************************************************************************
** $Id: $
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
#include "qlistbox.h"
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include "qslider.h"
#include "qtabwidget.h"
#include "qtabbar.h"
#include "qlistview.h"
#include "qbitmap.h"
#include "qcleanuphandler.h"

#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#include <limits.h>


static const int windowsItemFrame		=  2; // menu item frame width
static const int windowsSepHeight		=  2; // separator item height
static const int windowsItemHMargin		=  3; // menu item hor text margin
static const int windowsItemVMargin		=  2; // menu item ver text margin
static const int windowsArrowHMargin		=  6; // arrow horizontal margin
static const int windowsTabSpacing		= 12; // space between text and tab
static const int windowsCheckMarkHMargin	=  2; // horiz. margins of check mark
static const int windowsRightBorder		= 12; // right border on windows
static const int windowsCheckMarkWidth		= 12; // checkmarks width on windows


static bool use2000style = TRUE;


/*!
  \class QWindowsStyle qwindowsstyle.h
  \brief The QWindowsStyle class provides a Microsoft Windows-like look and feel.
  \ingroup appearance

  This style is Qt's default GUI style on Windows.
*/

/*!
    Constructs a QWindowsStyle
*/
QWindowsStyle::QWindowsStyle() : QCommonStyle()
{
#if defined(Q_OS_WIN32)
    if (qWinVersion() == Qt::WV_2000 ||
	qWinVersion() == Qt::WV_98 ||
	qWinVersion() == Qt::WV_XP)
	use2000style = TRUE;
    else
	use2000style = FALSE;
#else
    use2000style = TRUE;
#endif
}

/*! \reimp */
QWindowsStyle::~QWindowsStyle()
{
}

/*! \reimp */
void QWindowsStyle::drawPrimitive( PrimitiveElement pe,
				   QPainter *p,
				   const QRect &r,
				   const QColorGroup &cg,
				   SFlags flags,
				   const QStyleOption& opt ) const
{
    switch (pe) {
    case PE_ButtonCommand:
	{
	    QBrush fill;

	    if (! (flags & Style_Down) && (flags & Style_On))
		fill = QBrush(cg.light(), Dense4Pattern);
	    else
		fill = cg.brush(QColorGroup::Button);

	    if (flags & Style_ButtonDefault && flags & Style_Down) {
		p->setPen(cg.dark());
		p->setBrush(fill);
		p->drawRect(r);
	    } else if (flags & (Style_Raised | Style_Down | Style_On | Style_Sunken))
		qDrawWinButton(p, r, cg, flags & (Style_Sunken | Style_Down |
						  Style_On), &fill);
	    else
		p->fillRect(r, fill);
	    break;
	}

    case PE_ButtonBevel:
    case PE_HeaderSection:
	{
	    QBrush fill;

	    if (! (flags & Style_Down) && (flags & Style_On))
		fill = QBrush(cg.light(), Dense4Pattern);
	    else
		fill = cg.brush(QColorGroup::Button);

	    if (flags & (Style_Raised | Style_Down | Style_On | Style_Sunken))
		qDrawWinButton(p, r, cg, flags & (Style_Down | Style_On), &fill);
	    else
		p->fillRect(r, fill);
	    break;
	}

    case PE_ButtonDefault:
	p->setPen(cg.shadow());
	p->drawRect(r);
	break;

    case PE_ButtonTool:
	{
	    QBrush fill;
	    bool stippled = FALSE;

	    if (! (flags & (Style_Down | Style_MouseOver)) &&
		(flags & Style_On) &&
		(flags & Style_Enabled) &&
		use2000style) {
		fill = QBrush(cg.light(), Dense4Pattern);
		stippled = TRUE;
	    } else
		fill = cg.brush(QColorGroup::Button);

	    if (flags & (Style_Raised | Style_Down | Style_On)) {
		if (flags & Style_AutoRaise) {
		    qDrawShadePanel(p, r, cg, flags & (Style_Down | Style_On),
				    1, &fill);

		    if (stippled) {
			p->setPen(cg.button());
			p->drawRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
		    }
		} else
		    qDrawWinButton(p, r, cg, flags & (Style_Down | Style_On),
				   &fill);
	    } else
		p->fillRect(r, fill);

	    break;
	}

    case PE_FocusRect:
	if (opt.isDefault())
	    p->drawWinFocusRect(r);
	else
	    p->drawWinFocusRect(r, opt.color());
	break;

    case PE_Indicator:
	{
#ifndef QT_NO_BUTTON
	    QBrush fill;
	    if (flags & Style_NoChange) {
		QBrush b = p->brush();
		QColor c = p->backgroundColor();
		p->setBackgroundMode( TransparentMode );
		p->setBackgroundColor( green );
		fill = QBrush(cg.base(), Dense4Pattern);
		p->setBackgroundColor( c );
		p->setBrush( b );
	    } else if (flags & Style_Down)
		fill = cg.brush( QColorGroup::Button );
	    else if (flags & Style_Enabled)
		fill = cg.brush( QColorGroup::Base );
	    else
		fill = cg.brush( QColorGroup::Background );

	    qDrawWinPanel( p, r, cg, TRUE, &fill );
	    if (! (flags & Style_Off)) {
		QPointArray a( 7*2 );
		int i, xx, yy;
		xx = r.x() + 3;
		yy = r.y() + 5;

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

		if (flags & Style_NoChange)
		    p->setPen( cg.dark() );
		else
		    p->setPen( cg.text() );

		p->drawLineSegments( a );
	    }
#endif
	    break;
	}

    case PE_ExclusiveIndicator:
	{
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
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

	    // make sure the indicator is square
	    QRect ir = r;

	    if (r.width() < r.height()) {
		ir.setTop(r.top() + (r.height() - r.width()) / 2);
		ir.setHeight(r.width());
	    } else if (r.height() < r.width()) {
		ir.setLeft(r.left() + (r.width() - r.height()) / 2);
		ir.setWidth(r.height());
	    }

	    p->eraseRect(ir);
	    bool reverse = QApplication::reverseLayout();
	    bool down = flags & Style_Down;
	    bool enabled = flags & Style_Enabled;
	    bool on = flags & Style_On;
	    QPointArray a;
	    if( reverse )
		a.setPoints( QCOORDARRLEN(rpts1), rpts1 );
	    else
		a.setPoints( QCOORDARRLEN(pts1), pts1 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.dark() );
	    p->drawPolyline( a );
	    if( reverse )
		a.setPoints( QCOORDARRLEN(rpts2), rpts2 );
	    else
		a.setPoints( QCOORDARRLEN(pts2), pts2 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.shadow() );
	    p->drawPolyline( a );
	    if( reverse )
		a.setPoints( QCOORDARRLEN(rpts3), rpts3 );
	    else
		a.setPoints( QCOORDARRLEN(pts3), pts3 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.midlight() );
	    p->drawPolyline( a );
	    if( reverse )
		a.setPoints( QCOORDARRLEN(rpts4), rpts4 );
	    else
		a.setPoints( QCOORDARRLEN(pts4), pts4 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.light() );
	    p->drawPolyline( a );
	    if( reverse )
		a.setPoints( QCOORDARRLEN(rpts5), rpts5 );
	    else
		a.setPoints( QCOORDARRLEN(pts5), pts5 );
	    a.translate( ir.x(), ir.y() );
	    QColor fillColor = ( down || !enabled ) ? cg.button() : cg.base();
	    p->setPen( fillColor );
	    p->setBrush( fillColor  ) ;
	    p->drawPolygon( a );
	    if ( on ) {
		p->setPen( NoPen );
		p->setBrush( cg.text() );
		p->drawRect( ir.x() + 5, ir.y() + 4, 2, 4 );
		p->drawRect( ir.x() + 4, ir.y() + 5, 4, 2 );
	    }
	    break;
	}

    case PE_Panel:
    case PE_PanelPopup:
	{
	    int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth)
			: opt.lineWidth();

	    if (lw == 2)
		qDrawWinPanel(p, r, cg, flags & Style_Sunken);
	    else
		QCommonStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	    break;
	}

    case PE_Splitter:
    case PE_DockWindowResizeHandle:
	qDrawWinPanel( p, r.x(), r.y(), r.width(), r.height(), cg );
	break;

    case PE_ScrollBarSubLine:
	if (use2000style) {
	    if (flags & Style_Down) {
		p->setPen( cg.dark() );
		p->setBrush( cg.brush( QColorGroup::Button ) );
		p->drawRect( r );
	    } else
		drawPrimitive(PE_ButtonBevel, p, r, cg, flags | Style_Raised);
	} else
	    drawPrimitive(PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
			  ((flags & Style_Down) ? Style_Down : Style_Raised));

	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
		      p, r, cg, flags);
	break;

    case PE_ScrollBarAddLine:
	if (use2000style) {
	    if (flags & Style_Down) {
		p->setPen( cg.dark() );
		p->setBrush( cg.brush( QColorGroup::Button ) );
		p->drawRect( r );
	    } else
		drawPrimitive(PE_ButtonBevel, p, r, cg, flags | Style_Raised);
	} else
	    drawPrimitive(PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
			  ((flags & Style_Down) ? Style_Down : Style_Raised));

	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
		      p, r, cg, flags);
	break;

    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:
	{
	    QBrush br;
	    QColor c = p->backgroundColor();

	    p->setPen(NoPen);
	    p->setBackgroundMode(OpaqueMode);

	    if (flags & Style_Down) {
		br = QBrush(cg.shadow(), Dense4Pattern);
		p->setBackgroundColor( cg.dark() );
		p->setBrush( QBrush(cg.shadow(), Dense4Pattern) );
	    } else {
		br = (cg.brush(QColorGroup::Light).pixmap() ?
		      cg.brush(QColorGroup::Light) :
		      QBrush(cg.light(), Dense4Pattern));
		p->setBrush(br);
	    }

	    p->drawRect(r);
	    p->setBackgroundColor(c);
	    break;
	}

    case PE_ScrollBarSlider:
	if (! (flags & Style_Enabled)) {
	    QBrush br = (cg.brush(QColorGroup::Light).pixmap() ?
			 cg.brush(QColorGroup::Light) :
			 QBrush(cg.light(), Dense4Pattern));
	    p->setPen(NoPen);
	    p->setBrush(br);
	    p->setBackgroundMode(OpaqueMode);
	    p->drawRect(r);
	} else
	    drawPrimitive(PE_ButtonBevel, p, r, cg, Style_Enabled | Style_Raised);
	break;

    default:
	if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft) {
	    QPointArray a;

	    switch ( pe ) {
	    case PE_ArrowUp:
		a.setPoints( 7, -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2 );
		break;

	    case PE_ArrowDown:
		a.setPoints( 7, -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1 );
		break;

	    case PE_ArrowRight:
		a.setPoints( 7, -2,-3, -2,3, -1,-2, -1,2, 0,-1, 0,1, 1,0 );
		break;

	    case PE_ArrowLeft:
		a.setPoints( 7, 0,-3, 0,3, -1,-2, -1,2, -2,-1, -2,1, -3,0 );
		break;

	    default:
		break;
	    }

	    if (a.isNull())
		return;

	    p->save();
	    if ( flags & Style_Down )
		p->translate( pixelMetric( PM_ButtonShiftHorizontal ),
			      pixelMetric( PM_ButtonShiftVertical ) );

	    if ( flags & Style_Enabled ) {
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
	    p->restore();
	} else
	    QCommonStyle::drawPrimitive(pe, p, r, cg, flags, opt);
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
				 SFlags flags,
				 const QStyleOption& opt ) const
{
    switch (element) {
#ifndef QT_NO_TABBAR
    case CE_TabBarTab:
	{
	    if ( !widget || !widget->parentWidget() )
		break;

	    const QTabBar * tb = (const QTabBar *) widget;
	    bool lastIsCurrent = FALSE;
	    bool selected = flags & Style_Selected;

	    if ( styleHint( SH_TabBar_Alignment, tb ) == AlignRight &&
		 tb->currentTab() == tb->indexOf(tb->count()-1) )
		lastIsCurrent = TRUE;

	    QRect r2( r );
	    if ( tb->shape()  == QTabBar::RoundedAbove ) {
		p->setPen( cg.midlight() );
		p->drawLine( r2.left(), r2.bottom(), r2.right(), r2.bottom() );
		p->setPen( cg.light() );
		p->drawLine( r2.left(), r2.bottom()-1, r2.right(), r2.bottom()-1 );
		if ( r2.left() == 0 )
		    p->drawPoint( tb->rect().bottomLeft() );

		if ( selected ) {
		    p->fillRect( QRect( r2.left()+1, r2.bottom()-1, r2.width()-3, 2),
				 cg.brush( QColorGroup::Background ));
		    p->setPen( cg.background() );
		    p->drawLine( r2.left()+1, r2.bottom(), r2.left()+1, r2.top()+2 );
		    p->setPen( cg.light() );
		} else {
		    p->setPen( cg.light() );
		    r2.setRect( r2.left() + 2, r2.top() + 2,
			       r2.width() - 4, r2.height() - 2 );
		}

		int x1, x2;
		x1 = r2.left();
		x2 = r2.right() - 2;
		p->drawLine( x1, r2.bottom()-1, x1, r2.top() + 2 );
		x1++;
		p->drawPoint( x1, r2.top() + 1 );
		x1++;
		p->drawLine( x1, r2.top(), x2, r2.top() );
		if ( r2.left() > 0 ) {
		    p->setPen( cg.midlight() );
		}
		x1 = r2.left();
		p->drawPoint( x1, r2.bottom());

		p->setPen( cg.midlight() );
		x1++;
		p->drawLine( x1, r2.bottom(), x1, r2.top() + 2 );
		x1++;
		p->drawLine( x1, r2.top()+1, x2, r2.top()+1 );

		p->setPen( cg.dark() );
		x2 = r2.right() - 1;
		p->drawLine( x2, r2.top() + 2, x2, r2.bottom() - 1 +
			     (selected ? 1:-1) );
		p->setPen( cg.shadow() );
		p->drawPoint( x2, r2.top() + 1 );
		p->drawPoint( x2, r2.top() + 1 );
		x2++;
		p->drawLine( x2, r2.top() + 2, x2, r2.bottom() -
			     (selected ? (lastIsCurrent ? 0:1) :2));
	    } else if ( tb->shape() == QTabBar::RoundedBelow ) {
		if ( selected ) {
		    p->fillRect( QRect( r2.left()+1, r2.top(), r2.width()-3, 1),
				 tb->palette().active().brush( QColorGroup::Background ));
		    p->setPen( cg.background() );
		    p->drawLine( r2.left()+1, r2.top(), r2.left()+1, r2.bottom()-2 );
		    p->setPen( cg.dark() );
		} else {
		    p->setPen( cg.dark() );
		    p->drawLine( r2.left(), r2.top(), r2.right(), r2.top() );
		    r2.setRect( r2.left() + 2, r2.top(),
			       r2.width() - 4, r2.height() - 2 );
		}

		p->drawLine( r2.right() - 1, r2.top(),
			     r2.right() - 1, r2.bottom() - 2 );
		p->drawPoint( r2.right() - 2, r2.bottom() - 2 );
		p->drawLine( r2.right() - 2, r2.bottom() - 1,
			     r2.left() + 1, r2.bottom() - 1 );
		p->drawPoint( r2.left() + 1, r2.bottom() - 2 );

		p->setPen( cg.shadow() );
		p->drawLine( r2.right(), r2.top(),
			     r2.right(), r2.bottom() - 1 );
		p->drawPoint( r2.right() - 1, r2.bottom() - 1 );
		p->drawLine( r2.right() - 1, r2.bottom(),
			     r2.left() + 2, r2.bottom() );

		p->setPen( cg.light() );
		p->drawLine( r2.left(), r2.top(),
			     r2.left(), r2.bottom() - 2 );

	    } else {
		QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
	    }
	    break;
	}
#endif // QT_NO_TABBAR

#ifndef QT_NO_POPUPMENU
    case CE_PopupMenuItem:
	{
	    if (! widget || opt.isDefault())
		break;

	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    QMenuItem *mi = opt.menuItem();
	    if ( !mi )
		break;

	    int tab = opt.tabWidth();
	    int maxpmw = opt.maxIconWidth();
	    bool dis = ! mi->isEnabled();
	    bool checkable = popupmenu->isCheckable();
	    bool act = flags & Style_Active;
	    int x, y, w, h;

	    r.rect(&x, &y, &w, &h);

	    if ( checkable ) {
		// space for the checkmarks
		if (use2000style)
		    maxpmw = QMAX( maxpmw, 20 );
		else
		    maxpmw = QMAX( maxpmw, 12 );
	    }

	    int checkcol = maxpmw;

	    if ( mi && mi->isSeparator() ) {                    // draw separator
		p->setPen( cg.dark() );
		p->drawLine( x, y, x+w, y );
		p->setPen( cg.light() );
		p->drawLine( x, y+1, x+w, y+1 );
		return;
	    }

	    QBrush fill = (act ?
			   cg.brush( QColorGroup::Highlight ) :
			   cg.brush( QColorGroup::Button ));
	    p->fillRect( x, y, w, h, fill);

	    if ( !mi )
		return;

	    int xpos = x;
	    if ( mi->isChecked() ) {
		if ( act && !dis )
		    qDrawShadePanel( p, xpos, y, checkcol, h,
				     cg, TRUE, 1, &cg.brush( QColorGroup::Button ) );
		else {
		    QBrush fill( cg.light(), Dense4Pattern );
		    qDrawShadePanel( p, xpos, y, checkcol, h, cg, TRUE, 1,
				     &fill );
		}
	    } else if (! act)
		p->fillRect(xpos, y, checkcol , h, cg.brush( QColorGroup::Button ));

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
		if ( act && !dis && !mi->isChecked() )
		    qDrawShadePanel( p, xpos, y, checkcol, h, cg, FALSE, 1,
				     &cg.brush( QColorGroup::Button ) );
		QRect cr( xpos, y, checkcol, h );
		QRect pmr( 0, 0, pixw, pixh );
		pmr.moveCenter( cr.center() );
		p->setPen( cg.text() );
		p->drawPixmap( pmr.topLeft(), pixmap );

		fill = (act ?
			cg.brush( QColorGroup::Highlight ) :
			cg.brush( QColorGroup::Button ));
		int xp;
		xp = xpos + checkcol + 1;
		p->fillRect( xp, y, w - checkcol - 1, h, fill);
	    } else  if ( checkable ) {  // just "checking"...
		if ( mi->isChecked() ) {
		    int xp = xpos + windowsItemFrame;

		    SFlags cflags = Style_Default;
		    if (! dis)
			cflags |= Style_Enabled;
		    if (act)
			cflags |= Style_On;

		    drawPrimitive(PE_CheckMark, p,
				  QRect(xp, y + windowsItemFrame,
					checkcol - 2*windowsItemFrame,
					h - 2*windowsItemFrame), cg, cflags);
		}
	    }

	    p->setPen( act ? cg.highlightedText() : cg.buttonText() );

	    QColor discol;
	    if ( dis ) {
		discol = cg.text();
		p->setPen( discol );
	    }

	    int xm = windowsItemFrame + checkcol + windowsItemHMargin;
	    xpos += xm;

	    if ( mi->custom() ) {
		int m = windowsItemVMargin;
		p->save();
		if ( dis && !act ) {
		    p->setPen( cg.light() );
		    mi->custom()->paint( p, cg, act, !dis,
					 xpos+1, y+m+1, w-xm-tab+1, h-2*m );
		    p->setPen( discol );
		}
		mi->custom()->paint( p, cg, act, !dis,
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
		    xp = x + w - tab - windowsRightBorder - windowsItemHMargin -
			 windowsItemFrame + 1;
		    if ( dis && !act ) {
			p->setPen( cg.light() );
			p->drawText( xp, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
			p->setPen( discol );
		    }
		    p->drawText( xp, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
		    s = s.left( t );
		}
		if ( dis && !act ) {
		    p->setPen( cg.light() );
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
		PrimitiveElement arrow;
		arrow = PE_ArrowRight;
		xpos = x+w - windowsArrowHMargin - windowsItemFrame - dim;
		if ( act ) {
		    if ( !dis )
			discol = white;
		    QColorGroup g2( discol, cg.highlight(),
				    white, white,
				    dis ? discol : white,
				    discol, white );

		    drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
				  g2, Style_Enabled);
		} else {
		    drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
				  cg, mi->isEnabled() ? Style_Enabled : Style_Default);
		}
	    }

	    break;
	}
#endif

    case CE_MenuBarItem:
	{
	    bool active = flags & Style_Active;
	    bool hasFocus = flags & Style_HasFocus;
	    bool down = flags & Style_Down;
	    QRect pr = r;

	    p->fillRect( r, cg.brush( QColorGroup::Button ) );
	    if ( active || hasFocus ) {
		QBrush b = cg.brush( QColorGroup::Button );
		if ( active && down )
		    p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
		if ( active && hasFocus )
		    qDrawShadeRect( p, r.x(), r.y(), r.width(), r.height(),
				    cg, active && down, 1, 0, &b );
		if ( active && down ) {
		    pr.setRect( r.x() + 2 + pixelMetric(PM_ButtonShiftHorizontal,
							widget),
				r.y() + 2 + pixelMetric(PM_ButtonShiftVertical,
							widget),
				r.width()-4, r.height()-4 );
		    p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
		}
	    }
	    QCommonStyle::drawControl(element, p, widget, pr, cg, flags, opt);
	    break;
	}

    default:
	QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
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

    case PM_MaximumDragDistance:
	ret = 20;
	break;

#ifndef QT_NO_SLIDER
    case PM_SliderLength:
	ret = 11;
	break;

	// Returns the number of pixels to use for the business part of the
	// slider (i.e., the non-tickmark portion). The remaining space is shared
	// equally between the tickmark regions.
    case PM_SliderControlThickness:
	{
	    const QSlider * sl = (const QSlider *) widget;
	    int space = (sl->orientation() == Horizontal) ? sl->height()
			: sl->width();
	    int ticks = sl->tickmarks();
	    int n = 0;
	    if ( ticks & QSlider::Above ) n++;
	    if ( ticks & QSlider::Below ) n++;
	    if ( !n ) {
		ret = space;
		break;
	    }

	    int thick = 6;	// Magic constant to get 5 + 16 + 5
	    if ( ticks != QSlider::Both && ticks != QSlider::NoMarks )
		thick += pixelMetric( PM_SliderLength, sl ) / 4;

	    space -= thick;
	    //### the two sides may be unequal in size
	    if ( space > 0 )
		thick += (space * 2) / (n + 2);
	    ret = thick;
	    break;
	}
#endif // QT_NO_SLIDER

    case PM_MenuBarFrameWidth:
	ret = 0;
	break;

#if defined(Q_WS_WIN)
    case PM_TitleBarHeight:
	if ( widget && widget->testWFlags( WStyle_Tool ) ) {
	    ret = GetSystemMetrics( SM_CYSMCAPTION );
	} else {
	    ret = GetSystemMetrics( SM_CYCAPTION );
	}
	break;
#endif

    case PM_SplitterWidth:
	ret = QMAX( 6, QApplication::globalStrut().width() );
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
				       const QStyleOption& opt ) const
{
    QSize sz(contentsSize);

    switch (contents) {
    case CT_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, opt);
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
#endif
	    break;
	}

    case CT_PopupMenuItem:
	{
#ifndef QT_NO_POPUPMENU
	    if (! widget || opt.isDefault())
		break;

	    const QPopupMenu *popup = (const QPopupMenu *) widget;
	    bool checkable = popup->isCheckable();
	    QMenuItem *mi = opt.menuItem();
	    int maxpmw = opt.maxIconWidth();
	    int w = sz.width(), h = sz.height();

	    if (mi->custom()) {
		w = mi->custom()->sizeHint().width();
		h = mi->custom()->sizeHint().height();
		if (! mi->custom()->fullSpan())
		    h += 2*windowsItemVMargin + 2*windowsItemFrame;
	    } else if ( mi->widget() ) {
	    } else if (mi->isSeparator()) {
		w = 10; // arbitrary
		h = windowsSepHeight;
	    } else {
		if (mi->pixmap())
		    h = QMAX(h, mi->pixmap()->height() + 2*windowsItemFrame);
		else if (! mi->text().isNull())
		    h = QMAX(h, popup->fontMetrics().height() + 2*windowsItemVMargin +
			     2*windowsItemFrame);

		if (mi->iconSet() != 0)
		    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height() +
			     2*windowsItemFrame);
	    }

	    if (! mi->text().isNull() && mi->text().find('\t') >= 0)
		w += windowsTabSpacing;
	    else if (mi->popup())
		w += 2*windowsArrowHMargin;

	    if (use2000style) {
		if (checkable && maxpmw < 20)
		    w += 20 - maxpmw;
	    } else {
		if (checkable && maxpmw < windowsCheckMarkWidth)
		    w += windowsCheckMarkWidth - maxpmw;
	    }
	    if (maxpmw)
		w += maxpmw + 6;
	    if (checkable || maxpmw > 0)
		w += windowsCheckMarkHMargin;
	    w += windowsRightBorder;

	    sz = QSize(w, h);
#endif
	    break;
	}

    default:
	sz = QCommonStyle::sizeFromContents(contents, widget, sz, opt);
	break;
    }

    return sz;
}

/*! \reimp
*/
void QWindowsStyle::polishPopupMenu( QPopupMenu* p)
{
#ifndef QT_NO_POPUPMENU
    if ( !p->testWState( WState_Polished ) )
        p->setCheckable( TRUE );
#endif
}

#ifndef QT_NO_IMAGEIO_XPM
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

static const char * dock_window_close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"##....##",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"##....##",
"........"};

// Message box icons, from page 210 of the Windows style guide.

// Hand-drawn to resemble Microsoft's icons, but in the Mac/Netscape
// palette.  The "question mark" icon, which Microsoft recommends not
// using but a lot of people still use, is left out.

/* XPM */
static const char * const information_xpm[]={
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaabbbbaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaaabbbbbbaaaaaaaaac....",
".*aaaaaaaaaaabbbbaaaaaaaaaaac...",
".*aaaaaaaaaaaaaaaaaaaaaaaaaac*..",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac*.",
"*aaaaaaaaaabbbbbbbaaaaaaaaaaac*.",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
"..*aaaaaaaaaabbbbbaaaaaaaaac***.",
"...caaaaaaabbbbbbbbbaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**..........."};
/* XPM */
static const char* const warning_xpm[]={
"32 32 4 1",
". c None",
"a c #ffff00",
"* c #000000",
"b c #999999",
".............***................",
"............*aaa*...............",
"...........*aaaaa*b.............",
"...........*aaaaa*bb............",
"..........*aaaaaaa*bb...........",
"..........*aaaaaaa*bb...........",
".........*aaaaaaaaa*bb..........",
".........*aaaaaaaaa*bb..........",
"........*aaaaaaaaaaa*bb.........",
"........*aaaa***aaaa*bb.........",
".......*aaaa*****aaaa*bb........",
".......*aaaa*****aaaa*bb........",
"......*aaaaa*****aaaaa*bb.......",
"......*aaaaa*****aaaaa*bb.......",
".....*aaaaaa*****aaaaaa*bb......",
".....*aaaaaa*****aaaaaa*bb......",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"...*aaaaaaaaa***aaaaaaaaa*bb....",
"...*aaaaaaaaaa*aaaaaaaaaa*bb....",
"..*aaaaaaaaaaa*aaaaaaaaaaa*bb...",
"..*aaaaaaaaaaaaaaaaaaaaaaa*bb...",
".*aaaaaaaaaaaa**aaaaaaaaaaa*bb..",
".*aaaaaaaaaaa****aaaaaaaaaa*bb..",
"*aaaaaaaaaaaa****aaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaa**aaaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
".*aaaaaaaaaaaaaaaaaaaaaaaaa*bbbb",
"..*************************bbbbb",
"....bbbbbbbbbbbbbbbbbbbbbbbbbbb.",
".....bbbbbbbbbbbbbbbbbbbbbbbbb.."};
/* XPM */
static const char* const critical_xpm[]={
"32 32 4 1",
". c None",
"a c #999999",
"* c #ff0000",
"b c #ffffff",
"...........********.............",
".........************...........",
".......****************.........",
"......******************........",
".....********************a......",
"....**********************a.....",
"...************************a....",
"..*******b**********b*******a...",
"..******bbb********bbb******a...",
".******bbbbb******bbbbb******a..",
".*******bbbbb****bbbbb*******a..",
"*********bbbbb**bbbbb*********a.",
"**********bbbbbbbbbb**********a.",
"***********bbbbbbbb***********aa",
"************bbbbbb************aa",
"************bbbbbb************aa",
"***********bbbbbbbb***********aa",
"**********bbbbbbbbbb**********aa",
"*********bbbbb**bbbbb*********aa",
".*******bbbbb****bbbbb*******aa.",
".******bbbbb******bbbbb******aa.",
"..******bbb********bbb******aaa.",
"..*******b**********b*******aa..",
"...************************aaa..",
"....**********************aaa...",
"....a********************aaa....",
".....a******************aaa.....",
"......a****************aaa......",
".......aa************aaaa.......",
".........aa********aaaaa........",
"...........aaaaaaaaaaa..........",
".............aaaaaaa............"};

#endif //QT_NO_IMAGEIO_XPM

/*!
 \reimp
 */
QPixmap QWindowsStyle::stylePixmap(StylePixmap stylepixmap,
				   const QWidget *widget,
				   const QStyleOption& opt) const
{
#ifndef QT_NO_IMAGEIO_XPM
    switch (stylepixmap) {
    case SP_TitleBarShadeButton:
	return QPixmap((const char **)qt_shade_xpm);
    case SP_TitleBarUnshadeButton:
	return QPixmap((const char **)qt_unshade_xpm);
    case SP_TitleBarNormalButton:
	return QPixmap((const char **)qt_normalizeup_xpm);
    case SP_TitleBarMinButton:
	return QPixmap((const char **)qt_minimize_xpm);
    case SP_TitleBarMaxButton:
	return QPixmap((const char **)qt_maximize_xpm);
    case SP_TitleBarCloseButton:
	return QPixmap((const char **)qt_close_xpm);
    case SP_DockWindowCloseButton:
	return QPixmap((const char **)dock_window_close_xpm );
    case SP_MessageBoxInformation:
	return QPixmap((const char **)information_xpm);
    case SP_MessageBoxWarning:
	return QPixmap((const char **)warning_xpm);
    case SP_MessageBoxCritical:
	return QPixmap((const char **)critical_xpm);
    default:
	break;
    }
#endif //QT_NO_IMAGEIO_XPM
    return QCommonStyle::stylePixmap(stylepixmap, widget, opt);
}

/*!\reimp
*/
void QWindowsStyle::drawComplexControl( ComplexControl ctrl, QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags,
					SCFlags sub,
					SCFlags subActive,
					const QStyleOption& opt ) const
{
    switch (ctrl) {
#ifndef QT_NO_LISTVIEW
    case CC_ListView:
	{
	    if (opt.isDefault())
		break;

	    QListViewItem *item = opt.listViewItem(),
			 *child = item->firstChild();

	    int linetop = 0, linebot = 0, y = r.y();
	    // each branch needs at most two lines, ie. four end points
	    int dotoffset = (item->itemPos() + item->height() - y) %2;
	    QPointArray dotlines( item->childCount() * 4 );
	    int c = 0;

	    // skip the stuff above the exposed rectangle
	    while ( child && y + child->height() <= 0 ) {
		y += child->totalHeight();
		child = child->nextSibling();
	    }

	    int bx = r.width() / 2;

	    // paint stuff in the magical area
	    while ( child && y < r.height() ) {
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
		    dotlines[c++] = QPoint( r.width(), linebot );
		    linetop = linebot + 5;
		} else {
		    // just dotlinery
		    dotlines[c++] = QPoint( bx+1, linebot );
		    dotlines[c++] = QPoint( r.width(), linebot );
		}

		y += child->totalHeight();
		child = child->nextSibling();
	    }

	    if ( child ) // there's a child, so move linebot to edge of rectangle
		linebot = r.height();

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
		qlv_cleanup_bitmap.add( &verticalLine );
		qlv_cleanup_bitmap.add( &horizontalLine );
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
	    break;
	}
#endif //QT_NO_LISTVIEW

#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
	if ( sub & SC_ComboBoxArrow ) {
	    SFlags flags = Style_Default;

	    qDrawWinPanel( p, r, cg, TRUE, widget->isEnabled() ?
			   &cg.brush( QColorGroup::Base ):
			   &cg.brush( QColorGroup::Background ) );

	    QRect ar =
		QStyle::visualRect( querySubControlMetrics( CC_ComboBox, widget,
							    SC_ComboBoxArrow ), widget );
	    if ( subActive == SC_ComboBoxArrow ) {
		p->setPen( cg.dark() );
		p->setBrush( cg.brush( QColorGroup::Button ) );
		p->drawRect( ar );
	    } else
		qDrawWinPanel( p, ar, cg, FALSE,
			       &cg.brush( QColorGroup::Button ) );

	    ar.addCoords( 2, 2, -2, -2 );
	    if ( widget->isEnabled() )
		flags |= Style_Enabled;

	    if ( subActive & Style_Sunken ) {
		flags |= Style_Sunken;
	    }
	    drawPrimitive( PE_ArrowDown, p, ar, cg, flags );
	}

	if ( sub & SC_ComboBoxEditField ) {
	    const QComboBox * cb = (const QComboBox *) widget;
	    QRect re =
		QStyle::visualRect( querySubControlMetrics( CC_ComboBox, widget,
							    SC_ComboBoxEditField ), widget );
	    if ( cb->hasFocus() && !cb->editable() )
		p->fillRect( re.x(), re.y(), re.width(), re.height(),
			     cg.brush( QColorGroup::Highlight ) );

	    if ( cb->hasFocus() ) {
		p->setPen( cg.highlightedText() );
		p->setBackgroundColor( cg.highlight() );

	    } else {
		p->setPen( cg.text() );
		p->setBackgroundColor( cg.background() );
	    }

	    if ( cb->hasFocus() && !cb->editable() ) {
		QRect re =
		    QStyle::visualRect( subRect( SR_ComboBoxFocusRect, cb ), widget );
		drawPrimitive( PE_FocusRect, p, re, cg, Style_FocusAtBorder, QStyleOption(cg.highlight()));
	    }
	}

	break;
#endif	// QT_NO_COMBOBOX

#ifndef QT_NO_SLIDER
    case CC_Slider:
	{
	    const QSlider *sl = (const QSlider *) widget;
	    int thickness  = pixelMetric( PM_SliderControlThickness, widget );
	    int len        = pixelMetric( PM_SliderLength, widget );
	    int ticks = sl->tickmarks();

	    QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
						  opt),
		  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
						  opt);

	    if ((sub & SC_SliderGroove) && groove.isValid()) {
		int mid = thickness / 2;

		if ( ticks & QSlider::Above )
		    mid += len / 8;
		if ( ticks & QSlider::Below )
		    mid -= len / 8;

		p->setPen( cg.shadow() );
		if ( sl->orientation() == Horizontal ) {
		    qDrawWinPanel( p, groove.x(), groove.y() + mid - 2,
				   groove.width(), 4, cg, TRUE );
		    p->drawLine( groove.x() + 1, groove.y() + mid - 1,
				 groove.x() + groove.width() - 3, groove.y() + mid - 1 );
		} else {
		    qDrawWinPanel( p, groove.x() + mid - 2, groove.y(),
				   4, groove.height(), cg, TRUE );
		    p->drawLine( groove.x() + mid - 1, groove.y() + 1,
				 groove.x() + mid - 1,
				 groove.y() + groove.height() - 3 );
		}
	    }

	    if (sub & SC_SliderTickmarks)
		QCommonStyle::drawComplexControl(ctrl, p, widget, r, cg, flags,
						 SC_SliderTickmarks, subActive,
						 opt );

	    if ( sub & SC_SliderHandle ) {
		// 4444440
		// 4333310
		// 4322210
		// 4322210
		// 4322210
		// 4322210
		// *43210*
		// **410**
		// ***0***
		enum  SliderDir { SlUp, SlDown, SlLeft, SlRight };

		const QColor c0 = cg.shadow();
		const QColor c1 = cg.dark();
		// const QColor c2 = g.button();
		const QColor c3 = cg.midlight();
		const QColor c4 = cg.light();

		int x = handle.x(), y = handle.y(),
		   wi = handle.width(), he = handle.height();

		int x1 = x;
		int x2 = x+wi-1;
		int y1 = y;
		int y2 = y+he-1;

		bool reverse = QApplication::reverseLayout();

		Orientation orient = sl->orientation();
		bool tickAbove = sl->tickmarks() == QSlider::Above;
		bool tickBelow = sl->tickmarks() == QSlider::Below;

		p->fillRect( x, y, wi, he, cg.brush( QColorGroup::Background ) );

		if ( (tickAbove && tickBelow) || (!tickAbove && !tickBelow) ) {
		    qDrawWinButton( p, QRect(x,y,wi,he), cg, FALSE,
				    &cg.brush( QColorGroup::Button ) );
		    return;
		}

		if ( sl->hasFocus() ) {
		    QRect re = subRect( SR_SliderFocusRect, sl );
		    drawPrimitive( PE_FocusRect, p, re, cg );
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
		    y1 = y1 + wi/2;
		    d =  (wi + 1) / 2 - 1;
		    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2,y1, x1+d,y1-d );
		    break;
		case SlDown:
		    y2 = y2 - wi/2;
		    d =  (wi + 1) / 2 - 1;
		    a.setPoints(5, x1,y1, x1,y2, x1+d,y2+d, x2,y2, x2,y1 );
		    break;
		case SlLeft:
		    d =  (he + 1) / 2 - 1;
		    x1 = x1 + he/2;
		    a.setPoints(5, x1,y1, x1-d,y1+d, x1,y2, x2,y2, x2,y1);
		    break;
		case SlRight:
		    d =  (he + 1) / 2 - 1;
		    x2 = x2 - he/2;
		    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2+d,y1+d, x2,y1 );
		    break;
		}

		QBrush oldBrush = p->brush();
		p->setBrush( cg.brush( QColorGroup::Button ) );
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
		    d = wi - d - 1;
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
		    d = wi - d - 1;
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
		    d = he - d - 1;
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
		    d = he - d - 1;
		    p->drawLine( x2, y2, x2+d, y2-d);
		    d--;
		    p->setPen( c3 );
		    p->drawLine(  x2, y1+1, x2+d, y1+1+d );
		    p->setPen( c1 );
		    p->drawLine( x2, y2-1, x2+d, y2-1-d);
		    break;
		}
	    }

	    break;
	}
#endif // QT_NO_SLIDER

    default:
	QCommonStyle::drawComplexControl( ctrl, p, widget, r, cg, flags, sub,
					  subActive, opt );
	break;
    }
}


/*! \reimp */
int QWindowsStyle::styleHint( StyleHint hint,
			      const QWidget *widget,
			      QStyleHintReturn *returnData ) const
{
    int ret;

    switch (hint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_MainWindow_SpaceBelowMenuBar:
    case SH_FontDialog_SelectAssociatedText:
    case SH_PopupMenu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_PopupMenu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
	ret = 1;
	break;

    case SH_ItemView_ChangeHighlightOnFocus:
#if defined(Q_WS_WIN)
	if ( qWinVersion() & WV_98 || qWinVersion() & WV_2000 || qWinVersion() & WV_XP )
	    ret = 1;
	else
#endif
	    ret = 0;
	break;

    default:
	ret = QCommonStyle::styleHint(hint, widget, returnData);
	break;
    }

    return ret;
}

#endif
