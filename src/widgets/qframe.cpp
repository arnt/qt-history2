/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.cpp#88 $
**
** Implementation of QFrame widget class
**
** Created : 950201
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qframe.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qframe.h"
#include "qbitmap.h"

/*!
  \class QFrame qframe.h
  \brief The QFrame class is the base class of widgets that (can) have a frame.

  \ingroup abstractwidgets
  \ingroup realwidgets

  It draws a frame and calls a virtual function, drawContents(), to
  fill in the frame.  This function is reimplemented by essentially
  all subclasses.  There are also two other less useful functions,
  drawFrame() and frameChanged().

  QMenuBar uses this to "raise" the menu bar above the surrounding
  screen:

  \code
    if ( style() == MotifStyle ) {
	setFrameStyle( QFrame::Panel | QFrame::Raised );
	setLineWidth( 2 );
    } else {
	setFrameStyle( QFrame::NoFrame );
    }
  \endcode

  The QFrame class can also be used directly for creating simple frames
  without any contents, for example like this:

  \code
    QFrame *emptyFrame = new QFrame( parentWidget );
    emptyFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    emptyFrame->setLineWidth( 2 );
  \endcode

  A frame widget has three attributes: \link setFrameStyle() frame
  style\endlink, a \link setLineWidth() line width\endlink and a \link
  setMidLineWidth() mid-line width\endlink.

  The frame style is specified by a frame shape and a shadow style.
  The frame shapes are \c NoFrame, \c Box, \c Panel, \c StyledPanel \c
  WinPanel, \c HLine and \c VLine, and the shadow styles are \c Plain,
  \c Raised and \c Sunken.

  The line width is the width of the frame border.

  The mid-line width specifies the width of an extra line in the
  middle of the frame, that uses a third color to obtain a special 3D
  effect.  Notice that a mid-line is only drawn for \c Box, \c HLine
  and \c VLine frames that are raised or sunken.

  <a name=picture></a>
  This table shows the most useful combinations of styles and widths
  (and some rather useless ones):

  <img src=frames.gif height=422 width=520 alt="Table of frame styles">

  For obvious reasons, \c NoFrame isn't shown.  The gray areas next to
  the \c VLine and \c HLine examples are there because the widgets in
  the illustration are taller/wider than the natural width of the
  lines.  frameWidth() returns the natural width of the line.

  The labels on the top and right are QLabel objects with frameStyle()
  \c Raised|Panel and lineWidth() 1.
*/


/*!
  Constructs a frame widget with frame style \c NoFrame and a 1 pixel frame
  width.

  The last argument exists for compatibility with Qt 1.x; it
  no longer has any meaning.  (In Qt 2.x, QFrame always allows the
  HLine and VLine styles.)

  The \e parent, \e name and \e f arguments are passed to the QWidget
  constructor.
*/

QFrame::QFrame( QWidget *parent, const char *name, WFlags f,
		bool )
    : QWidget( parent, name, f )
{
    frect  = QRect( 0, 0, 0, 0 );
    fstyle = NoFrame;
    lwidth = 1;
    mwidth = 0;
    mlwidth = 0;
    d = 0;
    updateFrameWidth();
}


/*!
  \fn int QFrame::frameStyle() const
  Returns the frame style.

  The default value is QFrame::NoFrame.

  \sa setFrameStyle(), frameShape(), frameShadow()
*/

/*!
  \fn Shape QFrame::frameShape() const
  Returns the frame shape value from the frame style.
  \sa setFrameShape(), frameStyle(), frameShadow()
*/

/*!
  \fn Shape QFrame::setFrameShape() const
  Sets the frame shape value of the frame style.
  \sa frameShape(), frameStyle(), setFrameShadow()
*/

/*!
  \fn Shadow QFrame::frameShadow() const
  Returns the frame shadow value from the frame style.
  \sa setFrameShadow(), frameStyle(), frameShape()
*/

/*!
  \fn void QFrame::setFrameShadow( Shadow ) const
  Sets the frame shadow value of the frame style.
  \sa frameShadow(), frameStyle(), setFrameShape()
*/

