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

#include "qstyle.h"
#ifndef QT_NO_STYLE
#include "qapplication.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qstyleoption.h"

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

#include <limits.h>

/*!
    \class QStyle
    \brief The QStyle class is an abstract base class that encapsulates the look and feel of a GUI.

    \ingroup appearance

    Qt contains a set of QStyle subclasses that emulate the styles of
    the different platforms supported by Qt (QWindowsStyle,
    QMacStyle, QMotifStyle, etc.). By default, these styles are built
    into the QtGui library. Styles can also be made available as
    plugins.

    Qt's built-in widgets use QStyle to perform nearly all of their
    drawing, ensuring that they look exactly like the equivalent
    native widgets. The diagram below shows a QComboBox in eight
    different styles.

    \img qstyle-comboboxes.png Eight combo boxes

    Topics:

    \tableofcontents

    \section1 Setting a Style

    The style of the entire application can be set using
    QApplication::setStyle(). It can also be specified by the user of
    the application, using the \c -style command-line option:

    \code
        ./myapplication -style motif
    \endcode

    If no style is specified, Qt will choose the most appropriate
    style for the user's platform or desktop environment.

    A style can also be set on an individual widget using
    QWidget::setStyle().

    \section1 Developing Style-Aware Custom Widgets

    If you are developing custom widgets and want them to look good on
    all platforms, you can use QStyle functions to perform parts of
    the widget drawing, such as drawItem(), drawPrimitive(),
    drawControl(), and drawComplexControl().

    Most QStyle draw functions take four arguments:
    \list
    \o an enum value specifying which graphical element to draw
    \o a QStyleOption specifying how and where to render that element
    \o a QPainter that should be used to draw the element
    \o a QWidget on which the drawing is performed (optional)
    \endlist

    For example, if you want to draw a focus rectangle on your
    widget, you can write:

    \quotefromfile snippets/styles/styles.cpp
    \skipto MyWidget::paintEvent
    \printuntil }

    QStyle gets all the information it needs to render the graphical
    element from QStyleOption. The widget is passed as the last
    argument in case the style needs it to perform special effects
    (such as animated default buttons on Mac OS X), but it isn't
    mandatory. In fact, you can use QStyle to draw on any paint
    device, not just widgets, by setting the QPainter properly.

    QStyleOption has various subclasses for the various types of
    graphical elements that can be drawn. For example,
    PE_FrameFocusRect expects a QStyleOptionFocusRect argument. This is
    documented for each enum value.

    To ensure that drawing operations are as fast as possible,
    QStyleOption and its subclasses have public data members. See the
    QStyleOption class documentation for details on how to use it.

    For convenience, Qt provides the QStylePainter class, which
    combines a QStyle, a QPainter, and a QWidget. This makes it
    possible to write

    \skipto QStylePainter painter
    \printline painter
    \dots
    \skipto drawPrimitive
    \printline drawPrimitive

    instead of

    \quotefromfile snippets/styles/styles.cpp
    \skipto QPainter painter
    \printline painter
    \dots
    \skipto drawPrimitive
    \printline drawPrimitive

    \section1 Creating a Custom Style

    If you want to design a custom look and feel for your
    application, the first step is to pick one of the base styles
    provided with Qt to build your custom style from. The choice will
    depend on which existing style resembles your style the most.

    Depending on which parts of the base style you want to change,
    you must reimplement the functions that are used to draw those
    parts of the interface. To illustrate this, we will modify the
    look of the spin box arrows drawn by QWindowsStyle. The arrows
    are \e{primitive elements} that are drawn by the drawPrimitive()
    function, so we need to reimplement that function. We need the
    following class declaration:

    \quotefile customstyle/customstyle.h
    \skipto class CustomStyle
    \printuntil };

    The PE_IndicatorSpinUp and PE_IndicatorSpinDown primitive
    elements are used by QSpinBox to draw its up and down arrows.
    Here's how to reimplement drawPrimitive() to draw them
    differently:

    \quotefile customstyle/customstyle.cpp
    \skipto CustomStyle::drawPrimitive
    \printuntil QWindowsStyle::drawPrimitive
    \printline }
    \printline }

    Notice that we don't use the \c widget argument, except to pass
    it on to QWindowStyle::drawPrimitive(). As mentioned earlier, the
    information about what is to be drawn and how it should be drawn
    is specified by a QStyleOption object, so there is no need to ask
    the widget.

    If you need to use the \c widget argument to obtain additional
    information, be careful to ensure that it isn't 0 and that it is
    of the correct type before using it. For example:

    \code
        QSpinBox *spinBox = qobject_cast<QSpinBox *>(widget);
        if (spinBox) {
            ...
        }
    \endcode

    When implementing a custom style, you cannot assume that the
    widget is a QSpinBox just because the enum value is called
    PE_IndicatorSpinUp or PE_IndicatorSpinUp.

    The documentation for the \l{widgets/styles}{Styles} example
    covers this topic in more detail.

    \section1 Using a Custom Style

    There are several ways of using a custom style in a Qt
    application. The simplest way is call the
    QApplication::setStyle() static function before creating the
    QApplication object:

    \include customstyle/main.cpp

    You can call QApplication::setStyle() at any time, but by calling
    it before the constructor, you ensure that the user's preference,
    set using the \c -style command-line option, is respected.

    You may want to make your style available for use in other
    applications, some of which may not be yours and are not available for
    you to recompile. The Qt Plugin system makes it possible to create
    styles as plugins. Styles created as plugins are loaded as shared
    objects at runtime by Qt itself. Please refer to the \link
    plugins-howto.html Qt Plugin\endlink documentation for more
    information on how to go about creating a style plugin.

    Compile your plugin and put it into \c $QTDIR/plugins/styles. We
    now have a pluggable style that Qt can load automatically. To use
    your new style with existing applications, simply start the
    application with the following argument:

    \code
        ./myapplication -style custom
    \endcode

    The application will use the look and feel from the custom style you
    implemented.

    \section1 Right-to-Left Desktops

    Languages written from right to left (such as Arabic and Hebrew)
    usually also mirror the whole layout of widgets, and require the
    light to come from the screen's top-right corner instead of
    top-left.

    If you create a custom style, you should take special care when
    drawing asymmetric elements to make sure that they also look
    correct in a mirrored layout. An easy way to test your styles is
    to run applications with the \c -reverse command-line option or
    to call QApplication::setLayoutDirection() in your \c main()
    function.

    Here are some things to keep in mind when making a style work well in a
    right-to-left environment:

    \list
    \o subControlRect() and subElementRect() return rectangles in screen coordinates
    \o \c{QStyleOption::direction} indicates in which direction the item should be drawn in
    \o If a style is not right-to-left aware it will display items as if it were left-to-right
    \o visualRect(), visualPos(), and visualAlignment() are helpful functions that will
       translate from logical to screen representations.
    \o alignedRect() will return a logical rect aligned for the current direction
    \endlist

    \sa QStyleOption, QStylePainter
*/

/*!
    Constructs a style object.
*/
QStyle::QStyle()
{
}

/*!
    Destroys the style object.
*/
QStyle::~QStyle()
{
}

/*!
    Initializes the appearance of \a widget.

    This function is called for every widget at some point after it
    has been fully created but just \e before it is shown for the very
    first time.

    Reasonable actions in this function might be to call
    QWidget::setBackgroundMode() for the widget. An example of highly
    unreasonable use would be setting the geometry! Reimplementing
    this function gives you a back-door through which you can change
    the appearance of a widget. With Qt 4.0's style engine you will
    rarely need to write your own polish(); instead reimplement
    drawItem(), drawPrimitive(), etc.

    The QWidget::inherits() function may provide enough information to
    allow class-specific customizations. But be careful not to
    hard-code things too much because new QStyle subclasses are
    expected to work reasonably with all current and \e future
    widgets.

    \sa unpolish()
*/
void QStyle::polish(QWidget * /* widget */)
{
}

/*!
    Undoes the initialization of widget \a{widget}'s appearance.

    This function is the counterpart to polish. It is called for every
    polished widget when the style is dynamically changed. The former
    style has to unpolish its settings before the new style can polish
    them again.

    \sa polish()
*/
void QStyle::unpolish(QWidget * /* widget */)
{
}

/*!
    \overload

    Late initialization of the QApplication object \a app.

    \sa unpolish()
*/
void QStyle::polish(QApplication * /* app */)
{
}

/*!
    \overload

    Undoes the polish of application \a app.

    \sa polish()
*/
void QStyle::unpolish(QApplication * /* app */)
{
}

