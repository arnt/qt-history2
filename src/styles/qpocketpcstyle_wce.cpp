/****************************************************************************
** $Id: $
**
** Implementation of PocketPC-like style class
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

#include "qpocketpcstyle_wce.h"

#ifndef QT_NO_STYLE_POCKETPC
#ifndef Q_QDOC

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
#include "qcheckbox.h"
#include "qprogressbar.h"
#include "qradiobutton.h"
#include "qbitmap.h"
#include "qtoolbutton.h"
#include "../widgets/qtitlebar_p.h"
#include "qcleanuphandler.h"

#include <limits.h>


static const int pocketpcItemFrame		=  1; // menu item frame width
static const int pocketpcSepHeight		=  1; // separator item height
static const int pocketpcItemHMargin		=  2; // menu item hor text margin
static const int pocketpcItemVMargin		=  1; // menu item ver text margin
static const int pocketpcArrowHMargin		=  4; // arrow horizontal margin
static const int pocketpcTabSpacing		=  6; // space between text and tab
static const int pocketpcCheckMarkHMargin	=  1; // horiz. margins of check mark
static const int pocketpcRightBorder		=  6; // right border on pocketpc
static const int pocketpcCheckMarkWidth		=  6; // checkmarks width on windows


/*!
  \class QPocketPCStyle qpocketpcstyle_wce.h
  \brief The QPocketPCStyle class provides Microsoft PocketPC look and feel.
  \ingroup appearance

  This class implements the look and feel known from the PocketPC
  platform. Naturally it is also Qt's default GUI style on PocketPC.
*/

/*!
    Constructs a QPocketPCStyle
*/
QPocketPCStyle::QPocketPCStyle()
{
}


/*! \reimp */
QPocketPCStyle::~QPocketPCStyle()
{
}


/*! \reimp */
void QPocketPCStyle::polish( QApplication* )
{
    QFont f = QApplication::font();

    f.setPointSize( f.pointSize() + 1 );
    f.setBold( TRUE );
    QApplication::setFont( f, TRUE, "QPopupMenu" );
    QApplication::setFont( f, TRUE, "QMenuBar" );

    QPalette pal = QApplication::palette();
    pal.setColor( QColorGroup::Background, pal.active().light() );
    // darker basecolor in list-widgets
//    pal.setColor( QColorGroup::Base, pal.active().base().dark(130) );

    QApplication::setPalette( pal, TRUE );
/*
    // different basecolor and highlighting in Q(Multi)LineEdit
    pal.setColor( QColorGroup::Base, QColor(211,181,181) );
    pal.setColor( QPalette::Active, QColorGroup::Highlight, pal.active().midlight() );
    pal.setColor( QPalette::Active, QColorGroup::HighlightedText, pal.active().text() );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight, pal.inactive().midlight() );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, pal.inactive().text() );
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight, pal.disabled().midlight() );
    pal.setColor( QPalette::Disabled, QColorGroup::HighlightedText, pal.disabled().text() );
    QApplication::setPalette( pal, TRUE, "QLineEdit" );
    QApplication::setPalette( pal, TRUE, "QMultiLineEdit" );
*/
    pal = QApplication::palette();
    pal.setColor( QColorGroup::Button, pal.active().midlight() ); // background() );
    QApplication::setPalette( pal, TRUE, "QMenuBar" );
    QApplication::setPalette( pal, TRUE, "QToolBar" );
}


/*! \reimp */
void QPocketPCStyle::unPolish( QApplication* )
{
    QFont f = QApplication::font();

    f.setPointSize( f.pointSize() - 1 );
    f.setBold( FALSE );
    QApplication::setFont( f, TRUE, "QPopupMenu" );
    QApplication::setFont( f, TRUE, "QMenuBar" );
//    QApplication::setFont( f, TRUE, "QComboBox" );
}


