/****************************************************************************
**
** Implementation of the QStyleOption structure.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qstyleoption.h"

/*!
    \class QStyleOption qstyleoption.h
    \brief QStyleOption and its sub-classes describe parameters that are used
    by QStyle functions.
    \ingroup appearance

    In previous Qt versions, many QStyle functions required a QWidget parameter
    of a certain type in order for it to work correctly. This coupled the
    QStyle closely to the widgets it was styling. In Qt 4.0, this restriction
    has been removed.  Instead, a QStyleOption or one of its sub-classes supply
    the information the function needs.

    QStyleOption and its sub-classes all have a version parameter. The current
    version is 0.  This version number must be specified explicitly when
    creating the structure.  The version number helps to ensure future
    expansion without ruining compatiblity with older styles.

    Contrary to the rest of Qt, there are few member functions and the access
    to the variables is direct. The "low-level" feel makes the structures use
    straight forward and empahsizes that these are parameters used by the style
    functions, not clases. As a downside it forces developers to be careful to
    initialize all the variables.

    QStyleOption and its sub-classes can be safely checked using the qt_cast()
    functions.
*/

/*!
  \enum QStyleOption::OptionType

  This enum is used internally by the QStyleOption, its sub-classes and
  qt_cast() to determine the type of style option. In general you do not need
  to worry about this unless you want to create your own QStyleOption
  sub-class.

  \value SO_Default Indicates a default QStyleOption
  \value SO_FocusRect Indicates a \l QStyleOptionFocusRect
  \value SO_Button Indicates a \l QStyleOptionButton
  \value SO_Tab Indicates a \l QStyleOptionTab
  \value SO_MenuItem Indicates a \l QStyleOptionMenuItem
  \value SO_Complex Indicates a \l QStyleOptionComplex
  \value SO_Slider Indicates a \l QStyleOptionSlider
  \value SO_Frame Indicates a \l QStyleOptionFrame
  \value SO_ProgressBar Indicates a \l QStyleOptionProgressBar
  \value SO_ListView Indicates a \l QStyleOptionListView
  \value SO_ListViewItem Indicates a \l QStyleOptionListViewItem
  \value SO_Header Indicates a \l QStyleOptionHeader
  \value SO_DockWindow Indicates a \l QStyleOptionDockWindow
  \value SO_SpinBox Indicates a \l QStyleOptionSpinBox
  \value SO_ToolButton Indicates a \l QStyleOptionToolButton
  \value SO_ComboBox Indicates a \l QStyleOptionComboBox
  \value SO_ToolBox Indicates a \l QStyleOptionToolBox
  \value SO_TitleBar Indicates a \l QStyleOptionTitleBar
  \value SO_ViewItem Indicates a \l QStyleOptionViewItem (used in Interviews)
  \value SO_CustomBase This is reserved for custom QStyleOptions,
                        all custom values must be above this value.

*/
/*!
  Construct a QStyleOption with version \a optionversion and type \a optiontype.
  Usually, you will only pass \a optionversion. The \a optiontype parameter is
  mainly used by the subclasses for RTTI information.
*/

QStyleOption::QStyleOption(int optionversion, int optiontype)
    : version(optionversion), type(optiontype), state(QStyle::Style_Default)
{
}

/*!
  A convenience function that intializes some of the variables in the
  QStyleOption structure based on the widget \a w.

  The init() function assigns the rect and palette variables to \a w's rect()
  and palette() respectively. It also initializes the state variable checking
  if the widget is enabled and if it has focus.
*/
void QStyleOption::init(const QWidget *w)
{
    state = QStyle::Style_Default;
    if (w->isEnabled())
        state |= QStyle::Style_Enabled;
    if (w->hasFocus())
        state |= QStyle::Style_HasFocus;
    rect = w->rect();
    palette = w->palette();
}

/*!
    \property QStyleOption::palette

    \brief The palette that should be used in when painting the control
*/

/*!
    \property QStyleOption::rect

    \brief The area that should be used for various calculations and painting.

    This can have overloaded meanings. For example, for \l CE_PushButton it would
    be the rectangle for the entire button, while for \l CE_PushButtonLabel it
    would be just the area for the label.
*/

/*!
    \property QStyleOption::state

    \brief The QStyle::StyleFlags that are used when drawing the control.

    Several flags can be OR'd together. See drawControl and drawPrimitive for
    further information.

    \sa QStyle::drawPrimitive() QStyle::drawControl() QStyle::drawComplexControl() QStyle::StyleFlags
*/