/*!
    \overload

    The style may have certain requirements for color palettes. In
    this function it has the chance to change the palette \a pal
    according to these requirements.

    \sa QPalette, QApplication::setPalette()
*/
void QStyle::polish(QPalette & /* pal */)
{
}

/*!
    Returns the appropriate area (see below) within rectangle \a rect in
    which to draw \a text using the font metrics \a metrics.

    The text is aligned in accordance with \a alignment. The \a enabled bool
    indicates whether or not the item is enabled.

    If \a rect is larger than the area needed to render the \a text
    the rectangle that is returned will be offset within \a rect in
    accordance with the alignment \a alignment. For example, if \a
    alignment is \c Qt::AlignCenter, the returned rectangle will be
    centered within \a rect. If \a rect is smaller than the area
    needed, the rectangle that is returned will be \e larger than \a
    rect (the smallest rectangle large enough to render the \a text).

    \sa Qt::Alignment
*/
QRect QStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rect, int alignment, bool enabled,
                       const QString &text) const
{
    QRect result;
    int x, y, w, h;
    rect.getRect(&x, &y, &w, &h);
    if (!text.isEmpty()) {
        result = metrics.boundingRect(x, y, w, h, alignment, text);
        if (!enabled && styleHint(SH_EtchDisabledText)) {
            result.setWidth(result.width()+1);
            result.setHeight(result.height()+1);
        }
    } else {
        result = QRect(x, y, w, h);
    }
    return result;
}

/*!
    Returns the appropriate area within rectangle \a rect in
    which to draw the \a pixmap with alignment defined in \a alignment.
*/
QRect QStyle::itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QRect result;
    int x, y, w, h;
    rect.getRect(&x, &y, &w, &h);
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += h/2 - pixmap.height()/2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += h - pixmap.height();
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += w - pixmap.width();
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += w/2 - pixmap.width()/2;
    else if ((alignment & Qt::AlignLeft) != Qt::AlignLeft && QApplication::isRightToLeft())
        x += w - pixmap.width();
    result = QRect(x, y, pixmap.width(), pixmap.height());
    return result;
}

/*!
    Draws the \a text in rectangle \a rect using \a painter and
    palette \a pal.

    Text is drawn using the painter's pen. If an explicit \a textRole
    is specified, then the text is drawn using the color specified in
    \a pal for the specified role.  The \a enabled bool indicates
    whether or not the item is enabled; when reimplementing this bool
    should influence how the item is drawn.

    The text is aligned and wrapped according to \a alignment.

    \sa Qt::Alignment
*/
void QStyle::drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                          bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    if (text.isEmpty())
        return;
    QPen savedPen;
    if (textRole != QPalette::NoRole) {
        savedPen = painter->pen();
        painter->setPen(pal.color(textRole));
    }
    if (!enabled) {
        if (styleHint(SH_DitherDisabledText)) {
            painter->drawText(rect, alignment, text);
            painter->fillRect(painter->boundingRect(rect, alignment, text), QBrush(painter->background().color(), Qt::Dense5Pattern));
            return;
        } else if (styleHint(SH_EtchDisabledText)) {
            QPen pen = painter->pen();
            painter->setPen(pal.light().color());
            painter->drawText(rect.adjusted(1, 1, 1, 1), alignment, text);
            painter->setPen(pen);
        }
    }
    painter->drawText(rect, alignment, text);
    if (textRole != QPalette::NoRole)
        painter->setPen(savedPen);
}

/*!
    Draws the \a pixmap in rectangle \a rect with alignment \a alignment using
    \a painter.
*/

void QStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment,
                            const QPixmap &pixmap) const
{
    QRect aligned = alignedRect(QApplication::layoutDirection(), QFlag(alignment), pixmap.size(), rect);
    QRect inter = aligned.intersect(rect);

    painter->drawPixmap(inter.x(), inter.y(), pixmap, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width(), inter.height());
}

/*!
    \enum QStyle::PrimitiveElement

    This enum represents a style's PrimitiveElements. A
    PrimitiveElement is a common GUI element, such as a checkbox
    indicator or button bevel.

    \value PE_PanelButtonCommand  Button used to initiate an action, for
        example, a QPushButton.
    \value PE_FrameDefaultButton  This frame around a default button, e.g. in a dialog.
    \value PE_PanelButtonBevel  Generic panel with a button bevel.
    \value PE_PanelButtonTool  Panel for a Tool button, used with QToolButton
    \value PE_IndicatorButtonDropDown  indicator for a drop down button, for example, a tool
                                       button that displays a menu.

    \value PE_FrameFocusRect  Generic focus indicator.

    \value PE_IndicatorArrowUp  Generic Up arrow.
    \value PE_IndicatorArrowDown  Generic Down arrow.
    \value PE_IndicatorArrowRight  Generic Right arrow.
    \value PE_IndicatorArrowLeft  Generic Left arrow.

    \value PE_IndicatorSpinUp  Up symbol for a spin widget, for example a QSpinBox.
    \value PE_IndicatorSpinDown  Down symbol for a spin widget.
    \value PE_IndicatorSpinPlus  Increase symbol for a spin widget.
    \value PE_IndicatorSpinMinus  Decrease symbol for a spin widget.

    \value PE_IndicatorViewItemCheck On/off indicator for a view item

    \value PE_IndicatorCheckBox  On/off indicator, for example, a QCheckBox.
    \value PE_IndicatorRadioButton  Exclusive on/off indicator, for example, a QRadioButton.

    \value PE_Q3DockWindowSeparator  Item separator for Qt 3 compatible dock window
                                     and toolbar contents.
    \value PE_IndicatorDockWidgetResizeHandle  Resize handle for dock windows.

    \value PE_Frame  Generic frame; see also QFrame.
    \value PE_FrameMenu  Frame for popup windows/menus; see also QMenu.
    \value PE_PanelMenuBar  Panel for menu bars.
    \value PE_FrameDockWidget  Panel frame for dock windows and toolbars.
    \value PE_FrameTabWidget  Frame for tab widgets.
    \value PE_FrameLineEdit  Panel frame for line edits.
    \value PE_FrameGroupBox  Panel frame around group boxes.
    \value PE_FrameButtonBevel  Panel frame for a button bevel
    \value PE_FrameButtonTool  Panel frame for a tool button

    \value PE_IndicatorHeaderArrow  Arrow used to indicate sorting on a list or table
        header
    \value PE_FrameStatusBar Frame for a section of a status bar; see also QStatusBar.

    \value PE_FrameWindow  Frame around a MDI window or a docking window.

    \value PE_Q3Separator  Qt 3 compatible generic separator.

    \value PE_IndicatorMenuCheckMark  Check mark used in a menu.

    \value PE_IndicatorProgressChunk  Section of a progress bar indicator; see also QProgressBar.

    \value PE_Q3CheckListController  Qt 3 compatible Controller part of a list view item.
    \value PE_Q3CheckListIndicator  Qt 3 compatible Checkbox part of a list view item.
    \value PE_Q3CheckListExclusiveIndicator  Qt 3 compatible Radio button part of a list view item.

    \value PE_IndicatorBranch  Lines used to represent the branch of a tree in a tree view.
    \value PE_IndicatorToolBarHandle  The handle of a toolbar.
    \value PE_IndicatorToolBarSeparator  The separator in a toolbar.
    \value PE_PanelToolBar  The panel for a toolbar.
    \value PE_PanelTipLabel The panel for a tip label.
    \value PE_FrameTabBarBase The frame that is drawn for a tabbar, ususally drawn for a tabbar that isn't part of a tab widget

    \value PE_CustomBase  Base value for custom PrimitiveElements.
        All values above this are reserved for custom use. Custom
        values must be greater than this value.

    \sa drawPrimitive()
*/
/*! \enum QStyle::SFlags
    \internal
*/
/*! \enum QStyle::SCFlags
    \internal
*/

/*!
    \enum QStyle::StateFlag

    This enum represents flags for drawing PrimitiveElements. Not all
    primitives use all of these flags. Note that these flags may mean
    different things to different primitives.

    \value State_Active
    \value State_AutoRaise
    \value State_Bottom
    \value State_Children
    \value State_None
    \value State_DownArrow
    \value State_Editing
    \value State_Enabled
    \value State_FocusAtBorder
    \value State_HasFocus
    \value State_HasFocus
    \value State_Horizontal
    \value State_Item
    \value State_MouseOver
    \value State_NoChange
    \value State_Off
    \value State_On
    \value State_Open
    \value State_Raised
    \value State_Selected
    \value State_Sibling
    \value State_Sunken
    \value State_Top
    \value State_UpArrow
    \value State_KeyboardFocusChange
    \omitvalue State_Default

    \sa drawPrimitive()
*/

