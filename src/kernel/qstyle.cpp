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

/*! \enum Qt::GUIStyle
    \value WindowsStyle
    \value MotifStyle
    \value MacStyle, \e obsolete
    \value Win3Style, \e obsolete
    \value PMStyle, \e obsolete
*/
/*!
    \enum Qt::UIEffect
    \value UI_General
    \value UI_AnimateMenu
    \value UI_FadeMenu
    \value UI_AnimateCombo
    \value UI_AnimateTooltip
    \value UI_FadeTooltip
*/
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
  Multiple flags can be used.

  For example, a pressed button would be drawn with the flags \e PStyle_Enabled and
  \e PStyle_Down.

  The \a data argument can be used to control how various PrimitiveOperations are drawn.
  Note that \a data can be zero even for PrimitiveOperationss that make use of extra
  data. When data is non-zero, the data is used as follows:

  <p align=center>
  <table cellpadding=2 cellspacing=2 border=0>
    <tr bgcolor=#e5e5e5>
      <td valign=top width=30%><b>PO_FocusRect</td>
      <td valign=top width=20%>data[0]</td>
      <td valign=top width=20%>const QColor *</td>
      <td valign=top width=30%>pointer to the background color the on which focus rect is
          being drawn </td>
    </tr>
    <tr bgcolor=#ffffff>
      <td valign=top><b>PO_Panel, PO_PanelPopup, PO_PanelMenuBar, PO_PanelDockWindow</b>
      </td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>line width for drawing the panel.</td>
    </tr>
    <tr bgcolor=#e5e5e5>
      <td valign=top><b>PO_GroupBoxFrame</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>int *</td>
      <td valign=top>frame shape for the group box.  See the documentation for QFrame
          for more details.</td>
    </tr>
    <tr bgcolor=#e5e5e5>
      <td valign=top></td>
      <td valign=top>data[1]</td>
      <td valign=top>int *</td>
      <td valign=top>frame shadow for the group box.</td>
    </tr>
    <tr bgcolor=#e5e5e5>
      <td valign=top></td>
      <td valign=top>data[2]</td>
      <td valign=top>int *</td>
      <td valign=top>line width for the group box.</td>
    </tr>
    <tr bgcolor=#e5e5e5>
      <td valign=top></td>
      <td valign=top>data[3]</td>
      <td valign=top>int *</td>
      <td valign=top>mid-line width for the group box.</td>
    </tr>
  </table>
  </p>

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
  flags can be used.

  The \a widget argument is a pointer to a widget.  The widget can be cast to the
  appropriate type based on the value of \a element:

  \list
    \i \e CE_PushButton, \e CE_PushButtonLabel - \a widget can be cast to a QPushButton.
    \i \e CE_CheckBox, \e CE_CheckBoxLabel - \a widget can be cast to a QCheckBox.
    \i \e CE_RadioButton, \e CE_RadioButtonLabel - \a widget can be cast to a
       QRadioButton.
    \i \e CE_TabBarTab, \e CE_TabBarLabel - \a widget can be cast to a QTabBar.
    \i \e CE_ProgressBarGroove, \e CE_ProgressBarContents, \e CE_ProgressBarLabel
       - \a widget can be cast to a QProgressBar.
    \i \e CE_PopupMenuItem - \a widget can be cast to a QPopupMenu.
    \i \e CE_MenuBarItem - \a widget can be cast to a QMenuBar.
  \endlist

  The \a data argument can be used to pass extra information required when drawing
  the ControlElement.  Note that \a data can be zero even for ControlElements that make
  use of the extra data.  When data is non-zero, the data is used as follows:

  <p align=center>
  <table cellpadding=2 cellspacing=2 border=0 width=100%>
    <tr bgcolor=#e5e5e5>
      <td valign=top width=30%><b>CE_TabBarLabel</b></td>
      <td valign=top width=20%>data[0]</td>
      <td valign=top width=20%>QTab *</td>
      <td valign=top width=30%>pointer to the QTab being drawn.</td>
    </tr>
    <tr bgcolor=#ffffff>
      <td valign=top><b>CE_PopupMenuItem</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>QMenuItem *</td>
      <td valign=top>pointer to the menu item being drawn.</td>
    </tr>
    <tr bgcolor=#ffffff>
      <td valign=top></td>
      <td valign=top>data[1]</td>
      <td valign=top>int *</td>
      <td valign=top>width of the tab column where key accelerators are drawn.</td>
    </tr>
    <tr bgcolor=#ffffff>
      <td valign=top></td>
      <td valign=top>data[2]</td>
      <td valign=top>int *</td>
      <td valign=top>maximum width of the check column where checkmarks and iconsets
          are drawn.</td>
    </tr>
    <tr bgcolor=#e5e5e5>
      <td valign=top><b>CE_MenuBarItem</b></td>
      <td valign=top>data[0]</td>
      <td valign=top>QMenuItem *</td>
      <td valign=top>pointer to the menu item being drawn.</td>
    </tr>
  </table>
  </p>

  \sa ControlElement, ControlElementFlags
