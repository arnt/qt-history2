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
#include "qtabwidget.h"
#include "qlistview.h"
#include "qsplitter.h"
#include "qslider.h"
#include "qcombobox.h"
#include "qdockwindow.h"
#include "qdockarea.h"
#include "qprogressbar.h"
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
				 void **data ) const
{
    switch( op ) {
    case PO_ButtonCommand:
    case PO_ButtonBevel:
    case PO_ButtonTool:
    case PO_HeaderSection:
	qDrawShadePanel( p, r, cg, bool(flags & (PStyle_Down | PStyle_On)),
			 pixelMetric(PM_DefaultFrameWidth),
			 &cg.brush(QColorGroup::Button) );
	break;

    case PO_Indicator: {
#ifndef QT_NO_BUTTON
	bool on = flags & PStyle_On;
	bool down = flags & PStyle_Down;
	bool showUp = !( down ^ on );
	QBrush fill = showUp || flags & PStyle_NoChange ? cg.brush( QColorGroup::Button ) : cg.brush(QColorGroup::Mid );
	if ( flags & PStyle_NoChange ) {
	    qDrawPlainRect( p, r, cg.text(),
			    1, &fill );
	    p->drawLine( r.x() + r.width() - 1, r.y(),
			 r.x(), r.y() + r.height() - 1);
	} else
	    qDrawShadePanel( p, r, cg, !showUp,
			     pixelMetric(PM_DefaultFrameWidth), &fill );
#endif
	break;
    }

    case PO_ExclusiveIndicator:
	{
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
	    static QCOORD inner_pts[] =         // used for filling diamond
		{ 2,6, 6,2, 10,6, 6,10 };
	    static QCOORD top_pts[] =           // top (^) of diamond
		{ 0,6, 6,0 , 11,5, 10,5, 6,1, 1,6, 2,6, 6,2, 9,5 };
	    static QCOORD bottom_pts[] =                // bottom (V) of diamond
		{ 1,7, 6,12, 12,6, 11,6, 6,11, 2,7, 3,7, 6,10, 10,6 };

	    bool on = flags & PStyle_On;
	    bool down = flags & PStyle_Down;
	    bool showUp = !(down ^ on );
	    QPointArray a( QCOORDARRLEN(inner_pts), inner_pts );
	    p->eraseRect( r );
	    p->setPen( NoPen );
	    p->setBrush( showUp ? cg.brush( QColorGroup::Button ) :
			 cg.brush( QColorGroup::Mid ) );
	    a.translate( r.x(), r.y() );
	    p->drawPolygon( a );
	    p->setPen( showUp ? cg.light() : cg.dark() );
	    p->setBrush( NoBrush );
	    a.setPoints( QCOORDARRLEN(top_pts), top_pts );
	    a.translate( r.x(), r.y() );
	    p->drawPolyline( a );
	    p->setPen( showUp ? cg.dark() : cg.light() );
	    a.setPoints( QCOORDARRLEN(bottom_pts), bottom_pts );
	    a.translate( r.x(), r.y() );
	    p->drawPolyline( a );

	    break;
	}

    case PO_ArrowUp:
    case PO_ArrowDown:
    case PO_ArrowRight:
    case PO_ArrowLeft:
	{
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

	    // adjust size and center (to fix rotation below)
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

	    if ( op == PO_ArrowUp || op == PO_ArrowLeft ) {
		matrix.translate( rect.x(), rect.y() );
		if ( vertical ) {
		    matrix.translate( 0, rect.height() - 1 );
		    matrix.rotate( -90 );
		} else {
		    matrix.translate( rect.width() - 1, rect.height() - 1 );
		    matrix.rotate( 180 );
		}
		if ( flags & PStyle_Down )
		    colspec = horizontal ? 0x2334 : 0x2343;
		else
		    colspec = horizontal ? 0x1443 : 0x1434;
	    } else if ( op == PO_ArrowDown || op == PO_ArrowRight ) {
		matrix.translate( rect.x(), rect.y() );
		if ( vertical ) {
		    matrix.translate( rect.width() - 1, 0 );
		    matrix.rotate( 90 );
		}
		if ( flags & PStyle_Down )
		    colspec = horizontal ? 0x2443 : 0x2434;
		else
		    colspec = horizontal ? 0x1334 : 0x1343;
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
	    p->drawPolygon( bFill );
	    p->setBrush( NoBrush );

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
	    break;
	}

    case PO_SpinWidgetPlus:
    case PO_SpinWidgetMinus:
	{
	    p->save();
	    int fw = pixelMetric( PM_DefaultFrameWidth );
	    QRect br;
	    br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
			r.height() - fw*2 );

	    if ( flags & PStyle_Sunken )
		p->fillRect( r, cg.brush( QColorGroup::Dark ) );
	    else
		p->fillRect( r, cg.brush( QColorGroup::Button ) );

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
	    break;
	}

    case PO_SpinWidgetUp:
    case PO_SpinWidgetDown:
	{
	    p->save();
	    int fw = pixelMetric( PM_DefaultFrameWidth );;
	    QRect br;
	    br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
			r.height() - fw*2 );
	    if ( flags & PStyle_Sunken )
		p->fillRect( br, cg.brush( QColorGroup::Mid ) );
	    else
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
	    break;
	}

    case PO_DockWindowHandle:
	{
	    p->save();
	    p->translate( r.x(), r.y() );

	    QColor dark( cg.dark() );
	    QColor light( cg.light() );
	    unsigned int i;
	    if ( flags & PStyle_Vertical ) {
		int w = r.width();
		if ( w > 6 ) {
		    if ( flags & PStyle_On )
			p->fillRect( 1, 1, w - 2, 9, cg.highlight() );
		    QPointArray a( 2 * ((w-6)/3) );

		    int x = 3 + (w%3)/2;
		    p->setPen( dark );
		    p->drawLine( 1, 8, w-2, 8 );
		    for( i=0; 2*i < a.size(); i ++ ) {
			a.setPoint( 2*i, x+1+3*i, 6 );
			a.setPoint( 2*i+1, x+2+3*i, 3 );
		    }
		    p->drawPoints( a );
		    p->setPen( light );
		    p->drawLine( 1, 9, w-2, 9 );
		    for( i=0; 2*i < a.size(); i++ ) {
			a.setPoint( 2*i, x+3*i, 5 );
			a.setPoint( 2*i+1, x+1+3*i, 2 );
		    }
		    p->drawPoints( a );
		    // if ( drawBorder ) {
		    // p->setPen( QPen( Qt::darkGray ) );
		    // p->drawLine( r.width() - 1, 0,
		    // r.width() - 1, tbExtent );
		    // }
		}
	    } else {
		int h = r.height();
		if ( h > 6 ) {
		    if ( flags & PStyle_On )
			p->fillRect( 1, 1, 8, h - 2, cg.highlight() );
		    QPointArray a( 2 * ((h-6)/3) );
		    int y = 3 + (h%3)/2;
		    p->setPen( dark );
		    p->drawLine( 8, 1, 8, h-2 );
		    for( i=0; 2*i < a.size(); i ++ ) {
			a.setPoint( 2*i, 5, y+1+3*i );
			a.setPoint( 2*i+1, 2, y+2+3*i );
		    }
		    p->drawPoints( a );
		    p->setPen( light );
		    p->drawLine( 9, 1, 9, h-2 );
		    for( i=0; 2*i < a.size(); i++ ) {
			a.setPoint( 2*i, 4, y+3*i );
			a.setPoint( 2*i+1, 1, y+1+3*i );
		    }
		    p->drawPoints( a );
		    // if ( drawBorder ) {
		    // p->setPen( QPen( Qt::darkGray ) );
		    // p->drawLine( 0, r.height() - 1,
		    // tbExtent, r.height() - 1 );
		    // }
		}
	    }
	    p->restore();
	    break;
	}

    case PO_Splitter:
    case PO_DockWindowResizeHandle:
	{
	    const int motifOffset = 10;
 	    int sw = pixelMetric( PM_SplitterWidth );
	    if ( flags & PStyle_Horizontal ) {
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
 	    break;
	}

    case PO_CheckMark:
	{
	    const int markW = 6;
	    const int markH = 6;
	    int posX = r.x() + ( r.width()  - markW ) / 2 - 1;
	    int posY = r.y() + ( r.height() - markH ) / 2;
	    int dfw = pixelMetric(PM_DefaultFrameWidth);

	    if (dfw < 2) {
		// Could do with some optimizing/caching...
		QPointArray a( 7*2 );
		int i, xx, yy;
		xx = posX;
		yy = 3 + posY;
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
		if ( ! (flags & PStyle_Enabled) && ! (flags & PStyle_On) ) {
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

		qDrawShadePanel( p, posX-2, posY-2, markW+4, markH+6, cg, TRUE, dfw);
	    } else
		qDrawShadePanel( p, posX, posY, markW, markH, cg, TRUE, dfw,
				 &cg.brush( QColorGroup::Mid ) );

	    break;
	}

    case PO_ScrollBarSubLine:
	drawPrimitive(((flags & PStyle_Horizontal) ? PO_ArrowLeft : PO_ArrowUp),
		      p, r, cg, PStyle_Enabled | flags);
	break;

    case PO_ScrollBarAddLine:
	drawPrimitive(((flags & PStyle_Horizontal) ? PO_ArrowRight : PO_ArrowDown),
		      p, r, cg, PStyle_Enabled | flags);
	break;

    case PO_ScrollBarSubPage:
    case PO_ScrollBarAddPage:
	p->fillRect(r, cg.brush(QColorGroup::Mid));
	break;

    case PO_ScrollBarSlider:
	drawPrimitive(PO_ButtonBevel, p, r, cg, PStyle_Enabled | PStyle_Raised);
	break;

    case PO_ProgressBarChunk:
	p->fillRect( r.x(), r.y() + 2, r.width() - 2,
	    r.height() - 4, cg.brush(QColorGroup::Highlight));
	break;

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
			       CFlags how,
			       void **data ) const
{
    switch( element ) {
    case CE_PushButton:
	{
 	    int diw, x1, y1, x2, y2;
 	    const QPushButton *btn;
	    QColorGroup newCg = cg;
 	    btn = ( const QPushButton * )widget;
 	    p->setPen( cg.foreground() );
 	    p->setBrush( QBrush( cg.button(), NoBrush ) );
 	    diw = pixelMetric( PM_ButtonDefaultIndicator );
 	    r.coords( &x1, &y1, &x2, &y2 );
 	    if ( btn->isDefault() || btn->autoDefault() ) {
 		x1 += diw;
 		y1 += diw;
 		x2 -= diw;
 		y2 -= diw;
 	    }
 	    QBrush fill;
 	    if ( btn->isDown() )
 		fill = newCg.brush( QColorGroup::Mid );
 	    else if ( btn->isOn() )
 		fill = QBrush( newCg.mid(), Dense4Pattern );
 	    else
 		fill = newCg.brush( QColorGroup::Button );

	    newCg.setBrush( QColorGroup::Button, fill );
 	    if ( btn->isDefault() ) {
 		if ( diw == 0 ) {
		    QPointArray a;
		    a.setPoints( 9,
				 x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
				 x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1 );
 		    p->setPen( newCg.shadow() );
 		    p->drawPolygon( a );
 		    x1 += 2;
 		    y1 += 2;
 		    x2 -= 2;
 		    y2 -= 2;
 		} else {
 		    qDrawShadePanel( p, r, newCg, TRUE );
 		}
 	    }
 	    if ( !btn->isFlat() || btn->isOn() || btn->isDown() ) {
		QRect tmp( x1, y1, x2 - x1 + 1, y2 - y1 + 1 );
		PFlags flags = PStyle_Default;
		if ( btn->isOn())
		    flags |= PStyle_On;
		if (btn->isDown())
		    flags |= PStyle_Down;
		drawPrimitive( PO_ButtonCommand, p,
			       tmp, newCg,
 			       flags );
	    }
 	    if ( p->brush().style() != NoBrush )
 		p->setBrush( NoBrush );
	    break;
	}

    case CE_TabBarTab:
	{
	    if ( !widget || !widget->parentWidget() )
		break;

	    QTabBar * tb = (QTabBar *) widget;
	    int dfw = pixelMetric( PM_DefaultFrameWidth, tb );
	    bool selected = how & CStyle_Selected;
	    int o =  dfw > 1 ? 1 : 0;
	    bool lastIsCurrent = FALSE;

	    if ( styleHint( SH_TabBar_Alignment, tb ) == AlignRight &&
		 tb->currentTab() == tb->indexOf(tb->count()-1) )
		lastIsCurrent = TRUE;

	    QRect r( r );
	    if ( tb->shape()  == QTabBar::RoundedAbove ) {
		if ( o ) {
		    p->setPen( tb->colorGroup().light() );
		    p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
		    p->setPen( tb->colorGroup().light() );
		    p->drawLine( r.left(), r.bottom()-1, r.right(), r.bottom()-1 );
		    if ( r.left() == 0 )
			p->drawPoint( tb->rect().bottomLeft() );
		}
		else {
		    p->setPen( tb->colorGroup().light() );
		    p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
		}

		if ( selected ) {
		    p->fillRect( QRect( r.left()+1, r.bottom()-o, r.width()-3, 2),
				 tb->palette().active().brush( QColorGroup::Background ));
		    p->setPen( tb->colorGroup().background() );
		    // p->drawLine( r.left()+1, r.bottom(), r.right()-2, r.bottom() );
		    // if (o)
		    // p->drawLine( r.left()+1, r.bottom()-1, r.right()-2, r.bottom()-1 );
		    p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top()+2 );
		    p->setPen( tb->colorGroup().light() );
		} else {
		    p->setPen( tb->colorGroup().light() );
		    r.setRect( r.left() + 2, r.top() + 2,
			       r.width() - 4, r.height() - 2 );
		}

		p->drawLine( r.left(), r.bottom()-1, r.left(), r.top() + 2 );
		p->drawPoint( r.left()+1, r.top() + 1 );
		p->drawLine( r.left()+2, r.top(),
			     r.right() - 2, r.top() );
		p->drawPoint( r.left(), r.bottom());

		if ( o ) {
		    p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top() + 2 );
		    p->drawLine( r.left()+2, r.top()+1,
				 r.right() - 2, r.top()+1 );
		}

		p->setPen( tb->colorGroup().dark() );
		p->drawLine( r.right() - 1, r.top() + 2,
			     r.right() - 1, r.bottom() - 1 + (selected ? o : -o));
		if ( o ) {
		    p->drawPoint( r.right() - 1, r.top() + 1 );
		    p->drawLine( r.right(), r.top() + 2, r.right(),
				 r.bottom() -
				 (selected ? (lastIsCurrent ? 0:1):1+o));
		    p->drawPoint( r.right() - 1, r.top() + 1 );
		}
	    } else if ( tb->shape()  == QTabBar::RoundedBelow ) {
		if ( selected ) {
		    p->fillRect( QRect( r.left()+1, r.top(), r.width()-3, 1),
				 tb->palette().active().brush( QColorGroup::Background ));
		    p->setPen( tb->colorGroup().background() );
		    // p->drawLine( r.left()+1, r.top(), r.right()-2, r.top() );
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

		if (dfw > 1) {
		    p->drawLine( r.right(), r.top(),
				 r.right(), r.bottom() - 1 );
		    p->drawPoint( r.right() - 1, r.bottom() - 1 );
		    p->drawLine( r.right() - 1, r.bottom(),
				 r.left() + 2, r.bottom() );
		}

		p->setPen( tb->colorGroup().light() );
		p->drawLine( r.left(), r.top(),
			     r.left(), r.bottom() - 2 );

	    } else {
		QCommonStyle::drawControl( element, p, widget, r, cg, how, data );
	    }
	    break;
	}

    case CE_ProgressBarGroove:
	qDrawShadePanel(p, r, cg, TRUE, 2);
	break;

    case CE_ProgressBarLabel:
	{
	    QProgressBar * pb = (QProgressBar *) widget;
	    const int unit_width = pixelMetric( PM_ProgressBarChunkWidth, pb );
	    int u = r.width() / unit_width;
	    int p_v = pb->progress();
	    int t_s = pb->totalSteps();
	    if ( u > 0 && pb->progress() >= INT_MAX / u && t_s >= u ) {
		// scale down to something usable.
		p_v /= u;
		t_s /= u;
	    }
	    int nu = ( u * p_v + t_s/2 ) / t_s;
	    int x = unit_width * nu;
	    if ( pb->percentageVisible() && pb->totalSteps() ) {
		p->setPen( cg.highlightedText() );
		p->setClipRect( r.x(), r.y(), x, r.height() );
		p->drawText( r, AlignCenter | SingleLine, pb->progressString() );
		if ( pb->progress() != pb->totalSteps() ) {
		    p->setClipRect( r.x() + x, r.y(), r.width() - x, r.height() );
		    p->setPen( cg.highlight() );
		    p->drawText( r, AlignCenter | SingleLine, pb->progressString() );
		}
	    }
	    break;
	}

#ifndef QT_NO_POPUPMENU
    case CE_PopupMenuItem:
	{
	    if (! widget || ! data)
		break;

	    QPopupMenu *popupmenu = (QPopupMenu *) widget;
	    QMenuItem *mi = (QMenuItem *) data[0];
	    if ( !mi )
		break;

	    int tab = *((int *) data[1]);
	    int maxpmw = *((int *) data[2]);
	    bool dis = ! mi->isEnabled();
	    bool checkable = popupmenu->isCheckable();
	    bool act = how & CStyle_Selected;
	    int x, y, w, h;

	    r.rect(&x, &y, &w, &h);

	    if ( checkable )
		maxpmw = QMAX( maxpmw, motifCheckMarkSpace );

	    int checkcol = maxpmw;

	    if ( mi && mi->isSeparator() ) {                    // draw separator
		p->setPen( cg.dark() );
		p->drawLine( x, y, x+w, y );
		p->setPen( cg.light() );
		p->drawLine( x, y+1, x+w, y+1 );
		return;
	    }

	    int pw = motifItemFrame;

	    if ( act && !dis ) {                        // active item frame
		if (pixelMetric( PM_DefaultFrameWidth ) > 1)
		    qDrawShadePanel( p, x, y, w, h, cg, FALSE, pw,
				     &cg.brush( QColorGroup::Button ) );
		else
		    qDrawShadePanel( p, x+1, y+1, w-2, h-2, cg, TRUE, 1,
				     &cg.brush( QColorGroup::Button ) );
	    }
	    else                                // incognito frame
		p->fillRect(x, y, w, h, cg.brush( QColorGroup::Button ));

	    if ( !mi )
		return;

	    if ( mi->isChecked() ) {
		if ( mi->iconSet() ) {
		    qDrawShadePanel( p, x+motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame,
				     cg, TRUE, 1, &cg.brush( QColorGroup::Midlight ) );
		}
	    } else if ( !act ) {
		p->fillRect(x+motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame,
			    cg.brush( QColorGroup::Button ));
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
		p->setPen( cg.text() );
		p->drawPixmap( pmr.topLeft(), pixmap );

	    } else  if ( checkable ) {  // just "checking"...
		int mw = checkcol;
		int mh = h - 2*motifItemFrame;
		if ( mi->isChecked() ) {
		    PFlags cflags = PStyle_Default;
		    if (! dis)
			cflags |= PStyle_Enabled;
		    if (act)
			cflags |= PStyle_On;

		    drawPrimitive(PO_CheckMark, p,
				  QRect(x+motifItemFrame, y+motifItemFrame, mw, mh),
				  cg, cflags);
		}
	    }


	    p->setPen( cg.buttonText() );

	    QColor discol;
	    if ( dis ) {
		discol = cg.text();
		p->setPen( discol );
	    }

	    int xm = motifItemFrame + checkcol + motifItemHMargin;

	    if ( mi->custom() ) {
		int m = motifItemVMargin;
		p->save();
		mi->custom()->paint( p, cg, act, !dis,
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
		if ( act )
		    drawPrimitive(PO_ArrowRight, p,
				  QRect(x+w - motifArrowHMargin - motifItemFrame - dim,
					y+h/2-dim/2, dim, dim), cg,
				  (PStyle_Down |
				   (mi->isEnabled() ? PStyle_Enabled : PStyle_Default)));
		else
		    drawPrimitive(PO_ArrowRight, p,
				  QRect(x+w - motifArrowHMargin - motifItemFrame - dim,
					y+h/2-dim/2, dim, dim), cg,
				  (mi->isEnabled() ? PStyle_Enabled : PStyle_Default));
	    }

	    break;
	}
#endif // QT_NO_POPUPMENU

    case CE_MenuBarItem:
 	{
 	    if ( how & CStyle_Active )  // active item
 		qDrawShadePanel( p, r, cg, FALSE, motifItemFrame,
 				 &cg.brush(QColorGroup::Button) );
 	    else  // other item
 		p->fillRect( r, cg.brush(QColorGroup::Button) );
	    QCommonStyle::drawControl( element, p, widget, r, cg, how, data );
 	    break;
 	}

    default:
	QCommonStyle::drawControl( element, p, widget, r, cg, how, data );
	break;
    }
}

static int get_combo_extra_width( int h, int *return_awh=0 )
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
//     if( QApplication::reverseLayout() )
//         ax = r.x();
//     else
    ax = r.x() + r.width() - ew;
    ax  += (ew-awh)/2;
}

void QMotifStyle::drawComplexControl( ComplexControl control,
				     QPainter *p,
				     const QWidget *widget,
				     const QRect &r,
				     const QColorGroup &cg,
				     CFlags flags,
				     SCFlags sub,
				     SCFlags subActive,
				     void **data ) const
{
    switch ( control ) {
    case CC_SpinWidget:
	if ( sub & SC_SpinWidgetUp || sub & SC_SpinWidgetDown )
	    QCommonStyle::drawComplexControl( control, p, widget, r, cg, flags,
					      sub, subActive, data );
	if ( sub & SC_SpinWidgetFrame )
	    qDrawShadePanel( p, r, cg, TRUE,
			     pixelMetric( PM_DefaultFrameWidth) );
	break;

    case CC_Slider:	
	if ( sub == SC_All || sub & SC_SliderGroove ) {
	    QSlider * sl = (QSlider *) widget;

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
		qDrawShadePanel( p, x, y, wi, he, cg, TRUE, 2,
				 &cg.brush( QColorGroup::Mid ) );
		sl->erase( 0, 0, sl->width(), tickOffset );
		sl->erase( 0, tickOffset + thickness, sl->width(), sl->height() );
	    } else {
		qDrawShadePanel( p, x, y, wi, he, cg, TRUE, 2,
				 &cg.brush( QColorGroup::Mid ) );
		sl->erase( 0, 0,  tickOffset, sl->height() );
		sl->erase( tickOffset + thickness, 0, sl->width(), sl->height() );
	    }
	}

	if ( sub == SC_All || sub & SC_SliderTickmarks )
	    QCommonStyle::drawComplexControl( control, p, widget, r, cg, flags,
					      SC_SliderTickmarks, subActive,
					      data );

	if ( sub == SC_All || sub & SC_SliderHandle ) {
	    QSlider * sl = (QSlider *) widget;

	    if ( sl->hasFocus() ) {
		QRect re = subRect( SR_SliderFocusRect, sl );
		drawPrimitive( PO_FocusRect, p, re, cg );
	    }

	    QRect re = querySubControlMetrics( CC_Slider, widget, SC_SliderHandle,
					       data );
	    drawPrimitive( PO_ButtonBevel, p, re, cg );
	    if ( sl->orientation() == Horizontal ) {
		QCOORD mid = re.x() + re.width() / 2;
		qDrawShadeLine( p, mid,  re.y(), mid,  re.y() + re.height() - 2,
				cg, TRUE, 1);
	    } else {
		QCOORD mid = re.y() + re.height() / 2;
		qDrawShadeLine( p, re.x(), mid,  re.x() + re.width() - 2, mid,
				cg, TRUE, 1);
	    }
	}

	break;

    case CC_ComboBox:
	if ( sub & SC_ComboBoxArrow ) {
	    QComboBox * cb = (QComboBox *) widget;
	    int awh, ax, ay, sh, sy, dh, ew;
	    int fw = pixelMetric( PM_DefaultFrameWidth, cb);

	    drawPrimitive( PO_ButtonCommand, p, r, cg, flags );
	    QRect ar = QStyle::visualRect( querySubControlMetrics( CC_ComboBox, cb, SC_ComboBoxArrow,
								   data ), cb );
	    drawPrimitive( PO_ArrowDown, p, ar, cg, flags | PStyle_Enabled );

	    QRect tr = r;
	    tr.addCoords( fw, fw, -fw, -fw );
	    get_combo_parameters( tr, ew, awh, ax, ay, sh, dh, sy );

	    // draws the shaded line under the arrow
	    p->setPen( cg.light() );
	    p->drawLine( ar.x(), sy, ar.x()+awh-1, sy );
	    p->drawLine( ar.x(), sy, ar.x(), sy+sh-1 );
	    p->setPen( cg.dark() );
	    p->drawLine( ar.x()+1, sy+sh-1, ar.x()+awh-1, sy+sh-1 );
	    p->drawLine( ar.x()+awh-1, sy+1, ar.x()+awh-1, sy+sh-1 );

	    if ( cb->hasFocus() ) {
		QRect re = QStyle::visualRect( subRect( SR_ComboBoxFocusRect, cb ), cb );
		drawPrimitive( PO_FocusRect, p, re, cg );
	    }
	}

	if ( sub & SC_ComboBoxEditField ) {
	    QComboBox * cb = (QComboBox *) widget;
	    if ( cb->editable() ) {
		QRect er = QStyle::visualRect( querySubControlMetrics( CC_ComboBox, cb,
								       SC_ComboBoxEditField ), cb );
		er.addCoords( -1, -1, 1, 1);
		qDrawShadePanel( p, er, cg, TRUE, 1,
				 &cg.brush( QColorGroup::Button ));
	    }
	}
	break;

    case CC_ScrollBar:
	{
	    if (sub == (SC_ScrollBarAddLine | SC_ScrollBarSubLine | SC_ScrollBarAddPage |
			SC_ScrollBarSubPage | SC_ScrollBarFirst | SC_ScrollBarLast |
			SC_ScrollBarSlider))
		qDrawShadePanel(p, widget->rect(), cg, TRUE,
				pixelMetric(PM_DefaultFrameWidth, widget),
				&cg.brush(QColorGroup::Mid));
	    QCommonStyle::drawComplexControl(control, p, widget, r, cg, flags, sub,
					     subActive, data);
	    break;
	}

#ifndef QT_NO_LISTVIEW
    case CC_ListView:
	{
	    if (! data)
		break;

	    QListViewItem *item = (QListViewItem *) data[0];
	    QListViewItem *child = item->firstChild();
	    int linetop = 0, linebot = 0, y = r.y();

	    // each branch needs at most two lines, ie. four end points
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

	    int line; // index into dotlines
	    p->setPen( cg.text() );
	    for( line = 0; line < c; line += 2 ) {
		p->drawLine( dotlines[line].x(), dotlines[line].y(),
			     dotlines[line+1].x(), dotlines[line+1].y() );
	    }

	    break;
	}
#endif // QT_NO_LISTVIEW

    default:
	QCommonStyle::drawComplexControl( control, p, widget, r, cg, flags,
					  sub, subActive, data );
    }
}