/*!
    \fn void QStyle::drawPrimitive(PrimitiveElement elem, const QStyleOption *option, \
                                   QPainter *painter, const QWidget *widget) const

    Draw the primitive option \a elem with \a painter using the style
    options specified by \a option.

    The \a widget argument is optional and may contain a widget that may
    aid in drawing the primitive.

    What follows is a table of the elements and the QStyleOption
    structure the \a option parameter can be cast to. The flags
    stored in the QStyleOption state variable are also listed. If a
    PrimitiveElement is not listed here, it uses a plain
    QStyleOption.

    The QStyleOption is the the following for the following types of PrimitiveElements.
    \table
    \header \o PrimitiveElement \o Option Cast \o Style Flag \o Remark
    \row \o \l PE_FrameFocusRect \o \l QStyleOptionFocusRect
         \o \l State_FocusAtBorder
         \o Whether the focus is is at the border or inside the widget.
    \row \o{1,2} \l PE_IndicatorCheckBox \o{1,2} \l QStyleOptionButton
          \o \l State_NoChange \o Indicates a "tri-state" checkbox.
    \row \o \l State_On \o Indicates the indicator is checked.

    \row \o \l PE_IndicatorRadioButton \o \l QStyleOptionButton
          \o \l State_On \o Indicates that a radio button is selected.
    \row \o{1,3} \l PE_Q3CheckListExclusiveIndicator, \l PE_Q3CheckListIndicator
         \o{1,3} \l QStyleOptionQ3ListView \o \l State_On
         \o Indicates whether or not the controller is selected.
    \row \o \l State_NoChange \o Indicates a "tri-state" controller.
    \row \o \l State_Enabled \o Indicates the controller is enabled.
    \row \o{1,2} \l PE_IndicatorBranch \o{1,2} \l QStyleOption
         \o \l State_DownArrow \o Indicates that the Tree Branch is pressed
    \row \o \l State_Open \o Indicates that the tree branch is expanded.
    \row \o \l PE_IndicatorHeaderArrow \o \l QStyleOptionHeader
         \o \l State_UpArrow \o Indicates that the arrow should be drawn up;
         otherwise it should be down.
    \row \o \l PE_FrameGroupBox, \l PE_Frame, \l PE_FrameLineEdit,
            \l PE_FrameMenu, \l PE_FrameDockWidget
         \o \l QStyleOptionFrame \o \l State_Sunken
         \o Indicates that the Frame should be sunken.
    \row \o \l PE_IndicatorToolBarHandle \o \l QStyleOption
         \o \l State_Horizontal \o Indicates that the window handle is horizontal
         instead of vertical.
    \row \o \l PE_Q3DockWindowSeparator \o \l QStyleOption
         \o \l State_Horizontal \o Indicates that the separator is horizontal
         instead of vertical.
    \row \o \l PE_IndicatorSpinPlus, \l PE_IndicatorSpinMinus, \l PE_IndicatorSpinUp,
            \l PE_IndicatorSpinDown,
         \o \l QStyleOptionSpinBox
         \o \l State_Sunken \o Indicates that the button is pressed.
    \endtable

    \sa PrimitiveElement, State, QStyleOption
*/

/*!
    \enum QStyle::ControlElement

    This enum represents a ControlElement. A ControlElement is part of
    a widget that performs some action or displays information to the
    user.

    \value CE_PushButton  A QPushButton, draws CE_PushButtonBevel, CE_PushButtonLabel and PE_FrameFocusRect
    \value CE_PushButtonBevel  The bevel and default indicator of a QPushButton.
    \value CE_PushButtonLabel  The label (icon with text or pixmap) of a QPushButton

    \value CE_DockWidgetTitle  Dock window title.
    \value CE_Splitter  Splitter handle; see also QSplitter.


    \value CE_CheckBox  A QCheckBox, draws a PE_IndicatorCheckBox, a CE_CheckBoxLabel and a PE_FrameFocusRect
    \value CE_CheckBoxLabel  The label (text or pixmap) of a QCheckBox

    \value CE_RadioButton  A QRadioButton, draws a PE_ExclusiveRadioButton, a CE_RadioButtonLabel and a PE_FrameFocusRect
    \value CE_RadioButtonLabel  The label (text or pixmap) of a QRadioButton

    \value CE_TabBarTab       The tab and label within a QTabBar
    \value CE_TabBarTabShape  The tab shape within a tab bar
    \value CE_TabBarTabLabel  The label within a tab

    \value CE_ProgressBar  A QProgressBar, draws CE_ProgressBarGroove, CE_ProgressBarContents and CE_ProgressBarLabel
    \value CE_ProgressBarGroove  The groove where the progress
        indicator is drawn in a QProgressBar
    \value CE_ProgressBarContents  The progress indicator of a QProgressBar
    \value CE_ProgressBarLabel  The text label of a QProgressBar

    \value CE_ToolButtonLabel  A tool button's label

    \value CE_MenuBarItem  A menu item in a QMenuBar
    \value CE_MenuBarEmptyArea  The empty area of a QMenuBar

    \value CE_MenuItem  A menu item in a QMenu
    \value CE_MenuScroller  Scrolling areas in a QMenu when the
        style supports scrolling
    \value CE_MenuTearoff  A menu item representing the tear off section of
        a QMenu
    \value CE_MenuEmptyArea  The area in a menu without menu items
    \value CE_MenuHMargin  The horizontal extra space on the left/right of a menu
    \value CE_MenuVMargin  The vertical extra space on the top/bottom of a menu

    \value CE_Q3DockWindowEmptyArea  The empty area of a QDockWidget

    \value CE_ToolBoxTab  The toolbox's tab area
    \value CE_SizeGrip  Window resize handle; see also QSizeGrip.

    \value CE_Header         A header
    \value CE_HeaderSection  A header section
    \value CE_HeaderLabel    The header's label

    \value CE_ScrollBarAddLine  Scroll bar line increase indicator.
                                (i.e., scroll down); see also QScrollBar.
    \value CE_ScrollBarSubLine  Scroll bar line decrease indicator (i.e., scroll up).
    \value CE_ScrollBarAddPage  Scolllbar page increase indicator (i.e., page down).
    \value CE_ScrollBarSubPage  Scroll bar page decrease indicator (i.e., page up).
    \value CE_ScrollBarSlider   Scroll bar slider.
    \value CE_ScrollBarFirst    Scroll bar first line indicator (i.e., home).
    \value CE_ScrollBarLast     Scroll bar last line indicator (i.e., end).

    \value CE_RubberBand        Rubber band used in such things as iconview.

    \value CE_FocusFrame        Focus Frame that can is style controled.

    \value CE_CustomBase  Base value for custom ControlElements;
    custom values must be greater than this value
    \value CE_ComboBoxLabel The label of a non-editable QComboBox

    \sa drawControl()
*/

