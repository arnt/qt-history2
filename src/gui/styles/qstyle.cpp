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

    If you are developing custom widgets and want them to look good
    on all platforms, you can use QStyle functions to perform parts
    of the widget drawing, such as drawItem(), drawPrimitive(),
    drawControl(), drawControlMask(), drawComplexControl(), and
    drawComplexControlMask().

    Most draw functions take four arguments:
    \list
    \i an enum value specifying which graphical element to draw
    \i a QStyleOption specifying how and where to render that element
    \i a QPainter that should be used to draw the element
    \i a QWidget on which the drawing is performed (optional)
    \endlist

    For example, if you want to draw a focus rectangle on your
    widget, you can write:

    \code
        void MyWidget::paintEvent(QPaintEvent *event)
        {
            QPainter painter(this);
            ...

            QStyleOptionFocusRect option(1);
            option.init(this);
            option.backgroundColor = palette().color(QPalette::Background);

            style().drawPrimitive(QStyle::PE_FocusRect, &option, &painter, this);
        }
    \endcode

    QStyle gets all the information it needs to render the graphical
    element from QStyleOption. The widget is passed as the last
    argument in case the style needs it to perform special effects
    (such as animated default buttons on Mac OS X), but it isn't
    mandatory. In fact, you can use QStyle to draw on any paint
    device, not just widgets, by setting up the QPainter properly.

    QStyleOption has various subclasses for the various types of
    graphical elements that can be drawn. For example, \c
    PE_FocusRect expects a QStyleOptionFocusRect argument. This is
    documented for each enum value.

    To ensure that drawing operations are as fast as possible,
    QStyleOption and its subclasses are "plain old data" classes,
    with public data members. See the QStyleOption class
    documentation for details on how to use it.

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

    The \c PE_SpinBoxUp and \c PE_SpinBoxDown primitive elements are
    used by QSpinBox to draw its up and down arrows. Here's how to
    reimplement drawPrimitive() to draw them differently:

    \quotefile customstyle/customstyle.cpp
    \skipto ::drawPrimitive
    \printuntil /^\}/

    Notice that we don't use the \c widget argument, except to pass
    it on to QWindowStyle::drawPrimitive(). As mentioned earlier, the
    information about what is to be drawn and how it should be drawn
    is specified by a QStyleOption object, so there is no need to ask
    the widget.

    If you need to use the \c widget argument to obtain additional
    information, be careful to ensure that it isn't 0 and that it is
    of the correct type before using it. For example:

    \code
        QSpinBox *spinBox = qt_cast<QSpinBox *>(widget);
        if (spinBox) {
            ...
        }
    \endcode

    When implementing a custom style, you cannot assume that the
    widget is a QSpinBox just because the enum value is called \c
    PE_SpinBoxUp or \c PE_SpinBoxUp.

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
    to call QApplication::setReverseLayout() in your main().

    The actual reverse layout is performed automatically when
    possible. However, for the sake of flexibility, the translation
    cannot be performed everywhere. The documentation for each QStyle
    function states whether the function expects (or returns) logical
    or screen coordinates. Using logical coordinates (in
    ComplexControls, for example) provides great flexibility in
    controlling the look of a widget. Use visualRect() when necessary
    to translate logical coordinates into screen coordinates for
    drawing.

    \sa QStyleOption
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

    \sa unPolish()
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
void QStyle::unPolish(QWidget * /* widget */)
{
}

/*!
    \overload

    Late initialization of the QApplication object \a app.

    \sa unPolish()
*/
void QStyle::polish(QApplication * /* app */)
{
}

/*!
    \overload

    Undoes the polish of application \a app.

    \sa polish()
*/
void QStyle::unPolish(QApplication * /* app */)
{
}

/*!
    \overload

    The style may have certain requirements for color palettes. In
    this function it has the chance to change the palette \a pal
    according to these requirements.

    \sa QPalette, QApplication::setPalette()
*/
void QStyle::polish(QPalette &/*pal*/)
{
}

/*!
    Returns the appropriate area (see below) within rectangle \a rect in
    which to draw \a text using the font metrics \a metrics.

    If \a len is -1 (the default) all the \a text is drawn; otherwise
    only the first \a len characters of \a text are drawn. The text
    is aligned in accordance with \a alignment. The \a enabled bool
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
QRect QStyle::itemRect(const QFontMetrics &metrics, const QRect &rect, int alignment, bool enabled,
                       const QString &text, int len) const
{
    QRect result;
    int x, y, w, h;
    rect.getRect(&x, &y, &w, &h);
    if (!text.isEmpty()) {
        result = metrics.boundingRect(x, y, w, h, alignment, text, len);
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
    \overload

    Returns the appropriate area within rectangle \a rect in
    which to draw the \a pixmap.
*/
QRect QStyle::itemRect(const QRect &rect, int alignment, const QPixmap &pixmap) const
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
    else if ((alignment & Qt::AlignLeft) != Qt::AlignLeft && QApplication::reverseLayout())
        x += w - pixmap.width();
    result = QRect(x, y, pixmap.width(), pixmap.height());
    return result;
}