/*! \reimp */
void QPocketPCStyle::drawPrimitive( PrimitiveElement pe,
				   QPainter *p,
				   const QRect &r,
				   const QColorGroup &cg,
				   SFlags flags,
				   const QStyleOption& opt ) const
{
    static const QCOORD radioOutline[] = { 1,3, 3,1, 4,1, 5,0, 9,0, 10,1, 11,1, 13,3, 13,4, 14,5,
	14,9, 13,10, 13,11, 11,13, 10,13, 9,14, 5,14, 4,13, 3,13, 1,11, 1,10, 0,9, 0,5, 1,4, 1,3 };
    static const QCOORD radioDot[]     = { 6,3,  8,3,  4,4, 10,4,  4,5, 10,5,  3,6,  11,6, 3,7,
					   11,7, 3,8, 11,8,  4,9, 10,9, 4,10, 10,10, 6,11, 8,11 };
    static const QCOORD tick[18]       = { 3,6, 6,9, 11,4, 11,5, 6,10, 3,7, 3,8, 6,11, 11,6  };
    static const QCOORD arrowUp[16]    = { -3, 1,  3, 1, -2, 0,  2, 0, -1,-1,  1,-1,  0,-2,  0,-2 };
    static const QCOORD arrowDown[16]  = { -3,-2,  3,-2, -2,-1,  2,-1, -1, 0,  1, 0,  0, 1,  0, 1 };
// ### positioning needs fixing
    static const QCOORD arrowRight[16] = { -2,-3, -2, 3, -1,-2, -1, 2,  0,-1,  0, 1,  1, 0,  1, 0 };
    static const QCOORD arrowLeft[16]  = {  0,-3,  0, 3, -1,-2, -1, 2, -2,-1, -2, 1, -3, 0, -3, 0 };
    static const QCOORD *arrows[4]     = { arrowUp, arrowDown, arrowRight, arrowLeft };

#define CreateQPointArray(pts)	QPointArray( sizeof(pts)/(sizeof(QCOORD)*2), pts )

    int arrow = 3;

    switch (pe) {
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_ButtonDropDown:
    case PE_Splitter:
    case PE_DockWindowResizeHandle:
        p->setBrush( (flags & Style_Down) ? Qt::black : Qt::lightGray );
		p->setPen( Qt::black );
		p->drawRect( r );
	break;
    case PE_HeaderSection:
    case PE_Panel:
    case PE_PanelPopup:
    case PE_PanelMenuBar:
    case PE_GroupBoxFrame:
	p->setBrush( Qt::white );
	p->setPen( Qt::black );
	p->drawRect( r );
	break;
    case PE_Separator:
	p->setBrush( NoBrush );
	p->setPen( Qt::lightGray );
	p->drawRect( r );
	break;
    case PE_FocusRect:
	// I'm not sure how much focus rects are used on pocketPC
	p->setBrush( NoBrush );
	p->setPen( Qt::lightGray );
	p->drawRect( r );
	break;
    case PE_StatusBarSection:
	p->setBrush( Qt::lightGray );
	p->setPen( Qt::black );
	p->drawRect( r );
	break;
//    case PE_IndicatorMask:
    case PE_Indicator:
	p->setBrush( Qt::white );
	p->setPen( Qt::black );
	p->drawRect( r );   // Draw the box around the control
	if ( flags & Style_On )
	    p->drawPolyline( CreateQPointArray( tick ) );   // Draw the tick if it is on
	break;
//    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator:
	p->setPen( Qt::black );
	p->drawPolyline( CreateQPointArray( radioOutline ) );
	if ( flags & Style_On )	// radio button dot shown when it is selected
	    p->drawPolyline( CreateQPointArray( radioDot ) );
	break;
    case PE_ScrollBarSubLine:
        drawPrimitive(PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
			  ((flags & Style_Down) ? Style_Down : Style_Raised));
	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp), p, r, cg, flags);
	break;
    case PE_ScrollBarAddLine:
        drawPrimitive(PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
			  ((flags & Style_Down) ? Style_Down : Style_Raised));
	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown), p, r, cg, flags);
	break;
    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:
	p->setBrush( Qt::white ); // on pocketPC it doesn't change appearence when pressed
	p->setPen( Qt::black );
	p->drawRect( r );
	break;
    case PE_ScrollBarSlider: {
	p->setBrush( Qt::lightGray ); // on pocketPC it doesn't change appearence when pressed
	p->setPen( Qt::black );
	p->drawRect( r );
	// the 3 little lines on the slider button
	int midy = r.y() + r.height() / 2;
	p->drawLine( r.x() + 3, midy - 2, r.x() + r.width() - 4, midy - 2 );
	p->drawLine( r.x() + 3, midy + 0, r.x() + r.width() - 4, midy + 0 );
	p->drawLine( r.x() + 3, midy + 2, r.x() + r.width() - 4, midy + 2 );
	break; }
    case PE_ArrowUp:
	arrow--;
    case PE_ArrowDown:
	arrow--;
    case PE_ArrowRight:
	arrow--;
    case PE_ArrowLeft: {
        QPointArray a( 8, arrows[arrow] );
	a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
	p->setPen( (flags & Style_Down) ? Qt::white : Qt::black );
        p->drawLineSegments( a );         // draw arrow
	break; }












