/****************************************************************************
** $Id: $
**
** Implementation of Motif-like style class
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

#include "qmotifstyle.h"

#ifndef QT_NO_STYLE_MOTIF

#include "qpopupmenu.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qwidget.h"
#include "qpushbutton.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qlistview.h"
#include "qsplitter.h"
#include <limits.h>



// old constants that might still be useful...
static const int motifItemFrame         = 2;    // menu item frame width
static const int motifSepHeight         = 2;    // separator item height
static const int motifItemHMargin       = 3;    // menu item hor text margin
static const int motifItemVMargin       = 2;    // menu item ver text margin
static const int motifArrowHMargin      = 6;    // arrow horizontal margin
static const int motifTabSpacing        = 12;   // space between text and tab
static const int motifCheckMarkHMargin  = 2;    // horiz. margins of check mark
static const int motifCheckMarkSpace    = 12;


/*!
  \class QMotifStyle qmotifstyle.h
  \brief The QMotifStyle class provides Motif look and feel.
  \ingroup appearance

  This class implements the Motif look and feel. It almost completely
  resembles the original Motif look as defined by the Open Group, but
  also contains minor improvements. The Motif style is Qt's default
  GUI style on UNIX platforms.
*/

/*!
    Constructs a QMotifStyle.

    If useHighlightCols is FALSE (default value), then the style will
    polish the application's color palette to emulate the Motif way of
    highlighting, which is a simple inversion between the base and the
    text color.
*/
QMotifStyle::QMotifStyle( bool useHighlightCols ) : QCommonStyle(MotifStyle)
{
    highlightCols = useHighlightCols;
}

/*!\reimp
*/
QMotifStyle::~QMotifStyle()
{
}

/*!\reimp
*/
int QMotifStyle::buttonDefaultIndicatorWidth() const
{
#define Q_NICE_MOTIF_DEFAULT_BUTTON
#ifdef Q_NICE_MOTIF_DEFAULT_BUTTON
    return 3;
#endif
}

/*!\reimp
*/
int QMotifStyle::sliderThickness() const
{
#define Q_NICE_MOTIF_SLIDER_THICKNESS
#ifdef Q_NICE_MOTIF_SLIDER_THICKNESS
    return 24;
#endif
}

/*!
  If the argument is FALSE, the style will polish the
  application's color palette to emulate the
  Motif way of highlighting, which is a simple inversion between the
  base and the text color.

  The effect will show up the next time a application palette is set
  via QApplication::setPalette(). The current color palette of the
  application remains unchanged.

  \sa QStyle::polish( QPalette& )
 */
void QMotifStyle::setUseHighlightColors( bool arg )
{
    highlightCols = arg;
}

/*!
  Returns whether the style treats the highlight colors of the palette
  Motif-like, which is a simple inversion between the base and the
  text color. The default is FALSE.
 */
bool QMotifStyle::useHighlightColors() const
{
    return highlightCols;
}

/*! \reimp */

void QMotifStyle::polish( QPalette& pal )
{
    if ( pal.active().light() == pal.active().base() ) {
        QColor nlight = pal.active().light().dark(108 );
        pal.setColor( QPalette::Active, QColorGroup::Light, nlight ) ;
        pal.setColor( QPalette::Disabled, QColorGroup::Light, nlight ) ;
        pal.setColor( QPalette::Inactive, QColorGroup::Light, nlight ) ;
    }

    if ( highlightCols )
        return;

    // force the ugly motif way of highlighting *sigh*
    QColorGroup disabled = pal.disabled();
    QColorGroup active = pal.active();

    pal.setColor( QPalette::Active, QColorGroup::Highlight,
                  active.text() );
    pal.setColor( QPalette::Active, QColorGroup::HighlightedText,
                  active.base());
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight,
                  disabled.text() );
    pal.setColor( QPalette::Disabled, QColorGroup::HighlightedText,
                  disabled.base() );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight,
                  active.text() );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText,
                  active.base() );
}

/*!
 \reimp
 \internal
 Keep QStyle::polish() visible.
*/
void QMotifStyle::polish( QWidget* w )
{
    QStyle::polish(w);
}

/*!
 \reimp
 \internal
 Keep QStyle::polish() visible.
*/
void QMotifStyle::polish( QApplication* a )
{
    QStyle::polish(a);
}