/*!
    \property QStyleOption::type

    \brief The \l OptionType of the QStyleOption
*/

/*!
    \property QStyleOption::version

    \brief The version of the QStyleOption

    The current default is 0.
*/

/*!
    \class QStyleOptionFocusRect qstyleoption.h
    \brief QStyleOptionFocusRect describes the parameters for drawing a focus
    rect with QStyle.

    Note that on Mac OS X, the focus rect is handled by a FocusWidget, so be
    sure that you only use the focus rect for things that really need a focus
    rect or that it is not calling the QMacStyle::drawPrimitive in that case.

    \ingroup appearance
*/

/*!
  \fn QStyleOptionFocusRect::QStyleOptionFocusRect(int version)

  Construct a QStyleOptionFocusRect with version number \a version
*/

/*!
    \property QStyleOptionFocusRect::backgroundColor

    \brief The background color on which the focus rect is being drawn.
*/

/*!
    \class QStyleOptionFrame qstyleoption.h
    \brief QStyleOptionFrame describes the parameters for drawing a frame.

    QStyleOptionFrame is used for drawing several objects inside of Qt. Among
    them: QFrame, QGroupBox, QLineEdits, and the frame around a QMenu.
*/

/*!
    \fn QStyleOptionFrame::QStyleOptionFrame(int version)

    Construct a QStyleOptionFrame with version number \a version. The other
    members of the struct are set to zero.
*/

/*!
    \property QStyleOptionFrame::lineWidth

    \brief The line width for drawing the panel.
*/

/*!
    \property QStyleOptionFrame::midLineWidth

    \brief The mid-line width for drawing the panel. This is usually used in
    drawing sunken or raised frames.
*/

/*!
    \class QStyleOptionHeader qstyleoption.h
    \brief QStyleOptionHeader describes parameters for drawing a header.

    The QStleOptionHeader structure is used for drawing the header pane, the
    header sort arrow, and the header label.
*/

/*!
    \fn QStyleOptionHeader::QStyleOptionHeader(int version)

    Construct a QStyleOptionHeader struct with version number \a version. The
    members are either null, empty, or zero. It is your responsibilty to set
    the correct values yourself.
*/

/*!
    \property QStyleOptionHeader::section

    \brief Which section of the header is being painted.
*/

/*!
    \property QStyleOptionHeader::text

    \brief The text (if any) of the header.
*/

/*!
    \property QStyleOptionHeader::icon

    \brief The iconset (if any) of the header.
*/

/*!
    \class QStyleOptionButton qstyleoption.h
    \brief QStyleOptionButton describes the parameters to draw buttons.

    The QStyleOptionButton structure is used to draw \l QPushbuttons, \l
    QCheckBoxes, and \l QRadioButtons. \l QStyleOptionToolButton is used to draw
    a \l QToolButton.
*/

/*!
    \enum QStyleOptionButton::ButtonFeature

    This enum describles the different types of features a push button can have.

    \value None	Indicates a normal push button.
    \value Flat Indicates a flat push button.
    \value HasMenu Indicates that the button has a drop down menu.

    \sa QStyleOptionButton::features
*/

/*!
    \fn QStyleOptionButton::QStyleOptionButton(int version)

    Constructs a QStyleOptionButton with version \a version. The rest of the
    parameters are either zero, null, or empty. It is the developer's
    responsibilty to populate the structure with the proper parameters.
*/

/*!
    \property QStyleOptionButton::features
    \brief The features for the button

    This variable is a bitwise OR of the features that describe this button.
    \sa ButtonFeature
*/

/*!
    \property QStyleOptionButton::text
    \brief The text (if any) of the button.
*/

/*!
    \property QStyleOptionButton::icon
    \brief The iconset of the button (if any).
*/

/*!
    \class QStyleOptionTab qstyleoption.h
    \brief The QStyleOptionTab structure describe parameters for drawing a TabBar.

    The QStyleOptionTab structure is used for drawing the \l QTabBar and the
    pane for \l QTabWidget.
*/

/*!
    \fn QStyleOptionTab::QStyleOptionTab(int version)

    Constructs a QStyleOptionTab structure with version \a version. The
    parameters are either 0, empty or null. It is the developer's
    responsibility to populate the structure with proper values.
*/