/*
    case PE_HeaderArrow:
	p->save();
	if ( flags & Style_Down ) {
	    QPointArray pa( 3 );
	    p->setPen( cg.light() );
	    p->drawLine( r.x() + r.width(), r.y(), r.x() + r.width() / 2, r.height() );
	    p->setPen( cg.dark() );
	    pa.setPoint( 0, r.x() + r.width() / 2, r.height() );
	    pa.setPoint( 1, r.x(), r.y() );
	    pa.setPoint( 2, r.x() + r.width(), r.y() );
	    p->drawPolyline( pa );
	} else {
	    QPointArray pa( 3 );
	    p->setPen( cg.light() );
	    pa.setPoint( 0, r.x(), r.height() );
	    pa.setPoint( 1, r.x() + r.width(), r.height() );
	    pa.setPoint( 2, r.x() + r.width() / 2, r.y() );
	    p->drawPolyline( pa );
	    p->setPen( cg.dark() );
	    p->drawLine( r.x(), r.height(), r.x() + r.width() / 2, r.y() );
	}
	p->restore();
	break;


    case PE_SpinWidgetPlus:
    case PE_SpinWidgetMinus: {
	p->save();
	int fw = pixelMetric( PM_DefaultFrameWidth, 0 );
	QRect br;
	br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
		    r.height() - fw*2 );

	p->fillRect( br, cg.brush( QColorGroup::Button ) );
	p->setPen( cg.buttonText() );
	p->setBrush( cg.buttonText() );

	int length;
	int x = r.x(), y = r.y(), w = r.width(), h = r.height();
	if ( w <= 8 || h <= 6 )
	    length = QMIN( w-2, h-2 );
	else
	    length = QMIN( 2*w / 3, 2*h / 3 );

	if ( !(length & 1) )
	    length -=1;
	int xmarg = ( w - length ) / 2;
	int ymarg = ( h - length ) / 2;

	p->drawLine( x + xmarg, ( y + h / 2 - 1 ),
		     x + xmarg + length - 1, ( y + h / 2 - 1 ) );
	if ( pe == PE_SpinWidgetPlus )
	    p->drawLine( ( x+w / 2 ) - 1, y + ymarg,
			 ( x+w / 2 ) - 1, y + ymarg + length - 1 );
	p->restore();
	break; }

    case PE_SpinWidgetUp:
    case PE_SpinWidgetDown: {
	p->save();
	int fw = pixelMetric( PM_DefaultFrameWidth, 0 );
	QRect br;
	br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
		    r.height() - fw*2 );
	p->fillRect( br, cg.brush( QColorGroup::Button ) );
	int x = r.x(), y = r.y(), w = r.width(), h = r.height();
	int sw = w-4;
	if ( sw < 3 )
	    return;
	else if ( !(sw & 1) )
	    sw--;
	sw -= ( sw / 7 ) * 2;	// Empty border
	int sh = sw/2 + 2;      // Must have empty row at foot of arrow

	int sx = x + w / 2 - sw / 2 - 1;
	int sy = y + h / 2 - sh / 2 - 1;

	QPointArray a;
	if ( pe == PE_SpinWidgetDown )
	    a.setPoints( 3,  0, 1,  sw-1, 1,  sh-2, sh-1 );
	else
	    a.setPoints( 3,  0, sh-1,  sw-1, sh-1,  sh-2, 1 );
	int bsx = 0;
	int bsy = 0;
	if ( flags & Style_Sunken ) {
	    bsx = pixelMetric(PM_ButtonShiftHorizontal);
	    bsy = pixelMetric(PM_ButtonShiftVertical);
	}
	p->translate( sx + bsx, sy + bsy );
	p->setPen( cg.buttonText() );
	p->setBrush( cg.buttonText() );
	p->drawPolygon( a );
	p->restore();
	break; }

    case PE_DockWindowHandle: {
	bool highlight = flags & Style_On;

	p->save();
	p->translate( r.x(), r.y() );
	if ( flags & Style_Vertical ) {
	    if ( r.width() > 4 ) {
		qDrawShadePanel( p, 2, 4, r.width() - 4, 3,
				 cg, highlight, 1, 0 );
		qDrawShadePanel( p, 2, 7, r.width() - 4, 3,
				 cg, highlight, 1, 0 );
	    }
	} else {
	    if ( r.height() > 4 ) {
		qDrawShadePanel( p, 4, 2, 3, r.height() - 4,
				 cg, highlight, 1, 0 );
		qDrawShadePanel( p, 7, 2, 3, r.height() - 4,
				 cg, highlight, 1, 0 );
	    }
	}
	p->restore();
	break;
    }

    case PE_DockWindowSeparator: {
	QPoint p1, p2;
	if ( flags & Style_Vertical ) {
	    p1 = QPoint( 0, r.height()/2 );
	    p2 = QPoint( r.width(), p1.y() );
	} else {
	    p1 = QPoint( r.width()/2, 0 );
	    p2 = QPoint( p1.x(), r.height() );
	}
	qDrawShadeLine( p, p1, p2, cg, 1, 1, 0 );
	break; }

    case PE_PanelDockWindow: {
	int lw = opt.isDefault() ? pixelMetric(PM_DockWindowFrameWidth)
			: opt.lineWidth();

	qDrawShadePanel(p, r, cg, FALSE, lw);
	break; }

    case PE_SizeGrip: {
	p->save();

	int x, y, w, h;
	r.rect(&x, &y, &w, &h);

	int sw = QMIN( h,w );
	if ( h > w )
	    p->translate( 0, h - w );
	else
	    p->translate( w - h, 0 );

	int sx = x;
	int sy = y;
	int s = sw / 3;

	if ( QApplication::reverseLayout() ) {
	    sx = x + sw;
	    for ( int i = 0; i < 4; ++i ) {
		p->setPen( QPen( cg.light(), 1 ) );
		p->drawLine(  x, sy - 1 , sx + 1,  sw );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  x, sy, sx,  sw );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  x, sy + 1, sx - 1,  sw );
		sx -= s;
		sy += s;
	    }
	} else {
	    for ( int i = 0; i < 4; ++i ) {
		p->setPen( QPen( cg.light(), 1 ) );
		p->drawLine(  sx-1, sw, sw,  sy-1 );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  sx, sw, sw,  sy );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  sx+1, sw, sw,  sy+1 );
		sx += s;
		sy += s;
	    }
	}

	p->restore();
	break; }

    case PE_CheckMark: {
	const int markW = r.width() > 7 ? 7 : r.width();
	const int markH = markW;
	int posX = r.x() + ( r.width() - markW )/2 + 1;
	int posY = r.y() + ( r.height() - markH )/2;

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
	if ( !(flags & Style_Enabled) && !(flags & Style_On)) {
	    int pnt;
	    p->setPen( cg.highlightedText() );
	    QPoint offset(1,1);
	    for ( pnt = 0; pnt < (int)a.size(); pnt++ )
		a[pnt] += offset;
	    p->drawLineSegments( a );
	    for ( pnt = 0; pnt < (int)a.size(); pnt++ )
		a[pnt] -= offset;
	}
	p->setPen( cg.text() );
	p->drawLineSegments( a );
	break; }

    case PE_ProgressBarChunk:
	p->fillRect( r.x(), r.y() + 3, r.width() -2, r.height() - 6,
	    cg.brush(QColorGroup::Highlight));
	break;
*/


    default:
	QWindowsStyle::drawPrimitive( pe, p, r, cg, flags, opt );
	break;
    }
}


/*!
  \reimp
*/
void QPocketPCStyle::drawControl( ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 SFlags how,
				 const QStyleOption& opt ) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QPocketPCStyle::drawControl: widget parameter cannot be zero!");
	return;
    }
#endif

    SFlags flags = Style_Default;
    if (widget->isEnabled())
	flags |= Style_Enabled;

    switch (element) {
    case CE_PushButton:
	{
	    const QPushButton *button = (const QPushButton *) widget;
	    QRect br = r;
	    int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);

	    SFlags flags = Style_Default;
	    if (button->isEnabled())
		flags |= Style_Enabled;
	    if (button->isDown())
		flags |= Style_Down;
	    if (button->isOn())
		flags |= Style_On;
	    if (! button->isFlat() && ! (flags & Style_Down))
		flags |= Style_Raised;

	    drawPrimitive(PE_ButtonCommand, p, br, cg, flags);
	    break;
	}

    case CE_PushButtonLabel:
	{
	    const QPushButton *button = (const QPushButton *) widget;
	    QRect ir = r;

	    if (button->isDown() || button->isOn()) {
		flags |= Style_Sunken;
	    }

	    if (button->isMenuButton()) {
		int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
		QRect ar(ir.right() - mbi, ir.y() + 2, mbi - 4, ir.height() - 4);
		drawPrimitive(PE_ArrowDown, p, ar, cg, flags, opt);
		ir.setWidth(ir.width() - mbi);
	    }

	    if ( button->iconSet() && ! button->iconSet()->isNull() ) {
		QIconSet::Mode mode =
		    button->isEnabled() ? QIconSet::Normal : QIconSet::Disabled;
		if ( mode == QIconSet::Normal && button->hasFocus() )
		    mode = QIconSet::Active;

		QIconSet::State state = QIconSet::Off;
		if ( button->isToggleButton() && button->isOn() )
		    state = QIconSet::On;

		QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode, state );
		int pixw = pixmap.width();
		int pixh = pixmap.height();
		p->drawPixmap( ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap );

		ir.moveBy(pixw + 4, 0);
		ir.setWidth(ir.width() - pixw + 4);
	    }

	    drawItem(p, ir, AlignCenter | ShowPrefix, cg,
		flags & Style_Enabled, button->pixmap(), button->text(), -1,
		(flags & Style_Sunken) ? &Qt::white : &Qt::black );

	    if (button->hasFocus()) {
		p->setBrush( NoBrush );
		p->setPen( Qt::black );
		p->drawRect( QRect( ir.x() + 1, ir.y() + 1, ir.width() - 2, ir.height() - 2) );
	    }
	    break;
	}