/*!
  Sets the frame style to \e style.

  The \e style is the bitwise OR between a frame shape and a frame
  shadow style.  See the <a href="#picture">illustration</a> in the
  class documentation.

  The frame shapes are:
  <ul>
  <li> \c NoFrame draws nothing. Naturally, you should not specify a shadow
  style if you use this.
  <li> \c Box draws a rectangular box.  The contents appear to be
  level with the surrounding screen, but the border itself may be
  raised or sunken.
  <li> \c Panel draws a rectangular panel that can be raised or sunken.
  <li> \c StyledPanel draws a rectangular panel with a look depending on
  the current GUI style.  It can be raised or sunken.
  <li> \c WinPanel draws a rectangular panel that can be raised or
  sunken, very like those in Windows 95.  Specifying this shape sets
  the line width to 2 pixels.  WinPanel is provided for compatibility.
  For GUI style independence we recommend using StyledPanel with
  setLineWidth(2) instead.
  <li> \c HLine draws a horizontal line (vertically centered).
  <li> \c VLine draws a vertical line (horizontally centered).
  </ul>

  The shadow styles are:
  <ul>
  <li> \c Plain draws using the palette foreground color (without any
  3D effect).
  <li> \c Raised draws a 3D raised line using the light and dark
  colors of the current color group.
  <li> \c Sunken draws a 3D sunken line using the light and dark
  colors of the current color group.
  </ul>

  If a mid-line width greater than 0 is specified, an additional line
  is drawn for \c Raised or \c Sunken \c Box, \c HLine and \c VLine
  frames.  The mid color of the current color group is used for
  drawing middle lines.

  \sa <a href="#picture">Illustration</a>, frameStyle(), 
  colorGroup(), QColorGroup
*/

void QFrame::setFrameStyle( int style )
{
#if defined(CHECK_RANGE)
    bool shape	= (style & MShape)  != 0;
    bool shadow = (style & MShadow) != 0;
    if ( shape != shadow )
	qWarning( "QFrame::setFrameStyle: (%s) Incomplete frame style",
		 name( "unnamed" ) );
#endif
    fstyle = (short)style;
    updateFrameWidth();
}

/*!
  \fn int QFrame::lineWidth() const
  Returns the line width.  (Note that the \e total line width
  for \c HLine and \c VLine is given by frameWidth(), not
  lineWidth().)

  The default value is 1.

  \sa setLineWidth(), midLineWidth(), frameWidth()
*/


/*!
  Sets the line width to \e w.
  \sa frameWidth(), lineWidth(), setMidLineWidth()
*/

void QFrame::setLineWidth( int w )
{
    lwidth = (short)w;
    updateFrameWidth();
}

/*!
  \fn int QFrame::midLineWidth() const
  Returns the width of the mid-line.

  The default value is 0.

  \sa setMidLineWidth(), lineWidth(), frameWidth()
*/

/*!
  Sets the width of the mid-line to \e w.
  \sa midLineWidth(), setLineWidth()
*/

void QFrame::setMidLineWidth( int w )
{
    mlwidth = (short)w;
    updateFrameWidth();
}



/*!
  \fn int QFrame::margin() const
  Returns the width of the margin. The margin is the distance between the
  innermost pixel of the frame and the outermost pixel of contentsRect().
  It is included in frameWidth().

  The margin is filled according to backgroundMode().

  The default value is 0.

  \sa setMargin(), lineWidth(), frameWidth()
*/

/*!
  Sets the width of the margin to \e w.
  \sa margin(), setLineWidth()
*/

void QFrame::setMargin( int w )
{
    mwidth = (short)w;
    updateFrameWidth();
}


/*!
  \internal
  Updated the fwidth parameter.
*/

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
	    fwidth = (short)(lwidth*2 + midLineWidth() );
	    break;
	}
	break;

    case Panel:
    case StyledPanel:
	switch ( style ) {
	case Plain:
	case Raised:
	case Sunken:
	    fwidth = lwidth;
	    break;
	}
	break;

    case WinPanel:
	switch ( style ) {
	case Plain:
	case Raised:
	case Sunken:
	    fwidth = lwidth = 2;
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
	    fwidth = (short)(lwidth*2 + midLineWidth());
	    break;
	}
	break;
    }

    if ( fwidth == -1 )				// invalid style
	fwidth = 0;

    fwidth += margin();

    frameChanged();
}


/*!
  \fn int QFrame::frameWidth() const
  Returns the width of the frame that is drawn.

  Note that the frame width depends on the \link setFrameStyle() frame
  style \endlink, not only the line width and the mid line width.  For
  example, the style \c NoFrame always has a frame width 0, while the
  style \c Panel has a frame width equivalent to the line width.
  The frame width also includes the margin.

  \sa lineWidth(), midLineWidth(), frameStyle(), margin()
*/