void QMotifStyle::drawPrimitive( PrimitiveOperation op,
				 QPainter *p,
				 const QRect &r,
				 const QColorGroup &cg,
				 PFlags flags,
				 void *data ) const
{
    switch( op ) {
    case PO_ButtonCommand:
    case PO_ButtonBevel:
    case PO_HeaderSection:
	qDrawShadePanel( p, r, cg, bool(flags & PStyle_Sunken),
			 pixelMetric(PM_DefaultFrameWidth),
			 &cg.brush(QColorGroup::Button) );
	break;
	
    case PO_FocusRect:
	// PO_FocusRect is handled in QCommonStyle...
	QCommonStyle::drawPrimitive( op, p, r, cg, flags, data );
	break;
	
    case PO_Indicator: {
#ifndef QT_NO_BUTTON
	bool on = flags & PStyle_Off;
	bool showUp = !( flags ^ on );
	QBrush fill;
	if ( flags & PStyle_NoChange ) {
	    qDrawPlainRect( p, r, cg.text(),
			    1, &fill );
	    p->drawLine( r.x() + r.width() - 1, r.y(),
			 r.x(), r.y() + r.height() - 1);
	} else
	    qDrawShadePanel( p, r, cg, !showUp,
			     pixelMetric(PM_DefaultFrameWidth), &fill );
#endif
	break; }
    case PO_ExclusiveIndicator: {
	p->setBrush( Qt::color1 );
	p->setPen( Qt::color1 );
	
	QPointArray a;
	a.setPoints( 4, 0, 6, 6, 0, 12, 6, 6, 12 );
	a.translate( r.x(), r.y() );
	p->drawPolygon( a );
	break; }
    case PO_MenuBarItem: {
	if ( flags & PStyle_On )  // active item
	    qDrawShadePanel( p, r, cg, FALSE, motifItemFrame,
			     &cg.brush(QColorGroup::Button) );
	else  // other item
	    p->fillRect( r, cg.brush(QColorGroup::Button) );
	QCommonStyle::drawPrimitive( op, p, r, cg, flags, data );
	break; }
    case PO_Panel:
    case PO_PanelPopup: {
	// not sure if this is correct...
	void **sdata = (void **) data;
	int lw = pixelMetric(PM_DefaultFrameWidth);
	if ( sdata )
	    lw = *((int *) sdata[0]);
	
	if ( lw == 2 )
	    qDrawShadePanel( p, r, cg, bool(flags & PStyle_Sunken), 0,
			     &cg.brush(QColorGroup::Button) );
	else
	    QCommonStyle::drawPrimitive( op, p, r, cg, flags, data );
	break; }
    case PO_ArrowUp:
    case PO_ArrowDown:
    case PO_ArrowRight:
    case PO_ArrowLeft: {
	QRect rect = r;
	QPointArray bFill;
	QPointArray bTop;
	QPointArray bBot;
	QPointArray bLeft;
	QWMatrix matrix;
	bool vertical = op == PO_ArrowUp || op == PO_ArrowDown;
	bool horizontal = !vertical;
	int dim = rect.width() < rect.height() ? rect.width() : rect.height();
	int colspec = 0x0000;	
	if ( dim < 2 )
	    break;
	if ( rect.width() > dim ) {
	    rect.setX( rect.x() + ((rect.width() - dim ) / 2) );
	    rect.setWidth( dim );
	}
	if ( rect.height() > dim ) {
	    rect.setY( rect.y() + ((rect.height() - dim ) / 2 ));
	    rect.setHeight( dim );
	}
	if ( dim > 3 ) {
	    if ( dim > 6 )
		bFill.resize( dim & 1 ? 3 : 4 );
	    bTop.resize( (dim / 2) * 2 );
	    bBot.resize( dim & 1 ? dim + 1 : dim );
	    bLeft.resize( dim > 4 ? 4 : 2 );
	    bLeft.putPoints( 0, 2, 0, 0, 0, dim - 1 );
	    if ( dim > 4 )
		bLeft.putPoints( 2 ,2, 1, 2, 1, dim - 3 );
	    bTop.putPoints( 0, 4, 1, 0, 1, 1, 2, 1, 3, 1 );
	    bBot.putPoints( 0, 4, 1, dim - 1, 1, dim - 2, 2, dim - 2, 3, dim - 2 );
	    for ( int i = 0; i < dim / 2 - 2; i++ ) {
		bTop.putPoints( i * 2 + 4, 2, 2 + i * 2, 2 + i, 5 + i * 2, 2 + i );
		bBot.putPoints( i * 2 + 4, 2, 2 + i * 2, dim - 3 - i,
				5 + i * 2, dim - 3 - i );
	    }
	    if ( dim & 1 )
		bBot.putPoints(dim - 1, 2, dim - 3, dim / 2, dim - 1, dim / 2);
	    if ( dim > 6 ) {
		bFill.putPoints( dim - 1, 2, dim - 3, dim / 2, dim - 1, dim / 2 );
		if ( dim & 1 )
		    bFill.setPoints( 2, dim - 3, dim / 2 );
		else
		    bFill.putPoints( 2, 2, dim - 4, dim / 2 - 1, dim - 4,
				     dim / 2 );
	    }
	} else {
	    if ( dim == 3 ) {
		bLeft.setPoints( 4, 0, 0, 0, 2, 1, 1, 1, 1 );
		bTop.setPoints( 2, 1, 0, 1, 0 );
		bBot.setPoints( 2, 1, 2, 2, 1 );
	    } else {
		bLeft.setPoints( 2, 0, 0, 0, 1 );
		bTop.setPoints( 2, 1, 0, 1, 0 );
		bBot.setPoints( 2, 1, 1, 1, 1 );
	    }
	}
	
	if ( op == PO_ArrowUp || op == PO_ArrowDown ) {
	    matrix.translate( rect.x(), rect.y() );
	    if ( vertical ) {
		matrix.translate( rect.x(), rect.y() );
		matrix.rotate( -90 );
	    } else {
		matrix.translate( r.width() - 1, r.height() - 1 );
		matrix.rotate( 180 );
	    }
	    if ( flags & PStyle_Sunken )
		colspec = horizontal ? 0x2334 : 0x2343;
	    else
		colspec = horizontal ? 0x1443 : 0x1434;
	}
	QColor *cols[5];
	if ( flags & PStyle_Enabled ) {
	    cols[0] = 0;
	    cols[1] = (QColor *)&cg.button();
	    cols[2] = (QColor *)&cg.mid();
	    cols[3] = (QColor *)&cg.light();
	    cols[4] = (QColor *)&cg.dark();
	} else {
	    cols[0] = 0;
	    cols[1] = (QColor *)&cg.button();
	    cols[2] = (QColor *)&cg.button();
	    cols[3] = (QColor *)&cg.button();
	    cols[4] = (QColor *)&cg.button();
	}

#define CMID *cols[ (colspec>>12) & 0xf ]
#define CLEFT *cols[ (colspec>>8) & 0xf ]
#define CTOP *cols[ (colspec>>4) & 0xf ]
#define CBOT *cols[ colspec & 0xf ]
	
	QPen savePen = p->pen();
	QBrush saveBrush = p->brush();
	QWMatrix wxm = p->worldMatrix();
	QPen pen( NoPen );
	QBrush brush = cg.brush( flags & PStyle_Enabled ? QColorGroup::Button :
				 QColorGroup::Mid );
	p->setPen( pen );
	p->setBrush( brush );
	p->setWorldMatrix( matrix, TRUE );
	p->drawPolygon( NoBrush );
	p->setPen( CLEFT );
	p->drawLineSegments( bLeft );
	p->setPen( CTOP );
	p->drawLineSegments( bTop );
	p->setPen( CBOT );
	p->drawLineSegments( bBot );

	p->setWorldMatrix( wxm );
	p->setBrush( saveBrush );
	p->setPen( savePen );
#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT
	break; }	
	// just pass it on...
    default:
	QCommonStyle::drawPrimitive( op, p, r, cg, flags, data );
	break;
    }
}
	

void QMotifStyle::drawControl( ControlElement element,
			       QPainter *p,
			       const QWidget *widget,
			       const QRect &r,
			       const QColorGroup &cg,
			       CFlags how, void *data ) const
{
    switch( element ) {
    case CE_Splitter: {
	const int motifOffset = 10;
	int sw;
	QSplitter *split;	
	sw = pixelMetric( PM_SplitterWidth );
	split = (QSplitter *)widget;
	if ( split->orientation() == QSplitter::Horizontal ) {	
	    QCOORD xPos = r.x() + r.width() / 2;
	    QCOORD kPos = motifOffset;
	    QCOORD kSize = sw - 2;
	
	    qDrawShadeLine( p, xPos, kPos + kSize - 1, xPos, r.height(), cg );
	    qDrawShadePanel( p, xPos - sw / 2 + 1, kPos, kSize, kSize, cg,
			     FALSE, 1, &cg.brush( QColorGroup::Button ) );
	    qDrawShadeLine( p, xPos, 0, xPos, kPos, cg );
	} else {
	    QCOORD yPos = r.y() + r.height() / 2;
	    QCOORD kPos = r.width() - motifOffset - sw;
	    QCOORD kSize = sw - 2;
	
	    qDrawShadeLine( p, 0, yPos, kPos, yPos, cg );
	    qDrawShadePanel( p, kPos, yPos - sw / 2 + 1, kSize, kSize,
			     cg, FALSE, 1, &cg.brush( QColorGroup::Button ) );
	    qDrawShadeLine( p, kPos + kSize - 1, yPos, r.width(), yPos, cg );
	}
	break; }
    case CE_PushButton: {
	int dw,
	    x1, y1, x2, y2;
	QPushButton *btn;
	btn = ( QPushButton * )widget;
	p->setPen( cg.foreground() );
	p->setBrush( QBrush( cg.button(), NoBrush ) );
	dw = pixelMetric( PM_ButtonDefaultIndicator );
	r.coords( &x1, &y1, &x2, &y2 );
	if ( btn->isDefault() || btn->autoDefault() ) {
	    x1 += dw;
	    y1 += dw;
	    x2 -= dw;
	    y2 -= dw;
	}
	QBrush fill;
	if ( btn->isDown() )
	    fill = cg.brush( QColorGroup::Mid );
	else if ( btn->isOn() )
	    fill = QBrush( cg.mid(), Dense4Pattern );
	else
	    fill = cg.brush( QColorGroup::Button );
	
	if ( btn->isDefault() ) {
	    if ( dw == 0 ) {
		QPointArray a;
		a.setPoints( 9,
			     x1, y1, x2, y1, x2, y2, x1, y2, x1, y1 + 1,
			     x2 - 1, y1 + 1, x2 - 1, y2 -1, x1 + 1, y2 - 1,
			     x1 + 1, y1 + 1 );
		p->setPen( cg.shadow() );
		p->drawPolygon( a );
		x1 += 2;
		y1 += 2;
		x2 -= 2;
		y2 -= 2;
	    } else {
		qDrawShadePanel( p, r, cg, TRUE );
	    }
	}
	
	if ( !btn->isFlat() || btn->isOn() || btn->isDown() )
	    drawPrimitive( PO_ButtonCommand, p,
			   QRect( x1, y1, x2 - x1 + 1, y2 - y1 + 1 ), cg,
			   PStyle_Default | PStyle_Sunken );
	if ( p->brush().style() != NoBrush )
	    p->setBrush( NoBrush );
	break; }
    case CE_TabBarTab: {
	QRect br = r;
	int o;
	QTabBar *tb;
	bool selected = how & CStyle_Selected;
	tb = (QTabBar *)widget;
	o = pixelMetric( PM_DefaultFrameWidth ) > 1 ? 1 : 0;
	if ( tb->shape() == QTabBar::RoundedAbove ) {
	    if ( o ) {
		p->setPen( cg.light() );
		p->drawLine( br.left(), br.bottom(), br.right(), br.bottom() );
		p->setPen( cg.light() );
		p->drawLine( br.left(), br.bottom() - 1, br.right(),
			     br.bottom() - 1 );
		if ( br.left() == 0 )
		    p->drawPoint( tb->rect().bottomLeft() );
	    } else {
		p->setPen( cg.light() );
		p->drawLine( br.left(), br.bottom(), br.right(), br.bottom() );
	    }
	
	    if ( selected ) {
		p->fillRect( QRect( br.left() + 1, br.bottom() - o,
				    br.width() - 3, 2 ),
			     tb->palette().active().brush(QColorGroup::Background));
		p->setPen( cg.background() );
		p->drawLine( br.left() + 1, br.bottom(),
			     br.right() - 2, br.bottom());
		if ( o )
		    p->drawLine( br.left() + 1, br.bottom() - 1,
				 br.right() - 2, br.bottom() - 1 );
		p->drawLine( br.left() + 1, br.bottom(),
			     br.left() + 1, br.top() + 2 );
		p->setPen( cg.light() );
	    } else {
		p->setPen( cg.light() );
		br.setRect( br.left() + 2, br.top() + 2,
			   r.width() - 4, r.height() - 2 );
	    }
	    p->drawLine( br.left(), br.bottom() - 1, br.left(), br.top() + 2 );
	    p->drawPoint( br.left() + 1, br.top() + 1 );
	    p->drawLine( br.left() + 2, br.top(), br.right() - 2, br.top() );
	    p->drawPoint( br.left(), br.bottom() );
	
	    if ( o ) {
		p->drawLine( br.left() + 1, br.bottom(), br.left() + 1,
			     br.top() + 2 );
		p->drawLine( br.left() + 2, br.bottom() - 1,
			     br.right() - 2, br.top() + 1 );
	    }
	    p->setPen( cg.dark() );
	    p->drawLine( br.right() - 1, br.top() + 2, br.right() - 1,
			 br.bottom() - 1 + (selected ? o : - o ) );
	    if ( o ) {
		p->drawPoint( br.right() - 1, br.top() + 1 );
		p->drawLine( br.right(), br.top() + 2, br.right(),
			     br.bottom() - (selected ? 1: 1 + o) );
	    }
	} else if ( tb->shape() == QTabBar::RoundedBelow ) {
	    if ( selected ) {
		p->fillRect( QRect( br.left() + 1, br.top(), br.width() - 3, 1 ),
			     tb->palette().active().brush(QColorGroup::Background));
		p->setPen( cg.background() );
		p->drawLine( br.left() + 1, br.top(), br.right() - 2, br.top() );
		p->drawLine( br.left() + 1, br.top(),
			     br.left() + 1, br.bottom() - 2);
		p->setPen( cg.dark() );
	    } else {
		p->setPen( cg.dark() );
		p->drawLine( br.left(), br.top(), br.right(), br.top() );
		br.setRect( br.left() + 2, br.top(),
			   br.width() - 4, br.height() - 2);
	    }
	
	    p->drawLine( br.right() - 1, br.top(),
			 br.right() - 1, br.bottom() - 2 );
	    p->drawPoint( br.right() - 2, br.bottom() - 2 );
	    p->drawLine( br.right() - 2, br.bottom() - 1,
			 br.left() + 1, r.bottom() - 1 );
	    p->drawPoint( br.left() + 1, br.bottom() - 2 );
	    if ( pixelMetric( PM_DefaultFrameWidth ) > 1 ) {
		p->drawLine( br.right(), br.top(),
			     br.right(), br.bottom() - 1 );
		p->drawPoint( br.right() - 1, br.bottom() - 1 );
		p->drawLine( br.right() - 1, br.bottom(),
			     br.left() + 2, br.bottom() );
	    }
	
	    p->setPen( cg.light() );
	    p->drawLine( br.left(), br.top(), br.left(), br.bottom() - 2 );
	} else {
	    QCommonStyle::drawControl( element, p, widget, br, cg, how, data );
	}
	break; }
    default:
	QCommonStyle::drawControl( element, p, widget, r, cg, how, data );
	break;
    }
}

