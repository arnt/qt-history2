/****************************************************************************
** $Id: $
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
#include "../kernel/qapplication_p.h"
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

/*! \fn int QCommonStyle::menuBarFrameWidth() const

  \reimp
*/

/*!
  Constructs a QCommonStyle that provides the style \a s.  This determines
  the default behavior of the virtual functions.
*/
QCommonStyle::QCommonStyle(GUIStyle s) : QStyle(s)
{
}

/*! \reimp */
QCommonStyle::~QCommonStyle()
{
}



#define TITLEBAR_PAD 3
#define TITLEBAR_SEPARATION 1
#define TITLEBAR_PIXMAP_WIDTH 12
#define TITLEBAR_PIXMAP_HEIGHT 12
#define TITLEBAR_CONTROL_WIDTH (TITLEBAR_PAD+TITLEBAR_PIXMAP_WIDTH)
#define TITLEBAR_CONTROL_HEIGHT (TITLEBAR_PAD+TITLEBAR_PIXMAP_HEIGHT)




// New QStyle API


/*!
  Draws a primitive operation.
*/
void QCommonStyle::drawPrimitive( PrimitiveOperation op,
				  QPainter *p,
				  const QRect &r,
				  const QColorGroup &cg,
				  PFlags flags,
				  void **data ) const
{
    switch (op) {
    case PO_StatusBarSection:
	qDrawShadeRect( p, r, cg, TRUE, 1, 0, 0 );
	break;

    case PO_ButtonCommand:
    case PO_ButtonBevel:
    case PO_ButtonTool:
    case PO_ButtonDropDown:
    case PO_HeaderSection:
	qDrawShadePanel(p, r, cg, flags & PStyle_Sunken, 1,
			&cg.brush(QColorGroup::Button));
	break;

    case PO_Separator:
	qDrawShadeLine( p, r.left(), r.top(), r.right(), r.bottom(), cg,
			flags & PStyle_Sunken, 1, 0);
	break;

    case PO_FocusRect: {
	const QColor *bg = 0;

	if (data)
	    bg = (const QColor *) data[0];

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

	if (flags & PStyle_FocusAtBorder)
	    p->drawRect(QRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2));
	else
	    p->drawRect(r);

	p->setPen(oldPen);
	break; }

    case PO_SpinWidgetPlus:
    case PO_SpinWidgetMinus: {
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
	if ( op == PO_SpinWidgetPlus )
	    p->drawLine( ( x+w / 2 ) - 1, y + ymarg,
			 ( x+w / 2 ) - 1, y + ymarg + length - 1 );
	p->restore();
	break; }

    case PO_SpinWidgetUp:
    case PO_SpinWidgetDown: {
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
	if ( op == PO_SpinWidgetDown )
	    a.setPoints( 3,  0, 1,  sw-1, 1,  sh-2, sh-1 );
	else
	    a.setPoints( 3,  0, sh-1,  sw-1, sh-1,  sh-2, 1 );
	int bsx = 0;
	int bsy = 0;
	if ( flags & PStyle_Sunken ) {
	    bsx = pixelMetric(PM_ButtonShiftHorizontal);
	    bsy = pixelMetric(PM_ButtonShiftVertical);
	}
	p->translate( sx + bsx, sy + bsy );
	p->setPen( cg.buttonText() );
	p->setBrush( cg.buttonText() );
	p->drawPolygon( a );
	p->restore();
	break; }

    case PO_Indicator: {
	if (flags & PStyle_NoChange) {
	    p->setPen(cg.foreground());
	    p->fillRect(r, cg.brush(QColorGroup::Button));
	    p->drawRect(r);
	    p->drawLine(r.topLeft(), r.bottomRight());
	} else
	    qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
			    cg, flags & (PStyle_Sunken | PStyle_On), 1,
			    &cg.brush(QColorGroup::Button));
	break; }

    case PO_IndicatorMask: {
	p->fillRect(r, color1);
	break; }

    case PO_ExclusiveIndicator: {
	QRect ir = r;
	p->setPen(cg.dark());
	p->drawArc(r, 0, 5760);

	if (flags & (PStyle_Sunken | PStyle_On)) {
	    ir.addCoords(2, 2, -2, -2);
	    p->setBrush(cg.foreground());
	    p->drawEllipse(ir);
	}

	break; }

    case PO_ExclusiveIndicatorMask: {
	p->setPen(color1);
	p->setBrush(color1);
	p->drawEllipse(r);
	break; }

    case PO_DockWindowHandle: {
	bool highlight = flags & PStyle_On;

	p->save();
	p->translate( r.x(), r.y() );
	if ( flags & PStyle_Vertical ) {
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
	break; }

    case PO_DockWindowSeparator: {
	QPoint p1, p2;
	if ( flags & PStyle_Vertical ) {
	    p1 = QPoint( 0, r.height()/2 );
	    p2 = QPoint( r.width(), p1.y() );
	} else {
	    p1 = QPoint( r.width()/2, 0 );
	    p2 = QPoint( p1.x(), r.height() );
	}
	qDrawShadeLine( p, p1, p2, cg, 1, 1, 0 );
	break; }

    case PO_PanelDockWindow: {
	int lw = pixelMetric(PM_DockWindowFrameWidth);
	if (data)
	    lw = *((int *) data[0]);

	qDrawShadePanel(p, r, cg, FALSE, lw);
	break; }

    case PO_PanelMenuBar: {
	int lw = pixelMetric(PM_MenuBarFrameWidth);
	if (data)
	    lw = *((int *) data[0]);

	qDrawShadePanel(p, r, cg, FALSE, lw);
	break; }

    case PO_MenuBarItem: {
	QMenuItem * mi;

	if ( data ) {
	    mi = (QMenuItem *) data[0];
	    drawItem( p, r, AlignCenter|ShowPrefix|DontClip|SingleLine, cg,
		      mi->isEnabled(), mi->pixmap(), mi->text(), -1,
		      &cg.buttonText() );
	}
	break; }

    case PO_SizeGrip: {
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

	p->restore();
	break; }

    case PO_CheckMark: {
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
	if ( ! ((flags & PStyle_Enabled) && (flags & PStyle_On))) {
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

    case PO_GroupBoxFrame: {
	if ( !data )
	    break;

	// evil hacks
	QFrame::Shape	type 	= (QFrame::Shape) ((int) data[0]);
	QFrame::Shadow	cstyle 	= (QFrame::Shadow) ((int) data[1]);
	int 		lwidth 	= (int) data[2];
	int 		mlwidth = (int) data[3];

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
	break; }

    default:
	break;
    }
}

/*!
  Draws a control.
*/
void QCommonStyle::drawControl( ControlElement element,
				QPainter *p,
				const QWidget *widget,
				const QRect &r,
				const QColorGroup &cg,
				CFlags how,
				void **data ) const
{
    PFlags flags = PStyle_Default;
    if (widget->isEnabled())
	flags |= PStyle_Enabled;

    switch (element) {
    case CE_PushButton: {
	QPushButton *button = (QPushButton *) widget;

	if (button->isOn())
	    flags |= PStyle_On;
	if (button->isDown())
	    flags |= PStyle_Sunken;
	else if (! button->isFlat() && ! (flags & PStyle_Sunken))
	    flags |= PStyle_Raised;

	drawPrimitive(PO_ButtonCommand, p, r, cg, flags, data);
	break; }

    case CE_PushButtonLabel: {
	QPushButton *button = (QPushButton *) widget;
	QRect ir = r;

	if (button->isDown() || button->isOn())
	    flags |= PStyle_Sunken;

	if (button->isDown() || button->isOn())
	    ir.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
		      pixelMetric(PM_ButtonShiftVertical, widget));

	if (button->isMenuButton()) {
	    int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
	    QRect ar(ir.right() - mbi, ir.y() + 2, mbi - 4, ir.height() - 4);
	    drawPrimitive(PO_ArrowDown, p, ar, cg, flags, data);
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
		 flags & PStyle_Enabled, button->pixmap(), button->text());

	if (button->hasFocus())
	    drawPrimitive(PO_FocusRect, p, subRect(SR_PushButtonFocusRect, widget),
			  cg, flags);
	break; }

    case CE_CheckBox: {
	// many people expect to checkbox to be square, do that here.
	QRect ir = r;

	if (r.width() < r.height()) {
	    ir.setTop(r.top() + (r.height() - r.width()) / 2);
	    ir.setHeight(r.width());
	} else if (r.height() < r.width()) {
	    ir.setLeft(r.left() + (r.width() - r.height()) / 2);
	    ir.setWidth(r.height());
	}
	QCheckBox *checkbox = (QCheckBox *) widget;

	if (checkbox->isDown())
	    flags |= PStyle_Sunken;
	if (checkbox->state() == QButton::On)
	    flags |= PStyle_On;
	else if (checkbox->state() == QButton::Off)
	    flags |= PStyle_Off;
	else if (checkbox->state() == QButton::NoChange)
	    flags |= PStyle_NoChange;

	drawPrimitive(PO_Indicator, p, ir, cg, flags, data);
	break; }

    case CE_CheckBoxLabel: {
	QCheckBox *checkbox = (QCheckBox *) widget;

	drawItem(p, r, AlignAuto | AlignVCenter | ShowPrefix, cg,
		 flags & PStyle_Enabled, checkbox->pixmap(), checkbox->text());

	if (checkbox->hasFocus())
	    drawPrimitive(PO_FocusRect, p, subRect(SR_CheckBoxFocusRect, widget),
			  cg, flags);
	break; }

    case CE_RadioButton: {
	// many people expect to checkbox to be square, do that here.
	QRect ir = r;

	if (r.width() < r.height()) {
	    ir.setTop(r.top() + (r.height() - r.width()) / 2);
	    ir.setHeight(r.width());
	} else if (r.height() < r.width()) {
	    ir.setLeft(r.left() + (r.width() - r.height()) / 2);
	    ir.setWidth(r.height());
	}
	QRadioButton *radiobutton = (QRadioButton *) widget;

	if (radiobutton->isDown())
	    flags |= PStyle_Sunken;
	if (radiobutton->state() == QButton::On)
	    flags |= PStyle_On;
	else if (radiobutton->state() == QButton::Off)
	    flags |= PStyle_Off;

	drawPrimitive(PO_ExclusiveIndicator, p, r, cg, flags, data);
	break; }

    case CE_RadioButtonLabel: {
	QRadioButton *radiobutton = (QRadioButton *) widget;

	drawItem(p, r, AlignAuto | AlignVCenter | ShowPrefix, cg,
		 flags & PStyle_Enabled, radiobutton->pixmap(), radiobutton->text());

	if (radiobutton->hasFocus())
	    drawPrimitive(PO_FocusRect, p, subRect(SR_RadioButtonFocusRect, widget),
			  cg, flags);
	break; }

    case CE_TabBarTab: {
	QTabBar * tb = (QTabBar *) widget;
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

	    if ( how & CStyle_Selected )
		p->setBrush( cg.base() );
	    else
		p->setBrush( cg.background() );
	    p->setPen( cg.foreground() );
	    p->drawPolygon( a );
	    p->setBrush( NoBrush );
	}
	break; }

    case CE_ProgressBar: {
	QProgressBar *progressbar = (QProgressBar *) widget;

	qDrawShadePanel(p, r, cg, TRUE, 1, &cg.brush(QColorGroup::Background));

	if (! progressbar->totalSteps()) {
	    // draw busy indicator
	    int w = r.width();
	    int x = progressbar->progress() % (w * 2);
	    if (x > w)
		x = 2 * w - x;
	    x += r.x();
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
	    for (int i=0; i<nu; i++) {
		p->fillRect(r.x() + x + 3, r.y() + 3, unit_width - 2, r.height() - 6,
			    cg.brush(QColorGroup::Highlight));
		x += unit_width;
	    }
	}

	break; }

    case CE_ProgressBarLabel: {
	QProgressBar *progressbar = (QProgressBar *) widget;
	drawItem(p, r, AlignCenter | SingleLine, cg, progressbar->isEnabled(), 0,
		 progressbar->progressString());

	// MOTIF CODE:
	// if ( !hasExtraIndicator && percentage_visible && total_steps ) {
	// paint.setPen( colorGroup().highlightedText() );
	// paint.setClipRect( bar.x(), bar.y(), x+2, bar.height() );
	// paint.drawText( bar, AlignCenter | SingleLine, progress_str );
	// if ( progress_val != total_steps ) {
	// paint.setClipRect( bar.x() + x+2, bar.y(), bar.width() - x - 2, bar.height() );
	// paint.setPen( colorGroup().highlight() );
	// paint.drawText( bar, AlignCenter | SingleLine, progress_str );
	// }

	break; }

    default:
	break;
    }
}