/*!
    \obsolete

    Returns the appropriate area within rectangle \a rect in which to
    draw the \a pixmap and \a text using \a painter. The \a alignment
    parameter determine the \a pixmap's alignment.
*/
QRect QStyle::itemRect(QPainter *painter, const QRect &rect, int alignment, bool enabled,
                       const QPixmap &pixmap, const QString &text, int len) const
{
    return !pixmap.isNull() ? itemRect(rect, alignment, pixmap)
                            : itemRect(painter->fontMetrics(), rect, alignment, enabled, text, len);
}

/*!
    Draws the \a text in rectangle \a rect using \a painter and
    palette \a pal.

    The pen color is specified with \a penColor. The
    \a enabled bool indicates whether or not the item is enabled;
    when reimplementing this bool should influence how the item is
    drawn.

    If \a len is -1 (the default), all the \a text is drawn;
    otherwise only the first \a len characters of \a text are drawn.
    The text is aligned and wrapped according to \a
    alignment.

    \sa Qt::Alignment
*/
void QStyle::drawItem(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                      bool enabled, const QString& text, int len, const QColor *penColor) const
{
    int x, y, w, h;
    rect.getRect(&x, &y, &w, &h);

    painter->setPen(penColor ? *penColor : pal.foreground().color());
    if (!text.isEmpty()) {
        if (!enabled && styleHint(SH_EtchDisabledText)) {
            painter->setPen(pal.light());
            painter->drawText(x+1, y+1, w, h, alignment, text, len);
            painter->setPen(pal.text());
        }
        painter->drawText(x, y, w, h, alignment, text, len);
    }
}

/*! \overload

    Draws the \a pixmap in rectangle \a rect using \a painter and the
    palette \a pal.
*/

void QStyle::drawItem(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                      bool enabled, const QPixmap &pixmap, const QColor *penColor) const
{
    int x, y, w, h;
    rect.getRect(&x, &y, &w, &h);

    painter->setPen(penColor?*penColor:pal.foreground().color());
    QPixmap pm(pixmap);
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += h/2 - pm.height()/2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += h - pm.height();
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += w - pm.width();
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += w/2 - pm.width()/2;
    else if (((alignment & Qt::AlignLeft) != Qt::AlignLeft) && QApplication::reverseLayout()) // Qt::AlignAuto && rightToLeft
        x += w - pm.width();

    QStyleOption option(0);
    option.palette = pal;
    if (!enabled)
        pm = generatedIconPixmap(IM_Disabled, pm, &option);

    int fillX = qMax(rect.x(), x);
    int fillY = qMax(rect.y(), y);
    int fillWidth = qMin(pm.width(), w);
    int fillHeight = qMin(pm.height(), h);
    painter->drawPixmap(fillX, fillY, pm, fillX - x, fillY - y, fillWidth, fillHeight);
}

/*!
    \fn void QStyle::drawItem(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal, bool enabled, const QPixmap &pixmap, const QString &text, int len, const QColor *penColor) const

    \overload

    Draws the \a pixmap in rectangle \a rect using \a painter and
    palette \a pal.
*/

