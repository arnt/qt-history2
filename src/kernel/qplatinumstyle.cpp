#include "qplatinumstyle.h"
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

QPlatinumStyle::QPlatinumStyle()
{
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::initialize( QApplication* app)
{
    QColor standardLightGray( 222, 222, 222 );
    QColor light( 255, 255, 255 );
    QColor dark (98, 101, 98);
    QColor mid (139, 137, 139);
    QColorGroup nor( QColor::black, standardLightGray,
			 light, dark, mid,
			 QColor::black, QColor::white, QColor::white, standardLightGray );
    QColorGroup dis( QColor::darkGray, standardLightGray,
			 light, dark, mid,
			 QColor::darkGray, QColor::white, QColor::white, nor.background() );
    QColorGroup act( QColor::black, standardLightGray,
			 light, dark, mid,
			 QColor::black, QColor::white, QColor::white, nor.background() );

    app->setPalette(QPalette(nor, dis, act), TRUE );

}

/*!
  Draws a press-sensitive shape.
*/
void QPlatinumStyle::drawButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{

    QPen oldPen = p->pen();

     if (!sunken) {
	 p->fillRect(x+3, y+3, w-6, h-6,fill?*fill:g.fillButton());
	 // the bright side
	 p->setPen(QColor::black);
	 p->drawLine(x, y, x+w-1, y);
	 p->drawLine(x, y, x, y+h-1);

	 p->setPen(g.button());
	 p->drawLine(x+1, y+1, x+w-2, y+1);
	 p->drawLine(x+1, y+1, x+1, y+h-2);

	 p->setPen(g.light());
	 p->drawLine(x+2, y+2, x+2, y+h-2);
	 p->drawLine(x+2, y+2, x+w-2, y+2);


	 // the dark side!

	 p->setPen(g.mid());
	 p->drawLine(x+3, y+h-3 ,x+w-3, y+h-3);
	 p->drawLine(x+w-3, y+3, x+w-3, y+h-3);

	 p->setPen(g.dark());
	 p->drawLine(x+2, y+h-2 ,x+w-2, y+h-2);
	 p->drawLine(x+w-2, y+2, x+w-2, y+h-2);

	 p->setPen(QColor::black);
	 p->drawLine(x+1, y+h-1,x+w-1, y+h-1);
	 p->drawLine(x+w-1, y, x+w-1, y+h-1);


 	 // top left corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x, y);
 	 p->drawPoint(x+1, y);
 	 p->drawPoint(x, y+1);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+1, y+1);
 	 p->setPen(QColor::white);
 	 p->drawPoint(x+3, y+3);
 	 // bottom left corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x, y+h-1);
 	 p->drawPoint(x+1, y+h-1);
 	 p->drawPoint(x, y+h-2);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+1, y+h-2);
 	 // top right corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x+w-1, y);
 	 p->drawPoint(x+w-2, y);
 	 p->drawPoint(x+w-1, y+1);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+w-2, y+1);
 	 // bottom right corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x+w-1, y+h-1);
 	 p->drawPoint(x+w-2, y+h-1);
 	 p->drawPoint(x+w-1, y+h-2);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+w-2, y+h-2);
 	 p->setPen(g.dark());
 	 p->drawPoint(x+w-3, y+h-3);
 	 p->setPen(g.mid());
 	 p->drawPoint(x+w-4, y+h-4);

     }
     else {
	 p->fillRect(x+2, y+2, w-4, h-4,fill?*fill:g.fillDark());

	 // the dark side
	 p->setPen(QColor::black);
	 p->drawLine(x, y, x+w-1, y);
	 p->drawLine(x, y, x, y+h-1);

	 p->setPen(g.dark().dark());
	 p->drawLine(x+1, y+1, x+w-2, y+1);
	 p->drawLine(x+1, y+1, x+1, y+h-2);


	 // the bright side!

	 p->setPen(g.button());
	 p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
	 p->drawLine(x+w-2, y+1, x+w-2, y+h-2);

	 p->setPen(g.dark());
	 p->drawLine(x, y+h-1,x+w-1, y+h-1);
	 p->drawLine(x+w-1, y, x+w-1, y+h-1);

 	 // top left corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x, y);
 	 p->drawPoint(x+1, y);
 	 p->drawPoint(x, y+1);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+1, y+1);
 	 p->setPen(g.dark().dark());
 	 p->drawPoint(x+3, y+3);
 	 // bottom left corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x, y+h-1);
 	 p->drawPoint(x+1, y+h-1);
 	 p->drawPoint(x, y+h-2);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+1, y+h-2);
 	 // top right corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x+w-1, y);
 	 p->drawPoint(x+w-2, y);
 	 p->drawPoint(x+w-1, y+1);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+w-2, y+1);
 	 // bottom right corner:
 	 p->setPen(g.background());
 	 p->drawPoint(x+w-1, y+h-1);
 	 p->drawPoint(x+w-2, y+h-1);
 	 p->drawPoint(x+w-1, y+h-2);
 	 p->setPen(QColor::black);
 	 p->drawPoint(x+w-2, y+h-2);
 	 p->setPen(g.dark());
 	 p->drawPoint(x+w-3, y+h-3);
 	 p->setPen(g.mid());
 	 p->drawPoint(x+w-4, y+h-4);
	

     }

     //     // top left corner:
//     p->setPen(g.background());
//     p->drawPoint(x, y);
//     p->drawPoint(x, y);


    p->setPen(oldPen);
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
QRect QPlatinumStyle::buttonRect( int x, int y, int w, int h){
    return QRect(x+1, y+1, w-2, h-2);
}

/*!
  mixes two colors to a new colors
  */
QColor QPlatinumStyle::mixedColor(const QColor &c1, const QColor &c2)
{
    int h1,s1,v1,h2,s2,v2;
    c1.hsv(&h1,&s1,&v1);
    c2.hsv(&h2,&s2,&v2);
    return QColor( (h1+h2)/2, (s1+s2)/2, (v1+v2)/2, QColor::Hsv );
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    QPen oldPen = p->pen();

    // small or non-square bevel buttons are drawn in a small style, others in a big style.
    if ( w * h < 1600 || QABS(w-h) > 10) {
	// small buttons
	
	if (!sunken) {
	    p->fillRect(x+2, y+2, w-4, h-4,fill?*fill:g.fillButton());
	    // the bright side
	    p->setPen(g.dark());
	    p->drawLine(x, y, x+w-1, y);
	    p->drawLine(x, y, x, y+h-1);

	    p->setPen(g.light());
	    p->drawLine(x+1, y+1, x+w-2, y+1);
	    p->drawLine(x+1, y+1, x+1, y+h-2);

	    // the dark side!

	    p->setPen(g.mid());
	    p->drawLine(x+2, y+h-2 ,x+w-2, y+h-2);
	    p->drawLine(x+w-2, y+2, x+w-2, y+h-3);

	    p->setPen(g.dark().dark());
	    p->drawLine(x+1, y+h-1,x+w-1, y+h-1);
	    p->drawLine(x+w-1, y+1, x+w-1, y+h-2);


	}
	else {
	    p->fillRect(x+2, y+2, w-4, h-4,fill?*fill:g.fillMid());

	    // the dark side
	    p->setPen(g.dark().dark());
	    p->drawLine(x, y, x+w-1, y);
	    p->drawLine(x, y, x, y+h-1);

	    p->setPen(g.mid().dark());
	    p->drawLine(x+1, y+1, x+w-2, y+1);
	    p->drawLine(x+1, y+1, x+1, y+h-2);


	    // the bright side!

	    p->setPen(g.button());
	    p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
	    p->drawLine(x+w-2, y+1, x+w-2, y+h-2);

	    p->setPen(g.dark());
	    p->drawLine(x, y+h-1,x+w-1, y+h-1);
	    p->drawLine(x+w-1, y, x+w-1, y+h-1);
	}
    }
    else {
	// big ones
	
	if (!sunken) {
	    p->fillRect(x+3, y+3, w-6, h-6,fill?*fill:g.fillButton());
	    // the bright side
	    p->setPen(g.button().dark());
	    p->drawLine(x, y, x+w-1, y);
	    p->drawLine(x, y, x, y+h-1);

	    p->setPen(g.button());
	    p->drawLine(x+1, y+1, x+w-2, y+1);
	    p->drawLine(x+1, y+1, x+1, y+h-2);

	    p->setPen(g.light());
	    p->drawLine(x+2, y+2, x+2, y+h-2);
	    p->drawLine(x+2, y+2, x+w-2, y+2);


	 // the dark side!

	    p->setPen(g.mid());
	    p->drawLine(x+3, y+h-3 ,x+w-3, y+h-3);
	    p->drawLine(x+w-3, y+3, x+w-3, y+h-3);

	    p->setPen(g.dark());
	    p->drawLine(x+2, y+h-2 ,x+w-2, y+h-2);
	    p->drawLine(x+w-2, y+2, x+w-2, y+h-2);

	    p->setPen(g.dark().dark());
	    p->drawLine(x+1, y+h-1,x+w-1, y+h-1);
	    p->drawLine(x+w-1, y+1, x+w-1, y+h-1);


	}
	else {
	    p->fillRect(x+3, y+3, w-6, h-6,fill?*fill:g.fillMid());

	    // the dark side
	    p->setPen(g.dark().dark().dark());
	    p->drawLine(x, y, x+w-1, y);
	    p->drawLine(x, y, x, y+h-1);

	    p->setPen(g.dark().dark());
	    p->drawLine(x+1, y+1, x+w-2, y+1);
	    p->drawLine(x+1, y+1, x+1, y+h-2);

	    p->setPen(g.mid().dark());
	    p->drawLine(x+2, y+2, x+2, y+w-2);
	    p->drawLine(x+2, y+2, x+w-2, y+2);


	    // the bright side!

	    p->setPen(g.button());
	    p->drawLine(x+2, y+h-3 ,x+w-3, y+h-3);
	    p->drawLine(x+w-3, y+3, x+w-3, y+h-3);

	    p->setPen(g.midlight());
	    p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
	    p->drawLine(x+w-2, y+1, x+w-2, y+h-2);

	    p->setPen(g.dark());
	    p->drawLine(x, y+h-1,x+w-1, y+h-1);
	    p->drawLine(x+w-1, y, x+w-1, y+h-1);


	    // corners
	    p->setPen( mixedColor(g.dark().dark().dark(), g.dark()) );
	    p->drawPoint( x, y+h-1 );
	    p->drawPoint( x+w-1, y);

	    p->setPen( mixedColor(g.dark().dark(), g.midlight() ) );
	    p->drawPoint( x+1, y+h-2 );
	    p->drawPoint( x+w-2, y+1);

	    p->setPen( mixedColor(g.mid().dark(), g.button() ) );
	    p->drawPoint( x+2, y+h-3 );
	    p->drawPoint( x+w-3, y+2);
	}
    }
    p->setPen(oldPen);

}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void
QPlatinumStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
    QColorGroup g = btn->colorGroup();
    int x1, y1, x2, y2;

    btn->rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

    p->setPen( g.foreground() );
    p->setBrush( QBrush(g.button(),NoBrush) );

    QBrush fill;
    if ( btn->isDown() )
	fill = g.fillDark();
    else if ( btn->isOn() )
	fill = QBrush( g.mid(), Dense4Pattern );
    else
	fill = g.fillButton();	