void QMotifStyle::drawComplexControl( ComplexControl control,
				     QPainter *p,
				     const QWidget *w,
				     const QRect &r,
				     const QColorGroup &cg,
				     CFlags flags,
				     SCFlags sub,
				     SCFlags subActive,
				     void *data ) const
{
    switch ( control ) {
    case CC_SpinWidget:
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
	break;
    default:
	QCommonStyle::drawComplexControl( control, p, w, r, cg, flags,
					  sub, subActive, data );
    }
}

void QMotifStyle::drawSubControl( SCFlags subCtrl,
				 QPainter *p,
				 const QWidget *w,
				 const QRect &r,
				 const QColorGroup &cg,
				 CFlags flags,
				 SCFlags subActive, void *data ) const
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
	
    default:
	break;
    }

	//cheat
    //    QCommonStyle::drawSubControl(subCtrl, p, w, r, cg, flags, subActive, data);
}

int QMotifStyle::pixelMetric( PixelMetric metic, const QWidget *widget ) const
{
     int ret;

//     switch( metric ) {
//     case PM_ButtonDefaultIndicator:
//     case PM_ButtonShiftHorizontal:
//     case PM_ButtonShiftVertical:
// #define Q_NICE_MOTIF_DEFAULT_BUTTON
// #ifdef Q_NICE_MOTIF_DEFAULT_BUTTON	
// 	ret = 3;
// #endif	
// 	break;
//     case PM_SliderMaximumDragDistance:
//     case PM_ScrollBarMaximumDragDistance: {
// 	QScrollBar *sb = (QScrollBar*) widget;
// 	int maxLength,
// 	    sliderMin,
// 	    buttonDim,
// 	    sliderLength,
// 	    b = MOTIF_BORDER,	
// 	    length = HORIZONTAL ? sb->width() : sb->height(),
// 	    extent = HORIZONTAL ? sb->height() : sb->width();
// 	if ( length > ( extent - b * 2 - 1) * 2 + b * 2 )
// 	    buttonDim = extent - b * 2;
// 	else
// 	    buttonDim = ( length - b * 2) / 2 - 1;
	
// 	sliderMin = b + buttonDim;
// 	maxLength = length - b * 2 - buttonDim * 2;
	
// 	if ( sb->maxValue() == sb->minValue() ) {
// 	    sliderLength = maxLength;
// 	} else {
// 	    uint range = sb->maxValue() - sb->minValue();
// 	    sliderLength = (sb->pageStep() * maxLength ) /
// 			   (range + sb->pageStep());
// 	    if ( sliderLength < SLIDER_MIN || range > INT_MAX / 2 )
// 		sliderLength = SLIDER_MIN;
// 	    if ( sliderLength > maxLength )
// 		sliderLength = maxLength;
// 	}
// 	ret = sliderMin + maxLength - sliderLength;
// 	break }
//     case PM_SliderLength: {
// 	// dang duplicated code!
// 	QScrollBar *sb;
// 	sb = (QScrollBar *)widget;
// 	if ( sb->maxValue() == sb->minValue() ) {
// 	    ret = maxLength;
// 	} else {
// 	    uint range = sb->maxValue() - sb->minValue();
// 	    ret = (sb->pageStep() * maxLength ) / (range + sb->pageStep());
// 	    if ( ret < SLIDER_MIN || range > INT_MAX / 2 )
// 		ret = SLIDER_MIN;
// 	    if ( ret > maxLength )
// 		ret = maxLength;
// 	}
// 	break; }
//     case
	
	
	
	
	
    // cheat
    ret =  QCommonStyle::pixelMetric( metic, widget );
    return ret;
}


QRect QMotifStyle::querySubControlMetrics( ComplexControl control,
					   const QWidget *widget,
					   SubControl sc,
					   void *data ) const
{
    switch ( control ) {
    case SC_SpinWidgetUp:
	break;
    case SC_SpinWidgetDown:
	break;
    case SC_SpinWidgetFrame:
	break;
    default:
	QCommonStyle::querySubControlMetrics( control, widget, sc, data );
    }
}

QSize QMotifStyle::sizeFromContents( ContentsType contents,
				     const QWidget *w,
				     const QSize &contentsSize,
				     void *data ) const
{
    //cheat
    return QCommonStyle::sizeFromContents( contents, w, contentsSize, data );
}

/*! \reimp */