/*!
    \enum QStyle::PrimitiveElement

    This enum represents a style's PrimitiveElements. A
    PrimitiveElement is a common GUI element, such as a checkbox
    indicator or button bevel.

    \value PE_ButtonCommand  Button used to initiate an action, for
        example, a QPushButton.
    \value PE_ButtonDefault  This button is the default button, e.g.
        in a dialog.
    \value PE_ButtonBevel  Generic button bevel.
    \value PE_ButtonTool  Tool button, for example, a QToolButton.
    \value PE_ButtonDropDown  Drop down button, for example, a tool
        button that displays a popup menu, for example, QMenu.

    \value PE_FocusRect  Generic focus indicator.

    \value PE_ArrowUp  Up arrow.
    \value PE_ArrowDown  Down arrow.
    \value PE_ArrowRight  Right arrow.
    \value PE_ArrowLeft  Left arrow.

    \value PE_SpinBoxUp  Up symbol for a spin widget, for example a
        QSpinBox.
    \value PE_SpinBoxDown  Down symbol for a spin widget.
    \value PE_SpinBoxPlus  Increase symbol for a spin widget.
    \value PE_SpinBoxMinus  Decrease symbol for a spin widget.

    \value PE_Indicator  On/off indicator, for example, a QCheckBox.
    \value PE_IndicatorMask  Bitmap mask for an indicator.
    \value PE_ExclusiveIndicator  Exclusive on/off indicator, for
        example, a QRadioButton.
    \value PE_ExclusiveIndicatorMask  Bitmap mask for an exclusive indicator.

    \value PE_DockWindowHandle  Tear off handle for dock windows and
        toolbars (e.g., \l{QDockWindow} and \l{QToolBar})
    \value PE_DockWindowSeparator  Item separator for dock window and
        toolbar contents.
    \value PE_DockWindowResizeHandle  Resize handle for dock windows.

    \value PE_Splitter  Splitter handle; see also QSplitter.

    \value PE_Panel  Generic panel frame; see also QFrame.
    \value PE_PanelPopup  Panel frame for popup windows/menus; see also QMenu.
    \value PE_PanelMenuBar  Panel frame for menu bars.
    \value PE_PanelDockWindow  Panel frame for dock windows and toolbars.
    \value PE_PanelTabWidget  Panel frame for tab widgets.
    \value PE_PanelLineEdit  Panel frame for line edits.
    \value PE_PanelGroupBox  Panel frame for group boxes.

    \value PE_TabBarBase  Area below tabs in a tab widget, for example,
        QTab.

    \value PE_MenuFrame  Frame displayed in a QMenu
    \value PE_MenuBarFrame  Frame displayed in a QMenuBar

    \value PE_HeaderSection  Section of a list or table header; see also
        QHeader.
    \value PE_HeaderArrow  Arrow used to indicate sorting on a list or table
        header
    \value PE_StatusBarSection  Section of a status bar; see also
        QStatusBar.

    \value PE_GroupBoxFrame  Frame around a group box; see also
        QGroupBox.
    \value PE_WindowFrame  Frame around a MDI window or a docking window.

    \value PE_Separator  Generic separator.

    \value PE_SizeGrip  Window resize handle; see also QSizeGrip.

    \value PE_CheckMark  Generic check mark; see also QCheckBox.

    \value PE_ScrollBarAddLine  Scroll bar line increase indicator.
        (i.e., scroll down); see also QScrollBar.
    \value PE_ScrollBarSubLine  Scroll bar line decrease indicator (i.e., scroll up).
    \value PE_ScrollBarAddPage  Scolllbar page increase indicator (i.e., page down).
    \value PE_ScrollBarSubPage  Scroll bar page decrease indicator (i.e., page up).
    \value PE_ScrollBarSlider  Scroll bar slider.
    \value PE_ScrollBarFirst  Scroll bar first line indicator (i.e., home).
    \value PE_ScrollBarLast  Scroll bar last line indicator (i.e., end).

    \value PE_ProgressBarChunk  Section of a progress bar indicator; see
        also QProgressBar.

    \value PE_CheckListController  Controller part of a list view item.
    \value PE_CheckListIndicator  Checkbox part of a list view item.
    \value PE_CheckListExclusiveIndicator  Radio button part of a
    list view item.
    \value PE_RubberBand  Rubber band used in such things as iconview.
    \omitvalue PE_RubberBandMask

    \value PE_TreeBranch  Lines used to represent the branch of a tree
    in a tree view.
    \value PE_SpinBoxSlider The optional slider part of a spin box.

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
    \enum QStyle::StyleFlags

    This enum represents flags for drawing PrimitiveElements. Not all
    primitives use all of these flags. Note that these flags may mean
    different things to different primitives. For an explanation of
    the relationship between primitives and their flags, as well as
    the different meanings of the flags, see the \link
    customstyles.html Style overview\endlink.

    \value Style_Active
    \value Style_AutoRaise
    \value Style_Bottom
    \value Style_ButtonDefault
    \value Style_Children
    \value Style_Default
    \value Style_Down
    \value Style_Editing
    \value Style_Enabled
    \value Style_FocusAtBorder
    \value Style_HasFocus
    \value Style_HasFocus
    \value Style_Horizontal
    \value Style_Item
    \value Style_MouseOver
    \value Style_NoChange
    \value Style_Off
    \value Style_On
    \value Style_Open
    \value Style_Raised
    \value Style_Rectangle
    \value Style_Selected
    \value Style_Sibling
    \value Style_Sunken
    \value Style_Top
    \value Style_Up

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
    \header \i PrimitiveElement \i Option Cast \i Style Flag \i Remark
    \row \i \l PE_FocusRect \i \l QStyleOptionFocusRect
         \i \l Style_FocusAtBorder
         \i Whether the focus is is at the border or inside the widget.
    \row \i{1,2} \l PE_Indicator \i{1,2} \l QStyleOptionButton
          \i \l Sytle_NoChange \i Indicates a "tri-state" checkbox.
    \row \i \l Style_On \i Indicates the indicator is checked.

    \row \i \l PE_ExclusiveIndicator \i \l QStyleOptionButton
          \i \l Style_On \i Indicates that a radio button is selected.
    \row \i{1,3} \l PE_CheckListExclusiveIndicator, \l PE_CheckListIndicator
         \i{1,3} \l QStyleOptionListView \i \l Style_On
         \i Indicates whether or not the controller is selected.
    \row \i \l Style_NoChange \i Indicates a "tri-state" controller.
    \row \i \l Style_Enable \i Indicates the controller is enabled.
    \row \i{1,2} \l PE_TreeBranch \i{1,2} \l QStyleOption
         \i \l Style_Down \i Indicates that the Tree Branch is pressed
    \row \i \l Style_Open \i Indicates that the tree branch is expanded.
    \row \i \l PE_HeaderArrow \i \l QStyleOptionHeader
         \i \l Style_Up \i Indicates that the arrow should be drawn up;
         otherwise it should be down.
    \row \i{1,3} \l PE_HeaderSection \i{1,3} \l QStyleOptionHeader
         \i \l Style_Sunken \i Indicates that the section is pressed.
    \row \i \l Style_Up \i Indicates that the sort indicator should be pointing up.
    \row \i \l Style_Off \i Indicates that the the section is not selected.
    \row \i \l PE_PanelGroupBox, \l PE_Panel, \l PE_PanelLineEdit,
            \l PE_PanelPopup, \l PE_PanelDockWindow
         \i \l QStyleOptionFrame \i \l Style_Sunken
         \i Indicates that the Frame should be sunken.
    \row \i \l PE_DockWindowHandle \i \l QStyleOptionDockWindow
         \i \l Style_Horizontal \i Indicates that the window handle is horizontal
         instead of vertical.
    \row \i \l PE_DockWindowSeparator \i \l QStyleOption
         \i \l Style_Horizontal \i Indicates that the separator is horizontal
         instead of vertical.
    \row \i \l PE_SpinBoxPlus, \l PE_SpinBoxMinus, \l PE_SpinBoxUp,
            \l PE_SpinBoxDown, \l PE_SpinBoxSlider
         \i \l QStyleOptionSpinBox
         \i \l Style_Sunken \i Indicates that the button is pressed.
    \endtable

    \sa PrimitiveElement, StyleFlags, QStyleOption
*/

