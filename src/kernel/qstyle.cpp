/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.cpp#50 $
**
** Implementation of QStyle class
**
** Created : 981231
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "qstyle.h"
#ifndef QT_NO_STYLE
#include "qapplication.h"
#include "qnamespace.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now
#include "qwidget.h"
#include "qimage.h"

#include <limits.h>


class QStylePrivate
{
public:
    QStylePrivate()
    {
    }
};


/*!
  \class QStyle qstyle.h
  \brief The QStyle class specifies the look and feel of a GUI.

  \ingroup appearance

  Although it is not possible to fully enumerate the look of graphic elements
  and the feel of widgets in a GUI, a large number of elements are common
  to many widgets.  The QStyle class allows the look of these elements to
  be modified across all widgets that use the QStyle functions.  It also
  provides two feel options: Motif and Windows.

  In Qt 1.x the look and feel option for widgets was specified by a
  single value - the GUIStyle.  Starting with Qt 2.0, this notion has
  been expanded to allow the look to be specified by virtual drawing
  functions.

  Derived classes may reimplement some or all of the drawing functions
  to modify the look of all widgets that use those functions.

  Languages written from right to left (as hebrew and arabic) usually
  also mirror the whole layout of widgets. If you design a style, you should
  take special care when drawing asymmetric elements to make sure
  they also look correct for a mirrored layout. You can start your application
  with -reverse to check the mirrored layout. Also notice, that for a reversed
  layout, the light usually comes from top right instead of top left.
*/

/*! \enum QStyle::ScrollControl
  This enum type defines<ul>
  <li> \c AddLine - control to scroll one line down, usually an arrow button
  <li> \c SubLine - control to scroll one line up, usually an arrow button
  <li> \c AddPage - control to scroll one page down
  <li> \c SubPage - control to scroll one page up
  <li> \c First - control to scroll to top of the range
  <li> \c Last - control to scroll to bottom of the range
  <li> \c Slider - the slider control
  <li> \c NoScroll - null value, indicates none of the visible controls
  </ul>
*/

/*!
  Constructs a QStyle that provides the style \a s.  This determines
  the default behavior of the virtual functions.
*/

QStyle::QStyle(GUIStyle s) : gs(s)
{
    d = new QStylePrivate;
}

/*!
  Constructs a QStyle that provides the style most appropriate for
  the operating system - WindowsStyle for Windows, MotifStyle for Unix.
*/
QStyle::QStyle() :
#ifdef Q_WS_X11
    gs(MotifStyle)
#else
    gs(WindowsStyle)
#endif
{
    d = new QStylePrivate;
}

/*!
  Destructs the style and frees all allocated resources.
*/
QStyle::~QStyle()
{
    delete d;
    d = 0;
}

/*!
  \fn GUIStyle QStyle::guiStyle() const

  Returns an indicator to the additional "feel" component of a
  style. Current supported values are Qt::WindowsStyle and Qt::MotifStyle.
*/



/*!
  Initializes the appearance of a widget.

  This function is called for every widget at some point after it has
  been fully created but just \e before it is shown the very first
  time.

  Reasonable actions in this function might be to call
  QWidget::setBackgroundMode for the widget. An example of highly
  unreasonable use would be setting the geometry!

  The QWidget::inherits() function may provide enough information to
  allow class-specific customizations.  But be careful not to hard-code
  things too much because new QStyle subclasses will be expected to work
  reasonably with all current \e and \e future widgets.

  \sa unPolish(QWidget*)
*/
void QStyle::polish( QWidget*)
{
}

/*!
  Undoes the initialization of a widget's appearance.

  This function is the counterpart to polish. It is called for every
  polished widget when the style is dynamically changed. The former
  style has to unpolish its settings before the new style can polish
  them again.

  \sa polish(QWidget*)
*/
void QStyle::unPolish( QWidget*)
{
}


/*!
  Late initialization of the QApplication object.

  \sa unPolish(QApplication*)
 */
void QStyle::polish( QApplication*)
{
}

/*!
  Undoes the application polish.

  \sa polish(QApplication*)
 */
void QStyle::unPolish( QApplication*)
{
}