#ifndef QT_NO_TABBAR
    case CE_TabBarTab:
	{
	    if ( !widget || !widget->parentWidget() )
		break;

	    bool selected = how & Style_Selected;

	    if ( selected ) {
		p->setBrush( Qt::white );
		p->setPen( Qt::black );
		p->drawRect( r );
	    } else {
	        p->setBrush( Qt::lightGray );
		p->setPen( Qt::black );
		p->drawRect( r );
	    }
	    break;
	}
    case CE_TabBarLabel:
	{
	    if ( opt.isDefault() )
		break;

	    const QTabBar * tb = (const QTabBar *) widget;
	    QTab * t = opt.tab();
	    //bool has_focus = opt.hasFocus();

	    QRect tr = r;
	    if ( t->identifier() == tb->currentTab() )
		tr.setBottom( tr.bottom() -
			      pixelMetric( QStyle::PM_DefaultFrameWidth, tb ) );

	    drawItem( p, tr, AlignCenter | ShowPrefix, cg, tb->isEnabled() &&
		      t->isEnabled(), 0, t->text() );

	    //if ( has_focus )
		//drawPrimitive( PE_FocusRect, p, r, cg );
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
	    bool act = how & Style_Selected;
	    int x, y, w, h;

	    r.rect(&x, &y, &w, &h);

	    if ( checkable ) {
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
		    int xp = xpos + pocketpcItemFrame;

		    SFlags cflags = Style_Default;
		    if (! dis)
			cflags |= Style_Enabled;
		    if (act)
			cflags |= Style_On;

		    drawPrimitive(PE_CheckMark, p,
				  QRect(xp, y + pocketpcItemFrame,
					checkcol - 2*pocketpcItemFrame,
					h - 2*pocketpcItemFrame), cg, cflags);
		}
	    }

	    p->setPen( act ? cg.highlightedText() : cg.buttonText() );

	    QColor discol;
	    if ( dis ) {
		discol = cg.text();
		p->setPen( discol );
	    }

	    int xm = pocketpcItemFrame + checkcol + pocketpcItemHMargin;
	    xpos += xm;

	    if ( mi->custom() ) {
		int m = pocketpcItemVMargin;
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
		int m = pocketpcItemVMargin;
		const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
		if ( t >= 0 ) {                         // draw tab text
		    int xp;
		    xp = x + w - tab - pocketpcRightBorder - pocketpcItemHMargin -
			 pocketpcItemFrame + 1;
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
		p->drawPixmap( xpos, y+pocketpcItemFrame, *pixmap );
		if ( pixmap->depth() == 1 )
		    p->setBackgroundMode( TransparentMode );
	    }
	    if ( mi->popup() ) {                        // draw sub menu arrow
		int dim = (h-2*pocketpcItemFrame) / 2;
		PrimitiveElement arrow;
		arrow = PE_ArrowRight;
		xpos = x+w - pocketpcArrowHMargin - pocketpcItemFrame - dim;
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
	    bool active = how & Style_Active;
	    bool hasFocus = how & Style_HasFocus;
	    bool down = how & Style_Down;
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
	    if (opt.isDefault())
		break;

	    QMenuItem *mi = opt.menuItem();
	    drawItem( p, r, AlignCenter|ShowPrefix|DontClip|SingleLine, cg,
		      mi->isEnabled(), mi->pixmap(), mi->text(), -1,
		      &cg.buttonText() );
	    //QCommonStyle::drawControl(element, p, widget, pr, cg, how, opt);
	    break;
	}

    case CE_CheckBox:
	{
	    // many people expect to checkbox to be square, do that here.
	    QRect ir = r;

	    if (r.width() < r.height()) {
		ir.setTop(r.top() + (r.height() - r.width()) / 2);
		ir.setHeight(r.width());
	    } else if (r.height() < r.width()) {
		ir.setLeft(r.left() + (r.width() - r.height()) / 2);
		ir.setWidth(r.height());
	    }

	    const QCheckBox *checkbox = (const QCheckBox *) widget;

	    if (checkbox->isDown())
		flags |= Style_Down;
	    if (checkbox->state() == QButton::On)
		flags |= Style_On;
	    else if (checkbox->state() == QButton::Off)
		flags |= Style_Off;
	    else if (checkbox->state() == QButton::NoChange)
		flags |= Style_NoChange;

	    drawPrimitive(PE_Indicator, p, ir, cg, flags, opt);
	    break;
	}

    case CE_CheckBoxLabel:
	{
	    const QCheckBox *checkbox = (const QCheckBox *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     flags & Style_Enabled, checkbox->pixmap(), checkbox->text());

	    if (checkbox->hasFocus()) {
		QRect fr = subRect(SR_CheckBoxFocusRect, widget);
		fr.moveBy(r.left() - 1, r.top());
		drawPrimitive(PE_FocusRect, p, fr, cg, flags);
	    }
	    break;
	}

    case CE_RadioButton:
	{
	    // many people expect to checkbox to be square, do that here.
	    QRect ir = r;

	    if (r.width() < r.height()) {
		ir.setTop(r.top() + (r.height() - r.width()) / 2);
		ir.setHeight(r.width());
	    } else if (r.height() < r.width()) {
		ir.setLeft(r.left() + (r.width() - r.height()) / 2);
		ir.setWidth(r.height());
	    }

	    const QRadioButton *radiobutton = (const QRadioButton *) widget;

	    if (radiobutton->isDown())
		flags |= Style_Down;
	    if (radiobutton->state() == QButton::On)
		flags |= Style_On;
	    else if (radiobutton->state() == QButton::Off)
		flags |= Style_Off;

	    drawPrimitive(PE_ExclusiveIndicator, p, ir, cg, flags, opt);
	    break;
	}

    case CE_RadioButtonLabel:
	{
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     flags & Style_Enabled, radiobutton->pixmap(), radiobutton->text());

	    if (radiobutton->hasFocus()) {
		QRect fr = subRect(SR_RadioButtonFocusRect, widget);
		fr.moveBy(r.left() - 1, r.top());
		drawPrimitive(PE_FocusRect, p, fr, cg, flags);
	    }
	    break;
	}

    case CE_ProgressBarGroove:
	p->setPen( Qt::black );
	p->drawRect( r );
	break;

