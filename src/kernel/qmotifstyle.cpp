/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmotifstyle.cpp#25 $
**
** Implementation of Motif-like style class
**
** Created : 981231
**
** Copyright (C) 1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qmotifstyle.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now
#include "qpixmap.h" // for now
#include "qpalette.h" // for now
#include "qwidget.h"
#include "qlabel.h"
#include "qimage.h"
#include "qpushbutton.h"
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include <limits.h>

/*!
  \class QMotifStyle qmotifstyle.h
  \brief Motif Look and Feel

  This class implements the Motif look and feel. It almost completely
  resembles the original Motif look as defined by the Open Group, but
  also contains minor improvements. The Motif style is Qt's default
  GUI style on UNIX plattforms.
*/

/*!
    Constructs a QMotifStyle.

    If useHighlightCols is FALSE (default value), then the style will
    polish the application's color palette to emulate the Motif way of
    highlighting, which is a simple inversion between the base and the
    text color.
*/
QMotifStyle::QMotifStyle( bool useHighlightCols ) : QStyle(MotifStyle)
{
    highlightCols = useHighlightCols;
}



/*!
  If the argument is FALSE, then the style will polish the
  application's color palette to emulate the
  Motif way of highlighting, which is a simple inversion between the
  base and the text color.

  The effect will show up the next time a application palette is set
  via QApplication::setPalette(). The current color palette of the
  application remains unchanged.

  \sa QStyle::polish( QPalette& ), selectionOnlyInverse()
 */
void QMotifStyle::setUseHighlightColors( bool arg)
{
    highlightCols = arg;
}

/*!
  Returns whether the style treats the highlight colors of the palette
  Motif-like, which is a simple inversion between the base and the
  text color. The default is FALSE.

  \sa setSelectionOnlyInverse()
 */
bool QMotifStyle::useHighlightColors() const
{
    return highlightCols;
}

/*! \reimp */