int QMotifStyle::pixelMetric( PixelMetric metric, const QWidget *widget ) const
{
     int ret;

    switch( metric ) {
    case PM_ButtonDefaultIndicator:
	ret = 3;
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_SplitterWidth:
	ret = QMAX( 10, QApplication::globalStrut().width() );
	break;

    case PM_SliderLength:
	ret = 30;
	break;

    case PM_SliderThickness:
	ret = 24;
	break;

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

	    space -= thick;
	    //### the two sides may be unequal in size
	    if ( space > 0 )
		thick += (space * 2) / (n + 2);
	    ret = thick;
	    break;
	}

    case PM_SliderSpaceAvailable:
	{
	    QSlider * sl = (QSlider *) widget;
	    if ( sl->orientation() == Horizontal )
		ret = sl->width() - pixelMetric( PM_SliderLength, sl ) - 6;
	    else
		ret = sl->height() - pixelMetric( PM_SliderLength, sl ) - 6;
	    break;
	}

    case PM_DockWindowHandleExtent:
	ret = 9;
	break;

    case PM_ProgressBarChunkWidth:
	ret = 1;
	break;

    default:
	ret =  QCommonStyle::pixelMetric( metric, widget );
	break;
    }
    return ret;
}


QRect QMotifStyle::querySubControlMetrics( ComplexControl control,
					   const QWidget *widget,
					   SubControl sc,
					   void **data ) const
{
    QRect rect;

    switch ( control ) {
    case CC_SpinWidget:
	{
	    if ( !widget )
		break;
	    int fw = pixelMetric( PM_SpinBoxFrameWidth, 0 );
	    QSize bs;
	    bs.setHeight( widget->height()/2 );
	    if ( bs.height() < 8 )
		bs.setHeight( 8 );
	    bs.setWidth( bs.height() * 8 / 5 ); // 1.6 -approximate golden mean
	    bs = bs.expandedTo( QApplication::globalStrut() );
	    int y = 0;
	    int x, lx, rx;
	    x = widget->width() - y - bs.width();
	    lx = fw;
	    rx = x - fw * 2;

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
		rect.setRect( widget->x(), widget->y(),
			      widget->width() - bs.width(), widget->height() );
	    default:
		break;
	    }
	    break;
	}

    case CC_Slider:
	{
	    switch ( sc ) {
	    case SC_SliderHandle: {
		QSlider * sl = (QSlider *) widget;
		int sliderPos = 0;
		int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
		int thickness  = pixelMetric( PM_SliderControlThickness, sl );
		int len   = pixelMetric( PM_SliderLength, sl );
		int motifBorder = 3;

		if ( data )
		    sliderPos = *((int *) data[0]);

		if ( sl->orientation() == Horizontal )
		    rect.setRect( sliderPos + motifBorder,
				  tickOffset + motifBorder, len,
				  thickness - 2*motifBorder );
		else
		    rect.setRect( tickOffset + motifBorder,
				  sliderPos + motifBorder,
				  thickness - 2*motifBorder, len );
		break; }

	    default:
		break;
	    }

	    break;
	}

    case CC_ScrollBar:
	{
	    if (! widget)
		break;

	    QScrollBar *scrollbar = (QScrollBar *) widget;
	    int sliderstart = 0;
	    int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
	    int fw = pixelMetric(PM_DefaultFrameWidth, widget);
	    int buttonw = sbextent - (fw * 2);
	    int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
			  scrollbar->width() : scrollbar->height()) -
			 (buttonw * 2) - (fw * 2);
	    int sliderlen;

	    if (data)
		sliderstart = *((int *) data[0]);
	    else
		sliderstart = sbextent;

	    // calculate slider length
	    if (scrollbar->maxValue() != scrollbar->minValue()) {
		uint range = scrollbar->maxValue() - scrollbar->minValue();
		sliderlen = (scrollbar->pageStep() * maxlen) /
			    (range + scrollbar->pageStep());

		if ( sliderlen < 9 || range > INT_MAX/2 )
		    sliderlen = 9;
		if ( sliderlen > maxlen )
		    sliderlen = maxlen;
	    } else
		sliderlen = maxlen;

	    switch (sc) {
	    case SC_ScrollBarSubLine:
		// top/left button
		rect.setRect(fw, fw, buttonw, buttonw);
		break;

	    case SC_ScrollBarAddLine:
		// bottom/right button
		if (scrollbar->orientation() == Qt::Horizontal)
		    rect.setRect(scrollbar->width() - sbextent + fw, fw,
				 buttonw, buttonw);
		else
		    rect.setRect(fw, scrollbar->height() - sbextent + fw,
				 buttonw, buttonw);
		break;

	    case SC_ScrollBarSubPage:
		if (scrollbar->orientation() == Qt::Horizontal)
		    rect.setRect(buttonw + fw, fw, sliderstart - buttonw - fw, buttonw);
		else
		    rect.setRect(fw, buttonw + fw, buttonw, sliderstart - buttonw - fw);
		break;

 	    case SC_ScrollBarAddPage:
 		if (scrollbar->orientation() == Qt::Horizontal)
 		    rect.setRect(sliderstart + sliderlen, fw,
				 maxlen - sliderstart - sliderlen + buttonw + fw,
				 buttonw);
 		else
 		    rect.setRect(fw, sliderstart + sliderlen, buttonw,
				 maxlen - sliderstart - sliderlen + buttonw + fw);
 		break;

	    case SC_ScrollBarGroove:
		if (scrollbar->orientation() == Qt::Horizontal)
		    rect.setRect(buttonw + fw, fw, maxlen, buttonw);
		else
		    rect.setRect(fw, buttonw + fw, buttonw, maxlen);
 		break;

 	    case SC_ScrollBarSlider:
 		if (scrollbar->orientation() == Qt::Horizontal)
 		    rect.setRect(sliderstart, fw, sliderlen, buttonw);
 		else
 		    rect.setRect(fw, sliderstart, buttonw, sliderlen);
 		break;

	    default:
		break;
	    }

	    break;
	}

    case CC_ComboBox:
	switch ( sc ) {
	case SC_ComboBoxArrow:
	    {
		QComboBox * cb = (QComboBox *) widget;
		int ew, awh, sh, dh, ax, ay, sy;
		int fw = pixelMetric( PM_DefaultFrameWidth, cb );
		QRect cr = cb->rect();
		cr.addCoords( fw, fw, -fw, -fw );
		get_combo_parameters( cr, ew, awh, ax, ay, sh, dh, sy );
		rect.setRect( ax, ay, awh, awh );
		break;
	    }

	case SC_ComboBoxEditField:
	    {
		QComboBox * cb = (QComboBox *) widget;
		int fw = pixelMetric( PM_DefaultFrameWidth, cb );
		rect = cb->rect();
		rect.addCoords( fw, fw, -fw, -fw );
		int ew = get_combo_extra_width( rect.height() );

 		rect.addCoords( 1, 1, -1-ew, -1 );
		break;
	    }

	default:
	    break;
	}
	break;

    default:
	return QCommonStyle::querySubControlMetrics( control, widget, sc, data );
    }

    return rect;
}

