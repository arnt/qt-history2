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
#include "qgroupbox.h"
#include <limits.h>


// NOT REVISED
/*!
  \class QCommonStyle qcommonstyle.h
  \brief The QCommonStyle class encapsulates the common Look and Feel of a GUI.

  \ingroup appearance

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

 /*!\reimp
*/
QCommonStyle::~QCommonStyle()
{
}


/*!\reimp
*/
void QCommonStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
                                    QMenuItem* mi, QColorGroup& g,
                                    bool, bool, bool )
{
#ifndef QT_NO_COMPLEXWIDGETS
    drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
            g, mi->isEnabled(), mi->pixmap(), mi->text(), -1, &g.buttonText() );
#endif
}



/*!\reimp
 */
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
        QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode );
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





/*!\reimp
 */
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

/*!\reimp
 */
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

/*!\reimp
 */
void QCommonStyle::drawTabMask( QPainter* p,  const  QTabBar* /* tb*/ , QTab* t, bool /* selected */ )
{
#ifndef QT_NO_TABBAR
    p->drawRect( t->rect() );
#endif
}

/*!\reimp
 */
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



QRect QCommonStyle::pushButtonContentsRect( QPushButton* btn ) const
{
#ifndef QT_NO_PUSHBUTTON
    int fw = 0;
    if ( btn->isDefault() || btn->autoDefault() )
        fw = buttonDefaultIndicatorWidth();

    return buttonRect( fw, fw, btn->width()-2*fw, btn->height()-2*fw );
#endif
}



void QCommonStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken,
                     const QBrush *fill )
{
    QStyle::drawToolButton( p, x, y, w, h, g, sunken, fill );
}


void QCommonStyle::drawToolButton( QToolButton* btn, QPainter *p)
{
#ifndef QT_NO_TOOLBUTTON
    if ( !btn )
        return;

    int x = 0;
    int y = 0;
    int w = btn->width();
    int h = btn->height();
    const QColorGroup &g = btn->colorGroup();
    bool sunken = ( btn->isOn() && !btn->son ) || btn->isDown();
    QBrush fill = btn->colorGroup().brush( btn->backgroundMode() == QToolButton::PaletteBackground ?
                                           QColorGroup::Background : QColorGroup::Button );
    if ( guiStyle() == WindowsStyle && btn->isOn() )
        fill = QBrush( g.light(), Dense4Pattern );
#if defined(Q_WS_WIN)
    if ( btn->uses3D() || btn->isDown() || ( btn->isOn() && !btn->son ) ) {
        // WIN2000 is really tricky here!
        bool drawRect = btn->isOn();
        if ( guiStyle() == WindowsStyle && btn->isOn() &&
             ( qt_winver == Qt::WV_2000 ||
	       qt_winver == Qt::WV_98 ||
	       qt_winver == Qt::WV_XP ) &&
             btn->uses3D() ) {
            fill = btn->colorGroup().brush( QColorGroup::Button );
            drawRect = FALSE;
        }
        if ( guiStyle() == WindowsStyle &&
             ( qt_winver == Qt::WV_2000 ||
	       qt_winver == Qt::WV_98 ||
	       qt_winver == Qt::WV_XP ) &&
             btn->autoRaise() ) {
            drawPanel( p, x, y, w, h, g, sunken, 1, &fill );
            if ( drawRect ) {
                p->setPen( QPen( g.color( QColorGroup::Button ) ) );
                p->drawRect( x + 1, y + 1, w - 2, h - 2 );
            }
        } else {
            drawToolButton( p, x, y, w, h, g, sunken, &fill );
        }
    } else if ( btn->parentWidget() && btn->parentWidget()->backgroundPixmap() &&
                !btn->parentWidget()->backgroundPixmap()->isNull() ) {
        p->drawTiledPixmap( 0, 0, btn->width(), btn->height(),
                            *btn->parentWidget()->backgroundPixmap(),
                            btn->x(), btn->y() );
    }
#else
    if ( btn->uses3D() || btn->isDown() || ( btn->isOn() && !btn->son ) ) {
        drawToolButton( p, x, y, w, h, g, sunken, &fill );
    } else if ( btn->parentWidget() && btn->parentWidget()->backgroundPixmap() &&
              !btn->parentWidget()->backgroundPixmap()->isNull() ) {
        p->drawTiledPixmap( 0, 0, btn->width(), btn->height(),
                            *btn->parentWidget()->backgroundPixmap(),
                            btn->x(), btn->y() );
    } else {
        if ( btn->parentWidget() )
            fill = QBrush( btn->parentWidget()->backgroundColor() );
        drawToolButton( p, x - 10, y - 10, w + 20, h + 20, g, sunken, &fill );
    }
#endif
#endif
}