void QMotifStyle::drawIndicator( QPainter* p,
                                 int x, int y, int w, int h, const QColorGroup &g,
                                 int s, bool down, bool /*enabled*/ )
{
    bool on = s != QButton::Off;
    bool showUp = !(down ^ on);
    QBrush fill =  showUp || s == QButton::NoChange
                ? g.brush( QColorGroup::Button ) : g.brush( QColorGroup::Mid );
    if ( s == QButton::NoChange ) {
        qDrawPlainRect( p, x, y, w, h, g.text(), 1, &fill );
        p->drawLine(x+w-1,y,x,y+h-1);
    } else
        qDrawShadePanel( p, x, y, w, h, g, !showUp, pixelMetric( PM_DefaultFrameWidth ), &fill );
}


/*! \reimp */

QSize
QMotifStyle::indicatorSize() const
{
    return QSize(13,13);
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
/*! \reimp */

void QMotifStyle::drawExclusiveIndicator( QPainter* p,
                                   int x, int y, int w, int h, const QColorGroup &g,
                                   bool on, bool down, bool /* enabled */ )
{
    static QCOORD inner_pts[] =         // used for filling diamond
    { 2,6, 6,2, 10,6, 6,10 };
    static QCOORD top_pts[] =           // top (^) of diamond
    { 0,6, 6,0 , 11,5, 10,5, 6,1, 1,6, 2,6, 6,2, 9,5 };
    static QCOORD bottom_pts[] =                // bottom (V) of diamond
    { 1,7, 6,12, 12,6, 11,6, 6,11, 2,7, 3,7, 6,10, 10,6 };

    bool showUp = !(down ^ on );
    QPointArray a( QCOORDARRLEN(inner_pts), inner_pts );
    p->eraseRect( x, y, w, h );
    p->setPen( NoPen );
    p->setBrush( showUp ? g.brush( QColorGroup::Button ) :
                          g.brush( QColorGroup::Mid ) )  ;
    a.translate( x, y );
    p->drawPolygon( a );                        // clear inner area
    p->setPen( showUp ? g.light() : g.dark() );
    p->setBrush( NoBrush );
    a.setPoints( QCOORDARRLEN(top_pts), top_pts );
    a.translate( x, y );
    p->drawPolyline( a );                       // draw top part
    p->setPen( showUp ? g.dark() : g.light() );
    a.setPoints( QCOORDARRLEN(bottom_pts), bottom_pts );
    a.translate( x, y );
    p->drawPolyline( a );                       // draw bottom part
}


/*!
  Draws the mask of a mark indicating the state of an exclusive choice
*/
void
QMotifStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y, int, int, bool /* on */)
{
    p->setBrush( Qt::color1 );
    p->setPen( Qt::color1 );

    QPointArray a;
    a.setPoints( 4, 0,6, 6,0, 12,6, 6, 12 );
    a.translate(x,y);
    p->drawPolygon( a );
}


/*! \reimp */

QSize
QMotifStyle::exclusiveIndicatorSize() const
{
    return QSize(13,13);
}




/*! \reimp */

void
QMotifStyle::drawArrow( QPainter *p, ArrowType type, bool down,
                 int x, int y, int w, int h,
                 const QColorGroup &g, bool enabled, const QBrush * /* fill */ )
{
    // ### may be worth caching these as pixmaps, especially with the
    //      cost of rotate() for vertical arrows.

    QPointArray bFill;                          // fill polygon
    QPointArray bTop;                           // top shadow.
    QPointArray bBot;                           // bottom shadow.
    QPointArray bLeft;                          // left shadow.
    QWMatrix    matrix;                         // xform matrix
    bool vertical = type == UpArrow || type == DownArrow;
    bool horizontal = !vertical;
    int  dim = w < h ? w : h;
    int  colspec = 0x0000;                      // color specification array

    if ( dim < 2 )                              // too small arrow
        return;

    // adjust size and center (to fix rotation below)
    if ( w >  dim ) {
        x += (w-dim)/2;
        w = dim;
    }
    if ( h > dim ) {
        y += (h-dim)/2;
        h = dim;
    }

    if ( dim > 3 ) {
        if ( dim > 6 )
            bFill.resize( dim & 1 ? 3 : 4 );
        bTop.resize( (dim/2)*2 );
        bBot.resize( dim & 1 ? dim + 1 : dim );
        bLeft.resize( dim > 4 ? 4 : 2 );
        bLeft.putPoints( 0, 2, 0,0, 0,dim-1 );
        if ( dim > 4 )
            bLeft.putPoints( 2, 2, 1,2, 1,dim-3 );
        bTop.putPoints( 0, 4, 1,0, 1,1, 2,1, 3,1 );
        bBot.putPoints( 0, 4, 1,dim-1, 1,dim-2, 2,dim-2, 3,dim-2 );

        for( int i=0; i<dim/2-2 ; i++ ) {
            bTop.putPoints( i*2+4, 2, 2+i*2,2+i, 5+i*2, 2+i );
            bBot.putPoints( i*2+4, 2, 2+i*2,dim-3-i, 5+i*2,dim-3-i );
        }
        if ( dim & 1 )                          // odd number size: extra line
            bBot.putPoints( dim-1, 2, dim-3,dim/2, dim-1,dim/2 );
        if ( dim > 6 ) {                        // dim>6: must fill interior
            bFill.putPoints( 0, 2, 1,dim-3, 1,2 );
            if ( dim & 1 )                      // if size is an odd number
                bFill.setPoint( 2, dim - 3, dim / 2 );
            else
                bFill.putPoints( 2, 2, dim-4,dim/2-1, dim-4,dim/2 );
        }
    }
    else {
        if ( dim == 3 ) {                       // 3x3 arrow pattern
            bLeft.setPoints( 4, 0,0, 0,2, 1,1, 1,1 );
            bTop .setPoints( 2, 1,0, 1,0 );
            bBot .setPoints( 2, 1,2, 2,1 );
        }
        else {                                  // 2x2 arrow pattern
            bLeft.setPoints( 2, 0,0, 0,1 );
            bTop .setPoints( 2, 1,0, 1,0 );
            bBot .setPoints( 2, 1,1, 1,1 );
        }
    }

    if ( type == UpArrow || type == LeftArrow ) {
        matrix.translate( x, y );
        if ( vertical ) {
            matrix.translate( 0, h - 1 );
            matrix.rotate( -90 );
        } else {
            matrix.translate( w - 1, h - 1 );
            matrix.rotate( 180 );
        }
        if ( down )
            colspec = horizontal ? 0x2334 : 0x2343;
        else
            colspec = horizontal ? 0x1443 : 0x1434;
    }
    else if ( type == DownArrow || type == RightArrow ) {
        matrix.translate( x, y );
        if ( vertical ) {
            matrix.translate( w-1, 0 );
            matrix.rotate( 90 );
        }
        if ( down )
            colspec = horizontal ? 0x2443 : 0x2434;
        else
            colspec = horizontal ? 0x1334 : 0x1343;
    }

    QColor *cols[5];
    if ( enabled ) {
        cols[0] = 0;
        cols[1] = (QColor *)&g.button();
        cols[2] = (QColor *)&g.mid();
        cols[3] = (QColor *)&g.light();
        cols[4] = (QColor *)&g.dark();
    } else {
        cols[0] = 0;
        cols[1] = (QColor *)&g.button();
        cols[2] = (QColor *)&g.button();
        cols[3] = (QColor *)&g.button();
        cols[4] = (QColor *)&g.button();
    }
#define CMID    *cols[ (colspec>>12) & 0xf ]
#define CLEFT   *cols[ (colspec>>8) & 0xf ]
#define CTOP    *cols[ (colspec>>4) & 0xf ]
#define CBOT    *cols[ colspec & 0xf ]

    QPen     savePen   = p->pen();              // save current pen
    QBrush   saveBrush = p->brush();            // save current brush
    QWMatrix wxm = p->worldMatrix();
    QPen     pen( NoPen );
    QBrush brush = g.brush( enabled?QColorGroup::Button:QColorGroup::Mid );

    p->setPen( pen );
    p->setBrush( brush );
    p->setWorldMatrix( matrix, TRUE );          // set transformation matrix
    p->drawPolygon( bFill );                    // fill arrow
    p->setBrush( NoBrush );                     // don't fill

    p->setPen( CLEFT );
    p->drawLineSegments( bLeft );
    p->setPen( CTOP );
    p->drawLineSegments( bTop );
    p->setPen( CBOT );
    p->drawLineSegments( bBot );

    p->setWorldMatrix( wxm );
    p->setBrush( saveBrush );                   // restore brush
    p->setPen( savePen );                       // restore pen

#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT

}


