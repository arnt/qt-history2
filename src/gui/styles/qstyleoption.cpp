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
            option.state = isDown() ? QStyle::State_Down : QStyle::State_Raised;
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
    \value SO_TabWidgetFrame \l QStyleOptionTabWidgetFrame
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
    if (widget->underMouse())
        state |= QStyle::State_MouseOver;
    if (widget->window()->isActiveWindow())
        state |= QStyle::State_Active;

    direction = widget->layoutDirection();
    rect = widget->rect();
    palette = widget->palette();
    fontMetrics = widget->fontMetrics();
}

QStyleOption::QStyleOption(const QStyleOption &other)
    : version(Version), type(Type), state(other.state),
      direction(other.direction), rect(other.rect), fontMetrics(other.fontMetrics),
      palette(other.palette)
{
}

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
    \property QStyleOption::palette
    \brief the palette that should be used in when painting the control
*/

/*!
    \property QStyleOption::rect
    \brief the area that should be used for various calculations and painting.

    This can have different meanings for different types of elements.
    For example, for \l QStyle::CE_PushButton it would be the
    rectangle for the entire button, while for \l
    QStyle::CE_PushButtonLabel it would be just the area for the push
    button label.
*/

/*!
    \property QStyleOption::state
    \brief the style flags that are used when drawing the control

    \sa QStyle::drawPrimitive(), QStyle::drawControl(), QStyle::drawComplexControl(),
        QStyle::StyleFlags
*/

/*!
    \property QStyleOption::type
    \brief the option type of the style option

    \sa OptionType
*/

/*!
    \property QStyleOption::version
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
}

/*!
    \internal
*/
QStyleOptionFocusRect::QStyleOptionFocusRect(int version)
    : QStyleOption(version, SO_FocusRect)
{
}

/*!
    \property QStyleOptionFocusRect::backgroundColor
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
    \property QStyleOptionFrame::lineWidth
    \brief The line width for drawing the panel.
*/

/*!
    \property QStyleOptionFrame::midLineWidth
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
      selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None)
{
}

/*!
    \internal
*/
QStyleOptionHeader::QStyleOptionHeader(int version)
    : QStyleOption(version, SO_Header),
      section(0), textAlignment(0), iconAlignment(0),
      position(QStyleOptionHeader::Beginning),
      selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None)
{
}


/*!
    \property QStyleOptionHeader::section
    \brief Which section of the header is being painted.
*/

/*!
    \property QStyleOptionHeader::text
    \brief The text of the header.
*/

/*!
    \property QStyleOptionHeader::textAlignment
    \brief The alignment flags for the text of the header.

    \sa Qt::Alignment
*/

/*!
    \property QStyleOptionHeader::icon
    \brief The icon of the header.
*/

/*!
    \property QStyleOptionHeader::iconAlignment
    \brief The alignment flags for the icon of the header.

    \sa Qt::Alignment
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
    \property QStyleOptionButton::features
    \brief The features for the button

    This variable is a bitwise OR of the features that describe this button.

    \sa ButtonFeature
*/

/*!
    \property QStyleOptionButton::text
    \brief The text of the button.
*/

/*!
    \property QStyleOptionButton::icon
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

/*! \enum QStyleOptionTab::TabPosition

    \value Beginning The tab is the first tab in the tab bar.
    \value Middle The tab is neither the first nor the last tab in the tab bar.
    \value End The tab is the last tab in the tab bar.
    \value OnlyOneTab The tab is both the first and the last tab in the tab bar.

    \sa position
*/


/*! \enum QStyleOptionTab::SelectedPosition

    \value NotAdjacent The tab is not adjacent to a selected tab (or is the selected tab).
    \value NextIsSelected The next tab (typically the tab on the right) is selected.
    \value PreviousIsSelected The previous tab (typically the tab on the left) is selected.

    \sa selectedPosition
*/

/*!
    \property QStyleOptionTab::selectedPosition

    \brief The position of the selected tab in relation to this tab. Some styles
    need to draw a tab differently depending on whether or not it is adjacent
    to the selected tab.
*/

/*!
    \property QStyleOptionTab::shape
    \brief The tab shape used to draw the tab.
    \sa QTabBar::Shape
*/