/*!
  Returns the frame rectangle.

  The default frame rectangle is equivalent to the \link
  QWidget::rect() widget rectangle\endlink.

  \sa setFrameRect()
*/

QRect QFrame::frameRect() const
{
    if ( frect.isNull() )
	return rect();
    else
	return frect;
}


/*!
  Sets the frame rectangle to \e r.

  The frame rectangle is the rectangle the frame is drawn in.  By
  default, this is the entire widget.  Calling setFrameRect() does \e
  not cause a widget update.

  If \e r is a null rectangle (for example
  <code>QRect(0,0,0,0)</code>), then the frame rectangle is equivalent
  to the \link QWidget::rect() widget rectangle\endlink.

  \sa frameRect(), contentsRect()
*/

void QFrame::setFrameRect( const QRect &r )
{
    frect = r.isValid() ? r : rect();
}


/*!
  Returns the rectangle inside the frame.
  \sa frameRect(), drawContents()
*/

QRect QFrame::contentsRect() const
{
    QRect r = frameRect();
    int	  w = frameWidth();			// total width
    r.setRect( r.x()+w, r.y()+w, r.width()-w*2, r.height()-w*2 );
    return r;
}

QSize QFrame::sizeHint() const
{
    switch (fstyle & MShape) {
    case HLine:
	return QSize(-1,3);
    case VLine:
	return QSize(3,-1);
    default:
	return QWidget::sizeHint();
    }
}



/*!
  If this is a  line, it may stretch in the direction of the line, but it is
  fixed in the other direction. If this is a normal frame, use QWidget's
  default behavior.
*/

QSizePolicy QFrame::sizePolicy() const
{
    switch (fstyle & MShape) {
    case HLine:
	return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
    case VLine:
	return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
    default:
	return QWidget::sizePolicy();
    }


}


/*!
  Handles paint events for the frame.

  Paints the frame and the contents.

  Opens the painter on the frame and calls first drawFrame(), then
  drawContents().
*/

void QFrame::paintEvent( QPaintEvent *event )
{
    QPainter paint( this );

    if ( !contentsRect().contains( event->rect() ) ) {
	paint.save();
	paint.setClipRegion( event->region().intersect(frameRect()) );
	drawFrame( &paint );
	paint.restore();
    }
    if ( event->rect().intersects( contentsRect() ) &&
	 (fstyle & MShape) != HLine && (fstyle & MShape) != VLine ) {
	paint.setClipRegion( event->region().intersect( contentsRect() ) );
	drawContents( &paint );
    }
}


/*!
  Handles resize events for the frame.

  Adjusts the frame rectangle for the resized widget.  The frame
  rectangle is elastic, the surrounding area is static.

  The resulting frame rectangle may be null or invalid.  You can use
  setMinimumSize() to avoid that possibility.

  Nothing is done if the frame rectangle is a \link QRect::isNull()
  null rectangle\endlink already.
*/

void QFrame::resizeEvent( QResizeEvent *e )
{
    if ( !frect.isNull() ) {
	QRect r( frect.x(), frect.y(),
		 width()  - (e->oldSize().width()  - frect.width()),
		 height() - (e->oldSize().height() - frect.height()) );
	setFrameRect( r );
    }

    if ( autoMask())
	updateMask();
}


/*!
  Draws the frame using the current frame attributes and color
  group.  The rectangle inside the frame is not affected.

  This function is virtual, but in general you do not need to
  reimplement it.  If you do, note that the QPainter is already open
  and must remain open.

  \sa frameRect(), contentsRect(), drawContents(), frameStyle(), setPalette(), drawFrameMask()
*/