/*!
    \enum QStyle::ControlElement

    This enum represents a ControlElement. A ControlElement is part of
    a widget that performs some action or displays information to the
    user.

    \value CE_PushButton  The bevel and default indicator of a QPushButton
    \value CE_PushButtonLabel  The label (iconset with text or pixmap)
        of a QPushButton

    \value CE_CheckBox  The indicator of a QCheckBox
    \value CE_CheckBoxLabel  The label (text or pixmap) of a QCheckBox

    \value CE_RadioButton  The indicator of a QRadioButton
    \value CE_RadioButtonLabel  The label (text or pixmap) of a QRadioButton

    \value CE_TabBarTab  The tab within a QTabBar (a QTab)
    \value CE_TabBarLabel  The label within a QTab

    \value CE_ProgressBarGroove  The groove where the progress
        indicator is drawn in a QProgressBar
    \value CE_ProgressBarContents  The progress indicator of a QProgressBar
    \value CE_ProgressBarLabel  The text label of a QProgressBar

    \value CE_ToolButtonLabel  A tool button's label
    \value CE_ToolBarButton

    \value CE_MenuBarItem  A menu item in a QMenuBar
    \value CE_MenuBarEmptyArea  The empty area of a QMenuBar

    \value CE_MenuItem  A menu item in a QMenu
    \value CE_MenuScroller  Scrolling areas in a QMenu when the
        style supports scrolling
    \value CE_MenuTearoff  A menu item representing the tear off section of
        a QMenu
    \value CE_MenuEmptyArea  The area in a menu without menu items
    \value CE_MenuVMargin  The vertical extra space on the top/bottom of a menu
    \value CE_MenuHMargin  The horizontal extra space on the left/right of a menu

    \value CE_DockWindowEmptyArea  The empty area of a QDockWindow

    \value CE_ToolBoxTab  The toolbox's tab area
    \value CE_HeaderLabel  The header's label

    \value CE_CustomBase  Base value for custom ControlElements;
    custom values must be greater than this value

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
    \header \i ControlElement \i Option Cast \i Style Flag \i Remark
    \row \i{1,4} \l CE_MenuItem, \l CE_MenuBarItem
         \i{1,4} \l QStyleOptionMenuItem
         \i \l Style_Active \i The menu item is the current item
    \row \i \l Style_Enabled \i The item is enabled
    \row \i \l Style_Down
         \i Set if the menu item is down
         (i.e., if the mouse button or the space bar is pressed)
    \row \i \l Style_HasFocus \i Set if the menubar has input focus
    \row \i{1,6} \l CE_PushButton, \l CE_PushButtonLabel
         \i{1,6} \l QStyleOptionButton
         \i \l Style_Enabled \i Set if the button is enabled
    \row \i \l Style_HasFocus \i Set if the button has input focus
    \row \i \l Style_Raised \i Set if the button is not down, not on and not flat
    \row \i \l Style_On \i Set if the button is a toggle button and is toggled on
    \row \i \l Style_Down
         \i Set if the button is down (i.e., the mouse button or the
         space bar is pressed on the button)
    \row \i \l Style_ButtonDefault \i Set if the button is a default button

    \row \i{1,6} \l CE_RadioButton, \l CE_RadioButtonLabel,
                 \l CE_CheckBox, \l CE_CheckBoxLabel
         \i{1,6} \l QStyleOptionButton
         \i \l Style_Enabled \i Set if the button is enabled
    \row \i \l Style_HasFocus \i Set if the button has input focus
    \row \i \l Style_On \i Set if the button is checked
    \row \i \l Style_Off \i Set if the button is not checked
    \row \i \l Style_NoChange \i Set if the button is in the NoChange state
    \row \i \l Style_Down
         \i Set if the button is down (i.e., the mouse button or
         the space bar is pressed on the button)
    \row \i{1,2} \l CE_ProgressBarContents, \l CE_ProgressBarLabel,
                 \l CE_ProgressBarGroove
         \i{1,2} \l QStyleOptionProgressBar
         \i \l Style_Enabled \i Set if the progressbar is enabled
    \row \i \l Style_HasFocus \i Set if the progressbar has input focus
    \row \i \l CE_HeaderLabel \i \l QStyleOptionHeader \i \i
    \row \i{1,7} \l CE_ToolButtonLabel
         \i{1,7} \l QStyleOptionToolButton
         \i \l Style_Enabled \i Set if the tool button is enabled
    \row \i \l Style_HasFocus \i Set if the tool button has input focus
    \row \i \l Style_Down
         \i Set if the tool button is down (i.e., a mouse button or
         the space bar is pressed)
    \row \i \l Style_On \i Set if the tool button is a toggle button and is toggled on
    \row \i \l Style_AutoRaise \i Set if the tool button has auto-raise enabled
    \row \i \l Style_MouseOver \i Set if the mouse pointer is over the tool button
    \row \i \l Style_Raised \i Set if the button is not down and is not on
    \row \i \l CE_ToolBoxTab \i \l QStyleOptionToolBox
         \i \l Style_Selected \i The tab is the currently selected tab
    \endtable

    \sa ControlElement, StyleFlags, QStyleOption
*/

