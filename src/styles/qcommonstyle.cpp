/****************************************************************************
** $Id$
**
** Implementation of the QCommonStyle class
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

#include "qcommonstyle.h"

#ifndef QT_NO_STYLE

#include "qmenubar.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpushbutton.h"
#include "qtabbar.h"
#include "qscrollbar.h"
#include "qtoolbutton.h"
#include "qtoolbar.h"
#include "qdockarea.h"
#include "qspinbox.h"
#include "qrangecontrol.h"
#include "qgroupbox.h"
#include "qslider.h"
#include "qlistview.h"
#include "qcheckbox.h"
#include "qradiobutton.h"
#include "qbitmap.h"
#include "qprogressbar.h"
#include <limits.h>
#include "../widgets/qtitlebar_p.h"

// NOT REVISED
/*!
  \class QCommonStyle qcommonstyle.h
  \brief The QCommonStyle class encapsulates the common Look and Feel of a GUI.

  \ingroup appearance

  This abstract class implements some of the widget's look and feel
  that is common to all GUI styles provided and shipped as part of Qt.

  All the functions are documented in \l QStyle.
*/

/*!
    \enum Qt::ArrowType

    \value UpArrow
    \value DownArrow
    \value LeftArrow
    \value RightArrow

*/

static QPainter *activePainter = 0;

/*!
  Constructs a QCommonStyle.
*/
QCommonStyle::QCommonStyle() : QStyle()
{
    activePainter = 0;
}

/*! \reimp */
QCommonStyle::~QCommonStyle()
{
    activePainter = 0;
}


#define TITLEBAR_PAD 3
#define TITLEBAR_SEPARATION 1
#define TITLEBAR_PIXMAP_WIDTH 12
#define TITLEBAR_PIXMAP_HEIGHT 12
#define TITLEBAR_CONTROL_WIDTH (TITLEBAR_PAD+TITLEBAR_PIXMAP_WIDTH)
#define TITLEBAR_CONTROL_HEIGHT (TITLEBAR_PAD+TITLEBAR_PIXMAP_HEIGHT)


/*! \reimp */
void QCommonStyle::drawPrimitive( PrimitiveElement pe,
				  QPainter *p,
				  const QRect &r,
				  const QColorGroup &cg,
				  SFlags flags,
				  const QStyleOption& opt ) const
{
    activePainter = p;

    switch (pe) {
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

    case PE_StatusBarSection:
	qDrawShadeRect( p, r, cg, TRUE, 1, 0, 0 );
	break;

    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_ButtonDropDown:
    case PE_HeaderSection:
	qDrawShadePanel(p, r, cg, flags & (Style_Sunken | Style_Down | Style_On) , 1,
			&cg.brush(QColorGroup::Button));
	break;

    case PE_Separator:
	qDrawShadeLine( p, r.left(), r.top(), r.right(), r.bottom(), cg,
			flags & Style_Sunken, 1, 0);
	break;

    case PE_FocusRect: {
	const QColor *bg = 0;

	if (!opt.isDefault())
	    bg = &opt.color();

	QPen oldPen = p->pen();

	if (bg) {
	    int h, s, v;
	    bg->hsv(&h, &s, &v);
	    if (v >= 128)
		p->setPen(Qt::black);
	    else
		p->setPen(Qt::white);
	} else
	    p->setPen(cg.foreground());

	if (flags & Style_FocusAtBorder)
	    p->drawRect(QRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2));
	else
	    p->drawRect(r);

	p->setPen(oldPen);
	break; }

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
	    break;
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

    case PE_Indicator: {
	if (flags & Style_NoChange) {
	    p->setPen(cg.foreground());
	    p->fillRect(r, cg.brush(QColorGroup::Button));
	    p->drawRect(r);
	    p->drawLine(r.topLeft(), r.bottomRight());
	} else
	    qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
			    cg, flags & (Style_Sunken | Style_On), 1,
			    &cg.brush(QColorGroup::Button));
	break; }

    case PE_IndicatorMask: {
	p->fillRect(r, color1);
	break; }

    case PE_ExclusiveIndicator: {
	QRect ir = r;
	p->setPen(cg.dark());
	p->drawArc(r, 0, 5760);

	if (flags & (Style_Sunken | Style_On)) {
	    ir.addCoords(2, 2, -2, -2);
	    p->setBrush(cg.foreground());
	    p->drawEllipse(ir);
	}

	break; }

    case PE_ExclusiveIndicatorMask: {
	p->setPen(color1);
	p->setBrush(color1);
	p->drawEllipse(r);
	break; }

    case PE_DockWindowHandle: {
	bool highlight = flags & Style_On;

	p->save();
	p->translate( r.x(), r.y() );
	if ( flags & Style_Horizontal ) {
	    if ( r.height() > 4 ) {
		qDrawShadePanel( p, 4, 2, 3, r.height() - 4,
				 cg, highlight, 1, 0 );
		qDrawShadePanel( p, 7, 2, 3, r.height() - 4,
				 cg, highlight, 1, 0 );
	    }
	} else {
	    if ( r.width() > 4 ) {
		qDrawShadePanel( p, 2, 4, r.width() - 4, 3,
				 cg, highlight, 1, 0 );
		qDrawShadePanel( p, 2, 7, r.width() - 4, 3,
				 cg, highlight, 1, 0 );
	    }
	}
	p->restore();
	break;
    }

    case PE_DockWindowSeparator: {
	QPoint p1, p2;
	if ( flags & Style_Horizontal ) {
	    p1 = QPoint( r.width()/2, 0 );
	    p2 = QPoint( p1.x(), r.height() );
	} else {
	    p1 = QPoint( 0, r.height()/2 );
	    p2 = QPoint( r.width(), p1.y() );
	}
	qDrawShadeLine( p, p1, p2, cg, 1, 1, 0 );
	break; }

    case PE_Panel:
    case PE_PanelPopup: {
	int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth)
		    : opt.lineWidth();

	qDrawShadePanel(p, r, cg, (flags & Style_Sunken), lw);
	break; }

    case PE_PanelDockWindow: {
	int lw = opt.isDefault() ? pixelMetric(PM_DockWindowFrameWidth)
		    : opt.lineWidth();

	qDrawShadePanel(p, r, cg, FALSE, lw);
	break; }

    case PE_PanelMenuBar: {
	int lw = opt.isDefault() ? pixelMetric(PM_MenuBarFrameWidth)
		    : opt.lineWidth();

	qDrawShadePanel(p, r, cg, FALSE, lw, &cg.brush(QColorGroup::Button));
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

    case PE_GroupBoxFrame: {
#ifndef QT_NO_FRAME
	if ( opt.isDefault() )
	    break;

	QFrame::Shape	type 	= (QFrame::Shape)opt.frameShape();
	QFrame::Shadow	cstyle 	= (QFrame::Shadow)opt.frameShadow();
	int 		lwidth 	= opt.lineWidth();
	int 		mlwidth = opt.midLineWidth();

	int x = r.x();
	int y = r.y();
	int w = r.width();
	int h = r.height();

	switch ( type ) {
	case QFrame::Box:
	    if ( cstyle == QFrame::Plain )
		qDrawPlainRect( p, x, y, w, h, cg.foreground(), lwidth );
	    else
		qDrawShadeRect( p, x, y, w, h, cg, cstyle == QFrame::Sunken,
				lwidth, mlwidth );
	    break;

	case QFrame::StyledPanel:
	    if ( cstyle == QFrame::Plain )
		qDrawPlainRect( p, x, y, w, h, cg.foreground(), lwidth );
	    break;

	case QFrame::PopupPanel:
	    if ( cstyle == QFrame::Plain )
		qDrawPlainRect( p, x, y, w, h, cg.foreground(), lwidth );
	    break;

	case QFrame::Panel:
	    if ( cstyle == QFrame::Plain )
		qDrawPlainRect( p, x, y, w, h, cg.foreground(), lwidth );
	    else
		qDrawShadePanel( p, x, y, w, h, cg, cstyle == QFrame::Sunken,
				 lwidth );
	    break;

	case QFrame::WinPanel:
	    if ( cstyle == QFrame::Plain )
		qDrawPlainRect( p, x, y, w, h, cg.foreground(), 2 );
	    else
		qDrawWinPanel( p, x, y, w, h, cg, cstyle == QFrame::Sunken );
	    break;
	default:
	    break;
	}
#endif
	break; }

    case PE_ProgressBarChunk:
	p->fillRect( r.x(), r.y() + 3, r.width() -2, r.height() - 6,
	    cg.brush(QColorGroup::Highlight));
	break;

    default:
	break;
    }

    activePainter = 0;
}