#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarContents:
	{
	    const QProgressBar *progressbar = (const QProgressBar *) widget;

	    bool reverse = QApplication::reverseLayout();
	    if ( !progressbar->totalSteps() ) {
		// draw busy indicator
		int w = r.width();
		int x = progressbar->progress() % (w * 2);
		if (x > w)
		    x = 2 * w - x;
		x = reverse ? r.right() - x : x + r.x();
		p->setPen( QPen(cg.highlight(), 4) );
		p->drawLine(x, r.y() + 1, x, r.height() - 2);
	    } else {
		const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, widget);
		int u = (r.width() - 4) / unit_width;
		int p_v = progressbar->progress();
		int t_s = progressbar->totalSteps();

		if ( u > 0 && p_v >= INT_MAX / u && t_s >= u ) {
		    // scale down to something usable.
		    p_v /= u;
		    t_s /= u;
		}

		int nu = ( u * p_v + t_s / 2 ) / t_s;
		if (nu * unit_width > r.width() - 4)
		    nu--;

		// Draw nu units out of a possible u of unit_width width, each
		// a rectangle bordered by background color, all in a sunken panel
		// with a percentage text display at the end.
		int x = 0;
		int x0 = reverse ? r.right() - unit_width : r.x() + 2;
		for (int i=0; i<nu; i++) {
		    drawPrimitive( PE_ProgressBarChunk, p,
				   QRect( x0+x, r.y(), unit_width, r.height() ),
				   cg, Style_Default, opt );
		    x += reverse ? -unit_width: unit_width;
		}
	    }
	}
	break;

    case CE_ProgressBarLabel:
	{
	    const QProgressBar *progressbar = (const QProgressBar *) widget;
	    drawItem(p, r, AlignCenter | SingleLine, cg, progressbar->isEnabled(), 0,
		     progressbar->progressString());
	}
	break;
#endif // QT_NO_PROGRESSBAR

    default:
	break;

    }
}


/*!
  \reimp
*/
int QPocketPCStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret;

    switch (metric) {

#ifndef QT_NO_SLIDER
    case PM_SliderLength:
	ret = 10; // 11;
	break;

	// Returns the number of pixels to use for the business part of the
	// slider (i.e., the non-tickmark portion). The remaining space is shared
	// equally between the tickmark regions.
    case PM_SliderControlThickness:
	{
	    QSlider * sl = (QSlider *) widget;
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

    case PM_SplitterWidth:
	ret = 6; // QMAX( 6, QApplication::globalStrut().width() );
	break;

    case PM_ScrollBarSliderMin:
	ret = 9; // 9;
	break;

    case PM_ButtonMargin:
	ret = 8; // 6;
	break;

    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;

    case PM_MenuButtonIndicator:
	if (! widget)
	    ret = 8; // 12;
	else
	    ret = 8; // QMAX(12, (widget->height() - 4) / 3);
	break;

    case PM_ButtonShiftHorizontal:
	ret = 0;
	break;

    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_SpinBoxFrameWidth:
	ret = 1; // 2;
	break;

    case PM_DefaultFrameWidth:
	ret = 0; // 2;
	break;

    case PM_ScrollBarExtent:
	ret = 13; // 16;
	break;

    case PM_MaximumDragDistance:
	ret = -1;
	break;

#ifndef QT_NO_SLIDER
    case PM_SliderThickness:
	ret = 16; // 16;
	break;

    case PM_SliderTickmarkOffset:
	{
	    if (! widget) {
		ret = 0;
		break;
	    }

	    const QSlider * sl = (const QSlider *) widget;
	    int space = (sl->orientation() == Horizontal) ? sl->height() :
			sl->width();
	    int thickness = pixelMetric( PM_SliderControlThickness, sl );
	    int ticks = sl->tickmarks();

	    if ( ticks == QSlider::Both )
		ret = (space - thickness) / 2;
	    else if ( ticks == QSlider::Above )
		ret = space - thickness;
	    else
		ret = 0;
	    break;
	}

    case PM_SliderSpaceAvailable:
	{
	    const QSlider * sl = (const QSlider *) widget;
	    if ( sl->orientation() == Horizontal )
		ret = sl->width() - pixelMetric( PM_SliderLength, sl );
	    else
		ret = sl->height() - pixelMetric( PM_SliderLength, sl );
	    break;
	}
#endif // QT_NO_SLIDER

    case PM_DockWindowSeparatorExtent:
	ret = 3; // 6;
	break;

    case PM_DockWindowHandleExtent:
	ret = 8; // 11;
	break;

    case PM_DockWindowFrameWidth:
	ret = 1;
	break;

    case PM_MenuBarFrameWidth:
	ret = 0;
	break;

    case PM_TabBarTabOverlap:
	ret = 1;
	break;

    case PM_TabBarBaseHeight:
	ret = 0;
	break;

    case PM_TabBarBaseOverlap:
	ret = 1; // 0
	break;

    case PM_TabBarTabHSpace:
	ret = 20; // 24;
	break;

#ifndef QT_NO_TABBAR
    case PM_TabBarTabVSpace:
	{
	    const QTabBar * tb = (const QTabBar *) widget;
	    if ( tb->shape() == QTabBar::RoundedAbove ||
		 tb->shape() == QTabBar::RoundedBelow )
		ret = 10;
	    else
		ret = 0;
	    break;
	}
#endif

    case PM_ProgressBarChunkWidth:
	ret = 9;
	break;

    case PM_IndicatorWidth:
	ret = 15;
	break;

    case PM_IndicatorHeight:
	ret = 15;
	break;

    case PM_ExclusiveIndicatorWidth:
	ret = 15;
	break;

    case PM_ExclusiveIndicatorHeight:
	ret = 15;
	break;

    default:
	ret = 0;
	break;
    }

    return ret;
}