/*!
    \fn void QStyle::drawControlMask(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const

    Draw a bitmask for the ControlElement \a element using the \a
    painter with the style options specified by \a option. See
    drawControl() for an explanation of the use of \a option and \a
    widget.
*/

/*!
    \enum QStyle::SubRect

    This enum represents a sub-area of a widget. Style implementations
    use these areas to draw the different parts of a widget.

    \value SR_PushButtonContents  Area containing the label (iconset
        with text or pixmap)
    \value SR_PushButtonFocusRect  Area for the focus rect (usually
        larger than the contents rect)

    \value SR_CheckBoxIndicator  Area for the state indicator (e.g., check mark)
    \value SR_CheckBoxContents  Area for the label (text or pixmap)
    \value SR_CheckBoxFocusRect  Area for the focus indicator

    \value SR_RadioButtonIndicator  Area for the state indicator
    \value SR_RadioButtonContents  Area for the label
    \value SR_RadioButtonFocusRect  Area for the focus indicator

    \value SR_ComboBoxFocusRect  Area for the focus indicator

    \value SR_SliderFocusRect  Area for the focus indicator

    \value SR_DockWindowHandleRect  Area for the tear-off handle

    \value SR_ProgressBarGroove  Area for the groove
    \value SR_ProgressBarContents  Area for the progress indicator
    \value SR_ProgressBarLabel  Area for the text label

    \value SR_ToolButtonContents  Area for the tool button's label

    \value SR_DialogButtonAccept  Area for a dialog's accept button
    \value SR_DialogButtonReject  Area for a dialog's reject button
    \value SR_DialogButtonApply  Area for a dialog's apply button
    \value SR_DialogButtonHelp  Area for a dialog's help button
    \value SR_DialogButtonAll  Area for a dialog's all button
    \value SR_DialogButtonRetry  Area for a dialog's retry button
    \value SR_DialogButtonAbort  Area for a dialog's abort button
    \value SR_DialogButtonIgnore  Area for a dialog's ignore button
    \value SR_DialogButtonCustom  Area for a dialog's custom widget
    area (in the button row)

    \value SR_ToolBoxTabContents  Area for a toolbox tab's icon and label

    \value SR_CustomBase  Base value for custom ControlElements
    Custom values must be greater than this value

    \value SR_ToolBarButtonContents
    \value SR_ToolBarButtonMenu

    \value SR_HeaderArrow
    \value SR_HeaderLabel

    \value SR_PanelTab

    \sa subRect()
*/

/*!
    \fn QRect QStyle::subRect(SubRect subRect, const QStyleOption *option, \
                              const QFontMetrics &metrics, const QWidget *widget) const

    Returns the sub-area \a subRect as described in \a option in logical
    coordinates.

    The \a metrics argument may be helpful in determining the size of the sub-area.

    The \a widget argument is optional and may contain a widget that
    may aid determining the subRect.

    The QStyleOption can be cast to the appropriate type based on the
    value of \a subRect. See the table below for the appropriate \a
    widget casts:

    \table
    \header \i SubRect \i Option Cast
    \row \i \l SR_PushButtonContents   \i \l QStyleOptionButton
    \row \i \l SR_PushButtonFocusRect  \i \l QStyleOptionButton
    \row \i \l SR_CheckBoxIndicator    \i \l QStyleOptionButton
    \row \i \l SR_CheckBoxContents     \i \l QStyleOptionButton
    \row \i \l SR_CheckBoxFocusRect    \i \l QStyleOptionButton
    \row \i \l SR_RadioButtonIndicator \i \l QStyleOptionButton
    \row \i \l SR_RadioButtonContents  \i \l QStyleOptionButton
    \row \i \l SR_RadioButtonFocusRect \i \l QStyleOptionButton
    \row \i \l SR_ComboBoxFocusRect    \i \l QStyleOptionComboBox
    \row \i \l SR_DockWindowHandleRect \i \l QStyleOptionDockWindow
    \row \i \l SR_ProgressBarGroove    \i \l QStyleOptionProgressBar
    \row \i \l SR_ProgressBarContents  \i \l QStyleOptionProgressBar
    \row \i \l SR_ProgressBarLabel     \i \l QStyleOptionProgressBar
    \endtable

    \sa SubRect QStyleOption
*/

