/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.cpp#16 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qframe.cpp#16 $";
#endif


/*!
\class QFrame qframe.h
\brief The QFrame class is the base class of widgets that have an (optional)
frame.

The QLabel and QGroupBox widgets are examples of widgets that inherit
QFrame to allow frames around these widgets.
The QFrame class can also be used directly for creating simple frames
without any contents.

A frame widget has a frame style, a line rame width and a mid-line
width.

The frame style is specified by a frame shape and a shadow style.
The frame shapes are \c NoFrame, \c Box, \c Panel, \c HLine and
\c VLine.  Notice that the two latter ones specify lines, not rectangles.
The shadow styles are \c Plain, \c Raised and \c Sunken. See setFrameStyle()
for a description of frame types and frame styles.

The line width is the width of the frame border.

The mid-line width specifies the width of an extra line in the middle of
the border, that uses a third color to obtain a special 3D effect.
Notice that a mid-line will only be draw for \c Box, \c HLine and
\c VLine frames that are raised or sunken.

Example of use:
\code
  QFrame *f = new QFrame;
  f->setFrameStyle( QFrame::Panel | QFrame::Sunken );
\endcode
*/


/*!
Constructs a frame widget with frame style \c NoFrame and 1 pixel frame width.

The \e allowLines argument can be set to FALSE to disallow \c HLine and
\c VLine styles.

The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QFrame::QFrame( QWidget *parent, const char *name, WFlags f,
		bool allowLines )
    : QWidget( parent, name, f )
{
    initMetaObject();
    frect  = QRect( 0, 0, 0, 0 );
    fstyle = NoFrame;				// set default frame style
    lwidth = 1;
    mwidth = 0;
    lineok = (short)allowLines;
    updateFrameWidth();
}


/*!
\fn int QFrame::frameStyle() const
Returns the frame style.

The default value is QFrame::NoFrame.
\sa setFrameStyle().
*/

/*!
Sets the frame style to \e style.

The \e style is the bitwise OR between a frame geometry style and a frame
shadow style.

The frame shapes are:
<ul>
<li> \c NoFrame draws nothing.
<li> \c Box draws a rectangular box.
<li> \c Panel draws a rectangular panel that can be raised or sunken.
<li> \c HLine draws a horizontal line (vertically centered).
<li> \c VLine draws a vertical line (horizontally centered).
</ul>

The shadow styles are:
<ul>
<li> \c Plain draws using the palette foreground color.
<li> \c Raised draws a 3D raised line using the palette light and dark colors.
<li> \c Sunken draws a 3D sunken line using the palette light and dark colors.
</ul>

\c Raised and \c Sunken will draw an additional middle line for \c Panel,
\c HLine and \c VLine if a mid-line width greater than 0 is specified.
The palette mid color is used for drawing middle lines.

\warning Trying to set a \c HLine or \c VLine will have no effect if line
shapes are disallowed.  Line shapes are allowed by default.

\sa frameStyle(), lineShapesOk(), QPalette */

void QFrame::setFrameStyle( int style )
{
    if ( !lineShapesOk() ) {
	int t = style & QFrame::MShape;
	if ( t == QFrame::HLine || t == QFrame::VLine )
	    return;
    }
    fstyle = (short)style;
    updateFrameWidth();
}

/*!
\fn bool QFrame::lineShapesOk() const
Returns TRUE if line shapes (\c HLine or \c VLine) are allowed, or FALSE if
they are not allowed.

It is only possible to disallow line shapes in the constructor.
The default value is TRUE.
*/

/*!
\fn int QFrame::lineWidth() const
Returns the line width.

The default value is 1.
\sa setLineWidth(), frameWidth()
*/

/*!
Sets the line width to \e fw.
\sa lineWidth(), frameWidth()
*/

void QFrame::setLineWidth( int fw )
{
    lwidth = (short)fw;
    updateFrameWidth();
}

/*!
\fn int QFrame::midLineWidth() const
Returns the width of the mid-line.

The default value is 0.
\sa setMidLineWidth(), frameWidth()
*/

/*!
Sets the width of the mid-line to \e mw.
\sa midLineWidth(), frameWidth()
*/

void QFrame::setMidLineWidth( int mw )
{
    mwidth = (short)mw;
    updateFrameWidth();
}


