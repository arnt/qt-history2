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
*/


/*!
    Constructs a QMotifStyle
*/
QMotifStyle::QMotifStyle() : QStyle(MotifStyle)
{
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QMotifStyle::initialize( QApplication* app)
{
    // force the ugly motif way of highlighting *sigh*
    QColorGroup normal = app->palette()->normal();
    QColorGroup disabled = app->palette()->disabled();
    QColorGroup active = app->palette()->active();
    normal.setHighlight( Qt::black );
    normal.setHighlightedText( normal.base() );
    disabled.setHighlight( Qt::black );
    disabled.setHighlightedText( disabled.base() );
    active.setHighlight( Qt::black );
    active.setHighlightedText( active.base() );
    app->setPalette(QPalette(normal, disabled, active), TRUE); // TODO
    // really TRUE? when is this called?Ideally, before the first widget
    // is constructed.... #####
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */

void QMotifStyle::drawIndicator( QPainter* p,
				 int x, int y, int w, int h, const QColorGroup &g,
				 bool on, bool down, bool /* enabled */ )
{
    bool showUp = !(down ^ on);
    QBrush fill =  showUp ? g.fillButton() : g.fillMid();
    qDrawShadePanel( p, x, y, w, h, g, !showUp, 2, &fill );
}


/*!
  Reimplementation from QStyle

  \sa QStyle
  */
QSize
QMotifStyle::indicatorSize() const
{
    return QSize(13,13);
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
/*!
  Reimplementation from QStyle

  \sa QStyle
  */
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
    p->setBrush( showUp?  g.fillButton() : g.fillMid() );
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


/*!
  Reimplementation from QStyle

  \sa QStyle
  */
QSize
QMotifStyle::exclusiveIndicatorSize() const
{
    return QSize(13,13);
}




/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void
QMotifStyle::drawArrow( QPainter *p, ArrowType type, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool /* enabled */, const QBrush * /* fill */ )
{
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
    QBrush brush = g.fillButton();

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
    qDrawShadePanel( p, x, y, w, h, g, sunken,
		     2, fill?fill:(sunken?&g.fillMid():&g.fillButton()));
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QMotifStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    QMotifStyle::drawButton(p, x, y, w, h, g, sunken, fill);
}


/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void
QMotifStyle::drawFocusRect( QPainter* p,
			    const QRect& r, const QColorGroup &/*g */, const QColor* col)
{
    if (col && *col == Qt::black)
	p->setPen( Qt::white );
    else
	p->setPen( Qt::black);
    p->drawRect( r );
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
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
	fill = g.fillMid();
    else if ( btn->isOn() )
	fill = QBrush( g.mid(), Dense4Pattern );
    else
	fill = g.fillButton();	

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
#define MOTIF_BORDER	2
#define SLIDER_MIN	9 // ### motif says 6 but that's too small

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QMotifStyle::scrollBarMetrics( const QScrollBar* sb, int *sliderMin, int *sliderMax, int *sliderLength )
{
    int buttonDim, maxLength;
    int b = MOTIF_BORDER;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2  )
	buttonDim = extent - b*2;
    else
	buttonDim = ( length - b*2 )/2 - 1;

    *sliderMin = b + buttonDim;
    maxLength  = length - b*2 - buttonDim*2;

    if ( sb->maxValue() == sb->minValue() ) {
	*sliderLength = maxLength;
    } else {
	*sliderLength = (sb->pageStep()*maxLength)/
			(sb->maxValue()-sb->minValue()+sb->pageStep());
	if ( *sliderLength < SLIDER_MIN )
	    *sliderLength = SLIDER_MIN;
	if ( *sliderLength > maxLength )
	    *sliderLength = maxLength;
    }
    *sliderMax = *sliderMin + maxLength - *sliderLength;

}


/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QMotifStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb, int sliderStart, uint controls, uint activeControl )
{
#define ADD_LINE_ACTIVE ( activeControl == ADD_LINE )
#define SUB_LINE_ACTIVE ( activeControl == SUB_LINE )
    QColorGroup g  = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength;
    scrollBarMetrics( sb, &sliderMin, &sliderMax, &sliderLength );

    int b = MOTIF_BORDER;
    int dimB = sliderMin - b;
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

    QBrush fill = g.fillMid();
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
			 FALSE, &g.fillButton() );

	//	qDrawShadePanel( p, sliderR, g, FALSE, 2, &g.fillButton() );
	p->setBrushOrigin(bo);
    }

}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QMotifStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
{
    QRect r = btn->rect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );

    int x1, y1, x2, y2;
    btn->rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates
    int dx = 0;
    int dy = 0;
    if ( btn->isMenuButton() )
	dx = (y2-y1) / 3;
    if ( dx || dy )
	p->translate( dx, dy );

    x += 2;  y += 2;  w -= 4;  h -= 4;
    drawItem( p, x, y, w, h,
	      AlignCenter|ShowPrefix,
	      btn->colorGroup(), btn->isEnabled(),
	      btn->pixmap(), btn->text(), -1, &btn->colorGroup().buttonText() );
    if ( dx || dy )
	p->translate( -dx, -dy );

}



int QMotifStyle::sliderLength() const
{
    return 30;
}
void QMotifStyle::drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			      Orientation orient, bool, bool )
{
    drawBevelButton( p, x, y, w, h, g, FALSE, &g.fillButton() );
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

void QMotifStyle::drawSliderGroove( QPainter *p,
				      int x, int y, int w, int h,
				      const QColorGroup& g, QCOORD /*c */,
				      Orientation )
{
    qDrawShadePanel( p, x, y, w, h, g, TRUE );
}