/*!
    \enum QStyle::ComplexControl

    This enum represents a ComplexControl. ComplexControls have
    different behavior depending upon where the user clicks on them
    or which keys are pressed.

    \value CC_SpinBox
    \value CC_ComboBox
    \value CC_ScrollBar
    \value CC_Slider
    \value CC_ToolButton
    \value CC_TitleBar
    \value CC_ListView

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
    \value SC_SpinBoxButtonField  Spinwidget button field
    \value SC_SpinBoxSlider  Spinwidget optional slider

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

    \value SC_ListView  The list view area
    \value SC_ListViewExpand  Expand item (i.e., show/hide child items)
    \value SC_All  Special value that matches all SubControls
    \omitvalue SC_ListViewBranch

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
    \header \i ComplexControl \i Option Cast \i Style Flag \i Remark
    \row \i{1,2} \l{CC_SpinBox} \i{1,2} \l QStyleOptionSpinBox
         \i \l Style_Enabled \i Set if the spin box is enabled
    \row \i \l Style_HasFocus \i Set if the spin box has input focus

    \row \i{1,2} \l {CC_ComboBox} \i{1,2} \l QStyleOptionComboBox
         \i \l Style_Enabled \i Set if the combobox is enabled
    \row \i \l Style_HasFocus \i Set if the combobox has input focus

    \row \i{1,2} \l {CC_ScrollBar} \i{1,2} \l QStyleOptionSlider
         \i \l Style_Enabled \i Set if the scroll bar is enabled
    \row \i \l Style_HasFocus \i Set if the scroll bar has input focus

    \row \i{1,2} \l {CC_Slider} \i{1,2} \l QStyleOptionSlider
         \i \l Style_Enabled \i Set if the slider is enabled
    \row \i \l Style_HasFocus \i Set if the slider has input focus

    \row \i{1,6} \l {CC_ToolButton} \i{1,6} \l QStyleOptionToolButton
         \i \l Style_Enabled \i Set if the tool button is enabled
    \row \i \l Style_HasFocus \i Set if the tool button has input focus
    \row \i \l Style_Down \i Set if the tool button is down (i.e., a mouse
        button or the space bar is pressed)
    \row \i \l Style_On \i Set if the tool button is a toggle button
        and is toggled on
    \row \i \l Style_AutoRaise \i Set if the tool button has auto-raise enabled
    \row \i \l Style_Raised \i Set if the button is not down, not on, and doesn't
        contain the mouse when auto-raise is enabled

    \row \i \l{CC_TitleBar} \i \l QStyleOptionTitleBar
         \i \l Style_Enabled \i Set if the title bar is enabled

    \row \i \l{CC_ListView} \i \l QStyleOptionListView
         \i \l Style_Enabled \i Set if the list view is enabled
    \endtable

    \sa ComplexControl SubControl QStyleOptionComplex
*/

/*!
    \fn void QStyle::drawComplexControlMask(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const

    Draws a bitmask for the ComplexControl \a control using \a
    painter with the style options specified by \a option. See
    drawComplexControl() for an explanation of the use of the \a
    widget and \a option arguments.

    The \c rect member of the QStyleOptionComplex \a option parameter
    should be expressed in logical coordinates. Reimplementations of
    this function can use visualRect() to convert the logical
    coordinates into screen coordinates before calling
    drawPrimitive() or drawControl().

    \sa drawComplexControl() ComplexControl QStyleOptionComplex
*/

/*!
    \fn QRect QStyle::querySubControlMetrics(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const

    Returns the rectangle for the SubControl \a subControl in the
    ComplexControl \a control, with the style options specified by \a
    option in logical coordinates.

    The \a option argument is a pointer to a QStyleOptionComplex or one of its
    subclasses. The structure can be cast to the appropriate type based on the
    value of \a control. See drawComplexControl() for details.

    The \a widget is optional and can contain additional information
    for the function.

    \sa drawComplexControl() ComplexControl SubControl QStyleOptionComplex
*/

