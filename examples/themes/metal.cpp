/****************************************************************************
** $Id: $
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


void MetalStyle::drawPrimitive( PrimitiveElement pe, QPainter *p,
				const QRect &r, const QColorGroup &cg,
				SFlags flags ) const
{
    switch ( pe ) {
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_Panel:
    case PE_HeaderSection:
	{
	    static QImage *img1;
	    if ( !img1 ) {
		img1 = new QImage( metal_xpm );
	    }
	
	    QImage scaledImage = img1->smoothScale( r.width(), r.height() );
	    QPixmap pix;
	    pix.convertFromImage( scaledImage );
	    p->drawPixmap( r.x(), r.y(), pix );
	    QColorGroup g2;
	    g2.setColor( QColorGroup::Light, white );
	    g2.setColor( QColorGroup::Dark, black );
	    if ( flags &
		 (Style_Raised | Style_Down | Style_On | Style_Sunken ) ) {
		qDrawShadePanel( p, r, g2,
				 flags & (Style_Sunken | Style_On | Style_Down),
				 flags & (Style_Sunken | Style_On | Style_Down)?2:1);
	    } else  // flat buttons...
		p->fillRect( r, g2.brush(QColorGroup::Button) );
	    break;
	}
    default:
	QWindowsStyle::drawPrimitive( pe, p, r, cg, flags );
	break;
    }
}

void MetalStyle::drawControl( ControlElement ce, QPainter *p,
			      const QWidget *widget, const QRect &r,
			      const QColorGroup &cg, SFlags how,
			      void **data ) const
{
    switch( ce ) {
    case CE_PushButton:
	{
	    QColorGroup cg2( cg );
	    const QPushButton *btn;
	    btn = (const QPushButton *)widget;

	
	    int x1, y1, x2, y2;
	    r.coords( &x1, &y1, &x2, &y2 );
	
	    p->setPen( cg.foreground() );
	    p->setBrush( QBrush(cg.button(), NoBrush) );
	
	    QBrush fill;
	    SFlags flags = Style_Default;
	
	    if ( btn->isDown() ) {
		fill = cg2.brush( QColorGroup::Mid );
		flags |= Style_Down;
	    } else if ( btn->isOn() ) {
		fill = QBrush( cg2.mid(), Dense4Pattern );
		flags |= Style_On;
	    } else
		fill = cg2.brush( QColorGroup::Button );
	
	    if ( ! btn->isFlat() && !(flags & Style_Down) )
		flags |= Style_Raised;
	
	    cg2.setBrush( QColorGroup::Button, fill );
	
	    if ( btn->isDefault() ) {
		flags |= Style_Default;
		QPointArray a;
		a.setPoints( 0,
			     x1, y1, x2, y1, x2, y2, x1, y2, x1, y1 + 1,
			     x2 - 1, y1 + 1, x2 - 1, y2 - 1, x1 + 1,
			     y2 - 1, x1 + 1, y1 + 1 );
		p->setPen( Qt::black );
		p->drawPolyline( a );
		x1 += 2;
		y1 += 2;
		x2 -= 2;
		y2 -= 2;
	    }
	
	    drawPrimitive( PE_ButtonCommand, p, QRect(x1, y1, x2 - x1 + 1,
						      y2 - y1 + 1),
			   cg2,	flags );
	    if ( btn->isMenuButton() ) {
		int dx = ( y1 - y2 - 4 ) / 3;
		if ( btn->isEnabled() )
		    flags |= Style_Enabled;
		drawPrimitive( PE_ArrowDown, p, QRect( x2 - dx, dx, y1, y2 - y1),
			       cg, flags );
	    }
	
	    if ( p->brush().style() != NoBrush )
		p->setBrush( NoBrush );
	    break;
	}
    case CE_PushButtonLabel:
	{
	    int x, y, w, h,
	        x1, y1, x2, y2,
		dx = 0,
		dy = 0;
	
	    const QPushButton *btn;
	    btn = (const QPushButton*)widget;

	    r.rect( &x, &y, &w, &h );
	    r.coords( &x1, &y1, &x2, &y2 );
	    if( btn->isMenuButton() )
		dx = ( y2 - y1 ) / 3;
	    if ( btn->isOn() || btn->isDown() ) {
		dx--;
		dy--;
	    }
	    if ( dx || dy )
		p->translate( dx, dy );
	
	    x += 2;
	    y += 2;
	    w -= 4;
	    h -= 4;
	    drawItem( p, QRect(x, y, w, h), AlignCenter|ShowPrefix, cg,
		      btn->isEnabled(), btn->pixmap(), btn->text(), -1,
		      (btn->isDown() || btn->isOn()) ? &cg.brightText() :
		      &cg.buttonText() );
	    break;
	}
    default:
	QWindowsStyle::drawControl( ce, p, widget, r, cg, how, data );
	break;
    }
}