/*!
    \property QStyleOptionTab::shape
    \brief The tab shape used to draw the tab.
    \sa QTabBar::Shape
*/

/*!
    \property QStyleOptionTab::text
    \brief The text of the tab (if any).
*/

/*!
    \property QStyleOptionTab::icon
    \brief The iconset for the tab (if any).
*/

/*!
    \property QStyleOptionTab::row
    \brief which row the tab is currently in. 0 indicates the front row.

    Currently this property is not used.
*/

/*!
    \class QStyleOptionProgressBar qstyleoption.h
    \brief QStyleOptionProgressBar describes the parameters necessary for drawing a progress bar.

    The QStyleOptionProgressBar structure is used to draw \l QProgressBars.
*/

/*!
    \enum QStyleOptionProgressBar::ProgressBarFeature

    This describes features for a progress bar.
    \value None Indicates a normal progress bar.
    \value CenterIndicator Indicates that the percentage indicator \l
    progressBarString should be centered.
    \value PercentageVisible Indicates that the \l progressBarString should also be drawn
    \sa QProgressBar::percentageVisible.
    \value IndicatorFollowsStyle Indicates that the \l progressBarString should
    follow the style.

    \sa features QProgressBar::indicatorFollowsStyle QProgressBar::centerIndicator
*/

/*!
    \fn QStyleOptionProgressBar::QStyleOptionProgressBar(int version)

    Construct a QStyleOptionProgressBar with version \a version. The values are
    either 0 or empty. It is the responsibilty of the developer to populate the
    structure with the proper values.
*/

/*!
    \property QStyleOptionProgressBar::features
    \brief Describes the features of the progress bar.

    This is a bitwise OR of the features of the progress bar.
    \sa ProgressBarFeature
*/

/*!
    \property QStyleOptionProgressBar::progressString
    \brief The progress of the progress bar as a string.

    This is the progress in terms of a string. If the string is empty, it
    indicates that the progress bar hasn't started yet.

    \sa QProgressBar::progressString
*/

/*!
    \property QStyleOptionProgressBar::totalSteps
    \brief The total steps for the progress bar.

    The total number of steps for the progress bar. If the totalSteps is zero,
    it indicates that a busy indicator should be drawn instead of a standard
    progress bar.

    \sa QProgressBar::totalSteps
*/

/*!
    \property QStyleOptionProgressBar::progress
    \brief the current progress for the progress bar.

    The current progress. If the value is -1, it indicates that the progress
    hasn't started yet.
*/

/*!
    \class QStyleOptionMenuItem qstyleoption.h
    \brief QStyleOptionMenuItem describes the parameter necessary for drawing a menu item.

    The QStyleOptionMenuItem is used for drawing menu items from \l QMenu.
    It is also used for drawing a variety of other menu related things.
*/

/*!
    \fn QStyleOptionMenuItem::QStyleOptionMenuItem(int version)

    Constructs a QStyleOptionMenuItem with version \a version. The values of
    the structure are either zero, null, or empty. It is the responsibilty of
    the developer to make sure these have proper values.
*/

/*!
    \enum QStyleOptionMenuItem::MenuItemType
    These values indicate the type of menu item that the structure describes.
    \value Normal A normal menu item.
    \value Separator A menu separator.
    \value SubMenu Indicates the menu item points to a sub-menu.
    \value Scroller A popup menu scroller (currently only used on Mac OS X).
    \value TearOff A tear-off handle for the menu.
    \value Margin The margin of the menu.
    \value EmptyArea The empty area of the menu.

*/

/*!
    \enum QStyleOptionMenuItem::CheckState

    These enums are used to indicate whether or not a check mark should be
    drawn for the item, or even if it should be drawn at all.

    \value NotCheckable The item is not checkable.
    \value Checked The item is checked.
    \value Unchecked The item can be checked, but currently is not checked.

    \sa QAction::checkable QAction::checked
*/

/*!
    \property QStyleOptionMenuItem::menuItemType
    \brief The current menuItemType for the structure.
    \sa MenuItemType
*/

/*!
    \property QStyleOptionMenuItem::checkState
    \brief The checkmark state for the structure.
    \sa CheckState
*/

/*!
    \property QStyleOptionMenuItem::menuRect
    \brief The rect for the entire menu.
*/