void QCommonStyle::drawTitleBar( QPainter *p,
		       const QRect &r, const QColor &left, const QColor &right,
		       bool /*active*/ )
{
    if ( left != right ) {
	double rS = left.red();
	double gS = left.green();
	double bS = left.blue();

	double rD = double(right.red() - rS) / r.width();
	double gD = double(right.green() - gS) / r.width();
	double bD = double(right.blue() - bS) / r.width();

	for ( int x = r.x(); x < r.width(); x++ ) {
	    rS+=rD;
	    gS+=gD;
	    bS+=bD;
	    p->setPen( QColor( (int)rS, (int)gS, (int)bS ) );
	    p->drawLine( x, r.y(), x, r.height() );
	}
    } else {
	p->fillRect( r, left );
    }
}

void QCommonStyle::drawTitleBarLabel( QPainter *p,
		       const QRect &r, const QString &text,
		       const QColor &tc, bool /*active*/ )
{
    p->setPen( tc );
    p->drawText( r, AlignAuto | AlignVCenter | SingleLine, text );
}

void QCommonStyle::drawTitleBarButton( QPainter *p, const QRect &r, const QColorGroup &g, bool down )
{
    drawToolButton( p, r.x(), r.y(), r.width(), r.height(), g, down );
}

void QCommonStyle::drawTitleBarButtonLabel( QPainter *p, const QRect &r, const QPixmap *pm, int /*button*/, bool down )
{
    if ( pm ) {
	QSize sdiff = r.size() - pm->size();
	int x = 0;
	int y = 0;
	if ( down )
	    getButtonShift( x, y );

	p->drawPixmap( x + sdiff.width() / 2, y + sdiff.height() / 2 ,*pm );
    }
}

// header
void QCommonStyle::drawHeaderSection( QPainter *p, const QRect &rect, const QColorGroup &g, bool down )
{
    drawBevelButton( p, rect.x(), rect.y(), rect.width(), rect.height(), g, down );
}

// spinbox
void QCommonStyle::drawSpinBoxButton( QPainter *p, const QRect &rect, const QColorGroup &g,
		    const QSpinBox * /*sp*/, bool /*upDown*/, bool /*enabled*/, bool down )
{
    drawButton( p, rect.x(), rect.y(), rect.width(), rect.height(), g, down );
}

void QCommonStyle::drawSpinBoxSymbol( QPainter *p, const QRect &rect, const QColorGroup &g, const QSpinBox *sp,
			    bool downbtn, bool /*enabled*/, bool down )
{
    if ( sp->buttonSymbols() == QSpinBox::PlusMinus ) {
	
	int length;
	if ( rect.width() <= 8 || rect.height() <= 6 ) {
	    length = QMIN( rect.width()-2, rect.height()-2 );
	}
	else {
	    length = QMIN( 2*rect.width() / 3, 2*rect.height() / 3 );
	    p->setPen( QPen( g.buttonText(), 1 ) );
	}
	int xmarg = ( rect.width() - length ) / 2;
	int ymarg = ( rect.height() - length ) / 2;

	p->drawLine( rect.left() + xmarg, ( rect.y() + rect.height() / 2 - 1 ),
		     rect.right() - xmarg, ( rect.y() + rect.height() / 2 - 1 ) );
	if ( !downbtn )
	    p->drawLine( ( rect.x() + rect.width() / 2 ) - 1, rect.top() + ymarg,
			 ( rect.x() + rect.width() / 2 ) - 1, rect.bottom() - ymarg );
    } else {
	int w = rect.width()-4;
	if ( w < 3 )
	    return;
	else if ( !(w & 1) )
	    w--;
	w -= ( w / 7 ) * 2;		// Empty border
	int h = w/2 + 2;        // Must have empty row at foot of arrow

	int x = rect.x() + rect.width() / 2 - w / 2 - 1;
	int y = rect.y() + rect.height() / 2 - h / 2 - 1;

	QPointArray a;
	if ( downbtn )
	    a.setPoints( 3,  0, 1,  w-1, 1,  h-2, h-1 );
	else
	    a.setPoints( 3,  0, h-1,  w-1, h-1,  h-2, 1 );
	p->save();
	int sx = 0;
	int sy = 0;
	if ( down )
	    getButtonShift( sx, sy );
	p->translate( x + sx, y + sy );
	p->setBrush( g.foreground() );
	p->drawPolygon( a );
	p->restore();
    }
}