/*! \reimp */
void QCommonStyle::drawControl( ControlElement element,
				QPainter *p,
				const QWidget *widget,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags,
				const QStyleOption& opt ) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::drawControl: widget parameter cannot be zero!");
	return;
    }
#endif

    activePainter = p;

    switch (element) {
    case CE_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    QRect br = r;
	    int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);

	    if (button->isDefault() || button->autoDefault()) {
		if ( button->isDefault())
		    drawPrimitive(PE_ButtonDefault, p, br, cg, flags);

		br.setCoords(br.left()   + dbi,
			     br.top()    + dbi,
			     br.right()  - dbi,
			     br.bottom() - dbi);
	    }

	    drawPrimitive(PE_ButtonCommand, p, br, cg, flags);
#endif
	    break;
	}

    case CE_PushButtonLabel:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    QRect ir = r;

	    if (button->isDown() || button->isOn()) {
		flags |= Style_Sunken;
		ir.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
			  pixelMetric(PM_ButtonShiftVertical, widget));
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
		ir.setWidth(ir.width() - (pixw + 4));
	    }
	    drawItem(p, ir, AlignCenter | ShowPrefix, cg,
		     flags & Style_Enabled, button->pixmap(), button->text(),
		     button->text().length(), &(cg.buttonText()) );

	    if (flags & Style_HasFocus)
		drawPrimitive(PE_FocusRect, p, subRect(SR_PushButtonFocusRect, widget),
			      cg, flags);
#endif
	    break;
	}

    case CE_CheckBox:
	drawPrimitive(PE_Indicator, p, r, cg, flags, opt);
	break;

    case CE_CheckBoxLabel:
	{
#ifndef QT_NO_CHECKBOX
	    const QCheckBox *checkbox = (const QCheckBox *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     flags & Style_Enabled, checkbox->pixmap(), checkbox->text());

	    if (flags & Style_HasFocus) {
		QRect fr = visualRect(subRect(SR_CheckBoxFocusRect, widget), widget);
		drawPrimitive(PE_FocusRect, p, fr, cg, flags);
	    }
#endif
	    break;
	}

    case CE_RadioButton:
	drawPrimitive(PE_ExclusiveIndicator, p, r, cg, flags, opt);
	break;

    case CE_RadioButtonLabel:
	{
#ifndef QT_NO_RADIOBUTTON
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     flags & Style_Enabled, radiobutton->pixmap(), radiobutton->text());

	    if (flags & Style_HasFocus) {
		QRect fr = visualRect(subRect(SR_RadioButtonFocusRect, widget), widget);
		drawPrimitive(PE_FocusRect, p, fr, cg, flags);
	    }
#endif
	    break;
	}