/*!
    \fn SubControl QStyle::querySubControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos, const QWidget *widget) const

    Returns the SubControl in the ComplexControl \a control with the
    style options specified by \a option at the point \a pos. The \a
    option argument is a pointer to a QStyleOptionComplex structure
    or one of its subclasses. The structure can be cast to the
    appropriate type based on the value of \a control. See
    drawComplexControl() for details.

    The \a widget argument is optional and can contain additional
    information for the functions.

    Note that \a pos is expressed in screen coordinates. When using
    querySubControlMetrics() to check for hits and misses, use
    visualRect() to change the logical coordinates into screen
    coordinates.

    \sa drawComplexControl() ComplexControl SubControl querySubControlMetrics() QStyleOptionComplex
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
    \value PM_SpinBoxFrameWidth  Frame width of a spin box
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

    \value PM_DockWindowSeparatorExtent  Width of a separator in a
        horizontal dock window and the height of a separator in a
        vertical dock window
    \value PM_DockWindowHandleExtent  Width of the handle in a
        horizontal dock window and the height of the handle in a
        vertical dock window
    \value PM_DockWindowFrameWidth  Frame width of a dock window

    \value PM_MenuBarFrameWidth  Frame width of a menubar
    \value PM_MenuBarItemSpacing  Spacing between menubar items
    \value PM_MenuBarHMargin  Spacing between menubar items and top/bottom of bar
    \value PM_MenuBarVMargin  Spacing between menubar items and left/right of bar

    \value PM_ToolBarItemSpacing  Spacing between toolbar items

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

    \value PM_MenuFrameWidth  Border width (applied on all sides) for a QMenu
    \value PM_MenuHMargin  Additional border (used on left and right) for a QMenu
    \value PM_MenuVMargin  Additional border (used for bottom and top) for a QMenu
    \value PM_MenuScrollerHeight  Height of the scroller area in a QMenu
    \value PM_MenuScrollerHeight  Height of the scroller area in a QMenu
    \value PM_MenuTearoffHeight  Height of a tear off area in a QMenu
    \value PM_MenuDesktopFrameWidth

    \value PM_CheckListButtonSize  Area (width/height) of the
        checkbox/radio button in a QCheckListItem
    \value PM_CheckListControllerSize  Area (width/height) of the
        controller in a QCheckListItem

    \value PM_DialogButtonsSeparator  Distance between buttons in a dialog buttons widget
    \value PM_DialogButtonsButtonWidth  Minimum width of a button in a dialog buttons widget
    \value PM_DialogButtonsButtonHeight  Minimum height of a button in a dialog buttons widget

    \value PM_HeaderMarkSize
    \value PM_HeaderGripMargin
    \value PM_HeaderMargin
    \value PM_SpinBoxSliderHeight The height of the optional spin box slider

    \value PM_CustomBase  Base value for custom ControlElements
    Custom values must be greater than this value

    \value PM_DefaultToplevelMargin
    \value PM_DefaultChildMargin
    \value PM_DefaultLayoutSpacing

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
    \header \i PixelMetric \i Option Cast
    \row \i \l PM_SliderControlThickness \i \l QStyleOptionSlider
    \row \i \l PM_SliderLength           \i \l QStyleOptionSlider
    \row \i \l PM_SliderTickmarkOffset   \i \l QStyleOptionSlider
    \row \i \l PM_SliderSpaceAvailable   \i \l QStyleOptionSlider
    \row \i \l PM_ScrollBarExtent        \i \l QStyleOptionSlider
    \row \i \l PM_TabBarTabOverlap       \i \l QStyleOptionTab
    \row \i \l PM_TabBarTabHSpace        \i \l QStyleOptionTab
    \row \i \l PM_TabBarTabVSpace        \i \l QStyleOptionTab
    \row \i \l PM_TabBarBaseHeight       \i \l QStyleOptionTab
    \row \i \l PM_TabBarBaseOverlap      \i \l QStyleOptionTab
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
    \value CT_DockWindow
    \value CT_Header
    \value CT_HeaderSection
    \value CT_LineEdit
    \value CT_Menu
    \value CT_Menu
    \value CT_MenuBar
    \value CT_MenuBarItem
    \value CT_MenuItem
    \value CT_ProgressBar
    \value CT_PushButton
    \value CT_RadioButton
    \value CT_SizeGrip
    \value CT_Slider
    \value CT_SpinBox
    \value CT_Splitter
    \value CT_TabBarTab
    \value CT_TabWidget
    \value CT_ToolButton
    \value CT_ToolBarButton

    \value CT_CustomBase  Base value for custom ControlElements.
    Custom values must be greater than this value.

    \sa sizeFromContents()
*/

/*!
    \fn QSize QStyle::sizeFromContents(ContentsType type, const QStyleOption *option, \
                                       const QSize &contentsSize, const QFontMetrics &metrics, \
                                       const QWidget *widget) const

    Returns the size of styled object described in \a option based on the
    contents size \a contentsSize. The font metrics in \a metrics can aid
    in determining the size.

    The \a option argument is a pointer to a QStyleOption or one of its
    subclasses. The \a option can be cast to the appropriate type based
    on the value of \a type. The widget \a widget is optional argument and can
    contain extra information used for calculating the size.
    See the table below for the appropriate \a option usage:

    \table
    \header \i ContentsType   \i Option Cast
    \row \i \l CT_PushButton  \i \l QStyleOptionButton
    \row \i \l CT_CheckBox    \i \l QStyleOptionButton
    \row \i \l CT_RadioButton \i \l QStyleOptionButton
    \row \i \l CT_ToolButton  \i \l QStyleOptionToolButton
    \row \i \l CT_ComboBox    \i \l QStyleOptionComboBox
    \row \i \l CT_Splitter    \i \l QStyleOption
    \row \i \l CT_DockWindow  \i \l QStyleOptionDockWindow
    \row \i \l CT_ProgressBar \i \l QStyleOptionProgressBar
    \row \i \l CT_MenuItem    \i \l QStyleOptionMenuItem
    \endtable

    \sa ContentsType QStyleOption
*/

/*!
    \enum QStyle::StyleHint

    This enum represents a StyleHint. A StyleHint is a general look
    and/or feel hint.

    \value SH_EtchDisabledText Disabled text is "etched" as it is on Windows.

    \value SH_GUIStyle The GUI style to use.

    \value SH_ScrollBar_BackgroundRole  The background role for a
        QScrollBar.

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

    \value SH_ListViewExpand_SelectMouseType  Which type of mouse event should
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

    \omitvalue SH_ScrollBar_BackgroundMode

    \value SH_BlinkCursorWhenTextSelected  Whether cursor should blink
        when text is selected.

    \value SH_RichText_FullWidthSelection  Whether richtext selections
        should extend to the full width of the document.

    \value SH_GroupBox_TextLabelVerticalAlignment  How to vertically align a
        groupbox's text label.

    \value SH_GroupBox_TextLabelColor  How to paint a groupbox's text label.

    \value SH_DialogButtons_DefaultButton  Which button gets the
        default status in a dialog's button widget.

    \value SH_ToolButton_Uses3D  Whether QToolButtons should
    use a 3D frame when the mouse is over them.

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

    \value SH_CustomBase  Base value for custom ControlElements.
    Custom values must be greater than this value.

    \omitvalue SH_UnderlineAccelerator

    \sa styleHint()
*/

