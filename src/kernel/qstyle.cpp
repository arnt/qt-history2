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

/*!
  \enum Qt::GUIStyle

  \obsolete

  This enum only exists to keep existing source working and will be removed before
  the Qt 3.0 release.

  \value WindowsStyle
  \value MotifStyle
  \value MacStyle
  \value Win3Style
  \value PMStyle
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
  \obsolete

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
  Polishes the popup menu according to the GUI style. This is usually means
  setting the mouse tracking ( QPopupMenu::setMouseTracking() ) and whether
  the menu is checkable by default ( QPopupMenu::setCheckable() ).
*/
void QStyle::polishPopupMenu( QPopupMenu *)
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

  \value PO_ButtonCommand  button that performs an action/command, for example: a
  push button.
  \value PO_ButtonBevel  general purpose button bevel.
  \value PO_ButtonTool  tool button, for example: a button in a toolbar.
  \value PO_ButtonDropDown  drop down button, for example: a tool button that displays
  a popup menu.


  \value PO_FocusRect  general purpose focus indicator.


  \value PO_ArrowUp  up arrow.
  \value PO_ArrowDown  down arrow.
  \value PO_ArrowRight  right arrow.
  \value PO_ArrowLeft  left arrow.


  \value PO_SpinWidgetUp  up symbol for a spin widget.
  \value PO_SpinWidgetDown down symbol for a spin widget.
  \value PO_SpinWidgetPlus  increase symbol for a spin widget.
  \value PO_SpinWidgetMinus  decrease symbol for a spin widget.

  For more explanation of a spin widget, see the QSpinBox documentation.


  \value PO_Indicator  on/off indicator, for example: a check box.
  \value PO_IndicatorMask  bitmap mask for an indicator.
  \value PO_ExclusiveIndicator  exclusive on/off indicator, for example: a radio button.
  \value PO_ExclusiveIndicatorMask  bitmap mask for an exclusive indicator.


  \value PO_DockWindowHandle  tear off handle for dock windows and toolbars.
  \value PO_DockWindowSeparator  item separator for dock window and toolbar contents.
  \value PO_DockWindowResizeHandle  resize handle for dock windows.


  \value PO_Splitter  splitter handle.


  \value PO_Panel  general purpose panel frame.
  \value PO_PanelPopup  panel frame for popup windows/menus.
  \value PO_PanelMenuBar  panel frame for menu bars.
  \value PO_PanelDockWindow  panel frame for dock windows and toolbars.

  For more information on dock windows, see the QDockWindow documentation.


  \value PO_TabBarBase  area below tabs in a tab widget.


  \value PO_HeaderSection  section of a list/table header.
  \value PO_StatusBarSection  section of a status bar.


  \value PO_GroupBoxFrame  frame around a group box.


  \value PO_Separator  general purpose separator.


  \value PO_SizeGrip  window resize handle.


  \value PO_CheckMark  general purpose check mark.


  \value PO_ScrollBarAddLine  scrollbar line increase indicator (ie. scroll down).
  \value PO_ScrollBarSubLine  scrollbar line decrease indicator (ie. scroll up).
  \value PO_ScrollBarAddPage  scolllbar page increase indicator (ie. page down).
  \value PO_ScrollBarSubPage  scrollbar page decrease indicator (ie. page up).
  \value PO_ScrollBarSlider  scrollbar slider
  \value PO_ScrollBarFirst  scrollbar first line indicator (ie. home).
  \value PO_ScrollBarLast  scrollbar last line indicator (ie. end).


  \value PO_ProgressBarChunk  section of a progress bar indicator.
*/