/*!
    \property QStyleOptionMenuItem::text
    \brief The text (if any) for the menu item.

    Note that the text format is something like this:
    MenuText<Tab>MenuShortCut

    If the menu item does not have a shortcut it will just contain the menu
    item's text.
*/

/*!
    \property QStyleOptionMenuItem::icon
    \brief The icon (if any) for the menu item.
*/

/*!
    \property QStyleOptionMenuItem::maxIconWidth
    \brief the maximum icon width for the icon in the menu item.

    This can be used for drawing the icon into the correct place or properly
    aligning items. maxIconWidth is set regardless of whether or not the menu
    item has an icon or not.
*/

/*!
    \property QStyleOptionMenuItem::tabWidth
    \brief The tab width for the menu item.

    The tab width is the distance between the text of the menu item and the
    shortcut if it contains one.
*/


/*!
    \property QStyleOptionMenuItem::font
    \brief The font used for the menu item text.

    This is the font that should be used for drawing the menu text minus the
    shortcut. The shortcut is usually drawn using the painter's font.
*/

/*!
    \class QStyleOptionComplex qstyleoption.h
    \brief The QStyleOptionComplex structure contains parameters that are
    common to all complex controls.

    This class is not used on its own. Instead it is used to derive other
    complex control options, for example \l QStyleOptionSlider and
    \l QStyleOptionSpinbox.
*/

/*!
    \fn QStyleOptionComplex::QStyleOptionComplex(int version, int type = SO_Complex)

    Constructs a QStyleOptioncomplex of type \a type and version \a version.
    Usually this constructor is called by structures derived
    QStyleOptionComplex.  The \l parts parameter is initialized to \l
    QStyle::SC_All and the \l activeParts is linked to \l QStyle::SC_None.
*/

/*!
    \property QStyleOptionComplex::parts
    \brief The sub-controls that need to be painted.

    This is a bitwise OR of the various parts that need to be drawn for the complex control.
    \sa QStyle::SubControl
*/

/*!
    \property QStyleOptionComplex::activeParts
    \brief The sub-controls that are active for the complex control.

    This a bitwise OR of the various parts that are "active" (pressed) for the complex control.
    \sa QStyle::SubControl
*/

/*!
    \class QStyleOptionSlider qstyleoption.h
    \brief QStyleOptionSlider describes the parameters needed for drawing a slider.

    The QStyleOptionSlider structure is used for drawing both a \l QSlider and a \l QScrollBar.
*/

/*!
    \fn QStyleOptionSlider::QStyleOptionSlider(int version)
    Construct a QStyleOptionSlider with version \a version.

    All the values in the structure are initialized to zero. It is the
    responsibility of the developer to set them to the proper value.
*/

/*!
    \property QStyleOptionSlider::orientation
    \brief Indicates whether the slider is horizontal or vertical.
    \sa Qt::Orientation
*/

/*!
    \property QStyleOptionSlider::minimum
    \brief The minimum value for the slider.
*/

/*!
    \property QStyleOptionSlider::maximum
    \brief The maximum value for the slider.
*/

/*!
    \property QStyleOptionSlider::tickmarks
    \brief Indicates the type of tickmarks the slider should have, if any.
    \sa QSlider::TickSetting
*/

/*!
    \property QStyleOptionSlider::tickInterval
    \brief The interval that should be drawn between tickmarks.
*/

/*!
    \property QStyleOptionSlider::useRightToLeft
    \brief Indicates slider control orientation.

    Normally a slider increases as it moves up or to the right, useRightToLeft
    indicates that it should do the opposite (increase as it moves down or to
    the left). Note that this is different than \l QApplication::reverse().

    \sa QStyle::positionFromValue QStyle::valueFromPosition QAbstractSlider::invertedAppearance
*/

/*!
    \property QStyleOptionSlider::sliderPosition
    \brief The position of the slider handle.

    If the slider has active feedback (QAbstractSlider::tracking is true). This
    value will be the same as \l sliderValue. Otherwise, it will have the
    current position of the handle.

    \sa QAbstractSlider::tracking sliderValue
*/

/*!
    \property QStyleOptionSlider::sliderValue
    \brief The value of the slider.

    If the slider has active feedback (QAbstractSlider::tracking is true). This
    value will be the same as \l sliderPosition. Otherwise, it will have the
    value the slider had before the mouse was pressed.

    \sa QAbstractSlider::tracking sliderPosition
*/

/*!
    \property QStyleOptionSlider::singleStep
    \brief The size of the single step of the slider.
    \sa QAbstractSlider::singleStep
*/

