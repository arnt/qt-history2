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
    \class QStyle qstyle.h
    \brief The QStyle class specifies the look and feel of a GUI.

    \ingroup appearance

    A large number of GUI elements are common to many widgets. The
    QStyle class allows the look of these elements to be modified
    across all widgets that use the QStyle functions. It also
    provides "feel" options for widgets.

    Although it is not possible to completely enumerate the look of
    graphical elements and the feel of widgets in a GUI, QStyle
    provides a considerable amount of control and customisability.

    In Qt 1.x the look and feel option for widgets was specified by a
    single value: the Qt::GUIStyle. Starting with Qt 2.0, this notion was
    expanded to allow the look to be specified by virtual drawing
    functions.

    Derived classes may reimplement some or all of the drawing
    functions to modify the look of all widgets that use those
    functions.

    Languages written from right to left (such as Arabic and Hebrew)
    usually also mirror the whole layout of widgets. If you design a
    style, you should take special care when drawing asymmetric
    elements to make sure that they also look correct in a mirrored
    layout. Start your application with \c -reverse to check the
    mirrored layout. Also notice that, for a reversed layout, the
    light usually comes from top right instead of the top left.

    The actual reverse layout is performed automatically when
    possible. However, for the sake of flexibility, the translation
    cannot be performed everywhere. The documentation for each
    function in the QStyle API states whether the function
    expects/returns logical or screen coordinates. Using logical
    coordinates (in ComplexControls, for example) provides great
    flexibility in controlling the look of a widget. Use visualRect()
    when necessary to translate logical coordinates into screen
    coordinates for drawing.

    In Qt versions prior to 3.0, if you wanted a low level route into
    changing the appearance of a widget, you had to reimplement
    polish(). With 3.0 and later, the recommended approach
    is to reimplement the draw functions, for example drawItem(),
    drawPrimitive(), drawControl(), drawControlMask(),
    drawComplexControl(), and drawComplexControlMask(). Each of these
    functions is called with a range of parameters that provide
    information that you can use to determine how to draw them, e.g.
    style flags, rectangle, color group, etc.

    In Qt 4.0, the majority of the information about what is to be
    drawn and how it should be drawn, is specified by a QStyleOption
    structure. The widget parameter that was mandatory in Qt 3.0, is
    now optional, but it has been kept for developers who may want
    it. Developers who use the widget pointer must be careful and use
    should use qt_cast() to get the correct widget as there are no
    guarantees about what class the widget will be.

    For information on changing elements of an existing style or
    creating your own style see the \link customstyles.html Style
    overview\endlink.

    Styles can also be created as \link plugins-howto.html
    plugins\endlink.
*/

/*!
  \enum Qt::GUIStyle

  \obsolete

  \value WindowsStyle
  \value MotifStyle
    \omitvalue MacStyle
    \omitvalue Win3Style
    \omitvalue PMStyle
*/

/*!
    \enum Qt::UIEffect

    \value UI_General
    \value UI_AnimateMenu
    \value UI_FadeMenu
    \value UI_AnimateCombo
    \value UI_AnimateTooltip
    \value UI_FadeTooltip

    \omitvalue UI_AnimateToolBox
*/

/*!
    Constructs a QStyle.
*/
QStyle::QStyle()
{ }

/*!
    Destroys the style and frees all allocated resources.
*/
QStyle::~QStyle()
{ }

/*
  \fn Qt::GUIStyle QStyle::guiStyle() const
  \obsolete

  Returns an indicator to the additional "feel" component of a
  style. Current supported values are Qt::WindowsStyle and Qt::MotifStyle.
*/



/*!
    Initializes the appearance of widget \a w.

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
void QStyle::polish(QWidget* /*w*/)
{
}

/*!
    Undoes the initialization of widget \a w's appearance.

    This function is the counterpart to polish. It is called for every
    polished widget when the style is dynamically changed. The former
    style has to unpolish its settings before the new style can polish
    them again.

    \sa polish()
*/
void QStyle::unPolish(QWidget* /*w*/)
{
}


/*!
    \overload

    Late initialization of the QApplication object \a app.

    \sa unPolish()
*/
void QStyle::polish(QApplication* /*app*/)
{
}

/*!
    \overload

    Undoes the polish of application \a app.

    \sa polish()
*/
void QStyle::unPolish(QApplication* /*app*/)
{
}

/*!
    \overload

    The style may have certain requirements for color palettes. In
    this function it has the chance to change the palette \a pal according to
    these requirements.

    \sa QPalette, QApplication::setPalette()
*/
void QStyle::polish(QPalette &/*pal*/)
{
}

