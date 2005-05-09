/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qstyleoption.h"
#include "qapplication.h"
#ifdef Q_WS_MAC
# include "private/qt_mac_p.h"
#endif
#ifndef QT_NO_DEBUG
#include <qdebug.h>
#endif

/*!
    \class QStyleOption
    \brief The QStyleOption class stores the parameters used by QStyle functions.

    \ingroup appearance

    QStyleOption and its subclasses contain all the information that
    QStyle functions need to draw a graphical element.

    For performance reasons, there are few member functions and the
    access to the member variables is direct (i.e., using the \c . or
    \c -> operator). This low-level feel makes the structures
    straightforward to use and emphasizes that these are simply
    parameters used by the style functions.

    The caller of a QStyle function usually creates QStyleOption
    objects on the stack. This combined with Qt's extensive use of
    \l{implicit sharing} for types such as QString, QPalette, and
    QColor ensures that no memory allocation needlessly takes place.

    The following code snippet shows how to use a specific
    QStyleOption subclass to paint a push button:

    \code
        void MyPushButton::paintEvent()
        {
            QStyleOptionButton option;
            option.init(this);
            option.state = isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
            if (isDefault())
                option.features |= QStyleOptionButton::DefaultButton;
            option.text = text();
            option.icon = icon();

            QPainter painter(this);
            style().drawControl(QStyle::CE_PushButton, &option, &painter, this);
        }
    \endcode

    In our example, the control is a QStyle::CE_PushButton, and
    according to the QStyle::drawControl() documentation the
    corresponding class is QStyleOptionButton.

    When reimplementing QStyle functions that take a QStyleOption
    parameter, you often need to cast the QStyleOption to a subclass.
    For safety, you can use qstyleoption_cast<T>() to ensure that the pointer
    type is correct. For example:

    \code
        void MyStyle::drawPrimitive(PrimitiveElement element,
                                    const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget)
        {
            if (element == PE_FocusRect) {
                const QStyleOptionFocusRect *focusRectOption =
                        qstyleoption_cast<const QStyleOptionFocusRect *>(option);
                if (focusRectOption) {
                    ...
                }
            } else {
                ...
            }
        }
    \endcode

    qstyleoption_cast<T>() will return 0 if the object to which \c option
    points isn't of the correct type.

    \sa QStyle, QStylePainter
*/

/*!
    \enum QStyleOption::OptionType

    This enum is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In general you do not need
    to worry about this unless you want to create your own QStyleOption
    subclass and your own styles.

    \value SO_Default QStyleOption
    \value SO_FocusRect \l QStyleOptionFocusRect
    \value SO_Button \l QStyleOptionButton
    \value SO_Tab \l QStyleOptionTab
    \value SO_TabWidgetFrame \l QStyleOptionTabBarBase
    \value SO_TabBarBase \l QStyleOptionTabWidgetFrame
    \value SO_MenuItem \l QStyleOptionMenuItem
    \value SO_Complex \l QStyleOptionComplex
    \value SO_Slider \l QStyleOptionSlider
    \value SO_Frame \l QStyleOptionFrame
    \value SO_ProgressBar \l QStyleOptionProgressBar
    \value SO_Q3ListView \l QStyleOptionQ3ListView
    \value SO_Q3ListViewItem \l QStyleOptionQ3ListViewItem
    \value SO_Header \l QStyleOptionHeader
    \value SO_Q3DockWindow \l QStyleOptionQ3DockWindow
    \value SO_DockWidget \l QStyleOptionDockWidget
    \value SO_SpinBox \l QStyleOptionSpinBox
    \value SO_ToolButton \l QStyleOptionToolButton
    \value SO_ComboBox \l QStyleOptionComboBox
    \value SO_ToolBox \l QStyleOptionToolBox
    \value SO_RubberBand \l QStyleOptionRubberBand
    \value SO_TitleBar \l QStyleOptionTitleBar
    \value SO_ViewItem \l QStyleOptionViewItem (used in Interviews)
    \value SO_CustomBase Reserved for custom QStyleOptions;
                         all custom controls values must be above this value
    \value SO_ComplexCustomBase Reserved for custom QStyleOptions;
                         all custom complex controls values must be above this value
*/

/*!
    Constructs a QStyleOption with version \a version and type \a
    type.

    The version has no special meaning for QStyleOption; it can be
    used by subclasses to distinguish between different version of
    the same option type.

    The \l state member variable is initialized to
    QStyle::State_None.

    \sa version, type
*/

QStyleOption::QStyleOption(int version, int type)
    : version(version), type(type), state(QStyle::State_None),
      direction(QApplication::layoutDirection()), fontMetrics(QFont())
{
}


/*!
    Destroys the style option object.
*/
QStyleOption::~QStyleOption()
{
}

/*!
    Initializes the \l state, \l direction, \l rect, \l palette, and
    \l fontMetrics member variables based on \a widget.

    This function is provided only for convenience. You can also
    initialize the variables manually if you want.
*/
void QStyleOption::init(const QWidget *widget)
{
    state = QStyle::State_None;
    if (widget->isEnabled())
        state |= QStyle::State_Enabled;
    if (widget->hasFocus())
        state |= QStyle::State_HasFocus;
    if (widget->window()->testAttribute(Qt::WA_KeyboardFocusChange))
        state |= QStyle::State_KeyboardFocusChange;
    if (widget->underMouse())
        state |= QStyle::State_MouseOver;
    if (widget->window()->isActiveWindow())
        state |= QStyle::State_Active;

    direction = widget->layoutDirection();
    rect = widget->rect();
    palette = widget->palette();
    fontMetrics = widget->fontMetrics();
}

/*!
   Constructs a copy of \a other.
*/
QStyleOption::QStyleOption(const QStyleOption &other)
    : version(Version), type(Type), state(other.state),
      direction(other.direction), rect(other.rect), fontMetrics(other.fontMetrics),
      palette(other.palette)
{
}

/*!
    Assign \a other to this QStyleOption.
*/
QStyleOption &QStyleOption::operator=(const QStyleOption &other)
{
    state = other.state;
    direction = other.direction;
    rect = other.rect;
    fontMetrics = other.fontMetrics;
    palette = other.palette;
    return *this;
}

/*!
    \variable QStyleOption::Type

    Equals SO_Default.
*/

/*!
    \variable QStyleOption::Version

    Equals 1.
*/

/*!
    \variable QStyleOption::palette
    \brief the palette that should be used when painting the control
*/

/*!
    \variable QStyleOption::direction
    \brief the text layout direction that should be used when drawing text in the control
*/

/*!
    \variable QStyleOption::fontMetrics
    \brief the font metrics that should be used when drawing text in the control
*/

/*!
    \variable QStyleOption::rect
    \brief the area that should be used for various calculations and painting.

    This can have different meanings for different types of elements.
    For example, for \l QStyle::CE_PushButton it would be the
    rectangle for the entire button, while for \l
    QStyle::CE_PushButtonLabel it would be just the area for the push
    button label.
*/