#ifndef QT_NO_TABBAR
    case CE_TabBarTab:
	{
	    const QTabBar * tb = (const QTabBar *) widget;

	    if ( tb->shape() == QTabBar::TriangularAbove ||
		 tb->shape() == QTabBar::TriangularBelow ) {
		// triangular, above or below
		int y;
		int x;
		QPointArray a( 10 );
		a.setPoint( 0, 0, -1 );
		a.setPoint( 1, 0, 0 );
		y = r.height()-2;
		x = y/3;
		a.setPoint( 2, x++, y-1 );
		a.setPoint( 3, x++, y );
		a.setPoint( 3, x++, y++ );
		a.setPoint( 4, x, y );

		int i;
		int right = r.width() - 1;
		for ( i = 0; i < 5; i++ )
		    a.setPoint( 9-i, right - a.point( i ).x(), a.point( i ).y() );

		if ( tb->shape() == QTabBar::TriangularAbove )
		    for ( i = 0; i < 10; i++ )
			a.setPoint( i, a.point(i).x(),
				    r.height() - 1 - a.point( i ).y() );

		a.translate( r.left(), r.top() );

		if ( flags & Style_Selected )
		    p->setBrush( cg.base() );
		else
		    p->setBrush( cg.background() );
		p->setPen( cg.foreground() );
		p->drawPolygon( a );
		p->setBrush( NoBrush );
	    }
	    break;
	}

    case CE_TabBarLabel:
	{
	    if ( opt.isDefault() )
		break;

	    const QTabBar * tb = (const QTabBar *) widget;
	    QTab * t = opt.tab();

	    QRect tr = r;
	    if ( t->identifier() == tb->currentTab() )
		tr.setBottom( tr.bottom() -
			      pixelMetric( QStyle::PM_DefaultFrameWidth, tb ) );

	    drawItem( p, tr, AlignCenter | ShowPrefix, cg,
		      flags & Style_Enabled, 0, t->text() );

	    if ( (flags & Style_HasFocus) && !t->text().isEmpty() )
		drawPrimitive( PE_FocusRect, p, r, cg );
	    break;
	}
#endif // QT_NO_TABBAR

    case CE_ProgressBarGroove:
	qDrawShadePanel(p, r, cg, TRUE, 1, &cg.brush(QColorGroup::Background));
	break;

#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarContents:
	{
	    const QProgressBar *progressbar = (const QProgressBar *) widget;

	    bool reverse = QApplication::reverseLayout();
	    if ( !progressbar->totalSteps() ) {
		// draw busy indicator
		int w = r.width() - 4;
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
	    QColor penColor = cg.highlightedText();
	    QColor *pcolor = 0;
	    if ( progressbar->centerIndicator() && !progressbar->indicatorFollowsStyle() &&
		 progressbar->progress()*2 >= progressbar->totalSteps() )
		pcolor = &penColor;
	    drawItem(p, r, AlignCenter | SingleLine, cg, flags & Style_Enabled, 0,
		     progressbar->progressString(), -1, pcolor );
	}
	break;
#endif // QT_NO_PROGRESSBAR

    case CE_MenuBarItem:
	{
#ifndef QT_NO_MENUDATA
	    if (opt.isDefault())
		break;

	    QMenuItem *mi = opt.menuItem();
	    drawItem( p, r, AlignCenter|ShowPrefix|DontClip|SingleLine, cg,
		      flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		      &cg.buttonText() );
#endif
	    break;
	}

#ifndef QT_NO_TOOLBUTTON
    case CE_ToolButtonLabel:
	{
	    const QToolButton *toolbutton = (const QToolButton *) widget;
	    QRect rect = r;
	    Qt::ArrowType arrowType = opt.isDefault()
			? Qt::DownArrow : opt.arrowType();

	    if (flags & (Style_Down | Style_On))
		rect.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
			    pixelMetric(PM_ButtonShiftVertical, widget));

	    if (!opt.isDefault()) {
		PrimitiveElement pe;
		switch (arrowType) {
		case Qt::LeftArrow:  pe = PE_ArrowLeft;  break;
		case Qt::RightArrow: pe = PE_ArrowRight; break;
		case Qt::UpArrow:    pe = PE_ArrowUp;    break;
		default:
		case Qt::DownArrow:  pe = PE_ArrowDown;  break;
		}

		drawPrimitive(pe, p, rect, cg, flags, opt);
	    } else {
		QColor btext = cg.buttonText();

		if (toolbutton->iconSet().isNull() &&
		    ! toolbutton->text().isNull() &&
		    ! toolbutton->usesTextLabel()) {
		    drawItem(p, rect, AlignCenter | ShowPrefix, cg,
			     flags & Style_Enabled, 0, toolbutton->text(),
			     toolbutton->text().length(), &btext);
		} else {
		    QPixmap pm;
		    QIconSet::Size size =
			toolbutton->usesBigPixmap() ? QIconSet::Large : QIconSet::Small;
		    QIconSet::State state =
			toolbutton->isOn() ? QIconSet::On : QIconSet::Off;
		    QIconSet::Mode mode;
		    if (! toolbutton->isEnabled())
			mode = QIconSet::Disabled;
		    else if (flags & (Style_Down | Style_On | Style_Raised))
			mode = QIconSet::Active;
		    else
			mode = QIconSet::Normal;
		    pm = toolbutton->iconSet().pixmap( size, mode, state );

		    if ( toolbutton->usesTextLabel() ) {
			p->setFont( toolbutton->font() );

			QRect pr = rect, tr = rect;
			int fh = p->fontMetrics().height();
			pr.addCoords( 0, 1, 0, -fh-3 );
			tr.addCoords( 0, pr.bottom(), 0, -3 );
			drawItem( p, pr, AlignCenter, cg, TRUE, &pm, QString::null );
			drawItem( p, tr, AlignCenter | ShowPrefix, cg,
				  flags & Style_Enabled, 0, toolbutton->textLabel(),
				  toolbutton->textLabel().length(), &btext);
		    } else
			drawItem( p, rect, AlignCenter, cg, TRUE, &pm, QString::null );
		}
	    }

	    break;
	}
#endif // QT_NO_TOOLBUTTON

    default:
	break;
    }

    activePainter = 0;
}

/*! \reimp */
void QCommonStyle::drawControlMask( ControlElement control,
				    QPainter *p,
				    const QWidget *widget,
				    const QRect &r,
				    const QStyleOption& opt ) const
{
    Q_UNUSED(widget);

    activePainter = p;

    QColorGroup cg(color1,color1,color1,color1,color1,color1,color1,color1,color0);

    switch (control) {
    case CE_PushButton:
	drawPrimitive(PE_ButtonCommand, p, r, cg, Style_Default, opt);
	break;

    case CE_CheckBox:
	drawPrimitive(PE_IndicatorMask, p, r, cg, Style_Default, opt);
	break;

    case CE_RadioButton:
	drawPrimitive(PE_ExclusiveIndicatorMask, p, r, cg, Style_Default, opt);
	break;

    default:
	p->fillRect(r, color1);
	break;
    }

    activePainter = 0;
}