/*!
  \enum QStyle::PrimitiveOperationFlags

  This enum represents flags for drawing PrimitiveOperations.  Not all primitives
  use all of these flags.  Note that these flags can have different meaning for
  different primitives.  For an explanation of the relationship of various primitives
  and flags, as well as the different meanings of the flags, see the Style overview.

  \value PStyle_Default
  \value PStyle_Enabled
  \value PStyle_Raised
  \value PStyle_Sunken
  \value PStyle_Off
  \value PStyle_NoChange
  \value PStyle_On
  \value PStyle_Down
  \value PStyle_Horizontal
  \value PStyle_Vertical
  \value PStyle_HasFocus
  \value PStyle_Top
  \value PStyle_Bottom
  \value PStyle_FocusAtBorder
  \value PStyle_AutoRaise
  \value PStyle_MouseOver
*/

/*!
  \fn void QStyle::drawPrimitive ( PrimitiveOperation op, QPainter *p, const QRect &r, const QColorGroup &cg, PFlags flags = PStyle_Default, void **data = 0 ) const;

  Draws the style PrimitiveOperation \a op using the painter \a p in the
  area \a r.  Colors are used from the color group \a cg.

  The \a flags argument is used to control how the PrimitiveOperation is drawn.
  Multiple flags can be OR'ed together.

  For example, a pressed button would be drawn with the flags \e PStyle_Enabled and
  \e PStyle_Down.

  The \a data argument can be used to control how various PrimitiveOperations are drawn.
  Note that \a data can be zero even for PrimitiveOperationss that make use of extra
  data. When data is non-zero, the data is used as follows:

  <center>
  <table cellpadding=4 cellspacing=2 border=0>
    <tr bgcolor=#A2C511>
      <th>PrimitiveOperation</th>
      <th>Data</th>
      <th>Data Cast</th>
      <th>Notes</th>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>PO_FocusRect</td>
      <td valign=top>data[0]</td>
      <td valign=top>const QColor *</td>
      <td valign=top>pointer to the background color the on which focus rect is
          being drawn.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>PO_Panel</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>line width for drawing the panel.</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>PO_PanelPopup</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>line width for drawing the panel.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>PO_PanelMenuBar</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>line width for drawing the panel.</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>PO_PanelDockWindow</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>line width for drawing the panel.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>PO_GroupBoxFrame</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>frame shape for the group box.  See the documentation for QFrame
          for more details.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top></td>
      <td valign=top>data[1]</td>
      <td valign=top>int *</td>
      <td valign=top>frame shadow for the group box.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top></td>
      <td valign=top>data[2]</td>
      <td valign=top>int *</td>
      <td valign=top>line width for the group box.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top></td>
      <td valign=top>data[3]</td>
      <td valign=top>int *</td>
      <td valign=top>mid-line width for the group box.</td>
    </tr>
  </table>
  </center>

  For all other PrimitiveOperations, \a data is unused.

  \sa PrimitiveOperation, PrimitiveOperationFlags
*/

/*!
  \enum QStyle::ControlElement

  This enum represents a ControlElement.  A ControlElement is part of a widget that
  does some action or display information to the user.


  \value CE_PushButton  the bevel and default indicator of a QPushButton.
  \value CE_PushButtonLabel  the label ( iconset with text or pixmap ) of a QPushButton.


  \value CE_CheckBox  the indicator of a QCheckBox.
  \value CE_CheckBoxLabel  the label ( text or pixmap ) of a QCheckBox.


  \value CE_RadioButton  the indicator of a QRadioButton.
  \value CE_RadioButtonLabel  the label ( text or pixmap ) of a QRadioButton.


  \value CE_TabBarTab  the tab within a QTabBar ( a QTab ).
  \value CE_TabBarLabel  the label with in a QTab.


  \value CE_ProgressBarGroove  the groove where the progress indicator is drawn in
         a QProgressBar.
  \value CE_ProgressBarContents  the progress indicator of a QProgressBar.
  \value CE_ProgressBarLabel  the text label of a QProgressBar.


  \value CE_PopupMenuItem  a menu item of a QPopupMenu.


  \value CE_MenuBarItem  a menu item of a QMenuBar.
*/