//     if ( btn->isDefault() ) {
// 	QPointArray a;
// 	a.setPoints( 9,
// 		     x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
// 		     x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1 );
// 	p->setPen( black );
// 	p->drawPolyline( a );
// 	x1 += 2;
// 	y1 += 2;
// 	x2 -= 2;
// 	y2 -= 2;
//     }
	
    // small or square buttons as well as toggle buttons are bevel buttons (what a heuristic....)
    if ( btn->isToggleButton()
	 || btn->width() * btn->height() < 1600 || QABS( btn->width() - btn->height()) < 10 )
	drawBevelButton( p, x1, y1, x2-x1+1, y2-y1+1, g, btn->isOn() || btn->isDown(),
			 &fill );
    else
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

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
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
	      btn->pixmap(), btn->text(), -1, btn->isDown() || btn->isOn());
    if ( dx || dy )
	p->translate( -dx, -dy );

}


#define HORIZONTAL	(sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2
#define SLIDER_MIN	9 // ### motif says 6 but that's too small

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::scrollbarMetrics( const QScrollBar* sb, int *sliderMin, int *sliderMax, int *sliderLength )
{
    int buttonDim, maxLength;
    int b = 0;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2 )
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

      if (maxLength >=  buttonDim)
 	 *sliderLength = buttonDim; // macintosh

    *sliderMax = *sliderMin + maxLength - *sliderLength;

}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::drawScrollbarBackground( QPainter *p, int x, int y, int w, int h,
					      const QColorGroup &g, bool horizontal, const QBrush* fill)
{
    QPen oldPen = p->pen();

    if (w < 3 || h < 3) {
	p->fillRect(x, y, w, h, fill?*fill:g.fillMid());
	p->setPen(QColor::black);
	p->drawRect(x, y, w, h);
	p->setPen(oldPen);
	return;
    }


    if (horizontal) {
	p->fillRect(x+2, y+2, w-2, h-4,fill?*fill:g.fillMid());

	// the dark side
	p->setPen(g.dark().dark());
	p->drawLine(x, y, x+w-1, y);
	p->setPen(QColor::black);
	p->drawLine(x, y, x, y+h-1);

	p->setPen(g.mid().dark());
	p->drawLine(x+1, y+1, x+w-1, y+1);
	p->drawLine(x+1, y+1, x+1, y+h-2);

	// the bright side!

	p->setPen(g.button());
	p->drawLine(x+1, y+h-2 ,x+w-1, y+h-2);
	//p->drawLine(x+w-2, y+1, x+w-2, y+h-2);
	
	p->setPen(QColor::black);
	p->drawLine(x, y+h-1,x+w-1, y+h-1);
	// p->drawLine(x+w-1, y, x+w-1, y+h-1);
    }
    else {
	p->fillRect(x+2, y+2, w-4, h-2,fill?*fill:g.fillMid());

	// the dark side
	p->setPen(g.dark().dark());
	p->drawLine(x, y, x+w-1, y);
	p->setPen(QColor::black);
	p->drawLine(x, y, x, y+h-1);

	p->setPen(g.mid().dark());
	p->drawLine(x+1, y+1, x+w-2, y+1);
	p->drawLine(x+1, y+1, x+1, y+h-1);


	// the bright side!

	p->setPen(g.button());
	//p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
	p->drawLine(x+w-2, y+1, x+w-2, y+h-1);

	p->setPen(QColor::black);
	//p->drawLine(x, y+h-1,x+w-1, y+h-1);
	p->drawLine(x+w-1, y, x+w-1, y+h-1);

    }
    p->setPen(oldPen);

}