/*! \reimp */
QRect QCommonStyle::subRect(SubRect r, const QWidget *widget) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::subRect: widget parameter cannot be zero!");
	return QRect();
    }
#endif

    QRect rect, wrect(widget->rect());

    switch (r) {
    case SR_PushButtonContents:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    int dx1, dx2;

	    dx1 = pixelMetric(PM_DefaultFrameWidth, widget);
	    if (button->isDefault() || button->autoDefault())
		dx1 += pixelMetric(PM_ButtonDefaultIndicator, widget);
	    dx2 = dx1 * 2;

	    rect.setRect(wrect.x()      + dx1,
			 wrect.y()      + dx1,
			 wrect.width()  - dx2,
			 wrect.height() - dx2);
#endif
	    break;
	}

    case SR_PushButtonFocusRect:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    int dbw1 = 0, dbw2 = 0;
	    if (button->isDefault() || button->autoDefault()) {
		dbw1 = pixelMetric(PM_ButtonDefaultIndicator, widget);
		dbw2 = dbw1 * 2;
	    }

	    int dfw1 = pixelMetric(PM_DefaultFrameWidth, widget) * 2,
		dfw2 = dfw1 * 2;

	    rect.setRect(wrect.x()      + dfw1 + dbw1,
			 wrect.y()      + dfw1 + dbw1,
			 wrect.width()  - dfw2 - dbw2,
			 wrect.height() - dfw2 - dbw2);
#endif
	    break;
	}

    case SR_CheckBoxIndicator:
	{
	    int h = pixelMetric( PM_IndicatorHeight );
	    rect.setRect(0, ( wrect.height() - h ) / 2,
			 pixelMetric( PM_IndicatorWidth ), h );
	    break;
	}

    case SR_CheckBoxContents:
	{
#ifndef QT_NO_CHECKBOX
	    QRect ir = subRect(SR_CheckBoxIndicator, widget);
	    rect.setRect(ir.right() + 10, wrect.y(),
			 wrect.width() - ir.width() - 10, wrect.height());
#endif
	    break;
	}

    case SR_CheckBoxFocusRect:
	{
#ifndef QT_NO_CHECKBOX
	    const QCheckBox *checkbox = (const QCheckBox *) widget;
	    QRect cr = subRect(SR_CheckBoxContents, widget);

	    // don't create a painter if we have an active one
	    QPainter *p = 0;
	    if (! activePainter)
		p = new QPainter(checkbox);
	    rect = itemRect((activePainter ? activePainter : p),
			    cr, AlignLeft | AlignVCenter | ShowPrefix,
			    checkbox->isEnabled(),
			    checkbox->pixmap(),
			    checkbox->text());

	    delete p;

	    rect.addCoords( -3, -2, 3, 2 );
	    rect = rect.intersect(wrect);
#endif
	    break;
	}

    case SR_RadioButtonIndicator:
	{
	    int h = pixelMetric( PM_ExclusiveIndicatorHeight );
	    rect.setRect(0, ( wrect.height() - h ) / 2,
			 pixelMetric( PM_ExclusiveIndicatorWidth ), h );
	    break;
	}

    case SR_RadioButtonContents:
	{
	    QRect ir = subRect(SR_RadioButtonIndicator, widget);
	    rect.setRect(ir.right() + 10, wrect.y(),
			 wrect.width() - ir.width() - 10, wrect.height());
	    break;
	}

    case SR_RadioButtonFocusRect:
	{
#ifndef QT_NO_RADIOBUTTON
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;
	    QRect cr = subRect(SR_RadioButtonContents, widget);

	    // don't create a painter if we have an active one
	    QPainter *p = 0;
	    if (! activePainter)
		p = new QPainter(radiobutton);
	    rect = itemRect((activePainter ? activePainter : p),
			    cr, AlignLeft | AlignVCenter | ShowPrefix,
			    radiobutton->isEnabled(),
			    radiobutton->pixmap(),
			    radiobutton->text());
	    delete p;

	    rect.addCoords( -3, -2, 3, 2 );
	    rect = rect.intersect(wrect);
#endif
	    break;
	}

    case SR_ComboBoxFocusRect:
	rect.setRect(3, 3, widget->width()-6-16, widget->height()-6);
	break;

#ifndef QT_NO_SLIDER
    case SR_SliderFocusRect:
	{
	    const QSlider * sl = (const QSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int thickness  = pixelMetric( PM_SliderControlThickness, sl );

	    if ( sl->orientation() == Horizontal )
		rect.setRect( 0, tickOffset-1, sl->width(), thickness+2 );
	    else
		rect.setRect( tickOffset-1, 0, thickness+2, sl->height() );
	    rect = rect.intersect( sl->rect() ); // ## is this really necessary?
	    break;
	}
#endif // QT_NO_SLIDER

#ifndef QT_NO_MAINWINDOW
    case SR_DockWindowHandleRect:
	{
	    if (! widget->parentWidget())
		break;

	    const QDockWindow * dw = (const QDockWindow *) widget->parentWidget();

	    if ( !dw->area() || !dw->isCloseEnabled() )
		rect.setRect( 0, 0, widget->width(), widget->height() );
	    else {
		if ( dw->area()->orientation() == Horizontal )
		    rect.setRect(0, 15, widget->width(), widget->height() - 15);
		else
		    rect.setRect(0, 1, widget->width() - 15, widget->height() - 1);
	    }
	    break;
	}
#endif // QT_NO_MAINWINDOW

    case SR_ProgressBarGroove:
    case SR_ProgressBarContents:
	{
#ifndef QT_NO_PROGRESSBAR
	    QFontMetrics fm( ( widget ? widget->fontMetrics() :
			       QApplication::fontMetrics() ) );
	    const QProgressBar *progressbar = (const QProgressBar *) widget;
	    int textw = 0;
	    if (progressbar->percentageVisible())
		textw = fm.width("100%") + 6;

	    if (progressbar->indicatorFollowsStyle() ||
		! progressbar->centerIndicator())
		rect.setCoords(wrect.left(), wrect.top(),
			       wrect.right() - textw, wrect.bottom());
	    else
		rect = wrect;
#endif
	    break;
	}

    case SR_ProgressBarLabel:
	{
#ifndef QT_NO_PROGRESSBAR
	    QFontMetrics fm( ( widget ? widget->fontMetrics() :
			       QApplication::fontMetrics() ) );
	    const QProgressBar *progressbar = (const QProgressBar *) widget;
	    int textw = 0;
	    if (progressbar->percentageVisible())
		textw = fm.width("100%") + 6;

	    if (progressbar->indicatorFollowsStyle() ||
		! progressbar->centerIndicator())
		rect.setCoords(wrect.right() - textw, wrect.top(),
			       wrect.right(), wrect.bottom());
	    else
		rect = wrect;
#endif
	    break;
	}

    case SR_ToolButtonContents:
	rect = querySubControlMetrics(CC_ToolButton, widget, SC_ToolButton);
	break;

    default:
	rect = wrect;
	break;
    }

    return rect;
}