#define TITLEBAR_PAD 3
#define TITLEBAR_SEPARATION 1
#define TITLEBAR_PIXMAP_WIDTH 12
#define TITLEBAR_PIXMAP_HEIGHT 12
#define TITLEBAR_CONTROL_WIDTH (TITLEBAR_PAD+TITLEBAR_PIXMAP_WIDTH)
#define TITLEBAR_CONTROL_HEIGHT (TITLEBAR_PAD+TITLEBAR_PIXMAP_HEIGHT)


/*!
 \reimp
 */
QRect QPocketPCStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *widget,
					    SubControl sc,
					    const QStyleOption& opt ) const
{
    QRect rect;

#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::querySubControlMetrics: widget parameter cannot be zero!");
	return rect;
    }
#endif


    switch ( control ) {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	const QScrollBar *scrollbar = (const QScrollBar *) widget;

	const int buttonWidth = 22;
	const int barStart = 0;
	const int grooveStart = buttonWidth;
	int sliderStart = scrollbar->sliderStart();
	int sliderEnd = 0;
	int grooveEnd = 0;
	int barEnd = 0;
	int height = 0;

	if (scrollbar->orientation() == Qt::Horizontal) {
	    barEnd = scrollbar->width() - 1;
	    height = scrollbar->height() - 1;
	} else {
	    barEnd = scrollbar->height() - 1;
	    height = scrollbar->width() - 1;
	}

	grooveEnd = barEnd - buttonWidth;

	int maxlen = grooveEnd - buttonWidth + 1;
	uint range = scrollbar->maxValue() - scrollbar->minValue();

	// calculate slider length
	if (range) {
	    const int slidermin = 9;
	    int sliderlen = (scrollbar->pageStep() * maxlen) / (range + scrollbar->pageStep());
	    if ( sliderlen < slidermin || range > INT_MAX / 2 )
		sliderlen = slidermin;
	    if ( sliderlen > maxlen )
		sliderlen = maxlen;
	    sliderEnd = sliderStart + sliderlen;
	} else
	    sliderEnd = sliderStart + maxlen;

	int x1 = 0, x2 = 0;

	switch (sc) {
	    case SC_ScrollBarSubLine: // top/left button
		x1 = barStart; x2 = grooveStart; break;
	    case SC_ScrollBarSubPage: // between top/left button and slider
		x1 = grooveStart; x2 = sliderStart; break;
	    case SC_ScrollBarSlider:
		x1 = sliderStart; x2 = sliderEnd; break;
	    case SC_ScrollBarAddPage: // between bottom/right button and slider
		x1 = sliderEnd; x2 = grooveEnd; break;
	    case SC_ScrollBarAddLine: // bottom/right button
		x1 = grooveEnd; x2 = barEnd; break;
	    case SC_ScrollBarGroove:
		x1 = grooveStart; x2 = grooveEnd; break;
	    default:
		break;
	}

	// rotate the rectangle if it is vertical or not
	if (scrollbar->orientation() == Qt::Horizontal)
	    rect.setCoords(x1, 0, x2, height);
	else
	    rect.setCoords(0, x1, height, x2);

	break; }
#endif // QT_NO_SCROLLBAR









    case CC_SpinWidget: {
	int fw = pixelMetric( PM_SpinBoxFrameWidth, widget);
	QSize bs;
	bs.setHeight( widget->height()/2 - fw );
	if ( bs.height() < 8 )
	    bs.setHeight( 8 );
	bs.setWidth( bs.height() * 8 / 5 ); // 1.6 -approximate golden mean
	bs = bs.expandedTo( QApplication::globalStrut() );
	int y = fw;
	int x, lx, rx;
	x = widget->width() - y - bs.width();
	lx = fw;
	rx = x - fw;
	switch ( sc ) {
	case SC_SpinWidgetUp:
	    rect.setRect(x, y, bs.width(), bs.height());
	    break;
	case SC_SpinWidgetDown:
	    rect.setRect(x, y + bs.height(), bs.width(), bs.height());
	    break;
	case SC_SpinWidgetButtonField:
	    rect.setRect(x, y, bs.width(), widget->height() - 2*fw);
	    break;
	case SC_SpinWidgetEditField:
	    rect.setRect(lx, fw, rx, widget->height() - 2*fw);
	    break;
	case SC_SpinWidgetFrame:
	    return widget->rect();
	default:
	    break;
	}

	break; }

    case CC_ComboBox: {
	int x = 0, y = 0, wi = widget->width(), he = widget->height();
	int xpos = x;
	xpos += wi - 2 - 16;

	switch ( sc ) {
	case SC_ComboBoxFrame:
	    rect = widget->rect();
	    break;
	case SC_ComboBoxArrow:
	    rect.setRect(xpos, y+2, 16, he-4);
	    break;
	case SC_ComboBoxEditField:
	    rect.setRect(x+3, y+3, wi-6-16, he-6);
	    break;
	default:
	    break;
	}

	break; }


