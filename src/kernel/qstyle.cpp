/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.cpp#28 $
**
** Implementation of QStyle class
**
** Created : 981231
**
** Copyright (C) 1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qstyle.h"
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
  \class QStyle qstyle.h
  \brief Encapsulates common Look and Feel of a GUI.

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
  \fn GUIStyle QStyle::guiStyle() const


  Returns an indicator to the additional "feel" component of a
  style. Current supported values are Windows and Motif.
*/


/*!
  Late initialization of the QApplication object like settings an
  appropriate color palette.

  \sa unPolish, QPalette, QApplication::setPalette()
 */
void QStyle::polish( QApplication*)
{
}

/*!
  Redo the application polisih

  \sa polish, QPalette, QApplication::setPalette()
 */
void QStyle::unPolish( QApplication*)
{
}

/*!
  Initializes the appearance of a widget.

  This function is called for every widget, after it has been fully
  created just \e before it is shown the very first time.

  Reasonable actions in this function might be to set the
  \link QWidget::backgroundMode()\endlink of the widget
  and the background pixmap, for example.  Unreasonable use
  would be setting the geometry!

  The QWidget::inherits() function may provide enough information to
  allow class-specific customizations.  But be careful not to hard-code
  things too much, as new QStyle sub-classes will be expected to work
  reasonably with all current \e and \e future widgets.

  The default implementation does nothing.

  \sa unPolish(QWidget*)
*/
void QStyle::polish( QWidget*)
{
}

/*!
  Undos the initializataion of a widget's appearance

  This function is the counterpart to polish. Is is called for every
  polished widget when the style is dynamically changed. The former
  style has to un-polish its settings before the new style can polish
  them again.

  The default implementation does nothing.

  \sa polish(QWidget*)
*/
void QStyle::unPolish( QWidget*)
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
		const QPixmap *pixmap, const QString& text, int len, const QColor* penColor )
{
    qDrawItem( p, gs, x, y, w, h, flags, g, enabled, pixmap, text, len, penColor );
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
  \fn void QStyle::drawButton( QPainter *, int , int , int , int ,
			     const QColorGroup &, bool, const QBrush* )
  Draws a press-sensitive shape in the style of a full featured  push button
*/

/*!
  \fn void QStyle::drawBevelButton( QPainter *, int , int , int , int ,
			     const QColorGroup &, bool, const QBrush* )

  Draws a press-sensitive shape in the style of a bevel button.
*/

/*!
  Draws a press-sensitive shape in the style of a toolbar button

  The default implementation calls drawBevelButton()
  \sa drawBevelButton()
*/
void QStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g, bool sunken, const QBrush* fill)
{
    drawBevelButton(p, x, y, w, h, g, sunken, fill);
}


/*!
  Returns the rectangle available for contents in a push
  button. Usually this is the entire rectangle but it may also be
  smaller when you think about rounded buttons.
*/
QRect QStyle::buttonRect( int x, int y, int w, int h){
    return QRect(x, y, w, h);
}

/*!
  Draw the mask of a pushbutton. Useful if a rounded pushbuttons needs
  to be transparent because the style uses a fancy background pixmap.
*/
void QStyle::drawButtonMask( QPainter *, int , int , int , int )
{
}

/*!
  Draws a press-sensitive shape in the style of a combo box or menu button
*/

void QStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool sunken,
			      bool /* editable */,
			      bool /*enabled */,
			      const QBrush *fill )
{
    drawButton(p, x, y, w, h, g, sunken, fill);
}


/*!
  Returns the rectangle available for contents in a combo box
  button. Usually this is the entire rectangle without the nifty menu
  indicator, but it may also be smaller when you think about rounded
  buttons.
*/
QRect QStyle::comboButtonRect( int x, int y, int w, int h)
{
    return buttonRect(x+3, y+3, w-6-21, h-6);
}

/*!
  Returns the rectangle used to draw the the focus rectangle in a combo box.
*/
QRect QStyle::comboButtonFocusRect( int x, int y, int w, int h)
{
    return buttonRect(x+4, y+4, w-8-21, h-8);
}