/*!
    \fn void QStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const

    Draws the ControlElement \a element with \a painter with the
    style options specified by \a option.

    The \a widget argument is optional and may contain a widget that
    may aid in drawing the control.

    What follows is a table of the elements and the QStyleOption
    structure the \a option parameter can cast to. The flags stored
    in the QStyleOption state variable are also listed. If a
    ControlElement is not listed here, it uses a plain QStyleOption.

    \table
    \header \o ControlElement \o Option Cast \o Style Flag \o Remark
    \row \o{1,4} \l CE_MenuItem, \l CE_MenuBarItem
         \o{1,4} \l QStyleOptionMenuItem
         \o \l State_Selected \o The menu item is currently selected item
    \row \o \l State_Enabled \o The item is enabled
    \row \o \l State_DownArrow
         \o Set if the menu item is down
         (i.e., if the mouse button or the space bar is pressed)
    \row \o \l State_HasFocus \o Set if the menubar has input focus
    \row \o{1,5} \l CE_PushButton, \l CE_PushButtonLabel
         \o{1,5} \l QStyleOptionButton
         \o \l State_Enabled \o Set if the button is enabled
    \row \o \l State_HasFocus \o Set if the button has input focus
    \row \o \l State_Raised \o Set if the button is not down, not on and not flat
    \row \o \l State_On \o Set if the button is a toggle button and is toggled on
    \row \o \l State_Sunken
         \o Set if the button is down (i.e., the mouse button or the
         space bar is pressed on the button)

    \row \o{1,6} \l CE_RadioButton, \l CE_RadioButtonLabel,
                 \l CE_CheckBox, \l CE_CheckBoxLabel
         \o{1,6} \l QStyleOptionButton
         \o \l State_Enabled \o Set if the button is enabled
    \row \o \l State_HasFocus \o Set if the button has input focus
    \row \o \l State_On \o Set if the button is checked
    \row \o \l State_Off \o Set if the button is not checked
    \row \o \l State_NoChange \o Set if the button is in the NoChange state
    \row \o \l State_Sunken
         \o Set if the button is down (i.e., the mouse button or
         the space bar is pressed on the button)
    \row \o{1,2} \l CE_ProgressBarContents, \l CE_ProgressBarLabel,
                 \l CE_ProgressBarGroove
         \o{1,2} \l QStyleOptionProgressBar
         \o \l State_Enabled \o Set if the progressbar is enabled
    \row \o \l State_HasFocus \o Set if the progressbar has input focus
    \row \o \l CE_Header, \l CE_HeaderSection, \c CE_HeaderLabel \o \l QStyleOptionHeader \o \o
    \row \o{1,7} \l CE_ToolButtonLabel
         \o{1,7} \l QStyleOptionToolButton
         \o \l State_Enabled \o Set if the tool button is enabled
    \row \o \l State_HasFocus \o Set if the tool button has input focus
    \row \o \l State_Sunken
         \o Set if the tool button is down (i.e., a mouse button or
         the space bar is pressed)
    \row \o \l State_On \o Set if the tool button is a toggle button and is toggled on
    \row \o \l State_AutoRaise \o Set if the tool button has auto-raise enabled
    \row \o \l State_MouseOver \o Set if the mouse pointer is over the tool button
    \row \o \l State_Raised \o Set if the button is not down and is not on
    \row \o \l CE_ToolBoxTab \o \l QStyleOptionToolBox
         \o \l State_Selected \o The tab is the currently selected tab
    \row \o{1,3} \l CE_HeaderSection \o{1,3} \l QStyleOptionHeader
         \o \l State_Sunken \o Indicates that the section is pressed.
    \row \o \l State_UpArrow \o Indicates that the sort indicator should be pointing up.
    \row \o \l State_DownArrow \o Indicates that the sort indicator should be pointing down.
    \endtable

    \sa ControlElement, State, QStyleOption
*/

/*!
    \enum QStyle::SubElement

    This enum represents a sub-area of a widget. Style implementations
    use these areas to draw the different parts of a widget.

    \value SE_PushButtonContents  Area containing the label (icon
        with text or pixmap)
    \value SE_PushButtonFocusRect  Area for the focus rect (usually
        larger than the contents rect)

    \value SE_CheckBoxIndicator  Area for the state indicator (e.g., check mark)
    \value SE_CheckBoxContents  Area for the label (text or pixmap)
    \value SE_CheckBoxFocusRect  Area for the focus indicator
    \value SE_CheckBoxClickRect  Clickable area, defaults to SE_CheckBoxFocusRect

    \value SE_RadioButtonIndicator  Area for the state indicator
    \value SE_RadioButtonContents  Area for the label
    \value SE_RadioButtonFocusRect  Area for the focus indicator
    \value SE_RadioButtonClickRect  Clickable area, defaults to SE_RadioButtonFocusRect

    \value SE_ComboBoxFocusRect  Area for the focus indicator

    \value SE_SliderFocusRect  Area for the focus indicator

    \value SE_Q3DockWindowHandleRect  Area for the tear-off handle

    \value SE_ProgressBarGroove  Area for the groove
    \value SE_ProgressBarContents  Area for the progress indicator
    \value SE_ProgressBarLabel  Area for the text label

    \value SE_DialogButtonAccept  Area for a dialog's accept button
    \value SE_DialogButtonReject  Area for a dialog's reject button
    \value SE_DialogButtonApply  Area for a dialog's apply button
    \value SE_DialogButtonHelp  Area for a dialog's help button
    \value SE_DialogButtonAll  Area for a dialog's all button
    \value SE_DialogButtonRetry  Area for a dialog's retry button
    \value SE_DialogButtonAbort  Area for a dialog's abort button
    \value SE_DialogButtonIgnore  Area for a dialog's ignore button
    \value SE_DialogButtonCustom  Area for a dialog's custom widget area (in the button row)

    \value SE_HeaderArrow
    \value SE_HeaderLabel

    \value SE_TabWidgetLeftCorner
    \value SE_TabWidgetRightCorner
    \value SE_TabWidgetTabBar
    \value SE_TabWidgetTabContents
    \value SE_TabWidgetTabPane
    \value SE_ToolBoxTabContents  Area for a toolbox tab's icon and label

    \value SE_ViewItemCheckIndicator Area for a view item's check mark

    \value SE_CustomBase  Base value for custom ControlElements
    Custom values must be greater than this value

    \sa subElementRect()
*/

/*!
    \fn QRect QStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const

    Returns the sub-area \a element as described in \a option in logical
    coordinates.

    The \a widget argument is optional and may contain a widget that
    may aid determining the subRect.

    The QStyleOption can be cast to the appropriate type based on the
    value of \a element. See the table below for the appropriate \a
    option casts:

    \table
    \header \o SubElement \o Option Cast
    \row \o \l SE_PushButtonContents   \o \l QStyleOptionButton
    \row \o \l SE_PushButtonFocusRect  \o \l QStyleOptionButton
    \row \o \l SE_CheckBoxIndicator    \o \l QStyleOptionButton
    \row \o \l SE_CheckBoxContents     \o \l QStyleOptionButton
    \row \o \l SE_CheckBoxFocusRect    \o \l QStyleOptionButton
    \row \o \l SE_RadioButtonIndicator \o \l QStyleOptionButton
    \row \o \l SE_RadioButtonContents  \o \l QStyleOptionButton
    \row \o \l SE_RadioButtonFocusRect \o \l QStyleOptionButton
    \row \o \l SE_ComboBoxFocusRect    \o \l QStyleOptionComboBox
    \row \o \l SE_Q3DockWindowHandleRect \o \l QStyleOptionQ3DockWindow
    \row \o \l SE_ProgressBarGroove    \o \l QStyleOptionProgressBar
    \row \o \l SE_ProgressBarContents  \o \l QStyleOptionProgressBar
    \row \o \l SE_ProgressBarLabel     \o \l QStyleOptionProgressBar
    \endtable

    \sa SubElement QStyleOption
*/

/*!
    \enum QStyle::ComplexControl

    This enum represents a ComplexControl. ComplexControls have
    different behavior depending upon where the user clicks on them
    or which keys are pressed.

    \value CC_SpinBox           A spinbox, like QSpinBox
    \value CC_ComboBox          A combobox, like QComboBox
    \value CC_ScrollBar         A scroll bar, like QScrollBar
    \value CC_Slider            A slider, like QSlider
    \value CC_ToolButton        A tool button, like QToolButton
    \value CC_TitleBar          A Title bar, like what is used in Q3Workspace
    \value CC_Q3ListView        Used for drawing the Q3ListView class
    \value CC_Dial              A dial, like QDial

    \value CC_CustomBase  Base value for custom ControlElements.
    Custom values must be greater than this value.

    \sa SubControl drawComplexControl()
*/