/*!
    \property QStyleOptionSlider::pageStep
    \brief The size of the page step of the slider.
    \sa QAbstractSlider::pageStep
*/

/*!
    \class QStyleOptionSpinBox qstyleoption.h
    \brief QStyleOptionSpinBox describes the parameters necessary for drawing a spin box.

    The QStyleOptionSpinBox is used for drawing a QSpinBox and QDateTimeEdit.
*/

/*!
    \fn QStyleOptionSpinBox::QStyleOptionSpinBox(int version)
    Construct a QStyleOptionSpinBox with version \a version.

    The values of the structure are set to zero. It is the Developer's
    responsiblity to set them to proper values.
*/

/*!
    \property QStyleOptionSpinBox::buttonSymbols
    \brief The type of button symbols to draw for the spin box.
    \sa QAbstractSpinBox::ButtonSymbols
*/

/*!
    \property QStyleOptionSpinBox::stepEnabled
    \brief Indicates which buttons of the spin box are enabled.
    \sa QAbstractSpinBox::StepEnabled
*/

/*!
    \property QStyleOptionSpinBox::percentage
    \brief The percentage of the spin box

    The quotient of the current value divided by the and maximum value.
*/

/*!
    \property QStyleOptionSpinBox::slider
    \brief Indicates whether the optional slider indicator should be drawn.
*/

/*!
    \class QStyleOptionListViewItem qsytleoption.h
    \brief QStyleOptionListViewItem describes an item drawn in a Q3ListView

    This is used by the compat Q3ListView to draw its items. It should be
    avoided for new clases.

    \sa Q3ListView Q3ListViewItem
*/

/*!
    \enum QStyleOptionListViewItem::ListViewItemFeature
    This enum describes the features a list view item can have.
    \value None A standard item
    \value Expandable The item has children that can be shown
    \value MultiLine The item is more than one line tall.
    \value Visible The item is visible
    \value ParentControl The item's parent is a type of item control (QCheckListItem::Controller)

    \sa Q3ListViewItem::isVisible() Q3ListViewItem::multiLinesEnabled() Q3ListViewItem::isExpandable() features
*/

/*!
    \fn QStyleOptionListViewItem::QStyleOptionListViewItem(int version)

    Construct a QStyleOptionListViewItem with version \a version.

    All values in the structure are set to zero. It is the responsibility of
    the developer to set all appropriate values.
*/

/*!
    \property QStyleOptionListViewItem::features
    \brief The features for this item

    This is a bitwise OR of the features of the item.
    \sa ListViewItemFeature
*/

/*!
    \property QStyleOptionListViewItem::height
    \brief The height of the item

    This is the height of just the item (not it's children).
    \sa Q3ListViewItem::height()
*/

/*!
    \property QStyleOptionListViewItem::totalHeight
    \brief The total height of the item

    This is the height of the item including its children.
    \sa Q3ListViewItem::totalHeight()
*/

/*!
    \property QStyleOptionListViewItem::itemY
    \brief The Y-coordinate for the item
    \sa Q3ListViewItem::itemY()
*/

/*!
    \property QStyleOptionListViewItem::childCount
    \brief The number of children the item has.
*/

/*!
    \class QStyleOptionListView qstyleoption.h
    \brief QStyleOptionListView describes the parameters for drawing a Q3ListView.

    The structure is used for drawing the compat \l Q3ListView. It is not
    recommended for use in new code.
*/

/*!
    \fn QStyleOptionListView::QStyleOptionListView(int version)
    Creates a QStyleOptionListView structure with version \a version

    The values in the structure are either zero, null, or empty. It is the
    developer's responsibility to set them to correct values.
*/

/*!
    \property QStyleOptionListView::items
    \brief A list of items in the \l Q3ListView.

    This is a list of \l {QStyleOptionListViewItem}s. The first item can be
    used for most of the calculation that are needed for listview. Any
    additional items are the children of this first item. Which may be used for
    additional information.

    \sa QStyleOptionListViewItem
*/

/*!
    \property QStyleOptionListView::viewportPalette
    \brief The palette of the viewport of the \l Q3ListView.
*/

/*!
    \property QStyleOptionListView::viewportBGRole
    \brief The background role of the viewport of the \l Q3ListView.

    \sa QWidget::backgroundRole()
*/