/*!
  Draws a control mask.
*/
void QCommonStyle::drawControlMask( ControlElement control,
				    QPainter *p,
				    const QWidget *widget,
				    const QRect &r,
				    void **data ) const
{
    Q_UNUSED(widget);

    QColorGroup cg(color1,color1,color1,color1,color1,color1,color1,color1,color0);

    switch (control) {
    case CE_PushButton:
	drawPrimitive(PO_ButtonCommand, p, r, cg, PStyle_Default, data);
	break;

    case CE_CheckBox:
	drawPrimitive(PO_IndicatorMask, p, r, cg, PStyle_Default, data);
	break;

    case CE_RadioButton:
	drawPrimitive(PO_ExclusiveIndicatorMask, p, r, cg, PStyle_Default, data);
	break;

    default:
	p->fillRect(r, color1);
	break;
    }
}

/*!
  Returns a contents(?) subrect.
*/
QRect QCommonStyle::subRect(SubRect r, const QWidget *widget) const
{
    QRect rect, wrect(widget->rect());

    switch (r) {
    case SR_PushButtonContents: {
	QPushButton *button = (QPushButton *) widget;
	int dx1, dx2;

	dx1 = (pixelMetric(PM_ButtonMargin, widget) / 2) +
	      pixelMetric(PM_DefaultFrameWidth, widget);
       	if (button->isDefault() || button->autoDefault())
	    dx1 += pixelMetric(PM_ButtonDefaultIndicator, widget);
	dx2 = dx1 * 2;

	rect.setRect(wrect.left()   + dx1,
		     wrect.top()    + dx1,
		     wrect.right()  - dx2 + 1,
		     wrect.bottom() - dx2 + 1);
	break; }

    case SR_PushButtonFocusRect: {
	int dfw1 = pixelMetric(PM_DefaultFrameWidth, widget) * 2,
	    dfw2 = dfw1 * 2;
	rect.setRect(wrect.left()   + dfw1,
		     wrect.top()    + dfw1,
		     wrect.right()  - dfw2 + 1,
		     wrect.bottom() - dfw2 + 1);
	break; }

    case SR_CheckBoxIndicator:
	rect.setRect(0, 0, 13, QMAX(13, wrect.height()));
	break;

    case SR_CheckBoxContents: {
	QRect ir = subRect(SR_CheckBoxIndicator, widget);
	rect.setRect(ir.right() + 10, wrect.y(),
		     wrect.width() - ir.width() - 10, wrect.height());
	break; }

    case SR_CheckBoxFocusRect: {
	QCheckBox *checkbox = (QCheckBox *) widget;
	QRect ir = subRect(SR_CheckBoxIndicator, widget);

	QPainter p(checkbox);
	rect = itemRect(&p, wrect, AlignAuto | AlignVCenter | ShowPrefix,
			checkbox->isEnabled(), checkbox->pixmap(), checkbox->text());

	rect.moveBy(ir.right() + 10, 0);
	rect.setLeft( rect.left() - 3 );
	rect.setRight( rect.right() + 2 );
	rect.setTop( rect.top() - 2 );
	rect.setBottom( rect.bottom() + 2);
	rect = rect.intersect(wrect);
	break; }

    case SR_RadioButtonIndicator:
	rect.setRect(0, 0, 12, QMAX(12, wrect.height()));
	break;

    case SR_RadioButtonContents: {
	QRect ir = subRect(SR_RadioButtonIndicator, widget);
	rect.setRect(ir.right() + 10, wrect.y(),
		     wrect.width() - ir.width() - 10, wrect.height());
	break; }

    case SR_RadioButtonFocusRect: {
	QRadioButton *radiobutton = (QRadioButton *) widget;
	QRect ir = subRect(SR_RadioButtonIndicator, widget);

	QPainter p(radiobutton);
	rect = itemRect(&p, wrect, AlignAuto | AlignVCenter | ShowPrefix,
			radiobutton->isEnabled(), radiobutton->pixmap(),
			radiobutton->text());

	rect.moveBy(ir.right() + 10, 0);
	rect.setLeft( rect.left() - 3 );
	rect.setRight( rect.right() + 2 );
	rect.setTop( rect.top() - 2 );
	rect.setBottom( rect.bottom() + 2);
	rect = rect.intersect(wrect);
	break; }

    case SR_ComboBoxFocusRect:
	rect.setRect(3, 3, widget->width()-6-16, widget->height()-6);
	if( QApplication::reverseLayout() )
	    rect.moveBy( 2 + 16, 0 );
	break;

    case SR_SliderFocusRect: {
	QSlider * sl = (QSlider *) widget;
	int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	int thickness  = pixelMetric( PM_SliderControlThickness, sl );

	if ( sl->orientation() == Horizontal )
	    rect.setRect( 0, tickOffset-1, sl->width(), thickness+2 );
	else
	    rect.setRect( tickOffset-1, 0, thickness+2, sl->height() );
	rect = rect.intersect( sl->rect() ); // ## is this really necessary?
	break; }

    case SR_DockWindowHandleRect: {
	if ( !widget || !widget->parent() )
	    break;

	QDockWindow * dw = (QDockWindow *) widget->parent();

	if ( !dw->area() || !dw->isCloseEnabled() )
	    rect.setRect( 0, 0, widget->width(), widget->height() );
	else {
	    if ( dw->area()->orientation() == Horizontal )
		rect.setRect(0, 15, widget->width(), widget->height() - 15);
	    else
		rect.setRect(0, 1, widget->width() - 15, widget->height() - 1);
	}
	break; }

    case SR_ProgressBarContents: {
	QFontMetrics fm(widget ? widget->fontMetrics() : QApplication::font());
	int textw = fm.width("100%") + 6;
	rect.setCoords(wrect.left(), wrect.top(),
		       wrect.right() - textw, wrect.bottom());
	break; }

    case SR_ProgressBarLabel: {
	QFontMetrics fm(widget ? widget->fontMetrics() : QApplication::font());
	int textw = fm.width("100%") + 6;
	rect.setCoords(wrect.right() - textw, wrect.top(),
		       wrect.right(), wrect.bottom());
	break; }

    default:
	rect = wrect;
	break;
    }

    return rect;
}

