/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.cpp#1 $
**
** Implementation of QFrame widget class
**
** Author  : Haavard Nord
** Created : 950201
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qframe.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qframe.cpp#1 $";
#endif


#define lightColor white		/* humbug!!! */
#define darkColor  darkGray


QFrame::QFrame( QWidget *parent, const char *name ) : QWidget( parent, name )
{
    initMetaObject();
    fstyle = Box | Plain;			// set default frame style
    lwidth = 1;
    mwidth = 0;
    setForegroundColor( black );
    setBackgroundColor( lightGray );
}


void QFrame::setFrameStyle( int fs )
{
    fstyle = (short)fs;
    update();
}

void QFrame::setLineWidth( int lw )
{
    lwidth = lw;
}

void QFrame::setMidLineWidth( int mw )
{
    mwidth = mw;
}


void QFrame::paintEvent( QPaintEvent * )
{
    QPainter paint;
    paint.begin( this );
    drawFrame( &paint );
    drawContents( &paint );
    paint.end();
}


void QFrame::drawFrame( QPainter *p )
{
    QRect     r = clientRect();
    QPoint    p1, p2;
    QPainter *paint = p;
    int	      type  = fstyle & MType;
    int	      style = fstyle & MStyle;
    QColor    fgcol = foregroundColor();

    switch ( type ) {

        case Box:
	    switch ( style ) {
	        case Plain:
		    paint->drawShadeRect( r, fgcol, fgcol, lwidth );
		    break;
	        case Raised:
		    paint->drawShadeRect( r, lightColor, darkColor, lwidth );
		    break;
	        case Sunken:
		    paint->drawShadeRect( r, darkColor, lightColor, lwidth );
		    break;
	    }
	    break;

        case Panel:
	    switch ( style ) {
	        case Plain:
		    paint->drawShadePanel( r, fgcol, fgcol, lwidth );
		    break;
	        case Raised:
		    paint->drawShadePanel( r, lightColor, darkColor );
		    break;
	        case Sunken:
		    paint->drawShadePanel( r, darkColor, lightColor );
		    break;
	    }
	    break;

        case HLine:
        case VLine:
	    if ( type == HLine ) {
		p1 = QPoint( r.x(), r.height()/2 );
		p2 = QPoint( r.x()+r.width(), p1.y() );
	    }
	    else {
		p1 = QPoint( r.x()+r.width()/2, 0 );
		p2 = QPoint( p1.x(), r.height() );
	    }
	    switch ( style ) {
	        case Plain:
		    paint->drawShadeLine( p1, p2, fgcol, fgcol,
					  lwidth, fgcol, mwidth );
		    break;
	        case Raised:
		    paint->drawShadeLine( p1, p2, lightColor, darkColor,
					  lwidth, fgcol, mwidth );
		    break;
	        case Sunken:
		    paint->drawShadeLine( p1, p2, darkColor, lightColor,
					  lwidth, fgcol, mwidth );
		    break;
	    }
	    break;
    }
    if ( paint->pen().width() > 0 )			// restore pen width
	paint->pen().setWidth( 0 );
}


void QFrame::drawContents( QPainter * )
{
}