void QMotifStyle::polish( QPalette& pal)
{
    if ( highlightCols )
	return;

    // force the ugly motif way of highlighting *sigh*
    QColorGroup normal = pal.normal();
    QColorGroup disabled = pal.disabled();
    QColorGroup active = pal.active();

//     int h,s,v;
//     normal.text().hsv(&h,&s,&v);
//     if (v >= 255-50) {
// 	normal.setHighlight( Qt::white );
// 	normal.setHighlightedText( normal.base() );
// 	disabled.setHighlight( Qt::white );
// 	disabled.setHighlightedText( disabled.base() );
// 	active.setHighlight( Qt::white );
// 	active.setHighlightedText( active.base() );
//     } else {
// 	normal.setHighlight( Qt::black );
// 	normal.setHighlightedText( normal.base() );
// 	disabled.setHighlight( Qt::black );
// 	disabled.setHighlightedText( disabled.base() );
// 	active.setHighlight( Qt::black );
// 	active.setHighlightedText( active.base() );
//     }

    pal.setColor( QPalette::Normal, QColorGroup::Highlight,
		  normal.text() );
    pal.setColor( QPalette::Normal, QColorGroup::HighlightedText,
		  normal.base());
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight,
		  disabled.text() );
    pal.setColor( QPalette::Disabled, QColorGroup::HighlightedText,
		  disabled.base() );
    pal.setColor( QPalette::Active, QColorGroup::Highlight,
		  active.text() );
    pal.setColor( QPalette::Active, QColorGroup::HighlightedText,
		  active.base() );
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
	qDrawShadePanel( p, x, y, w, h, g, !showUp, defaultFrameWidth(), &fill );
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
    static QCOORD inner_pts[] =		// used for filling diamond
    { 2,6, 6,2, 10,6, 6,10 };
    static QCOORD top_pts[] =		// top (^) of diamond
    { 0,6, 6,0 , 11,5, 10,5, 6,1, 1,6, 2,6, 6,2, 9,5 };
    static QCOORD bottom_pts[] =		// bottom (V) of diamond
    { 1,7, 6,12, 12,6, 11,6, 6,11, 2,7, 3,7, 6,10, 10,6 };

    bool showUp = !(down ^ on );
    QPointArray a( QCOORDARRLEN(inner_pts), inner_pts );
    p->eraseRect( x, y, w, h );
    p->setPen( NoPen );
    p->setBrush( showUp ? g.brush( QColorGroup::Button ) :
                          g.brush( QColorGroup::Mid ) )  ;
    a.translate( x, y );
    p->drawPolygon( a );			// clear inner area
    p->setPen( showUp ? g.light() : g.dark() );
    p->setBrush( NoBrush );
    a.setPoints( QCOORDARRLEN(top_pts), top_pts );
    a.translate( x, y );
    p->drawPolyline( a );			// draw top part
    p->setPen( showUp ? g.dark() : g.light() );
    a.setPoints( QCOORDARRLEN(bottom_pts), bottom_pts );
    a.translate( x, y );
    p->drawPolyline( a );			// draw bottom part
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
		 const QColorGroup &g, bool /* enabled */, const QBrush * /* fill */ )
{
    // ### may be worth caching these as pixmaps, especially with the
    //	    cost of rotate() for vertical arrows.

    QPointArray bFill;				// fill polygon
    QPointArray bTop;				// top shadow.
    QPointArray bBot;				// bottom shadow.
    QPointArray bLeft;				// left shadow.
    QWMatrix	matrix;				// xform matrix
    bool vertical = type == UpArrow || type == DownArrow;
    bool horizontal = !vertical;
    int	 dim = w < h ? w : h;
    int	 colspec = 0x0000;			// color specification array

    if ( dim < 2 )				// too small arrow
	return;

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
	if ( dim & 1 )				// odd number size: extra line
	    bBot.putPoints( dim-1, 2, dim-3,dim/2, dim-1,dim/2 );
	if ( dim > 6 ) {			// dim>6: must fill interior
	    bFill.putPoints( 0, 2, 1,dim-3, 1,2 );
	    if ( dim & 1 )			// if size is an odd number
		bFill.setPoint( 2, dim - 3, dim / 2 );
	    else
		bFill.putPoints( 2, 2, dim-4,dim/2-1, dim-4,dim/2 );
	}
    }
    else {
	if ( dim == 3 ) {			// 3x3 arrow pattern
	    bLeft.setPoints( 4, 0,0, 0,2, 1,1, 1,1 );
	    bTop .setPoints( 2, 1,0, 1,0 );
	    bBot .setPoints( 2, 1,2, 2,1 );
	}
	else {					// 2x2 arrow pattern
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
    cols[0] = 0;
    cols[1] = (QColor *)&g.button();
    cols[2] = (QColor *)&g.mid();
    cols[3] = (QColor *)&g.light();
    cols[4] = (QColor *)&g.dark();
#define CMID	*cols[ (colspec>>12) & 0xf ]
#define CLEFT	*cols[ (colspec>>8) & 0xf ]
#define CTOP	*cols[ (colspec>>4) & 0xf ]
#define CBOT	*cols[ colspec & 0xf ]

    QPen     savePen   = p->pen();		// save current pen
    QBrush   saveBrush = p->brush();		// save current brush
    QWMatrix wxm = p->worldMatrix();
    QPen     pen( NoPen );
    QBrush brush = g.brush( QColorGroup::Button );

    p->setPen( pen );
    p->setBrush( brush );
    p->setWorldMatrix( matrix, TRUE );		// set transformation matrix
    p->drawPolygon( bFill );			// fill arrow
    p->setBrush( NoBrush );			// don't fill

    p->setPen( CLEFT );
    p->drawLineSegments( bLeft );
    p->setPen( CTOP );
    p->drawLineSegments( bTop );
    p->setPen( CBOT );
    p->drawLineSegments( bBot );

    p->setWorldMatrix( wxm );
    p->setBrush( saveBrush );			// restore brush
    p->setPen( savePen );			// restore pen

#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT

}


/*!
  Draws a press-sensitive shape.
*/
void QMotifStyle::drawButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    qDrawShadePanel( p, x, y, w, h, g, sunken, defaultFrameWidth(),
		     fill ? fill : (sunken ?
				    &g.brush( QColorGroup::Mid )      :
				    &g.brush( QColorGroup::Button ) ));
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
			    const QRect& r, const QColorGroup &/*g */, const QColor* bg, bool atBorder)
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
	p->setPen( Qt::black);
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

    btn->rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

    p->setPen( g.foreground() );
    p->setBrush( QBrush(g.button(),NoBrush) );

    QBrush fill;
    if ( btn->isDown() )
	fill = g.brush( QColorGroup::Mid );
    else if ( btn->isOn() )
	fill = QBrush( g.mid(), Dense4Pattern );
    else
	fill = g.brush( QColorGroup::Button );	

    if ( btn->isDefault() ) {
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
    }
	
    drawButton( p, x1, y1, x2-x1+1, y2-y1+1, g, btn->isOn() || btn->isDown(),
		&fill );
	

    if ( btn->isMenuButton() ) {
	int dx = (y1-y2-4)/3;
	drawArrow( p, DownArrow, FALSE,
		   x2 - dx, dx, y1, y2 - y1,
		   g, btn->isEnabled() );
    }

    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );
}