/*!\reimp
*/
void QMotifStyle::drawButton( QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool sunken, const QBrush* fill)
{
    drawPrimitive( PO_ButtonCommand, p, QRect( x, y, w, h), g,
		   sunken ? PStyle_Sunken : PStyle_Default );
//     qDrawShadePanel( p, x, y, w, h, g, sunken, pixelMetric(),
//                      fill ? fill : (sunken ?
//                                     &g.brush( QColorGroup::Mid )      :
//                                     &g.brush( QColorGroup::Button ) ));
}

/*! \reimp */

void QMotifStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool sunken, const QBrush* fill)
{
    QMotifStyle::drawButton(p, x, y, w, h, g, sunken, fill);
}


/*! \reimp */

void
QMotifStyle::drawFocusRect( QPainter* p,
                            const QRect& r, const QColorGroup &g , const QColor* bg, bool atBorder)
{
    if (bg ) {
        int h,s,v;
        bg->hsv(&h,&s,&v);
        if (v >= 128)
            p->setPen( Qt::black );
        else
            p->setPen( Qt::white );
    }
    else
        p->setPen( g.foreground() );
    p->setBrush( NoBrush );
    if ( atBorder )
        p->drawRect( QRect( r.x()+1, r.y()+1, r.width()-2, r.height()-2 ) );
    else
        p->drawRect( r );

}


/*! \reimp */

void
QMotifStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
    QColorGroup g = btn->colorGroup();
    int x1, y1, x2, y2;

    btn->rect().coords( &x1, &y1, &x2, &y2 );   // get coordinates

    p->setPen( g.foreground() );
    p->setBrush( QBrush(g.button(),NoBrush) );

    int diw = buttonDefaultIndicatorWidth();
    if ( btn->isDefault() || btn->autoDefault() ) {
        x1 += diw;
        y1 += diw;
        x2 -= diw;
        y2 -= diw;
    }

    QBrush fill;
    if ( btn->isDown() )
        fill = g.brush( QColorGroup::Mid );
    else if ( btn->isOn() )
        fill = QBrush( g.mid(), Dense4Pattern );
    else
        fill = g.brush( QColorGroup::Button );

    if ( btn->isDefault() ) {
        if ( diw == 0 ) {
            QPointArray a;
            a.setPoints( 9,
                         x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
                         x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1 );
            p->setPen( g.shadow() );
            p->drawPolyline( a );
            x1 += 2;
            y1 += 2;
            x2 -= 2;
            y2 -= 2;
        } else {
            qDrawShadePanel( p, btn->rect(), g, TRUE );
        }
    }

    if ( !btn->isFlat() || btn->isOn() || btn->isDown() )
        drawButton( p, x1, y1, x2-x1+1, y2-y1+1, g, btn->isOn() || btn->isDown(),
                    &fill );

    if ( p->brush().style() != NoBrush )
        p->setBrush( NoBrush );
}



/*! \reimp */
void QMotifStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe, int& overlap) const
{
    QCommonStyle::tabbarMetrics( t, hframe, vframe, overlap );
}

/*! \reimp */
void QMotifStyle::drawTab( QPainter* p, const QTabBar* tb, QTab* t , bool selected )
{
    drawControl( CE_TabBarTab, p, tb, t->rect(), tb->colorGroup(),
		 selected ? CStyle_Selected : CStyle_Default );
   //  QRect r( t->rect() );
//     int o = defaultFrameWidth() > 1 ? 1 : 0;

//     if ( tb->shape()  == QTabBar::RoundedAbove ) {
//         if ( o ) {
//             p->setPen( tb->colorGroup().light() );
//             p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
//             p->setPen( tb->colorGroup().light() );
//             p->drawLine( r.left(), r.bottom()-1, r.right(), r.bottom()-1 );
//             if ( r.left() == 0 )
//                 p->drawPoint( tb->rect().bottomLeft() );
//         }
//         else {
//             p->setPen( tb->colorGroup().light() );
//             p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
//         }

//         if ( selected ) {
//             p->fillRect( QRect( r.left()+1, r.bottom()-o, r.width()-3, 2),
//                          tb->palette().active().brush( QColorGroup::Background ));
//             p->setPen( tb->colorGroup().background() );
//          p->drawLine( r.left()+1, r.bottom(), r.right()-2, r.bottom() );
//          if (o)
//              p->drawLine( r.left()+1, r.bottom()-1, r.right()-2, r.bottom()-1 );
//             p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top()+2 );
//             p->setPen( tb->colorGroup().light() );
//         } else {
//             p->setPen( tb->colorGroup().light() );
//             r.setRect( r.left() + 2, r.top() + 2,
//                        r.width() - 4, r.height() - 2 );
//         }

//         p->drawLine( r.left(), r.bottom()-1, r.left(), r.top() + 2 );
//         p->drawPoint( r.left()+1, r.top() + 1 );
//         p->drawLine( r.left()+2, r.top(),
//                      r.right() - 2, r.top() );
//         p->drawPoint( r.left(), r.bottom());

//         if ( o ) {
//             p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top() + 2 );
//             p->drawLine( r.left()+2, r.top()+1,
//                          r.right() - 2, r.top()+1 );
//         }

//         p->setPen( tb->colorGroup().dark() );
//         p->drawLine( r.right() - 1, r.top() + 2,
//                      r.right() - 1, r.bottom() - 1 + (selected?o:-o));
//         if ( o ) {
//             p->drawPoint( r.right() - 1, r.top() + 1 );
//             p->drawLine( r.right(), r.top() + 2, r.right(), r.bottom() - (selected?1:1+o));
//             p->drawPoint( r.right() - 1, r.top() + 1 );
//         }
//     } else if ( tb->shape()  == QTabBar::RoundedBelow ) {
//         if ( selected ) {
//             p->fillRect( QRect( r.left()+1, r.top(), r.width()-3, 1),
//                          tb->palette().active().brush( QColorGroup::Background ));
//             p->setPen( tb->colorGroup().background() );
//          p->drawLine( r.left()+1, r.top(), r.right()-2, r.top() );
//             p->drawLine( r.left()+1, r.top(), r.left()+1, r.bottom()-2 );
//             p->setPen( tb->colorGroup().dark() );
//         } else {
//             p->setPen( tb->colorGroup().dark() );
//             p->drawLine( r.left(), r.top(), r.right(), r.top() );
//             r.setRect( r.left() + 2, r.top(),
//                        r.width() - 4, r.height() - 2 );
//         }

//         p->drawLine( r.right() - 1, r.top(),
//                      r.right() - 1, r.bottom() - 2 );
//         p->drawPoint( r.right() - 2, r.bottom() - 2 );
//         p->drawLine( r.right() - 2, r.bottom() - 1,
//                      r.left() + 1, r.bottom() - 1 );
//         p->drawPoint( r.left() + 1, r.bottom() - 2 );

//         if (defaultFrameWidth() > 1) {
//             p->drawLine( r.right(), r.top(),
//                          r.right(), r.bottom() - 1 );
//             p->drawPoint( r.right() - 1, r.bottom() - 1 );
//             p->drawLine( r.right() - 1, r.bottom(),
//                          r.left() + 2, r.bottom() );
//         }

//         p->setPen( tb->colorGroup().light() );
//         p->drawLine( r.left(), r.top(),
//                      r.left(), r.bottom() - 2 );

//     } else {
//         QCommonStyle::drawTab( p, tb, t, selected );
//     }

}



#define HORIZONTAL      (sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL        !HORIZONTAL
#define MOTIF_BORDER    pixelMetric( PM_DefaultFrameWidth )
#define SLIDER_MIN      9 // ### motif says 6 but that's too small


/*! \reimp */