/*!
  \enum QStyle::ControlElementFlags

  This enum represents flags for drawing ControlElements.  Not all controls use all
  of these flags.  Note that these flags can have different meaning for different
  controls.  For an explanation of the relationship of various controls and flags,
  as well as the different meanings of the flags, see the Style overview.

  \value CStyle_Default
  \value CStyle_Selected
  \value CStyle_HasFocus
  \value CStyle_Active
*/

/*!
  \fn void QStyle::drawControl ( ControlElement element, QPainter *p, const QWidget *widget, const QRect &r, const QColorGroup &cg, CFlags how = CStyle_Default, void **data = 0 ) const;

  Draws the ControlElement \a element using the painter \a p in the area \a r.  Colors
  are used from the color group \a cg.

  The \a how argument is used to control how the ControlElement is drawn.  Multiple
  flags can be OR'ed together.

  The \a widget argument is a pointer to a QWidget or one of its subclasses.  The
  widget can be cast to the appropriate type based on the value of \a element. The
  \a data argument can be used to pass extra information required when drawing the
  ControlElement.  Note that \a data can be zero even for ControlElements that make
  use of the extra data.  See the table below for the appropriate \a widget and
  \a data casts:

  <center>
  <table cellpadding=4 cellspacing=2 border=0>
    <tr bgcolor=#A2C511>
      <th>ControlElement</th>
      <th>Widget Cast</th>
      <th>Data</th>
      <th>Data Cast</th>
      <th>Notes</th>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CE_PushButton</b></td>
      <td valign=top>const QPushButton *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CE_PushButtonLabel</b></td>
      <td valign=top>const QPushButton *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CE_CheckBox</b></td>
      <td valign=top>const QCheckBox *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CE_CheckboxLabel</b></td>
      <td valign=top>const QCheckBox *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CE_RadioButton</b></td>
      <td valign=top>const QRadioButton *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CE_RadioButtonLabel</b></td>
      <td valign=top>const QRadioButton *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CE_TabBarTab</b></td>
      <td valign=top>const QTabBar *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CE_TabBarLabel</b></td>
      <td valign=top>const QTabBar *</td>
      <td valign=top>data[0]</td>
      <td valign=top>QTab *</td>
      <td valign=top>pointer to the QTab being drawn.</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CE_ProgressBarGroove</b></td>
      <td valign=top>const QProgressBar *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CE_ProgressBarContents</b></td>
      <td valign=top>const QProgressBar *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CE_ProgressBarLabel</b></td>
      <td valign=top>const QProgressBar *</b></td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CE_PopupMenuItem</b></td>
      <td valign=top>const QPopupMenu *</td>
      <td valign=top>data[0]</td>
      <td valign=top>QMenuItem *</td>
      <td valign=top>pointer to the menu item being drawn.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top></td>
      <td valign=top></td>
      <td valign=top>data[1]</td>
      <td valign=top>int *</td>
      <td valign=top>width of the tab column where key accelerators are drawn.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top></td>
      <td valign=top></td>
      <td valign=top>data[2]</td>
      <td valign=top>int *</td>
      <td valign=top>maximum width of the check column where checkmarks and iconsets
          are drawn.</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CE_MenuBarItem</b></td>
      <td valign=top>const QMenuBar *</td>
      <td valign=top>data[0]</td>
      <td valign=top>QMenuItem *</td>
      <td valign=top>pointer to the menu item being drawn.</td>
    </tr>
  </table>
  </center>

  \sa ControlElement, ControlElementFlags
*/

/*!
  \fn void QStyle::drawControlMask( ControlElement element, QPainter *p, const QWidget *widget, const QRect &r, void **data = 0 ) const;

  Draw a bitmask for the ControlElement \a element using the painter \a p in the
  area \r.  See the documentation for drawControl() for an explanation on use
  of the \a widget and \a data arguments.

  \sa drawControl(), ControlElement
*/

