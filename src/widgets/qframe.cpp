/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.cpp#10 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qframe.cpp#10 $";
#endif


/*!
\class QFrame qframe.h
\brief The QFrame class is the base class of widgets that have an (optional)
frame.

The QLabel and QGroupBox widgets are examples of widgets that inherit
QFrame to allow frames around these widgets.
The QFrame class can also be used directly for creating simple frames
without any contents.

A frame widget has a frame type, a frame style a frame width and a mid-line
width.

The different frame types are \c NoFrame, \c Box, \c Panel, \c HLine and
\c VLine.  Notice that the two latter ones specify lines, not rectangles.

The frame styles are \c Plain, \c Raised and \c Sunken.  See setFrame()
for a description of frame types and frame styles.

The frame width is the width of the frame border.

The mid-line width specifies the width of an extra line in the middle of
the border, that uses a third color to get a special 3D effect.

Example of use:
\code
  QFrame *f = new QFrame;
  f->setFrame( QFrame::Panel | QFrame::Sunken );
\endcode
*/


/*!
Constructs a frame widget with frame style \c NoFrame and 1 pixel frame width.

The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QFrame::QFrame( QWidget *parent, const char *name ) : QWidget( parent, name )
{
    initMetaObject();
    frect  = QRect( 0, 0, 0, 0 );
    fstyle = NoFrame;				// set default frame style
    fwidth = 1;
    mwidth = 0;
}

/*!
Returns the geometry of the rectangle inside the frame.

\sa frameRect().
*/

QRect QFrame::contentsRect() const
{
    QRect r = frameRect();
    int tw = fwidth+mwidth;			// total width
    r.setRect( r.x()+tw, r.y()+tw, r.width()-tw*2, r.height()-tw*2 );
    return r;
}


/*!
\fn int QFrame::frame() const
Returns the frame type and style.
\sa setFrame().
*/

/*!
Sets the frame type and style to \e f.

The \e f is the bitwise OR between a frame type and a style.

The frame types are:
<ul>
<li> \c NoFrame draws nothing.
<li> \c Box draws a rectangular box.
<li> \c Panel draws a rectangular panel that can be raised or sunken.
<li> \c HLine draws a horizontal line (vertically centered).
<li> \c VLine draws a vertical line (horizontally centered).
</ul>

The frame styles are:
<ul>
<li> \c Plain draws using the palette foreground color.
<li> \c Raised draws a 3D raised line using the palette light and dark colors.
<li> \c Sunken draws a 3D sunken line using the palette light and dark colors.
</ul>

\c Raised and \c Sunken will draw an additional middle line if a mid-line
width greater than 0 was specified.

\sa frame(), QPalette.
*/

void QFrame::setFrame( int f )
{
    fstyle = (short)f;
}

/*!
\fn int QFrame::frameWidth() const
Returns the frame width.
\sa setFrameWidth().
*/

/*!
Sets the frame width to \e fw.

\sa frameWidth()
*/

void QFrame::setFrameWidth( int fw )
{
    fwidth = fw;
}

/*!
\fn int QFrame::midLineWidth() const
Returns the width of the middle line.
\sa setMidLineWidth()
*/

/*!
Sets the width of the middle line to \e mw.

\sa midLineWidth().
*/

void QFrame::setMidLineWidth( int mw )
{
    mwidth = mw;
}


/*!
Returns the frame rectangle.

\sa setFrameRect()
*/

QRect QFrame::frameRect() const
{
    return frect.isNull() ? rect() : frect;
}


/*!
Sets the frame rectangle to \e r. 

If \e r is a null rectangle (for example <code>QRect(0,0,0,0)</code>),
then the frame rectangle follows the widget rectangle (QWidget::rect()).

\sa frameRect().
*/

void QFrame::setFrameRect( const QRect &r )
{
    frect = r;
}


/*!
Paints the frame.

Opens the painter on the frame and calls first drawFrame(), then
drawContents().
*/

void QFrame::paintEvent( QPaintEvent * )
{
    QPainter paint;
    paint.begin( this );
    drawFrame( &paint );
    drawContents( &paint );
    paint.end();
}


/*!
Internal function that draws the frame.
*/

void QFrame::drawFrame( QPainter *p )
{
    QPoint      p1, p2;
    QRect       r     = frameRect();
    QPainter   *paint = p;
    int	        type  = fstyle & MType;
    int	        style = fstyle & MStyle;
    QColorGroup g     = colorGroup();
    QColor      fg    = g.foreground();
    QColor	light = g.light();
    QColor	dark  = g.dark();
    QColor	mid   = g.mid();

    switch ( type ) {

	case Box:
	    switch ( style ) {
		case Plain:
		    paint->drawShadePanel( r, fg, fg, fwidth );
		    break;
		case Raised:
		    paint->drawShadeRect( r, light, dark, fwidth, mid, mwidth);
		    break;
		case Sunken:
		    paint->drawShadeRect( r, dark, light, fwidth, mid, mwidth);
		    break;
	    }
	    break;

	case Panel:
	    switch ( style ) {
		case Plain:
		    paint->drawShadePanel( r, fg, fg, fwidth );
		    break;
		case Raised:
		    paint->drawShadePanel( r, light, dark, fwidth );
		    break;
		case Sunken:
		    paint->drawShadePanel( r, dark, light, fwidth );
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
		    paint->drawShadeLine( p1, p2, fg, fg, fwidth, fg, mwidth );
		    break;
		case Raised:
		    paint->drawShadeLine( p1, p2, light, dark,
					  fwidth, mid, mwidth );
		    break;
		case Sunken:
		    paint->drawShadeLine( p1, p2, dark, light,
					  fwidth, mid, mwidth );
		    break;
	    }
	    break;
    }
}


/*!
Virtual function that draws the contents of the frame.

This function is reimplemented by subclasses that want to draw something
inside the frame.
*/

void QFrame::drawContents( QPainter *p )
{
    p = p;
}