/*!
    \variable QStyleOption::state
    \brief the style flags that are used when drawing the control

    \sa QStyle::drawPrimitive(), QStyle::drawControl(), QStyle::drawComplexControl(),
        QStyle::State
*/

/*!
    \variable QStyleOption::type
    \brief the option type of the style option

    \sa OptionType
*/

/*!
    \variable QStyleOption::version
    \brief the version of the style option

    This value can be used by subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast<T>(), you
    normally don't need to check it.
*/

/*!
    \class QStyleOptionFocusRect
    \brief The QStyleOptionFocusRect class is used to describe the
    parameters for drawing a focus rectangle with QStyle.
*/

/*!
    Constructs a QStyleOptionFocusRect. The members variables are
    initialized to default values.
*/

QStyleOptionFocusRect::QStyleOptionFocusRect()
    : QStyleOption(Version, SO_FocusRect)
{
    state |= QStyle::State_KeyboardFocusChange; // assume we had one, will be corrected in init()
}

/*!
    \internal
*/
QStyleOptionFocusRect::QStyleOptionFocusRect(int version)
    : QStyleOption(version, SO_FocusRect)
{
    state |= QStyle::State_KeyboardFocusChange;  // assume we had one, will be corrected in init()
}

/*!
    \variable QStyleOptionFocusRect::Type

    Equals SO_FocusRect.
*/

/*!
    \variable QStyleOptionFocusRect::Version

    Equals 1.
*/

/*!
    \fn QStyleOptionFocusRect::QStyleOptionFocusRect(const QStyleOptionFocusRect &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionFocusRect::backgroundColor
    \brief The background color on which the focus rectangle is being drawn.
*/

/*!
    \class QStyleOptionFrame
    \brief The QStyleOptionFrame class is used to describe the
    parameters for drawing a frame.

    QStyleOptionFrame is used for drawing several built-in Qt widget,
    including QFrame, QGroupBox, QLineEdit, and QMenu.
*/

/*!
    Constructs a QStyleOptionFrame. The members variables are
    initialized to default values.
*/