void QFrame::drawFrame( QPainter *p )
{
    QPoint	p1, p2;
    QRect	r     = frameRect();
    int		type  = fstyle & MShape;
    int		cstyle = fstyle & MShadow;
    const QColorGroup & g = colorGroup();

    switch ( type ) {

    case Box:
	if ( cstyle == Plain )
	    qDrawPlainRect( p, r, g.foreground(), lwidth );
	else
	    qDrawShadeRect( p, r, g, cstyle == Sunken, lwidth,
			    midLineWidth() );
	break;

    case Panel:
	if ( cstyle == Plain )
	    qDrawPlainRect( p, r, g.foreground(), lwidth );
	else
	    qDrawShadePanel( p, r, g, cstyle == Sunken, lwidth );
	break;

    case StyledPanel:
	if ( cstyle == Plain )
	    qDrawPlainRect( p, r, g.foreground(), lwidth );
	else
	    style().drawPanel( p, r.x(), r.y(), r.width(), r.height(), g, cstyle == Sunken, lwidth );
	break;

    case WinPanel:
	if ( cstyle == Plain )
	    qDrawPlainRect( p, r, g.foreground(), lwidth );
	else
	    qDrawWinPanel( p, r, g, cstyle == Sunken );
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
	if ( cstyle == Plain ) {
	    QPen oldPen = p->pen();
	    p->setPen( QPen(g.foreground(),lwidth) );
	    p->drawLine( p1, p2 );
	    p->setPen( oldPen );
	}
	else
	    qDrawShadeLine( p, p1, p2, g, cstyle == Sunken,
			    lwidth, midLineWidth() );
	break;
    }
}


/*!
  Virtual function that draws the contents of the frame.

  The QPainter is already open when you get it, and you must leave it
  open.  Painter \link QPainter::setWorldMatrix() transformations\endlink
  are switched off on entry.  If you transform the painter, remember to
  take the frame into account and \link QPainter::resetXForm() reset
  transformation\endlink before returning.

  This function is reimplemented by subclasses that draw something
  inside the frame.  It should draw only inside contentsRect(). The
  default function does nothing.

  \sa contentsRect(), QPainter::setClipRect(), drawContentsMask()
*/

void QFrame::drawContents( QPainter * )
{
}


/*!
  Virtual function that is called when the frame style, line width or
  mid-line width changes.

  This function can be reimplemented by subclasses that need to know
  when the frame attributes change.
*/

void QFrame::frameChanged()
{
}

/*!

 Reimplementation of QWidget::updateMask(). Draws the mask of the
 frame when transparency is required.

 This function calls the virtual functions drawFrameMask() and
 drawContentsMask(). These are the ones you may want to reimplement
 in subclasses.

 \sa QWidget::setAutoMask(), drawFrameMask(), drawContentsMask()

*/
void QFrame::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );
    QPainter p( &bm, this );
    p.setPen( color1 );
    p.setBrush( color1 );
    drawFrameMask( &p );
    drawContentsMask( &p );
    p.end();
    setMask( bm );
}


/*!
  Virtual function that draws the mask of the frame's frame.

  If you reimplemented drawFrame(QPainter*) and your widget should
  support transparency you probably have to re-implement this function as well.

  The default implementation is empty.

  \sa drawFrame(), updateMask(), QWidget::setAutoMask(), QPainter::setClipRect()
*/
void QFrame::drawFrameMask( QPainter* p )
{
    QPoint	p1, p2;
    QRect	r     = frameRect();
    int		type  = fstyle & MShape;
    int		style = fstyle & MShadow;

    QColorGroup g(color1, color1, color1, color1, color1, color1, color1, color1, color0);

    switch ( type ) {

    case Box:
	if ( style == Plain )
	    qDrawPlainRect( p, r, g.foreground(), lwidth );
	else
	    qDrawShadeRect( p, r, g, style == Sunken, lwidth,
			    midLineWidth() );
	break;

    case Panel:
	if ( style == Plain )
	    qDrawPlainRect( p, r, g.foreground(), lwidth );
	else
	    qDrawShadePanel( p, r, g, style == Sunken, lwidth );
	break;

    case WinPanel:
	if ( style == Plain )
	    qDrawPlainRect( p, r, g.foreground(), lwidth );
	else
	    qDrawWinPanel( p, r, g, style == Sunken );
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
	if ( style == Plain ) {
	    QPen oldPen = p->pen();
	    p->setPen( QPen(g.foreground(),lwidth) );
	    p->drawLine( p1, p2 );
	    p->setPen( oldPen );
	}
	else
	    qDrawShadeLine( p, p1, p2, g, style == Sunken,
			    lwidth, midLineWidth() );
	break;
    }
}

/*!
  Virtual function that draws the mask of the frame's contents.

  If you reimplemented drawContents(QPainter*) and your widget should
  support transparency you probably have to re-implement this function as well.

  The default implementation is empty.

  \sa drawContents(), updateMask(), QWidget::setAutoMask(), contentsRect(), QPainter::setClipRect()
*/
void QFrame::drawContentsMask( QPainter* )
{
}