void QMotifStyle::scrollBarMetrics( const QScrollBar* sb,
                                    int &sliderMin, int &sliderMax,
                                    int &sliderLength, int &buttonDim ) const
{
    int maxLength;
    int b = MOTIF_BORDER;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2  )
        buttonDim = extent - b*2;
    else
        buttonDim = ( length - b*2 )/2 - 1;

    sliderMin = b + buttonDim;
    maxLength  = length - b*2 - buttonDim*2;

    if ( sb->maxValue() == sb->minValue() ) {
        sliderLength = maxLength;
    } else {
        uint range = sb->maxValue()-sb->minValue();
        sliderLength = (sb->pageStep()*maxLength)/
                        (range + sb->pageStep());
        if ( sliderLength < SLIDER_MIN || range > INT_MAX/2 )
            sliderLength = SLIDER_MIN;
        if ( sliderLength > maxLength )
            sliderLength = maxLength;
    }
    sliderMax = sliderMin + maxLength - sliderLength;

}


/*! \reimp */

void QMotifStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb,
                                         int sliderStart, uint controls,
                                         uint activeControl )
{
// #define ADD_LINE_ACTIVE ( activeControl == AddLine )
// #define SUB_LINE_ACTIVE ( activeControl == SubLine )
//     QColorGroup g  = sb->colorGroup();

//     int sliderMin, sliderMax, sliderLength, buttonDim;
//     scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

//     if ( controls == (AddLine | SubLine | AddPage | SubPage | Slider | First | Last ) )
//         qDrawShadePanel( p, sb->rect(), g, TRUE );

//     if (sliderStart > sliderMax) { // sanity check
//         sliderStart = sliderMax;
//     }

//     int b = MOTIF_BORDER;
//     int dimB = buttonDim;
//     QRect addB;
//     QRect subB;
//     QRect addPageR;
//     QRect subPageR;
//     QRect sliderR;
//     int addX, addY, subX, subY;
//     int length = HORIZONTAL ? sb->width()  : sb->height();
//     int extent = HORIZONTAL ? sb->height() : sb->width();

//     if ( HORIZONTAL ) {
//         subY = addY = ( extent - dimB ) / 2;
//         subX = b;
//         addX = length - dimB - b;
//     } else {
//         subX = addX = ( extent - dimB ) / 2;
//         subY = b;
//         addY = length - dimB - b;
//     }

//     subB.setRect( subX,subY,dimB,dimB );
//     addB.setRect( addX,addY,dimB,dimB );

//     int sliderEnd = sliderStart + sliderLength;
//     int sliderW = extent - b*2;
//     if ( HORIZONTAL ) {
//         subPageR.setRect( subB.right() + 1, b,
//                           sliderStart - subB.right() - 1 , sliderW );
//         addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
//         sliderR .setRect( sliderStart, b, sliderLength, sliderW );
//     } else {
//         subPageR.setRect( b, subB.bottom() + 1, sliderW,
//                           sliderStart - subB.bottom() - 1 );
//         addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
//         sliderR .setRect( b, sliderStart, sliderW, sliderLength );
//     }

//     if ( controls & AddLine )
//         drawArrow( p, VERTICAL ? DownArrow : RightArrow,
//                    ADD_LINE_ACTIVE, addB.x(), addB.y(),
//                    addB.width(), addB.height(), g, TRUE );
//     if ( controls & SubLine )
//         drawArrow( p, VERTICAL ? UpArrow : LeftArrow,
//                    SUB_LINE_ACTIVE, subB.x(), subB.y(),
//                    subB.width(), subB.height(), g, TRUE );

//     QBrush fill = g.brush( QColorGroup::Mid );
//     if (sb->backgroundPixmap() ){
//         fill = QBrush( g.mid(), *sb->backgroundPixmap() );
//     }

//     if ( controls & SubPage )
//         p->fillRect( subPageR, fill );

//     if ( controls & AddPage )
//         p->fillRect( addPageR, fill );

//     if ( controls & Slider ) {
//         QPoint bo = p->brushOrigin();
//         p->setBrushOrigin(sliderR.topLeft());
//         if ( sliderR.isValid() )
//             drawBevelButton( p, sliderR.x(), sliderR.y(),
//                              sliderR.width(), sliderR.height(), g,
//                              FALSE, &g.brush( QColorGroup::Button ) );

//         //      qDrawShadePanel( p, sliderR, g, FALSE, 2, &g.fillButton() );
//         p->setBrushOrigin(bo);
//     }

}


/*!\reimp
 */
int QMotifStyle::sliderLength() const
{
    return 30;
}

/*!\reimp
 */
void QMotifStyle::drawSlider( QPainter *p,
                             int x, int y, int w, int h,
                             const QColorGroup &g,
                              Orientation orient, bool, bool )
{
    drawBevelButton( p, x, y, w, h, g, FALSE, &g.brush( QColorGroup::Button) );
    if ( orient == Horizontal ) {
        QCOORD mid = x + w / 2;
        qDrawShadeLine( p, mid,  y , mid,  y + h - 2,
                        g, TRUE, 1);
    } else {
        QCOORD mid = y +h / 2;
        qDrawShadeLine( p, x, mid,  x + w - 2, mid,
                        g, TRUE, 1);
    }
}

/*!\reimp
 */
void QMotifStyle::drawSliderGroove( QPainter *p,
                                      int x, int y, int w, int h,
                                      const QColorGroup& g, QCOORD /*c */,
                                      Orientation )
{
    qDrawShadePanel( p, x, y, w, h, g, TRUE, 1, &g.brush( QColorGroup::Mid ) );
}


/*! \reimp
*/

int QMotifStyle::splitterWidth() const
{
    return QMAX( 10, QApplication::globalStrut().width() );
}


/*! \reimp
*/

void QMotifStyle::drawSplitter( QPainter *p, int x, int y, int w, int h,
  const QColorGroup &g, Orientation orient)
{
    const int motifOffset = 10;
    int sw = splitterWidth();
    if ( orient == Horizontal ) {
            QCOORD xPos = x + w/2;
            QCOORD kPos = motifOffset;
            QCOORD kSize = sw - 2;

            qDrawShadeLine( p, xPos, kPos + kSize - 1 ,
                            xPos, h, g );
            qDrawShadePanel( p, xPos-sw/2+1, kPos,
                             kSize, kSize, g, FALSE, 1,
                             &g.brush( QColorGroup::Button ));
            qDrawShadeLine( p, xPos, 0, xPos, kPos, g );
        } else {
            QCOORD yPos = y + h/2;
            QCOORD kPos = w - motifOffset - sw;
            QCOORD kSize = sw - 2;

            qDrawShadeLine( p, 0, yPos, kPos, yPos, g );
            qDrawShadePanel( p, kPos, yPos-sw/2+1,
                             kSize, kSize, g, FALSE, 1,
                             &g.brush( QColorGroup::Button ));
            qDrawShadeLine( p, kPos + kSize -1, yPos,
                            w, yPos, g );
        }

}

/*! \reimp
*/
void QMotifStyle::polishPopupMenu( QPopupMenu* p)
{
    p->setLineWidth( pixelMetric( PM_DefaultFrameWidth ) );
    p->setMouseTracking( FALSE );
    if ( !p->testWState( WState_Polished ) )
        p->setCheckable( FALSE );
    p->setLineWidth( 2 );
}




/*! \reimp
*/
void QMotifStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g,
                                 bool act, bool dis )
{
//     const int markW = 6;
//     const int markH = 6;
//     int posX = x + ( w - markW )/2 - 1;
//     int posY = y + ( h - markH )/2;

//     if ( defaultFrameWidth() < 2) {
//         // Could do with some optimizing/caching...
//         QPointArray a( 7*2 );
//         int i, xx, yy;
//         xx = posX;
//         yy = 3 + posY;
//         for ( i=0; i<3; i++ ) {
//             a.setPoint( 2*i,   xx, yy );
//             a.setPoint( 2*i+1, xx, yy+2 );
//             xx++; yy++;
//         }
//         yy -= 2;
//         for ( i=3; i<7; i++ ) {
//             a.setPoint( 2*i,   xx, yy );
//             a.setPoint( 2*i+1, xx, yy+2 );
//             xx++; yy--;
//         }
//         if ( dis && !act ) {
//             int pnt;
//             p->setPen( g.highlightedText() );
//             QPoint offset(1,1);
//             for ( pnt = 0; pnt < (int)a.size(); pnt++ )
//                 a[pnt] += offset;
//             p->drawLineSegments( a );
//             for ( pnt = 0; pnt < (int)a.size(); pnt++ )
//                 a[pnt] -= offset;
//         }
//         p->setPen( g.text() );
//         p->drawLineSegments( a );

//         qDrawShadePanel( p, posX-2, posY-2, markW+4, markH+6, g, TRUE,
//                          defaultFrameWidth());
//     }
//     else {
//         qDrawShadePanel( p, posX, posY, markW, markH, g, TRUE,
//                     defaultFrameWidth(), &g.brush( QColorGroup::Mid ) );
//     }
}