/*!
    \property QStyleOptionListView::sortColumn
    \brief The sort column of the listview.

    \sa Q3ListView::sortColumn()
*/

/*!
    \property QStyleOptionListView::itemMargin
    \brief The margin for items in the listview.

    \sa Q3ListView::itemMargin()
*/

/*!
    \property QStyleOptionListView::treeStepSize
    \brief The number of pixel to offset children items from their parents.

    \sa Q3ListView::treeStepSize()
*/

/*!
    \property QStyleOptionListView::rootIsDecorated
    \brief Whether root items are decorated or not

    \sa Q3ListView::rootIsDecorated()
*/

/*!
    \class QStyleOptionDockWindow
    \brief The QStyleOptionDockWindow describes parameters for drawing various
    parts of \l Q3DockWindows.

    This structure is used for drawing the old Q3DockWindow and its parts. It
    is not recommended for new classes.
*/

/*!
    \fn QStyleOptionDockWindow::QStyleOptionDockWindow(int version)
    Constructs a QStyleOptionDockWindow with version \a version.

    The values in the structure will be set to false. It is the responsibility
    of the developer to set them correctly.
*/

/*!
    \property QStyleOptionDockWindow::docked
    \brief Indicates that the dock window is currently docked.
*/

/*!
    \property QStyleOptionDockWindow::isCloseEnabled
    \brief Indicates that the dock window has a close button.
*/

/*!
    \class QStyleOptionToolButton qstyleoption.h
    \brief QStyleOptionToolButton describes the parameters for drawing a tool button.

    The QStyleOptionToolButton structure is used for drawing \l {QToolButton}s
*/

/*!
    \enum QStyleOptionToolButton::ToolButtonFeature
    Describes the various features that a tool button can have.

    \value None A normal tool button.
    \value Arrow The tool button is an arrow.
    \value TextLabel The tool button has a text label.
    \value Menu The tool button has a menu.
    \value PopupDelay There is a delay to showing the menu.
    \value BigPixmap The tool button uses big pixmaps.

    \sa features QToolButton::usesTextLabel(), QToolButton::popupDelay(), QToolButton::usesBigPixmap()
*/

/*!
    \fn QStyleOptionToolButton::QStyleOptionToolButton(int version)

    Construct a QStyleOptionToolButton with version \a version.

    The values of the structure are either zero, empty, or null. It is the
    responsibility of the developer to set them correctly.
*/

/*!
    \property QStyleOptionToolButton::features
    \brief The features of the tool button.

    This is a bitwise OR describing the features of the button.
    \sa ToolButtonFeature
*/

/*!
    \property QStyleOptionToolButton::icon
    \brief The iconset (if any) for the tool button.
*/

/*!
    \property QStyleOptionToolButton::text
    \brief The text (if any) of the tool button.

    Note that this is only used if the \l TextLabel feature is part of the features.
    \sa ToolButtonFeatures
*/

/*!
    \property QStyleOptionToolButton::arrowType
    \brief The direction of the arrow for the tool button

    Note that this is only used if the \l Arrow feature is part of the features.
    \sa ToolButtonFeatures
*/

/*!
    \property QStyleOptionToolButton::bgRole
    \brief The background role of the tool button
    \sa QWidget::backgroundRole()
*/

/*!
    \property QStyleOptionToolButton::parentBGRole
    \brief The background role of the parent of the tool button

    This can also be the same as bgRole if the button has no parent.
    \sa QWidget::backgroundRole()
*/

/*!
    \property QStyleOptionToolButton::parentPalette
    \brief The parent's palette (if the tool button has one).
*/

/*!
    \property QStyleOptionToolButton::pos
    \brief The position of the tool button
*/

/*!
    \property QStyleOptionToolButton::font
    \brief The font that is used for the text.

    Note that this is only used if the \l TextLabel feature is part of the features.
    \sa ToolButtonFeatures
*/

/*!
    \property QStyleOptionToolButton::textPosition
    \brief The position of the text in the tool button.

    Note that this is only used if the \l TextLabel feature is part of the features.
    \sa ToolButtonFeatures
*/

/*!
    \class QStyleOptionComboBox qstyleoption.h
    \brief QStyleOptionComboBox describes the parameter for drawing a combobox.

    The QStyleOptionComboBox structure is used for drawing QComboBox.
*/