#ifndef QT_NO_RANGECONTROL
/*
  I really need this and I don't want to expose it in QRangeControl..
*/
static int qPositionFromValue( const QRangeControl * rc, int logical_val,
			       int span )
{
    if ( span <= 0 || logical_val < rc->minValue() ||
	 rc->maxValue() <= rc->minValue() )
	return 0;
    if ( logical_val > rc->maxValue() )
	return span;

    uint range = rc->maxValue() - rc->minValue();
    uint p = logical_val - rc->minValue();

    if ( range > (uint)INT_MAX/4096 ) {
	const int scale = 4096*2;
	return ( (p/scale) * span ) / (range/scale);
	// ### the above line is probably not 100% correct
	// ### but fixing it isn't worth the extreme pain...
    } else if ( range > (uint)span ) {
	return (2*p*span + range) / (2*range);
    } else {
	uint div = span / range;
	uint mod = span % range;
	return p*div + (2*p*mod + range) / (2*range);
    }
    //equiv. to (p*span)/range + 0.5
    // no overflow because of this implicit assumption:
    // span <= 4096
}
#endif // QT_NO_RANGECONTROL

/*! \reimp */
void QCommonStyle::drawComplexControl( ComplexControl control,
				       QPainter *p,
				       const QWidget *widget,
				       const QRect &r,
				       const QColorGroup &cg,
				       SFlags flags,
				       SCFlags controls,
				       SCFlags active,
				       const QStyleOption& opt ) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::drawComplexControl: widget parameter cannot be zero!");
	return;
    }
#endif

    activePainter = p;

    switch (control) {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    QRect addline, subline, addpage, subpage, slider, first, last;
	    bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	    subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
	    addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
	    subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
	    addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
	    slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
	    first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
	    last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);

       	    if ((controls & SC_ScrollBarSubLine) && subline.isValid())
		drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarAddLine) && addline.isValid())
		drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
		drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
       	    if ((controls & SC_ScrollBarFirst) && first.isValid())
		drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarFirst) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarLast) && last.isValid())
		drawPrimitive(PE_ScrollBarLast, p, last, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarLast) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
		drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSlider) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));

		// ### perhaps this should not be able to accept focus if maxedOut?
		if (scrollbar->hasFocus()) {
		    QRect fr(slider.x() + 2, slider.y() + 2,
			     slider.width() - 5, slider.height() - 5);
		    drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
		}
	    }

	    break;
	}
#endif // QT_NO_SCROLLBAR

#ifndef QT_NO_TOOLBUTTON
    case CC_ToolButton:
	{
	    const QToolButton *toolbutton = (const QToolButton *) widget;

	    QRect button, menuarea;
	    button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
	    menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

	    SFlags bflags = flags,
		   mflags = flags;

	    if (active & SC_ToolButton)
		bflags |= Style_Down;
	    if (active & SC_ToolButtonMenu)
		mflags |= Style_Down;

	    if (controls & SC_ToolButton) {
		if (bflags & (Style_Down | Style_On | Style_Raised))
		    drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);
		else if ( toolbutton->parentWidget() &&
			  toolbutton->parentWidget()->backgroundPixmap() &&
			  ! toolbutton->parentWidget()->backgroundPixmap()->isNull() ) {
		    QPixmap pixmap =
			*(toolbutton->parentWidget()->backgroundPixmap());

		    p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
		}
	    }

	    if (controls & SC_ToolButtonMenu) {
		if (mflags & (Style_Down | Style_On | Style_Raised))
		    drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
		drawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, opt);
	    }

	    if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
		QRect fr = toolbutton->rect();
		fr.addCoords(3, 3, -3, -3);
		drawPrimitive(PE_FocusRect, p, fr, cg);
	    }

	    break;
	}