QSize QMotifStyle::sizeFromContents( ContentsType contents,
				     const QWidget *widget,
				     const QSize &contentsSize,
				     void **data ) const
{
    QSize sz(contentsSize);

    switch(contents) {
    case CT_PushButton:
	{
	    QPushButton *button = (QPushButton *) widget;
	    sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, data);
	    if ((button->isDefault() || button->autoDefault()) &&
		sz.width() < 80 && ! button->pixmap())
		sz.setWidth(80);
	    break;
	}

    case CT_PopupMenuItem:
	{
	    if (! widget || ! data)
		break;

	    QPopupMenu *popup = (QPopupMenu *) widget;
	    bool checkable = popup->isCheckable();
	    QMenuItem *mi = (QMenuItem *) data[0];
	    int maxpmw = *((int *) data[1]);
	    int w = sz.width(), h = sz.height() + 2*motifItemVMargin + 2*motifItemFrame;

	    if ( mi->isSeparator() ) {
		w = 10;
		h = motifSepHeight;
		break;
	    }

	    // a little bit of border can never harm
	    w += 2*motifItemHMargin + 2*motifItemFrame;

	    if ( !mi->text().isNull() && mi->text().find('\t') >= 0 )
		// string contains tab
		w += motifTabSpacing;
	    else if (mi->popup())
		// submenu indicator needs some room if we don't have a tab column
		w += motifArrowHMargin + 2*motifItemFrame;

	    if ( checkable && maxpmw <= 0)
		// if we are checkable and have no iconsets, add space for a checkmark
		w += motifCheckMarkSpace;
	    else if (checkable && maxpmw < motifCheckMarkSpace)
		// make sure the check-column is wide enough if we have iconsets
		w += (motifCheckMarkSpace - maxpmw);

	    // if we have a check-column ( iconsets of checkmarks), add space
	    // to separate the columns
	    if ( maxpmw > 0 || checkable )
		w += motifCheckMarkHMargin;

	    sz = QSize(w, h);
	    break;
	}

    default:
	sz = QCommonStyle::sizeFromContents( contents, widget, contentsSize, data );
	break;
    }

    return sz;
}