/*!
  \enum QStyle::SubRect

  This enum represents a sub-area of a widget.  Style implementations would use these
  areas to draw the different parts of a widget.

  \value SR_PushButtonContents  area containing the label (iconset with text or pixmap).
  \value SR_PushButtonFocusRect  area for the focus rect (usually larger than the
         contents rect).

  \value SR_CheckBoxIndicator  area for the state indicator (eg. check mark).
  \value SR_CheckBoxContents  area for the label (text or pixmap).
  \value SR_CheckBoxFocusRect  area for the focus indicator.

  \value SR_RadioButtonIndicator  area for the state indicator.
  \value SR_RadioButtonContents  area for the label.
  \value SR_RadioButtonFocusRect  area for the focus indicator.

  \value SR_ComboBoxFocusRect  area for the focus indicator.

  \value SR_SliderFocusRect  area for the focus indicator.

  \value SR_DockWindowHandleRect  area for the tear-off handle.

  \value SR_ProgressBarGroove  area for the groove.
  \value SR_ProgressBarContents  area for the progress indicator.
  \value SR_ProgressBarLabel  area for the text label.

  \sa subRect()
*/

/*!
  \fn QRect QStyle::subRect( SubRect subrect, const QWidget *widget ) const;

  Returns the sub-area \a subrect for \a widget.

  The \a widget argument is a pointer to a QWidget or one of its subclasses.  The
  widget can be cast to the appropriate type based on the value of \a subrect.  See
  the table below for the appropriate \a widget casts:

  <center>
  <table cellpadding=4 cellspacing=2 border=0>
    <tr bgcolor=#A2C511>
      <th>SubRect</th>
      <th>Widget Cast</th>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SR_PushButtonContents</b></td>
      <td valign=top>const QPushButton *</td>
     </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SR_PushButtonFocusRect</b></td>
      <td valign=top>const QPushButton *</td>
     </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SR_CheckBoxIndicator</b></td>
      <td valign=top>const QCheckBox *</td>
     </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SR_CheckBoxContents</b></td>
      <td valign=top>const QCheckBox *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SR_CheckBoxFocusRect</b></td>
      <td valign=top>const QCheckBox *</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SR_RadioButtonIndicator</b></td>
      <td valign=top>const QRadioButton *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SR_RadioButtonContents</b></td>
      <td valign=top>const QRadioButton *</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SR_RadioButtonFocusRect</b></td>
      <td valign=top>const QRadioButton *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SR_ComboBoxFocusRect</b></td>
      <td valign=top>const QComboBox *</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SR_DockWindowHandleRect</b></td>
      <td valign=top>const QWidget *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SR_ProgressBarGroove</b></td>
      <td valign=top>const QProgressBar *</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SR_ProgressBarContents</b></td>
      <td valign=top>const QProgressBar *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SR_ProgressBarLabel</b></td>
      <td valign=top>const QProgressBar *</td>
    </tr>
  </table>
  </center>

  NOTE: The tear-off handle (SR_DockWindowHandleRect) for QDockWindow is a private
  class.  You can gain access to the QDockWindow by using the QWidget::parentWidget()
  function.  For example:

  \code
  if (! widget->parentWidget())
      return;
  const QDockWindow *dw = (const QDockWindow *) widget->parentWidget();
  \endcode

  \sa SubRect
*/

/*!
  \enum QStyle::ComplexControl

  This enum represents a ComplexControl.  A ComplexControl is different from a
  ControlElement.  ComplexControls have different behavior depending upon where
  the user clicks or which keys are pressed.

  \value CC_SpinWidget
  \value CC_ComboBox
  \value CC_ScrollBar
  \value CC_Slider
  \value CC_ToolButton
  \value CC_TitleBar
  \value CC_ListView

  \sa SubControl
*/

