/****************************************************************************
** $Id: //depot/qt/main/src/styles/qcommonstyle.cpp#33 $
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
#include "qlistview.h"
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
void QCommonStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
{
#ifndef QT_NO_PUSHBUTTON
    QRect r = pushButtonContentsRect( btn );
    if ( btn->isDown() || btn->isOn() ){
	int sx = 0;
	int sy = 0;
	getButtonShift(sx, sy);
	r.moveBy( sx, sy );
    }
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( btn->isMenuButton() ) {
	int dx = menuButtonIndicatorWidth( btn->height() );
	drawArrow( p, DownArrow, FALSE,
		   x+w-dx, y+2, dx-4, h-4,
		   btn->colorGroup(),
		   btn->isEnabled() );
	w -= dx;
    }

    if ( btn->iconSet() && !btn->iconSet()->isNull() ) {
	QIconSet::Mode mode = btn->isEnabled()
			      ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && btn->hasFocus() )
	    mode = QIconSet::Active;
	QIconSet::State state = QIconSet::Off;
	if ( btn->isToggleButton() && btn->isOn() )
	    state = QIconSet::On;
	QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode, state );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	p->drawPixmap( x+2, y+h/2-pixh/2, pixmap );
	x += pixw + 4;
	w -= pixw + 4;
    }
    drawItem( p, x, y, w, h,
	      AlignCenter | ShowPrefix,
	      btn->colorGroup(), btn->isEnabled(),
	      btn->pixmap(), btn->text(), -1, &btn->colorGroup().buttonText() );
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
QStyle::ScrollControl QCommonStyle::scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p )
{
#ifndef QT_NO_SCROLLBAR
    if ( !sb->rect().contains( p ) )
	return NoScroll;
    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );
    pos = (sb->orientation() == QScrollBar::Horizontal)? p.x() : p.y();
    if ( pos < sliderMin )
	return SubLine;
    if ( pos < sliderStart )
	return SubPage;
    if ( pos < sliderStart + sliderLength )
	return Slider;
    if ( pos < sliderMax + sliderLength )
	return AddPage;
    return AddLine;
#endif
}

/*! \reimp */
QRect QCommonStyle::pushButtonContentsRect( QPushButton* btn ) const
{
#ifndef QT_NO_PUSHBUTTON
    int fw = 0;
    if ( btn->isDefault() || btn->autoDefault() )
	fw = buttonDefaultIndicatorWidth();

    return buttonRect( fw, fw, btn->width()-2*fw, btn->height()-2*fw );
#endif
}

/*! \reimp */
void QCommonStyle::drawHeaderSection( QPainter *p, int x, int y, int w, int h,
				     const QColorGroup &g, bool down )
{
    drawBevelButton( p, x, y, w, h, g, down );
}

/*! \reimp */
void QCommonStyle::drawSpinWidgetButton( QPainter *p, int x, int y, int w,
					 int h, const QColorGroup &g,
					 QSpinWidget * /* sw */,
					 bool /* downbtn */, bool /* enabled */,
					 bool down )
{
    drawButton( p, x, y, w, h, g, down );
}

/*! \reimp */
void QCommonStyle::drawSpinWidgetSymbol( QPainter *p, int x, int y, int w,
					 int h, const QColorGroup &g,
					 QSpinWidget *sw, bool downbtn,
					 bool /* enabled */, bool down )
{
    p->save();
    if ( sw->buttonSymbols() == QSpinWidget::PlusMinus ) {
	p->setPen( g.buttonText() );
	p->setBrush( g.buttonText() );

	int length;
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
	if ( !downbtn )
	    p->drawLine( ( x+w / 2 ) - 1, y + ymarg,
			 ( x+w / 2 ) - 1, y + ymarg + length - 1 );
    } else {
	int sw = w-4;
	if ( sw < 3 )
	    return;
	else if ( !(sw & 1) )
	    sw--;
	sw -= ( sw / 7 ) * 2;	// Empty border
	int sh = sw/2 + 2;        // Must have empty row at foot of arrow

	int sx = x + w / 2 - sw / 2 - 1;
	int sy = y + h / 2 - sh / 2 - 1;

	QPointArray a;
	if ( downbtn )
	    a.setPoints( 3,  0, 1,  sw-1, 1,  sh-2, sh-1 );
	else
	    a.setPoints( 3,  0, sh-1,  sw-1, sh-1,  sh-2, 1 );
	int bsx = 0;
	int bsy = 0;
	if ( down )
	    getButtonShift( bsx, bsy );
	p->translate( sx + bsx, sy + bsy );
	p->setPen( g.buttonText() );
	p->setBrush( g.buttonText() );
	p->drawPolygon( a );
    }
    p->restore();
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
	if(down)
	    getButtonShift(xoff, yoff);
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
	if(down)
	    getButtonShift(xoff, yoff);
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
	if(down)
	    getButtonShift(xoff, yoff);
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
	qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(), cg,
			flags & PStyle_Sunken, 1);
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

	drawPrimitive(PO_ButtonCommand, p, r, cg, flags);
	break; }

    case CE_PushButtonLabel: {
	QPushButton *button = (QPushButton *) widget;
	QRect ir = r;
	if (button->isDown())
	    ir.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
		      pixelMetric(PM_ButtonShiftVertical, widget));

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

	drawItem(p, ir, AlignCenter, cg, button ? button->isEnabled() : true,
		 button->pixmap(), button->text());
	break; }
    }
}


/*
  Returns a contents(?) subrect.
*/
QRect QCommonStyle::subRect(SubRect r, const QWidget *widget) const
{
    return QRect();
}


/*!
  Draws a complex control.
*/
void QCommonStyle::drawComplexControl( ComplexControl control,
				       QPainter* p,
				       const QWidget *w,
				       const QRect &r,
				       const QColorGroup &cg,
				       CFlags flags,
				       SCFlags sub,
				       SCFlags subActive,
				       void *data ) const
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
    return QRect();
}


/*!
  Returns the subcontrol in a complex control.
*/
QCommonStyle::SubControl QCommonStyle::querySubControl(ComplexControl control,
						       const QWidget *widget,
						       const QPoint &pos,
						       void *data ) const
{
    return SC_None;
}


/*!
  Returns a pixel metric.
*/
int QCommonStyle::pixelMetric(PixelMetric m, const QWidget * widget) const
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

    case PM_DefaultFrameWidth:
	ret = 2;
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
    switch (contents) {
    case CT_PushButtonContents: {
	QPushButton *button = (QPushButton *) widget;
	int w = contentsSize.width(),
	    h = contentsSize.height(),
	   bm = pixelMetric(PM_ButtonMargin, widget),
	   fw = pixelMetric(PM_DefaultFrameWidth, widget);

	w += bm;
	h += bm;

	if (button->isDefault() || button->autoDefault()) {
	    int dbw = pixelMetric(PM_ButtonDefaultIndicator, widget) * 2;
	    w += dbw;
	    h += dbw;
	}

	return QSize(w, h); }
    }

    return contentsSize;
}


/*!
  Returns a feel hint.
*/
int QCommonStyle::feelHint(FeelHint, const QWidget *, void **) const
{
    return 0;
}