#ifndef QT_NO_SLIDER
    case CC_Slider:
	{
	    const QSlider * sl = (const QSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int thickness = pixelMetric( PM_SliderControlThickness, sl );

	    switch ( sc ) {
	    case SC_SliderHandle:
		{
		    const QSlider * sl = (const QSlider *) widget;
		    int sliderPos = 0;
		    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
		    int thickness  = pixelMetric( PM_SliderControlThickness, sl );
		    int len   = pixelMetric( PM_SliderLength, sl );

		    if ( !opt.isDefault() )
			sliderPos = sl->sliderStart();

		    if ( sl->orientation() == Horizontal )
			rect.setRect( sliderPos, tickOffset, len, thickness );
		    else
			rect.setRect( tickOffset, sliderPos, thickness, len );
		    break;
		}

	    case SC_SliderGroove:
		{
		    int x, y, wi, he;

		    if ( sl->orientation() == Horizontal ) {
			x = 0;
			y = tickOffset;
			wi = sl->width();
			he = thickness;
		    } else {
			x = tickOffset;
			y = 0;
			wi = thickness;
			he = sl->height();
		    }

		    rect.setRect(x, y, wi, he);
		    break;
		}

	    default:
		break;
	    }

	    break;
	}
#endif // QT_NO_SLIDER

#ifndef QT_NO_TOOLBUTTON
    case CC_ToolButton:
	{
	    const QToolButton *toolbutton = (const QToolButton *) widget;
	    int mbi = pixelMetric(PM_MenuButtonIndicator, widget);

	    rect = toolbutton->rect();

	    switch (sc) {
	    case SC_ToolButton:
		if (toolbutton->popup() && ! toolbutton->popupDelay())
		    rect.addCoords(0, 0, -mbi, 0);
		break;

	    case SC_ToolButtonMenu:
		if (toolbutton->popup() && ! toolbutton->popupDelay())
		    rect.addCoords(rect.width() - mbi, 0, 0, 0);
		break;

	    default:
		break;
	    }
	    break;
	}
#endif // QT_NO_TOOLBUTTON

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
	{
	    const QTitleBar *titlebar = (const QTitleBar *) widget;

	    switch (sc) {
	    case SC_TitleBarLabel:
		{
		    const QTitleBar *titlebar = (QTitleBar*)widget;
		    QRect ir( 0, 0, titlebar->width(), titlebar->height() );
		    if( titlebar->window() ) {
			if ( titlebar->window()->testWFlags( WStyle_Tool ) ) {
			    if ( titlebar->window()->testWFlags( WStyle_SysMenu ) )
				ir.addCoords( 0, 0, -TITLEBAR_CONTROL_WIDTH-TITLEBAR_SEPARATION-2, 0 );
			    if ( titlebar->window()->testWFlags( WStyle_MinMax ) )
				ir.addCoords( 0, 0, -TITLEBAR_CONTROL_WIDTH-2, 0 );
			} else {
			    if ( titlebar->window()->testWFlags( WStyle_SysMenu ) )
				ir.addCoords( TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH+2, 0,
					     -TITLEBAR_CONTROL_WIDTH-TITLEBAR_SEPARATION-2, 0 );
			    if ( titlebar->window()->testWFlags( WStyle_Minimize ) )
				ir.addCoords( 0, 0, -2*TITLEBAR_CONTROL_WIDTH-2, 0 );
			    else if ( titlebar->window()->testWFlags( WStyle_Maximize ) )
				ir.addCoords( 0, 0, -TITLEBAR_CONTROL_WIDTH-2, 0 );
			}
		    }
		    rect = ir;
		}
		break;

	    case SC_TitleBarCloseButton:
		rect.setRect(titlebar->width()-(TITLEBAR_CONTROL_WIDTH +
						TITLEBAR_SEPARATION), 2,
			     TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
		break;

	    case SC_TitleBarMaxButton:
	    case SC_TitleBarShadeButton:
	    case SC_TitleBarUnshadeButton:
		rect.setRect(titlebar->width()-((TITLEBAR_CONTROL_HEIGHT +
						 TITLEBAR_SEPARATION) * 2), 2,
			     TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
		break;

	    case SC_TitleBarMinButton:
	    case SC_TitleBarNormalButton:
		rect.setRect(titlebar->width()-((TITLEBAR_CONTROL_HEIGHT +
						 TITLEBAR_SEPARATION) * 3), 2,
			     TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
		break;

	    case SC_TitleBarSysMenu:
		rect.setRect(2 + TITLEBAR_SEPARATION, 2, TITLEBAR_CONTROL_WIDTH,
			     TITLEBAR_CONTROL_HEIGHT);
		break;

	    default:
		break;
	    }
	    break;
	}
#endif //QT_NO_TITLEBAR

    default:
	break;
    }

    return rect;
}


/*!
  \reimp
*/
QSize QPocketPCStyle::sizeFromContents( ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       const QStyleOption& opt ) const
{
    QSize sz(contentsSize);

#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::sizeFromContents: widget parameter cannot be zero!");
	return sz;
    }
#endif

    switch (contents) {

    case CT_PopupMenuItem:
	{
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
		    h += 2*pocketpcItemVMargin + 2*pocketpcItemFrame;
	    } else if ( mi->widget() ) {
	    } else if (mi->isSeparator()) {
		w = 10; // arbitrary
		h = pocketpcSepHeight;
		break;
	    } else {
		if (mi->pixmap())
		    h = QMAX(h, mi->pixmap()->height() + 2*pocketpcItemFrame);
		else if (! mi->text().isNull())
		    h = QMAX(h, popup->fontMetrics().height() + 2*pocketpcItemVMargin +
			     2*pocketpcItemFrame);

		if (mi->iconSet() != 0)
		    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height() +
			     2*pocketpcItemFrame);
	    }

	    if (! mi->text().isNull() && mi->text().find('\t') >= 0)
		w += pocketpcTabSpacing;
	    else if (mi->popup())
		w += 2*pocketpcArrowHMargin;

	    if (checkable && maxpmw < pocketpcCheckMarkWidth)
	        w += pocketpcCheckMarkWidth - maxpmw;
	    if (maxpmw)
		w += maxpmw + 6;
	    if (checkable || maxpmw > 0)
		w += pocketpcCheckMarkHMargin;
	    w += pocketpcRightBorder;

	    sz = QSize(w, h);
	    break;
	}

    case CT_PushButton:
	{
	    const QPushButton *button = (const QPushButton *) widget;
	    int w = contentsSize.width(),
		h = contentsSize.height(),
	       bm = pixelMetric(PM_ButtonMargin, widget),
	       fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;

	    w += bm + fw;
	    h += bm + fw;

	    if (button->isDefault() || button->autoDefault()) {
		int dbw = pixelMetric(PM_ButtonDefaultIndicator, widget) * 2;
		w += dbw;
		h += dbw;
	    }

	    sz = QSize(w, h);
	    break;
	}

    case CT_CheckBox:
	{
	    const QCheckBox *checkbox = (const QCheckBox *) widget;
	    QRect irect = subRect(SR_CheckBoxIndicator, widget);
	    int h = pixelMetric( PM_IndicatorHeight, widget );
	    sz += QSize(irect.right() + (checkbox->text().isEmpty() ? 0 : 10), 4 );
	    sz.setHeight( QMAX( sz.height(), h ) );
	    break;
	}

    case CT_RadioButton:
	{
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;
	    QRect irect = subRect(SR_RadioButtonIndicator, widget);
	    int h = pixelMetric( PM_ExclusiveIndicatorHeight, widget );
	    sz += QSize(irect.right() + (radiobutton->text().isEmpty() ? 0 : 10), 4 );
	    sz.setHeight( QMAX( sz.height(), h ) );
	    break;
	}

    case CT_ToolButton:
	{
	    sz = QSize(sz.width() + 7, sz.height() + 6);
	    break;
	}

    case CT_ComboBox:
	{
	    sz = QSize(sz.width() + 18, sz.height() + 5);
	    break;
	}

    case CT_ProgressBar:
	// just return the contentsSize for now
	// fall through intended

    default:
	break;
    }

    return sz;
}

