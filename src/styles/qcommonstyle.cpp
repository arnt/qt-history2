/****************************************************************************
** $Id: //depot/qt/main/src/styles/qcommonstyle.cpp#49 $
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

#define INCLUDE_MENUITEM_DEF
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
#include "qspinbox.h"
#include "qrangecontrol.h"
#include "qgroupbox.h"
#include "qslider.h"
#include "qlistview.h"
#include "qcheckbox.h"
#include "qradiobutton.h"
#include "qbitmap.h"
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

/*! \reimp */
void QCommonStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
				    QMenuItem* mi, QColorGroup& g,
				    bool, bool, bool )
{
#ifndef QT_NO_COMPLEXWIDGETS
    drawItem( p, QRect(x, y, w, h), AlignCenter|ShowPrefix|DontClip|SingleLine,
	      g, mi->isEnabled(), mi->pixmap(), mi->text(), -1, &g.buttonText() );
#endif
}

/*! \reimp */
void QCommonStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe,
				  int& overlap) const
{
#ifndef QT_NO_TABBAR
    overlap = 3;
    hframe = 24;
    vframe = 0;
    if ( t->shape() == QTabBar::RoundedAbove || t->shape() == QTabBar::RoundedBelow )
	vframe += 10;
#endif
}

/*! \reimp */
void QCommonStyle::drawTab( QPainter* p,  const  QTabBar* tb, QTab* t , bool selected )
{
#ifndef QT_NO_TABBAR
    if ( tb->shape() == QTabBar::TriangularAbove || tb->shape() == QTabBar::TriangularBelow ) {
	// triangular, above or below
	int y;
	int x;
	QRect r = t->rect();
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

	if ( selected )
	    p->setBrush( tb->colorGroup().base() );
	else
	    p->setBrush( tb->colorGroup().background() );
	p->setPen( tb->colorGroup().foreground() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
    }
#endif
}

/*! \reimp */
void QCommonStyle::drawHeaderSection( QPainter *p, int x, int y, int w, int h,
				     const QColorGroup &g, bool down )
{
    drawPrimitive(PO_ButtonBevel, p, QRect(x, y, w, h), g,
		  down ? PStyle_Sunken : PStyle_Default);
}

/*! \reimp */
void QCommonStyle::drawGroupBoxTitle( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QString &text, bool enabled )
{
    drawItem( p, QRect(x, y, w, h), AlignCenter + ShowPrefix, g, enabled, 0, text );
}

/*! \reimp */
void QCommonStyle::drawGroupBoxFrame( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QGroupBox *gb )
{
    QPoint		p1, p2;
    QFrame::Shape	type  = gb->frameShape();
    QFrame::Shadow	cstyle = gb->frameShadow();
    int			lwidth = gb->lineWidth();
    int			mlwidth = gb->midLineWidth();

    switch ( type ) {
    case QFrame::Box:
	if ( cstyle == QFrame::Plain )
	    qDrawPlainRect( p, x, y, w, h, g.foreground(), lwidth );
	else
	    qDrawShadeRect( p, x, y, w, h, g, cstyle == QFrame::Sunken, lwidth, mlwidth );
	break;

    case QFrame::StyledPanel:
	if ( cstyle == QFrame::Plain )
	    qDrawPlainRect( p, x, y, w, h, g.foreground(), lwidth );
	else
	    drawPanel( p, x, y, w, h, g, cstyle == QFrame::Sunken, lwidth );
	break;

    case QFrame::PopupPanel:
	if ( cstyle == QFrame::Plain )
	    qDrawPlainRect( p, x, y, w, h, g.foreground(), lwidth );
	else
	    drawPopupPanel( p, x, y, w, h, g, lwidth );
	break;

    case QFrame::Panel:
	if ( cstyle == QFrame::Plain )
	    qDrawPlainRect( p, x, y, w, h, g.foreground(), lwidth );
	else
	    qDrawShadePanel( p, x, y, w, h, g, cstyle == QFrame::Sunken, lwidth );
	break;

    case QFrame::WinPanel:
	if ( cstyle == QFrame::Plain )
	    qDrawPlainRect( p, x, y, w, h, g.foreground(), 2 );
	else
	    qDrawWinPanel( p, x, y, w, h, g, cstyle == QFrame::Sunken );
	break;
    case QFrame::MenuBarPanel:
	drawMenuBarPanel( p, x, y, w, h, g );
	break;
    case QFrame::ToolBarPanel:
	drawToolBarPanel( p, x, y, w, h, g );
	break;
    case QFrame::HLine:
    case QFrame::VLine:
	if ( type == QFrame::HLine ) {
	    p1 = QPoint( x, h/2 );
	    p2 = QPoint( x+w, p1.y() );
	}
	else {
	    p1 = QPoint( x+w/2, 0 );
	    p2 = QPoint( p1.x(), h );
	}
	if ( cstyle == QFrame::Plain ) {
	    QPen oldPen = p->pen();
	    p->setPen( QPen(g.foreground(),lwidth) );
	    p->drawLine( p1, p2 );
	    p->setPen( oldPen );
	}
	else
	    qDrawShadeLine( p, p1, p2, g, cstyle == QFrame::Sunken,
			    lwidth, mlwidth );
	break;
    default:
	break;
    }
}

/*! \reimp */
void QCommonStyle::drawStatusBarSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool /*permanent*/ )
{
    qDrawShadeRect( p, x, y, w, h, g, TRUE, 1, 0, 0 );
}