/*! \reimp
*/
int QMotifStyle::extraPopupMenuItemWidth( bool checkable, int maxpmw,
                                          QMenuItem* mi,
                                          const QFontMetrics&/* fm */) const
{
    int w = 2*motifItemHMargin + 2*motifItemFrame; // a little bit of border can never harm

    if ( mi->isSeparator() )
        return 10; // arbitrary
    else if ( mi->pixmap() )
        w += mi->pixmap()->width();     // pixmap only

    if ( !mi->text().isNull() ) {
        if ( mi->text().find('\t') >= 0 )       // string contains tab
            w += motifTabSpacing;
    }

    if ( checkable )
        maxpmw = QMAX( maxpmw, motifCheckMarkSpace );
    w += maxpmw;

    if ( maxpmw > 0 || checkable ) // we have a check-column ( iconsets or checkmarks)
        w += motifCheckMarkHMargin; // add space to separate the columns

    return w;
}

/*! \reimp
*/
int QMotifStyle::popupMenuItemHeight( bool /* checkable*/, QMenuItem* mi,
                                      const QFontMetrics& fm ) const
{
    int h = 0;
    if ( mi->isSeparator() ) {                  // separator height
        h = motifSepHeight;
    } else if ( mi->pixmap() ) {                // pixmap height
        h = mi->pixmap()->height() + 2*motifItemFrame;
    } else {                                    // text height
        h = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    }
    if ( !mi->isSeparator() && mi->iconSet() != 0 ) {
        h = QMAX( h, mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height() + 2*motifItemFrame );
        h += 2;                         // Room for check rectangle
    }
    if ( mi->custom() )
        h = QMAX( h, mi->custom()->sizeHint().height() + 2*motifItemVMargin + 2*motifItemFrame );
    return h;
}

/*! \reimp
*/
void QMotifStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal,
                                     bool act, bool enabled,
                                     int x, int y, int w, int h )
{
    const QColorGroup & g = pal.active();
    bool dis      = !enabled;
    QColorGroup itemg = dis ? pal.disabled() : pal.active();

    if ( checkable )
        maxpmw = QMAX( maxpmw, motifCheckMarkSpace );
    int checkcol          =     maxpmw;

    if ( mi && mi->isSeparator() ) {                    // draw separator
        p->setPen( g.dark() );
        p->drawLine( x, y, x+w, y );
        p->setPen( g.light() );
        p->drawLine( x, y+1, x+w, y+1 );
        return;
    }

    int pw = motifItemFrame;

    if ( act && !dis ) {                        // active item frame
        if (pixelMetric( PM_DefaultFrameWidth ) > 1)
            qDrawShadePanel( p, x, y, w, h, g, FALSE, pw,
                             &g.brush( QColorGroup::Button ) );
        else
            qDrawShadePanel( p, x+1, y+1, w-2, h-2, g, TRUE, 1,
                             &g.brush( QColorGroup::Button ) );
    }
    else                                // incognito frame
        p->fillRect(x, y, w, h, g.brush( QColorGroup::Button ));

    if ( !mi )
        return;

    if ( mi->isChecked() ) {
        if ( mi->iconSet() ) {
            qDrawShadePanel( p, x+motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame,
                             g, TRUE, 1, &g.brush( QColorGroup::Midlight ) );
        }
    } else if ( !act ) {
        p->fillRect(x+motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame,
                    g.brush( QColorGroup::Button ));
    }

    if ( mi->iconSet() ) {              // draw iconset
        QIconSet::Mode mode = QIconSet::Normal; // no disabled icons in Motif
        if (act && !dis )
            mode = QIconSet::Active;
	QPixmap pixmap;
	if ( checkable && mi->isChecked() )
	    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
	else
	    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );

        int pixw = pixmap.width();
        int pixh = pixmap.height();
        QRect cr( x + motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame );
        QRect pmr( 0, 0, pixw, pixh );
        pmr.moveCenter( cr.center() );
        p->setPen( itemg.text() );
        p->drawPixmap( pmr.topLeft(), pixmap );

    } else  if ( checkable ) {  // just "checking"...
        int mw = checkcol;
        int mh = h - 2*motifItemFrame;
        if ( mi->isChecked() ) {
            drawCheckMark( p, x+motifItemFrame,
                           y+motifItemFrame, mw, mh, itemg, act, dis );
        }
    }


    p->setPen( g.buttonText() );

    QColor discol;
    if ( dis ) {
        discol = itemg.text();
        p->setPen( discol );
    }

    int xm = motifItemFrame + checkcol + motifItemHMargin;

    if ( mi->custom() ) {
        int m = motifItemVMargin;
        p->save();
        mi->custom()->paint( p, itemg, act, enabled,
                             x+xm, y+m, w-xm-tab+1, h-2*m );
        p->restore();
    }
    QString s = mi->text();
    if ( !s.isNull() ) {                        // draw text
        int t = s.find( '\t' );
        int m = motifItemVMargin;
        const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
        if ( t >= 0 ) {                         // draw tab text
            p->drawText( x+w-tab-motifItemHMargin-motifItemFrame,
                         y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
            s = s.left( t );
        }
        p->drawText( x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
    } else if ( mi->pixmap() ) {                        // draw pixmap
        QPixmap *pixmap = mi->pixmap();
        if ( pixmap->depth() == 1 )
            p->setBackgroundMode( OpaqueMode );
        p->drawPixmap( x+xm, y+motifItemFrame, *pixmap );
        if ( pixmap->depth() == 1 )
            p->setBackgroundMode( TransparentMode );
    }
    if ( mi->popup() ) {                        // draw sub menu arrow
        int dim = (h-2*motifItemFrame) / 2;
        if ( act ) {
            drawArrow( p, RightArrow,
                       mi->isEnabled(),
                       x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
                       dim, dim, g,
                       mi->isEnabled() );
        } else {
            drawArrow( p, RightArrow,
                       FALSE,
                       x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
                       dim, dim, g, mi->isEnabled() );
        }
    }
}

int get_combo_extra_width( int h, int *return_awh=0 )
{
    int awh;
    if ( h < 8 ) {
        awh = 6;
    } else if ( h < 14 ) {
        awh = h - 2;
    } else {
        awh = h/2;
    }
    if ( return_awh )
        *return_awh = awh;
    return awh*3/2;
}


static void get_combo_parameters( const QRect &r,
                                  int &ew, int &awh, int &ax,
                                  int &ay, int &sh, int &dh,
                                  int &sy )
{
    ew = get_combo_extra_width( r.height(), &awh );

    sh = (awh+3)/4;
    if ( sh < 3 )
        sh = 3;
    dh = sh/2 + 1;

    ay = r.y() + (r.height()-awh-sh-dh)/2;
    if ( ay < 0 ) {
        //panic mode
        ay = 0;
        sy = r.height();
    } else {
        sy = ay+awh+dh;
    }
    if( QApplication::reverseLayout() )
        ax = r.x();
    else
        ax = r.x() + r.width() - ew;
    ax  += (ew-awh)/2;
}

/*! \reimp
 */
void QMotifStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g,
                                    bool /* sunken */,
                                    bool editable,
                                    bool /*enabled */,
                                    const QBrush *fb )
{
//     QBrush fill = fb ? *fb : g.brush( QColorGroup::Button );

//     int awh, ax, ay, sh, sy, dh, ew;
//     get_combo_parameters( buttonRect(x,y,w,h), ew, awh, ax, ay, sh, dh, sy );

//     drawButton( p, x, y, w, h, g, FALSE, &fill );

//     qDrawArrow( p, DownArrow, MotifStyle, FALSE,
//                 ax, ay, awh, awh, g, TRUE );

//     p->setPen( g.light() );
//     p->drawLine( ax, sy, ax+awh-1, sy );
//     p->drawLine( ax, sy, ax, sy+sh-1 );
//     p->setPen( g.dark() );
//     p->drawLine( ax+1, sy+sh-1, ax+awh-1, sy+sh-1 );
//     p->drawLine( ax+awh-1, sy+1, ax+awh-1, sy+sh-1 );

//     if ( editable ) {
//         QRect r( comboButtonRect(x,y,w,h) );
//         qDrawShadePanel( p, QRect(r.x()-1, r.y()-1, r.width()+2, r.height()+2), g, TRUE, 1, &fill );
//     }
}