// groupbox
void QCommonStyle::drawGroupBoxTitle( QPainter *p, const QRect &rect, const QColorGroup &g, const QString &text, bool enabled )
{
    drawItem( p, rect.x(), rect.y(), rect.width(), rect.height(), AlignCenter + ShowPrefix, g, enabled, 0, text );
}

void QCommonStyle::drawGroupBoxFrame( QPainter *p, const QRect & /*rect*/, const QColorGroup &g, const QGroupBox *gb )
{
    QRect		r = gb->frameRect();
    QPoint		p1, p2;
    QFrame::Shape	type  = gb->frameShape();
    QFrame::Shadow	cstyle = gb->frameShadow();
    int			lwidth = gb->lineWidth();
    int			mlwidth = gb->midLineWidth();

    switch ( type ) {
    case QFrame::Box:
        if ( cstyle == QFrame::Plain )
            qDrawPlainRect( p, r, g.foreground(), lwidth );
        else
            qDrawShadeRect( p, r, g, cstyle == QFrame::Sunken, lwidth,
                            mlwidth );
        break;

    case QFrame::StyledPanel:
        if ( cstyle == QFrame::Plain )
            qDrawPlainRect( p, r, g.foreground(), lwidth );
        else
            drawPanel( p, r.x(), r.y(), r.width(), r.height(), g, cstyle == QFrame::Sunken, lwidth );
        break;

    case QFrame::PopupPanel:
        if ( cstyle == QFrame::Plain )
            qDrawPlainRect( p, r, g.foreground(), lwidth );
        else
            drawPopupPanel( p, r.x(), r.y(), r.width(), r.height(), g, lwidth );
        break;

    case QFrame::Panel:
        if ( cstyle == QFrame::Plain )
            qDrawPlainRect( p, r, g.foreground(), lwidth );
        else
            qDrawShadePanel( p, r, g, cstyle == QFrame::Sunken, lwidth );
        break;

    case QFrame::WinPanel:
        if ( cstyle == QFrame::Plain )
            qDrawPlainRect( p, r, g.foreground(), 2 );
        else
            qDrawWinPanel( p, r, g, cstyle == QFrame::Sunken );
        break;
    case QFrame::MenuBarPanel:
        drawMenuBarPanel( p, r.x(), r.y(), r.width(), r.height(), g );
        break;
    case QFrame::ToolBarPanel:
        drawToolBarPanel( p, r.x(), r.y(), r.width(), r.height(), g );
        break;
    case QFrame::HLine:
    case QFrame::VLine:
        if ( type == QFrame::HLine ) {
            p1 = QPoint( r.x(), r.height()/2 );
            p2 = QPoint( r.x()+r.width(), p1.y() );
        }
        else {
            p1 = QPoint( r.x()+r.width()/2, 0 );
            p2 = QPoint( p1.x(), r.height() );
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

// statusbar
void QCommonStyle::drawStatusBarSection( QPainter *p, const QRect &rect, const QColorGroup &g, bool /*permanent*/ )
{
    qDrawShadeRect( p, rect, g, TRUE, 1, 0, 0 );
}

void QCommonStyle::drawSizeGrip( QPainter *p, const QRect &rect, const QColorGroup &g )
{
    p->save();
    int w = QMIN( rect.height(),rect.width() );
    if ( rect.height() > rect.width() )
	p->translate( 0, rect.height() - rect.width() );
    else
	p->translate( rect.width() - rect.height(), 0 );

    int x = rect.x();
    int y = rect.y();
    int s = w / 3;

    for ( int i = 0; i < 3; ++i ) {
	p->setPen( QPen( g.mid(), 1 ) );
	p->drawLine(  x-1, w, w,  y-1 );
	p->setPen( QPen( g.dark(), 1 ) );
	p->drawLine(  x, w, w,  y );
	x += s;
	y += s;
    }
    p->setPen( QPen( g.mid(), 1 ) );
    p->drawPoint( w-2, w-2 );
    p->setPen( QPen( g.dark(), 1 ) );
    p->drawPoint( w-1, w-1 );

    p->restore();
}

#endif