/*!
  The style may have certain requirements for color palettes.  In this
  function it has the chance to change the palette according to these
  requirements.

  \sa QPalette, QApplication::setPalette()
 */
void QStyle::polish( QPalette&)
{
}


/*!
  Returns the appropriate area within a rectangle in which to
  draw text or a pixmap.
*/
QRect QStyle::itemRect( QPainter *p, int x, int y, int w, int h,
                        int flags, bool enabled, const QPixmap *pixmap,
                        const QString& text, int len ) const
{
    return qItemRect( p, gs, x, y, w, h, flags, enabled, pixmap, text, len );
}

/*!
  Draws text or a pixmap in an area.
*/
void QStyle::drawItem( QPainter *p, int x, int y, int w, int h,
                       int flags, const QColorGroup &g, bool enabled,
                       const QPixmap *pixmap, const QString& text, int len,
                       const QColor* penColor )
{
    qDrawItem( p, gs, x, y, w, h, flags, g, enabled, pixmap, text, len, penColor );
}


/*!
  Draws a line to separate parts of the visual interface.
*/
void QStyle::drawSeparator( QPainter *p, int x1, int y1, int x2, int y2,
                            const QColorGroup &g, bool sunken,
                            int lineWidth, int midLineWidth )
{
    qDrawShadeLine( p, x1, y1, x2, y2, g, sunken, lineWidth, midLineWidth );
}

/*!
  Draws a simple rectangle to separate parts of the visual interface.
*/
void QStyle::drawRect( QPainter *p, int x, int y, int w, int h,
                       const QColor &c, int lineWidth,
                       const QBrush *fill )
{
    qDrawPlainRect( p, x, y, w, h, c, lineWidth, fill );
}

/*!
  Draws an emphasized rectangle to strongly separate parts of the
  visual interface.
*/
void QStyle::drawRectStrong( QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, bool sunken,
                             int lineWidth, int midLineWidth,
                             const QBrush *fill )
{
    qDrawShadeRect( p, x, y, w, h, g, sunken, lineWidth, midLineWidth, fill );
}

/*!
  \fn void QStyle::drawButton( QPainter *, int , int , int , int ,
                             const QColorGroup &, bool, const QBrush* )
  Draws a press-sensitive shape in the style of a full-featured push button

  \sa buttonRect()
*/

/*!
  \fn void QStyle::drawBevelButton( QPainter *, int , int , int , int ,
                             const QColorGroup &, bool, const QBrush* )

  Draws a press-sensitive shape in the style of a bevel button.

  \sa bevelButtonRect()
*/


/*!
  Returns the rectangle available for contents in a bevel
  button. Usually this is the entire rectangle minus the border, but
  it may also be smaller when you think about rounded buttons.

  \sa drawBevelButton()
*/
QRect QStyle::bevelButtonRect( int x, int y, int w, int h) const
{
    int fw = defaultFrameWidth();
    return QRect(x+fw, y+fw, w-2*fw, h-2*fw);
}


/*!
  Draws a press-sensitive shape in the style of a tool bar button

  The default implementation calls drawBevelButton().
  \sa drawBevelButton()
*/
void QStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, bool sunken,
                             const QBrush* fill)
{
    drawBevelButton(p, x, y, w, h, g, sunken, fill);
}

/*!
  Returns the rectangle available for contents in a tool
  button. Usually this is the entire rectangle minus the border, but
  it may also be smaller when you think about rounded buttons.

  The default implementation returns bevelButtonRect().

  \sa drawToolButton()
*/
QRect QStyle::toolButtonRect( int x, int y, int w, int h){
    return bevelButtonRect( x, y, w, h );
}



/*!
  Returns the rectangle available for contents in a push
  button. Usually this is the entire rectangle minus the border, but
  it may also be smaller when you think about rounded buttons.

  \sa drawButton()
*/
QRect QStyle::buttonRect( int x, int y, int w, int h) const
{
    int fw = defaultFrameWidth();
    return QRect(x+fw, y+fw, w-2*fw, h-2*fw);
}