/*!
    \enum QStyle::SubControl

    This enum represents a SubControl within a ComplexControl.

    \value SC_None  Special value that matches no other SubControl.

    \value SC_ScrollBarAddLine  Scroll bar add line (i.e., down/right
        arrow); see also QScrollBar
    \value SC_ScrollBarSubLine  Scroll bar sub line (i.e., up/left arrow)
    \value SC_ScrollBarAddPage  Scroll bar add page (i.e., page down)
    \value SC_ScrollBarSubPage  Scroll bar sub page (i.e., page up)
    \value SC_ScrollBarFirst  Scroll bar first line (i.e., home)
    \value SC_ScrollBarLast  Scroll bar last line (i.e., end)
    \value SC_ScrollBarSlider  Scroll bar slider handle
    \value SC_ScrollBarGroove  Special sub-control which contains the
        area in which the slider handle may move

    \value SC_SpinBoxUp  Spinwidget up/increase; see also QSpinBox
    \value SC_SpinBoxDown  Spinwidget down/decrease
    \value SC_SpinBoxFrame  Spinwidget frame
    \value SC_SpinBoxEditField  Spinwidget edit field

    \value SC_ComboBoxEditField  Combobox edit field; see also QComboBox
    \value SC_ComboBoxArrow  Combobox arrow
    \value SC_ComboBoxFrame  Combobox frame
    \value SC_ComboBoxListBoxPopup  Combobox list box

    \value SC_SliderGroove  Special sub-control which contains the area
        in which the slider handle may move
    \value SC_SliderHandle  Slider handle
    \value SC_SliderTickmarks  Slider tickmarks

    \value SC_ToolButton  Tool button (see also QToolButton)
    \value SC_ToolButtonMenu  Sub-control for opening a popup menu in a
        tool button; see also Q3PopupMenu

    \value SC_TitleBarSysMenu  System menu button (i.e., restore, close, etc.)
    \value SC_TitleBarMinButton  Minimize button
    \value SC_TitleBarMaxButton  Maximize button
    \value SC_TitleBarCloseButton  Close button
    \value SC_TitleBarLabel  Window title label
    \value SC_TitleBarNormalButton  Normal (restore) button
    \value SC_TitleBarShadeButton  Shade button
    \value SC_TitleBarUnshadeButton  Unshade button
    \value SC_TitleBarContextHelpButton Context Help button

    \value SC_Q3ListView  The list view area
    \value SC_Q3ListViewExpand  Expand item (i.e., show/hide child items)

    \value SC_DialHandle The handle of the dial (i.e. what you use to control the dial)
    \value SC_DialGroove The groove for the dial
    \value SC_DialTickmarks The tickmarks for the dial


    \value SC_All  Special value that matches all SubControls
    \omitvalue SC_Q3ListViewBranch

    \sa ComplexControl
*/

/*!
    \fn void QStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const

    Draws the ComplexControl \a control using \a painter with the
    style options specified by \a option.

    The \a widget argument is optional and may contain a widget to
    aid in drawing \a control.

    The \a option parameter is a pointer to a QStyleOptionComplex
    structure that can be cast to the correct structure. Note that
    the \c rect member of \a option must be in logical coordinates.
    Reimplementations of this function should use visualRect() to
    change the logical coordinates into screen coordinates before
    calling drawPrimitive() or drawControl().

    Here is a table listing the elements and what they can be cast to,
    along with an explaination of the flags.

    \table
    \header \o ComplexControl \o Option Cast \o Style Flag \o Remark
    \row \o{1,2} \l{CC_SpinBox} \o{1,2} \l QStyleOptionSpinBox
         \o \l State_Enabled \o Set if the spin box is enabled
    \row \o \l State_HasFocus \o Set if the spin box has input focus

    \row \o{1,2} \l {CC_ComboBox} \o{1,2} \l QStyleOptionComboBox
         \o \l State_Enabled \o Set if the combobox is enabled
    \row \o \l State_HasFocus \o Set if the combobox has input focus

    \row \o{1,2} \l {CC_ScrollBar} \o{1,2} \l QStyleOptionSlider
         \o \l State_Enabled \o Set if the scroll bar is enabled
    \row \o \l State_HasFocus \o Set if the scroll bar has input focus

    \row \o{1,2} \l {CC_Slider} \o{1,2} \l QStyleOptionSlider
         \o \l State_Enabled \o Set if the slider is enabled
    \row \o \l State_HasFocus \o Set if the slider has input focus

    \row \o{1,2} \l {CC_Dial} \o{1,2} \l QStyleOptionSlider
         \o \l State_Enabled \o Set if the dial is enabled
    \row \o \l State_HasFocus \o Set if the dial has input focus

    \row \o{1,6} \l {CC_ToolButton} \o{1,6} \l QStyleOptionToolButton
         \o \l State_Enabled \o Set if the tool button is enabled
    \row \o \l State_HasFocus \o Set if the tool button has input focus
    \row \o \l State_DownArrow \o Set if the tool button is down (i.e., a mouse
        button or the space bar is pressed)
    \row \o \l State_On \o Set if the tool button is a toggle button
        and is toggled on
    \row \o \l State_AutoRaise \o Set if the tool button has auto-raise enabled
    \row \o \l State_Raised \o Set if the button is not down, not on, and doesn't
        contain the mouse when auto-raise is enabled

    \row \o \l{CC_TitleBar} \o \l QStyleOptionTitleBar
         \o \l State_Enabled \o Set if the title bar is enabled

    \row \o \l{CC_Q3ListView} \o \l QStyleOptionQ3ListView
         \o \l State_Enabled \o Set if the list view is enabled

    \endtable

    \sa ComplexControl SubControl QStyleOptionComplex
*/


/*!
    \fn QRect QStyle::subControlRect(ComplexControl control,
        const QStyleOptionComplex *option, SubControl subControl,
        const QWidget *widget) const = 0

    Returns the rectangle for the SubControl \a subControl in the
    ComplexControl \a control, with the style options specified by \a
    option in visual coordinates.

    The \a option argument is a pointer to a QStyleOptionComplex or one of its
    subclasses. The structure can be cast to the appropriate type based on the
    value of \a control. See drawComplexControl() for details.

    The \a widget is optional and can contain additional information
    for the function.

    \sa drawComplexControl() ComplexControl SubControl QStyleOptionComplex
*/

/*!
    \fn QStyle::SubControl QStyle::hitTestComplexControl(ComplexControl control,
        const QStyleOptionComplex *option, const QPoint &pos,
        const QWidget *widget) const = 0

    Returns the SubControl in the ComplexControl \a control with the
    style options specified by \a option at the point \a pos. The \a
    option argument is a pointer to a QStyleOptionComplex structure
    or one of its subclasses. The structure can be cast to the
    appropriate type based on the value of \a control. See
    drawComplexControl() for details.

    The \a widget argument is optional and can contain additional
    information for the functions.

    Note that \a pos is expressed in screen coordinates.

    \sa drawComplexControl() ComplexControl SubControl subControlRect() QStyleOptionComplex
*/