/*!
    \property QStyleOptionTab::text
    \brief The text of the tab.
*/

/*!
    \property QStyleOptionTab::icon
    \brief The icon for the tab.
*/

/*!
    \property QStyleOptionTab::row
    \brief which row the tab is currently in

    0 indicates the front row.

    Currently this property can only be 0.
*/

/*!
    \property QStyleOptionTab::position
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
    \property QStyleOptionProgressBar::minimum
    \brief The minimum value for the progress bar

    This is the minimum value in the progress bar.
    \sa QProgressBar::minimum
*/

/*!
    \property QStyleOptionProgressBar::maximum
    \brief The maximum value for the progress bar

    This is the maximum value in the progress bar.
    \sa QProgressBar::maximum
*/

/*!
    \property QStyleOptionProgressBar::text
    \brief The text for the progress bar.

    The progress bar text is usually just the progress expressed as a string.
    An empty string indicates that the progress bar has not started yet.

    \sa QProgressBar::text
*/

/*!
    \property QStyleOptionProgressBar::textVisible
    \brief A flag indicating whether or not text is visible.

    If this flag is true then the text is visible. Otherwise, the text is not visible.

    \sa QProgressBar::textVisible
*/


/*!
    \property QStyleOptionProgressBar::textAlignment
    \brief The text alignment for the text in the QProgressBar

    This can be used as a guide on where the text should be in the progressbar.

    \sa QProgressBar::textAlignment
*/

/*!
    \property QStyleOptionProgressBar::progress
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
      checkType(NotCheckable), checked(false), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionMenuItem::QStyleOptionMenuItem(int version)
    : QStyleOption(version, SO_MenuItem), menuItemType(Normal),
      checkType(NotCheckable), checked(false), maxIconWidth(0), tabWidth(0)
{
}


/*!
    \enum QStyleOptionMenuItem::MenuItemType

    These values indicate the type of menu item that the structure describes.

    \value Normal A normal menu item.
    \value DefaultItem A menu item that is the default action \sa QMenu::defaultAction().
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

    \sa QAction::checkable QAction::checked QActionGroup::exculsive
*/

/*!
    \property QStyleOptionMenuItem::menuItemType

    \brief the type of menu item

    \sa MenuItemType
*/

/*!
    \property QStyleOptionMenuItem::checkType
    \brief The type of checkmark of the menu item
    \sa CheckType
*/

/*!
    \property QStyleOptionMenuItem::checked
    \brief whether the menu item is checked or not.
*/

/*!
    \property QStyleOptionMenuItem::menuRect
    \brief The rectangle for the entire menu.
*/

/*!
    \property QStyleOptionMenuItem::text
    \brief The text for the menu item.

    Note that the text format is something like this "Menu
    text\bold{\\t}Shortcut".

    If the menu item doesn't have a shortcut, it will just contain
    the menu item's text.
*/

/*!
    \property QStyleOptionMenuItem::icon
    \brief The icon for the menu item.
*/

/*!
    \property QStyleOptionMenuItem::maxIconWidth
    \brief the maximum icon width for the icon in the menu item.

    This can be used for drawing the icon into the correct place or
    properly aligning items. The variable must be set regardless of
    whether or not the menu item has an icon.
*/

/*!
    \property QStyleOptionMenuItem::tabWidth
    \brief The tab width for the menu item.

    The tab width is the distance between the text of the menu item
    and the shortcut.
*/


/*!
    \property QStyleOptionMenuItem::font
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
    \property QStyleOptionComplex::subControls
    \brief The sub-controls that need to be painted.

    This is a bitwise OR of the various sub-controls that need to be drawn for the complex control.

    \sa QStyle::SubControl
*/

/*!
    \property QStyleOptionComplex::activeSubControls
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
      sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0)
{
}

/*!
    \internal
*/
QStyleOptionSlider::QStyleOptionSlider(int version)
    : QStyleOptionComplex(version, SO_Slider), minimum(0), maximum(0),
      tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
      sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0)
{
}