#endif // QT_NO_TOOLBUTTON

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
	{
	    const QTitleBar *titlebar = (const QTitleBar *) widget;
	    if ( controls & SC_TitleBarLabel ) {
		QColorGroup cgroup = titlebar->isActive() ?
		    titlebar->palette().active() : titlebar->palette().inactive();

		QColor left = cgroup.highlight();
		QColor right = cgroup.base();

		if ( left != right ) {
		    double rS = left.red();
		    double gS = left.green();
		    double bS = left.blue();

		    const double rD = double(right.red() - rS) / titlebar->width();
		    const double gD = double(right.green() - gS) / titlebar->width();
		    const double bD = double(right.blue() - bS) / titlebar->width();

		    const int w = titlebar->width();
		    for ( int sx = 0; sx < w; sx++ ) {
			rS+=rD;
			gS+=gD;
			bS+=bD;
			p->setPen( QColor( (int)rS, (int)gS, (int)bS ) );
			p->drawLine( sx, 0, sx, titlebar->height() );
		    }
		} else {
		    p->fillRect( titlebar->rect(), left );
		}

		QRect ir = querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarLabel );

		p->setPen( cgroup.highlightedText() );
		p->drawText(ir.x()+2, ir.y(), ir.width(), ir.height(),
			    AlignAuto | AlignVCenter | SingleLine, titlebar->visibleText() );
	    }

	    if ( titlebar->window() ) {
		QRect ir;
		bool down = FALSE;
		QPixmap pm;

		if ( controls & SC_TitleBarCloseButton ) {
		    ir = querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarCloseButton );
		    down = active & SC_TitleBarCloseButton;
		    pm = stylePixmap(SP_TitleBarCloseButton, widget);
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);

		    if( down )
			ir.addCoords( pixelMetric(PM_ButtonShiftHorizontal), pixelMetric(PM_ButtonShiftVertical), 0, 0 );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		}

		if ( controls & SC_TitleBarMaxButton ) {
		    ir = querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarMaxButton );

		    down = active & SC_TitleBarMaxButton;
		    pm = QPixmap(stylePixmap(SP_TitleBarMaxButton, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);

		    if( down )
			ir.addCoords( pixelMetric(PM_ButtonShiftHorizontal), pixelMetric(PM_ButtonShiftVertical), 0, 0 );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		}

		if ( controls & SC_TitleBarNormalButton || controls & SC_TitleBarMinButton ) {
		    ir = querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarMinButton );
		    QStyle::SubControl ctrl = (controls & SC_TitleBarNormalButton ?
					       SC_TitleBarNormalButton :
					       SC_TitleBarMinButton);
		    QStyle::StylePixmap spixmap = (controls & SC_TitleBarNormalButton ?
						   SP_TitleBarNormalButton :
						   SP_TitleBarMinButton);
		    down = active & ctrl;
		    pm = QPixmap(stylePixmap(spixmap, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);

		    if( down )
			ir.addCoords( pixelMetric(PM_ButtonShiftHorizontal), pixelMetric(PM_ButtonShiftVertical), 0, 0 );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		}

		if ( controls & SC_TitleBarShadeButton ) {
		    ir = querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarShadeButton );

		    down = active & SC_TitleBarShadeButton;
		    pm = QPixmap(stylePixmap(SP_TitleBarShadeButton, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);
		    if( down )
			ir.addCoords( pixelMetric(PM_ButtonShiftHorizontal), pixelMetric(PM_ButtonShiftVertical), 0, 0 );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		}

		if ( controls & SC_TitleBarUnshadeButton ) {
		    ir = querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarUnshadeButton );

		    down = active & SC_TitleBarUnshadeButton;
		    pm = QPixmap(stylePixmap(SP_TitleBarUnshadeButton, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);
		    if( down )
			ir.addCoords( pixelMetric(PM_ButtonShiftHorizontal), pixelMetric(PM_ButtonShiftVertical), 0, 0 );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		}

#ifndef QT_NO_WIDGET_TOPEXTRA
		if ( controls & SC_TitleBarSysMenu ) {
		    if ( titlebar->icon() ) {
			ir = querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarSysMenu );
			drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, titlebar->icon(), QString::null );
		    }
		}
#endif
	    }

	    break;
	}
#endif //QT_NO_TITLEBAR

    case CC_SpinWidget: {
#ifndef QT_NO_SPINWIDGET
	const QSpinWidget * sw = (const QSpinWidget *) widget;
	SFlags flags;
	PrimitiveElement pe;

	if ( controls & SC_SpinWidgetFrame )
	    qDrawWinPanel( p, r, cg, TRUE ); //cstyle == Sunken );

	if ( controls & SC_SpinWidgetUp ) {
	    flags = Style_Default | Style_Enabled;
	    if (active == SC_SpinWidgetUp ) {
		flags |= Style_On;
		flags |= Style_Sunken;
	    } else
		flags |= Style_Raised;
	    if ( sw->buttonSymbols() == QSpinWidget::PlusMinus )
		pe = PE_SpinWidgetPlus;
	    else
		pe = PE_SpinWidgetUp;

	    QRect re = sw->upRect();
	    QColorGroup ucg = sw->isUpEnabled() ? cg : sw->palette().disabled();
	    drawPrimitive(PE_ButtonBevel, p, re, ucg, flags);
	    drawPrimitive(pe, p, re, ucg, flags);
	}

	if ( controls & SC_SpinWidgetDown ) {
	    flags = Style_Default | Style_Enabled;
	    if (active == SC_SpinWidgetDown ) {
		flags |= Style_On;
		flags |= Style_Sunken;
	    } else
		flags |= Style_Raised;
	    if ( sw->buttonSymbols() == QSpinWidget::PlusMinus )
		pe = PE_SpinWidgetMinus;
	    else
		pe = PE_SpinWidgetDown;

	    QRect re = sw->downRect();
	    QColorGroup dcg = sw->isDownEnabled() ? cg : sw->palette().disabled();
	    drawPrimitive(PE_ButtonBevel, p, re, dcg, flags);
	    drawPrimitive(pe, p, re, dcg, flags);
	}
#endif
	break; }

#ifndef QT_NO_SLIDER
    case CC_Slider:
	switch ( controls ) {
	case SC_SliderTickmarks: {
	    const QSlider * sl = (const QSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int ticks = sl->tickmarks();
	    int thickness = pixelMetric( PM_SliderControlThickness, sl );
	    int len = pixelMetric( PM_SliderLength, sl );
	    int available = pixelMetric( PM_SliderSpaceAvailable, sl );
	    int interval = sl->tickInterval();

	    if ( interval <= 0 ) {
		interval = sl->lineStep();
		if ( qPositionFromValue( sl, interval, available ) -
		     qPositionFromValue( sl, 0, available ) < 3 )
		    interval = sl->pageStep();
	    }

	    int fudge = len / 2 + 1;
	    int pos;

	    if ( ticks & QSlider::Above ) {
		if (sl->orientation() == Horizontal)
		    p->fillRect(0, 0, sl->width(), tickOffset,
				cg.brush(QColorGroup::Background));
		else
		    p->fillRect(0, 0, tickOffset, sl->width(),
				cg.brush(QColorGroup::Background));
		p->setPen( cg.foreground() );
		int v = sl->minValue();
		if ( !interval )
		    interval = 1;
		while ( v <= sl->maxValue() + 1 ) {
		    pos = qPositionFromValue( sl, v, available ) + fudge;
		    if ( sl->orientation() == Horizontal )
			p->drawLine( pos, 0, pos, tickOffset-2 );
		    else
			p->drawLine( 0, pos, tickOffset-2, pos );
		    v += interval;
		}
	    }

	    if ( ticks & QSlider::Below ) {
		int avail = (sl->orientation() == Horizontal) ? sl->height() :
			    sl->width();
		avail -= tickOffset + thickness;

		if (sl->orientation() == Horizontal)
		    p->fillRect(0, tickOffset + thickness, sl->width(), tickOffset,
				cg.brush(QColorGroup::Background));
		else
		    p->fillRect(tickOffset + thickness, 0, tickOffset, sl->height(),
				cg.brush(QColorGroup::Background));
		p->setPen( cg.foreground() );
		int v = sl->minValue();
		if ( !interval )
		    interval = 1;
		while ( v <= sl->maxValue() + 1 ) {
		    pos = qPositionFromValue( sl, v, available ) + fudge;
		    if ( sl->orientation() == Horizontal )
			p->drawLine( pos, tickOffset+thickness+1, pos,
				     tickOffset+thickness+1 + available-2 );
		    else
			p->drawLine( tickOffset+thickness+1, pos,
				     tickOffset+thickness+1 + available-2,
				     pos );
		    v += interval;
		}

	    }

	    break; }
	}
	break;
#endif // QT_NO_SLIDER

    default:
	break;
    }

    activePainter = 0;
}