*/

/*!
  \fn void QStyle::drawControlMask( ControlElement element, QPainter *p, const QWidget *widget, const QRect &r, void **data = 0 ) const;

  Draw a bitmask for the ControlElement \a element using the painter \a p in the
  area \r.  See the documentation for drawControl() for an explanation on use
  of the \a data argument.

  \sa drawControl(), ControlElement
*/

/*!
  \enum QStyle::SubRect


  \value SR_PushButtonContents
  \value SR_PushButtonFocusRect

  \value SR_CheckBoxIndicator
  \value SR_CheckBoxContents
  \value SR_CheckBoxFocusRect

  \value SR_RadioButtonIndicator
  \value SR_RadioButtonContents
  \value SR_RadioButtonFocusRect

  \value SR_ComboBoxFocusRect

  \value SR_SliderFocusRect

  \value SR_DockWindowHandleRect

  \value SR_ProgressBarGroove
  \value SR_ProgressBarContents
  \value SR_ProgressBarLabel
*/

/*!
  \fn QRect QStyle::subRect( SubRect r, const QWidget *widget ) const;

  document me!
*/

/*!
  \enum QStyle::ComplexControl


  \value CC_SpinWidget
  \value CC_ComboBox
  \value CC_ScrollBar
  \value CC_Slider
  \value CC_ToolButton
  \value CC_TitleBar
  \value CC_ListView
*/

/*!
  \enum QStyle::SubControl


  \value SC_None

  \value SC_ScrollBarAddLine
  \value SC_ScrollBarSubLine
  \value SC_ScrollBarAddPage
  \value SC_ScrollBarSubPage
  \value SC_ScrollBarFirst
  \value SC_ScrollBarLast
  \value SC_ScrollBarSlider
  \value SC_ScrollBarGroove

  \value SC_SpinWidgetUp
  \value SC_SpinWidgetDown
  \value SC_SpinWidgetFrame
  \value SC_SpinWidgetEditField
  \value SC_SpinWidgetButtonField

  \value SC_ComboBoxEditField
  \value SC_ComboBoxArrow

  \value SC_SliderGroove
  \value SC_SliderHandle
  \value SC_SliderTickmarks

  \value SC_ToolButton
  \value SC_ToolButtonMenu

  \value SC_TitleBarSysMenu
  \value SC_TitleBarMinButton
  \value SC_TitleBarMaxButton
  \value SC_TitleBarCloseButton
  \value SC_TitleBarLabel
  \value SC_TitleBarNormalButton
  \value SC_TitleBarShadeButton
  \value SC_TitleBarUnshadeButton

  \value SC_ListView
  \value SC_ListViewBranch
  \value SC_ListViewExpand

  \value SC_All
*/

/*!
  \fn void QStyle::drawComplexControl( ComplexControl control, QPainter *p, const QWidget *widget, const QRect &r, const QColorGroup &cg, CFlags flags = CStyle_Default, SCFlags sub = SC_All, SCFlags subActive = SC_None, void **data = 0 ) const;

  document me!
*/

/*!
  \fn void QStyle::drawComplexControlMask( ComplexControl control, QPainter *p, const QWidget *widget, const QRect &r, void **data = 0 ) const;

  document me!
*/

/*!
  \fn QRect QStyle::querySubControlMetrics( ComplexControl control, const QWidget *widget, SubControl sc, void **data = 0 ) const;

  document me!
*/

/*!
  \fn SubControl QStyle::querySubControl( ComplexControl control, const QWidget *widget, const QPoint &pos, void **data = 0 ) const;

  document me!
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


  \value CT_PushButton
  \value CT_CheckBox
  \value CT_RadioButton
  \value CT_ToolButton
  \value CT_ComboBox
  \value CT_Splitter
  \value CT_DockWindow
  \value CT_ProgressBar
  \value CT_PopupMenuItem
*/

/*!
  \fn QSize QStyle::sizeFromContents( ContentsType contents, const QWidget *widget, const QSize &contentsSize, void **data = 0 ) const;

  document me!
*/

/*!
  \enum QStyle::StyleHint


  \value SH_ScrollBar_BackgroundMode
  \value SH_ScrollBar_MiddleClickAbsolutePosition
  \value SH_ScrollBar_ScrollWhenPointerLeavesControl


  \value SH_TabBar_Alignment
*/

/*!
  \fn int QStyle::styleHint( StyleHint stylehint, const QWidget *widget = 0, void ***returnData = 0 ) const;

  document me!
*/

/*!
  \enum QStyle::StylePixmap


  \value SP_TitleBarMinButton
  \value SP_TitleBarMaxButton
  \value SP_TitleBarCloseButton
  \value SP_TitleBarNormalButton
  \value SP_TitleBarShadeButton
  \value SP_TitleBarUnshadeButton
  \value SP_DockWindowCloseButton
*/

/*!
  \fn QPixmap QStyle::stylePixmap( StylePixmap stylepixmap, const QWidget *widget = 0, void **data = 0 ) const;

  document me!
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