/*!
    \property QStyleOptionSlider::orientation
    \brief the slider's orientation (horizontal or vertical)

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
    \property QStyleOptionSlider::tickPosition
    \brief the position of the slider's tick marks, if any.

    \sa QSlider::TickPosition
*/

/*!
    \property QStyleOptionSlider::tickInterval
    \brief The interval that should be drawn between tick marks.
*/

/*!
    \property QStyleOptionSlider::upsideDown
    \brief Indicates slider control orientation.

    Normally a slider increases as it moves up or to the right; upsideDown
    indicates that it should do the opposite (increase as it moves down or to
    the left).

    \sa QStyle::positionFromValue(), QStyle::valueFromPosition(),
        QAbstractSlider::invertedAppearance
*/

/*!
    \property QStyleOptionSlider::sliderPosition
    \brief The position of the slider handle.

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same
    as \l sliderValue. Otherwise, it will have the current position
    of the handle.

    \sa QAbstractSlider::tracking, sliderValue
*/

/*!
    \property QStyleOptionSlider::sliderValue
    \brief The value of the slider.

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same
    as \l sliderPosition. Otherwise, it will have the value the
    slider had before the mouse was pressed.

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
      stepEnabled(QAbstractSpinBox::StepNone), percentage(0.0), showSliderIndicator(false),
      showFrame(true)
{
}

/*!
    \internal
*/
QStyleOptionSpinBox::QStyleOptionSpinBox(int version)
    : QStyleOptionComplex(version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
      stepEnabled(QAbstractSpinBox::StepNone), percentage(0.0), showSliderIndicator(false),
      showFrame(true)
{
}


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

    The percentage is the quotient of the current value divided by
    the maximum value.
*/

/*!
    \property QStyleOptionSpinBox::showSliderIndicator
    \brief Indicates whether the slider indicator should be drawn.
*/

/*!
    \property QStyleOptionSpinBox::showFrame
    \brief Indicates whether a frame should be drawn.
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
    \property QStyleOptionQ3ListViewItem::features
    \brief The features for this item

    This variable is a bitwise OR of the features of the item.

    \sa ListViewItemFeature
*/

/*!
    \property QStyleOptionQ3ListViewItem::height
    \brief The height of the item

    This doesn't include the height of the item's children.

    \sa Q3ListViewItem::height()
*/

/*!
    \property QStyleOptionQ3ListViewItem::totalHeight
    \brief The total height of the item, including its children

    \sa Q3ListViewItem::totalHeight()
*/

/*!
    \property QStyleOptionQ3ListViewItem::itemY
    \brief The Y-coordinate for the item

    \sa Q3ListViewItem::itemY()
*/

/*!
    \property QStyleOptionQ3ListViewItem::childCount
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
    \property QStyleOptionQ3ListView::items
    \brief A list of items in the \l Q3ListView.

    This is a list of \l {QStyleOptionQ3ListViewItem}s. The first item
    can be used for most of the calculation that are needed for
    drawing a list view. Any additional items are the children of
    this first item, which may be used for additional information.

    \sa QStyleOptionQ3ListViewItem
*/

/*!
    \property QStyleOptionQ3ListView::viewportPalette
    \brief The palette of Q3ListView's viewport.
*/

/*!
    \property QStyleOptionQ3ListView::viewportBGRole
    \brief The background role of \l Q3ListView's viewport.

    \sa QWidget::backgroundRole()
*/

/*!
    \property QStyleOptionQ3ListView::sortColumn
    \brief The sort column of the list view.

    \sa Q3ListView::sortColumn()
*/

/*!
    \property QStyleOptionQ3ListView::itemMargin
    \brief The margin for items in the list view.

    \sa Q3ListView::itemMargin()
*/

/*!
    \property QStyleOptionQ3ListView::treeStepSize
    \brief The number of pixel to offset children items from their parents.

    \sa Q3ListView::treeStepSize()
*/

/*!
    \property QStyleOptionQ3ListView::rootIsDecorated
    \brief Whether root items are decorated

    \sa Q3ListView::rootIsDecorated()
*/

/*!
    \class QStyleOptionQ3DockWindow
    \brief The QStyleOptionQ3DockWindow class is used to describe the
    parameters for drawing various parts of \l Q3DockWindows.

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
    \property QStyleOptionQ3DockWindow::docked
    \brief Indicates that the dock window is currently docked.
*/

/*!
    \property QStyleOptionQ3DockWindow::closeEnabled
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
    : QStyleOption(Version, SO_DockWidget), moveable(false)
{
}

/*!
    \internal
*/
QStyleOptionDockWidget::QStyleOptionDockWidget(int version)
    : QStyleOption(version, SO_DockWidget), closable(false),
      moveable(false), floatable(false)
{
}

/*!
    \property QStyleOptionDockWidget::title
    \brief The title of the dock window
*/

/*!
    \property QStyleOptionDockWidget::closable
    \brief Indicates that the dock window is closable.
*/

/*!
    \property QStyleOptionDockWidget::moveable
    \brief Indicates that the dock window is moveable.
*/

/*!
    \property QStyleOptionDockWidget::floatable
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
    \property QStyleOptionToolButton::features
    \brief The features of the tool button.

    This variable is a bitwise OR describing the features of the button.

    \sa ToolButtonFeature
*/

/*!
    \property QStyleOptionToolButton::icon
    \brief The icon for the tool button.
*/

/*!
    \property QStyleOptionToolButton::text
    \brief The text of the tool button.

    This value is only used if \l features includes \l TextLabel.
*/

/*!
    \property QStyleOptionToolButton::arrowType
    \brief The direction of the arrow for the tool button

    This value is only used if \l features includes \l Arrow.
*/

/*!
    \property QStyleOptionToolButton::toolButtonStyle
    \brief Used to describe the appearance of a tool button

    \sa QToolButton::toolButtonStyle()
*/

/*!
    \property QStyleOptionToolButton::pos
    \brief The position of the tool button
*/

/*!
    \property QStyleOptionToolButton::font
    \brief The font that is used for the text.

    This value is only used if \l features includes \l TextLabel.
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
    : QStyleOptionComplex(Version, SO_ComboBox), editable(false)
{
}

/*!
    \internal
*/
QStyleOptionComboBox::QStyleOptionComboBox(int version)
    : QStyleOptionComplex(version, SO_ComboBox), editable(false)
{
}

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
    \property QStyleOptionToolBox::icon
    \brief The icon for the tool box tab.
*/

/*!
    \property QStyleOptionToolBox::text
    \brief The text for the tool box tab.
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
    \internal
*/
QStyleOptionTitleBar::QStyleOptionTitleBar(int version)
    : QStyleOptionComplex(version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}


/*!
    \property QStyleOptionTitleBar::text
    \brief The text of the title bar.
*/

/*!
    \property QStyleOptionTitleBar::icon
    \brief The icon for the title bar.
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
    : QStyleOption(Version, SO_ViewItem), displayAlignment(0), decorationAlignment(0),
      decorationPosition(Left)
{
}

/*!
    \internal
*/
QStyleOptionViewItem::QStyleOptionViewItem(int version)
    : QStyleOption(version, SO_ViewItem), displayAlignment(0), decorationAlignment(0),
      decorationPosition(Left)
{
}

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
    \property QStyleOptionViewItem::font
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
    \class QStyleHintReturn
    \brief The QStyleHintReturn is used for style hints that return more than a simple int.

    \ingroup appearance

    QStyleHintReturn and its subclasses are used to pass information
    from a style back to the querying widget. This is most usefull
    when the return value from QStyle::styleHint() is not enough
    detail (for example when a mask is to be returned).

    ### --Sam
*/

/*!
    \enum QStyleOption::HitReturnType

    \value SH_Default QStyleHintReturn
    \value SH_Mask \l SH_RubberBand_Mask SH_FocusFrame_Mask
*/

/*!
    \property QStyleHintReturn::type
    \brief the type of the style hint container

    \sa HintReturnType
*/

/*!
    \property QStyleHintReturn::version
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

    \sa version, type
*/

QStyleHintReturn::QStyleHintReturn(int version, int type)
    : version(version), type(type)
{
}

QStyleHintReturn::~QStyleHintReturn()
{

}

QStyleHintReturnMask::QStyleHintReturnMask() : QStyleHintReturn(Version, Type)
{

}

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