/*!
    \enum QStyle::PixelMetric

    This enum represents a PixelMetric. A PixelMetric is a style
    dependent size represented as a single pixel value.

    \value PM_ButtonMargin  Amount of whitespace between push button
        labels and the frame
    \value PM_ButtonDefaultIndicator  Width of the default-button indicator frame
    \value PM_MenuButtonIndicator  Width of the menu button indicator
        proportional to the widget height
    \value PM_ButtonShiftHorizontal  Horizontal contents shift of a
        button when the button is down
    \value PM_ButtonShiftVertical  Vertical contents shift of a button when the
        button is down

    \value PM_DefaultFrameWidth  Default frame width (usually 2)
    \value PM_SpinBoxFrameWidth  Frame width of a spin box, defaults to PM_DefaultFrameWidth
    \value PM_ComboBoxFrameWidth Frame width of a combo box, defaults to PM_DefaultFrameWidth.
    \value PM_MDIFrameWidth  Frame width of an MDI window
    \value PM_MDIMinimizedWidth  Width of a minimized MDI window

    \value PM_MaximumDragDistance  Some feels require the scroll bar or
        other sliders to jump back to the original position when the
        mouse pointer is too far away while dragging; a value of -1
        disables this behavior

    \value PM_ScrollBarExtent  Width of a vertical scroll bar and the
        height of a horizontal scroll bar
    \value PM_ScrollBarSliderMin  The minimum height of a vertical
        scroll bar's slider and the minimum width of a horizontal
        scroll bar's slider

    \value PM_SliderThickness  Total slider thickness
    \value PM_SliderControlThickness  Thickness of the slider handle
    \value PM_SliderLength  Length of the slider
    \value PM_SliderTickmarkOffset  The offset between the tickmarks
        and the slider
    \value PM_SliderSpaceAvailable  The available space for the slider to move

    \value PM_DockWidgetSeparatorExtent  Width of a separator in a
        horizontal dock window and the height of a separator in a
        vertical dock window
    \value PM_DockWidgetHandleExtent  Width of the handle in a
        horizontal dock window and the height of the handle in a
        vertical dock window
    \value PM_DockWidgetFrameWidth  Frame width of a dock window

    \value PM_MenuBarPanelWidth  Frame width of a menubar, defaults to PM_DefaultFrameWidth
    \value PM_MenuBarItemSpacing  Spacing between menubar items
    \value PM_MenuBarHMargin  Spacing between menubar items and left/right of bar
    \value PM_MenuBarVMargin  Spacing between menubar items and top/bottom of bar

    \value PM_ToolBarFrameWidth  Width of the frame around toolbars
    \value PM_ToolBarHandleExtent Width of a toolbar handle in a
        horizontal toolbar and the height of the handle in a vertical toolbar
    \value PM_ToolBarItemMargin  Spacing between the toolbar frame and the items
    \value PM_ToolBarItemSpacing  Spacing between toolbar items
    \value PM_ToolBarSeparatorExtent Width of a toolbar separator in a
        horizontal toolbar and the height of a separator in a vertical toolbar
    \value PM_ToolBarExtensionExtent Width of a toolbar extension
         button in a horizontal toolbar and the height of the button in a
         vertical toolbar

    \value PM_TabBarTabOverlap  Number of pixels the tabs should overlap
    \value PM_TabBarTabHSpace  Extra space added to the tab width
    \value PM_TabBarTabVSpace  Extra space added to the tab height
    \value PM_TabBarBaseHeight  Height of the area between the tab bar
        and the tab pages
    \value PM_TabBarBaseOverlap  Number of pixels the tab bar overlaps
        the tab bar base
    \value PM_TabBarScrollButtonWidth
    \value PM_TabBarTabShiftHorizontal  Horizontal pixel shift when a
        tab is selected
    \value PM_TabBarTabShiftVertical  Vertical pixel shift when a
        tab is selected

    \value PM_ProgressBarChunkWidth  Width of a chunk in a progress bar indicator

    \value PM_SplitterWidth  Width of a splitter

    \value PM_TitleBarHeight  Height of the title bar

    \value PM_IndicatorWidth  Width of a check box indicator
    \value PM_IndicatorHeight  Height of a checkbox indicator
    \value PM_ExclusiveIndicatorWidth  Width of a radio button indicator
    \value PM_ExclusiveIndicatorHeight  Height of a radio button indicator

    \value PM_MenuPanelWidth  Border width (applied on all sides) for a QMenu
    \value PM_MenuHMargin  Additional border (used on left and right) for a QMenu
    \value PM_MenuVMargin  Additional border (used for bottom and top) for a QMenu
    \value PM_MenuScrollerHeight  Height of the scroller area in a QMenu
    \value PM_MenuScrollerHeight  Height of the scroller area in a QMenu
    \value PM_MenuTearoffHeight  Height of a tear off area in a QMenu
    \value PM_MenuDesktopFrameWidth

    \value PM_CheckListButtonSize  Area (width/height) of the
        checkbox/radio button in a Q3CheckListItem
    \value PM_CheckListControllerSize  Area (width/height) of the
        controller in a Q3CheckListItem

    \value PM_DialogButtonsSeparator  Distance between buttons in a dialog buttons widget
    \value PM_DialogButtonsButtonWidth  Minimum width of a button in a dialog buttons widget
    \value PM_DialogButtonsButtonHeight  Minimum height of a button in a dialog buttons widget

    \value PM_HeaderMarkSize
    \value PM_HeaderGripMargin
    \value PM_HeaderMargin
    \value PM_SpinBoxSliderHeight The height of the optional spin box slider

    \value PM_CustomBase  Base value for custom ControlElements
    Custom values must be greater than this value

    \value PM_DefaultTopLevelMargin
    \value PM_DefaultChildMargin
    \value PM_DefaultLayoutSpacing

    \value PM_ToolBarIconSize Default tool bar icon size, defaults to PM_SmallIconSize
    \value PM_SmallIconSize Default small icon size
    \value PM_LargeIconSize Default large icon size

    \value PM_FocusFrameHMargin Horizontal margin that the focus frame will outset the widget by.
    \value PM_FocusFrameVMargin Vertical margin that the focus frame will outset the widget by.
    \value PM_IconViewIconSize
    \value PM_ListViewIconSize

    \sa pixelMetric()
*/

/*!
    \fn int QStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;

    Returns the pixel metric for the given \a metric. The \a option
    and \a widget can be used for calculating the metric. The \a
    option can be cast to the appropriate type based on the value of
    \a metric. Note that \a option may be zero even for PixelMetrics
    that can make use of \a option. See the table below for the
    appropriate \a option casts:

    \table
    \header \o PixelMetric \o Option Cast
    \row \o \l PM_SliderControlThickness \o \l QStyleOptionSlider
    \row \o \l PM_SliderLength           \o \l QStyleOptionSlider
    \row \o \l PM_SliderTickmarkOffset   \o \l QStyleOptionSlider
    \row \o \l PM_SliderSpaceAvailable   \o \l QStyleOptionSlider
    \row \o \l PM_ScrollBarExtent        \o \l QStyleOptionSlider
    \row \o \l PM_TabBarTabOverlap       \o \l QStyleOptionTab
    \row \o \l PM_TabBarTabHSpace        \o \l QStyleOptionTab
    \row \o \l PM_TabBarTabVSpace        \o \l QStyleOptionTab
    \row \o \l PM_TabBarBaseHeight       \o \l QStyleOptionTab
    \row \o \l PM_TabBarBaseOverlap      \o \l QStyleOptionTab
    \endtable

    In general, the \a widget argument is not used.
*/

/*!
    \enum QStyle::ContentsType

    This enum represents a ContentsType. It is used to calculate sizes
    for the contents of various widgets.

    \value CT_CheckBox
    \value CT_ComboBox
    \value CT_DialogButtons
    \value CT_Q3DockWindow
    \value CT_HeaderSection
    \value CT_LineEdit
    \value CT_Menu
    \value CT_Q3Header
    \value CT_MenuBar
    \value CT_MenuBarItem
    \value CT_MenuItem
    \value CT_ProgressBar
    \value CT_PushButton
    \value CT_RadioButton
    \value CT_SizeGrip
    \value CT_Slider
    \value CT_ScrollBar
    \value CT_SpinBox
    \value CT_Splitter
    \value CT_TabBarTab
    \value CT_TabWidget
    \value CT_ToolButton

    \value CT_CustomBase  Base value for custom ControlElements.
    Custom values must be greater than this value.

    \sa sizeFromContents()
*/

/*!
    \fn QSize QStyle::sizeFromContents(ContentsType type, const QStyleOption *option, \
                                       const QSize &contentsSize, const QWidget *widget) const

    Returns the size of styled object described in \a option based on the
    contents size \a contentsSize.

    The \a option argument is a pointer to a QStyleOption or one of its
    subclasses. The \a option can be cast to the appropriate type based
    on the value of \a type. The widget \a widget is optional argument and can
    contain extra information used for calculating the size.
    See the table below for the appropriate \a option usage:

    \table
    \header \o ContentsType    \o Option Cast
    \row \o \l CT_PushButton   \o \l QStyleOptionButton
    \row \o \l CT_CheckBox     \o \l QStyleOptionButton
    \row \o \l CT_RadioButton  \o \l QStyleOptionButton
    \row \o \l CT_ToolButton   \o \l QStyleOptionToolButton
    \row \o \l CT_ComboBox     \o \l QStyleOptionComboBox
    \row \o \l CT_Splitter     \o \l QStyleOption
    \row \o \l CT_Q3DockWindow \o \l QStyleOptionQ3DockWindow
    \row \o \l CT_ProgressBar  \o \l QStyleOptionProgressBar
    \row \o \l CT_MenuItem     \o \l QStyleOptionMenuItem
    \endtable

    \sa ContentsType QStyleOption
*/