/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::drawScrollbarControls( QPainter* p, const QScrollBar* sb, int sliderStart, uint controls, uint activeControl )
{
#define ADD_LINE_ACTIVE ( activeControl == ADD_LINE )
#define SUB_LINE_ACTIVE ( activeControl == SUB_LINE )
    QColorGroup g  = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength;
    scrollbarMetrics( sb, &sliderMin, &sliderMax, &sliderLength );

    int b = 0;
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

    bool maxedOut = (sb->maxValue() == sb->minValue());
    if ( controls & ADD_LINE ) {
 	drawBevelButton( p, addB.x(), addB.y(),
 			 addB.width(), addB.height(), g,
 			 ADD_LINE_ACTIVE);
	p->setPen(QColor::black);
	p->drawRect( addB );
	drawArrow( p, VERTICAL ? DownArrow : RightArrow,
		   FALSE, addB.x()+2, addB.y()+2,
		   addB.width()-4, addB.height()-4, g, !maxedOut,
		   ADD_LINE_ACTIVE?&g.fillMid():&g.fillButton());
    }
    if ( controls & SUB_LINE ) {
	drawBevelButton( p, subB.x(), subB.y(),
			 subB.width(), subB.height(), g,
			 SUB_LINE_ACTIVE );
	p->setPen(QColor::black);
	p->drawRect( subB );
	drawArrow( p, VERTICAL ? UpArrow : LeftArrow,
		    FALSE, subB.x()+2, subB.y()+2,
		   subB.width()-4, subB.height()-4, g, !maxedOut,
		   SUB_LINE_ACTIVE?&g.fillMid():&g.fillButton());
    }


    if ( controls & SUB_PAGE )
	drawScrollbarBackground( p, subPageR.x(), subPageR.y(), subPageR.width(),
				 subPageR.height(),
				 g, HORIZONTAL );
    if ( controls & ADD_PAGE )
	drawScrollbarBackground( p, addPageR.x(), addPageR.y(), addPageR.width(),
				 addPageR.height(),
				 g, HORIZONTAL );
    if ( controls & SLIDER ) {
	QPoint bo = p->brushOrigin();
	p->setBrushOrigin(sliderR.topLeft());
	drawBevelButton( p, sliderR.x(), sliderR.y(),
			 sliderR.width(), sliderR.height(), g,
			 FALSE, &g.fillButton() );
	p->setBrushOrigin(bo);
	drawRiffles(p, sliderR.x(), sliderR.y(),
		    sliderR.width(), sliderR.height(), g, HORIZONTAL);
	p->setPen(QColor::black);
	p->drawRect( sliderR );
    }

    // ### perhaps this should not be able to accept focus if maxedOut?
    if ( sb->hasFocus() && (controls & SLIDER) )
	p->drawWinFocusRect( sliderR.x()+2, sliderR.y()+2,
			     sliderR.width()-5, sliderR.height()-5,
			     sb->backgroundColor() );

}