/*!
    \fn int QStyle::styleHint(StyleHint hint, const QStyleOption *option, \
                              const QWidget *widget, QStyleHintReturn *returnData) const

    Returns the style hint \a hint for \a widget descriped in the
    QStyleOption \a option. Currently \a returnData and \a widget are
    not used; they are provided for future enhancement. The \a option
    parameters is used only in SH_ComboBox_Popup and
    SH_GroupBox_TextLabelColor.

    For an explanation of the return value, see \l StyleHint.
*/

/*!
    \enum QStyle::StandardPixmap

    This enum represents a StandardPixmap. A StandardPixmap is a pixmap that
    can follow some existing GUI style or guideline.

    \value SP_TitleBarMinButton  Minimize button on title bars (e.g.,
        in QWorkspace)
    \value SP_TitleBarMaxButton  Maximize button on title bars
    \value SP_TitleBarCloseButton  Close button on title bars
    \value SP_TitleBarNormalButton  Normal (restore) button on title bars
    \value SP_TitleBarShadeButton  Shade button on title bars
    \value SP_TitleBarUnshadeButton  Unshade button on title bars
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
    \value SP_DockWindowCloseButton  Close button on dock windows (see also QDockWindow)
    \value SP_CustomBase  Base value for custom ControlElements;
    custom values must be greater than this value

    \sa standardPixmap()
*/

/*!
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
    \fn QPixmap QStyle::generatedIconPixmap(IconMode iconMode, const QPixmap &pixmap, \
                                    const QStyleOption *option) const

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
    \fn QRect QStyle::visualRect(const QRect &logicalRect, const QWidget *widget);

    Returns the rectangle \a logicalRect expressed in screen
    coordinates. The bounding rectangle for \a widget is used to
    perform the translation.

    This function is provided to aid style implementors in supporting
    right-to-left desktops.

    \sa QApplication::reverseLayout()
*/
QRect QStyle::visualRect(const QRect &logicalRect, const QWidget *widget)
{
    if (!QApplication::reverseLayout())
        return logicalRect;
    QRect boundingRect = widget->rect();
    QRect rect = logicalRect;
    rect.moveBy(2 * (boundingRect.right() - logicalRect.right()) +
                logicalRect.width() - boundingRect.width(), 0);
    return rect;
}

/*!
    \overload

    Returns the rectangle \a logicalRect converted to screen
    coordinates. The \a boundingRect rectangle is used to perform the
    translation.

    This function is provided to aid style implementors in supporting
    right-to-left desktops.

    \sa QApplication::reverseLayout()
*/
QRect QStyle::visualRect(const QRect &logicalRect, const QRect &boundingRect)
{
    if (!QApplication::reverseLayout())
        return logicalRect;
    QRect rect = logicalRect;
    rect.moveBy(2 * (boundingRect.right() - logicalRect.right()) +
                logicalRect.width() - boundingRect.width(), 0);
    return rect;
}

/*!
    Returns the point \a logicalPos converted to screen coordinates.
    The bounding rectangle for \a widget is used to perform the
    translation.

    This function is provided to aid style implementors in supporting
    right-to-left desktops.

    \sa QApplication::reverseLayout()
*/
QPoint QStyle::visualPos(const QPoint &logicalPos, const QWidget *widget)
{
    if (!QApplication::reverseLayout())
        return logicalPos;
    return QPoint(widget->rect().right() - logicalPos.x(), logicalPos.y());
}

/*!
    \overload

    Returns the point \a logicalPos converted to screen coordinates.
    The \a boundingRect rectangle is used to perform the translation.

    This function is provided to aid style implementors in supporting
    right-to-left desktops.

    \sa QApplication::reverseLayout()
*/
QPoint QStyle::visualPos(const QPoint &logicalPos, const QRect &boundingRect)
{
    if (!QApplication::reverseLayout())
        return logicalPos;
    return QPoint(boundingRect.right() - logicalPos.x(), logicalPos.y());
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

    \sa valueFromPosition()
*/

int QStyle::positionFromValue(int min, int max, int logicalValue, int span, bool upsideDown)
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

    \sa positionFromValue()
*/

int QStyle::valueFromPosition(int min, int max, int pos, int span, bool upsideDown)
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

/*! \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                              int flags, const QColorGroup &colorgroup, bool enabled,
                              const QString &text, int len = -1,
                              const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*! \fn void QStyle::drawItem(QPainter *p, const QRect &r,
                              int flags, const QColorGroup colorgroup, bool enabled,
                              const QPixmap &pixmap,
                              const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

/*! void QStyle::drawItem(QPainter *p, const QRect &r,
                          int flags, const QColorGroup colorgroup, bool enabled,
                          const QPixmap *pixmap,
                          const QString &text, int len = -1,
                          const QColor *penColor = 0) const

    Use one of the drawItem() overloads that takes a QPalette instead
    of a QColorGroup.
*/

#endif // QT_NO_STYLE
