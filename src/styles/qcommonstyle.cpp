/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcommonstyle.cpp#6 $
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
    drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
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
void QCommonStyle::drawTabMask( QPainter* p,  const  QTabBar* /* tb*/ , QTab* t, bool /* selected */ )
{
#ifndef QT_NO_TABBAR
    p->drawRect( t->rect() );
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
void QCommonStyle::drawRangeControlWidgetButton( QPainter *p, int x, int y, int w, int h,
						 const QColorGroup &g, QRangeControlWidget*,
						 bool /*downbtn*/, bool /*enabled*/, bool down )
{
    drawButton( p, x, y, w, h, g, down );
}

/*! \reimp */
void QCommonStyle::drawRangeControlWidgetSymbol( QPainter *p, int x, int y, int w, int h,
						 const QColorGroup &g, QRangeControlWidget* rc,
						 bool downbtn, bool /*enabled*/, bool down )
{
    p->save();
    if ( rc->buttonSymbols() == QRangeControlWidget::PlusMinus ) {
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
    drawItem( p, x, y, w, h, AlignCenter + ShowPrefix, g, enabled, 0, text );
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

#define TITLEBAR_PAD 2
#define TITLEBAR_SEPARATION 1
#define TITLEBAR_PIXMAP_WIDTH 12
#define TITLEBAR_PIXMAP_HEIGHT 12
#define TITLEBAR_CONTROL_WIDTH (TITLEBAR_PAD+TITLEBAR_PIXMAP_WIDTH)
#define TITLEBAR_CONTROL_HEIGHT (TITLEBAR_PAD+TITLEBAR_PIXMAP_HEIGHT)

/*!
 \reimp
 */
void QCommonStyle::titleBarMetrics( const QTitleBar*tb, int &ctrlW, int &ctrlH, 
				    int &titleH, int &titleW) const
{
    titleH = 18;
    if(tb->window) {
	titleW = tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*3)-
		 (TITLEBAR_PAD+TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH);
	ctrlH = TITLEBAR_CONTROL_HEIGHT;
	ctrlW = TITLEBAR_CONTROL_WIDTH;
    } else {
	titleW = tb->width();
	ctrlH = ctrlW = 0;
    }
}

/*!
 \reimp
 */
void QCommonStyle::drawTitleBarControls( QPainter*p,  const QTitleBar*tb,
					uint controls, uint activeControl )
{
    if(tb->window) {
	QRegion titleReg(0, 0, tb->width(), tb->height());
	QRect r;

	r = QRect(tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)),
		  2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	titleReg -= r;
	if(controls & TitleCloseButton) {
	    QPixmap pm(titleBarPixmap(tb, TitleCloseButton));
	    drawToolButton( p, r.x(), r.y(), r.width(), r.height(), 
			    tb->colorGroup(), FALSE, activeControl & TitleCloseButton, TRUE );
	    p->drawPixmap( r.x()+(TITLEBAR_PAD/2), TITLEBAR_PAD, pm );
	}

	r = QRect(tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*2),
		  2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	titleReg -= r;
	if(controls & TitleMaxButton) {
	    QPixmap pm(titleBarPixmap(tb, TitleMaxButton));
	    drawToolButton( p, r.x(), r.y(), r.width(), r.height(), 
			    tb->colorGroup(), FALSE, activeControl & TitleMaxButton, TRUE );
	    p->drawPixmap( r.x()+(TITLEBAR_PAD/2), TITLEBAR_PAD, pm );
	}

	r = QRect(tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*3),
		  2, TITLEBAR_CONTROL_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	titleReg -= r;
	if(controls & TitleMinButton || controls & TitleNormalButton) {
	    QStyle::TitleControl ctrl = controls & TitleNormalButton ? TitleNormalButton : TitleMinButton;
	    QPixmap pm(titleBarPixmap(tb, ctrl));
	    drawToolButton( p, r.x(), r.y(), r.width(), r.height(), 
			    tb->colorGroup(), FALSE, activeControl & ctrl, TRUE );
	    p->drawPixmap( r.x()+(TITLEBAR_PAD/2), TITLEBAR_PAD, pm );
	}

	r = QRect(TITLEBAR_PAD+TITLEBAR_SEPARATION, 2, TITLEBAR_PIXMAP_WIDTH, TITLEBAR_CONTROL_HEIGHT);
	titleReg -= r;
	if(controls & TitleSysMenu) 
	    p->drawPixmap( r.x(), r.y(), tb->pixmap );

	p->setClipRegion(titleReg);
    }

    if(controls & TitleLabel) {
	QColor left = tb->act || !tb->window ? tb->aleftc : tb->ileftc;
	QColor right = tb->act || !tb->window ? tb->arightc : tb->irightc;
	if ( left != right ) {
	    double rS = left.red();
	    double gS = left.green();
	    double bS = left.blue();

	    double rD = double(right.red() - rS) / tb->width();
	    double gD = double(right.green() - gS) / tb->width();
	    double bD = double(right.blue() - bS) / tb->width();

	    for ( int sx = 0; sx < 0; sx++ ) {
		rS+=rD;
		gS+=gD;
		bS+=bD;
		p->setPen( QColor( (int)rS, (int)gS, (int)bS ) );
		p->drawLine( sx, 0, sx, tb->height() );
	    }
	} else {
	    p->fillRect( 0, 0, tb->width(), tb->height(), left );
	}

	p->setPen( tb->act || !tb->window ? tb->atextc : tb->itextc );
	QRect r;
	if(tb->window)
	    r = QRect( TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH, 0,
		       tb->width()-((TITLEBAR_CONTROL_WIDTH+TITLEBAR_SEPARATION)*3)-
		       (TITLEBAR_PAD+TITLEBAR_SEPARATION+TITLEBAR_CONTROL_WIDTH),
		       tb->height());
	else
	    r = QRect(0, 0, tb->width(), tb->height());
	p->drawText(r.x()+2, r.y(), r.width(), r.height(), 
		    AlignAuto | AlignVCenter | SingleLine, tb->cuttext );
    }
}

/*!
 \reimp
 */
QStyle::TitleControl QCommonStyle::titleBarPointOver( const QTitleBar*tb, const QPoint&point )
{
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
}

/*!
 \reimp
 */
QStyle::ListViewItemControl
QCommonStyle::listViewItemPointOver( const QListViewItem *i, const QPoint &pos )
{
    if(pos.x() >= 0 && pos.x() < i->listView()->treeStepSize())
       return ListViewExpand;
    return ListViewNone;    
}

#endif