/*!
    \enum QStyle::StyleHint

    This enum represents a StyleHint. A StyleHint is a general look
    and/or feel hint.

    \value SH_EtchDisabledText Disabled text is "etched" as it is on Windows.

    \value SH_DitherDisabledText

    \value SH_GUIStyle The GUI style to use.

    \value SH_ScrollBar_MiddleClickAbsolutePosition  A boolean value.
        If true, middle clicking on a scroll bar causes the slider to
        jump to that position. If false, middle clicking is
        ignored.

    \value SH_ScrollBar_LeftClickAbsolutePosition  A boolean value.
        If true, left clicking on a scroll bar causes the slider to
        jump to that position. If false, left clicking will
        behave as appropriate for each control.

    \value SH_ScrollBar_ScrollWhenPointerLeavesControl  A boolean
        value. If true, when clicking a scroll bar SubControl, holding
        the mouse button down and moving the pointer outside the
        SubControl, the scroll bar continues to scroll. If false, the
        scollbar stops scrolling when the pointer leaves the
        SubControl.

    \value SH_TabBar_Alignment  The alignment for tabs in a
        QTabWidget. Possible values are \c Qt::AlignLeft, \c
        Qt::AlignCenter and \c Qt::AlignRight.

    \value SH_Header_ArrowAlignment The placement of the sorting
        indicator may appear in list or table headers. Possible values
        are \c Qt::Left or \c Qt::Right.

    \value SH_Slider_SnapToValue  Sliders snap to values while moving,
        as they do on Windows.

    \value SH_Slider_SloppyKeyEvents  Key presses handled in a sloppy
        manner, i.e., left on a vertical slider subtracts a line.

    \value SH_ProgressDialog_CenterCancelButton  Center button on
        progress dialogs, like Motif, otherwise right aligned.

    \value SH_ProgressDialog_TextLabelAlignment Type Qt::Alignment.
        Text label alignment in progress dialogs; Qt::Center on
        windows, Qt::VCenter otherwise.

    \value SH_PrintDialog_RightAlignButtons  Right align buttons in
        the print dialog, as done on Windows.

    \value SH_MainWindow_SpaceBelowMenuBar One or two pixel space between
        the menubar and the dockarea, as done on Windows.

    \value SH_FontDialog_SelectAssociatedText Select the text in the
        line edit, or when selecting an item from the listbox, or when
        the line edit receives focus, as done on Windows.

    \value SH_Menu_AllowActiveAndDisabled  Allows disabled menu
        items to be active.

    \value SH_Menu_SpaceActivatesItem  Pressing the space bar activates
        the item, as done on Motif.

    \value SH_Menu_SubMenuPopupDelay  The number of milliseconds
        to wait before opening a submenu (256 on windows, 96 on Motif).

    \value SH_Menu_Scrollable Whether popup menus must support scrolling.

    \value SH_Menu_SloppySubMenus Whether popupmenu's must support
        sloppy submenu; as implemented on Mac OS.

    \value SH_ScrollView_FrameOnlyAroundContents  Whether scrollviews
        draw their frame only around contents (like Motif), or around
        contents, scroll bars and corner widgets (like Windows).

    \value SH_MenuBar_AltKeyNavigation  Menu bars items are navigable
        by pressing Alt, followed by using the arrow keys to select
        the desired item.

    \value SH_ComboBox_ListMouseTracking  Mouse tracking in combobox
        drop-down lists.

    \value SH_Menu_MouseTracking  Mouse tracking in popup menus.

    \value SH_MenuBar_MouseTracking  Mouse tracking in menubars.

    \value SH_Menu_FillScreenWithScroll Whether scrolling popups
    should fill the screen as they are scrolled.

    \value SH_ItemView_ChangeHighlightOnFocus  Gray out selected items
        when losing focus.

    \value SH_Widget_ShareActivation  Turn on sharing activation with
        floating modeless dialogs.

    \value SH_TabBar_SelectMouseType  Which type of mouse event should
        cause a tab to be selected.

    \value SH_Q3ListViewExpand_SelectMouseType  Which type of mouse event should
        cause a list view expansion to be selected.

    \value SH_TabBar_PreferNoArrows  Whether a tabbar should suggest a size
        to prevent scoll arrows.

    \value SH_ComboBox_Popup  Allows popups as a combobox drop-down
        menu.

    \value SH_Workspace_FillSpaceOnMaximize  The workspace should
        maximize the client area.

    \value SH_TitleBar_NoBorder  The title bar has no border.

    \value SH_ScrollBar_StopMouseOverSlider  Stops auto-repeat when
        the slider reaches the mouse position.

    \value SH_BlinkCursorWhenTextSelected  Whether cursor should blink
        when text is selected.

    \value SH_RichText_FullWidthSelection  Whether richtext selections
        should extend to the full width of the document.

    \value SH_GroupBox_TextLabelVerticalAlignment  How to vertically align a
        groupbox's text label.

    \value SH_GroupBox_TextLabelColor  How to paint a groupbox's text label.

    \value SH_DialogButtons_DefaultButton  Which button gets the
        default status in a dialog's button widget.

    \value SH_ToolBox_SelectedPageTitleBold  Boldness of the selected
    page title in a QToolBox.

    \value SH_LineEdit_PasswordCharacter  The Unicode character to be
    used for passwords.

    \value SH_Table_GridLineColor

    \value SH_UnderlineShortcut  Whether shortcuts are underlined.

    \value SH_SpinBox_AnimateButton  Animate a click when up or down is
    pressed in a spin box.
    \value SH_SpinBox_KeyPressAutoRepeatRate  Auto-repeat interval for
    spinbox key presses.
    \value SH_SpinBox_ClickAutoRepeatRate  Auto-repeat interval for
    spinbox mouse clicks.
    \value SH_TipLabel_Opacity  An integer indicating the opacity for
    the tip label, 0 is completely transparent, 255 is completely
    opaque.
    \value SH_DrawMenuBarSeparator  Indicates whether or not the menubar draws separators.
    \value SH_TitlebarModifyNotification  Indicates if the titlebar should show
    a '*' for windows that are modified.

    \value SH_Button_FocusPolicy The default focus policy for buttons.

    \value SH_CustomBase  Base value for custom ControlElements.
    Custom values must be greater than this value.

    \value SH_MenuBar_DismissOnSecondClick A boolean indicating if a menu in
     the menubar should be dismissed when it is clicked on a second time. (Example:
     Clicking and releasing on the File Menu in a menubar and then
     immediately clicking on the File Menu again.)

     \value SH_MessageBox_UseBorderForButtonSpacing A boolean indicating what the to
     use the border of the buttons (computed as half the button height) for the spacing
     of the button in a message box.

     \value SH_TitleBar_AutoRaise A boolean indicating whether
     controls on a title bar ought to update when the mouse is over them.

     \value SH_ToolButton_PopupDelay An int indicating the popup delay in milliseconds
     for menus attached to tool buttons.

     \value SH_FocusFrame_Mask The mask of the focus frame.

     \value SH_RubberBand_Mask The mask of the rubber band.

     \value SH_FrameWindow_Mask The mask of the frame window.

     \value SH_SpinControls_DisableOnBounds Determines if the spin controls will shown
     as disabled when reaching the spin range boundary.

     \value SH_Dial_BackgroundRole Defines the style's preferred
     background role (as QPalette::ColorRole) for a dial widget.

     \value SH_ScrollBar_BackgroundMode The backgroundMode() for a scroll bar.

     \value SH_ComboBox_LayoutDirection The layout direction for the combo box.
     By default it should be the same value as the QStyleOption's
     \c{direction}.

    \value SH_ItemView_EllipsisLocation The location where ellipses should be
    added for item text that is too long to fit in an view item.

    \value SH_ItemView_AlternatingRowColors Whether to draw the background
    using alternating colors.
        
    \value SH_TreeView_ShowBranchSelected When an item in the tree view is selected,
    also highlight the branch.

     \omitvalue SH_UnderlineAccelerator

    \sa styleHint()
*/

/*!
    \fn int QStyle::styleHint(StyleHint hint, const QStyleOption *option, \
                              const QWidget *widget, QStyleHintReturn *returnData) const

    Returns the style hint \a hint for \a widget described in the QStyleOption
    \a option. Currently \a returnData and \a widget are not used; they are
    provided for future enhancement. The \a option parameter is used only in
    SH_ComboBox_Popup, SH_ComboBox_LayoutDirection, and
    SH_GroupBox_TextLabelColor.

    For an explanation of the return value, see \l StyleHint.
*/

/*!
    \enum QStyle::StandardPixmap

    This enum represents a StandardPixmap. A StandardPixmap is a pixmap that
    can follow some existing GUI style or guideline.

    \value SP_TitleBarMinButton  Minimize button on title bars (e.g.,
        in QWorkspace)
    \value SP_TitleBarMenuButton Menu button on a title bar
    \value SP_TitleBarMaxButton  Maximize button on title bars
    \value SP_TitleBarCloseButton  Close button on title bars
    \value SP_TitleBarNormalButton  Normal (restore) button on title bars
    \value SP_TitleBarShadeButton  Shade button on title bars
    \value SP_TitleBarUnshadeButton  Unshade button on title bars
    \value SP_TitleBarContextHelpButton The Context help button on title bars
    \value SP_MessageBoxInformation  The "information" icon
    \value SP_MessageBoxWarning  The "warning" icon
    \value SP_MessageBoxCritical  The "critical" icon
    \value SP_MessageBoxQuestion  The "question" icon
    \value SP_DesktopIcon
    \value SP_TrashIcon
    \value SP_ComputerIcon
    \value SP_DriveFDIcon
    \value SP_DriveHDIcon
    \value SP_DriveCDIcon
    \value SP_DriveDVDIcon
    \value SP_DriveNetIcon
    \value SP_DirOpenIcon
    \value SP_DirClosedIcon
    \value SP_DirLinkIcon
    \value SP_FileIcon
    \value SP_FileLinkIcon
    \value SP_FileDialogStart
    \value SP_FileDialogEnd
    \value SP_FileDialogToParent
    \value SP_FileDialogNewFolder
    \value SP_FileDialogDetailedView
    \value SP_FileDialogInfoView
    \value SP_FileDialogContentsView
    \value SP_FileDialogListView
    \value SP_FileDialogBack
    \value SP_DockWidgetCloseButton  Close button on dock windows (see also QDockWidget)
    \value SP_ToolBarHorizontalExtensionButton Extension button for horizontal toolbars
    \value SP_ToolBarVerticalExtensionButton Extension button for vertical toolbars
    \value SP_CustomBase  Base value for custom ControlElements;
    custom values must be greater than this value

    \sa standardPixmap()
*/