/*!
  Draws the mask of a push button. Useful if a rounded push button needs
  to be transparent because the style uses a fancy background pixmap.

  \sa drawButtonMask()
*/
void QStyle::drawButtonMask( QPainter * p, int x, int y, int w, int h )
{
    QPen oldPen = p->pen();
    QBrush oldBrush = p->brush();

    p->fillRect( x, y, w, h, QBrush(Qt::color1) );

    p->setBrush( oldBrush );
    p->setPen( oldPen );
}

/*!
\fn void QStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken,
                              bool editable ,
                              bool enabled ,
                              const QBrush *fill )
  Draws a press-sensitive shape in the style of a combo box or menu button.
*/
void QStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken,
                              bool /* editable */,
                              bool /*enabled */,
                              const QBrush *fill )
{
    drawButton(p, x, y, w, h, g, sunken, fill);
}


/*! Returns the rectangle available for contents in a combo box
  button. Usually this is the entire rectangle without the nifty menu
  indicator, but it may also be smaller when you think about rounded
  buttons.
*/
QRect QStyle::comboButtonRect( int x, int y, int w, int h) const
{
    int xpos = x;
    if( QApplication::reverseLayout() )
        xpos += 21;
    return buttonRect(xpos, y, w-21, h);
}

/*!
  Returns the rectangle used to draw the focus rectangle in a combo box.
*/
QRect QStyle::comboButtonFocusRect( int x, int y, int w, int h) const
{
    return buttonRect(x+2, y+2, w-4-21, h-4);
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
  \overload void QStyle::drawToolButton( QToolButton*, QPainter *)

  Draws a tool button. This function will normally call drawToolButton()
  with arguments according to the current state of the tool button.

  \sa QToolButton::drawButton()
*/

/*!
  \fn void QStyle::drawPushButton( QPushButton*, QPainter *)

  Draws a push button. This function will normally call drawButton()
  with arguments according to the current state of the push button.

  \sa drawPushButtonLabel(), QPushButton::drawButton()
*/

/*!
  \fn void QStyle::drawPushButtonLabel( QPushButton*, QPainter *)

  Draws the label of a push button. This function will normally call
  drawItem() with arguments according to the current state of the
  push button.

  In reimplementations of this function, you will find
  pushButtonContentsRect() useful.

  \sa drawPushButton(), QPushButton::drawButtonLabel()
*/




/*! \fn void QStyle::getButtonShift( int &x, int &y) const
  Some GUI styles shift the contents of a button when the button is down.
  The default implementation returns 0 for both x and y.
 */
void QStyle::getButtonShift( int &x, int &y) const
{
    x = 0;
    y = 0;
}



/*!
  The default frame width, usually 2.
 */
int QStyle::defaultFrameWidth() const
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
#if defined(QT_CHECK_RANGE)
    Q_ASSERT( w > 0 && h > 0 && lineWidth >= 0 );
#endif
    QPen oldPen = p->pen();                     // save pen
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
    for ( i=0; i<lineWidth; i++ ) {             // top shadow
        a.setPoint( n++, x1, y1++ );
        a.setPoint( n++, x2--, y2++ );
    }
    x2 = x1;
    y1 = y+h-2;
    for ( i=0; i<lineWidth; i++ ) {             // left shadow
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
    for ( i=0; i<lineWidth; i++ ) {             // bottom shadow
        a.setPoint( n++, x1++, y1-- );
        a.setPoint( n++, x2, y2-- );
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lineWidth-1;
    for ( i=0; i<lineWidth; i++ ) {             // right shadow
        a.setPoint( n++, x1--, y1++ );
        a.setPoint( n++, x2--, y2 );
    }
    p->drawLineSegments( a );
    if ( fill ) {                               // fill with fill color
        QBrush oldBrush = p->brush();
        p->setPen( NoPen );
        p->setBrush( *fill );
        p->drawRect( x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2 );
        p->setBrush( oldBrush );
    }
    p->setPen( oldPen );                        // restore pen
}


/*!
  Draws a panel suitable as a frame for popup windows.
*/
void QStyle::drawPopupPanel( QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &cg, int lineWidth,
                     const QBrush *fill )
{
    drawPanel( p, x, y, w, h, cg, FALSE, lineWidth, fill );
}

/*!
  Returns the frame width for menu bars.
 */
int QStyle::menuBarFrameWidth() const
{
    return 2;
}

/*!
  Draws a panel suitable for menu bars.
*/
void QStyle::drawMenuBarPanel( QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &cg, const QBrush *fill )
{
    drawPanel( p, x, y, w, h, cg, FALSE, menuBarFrameWidth(), fill );
}

/*!
  Returns the frame width for tool bars, usually 1.
 */
int QStyle::toolBarFrameWidth() const
{
    return 1;

}
/*!
  Returns the size for a tool bar separator.
 */
QSize QStyle::toolBarSeparatorSize( Qt::Orientation orientation ) const
{
    if( orientation == Vertical )
        return QSize( 0, 6 );
    else
        return QSize( 6, 0 );
}

/*!
  Draws a panel suitable for tool bars.
*/
void QStyle::drawToolBarPanel( QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &cg, const QBrush *fill )
{
    drawPanel( p, x, y, w, h, cg, FALSE, toolBarFrameWidth(), fill );
}

/*!
  Draws a tool bar separator.
*/
void QStyle::drawToolBarSeparator( QPainter * p, int /*x*/, int /*y*/,
                                   int w, int h, const QColorGroup &cg,
                                   Orientation orientation )
{
    QPoint p1, p2;

    if ( orientation == Vertical ) {
        p1 = QPoint( 0, h/2 );
        p2 = QPoint( w, p1.y() );
    } else {
        p1 = QPoint( w/2, 0 );
        p2 = QPoint( p1.x(), h );
    }
    qDrawShadeLine( p, p1, p2, cg, 1, 1, 0 );
}


/*!
  \fn void QStyle::drawArrow( QPainter *p, Qt::ArrowType type, bool down, int x, int y, int w, int h, const QColorGroup &g, bool enabled, const QBrush *fill)
  Draws an arrow to indicate direction. Used, for example, in scroll bars and spin-boxes.
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
void QStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w,
                                         int h, bool /* on */)
{
    p->fillRect(x, y, w, h, Qt::color1);
}

/*!
  \fn QSize QStyle::indicatorSize() const
  Returns the size of the mark used to indicate choice.
*/


/*!
  \fn void QStyle::drawIndicator( QPainter* , int , int , int , int , const QColorGroup &,
                       int state , bool , bool  )
  Draws a mark indicating the state of a choice.
*/

/*!
  Draws the mask of a mark indicating the state of a choice.
*/
void
QStyle::drawIndicatorMask( QPainter *p, int x, int y, int w, int h, int /*state*/ )
{
    p->fillRect(x, y, w, h, Qt::color1);
}

/*!
  \fn void QStyle::drawFocusRect( QPainter* p,
                const QRect& r, const QColorGroup &g , const QColor*, bool atBorder)

  Draws a mark indicating that keyboard focus is on \a r. \a atBorder
  indicates whether the focus rectangle is at the border of an item
  (for example, an item in a list box). Certain styles (Motif style is
  the most prominent example) might then have to shrink the rectangle a bit
  to ensure that the focus rectangle is visible.
*/


/* \fn void QStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe, int& overlap)

  TO DO.
 */

/* \fn void QStyle::drawTab( QPainter* p,  const  QTabBar* tb, QTab* t , bool selected )

   TO DO.

*/

/* \fn void QStyle::drawTabMask( QPainter* p,  const  QTabBar* tb , QTab* t, bool selected )

TO DO.

*/

/*!

  \fn void QStyle::scrollBarMetrics( const QScrollBar*, int &, int &, int &, int& ) const

  Returns the metrics of the passed scroll bar: sliderMin, sliderMax,
  sliderLength, and buttonDim.

*/


/*! \fn QStyle::ScrollControl QStyle::scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p)

  Returns the scroll bar control under the passed point.
 */

/*!

  \fn  void QStyle::drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls,
  uint activeControl )

  Draws the given scroll bar.  Used internally by QScrollbar.

  The controls are either ADD_LINE, SUB_LINE, ADD_PAGE, SUB_PAGE,
  FIRST, LAST, SLIDER or NONE.

  Controls is a combination of these; activeControl is the control
  currently pressed down.
 */


/*!
  \fn int QStyle::sliderLength() const;

  The length of a slider.

*/

/*!
  \fn void QStyle::drawSlider( QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, Orientation, bool tickAbove, bool tickBelow)
  Draws a slider.

*/

/*!
  Draws the mask of a slider
*/
void QStyle::drawSliderMask( QPainter *p,
                        int x, int y, int w, int h,
                        Orientation, bool, bool )
{
    p->fillRect(x, y, w, h, Qt::color1);
}


/*!
  \fn  void QStyle::drawSliderGroove( QPainter *p,  int x, int y, int w, int h,
                                   const QColorGroup& g, QCOORD c, Orientation )

  Draws a slider groove.

*/



/*
  Draws the mask of a slider groove
*/
void QStyle::drawSliderGrooveMask( QPainter *p,int x, int y, int w, int h,
                                   QCOORD c, Orientation orient )
{
    QColorGroup g(Qt::color1, Qt::color1, Qt::color1, Qt::color1, Qt::color1,
                  Qt::color1, Qt::color1, Qt::color1, Qt::color0);
    drawSliderGroove( p, x, y, w, h, g, c, orient );
}

/*!
  Some styles require the scroll bar (or other slider) to jump back to
  the original position when the mouse pointer is too far away while
  dragging.

  This behavior can be customized with this function. The default is -1
  (no jump back); Windows requires 20 (weird jump back).
*/
int QStyle::maximumSliderDragDistance() const
{
    return -1;
}


/*!
  \fn int QStyle::splitterWidth() const

  Returns the width of a splitter handle.

  \sa drawSplitter()
*/

/*!
  \fn void QStyle::drawSplitter( QPainter *p,
                             int x, int y, int w, int h,
                             const QColorGroup &g,
                             Orientation orient)

  Using painter \a p and color group \a g, draws a splitter handle in
  the rectangle described by \a x, \a y, \a w, \a h. The orientation
  is \a orient.

  \sa splitterWidth()
*/


/*! \fn void QStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g,
                                bool act, bool dis )

Draws a check mark suitable for check boxes and checkable menu items.

*/
/*!  \fn void QStyle::polishPopupMenu( QPopupMenu* p)

  Polishes the popup menu \a p according to the GUI style. This
  usually means setting the mouse tracking (see
  QPopupMenu::setMouseTracking()) and whether the menu is checkable by
  default (see QPopupMenu::setCheckable()).
*/


/*! \fn int QStyle::extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem* mi, const QFontMetrics& fm ) const

  Returns the extra width of a menu item \a mi, which means all the extra
  pixels besides the space that the menu item text requires. \a checkable
  defines whether the menu has a check column. \a maxpmw is the
  maximium width of all iconsets within a check column and \a fm
  defines the font metrics used to draw the label. This is
  particulary useful for calculating a suitable size for a submenu
  indicator or the column separation, including the tab column used to
  indicate item accelerators.
 */

/*!
  Returns the width of the arrow indicating popup submenus.
  \a fm defines the font metrics used to draw the popup menu.
 */
int QStyle::popupSubmenuIndicatorWidth( const QFontMetrics& fm  ) const
{
    return fm.ascent() + 6; // motifArrowHMargin
}



/*! \fn int QStyle::popupMenuItemHeight( bool checkable, QMenuItem* mi, const QFontMetrics& fm ) const

  Returns the height of the menu item \a mi. \a checkable defines
  whether the menu has a check column; \a fm defines the font metrics
  used to draw the label.
 */

/*! \fn void QStyle::drawPopupMenuItem( QPainter* p, bool checkable,
                                    int maxpmw, int tab, QMenuItem* mi,
                                    const QPalette& pal, bool act,
                                    bool enabled,
                                    int x, int y, int w, int h);

 Draws the menu item \a mi using the painter \a p. The painter is preset
 to the right font. \a maxpmw is the maximium width of all iconsets within
 a check column. \a tab specifies the minimum number of pixels necessary
 to draw all labels of the menu without their accelerators (which are
 separated by a tab character in the label text). \a pal is the palette;
 \a act and \a enabled define whether the item is active (i.e.,
 highlighted) or enabled respectively. Finally, \a x, \a y, \a w and \a h
 determine the geometry of the entire item.

 Note that \a mi can be 0 when a multi-column popup menu ins being
 drawn. In that case, drawPopupMenuItem() simply draws the appropriate
 item background.
*/


/*!
  Returns a QSize containing the width of a vertical scroll bar and
  the height of a horizontal scroll bar in this style.

  In this version of the Qt library subclasses must call
  setScrollBarExtent() to change the extent of scroll bars. In a future
  version of Qt this function will become virtual.
*/
QSize QStyle::scrollBarExtent() const
{
    return QSize( 16, 16 ).expandedTo( QApplication::globalStrut() );
}

/*! Returns the extent of a tool bar handle.

  (The extent of a horizontal tool bar's handle is the handle's height.)
*/

int QStyle::toolBarHandleExtent() const
{
    if ( guiStyle() == Qt::MotifStyle )
        return 9;
    return 11;
}

/*!
  Draws the handle for the tool bar using the painter \a p with the handle
  coordinates \a r. \a orientation gives the orientation of the tool bar;
  the handle is drawn \a highlighted if \a highlight is TRUE, otherwise
  not. \a cg is the QColorGroup of the tool bar; and if \a drawBorder is
  TRUE a border around the handle may be drawn.
*/
void QStyle::drawToolBarHandle( QPainter *p, const QRect &r,
                                Qt::Orientation orientation,
                                bool highlight, const QColorGroup &cg, bool )
{
    p->save();
    p->translate( r.x(), r.y() );

    if ( orientation == Qt::Vertical ) {
        if ( r.width() > 4 ) {
            qDrawShadePanel( p, 2, 4, r.width() - 4, 3,
                             cg, highlight, 1, 0 );
            qDrawShadePanel( p, 2, 7, r.width() - 4, 3,
                             cg, highlight, 1, 0 );
        }
    } else {
        if ( r.height() > 4 ) {
            qDrawShadePanel( p, 4, 2, 3, r.height() - 4,
                             cg, highlight, 1, 0 );
            qDrawShadePanel( p, 7, 2, 3, r.height() - 4,
                             cg, highlight, 1, 0 );
        }
    }
    p->restore();
}


/*!
  Returns the width of the indicator frame for a default button.

  In this version of the Qt library, subclasses must call
  setButtonDefaultIndicatorWidth() to change the frame width. In a
  future version of Qt this function will become virtual.
*/
int QStyle::buttonDefaultIndicatorWidth() const
{
    return 0;
}

/*!
  \fn QRect QStyle::pushButtonContentsRect( QPushButton* btn ) const

  Returns the contents rectangle of a push button \a btn. The contents
  rectangle is the space available for the button label.

  The result depends on the look (buttonRect()), whether the button needs
  space for a default indicator (buttonDefaultIndicatorWidth()), and
  whether it is pushed down and needs to be shifted (getButtonShift()).
 */



/*!
  Returns the width of the menu button indicator for a given button
  height \a h.
 */
int QStyle::menuButtonIndicatorWidth( int h ) const
{
    return QMAX( 12, (h-4)/3 );
}

/*! \fn void QStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
                                    QMenuItem* mi, QColorGroup& g,
                                    bool enabled, bool )

  Draws the menu item \a mi using the painter \a p and the ButtonText
  color of \a g. The painter is preset to the right font. \a x, \a y, \a w
  and \a h determine the geometry of the entire item.

  In a future version of the Qt library this function will become
  obsolete, and subclasses will be able to reimplement drawMenuBarItem().

  \sa drawPopupMenuItem()
*/



/*!
  Returns the amount of whitespace between pushbutton labels and
  the frame in this style.
*/
int QStyle::buttonMargin() const
{
    return 6;
}

/*!
  Returns the thickness of a slider in this style.  The thickness is
  dimension perpendicular to the slider motion (e.g. the height for
  a horizontal slider).
*/
int QStyle::sliderThickness() const
{
    return 16;
}

#endif // QT_NO_STYLE