#define HORIZONTAL	(sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	defaultFrameWidth();
#define SLIDER_MIN	9 // ### motif says 6 but that's too small


/*! \reimp */

void QMotifStyle::scrollBarMetrics( const QScrollBar* sb, int &sliderMin, int &sliderMax, int &sliderLength, int &buttonDim )
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

void QMotifStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb, int sliderStart, uint controls, uint activeControl )
{
#define ADD_LINE_ACTIVE ( activeControl == ADD_LINE )
#define SUB_LINE_ACTIVE ( activeControl == SUB_LINE )
    QColorGroup g  = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax) { // sanity check
	sliderStart = sliderMax;
    }

    int b = MOTIF_BORDER;
    int dimB = buttonDim;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( HORIZONTAL ) {
	subY = addY = ( extent - dimB ) / 2;
	subX = b;
	addX = length - dimB - b;
    } else {
	subX = addX = ( extent - dimB ) / 2;
	subY = b;
	addY = length - dimB - b;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
	subPageR.setRect( subB.right() + 1, b,
			  sliderStart - subB.right() - 1 , sliderW );
	addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
	sliderR .setRect( sliderStart, b, sliderLength, sliderW );
    } else {
	subPageR.setRect( b, subB.bottom() + 1, sliderW,
			  sliderStart - subB.bottom() - 1 );
	addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
	sliderR .setRect( b, sliderStart, sliderW, sliderLength );
    }

    if ( controls & ADD_LINE )
	drawArrow( p, VERTICAL ? DownArrow : RightArrow,
		   ADD_LINE_ACTIVE, addB.x(), addB.y(),
		   addB.width(), addB.height(), g, sb->value()<sb->maxValue() );
    if ( controls & SUB_LINE )
	drawArrow( p, VERTICAL ? UpArrow : LeftArrow,
		   SUB_LINE_ACTIVE, subB.x(), subB.y(),
		   subB.width(), subB.height(), g, sb->value()>sb->minValue() );

    QBrush fill = g.brush( QColorGroup::Mid );
    if (sb->backgroundPixmap() ){
	fill = QBrush( g.mid(), *sb->backgroundPixmap() );
    }

    if ( controls & SUB_PAGE )
	p->fillRect( subPageR, fill );
	
    if ( controls & ADD_PAGE )
	p->fillRect( addPageR, fill );

    if ( controls & SLIDER ) {
	QPoint bo = p->brushOrigin();
	p->setBrushOrigin(sliderR.topLeft());
	drawBevelButton( p, sliderR.x(), sliderR.y(),
			 sliderR.width(), sliderR.height(), g,
			 FALSE, &g.brush( QColorGroup::Button ) );

	//	qDrawShadePanel( p, sliderR, g, FALSE, 2, &g.fillButton() );
	p->setBrushOrigin(bo);
    }

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
    qDrawShadePanel( p, x, y, w, h, g, TRUE );
}