/*!
  \enum QStyle::SubControl

  This enum represents a SubControl within a ComplexControl.

  \value SC_None   special value that matches no other SubControl.


  \value SC_ScrollBarAddLine  scrollbar add line (ie. down/right arrow).
  \value SC_ScrollBarSubLine  scrollbar sub line (ie. up/left arrow).
  \value SC_ScrollBarAddPage  scrollbar add page (ie. page down).
  \value SC_ScrollBarSubPage  scrollbar sub page (ie. page up).
  \value SC_ScrollBarFirst  scrollbar first line (ie. home).
  \value SC_ScrollBarLast  scrollbar last line (ie. end).
  \value SC_ScrollBarSlider  scrollbar slider handle.
  \value SC_ScrollBarGroove  special subcontrol which contains the area in which the
         slider handle may move.


  \value SC_SpinWidgetUp  spinwidget up/increase.
  \value SC_SpinWidgetDown  spinwidget down/decrease.
  \value SC_SpinWidgetFrame  spinwidget frame.
  \value SC_SpinWidgetEditField  spinwidget edit field.
  \value SC_SpinWidgetButtonField  spinwidget button field.


  \value SC_ComboBoxEditField  combobox edit field.
  \value SC_ComboBoxArrow  combobox arrow


  \value SC_SliderGroove  special subcontrol which contains the area in which the
         slider handle may move.
  \value SC_SliderHandle  slider handle.
  \value SC_SliderTickmarks  slider tickmarks.


  \value SC_ToolButton  tool button.
  \value SC_ToolButtonMenu subcontrol for opening a popup menu in a tool button.


  \value SC_TitleBarSysMenu  system menu button (ie. restore, close, etc.).
  \value SC_TitleBarMinButton  minimize button.
  \value SC_TitleBarMaxButton  maximize button.
  \value SC_TitleBarCloseButton  close button.
  \value SC_TitleBarLabel  window title label.
  \value SC_TitleBarNormalButton  normal (restore) button.
  \value SC_TitleBarShadeButton  shade button.
  \value SC_TitleBarUnshadeButton  unshade button.


  \value SC_ListView  ???
  \value SC_ListViewBranch  ???
  \value SC_ListViewExpand  expand item (ie. show/hide child items).


  \value SC_All  special value that matches all SubControls.


  \sa ComplexControl
*/

/*!
  \fn void QStyle::drawComplexControl( ComplexControl control, QPainter *p, const QWidget *widget, const QRect &r, const QColorGroup &cg, CFlags flags = CStyle_Default, SCFlags sub = SC_All, SCFlags subActive = SC_None, void **data = 0 ) const;

  Draws the ComplexControl \a control using the painter \a p in the area \a r.  Colors
  are used from the color group \a cg.  The \a sub argument specifies which SubControls
  to draw.  Multiple SubControls can be OR'ed together.  The \a subActive argument
  specifies which SubControl to draw as active.

  The \a flags argument is used to control how the ComplexControl is drawn.  Multiple
  flags can OR'ed together.

  The \a widget argument is a pointe rto a QWidget or one of its subclasses.  The widget
  can be cast to the appropriate type based on the value of \a control.  The \a data
  argument can be used to pass extra information required when drawing the
  ComplexControl.  Note that \a data can be zero even for ComplexControls that make
  use of the extra data.  See the table below for the appropriate \a widget and
  \a data casts:

  <center>
  <table cellpadding=4 cellspacing=2 border=0>
    <tr bgcolor=#A2C511>
      <th>ComplexControl</th>
      <th>Widget Cast</th>
      <th>Data</th>
      <th>Data Cast</th>
      <th>Notes</th>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CC_SpinWidget</b></td>
      <td valign=top>const QSpinWidget *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CC_ComboBox</b></td>
      <td valign=top>const QComboBox *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CC_ScrollBar</b></td>
      <td valign=top>const QScrollBar *</td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>position in pixels for the start of the slider handle.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CC_Slider</b></td>
      <td valign=top>const QSlider *</td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>position in pixel for the start of the slider handle.</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CC_ToolButton</b></td>
      <td valign=top>const QToolButton *</td>
      <td valign=top>data[0]</td>
      <td valign=top>bool *</td>
      <td valign=top>when the tool button has auto-raise enabled, this bool is TRUE when
          the mouse is over the tool button, FALSE otherwise.</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top></td>
      <td valign=top></td>
      <td valign=top>data[1]</td>
      <td valign=top>bool *</td>
      <td valign=top>this bool TRUE when the tool button only contains an arrow, FALSE
          otherwise.</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top></td>
      <td valign=top></td>
      <td valign=top>data[2]</td>
      <td valign=top>ArrowType *</td>
      <td valign=top>when the tool button only contains an arrow, this is the arrow
          type.</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CC_TitleBar</b></td>
      <td valign=top>const QWidget *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CC_ListView</b></td>
      <td valign=top>const QListView *</td>
      <td valign=top>data[0]</td>
      <td valign=top>QListViewItem *</td>
      <td valign=top>pointer to the item that needs branches drawn</td>
    </tr>
  </table>
  </center>

  \sa ComplexControl, SubControl
*/