/*! \reimp */
void QCommonStyle::drawSizeGrip( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    p->save();
    int sw = QMIN( h,w );
    if ( h > w )
	p->translate( 0, h - w );
    else
	p->translate( w - h, 0 );

    int sx = x;
    int sy = y;
    int s = sw / 3;

    for ( int i = 0; i < 3; ++i ) {
	p->setPen( QPen( g.mid(), 1 ) );
	p->drawLine(  sx-1, sw, sw,  sy-1 );
	p->setPen( QPen( g.dark(), 1 ) );
	p->drawLine(  sx, sw, sw,  sy );
	sx += s;
	sy += s;
    }
    p->setPen( QPen( g.mid(), 1 ) );
    p->drawPoint( sw-2, sw-2 );
    p->setPen( QPen( g.dark(), 1 ) );
    p->drawPoint( sw-1, sw-1 );

    p->restore();
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
void QCommonStyle::titleBarMetrics( const QTitleBar*tb, int &ctrlW, int &ctrlH,
				    int &titleW, int &titleH) const
{
#ifndef QT_NO_TITLEBAR
    titleH = 18;
    if(tb->window) {
	titleW = tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*3)-
		 (TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH)-4;
	ctrlH = TITLEBAR_CONTROL_HEIGHT;
	ctrlW = TITLEBAR_CONTROL_WIDTH;
    } else {
	titleW = tb->width();
	ctrlH = ctrlW = 0;
    }
#endif //QT_NO_TITLEBAR
}

/*!
 \reimp
 */
void QCommonStyle::drawTitleBarControls( QPainter*p,  const QTitleBar*tb,
					uint controls, uint activeControl )
{
#ifndef QT_NO_TITLEBAR
    QColor left = tb->act || !tb->window ? tb->aleftc : tb->ileftc;
    QColor right = tb->act || !tb->window ? tb->arightc : tb->irightc;
    if ( left != right ) {
	double rS = left.red();
	double gS = left.green();
	double bS = left.blue();

	double rD = double(right.red() - rS) / tb->width();
	double gD = double(right.green() - gS) / tb->width();
	double bD = double(right.blue() - bS) / tb->width();

	int w = tb->width();
	for ( int sx = 0; sx < w; sx++ ) {
	    rS+=rD;
	    gS+=gD;
	    bS+=bD;
	    p->setPen( QColor( (int)rS, (int)gS, (int)bS ) );
	    p->drawLine( sx, 0, sx, tb->height() );
	}
    } else {
	p->fillRect( tb->rect(), left );
    }
    QRect r;
    if(tb->window)
	r = QRect( TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH + 2, 0,
		   tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*3)-
		   (TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH),
		   tb->height());
    else
	r = QRect( 0, 0, tb->width(), tb->height());

    p->setPen( tb->act || !tb->window ? tb->atextc : tb->itextc );
    p->drawText(r.x()+2, r.y(), r.width(), r.height(),
		AlignAuto | AlignVCenter | SingleLine, tb->cuttext );

    if(tb->window) {
	QRect r(tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)),
		2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	bool down = activeControl & TitleCloseButton;
	QPixmap pm(titleBarPixmap(tb, TitleCloseButton));
	drawToolButton( p, r.x(), r.y(), r.width(), r.height(),
			tb->colorGroup(), FALSE, down, TRUE, FALSE );
	int xoff = 0, yoff = 0;
	if (down) {
	    xoff = pixelMetric(PM_ButtonShiftHorizontal);
	    yoff = pixelMetric(PM_ButtonShiftVertical);
	}
	drawItem( p, QRect(r.x()+xoff, r.y()+yoff, TITLEBAR_CONTROL_WIDTH,
			   TITLEBAR_CONTROL_HEIGHT),
		  AlignCenter, tb->colorGroup(), TRUE, &pm, QString::null );

	r = QRect(tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*2),
		  2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);

	down = activeControl & TitleMaxButton;
	pm = QPixmap(titleBarPixmap(tb, TitleMaxButton));
	drawToolButton( p, r.x(), r.y(), r.width(), r.height(),
			tb->colorGroup(), FALSE, down, TRUE, FALSE );
	xoff = 0;
	yoff = 0;
	if(down) {
	    xoff = pixelMetric(PM_ButtonShiftHorizontal);
	    yoff = pixelMetric(PM_ButtonShiftVertical);
	}
	drawItem( p, QRect(r.x()+xoff, r.y()+yoff, TITLEBAR_CONTROL_WIDTH,
			   TITLEBAR_CONTROL_HEIGHT),
		  AlignCenter, tb->colorGroup(), TRUE, &pm, QString::null );

	r = QRect(tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*3),
		  2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	QStyle::TitleControl ctrl = controls & TitleNormalButton ? TitleNormalButton : TitleMinButton;
	down = activeControl & ctrl;
	pm = QPixmap(titleBarPixmap(tb, ctrl));
	drawToolButton( p, r.x(), r.y(), r.width(), r.height(),
			tb->colorGroup(), FALSE, down, TRUE, FALSE );
	xoff=0, yoff=0;
	if(down) {
	    xoff = pixelMetric(PM_ButtonShiftHorizontal);
	    yoff = pixelMetric(PM_ButtonShiftVertical);
	}
	drawItem( p, QRect(r.x()+xoff, r.y()+yoff, TITLEBAR_CONTROL_WIDTH,
			   TITLEBAR_CONTROL_HEIGHT),
		  AlignCenter, tb->colorGroup(), TRUE, &pm, QString::null );

 	if ( !tb->pixmap.isNull() ) {
	    r = QRect( 2 + TITLEBAR_CONTROL_WIDTH / 2 - tb->pixmap.width() / 2,
		       tb->height() / 2 - tb->pixmap.height() / 2,
		       tb->pixmap.width(), tb->pixmap.height() );

	    p->drawPixmap( r.x(), r.y(), tb->pixmap, 0, 0, tb->pixmap.width(), tb->pixmap.height() );
 	}
    }