QRect QMotifStyle::subRect( SubRect r, const QWidget *widget ) const
{
    QRect rect;

    switch ( r ) {
    case SR_SliderFocusRect:
	rect = QCommonStyle::subRect( r, widget );
	rect.addCoords( 2, 2, -2, -3 ); // ### fix this!
	break;

    case SR_ComboBoxFocusRect:
	{
	    int awh, ax, ay, sh, sy, dh, ew;
	    int fw = pixelMetric( PM_DefaultFrameWidth, widget );
	    QRect tr = widget->rect();

	    tr.addCoords( fw, fw, -fw, -fw );
	    get_combo_parameters( tr, ew, awh, ax, ay, sh, dh, sy );
	    rect.setRect(ax-2, ay-2, awh+4, awh+sh+dh+4);
	    break;
	}

    case SR_DockWindowHandleRect:
	{
	    if ( !widget || !widget->parent() )
		break;

	    QDockWindow * dw = (QDockWindow *) widget->parent();
	    if ( !dw->area() || !dw->isCloseEnabled() )
		rect.setRect( 0, 0, widget->width(), widget->height() );
	    else {
		if ( dw->area()->orientation() == Horizontal )
		    rect.setRect(2, 15, widget->width()-2, widget->height() - 15);
		else
		    rect.setRect(0, 2, widget->width() - 15, widget->height() - 2);
	    }
	    break;
	}

    case SR_ProgressBarContents:
    case SR_ProgressBarGroove:
	rect = widget->rect();
	break;

    case SR_ProgressBarLabel:
	rect = widget->rect();
	break;

    default:
	rect = QCommonStyle::subRect( r, widget );
    }

    return rect;
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
QPixmap QMotifStyle::stylePixmap(StylePixmap sp, const QWidget *, void **) const
{
    switch (sp) {
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
    return QPixmap();
}


#endif