/*! \reimp */
void QCommonStyle::drawComplexControlMask( ComplexControl control,
					   QPainter *p,
					   const QWidget *widget,
					   const QRect &r,
					   const QStyleOption& opt ) const
{
    Q_UNUSED(control);
    Q_UNUSED(widget);
    Q_UNUSED(opt);

    p->fillRect(r, color1);
}


/*! \reimp */
QRect QCommonStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *widget,
					    SubControl sc,
					    const QStyleOption&  ) const
{

    QRect rect;

#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::querySubControlMetrics: widget parameter cannot be zero!");
	return rect;
    }
#endif


    switch ( control ) {
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
	    rect = widget->rect();
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

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	const QScrollBar *scrollbar = (const QScrollBar *) widget;
	int sliderstart = 0;
	int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
	int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
		      scrollbar->width() : scrollbar->height()) - (sbextent * 2);
	int sliderlen;

	sliderstart = scrollbar->sliderStart();

	// calculate slider length
	if (scrollbar->maxValue() != scrollbar->minValue()) {
	    uint range = scrollbar->maxValue() - scrollbar->minValue();
	    sliderlen = (scrollbar->pageStep() * maxlen) /
			(range + scrollbar->pageStep());

	    int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
	    if ( sliderlen < slidermin || range > INT_MAX / 2 )
		sliderlen = slidermin;
	    if ( sliderlen > maxlen )
		sliderlen = maxlen;
	} else
	    sliderlen = maxlen;

	switch (sc) {
	case SC_ScrollBarSubLine:
	    // top/left button
	    rect.setRect(0, 0, sbextent, sbextent);
	    break;

	case SC_ScrollBarAddLine:
	    // bottom/right button
	    if (scrollbar->orientation() == Qt::Horizontal)
		rect.setRect(scrollbar->width() - sbextent, 0, sbextent, sbextent);
	    else
		rect.setRect(0, scrollbar->height() - sbextent, sbextent, sbextent);
	    break;

	case SC_ScrollBarSubPage:
	    // between top/left button and slider
	    if (scrollbar->orientation() == Qt::Horizontal)
		rect.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
	    else
		rect.setRect(0, sbextent, sbextent, sliderstart - sbextent);
	    break;

	case SC_ScrollBarAddPage:
	    // between bottom/right button and slider
	    if (scrollbar->orientation() == Qt::Horizontal)
		rect.setRect(sliderstart + sliderlen, 0,
			     maxlen - sliderstart - sliderlen + sbextent, sbextent);
	    else
		rect.setRect(0, sliderstart + sliderlen,
			     sbextent, maxlen - sliderstart - sliderlen + sbextent);
	    break;

	case SC_ScrollBarGroove:
	    if (scrollbar->orientation() == Qt::Horizontal)
		rect.setRect(sbextent, 0, scrollbar->width() - sbextent * 2,
			     scrollbar->height());
	    else
		rect.setRect(0, sbextent, scrollbar->width(),
			     scrollbar->height() - sbextent * 2);
	    break;

	case SC_ScrollBarSlider:
	    if (scrollbar->orientation() == Qt::Horizontal)
		rect.setRect(sliderstart, 0, sliderlen, sbextent);
	    else
		rect.setRect(0, sliderstart, sbextent, sliderlen);

	    break;

	default:
	    break;
	}

	break; }
#endif // QT_NO_SCROLLBAR

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

#if !defined(QT_NO_TOOLBUTTON) && !defined(QT_NO_POPUPMENU)
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
#endif // QT_NO_TOOLBUTTON && QT_NO_POPUPMENU

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


/*! \reimp */
QStyle::SubControl QCommonStyle::querySubControl(ComplexControl control,
						 const QWidget *widget,
						 const QPoint &pos,
						 const QStyleOption& opt ) const
{
    SubControl ret = SC_None;

    switch (control) {
#ifndef QT_NO_LISTVIEW
    case CC_ListView:
	{
	    if(pos.x() >= 0 && pos.x() <
	       opt.listViewItem()->listView()->treeStepSize())
		ret = SC_ListViewExpand;
	    break;
	}
#endif

    case CC_ScrollBar:
	{
	    QRect r;
	    uint ctrl = SC_ScrollBarAddLine;

	    // we can do this because subcontrols were designed to be masks as well...
	    while (ret == SC_None && ctrl <= SC_ScrollBarGroove) {
		r = querySubControlMetrics(control, widget,
					   (QStyle::SubControl) ctrl, opt);
		if (r.isValid() && r.contains(pos))
		    ret = (QStyle::SubControl) ctrl;

		ctrl <<= 1;
	    }

	    break;
	}

    case CC_TitleBar:
	{
#ifndef QT_NO_TITLEBAR
	    const QTitleBar *titlebar = (QTitleBar*)widget;
	    QRect r;
	    uint ctrl = SC_TitleBarLabel;

	    // we can do this because subcontrols were designed to be masks as well...
	    while (ret == SC_None && ctrl <= SC_TitleBarUnshadeButton) {
		r = querySubControlMetrics( control, widget, (QStyle::SubControl) ctrl, opt );
		if (r.isValid() && r.contains(pos))
		    ret = (QStyle::SubControl) ctrl;

		ctrl <<= 1;
	    }
	    if ( titlebar->window() ) {
		if ( ret == SC_TitleBarMaxButton && titlebar->window()->testWFlags( WStyle_Tool ) ) {
		    if ( titlebar->window()->isMinimized() )
			ret = SC_TitleBarUnshadeButton;
		    else
			ret = SC_TitleBarShadeButton;
		} else if ( ret == SC_TitleBarMinButton && !titlebar->window()->testWFlags( WStyle_Tool ) ) {
		    if ( titlebar->window()->isMinimized() )
			ret = QStyle::SC_TitleBarNormalButton;
		}
	    }
#endif
	    break;
	}

    default:
	break;
    }

    return ret;
}