#endif //QT_NO_TITLEBAR
}

/*!
 \reimp
 */
QStyle::TitleControl QCommonStyle::titleBarPointOver( const QTitleBar*tb, const QPoint&point )
{
#ifndef QT_NO_TITLEBAR
    if(tb->window) {
	if(QRect(tb->width()-(TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION), 2,
		 TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT).contains(point))
	    return TitleCloseButton;
	else if(QRect(tb->width()-((TITLEBAR_CONTROL_HEIGHT+TITLEBAR_SEPARATION) * 2), 2,
		      TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT).contains(point))
	    return TitleMaxButton;
	else if(QRect(tb->width()-((TITLEBAR_CONTROL_HEIGHT+TITLEBAR_SEPARATION) * 3), 2,
		      TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT).contains(point))
	    return TitleMinButton;
	else if(QRect(2+TITLEBAR_SEPARATION, 2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT).contains(point))
	    return TitleSysMenu;
    }
    return TitleLabel;
#endif //QT_NO_TITLEBAR
}

/*!
 \reimp
 */
QStyle::ListViewItemControl
QCommonStyle::listViewItemPointOver( const QListViewItem *i, const QPoint &pos )
{
#ifndef QT_NO_LISTVIEW
    if(pos.x() >= 0 && pos.x() < i->listView()->treeStepSize())
       return ListViewExpand;
#endif
    return ListViewNone;
}

#endif








// New QStyle API


/*!
  Draws a primitive operation.
*/
void QCommonStyle::drawPrimitive( PrimitiveOperation op,
				  QPainter *p,
				  const QRect &r,
				  const QColorGroup &cg,
				  PFlags flags,
				  void *data ) const
{
    switch (op) {
    case PO_ButtonCommand:
    case PO_ButtonBevel:
    case PO_ButtonTool:
	qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
			cg, flags & PStyle_Sunken, 1,
			&cg.brush(QColorGroup::Button));
	break;

    case PO_FocusRect: {
	QPen oldPen = p->pen();
	p->setPen(cg.dark());
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
	// make sure the indicator is square
	QRect ir = r;

	if (r.width() < r.height()) {
	    ir.setTop(r.top() + (r.height() - r.width()) / 2);
	    ir.setHeight(r.width());
	} else if (r.height() < r.width()) {
	    ir.setLeft(r.left() + (r.width() - r.height()) / 2);
	    ir.setWidth(r.height());
	}

	if (flags & PStyle_NoChange) {
	    p->setPen(cg.foreground());
	    p->fillRect(ir, cg.brush(QColorGroup::Button));
	    p->drawLine(ir.topLeft(), ir.bottomRight());
	} else
	    qDrawShadePanel(p, ir.x(), ir.y(), ir.width(), ir.height(),
			    cg, flags & (PStyle_Sunken | PStyle_On), 1,
			    &cg.brush(QColorGroup::Button));
	break; }

    case PO_IndicatorMask: {
	// make sure the indicator is square
	QRect ir = r;

	if (r.width() < r.height()) {
	    ir.setTop(r.top() + (r.height() - r.width()) / 2);
	    ir.setHeight(r.width());
	} else if (r.height() < r.width()) {
	    ir.setLeft(r.left() + (r.width() - r.height()) / 2);
	    ir.setWidth(r.height());
	}

	p->fillRect(ir, color1);
	break; }

    default:
	break;
    }
}


