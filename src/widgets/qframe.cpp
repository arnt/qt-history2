/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.cpp#4 $
**
** Implementation of QFrame widget class
**
** Author  : Haavard Nord
** Created : 950201
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qframe.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qframe.cpp#4 $";
#endif


#define lightColor  white			// humbug!!!
#define darkColor   darkGray
#define midColor    gray


QFrame::QFrame( QWidget *parent, const char *name ) : QWidget( parent, name )
{
    initMetaObject();
    frect  = QRect( 0, 0, 0, 0 );
    fstyle = NoFrame;				// set default frame style
    fwidth = 1;
    mwidth = 0;
    setForegroundColor( black );
    setBackgroundColor( lightGray );
    if ( !frect.isNull() )
	debug("OIOIOI");
}


QRect QFrame::contentsRect() const
{
    QRect r = frameRect();
    int tw = fwidth+mwidth;
    r.setRect( r.x()+tw, r.y()+tw, r.width()-tw*2, r.height()-tw*2 );
    return r;
}


void QFrame::setFrame( int f )
{
    fstyle = (short)f;
}

void QFrame::setFrameWidth( int fw )
{
    fwidth = fw;
}

void QFrame::setMidLineWidth( int mw )
{
    mwidth = mw;
}


void QFrame::setFrameRect( const QRect &r )
{
    frect = r;
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
    QRect     r = frameRect();
    QPoint    p1, p2;
    QPainter *paint = p;
    int	      type  = fstyle & MType;
    int	      style = fstyle & MStyle;
    QColor    fgcol = foregroundColor();

    switch ( type ) {

	case Box:
	    switch ( style ) {
		case Plain:
		    paint->drawShadeRect( r, fgcol, fgcol, fwidth, 
					  fgcol, mwidth );
		    break;
		case Raised:
		    paint->drawShadeRect( r, lightColor, darkColor, fwidth,
					  midColor, mwidth );
		    break;
		case Sunken:
		    paint->drawShadeRect( r, darkColor, lightColor, fwidth,
					  midColor, mwidth );
		    break;
	    }
	    break;

	case Panel:
	    switch ( style ) {
		case Plain:
		    paint->drawShadePanel( r, fgcol, fgcol, fwidth );
		    break;
		case Raised:
		    paint->drawShadePanel( r, lightColor, darkColor, fwidth );
		    break;
		case Sunken:
		    paint->drawShadePanel( r, darkColor, lightColor, fwidth );
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
					  fwidth, fgcol, mwidth );
		    break;
		case Raised:
		    paint->drawShadeLine( p1, p2, lightColor, darkColor,
					  fwidth, midColor, mwidth );
		    break;
		case Sunken:
		    paint->drawShadeLine( p1, p2, darkColor, lightColor,
					  fwidth, midColor, mwidth );
		    break;
	    }
	    break;
    }
}


void QFrame::drawContents( QPainter * )
{
}