/*
  I really need this and I don't want to expose it in QRangeControl..
*/
static int qPositionFromValue( QRangeControl * rc, int logical_val,
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


/*!
  Draws a complex control.
*/
void QCommonStyle::drawComplexControl( ComplexControl control,
				       QPainter *p,
				       const QWidget *widget,
				       const QRect &r,
				       const QColorGroup &cg,
				       CFlags,
				       SCFlags controls,
				       SCFlags active,
				       void **data ) const
{
    switch (control) {
    case CC_ToolButton: {
	QToolButton *toolbutton = (QToolButton *) widget;

	QRect button, menuarea;
	button   = querySubControlMetrics(control, widget, SC_ToolButton, data);
	menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, data);

	bool drawraised = FALSE;
	bool drawarrow = FALSE;
	Qt::ArrowType arrowType = Qt::DownArrow;
	if (data) {
	    drawraised = *((bool *) data[0]);
	    drawarrow  = *((bool *) data[1]);
	    arrowType  = *((Qt::ArrowType *) data[2]);
	}

	PFlags bflags = PStyle_Default, mflags = PStyle_Default;
	if (drawraised) {
	    bflags |= PStyle_Raised;
	    mflags |= PStyle_Raised;
	}

	if (toolbutton->isEnabled()) {
	    bflags |= PStyle_Enabled;
	    mflags |= PStyle_Enabled;
	}

	if (active & SC_ToolButton)
	    bflags |= PStyle_Sunken;
	if (active & SC_ToolButtonMenu)
	    mflags |= PStyle_Sunken;

	if (controls & SC_ToolButton) {
	    if (drawraised || (bflags & PStyle_Sunken))
		drawPrimitive(PO_ButtonTool, p, button, cg, bflags, data);
	    else if ( toolbutton->parentWidget() &&
		      toolbutton->parentWidget()->backgroundPixmap() &&
		      ! toolbutton->parentWidget()->backgroundPixmap()->isNull() )
		p->drawTiledPixmap( r, *(toolbutton->parentWidget()->backgroundPixmap()),
				    toolbutton->pos() );

	    if (bflags & PStyle_Sunken)
		button.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
			      pixelMetric(PM_ButtonShiftVertical, widget));

	    if (drawarrow) {
		PrimitiveOperation op;
		switch (arrowType) {
		case Qt::LeftArrow:  op = PO_ArrowLeft;  break;
		case Qt::RightArrow: op = PO_ArrowRight; break;
		case Qt::UpArrow:    op = PO_ArrowUp;    break;
		default:
		case Qt::DownArrow:  op = PO_ArrowDown;  break;
		}

		drawPrimitive(op, p, button, cg, bflags, data);
	    } else {
		QColor btext = cg.buttonText();

		if (toolbutton->iconSet().isNull() &&
		    ! toolbutton->text().isNull() &&
		    ! toolbutton->usesTextLabel()) {
		    drawItem(p, button, AlignCenter | ShowPrefix, cg,
			     bflags & PStyle_Enabled, 0, toolbutton->text(),
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
		    else if (drawraised)
			mode = QIconSet::Active;
		    else
			mode = QIconSet::Normal;
		    pm = toolbutton->iconSet().pixmap( size, mode, state );

		    if ( toolbutton->usesTextLabel() ) {
			p->setFont( toolbutton->font() );

			QRect pr = button, tr = button;
			int fh = p->fontMetrics().height();
			pr.addCoords(0, 0, 0, -fh);
			tr.addCoords(0, tr.height() - fh, 0, 0);
			drawItem( p, pr, AlignCenter, cg, TRUE, &pm, QString::null );
			drawItem( p, tr, AlignCenter | ShowPrefix, cg,
				  bflags & PStyle_Enabled, 0, toolbutton->textLabel(),
				  toolbutton->textLabel().length(), &btext);
		    } else
			drawItem( p, button, AlignCenter, cg, TRUE, &pm, QString::null );
		}
	    }
	}

	if (controls & SC_ToolButtonMenu) {
	    if (drawraised || (mflags & PStyle_Sunken))
		drawPrimitive(PO_ButtonDropDown, p, menuarea, cg, mflags, data);
	    drawPrimitive(PO_ArrowDown, p, menuarea, cg, mflags, data);
	}

	if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
	    QRect fr = toolbutton->rect();
	    fr.addCoords(3, 3, -3, -3);
	    drawPrimitive(PO_FocusRect, p, fr, cg);
	}

	break; }

    case CC_TitleBar: {
#ifndef QT_NO_TITLEBAR
	QTitleBar *titlebar = (QTitleBar *) widget;

	QColor left = titlebar->act || !titlebar->window ?
		      titlebar->aleftc : titlebar->ileftc;
	QColor right = titlebar->act || !titlebar->window ?
		       titlebar->arightc : titlebar->irightc;
	if ( left != right ) {
	    double rS = left.red();
	    double gS = left.green();
	    double bS = left.blue();

	    double rD = double(right.red() - rS) / titlebar->width();
	    double gD = double(right.green() - gS) / titlebar->width();
	    double bD = double(right.blue() - bS) / titlebar->width();

	    int w = titlebar->width();
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

	QRect ir;
	if(titlebar->window)
	    ir = QRect( TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH + 2, 0,
			titlebar->width()-((TITLEBAR_CONTROL_WIDTH+
					    TITLEBAR_SEPARATION)*3)-
			(TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH),
			titlebar->height());
	else
	    ir = QRect( 0, 0, titlebar->width(), titlebar->height());

	p->setPen( titlebar->act || !titlebar->window ?
		   titlebar->atextc : titlebar->itextc );
	p->drawText(ir.x()+2, ir.y(), ir.width(), ir.height(),
		    AlignAuto | AlignVCenter | SingleLine, titlebar->cuttext );

	if (titlebar->window) {
	    QRect ir(titlebar->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)),
		     2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	    bool down = active & SC_TitleBarCloseButton;
	    QPixmap pm(stylePixmap(SP_TitleBarCloseButton, widget));
	    drawPrimitive(PO_ButtonTool, p, ir, titlebar->colorGroup(),
			  down ? PStyle_Sunken : PStyle_Raised);
	    int xoff = 0, yoff = 0;
	    if (down) {
		xoff = pixelMetric(PM_ButtonShiftHorizontal);
		yoff = pixelMetric(PM_ButtonShiftVertical);
	    }
	    drawItem( p, QRect(ir.x()+xoff, ir.y()+yoff, TITLEBAR_CONTROL_WIDTH,
			       TITLEBAR_CONTROL_HEIGHT),
		      AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );

	    ir = QRect(titlebar->width()-((TITLEBAR_CONTROL_WIDTH+
					   TITLEBAR_SEPARATION)*2),
		       2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);

	    down = active & SC_TitleBarMaxButton;
	    pm = QPixmap(stylePixmap(SP_TitleBarMaxButton, widget));
	    drawPrimitive(PO_ButtonTool, p, ir, titlebar->colorGroup(),
			  down ? PStyle_Sunken : PStyle_Raised);
	    xoff = 0;
	    yoff = 0;
	    if(down) {
		xoff = pixelMetric(PM_ButtonShiftHorizontal);
		yoff = pixelMetric(PM_ButtonShiftVertical);
	    }
	    drawItem( p, QRect(ir.x()+xoff, ir.y()+yoff, TITLEBAR_CONTROL_WIDTH,
			       TITLEBAR_CONTROL_HEIGHT),
		      AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );

	    ir = QRect(titlebar->width()-((TITLEBAR_CONTROL_WIDTH+
					   TITLEBAR_SEPARATION)*3),
		       2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	    QStyle::SubControl ctrl = (controls & SC_TitleBarNormalButton ?
				       SC_TitleBarNormalButton :
				       SC_TitleBarMinButton);
	    QStyle::StylePixmap spixmap = (controls & SC_TitleBarNormalButton ?
					   SP_TitleBarNormalButton :
					   SP_TitleBarMinButton);
	    down = active & ctrl;
	    pm = QPixmap(stylePixmap(spixmap, widget));
	    drawPrimitive(PO_ButtonTool, p, ir, titlebar->colorGroup(),
			  down ? PStyle_Sunken : PStyle_Raised);
	    xoff=0, yoff=0;
	    if(down) {
		xoff = pixelMetric(PM_ButtonShiftHorizontal);
		yoff = pixelMetric(PM_ButtonShiftVertical);
	    }
	    drawItem( p, QRect(ir.x()+xoff, ir.y()+yoff, TITLEBAR_CONTROL_WIDTH,
			       TITLEBAR_CONTROL_HEIGHT),
		      AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );

	    if ( !titlebar->pixmap.isNull() ) {
		ir = QRect( 2 + TITLEBAR_CONTROL_WIDTH / 2 -
			    titlebar->pixmap.width() / 2,
			    titlebar->height() / 2 - titlebar->pixmap.height() / 2,
			    titlebar->pixmap.width(), titlebar->pixmap.height() );

		p->drawPixmap( ir.x(), ir.y(), titlebar->pixmap, 0, 0,
			       titlebar->pixmap.width(), titlebar->pixmap.height() );
	    }
	}
#endif //QT_NO_TITLEBAR
      	break; }

    case CC_SpinWidget:
	switch( controls ) {
	case SC_SpinWidgetUp:
	case SC_SpinWidgetDown: {
	    QSpinWidget * sw = (QSpinWidget *) widget;
	    PFlags flags = PStyle_Default;
	    PrimitiveOperation op = (controls == SC_SpinWidgetUp) ?
		PO_SpinWidgetUp : PO_SpinWidgetDown;

	    flags |= PStyle_Enabled;
	    if (active == controls ) {
		flags |= PStyle_On;
		flags |= PStyle_Sunken;
	    } else
		flags |= PStyle_Raised;

	    if ( sw->buttonSymbols() == QSpinWidget::PlusMinus ) {
		if ( controls == SC_SpinWidgetUp )
		    op = PO_SpinWidgetPlus;
		else
		    op = PO_SpinWidgetMinus;
	    }

	    drawPrimitive(PO_ButtonBevel, p, r, cg, flags);
	    drawPrimitive(op, p, r, cg, flags);
	    break; }

	case SC_SpinWidgetFrame:
	    qDrawWinPanel( p, r, cg, TRUE ); //cstyle == Sunken );
	    break;
	}
	break;

    case CC_Slider:
	switch ( controls ) {
	case SC_SliderTickmarks: {
	    QSlider * sl = (QSlider *) widget;
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

    default:
	break;
    }
}


/*!
  Draws a complex control mask.
*/
void QCommonStyle::drawComplexControlMask( ComplexControl control,
					   QPainter *p,
					   const QWidget *widget,
					   const QRect &r,
					   void **data ) const
{
    Q_UNUSED(control);
    Q_UNUSED(widget);
    Q_UNUSED(data);

    p->fillRect(r, color1);
}


/*!
  Returns the metrics of a subcontrol in a complex control.
*/
QRect QCommonStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *w,
					    SubControl sc,
					    void **data ) const
{
    QRect rect;

    switch ( control ) {
    case CC_SpinWidget: {
	if ( !w )
	    break;
	int fw = pixelMetric( PM_SpinBoxFrameWidth, 0 );
	QSize bs;
	bs.setHeight( w->height()/2 - fw );
	if ( bs.height() < 8 )
	    bs.setHeight( 8 );
	bs.setWidth( bs.height() * 8 / 5 ); // 1.6 -approximate golden mean
	bs = bs.expandedTo( QApplication::globalStrut() );
	int y = fw;
	int x, lx, rx;
	if ( QApplication::reverseLayout() ) {
	    x = y;
	    lx = x + bs.width() + fw;
	    rx = w->width() - fw;
	} else {
	    x = w->width() - y - bs.width();
	    lx = fw;
	    rx = x - fw;
	}
	switch ( sc ) {
	case SC_SpinWidgetUp:
	    rect.setRect(x, y, bs.width(), bs.height());
	    break;
	case SC_SpinWidgetDown:
	    rect.setRect(x, y + bs.height(), bs.width(), bs.height());
	    break;
	case SC_SpinWidgetButtonField:
	    rect.setRect(x, y, bs.width(), w->height() - 2*fw);
	    break;
	case SC_SpinWidgetEditField:
	    rect.setRect(lx, fw, rx, w->height() - 2*fw);
	    break;
	case SC_SpinWidgetFrame:
	    return w->rect();
	default:
	    break;
	}

	break; }

    case CC_ComboBox: {
	int x = 0, y = 0, wi = w->width(), he = w->height();
	int xpos = x;
	bool reverse = QApplication::reverseLayout();

	if ( !reverse )
	    xpos += wi - 2 - 16;

	switch ( sc ) {
	case SC_ComboBoxArrow:
	    rect.setRect(xpos, y+2, 16, he-4);
	    break;
	case SC_ComboBoxEditField:
	    rect.setRect(x+3, y+3, wi-6-16, he-6);
	    if( reverse )
		rect.moveBy( 2 + 16, 0 );
	    break;
	default:
	    break;
	}

	break; }

    case CC_ScrollBar: {
	if (! w)
	    break;

	QScrollBar *scrollbar = (QScrollBar *) w;
	int sliderstart = 0;
	int sbextent = pixelMetric(PM_ScrollBarExtent, w);
	int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
		      scrollbar->width() : scrollbar->height()) - (sbextent * 2);
	int sliderlen;

	if (data)
	    sliderstart = *((int*) data[0]);
	else
	    sliderstart = sbextent;

	// calculate slider length
	if (scrollbar->maxValue() != scrollbar->minValue()) {
	    sliderlen = (scrollbar->pageStep() * maxlen)/
			(scrollbar->maxValue() -
			 scrollbar->minValue() +
			 scrollbar->pageStep());

	    uint range = scrollbar->maxValue() - scrollbar->minValue();
	    if ( sliderlen < 9 || range > INT_MAX / 2 )
		sliderlen = 9;
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

    case CC_Slider: {
	switch ( sc ) {
	case SC_SliderHandle: {
	    QSlider * sl = (QSlider *) w;
	    int sliderPos = 0;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int thickness  = pixelMetric( PM_SliderControlThickness, sl );
	    int len   = pixelMetric( PM_SliderLength, sl );

	    if ( data )
		sliderPos = *((int *) data[0]);

	    if ( sl->orientation() == Horizontal )
		rect.setRect( sliderPos, tickOffset, len, thickness );
	    else
		rect.setRect( tickOffset, sliderPos, thickness, len );
	    break; }

	case SC_SliderGroove:
	    break;
	default:
	    break;
	}
	break; }

    case CC_ToolButton: {
	QToolButton *toolbutton = (QToolButton *) w;
	int mbi = pixelMetric(PM_MenuButtonIndicator, w);

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
	break; }

    case CC_TitleBar: {
#ifndef QT_NO_TITLEBAR
	QTitleBar *titlebar = (QTitleBar *) w;

	if(titlebar->window) {
	    switch (sc) {
	    case SC_TitleBarLabel:
		rect.setCoords(TITLEBAR_CONTROL_WIDTH, 0,
			       titlebar->width()-((TITLEBAR_CONTROL_HEIGHT +
						   TITLEBAR_SEPARATION) * 3),
			       titlebar->height());
		break;

	    case SC_TitleBarCloseButton:
		rect.setRect(titlebar->width()-(TITLEBAR_CONTROL_WIDTH +
						TITLEBAR_SEPARATION), 2,
			     TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
		break;

	    case SC_TitleBarMaxButton:
		rect.setRect(titlebar->width()-((TITLEBAR_CONTROL_HEIGHT +
						 TITLEBAR_SEPARATION) * 2), 2,
			     TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
		break;

	    case SC_TitleBarMinButton:
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
	} else if (sc != SC_TitleBarLabel)
	    rect = QRect();
#endif //QT_NO_TITLEBAR
	break; }

    default:
	break;
    }

    return rect;
}


/*!
  Returns the subcontrol in a complex control.
*/
QStyle::SubControl QCommonStyle::querySubControl(ComplexControl control,
						 const QWidget *widget,
						 const QPoint &pos,
						 void **data ) const
{
    SubControl ret = SC_None;

    switch (control) {
    case CC_ListView: {
#ifndef QT_NO_LISTVIEW
	if(pos.x() >= 0 && pos.x() <
	   ((QListViewItem *) data[0])->listView()->treeStepSize())
	    ret = SC_ListViewExpand;
#endif
	break; }

    case CC_ScrollBar: {
	QRect r;
	uint ctrl = SC_ScrollBarAddLine;

	// we can do this because subcontrols were designed to be masks as well...
	while (ret == SC_None && ctrl < SC_ScrollBarGroove) {
	    r = querySubControlMetrics(control, widget, (QStyle::SubControl) ctrl, data);
	    if (r.isValid() && r.contains(pos))
		ret = (QStyle::SubControl) ctrl;

	    ctrl <<= 1;
	}

	break; }

    case CC_TitleBar: {
	QRect r;
	uint ctrl = SC_TitleBarSysMenu;

	// we can do this because subcontrols were designed to be masks as well...
	while (ret == SC_None && ctrl < SC_TitleBarUnshadeButton) {
	    r = querySubControlMetrics(control, widget, (QStyle::SubControl) ctrl, data);
	    if (r.isValid() && r.contains(pos))
		ret = (QStyle::SubControl) ctrl;

	    ctrl <<= 1;
	}

	break; }

    default:
	break;
    }

    return ret;
}


/*!
  Returns a pixel metric.
*/
int QCommonStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
    int ret;

    switch (m) {
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

    case PM_SliderMaximumDragDistance:
    case PM_ScrollBarMaximumDragDistance:
	ret = -1;
	break;

    case PM_SliderThickness:
	ret = 16;
	break;

    case PM_SliderTickmarkOffset: {
	QSlider * sl = (QSlider *) widget;
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
	break; }

    case PM_SliderSpaceAvailable: {
	QSlider * sl = (QSlider *) widget;
	if ( sl->orientation() == Horizontal )
	    ret = sl->width() - pixelMetric( PM_SliderLength, sl );
	else
	    ret = sl->height() - pixelMetric( PM_SliderLength, sl );
	break; }

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

    case PM_TabBarOverlap:
	ret = 3;
	break;

    case PM_TabBarBaseHeight:
	ret = 0;
	break;

    case PM_TabBarBaseOverlap:
	ret = 0;
	break;

    case PM_TabBarHorizontalFrame:
	ret = 24;
	break;

    case PM_TabBarVerticalFrame: {
	QTabBar * tb = (QTabBar *) widget;
	if ( tb->shape() == QTabBar::RoundedAbove ||
	     tb->shape() == QTabBar::RoundedBelow )
	    ret = 10;
	else
	    ret = 0;
	break; }

    case PM_ProgressBarChunkWidth:
	ret = 9;
	break;

    default:
	ret = 0;
	break;
    }

    return ret;
}


/*!
  Returns the size for the contents.
*/
QSize QCommonStyle::sizeFromContents(ContentsType contents,
				     const QWidget *widget,
				     const QSize &contentsSize,
				     void **data ) const
{
    QSize sz(contentsSize);

    switch (contents) {
    case CT_PushButton: {
	QPushButton *button = (QPushButton *) widget;
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
	break; }

    case CT_CheckBox: {
	QCheckBox *checkbox = (QCheckBox *) widget;
	QSize isz = subRect(SR_CheckBoxIndicator, widget).size();
	sz += QSize(isz.width() + (checkbox->text().isEmpty() ? 0 : 10), 4);
	break; }

    case CT_RadioButton: {
	QRadioButton *radiobutton = (QRadioButton *) widget;
	QSize isz = subRect(SR_RadioButtonIndicator, widget).size();
	sz += QSize(isz.width() + (radiobutton->text().isEmpty() ? 0 : 10), 4);
	break; }

    case CT_ToolButton: {
	sz = QSize(sz.width() + 7, sz.height() + 6);
    	break; }

    case CT_ComboBox: {
	int dfw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
	sz = QSize(sz.width() + dfw + 21, sz.height() + dfw);
	break; }

    case CT_PopupMenuItem: {
	if (! widget || ! data)
	    break;

	QPopupMenu *popup = (QPopupMenu *) widget;
	bool checkable = popup->isCheckable();
	QMenuItem *mi = (QMenuItem *) data[0];
	int maxpmw = *((int *) data[1]);
	int w = sz.width(), h = sz.height() + 8;

	if (mi->isSeparator()) {
	    w = 10;
	    h = 2;
	    break;
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
	break; }

    case CT_ProgressBar:
	// just return the contentsSize for now
	// fall through intended

    default:
	break;
    }

    return sz;
}


/*!
  Returns a style (look and feel) hint.
*/
int QCommonStyle::styleHint(StyleHint sh, const QWidget *, void ***) const
{
    int ret;

    switch (sh) {
    case SH_ScrollBar_BackgroundMode:
	ret = QWidget::PaletteBackground;
	break;

    default:
	ret = 0;
	break;
    }

    return ret;
}


/*!
  Returns a style pixmap.
*/
QPixmap QCommonStyle::stylePixmap(StylePixmap sp, const QWidget *, void **) const
{
    return QPixmap();
}


#endif // QT_NO_STYLE