/*!
  Draw the mask of a combo box button. Useful if a rounded buttons
  needs to be transparent because the style uses a fancy background
  pixmap.
*/

void QStyle::drawComboButtonMask( QPainter *p, int x, int y, int w, int h)
{
    drawButtonMask(p, x, y, w, h);
}


/*!
  \fn void QStyle::drawPushButton( QPushButton*, QPainter *)

  Draws a pushbutton. This function will normally call drawButton()
  with arguments according to the current state of the pushbutton.

  \sa drawPushButtonLabel(), QPushButton::drawButton()
*/

/*!
  \fn void QStyle::drawPushButtonLabel( QPushButton*, QPainter *)

  Draws the label of a pushbutton. This function will normally call
  drawItem() with arguments according to the current state of the
  pushbutton.

  \sa drawPushButton(), QPushButton::drawButtonLabel()
*/

void QStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
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

    if ( btn->isDown() || btn->isOn() ){
	int sx = 0;
	int sy = 0;
	getButtonShift(sx, sy);
	x+=sx;
	y+=sy;
    }
    x += 2;  y += 2;  w -= 4;  h -= 4;
    drawItem( p, x, y, w, h,
	       AlignCenter|ShowPrefix,
	       btn->colorGroup(), btn->isEnabled(),
	       btn->pixmap(), btn->text(), -1, &btn->colorGroup().buttonText() );

    if ( dx || dy )
	p->translate( -dx, -dy );
}



/*!
  Some GUI styles shift the contents of a button when the button is down.
  The default implementation returns 0 for both x and y.
 */
void QStyle::getButtonShift( int &x, int &y)
{
    x = 0;
    y = 0;
}



/*!
  The default frame width, usually 2.
 */
int QStyle::defaultFrameWidth()
{
    return 2;
}

/*!
  Draws a panel to separate parts of the visual interface.
*/
void
QStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
		const QColorGroup &g, bool sunken,
		int lineWidth, const QBrush* fill)
{
        if ( w == 0 || h == 0 )
	return;
#if defined(CHECK_RANGE)
    ASSERT( w > 0 && h > 0 && lineWidth >= 0 );
#endif
    QPen oldPen = p->pen();			// save pen
    QPointArray a( 4*lineWidth );
    if ( sunken )
	p->setPen( g.dark() );
    else
	p->setPen( g.light() );
    int x1, y1, x2, y2;
    int i;
    int n = 0;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for ( i=0; i<lineWidth; i++ ) {		// top shadow
	a.setPoint( n++, x1, y1++ );
	a.setPoint( n++, x2--, y2++ );
    }
    x2 = x1;
    y1 = y+h-2;
    for ( i=0; i<lineWidth; i++ ) {		// left shadow
	a.setPoint( n++, x1++, y1 );
	a.setPoint( n++, x2++, y2-- );
    }
    p->drawLineSegments( a );
    n = 0;
    if ( sunken )
	p->setPen( g.light() );
    else
	p->setPen( g.dark() );
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for ( i=0; i<lineWidth; i++ ) {		// bottom shadow
	a.setPoint( n++, x1++, y1-- );
	a.setPoint( n++, x2, y2-- );
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lineWidth-1;
    for ( i=0; i<lineWidth; i++ ) {		// right shadow
	a.setPoint( n++, x1--, y1++ );
	a.setPoint( n++, x2--, y2 );
    }
    p->drawLineSegments( a );
    if ( fill ) {				// fill with fill color
	QBrush oldBrush = p->brush();
	p->setPen( NoPen );
	p->setBrush( *fill );
	p->drawRect( x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2 );
	p->setBrush( oldBrush );
    }
    p->setPen( oldPen );			// restore pen
}

/*!
  \fn void QStyle::drawArrow( QPainter *p, Qt::ArrowType type, bool down,
		     int x, int y, int w, int h,
		     const QColorGroup &g, bool enabled, const QBrush *fill)
  Draws a button indicating direction.
*/

/*!
  \fn QSize QStyle::exclusiveIndicatorSize() const
  Returns the size of the mark used to indicate exclusive choice.
*/


/*!
  \fn void QStyle::drawExclusiveIndicator( QPainter* , int x, int y, int w, int h,
		const QColorGroup &, bool on, bool down, bool enabled )
  Draws a mark indicating the state of an exclusive choice.
*/


/*!
  Draws the mask of a mark indicating the state of an exclusive choice
*/
void
QStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool /* on */)
{
    p->fillRect(x, y, w, h, color1);
}