/*###
  \enum QStyle::IconMode

  This enum represents the effects performed on a pixmap to achieve a
  GUI style's perferred way of representing the image in different
  states.

  \value IM_Disabled  A disabled pixmap (drawn on disabled widgets)
  \value IM_Active  An active pixmap (drawn on active tool buttons and menu items)
  \value IM_CustomBase  Base value for custom PixmapTypes; custom
  values must be greater than this value

  \sa generatedIconPixmap()
*/

/*!
    \fn QPixmap QStyle::generatedIconPixmap(QIcon::Mode iconMode,
        const QPixmap &pixmap, const QStyleOption *option) const

    \overload

    Returns a pixmap styled to conform to \a iconMode description
    out of \a pixmap, and taking into account the palette specified by
    \a option.

    The \a option can pass extra information, but it must contain a palette.

    Not all types of pixmaps will change from their input in which
    case the result will simply be the pixmap passed in.
*/

/*!
    \fn QPixmap QStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option, \
                                       const QWidget *widget) const

    Returns a pixmap for \a standardPixmap.

    The \a option argument can be used to pass extra information required
    when drawing the ControlElement. Currently, the \a option argument is unused.

    The \a widget argument is optional and may contain a widget that
    may aid in drawing the control.
*/


/*!
    Returns the rectangle \a logicalRect converted to screen coordinates based
    on \a direction. The \a boundingRect rectangle is used to perform the
    translation.

    This function is provided to aid style implementors in supporting
    right-to-left desktops. It typically is used in subControlRect().

    \sa QWidget::layoutDirection
*/
QRect QStyle::visualRect(Qt::LayoutDirection direction, const QRect &boundingRect, const QRect &logicalRect)
{
    if (direction == Qt::LeftToRight)
        return logicalRect;
    QRect rect = logicalRect;
    rect.translate(2 * (boundingRect.right() - logicalRect.right()) +
                   logicalRect.width() - boundingRect.width(), 0);
    return rect;
}

/*!
    Returns the point \a logicalPos converted to screen coordinates based on
    \a direction.  The \a boundingRect rectangle is used to perform the
    translation.

    \sa QWidget::layoutDirection
*/
QPoint QStyle::visualPos(Qt::LayoutDirection direction, const QRect &boundingRect, const QPoint &logicalPos)
{
    if (direction == Qt::LeftToRight)
        return logicalPos;
    return QPoint(boundingRect.right() - logicalPos.x(), logicalPos.y());
}

/*!
     Returns a new rectangle of \a size that is aligned to \a
     rectangle according to \a alignment and based on \a direction.
 */
QRect QStyle::alignedRect(Qt::LayoutDirection direction, Qt::Alignment alignment, const QSize &size, const QRect &rectangle)
{
    alignment = visualAlignment(direction, alignment);
    int x = rectangle.x();
    int y = rectangle.y();
    int w = size.width();
    int h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += rectangle.size().height()/2 - h/2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rectangle.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rectangle.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += rectangle.size().width()/2 - w/2;
    return QRect(x, y, w, h);
}

/*!

  Transforms an \a alignment of Qt::AlignLeft or Qt::AlignRight
  without Qt::AlignAbsolute into Qt::AlignLeft or Qt::AlignRight with
  Qt::AlignAbsolute according to the layout \a direction. The other
  alignment flags are left untouched.

  If no horizontal aligment was specified, the function returns the
  default alignment for the layout \a direction.

  QWidget::layoutDirection
*/
Qt::Alignment QStyle::visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
{
    if (!(alignment & Qt::AlignHorizontal_Mask))
        alignment |= Qt::AlignLeft;
    if ((alignment & Qt::AlignAbsolute) == 0 && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
        if (direction == Qt::RightToLeft)
            alignment ^= (Qt::AlignLeft | Qt::AlignRight);
        alignment |= Qt::AlignAbsolute;
    }
    return alignment;
}

/*!
    Converts \a logicalValue to a pixel position. \a min maps to 0, \a
    max maps to \a span and other values are distributed evenly
    in-between.

    This function can handle the entire integer range without
    overflow, providing \a span is less than 4096.

    By default, this function assumes that the maximum value is on the
    right for horizontal items and on the bottom for vertical items.
    Set \a upsideDown to true to reverse this behavior.

    \sa sliderValueFromPosition()
*/

int QStyle::sliderPositionFromValue(int min, int max, int logicalValue, int span, bool upsideDown)
{
    if (span <= 0 || logicalValue < min || max <= min)
        return 0;
    if (logicalValue > max)
        return upsideDown ? span : min;

    uint range = max - min;
    uint p = upsideDown ? max - logicalValue : logicalValue - min;

    if (range > (uint)INT_MAX/4096) {
        const int scale = 4096*2;
        return ((p / scale) * span) / (range / scale);
        // ### the above line is probably not 100% correct
        // ### but fixing it isn't worth the extreme pain...
    } else if (range > (uint)span) {
        return (2 * p * span + range) / (2*range);
    } else {
        uint div = span / range;
        uint mod = span % range;
        return p * div + (2 * p * mod + range) / (2 * range);
    }
    // equiv. to (p * span) / range + 0.5
    // no overflow because of this implicit assumption:
    // span <= 4096
}

/*!
    Converts the pixel position \a pos to a value. 0 maps to \a min,
    \a span maps to \a max and other values are distributed evenly
    in-between.

    This function can handle the entire integer range without
    overflow.

    By default, this function assumes that the maximum value
    is on the right for horizontal items and on the bottom for
    vertical items. Set \a upsideDown to true to reverse this behavior.

    \sa sliderPositionFromValue()
*/

int QStyle::sliderValueFromPosition(int min, int max, int pos, int span, bool upsideDown)
{
    if (span <= 0 || pos <= 0)
        return upsideDown ? max : min;
    if (pos >= span)
        return upsideDown ? min : max;

    uint range = max - min;

    if ((uint)span > range) {
        int tmp = (2 * pos * range + span) / (2 * span);
        return upsideDown ? max - tmp : tmp + min;
    } else {
        uint div = range / span;
        uint mod = range % span;
        int tmp = pos * div + (2 * pos * mod + span) / (2 * span);
        return upsideDown ? max - tmp : tmp + min;
    }
    // equiv. to min + (pos*range)/span + 0.5
    // no overflow because of this implicit assumption:
    // pos <= span < sqrt(INT_MAX+0.0625)+0.25 ~ sqrt(INT_MAX)
}

/*### \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                              int flags, const QColorGroup &colorgroup, bool enabled,
                              const QString &text, int len = -1,
                              const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*### \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                              int flags, const QColorGroup colorgroup, bool enabled,
                              const QPixmap &pixmap,
                              const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*### \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                          int flags, const QColorGroup colorgroup, bool enabled,
                          const QPixmap *pixmap,
                          const QString &text, int len = -1,
                          const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*!
     Returns the style's standard palette. On systems that support
     system colors, the style's standard palette is not used.
 */
QPalette QStyle::standardPalette()
{
#ifdef Q_WS_X11
    QColor background;
    if (QX11Info::appDepth() > 8)
        background = QColor(0xd4, 0xd0, 0xc8); // win 2000 grey
    else
        background = QColor(192, 192, 192);
#else
    QColor background(0xd4, 0xd0, 0xc8); // win 2000 grey
#endif
    QColor light(background.light());
    QColor dark(background.dark());
    QColor mid(Qt::gray);
    QPalette palette(Qt::black, background, light, dark, mid, Qt::black, Qt::white);
    palette.setBrush(QPalette::Disabled, QPalette::Foreground, dark);
    palette.setBrush(QPalette::Disabled, QPalette::Text, dark);
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, dark);
    palette.setBrush(QPalette::Disabled, QPalette::Base, background);
    return palette;
}

#endif // QT_NO_STYLE