/*!
    \fn QStyleOptionComboBox::QStyleOptionComboBox(int version)

    Create a QStyleOptionComboBox structure with version \a version

    The values of the structure are set to zero or null. It is the developer's
    responsibility to set them correctly.
*/

/*!
    \property QStyleOptionComboBox::editable
    \brief whether or not the combobox is editable or not.
    \sa QComboBox::isEditable()
*/

/*!
    \property QStyleOptionComboBox::popupRect
    \brief The popup rectangle for the combobox.
*/

/*!
    \class QStyleOptionToolBox qstyleoption.h
    \brief QStyleOptionToolBox describes the parameters needed for drawing a tool box.

    The QStyleOptionToolBox structure is used for drawing parts of the \l QToolBox.
*/

/*!
    \fn QStyleOptionToolBox::QStyleOptionToolBox(int version)
    Create a QStyleOptionToolBox structure with version \a version

    The values of the structure are set to zero, null, or empty. It is the
    developers responsibility to set them correctly.
*/

/*!
    \property QStyleOptionToolBox::icon
    \brief The iconset (if any) for the tool box tab.
*/

/*!
    \property QStyleOptionToolBox::text
    \brief The text (if any) for the tool box tab.
*/

/*!
    \property QStyleOptionToolBox::bgRole
    \brief The background role for the tool box tab.
*/

/*!
    \property QStyleOptionToolBox::currentWidgetBGRole
    \brief The background role for the current widget.

    This may be useful if you wish to draw the selected tab differently.
*/

/*!
    \property QStyleOptionToolBox::currentWidgetPalette
    \brief The palette for the current widget.

    This may be useful if you wish the draw the selected tab differently.
*/

/*!
    \class QStyleOptionTitleBar qstyleoption.h
    \brief QStyleOptionTitleBar describes the parameters for drawing a title bar.

    The QStyleOptionTitleBar structure is used to draw QTitleBar an internal
    class.
*/

/*!
    \fn QStyleOptionTitleBar::QStyleOptionTitleBar(int version)
    Construct a QStyleOptionTitleBar with version \a version.

    The values of the structure will be zero, null, or empty. It is the
    responsibility of the developer to set them correctly.
*/

/*!
    \property QStyleOptionTitleBar::text
    \brief The text of the title bar.
*/

/*!
    \property QStyleOptionTitleBar::icon
    \brief The icon for the title bar (if any).
*/

/*!
    \property QStyleOptionTitleBar::titleBarState
    \brief The state of the title bar.
    This is basically the window state of the underlying widget.
    \sa QWidget::windowState()
*/

/*!
    \property QStyleOptionTitleBar::titleBarFlags
    \brief The widget flags for the title bar.
    \sa Qt::WFlags
*/

/*!
    \class QStyleOptionViewItem qstyleoption.h
    \brief QStyleOptionViewItem describes parameters to draw an item in Interviews.

    The QStyleOptionViewItem structure is used by Interviews to draw its items.
*/

/*!
    \enum QStyleOptionViewItem::Position
    This enum describes the position of the item's decoration.
    \value Left
    \value Right
    \value Top
    \value Bottom

    \sa decorationPosition
*/

/*!
    \enum QStyleOptionViewItem::Size
    This enum describes the size of the item's decoration size.
    \value Small
    \value Large

    \sa decorationSize
*/

/*!
    \fn QStyleOptionViewItem::QStyleOptionViewItem(int version)
    Construct a QStyleOptionViewItem with version \a version.

    The values of the structure are set to zero. It is the responsibility of
    the developer to set them to the correct values.
*/

/*!
    \property QStyleOptionViewItem::displayAlignment
    \brief The alignment of the display value for the item.
*/

/*!
    \property QStyleOptionViewItem::decorationAlignment
    \brief The alignment of the decoration for the item.
*/

/*!
    \property QStyleOptionViewItem::decorationPosition
    \brief The position of the decoration for the item.

    \sa Position
*/

/*!
    \property QStyleOptionViewItem::decorationSize
    \brief The size ofthe decoration for the item.

    \sa Size
*/

/*!
    \fn T qt_cast(const QStyleOption *opt)
    Returns a T or 0 depending on the type of \a opt. This is equivalent of a dynamic_cast.
*/

/*!
    \fn T qt_cast(QStyleOption *opt)
    \overload
    This is the non-const version of the qt_cast taking \a opt.

    Returns a T or 0 depending on the type of \a opt. This is equivalent of a dynamic_cast.
*/