/*!
    Returns the appropriate area (see below) within rectangle \a r in
    which to draw \a text using the font metrics \a fm. If \a len is
    -1 (the default) all the \a text is drawn; otherwise only the
    first \a len characters of \a text are drawn. The text is aligned
    in accordance with the alignment \a flags (see
    \l{Qt::Alignment}). The \a enabled bool indicates whether or
    not the item is enabled.

    If \a r is larger than the area needed to render the \a text the
    rectangle that is returned will be offset within \a r in
    accordance with the alignment \a flags. For example if \a flags is
    \c Qt::AlignCenter the returned rectangle will be centered within \a
    r. If \a r is smaller than the area needed the rectangle that is
    returned will be \e larger than \a r (the smallest rectangle large
    enough to render the \a text).
*/
QRect QStyle::itemRect(const QFontMetrics &fm, const QRect &r,
                        int flags, bool enabled,
                        const QString& text, int len) const
{
    QRect result;
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    if (!text.isEmpty()) {
        result = fm.boundingRect(x, y, w, h, flags, text, len);
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

    Returns the appropriate area within rectangle \a r in
    which to draw the \a pixmap. The \a flags determine the \a
    pixmap's alignment.
*/
QRect QStyle::itemRect(const QRect &r,
                        int flags, const QPixmap &pixmap) const
{
    QRect result;
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    if ((flags & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += h/2 - pixmap.height()/2;
    else if ((flags & Qt::AlignBottom) == Qt::AlignBottom)
        y += h - pixmap.height();
    if ((flags & Qt::AlignRight) == Qt::AlignRight)
        x += w - pixmap.width();
    else if ((flags & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += w/2 - pixmap.width()/2;
    else if ((flags & Qt::AlignLeft) != Qt::AlignLeft && QApplication::reverseLayout())
        x += w - pixmap.width();
    result = QRect(x, y, pixmap.width(), pixmap.height());
    return result;
}

/*!
  \obsolete

    Returns the appropriate area within rectangle \a r in which to draw the \a
    pixmap and \a text using the painter \a p. The \a flags determine the \a
    pixmap's alignment.
*/
QRect QStyle::itemRect(QPainter *p, const QRect &r,
                        int flags, bool enabled,
                        const QPixmap &pixmap,
                        const QString &text, int len) const
{
    return !pixmap.isNull() ? itemRect(r, flags, pixmap)
                            : itemRect(p->fontMetrics(), r, flags, enabled, text, len);
}

/*!
    Draws the \a text in rectangle \a r using painter \a p and palette
    \a pal. The pen color is specified with \a penColor. The \a
    enabled bool indicates whether or not the item is enabled; when
    reimplementing this bool should influence how the item is
    drawn. If \a len is -1 (the default) all the \a text is drawn;
    otherwise only the first \a len characters of \a text are
    drawn. The text is aligned and wrapped according to the alignment
    \a flags (see \l{Qt::Alignment}).
*/
void QStyle::drawItem(QPainter *p, const QRect &r,
                       int flags, const QPalette &pal, bool enabled,
                       const QString& text, int len,
                       const QColor *penColor) const
{
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);

    p->setPen(penColor ? *penColor : pal.foreground().color());
    if (!text.isEmpty()) {
        if (!enabled && styleHint(SH_EtchDisabledText)) {
            p->setPen(pal.light());
            p->drawText(x+1, y+1, w, h, flags, text, len);
            p->setPen(pal.text());
        }
        p->drawText(x, y, w, h, flags, text, len);
    }
}

/*! \overload

    Draws the \a pixmap in rectangle \a r using painter \a p and the
    palette \a pal.
*/

void QStyle::drawItem(QPainter *p, const QRect &r,
                       int flags, const QPalette &pal, bool enabled,
                       const QPixmap &pixmap,
                       const QColor *penColor) const
{
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);

    p->setPen(penColor?*penColor:pal.foreground().color());
    QPixmap pm(pixmap);
    if ((flags & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += h/2 - pm.height()/2;
    else if ((flags & Qt::AlignBottom) == Qt::AlignBottom)
        y += h - pm.height();
    if ((flags & Qt::AlignRight) == Qt::AlignRight)
        x += w - pm.width();
    else if ((flags & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += w/2 - pm.width()/2;
    else if (((flags & Qt::AlignLeft) != Qt::AlignLeft) && QApplication::reverseLayout()) // Qt::AlignAuto && rightToLeft
        x += w - pm.width();

    QStyleOption opt(0);
    opt.palette = pal;
    if (!enabled)
        pm = stylePixmap(PT_Disabled, pm, &opt);

    int fillX = qMax(r.x(), x);
    int fillY = qMax(r.y(), y);
    int fillWidth = qMin(pm.width(), w);
    int fillHeight = qMin(pm.height(), h);
    p->drawPixmap(fillX, fillY, pm, fillX - x, fillY - y, fillWidth, fillHeight);
}

/*!
    \fn void QStyle::drawItem(QPainter *p, const QRect &r, int flags, const QPalette &pal, bool enabled, const QPixmap &pixmap, const QString &text, int len, const QColor *penColor) const

    \overload

    Draws the \a pixmap in rectangle \a r using painter \a p and palette \a pal.
*/

/*!
    \enum QStyle::PrimitiveElement

    This enum represents a style's PrimitiveElements. A
    PrimitiveElement is a common GUI element, such as a checkbox
    indicator or button bevel.

    \value PE_ButtonCommand  button used to initiate an action, for
        example, a QPushButton.
    \value PE_ButtonDefault  this button is the default button, e.g.
        in a dialog.
    \value PE_ButtonBevel  generic button bevel.
    \value PE_ButtonTool  tool button, for example, a QToolButton.
    \value PE_ButtonDropDown  drop down button, for example, a tool
        button that displays a popup menu, for example, QMenu.


    \value PE_FocusRect  generic focus indicator.


    \value PE_ArrowUp  up arrow.
    \value PE_ArrowDown  down arrow.
    \value PE_ArrowRight  right arrow.
    \value PE_ArrowLeft  left arrow.


    \value PE_SpinBoxUp  up symbol for a spin widget, for example a
        QSpinBox.
    \value PE_SpinBoxDown down symbol for a spin widget.
    \value PE_SpinBoxPlus  increase symbol for a spin widget.
    \value PE_SpinBoxMinus  decrease symbol for a spin widget.


    \value PE_Indicator  on/off indicator, for example, a QCheckBox.
    \value PE_IndicatorMask  bitmap mask for an indicator.
    \value PE_ExclusiveIndicator  exclusive on/off indicator, for
        example, a QRadioButton.
    \value PE_ExclusiveIndicatorMask  bitmap mask for an exclusive indicator.


    \value PE_DockWindowHandle  tear off handle for dock windows and
        toolbars, for example \l{QDockWindow}s and \l{QToolBar}s.
    \value PE_DockWindowSeparator  item separator for dock window and
        toolbar contents.
    \value PE_DockWindowResizeHandle  resize handle for dock windows.

    \value PE_Splitter  splitter handle; see also QSplitter.


    \value PE_Panel  generic panel frame; see also QFrame.
    \value PE_PanelPopup  panel frame for popup windows/menus; see also QMenu.
    \value PE_PanelMenuBar  panel frame for menu bars.
    \value PE_PanelDockWindow  panel frame for dock windows and toolbars.
    \value PE_PanelTabWidget  panel frame for tab widgets.
    \value PE_PanelLineEdit  panel frame for line edits.
    \value PE_PanelGroupBox  panel frame for group boxes.

    \value PE_TabBarBase  area below tabs in a tab widget, for example,
        QTab.

    \value PE_MenuFrame frame displayed in a QMenu
    \value PE_MenuBarFrame frame displayed in a QMenuBar

    \value PE_HeaderSection  section of a list or table header; see also
        QHeader.
    \value PE_HeaderArrow arrow used to indicate sorting on a list or table
        header
    \value PE_StatusBarSection  section of a status bar; see also
        QStatusBar.


    \value PE_GroupBoxFrame  frame around a group box; see also
        QGroupBox.
    \value PE_WindowFrame  frame around a MDI window or a docking window.


    \value PE_Separator  generic separator.


    \value PE_SizeGrip  window resize handle; see also QSizeGrip.


    \value PE_CheckMark  generic check mark; see also QCheckBox.


    \value PE_ScrollBarAddLine  scrollbar line increase indicator.
        (i.e. scroll down); see also QScrollBar.
    \value PE_ScrollBarSubLine  scrollbar line decrease indicator (i.e. scroll up).
    \value PE_ScrollBarAddPage  scolllbar page increase indicator (i.e. page down).
    \value PE_ScrollBarSubPage  scrollbar page decrease indicator (i.e. page up).
    \value PE_ScrollBarSlider  scrollbar slider.
    \value PE_ScrollBarFirst  scrollbar first line indicator (i.e. home).
    \value PE_ScrollBarLast  scrollbar last line indicator (i.e. end).


    \value PE_ProgressBarChunk  section of a progress bar indicator; see
        also QProgressBar.

    \value PE_CheckListController controller part of a listview item.
    \value PE_CheckListIndicator checkbox part of a listview item.
    \value PE_CheckListExclusiveIndicator radiobutton part of a
    listview item.
    \value PE_RubberBand rubber band used in such things as iconview.
    \omitvalue PE_RubberBandMask

    \value PE_TreeBranch lines used to represent the branch of a tree
    in a tree view.
    \value PE_SpinBoxSlider The optional slider part of a spin box.

    \value PE_CustomBase  base value for custom PrimitiveElements.
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
    \fn void QStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const = 0

    Draw the primitive option \a pe with painter \a p using the style
    options specified by \a opt.

    The \a w argument is optional and may contain a widget that may
    aid in drawing the primitive.

    The \a opt parameter can be qt_cast()'ed to the correct sub-class
    of the QStyleOption.

    What follows is a table of the elements and the QStyleOption
    structure they can be cast to. The flags stored in the QStyleOption
    state variable are also listed. If a PrimitiveElement is not
    listed here, then it uses the default QStyleOption.

    The QStyleOption is the the following for the following types of PrimitiveElements.
    \table
    \header \i PrimitiveElement \i Option Cast \i Style Flags \i Notes
    \row \i \l PE_FocusRect \i (const \l QStyleOptionFocusRect *)
         \i \l Style_FocusAtBorder
         \i Whether the focus is is at the border or inside the widget.
    \row \i{1,2} \l PE_Indicator \i{1,2} (const \l QStyleOptionButton *)
          \i \l Sytle_NoChange \i Indicates a "tri-state" checkbox.
    \row \i \l Style_On \i Indicates the indicator is checked.

    \row \i \l PE_ExclusiveIndicator \i (const \l QStyleOptionButton *)
          \i \l Style_On \i Indicates that a radiobutton is selected.
    \row \i{1,3} \l PE_CheckListExclusiveIndicator and \l PE_CheckListIndicator
         \i{1,3} (const \l QStyleOptionListView *) \i \l Style_On
         \i Indicates whether or not the controller is selected.
    \row \i \l Style_NoChange \i Indicates a "tri-state" controller.
    \row \i \l Style_Enable \i Indicates the controller is enabled.
    \row \i{1,2} \l PE_TreeBranch \i{1,2} (const \l QStyleOption *)
         \i \l Style_Down \i Indicates that the Tree Branch is pressed
    \row \i \l Style_Open \i Indicates that the tree branch is expanded.
    \row \i \l PE_HeaderArrow \i (const \l QStyleOptionHeader *)
         \i \l Style_Up \i Indicates that the arrow should be drawn up;
         otherwise it should be down.
    \row \i{1,3} \l PE_HeaderSection \i{1,3} (const \l QStyleOptionHeader *)
         \i \l Style_Sunken \i Indicates that the section is pressed.
    \row \i \l Style_Up \i Indicates that the sort indicator should be pointing up.
    \row \i \l Style_Off \i Indicates that the the section is not selected.
    \row \i \l PE_PanelGroupBox, \l PE_Panel, \l PE_PanelLineEdit,
            \l PE_PanelPopup, and \l PE_PanelDockWindow
         \i (const \l QStyleOptionFrame *) \i \l Style_Sunken
         \i Indicates that the Frame should be sunken.
    \row \i \l PE_DockWindowHandle \i (const \l QStyleOptionDockWindow *)
         \i \l Style_Horizontal \i Indicates that the window handle is horizontal
         instead of vertical.
    \row \i \l PE_DockWindowSeparator \i (const \l QStyleOption *)
         \i \l Style_Horizontal \i Indicates that the separator is horizontal
         instead of vertical.
    \row \i \l PE_SpinBoxPlus, \l PE_SpinBoxMinus, \l PE_SpinBoxUp,
            \l PE_SpinBoxDown, and \l PE_SpinBoxSlider
         \i (const \l QStyleOptionSpinBox *)
         \i \l Style_Sunken \i Indicates that the button is pressed.
    \endtable

    \sa PrimitiveElement, StyleFlags, QStyleOption
*/

/*!
    \enum QStyle::ControlElement

    This enum represents a ControlElement. A ControlElement is part of
    a widget that performs some action or displays information to the
    user.

    \value CE_PushButton  the bevel and default indicator of a QPushButton.
    \value CE_PushButtonLabel  the label (iconset with text or pixmap)
        of a QPushButton.

    \value CE_CheckBox  the indicator of a QCheckBox.
    \value CE_CheckBoxLabel  the label (text or pixmap) of a QCheckBox.

    \value CE_RadioButton  the indicator of a QRadioButton.
    \value CE_RadioButtonLabel  the label (text or pixmap) of a QRadioButton.

    \value CE_TabBarTab  the tab within a QTabBar (a QTab).
    \value CE_TabBarLabel  the label within a QTab.

    \value CE_ProgressBarGroove  the groove where the progress
        indicator is drawn in a QProgressBar.
    \value CE_ProgressBarContents  the progress indicator of a QProgressBar.
    \value CE_ProgressBarLabel  the text label of a QProgressBar.

    \value CE_ToolButtonLabel a tool button's label.
    \value CE_ToolBarButton

    \value CE_MenuBarItem  a menu item in a QMenuBar.
    \value CE_MenuBarEmptyArea the empty area of a QMenuBar.

    \value CE_MenuItem  a menu item in a QMenu.
    \value CE_MenuScroller scrolling areas in a QMenu when the
        style supports scrolling.
    \value CE_MenuTearoff a menu item representing the tear off section of
        a QMenu.
    \value CE_MenuEmptyArea the area in a menu without menu items.
    \value CE_MenuVMargin the vertical extra space on the top/bottom of a menu.
    \value CE_MenuHMargin the horizontal extra space on the left/right of a menu.

    \value CE_DockWindowEmptyArea the empty area of a QDockWindow.

    \value CE_ToolBoxTab the toolbox's tab area.
    \value CE_HeaderLabel the header's label.

    \value CE_CustomBase  base value for custom ControlElements.
    Custom values must be greater than this value.

    \sa drawControl()
*/

/*!
    \fn void QStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *widget = 0) const = 0

    Draws the ControlElement \a element with painter \a p with the
    style options specified by \a opt.

    The \a widget argument is optional and may contain a widget that
    may aid in drawing the control.

    The \a opt parameter can be qt_cast()'ed to the correct sub-class
    of the QStyleOption.

    What follows is a table of the elements and the QStyleOption
    structure they can cast to. The flags stored in the QStyleOption
    state variable are also listed. If a ControlElement is not listed
    here, then it uses the default QStyleOption.

    \table
    \header \i ControlElement \i Option Cast \i Style Flags \i Notes
    \row \i{1,4} \l CE_MenuItem and \l CE_MenuBarItem
         \i{1,4} const \l QStyleOptionMenuItem
         \i \l Style_Active \i The menu item is the current item.
    \row \i \l Style_Enabled \i The item is enabled.
    \row \i \l Style_Down
         \i Set if the menu item is down
         (i.e., if the mouse button or the space bar is pressed).
    \row \i \l Style_HasFocus \i Set if the menubar has input focus.
    \row \i{1,6} \l CE_PushButton and \l CE_PushButtonLabel
         \i{1,6} const \l QStyleOptionButton
         \i \l Style_Enabled \i Set if the button is enabled.
    \row \i \l Style_HasFocus \i Set if the button has input focus.
    \row \i \l Style_Raised \i Set if the button is not down, not on and not flat.
    \row \i \l Style_On \i Set if the button is a toggle button and is toggled on.
    \row \i \l Style_Down
         \i Set if the button is down (i.e., the mouse button or the
         space bar is pressed on the button).
    \row \i \l Style_ButtonDefault \i Set if the button is a default button.

    \row \i{1,6} \l CE_RadioButton, \l CE_RadioButtonLabel,
                 \l CE_CheckBox, and \l CE_CheckBoxLabel
         \i{1,6} const \l QStyleOptionButton
         \i \l Style_Enabled \i Set if the button is enabled.
    \row \i \l Style_HasFocus \i Set if the button has input focus.
    \row \i \l Style_On \i Set if the button is checked.
    \row \i \l Style_Off \i Set if the button is not checked.
    \row \i \l Style_NoChange \i Set if the button is in the NoChange state.
    \row \i \l Style_Down
         \i Set if the button is down (i.e., the mouse button or
         the space bar is pressed on the button).
    \row \i{1,2} \l CE_ProgressBarContents, \l CE_ProgressBarLabel,
                 \l CE_ProgressBarGroove
         \i{1,2} const \l QStyleOptionProgressBar
         \i \l Style_Enabled \i Set if the progressbar is enabled.
    \row \i \l Style_HasFocus \i Set if the progressbar has input focus.
    \row \i \l CE_HeaderLabel \i const \l QStyleOptionHeader \i \i
    \row \i{1,7} \l CE_ToolButtonLabel
         \i{1,7} const \l QStyleOptionToolButton
         \i \l Style_Enabled \i Set if the toolbutton is enabled.
    \row \i \l Style_HasFocus \i Set if the toolbutton has input focus.
    \row \i \l Style_Down
         \i Set if the toolbutton is down (i.e., a mouse button or
         the space bar is pressed).
    \row \i \l Style_On \i Set if the toolbutton is a toggle button and is toggled on.
    \row \i \l Style_AutoRaise \i Set if the toolbutton has auto-raise enabled.
    \row \i \l Style_MouseOver \i Set if the mouse pointer is over the toolbutton.
    \row \i \l Style_Raised \i Set if the button is not down and is not on.
    \row \i \l CE_ToolBoxTab \i const \l QStyleOptionToolBox
         \i \l Style_Selected \i The tab is the currently selected tab.
    \endtable

    \sa ControlElement, StyleFlags, QStyleOption
*/

/*!
    \fn void QStyle::drawControlMask(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *widget = 0) const = 0

    Draw a bitmask for the ControlElement \a element using the painter \a p
    with the style options specified by \a opt. See drawControl() for
    an explanation of the use of \a opt and \a widget.

*/

/*!
    \enum QStyle::SubRect

    This enum represents a sub-area of a widget. Style implementations
    use these areas to draw the different parts of a widget.

    \value SR_PushButtonContents  area containing the label (iconset
        with text or pixmap).
    \value SR_PushButtonFocusRect  area for the focus rect (usually
        larger than the contents rect).

    \value SR_CheckBoxIndicator  area for the state indicator (e.g. check mark).
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


    \value SR_ToolButtonContents area for the tool button's label.

    \value SR_DialogButtonAccept area for a dialog's accept button.
    \value SR_DialogButtonReject area for a dialog's reject button.
    \value SR_DialogButtonApply  area for a dialog's apply button.
    \value SR_DialogButtonHelp area for a dialog's help button.
    \value SR_DialogButtonAll area for a dialog's all button.
    \value SR_DialogButtonRetry area for a dialog's retry button.
    \value SR_DialogButtonAbort area for a dialog's abort button.
    \value SR_DialogButtonIgnore area for a dialog's ignore button.
    \value SR_DialogButtonCustom area for a dialog's custom widget
    area (in the button row).

    \value SR_ToolBoxTabContents area for a toolbox tab's icon and label

    \value SR_CustomBase  base value for custom ControlElements.
    Custom values must be greater than this value.

    \value SR_ToolBarButtonContents
    \value SR_ToolBarButtonMenu

    \sa subRect()
*/

/*!
    \fn QRect QStyle::subRect(SubRect subrect, const QStyleOption *opt, const QWidget *widget = 0) const = 0

    Returns the sub-area \a subrect as described in \a opt in logical
    coordinates.

    The \a widget argument is optional and may contain a widget that
    may aid determining the subRect.

    The QStyleOption can be qt_cast()'d to the appropriate type based
    on the value of \a subrect. See the table below for the
    appropriate \a widget casts:

    \table
    \header \i SubRect \i Widget Cast
    \row \i \l SR_PushButtonContents   \i (const \l QStyleOptionButton *)
    \row \i \l SR_PushButtonFocusRect  \i (const \l QStyleOptionButton *)
    \row \i \l SR_CheckBoxIndicator    \i (const \l QStyleOptionButton *)
    \row \i \l SR_CheckBoxContents     \i (const \l QStyleOptionButton *)
    \row \i \l SR_CheckBoxFocusRect    \i (const \l QStyleOptionButton *)
    \row \i \l SR_RadioButtonIndicator \i (const \l QStyleOptionButton *)
    \row \i \l SR_RadioButtonContents  \i (const \l QStyleOptionButton *)
    \row \i \l SR_RadioButtonFocusRect \i (const \l QStyleOptionButton *)
    \row \i \l SR_ComboBoxFocusRect    \i (const \l QStyleOptionComboBox *)
    \row \i \l SR_DockWindowHandleRect \i (const \l QStyleOptionDockWindow *)
    \row \i \l SR_ProgressBarGroove    \i (const \l QStyleOptionProgressBar *)
    \row \i \l SR_ProgressBarContents  \i (const \l QStyleOptionProgressBar *)
    \row \i \l SR_ProgressBarLabel     \i (const \l QStyleOptionProgressBar *)
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


    \value CC_CustomBase  base value for custom ControlElements.
    Custom values must be greater than this value.

    \sa SubControl drawComplexControl()
*/

/*!
    \enum QStyle::SubControl

    This enum represents a SubControl within a ComplexControl.

    \value SC_None   special value that matches no other SubControl.


    \value SC_ScrollBarAddLine  scrollbar add line (i.e. down/right
        arrow); see also QScrollbar.
    \value SC_ScrollBarSubLine  scrollbar sub line (i.e. up/left arrow).
    \value SC_ScrollBarAddPage  scrollbar add page (i.e. page down).
    \value SC_ScrollBarSubPage  scrollbar sub page (i.e. page up).
    \value SC_ScrollBarFirst        scrollbar first line (i.e. home).
    \value SC_ScrollBarLast        scrollbar last line (i.e. end).
    \value SC_ScrollBarSlider        scrollbar slider handle.
    \value SC_ScrollBarGroove        special sub-control which contains the
        area in which the slider handle may move.


    \value SC_SpinBoxUp  spinwidget up/increase; see also QSpinBox.
    \value SC_SpinBoxDown  spinwidget down/decrease.
    \value SC_SpinBoxFrame  spinwidget frame.
    \value SC_SpinBoxEditField  spinwidget edit field.
    \value SC_SpinBoxButtonField  spinwidget button field.
    \value SC_SpinBoxSlider  spinwidget optional slider.


    \value SC_ComboBoxEditField  combobox edit field; see also QComboBox.
    \value SC_ComboBoxArrow  combobox arrow
    \value SC_ComboBoxFrame combobox frame
    \value SC_ComboBoxListBoxPopup combobox list box

    \value SC_SliderGroove  special sub-control which contains the area
        in which the slider handle may move.
    \value SC_SliderHandle  slider handle.
    \value SC_SliderTickmarks  slider tickmarks.


    \value SC_ToolButton  tool button; see also QToolbutton.
    \value SC_ToolButtonMenu sub-control for opening a popup menu in a
        tool button; see also Q3PopupMenu.


    \value SC_TitleBarSysMenu system menu button (i.e. restore, close, etc.).
    \value SC_TitleBarMinButton  minimize button.
    \value SC_TitleBarMaxButton  maximize button.
    \value SC_TitleBarCloseButton  close button.
    \value SC_TitleBarLabel  window title label.
    \value SC_TitleBarNormalButton  normal (restore) button.
    \value SC_TitleBarShadeButton  shade button.
    \value SC_TitleBarUnshadeButton  unshade button.


    \value SC_ListView  the list view area.
    \omitvalue SC_ListViewBranch
    \value SC_ListViewExpand  expand item (i.e. show/hide child items).


    \value SC_All  special value that matches all SubControls.


    \sa ComplexControl
*/

/*!
    \fn void QStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget = 0) const = 0

    Draws the ComplexControl \a cc using the painter \a p with the
    style options specified by \a opt.

    The \a widget argument is optional and may contain a widget to aid
    in drawing \a cc.

    The \a opt parameter is a pointer to a QStyleOptionComplex
    structure that can be qt_cast'ed to the correct structure. Note
    that the rect member of \a opt should be in logical coordinates.
    Reimplementations of this function should use visualRect() to
    change the logical coordinates into screen coordinates when using
    drawPrimitive() and drawControl().

    Here is a table listing the elements and what they can be cast to,
    along with an explaination of the flags.

    \table
    \header \i Complex Control \i Option Cast \i Style Flags \i Notes
    \row \i{1,2} \l{CC_SpinBox} \i{1,2} (const \l QStyleOptionSpinBox *)
         \i \l Style_Enabled \i Set if the spinwidget is enabled.
    \row \i \l Style_HasFocus \i Set if the spinwidget has input focus.

    \row \i{1,2} \l {CC_ComboBox} \i{1,2} (const \l QStyleOptionComboBox *)
         \i \l Style_Enabled \i Set if the combobox is enabled.
    \row \i \l Style_HasFocus \i Set if the combobox has input focus.

    \row \i{1,2} \l {CC_ScrollBar} \i{1,2} (const \l QStyleOptionSlider *)
         \i \l Style_Enabled \i Set if the scrollbar is enabled.
    \row \i \l Style_HasFocus \i Set if the scrollbar has input focus.

    \row \i \l {CC_Slider} \i(const \l QStyleOptionSlider *)
         \i \l Style_Enabled \i Set if the slider is enabled.

    \row \i \i \i \l Style_HasFocus \i Set if the slider has input focus.

    \row \i{1,6} \l {CC_ToolButton} \i{1,6} (const \l QStyleOptionToolButton *)
         \i \l Style_Enabled \i Set if the toolbutton is enabled.
    \row \i \l Style_HasFocus \i Set if the toolbutton has input focus.
    \row \i \l Style_Down \i Set if the toolbutton is down (i.e. a mouse
        button or the space bar is pressed).
    \row \i \l Style_On \i Set if the toolbutton is a toggle button
        and is toggled on.
    \row \i \l Style_AutoRaise \i Set if the toolbutton has auto-raise enabled.
    \row \i \l Style_Raised \i Set if the button is not down, not on, and doesn't
        contain the mouse when auto-raise is enabled.

    \row \i \l{CC_TitleBar} \i (const \l QStyleOptionTitleBar *)
         \i \l Style_Enabled \i Set if the title bar is enabled.

    \row \i \l{CC_ListView} \i (const \l QStyleOptionListView *)
         \i \l Style_Enabled \i Set if the listview is enabled.
    \endtable

    \sa ComplexControl SubControl QStyleOptionComplex
*/

/*!
    \fn void QStyle::drawComplexControlMask(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget = 0) const = 0

    Draw a bitmask for the ComplexControl \a cc using the painter \a p
    with the style options specified by \a opt. See
    drawComplexControl() for an explanation of the use of the \a
    widget and \a opt arguments.

    The rect int the QStyleOptionComplex \a opt should be in logical
    coordinates. Reimplementations of this function should use visualRect() to
    change the logical corrdinates into screen coordinates when using
    drawPrimitive() and drawControl().

    \sa drawComplexControl() ComplexControl QStyleOptionComplex
*/

/*!
    \fn QRect QStyle::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget = 0) const = 0

    Returns the rect for the SubControl \a sc in the ComplexControl \a
    cc, with the style options specified by \a opt in logical
    coordinates.

    The \a opt argument is a pointer to a QStyleOptionComplex or one of its
    subclasses. The structure can be cast to the appropriate type based on the
    value of \a cc. The \a widget is optional and can contain additional
    information for the function. See drawComplexControl() for an explanation
    of the \a widget and \a opt arguments.

    \sa drawComplexControl() ComplexControl SubControl QStyleOptionComplex
*/

/*!
    \fn SubControl QStyle::querySubControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pos, const QWidget *widget = 0) const = 0

    Returns the SubControl in the ComplexControl \a cc with the style
    options specified by \a opt at the point \a pos. The \a opt
    argument is a pointer to a QStyleOptionComplex structure or one of
    its subclasses. The structure can be cast to the appropriate type
    based on the value of \a cc. The \a widget argument is optional
    and can contain additional information for the functions. See
    drawComplexControl() for an explanation of the \a widget and \a
    opt arguments.

    Note that \a pos is passed in screen coordinates. When using
    querySubControlMetrics() to check for hits and misses, use
    visualRect() to change the logical coordinates into screen
    coordinates.

    \sa drawComplexControl() ComplexControl SubControl querySubControlMetrics() QStyleOptionComplex
*/

/*!
    \enum QStyle::PixelMetric

    This enum represents a PixelMetric. A PixelMetric is a style
    dependent size represented as a single pixel value.

    \value PM_ButtonMargin  amount of whitespace between push button
        labels and the frame.
    \value PM_ButtonDefaultIndicator  width of the default-button indicator frame.
    \value PM_MenuButtonIndicator  width of the menu button indicator
        proportional to the widget height.
    \value PM_ButtonShiftHorizontal  horizontal contents shift of a
        button when the button is down.
    \value PM_ButtonShiftVertical  vertical contents shift of a button when the
        button is down.

    \value PM_DefaultFrameWidth  default frame width, usually 2.
    \value PM_SpinBoxFrameWidth  frame width of a spin box.
    \value PM_MDIFrameWidth frame width of an MDI window.
    \value PM_MDIMinimizedWidth width of a minimized MDI window.

    \value PM_MaximumDragDistance  Some feels require the scrollbar or
        other sliders to jump back to the original position when the
        mouse pointer is too far away while dragging. A value of -1
        disables this behavior.

    \value PM_ScrollBarExtent  width of a vertical scrollbar and the
        height of a horizontal scrollbar.
    \value PM_ScrollBarSliderMin the minimum height of a vertical
        scrollbar's slider and the minimum width of a horizontal
        scrollbar's slider.

    \value PM_SliderThickness  total slider thickness.
    \value PM_SliderControlThickness  thickness of the slider handle.
    \value PM_SliderLength length of the slider.
    \value PM_SliderTickmarkOffset the offset between the tickmarks
        and the slider.
    \value PM_SliderSpaceAvailable  the available space for the slider to move.

    \value PM_DockWindowSeparatorExtent  width of a separator in a
        horizontal dock window and the height of a separator in a
        vertical dock window.
    \value PM_DockWindowHandleExtent  width of the handle in a
        horizontal dock window and the height of the handle in a
        vertical dock window.
    \value PM_DockWindowFrameWidth  frame width of a dock window.

    \value PM_MenuBarFrameWidth  frame width of a menubar.
    \value PM_MenuBarItemSpacing  spacing between menubar items.
    \value PM_MenuBarHMargin  spacing between menubar items and top/bottom of bar.
    \value PM_MenuBarVMargin  spacing between menubar items and left/right of bar.

    \value PM_ToolBarItemSpacing  spacing between toolbar items.

    \value PM_TabBarTabOverlap number of pixels the tabs should overlap.
    \value PM_TabBarTabHSpace extra space added to the tab width.
    \value PM_TabBarTabVSpace extra space added to the tab height.
    \value PM_TabBarBaseHeight height of the area between the tab bar
        and the tab pages.
    \value PM_TabBarBaseOverlap number of pixels the tab bar overlaps
        the tab bar base.
    \value PM_TabBarScrollButtonWidth
    \value PM_TabBarTabShiftHorizontal horizontal pixel shift when a
        tab is selected.
    \value PM_TabBarTabShiftVertical vertical pixel shift when a
        tab is selected.

    \value PM_ProgressBarChunkWidth  width of a chunk in a progress bar indicator.

    \value PM_SplitterWidth  width of a splitter.

    \value PM_TitleBarHeight height of the title bar.

    \value PM_IndicatorWidth  width of a check box indicator.
    \value PM_IndicatorHeight  height of a checkbox indicator.
    \value PM_ExclusiveIndicatorWidth  width of a radio button indicator.
    \value PM_ExclusiveIndicatorHeight  height of a radio button indicator.

    \value PM_MenuFrameWidth border width (applied on all sides) for a QMenu.
    \value PM_MenuHMargin additional border (used on left and right) for a QMenu.
    \value PM_MenuVMargin additional border (used for bottom and top) for a QMenu.
    \value PM_MenuScrollerHeight height of the scroller area in a QMenu.
    \value PM_MenuScrollerHeight height of the scroller area in a QMenu.
    \value PM_MenuTearoffHeight height of a tear off area in a QMenu.
    \value PM_MenuDesktopFrameWidth

    \value PM_CheckListButtonSize area (width/height) of the
        checkbox/radiobutton in a QCheckListItem
    \value PM_CheckListControllerSize area (width/height) of the
        controller in a QCheckListItem

    \value PM_DialogButtonsSeparator distance between buttons in a dialog buttons widget.
    \value PM_DialogButtonsButtonWidth minimum width of a button in a dialog buttons widget.
    \value PM_DialogButtonsButtonHeight minimum height of a button in a dialog buttons widget.

    \value PM_HeaderMarkSize
    \value PM_HeaderGripMargin
    \value PM_HeaderMargin
    \value PM_SpinBoxSliderHeight The height of the optional spin box slider.

    \value PM_CustomBase  base value for custom ControlElements.
    Custom values must be greater than this value.

    \value PM_DefaultToplevelMargin
    \value PM_DefaultChildMargin
    \value PM_DefaultLayoutSpacing

    \sa pixelMetric()
*/

/*!
    \fn int QStyle::pixelMetric(PixelMetric metric, const QStyleOption *opt = 0, const QWidget *widget = 0) const;

    Returns the pixel metric for the given \a metric. The \a opt and
    \a widget can be used for calculating the metric.
    The \a opt can be qt_cast to the appropriate type based on the value of
    \a metric. Note that \a opt may be zero even for PixelMetrics
    that can make use of \a opt. See the table below for the
    appropriate \a opt casts:

    \table
    \header \i PixelMetric \i Option Cast
    \row \i \l PM_SliderControlThickness \i (const \l QStyleOptionSlider *)
    \row \i \l PM_SliderLength           \i (const \l QStyleOptionSlider *)
    \row \i \l PM_SliderTickmarkOffset   \i (const \l QStyleOptionSlider *)
    \row \i \l PM_SliderSpaceAvailable   \i (const \l QStyleOptionSlider *)
    \row \i \l PM_ScrollBarExtent        \i (const \l QStyleOptionSlider *)
    \row \i \l PM_TabBarTabOverlap       \i (const \l QStyleOptionTab *)
    \row \i \l PM_TabBarTabHSpace        \i (const \l QStyleOptionTab *)
    \row \i \l PM_TabBarTabVSpace        \i (const \l QStyleOptionTab *)
    \row \i \l PM_TabBarBaseHeight       \i (const \l QStyleOptionTab *)
    \row \i \l PM_TabBarBaseOverlap      \i (const \l QStyleOptionTab *)
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

    \value CT_CustomBase  base value for custom ControlElements.
    Custom values must be greater than this value.

    \sa sizeFromContents()
*/

/*!
    \fn QSize QStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QFontMetrics &fm, const QWidget *w = 0) const = 0

    Returns the size of styled object described in \a opt based on the
    contents size \a contentsSize. The font metrics in \a fm can aid
    in determining the size.

    The \a opt argument is a pointer to a QStyleOption or one of its
    subclasses. The \a opt can be cast to the appropriate type based
    on the value of \a ct. The widget \a w is optional argument and can
    contain extra information used for calculating the size.
    See the table below for the appropriate \a opt usage:

    \table
    \header \i ContentsType   \i Option Cast
    \row \i \l CT_PushButton  \i (const \l QStyleOptionButton *)
    \row \i \l CT_CheckBox    \i (const \l QStyleOptionButton *)
    \row \i \l CT_RadioButton \i (const \l QStyleOptionButton *)
    \row \i \l CT_ToolButton  \i (const \l QStyleOptionToolButton *)
    \row \i \l CT_ComboBox    \i (const \l QStyleOptionComboBox *)
    \row \i \l CT_Splitter    \i (const \l QStyleOption *)
    \row \i \l CT_DockWindow  \i (const \l QStyleOptionDockWindow *)
    \row \i \l CT_ProgressBar \i (const \l QStyleOptionProgressBar *)
    \row \i \l CT_MenuItem    \i (const \l QStyleOptionMenuItem *)
    \endtable

    \sa ContentsType QStyleOption
*/

/*!
    \enum QStyle::StyleHint

    This enum represents a StyleHint. A StyleHint is a general look
    and/or feel hint.

    \value SH_EtchDisabledText disabled text is "etched" as it is on Windows.

    \value SH_GUIStyle the GUI style to use.

    \value SH_ScrollBar_BackgroundRole  the background role for a
        QScrollBar.

    \value SH_ScrollBar_MiddleClickAbsolutePosition  a boolean value.
        If true, middle clicking on a scrollbar causes the slider to
        jump to that position. If false, middle clicking is
        ignored.

    \value SH_ScrollBar_LeftClickAbsolutePosition  a boolean value.
        If true, left clicking on a scrollbar causes the slider to
        jump to that position. If false, left clicking will
        behave as appropriate for each control.

    \value SH_ScrollBar_ScrollWhenPointerLeavesControl  a boolean
        value. If true, when clicking a scrollbar SubControl, holding
        the mouse button down and moving the pointer outside the
        SubControl, the scrollbar continues to scroll. If false, the
        scollbar stops scrolling when the pointer leaves the
        SubControl.

    \value SH_TabBar_Alignment  the alignment for tabs in a
        QTabWidget. Possible values are \c Qt::AlignLeft, \c
        Qt::AlignCenter and \c Qt::AlignRight.

    \value SH_Header_ArrowAlignment the placement of the sorting
        indicator may appear in list or table headers. Possible values
        are \c Qt::Left or \c Qt::Right.

    \value SH_Slider_SnapToValue  sliders snap to values while moving,
        as they do on Windows.

    \value SH_Slider_SloppyKeyEvents  key presses handled in a sloppy
        manner, i.e. left on a vertical slider subtracts a line.

    \value SH_ProgressDialog_CenterCancelButton  center button on
        progress dialogs, like Motif, otherwise right aligned.

    \value SH_ProgressDialog_TextLabelAlignment Qt::Alignment --
        text label alignment in progress dialogs; Center on windows,
        Auto|VCenter otherwise.

    \value SH_PrintDialog_RightAlignButtons  right align buttons in
        the print dialog, as done on Windows.

    \value SH_MainWindow_SpaceBelowMenuBar 1 or 2 pixel space between
        the menubar and the dockarea, as done on Windows.

    \value SH_FontDialog_SelectAssociatedText select the text in the
        line edit, or when selecting an item from the listbox, or when
        the line edit receives focus, as done on Windows.

    \value SH_Menu_AllowActiveAndDisabled  allows disabled menu
        items to be active.

    \value SH_Menu_SpaceActivatesItem  pressing the space bar activates
        the item, as done on Motif.

    \value SH_Menu_SubMenuPopupDelay  the number of milliseconds
        to wait before opening a submenu; 256 on windows, 96 on Motif.

    \value SH_Menu_Scrollable whether popupmenu's must support
        scrolling.

    \value SH_Menu_SloppySubMenus whether popupmenu's must support
        sloppy submenu; as implemented on Mac OS.

    \value SH_ScrollView_FrameOnlyAroundContents  whether scrollviews
        draw their frame only around contents (like Motif), or around
        contents, scrollbars and corner widgets (like Windows).

    \value SH_MenuBar_AltKeyNavigation  menubars items are navigable
        by pressing Alt, followed by using the arrow keys to select
        the desired item.

    \value SH_ComboBox_ListMouseTracking  mouse tracking in combobox
        dropdown lists.

    \value SH_Menu_MouseTracking  mouse tracking in popup menus.

    \value SH_MenuBar_MouseTracking  mouse tracking in menubars.

    \value SH_Menu_FillScreenWithScroll whether scrolling popups
    should fill the screen as they are scrolled.

    \value SH_ItemView_ChangeHighlightOnFocus  gray out selected items
        when losing focus.

    \value SH_Widget_ShareActivation turn on sharing activation with
        floating modeless dialogs.

    \value SH_TabBar_SelectMouseType which type of mouse event should
        cause a tab to be selected.

    \value SH_ListViewExpand_SelectMouseType which type of mouse event should
        cause a listview expansion to be selected.

    \value SH_TabBar_PreferNoArrows whether a tabbar should suggest a size
        to prevent scoll arrows.

    \value SH_ComboBox_Popup allows popups as a combobox dropdown
        menu.

    \value SH_Workspace_FillSpaceOnMaximize the workspace should
        maximize the client area.

    \value SH_TitleBar_NoBorder the title bar has no border.

    \value SH_ScrollBar_StopMouseOverSlider stops auto-repeat when
        the slider reaches the mouse position.

    \omitvalue SH_ScrollBar_BackgroundMode

    \value SH_BlinkCursorWhenTextSelected whether cursor should blink
        when text is selected.

    \value SH_RichText_FullWidthSelection whether richtext selections
        should extend to the full width of the document.

    \value SH_GroupBox_TextLabelVerticalAlignment how to vertically align a
        groupbox's text label.

    \value SH_GroupBox_TextLabelColor how to paint a groupbox's text label.

    \value SH_DialogButtons_DefaultButton which button gets the
        default status in a dialog's button widget.

    \value SH_ToolButton_Uses3D indicates whether QToolButtons should
    use a 3D frame when the mouse is over them

    \value SH_ToolBox_SelectedPageTitleBold Boldness of the selected
    page title in a QToolBox.

    \value SH_LineEdit_PasswordCharacter The QChar Unicode character
    to be used for passwords.

    \value SH_Table_GridLineColor

    \value SH_UnderlineShortcut whether shortcuts are underlined.

    \value SH_SpinBox_AnimateButton animate a click when up or down is
    pressed in a spin box.
    \value SH_SpinBox_KeyPressAutoRepeatRate auto-repeat interval for
    spinbox key presses.
    \value SH_SpinBox_ClickAutoRepeatRate auto-repeat interval for
    spinbox mouse clicks.
    \value SH_TipLabel_Opacity An integer indicating the opacity for
    the tip label, 0 is completely transparent, 255 is completely
    opaque.
    \value SH_DrawMenuBarSeparator indicates whether or not the menubar draws separators.
    \value SH_TitlebarModifyNotification indicates if the titlebar should show
    a '*' for windows that are modified.


    \value SH_CustomBase  base value for custom ControlElements.
    Custom values must be greater than this value.

    \omitvalue SH_UnderlineAccelerator

    \sa styleHint()
*/

/*!
    \fn int QStyle::styleHint(StyleHint stylehint, const QStyleOption *opt = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const;

    Returns the style hint \a stylehint for \a widget descriped in the QStyleOption \a opt.
    Currently \a returnData and \a widget are not used, it is allowed for future enhancement.
    The \a opt parameters is used only in SH_ComboBox_Popup and SH_GroupBox_TextLabelColor
    For an explanation of the return value see \l StyleHint.
*/

/*!
    \enum QStyle::StylePixmap

    This enum represents a StylePixmap. A StylePixmap is a pixmap that
    can follow some existing GUI style or guideline.

    \value SP_TitleBarMinButton  minimize button on title bars. For
        example, in a QWorkspace.
    \value SP_TitleBarMaxButton  maximize button on title bars.
    \value SP_TitleBarCloseButton  close button on title bars.
    \value SP_TitleBarNormalButton  normal (restore) button on title bars.
    \value SP_TitleBarShadeButton  shade button on title bars.
    \value SP_TitleBarUnshadeButton  unshade button on title bars.
    \value SP_MessageBoxInformation the 'information' icon.
    \value SP_MessageBoxWarning the 'warning' icon.
    \value SP_MessageBoxCritical the 'critical' icon.
    \value SP_MessageBoxQuestion the 'question' icon.
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

    \value SP_DockWindowCloseButton  close button on dock windows;
        see also QDockWindow.


    \value SP_CustomBase  base value for custom ControlElements.
    Custom values must be greater than this value.

    \sa stylePixmap()
*/


/*!
  \enum QStyle::PixmapType

  This enum represents the effects performed on a pixmap to achieve a
  GUI style's perferred way of representing the image in different
  states.

  \value PT_Disabled  a disabled pixmap (drawn on disabled widgets).
  \value PT_Active   an active pixmap (drawn on active
  toolbuttons/menuitems).

  \value PT_CustomBase base value for custom PixmapTypes. Custom
  values must be greater than this value.

  \sa stylePixmap()
*/

/*!
    \fn QPixmap QStyle::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap, const QStyleOption *opt) const

    \overload

    Returns a pixmap styled to conform to \a pixmaptype description
    out of \a pixmap, and taking into account the palette specified by
    \a opt.

    The \a opt can pass extra information, but it must contain a palette.

    Not all types of pixmaps will change from their input in which
    case the result will simply be the pixmap passed in.
*/

/*!
    \fn QPixmap QStyle::stylePixmap(StylePixmap stylepixmap, const QStyleOption *opt, const QWidget *widget) const

    Returns a pixmap for \a stylepixmap.

    The \a opt argument can be used to pass extra information required
    when drawing the ControlElement. Currently, the \a opt argument is unused.

    The \a widget argument is optional and may contain a widget that
    may aid in drawing the control.

    \sa StylePixmap
*/

/*!
    \fn QRect QStyle::visualRect(const QRect &logical, const QWidget *w);

    Returns the rect \a logical in screen coordinates. The bounding
    rect for widget \a w is used to perform the translation. This
    function is provided to aid style implementors in supporting
    right-to-left mode.

    \sa QApplication::reverseLayout()
*/
QRect QStyle::visualRect(const QRect &logical, const QWidget *w)
{
    if (!QApplication::reverseLayout())
        return logical;
    QRect boundingRect = w->rect();
    QRect r = logical;
    r.moveBy(2*(boundingRect.right() - logical.right()) +
              logical.width() - boundingRect.width(), 0);
    return r;
}

/*!
    \fn QRect QStyle::visualRect(const QRect &logical, const QRect &bounding);

    \overload

    Returns the rect \a logical in screen coordinates. The rect \a
    bounding is used to perform the translation. This function is
    provided to aid style implementors in supporting right-to-left
    mode.

    \sa QApplication::reverseLayout()
*/
QRect QStyle::visualRect(const QRect &logical, const QRect &boundingRect)
{
    if (!QApplication::reverseLayout())
        return logical;
    QRect r = logical;
    r.moveBy(2*(boundingRect.right() - logical.right()) +
              logical.width() - boundingRect.width(), 0);
    return r;
}

/*!
    \fn QPoint QStyle::visualPos(const QPoint &logical, const QWidget *w)

    Returns the \a logical point in screen coordinates. The bounding
    rect for widget \a w is used to perform the translation. This
    function is provided to aid style implementors in supporting
    right-to-left mode.

    \sa QApplication::reverseLayout()
*/
QPoint QStyle::visualPos(const QPoint &logical, const QWidget *w)
{
    if (!QApplication::reverseLayout())
        return logical;
    return QPoint(w->rect().right() - logical.x(), logical.y());
}

/*!
    \fn QPoint QStyle::visualPos(const QPoint &logical, const QRect &bounding)

    \overload

    Returns the \a logical point in screen coordinates. The \a
    bounding rectangle is used to perform the translation. This
    function is provided to aid style implementors in supporting
    right-to-left mode.

    \sa QApplication::reverseLayout()
*/
QPoint QStyle::visualPos(const QPoint &logical, const QRect &boundingRect)
{
    if (!QApplication::reverseLayout())
        return logical;
    return QPoint(boundingRect.right() - logical.x(), logical.y());
}



/*!
    Converts \a logical_val to a pixel position. \a min maps to 0, \a
    max maps to \a span and other values are distributed evenly
    in-between.

    This function can handle the entire integer range without
    overflow, providing \a span is \<= 4096.

    By default, this function assumes that the maximum value is on the
    right for horizontal items and on the bottom for vertical items.
    Set \a upsideDown to true to reverse this behavior.

    \sa valueFromPosition()
*/

int QStyle::positionFromValue(int min, int max, int logical_val, int span, bool upsideDown)
{
    if (span <= 0 || logical_val < min || max <= min)
        return 0;
    if (logical_val > max)
        return upsideDown ? span : min;

    uint range = max - min;
    uint p = upsideDown ? max - logical_val : logical_val - min;

    if (range > (uint)INT_MAX/4096) {
        const int scale = 4096*2;
        return ((p/scale) * span) / (range/scale);
        // ### the above line is probably not 100% correct
        // ### but fixing it isn't worth the extreme pain...
    } else if (range > (uint)span) {
        return (2*p*span + range) / (2*range);
    } else {
        uint div = span / range;
        uint mod = span % range;
        return p*div + (2*p*mod + range) / (2*range);
    }
    //equiv. to (p*span)/range + 0.5
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

#endif // QT_NO_STYLE