/*!
  draw the nifty Macintosh decoration used on  sliders
  */
void QPlatinumStyle::drawRiffles( QPainter* p,  int x, int y, int w, int h,
		      const QColorGroup &g, bool horizontal )
{
	if (!horizontal) {
	    if (h > 8) {
		int n = h / 4;
		int my = y+h/2-n;
		int i ;
		p->setPen(g.light());
		for (i=0; i<n; i++) {
		    p->drawLine(x+3, my+2*i, x+w-5, my+2*i);
		}
		p->setPen(g.dark());
		my++;
		for (i=0; i<n; i++) {
		    p->drawLine(x+4, my+2*i, x+w-4, my+2*i);
		}
	    }
	}
	else {
	    if (w > 8) {
		int n = w / 4;
		int mx = x+w/2-n;
		int i ;
		p->setPen(g.light());
		for (i=0; i<n; i++) {
		    p->drawLine(mx+2*i, y+3, mx + 2*i, y+h-5);
		}
		p->setPen(g.dark());
		mx++;
		for (i=0; i<n; i++) {
		    p->drawLine(mx+2*i, y+4, mx + 2*i, y+h-4);
		}
	    }
	}
}


/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::drawIndicator( QPainter* p,
				    int x, int y, int w, int h, const QColorGroup &g,
				    bool on , bool down, bool /*enabled */ )
{
    QBrush fill;
    if ( down )
	fill = g.fillDark();
    else
	fill = g.fillButton();

    drawBevelButton( p, x, y, w-2, h, g,
		     down, &fill );
    p->fillRect(x+w-2, y, 2, h, g.fillBackground() );
    p->setPen( QColor::black );
    p->drawRect( x, y, w-2, h );

    static QCOORD check_mark[] = {
	3,5, 5,5,  4,6, 5,6,  5,7, 6,7,  5,8, 6,8,	6,9, 9,9,
	6,10, 8,10,	 7,11, 8,11,  7,12, 7,12,  8,8, 9,8,  8,7, 10,7,
	9,6, 10,6,	9,5, 11,5,  10,4, 11,4,	 10,3, 12,3,
	11,2, 12,2,	 11,1, 13,1,  12,0, 13,0 };
    static QCOORD check_mark_pix[] = {
	3,6, 6,6, 4,7, 7,8, 5,9, 6,11, 8,12, 9,10, 10,8, 8,6,
	11,6, 9,4, 12,4, 10,2, 13,2 };
    if (on) {
	QPen oldPen = p->pen();
// 	p->setPen (QPen(g.text(), 2));
// 	p->drawLine( x+2, y+h/2-1, x+w/2-1, y+h-4);
// 	p->drawLine(x+w/2-1, y+h-4, x+w, 0);
// 	p->setPen( oldPen );
	
	int x1 = x;
	int y1 = y;
	if ( down ) {			// shift check mark
	    x1++;
	    y1++;
	}
	QPointArray amark( sizeof(check_mark)/(sizeof(QCOORD)*2),
			   check_mark );
	amark.translate( x1, y1 );
	p->setPen( g.foreground() );
	p->drawLineSegments( amark );
	p->setPen( g.dark() );
	for ( int i=0; i<(int)(sizeof(check_mark_pix)/sizeof(QCOORD));
	      i+=2 )
	    p->drawPoint( x1 + check_mark_pix[i],
			  y1 + check_mark_pix[i+1] );
	p->setPen( oldPen );
    }


}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void
QPlatinumStyle::drawIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on)
{
    p->fillRect(x, y, w-2, h, QColor::color1);
    if (on) {
	QPen oldPen = p->pen();
	p->setPen (QPen(QColor::color1, 2));
	p->drawLine( x+2, y+h/2-1, x+w/2-1, y+h-4);
	p->drawLine(x+w/2-1, y+h-4, x+w, 0);
	p->setPen( oldPen );
    }
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
QSize
QPlatinumStyle::indicatorSize() const
{
    return QSize(15,13);
}



#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::drawExclusiveIndicator( QPainter* p,
				   int x, int y, int w, int h, const QColorGroup &g,
				   bool on, bool down, bool /* enabled */ )
{
    static QCOORD pts1[] = {		// normal circle
	5,0, 10,0, 11,1, 12,1, 13,2, 14,3, 14,4, 15,5,
	15,10, 14,11, 14,12, 13,13, 12,14, 11,14, 10,15,
	5,15, 4,14, 3,14, 2,13, 1,12, 1,11, 0,10, 0,5,
	1,4, 1,3, 2,2, 3,1, 4,1 };
    static QCOORD pts2[] = {		// top left shadow
	5,1, 10,1,	3,2, 7,2,  2,3, 5,3,  2,4, 4,4,
	1,5, 3,5,  1,6, 1,10,  2,6, 2,7 };
    static QCOORD pts3[] = {		// bottom right, dark
	5,14, 10,14,  7,13, 12,13,	10,12, 13,12,
	11,11, 13,11,  12,10, 14,10,  13,8, 13,9,
	14,5, 14,9 };
    static QCOORD pts4[] = {		// bottom right, light
	5,14, 10,14,  9,13, 12,13,	11,12, 13,12,
	12,11, 13,11,  13,9, 13,10,	 14,5, 14,10 };
    static QCOORD pts5[] = {		// check mark
	6,4, 8,4, 10,6, 10,8, 8,10, 6,10, 4,8, 4,6 };
    static QCOORD pts6[] = {		// check mark extras
	4,5, 5,4,  9,4, 10,5,  10,9, 9,10,	5,10, 4,9 };
    p->eraseRect(x,y,w,h);
    p->setBrush((down||on)?g.fillDark():g.fillButton());
    p->setPen(NoPen);
    p->drawEllipse( x, y, 15, 15);
    p->setPen( QColor::black );
    QPointArray a( QCOORDARRLEN(pts1), pts1 );
    a.translate( x, y );
    p->drawPolyline( a );			// draw normal circle
    QColor tc, bc;
    QCOORD *bp;
    int	bl;
    if ( down || on) {			// pressed down or on
	tc = g.dark().dark();
	bc = g.light();
	bp = pts4;
	bl = QCOORDARRLEN(pts4);
    }
    else {					// released
	tc = g.light();
	bc = g.dark();
	bp = pts3;
	bl = QCOORDARRLEN(pts3);
    }
    p->setPen( tc );
    a.setPoints( QCOORDARRLEN(pts2), pts2 );
    a.translate( x, y );
    p->drawLineSegments( a );		// draw top shadow
    p->setPen( bc );
    a.setPoints( bl, bp );
    a.translate( x, y );
    p->drawLineSegments( a );
    if ( on ) {				// draw check mark
	int x1=x, y1=y;
	x1++;
	y1++;
	p->setBrush( g.foreground() );
	p->setPen( g.foreground() );
	a.setPoints( QCOORDARRLEN(pts5), pts5 );
	a.translate( x1, y1 );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
	p->setPen( g.dark() );
	a.setPoints( QCOORDARRLEN(pts6), pts6 );
	a.translate( x1, y1 );
	p->drawLineSegments( a );
    }
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
QSize
QPlatinumStyle::exclusiveIndicatorSize() const
{
    return QSize(18,18);
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
void QPlatinumStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool /* sunken */,
				  bool /* enabled */,
				  const QBrush *fill )
{
    QPen oldPen = p->pen();


    p->fillRect(x+2, y+2, w-4, h-4,fill?*fill:g.fillButton());
    // the bright side
    p->setPen(QColor::black);
    p->drawLine(x, y, x+w-1, y);
    p->drawLine(x, y, x, y+h-1);

    p->setPen(g.light());
    p->drawLine(x+1, y+1, x+w-2, y+1);
    p->drawLine(x+1, y+1, x+1, y+h-2);


	 // the dark side!


    p->setPen(g.mid());
    p->drawLine(x+2, y+h-2 ,x+w-2, y+h-2);
    p->drawLine(x+w-2, y+2, x+w-2, y+h-2);

    p->setPen(QColor::black);
    p->drawLine(x+1, y+h-1,x+w-1, y+h-1);
    p->drawLine(x+w-1, y, x+w-1, y+h-1);


    // top left corner:
    p->setPen(g.background());
    p->drawPoint(x, y);
    p->drawPoint(x+1, y);
    p->drawPoint(x, y+1);
    p->setPen(QColor::black);
    p->drawPoint(x+1, y+1);
//     p->setPen(white);
//     p->drawPoint(x+3, y+3);
    // bottom left corner:
    p->setPen(g.background());
    p->drawPoint(x, y+h-1);
    p->drawPoint(x+1, y+h-1);
    p->drawPoint(x, y+h-2);
    p->setPen(QColor::black);
    p->drawPoint(x+1, y+h-2);
    // top right corner:
    p->setPen(g.background());
    p->drawPoint(x+w-1, y);
    p->drawPoint(x+w-2, y);
    p->drawPoint(x+w-1, y+1);
    p->setPen(QColor::black);
    p->drawPoint(x+w-2, y+1);
    // bottom right corner:
    p->setPen(g.background());
    p->drawPoint(x+w-1, y+h-1);
    p->drawPoint(x+w-2, y+h-1);
    p->drawPoint(x+w-1, y+h-2);
    p->setPen(QColor::black);
    p->drawPoint(x+w-2, y+h-2);
    p->setPen(g.dark());
    p->drawPoint(x+w-3, y+h-3);
//     p->setPen(g.mid());
//     p->drawPoint(x+w-4, y+h-4);


//     drawButton(p, w-2-16,2,16,h-4, g, sunken );


    // now the arrow button

    {
	int xx = w-20;
	int yy = 0;
	int ww = 20;
	int hh = h;
	// the bright side

	 p->setPen(g.mid());
	 p->drawLine(xx, yy+2, xx, yy+hh-3);

	p->setPen(g.button());
	p->drawLine(xx+1, yy+1, xx+ww-2, yy+1);
	p->drawLine(xx+1, yy+1, xx+1, yy+hh-2);

	p->setPen(g.light());
	p->drawLine(xx+2, yy+2, xx+2, yy+hh-2);
	p->drawLine(xx+2, yy+2, xx+ww-2, yy+2);


	// the dark side!

	p->setPen(g.mid());
	p->drawLine(xx+3, yy+hh-3 ,xx+ww-3, yy+hh-3);
	p->drawLine(xx+ww-3, yy+3, xx+ww-3, yy+hh-3);

	p->setPen(g.dark());
	p->drawLine(xx+2, yy+hh-2 ,xx+ww-2, yy+hh-2);
	p->drawLine(xx+ww-2, yy+2, xx+ww-2, yy+hh-2);

	p->setPen(QColor::black);
	p->drawLine(xx+1, yy+hh-1,xx+ww-1, yy+hh-1);
	p->drawLine(xx+ww-1, yy, xx+ww-1, yy+hh-1);

	// top right corner:
	p->setPen(g.background());
	p->drawPoint(xx+ww-1, yy);
	p->drawPoint(xx+ww-2, yy);
	p->drawPoint(xx+ww-1, yy+1);
	p->setPen(QColor::black);
	p->drawPoint(xx+ww-2, yy+1);
	// bottom right corner:
	p->setPen(g.background());
	p->drawPoint(xx+ww-1, yy+hh-1);
	p->drawPoint(xx+ww-2, yy+hh-1);
	p->drawPoint(xx+ww-1, yy+hh-2);
	p->setPen(QColor::black);
	p->drawPoint(xx+ww-2, yy+hh-2);
	p->setPen(g.dark());
	p->drawPoint(xx+ww-3, yy+hh-3);
	p->setPen(g.mid());
	p->drawPoint(xx+ww-4, yy+hh-4);

	// and the arrows
	p->setPen( g.foreground() );
	QPointArray a;
	a.setPoints( 7, -3,1, 3,1, -2,0, 2,0, -1,-1, 1,-1, 0,-2 );
	a.translate( xx+ww/2, yy+hh/2-3 );
	p->drawLineSegments( a, 0, 3 );		// draw arrow
	p->drawPoint( a[6] );
	a.setPoints( 7, -3,-1, 3,-1, -2,0, 2,0, -1,1, 1,1, 0,2 );
	a.translate( xx+ww/2, yy+hh/2+2 );
	p->drawLineSegments( a, 0, 3 );		// draw arrow
	p->drawPoint( a[6] );

    }


    p->setPen( oldPen );

}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */
QRect QPlatinumStyle::comboButtonRect( int x, int y, int w, int h){
    return QRect(x+4, y+4, w-8-18, h-8);
}


int QPlatinumStyle::sliderLength() const
{
    return 17;
}

void QPlatinumStyle::drawSlider( QPainter *p,
				 int x, int y, int w, int h,
				 const QColorGroup &g,
				 SliderDirection dir)
{
    const QColor c0 = QColor::black;
    const QColor c1 = g.dark();
    //    const QColor c2 = g.button();
    const QColor c3 = g.light();

    int x1 = x;
    int x2 = x+w-1;
    int y1 = y;
    int y2 = y+h-1;
    int mx = w/2;


    QBrush oldBrush = p->brush();
    p->setBrush( g.fillButton() );
    p->setPen( NoPen );
    p->drawRect( x,y,w,h );
    p->setBrush( oldBrush );


    //### works only for Down ....

    switch ( dir ) {
	case SlUp:

	    break;
	case SlDown:
	    // black border
	    p->setPen( c0 );
	    p->drawLine(x1+1,y1,x2-1,y1);
	    p->drawLine(x1, y2-mx+2, x1+mx-2, y2);
	    p->drawLine(x2, y2-mx+2, x1+mx+2, y2);
	    p->drawLine(x1+mx-2, y2, x1+mx+2, y2);
	    p->drawLine(x1, y1+1, x1, y2-mx+2);
	    p->drawLine(x2, y1+1, x2, y2-mx+2);
	
	    // light shadow
	    p->setPen(c3);
	    p->drawLine(x1+1, y1+1,x2-1, y1+1);
	    p->drawLine(x1+1, y1+1, x1+1, y2-mx+2);
	
	    // dark shadow
	    p->setPen(c1);
	    p->drawLine(x2-1, y1+1, x2-1, y2-mx+2);
	    p->drawLine(x1+1, y2-mx+2, x1+mx-2, y2-1);
	    p->drawLine(x2-1, y2-mx+2, x1+mx+2, y2-1);
	    p->drawLine(x1+mx-2, y2-1, x1+mx+2, y2-1);

	    drawRiffles(p, x+2, y, w-4, h-5, g, TRUE);
	    break;
	case SlLeft:
	    break;
	case SlRight:
	    break;
    }

}


void QPlatinumStyle::drawSliderGroove( QPainter *p,
				      int x, int y, int w, int h,
				      const QColorGroup& g, QCOORD c,
				       bool /* horizontal */)

{

    p->fillRect(x, y, w, h, g.fillBackground());
    y = y+c-3;
    h = 7;
    p->fillRect(x, y, w, h, g.fillDark());

	 // the dark side
    p->setPen(g.dark());
    p->drawLine(x, y, x+w-1, y);
    p->drawLine(x, y, x, y+h-1);

    p->setPen(QColor::black);
    p->drawLine(x+1, y+1, x+w-2, y+1);
    p->drawLine(x+1, y+1, x+1, y+h-2);


	 // the bright side!

    p->setPen(QColor::black);
    p->drawLine(x+1, y+h-2 ,x+w-2, y+h-2);
    p->drawLine(x+w-2, y+1, x+w-2, y+h-2);

    p->setPen(g.light());
    p->drawLine(x, y+h-1,x+w-1, y+h-1);
    p->drawLine(x+w-1, y, x+w-1, y+h-1);

 	 // top left corner:
    p->setPen(g.background());
    p->drawPoint(x, y);
    p->drawPoint(x+1, y);
    p->drawPoint(x, y+1);
    p->setPen(QColor::black);
    p->drawPoint(x+1, y+1);
    // bottom left corner:
    p->setPen(g.background());
    p->drawPoint(x, y+h-1);
    p->drawPoint(x+1, y+h-1);
    p->drawPoint(x, y+h-2);
    p->setPen(g.light());
    p->drawPoint(x+1, y+h-2);
    // top right corner:
    p->setPen(g.background());
    p->drawPoint(x+w-1, y);
    p->drawPoint(x+w-2, y);
    p->drawPoint(x+w-1, y+1);
    p->setPen(g.dark());
    p->drawPoint(x+w-2, y+1);
    // bottom right corner:
    p->setPen(g.background());
    p->drawPoint(x+w-1, y+h-1);
    p->drawPoint(x+w-2, y+h-1);
    p->drawPoint(x+w-1, y+h-2);
    p->setPen(g.light());
    p->drawPoint(x+w-2, y+h-2);
    p->setPen(g.dark());
    p->drawPoint(x+w-3, y+h-3);

}

/*!
  Reimplementation from QWindowsStyle to disable the Windows typical jump
  back when dragging controls.

  \sa QStyle
  */
int QPlatinumStyle::maximumSliderDragDistance() const
{
    return -1;
}


