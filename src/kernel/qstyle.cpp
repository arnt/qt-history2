/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstyle.cpp#119 $
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
  \preliminary
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
  This enum type defines
  \value AddLine  control to scroll one line down, usually an arrow button
  \value SubLine  control to scroll one line up, usually an arrow button
  \value AddPage  control to scroll one page down
  \value SubPage  control to scroll one page up
  \value First  control to scroll to top of the range
  \value Last  control to scroll to bottom of the range
  \value Slider  the slider control
  \value NoScroll  null value, indicates none of the visible controls
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
  Destroys the style and frees all allocated resources.
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


/*!

 Sets the width and height of the area between the tab bar and the
 tab pages (called the tab bar extension area). \a overlap contains the
 number of pixels the tab bar overlaps the tab bar extension area.

 \sa drawTabBarExtension()
*/
void QStyle::tabBarExtensionMetrics( const QTabWidget *, int & width,
				     int & height, int & overlap ) const
{
    width   = 0;
    height  = 0;
    overlap = 0;
}

/*!

 Draws the area between the tab bar and the tab pages. Some GUI styles
 use this area to draw special decorations.

 \sa tabBarExtensionMetrics()
*/
void QStyle::drawTabBarExtension( QPainter *, int, int, int, int,
				  const QColorGroup &, const QTabWidget * )
{
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

const QWidget *qt_style_global_context = NULL;
/*!
   Returns the global static widget currently requesting a QStyle operations.
   This value will be modified whenever QApplication::style() or QWidget::style()
   is called, you may use this pointer to gain information about the widget
   you may be drawing on.
*/
const QWidget *QStyle::contextWidget()
{
    return qt_style_global_context;
}

/*!
  \fn void QStyle::drawHeaderSection ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g, bool down )
*/

/*!
  \fn void QStyle::drawSpinWidgetButton ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g, QSpinWidget * sw, bool downbtn, bool enabled, bool down )

  Draws the spinbox button of the spin widget \a sw with the painter \a p into the rectangle
  (\a x, \a y, \a w, \a h) using colorgroup \a g. The function draws the down button if \a downbtn is TRUE,
  otherwise the up button. \a enabled and \a down represent the state of the button.
*/

/*!
  \fn void QStyle::drawSpinWidgetSymbol ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g, QSpinWidget * sw, bool downbtn, bool enabled, bool down )

  Draws the spinbox button symbol of the spin widget \a sw with the painter \a p into the rectangle
  (\a x, \a y, \a w, \a h) using colorgroup \a g. The function draws the down symbol if \a downbtn is TRUE,
  otherwise the up symbol. \a enabled and \a down represent the state of the button.
*/

/*!
  \fn void QStyle::drawGroupBoxTitle ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g, const QString & text, bool enabled )

  Draws a groupbox title \a text with painter \a p into the rectangle (\a x, \a y, \a w, \a h)
  using colorgroup \a g. \a enabled represents the state of the groupbox.
*/

/*!
  \fn void QStyle::drawGroupBoxFrame ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g, const QGroupBox * gb )

  Draws the frame of groupbox \a gb with painter \a p into the rectangle
  (\a x, \a y, \a w, \a h) using colorgroup \a g.
*/

/*!
  \fn void QStyle::drawStatusBarSection ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g, bool permanent )

  Draws a statusbar section with painter \a p into the rectangle (\a x, \a y, \a w, \a h) using colorgroup \a g.
  \a permanent represents the type of the section. Permanent sections are usually sunken.
*/

/*!
  \fn void QStyle::drawSizeGrip ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g )

  Draws a size grip with painter \a p into the rectangle (\a x, \a y, \a w, \a h) using colorgroup \a g.
*/

/*!
  \fn int QStyle::progressChunkWidth () const

  Returns the width of a chunk of a progress bar.
*/

/*!
  \fn void QStyle::drawProgressBar ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g )

  Draws a progressbar with painter \a p into the rectangle (\a x, \a y, \a w, \a h) using colorgroup \a g.
*/

/*!
  \fn void QStyle::drawProgressChunk ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g )

  Draws a progressbar chunk with painter \a p into the rectangle (\a x, \a y, \a w, \a h) using
  colorgroup \a g.
*/

/*!
  \fn QPixmap QStyle::titleBarPixmap( const QTitleBar *tb, TitleControl ctrl)

  Returns the pixmap representing the action in ctrl.

*/

/*!
  \fn void QStyle::titleBarMetrics( const QTitleBar*tb, int& ctrlW, int& ctrlH, int& titleW, int& titleH) const

  Retrieves metrics about the titlebar tb. \a ctrlW, \a ctrlH, \a titleW, and \a titleH are set to
  the width and height of all controls on a title bar, and the width and height of the title label
  itself.
*/

/*!
  \fn void QStyle::drawTitleBarControls( QPainter*p,  const QTitleBar*tb, uint controls, uint activeControl )

  Paints parts of the titlebar \a tb with painter \a p. The parts painted must be OR'd into controls, and all
  active (or pressed controls) will be OR'd into activeControl.
*/

/*!
   \fn TitleControl QStyle::titleBarPointOver( const QTitleBar*tb, const QPoint& pos)

   Returns the control of titlebar \a tb under point \a pos, if none are available TitleNone will be returned.
*/

/*!
   \fn void QStyle::drawListViewItemBranch( QPainter *p, int y, int w, int h, const QColorGroup & cg, QListViewItem *i );

   Paints the branches of QListViewItem \a i.
*/

/*!
   \fn ListViewItemControl QStyle::listViewItemPointOver( const QListViewItem *i, const QPoint &p );

  Returns the listview item part of the item \a i that contains the point \a p.
*/







// New QStyle API

/*!
  Returns the appropriate area within a rectangle in which to
  draw text or a pixmap.
*/
QRect QStyle::itemRect( QPainter *p, const QRect &r,
			int flags, bool enabled, const QPixmap *pixmap,
			const QString& text, int len ) const
{
    return qItemRect( p, gs, r.x(), r.y(), r.width(), r.height(),
		      flags, enabled, pixmap, text, len );
}


/*!
  Draws text or a pixmap in an area.
*/
void QStyle::drawItem( QPainter *p, const QRect &r,
		       int flags, const QColorGroup &g, bool enabled,
		       const QPixmap *pixmap, const QString& text, int len,
		       const QColor* penColor ) const
{
    qDrawItem( p, gs, r.x(), r.y(), r.width(), r.height(),
	       flags, g, enabled, pixmap, text, len, penColor );
}


#endif // QT_NO_STYLE