/*
  Draws a control.
*/
void QCommonStyle::drawControl( ControlElement element,
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

	PFlags flags = PStyle_Default;
	if (button->isEnabled())
	    flags |= PStyle_Enabled;
	if (button->isDown())
	    flags |= PStyle_Sunken;

	drawPrimitive(PO_ButtonCommand, p, r, cg, flags, data);
	break; }

    case CE_PushButtonLabel: {
	QPushButton *button = (QPushButton *) widget;
	QRect ir = r;

	PFlags flags = PStyle_Default;
	if (button->isEnabled())
	    flags |= PStyle_Enabled;
	if (button->isDown())
	    flags |= PStyle_Sunken;

	if (button->isDown())
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
			  cg, flags, data);
	break; }

    case CE_PushButtonMask:
	drawPrimitive(PO_ButtonCommand, p, r, cg, PStyle_Default, data);
	break;

    case CE_CheckBox: {
	QCheckBox *checkbox = (QCheckBox *) widget;

	PFlags flags = PStyle_Default;
	if (checkbox->isEnabled())
	    flags |= PStyle_Enabled;
	if (checkbox->isDown())
	    flags |= PStyle_Sunken;
	if (checkbox->state() == QButton::On)
	    flags |= PStyle_On;
	else if (checkbox->state() == QButton::Off)
	    flags |= PStyle_Off;
	else if (checkbox->state() == QButton::NoChange)
	    flags |= PStyle_NoChange;

	drawPrimitive(PO_Indicator, p, r, cg, flags, data);
	break; }

    case CE_CheckBoxLabel: {
	QCheckBox *checkbox = (QCheckBox *) widget;

	PFlags flags = PStyle_Default;
	if (checkbox->isEnabled())
	    flags |= PStyle_Enabled;

	drawItem(p, r, AlignAuto | AlignVCenter | ShowPrefix, cg,
		 flags & PStyle_Enabled, checkbox->pixmap(), checkbox->text());

	if (checkbox->hasFocus())
	    drawPrimitive(PO_FocusRect, p, subRect(SR_CheckBoxFocusRect, widget),
			  cg, flags, data);
	break; }

    case CE_CheckBoxMask:
	drawPrimitive(PO_IndicatorMask, p, r, cg, PStyle_Default, data);
	break;

    default:
	break;
    }
}


/*
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
	QCheckBox *checkbox = (QCheckBox *) widget;
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

    default:
	rect = wrect;
	break;
    }

    return rect;
}


/*!
  Draws a complex control.
*/
void QCommonStyle::drawComplexControl( ComplexControl,
				       QPainter* ,
				       const QWidget *,
				       const QRect &,
				       const QColorGroup &,
				       CFlags,
				       SCFlags,
				       SCFlags,
				       void * ) const
{
}


/*!
  Returns the metrics of a subcontrol in a complex control.
*/
QRect QCommonStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *w,
					    SubControl sc,
					    void *data ) const
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
	case SC_ComboBoxFocusRect:
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
	int sbstart;

	if (data)
	    sliderstart = *((int*) data);
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
	    void ** sdata = (void **) data;
	    int sliderPos = 0, tickOffset = 0,
		thickness = pixelMetric( PM_SliderControlThickness, sl );
	    int len   = pixelMetric( PM_SliderLength, sl );
	    int space = (sl->orientation() == Horizontal) ? sl->height() :
		        sl->width();
	    int ticks = sl->tickmarks();

	    if ( sdata ) {
		sliderPos = *((int *) sdata[0]);
		tickOffset = *((int *) sdata[1]);
	    }

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
						 void *data ) const
{
    SubControl ret = SC_None;

    switch (control) {
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
				     void *data ) const
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
	QSize indicator = indicatorSize();

	sz = contentsSize +
	     QSize(indicator.width() + (checkbox->text().isEmpty() ? 0 : 10), 4);
	break; }

    default:
	break;
    }

    return sz;
}


/*!
  Returns a style (look and feel) hint.
*/
int QCommonStyle::styleHint(StyleHint sh, const QWidget *, void **) const
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