/*!
  \fn void QStyle::drawComplexControlMask( ComplexControl control, QPainter *p, const QWidget *widget, const QRect &r, void **data = 0 ) const;

  Draw a bitmask for the ComplexControl \a control using the painter \a p in the
  area \a r.  See the documentation for drawComplexControl() for an explanation on the
  use of the \a widget and \a data arguments.

  \sa drawComplexControl, ComplexControl
*/

/*!
  \fn QRect QStyle::querySubControlMetrics( ComplexControl control, const QWidget *widget, SubControl subcontrol, void **data = 0 ) const;

  Returns the rect for the SubControl \a subcontrol for \a widget.

  The \a widget argument is a pointer to a QWidget or one of its subclasses.  The
  widget can be cast to the appropriate type based on the value of \a control.
  The \a data argument can be used to pass extra information required when drawing the
  ComplexControl.  Note that \a data can be zero even for ComplexControls that make
  use of the extra data. See the documentation for drawComplexControl() for an
  explanation of the \a widget and \a data arguments.

  \sa drawComplexControl(), ComplexControl, SubControl
*/

/*!
  \fn SubControl QStyle::querySubControl( ComplexControl control, const QWidget *widget, const QPoint &pos, void **data = 0 ) const;

  Returns the SubControl for \a widget at the point \a pos.  The \a widget argument
  is a pointer to a QWidget or one of its subclasses,.  The widget can be case to the
  appropriate type based on the value of \a control.  The \a data argument can be
  used to pass extra information required when drawing the ComplexControl.  Note
  that \a data can be zero even for ComplexControls that make use of the extra data.
  See the documentation for drawComplexControl() for an explanation of the \a widget
  and \a data arguments.

  \sa drawComplexControl(), ComplexControl, SubControl, querySubControlMetrics()
*/

/*!
  \enum QStyle::PixelMetric


  \value PM_ButtonMargin
  \value PM_ButtonDefaultIndicator
  \value PM_MenuButtonIndicator
  \value PM_ButtonShiftHorizontal
  \value PM_ButtonShiftVertical


  \value PM_DefaultFrameWidth
  \value PM_SpinBoxFrameWidth


  \value PM_ScrollBarExtent
  \value PM_ScrollBarMaximumDragDistance


  \value PM_SliderThickness  total slider thickness
  \value PM_SliderControlThickness  thickness of the business part
  \value PM_SliderLength
  \value PM_SliderMaximumDragDistance
  \value PM_SliderTickmarkOffset
  \value PM_SliderSpaceAvailable  available space for slider to move


  \value PM_DockWindowSeparatorExtent
  \value PM_DockWindowHandleExtent
  \value PM_DockWindowFrameWidth


  \value PM_MenuBarFrameWidth


  \value PM_TabBarOverlap
  \value PM_TabBarHorizontalFrame
  \value PM_TabBarVerticalFrame
  \value PM_TabBarBaseHeight
  \value PM_TabBarBaseOverlap


  \value PM_ProgressBarChunkWidth


  \value PM_SplitterWidth


  \value PM_IndicatorWidth
  \value PM_IndicatorHeight
  \value PM_ExclusiveIndicatorWidth
  \value PM_ExclusiveIndicatorHeight
*/