/*! \reimp */
int QCommonStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
    int ret;

    switch (m) {
    case PM_TitleBarHeight: {
	ret = QMAX( widget->fontMetrics().lineSpacing(), 18 );
	if ( widget )
	    ret = QMAX( ret, widget->fontMetrics().lineSpacing() );
	break; }
    case PM_ScrollBarSliderMin:
	ret = 9;
	break;

    case PM_ButtonMargin:
	ret = 6;
	break;

    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;

    case PM_MenuButtonIndicator:
	if (! widget)
	    ret = 12;
	else
	    ret = QMAX(12, (widget->height() - 4) / 3);
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_SpinBoxFrameWidth:
    case PM_DefaultFrameWidth:
	ret = 2;
	break;

    case PM_ScrollBarExtent:
	ret = 16;
	break;

    case PM_MaximumDragDistance:
	ret = -1;
	break;

#ifndef QT_NO_SLIDER
    case PM_SliderThickness:
	ret = 16;
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
	ret = 6;
	break;

    case PM_DockWindowHandleExtent:
	ret = 11;
	break;

    case PM_DockWindowFrameWidth:
	ret = 1;
	break;

    case PM_MenuBarFrameWidth:
	ret = 2;
	break;

    case PM_TabBarTabOverlap:
	ret = 3;
	break;

    case PM_TabBarBaseHeight:
	ret = 0;
	break;

    case PM_TabBarBaseOverlap:
	ret = 0;
	break;

    case PM_TabBarTabHSpace:
	ret = 24;
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
	ret = 13;
	break;

    case PM_IndicatorHeight:
	ret = 13;
	break;

    case PM_ExclusiveIndicatorWidth:
	ret = 12;
	break;

    case PM_ExclusiveIndicatorHeight:
	ret = 12;
	break;

    default:
	ret = 0;
	break;
    }

    return ret;
}


/*! \reimp */
QSize QCommonStyle::sizeFromContents(ContentsType contents,
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
    case CT_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
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
#endif
	    break;
	}

    case CT_CheckBox:
	{
#ifndef QT_NO_CHECKBOX
	    const QCheckBox *checkbox = (const QCheckBox *) widget;
	    QRect irect = subRect(SR_CheckBoxIndicator, widget);
	    int h = pixelMetric( PM_IndicatorHeight, widget );
	    sz += QSize(irect.right() + (checkbox->text().isEmpty() ? 0 : 10), 4 );
	    sz.setHeight( QMAX( sz.height(), h ) );
#endif
	    break;
	}

    case CT_RadioButton:
	{
#ifndef QT_NO_RADIOBUTTON
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;
	    QRect irect = subRect(SR_RadioButtonIndicator, widget);
	    int h = pixelMetric( PM_ExclusiveIndicatorHeight, widget );
	    sz += QSize(irect.right() + (radiobutton->text().isEmpty() ? 0 : 10), 4 );
	    sz.setHeight( QMAX( sz.height(), h ) );
#endif
	    break;
	}

    case CT_ToolButton:
	{
	    sz = QSize(sz.width() + 7, sz.height() + 6);
	    break;
	}

    case CT_ComboBox:
	{
	    int dfw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
	    sz = QSize(sz.width() + dfw + 21, sz.height() + dfw + 1);
	    break;
	}

    case CT_PopupMenuItem:
	{
#ifndef QT_NO_POPUPMENU
	    if (opt.isDefault())
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
		    h += 8;
	    } else if ( mi->widget() ) {
	    } else if (mi->isSeparator()) {
		w = 10;
		h = 2;
	    } else {
		if (mi->pixmap())
		    h = QMAX(h, mi->pixmap()->height() + 4);
		else
		    h = QMAX(h, popup->fontMetrics().height() + 8);

		if (mi->iconSet() != 0)
		    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height() + 4);
	    }

	    if (! mi->text().isNull()) {
		if (mi->text().find('\t') >= 0)
		    w += 12;
	    }

	    if (maxpmw)
		w += maxpmw + 6;
	    if (checkable && maxpmw < 20)
		w += 20 - maxpmw;
	    if (checkable || maxpmw > 0)
		w += 2;
	    w += 12;

	    sz = QSize(w, h);
#endif
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


/*! \reimp */
int QCommonStyle::styleHint(StyleHint sh, const QWidget *, QStyleHintReturn *) const
{
    int ret;

    switch (sh) {
    case SH_TabBar_SelectMouseType:
	ret = QEvent::MouseButtonPress;
	break;

    case SH_GUIStyle:
	ret = WindowsStyle;
	break;

    case SH_ScrollBar_BackgroundMode:
	ret = QWidget::PaletteBackground;
	break;

    case SH_TabBar_Alignment:
    case SH_Header_ArrowAlignment:
	ret = Qt::AlignLeft;
	break;

    case SH_PopupMenu_SubMenuPopupDelay:
	ret = 256;
	break;

    case SH_ProgressDialog_TextLabelAlignment:
	ret = Qt::AlignCenter;
	break;

    default:
	ret = 0;
	break;
    }

    return ret;
}


/*! \reimp */
QPixmap QCommonStyle::stylePixmap(StylePixmap, const QWidget *, const QStyleOption&) const
{
    return QPixmap();
}


#endif // QT_NO_STYLE