/*! \reimp
*/
void QPocketPCStyle::polishPopupMenu( QPopupMenu* p)
{
#ifndef QT_NO_POPUPMENU
    p->setMouseTracking( TRUE );
    if ( !p->testWState( WState_Polished ) )
        p->setCheckable( TRUE );
    p->setLineWidth( 2 );
#endif
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

/*!
 \reimp
 */
QPixmap QPocketPCStyle::stylePixmap(StylePixmap stylepixmap,
				   const QWidget *widget,
				   const QStyleOption& opt) const
{
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
    default:
	break;
    }

    return QCommonStyle::stylePixmap(stylepixmap, widget, opt);
}


/*!\reimp
*/
void QPocketPCStyle::drawComplexControl( ComplexControl ctrl, QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags how,
					SCFlags sub,
					SCFlags subActive,
					const QStyleOption& opt ) const
{
    switch (ctrl) {
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
	if ( sub & SC_ComboBoxArrow ) {
	    SFlags flags = Style_Default;

	    p->setBrush( Qt::white );
	    p->setPen( Qt::black );
	    p->drawRect( r );

	    if ( widget->isEnabled() )
		flags |= Style_Enabled;

	    if ( subActive & Style_Sunken )
		flags |= Style_Sunken;

	    drawPrimitive( PE_ArrowDown, p, QRect( r.width() - 13, 0, 13, r.height() ), cg, flags );
	}

	if ( sub & SC_ComboBoxEditField ) {
	    const QComboBox * cb = (const QComboBox *) widget;

	    if ( cb->hasFocus() && !cb->editable() ) {
		p->fillRect( QRect( r.x() + 2, r.y() + 2, r.width() - 18, r.height() - 4 ), cg.brush( QColorGroup::Highlight ) );
		p->setPen( cg.highlightedText() );
		p->setBackgroundColor( cg.highlight() );
	    }
	}

	break;
#endif	// QT_NO_COMBOBOX






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

    case CC_SpinWidget:
	switch ( sub ) {
	case SC_SpinWidgetUp:
	case SC_SpinWidgetDown:
	    QCommonStyle::drawComplexControl( ctrl, p, widget, r, cg, how,
					      sub, subActive, opt );
	    break;
	case SC_SpinWidgetFrame:
	    qDrawWinPanel( p, r, cg, TRUE );
	    break;
	}
	break;

#ifndef QT_NO_SLIDER
    case CC_Slider:
	if ( sub & SC_SliderGroove ) {
	    const QSlider * sl = (const QSlider *) widget;

	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int thickness = pixelMetric( PM_SliderControlThickness, sl );
	    int mid   = thickness / 2;
	    int ticks = sl->tickmarks();
	    int len   = pixelMetric( PM_SliderLength, sl );
	    int x, y, wi, he;

	    if ( sl->orientation() == Horizontal ) {
		x = 0;
		y = tickOffset;
		wi = sl->width();
		he = thickness;
	    } else {
		x = tickOffset;
		y = 0;
		wi = thickness;
		he = sl->height();
	    }

	    if ( ticks & QSlider::Above )
		mid += len / 8;
	    if ( ticks & QSlider::Below )
		mid -= len / 8;

	    p->setPen( cg.shadow() );
	    if ( sl->orientation() == Horizontal ) {
		qDrawWinPanel( p, x, y + mid - 2,  wi, 4, cg, TRUE );
		p->drawLine( x+1, y + mid - 1, x + wi - 3, y + mid - 1 );
		((QSlider *) sl)->erase( 0, 0, sl->width(), tickOffset );
		((QSlider *) sl)->erase( 0, tickOffset + thickness, sl->width(), sl->height() );
	    } else {
		qDrawWinPanel( p, x + mid - 2, y, 4, he, cg, TRUE );
		p->drawLine( x + mid - 1, y + 1, x + mid - 1, y + he - 3 );
		((QSlider *) sl)->erase( 0, 0,  tickOffset, sl->height() );
		((QSlider *) sl)->erase( tickOffset + thickness, 0, sl->width(), sl->height() );
	    }
	}

	if ( sub & SC_SliderTickmarks )
	    QCommonStyle::drawComplexControl( ctrl, p, widget, r, cg, how,
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
	    //    const QColor c2 = g.button();
	    const QColor c3 = cg.midlight();
	    const QColor c4 = cg.light();

	    QRect re = querySubControlMetrics( CC_Slider, widget, SC_SliderHandle,
					       opt );
	    int x = re.x(), y = re.y(), wi = re.width(), he = re.height();

	    int x1 = x;
	    int x2 = x+wi-1;
	    int y1 = y;
	    int y2 = y+he-1;

	    bool reverse = QApplication::reverseLayout();

	    const QSlider * sl = (const QSlider *) widget;
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
#endif // QT_NO_SLIDER

	default:
	    QCommonStyle::drawComplexControl( ctrl, p, widget, r, cg, how, sub,
					      subActive, opt );
	break;
    }
}

#endif
#endif
