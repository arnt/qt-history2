/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.cpp#3 $
**
** Implementation of QStyle class
**
** Created : 980616
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qstyle.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now


/*!
  \class QStyle qstyle.h
  \brief Encapsulates common Look and Feel of a GUI.

  THIS CLASS IS NOT YET IN USE.

  While it is not possible to fully enumerate the look of graphic elements
  and the feel of widgets in a GUI, a large number of elements are common
  to many widgets.  The QStyle class allows the look of these elements to
  be modified across all widgets that use the QStyle methods.  It also
  provides two feel options - Motif and Windows.

  In previous versions of Qt, the look and feel option for widgets 
  was specified by a single value - the GUIStyle.  Starting with
  Qt 2.0, this notion has been expanded to allow the look to be
  specified by virtual drawing functions.

  Derived classes may override some or all of the drawing functions
  to modify the look of all widgets which utilize those functions.
*/

/*!
  Constructs a QStyle that provides the style \a s.  This determines
  the default behavior of the virtual functions.
*/
QStyle::QStyle(GUIStyle s) : gs(s)
{
}

/*!
  Constructs a QStyle that provides the style most appropriate for
  the operating system - WindowsStyle for Windows, MotifStyle for Unix.
*/
QStyle::QStyle() :
#ifdef _WS_X11_
    gs(MotifStyle)
#else
    gs(WindowsStyle)
#endif
{
}

/*!
  Destructs the style.
*/
QStyle::~QStyle()
{
}

/*!
  Initializes the appearance of a widget.
  This function is called for every widget,
  during the first time it is
  \link QWidget::show()\endlink shown, and when
  it is visible or is shown after the style of the widget is
  \link QWidget::setStyle() changed.\endlink

  Reasonable actions in this function might be to set the
  \link QWidget::backgroundMode()\endlink of the widget
  and the background pixmap, for example.  Unreasonable use
  would be setting the geometry!

  The QWidget::inherits() function may provide enough information to
  allow class-specific customizations.  But be careful not to hard-code
  things too much, as new QStyle sub-classes will be expected to work
  reasonably with all current \e and \e future widgets.  

  The default implementation does nothing.
*/
void QStyle::initializeLook( QWidget* )
{
}

/*!
  Returns the appropriate area within a rectangle in which to
  draw text or a pixmap.
*/
QRect
QStyle::itemRect( QPainter *p, int x, int y, int w, int h,
		int flags, bool enabled,
		const QPixmap *pixmap, const QString& text, int len )
{
    return qItemRect( p, gs, x, y, w, h, flags, enabled, pixmap, text, len );
}

/*!
  Draw text or a pixmap in an area.
*/
void
QStyle::drawItem( QPainter *p, int x, int y, int w, int h,
		int flags, const QColorGroup &g, bool enabled,
		const QPixmap *pixmap, const QString& text, int len )
{
    qDrawItem( p, gs, x, y, w, h, flags, g, enabled, pixmap, text, len );
}


/*!
  Draws a line to separate parts of the visual interface.
*/
void
QStyle::drawSeparator( QPainter *p, int x1, int y1, int x2, int y2,
		 const QColorGroup &g, bool sunken,
		 int lineWidth, int midLineWidth )
{
    qDrawShadeLine( p, x1, y1, x2, y2, g, sunken, lineWidth, midLineWidth );
}

/*!
  Draws a simple rectangle to separate parts of the visual interface.
*/
void
QStyle::drawRect( QPainter *p, int x, int y, int w, int h,
		const QColor &c, int lineWidth,
		const QBrush *fill )
{
    qDrawPlainRect( p, x, y, w, h, c, lineWidth, fill );
}

/*!
  Draws an emphasized rectangle to strongly separate parts of the visual interface.
*/
void
QStyle::drawRectStrong( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool sunken,
		 int lineWidth, int midLineWidth,
		 const QBrush *fill )
{
    qDrawShadeRect( p, x, y, w, h, g, sunken, lineWidth, midLineWidth, fill );
}

/*!
  Draws a press-sensitive shape.
*/
void
QStyle::drawButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool sunken,
		 const QBrush *fill )
{
    if ( gs == WindowsStyle ) {
	qDrawWinButton( p, x, y, w, h, g, sunken, fill );
    } else {
	// move code here ...
    }
}

/*!
  Draws a panel to separate parts of the visual interface.
*/
void
QStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
		const QColorGroup &g, bool sunken,
		int lineWidth, const QBrush *fill )
{
    if ( gs == WindowsStyle ) {
	qDrawWinPanel( p, x, y, w, h, g, sunken, fill );
    } else {
	qDrawShadePanel( p, x, y, w, h, g, sunken, lineWidth, fill );
    }
}

/*!
  Draws a button indicating direction.
*/
void
QStyle::drawArrow( QPainter *p, ArrowType type, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool enabled )
{
    qDrawArrow( p, ::ArrowType(type), gs, down, x, y, w, h, g, enabled );
}

/*!
  Returns the size of the mark used to indicate exclusive choice.
*/
QSize
QStyle::exclusiveIndicatorSize() const
{
    // move code here ...
    return QSize(5,5);
}

/*!
  Draws a mark indicating the state of an exclusive choice.
*/
void
QStyle::drawExclusiveIndicator( QPainter* /*translated*/,
		const QColorGroup &, bool on, bool down )
{
    // move code here ...
    on=down;
}

/*!
  Returns the size of the mark used to indicate choice.
*/
QSize
QStyle::indicatorSize() const
{
    // move code here ...
    return QSize(5,5);
}

/*!
  Draws a mark indicating the state of a choice.
*/
void
QStyle::drawIndicator( QPainter* /*translated*/,
		const QColorGroup &, bool on, bool down )
{
    // move code here ...
    on=down;
}

/*!
  Draws a mark indicating keyboard focus is on \a r.
*/
void
QStyle::drawFocusRect( QPainter* p,
		const QRect& r, const QColorGroup &g )
{
    // move code here ...
    if ( gs == WindowsStyle ) {
	p->drawWinFocusRect( r, g.background() );
    } else {
	p->setPen( black );
	p->drawRect( r );
    }
}