QStyleOptionFrame::QStyleOptionFrame()
    : QStyleOption(Version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionFrame::QStyleOptionFrame(int version)
    : QStyleOption(version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

/*!
    \fn QStyleOptionFrame::QStyleOptionFrame(const QStyleOptionFrame &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionFrame::Type

    Equals SO_Frame.
*/

/*!
    \variable QStyleOptionFrame::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionFrame::lineWidth
    \brief The line width for drawing the panel.
*/

/*!
    \variable QStyleOptionFrame::midLineWidth
    \brief The mid-line width for drawing the panel. This is usually used in
    drawing sunken or raised frames.
*/

/*!
    \class QStyleOptionHeader
    \brief The QStyleOptionHeader class is used to describe the
    parameters for drawing a header.

    The QStyleOptionHeader class is used for drawing the item views'
    header pane, header sort arrow, and header label.
*/

/*!
    Constructs a QStyleOptionHeader. The members variables are
    initialized to default values.
*/

QStyleOptionHeader::QStyleOptionHeader()
    : QStyleOption(QStyleOptionHeader::Version, SO_Header),
      section(0), textAlignment(0), iconAlignment(0),
      position(QStyleOptionHeader::Beginning),
      selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
      orientation(Qt::Horizontal)
{
}

/*!
    \internal
*/
QStyleOptionHeader::QStyleOptionHeader(int version)
    : QStyleOption(version, SO_Header),
      section(0), textAlignment(0), iconAlignment(0),
      position(QStyleOptionHeader::Beginning),
      selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
      orientation(Qt::Horizontal)
{
}

/*!
    \variable QStyleOptionHeader::orientation
    \brief the header's orientation (horizontal or vertical)

    \sa Qt::Orientation
*/

/*!
    \fn QStyleOptionHeader::QStyleOptionHeader(const QStyleOptionHeader &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionHeader::Type

    Equals SO_Header.
*/

/*!
    \variable QStyleOptionHeader::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionHeader::section
    \brief Which section of the header is being painted.
*/

/*!
    \variable QStyleOptionHeader::text
    \brief The text of the header.
*/

/*!
    \variable QStyleOptionHeader::textAlignment
    \brief The alignment flags for the text of the header.

    \sa Qt::Alignment
*/

/*!
    \variable QStyleOptionHeader::icon
    \brief The icon of the header.
*/

/*!
    \variable QStyleOptionHeader::iconAlignment
    \brief The alignment flags for the icon of the header.

    \sa Qt::Alignment
*/

/*!
    \variable QStyleOptionHeader::position
    \brief the section's position in relation to the other sections
*/

/*!
    \variable QStyleOptionHeader::selectedPosition
    \brief the section's position in relation to the selected section
*/

/*!
    \variable QStyleOptionHeader::sortIndicator
    \brief the direction the sort indicator should be drawn
*/

/*!
    \enum QStyleOptionHeader::SectionPosition

    This enum lets you know where the section's position is in relation to the other sections.

    \value Beginning At the beginining of the header
    \value Middle In the middle of the header
    \value End At the end of the header
    \value OnlyOneSection Only one header section
*/

/*!
    \enum QStyleOptionHeader::SelectedPosition

    This enum lets you know where the section's position is in relation to the selected section.

    \value NotAdjacent Not adjacent to the selected section
    \value NextIsSelected The next section is selected
    \value PreviousIsSelected The previous section is selected
    \value NextAndPreviousAreSelected Both the next and previous section are selected
*/

/*!
    \enum QStyleOptionHeader::SortIndicator

    Indicates which direction the sort indicator should be drawn
    \value None No sort indicator is needed
    \value SortUp Draw an up indicator
    \value SortDown Draw a down indicator
*/

/*!
    \class QStyleOptionButton
    \brief The QStyleOptionButton class is used to describe the
    parameters for drawing buttons.

    The QStyleOptionButton class is used to draw \l QPushButton, \l
    QCheckBox, and \l QRadioButton.

    \sa QStyleOptionToolButton
*/

/*!
    \enum QStyleOptionButton::ButtonFeature

    This enum describles the different types of features a push button can have.

    \value None	Indicates a normal push button.
    \value Flat Indicates a flat push button.
    \value HasMenu Indicates that the button has a drop down menu.
    \value DefaultButton Indicates that the button is a default button.
    \value AutoDefaultButton Indicates that the button is an auto default button.

    \sa features
*/

/*!
    Constructs a QStyleOptionButton. The members variables are
    initialized to default values.
*/

QStyleOptionButton::QStyleOptionButton()
    : QStyleOption(QStyleOptionButton::Version, SO_Button), features(None)
{
}

/*!
    \internal
*/
QStyleOptionButton::QStyleOptionButton(int version)
    : QStyleOption(version, SO_Button), features(None)
{
}

/*!
    \fn QStyleOptionButton::QStyleOptionButton(const QStyleOptionButton &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionButton::Type

    Equals SO_Button.
*/

/*!
    \variable QStyleOptionButton::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionButton::features
    \brief The features for the button

    This variable is a bitwise OR of the features that describe this button.

    \sa ButtonFeature
*/

/*!
    \variable QStyleOptionButton::text
    \brief The text of the button.
*/

/*!
    \variable QStyleOptionButton::icon
    \brief The icon of the button.
*/

/*!
    \class QStyleOptionTab
    \brief The QStyleOptionTab class is used to describe the
    parameters for drawing a tab bar.

    The QStyleOptionTab class is used for drawing \l QTabBar and the
    pane for \l QTabWidget.
*/

/*!
    Constructs a QStyleOptionTab object. The members variables are
    initialized to default values.
*/

QStyleOptionTab::QStyleOptionTab()
    : QStyleOption(QStyleOptionTab::Version, SO_Tab),
      shape(QTabBar::RoundedNorth),
      row(0),
      position(Beginning),
      selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

/*!
    \internal
*/
QStyleOptionTab::QStyleOptionTab(int version)
    : QStyleOption(version, SO_Tab),
      shape(QTabBar::RoundedNorth),
      row(0),
      position(Beginning),
      selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

/*!
    \fn QStyleOptionTab::QStyleOptionTab(const QStyleOptionTab &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionTab::Type

    Equals SO_Tab.
*/

/*!
    \variable QStyleOptionTab::Version

    Equals 1.
*/

/*! \enum QStyleOptionTab::TabPosition

    \value Beginning The tab is the first tab in the tab bar.
    \value Middle The tab is neither the first nor the last tab in the tab bar.
    \value End The tab is the last tab in the tab bar.
    \value OnlyOneTab The tab is both the first and the last tab in the tab bar.

    \sa position
*/

/*!
    \enum QStyleOptionTab::CornerWidget

    These flags indicate the corner widgets in a tab.

    \value NoCornerWidgets  There are no corner widgets
    \value LeftCornerWidget  Left corner widget
    \value RightCornerWidget Right corner widget

    \sa cornerWidgets
*/

/*! \enum QStyleOptionTab::SelectedPosition

    \value NotAdjacent The tab is not adjacent to a selected tab (or is the selected tab).
    \value NextIsSelected The next tab (typically the tab on the right) is selected.
    \value PreviousIsSelected The previous tab (typically the tab on the left) is selected.

    \sa selectedPosition
*/

/*!
    \variable QStyleOptionTab::selectedPosition

    \brief The position of the selected tab in relation to this tab. Some styles
    need to draw a tab differently depending on whether or not it is adjacent
    to the selected tab.
*/

/*!
    \variable QStyleOptionTab::cornerWidgets

    \brief Information on the cornerwidgets of the tab bar.

    \sa CornerWidget
*/


/*!
    \variable QStyleOptionTab::shape
    \brief The tab shape used to draw the tab.
    \sa QTabBar::Shape
*/

/*!
    \variable QStyleOptionTab::text
    \brief The text of the tab.
*/

/*!
    \variable QStyleOptionTab::icon
    \brief The icon for the tab.
*/

/*!
    \variable QStyleOptionTab::row
    \brief which row the tab is currently in

    0 indicates the front row.

    Currently this property can only be 0.
*/

/*!
    \variable QStyleOptionTab::position
    \brief the position of the tab in the tab bar
*/

/*!
    \class QStyleOptionProgressBar
    \brief The QStyleOptionProgressBar class is used to describe the
    parameters necessary for drawing a progress bar.

    The QStyleOptionProgressBar class is used to draw \l QProgressBar.
*/

/*!
    Constructs a QStyleOptionProgressBar. The members variables are
    initialized to default values.
*/

QStyleOptionProgressBar::QStyleOptionProgressBar()
    : QStyleOption(QStyleOptionProgressBar::Version, SO_ProgressBar),
      minimum(0), maximum(0), progress(0), textAlignment(0), textVisible(false)
{
}

/*!
    \internal
*/
QStyleOptionProgressBar::QStyleOptionProgressBar(int version)
    : QStyleOption(version, SO_ProgressBar),
      minimum(0), maximum(0), progress(0), textAlignment(0), textVisible(false)
{
}

/*!
    \fn QStyleOptionProgressBar::QStyleOptionProgressBar(const QStyleOptionProgressBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionProgressBar::Type

    Equals SO_ProgressBar.
*/

/*!
    \variable QStyleOptionProgressBar::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionProgressBar::minimum
    \brief The minimum value for the progress bar

    This is the minimum value in the progress bar.
    \sa QProgressBar::minimum
*/

/*!
    \variable QStyleOptionProgressBar::maximum
    \brief The maximum value for the progress bar

    This is the maximum value in the progress bar.
    \sa QProgressBar::maximum
*/

/*!
    \variable QStyleOptionProgressBar::text
    \brief The text for the progress bar.

    The progress bar text is usually just the progress expressed as a string.
    An empty string indicates that the progress bar has not started yet.

    \sa QProgressBar::text
*/

/*!
    \variable QStyleOptionProgressBar::textVisible
    \brief A flag indicating whether or not text is visible.

    If this flag is true then the text is visible. Otherwise, the text is not visible.

    \sa QProgressBar::textVisible
*/


/*!
    \variable QStyleOptionProgressBar::textAlignment
    \brief The text alignment for the text in the QProgressBar

    This can be used as a guide on where the text should be in the progressbar.
*/

/*!
    \variable QStyleOptionProgressBar::progress
    \brief the current progress for the progress bar.

    The current progress. A value of QStyleOptionProgressBar::minimum - 1
    indicates that the progress hasn't started yet.

    \sa QProgressBar::value
*/

/*!
    \class QStyleOptionMenuItem
    \brief The QStyleOptionMenuItem class is used to describe the
    parameter necessary for drawing a menu item.

    The QStyleOptionMenuItem is used for drawing menu items from \l
    QMenu. It is also used for drawing other menu-related widgets.
*/

/*!
    Constructs a QStyleOptionMenuItem. The members variables are
    initialized to default values.
*/

QStyleOptionMenuItem::QStyleOptionMenuItem()
    : QStyleOption(QStyleOptionMenuItem::Version, SO_MenuItem), menuItemType(Normal),
      checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionMenuItem::QStyleOptionMenuItem(int version)
    : QStyleOption(version, SO_MenuItem), menuItemType(Normal),
      checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \fn QStyleOptionMenuItem::QStyleOptionMenuItem(const QStyleOptionMenuItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionMenuItem::Type

    Equals SO_MenuItem.
*/

/*!
    \variable QStyleOptionMenuItem::Version

    Equals 1.
*/

/*!
    \enum QStyleOptionMenuItem::MenuItemType

    These values indicate the type of menu item that the structure describes.

    \value Normal A normal menu item.
    \value DefaultItem A menu item that is the default action as specified with \l QMenu::defaultAction().
    \value Separator A menu separator.
    \value SubMenu Indicates the menu item points to a sub-menu.
    \value Scroller A popup menu scroller (currently only used on Mac OS X).
    \value TearOff A tear-off handle for the menu.
    \value Margin The margin of the menu.
    \value EmptyArea The empty area of the menu.
*/

/*!
    \enum QStyleOptionMenuItem::CheckType

    These enums are used to indicate whether or not a check mark should be
    drawn for the item, or even if it should be drawn at all.

    \value NotCheckable The item is not checkable.
    \value Exclusive The item is an exclusive check item (like a radio button).
    \value NonExclusive The item is a non-exclusive check item (like a check box).

    \sa QAction::checkable, QAction::checked, QActionGroup::exclusive
*/

/*!
    \variable QStyleOptionMenuItem::menuItemType

    \brief the type of menu item

    \sa MenuItemType
*/

/*!
    \variable QStyleOptionMenuItem::checkType
    \brief The type of checkmark of the menu item
    \sa CheckType
*/

/*!
    \variable QStyleOptionMenuItem::checked
    \brief whether the menu item is checked or not.
*/

/*!
    \variable QStyleOptionMenuItem::menuHasCheckableItems
    \brief whether the menu as a whole has checkable items or not.

    If this option is set to false, then the menu has no checkable
    items. This makes it possible for GUI styles to save some
    horizontal space that would normally be used for the check column.
*/

/*!
    \variable QStyleOptionMenuItem::menuRect
    \brief The rectangle for the entire menu.
*/

/*!
    \variable QStyleOptionMenuItem::text
    \brief The text for the menu item.

    Note that the text format is something like this "Menu
    text\bold{\\t}Shortcut".

    If the menu item doesn't have a shortcut, it will just contain
    the menu item's text.
*/

/*!
    \variable QStyleOptionMenuItem::icon
    \brief The icon for the menu item.
*/

/*!
    \variable QStyleOptionMenuItem::maxIconWidth
    \brief the maximum icon width for the icon in the menu item.

    This can be used for drawing the icon into the correct place or
    properly aligning items. The variable must be set regardless of
    whether or not the menu item has an icon.
*/

/*!
    \variable QStyleOptionMenuItem::tabWidth
    \brief The tab width for the menu item.

    The tab width is the distance between the text of the menu item
    and the shortcut.
*/


/*!
    \variable QStyleOptionMenuItem::font
    \brief The font used for the menu item text.

    This is the font that should be used for drawing the menu text minus the
    shortcut. The shortcut is usually drawn using the painter's font.
*/

/*!
    \class QStyleOptionComplex
    \brief The QStyleOptionComplex class is used to hold parameters that are
    common to all complex controls.

    This class is not used on its own. Instead it is used to derive other
    complex control options, for example \l QStyleOptionSlider and
    \l QStyleOptionSpinBox.
*/

/*!
    Constructs a QStyleOptionComplex of type \a type and version \a
    version. Usually this constructor is called by subclasses.

    The \l subControls member is initialized to \l QStyle::SC_All.
    The \l activeSubControls member is initialized to \l
    QStyle::SC_None.
*/

QStyleOptionComplex::QStyleOptionComplex(int version, int type)
    : QStyleOption(version, type), subControls(QStyle::SC_All), activeSubControls(QStyle::SC_None)
{
}

/*!
    \fn QStyleOptionComplex::QStyleOptionComplex(const QStyleOptionComplex &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionComplex::Type

    Equals SO_Complex.
*/

/*!
    \variable QStyleOptionComplex::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionComplex::subControls
    \brief The sub-controls that need to be painted.

    This is a bitwise OR of the various sub-controls that need to be drawn for the complex control.

    \sa QStyle::SubControl
*/

/*!
    \variable QStyleOptionComplex::activeSubControls
    \brief The sub-controls that are active for the complex control.

    This a bitwise OR of the various sub-controls that are active (pressed) for the complex control.

    \sa QStyle::SubControl
*/

/*!
    \class QStyleOptionSlider
    \brief The QStyleOptionSlider class is used to describe the
    parameters needed for drawing a slider.

    The QStyleOptionSlider class is used for drawing \l QSlider and
    \l QScrollBar.
*/

/*!
    Constructs a QStyleOptionSlider. The members variables are
    initialized to default values.
*/

QStyleOptionSlider::QStyleOptionSlider()
    : QStyleOptionComplex(Version, SO_Slider), minimum(0), maximum(0),
      tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
      sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
      dialWrapping(false)
{
}

/*!
    \internal
*/
QStyleOptionSlider::QStyleOptionSlider(int version)
    : QStyleOptionComplex(version, SO_Slider), minimum(0), maximum(0),
      tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
      sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
      dialWrapping(false)
{
}

/*!
    \fn QStyleOptionSlider::QStyleOptionSlider(const QStyleOptionSlider &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionSlider::Type

    Equals SO_Slider.
*/

/*!
    \variable QStyleOptionSlider::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionSlider::orientation
    \brief the slider's orientation (horizontal or vertical)

    \sa Qt::Orientation
*/

/*!
    \variable QStyleOptionSlider::minimum
    \brief The minimum value for the slider.
*/

/*!
    \variable QStyleOptionSlider::maximum
    \brief The maximum value for the slider.
*/

/*!
    \variable QStyleOptionSlider::tickPosition
    \brief the position of the slider's tick marks, if any.

    \sa QSlider::TickPosition
*/

/*!
    \variable QStyleOptionSlider::tickInterval
    \brief The interval that should be drawn between tick marks.
*/

/*!
    \variable QStyleOptionSlider::notchTarget
    \brief The number of pixel between notches

    \sa QDial::notchTarget()
*/

/*!
    \variable QStyleOptionSlider::dialWrapping
    \brief Indicates whether or not the dial should wrap or not

    \sa QDial::wrapping()
*/

/*!
    \variable QStyleOptionSlider::upsideDown
    \brief Indicates slider control orientation.

    Normally a slider increases as it moves up or to the right; upsideDown
    indicates that it should do the opposite (increase as it moves down or to
    the left).

    \sa QStyle::sliderPositionFromValue(), QStyle::sliderValueFromPosition(),
        QAbstractSlider::invertedAppearance
*/

/*!
    \variable QStyleOptionSlider::sliderPosition
    \brief The position of the slider handle.

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same
    as \l sliderValue. Otherwise, it will have the current position
    of the handle.

    \sa QAbstractSlider::tracking, sliderValue
*/

/*!
    \variable QStyleOptionSlider::sliderValue
    \brief The value of the slider.

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same
    as \l sliderPosition. Otherwise, it will have the value the
    slider had before the mouse was pressed.

    \sa QAbstractSlider::tracking sliderPosition
*/

/*!
    \variable QStyleOptionSlider::singleStep
    \brief The size of the single step of the slider.

    \sa QAbstractSlider::singleStep
*/

/*!
    \variable QStyleOptionSlider::pageStep
    \brief The size of the page step of the slider.

    \sa QAbstractSlider::pageStep
*/

/*!
    \class QStyleOptionSpinBox
    \brief The QStyleOptionSpinBox class is used to describe the
    parameters necessary for drawing a spin box.

    The QStyleOptionSpinBox is used for drawing QSpinBox and QDateTimeEdit.
*/

/*!
    Constructs a QStyleOptionSpinBox. The members variables are
    initialized to default values.
*/

QStyleOptionSpinBox::QStyleOptionSpinBox()
    : QStyleOptionComplex(Version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
      stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

/*!
    \internal
*/
QStyleOptionSpinBox::QStyleOptionSpinBox(int version)
    : QStyleOptionComplex(version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
      stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

/*!
    \fn QStyleOptionSpinBox::QStyleOptionSpinBox(const QStyleOptionSpinBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionSpinBox::Type

    Equals SO_SpinBox.
*/

/*!
    \variable QStyleOptionSpinBox::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionSpinBox::buttonSymbols
    \brief The type of button symbols to draw for the spin box.

    \sa QAbstractSpinBox::ButtonSymbols
*/

/*!
    \variable QStyleOptionSpinBox::stepEnabled
    \brief Indicates which buttons of the spin box are enabled.

    \sa QAbstractSpinBox::StepEnabled
*/

/*!
    \variable QStyleOptionSpinBox::frame
    \brief Indicates whether whether the spin box has a frame.

*/

/*!
    \class QStyleOptionQ3ListViewItem
    \brief The QStyleOptionQ3ListViewItem class is used to describe an
    item drawn in a Q3ListView.

    This is used by the compatibility Q3ListView to draw its items.
    It should be avoided for new classes.

    \sa Q3ListView, Q3ListViewItem
*/

/*!
    \enum QStyleOptionQ3ListViewItem::Q3ListViewItemFeature

    This enum describes the features a list view item can have.

    \value None A standard item.
    \value Expandable The item has children that can be shown.
    \value MultiLine The item is more than one line tall.
    \value Visible The item is visible.
    \value ParentControl The item's parent is a type of item control (Q3CheckListItem::Controller).

    \sa features, Q3ListViewItem::isVisible(), Q3ListViewItem::multiLinesEnabled(),
        Q3ListViewItem::isExpandable()
*/

/*!
    Constructs a QStyleOptionQ3ListViewItem. The members variables are
    initialized to default values.
*/

QStyleOptionQ3ListViewItem::QStyleOptionQ3ListViewItem()
    : QStyleOption(Version, SO_Q3ListViewItem), features(None), height(0), totalHeight(0),
      itemY(0), childCount(0)
{
}

/*!
    \internal
*/
QStyleOptionQ3ListViewItem::QStyleOptionQ3ListViewItem(int version)
    : QStyleOption(version, SO_Q3ListViewItem), features(None), height(0), totalHeight(0),
      itemY(0), childCount(0)
{
}

/*!
    \fn QStyleOptionQ3ListViewItem::QStyleOptionQ3ListViewItem(const QStyleOptionQ3ListViewItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionQ3ListViewItem::Type

    Equals SO_Q3ListViewItem.
*/

/*!
    \variable QStyleOptionQ3ListViewItem::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionQ3ListViewItem::features
    \brief The features for this item

    This variable is a bitwise OR of the features of the item.

    \sa Q3ListViewItemFeature
*/

/*!
    \variable QStyleOptionQ3ListViewItem::height
    \brief The height of the item

    This doesn't include the height of the item's children.

    \sa Q3ListViewItem::height()
*/

/*!
    \variable QStyleOptionQ3ListViewItem::totalHeight
    \brief The total height of the item, including its children

    \sa Q3ListViewItem::totalHeight()
*/

/*!
    \variable QStyleOptionQ3ListViewItem::itemY
    \brief The Y-coordinate for the item

    \sa Q3ListViewItem::itemPos()
*/

/*!
    \variable QStyleOptionQ3ListViewItem::childCount
    \brief The number of children the item has.
*/

/*!
    \class QStyleOptionQ3ListView
    \brief The QStyleOptionQ3ListView class is used to describe the
    parameters for drawing a Q3ListView.

    The class is used for drawing the compat \l Q3ListView. It is not
    recommended for use in new code.
*/

/*!
    Creates a QStyleOptionQ3ListView. The members variables are
    initialized to default values.
*/

QStyleOptionQ3ListView::QStyleOptionQ3ListView()
    : QStyleOptionComplex(Version, SO_Q3ListView), sortColumn(0), itemMargin(0), treeStepSize(0),
      rootIsDecorated(false)
{
}

/*!
    \internal
*/
QStyleOptionQ3ListView::QStyleOptionQ3ListView(int version)
    : QStyleOptionComplex(version, SO_Q3ListView), sortColumn(0), itemMargin(0), treeStepSize(0),
      rootIsDecorated(false)
{
}

/*!
    \fn QStyleOptionQ3ListView::QStyleOptionQ3ListView(const QStyleOptionQ3ListView &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionQ3ListView::Type

    Equals SO_Q3ListView.
*/

/*!
    \variable QStyleOptionQ3ListView::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionQ3ListView::items
    \brief A list of items in the \l Q3ListView.

    This is a list of \l {QStyleOptionQ3ListViewItem}s. The first item
    can be used for most of the calculation that are needed for
    drawing a list view. Any additional items are the children of
    this first item, which may be used for additional information.

    \sa QStyleOptionQ3ListViewItem
*/

/*!
    \variable QStyleOptionQ3ListView::viewportPalette
    \brief The palette of Q3ListView's viewport.
*/

/*!
    \variable QStyleOptionQ3ListView::viewportBGRole
    \brief The background role of \l Q3ListView's viewport.

    \sa QWidget::backgroundRole()
*/

/*!
    \variable QStyleOptionQ3ListView::sortColumn
    \brief The sort column of the list view.

    \sa Q3ListView::sortColumn()
*/

/*!
    \variable QStyleOptionQ3ListView::itemMargin
    \brief The margin for items in the list view.

    \sa Q3ListView::itemMargin()
*/

/*!
    \variable QStyleOptionQ3ListView::treeStepSize
    \brief The number of pixel to offset children items from their parents.

    \sa Q3ListView::treeStepSize()
*/

/*!
    \variable QStyleOptionQ3ListView::rootIsDecorated
    \brief Whether root items are decorated

    \sa Q3ListView::rootIsDecorated()
*/

/*!
    \class QStyleOptionQ3DockWindow
    \brief The QStyleOptionQ3DockWindow class is used to describe the
    parameters for drawing various parts of a \l Q3DockWindow.

    This class is used for drawing the old Q3DockWindow and its
    parts. It is not recommended for new classes.
*/

/*!
    Constructs a QStyleOptionQ3DockWindow. The member variables are
    initialized to default values.
*/

QStyleOptionQ3DockWindow::QStyleOptionQ3DockWindow()
    : QStyleOption(Version, SO_Q3DockWindow), docked(false), closeEnabled(false)
{
}

/*!
    \internal
*/
QStyleOptionQ3DockWindow::QStyleOptionQ3DockWindow(int version)
    : QStyleOption(version, SO_Q3DockWindow), docked(false), closeEnabled(false)
{
}

/*!
    \fn QStyleOptionQ3DockWindow::QStyleOptionQ3DockWindow(const QStyleOptionQ3DockWindow &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionQ3DockWindow::Type

    Equals SO_Q3DockWindow.
*/

/*!
    \variable QStyleOptionQ3DockWindow::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionQ3DockWindow::docked
    \brief Indicates that the dock window is currently docked.
*/

/*!
    \variable QStyleOptionQ3DockWindow::closeEnabled
    \brief Indicates that the dock window has a close button.
*/

/*!
    \class QStyleOptionDockWidget
    \brief The QStyleOptionDockWidget class is used to describe the
    parameters for drawing a dock window.
*/

/*!
    Constructs a QStyleOptionDockWidget. The member variables are
    initialized to default values.
*/

QStyleOptionDockWidget::QStyleOptionDockWidget()
    : QStyleOption(Version, SO_DockWidget), movable(false)
{
}

/*!
    \internal
*/
QStyleOptionDockWidget::QStyleOptionDockWidget(int version)
    : QStyleOption(version, SO_DockWidget), closable(false),
      movable(false), floatable(false)
{
}

/*!
    \fn QStyleOptionDockWidget::QStyleOptionDockWidget(const QStyleOptionDockWidget &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionDockWidget::Type

    Equals SO_DockWidget.
*/

/*!
    \variable QStyleOptionDockWidget::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionDockWidget::title
    \brief The title of the dock window
*/

/*!
    \variable QStyleOptionDockWidget::closable
    \brief Indicates that the dock window is closable.
*/

/*!
    \variable QStyleOptionDockWidget::movable
    \brief Indicates that the dock window is movable.
*/

/*!
    \variable QStyleOptionDockWidget::floatable
    \brief Indicates that the dock window is floatable.
*/

/*!
    \class QStyleOptionToolButton
    \brief The QStyleOptionToolButton class is used to describe the
    parameters for drawing a tool button.

    The QStyleOptionToolButton class is used for drawing QToolButton.
*/

/*!
    \enum QStyleOptionToolButton::ToolButtonFeature
    Describes the various features that a tool button can have.

    \value None A normal tool button.
    \value Arrow The tool button is an arrow.
    \value Menu The tool button has a menu.
    \value PopupDelay There is a delay to showing the menu.

    \sa features, QToolButton::toolButtonStyle(), QToolButton::popupMode()
*/

/*!
    Constructs a QStyleOptionToolButton. The members variables are
    initialized to default values.
*/

QStyleOptionToolButton::QStyleOptionToolButton()
    : QStyleOptionComplex(Version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
    , toolButtonStyle(Qt::ToolButtonIconOnly)
{
}

/*!
    \internal
*/
QStyleOptionToolButton::QStyleOptionToolButton(int version)
    : QStyleOptionComplex(version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
    , toolButtonStyle(Qt::ToolButtonIconOnly)

{
}

/*!
    \fn QStyleOptionToolButton::QStyleOptionToolButton(const QStyleOptionToolButton &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionToolButton::Type

    Equals SO_ToolButton.
*/

/*!
    \variable QStyleOptionToolButton::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionToolButton::features
    \brief The features of the tool button.

    This variable is a bitwise OR describing the features of the button.

    \sa ToolButtonFeature
*/

/*!
    \variable QStyleOptionToolButton::icon
    \brief The icon for the tool button.

    \sa QToolButton::iconSize()
*/

/*!
    \variable QStyleOptionToolButton::iconSize
    \brief the size of the icon for the tool button
*/

/*!
    \variable QStyleOptionToolButton::text
    \brief The text of the tool button.

    This value is only used if toolButtonStyle is Qt::ToolButtonTextUnderIcon,
    Qt::ToolButtonTextBesideIcon, or Qt::ToolButtonTextOnly
*/

/*!
    \variable QStyleOptionToolButton::arrowType
    \brief The direction of the arrow for the tool button

    This value is only used if \l features includes \l Arrow.
*/

/*!
    \variable QStyleOptionToolButton::toolButtonStyle
    \brief Used to describe the appearance of a tool button

    \sa QToolButton::toolButtonStyle()
*/

/*!
    \variable QStyleOptionToolButton::pos
    \brief The position of the tool button
*/

/*!
    \variable QStyleOptionToolButton::font
    \brief The font that is used for the text.

    This value is only used if toolButtonStyle is Qt::ToolButtonTextUnderIcon,
    Qt::ToolButtonTextBesideIcon, or Qt::ToolButtonTextOnly
*/

/*!
    \class QStyleOptionComboBox
    \brief The QStyleOptionComboBox class is used to describe the
    parameter for drawing a combobox.

    The QStyleOptionComboBox class is used for drawing QComboBox.
*/

/*!
    Creates a QStyleOptionComboBox. The members variables are
    initialized to default values.
*/

QStyleOptionComboBox::QStyleOptionComboBox()
    : QStyleOptionComplex(Version, SO_ComboBox), editable(false), frame(true)
{
}

/*!
    \internal
*/
QStyleOptionComboBox::QStyleOptionComboBox(int version)
    : QStyleOptionComplex(version, SO_ComboBox), editable(false), frame(true)
{
}

/*!
    \fn QStyleOptionComboBox::QStyleOptionComboBox(const QStyleOptionComboBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionComboBox::Type

    Equals SO_ComboBox.
*/

/*!
    \variable QStyleOptionComboBox::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionComboBox::editable
    \brief whether or not the combobox is editable or not.

    \sa QComboBox::isEditable()
*/


/*!
    \variable QStyleOptionComboBox::frame
    \brief Indicates whether whether the combo box has a frame.
*/

/*!
    \variable QStyleOptionComboBox::currentText
    \brief The text for the current item of the combo box
*/

/*!
    \variable QStyleOptionComboBox::currentIcon
    \brief The icon for the current item of the combo box
*/

/*!
    \variable QStyleOptionComboBox::iconSize
    \brief The icon size for the current item of the combo box
*/

/*!
    \variable QStyleOptionComboBox::popupRect
    \brief The popup rectangle for the combobox.
*/

/*!
    \class QStyleOptionToolBox
    \brief The QStyleOptionToolBox class is used to describe the
    parameters needed for drawing a tool box.

    The QStyleOptionToolBox class is used for drawing QToolBox.
*/

/*!
    Creates a QStyleOptionToolBox. The members variables are
    initialized to default values.
*/

QStyleOptionToolBox::QStyleOptionToolBox()
    : QStyleOption(Version, SO_ToolBox)
{
}

/*!
    \internal
*/
QStyleOptionToolBox::QStyleOptionToolBox(int version)
    : QStyleOption(version, SO_ToolBox)
{
}

/*!
    \fn QStyleOptionToolBox::QStyleOptionToolBox(const QStyleOptionToolBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionToolBox::Type

    Equals SO_ToolBox.
*/

/*!
    \variable QStyleOptionToolBox::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionToolBox::icon
    \brief The icon for the tool box tab.
*/

/*!
    \variable QStyleOptionToolBox::text
    \brief The text for the tool box tab.
*/


/*!
    \class QStyleOptionRubberBand
    \brief The QStyleOptionRubberBand class is used to describe the
    parameters needed for drawing a rubber band.

    The QStyleOptionRubberBand class is used for drawing QRubberBand.
*/

/*!
    Creates a QStyleOptionRubberBand. The members variables are
    initialized to default values.
*/

QStyleOptionRubberBand::QStyleOptionRubberBand()
    : QStyleOption(Version, SO_RubberBand)
{
}

/*!
    \internal
*/
QStyleOptionRubberBand::QStyleOptionRubberBand(int version)
    : QStyleOption(version, SO_RubberBand)
{
}

/*!
    \fn QStyleOptionRubberBand::QStyleOptionRubberBand(const QStyleOptionRubberBand &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionRubberBand::Type

    Equals SO_RubberBand.
*/

/*!
    \variable QStyleOptionRubberBand::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionRubberBand::shape
    \brief The shape of the rubber band.
*/

/*!
    \variable QStyleOptionRubberBand::opaque
    \brief Whether the rubber band is required to be drawn in an opque style.
*/

/*!
    \class QStyleOptionTitleBar
    \brief The QStyleOptionTitleBar class is used to describe the
    parameters for drawing a title bar.

    The QStyleOptionTitleBar class is used to draw the title bars of
    QWorkspace's MDI children.
*/

/*!
    Constructs a QStyleOptionTitleBar. The members variables are
    initialized to default values.
*/

QStyleOptionTitleBar::QStyleOptionTitleBar()
    : QStyleOptionComplex(Version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}

/*!
    \fn QStyleOptionTitleBar::QStyleOptionTitleBar(const QStyleOptionTitleBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionTitleBar::Type

    Equals SO_TitleBar.
*/

/*!
    \variable QStyleOptionTitleBar::Version

    Equals 1.
*/

/*!
    \internal
*/
QStyleOptionTitleBar::QStyleOptionTitleBar(int version)
    : QStyleOptionComplex(version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}


/*!
    \variable QStyleOptionTitleBar::text
    \brief The text of the title bar.
*/

/*!
    \variable QStyleOptionTitleBar::icon
    \brief The icon for the title bar.
*/

/*!
    \variable QStyleOptionTitleBar::titleBarState
    \brief The state of the title bar.

    This is basically the window state of the underlying widget.

    \sa QWidget::windowState()
*/

/*!
    \variable QStyleOptionTitleBar::titleBarFlags
    \brief The widget flags for the title bar.

    \sa Qt::WFlags
*/

/*!
    \class QStyleOptionViewItem
    \brief The QStyleOptionViewItem class is used to describe the
    parameters used to draw an item in a view widget.

    The QStyleOptionViewItem class is used by Qt's model/view classes
    to draw their items.

    \sa {model-view-programming.html}{Model/View Programming}
*/

/*!
    \enum QStyleOptionViewItem::Position

    This enum describes the position of the item's decoration.

    \value Left On the left of the text.
    \value Right On the right of the text.
    \value Top Above the text.
    \value Bottom Below the text.

    \sa decorationPosition
*/

/*!
    Constructs a QStyleOptionViewItem. The members variables are
    initialized to default values.
*/

QStyleOptionViewItem::QStyleOptionViewItem()
    : QStyleOption(Version, SO_ViewItem),
      displayAlignment(0), decorationAlignment(0),
      textElideMode(Qt::ElideMiddle), decorationPosition(Left),
      showDecorationSelected(false)
{
}

/*!
    \internal
*/
QStyleOptionViewItem::QStyleOptionViewItem(int version)
    : QStyleOption(version, SO_ViewItem),
      displayAlignment(0), decorationAlignment(0),
      textElideMode(Qt::ElideMiddle), decorationPosition(Left),
      showDecorationSelected(false)
{
}

/*!
    \fn QStyleOptionViewItem::QStyleOptionViewItem(const QStyleOptionViewItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionViewItem::Type

    Equals SO_ViewItem.
*/

/*!
    \variable QStyleOptionViewItem::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionViewItem::displayAlignment
    \brief The alignment of the display value for the item.
*/

/*!
    \variable QStyleOptionViewItem::decorationAlignment
    \brief The alignment of the decoration for the item.
*/

/*!
    \variable QStyleOptionViewItem::decorationPosition
    \brief The position of the decoration for the item.

    \sa Position
*/

/*!
    \variable QStyleOptionViewItem::decorationSize
    \brief The size of the decoration for the item.

    \sa decorationAlignment, decorationPosition
*/

/*!
    \variable QStyleOptionViewItem::font
    \brief The font used for the item

    \sa QFont
*/

/*!
    \fn T qstyleoption_cast<T>(const QStyleOption *option)
    \relates QStyleOption

    Returns a T or 0 depending on the \l{QStyleOption::type}{type}
    and \l{QStyleOption::version}{version} of \a option.

    Example:

    \code
        void MyStyle::drawPrimitive(PrimitiveElement element,
                                    const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget)
        {
            if (element == PE_FocusRect) {
                const QStyleOptionFocusRect *focusRectOption =
                        qstyleoption_cast<const QStyleOptionFocusRect *>(option);
                if (focusRectOption) {
                    ...
                }
            }
            ...
        }
    \endcode

    \sa QStyleOption::type, QStyleOption::version
*/

/*!
    \fn T qstyleoption_cast<T>(QStyleOption *option)
    \overload
    \relates QStyleOption

    Returns a T or 0 depending on the type of \a option.
*/

/*!
    \class QStyleOptionTabWidgetFrame
    \brief The QStyleOptionTabWidgetFrame class is used to describe the
    parameters for drawing the frame around a tab widget.

    QStyleOptionTabWidgetFrame is used for drawing QTabWidget.
*/

/*!
    Constructs a QStyleOptionTabWidgetFrame. The members variables
    are initialized to default values.
*/
QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame()
    : QStyleOption(Version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
      shape(QTabBar::RoundedNorth)
{
}

/*! \internal */
QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame(int version)
    : QStyleOption(version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
      shape(QTabBar::RoundedNorth)
{
}

/*!
    \variable QStyleOptionTabWidgetFrame::Type

    Equals SO_TabWidgetFrame.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::lineWidth
    \brief The line width for drawing the panel.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::midLineWidth
    \brief The mid-line width for drawing the panel. This is usually used in
    drawing sunken or raised frames.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::shape
    \brief The tab shape used to draw the tabs.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::tabBarSize
    \brief The size of the tab bar.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::rightCornerWidgetSize
    \brief The size of the right-corner widget.
*/

/*! \variable QStyleOptionTabWidgetFrame::leftCornerWidgetSize
    \brief The size of the left-corner widget.
*/

QStyleOptionTabBarBase::QStyleOptionTabBarBase()
    : QStyleOption(Version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

/*! \internal */
QStyleOptionTabBarBase::QStyleOptionTabBarBase(int version)
    : QStyleOption(version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

/*!
    \variable QStyleOptionTabBarBase::Type

    Equals SO_TabBarBase.
*/

/*!
    \variable QStyleOptionTabBarBase::Version

    Equals 1.
*/

/*!
    \class QStyleHintReturn
    \brief The QStyleHintReturn class provides style hints that return more
    than basic data types.

    \ingroup appearance

    QStyleHintReturn and its subclasses are used to pass information
    from a style back to the querying widget. This is most useful
    when the return value from QStyle::styleHint() does not provide enough
    detail; for example, when a mask is to be returned.

    \omit
    ### --Sam
    \endomit
*/

/*!
    \enum QStyleHintReturn::HintReturnType

    \value SH_Default QStyleHintReturn
    \value SH_Mask \l QStyle::SH_RubberBand_Mask QStyle::SH_FocusFrame_Mask
*/

/*!
    \variable QStyleHintReturn::Type

    Equals SH_Default.
*/

/*!
    \variable QStyleHintReturn::Version

    Equals 1.
*/

/*!
    \variable QStyleHintReturn::type
    \brief the type of the style hint container

    \sa HintReturnType
*/

/*!
    \variable QStyleHintReturn::version
    \brief the version of the style hint return container

    This value can be used by subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast<T>(), you
    normally don't need to check it.
*/

/*!
    Constructs a QStyleHintReturn with version \a version and type \a
    type.

    The version has no special meaning for QStyleHintReturn; it can be
    used by subclasses to distinguish between different version of
    the same hint type.

    \sa QStyleOption::version, QStyleOption::type
*/

QStyleHintReturn::QStyleHintReturn(int version, int type)
    : version(version), type(type)
{
}

/*!
    \internal
*/

QStyleHintReturn::~QStyleHintReturn()
{

}

/*!
    \class QStyleHintReturnMask
    \brief The QStyleHintReturnMask class provides style hints that return a QRegion.

    \ingroup appearance

    \omit
    ### --Sam
    \endomit
*/

/*!
    Constructs a QStyleHintReturnMask. The member variables are
    initialized to default values.
*/
QStyleHintReturnMask::QStyleHintReturnMask() : QStyleHintReturn(Version, Type)
{
}

/*!
    \variable QStyleHintReturnMask::Type

    Equals SH_Mask.
*/

/*!
    \variable QStyleHintReturnMask::Version

    Equals 1.
*/

/*!
    \fn T qstyleoption_cast<T>(const QStyleHintReturn *hint)
    \relates QStyleHintReturn

    Returns a T or 0 depending on the \l{QStyleHintReturn::type}{type}
    and \l{QStyleHintReturn::version}{version} of \a hint.

    Example:

    \code
        int MyStyle::styleHint(StyleHint stylehint, const QStyleOption *opt,
                               const QWidget *widget, QStyleHintReturn* returnData) const;
        {
            if (stylehint == SH_RubberBand_Mask) {
                const QStyleHintReturnMask *maskReturn =
                        qstyleoption_cast<const QStyleHintReturnMask *>(hint);
                if (maskReturn) {
                    ...
                }
            }
            ...
        }
    \endcode

    \sa QStyleHintReturn::type, QStyleHintReturn::version
*/

/*!
    \fn T qstyleoption_cast<T>(QStyleHintReturn *hint)
    \overload
    \relates QStyleHintReturn

    Returns a T or 0 depending on the type of \a hint.
*/

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug debug, const QStyleOption::OptionType &optionType)
{
    switch (optionType) {
    case QStyleOption::SO_Default:
        debug << "SO_Default"; break;
    case QStyleOption::SO_FocusRect:
        debug << "SO_FocusRect"; break;
    case QStyleOption::SO_Button:
        debug << "SO_Button"; break;
    case QStyleOption::SO_Tab:
        debug << "SO_Tab"; break;
    case QStyleOption::SO_MenuItem:
        debug << "SO_MenuItem"; break;
    case QStyleOption::SO_Frame:
        debug << "SO_Frame"; break;
    case QStyleOption::SO_ProgressBar:
        debug << "SO_ProgressBar"; break;
    case QStyleOption::SO_ToolBox:
        debug << "SO_ToolBox"; break;
    case QStyleOption::SO_Header:
        debug << "SO_Header"; break;
    case QStyleOption::SO_Q3DockWindow:
        debug << "SO_Q3DockWindow"; break;
    case QStyleOption::SO_DockWidget:
        debug << "SO_DockWidget"; break;
    case QStyleOption::SO_Q3ListViewItem:
        debug << "SO_Q3ListViewItem"; break;
    case QStyleOption::SO_ViewItem:
        debug << "SO_ViewItem"; break;
    case QStyleOption::SO_TabWidgetFrame:
        debug << "SO_TabWidgetFrame"; break;
    case QStyleOption::SO_TabBarBase:
        debug << "SO_TabBarBase"; break;
    case QStyleOption::SO_RubberBand:
        debug << "SO_RubberBand"; break;
    case QStyleOption::SO_Complex:
        debug << "SO_Complex"; break;
    case QStyleOption::SO_Slider:
        debug << "SO_Slider"; break;
    case QStyleOption::SO_SpinBox:
        debug << "SO_SpinBox"; break;
    case QStyleOption::SO_ToolButton:
        debug << "SO_ToolButton"; break;
    case QStyleOption::SO_ComboBox:
        debug << "SO_ComboBox"; break;
    case QStyleOption::SO_Q3ListView:
        debug << "SO_Q3ListView"; break;
    case QStyleOption::SO_TitleBar:
        debug << "SO_TitleBar"; break;
    case QStyleOption::SO_CustomBase:
        debug << "SO_CustomBase"; break;
    case QStyleOption::SO_ComplexCustomBase:
        debug << "SO_ComplexCustomBase"; break;
        break;
    }
    return debug;
}

QDebug operator<<(QDebug debug, const QStyleOption &option)
{
    debug << "QStyleOption(";
    debug << QStyleOption::OptionType(option.type);
    debug << "," << (option.direction == Qt::RightToLeft ? "RightToLeft" : "LeftToRight");
    debug << "," << option.state;
    debug << "," << option.rect;
    debug << ")";
    return debug;
}
#endif