/*!
  \fn int QStyle::pixelMetric( PixelMetric metric, const QWidget *widget = 0 ) const;

  document me!
*/

/*!
  \enum QStyle::ContentsType

  This enum represents a ContentsType.  It is used to calculate sizes for the contents
  of various widgets.

  \value CT_PushButton
  \value CT_CheckBox
  \value CT_RadioButton
  \value CT_ToolButton
  \value CT_ComboBox
  \value CT_Splitter
  \value CT_DockWindow
  \value CT_ProgressBar
  \value CT_PopupMenuItem

  \sa sizeFromContents()
*/

/*!
  \fn QSize QStyle::sizeFromContents( ContentsType contents, const QWidget *widget, const QSize &contentsSize, void **data = 0 ) const;

  Returns the size of \a widget based on the contents size \a contentsSize.

  The \a widget argument is a pointer to a QWidget or one of its subclasses.  The
  widget can be cast to the appropriate type based on the value of \a contents. The
  \a data argument can be used to pass extra information required when calculating the
  size.  Note that \a data can be zero even for ContentsTypes that make use of the
  extra data.  See the table below for the appropriate \a widget and \a data casts:

  <center>
  <table cellpadding=4 cellspacing=2 border=0>
    <tr bgcolor=#A2C511>
      <th>ContentsType</th>
      <th>Widget Cast</th>
      <th>Data</th>
      <th>Data Cast</th>
      <th>Notes</th>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CT_PushButton</b></td>
      <td valign=top>const QPushButton *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CT_CheckBox</b></td>
      <td valign=top>const QCheckBox *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CT_RadioButton</b></td>
      <td valign=top>const QRadioButton *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CT_ToolButton</b></td>
      <td valign=top>const QToolButton *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CT_ComboBox</b></td>
      <td valign=top>const QComboBox *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CT_Splitter</b></td>
      <td valign=top>const QSplitter *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CT_DockWindow</b></td>
      <td valign=top>const QDockWindow *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>CT_ProgressBar</b></td>
      <td valign=top>const QProgressBar *</td>
      <td valign=top>unused</td>
      <td valign=top></td>
      <td valign=top></td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>CT_PopupMenuItem</b></td>
      <td valign=top>const QPopupMenu *</td>
      <td valign=top>data[0]</td>
      <td valign=top>QMenuItem *</td>
      <td valign=top>pointer to the menu item to use when calculating the size</td>
    </tr>
  </table>
  </center>

  \sa ContentsType
*/

/*!
  \enum QStyle::StyleHint

  This enum represents a StyleHint.  A StyleHint is a general Look and/or Feel hint.

  \value SH_ScrollBar_BackgroundMode  the background mode for a QScrollBar.  Possible
         values are any in the Qt::BackgroundMode enum.
  \value SH_ScrollBar_MiddleClickAbsolutePosition  a boolean value.  If TRUE, middle
         clicking on a scrollbar causes the slider to jump to that position.  If FALSE,
         the click is ignored.
  \value SH_ScrollBar_ScrollWhenPointerLeavesControl  a boolean value.  If TRUE, when
         clicking a scrollbar SubControl, holding the mouse button down and moving the
	 pointer outside the SubControl, the scrollbar continues to scroll.  If FALSE,
	 the scollbar stops scrolling when the pointer leaves the SubControl.


  \value SH_TabBar_Alignment  the alignment for tabs in a QTabWidget.  Possible values
         are Qt::AlignLeft, Qt::AlignCenter and Qt::AlignRight.

  \sa styleHint()
*/