void QFrame::updateFrameWidth()
{
    int type  = fstyle & MShape;
    int style = fstyle & MShadow;

    fwidth = -1;

    switch ( type ) {

	case NoFrame:
	    fwidth = 0;
	    break;

	case Box:
	    switch ( style ) {
		case Plain:
		    fwidth = lwidth;
		    break;
		case Raised:
		case Sunken:
		    fwidth = (short)(lwidth*2 + mwidth);
		    break;
	    }
	    break;

	case Panel:
	    switch ( style ) {
		case Plain:
		case Raised:
		case Sunken:
		    fwidth = lwidth;
		    break;
	    }
	    break;

	case HLine:
	case VLine:
	    switch ( style ) {
		case Plain:
		    fwidth = lwidth;
		    break;
		case Raised:
		case Sunken:
		    fwidth = (short)(lwidth*2 + mwidth);
		    break;
	    }
	    break;
    }

    if ( fwidth == -1 ) {			// invalid style?
	fwidth = 0;
#if defined(CHECK_RANGE)
	warning( "QFrame::updateFrameWidth: Internal error" );
#endif
    }
    frameChanged();
}


/*!
\fn int QFrame::frameWidth() const
Returns the width of the frame that will be drawn.

Notice that the frame width depends on the \link QFrame::setFrameStyle frame
style \endlink, not only the line width and the mid line width.  For
example, the style \c NoFrame will have a frame width 0, while the
style \c Panel will have a frame width equivalent to the line width.

\sa lineWidth(), midLineWidth(), frameStyle() */


/*!
Returns the frame rectangle.

The default frame rectangle is equivalent with the
\link QWidget::rect widget rectangle\endlink.
\sa setFrameRect()
*/

QRect QFrame::frameRect() const
{
    return frect.isNull() ? rect() : frect;
}


/*!
Sets the frame rectangle to \e r.

If \e r is a null rectangle (for example <code>QRect(0,0,0,0)</code>),
then the frame rectangle will be equivalent with the
\link QWidget::rect widget rectangle\endlink.

\sa frameRect().
*/

void QFrame::setFrameRect( const QRect &r )
{
    frect = r;
}


/*!
Returns the rectangle inside the frame.
\sa frameRect().
*/

QRect QFrame::contentsRect() const
{
    QRect r = frameRect();
    int	  w = frameWidth();			// total width
    r.setRect( r.x()+w, r.y()+w, r.width()-w*2, r.height()-w*2 );
    return r;
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
Adjusts the frame rectangle for the resized widget.
Nothing is done if the frame rectangle is a
\link QRect::isNull null rectangle\endlink.
*/

void QFrame::resizeEvent( QResizeEvent *e )
{
    if ( !frect.isNull() ) {
	QRect r( frect.x(), frect.y(),
		 width()  - (e->oldSize().width()  - frect.width()),
		 height() - (e->oldSize().height() - frect.height()) );
	setFrameRect( r );
    }
}


/*!
Internal function that draws the frame.
*/

void QFrame::drawFrame( QPainter *p )
{
    QPoint	p1, p2;
    QRect	r     = frameRect();
    QPainter   *paint = p;
    int		type  = fstyle & MShape;
    int		style = fstyle & MShadow;
    QColorGroup g     = colorGroup();
    QColor	fg    = g.foreground();
    QColor	light = g.light();
    QColor	dark  = g.dark();
    QColor	mid   = g.mid();

    switch ( type ) {

	case Box:
	    switch ( style ) {
		case Plain:
		    paint->drawShadePanel( r, fg, fg, lwidth );
		    break;
		case Raised:
		    paint->drawShadeRect( r, light, dark, lwidth, mid, mwidth);
		    break;
		case Sunken:
		    paint->drawShadeRect( r, dark, light, lwidth, mid, mwidth);
		    break;
	    }
	    break;

	case Panel:
	    switch ( style ) {
		case Plain:
		    paint->drawShadePanel( r, fg, fg, lwidth );
		    break;
		case Raised:
		    paint->drawShadePanel( r, light, dark, lwidth );
		    break;
		case Sunken:
		    paint->drawShadePanel( r, dark, light, lwidth );
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
		case Plain: {
		    QPen oldPen = paint->pen();
		    paint->setPen( QPen(fg,lwidth) );
		    paint->drawLine( p1, p2 );
		    paint->setPen( oldPen );
		    }
		    break;
		case Raised:
		    paint->drawShadeLine( p1, p2, light, dark,
					  lwidth, mid, mwidth );
		    break;
		case Sunken:
		    paint->drawShadeLine( p1, p2, dark, light,
					  lwidth, mid, mwidth );
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

void QFrame::drawContents( QPainter * )
{
}


/*!
Virtual function that is called when the frame style, line width or
mid-line width is changed.

This function is reimplemented by subclasses that need to know when
the frame attributes change.
*/

void QFrame::frameChanged()
{
}