/*! \reimp
 */
QRect QMotifStyle::comboButtonRect( int x, int y, int w, int h) const
{
//     QRect r = buttonRect( x, y, w, h );
//     int ew = get_combo_extra_width( r.height() );
//     if( QApplication::reverseLayout() )
//         r.moveBy( ew, 0 );
//     return QRect(r.x()+1, r.y()+1, r.width()-2-ew, r.height()-2);
}

/*! \reimp
 */
QRect QMotifStyle::comboButtonFocusRect( int x, int y, int w, int h ) const
{
//     int awh, ax, ay, sh, sy, dh, ew;
//     get_combo_parameters( buttonRect( x, y, w, h ),
//                           ew, awh, ax, ay, sh, dh, sy );
//     return QRect(ax-2, ay-2, awh+4, awh+sh+dh+4);
}

/*!\reimp
 */
void QMotifStyle::drawToolBarHandle( QPainter *p, const QRect &r, Qt::Orientation orientation,
                                bool highlight, const QColorGroup &cg,
                                bool drawBorder )
{
//     p->save();
//     p->translate( r.x(), r.y() );

//     QColor dark( cg.dark() );
//     QColor light( cg.light() );
//     unsigned int i;
//     if ( orientation == Qt::Vertical ) {
//         int w = r.width();
//         if ( w > 6 ) {
//             if ( highlight )
//                 p->fillRect( 1, 1, w - 2, 9, cg.highlight() );
//             QPointArray a( 2 * ((w-6)/3) );

//             int x = 3 + (w%3)/2;
//             p->setPen( dark );
//             p->drawLine( 1, 8, w-2, 8 );
//             for( i=0; 2*i < a.size(); i ++ ) {
//                 a.setPoint( 2*i, x+1+3*i, 6 );
//                 a.setPoint( 2*i+1, x+2+3*i, 3 );
//             }
//             p->drawPoints( a );
//             p->setPen( light );
//             p->drawLine( 1, 9, w-2, 9 );
//             for( i=0; 2*i < a.size(); i++ ) {
//                 a.setPoint( 2*i, x+3*i, 5 );
//                 a.setPoint( 2*i+1, x+1+3*i, 2 );
//             }
//             p->drawPoints( a );
//             if ( drawBorder ) {
//                 p->setPen( QPen( Qt::darkGray ) );
//                 p->drawLine( r.width() - 1, 0,
//                              r.width() - 1, toolBarHandleExtent() );
//             }
//         }
//     } else {
//         int h = r.height();
//         if ( h > 6 ) {
//             if ( highlight )
//                 p->fillRect( 1, 1, 8, h - 2, cg.highlight() );
//             QPointArray a( 2 * ((h-6)/3) );
//             int y = 3 + (h%3)/2;
//             p->setPen( dark );
//             p->drawLine( 8, 1, 8, h-2 );
//             for( i=0; 2*i < a.size(); i ++ ) {
//                 a.setPoint( 2*i, 5, y+1+3*i );
//                 a.setPoint( 2*i+1, 2, y+2+3*i );
//             }
//             p->drawPoints( a );
//             p->setPen( light );
//             p->drawLine( 9, 1, 9, h-2 );
//             for( i=0; 2*i < a.size(); i++ ) {
//                 a.setPoint( 2*i, 4, y+3*i );
//                 a.setPoint( 2*i+1, 1, y+1+3*i );
//             }
//             p->drawPoints( a );
//             if ( drawBorder ) {
//                 p->setPen( QPen( Qt::darkGray ) );
//                 p->drawLine( 0, r.height() - 1,
//                              toolBarHandleExtent(), r.height() - 1 );
//             }
//         }
//     }
//     p->restore();
}

/*! \reimp */
// void QMotifStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
//                                    QMenuItem* mi, QColorGroup& g, bool active,
//                                    bool down, bool hasFocus )
// {
//     QRect r( x, y, w, h);

//     if ( active ) // active item
//         qDrawShadePanel( p, r, g, FALSE, motifItemFrame,
//                          &g.brush( QColorGroup::Button ));
//     else // other item
//         p->fillRect(r, g.brush( QColorGroup::Button ));

//     QCommonStyle::drawMenuBarItem( p, r.left(), r.top(), r.width(), r.height(),
//                                    mi, g, active, down, hasFocus );
// }

/*!
 \reimp
 */
int QMotifStyle::spinBoxFrameWidth() const
{
    return 0;
}

/*!
 \reimp
 */
int QMotifStyle::progressChunkWidth() const
{
    return 1;
}

/*!
  \reimp
*/
void QMotifStyle::drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    qDrawShadePanel( p, x, y, w, h, g, TRUE, 1 );
    p->fillRect( x + 2, y + 2, w - 4, h - 4, g.base() );
}

/*!
 \reimp
 */
void QMotifStyle::drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    p->fillRect( x, y, w, h, g.highlight() );
}

/*!
 \reimp
 */
void
QMotifStyle::drawListViewItemBranch( QPainter *p, int y, int w, int h, const QColorGroup & cg, QListViewItem *item )
{
    QListViewItem *child = item->firstChild();
    int linetop = 0, linebot = 0;

    // each branch needs at most two lines, ie. four end points
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
	    QPointArray a;
	    if ( child->isOpen() )
		a.setPoints( 3, bx-2, linebot-2,
			     bx, linebot+2,
			     bx+2, linebot-2 ); //RightArrow
	    else
		a.setPoints( 3, bx-2, linebot-2,
			     bx+2, linebot,
			     bx-2, linebot+2 ); //DownArrow
	    p->setBrush( cg.text() );
	    p->drawPolygon( a );
	    p->setBrush( NoBrush );
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

    int line; // index into dotlines
    p->setPen( cg.text() );
    for( line = 0; line < c; line += 2 ) {
	p->drawLine( dotlines[line].x(), dotlines[line].y(),
		     dotlines[line+1].x(), dotlines[line+1].y() );
    }
}

static const char * const qt_close_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"   .    .   ",
"  ...  ...  ",
"   ......   ",
"    ....    ",
"    ....    ",
"   ......   ",
"  ...  ...  ",
"   .    .   ",
"            ",
"            "};

static const char * const qt_maximize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"     .      ",
"    ...     ",
"   .....    ",
"  .......   ",
" .........  ",
"            ",
"            ",
"            ",
"            "};

static const char * const qt_minimize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"            ",
" .........  ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            ",
"            ",
"            "};

#if 0 // ### not used???
static const char * const qt_normalize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"  .         ",
"  ..        ",
"  ...       ",
"  ....      ",
"  .....     ",
"  ......    ",
"  .......   ",
"            ",
"            ",
"            "};
#endif

static const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"  .......   ",
"   ......   ",
"    .....   ",
"     ....   ",
"      ...   ",
"       ..   ",
"        .   ",
"            ",
"            "};

static const char * const qt_shade_xpm[] = {
"12 12 2 1", "# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"............"};


static const char * const qt_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#########..",
"............",
"............"};


/*!
 \reimp
 */
QPixmap QMotifStyle::titleBarPixmap( const QTitleBar *, SubControl ctrl) const
{
    switch(ctrl) {
    case SC_TitleBarShadeButton:
	return QPixmap((const char **)qt_shade_xpm);
    case SC_TitleBarUnshadeButton:
	return QPixmap((const char **)qt_unshade_xpm);
    case SC_TitleBarNormalButton:
	return QPixmap((const char **)qt_normalizeup_xpm);
    case SC_TitleBarMinButton:
	return QPixmap((const char **)qt_minimize_xpm);
    case SC_TitleBarMaxButton:
	return QPixmap((const char **)qt_maximize_xpm);
    case SC_TitleBarCloseButton:
	return QPixmap((const char **)qt_close_xpm);
    }
    return QPixmap();
}



#endif