/*!
  \fn int QStyle::styleHint( StyleHint stylehint, const QWidget *widget = 0, void ***returnData = 0 ) const;

  Returns the style hint \a stylehint for \a widget.  Currently, \a widget and \a
  returnData are unused, and are provided only for future development considerations.

  \sa StyleHint
*/

/*!
  \enum QStyle::StylePixmap

  This enum represents a StylePixmap.  A StylePixmap is a pixmap that can follow some
  existing GUI style or guideline.


  \value SP_TitleBarMinButton  minimize button on titlebars.  For example, in a
         QWorkspace.
  \value SP_TitleBarMaxButton  maximize button on titlebars.
  \value SP_TitleBarCloseButton  close button on titlebars.
  \value SP_TitleBarNormalButton  normal (restore) button on titlebars.
  \value SP_TitleBarShadeButton  shade button on titlebars.
  \value SP_TitleBarUnshadeButton  unshade button on titlebars.


  \value SP_DockWindowCloseButton  close button on dock windows.  See also QDockWindow.
*/

/*!
  \fn QPixmap QStyle::stylePixmap( StylePixmap stylepixmap, const QWidget *widget = 0, void **data = 0 ) const;

  Returns a pixmap for \a stylepixmap.

  The \a data argument can be used to pass extra information required when drawing the
  ControlElement.  Note that \a data can be zero even for StylePixmaps that make
  use of the extra data.  Currently, the \a data argument is unused.

  The \a widget argument is a pointer to a QWidget or one of its subclasses.  The
  widget can be cast to the appropriate type based on the value of \a stylepixmap.
  See the table below for the appropriate \a widget casts:

  <center>
  <table cellpadding=4 cellspacing=2 border=0>
    <tr bgcolor=#A2C511>
      <th>StylePixmap</th>
      <th>Widget Cast</th>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SP_TitleBarMinButton</b></td>
      <td valign=top>const QWidget *</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SP_TitleBarMaxButton</b></td>
      <td valign=top>const QWidget *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SP_TitleBarCloseButton</b></td>
      <td valign=top>const QWidget *</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SP_TitleBarNormalButton</b></td>
      <td valign=top>const QWidget *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SP_TitleBarShadeButton</b></td>
      <td valign=top>const QWidget *</td>
    </tr>
    <tr bgcolor=#f0f0f0>
      <td valign=top><b>SP_TitleBarUnshadeButton</b></td>
      <td valign=top>const QWidget *</td>
    </tr>
    <tr bgcolor=#d0d0d0>
      <td valign=top><b>SP_DockWindowCloseButton</b></td>
      <td valign=top>const QDockWindow *</td>
    </tr>
  </table>
  </center>

  \sa StylePixmap
*/

/*!
  \fn QRect QStyle::visualRect( const QRect &logical, const QWidget *w );

  document me!
*/

/*!
  \fn QRect QStyle::visualRect( const QRect &logical, const QRect &bounding );

  document me!
*/


QRect QStyle::visualRect( const QRect &logical, const QWidget *w )
{
    QRect boundingRect = w->rect();
    QRect r = logical;
    if ( QApplication::reverseLayout() )
	r.moveBy( 2*(boundingRect.right() - logical.right()) +
		  logical.width() - boundingRect.width(), 0 );
    return r;
}

QRect QStyle::visualRect( const QRect &logical, const QRect &boundingRect )
{
    QRect r = logical;
    if ( QApplication::reverseLayout() )
	r.moveBy( 2*(boundingRect.right() - logical.right()) +
		  logical.width() - boundingRect.width(), 0 );
    return r;
}


#endif // QT_NO_STYLE