/*!
  \fn QSize QStyle::indicatorSize() const
  Returns the size of the mark used to indicate choice.
*/


/*!
  \fn void QStyle::drawIndicator( QPainter* , int , int , int , int , const QColorGroup &,
		       bool , bool , bool  )
  Draws a mark indicating the state of a choice.
*/

/*!
  Draws the mask of a mark indicating the state of a choice.
*/
void
QStyle::drawIndicatorMask( QPainter *p, int x, int y, int w, int h, bool /* on */)
{
    p->fillRect(x, y, w, h, color1);
}

/*!
  \fn void QStyle::drawFocusRect( QPainter* p,
		const QRect& r, const QColorGroup &g , const QColor*)

  Draws a mark indicating keyboard focus is on \a r.
*/

/*!

  \fn void QStyle::scrollBarMetrics( const QScrollBar*, int &, int &, int &, int& )

  Returns the metrics of the passed scrollbar: sliderMin, sliderMax,
  sliderLength and buttonDim.

*/


/*!
  Returns the scrollbar control under the passed point.
 */
QStyle::ScrollControl QStyle::scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p)
{
        if ( !sb->rect().contains( p ) )
	return NONE;
    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );
    pos = (sb->orientation() == QScrollBar::Horizontal)? p.x() : p.y();
    if ( pos < sliderMin )
	return SUB_LINE;
    if ( pos < sliderStart )
	return SUB_PAGE;
    if ( pos < sliderStart + sliderLength )
	return SLIDER;
    if ( pos < sliderMax + sliderLength )
	return ADD_PAGE;
    return ADD_LINE;
}

/*!

  \fn  void QStyle::drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls,
  uint activeControl )

  Draws the given scrollbar. Used internally by QScrollbar.

  The controls are either ADD_LINE, SUB_LINE, ADD_PAGE, SUB_PAGE,
  FIRST, LAST, SLIDER or NONE

  Controls is a combination of these, activeControl is the control
  currently pressed down.
 */


/*!
  \fn int QStyle::sliderLength() const;

  The length of a slider.

*/

/*!
  \fn void drawSlider( QPainter *p,
			     int x, int y, int w, int h,
			     const QColorGroup &g,
			     Orientation, bool tickAbove, bool tickBelow)
  Draws a slider.
			
*/

/*!
  Draws the mask of a slider
*/
void
QStyle::drawSliderMask( QPainter *p,
			int x, int y, int w, int h,
			Orientation, bool, bool )
{
    p->fillRect(x, y, w, h, color1);
}

/*!
  \fn  void QStyle::drawSliderGroove( QPainter *p,
				   int x, int y, int w, int h,
				   const QColorGroup& g, QCOORD c,
				   Orientation )

  Draws a slider groove

*/
		
		

/*!
  Draws the mask of a slider groove
*/
void
QStyle::drawSliderGrooveMask( QPainter *p,
				   int x, int y, int w, int h,
				   QCOORD /* c */,
				   Orientation )
{
    p->fillRect(x, y, w, h, color1);
}


/*!
  Some feels require the scrollbar or other sliders to jump back to
  the original position when the mouse pointer is too far away while
  dragging.

  This behavior can be customized with this function. The default is -1
  (no jump back) while Windows requires 20 (weird jump back).
*/
int QStyle::maximumSliderDragDistance() const
{
    return -1;
}



