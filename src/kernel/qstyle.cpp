/****************************************************************************
** $Id: $
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

/* ### Delete when revised
    \enum QStyle::ScrollControl
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
    \overload
  Late initialization of the QApplication object.

  \sa unPolish(QApplication*)
 */
void QStyle::polish( QApplication*)
{
}

/*!
    \overload
  Undoes the application polish.

  \sa polish(QApplication*)
 */
void QStyle::unPolish( QApplication*)
{
}

/*!
    \overload
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

/*!
  \enum QStyle::PrimitiveOperation

  This enum represents a style PrimitiveOperation.  A PrimitiveOperation is
  a common GUI element, such as a checkbox indicator or pushbutton bevel.

  \value PO_ButtonCommand
  \value PO_ButtonBevel
  \value PO_ButtonTool
  \value PO_ButtonDropDown

  \value PO_FocusRect

  \value PO_ArrowUp
  \value PO_ArrowDown
  \value PO_ArrowRight
  \value PO_ArrowLeft

  \value PO_SpinWidgetUp
  \value PO_SpinWidgetDown
  \value PO_SpinWidgetPlus
  \value PO_SpinWidgetMinus

  \value PO_Indicator
  \value PO_IndicatorMask
  \value PO_ExclusiveIndicator
  \value PO_ExclusiveIndicatorMask

  \value PO_DockWindowHandle
  \value PO_DockWindowSeparator
  \value PO_DockWindowResizeHandle

  \value PO_Splitter

  \value PO_MenuBarItem

  \value PO_Panel
  \value PO_PanelPopup
  \value PO_PanelMenuBar
  \value PO_PanelDockWindow

  \value PO_TabBarBase

  \value PO_HeaderSection
  \value PO_StatusBarSection

  \value PO_GroupBoxFrame

  \value PO_Separator

  \value PO_SizeGrip

  \value PO_CheckMark
*/

/*!
  \fn void QStyle::drawPrimitive ( PrimitiveOperation op, QPainter *p, const QRect &r, const QColorGroup &cg, PFlags flags = PStyle_Default, void **data = 0 ) const

  Draws the style primitive \a op using the painter \a p in the
  rect \a r.  Colors are used from the color group \cg.

  The \a flags argument is used to control how the primitive is drawn.  Multiple
  flags can be used.

  For example, a pressed button would be drawn with the flags PStyle_Enabled and
  PStyle_Down.

  \sa PrimitiveOperation, PrimitiveOperationFlags
*/


QRect QStyle::visualRect( const QRect &logical, const QWidget *w )
{
    QRect boundingRect = w->rect();
    QRect r = logical;
    if ( QApplication::reverseLayout() )
	r.moveBy( 2*(boundingRect.right() - logical.right()) + logical.width() - boundingRect.width(), 0 );
    return r;
}

QRect QStyle::visualRect( const QRect &logical, const QRect &boundingRect )
{
    QRect r = logical;
    if ( QApplication::reverseLayout() )
	r.moveBy( 2*(boundingRect.right() - logical.right()) + logical.width() - boundingRect.width(), 0 );
    return r;
}    


#endif // QT_NO_STYLE
