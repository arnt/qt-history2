/****************************************************************************
** $Id: //depot/qt/main/examples/themes/metal.cpp#5 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "metal.h"
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


/////////////////////////////////////////////////////////
#include "metal.xpm"
#include "stonedark.xpm"
#include "stone1.xpm"
#include "marble.xpm"
///////////////////////////////////////////////////////



MetalStyle::MetalStyle() : QWindowsStyle() { }

/*!
  Reimplementation from QStyle
 */
void MetalStyle::polish( QApplication *app)
{
    oldPalette = app->palette();

    // we simply create a nice QColorGroup with a couple of fancy
    // pixmaps here and apply to it all widgets

    QFont f("times", app->font().pointSize() );
    f.setBold( TRUE );
    f.setItalic( TRUE );
    app->setFont( f, TRUE, "QMenuBar");
    app->setFont( f, TRUE, "QPopupMenu");


    //QPixmap button( stone1_xpm );
    QPixmap button( stonedark_xpm );
    QPixmap background(marble_xpm);
#if 0

    int i;
    for (i=0; i<img.numColors(); i++) {
	QRgb rgb = img.color(i);
	QColor c(rgb);
	rgb = c.dark().rgb();
	img.setColor(i,rgb);
    }
    QPixmap mid;
    mid.convertFromImage(img);

    img = orig;
    for (i=0; i<img.numColors(); i++) {
	QRgb rgb = img.color(i);
	QColor c(rgb);
	rgb = c.light().rgb();
	img.setColor(i,rgb);
    }
    QPixmap light;
    light.convertFromImage(img);

    img = orig;
    for (i=0; i<img.numColors(); i++) {
	QRgb rgb = img.color(i);
	QColor c(rgb);
	rgb = c.dark().rgb();
	img.setColor(i,rgb);
    }
    QPixmap dark;
    dark.convertFromImage(img);
#else
    QPixmap dark( 1, 1 ); dark.fill( red.dark() );
    QPixmap mid( stone1_xpm );
    QPixmap light( stone1_xpm );//1, 1 ); light.fill( green );
#endif
    QPalette op = app->palette();

    QColor backCol( 227,227,227 );

    // QPalette op(white);
    QColorGroup active (op.active().foreground(),
		     QBrush(op.active().button(),button),
		     QBrush(op.active().light(), light),
		     QBrush(op.active().dark(), dark),
		     QBrush(op.active().mid(), mid),
		     op.active().text(),
		     Qt::white,
		     op.active().base(),//		     QColor(236,182,120),
		     QBrush(backCol, background)
		     );
    active.setColor( QColorGroup::ButtonText,  Qt::white  );
    active.setColor( QColorGroup::Shadow,  Qt::black  );
    QColorGroup disabled (op.disabled().foreground(),
		     QBrush(op.disabled().button(),button),
		     QBrush(op.disabled().light(), light),
		     op.disabled().dark(),
		     QBrush(op.disabled().mid(), mid),
		     op.disabled().text(),
		     Qt::white,
		     op.disabled().base(),//		     QColor(236,182,120),
		     QBrush(backCol, background)
		     );

    QPalette newPalette( active, disabled, active );
    app->setPalette( newPalette, TRUE );
}

/*!
  Reimplementation from QStyle
 */
void MetalStyle::unPolish( QApplication *app)
{
    app->setPalette(oldPalette, TRUE);
    app->setFont( app->font(), TRUE );
}

/*!
  Reimplementation from QStyle
 */
void MetalStyle::polish( QWidget* w)
{

   // the polish function sets some widgets to transparent mode and
    // some to translate background mode in order to get the full
    // benefit from the nice pixmaps in the color group.

    if (w->inherits("QPushButton")){
	w->setBackgroundMode( QWidget::NoBackground );
	return;
    }

    if ( !w->isTopLevel() ) {
	if ( w->backgroundPixmap() )
	    w->setBackgroundOrigin( QWidget::WindowOrigin );
    }
}

void MetalStyle::unPolish( QWidget* w)
{

   // the polish function sets some widgets to transparent mode and
    // some to translate background mode in order to get the full
    // benefit from the nice pixmaps in the color group.

    if (w->inherits("QPushButton")){
	w->setBackgroundMode( QWidget::PaletteButton );
	return;
    }
    if ( !w->isTopLevel() ) {
	if ( w->backgroundPixmap() )
	    w->setBackgroundOrigin( QWidget::WidgetOrigin );
    }

}

/*!
  Reimplementation from QStyle
 */
void MetalStyle::drawButton( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &, bool sunken, const QBrush*)
{

    static QImage *img1;
    if ( !img1 ) {
	img1 = new QImage(metal_xpm);
    }

    QImage scaledImage = img1->smoothScale( w, h );
    QPixmap pix;
    pix.convertFromImage( scaledImage );
    p->drawPixmap( x, y, pix );
    QColorGroup g2;
    g2.setColor( QColorGroup::Light,  white  );
    g2.setColor( QColorGroup::Dark,  black  );
    qDrawShadePanel( p, x, y, w, h, g2, sunken, sunken?2:1);
	

//    static QPixmap* darkpixmap = 0;
//    if (!pixmap) {
//	  pixmap = new QPixmap;
//	  pixmap->convertFromImage(img);
//	  for (int i=0; i<img.numColors(); i++) {
//	      QRgb rgb = img.color(i);
//	      QColor c(rgb);
//	      rgb = c.dark().rgb();
//	      img.setColor(i,rgb);
//	  }
//	  darkpixmap = new QPixmap;
//	  darkpixmap->convertFromImage(img);
//    }
//    if (!pixmap)
//	  return;
//    p->drawPixmap( x, y, *pixmap );

}

/*!
  Reimplementation from QStyle
 */
void MetalStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    MetalStyle::drawButton(p, x, y, w, h, g, sunken, fill);
}

/*!
  Reimplementation from QStyle
 */
void MetalStyle::drawPushButton( QPushButton* btn, QPainter *p)
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
	p->setPen( Qt::black );
	p->drawPolyline( a );
	x1 += 2;
	y1 += 2;
	x2 -= 2;
	y2 -= 2;
    }
	
    drawButton( p, x1, y1, x2-x1+1, y2-y1+1, g, btn->isOn() || btn->isDown(), &fill);
	

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
 */
void MetalStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
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
    if ( btn->isOn() || btn->isDown() ) {
	dx--;
	dy--;
    }
    if ( dx || dy )
	p->translate( dx, dy );

    x += 2;  y += 2;  w -= 4;  h -= 4;
    QColorGroup g = btn->colorGroup();
    drawItem( p, x, y, w, h,
	      AlignCenter|ShowPrefix,
	      g, btn->isEnabled(),
	      btn->pixmap(), btn->text(), -1,
	      (btn->isDown() || btn->isOn())?&btn->colorGroup().brightText():&btn->colorGroup().buttonText());

    if ( dx || dy )
	p->translate( -dx, -dy );
}


void MetalStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
			    const QColorGroup &g, bool sunken,
			    int lineWidth, const QBrush *fill )
{

    QStyle::drawPanel( p,  x,  y,  w,  h,
			    g, sunken,
			    lineWidth, fill );
}
