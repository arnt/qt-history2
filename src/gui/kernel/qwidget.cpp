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

#include "qapplication.h"
#include "qapplication_p.h"
#include "qbrush.h"
#include "qcleanuphandler.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qhash.h"
#include "qlayout.h"
#include "qmenu.h"
#include "qmetaobject.h"
#include "qpixmap.h"
#include "qpointer.h"
#include "qstack.h"
#include "qstyle.h"
#include "qstylefactory.h"
#include "qvariant.h"
#include "qwidget.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif
#ifdef Q_WS_MAC
# include "qt_mac_p.h"
#endif
#if defined(Q_WS_QWS)
#include "qwsmanager_qws.h"
#endif
#include "qpainter.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qdebug.h"

#include "qinputcontext.h"

#ifdef Q_WS_WIN
#include <private/qwininputcontext_p.h>
#endif

#if defined(Q_WS_X11)
#include <private/qpaintengine_x11_p.h>
#include "qx11info_x11.h"
#endif

#include "qwidget_p.h"
#include "qaction_p.h"

QWidgetPrivate::QWidgetPrivate(int version) :
        QObjectPrivate(version), extra(0), focus_child(0)
#ifndef QT_NO_LAYOUT
        ,layout(0)
#endif
        ,leftmargin(0), topmargin(0), rightmargin(0), bottommargin(0)
        ,fg_role(QPalette::Foreground)
        ,bg_role(QPalette::Background)
        ,hd(0)
#if defined(Q_WS_X11)
        ,picture(0)
#elif defined(Q_WS_MAC)
        ,cg_hd(0)
#endif
        ,polished(0)

{
    if (version != QObjectPrivateVersion)
        qFatal("Cannot mix incompatible Qt libraries");

    isWidget = true;
    memset(high_attributes, 0, sizeof(high_attributes));
}


QWidgetPrivate::~QWidgetPrivate()
{
    if (extra)
        deleteExtra();
}

/*!
    \internal
    This is an internal function, you should never call this.

    This function is called to remove focus from associated input
    context.

    \sa QInputContext::unsetFocus()
 */
void QWidgetPrivate::unfocusInputContext()
{
#ifndef QT_NO_IM
    Q_Q(QWidget);
    QInputContext *qic = q->inputContext();
    if ( qic ) {
	qic->setFocusWidget( 0 );
    }
#endif // QT_NO_IM
}

/*!
    \internal
    This is an internal function, you should never call this.

    This function is called to focus associated input context. The
    code intends to eliminate duplicate focus for the context even if
    the context is shared between widgets

    \sa QInputContext::setFocus()
 */
void QWidgetPrivate::focusInputContext()
{
#ifndef QT_NO_IM
    Q_Q(QWidget);
    QInputContext *qic = q->inputContext();
    if (qic) {
	if(qic->focusWidget() != q)
	    qic->setFocusWidget(q);
    }
#endif // QT_NO_IM
}

/*!
    This function returns the QInputContext for this widget. By
    default the input context is inherited from the widgets
    parent. For toplevels it is inherited from QApplication.

    You can override this and set a special input context for this
    widget by using the setInputContext() method.

    \sa setInputContext()
*/
QInputContext *QWidget::inputContext()
{
    Q_D(QWidget);
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        return 0;

    if (d->ic)
        return d->ic;
    return qApp->inputContext();
}

/*!
  This function sets the input context \a context
  on this widget.

  \sa inputContext()
*/
void QWidget::setInputContext(QInputContext *context)
{
    Q_D(QWidget);
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        return;
    if (d->ic)
	delete d->ic;
    d->ic = context;
}


/*!
    This function is called when text widgets need to be neutral state to
    execute text operations properly. See qlineedit.cpp and qtextedit.cpp as
    example.

    Ordinary reset that along with changing focus to another widget,
    moving the cursor, etc, is implicitly handled via
    unfocusInputContext() because whether reset or not when such
    situation is a responsibility of input methods. So we delegate the
    responsibility to the input context via unfocusInputContext(). See
    'Preedit preservation' section of the class description of
    QInputContext for further information.

    \sa QInputContext, unfocusInputContext(), QInputContext::unsetFocus()
*/
void QWidget::resetInputContext()
{
    if (!hasFocus())
        return;
#ifndef QT_NO_IM
    Q_Q(QWidget);
    QInputContext *qic = q->inputContext();
    if( qic )
	qic->reset();
#endif // QT_NO_IM
}


/*!
    \class QWidget qwidget.h
    \brief The QWidget class is the base class of all user interface objects.

    \ingroup abstractwidgets
    \mainclass

    The widget is the atom of the user interface: it receives mouse,
    keyboard and other events from the window system, and paints a
    representation of itself on the screen. Every widget is
    rectangular, and they are sorted in a Z-order. A widget is
    clipped by its parent and by the widgets in front of it.

    A widget that isn't embedded in a parent widget is called a
    window. Usually, windows have a frame and a title bar,  although
    it is also possible to create windows without such decoration using
    suitable \l{Qt::WindowFlags}{window flags}). In Qt,
    QMainWindow and the various subclasses of QDialog are the most
    common window types.

    A widget without a parent widget is always an independent window.

    Non-window widgets are child widgets. These are child windows
    in their parent widgets. You cannot usually distinguish a child
    widget from its parent visually. Most other widgets in Qt are
    useful only as child widgets. (It is possible to make, say, a
    button into a window, but most people prefer to put
    their buttons inside other widgets, e.g. QDialog.)

    If you want to use a QWidget to hold child widgets you will
    probably want to add a layout to the parent QWidget. (See \link
    layout.html Layouts\endlink.)

    QWidget has many member functions, but some of them have little
    direct functionality: for example, QWidget has a font property,
    but never uses this itself. There are many subclasses which
    provide real functionality, such as QPushButton, QListWidget and
    QTabWidget, etc.

    \section1 Groups of functions:

    \table
    \header \i Context \i Functions

    \row \i Window functions \i
        show(),
        hide(),
        raise(),
        lower(),
        close().

    \row \i Top-level windows \i
        isWindowModified(),
        setWindowModified(),
        windowTitle(),
        setWindowTitle(),
        windowIcon(),
        setWindowIcon(),
        windowIconText(),
        setWindowIconText(),
        isActiveWindow(),
        activateWindow(),
        showMinimized().
        showMaximized(),
        showFullScreen(),
        showNormal().

    \row \i Window contents \i
        update(),
        repaint(),
        scroll().

    \row \i Geometry \i
        pos(),
        size(),
        rect(),
        x(),
        y(),
        width(),
        height(),
        sizePolicy(),
        setSizePolicy(),
        sizeHint(),
        updateGeometry(),
        layout(),
        move(),
        resize(),
        setGeometry(),
        frameGeometry(),
        geometry(),
        childrenRect(),
        adjustSize(),
        mapFromGlobal(),
        mapFromParent()
        mapToGlobal(),
        mapToParent(),
        maximumSize(),
        minimumSize(),
        sizeIncrement(),
        setMaximumSize(),
        setMinimumSize(),
        setSizeIncrement(),
        setBaseSize(),
        setFixedSize()

    \row \i Mode \i
        isVisible(),
        isVisibleTo(),
        isMinimized(),
        isEnabled(),
        isEnabledTo(),
        isModal(),
        isWindow(),
        setEnabled(),
        hasMouseTracking(),
        setMouseTracking(),
        updatesEnabled(),
        setUpdatesEnabled(),
        visibleRegion().

    \row \i Look and feel \i
        style(),
        setStyle(),
        cursor(),
        setCursor()
        font(),
        setFont(),
        palette(),
        setPalette(),
        backgroundRole(),
        setBackgroundRole(),
        fontMetrics(),
        fontInfo().

    \row \i Keyboard focus functions \i
        setFocusPolicy(),
        focusPolicy(),
        hasFocus(),
        setFocus(),
        clearFocus(),
        setTabOrder(),
        setFocusProxy().

    \row \i Mouse and keyboard grabbing \i
        grabMouse(),
        releaseMouse(),
        grabKeyboard(),
        releaseKeyboard(),
        mouseGrabber(),
        keyboardGrabber().

    \row \i Event handlers \i
        event(),
        mousePressEvent(),
        mouseReleaseEvent(),
        mouseDoubleClickEvent(),
        mouseMoveEvent(),
        keyPressEvent(),
        keyReleaseEvent(),
        focusInEvent(),
        focusOutEvent(),
        wheelEvent(),
        enterEvent(),
        leaveEvent(),
        paintEvent(),
        moveEvent(),
        resizeEvent(),
        closeEvent(),
        dragEnterEvent(),
        dragMoveEvent(),
        dragLeaveEvent(),
        dropEvent(),
        childEvent(),
        showEvent(),
        hideEvent(),
        customEvent().
        changeEvent(),

    \row \i System functions \i
        parentWidget(),
        window(),
        setParent(),
        winId(),
        find(),
        metric().

    \row \i What's this help \i
        setWhatsThis()

    \row \i Internal kernel functions \i
        focusNextPrevChild(),

    \endtable

    Every widget's constructor accepts one or two standard arguments:
    \list 1
    \i \c{QWidget *parent = 0} is the parent of the new widget.
    If it is 0 (the default), the new widget will be a window.
    If not, it will be a child of \e parent, and be constrained by \e
    parent's geometry (unless you specify \c Qt::Window as
    window flag).
    \i \c{Qt::WFlags f = 0} (where available) sets the window flags; the
    default is suitable for almost all widgets, but to get, for
    example, a window without a window system frame, you
    must use special flags.
    \endlist

    The tictac/tictac.cpp example program is good example of a simple
    widget. It contains a few event handlers (as all widgets must), a
    few custom routines that are specific to it (as all useful widgets
    do), and has a few children and connections. Everything it does
    is done in response to an event: this is by far the most common way
    to design GUI applications.

    You will need to supply the content for your widgets yourself, but
    here is a brief run-down of the events, starting with the most common
    ones:

    \list

    \i paintEvent() - called whenever the widget needs to be
    repainted. Every widget which displays output must implement it.
    Painting using a QPainter can only take place in a paintEvent() or
    a function called by a paintEvent().

    \i resizeEvent() - called when the widget has been resized.

    \i mousePressEvent() - called when a mouse button is pressed.
    There are six mouse-related events, but the mouse press and mouse
    release events are by far the most important. A widget receives
    mouse press events when the mouse is inside it, or when it has
    grabbed the mouse using grabMouse().

    \i mouseReleaseEvent() - called when a mouse button is released.
    A widget receives mouse release events when it has received the
    corresponding mouse press event. This means that if the user
    presses the mouse inside \e your widget, then drags the mouse to
    somewhere else, then releases, \e your widget receives the release
    event. There is one exception: if a popup menu appears while the
    mouse button is held down, this popup immediately steals the mouse
    events.

    \i mouseDoubleClickEvent() - not quite as obvious as it might seem.
    If the user double-clicks, the widget receives a mouse press event
    (perhaps a mouse move event or two if they don't hold the mouse
    quite steady), a mouse release event and finally this event. It is
    \e{not possible} to distinguish a click from a double click until you've
    seen whether the second click arrives. (This is one reason why most GUI
    books recommend that double clicks be an extension of single clicks,
    rather than trigger a different action.)

    \endlist

    If your widget only contains child widgets, you probably do not need to
    implement any event handlers. If you want to detect a mouse click in
    a child widget call the child's underMouse() function inside the
    parent widget's mousePressEvent().

    Widgets that accept keyboard input need to reimplement a few more
    event handlers:

    \list

    \i keyPressEvent() - called whenever a key is pressed, and again
    when a key has been held down long enough for it to auto-repeat.
    Note that the Tab and Shift+Tab keys are only passed to the widget
    if they are not used by the focus-change mechanisms. To force those
    keys to be processed by your widget, you must reimplement
    QWidget::event().

    \i focusInEvent() - called when the widget gains keyboard focus
    (assuming you have called setFocusPolicy()). Well written widgets
    indicate that they own the keyboard focus in a clear but discreet
    way.

    \i focusOutEvent() - called when the widget loses keyboard focus.

    \endlist

    Some widgets will also need to reimplement some of the less common
    event handlers:

    \list

    \i mouseMoveEvent() - called whenever the mouse moves while a
    button is held down. This is useful for, for example, dragging. If
    you call setMouseTracking(true), you get mouse move events even
    when no buttons are held down. (See also the \link dnd.html drag
    and drop\endlink information.)

    \i keyReleaseEvent() - called whenever a key is released, and also
    while it is held down if the key is auto-repeating. In that case
    the widget receives a key release event and immediately a key press
    event for every repeat. Note that the Tab and Shift+Tab keys are
    only passed to the widget if they are not used by the focus-change
    mechanisms. To force those keys to be processed by your widget, you
    must reimplement QWidget::event().

    \i wheelEvent() -- called whenever the user turns the mouse wheel
    while the widget has the focus.

    \i enterEvent() - called when the mouse enters the widget's screen
    space. (This excludes screen space owned by any children of the
    widget.)

    \i leaveEvent() - called when the mouse leaves the widget's screen
    space.

    \i moveEvent() - called when the widget has been moved relative to its
    parent.

    \i closeEvent() - called when the user closes the widget (or when
    close() is called).

    \endlist

    There are also some rather obscure events. They are listed in
    \c qevent.h and you need to reimplement event() to handle them.
    The default implementation of event() handles Tab and Shift+Tab
    (to move the keyboard focus), and passes on most other events to
    one of the more specialized handlers above.

    When implementing a widget, there are a few more things to
    consider.

    \list

    \i In the constructor, be sure to set up your member variables
    early on, before there's any chance that you might receive an event.

    \i It is almost always useful to reimplement sizeHint() and to set
    the correct size policy with setSizePolicy(), so users of your class
    can set up layout management more easily. A size policy lets you
    supply good defaults for the layout management handling, so that
    other widgets can contain and manage yours easily. sizeHint()
    indicates a "good" size for the widget.

    \i If your widget is a window, setWindowTitle() and
    setWindowIcon() set the title bar and icon respectively.

    \endlist

    From Qt 4.0, QWidget automatically double-buffers its painting, so
    there's no need to write double-buffering code in paintEvent() to
    avoid flicker.

    \sa QEvent, QPainter, QGridLayout, QBoxLayout
*/


QWidgetMapper *QWidgetPrivate::mapper = 0;                // app global widget mapper


/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/


static QFont qt_naturalWidgetFont(QWidget* w) {
    QFont naturalfont = QApplication::font(w);
    if (! w->isWindow()) {
        if (! naturalfont.isCopyOf(QApplication::font()))
            naturalfont = naturalfont.resolve(w->parentWidget()->font());
        else
            naturalfont = w->parentWidget()->font();
    }
    naturalfont.resolve(0);
    return naturalfont;
}

static QPalette qt_naturalWidgetPalette(QWidget* w) {
    QPalette naturalpalette = QApplication::palette(w);
    if (! w->isWindow()) {
        if (! naturalpalette.isCopyOf(QApplication::palette()))
            naturalpalette = naturalpalette.resolve(w->parentWidget()->palette());
        else
            naturalpalette = w->parentWidget()->palette();
    }
    naturalpalette.resolve(0);
    return naturalpalette;
}


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

/*
    Widget state flags:
  \list
  \i Qt::WA_WState_Created The widget has a valid winId().
  \i Qt::WA_WState_Visible The widget is currently visible.
  \i Qt::WA_WState_Hidden The widget is hidden, i.e. it won't
  become visible unless you call show() on it. Qt::WA_WState_Hidden
  implies !Qt::WA_WState_Visible.
  \i Qt::WA_WState_CompressKeys Compress keyboard events.
  \i Qt::WA_WState_BlockUpdates Repaints and updates are disabled.
  \i Qt::WA_WState_InPaintEvent Currently processing a paint event.
  \i Qt::WA_WState_Reparented The widget has been reparented.
  \i Qt::WA_WState_ConfigPending A configuration (resize/move) event is pending.
  \i Qt::WA_WState_DND The widget supports drag and drop, see setAcceptDrops().
  \endlist
*/

/*!
    Constructs a widget which is a child of \a parent, with  widget
    flags set to \a f.

    If \a parent is 0, the new widget becomes a window. If
    \a parent is another widget, this widget becomes a child window
    inside \a parent. The new widget is deleted when its \a parent is
    deleted.

    The widget flags argument, \a f, is normally 0, but it can be set
    to customize the frame of a window (i.e. \a
    parent must be 0). To customize the frame, set the \c
    Qt::WStyle_Customize flag OR'ed with any of the \l{Qt::WFlag}s.

    If you add a child widget to an already visible widget you must
    explicitly show the child to make it visible.

    Note that the X11 version of Qt may not be able to deliver all
    combinations of style flags on all systems. This is because on
    X11, Qt can only ask the window manager, and the window manager
    can override the application's settings. On Windows, Qt can set
    whatever flags you want.
*/

QWidget::QWidget(QWidget *parent, Qt::WFlags f)
    : QObject(*new QWidgetPrivate, ((parent && (parent->windowType() == Qt::Desktop)) ? 0 : parent)), QPaintDevice(QInternal::Widget)
{
    d_func()->init(f);
}

#ifdef QT3_SUPPORT
/*!
    \overload
    \obsolete
 */
QWidget::QWidget(QWidget *parent, const char *name, Qt::WFlags f)
    : QObject(*new QWidgetPrivate, ((parent && (parent->windowType() == Qt::Desktop)) ? 0 : parent)), QPaintDevice(QInternal::Widget)
{
    d_func()->init(f);
    setObjectName(name);
}
#endif

/*! \internal
*/
QWidget::QWidget(QWidgetPrivate &dd, QWidget* parent, Qt::WFlags f)
    : QObject(dd, ((parent && (parent->windowType() == Qt::Desktop)) ? 0 : parent)), QPaintDevice(QInternal::Widget)
{
    d_func()->init(f);
}

void QWidgetPrivate::init(Qt::WFlags f)
{
    Q_Q(QWidget);
    q->data = &data;
    if (qApp->type() == QApplication::Tty)
        qWarning("QWidget: Cannot create a QWidget when no GUI is being used");

#ifdef QT_THREAD_SUPPORT
    if (!q->parent()) {
        Q_ASSERT_X(q->thread() == qApp->thread(), "QWidget",
                   "Widgets must be created in the GUI thread.");
    }
#endif

    data.fstrut_dirty = 1;

    data.winid = 0;
    data.widget_attributes = 0;
    data.window_flags = f;
    data.window_state = 0;
    data.focus_policy = 0;
    data.context_menu_policy = Qt::DefaultContextMenu;

    data.sizehint_forced = 0;
    data.is_closing = 0;
    data.in_show = 0;
    data.in_set_window_state = 0;

    q->setAttribute(Qt::WA_QuitOnClose); // might be cleared in create()

    q->create();                                        // platform-dependent init
#if defined(Q_WS_X11)
    data.fnt.x11SetScreen(xinfo.screen());
#endif // Q_WS_X11

    if (!(q->windowType() == Qt::Desktop))
        updateSystemBackground();
    if (q->isWindow()) {
        if (QApplication::isRightToLeft())
            q->setAttribute(Qt::WA_RightToLeft);
#ifdef Q_WS_MAC
        extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
        if(!qt_mac_is_macdrawer(q)) //special case
            q->setAttribute(Qt::WA_WState_Hidden);
#else
        q->setAttribute(Qt::WA_WState_Hidden);
#endif
        createTLExtra();
    } else {
        QWidget *parentWidget = q->parentWidget();
        // propagate palette
        data.pal = parentWidget->data->pal;
        data.pal.resolve(0);
        //propagate font
        data.fnt = parentWidget->data->fnt;
        data.fnt.resolve(0);
        // propagate enabled state
        if (!parentWidget->isEnabled())
            q->setAttribute(Qt::WA_Disabled, true);
        // propgate updates enabled state
        if (!parentWidget->updatesEnabled())
            q->setAttribute(Qt::WA_UpdatesDisabled, true);
        //propagate layout direction
        if (parentWidget->testAttribute(Qt::WA_RightToLeft))
            q->setAttribute(Qt::WA_RightToLeft);
        // new widgets do not show up in already visible parents
        if (parentWidget->isVisible())
            q->setAttribute(Qt::WA_WState_Hidden);
    }

    if (q->isWindow()) {
        focus_next = q;
    } else {
        // insert at the end of the focus chain
        QWidget *focus_handler = q->window();
        QWidget *w = focus_handler;
        while (w->d_func()->focus_next != focus_handler)
            w = w->d_func()->focus_next;
        w->d_func()->focus_next = q;
        focus_next = focus_handler;
    }

    q->setAttribute(Qt::WA_PendingMoveEvent);
    q->setAttribute(Qt::WA_PendingResizeEvent);

    if (++QWidgetPrivate::instanceCounter > QWidgetPrivate::maxInstances)
        QWidgetPrivate::maxInstances = QWidgetPrivate::instanceCounter;

    QEvent e(QEvent::Create);
    QApplication::sendEvent(q, &e);
    QApplication::postEvent(q, new QEvent(QEvent::PolishRequest));

    extraPaintEngine = 0;
}


/*!
    Creates a new widget window if \a window is 0, otherwise sets the
    widget's window to \a window.

    Initializes the window (sets the geometry etc.) if \a
    initializeWindow is true. If \a initializeWindow is false, no
    initialization is performed. This parameter only makes sense if \a
    window is a valid window.

    Destroys the old window if \a destroyOldWindow is true. If \a
    destroyOldWindow is false, you are responsible for destroying the
    window yourself (using platform native code).

    The QWidget constructor calls create(0,true,true) to create a
    window for this widget.
*/

void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_D(QWidget);
    if (testAttribute(Qt::WA_WState_Created) && window == 0)
        return;
    setAttribute(Qt::WA_WState_Created);                        // set created flag

    Qt::WindowType type = windowType();
    Qt::WindowFlags &flags = data->window_flags;

    if (type == Qt::Widget && !parentWidget()) {
        type = Qt::Window;
        flags |= Qt::Window;
    }
#ifdef QT3_SUPPORT
    if (flags & Qt::WStaticContents)
        setAttribute(Qt::WA_StaticContents);
    if (flags & Qt::WDestructiveClose)
	setAttribute(Qt::WA_DeleteOnClose);
    if (flags & Qt::WShowModal)
	setAttribute(Qt::WA_ShowModal);
    if (flags & Qt::WMouseNoMask)
	setAttribute(Qt::WA_MouseNoMask);
    if (flags & Qt::WGroupLeader)
	setAttribute(Qt::WA_GroupLeader);
    if (flags & Qt::WNoMousePropagation)
	setAttribute(Qt::WA_NoMousePropagation);
#endif
    if(type == Qt::Dialog && !testAttribute(Qt::WA_ShowModal)
       && parentWidget() && parentWidget()->testAttribute(Qt::WA_ShowModal))
        setAttribute(Qt::WA_ShowModal);

    if ( type != Qt::Widget && type != Qt::Window)
        setAttribute(Qt::WA_QuitOnClose, false);

    d->create_sys(window, initializeWindow, destroyOldWindow);

    // send and post remaining QObject events
    if (parent() && d->sendChildEvents) {
        QChildEvent e(QEvent::ChildAdded, this);
        QApplication::sendEvent(parent(), &e);
#ifdef QT3_SUPPORT
        QApplication::postEvent(parent(), new QChildEvent(QEvent::ChildInserted, this));
#endif
    }
}

/*!
    Destroys the widget.

    All this widget's children are deleted first. The application
    exits if this widget is the main widget.
*/

QWidget::~QWidget()
{
    Q_D(QWidget);
#if defined (QT_CHECK_STATE)
    if (paintingActive())
        qWarning("%s (%s): deleted while being painted", className(), name());
#endif

    // remove all actions from this widget
    for (int i = 0; i < d->actions.size(); ++i) {
        QActionPrivate *apriv = d->actions.at(i)->d_func();
        apriv->widgets.removeAll(this);
    }
    d->actions.clear();

    // Remove all shortcuts grabbed by this
    // widget, unless application is closing
    if (!QApplicationPrivate::is_app_closing && testAttribute(Qt::WA_GrabbedShortcut))
        qApp->d_func()->shortcutMap.removeShortcut(0, this, QKeySequence());

    // delete layout while we still are a valid widget
#ifndef QT_NO_LAYOUT
    delete d->layout;
#endif
    // Remove myself focus list
    // ### Focus: maybe remove children aswell?
    QWidget *w = this;
    while (w->d_func()->focus_next != this)
        w = w->d_func()->focus_next;
    w->d_func()->focus_next = d_func()->focus_next;
    d_func()->focus_next = 0;

#ifdef QT3_SUPPORT
    if (QApplicationPrivate::main_widget == this) {        // reset main widget
        QApplicationPrivate::main_widget = 0;
        qApp->quit();
    }
#endif

    clearFocus();

    if (isWindow() && !isExplicitlyHidden() && winId())
        hide();

    // A parent widget must destroy all its children before destroying itself
    while (!d->children.isEmpty())
        delete d->children.takeFirst();

    QApplication::removePostedEvents(this);

    destroy();                                        // platform-dependent cleanup

    --QWidgetPrivate::instanceCounter;

    QEvent e(QEvent::Destroy);
    QCoreApplication::sendEvent(this, &e);
}

int QWidgetPrivate::instanceCounter = 0;  // Current number of widget instances
int QWidgetPrivate::maxInstances = 0;     // Maximum number of widget instances

void QWidgetPrivate::setWinId(WId id)                // set widget identifier
{
    Q_Q(QWidget);
    if (!mapper)                                // mapper destroyed
        return;
    if (data.winid)
        mapper->remove(data.winid);

    data.winid = id;
#if defined(Q_WS_X11)
    hd = id; // X11: hd == ident
#endif
    if (id)
        mapper->insert(data.winid, q);
}

void QWidgetPrivate::createTLExtra()
{
    if (!extra)
        createExtra();
    if (!extra->topextra) {
        QTLWExtra* x = extra->topextra = new QTLWExtra;
#if defined(Q_WS_WIN) || defined(Q_WS_MAC) || defined (Q_WS_QWS)
        x->opacity = 255;
#endif
#ifndef QT_NO_WIDGET_TOPEXTRA
        x->icon = 0;
        x->iconPixmap = 0;
#endif
        x->fleft = x->fright = x->ftop = x->fbottom = 0;
        x->incw = x->inch = 0;
        x->basew = x->baseh = 0;
        x->normalGeometry = QRect(0,0,-1,-1);
#if defined(Q_WS_X11)
        x->embedded = 0;
        x->parentWinId = 0;
        x->spont_unmapped = 0;
        x->dnd = 0;
        x->uspos = 0;
        x->ussize = 0;
#endif
        x->savedFlags = 0;
#if defined(Q_WS_QWS)
        x->inPaintTransaction = false;
        x->backingStore = 0;
#if !defined(QT_NO_QWS_MANAGER)
        x->qwsManager = 0;
#endif
#endif
        createTLSysExtra();
    }
}

/*!
  \internal
  Creates the widget extra data.
*/

void QWidgetPrivate::createExtra()
{
    if (!extra) {                                // if not exists
        extra = new QWExtra;
        extra->minw = extra->minh = 0;
        extra->maxw = extra->maxh = QWIDGETSIZE_MAX;
#ifndef QT_NO_CURSOR
        extra->curs = 0;
#endif
        extra->topextra = 0;
        extra->style = 0;
        extra->size_policy = QSizePolicy(QSizePolicy::Preferred,
                                          QSizePolicy::Preferred);
        createSysExtra();
    }
}


/*!
  \internal
  Deletes the widget extra data.
*/

void QWidgetPrivate::deleteExtra()
{
    if (extra) {                                // if exists
#ifndef QT_NO_CURSOR
        delete extra->curs;
#endif
        deleteSysExtra();
        if (extra->topextra) {
            deleteTLSysExtra();
#ifndef QT_NO_WIDGET_TOPEXTRA
            delete extra->topextra->icon;
            delete extra->topextra->iconPixmap;
#endif
#if defined(Q_WS_QWS) && !defined(QT_NO_QWS_MANAGER)
            delete extra->topextra->qwsManager;
#endif
            delete extra->topextra;
        }
        delete extra;
        // extra->xic destroyed in QWidget::destroy()
        extra = 0;
    }
}

/*
  Returns true if the background is inherited; otherwise returns
  false.

  A widget does not inherit its parent's background if
  setBackgroundRole() was called, or a brush is defined for the
  background role.
*/

bool QWidgetPrivate::isBackgroundInherited() const
{
    Q_Q(const QWidget);

    // windows do not inherit their background
    if (q->isWindow() || q->windowType() == Qt::SubWindow)
        return false;

    const QPalette &pal = q->palette();
    QPalette::ColorRole bg = q->backgroundRole();
    QBrush brush = pal.brush(bg);

    // non opaque brushes leaves us no choice, we must inherit
    if (!brush.isOpaque())
        return true;

    // if we have a background role set, or a custom brush for the
    // background role, then the background is not inherited
    if (q->testAttribute(Qt::WA_SetBackgroundRole) || (pal.resolve() & (1<<bg)))
        return false;

    if (brush.style() == Qt::SolidPattern) {
        // the background is just a solid color. If there is no
        // propagated contents, then we claim as performance
        // optimization that it was not inheritet. This is the normal
        // case in standard Windows or Motif style.
        const QWidget *w = q->parentWidget();
        if (!w->testAttribute(Qt::WA_ContentsPropagated) && !w->d_func()->isBackgroundInherited())
            return false;
    }

    return true;
}

/*
  In case a widget inherits its parent's pixmap background or content,
  and possibly propagates it further to its own children, this
  function updates everthing that needs to be updated after a move.

  This is necessary because the pixmap offset has changed. This is not
  necessary on Mac OS X because due to the composite manager we get
  this for free, and as a result doing it here will actually force
  more paints than are necessary, this is a NO-OP for a composited
  windowing system like Mac OS X.
 */
void QWidgetPrivate::updateInheritedBackground()
{
#if !defined(Q_WS_MAC) && !defined(Q_WS_QWS)
    Q_Q(QWidget);
    if (!q->isVisible() || !isBackgroundInherited())
        return;
    q->repaint();
    for (int i = 0; i < children.size(); ++i)
        if (QWidget *w = qobject_cast<QWidget *>(children.at(i)))
            w->d_func()->updateInheritedBackground();
#endif
}

/*
  In case a widget propagates its or its ancestor's contents to its
  children, this function updates everything that needs to be updated
  after a resize.

  Call this only when Qt::WA_ContentsPropagated is set. This is not
  necessary on Mac OS X because due to the composite manager we get
  this for free, and as a result doing it here will actually force
  more paints than are necessary, this is a NO-OP for a composited
  windowing system like Mac OS X.
 */
void QWidgetPrivate::updatePropagatedBackground(const QRegion *reg)
{
#if !defined(Q_WS_MAC) && !defined(Q_WS_QWS)
    for (int i = 0; i < children.size(); ++i) {
        if (QWidget *w = qobject_cast<QWidget*>(children.at(i))) {
            if (reg && !reg->boundingRect().intersects(w->geometry()))
                continue;
            w->d_func()->updateInheritedBackground();
        }
    }
#else
    Q_UNUSED(reg)
#endif
}


void QWidgetPrivate::composeBackground(const QRect &crect)
{
    Q_Q(QWidget);
#if 0 //DEBUG
    static int ii = 0;
    //QColor bgCol = QColor::fromHsv(double(ii%5)/5.0, double(ii%33+22)/55.0, double(ii%17+33)/50.0);
    QColor bgCol = QColor::fromHsv((ii%5*360)/5, (255*(ii%33+22))/55, (255*(ii%17+33)/50));
    QPainter bgPainter(q);
    bgPainter.fillRect(crect, bgCol);
    ii = (ii+1) % 4095;
    return;
#endif


    QPoint offset;
    QVector<QWidget*> layers;
    QWidget *w = q;
    layers += w;

    // Build the stack of widgets to composite
    while (w->d_func()->isBackgroundInherited()) {
        offset += w->pos();
        layers += (w = w->parentWidget());
    }

    QWidget *top = w;
    for (int i=layers.size() - 1; i>=0; --i) {
        w = layers.at(i);

        // Remove the offset for previous layer.
        if (top != w)
            offset -= w->pos();

        // Do the background if needed.
        QBrush bgBrush = w->palette().brush(w->backgroundRole());
        if (w == top || (!bgBrush.isOpaque() && w->testAttribute(Qt::WA_SetPalette))) {
            QPainter bgPainter(q);
            bgPainter.setBrushOrigin(-offset);
            bgPainter.fillRect(crect, bgBrush);
        }

        // Propagate contents if enabled and w is not the actual widget.
        if (w->testAttribute(Qt::WA_ContentsPropagated) && i>0) {
            // Setup redirection from w to this widget.
            QRect rr = crect;
            rr.translate(offset);
            bool was_in_paint_event = w->testAttribute(Qt::WA_WState_InPaintEvent);
            w->setAttribute(Qt::WA_WState_InPaintEvent);
            QPainter::setRedirected(w, q, offset);
            QPaintEvent e(rr);
            QApplication::sendEvent(w, &e);
            w->setAttribute(Qt::WA_WState_InPaintEvent, was_in_paint_event);
            QPainter::restoreRedirected(w);
        }
    }
}


void QWidgetPrivate::setUpdatesEnabled_helper(bool enable)
{
    Q_Q(QWidget);

    if (enable && !q->isWindow() && q->parentWidget() && !q->parentWidget()->updatesEnabled())
        return; // nothing we can do

    if (enable != q->testAttribute(Qt::WA_UpdatesDisabled))
        return; // nothing to do

    q->setAttribute(Qt::WA_UpdatesDisabled, !enable);
    if (enable)
        q->update();

    Qt::WidgetAttribute attribute = enable ? Qt::WA_ForceUpdatesDisabled : Qt::WA_UpdatesDisabled;
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(children.at(i));
        if (w && !w->testAttribute(attribute))
            w->d_func()->setUpdatesEnabled_helper(enable);
    }
}

void QWidgetPrivate::propagatePaletteChange()
{
    Q_Q(QWidget);
    QEvent pc(QEvent::PaletteChange);
    QApplication::sendEvent(q, &pc);
    if(!children.isEmpty()) {
        for(int i = 0; i < children.size(); ++i) {
            QWidget *w = qobject_cast<QWidget*>(children.at(i));
            if(w && !w->isWindow())
                w->d_func()->resolvePalette();
        }
    }
#if defined(QT3_SUPPORT)
    q->paletteChange(q->palette()); // compatibility
#endif
}


/*
  Returns the widget's clipping rectangle.
*/
QRect QWidgetPrivate::clipRect() const
{
    Q_Q(const QWidget);
    QRect r = q->rect();
    const QWidget * w = q;
    int ox = 0;
    int oy = 0;
    while (w
            && w->isVisible()
            && !w->isWindow()
            && w->parentWidget()) {
        ox -= w->x();
        oy -= w->y();
        w = w->parentWidget();
        r = r.intersect(QRect(ox, oy, w->width(), w->height()));
    }
    if (!w->isVisible())
        return QRect();
    return r;
}

/*!
    \fn void QPixmap::fill(const QWidget *widget, const QPoint &offset)

    Fills the pixmap with the \a widget's background color or pixmap.

    The \a offset point is an offset in the widget.

    The point \a offset is a point in the widget's coordinate
    system. The pixmap's top-left pixel will be mapped to the point \a
    offset in the widget. This is significant if the widget has a
    background pixmap; otherwise the pixmap will simply be filled with
    the background color of the widget.
*/

void QPixmap::fill( const QWidget *widget, const QPoint &off )
{
    QPainter::setRedirected(widget, this, off);
    const_cast<QWidget *>(widget)->d_func()->composeBackground(widget->rect());
    QPainter::restoreRedirected(widget);
}

/*!
  \internal
  This function is called when a widget is hidden or destroyed.
  It resets some application global pointers that should only refer active,
  visible widgets.
*/

void QWidgetPrivate::deactivateWidgetCleanup()
{
    Q_Q(QWidget);
    // If this was the active application window, reset it
    if (qApp->activeWindow() == q)
        qApp->setActiveWindow(0);
    // If the is the active mouse press widget, reset it
#ifdef Q_WS_MAC
    extern QPointer<QWidget> qt_button_down;
#else
    extern QWidget *qt_button_down;
#endif
    if (q == qt_button_down)
        qt_button_down = 0;
}


/*!
    Returns a pointer to the widget with window identifer/handle \a
    id.

    The window identifier type depends on the underlying window
    system, see \c qwindowdefs.h for the actual definition. If there
    is no widget with this identifier, 0 is returned.
*/

QWidget *QWidget::find(WId id)
{
    return QWidgetPrivate::mapper ? QWidgetPrivate::mapper->value(id, 0) : 0;
}



/*!
    \fn WId QWidget::winId() const

    Returns the window system identifier of the widget.

    Portable in principle, but if you use it you are probably about to
    do something non-portable. Be careful.


    \sa find()
*/

/*!
    Returns the GUI style for this widget

    \sa QWidget::setStyle(), QApplication::setStyle(), QApplication::style()
*/

QStyle *QWidget::style() const
{
    Q_D(const QWidget);
    if (d->extra && d->extra->style)
        return d->extra->style;
    return qApp->style();
}

/*!
    Sets the widget's GUI style to \a style. Ownership of the style
    object is not transferred.

    If no style is set, the widget uses the application's style,
    QApplication::style() instead.

    Setting a widget's style has no effect on existing or future child
    widgets.

    \warning This function is particularly useful for demonstration
    purposes, where you want to show Qt's styling capabilities. Real
    applications should avoid it and use one consistent GUI style
    instead.

    \sa style(), QStyle, QApplication::style(), QApplication::setStyle()
*/

void QWidget::setStyle(QStyle *style)
{
    Q_D(QWidget);
    QStyle *old  = QWidget::style();
    d->createExtra();
    d->extra->style = style;
    if (!(windowType() == Qt::Desktop) // (except desktop)
         && d->polished) { // (and have been polished)
        old->unpolish(this);
        QWidget::style()->polish(this);
    }
    QEvent e(QEvent::StyleChange);
    QApplication::sendEvent(this, &e);
#ifdef QT3_SUPPORT
    styleChange(*old);
#endif
}

#ifdef QT3_SUPPORT
/*!
    \overload

    Sets the widget's GUI style to \a style using the QStyleFactory.
*/
QStyle* QWidget::setStyle(const QString &style)
{
    QStyle *s = QStyleFactory::create(style);
    setStyle(s);
    return s;
}
#endif

/*!
    \property QWidget::isWindow
    \brief whether the widget is an independent window

    A window is a widget that isn't visually the child of any other
    widget and that usually has a frame and a
    \l{QWidget::setWindowTitle()}{window title}.

    A window can have a \l{QWidget::parentWidget()}{parent widget}.
    It will then be grouped with its parent and deleted when the
    parent is deleted, minimized when the parent is minimized etc. If
    supported by the window manager, it will also have a common
    taskbar entry with its parent.

    QDialog and QMainWindow widgets are by default windows, even if a
    parent widget is specified in the constructor. This behavior is
    specified by the Qt::Window flag.

    \sa window(), isModal(), parentWidget()
*/

/*!
    \property QWidget::modal
    \brief whether the widget is a modal widget

    This property only makes sense for windows. A modal widget
    prevents widgets in all other windows from getting any input.

    \sa isWindow(), QDialog
*/

/*!
    \fn bool QWidget::underMouse() const

    Returns true if the widget is under the mouse cursor; otherwise
    returns false.

    This value is not updated properly during drag and drop
    operations.

    \sa enterEvent(),  leaveEvent()
*/

/*!
    \property QWidget::minimized
    \brief whether this widget is minimized (iconified)

    This property is only relevant for windows.

    \sa showMinimized(), visible, show(), hide(), showNormal(), maximized
*/
bool QWidget::isMinimized() const
{ return data->window_state & Qt::WindowMinimized; }

/*!
    Shows the widget minimized, as an icon.

    Calling this function only affects \l{isWindow()}{windows}.

    \sa showNormal(), showMaximized(), show(), hide(), isVisible(),
        isMinimized()
*/
void QWidget::showMinimized()
{
    bool isMin = isMinimized();
    if (isMin && isVisible()) return;

    ensurePolished();
#ifdef QT3_SUPPORT
    if (parent())
        QApplication::sendPostedEvents(parent(), QEvent::ChildInserted);
#endif

    if (!isMin)
        setWindowState((windowState() & ~Qt::WindowActive) | Qt::WindowMinimized);
    show();
}

/*!
    \property QWidget::maximized
    \brief whether this widget is maximized

    This property is only relevant for windows.

    Note that due to limitations in some window-systems, this does not
    always report the expected results (e.g. if the user on X11
    maximizes the window via the window manager, Qt has no way of
    distinguishing this from any other resize). This is expected to
    improve as window manager protocols evolve.

    \sa windowState(), showMaximized(), visible, show(), hide(), showNormal(), minimized
*/
bool QWidget::isMaximized() const
{ return data->window_state & Qt::WindowMaximized; }



/*!  Returns the current window state. The window state is a OR'ed
  combination of Qt::WindowState: \c Qt::WindowMinimized, \c
  Qt::WindowMaximized, \c Qt::WindowFullScreen and \c Qt::WindowActive.

  \sa Qt::WindowState setWindowState()
 */
Qt::WindowStates QWidget::windowState() const
{
    return (Qt::WindowStates)data->window_state;
}

/*!\internal

   The function sets the window state without sending any events. It
   exists mainly to keep Q3Workspace working.
 */
void QWidget::overrideWindowState(Qt::WindowStates newstate)
{
    data->window_state  = newstate;
}

/*!
  \fn void QWidget::setWindowState(Qt::WindowStates windowState)

  Sets the window state to \a windowState. The window state is a OR'ed
  combination of Qt::WindowState: \c Qt::WindowMinimized, \c
  Qt::WindowMaximized, \c Qt::WindowFullScreen and \c Qt::WindowActive.

  If the window is not visible (i.e. isVisible() returns false), the
  window state will take effect when show() is called. For visible
  windows, the change is immediate. For example, to toggle between
  full-screen and mormal mode, use the following code:

  \code
        w->setWindowState(w->windowState() ^ Qt::WindowFullScreen);
  \endcode

  In order to restore and activate a minimized window (while
  preserving its maximized and/or full-screen state), use the following:

  \code
        w->setWindowState(w->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
  \endcode

  Note: On some window systems \c Qt::WindowActive is not immediate, and may be
  ignored in certain cases.

  \sa Qt::WindowState windowState()
*/

/*!
    \property QWidget::fullScreen
    \brief whether the widget is full screen

    \sa windowState(), minimized, maximized
*/
bool QWidget::isFullScreen() const
{ return data->window_state & Qt::WindowFullScreen; }

/*!
    Shows the widget in full-screen mode.

    Calling this function only affects windows.

    To return from full-screen mode, call showNormal().

    Full-screen mode works fine under Windows, but has certain
    problems under X. These problems are due to limitations of the
    ICCCM protocol that specifies the communication between X11
    clients and the window manager. ICCCM simply does not understand
    the concept of non-decorated full-screen windows. Therefore, the
    best we can do is to request a borderless window and place and
    resize it to fill the entire screen. Depending on the window
    manager, this may or may not work. The borderless window is
    requested using MOTIF hints, which are at least partially
    supported by virtually all modern window managers.

    An alternative would be to bypass the window manager entirely and
    create a window with the Qt::WX11BypassWM flag. This has other severe
    problems though, like totally broken keyboard focus and very
    strange effects on desktop changes or when the user raises other
    windows.

    X11 window managers that follow modern post-ICCCM specifications
    support full-screen mode properly.

    \sa showNormal(), showMaximized(), show(), hide(), isVisible()
*/
void QWidget::showFullScreen()
{
    bool isFull = isFullScreen();
    if (isFull && isVisible())
	return;

    ensurePolished();
#ifdef QT3_SUPPORT
    if (parent())
        QApplication::sendPostedEvents(parent(), QEvent::ChildInserted);
#endif

    if (!isFull)
	setWindowState(windowState() | Qt::WindowFullScreen);
    show();
    activateWindow();
}

/*!
    Shows the widget maximized.

    Calling this function only affects \l{isWindow()}{windows}.

    On X11, this function may not work properly with certain window
    managers. See \l{geometry.html}{Window Geometry} for an explanation.

    \sa setWindowState(), showNormal(), showMinimized(), show(), hide(), isVisible()
*/
void QWidget::showMaximized()
{
    if (isMaximized() && isVisible() && !isMinimized())
	return;

    ensurePolished();
#ifdef QT3_SUPPORT
    if (parent())
        QApplication::sendPostedEvents(parent(), QEvent::ChildInserted);
#endif

    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowMaximized);
    show();
}

/*!
    Restores the widget after it has been maximized or minimized.

    Calling this function only affects \l{isWindow()}{windows}.

    \sa setWindowState(), showMinimized(), showMaximized(), show(), hide(), isVisible()
*/
void QWidget::showNormal()
{
    ensurePolished();
#ifdef QT3_SUPPORT
    if (parent())
        QApplication::sendPostedEvents(parent(), QEvent::ChildInserted);
#endif

    setWindowState(Qt::WindowNoState);
    show();
}

/*!
    Returns true if this widget would become enabled if \a ancestor is
    enabled; otherwise returns false.



    This is the case if neither the widget itself nor every parent up
    to but excluding \a ancestor has been explicitly disabled.

    isEnabledTo(0) is equivalent to isEnabled().

    \sa setEnabled() enabled
*/

bool QWidget::isEnabledTo(QWidget* ancestor) const
{
    const QWidget * w = this;
    while (w && !w->testAttribute(Qt::WA_ForceDisabled)
            && !w->isWindow()
            && w->parentWidget()
            && w->parentWidget() != ancestor)
        w = w->parentWidget();
    return !w->testAttribute(Qt::WA_ForceDisabled);
}

/*!
    Appends the action \a action to this widget's list of actions.

    All QWidgets have list of QActions, however they can be
    represented graphically in many different ways. The default use of
    the QAction list (as returned by actions()) is to create a context
    QMenu.

    \sa removeAction() QMenu
*/
void QWidget::addAction(QAction *action)
{
    insertAction(0, action);
}

/*!
    Appends the actions \a actions to this widget's list of actions.

    \sa removeAction() QMenu addAction()
*/
void QWidget::addActions(QList<QAction*> actions)
{
    for(int i = 0; i < actions.count(); i++)
        insertAction(0, actions.at(i));
}

/*!
    Inserts the action \a action to this widget's list of actions,
    before the action \a before. It appends the action if \a before is 0 or
    \a before is not a valid action for this widget.

    \sa addAction()
*/
void QWidget::insertAction(QAction *before, QAction *action)
{
    if(!action) {
        qWarning("Attempt to insert null action!");
        return;
    }

    Q_D(QWidget);
    if(d->actions.contains(action))
        d->actions.removeAll(action);
    int pos = d->actions.indexOf(before);
    if (pos < 0) {
        before = 0;
        pos = d->actions.size();
    }
    d->actions.insert(pos, action);

    QActionPrivate *apriv = action->d_func();
    apriv->widgets.append(this);

    QActionEvent e(QEvent::ActionAdded, action, before);
    QApplication::sendEvent(this, &e);
}

/*!
    Inserts the actions \a actions to this widget's list of actions,
    before the action \a before. It appends the action if \a before is 0 or
    \a before is not a valid action for this widget.

    \sa removeAction() QMenu insertAction()
*/
void QWidget::insertActions(QAction *before, QList<QAction*> actions)
{
    for(int i = 0; i < actions.count(); ++i)
        insertAction(before, actions.at(i));
}

/*!
    Removes the action \a action from this widget's list of actions.
*/
void QWidget::removeAction(QAction *action)
{
    if (!action)
        return;

    Q_D(QWidget);

    QActionPrivate *apriv = action->d_func();
    apriv->widgets.removeAll(this);

    if (d->actions.removeAll(action)) {
        QActionEvent e(QEvent::ActionRemoved, action);
        QApplication::sendEvent(this, &e);
    }
}

/*!
    Returns the (possibly empty) list of this widget's actions.
*/
QList<QAction*> QWidget::actions() const
{
    Q_D(const QWidget);
    return d->actions;
}

/*!
  \fn bool QWidget::isEnabledToTLW() const
  \obsolete

  This function is deprecated. It is equivalent to isEnabled()
*/

/*!
    \property QWidget::enabled
    \brief whether the widget is enabled

    An enabled widget handles keyboard and mouse events; a disabled
    widget does not.



    Some widgets display themselves differently when they are
    disabled. For example a button might draw its label grayed out. If
    your widget needs to know when it becomes enabled or disabled, you
    can use the changeEvent() with type QEvent::EnabledChange.

    Disabling a widget implicitly disables all its children. Enabling
    respectively enables all child widgets unless they have been
    explicitly disabled.

    \sa isEnabled(), isEnabledTo(), QKeyEvent, QMouseEvent, changeEvent()
*/
void QWidget::setEnabled(bool enable)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_ForceDisabled, !enable);
    d->setEnabled_helper(enable);
}

void QWidgetPrivate::setEnabled_helper(bool enable)
{
    Q_Q(QWidget);

    if (enable && !q->isWindow() && q->parentWidget() && !q->parentWidget()->isEnabled())
        return; // nothing we can do

    if (enable != q->testAttribute(Qt::WA_Disabled))
        return; // nothing to do

    q->setAttribute(Qt::WA_Disabled, !enable);
    updateSystemBackground();

    if (!enable && q->window()->focusWidget() == q) {
        bool parentIsEnabled = (!q->parentWidget() || q->parentWidget()->isEnabled());
        if (!parentIsEnabled || !q->focusNextPrevChild(true))
            q->clearFocus();
    }

    Qt::WidgetAttribute attribute = enable ? Qt::WA_ForceDisabled : Qt::WA_Disabled;
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(children.at(i));
        if (w && !w->testAttribute(attribute))
            w->d_func()->setEnabled_helper(enable);
    }
#if defined(Q_WS_X11)
    if (q->testAttribute(Qt::WA_SetCursor)) {
        // enforce the windows behavior of clearing the cursor on
        // disabled widgets
        extern void qt_x11_enforce_cursor(QWidget * w); // defined in qwidget_x11.cpp
        qt_x11_enforce_cursor(q);
    }
#endif
#ifdef Q_WS_WIN
    QWinInputContext::enable(q, q->testAttribute(Qt::WA_InputMethodEnabled) && enable);
#endif
    QEvent e(QEvent::EnabledChange);
    QApplication::sendEvent(q, &e);
#ifdef QT3_SUPPORT
    q->enabledChange(!enable); // compatibility
#endif
}

/*!
    \fn void QWidget::enabledChange(bool)

    \internal
    \obsolete
*/

/*!
    \fn void QWidget::paletteChange(const QPalette &)

    \internal
    \obsolete
*/

/*!
    \fn void QWidget::fontChange(const QFont &)

    \internal
    \obsolete
*/

/*!
    \fn void QWidget::windowActivationChange(bool)

    \internal
    \obsolete
*/

/*!
    \fn void QWidget::languageChange()

    \internal
    \obsolete
*/

/*!
    \fn void QWidget::styleChange(QStyle& style)

    \internal
    \obsolete
*/

/*!
    Disables widget input events if \a disable is true; otherwise
    enables input events.

    See the \l enabled documentation for more information.

    \sa isEnabledTo(), QKeyEvent, QMouseEvent, changeEvent()
*/
void QWidget::setDisabled(bool disable)
{
    setEnabled(!disable);
}

/*!
    \property QWidget::frameGeometry
    \brief geometry of the widget relative to its parent including any
    window frame

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of geometry issues with windows.

    \sa geometry() x() y() pos()
*/
QRect QWidget::frameGeometry() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup)) {
        if (data->fstrut_dirty)
            d->updateFrameStrut();
        QTLWExtra *top = d->topData();
        return QRect(data->crect.x() - top->fleft,
                     data->crect.y() - top->ftop,
                     data->crect.width() + top->fleft + top->fright,
                     data->crect.height() + top->ftop + top->fbottom);
    }
    return data->crect;
}

/*!
    \property QWidget::x

    \brief the x coordinate of the widget relative to its parent including
    any window frame

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of window geometry.

    \sa frameGeometry, y, pos
*/
int QWidget::x() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup)) {
        if (data->fstrut_dirty)
            d->updateFrameStrut();
        return data->crect.x() - d->topData()->fleft;
    }
    return data->crect.x();
}

/*!
    \property QWidget::y
    \brief the y coordinate of the widget relative to its parent and
    including any window frame

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of window geometry.

    \sa frameGeometry, x, pos
*/
int QWidget::y() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup)) {
        if (data->fstrut_dirty)
            d->updateFrameStrut();
        return data->crect.y() - d->topData()->ftop;
    }
    return data->crect.y();
}

/*!
    \property QWidget::pos
    \brief the position of the widget within its parent widget

    If the widget is a window, the position is that of the widget on
    the desktop, including its frame.

    When changing the position, the widget, if visible, receives a
    move event (moveEvent()) immediately. If the widget is not
    currently visible, it is guaranteed to receive an event before it
    is shown.

    move() is virtual, and all other overloaded move() implementations
    in Qt call it.

    \warning Calling move() or setGeometry() inside moveEvent() can
    lead to infinite recursion.

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of window geometry.

    \sa frameGeometry, size x(), y()
*/
QPoint QWidget::pos() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup)) {
        if (data->fstrut_dirty)
            d->updateFrameStrut();
        QTLWExtra *top = d->topData();
        return QPoint(data->crect.x() - top->fleft, data->crect.y() - top->ftop);
    }
    return data->crect.topLeft();
}

/*!
    \property QWidget::geometry
    \brief the geometry of the widget relative to its parent and
    excluding the window frame

    When changing the geometry, the widget, if visible, receives a
    move event (moveEvent()) and/or a resize event (resizeEvent())
    immediately. If the widget is not currently visible, it is
    guaranteed to receive appropriate events before it is shown.

    The size component is adjusted if it lies outside the range
    defined by minimumSize() and maximumSize().

    \warning Calling setGeometry() inside resizeEvent() or moveEvent()
    can lead to infinite recursion.

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of window geometry.

    \sa frameGeometry(), rect(), move(), resize(), moveEvent(),
        resizeEvent(), minimumSize(), maximumSize()
*/

/*!
    \property QWidget::normalGeometry
    \brief the geometry a toplevel widget has when it is not maximized or fullscreen

    For child widgets this property always holds an empty rect.

    \sa windowState, geometry
*/

/*!
    \property QWidget::size
    \brief the size of the widget excluding any window frame

    If the widget is visible when it is being resized, it receives a resize event
    (resizeEvent()) immediately. If the widget is not currently
    visible, it is guaranteed to receive an event before it is shown.

    The size is adjusted if it lies outside the range defined by
    minimumSize() and maximumSize(). For windows, the minimum size
    is always at least QSize(1, 1), and it might be larger, depending on
    the window manager.

    \warning Calling resize() or setGeometry() inside resizeEvent() can
    lead to infinite recursion.

    \sa pos, geometry, minimumSize, maximumSize, resizeEvent()
*/

/*!
    \property QWidget::width
    \brief the width of the widget excluding any window frame

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of window geometry.

    \sa geometry, height, size
*/

/*!
    \property QWidget::height
    \brief the height of the widget excluding any window frame

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of window geometry.

    \sa geometry, width, size
*/

/*!
    \property QWidget::rect
    \brief the internal geometry of the widget excluding any window
    frame

    The rect property equals QRect(0, 0, width(), height()).

    See the \link geometry.html Window Geometry documentation\endlink
    for an overview of window geometry.

    \sa size
*/


QRect QWidget::normalGeometry() const
{
    Q_D(const QWidget);
    if (!d->extra || !d->extra->topextra)
        return QRect();

    if (!isMaximized() && !isFullScreen())
        return geometry();

    return d->topData()->normalGeometry;
}


/*!
    \property QWidget::childrenRect
    \brief the bounding rectangle of the widget's children

    Hidden children are excluded.

    \sa childrenRegion() geometry()
*/

QRect QWidget::childrenRect() const
{
    Q_D(const QWidget);
    QRect r(0, 0, 0, 0);
    for (int i = 0; i < d->children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(d->children.at(i));
        if (w && !w->isExplicitlyHidden())
            r |= w->geometry();
    }
    return r;
}

/*!
    \property QWidget::childrenRegion
    \brief the combined region occupied by the widget's children

    Hidden children are excluded.

    \sa childrenRect() geometry()
*/

QRegion QWidget::childrenRegion() const
{
    Q_D(const QWidget);
    QRegion r;
    for (int i = 0; i < d->children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(d->children.at(i));
        if (w && !w->isExplicitlyHidden()) {
            QRegion mask = w->mask();
            if (mask.isEmpty())
                r |= w->geometry();
            else
                r |= mask;
        }
    }
    return r;
}


/*!
    \property QWidget::minimumSize
    \brief the widget's minimum size

    The widget cannot be resized to a smaller size than the minimum
    widget size. The widget's size is forced to the minimum size if
    the current size is smaller.

    If the widget has a layout, the minimum size will be set by the
    layout and not by setMinimumSize(). If you want to set the minimum
    size yourself, you must first call QLayout::setResizeMode(QLayout::Fixed).

    \sa minimumWidth, minimumHeight, maximumSize, sizeIncrement,
    QLayout::resizeMode
*/

QSize QWidget::minimumSize() const
{
    Q_D(const QWidget);
    return d->extra ? QSize(d->extra->minw, d->extra->minh) : QSize(0, 0);
}

/*!
    \property QWidget::maximumSize
    \brief the widget's maximum size

    The widget cannot be resized to a larger size than the maximum
    widget size.

    If the widget has a layout, the maximum size will be set by the
    layout and not by setMaximumSize(). If you want to set the maximum
    size yourself, you must first call QLayout::setResizeMode(QLayout::Fixed).

    \sa maximumWidth, maximumHeight, minimumSize, sizeIncrement,
    QLayout::resizeMode
*/

QSize QWidget::maximumSize() const
{
    Q_D(const QWidget);
    return d->extra ? QSize(d->extra->maxw, d->extra->maxh)
                 : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}


/*!
    \property QWidget::minimumWidth
    \brief the widget's minimum width

    This property corresponds to minimumSize().width().

    \sa minimumSize, minimumHeight
*/

/*!
    \property QWidget::minimumHeight
    \brief the widget's minimum height

    This property corresponds to minimumSize().height().

    \sa minimumSize, minimumWidth
*/

/*!
    \property QWidget::maximumWidth
    \brief the widget's maximum width

    This property corresponds to maximumSize().width().

    \sa maximumSize, maximumHeight
*/

/*!
    \property QWidget::maximumHeight
    \brief the widget's maximum height

    This property corresponds to maximumSize().height().

    \sa maximumSize, maximumWidth
*/

/*!
    \property QWidget::sizeIncrement
    \brief the size increment of the widget

    When the user resizes the window, the size will move in steps of
    sizeIncrement().width() pixels horizontally and
    sizeIncrement.height() pixels vertically, with baseSize() as the
    basis. Preferred widget sizes are for non-negative integers \e i
    and \e j:
    \code
        width = baseSize().width() + i * sizeIncrement().width();
        height = baseSize().height() + j * sizeIncrement().height();
    \endcode

    Note that while you can set the size increment for all widgets, it
    only affects windows.

    \warning The size increment has no effect under Windows, and may
    be disregarded by the window manager on X.

    \sa size, minimumSize, maximumSize
*/
QSize QWidget::sizeIncrement() const
{
    Q_D(const QWidget);
    return (d->extra && d->extra->topextra)
        ? QSize(d->extra->topextra->incw, d->extra->topextra->inch)
        : QSize(0, 0);
}

/*!
    \property QWidget::baseSize
    \brief the base size of the widget

    The base size is used to calculate a proper widget size if the
    widget defines sizeIncrement().

    \sa setSizeIncrement()
*/

QSize QWidget::baseSize() const
{
    Q_D(const QWidget);
    return (d->extra != 0 && d->extra->topextra != 0)
        ? QSize(d->extra->topextra->basew, d->extra->topextra->baseh)
        : QSize(0, 0);
}

/*!
    Sets both the minimum and maximum sizes of the widget to \a s,
    thereby preventing it from ever growing or shrinking.


    If the widget has a layout, the minimum and maximum sizes will automatically be set by the
    layout. If you want to set the minimum
    size yourself, you must first call QLayout::setResizeMode(QLayout::Fixed).


    If the widget has a layout, the layout automatically sets the
    widget's minimum and maximum sizes based on its contents. If you
    want to manually set a fixed size, you must call
    QLayout::setResizeMode(QLayout::FreeResize) before calling
    setFixedSize(). Alternatively, if you want the dialog to have a
    fixed size based on its contents, call
    QLayout::setResizeMode(QLayout::Fixed).

    \sa maximumSize, minimumSize, QLayout::resizeMode
*/

void QWidget::setFixedSize(const QSize & s)
{
    setMinimumSize(s);
    setMaximumSize(s);
    resize(s);
}


/*!
    \fn void QWidget::setFixedSize(int w, int h)
    \overload

    Sets the width of the widget to \a w and the height to \a h.
*/

void QWidget::setFixedSize(int w, int h)
{
    setMinimumSize(w, h);
    setMaximumSize(w, h);
    resize(w, h);
}

void QWidget::setMinimumWidth(int w)
{
    setMinimumSize(w, minimumSize().height());
}

void QWidget::setMinimumHeight(int h)
{
    setMinimumSize(minimumSize().width(), h);
}

void QWidget::setMaximumWidth(int w)
{
    setMaximumSize(w, maximumSize().height());
}

void QWidget::setMaximumHeight(int h)
{
    setMaximumSize(maximumSize().width(), h);
}

/*!
    Sets both the minimum and maximum width of the widget to \a w
    without changing the heights. Provided for convenience.

    \sa sizeHint() minimumSize() maximumSize() setFixedSize()
*/

void QWidget::setFixedWidth(int w)
{
    setMinimumSize(w, minimumSize().height());
    setMaximumSize(w, maximumSize().height());
}


/*!
    Sets both the minimum and maximum heights of the widget to \a h
    without changing the widths. Provided for convenience.

    \sa sizeHint() minimumSize() maximumSize() setFixedSize()
*/

void QWidget::setFixedHeight(int h)
{
    setMinimumSize(minimumSize().width(), h);
    setMaximumSize(maximumSize().width(), h);
}


/*!
    Translates the widget coordinate \a pos to the coordinate system
    of \a parent. The \a parent must not be 0 and must be a parent
    of the calling widget.

    \sa mapFrom() mapToParent() mapToGlobal() underMouse()
*/

QPoint QWidget::mapTo(QWidget * parent, const QPoint & pos) const
{
    QPoint p = pos;
    if (parent) {
        const QWidget * w = this;
        while (w != parent) {
            p = w->mapToParent(p);
            w = w->parentWidget();
        }
    }
    return p;
}


/*!
    Translates the widget coordinate \a pos from the coordinate system
    of \a parent to this widget's coordinate system. The \a parent
    must not be 0 and must be a parent of the calling widget.

    \sa mapTo() mapFromParent() mapFromGlobal() underMouse()
*/

QPoint QWidget::mapFrom(QWidget * parent, const QPoint & pos) const
{
    QPoint p(pos);
    if (parent) {
        const QWidget * w = this;
        while (w != parent) {
            p = w->mapFromParent(p);
            w = w->parentWidget();
        }
    }
    return p;
}


/*!
    Translates the widget coordinate \a pos to a coordinate in the
    parent widget.

    Same as mapToGlobal() if the widget has no parent.

    \sa mapFromParent() mapTo() mapToGlobal() underMouse()
*/

QPoint QWidget::mapToParent(const QPoint &pos) const
{
    return pos + data->crect.topLeft();
}

/*!
    Translates the parent widget coordinate \a pos to widget
    coordinates.

    Same as mapFromGlobal() if the widget has no parent.

    \sa mapToParent() mapFrom() mapFromGlobal() underMouse()
*/

QPoint QWidget::mapFromParent(const QPoint &pos) const
{
    return pos - data->crect.topLeft();
}


/*!
    Returns the window for this widget, i.e. the next ancestor widget
    that has (or could have) a window-system frame.

    If the widget is a window, the widget itself is returned.

    Typical usage is changing the window title:

    \code
        aWidget->window()->setWindowTitle("New Window Title");
    \endcode

    \sa isWindow()
*/

QWidget *QWidget::window() const
{
    QWidget *w = (QWidget *)this;
    QWidget *p = w->parentWidget();
    while (!w->isWindow() && p) {
        w = p;
        p = p->parentWidget();
    }
    return w;
}

/*! \fn QWidget *QWidget::topLevelWidget() const
    \obsolete

    Use window() instead.
*/

#ifdef QT3_SUPPORT
/*!
    Returns the color role used for painting the widget's background.

    Use QPalette(backgroundRole(()) instead.
*/
Qt::BackgroundMode QWidget::backgroundMode() const
{
    if (testAttribute(Qt::WA_NoSystemBackground))
        return Qt::NoBackground;
    switch(backgroundRole()) {
    case QPalette::Foreground:
        return Qt::PaletteForeground;
    case QPalette::Button:
        return Qt::PaletteButton;
    case QPalette::Light:
        return Qt::PaletteLight;
    case QPalette::Midlight:
        return Qt::PaletteMidlight;
    case QPalette::Dark:
        return Qt::PaletteDark;
    case QPalette::Mid:
        return Qt::PaletteMid;
    case QPalette::Text:
        return Qt::PaletteText;
    case QPalette::BrightText:
        return Qt::PaletteBrightText;
    case QPalette::Base:
        return Qt::PaletteBase;
    case QPalette::Background:
        return Qt::PaletteBackground;
    case QPalette::Shadow:
        return Qt::PaletteShadow;
    case QPalette::Highlight:
        return Qt::PaletteHighlight;
    case QPalette::HighlightedText:
        return Qt::PaletteHighlightedText;
    case QPalette::ButtonText:
        return Qt::PaletteButtonText;
    case QPalette::Link:
        return Qt::PaletteLink;
    case QPalette::LinkVisited:
        return Qt::PaletteLinkVisited;
    default:
        break;
    }
    return Qt::NoBackground;
}

/*!
    \fn void QWidget::setBackgroundMode(Qt::BackgroundMode
    widgetBackground, Qt::BackgroundMode paletteBackground)

    Sets the color role used for painting the widget's background to
    background mode \a widgetBackground. The \a paletteBackground mode
    parameter is ignored.
*/
void QWidget::setBackgroundMode(Qt::BackgroundMode m, Qt::BackgroundMode)
{
    Q_D(QWidget);
    if(m == Qt::NoBackground) {
        setAttribute(Qt::WA_NoSystemBackground, true);
        return;
    }
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAttribute(Qt::WA_SetForegroundRole, false);
    QPalette::ColorRole role = d->bg_role;;
    switch(m) {
    case Qt::FixedColor:
    case Qt::FixedPixmap:
        break;
    case Qt::PaletteForeground:
        role = QPalette::Foreground;
        break;
    case Qt::PaletteButton:
        role = QPalette::Button;
        break;
    case Qt::PaletteLight:
        role = QPalette::Light;
        break;
    case Qt::PaletteMidlight:
        role = QPalette::Midlight;
        break;
    case Qt::PaletteDark:
        role = QPalette::Dark;
        break;
    case Qt::PaletteMid:
        role = QPalette::Mid;
        break;
    case Qt::PaletteText:
        role = QPalette::Text;
        break;
    case Qt::PaletteBrightText:
        role = QPalette::BrightText;
        break;
    case Qt::PaletteBase:
        role = QPalette::Base;
        break;
    case Qt::PaletteBackground:
        role = QPalette::Background;
        break;
    case Qt::PaletteShadow:
        role = QPalette::Shadow;
        break;
    case Qt::PaletteHighlight:
        role = QPalette::Highlight;
        break;
    case Qt::PaletteHighlightedText:
        role = QPalette::HighlightedText;
        break;
    case Qt::PaletteButtonText:
        role = QPalette::ButtonText;
        break;
    case Qt::PaletteLink:
        role = QPalette::Link;
        break;
    case Qt::PaletteLinkVisited:
        role = QPalette::LinkVisited;
        break;
    case Qt::X11ParentRelative:
        d->fg_role = QPalette::Foreground;
        d->bg_role = QPalette::Background;
        setAttribute(Qt::WA_SetBackgroundRole, false);
    default:
        break;
    }
    setBackgroundRole(role);
}

/*!
    The widget mapper is no longer part of the public API.
*/
QT3_SUPPORT QWidgetMapper *QWidget::wmapper() { return QWidgetPrivate::mapper; }

#endif


/*!
  Returns the background role of the widget.

  The background role defines the brush from the widget's \l palette that
  is used to render the background.

  \sa setBackgroundRole(), foregroundRole()
 */
QPalette::ColorRole QWidget::backgroundRole() const
{
    const QWidget *w = this;

    while (!w->isWindow() && w->windowType() != Qt::SubWindow
           && !w->testAttribute(Qt::WA_SetBackgroundRole))
        w = w->parentWidget();
    return w->d_func()->bg_role;
}

/*!
  Sets the background role of the widget to \a role.

  The background role defines the brush from the widget's \l palette that
  is used to render the background.

  \sa backgroundRole(), foregroundRole()
 */

void QWidget::setBackgroundRole(QPalette::ColorRole role)
{
    Q_D(QWidget);
    QWidgetPrivate *dd = d; //### workaround for gcc 3.3.x compiler bug
    dd->bg_role = role;
    setAttribute(Qt::WA_SetBackgroundRole);
    if (!testAttribute(Qt::WA_SetForegroundRole))
        switch (role) {
        case QPalette::Button:
            dd->fg_role = QPalette::ButtonText;
            break;
        case QPalette::Base:
            dd->fg_role = QPalette::Text;
            break;
        case QPalette::Dark:
        case QPalette::Shadow:
            dd->fg_role = QPalette::Light;
            break;
        case QPalette::Highlight:
            dd->fg_role = QPalette::HighlightedText;
            break;
        default:
            dd->fg_role = QPalette::Foreground;
            break;
        }
    dd->updateSystemBackground();
    dd->propagatePaletteChange();
}

/*!
  Returns the foreground role.

  The foreground role defines the color from the widget's \l palette that
  is used to draw the foreground.

  If no explicit foreground role is set, returns a role that contrasts
  with the backgroundRole().

  \sa setForegroundRole(), backgroundRole()
 */
QPalette::ColorRole QWidget::foregroundRole() const
{
    const QWidget *w = this;
    while (!w->isWindow() && w->windowType() != Qt::SubWindow
           && (w->testAttribute(Qt::WA_ForegroundInherited)
               || (!w->testAttribute(Qt::WA_SetBackgroundRole)
                   && !w->testAttribute(Qt::WA_SetForegroundRole))))
        w = w->parentWidget();
    return w->d_func()->fg_role;
}

/*!
  Sets the foreground role of the widget to \a role.

  The foreground role defines the color from the widget's \l palette that
  is used to draw the foreground.

  \sa backgroundRole(), setForegroundRole()
 */
void QWidget::setForegroundRole(QPalette::ColorRole role)
{
    Q_D(QWidget);
    d->bg_role = role;
    setAttribute(Qt::WA_SetForegroundRole);
    d->updateSystemBackground();
    d->propagatePaletteChange();
}

/*!
    \property QWidget::palette
    \brief the widget's palette

    As long as no special palette has been set, this is either a
    special palette for the widget class, the parent's palette or (if
    this widget is a top level widget), the default application
    palette.

    \sa QApplication::palette()
*/
const QPalette &QWidget::palette() const
{
    if (!isEnabled()) {
        data->pal.setCurrentColorGroup(QPalette::Disabled);
    } else if (!isVisible() || isActiveWindow()) {
        data->pal.setCurrentColorGroup(QPalette::Active);
    } else {
#ifdef Q_WS_MAC
        extern bool qt_mac_can_clickThrough(const QWidget *); //qwidget_mac.cpp
        if (qt_mac_can_clickThrough(this))
            data->pal.setCurrentColorGroup(QPalette::Active);
        else
#endif
            data->pal.setCurrentColorGroup(QPalette::Inactive);
    }
    return data->pal;
}

void QWidget::setPalette(const QPalette &palette)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_SetPalette, palette.resolve() != 0);
    d->setPalette_helper(palette.resolve(qt_naturalWidgetPalette(this)));
}

void QWidgetPrivate::resolvePalette()
{
    Q_Q(QWidget);
    setPalette_helper(data.pal.resolve(qt_naturalWidgetPalette(q)));
}

void QWidgetPrivate::setPalette_helper(const QPalette &palette)
{
    if (data.pal == palette && data.pal.resolve() == palette.resolve())
        return;
    data.pal = palette;
    updateSystemBackground();
    propagatePaletteChange();
}


/*!
    \property QWidget::font
    \brief the font currently set for the widget

    The fontInfo() function reports the actual font that is being used
    by the widget.

    As long as no special font has been set, or after setFont(QFont())
    is called, this is either a special font for the widget class, the
    parent's font or (if this widget is a top level widget), the
    default application font.

    This code fragment sets a 12 point helvetica bold font:
    \code
    QFont f("Helvetica", 12, QFont::Bold);
    setFont(f);
    \endcode

    In addition to setting the font, setFont() informs all children
    about the change.

    \sa fontInfo() fontMetrics()
*/

void QWidget::setFont(const QFont &font)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_SetFont, font.resolve() != 0);
    d->setFont_helper(font);
}

void QWidgetPrivate::resolveFont()
{
    Q_Q(QWidget);
    setFont_helper(data.fnt.resolve(qt_naturalWidgetFont(q)));
}

void QWidgetPrivate::setFont_helper(const QFont &font)
{
    if (data.fnt == font && data.fnt.resolve() == font.resolve())
        return;

    Q_Q(QWidget);
#ifdef QT3_SUPPORT
    QFont old = data.fnt;
#endif
    data.fnt = font;
#if defined(Q_WS_X11)
    // make sure the font set on this widget is associated with the correct screen
    data.fnt.x11SetScreen(xinfo.screen());
#endif
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget*>(children.at(i));
        if (w && !w->isWindow())
            w->d_func()->resolveFont();
    }
    QEvent e(QEvent::FontChange);
    QApplication::sendEvent(q, &e);
#ifdef QT3_SUPPORT
    q->fontChange(old);
#endif
}

void QWidgetPrivate::setLayoutDirection_helper(Qt::LayoutDirection direction)
{
    Q_Q(QWidget);

    if ( (direction == Qt::RightToLeft) == q->testAttribute(Qt::WA_RightToLeft))
        return;
    q->setAttribute(Qt::WA_RightToLeft,  (direction == Qt::RightToLeft));
    if (!children.isEmpty()) {
        for (int i = 0; i < children.size(); ++i) {
            QWidget *w = qobject_cast<QWidget*>(children.at(i));
            if (w && !w->isWindow() && !w->testAttribute(Qt::WA_SetLayoutDirection))
                w->d_func()->setLayoutDirection_helper(direction);
        }
    }
    QEvent e(QEvent::LayoutDirectionChange);
    QApplication::sendEvent(q, &e);
}

void QWidgetPrivate::resolveLayoutDirection()
{
    Q_Q(const QWidget);
    if (!q->testAttribute(Qt::WA_SetLayoutDirection))
        setLayoutDirection_helper(q->isWindow() ? QApplication::layoutDirection() : q->parentWidget()->layoutDirection());
}

/*!\property QWidget::layoutDirection

   \brief the layout direction for this widget

   \sa QApplication::layoutDirection
 */
void QWidget::setLayoutDirection(Qt::LayoutDirection direction)
{
    Q_D(QWidget);

    setAttribute(Qt::WA_SetLayoutDirection);
    d->setLayoutDirection_helper(direction);
}

Qt::LayoutDirection QWidget::layoutDirection() const
{
    return testAttribute(Qt::WA_RightToLeft) ? Qt::RightToLeft : Qt::LeftToRight;
}

void QWidget::unsetLayoutDirection()
{
    Q_D(QWidget);
    setAttribute(Qt::WA_SetLayoutDirection, false);
    d->resolveLayoutDirection();
}

/*!
    \fn QFontMetrics QWidget::fontMetrics() const

    Returns the font metrics for the widget's current font.
    Equivalent to QFontMetrics(widget->font()).

    \sa font(), fontInfo(), setFont()
*/

/*!
    \fn QFontInfo QWidget::fontInfo() const

    Returns the font info for the widget's current font.
    Equivalent to QFontInto(widget->font()).

    \sa font(), fontMetrics(), setFont()
*/


/*!
    \property QWidget::cursor
    \brief the cursor shape for this widget

    The mouse cursor will assume this shape when it's over this
    widget. See the \link Qt::CursorShape list of predefined cursor
    objects\endlink for a range of useful shapes.

    An editor widget might use an I-beam cursor:
    \code
        setCursor(Qt::IBeamCursor);
    \endcode

    If no cursor has been set, or after a call to unsetCursor(), the
    parent's cursor is used. The function unsetCursor() has no effect
    on windows.

    \sa QApplication::setOverrideCursor()
*/

#ifndef QT_NO_CURSOR
QCursor QWidget::cursor() const
{
    Q_D(const QWidget);
    if (testAttribute(Qt::WA_SetCursor))
        return (d->extra && d->extra->curs)
            ? *d->extra->curs
            : QCursor(Qt::ArrowCursor);
    if (isWindow() || !parentWidget())
        return QCursor(Qt::ArrowCursor);
    return parentWidget()->cursor();
}
#endif
/*!
    \property QWidget::windowTitle
    \brief the window title (caption)

    This property only makes sense for windows. If no
    caption has been set, the title is an empty string.

    \sa windowIcon windowIconText
*/
QString QWidget::windowTitle() const
{
    Q_D(const QWidget);
    return d->extra && d->extra->topextra
        ? d->extra->topextra->caption
        : QString();
}

/*!
    \property QWidget::windowIcon
    \brief the widget's icon

    This property only makes sense for windows. If no icon
    has been set, windowIcon() returns the application icon.

    \sa windowIconText, windowTitle \link appicon.html Setting the Application Icon\endlink
*/
QIcon QWidget::windowIcon() const
{
    const QWidget *w = this;
    while (w) {
        const QWidgetPrivate *d = w->d_func();
        if (d->extra && d->extra->topextra && d->extra->topextra->icon)
            return *d->extra->topextra->icon;
        w = w->parentWidget();
    }
    return qApp->windowIcon();
}

void QWidget::setWindowIcon(const QIcon &icon)
{
    Q_D(QWidget);

    setAttribute(Qt::WA_SetWindowIcon, !icon.isNull());
    d->createTLExtra();

    if (!d->extra->topextra->icon)
        d->extra->topextra->icon = new QIcon();
    *d->extra->topextra->icon = icon;

    delete d->extra->topextra->iconPixmap;
    d->extra->topextra->iconPixmap = 0;

    d->setWindowIcon_sys();
    QEvent e(QEvent::WindowIconChange);
    QApplication::sendEvent(this, &e);
}


/*!
    \property QWidget::windowIconText
    \brief the widget's icon text

    This property only makes sense for windows. If no icon
    text has been set, this functions returns an empty string.

    \sa windowIcon, windowTitle
*/

QString QWidget::windowIconText() const
{
    Q_D(const QWidget);
    return (d->extra && d->extra->topextra) ? d->extra->topextra->iconText : QString();
}

/*!
    Returns the window's role, or an empty string.

    \sa windowIcon, windowTitle
*/

QString QWidget::windowRole() const
{
    Q_D(const QWidget);
    return (d->extra && d->extra->topextra) ? d->extra->topextra->role : QString();
}

/*!
    Sets the window's role to \a role. This only makes sense for
    windows on X11.
*/
void QWidget::setWindowRole(const QString &role)
{
#if defined(Q_WS_X11)
    Q_D(QWidget);
    d->topData()->role = role;
    d->setWindowRole(role.toUtf8().constData());
#else
    Q_UNUSED(role)
#endif
}

/*!
    \property QWidget::mouseTracking
    \brief whether mouse tracking is enabled for the widget

    If mouse tracking is disabled (the default), the widget only
    receives mouse move events when at least one mouse button is
    pressed while the mouse is being moved.

    If mouse tracking is enabled, the widget receives mouse move
    events even if no buttons are pressed.

    \sa mouseMoveEvent()
*/


/*!
    Sets the widget's focus proxy to widget \a w. If \a w is 0, the
    function resets this widget to have no focus proxy.

    Some widgets can "have focus", but create a child widget, such as
    QLineEdit, to actually handle the focus. In this case, the widget
    can set the line edit to be its focus proxy.

    setFocusProxy() sets the widget which will actually get focus when
    "this widget" gets it. If there is a focus proxy, setFocus() and
    hasFocus() operate on the focus proxy.

    \sa focusProxy()
*/

void QWidget::setFocusProxy(QWidget * w)
{
    Q_D(QWidget);
    if (!w && !d->extra)
        return;

    for (QWidget* fp  = w; fp; fp = fp->focusProxy()) {
        if (fp == this) {
            qWarning("%s (%s): already in focus proxy chain", metaObject()->className(), objectName().toLocal8Bit().constData());
            return;
        }
    }

    d->createExtra();
    d->extra->focus_proxy = w;
}


/*!
    Returns the focus proxy, or 0 if there is no focus proxy.

    \sa setFocusProxy()
*/

QWidget * QWidget::focusProxy() const
{
    Q_D(const QWidget);
    return d->extra ? (QWidget *)d->extra->focus_proxy : 0;
}


/*!
    \property QWidget::focus
    \brief whether this widget (or its focus proxy) has the keyboard
    input focus

    Effectively equivalent to \c {QApplication::focusWidget() == this}.

    \sa setFocus(), clearFocus(), setFocusPolicy(), QApplication::focusWidget()
*/
bool QWidget::hasFocus() const
{
    const QWidget* w = this;
    while (w->d_func()->extra && w->d_func()->extra->focus_proxy)
        w = w->d_func()->extra->focus_proxy;
    return (QApplication::focusWidget() == w);
}

/*!
    Gives the keyboard input focus to this widget (or its focus
    proxy) if this widget or one of its parents is the \link
    isActiveWindow() active window\endlink. The \a reason argument will
    be passed into any focus event sent from this function, it is used
    to give an explanation of what caused the widget to get focus.

    First, a focus out event is sent to the focus widget (if any) to
    tell it that it is about to lose the focus. Then a focus in event
    is sent to this widget to tell it that it just received the focus.
    (Nothing happens if the focus in and focus out widgets are the
    same.)

    setFocus() gives focus to a widget regardless of its focus policy,
    but does not clear any keyboard grab (see grabKeyboard()).

    Be aware that if the widget is hidden, it will not accept focus.

    \warning If you call setFocus() in a function which may itself be
    called from focusOutEvent() or focusInEvent(), you may get an
    infinite recursion.

    \sa hasFocus() clearFocus() focusInEvent() focusOutEvent()
    setFocusPolicy() QApplication::focusWidget() grabKeyboard()
    grabMouse()
*/

void QWidget::setFocus(Qt::FocusReason reason)
{
    if (!isEnabled())
        return;

    QWidget *f = this;
    while (f->d_func()->extra && f->d_func()->extra->focus_proxy)
        f = f->d_func()->extra->focus_proxy;

    if (QApplication::focusWidget() == f
#if defined(Q_WS_WIN)
        && GetFocus() == f->winId()
#endif
       )
        return;


    QWidget *w = f;
    if (isExplicitlyHidden()) {
        while (w && w->isExplicitlyHidden()) {
            w->d_func()->focus_child = f;
            w = w->isWindow() ? 0 : w->parentWidget();
        }
    } else {
        while (w) {
            w->d_func()->focus_child = f;
            w = w->isWindow() ? 0 : w->parentWidget();
        }
    }

    if (f->isActiveWindow()) {
        QWidget *prev = QApplication::focusWidget();
        if (prev) {
	    // This part is never executed when Q_WS_X11? Preceding XFocusOut
	    // had already reset focus_widget when received XFocusIn

	    // Don't reset input context explicitly here. Whether reset or not
	    // when focusing out is a responsibility of input methods. So we
	    // delegate the responsibility to input context via
	    // unfocusInputContext(). See 'Preedit preservation' section of
	    // QInputContext.
            if (prev != f) {
#if !defined(Q_WS_MAC)
		prev->d_func()->unfocusInputContext();
#else
		prev->resetInputContext();
#endif
	    }
        }

        QApplicationPrivate::setFocusWidget(f, reason);
        f->d_func()->focusInputContext();

#if defined(Q_WS_WIN)
        if (!(f->window()->windowType() == Qt::Popup))
            SetFocus(f->winId());
        else {
#endif
#ifndef QT_NO_ACCESSIBILITY
            QAccessible::updateAccessibility(f, 0, QAccessible::Focus);
#endif
#if defined(Q_WS_WIN)
        }
#endif
    }
}

/*!
    Takes keyboard input focus from the widget.

    If the widget has active focus, a \link focusOutEvent() focus out
    event\endlink is sent to this widget to tell it that it is about
    to lose the focus.

    This widget must enable focus setting in order to get the keyboard
    input focus, i.e. it must call setFocusPolicy().

    \sa hasFocus(), setFocus(), focusInEvent(), focusOutEvent(),
    setFocusPolicy(), QApplication::focusWidget()
*/

void QWidget::clearFocus()
{
    QWidget *w = this;
    while (w && w->d_func()->focus_child == this) {
        w->d_func()->focus_child = 0;
        w = w->isWindow() ? 0 : w->parentWidget();
    }
    if (hasFocus()) {
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
        Q_D(QWidget);
	d->unfocusInputContext();
#endif
        QApplicationPrivate::setFocusWidget(0, Qt::OtherFocusReason);
#if defined(Q_WS_WIN)
        if (!(windowType() == Qt::Popup) && GetFocus() == winId())
            SetFocus(0);
        else {
#endif
#ifndef QT_NO_ACCESSIBILITY
            QAccessible::updateAccessibility(this, 0, QAccessible::Focus);
#endif
#if defined(Q_WS_WIN)
        }
#endif
    }
}


/*!
    Finds a new widget to give the keyboard focus to, as appropriate
    for Tab and Shift+Tab, and returns true if it can find a new
    widget, or false if it can't.

    If \a next is true, this function searches "forwards", if \a next
    is false, it searches "backwards".

    Sometimes, you will want to reimplement this function. For
    example, a web browser might reimplement it to move its "current
    active link" forwards or backwards, and call
    QWidget::focusNextPrevChild() only when it reaches the last or
    first link on the "page".

    Child widgets call focusNextPrevChild() on their parent widgets,
    but only the window that contains the child widgets decides where
    to redirect focus. By reimplementing this function for an object,
    you thus gain control of focus traversal for all child widgets.
*/

bool QWidget::focusNextPrevChild(bool next)
{
    QWidget* p = parentWidget();
    if (!isWindow() && p)
        return p->focusNextPrevChild(next);

    extern bool qt_tab_all_widgets;
    uint focus_flag = qt_tab_all_widgets ? Qt::TabFocus : Qt::StrongFocus;

    QWidget *f = focusWidget();
    if (!f)
        f = this;

    QWidget *w = f;
    QWidget *test = f->d_func()->focus_next;
    while (test && test != f) {
        if ((test->focusPolicy() & focus_flag) == focus_flag
            && !(test->d_func()->extra && test->d_func()->extra->focus_proxy)
            && test->isVisibleTo(this) && test->isEnabled()) {
            w = test;
            if (next)
                break;
        }
        test = test->d_func()->focus_next;
    }
    if (w == f) {
        extern bool qt_in_tab_key_event; // defined in qapplication.cpp
        if (qt_in_tab_key_event) {
            w->window()->setAttribute(Qt::WA_KeyboardFocusChange);
            w->update();
        }
        return false;
    }
    w->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
    return true;
}

/*!
    Returns the last child of this widget that setFocus had been
    called on.  For top level widgets this is the widget that will get
    focus in case this window gets activated

    This is not the same as QApplication::focusWidget(), which returns
    the focus widget in the currently active window.
*/

QWidget *QWidget::focusWidget() const
{
    return const_cast<QWidget *>(d_func()->focus_child);
}

/*!
    Returns the next widget in this widget's focus chain.
*/
QWidget *QWidget::nextInFocusChain() const
{
    return const_cast<QWidget *>(d_func()->focus_next);
}

/*!
    \property QWidget::isActiveWindow
    \brief whether this widget's window is the active window

    The active window is the window that contains the widget that
    has keyboard focus.

    When popup windows are visible, this property is true for both the
    active window \e and for the popup.

    \sa activateWindow(), QApplication::activeWindow()
*/
bool QWidget::isActiveWindow() const
{
    QWidget *tlw = window();
    if((windowType() == Qt::SubWindow) && parentWidget())
        tlw = parentWidget()->window();
    if(tlw == qApp->activeWindow() || (isVisible() && (tlw->windowType() == Qt::Popup)))
        return true;
#ifdef Q_WS_MAC
    { //check process
        Boolean compare;
        ProcessSerialNumber current, front;
        GetCurrentProcess(&current);
        GetFrontProcess(&front);
        if(SameProcess(&current, &front, &compare) == noErr && !compare)
            return false;
    }
    extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
    if(qt_mac_is_macdrawer(tlw) &&
       tlw->parentWidget() && tlw->parentWidget()->isActiveWindow())
        return true;
#endif
    if(style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, this)) {
        if(((tlw->windowType() == Qt::Dialog) || (tlw->windowType() == Qt::Tool)) &&
           !tlw->testAttribute(Qt::WA_ShowModal) &&
           (!tlw->parentWidget() || tlw->parentWidget()->isActiveWindow()))
           return true;
        QWidget *w = qApp->activeWindow();
        if(!(windowType() == Qt::SubWindow) && w && (w->windowType() == Qt::SubWindow) &&
            w->parentWidget()->window() == tlw)
            return true;
        while(w && ((tlw->windowType() == Qt::Dialog) || (tlw->windowType() == Qt::Tool)) &&
              !w->testAttribute(Qt::WA_ShowModal) && w->parentWidget()) {
            w = w->parentWidget()->window();
            if(w == tlw)
                return true;
        }
    }
#if defined(Q_WS_WIN32)
    HWND parent = tlw->winId();
    HWND topparent = GetActiveWindow();
    while (parent) {
        parent = ::GetParent(parent);
        if (parent && parent == topparent)
            return true;
    }
#endif

    return false;
}

/*!
    Moves the \a second widget around the ring of focus widgets so
    that keyboard focus moves from the \a first widget to the \a
    second widget when the Tab key is pressed.

    Note that since the tab order of the \a second widget is changed,
    you should order a chain like this:

    \code
        setTabOrder(a, b); // a to b
        setTabOrder(b, c); // a to b to c
        setTabOrder(c, d); // a to b to c to d
    \endcode

    \e not like this:

    \code
        setTabOrder(c, d); // c to d   WRONG
        setTabOrder(a, b); // a to b AND c to d
        setTabOrder(b, c); // a to b to c, but not c to d
    \endcode

    If \a first or \a second has a focus proxy, setTabOrder()
    correctly substitutes the proxy.

    \sa setFocusPolicy(), setFocusProxy()
*/
void QWidget::setTabOrder(QWidget* first, QWidget *second)
{
    if (!first || !second || first->focusPolicy() == Qt::NoFocus || second->focusPolicy() == Qt::NoFocus)
        return;

    QWidget *fp = first->focusProxy();
    if (fp) {
        // If first is redirected, set first to the last child of first
        // that can take keyboard focus so that second is inserted after
        // that last child, and the focus order within first is (more
        // likely to be) preserved.
        QList<QWidget *> l = qFindChildren<QWidget *>(first);
        for (int i = l.size()-1; i >= 0; --i) {
            QWidget * next = l.at(i);
            if (next->window() == fp->window()) {
                fp = next;
                if (fp->focusPolicy() != Qt::NoFocus)
                    break;
            }
        }
        first = fp;
    }

    if (QWidget *sp = second->focusProxy())
        second = sp;

    QWidget *p = second;
    while (p->d_func()->focus_next != second)
        p = p->d_func()->focus_next;
    p->d_func()->focus_next = second->d_func()->focus_next;

    second->d_func()->focus_next = first->d_func()->focus_next;
    first->d_func()->focus_next = second;
}

/*!\internal

  Moves the relevant subwidgets of this widget from the \a oldtlw's
  tab chain to that of the new parent, if there's anything to move and
  we're really moving

  This function is called from QWidget::reparent() *after* the widget
  has been reparented.

  \sa reparent()
*/

void QWidgetPrivate::reparentFocusWidgets(QWidget * oldtlw)
{
    Q_Q(QWidget);
    if (oldtlw == q->window())
        return; // nothing to do

    if(focus_child)
        focus_child->clearFocus();

    // seperate the focus chain
    QWidget *topLevel = q->window();
    QWidget *w = q;
    QWidget *firstOld = 0;
    QWidget *firstNew = 0;
    QWidget *o = 0;
    QWidget *n = 0;
    do {
        if (w == q || q->isAncestorOf(w)) {
            if (!firstNew)
                firstNew = w;
            if (n)
                n->d_func()->focus_next = w;
            n = w;
        } else {
            if (!firstOld)
                firstOld = w;
            if (o)
                o->d_func()->focus_next = w;
            o = w;
        }
    } while ((w = w->d_func()->focus_next) != q);
    if(o)
        o->d_func()->focus_next = firstOld;
    if(n)
        n->d_func()->focus_next = firstNew;

    if (!q->isWindow()) {
        //insert chain
        w = topLevel;
        while (w->d_func()->focus_next != topLevel)
            w = w->d_func()->focus_next;
        w->d_func()->focus_next = q;
        n->d_func()->focus_next = topLevel;
    } else {
        n->d_func()->focus_next = q;
    }
}

/*!\internal

  Measures the shortest distance from a point to a rect.

  This function is called from QDesktopwidget::screen(QPoint) to find the
  closest screen for a point.
*/
int QWidgetPrivate::pointToRect(const QPoint &p, const QRect &r)
{
    int dx = 0;
    int dy = 0;
    if (p.x() < r.left())
        dx = r.left() - p.x();
    else if (p.x() > r.right())
        dx = p.x() - r.right();
    if (p.y() < r.top())
        dy = r.top() - p.y();
    else if (p.y() > r.bottom())
        dy = p.y() - r.bottom();
    return dx + dy;
}

/*!
    \property QWidget::frameSize
    \brief the size of the widget including any window frame
*/
QSize QWidget::frameSize() const
{
    Q_D(const QWidget);
    if (isWindow() && !(windowType() == Qt::Popup)) {
        if (data->fstrut_dirty)
            d->updateFrameStrut();
        QWidget *that = (QWidget *) this;
        QTLWExtra *top = that->d_func()->topData();
        return QSize(data->crect.width() + top->fleft + top->fright,
                      data->crect.height() + top->ftop + top->fbottom);
    }
    return data->crect.size();
}

/*! \fn void QWidget::move(int x, int y)

    \overload

    This corresponds to move(QPoint(\a x, \a y)).
*/

void QWidget::move(const QPoint &p)
{
    Q_D(QWidget);
    QPoint oldp = pos();
    setAttribute(Qt::WA_Moved);
    d->setGeometry_sys(p.x() + geometry().x() - QWidget::x(),
                       p.y() + geometry().y() - QWidget::y(),
                       width(), height(), true);
    if (oldp != pos())
        d->updateInheritedBackground();
}

/*! \fn void QWidget::resize(int w, int h)
    \overload

    This corresponds to resize(QSize(\a w, \a h)).
*/

void QWidget::resize(const QSize &s)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_Resized);
    QSize olds = size();
    d->setGeometry_sys(geometry().x(), geometry().y(), s.width(), s.height(), false);
    if (testAttribute(Qt::WA_ContentsPropagated) &&  olds != size())
        d->updatePropagatedBackground();
}

void QWidget::setGeometry(const QRect &r)
{
    Q_D(QWidget);
    QPoint oldp = pos();
    QSize olds = size();
    setAttribute(Qt::WA_Resized);
    setAttribute(Qt::WA_Moved);
    d->setGeometry_sys(r.x(), r.y(), r.width(), r.height(), true);

    if (testAttribute(Qt::WA_ContentsPropagated) && olds != size())
        d->updatePropagatedBackground();
    else if (oldp != pos())
        d->updateInheritedBackground();
}


/*!\fn void QWidget::setGeometry(int x, int y, int w, int h)
    \overload

    This corresponds to setGeometry(QRect(\a x, \a y, \a w, \a h)).
*/

/*!
    Sets the margins around the contents of the widget to have the
    sizes \a left, \a top, \a right, and \a bottom. The margins are
    used by the layout system, and may be used by subclasses to
    specify the area to draw in (e.g. excluding the frame).

    Changing the margins will trigger a resizeEvent().

    \sa contentsRect()
*/
void QWidget::setContentsMargins(int left, int top, int right, int bottom)
{
    Q_D(QWidget);
    if (left == d->leftmargin && top == d->topmargin
         && right == d->rightmargin && bottom == d->bottommargin)
        return;
    d->leftmargin = left;
    d->topmargin = top;
    d->rightmargin = right;
    d->bottommargin = bottom;

    if (QLayout *l=d->layout)
        l->update(); //force activate; will do updateGeometry
    else
        updateGeometry();

    if (isVisible()) {
        update();
        QResizeEvent e(data->crect.size(), data->crect.size());
        QApplication::sendEvent(this, &e);
    } else {
        setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}

/*!
    Returns the area inside the widget's margins.

    \sa setContentsMargins() contentsMarginSize()
*/
QRect QWidget::contentsRect() const
{
    Q_D(const QWidget);
    return QRect(QPoint(d->leftmargin, d->topmargin),
                 QPoint(data->crect.width() - 1 - d->rightmargin,
                        data->crect.height() - 1 - d->bottommargin));

}

/*!
    Returns the size of the widget's margin. This is the same as
    size() - contentsRect().size().

    \sa setContentsMargins() contentsRect()
*/
QSize QWidget::contentsMarginSize() const
{
    Q_D(const QWidget);
    return QSize(d->leftmargin + d->rightmargin, d->topmargin + d->bottommargin);
}

/*!
  \fn void QWidget::customContextMenuRequested(const QPoint &pos)

  This signal is emitted when the widget's \l contextMenuPolicy is
  Qt::CustomContextMenu, and the user has requested a context menu on
  the widget. The position \a pos is the position of the request in
  widget coordinates.

  \sa mapToGlobal() QMenu contextMenuPolicy
*/

/*!
    \property QWidget::contextMenuPolicy
    \brief how the widget shows a context menu

    The default value of this property is Qt::DefaultContextMenu,
    which means the contextMenuEvent() handler is called. Other values
    are Qt::NoContextMenu, Qt::ActionsContextMenu, and
    Qt::CustomContextMenu. With Qt::CustomContextMenu, the signal
    customContextMenuRequested() is emitted.

    \sa contextMenuEvent() customContextMenuRequested()
*/

Qt::ContextMenuPolicy QWidget::contextMenuPolicy() const
{
    return (Qt::ContextMenuPolicy)data->context_menu_policy;
}

void QWidget::setContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
    data->context_menu_policy = (uint) policy;
}

/*!
    \property QWidget::focusPolicy
    \brief the way the widget accepts keyboard focus

    The policy is \c Qt::TabFocus if the widget accepts keyboard
    focus by tabbing, \c Qt::ClickFocus if the widget accepts
    focus by clicking, \c Qt::StrongFocus if it accepts both, and
    \c Qt::NoFocus (the default) if it does not accept focus at
    all.

    You must enable keyboard focus for a widget if it processes
    keyboard events. This is normally done from the widget's
    constructor. For instance, the QLineEdit constructor calls
    setFocusPolicy(Qt::StrongFocus).

    \sa focusEnabled, focusInEvent(), focusOutEvent(), keyPressEvent(),
        keyReleaseEvent(), enabled
*/


Qt::FocusPolicy QWidget::focusPolicy() const
{
    return (Qt::FocusPolicy)data->focus_policy;
}

void QWidget::setFocusPolicy(Qt::FocusPolicy policy)
{
    data->focus_policy = (uint) policy;
}

/*!
    \property QWidget::updatesEnabled
    \brief whether updates are enabled

    An updates enabled widget receives paint events and has a system
    background; a disabled widget does not. This also implies that
    calling update() and repaint() has no effect if updates are
    disabled.

    setUpdatesEnabled() is normally used to disable updates for a
    short period of time, for instance to avoid screen flicker during
    large changes. In Qt, widgets normally do not generate screen
    flicker, but on X11 the server might erase regions on the screen
    when widgets get hidden before they can be replaced by other
    widgets. Disabling updates solves this.

    Example:
    \code
        setUpdatesEnabled(false);
        bigVisualChanges();
        setUpdatesEnabled(true);
    \endcode

    Disabling a widget implicitly disables all its children. Enabling
    respectively enables all child widgets unless they have been
    explicitly disabled. Re-enabling updates implicitly calls update()
    on the widget.

    \sa paintEvent()
*/
void QWidget::setUpdatesEnabled(bool enable)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_ForceUpdatesDisabled, !enable);
    d->setUpdatesEnabled_helper(enable);
}

/*!  \fn void QWidget::show()

    Shows the widget and its child widgets. This function is
    equivalent to setVisible(true).

    \sa showEvent(), hide(), setVisible(), showMinimized(), showMaximized(),
    showNormal(), isVisible()
*/


/*! \internal

   Makes the widget visible in the isVisible() meaning of the word.
   It is only called for toplevels or widgets with visible parents.
 */
void QWidgetPrivate::show_recursive()
{
    Q_Q(QWidget);
    // polish if necessary
    q->ensurePolished();

#ifdef QT3_SUPPORT
    if(sendChildEvents)
        QApplication::sendPostedEvents(q, QEvent::ChildInserted);
#endif
#ifndef QT_NO_LAYOUT
    if (!q->isWindow() && q->parentWidget()->d_func()->layout)
        q->parentWidget()->d_func()->layout->activate();
#endif
#ifndef QT_NO_LAYOUT
    // activate our layout before we and our children become visible
    if (layout)
        layout->activate();
#endif

    show_helper();
}

void QWidgetPrivate::show_helper()
{
    Q_Q(QWidget);
    data.in_show = true; // qws optimization
    // make sure we receive pending move and resize events
    if (q->testAttribute(Qt::WA_PendingMoveEvent)) {
        QMoveEvent e(data.crect.topLeft(), data.crect.topLeft());
        QApplication::sendEvent(q, &e);
        q->setAttribute(Qt::WA_PendingMoveEvent, false);
    }
    if (q->testAttribute(Qt::WA_PendingResizeEvent)) {
        QResizeEvent e(data.crect.size(), data.crect.size());
        QApplication::sendEvent(q, &e);
        q->setAttribute(Qt::WA_PendingResizeEvent, false);
    }

    // become visible before showing all children
    q->setAttribute(Qt::WA_WState_Visible);

    // finally show all children recursively
    showChildren(false);

#ifdef QT3_SUPPORT
    if (q->parentWidget() && sendChildEvents)
        QApplication::sendPostedEvents(q->parentWidget(),
                                        QEvent::ChildInserted);
#endif


    // popup handling: new popups and tools need to be raised, and
    // exisiting popups must be closed. Also propagate the current
    // windows's KeyboardFocusChange status.
    if (q->isWindow()) {
        if ((q->windowType() == Qt::Tool) || (q->windowType() == Qt::Popup) || q->windowType() == Qt::ToolTip) {
            q->raise();
            if (q->parentWidget() && q->parentWidget()->window()->testAttribute(Qt::WA_KeyboardFocusChange))
                q->setAttribute(Qt::WA_KeyboardFocusChange);
        } else {
            while (QApplication::activePopupWidget()) {
                if (!QApplication::activePopupWidget()->close())
                    break;
            }
        }
    }

    // On Windows, show the popup now so that our own focus handling
    // stores the correct old focus widget even if it's stolen in the
    // showevent
#if defined(Q_WS_WIN)
    if ((q->windowType() == Qt::Popup))
        qApp->d_func()->openPopup(q);
#endif

    // send the show event before showing the window
    QShowEvent showEvent;
    QApplication::sendEvent(q, &showEvent);

    if (q->testAttribute(Qt::WA_ShowModal))
        // QApplicationPrivate::enterModal *before* show, otherwise the initial
        // stacking might be wrong
        QApplicationPrivate::enterModal(q);

    q->setAttribute(Qt::WA_Mapped);
    show_sys();

#if !defined(Q_WS_WIN)
    if ((q->windowType() == Qt::Popup))
        qApp->d_func()->openPopup(q);
#endif

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(q, 0, QAccessible::ObjectShow);
#endif

    data.in_show = false;  // reset qws optimization
}

/*! \fn void QWidget::hide()

    Hides the widget. This function is equivalent to
    setVisible(false).

    \sa hideEvent(), isExplicitlyHidden(), show(), setVisible(), isVisible(), close()
*/

/*!\internal
 */
void QWidgetPrivate::hide_helper()
{
    Q_Q(QWidget);
    if ((q->windowType() == Qt::Popup))
        qApp->d_func()->closePopup(q);

    // Move test modal here.  Otherwise, a modal dialog could get
    // destroyed and we lose all access to its parent because we haven't
    // left modality.  (Eg. modal Progress Dialog)
    if (q->testAttribute(Qt::WA_ShowModal))
        QApplicationPrivate::leaveModal(q);

#if defined(Q_WS_WIN)
    if (q->isWindow() && !(q->windowType() == Qt::Popup) && q->parentWidget() && q->isActiveWindow())
        q->parentWidget()->activateWindow();        // Activate parent
#endif

    q->setAttribute(Qt::WA_Mapped, false);
    hide_sys();

    bool wasVisible = q->testAttribute(Qt::WA_WState_Visible);

    if (wasVisible) {
        q->setAttribute(Qt::WA_WState_Visible, false);

    }

    QHideEvent hideEvent;
    QApplication::sendEvent(q, &hideEvent);
    hideChildren(false);

    // next bit tries to move the focus if the focus widget is now
    // hidden.
    if (wasVisible) {
        QWidget *fw = QApplication::focusWidget();
        while (fw &&  !fw->isWindow()) {
            if (fw == q) {
                q->focusNextPrevChild(true);
                break;
            }
            fw = fw->parentWidget();
        }
    }


#ifndef QT_NO_ACCESSIBILITY
    if (wasVisible)
        QAccessible::updateAccessibility(q, 0, QAccessible::ObjectHide);
#endif
#ifndef QT_NO_LAYOUT
    // invalidate layout similar to updateGeometry()
    if (!q->isWindow() && q->parentWidget()) {
        if (q->parentWidget()->d_func()->layout)
            q->parentWidget()->d_func()->layout->update();
        if (wasVisible)
            QApplication::postEvent(q->parentWidget(), new QEvent(QEvent::LayoutRequest));
    }
#endif
}

/*!
    \fn bool QWidget::isExplicitlyHidden() const

    Returns true if the widget is explicity hidden, otherwise returns
    false.

    Widgets are explicitly hidden if they were created as independent
    windows, or if hide() or setVisible(false) was called.

    \sa setVisible(), hide()
*/

void QWidget::setVisible(bool visible)
{
    if (visible) { // show
        if (testAttribute(Qt::WA_WState_ExplicitShowHide) && !testAttribute(Qt::WA_WState_Hidden))
            return;

#ifdef Q_WS_X11
        if (windowType() == Qt::Window)
            QApplicationPrivate::applyX11SpecificCommandLineArguments(this);
#endif

        bool wasResized = testAttribute(Qt::WA_Resized);
        Qt::WindowStates initialWindowState = windowState();

        Q_D(QWidget);
        if (isWindow() && !testAttribute(Qt::WA_SetWindowIcon))
            d->setWindowIcon_sys();

        // polish if necessary
        ensurePolished();

        // remember that show was called explicitly
        setAttribute(Qt::WA_WState_ExplicitShowHide);
        // whether we need to inform the parent widget immediately
        bool needUpdateGeometry = !isWindow() && testAttribute(Qt::WA_WState_Hidden);
        // we are no longer hidden
        setAttribute(Qt::WA_WState_Hidden, false);

        if (needUpdateGeometry)
            updateGeometry();

#ifdef QT3_SUPPORT
        QApplication::sendPostedEvents(this, QEvent::ChildInserted);
#endif
#ifndef QT_NO_LAYOUT
        if (!isWindow() && parentWidget()->d_func()->layout)
            parentWidget()->d_func()->layout->activate();
#endif
#ifndef QT_NO_LAYOUT
        // activate our layout before we and our children become visible
        if (d->layout)
            d->layout->activate();
#endif

        // adjust size if necessary
        if (!wasResized
            && (isWindow() || !parentWidget()->d_func()->layout))  {
            if (isWindow()) {
                adjustSize();
                if (windowState() != initialWindowState)
                    setWindowState(initialWindowState);
            } else {
                adjustSize();
            }
            setAttribute(Qt::WA_Resized, false);
        }

        setAttribute(Qt::WA_KeyboardFocusChange, false);

        if (isWindow() || parentWidget()->isVisible())
            d->show_helper();

        QEvent showToParentEvent(QEvent::ShowToParent);
        QApplication::sendEvent(this, &showToParentEvent);
    } else { // hide
        if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden))
            return;

        Q_D(QWidget);
        setAttribute(Qt::WA_WState_Hidden);
        if (testAttribute(Qt::WA_WState_ExplicitShowHide))
            d->hide_helper();
        else
            setAttribute(Qt::WA_WState_ExplicitShowHide);
        QEvent hideToParentEvent(QEvent::HideToParent);
        QApplication::sendEvent(this, &hideToParentEvent);
    }
}

/*!\fn void QWidget::setHidden(bool hidden)

    Convenience function, equivalent to setVisible(!\a hidden).
*/

/*!\fn void QWidget::setShown(bool shown)

    Use setVisible(\a shown) instead.
*/

void QWidgetPrivate::showChildren(bool spontaneous)
{
    QList<QObject*> childList = children;
    for (int i = 0; i < childList.size(); ++i) {
        QWidget *widget = qobject_cast<QWidget*>(childList.at(i));
        if (!widget
            || widget->isWindow()
            || widget->testAttribute(Qt::WA_WState_Hidden))
            continue;
        if (spontaneous) {
            widget->setAttribute(Qt::WA_Mapped);
            widget->d_func()->showChildren(true);
            QShowEvent e;
            QApplication::sendSpontaneousEvent(widget, &e);
        } else {
            if (widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
                widget->d_func()->show_recursive();
            else
                widget->show();
        }
    }
}

void QWidgetPrivate::hideChildren(bool spontaneous)
{
    QList<QObject*> childList = children;
    for (int i = 0; i < childList.size(); ++i) {
        QWidget *widget = qobject_cast<QWidget*>(childList.at(i));
        if (!widget || widget->isWindow() || widget->testAttribute(Qt::WA_WState_Hidden))
            continue;
        if (spontaneous)
            widget->setAttribute(Qt::WA_Mapped, false);
        else
            widget->setAttribute(Qt::WA_WState_Visible, false);
        widget->d_func()->hideChildren(spontaneous);
        QHideEvent e;
        if (spontaneous)
            QApplication::sendSpontaneousEvent(widget, &e);
        else
            QApplication::sendEvent(widget, &e);
    }
}

bool QWidgetPrivate::close_helper(CloseMode mode)
{
    if (data.is_closing)
        return true;

    Q_Q(QWidget);
    data.is_closing = 1;

    QPointer<QWidget> that = q;

#ifdef QT3_SUPPORT
    bool isMain = (QApplicationPrivate::main_widget == q);
#endif
    bool quitOnClose = q->testAttribute(Qt::WA_QuitOnClose);

    if (mode != CloseNoEvent) {
        QCloseEvent e;
        if (mode == CloseWithSpontaneousEvent)
            QApplication::sendSpontaneousEvent(q, &e);
        else
            QApplication::sendEvent(q, &e);
        if (!that.isNull() && !e.isAccepted()) {
            data.is_closing = 0;
            return false;
        }
    }

    if (!that.isNull() && !q->isExplicitlyHidden())
        q->hide();

#ifdef QT3_SUPPORT
    if (isMain)
        qApp->quit();
#endif
    if (quitOnClose) {
        /* if there is no non-withdrawn top level window left
           (except the desktop, popups, or dialogs/tools with
           parents), we emit the lastWindowClosed signal */
        QWidgetList list = QApplication::topLevelWidgets();
        bool lastWindowClosed = true;
        for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (w->isExplicitlyHidden() || !w->testAttribute(Qt::WA_QuitOnClose))
                continue;
            lastWindowClosed = false;
            break;
        }
        if (lastWindowClosed)
            QApplicationPrivate::emitLastWindowClosed();
    }

    if (!that.isNull()) {
        data.is_closing = 0;
        if (q->testAttribute(Qt::WA_DeleteOnClose)) {
            q->setAttribute(Qt::WA_DeleteOnClose, false);
            q->deleteLater();
        }
    }
    return true;
}


/*!
    Closes this widget. Returns true if the widget was closed;
    otherwise returns false.

    First it sends the widget a QCloseEvent. The widget is \link
    hide() hidden\endlink if it \link QCloseEvent::accept()
    accepts\endlink the close event. If it \link QCloseEvent::ignore()
    ignores\endlink the event, nothing happens. The default
    implementation of QWidget::closeEvent() accepts the close event.

    If the widget has the \c Qt::WA_DeleteOnClose flag, the widget
    is also deleted. A close events is delivered to the widget no
    matter if the widget is visible or not.

    The \l QApplication::lastWindowClosed() signal is emitted when the
    last visible top level widget with the Qt::WA_QuitOnClose
    attribute set is closed. By default this attribute is set for all
    widgets except transient top level widgets such as splash screens,
    popup menus, and dialogs.

*/

bool QWidget::close()
{
    return d_func()->close_helper(QWidgetPrivate::CloseWithEvent);
}

/*!
    \property QWidget::visible
    \brief whether the widget is visible

    Calling setVisible(true) or show() sets the widget to visible
    status if all its parent widgets up to the window are visible. If
    an ancestor is not visible, the widget won't become visible until
    all its ancestors are shown. If its size or position has changed,
    Qt guarantees that a widget gets move and resize events just
    before it is shown. If the widget has not been resized yet, Qt
    will adjust the widget's size to a useful default using
    adjustSize().

    Calling setVisible(false) or hide() hides a widget explicitly. An
    explicitly hidden widget will never become visible, even if all
    its ancestors become visible, unless you show it.

    A widget receives show and hide events when its visibility status
    changes. Between a hide and a show event, there is no need to
    waste CPU cycles preparing or displaying information to the user.
    A video application, for example, might simply stop generating new
    frames.

    A widget that happens to be obscured by other windows on the
    screen is considered to be visible. The same applies to iconified
    windows and windows that exist on another virtual
    desktop (on platforms that support this concept). A widget
    receives spontaneous show and hide events when its mapping status
    is changed by the window system, e.g. a spontaneous hide event
    when the user minimizes the window, and a spontaneous show event
    when the window is restored again.

    You almost never have to reimplement the setVisible() function. If
    you need to change some settings before a widget is shown, use
    showEvent() instead. If you need to do some delayed initialization
    use the Polish event delivered to the event() method

    \sa show(), hide(), isExplicitlyHidden(), isVisibleTo(), isMinimized(),
    showEvent(), hideEvent()
*/


/*!
    Returns true if this widget would become visible if \a ancestor is
    shown; otherwise returns false.

    The true case occurs if neither the widget itself nor any parent
    up to but excluding \a ancestor has been explicitly hidden.

    This function will still return true if the widget is obscured by
    other windows on the screen, but could be physically visible if it
    or they were to be moved.

    isVisibleTo(0) is identical to isVisible().

    \sa show() hide() isVisible()
*/

bool QWidget::isVisibleTo(QWidget* ancestor) const
{
    if (!ancestor)
        return isVisible();
    const QWidget * w = this;
    while (w
            && !w->isExplicitlyHidden()
            && !w->isWindow()
            && w->parentWidget()
            && w->parentWidget() != ancestor)
        w = w->parentWidget();
    return !w->isExplicitlyHidden();
}

#ifdef QT3_SUPPORT
QRect QWidget::visibleRect() const
{
    return d_func()->clipRect();
}
#endif

/*!
    Returns the unobscured region where paint events can occur.

    For visible widgets, this is an approximation of the area not
    covered by other widgets; otherwise, this is an empty region.

    The repaint() function calls this function if necessary, so in
    general you do not need to call it.

*/
QRegion QWidget::visibleRegion() const
{
    return d_func()->clipRect();
}


/*!
    Adjusts the size of the widget to fit the contents.

    Uses sizeHint() if valid (i.e if the size hint's width and height
    are \>= 0), otherwise sets the size to the children rectangle (the
    union of all child widget geometries). For toplevel widgets, the
    function also takes the screen size into account.

    \sa sizeHint(), childrenRect()
*/

void QWidget::adjustSize()
{
    ensurePolished();

    QSize s = sizeHint();

    if (isWindow()) {
        Qt::Orientations exp;
#ifndef QT_NO_LAYOUT
        if (QLayout *l = layout()) {
            if (l->hasHeightForWidth())
                s.setHeight(l->totalHeightForWidth(s.width()));
            exp = l->expandingDirections();
        } else
#endif
        {
            if (sizePolicy().hasHeightForWidth())
                s.setHeight(heightForWidth(s.width()));
            exp = sizePolicy().expandingDirections();
        }
        if (exp & Qt::Horizontal)
            s.setWidth(qMax(s.width(), 200));
        if (exp & Qt::Vertical)
            s.setHeight(qMax(s.height(), 150));
#if defined(Q_WS_X11)
        QRect screen = QApplication::desktop()->screenGeometry(x11Info().screen());
#else // all others
        QRect screen = QApplication::desktop()->screenGeometry(pos());
#endif
        s.setWidth(qMin(s.width(), screen.width()*2/3));
        s.setHeight(qMin(s.height(), screen.height()*2/3));
    }

    if (!s.isValid()) {
        QRect r = childrenRect(); // get children rectangle
        if (r.isNull())
            return;
        s = r.size() + QSize(2 * r.x(),  2 * r.y());
    }

    resize(s);
}


/*!
    \property QWidget::sizeHint
    \brief the recommended size for the widget

    If the value of this property is an invalid size, no size is
    recommended.

    The default implementation of sizeHint() returns an invalid size
    if there is no layout for this widget, and returns the layout's
    preferred size otherwise.

    \sa QSize::isValid(), minimumSizeHint(), sizePolicy(),
    setMinimumSize(), updateGeometry()
*/

QSize QWidget::sizeHint() const
{
    Q_D(const QWidget);
#ifndef QT_NO_LAYOUT
    if (d->layout)
        return d->layout->totalSizeHint();
#endif
    return QSize(-1, -1);
}

/*!
    \property QWidget::minimumSizeHint
    \brief the recommended minimum size for the widget

    If the value of this property is an invalid size, no minimum size
    is recommended.

    The default implementation of minimumSizeHint() returns an invalid
    size if there is no layout for this widget, and returns the
    layout's minimum size otherwise. Most built-in widgets reimplement
    minimumSizeHint().

    \l QLayout will never resize a widget to a size smaller than
    minimumSizeHint.

    \sa QSize::isValid(), resize(), setMinimumSize(), sizePolicy()
*/
QSize QWidget::minimumSizeHint() const
{
    Q_D(const QWidget);
#ifndef QT_NO_LAYOUT
    if (d->layout)
        return d->layout->totalMinimumSize();
#endif
    return QSize(-1, -1);
}


/*!
    \fn QWidget *QWidget::parentWidget() const

    Returns the parent of this widget, or 0 if it does not have any
    parent widget.
*/


/*****************************************************************************
  QWidget event handling
 *****************************************************************************/

/*!
    This is the main event handler; it handles event \a e. You can
    reimplement this function in a subclass, but we recommend using
    one of the specialized event handlers instead.

    Key press and release events are treated differently from other
    events. event() checks for Tab and Shift+Tab and tries to move the
    focus appropriately. If there is no widget to move the focus to
    (or the key press is not Tab or Shift+Tab), event() calls
    keyPressEvent().

    Mouse and tablet event handling is also slightly special: only
    when the widget is \l enabled, event() will call the specialized
    handlers such as mousePressEvent(); otherwise it will discard the
    event.

    This function returns true if the event was recognized, otherwise
    it returns false.  If the recognized event was accepted (see \l
    QEvent::accepted), any further processing such as event
    propagation to the parent widget stops.

    \sa closeEvent(), focusInEvent(), focusOutEvent(), enterEvent(),
    keyPressEvent(), keyReleaseEvent(), leaveEvent(),
    mouseDoubleClickEvent(), mouseMoveEvent(), mousePressEvent(),
    mouseReleaseEvent(), moveEvent(), paintEvent(), resizeEvent(),
    QObject::event(), QObject::timerEvent()
*/

bool QWidget::event(QEvent *e)
{
    Q_D(QWidget);

    // ignore mouse events when disabled
    if (!isEnabled()) {
        switch(e->type()) {
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
        case QEvent::TabletMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::ContextMenu:
#ifndef QT_NO_WHEELEVENT
        case QEvent::Wheel:
#endif
            return false;
        default:
            break;
        }
    }
    switch (e->type()) {
    case QEvent::MouseMove:
        mouseMoveEvent((QMouseEvent*)e);
        break;

    case QEvent::MouseButtonPress:
	// Don't reset input context here. Whether reset or not is
	// a responsibility of input method. reset() will be
	// called by mouseHandler() of input method if necessary
	// via mousePressEvent() of text widgets.
#if 0
        resetInputContext();
#endif
        mousePressEvent((QMouseEvent*)e);
        break;

    case QEvent::MouseButtonRelease:
        mouseReleaseEvent((QMouseEvent*)e);
        break;

    case QEvent::MouseButtonDblClick:
        mouseDoubleClickEvent((QMouseEvent*)e);
        break;
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        wheelEvent((QWheelEvent*)e);
        break;
#endif
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
        tabletEvent((QTabletEvent*)e);
        break;
#ifdef QT3_SUPPORT
    case QEvent::Accel:
        e->ignore();
        return false;
#endif
    case QEvent::KeyPress: {
        QKeyEvent *k = (QKeyEvent *)e;
        bool res = false;
        if (!(k->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
            if (k->key() == Qt::Key_Backtab
                || (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier)))
                res = focusNextPrevChild(false);
            else if (k->key() == Qt::Key_Tab)
                res = focusNextPrevChild(true);
            if (res)
                break;
        }
        keyPressEvent(k);
        if (!k->isAccepted()
            && k->modifiers() & Qt::ShiftModifier && k->key() == Qt::Key_F1
            && d->whatsThis.size()) {
            QWhatsThis::showText(mapToGlobal(inputMethodQuery(Qt::ImMicroFocus).toRect().center()), d->whatsThis, this);
            k->accept();
        }
    }
        break;

    case QEvent::KeyRelease:
        keyReleaseEvent((QKeyEvent*)e);
        // fall through
    case QEvent::ShortcutOverride:
        break;

    case QEvent::InputMethod:
        inputMethodEvent((QInputMethodEvent *) e);
        break;

    case QEvent::PolishRequest:
        ensurePolished();
        break;

    case QEvent::Polish: {
        style()->polish(this);
        setAttribute(Qt::WA_WState_Polished);
        if (!testAttribute(Qt::WA_SetFont) && !QApplication::font(this).isCopyOf(QApplication::font()))
            d->resolveFont();
        if (!QApplication::palette(this).isCopyOf(QApplication::palette()))
            d->resolvePalette();
#ifdef QT3_SUPPORT
        if(d->sendChildEvents)
            QApplication::sendPostedEvents(this, QEvent::ChildInserted);
#endif
    }
        break;

    case QEvent::ApplicationWindowIconChange:
        if (isWindow() && !testAttribute(Qt::WA_SetWindowIcon))
            d->setWindowIcon_sys();
        break;

    case QEvent::FocusIn:
        focusInEvent((QFocusEvent*)e);
        break;

    case QEvent::FocusOut:
        focusOutEvent((QFocusEvent*)e);
        break;

    case QEvent::Enter:
        if (d->statusTip.size()) {
            QStatusTipEvent tip(d->statusTip);
            QApplication::sendEvent(this, &tip);
        }
        enterEvent(e);
        break;

    case QEvent::Leave:
        if (d->statusTip.size()) {
            QStatusTipEvent tip(QString::null);
            QApplication::sendEvent(this, &tip);
        }
        leaveEvent(e);
        break;

    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
        update();
        break;

    case QEvent::Paint:
        // At this point the event has to be delivered, regardless
        // whether the widget isVisible() or not because it
        // already went through the filters
        paintEvent((QPaintEvent*)e);
        break;

    case QEvent::Move:
        moveEvent((QMoveEvent*)e);
        break;

    case QEvent::Resize:
        resizeEvent((QResizeEvent*)e);
        break;

    case QEvent::Close:
        closeEvent((QCloseEvent *)e);
        break;

    case QEvent::ContextMenu:
        switch (data->context_menu_policy) {
        case Qt::DefaultContextMenu:
            contextMenuEvent(static_cast<QContextMenuEvent *>(e));
            break;
        case Qt::CustomContextMenu:
            emit customContextMenuRequested(static_cast<QContextMenuEvent *>(e)->pos());
            break;
        case Qt::ActionsContextMenu:
            if (d->actions.count()) {
                QMenu::exec(d->actions, static_cast<QContextMenuEvent *>(e)->globalPos());
                break;
            }
            // fall through
        default:
            e->ignore();
            break;
        }
        break;

#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
        dropEvent((QDropEvent*) e);
        break;

    case QEvent::DragEnter:
        dragEnterEvent((QDragEnterEvent*) e);
        break;

    case QEvent::DragMove:
        dragMoveEvent((QDragMoveEvent*) e);
        break;

    case QEvent::DragLeave:
        dragLeaveEvent((QDragLeaveEvent*) e);
        break;
#endif

    case QEvent::Show:
        showEvent((QShowEvent*) e);
        break;

    case QEvent::Hide:
        hideEvent((QHideEvent*) e);
        break;

    case QEvent::ShowWindowRequest:
        if (!isExplicitlyHidden())
            d->show_sys();
        break;

    case QEvent::ApplicationFontChange:
        d->resolveFont();
        break;
    case QEvent::ApplicationPaletteChange:
        if (!(windowType() == Qt::Desktop))
            d->resolvePalette();
        break;
    case QEvent::ToolBarChange:
    case QEvent::ActivationChange:
    case QEvent::EnabledChange:
    case QEvent::FontChange:
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::WindowTitleChange:
    case QEvent::IconTextChange:
    case QEvent::ModifiedChange:
    case QEvent::MouseTrackingChange:
    case QEvent::ParentChange:
    case QEvent::WindowStateChange:
        changeEvent(e);
        break;

    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate: {
#ifdef QT3_SUPPORT
        windowActivationChange(e->type() != QEvent::WindowActivate);
#endif
        if (isVisible()) {
            for(int role=0; role < (int)QPalette::NColorRoles; role++) {
                if(data->pal.brush(QPalette::Active, (QPalette::ColorRole)role) !=
                   data->pal.brush(QPalette::Inactive, (QPalette::ColorRole)role)) {
                    QPalette::ColorRole bg_role = backgroundRole();
                    if (!testAttribute(Qt::WA_NoSystemBackground) && bg_role < QPalette::NColorRoles &&
                         (role == bg_role || (role < bg_role && data->pal.brush(QPalette::Active, bg_role) !=
                                              data->pal.brush(QPalette::Inactive, bg_role))))
                        d->updateSystemBackground();
                    else if(role <= QPalette::Shadow)
                        update();
                    break;
                }
            }
        }
        QList<QObject*> childList = d->children;
        for (int i = 0; i < childList.size(); ++i) {
            QWidget *w = qobject_cast<QWidget *>(childList.at(i));
            if (w && w->isVisible() && !w->isWindow())
                QApplication::sendEvent(w, e);
        }
        break; }

    case QEvent::LanguageChange:
        changeEvent(e);
#ifdef QT3_SUPPORT
        languageChange();
#endif
        // fall through
    case QEvent::LocaleChange:
        {
            QList<QObject*> childList = d->children;
            for (int i = 0; i < childList.size(); ++i) {
                QObject *o = childList.at(i);
                QApplication::sendEvent(o, e);
            }
        }
        update();
        break;

    case QEvent::ApplicationLayoutDirectionChange:
        d->resolveLayoutDirection();
        break;

    case QEvent::LayoutDirectionChange:
        if (d->layout)
            d->layout->invalidate();
        update();
        break;

#if defined(Q_WS_X11)
    case QEvent::UpdateRequest:
        if (!d->invalidated_region.isEmpty()) {
            QRegion rgn = d->invalidated_region;
            d->invalidated_region = QRegion();
            repaint(rgn);
        }
        break;
#endif
#if defined(Q_WS_QWS)
    case QEvent::QWSUpdate:
        repaint(static_cast<QWSUpdateEvent*>(e)->region());
        break;
#endif

    case QEvent::WindowBlocked:
    case QEvent::WindowUnblocked:
        {
            QList<QObject*> childList = d->children;
            for (int i = 0; i < childList.size(); ++i) {
                QObject *o = childList.at(i);
                QApplication::sendEvent(o, e);
            }
        }
        break;

    case QEvent::ToolTip:
        if (d->toolTip.size() && isActiveWindow())
            QToolTip::showText(static_cast<QHelpEvent*>(e)->globalPos(), d->toolTip, this);
        else
            e->ignore();
        break;

#ifndef QT_NO_WHATSTHIS
    case QEvent::WhatsThis:
        if (d->whatsThis.size())
            QWhatsThis::showText(static_cast<QHelpEvent *>(e)->globalPos(), d->whatsThis, this);
        else
            e->ignore();
        break;
    case QEvent::QueryWhatsThis:
        if (d->whatsThis.isEmpty())
            e->ignore();
        break;
#endif
#ifndef QT_NO_ACCESSIBILITY
    case QEvent::AccessibilityHelp: {
        QAccessibleEvent *ev = static_cast<QAccessibleEvent *>(e);
        if (ev->child())
            return false;
        switch (ev->textType()) {
        case QAccessibleEvent::Description:
            ev->setValue(d->toolTip);
            break;
        case QAccessibleEvent::Help:
            ev->setValue(d->whatsThis);
            break;
        default:
            return false;
        }
        break; }
#endif
    case QEvent::EmbeddingControl:
        data->window_flags & ~Qt::WindowType_Mask;
        d->topData()->ftop = 0;
        d->topData()->fright = 0;
        d->topData()->fleft = 0;
        d->topData()->fbottom = 0;
        break;
    case QEvent::ActionAdded:
    case QEvent::ActionRemoved:
    case QEvent::ActionChanged:
        actionEvent((QActionEvent*)e);
        break;
    default:
        return QObject::event(e);
    }
    return true;
}

/*!
  This event handler can be reimplemented to handle state changes.

  The state being changed in this event can be retrieved through event \a
  e.

*/
void QWidget::changeEvent(QEvent * e)
{
    switch(e->type()) {
    case QEvent::EnabledChange:
        update();
#ifndef QT_NO_ACCESSIBILITY
        QAccessible::updateAccessibility(this, 0, QAccessible::StateChanged);
#endif
        break;

    case QEvent::FontChange:
    case QEvent::StyleChange:
        update();
        updateGeometry();
        break;

    case QEvent::PaletteChange:
        update();
        break;

    default:
        break;
    }
}

/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive mouse move events for the widget.

    If mouse tracking is switched off, mouse move events only occur if
    a mouse button is pressed while the mouse is being moved. If mouse
    tracking is switched on, mouse move events occur even if no mouse
    button is pressed.

    QMouseEvent::pos() reports the position of the mouse cursor,
    relative to this widget. For press and release events, the
    position is usually the same as the position of the last mouse
    move event, but it might be different if the user's hand shakes.
    This is a feature of the underlying window system, not Qt.

    \sa setMouseTracking(), mousePressEvent(), mouseReleaseEvent(),
    mouseDoubleClickEvent(), event(), QMouseEvent
*/

void QWidget::mouseMoveEvent(QMouseEvent * e)
{
    e->ignore();
}

/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive mouse press events for the widget.

    If you create new widgets in the mousePressEvent() the
    mouseReleaseEvent() may not end up where you expect, depending on
    the underlying window system (or X11 window manager), the widgets'
    location and maybe more.

    The default implementation implements the closing of popup widgets
    when you click outside the window. For other widget types it does
    nothing.

    \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
    mouseMoveEvent(), event(), QMouseEvent
*/

void QWidget::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
    if ((windowType() == Qt::Popup)) {
        e->accept();
        QWidget* w;
        while ((w = qApp->activePopupWidget()) && w != this){
            w->close();
            if (qApp->activePopupWidget() == w) // widget does not want to dissappear
                w->hide(); // hide at least
        }
        if (!rect().contains(e->pos())){
            close();
        }
    }
}

/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive mouse release events for the widget.

    \sa mousePressEvent(), mouseDoubleClickEvent(),
    mouseMoveEvent(), event(),  QMouseEvent
*/

void QWidget::mouseReleaseEvent(QMouseEvent * e)
{
    e->ignore();
}

/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive mouse double click events for the widget.

    The default implementation generates a normal mouse press event.

    Note that the widgets gets a mousePressEvent() and a
    mouseReleaseEvent() before the mouseDoubleClickEvent().

    \sa mousePressEvent(), mouseReleaseEvent() mouseMoveEvent(),
    event(), QMouseEvent
*/

void QWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    mousePressEvent(e);                        // try mouse press event
}

#ifndef QT_NO_WHEELEVENT
/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive wheel events for the widget.

    If you reimplement this handler, it is very important that you
    \link QWheelEvent ignore()\endlink the event if you do not handle
    it, so that the widget's parent can interpret it.

    The default implementation ignores the event.

    \sa QWheelEvent::ignore(), QWheelEvent::accept(), event(),
    QWheelEvent
*/

void QWidget::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}
#endif


/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive tablet events for the widget.

    If you reimplement this handler, it is very important that you
    \link QTabletEvent ignore()\endlink the event if you do not handle
    it, so that the widget's parent can interpret it.

    The default implementation ignores the event.

    \sa QTabletEvent::ignore(), QTabletEvent::accept(), event(),
    QTabletEvent
*/

void QWidget::tabletEvent(QTabletEvent *e)
{
    e->ignore();
}

/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive key press events for the widget.

    A widget must call setFocusPolicy() to accept focus initially and
    have focus in order to receive a key press event.

    If you reimplement this handler, it is very important that you
    \link QKeyEvent ignore()\endlink the event if you do not
    understand it, so that the widget's parent can interpret it.

    The default implementation closes popup widgets if the user
    presses Esc. Otherwise the event is ignored.

    \sa keyReleaseEvent(), QKeyEvent::ignore(), setFocusPolicy(),
    focusInEvent(), focusOutEvent(), event(), QKeyEvent
*/

void QWidget::keyPressEvent(QKeyEvent *e)
{
    if ((windowType() == Qt::Popup) && e->key() == Qt::Key_Escape) {
        e->accept();
        close();
    } else {
        e->ignore();
    }
}

/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive key release events for the widget.

    A widget must \link setFocusPolicy() accept focus\endlink
    initially and \link hasFocus() have focus\endlink in order to
    receive a key release event.

    If you reimplement this handler, it is very important that you
    \link QKeyEvent ignore()\endlink the release if you do not
    understand it, so that the widget's parent can interpret it.

    The default implementation ignores the event.

    \sa keyPressEvent(), QKeyEvent::ignore(), setFocusPolicy(),
    focusInEvent(), focusOutEvent(), event(), QKeyEvent
*/

void QWidget::keyReleaseEvent(QKeyEvent *e)
{
    e->ignore();
}

/*!
    \fn void QWidget::focusInEvent(QFocusEvent *event)

    This event handler can be reimplemented in a subclass to receive
    keyboard focus events (focus received) for the widget. The event
    is passed in the \a event parameter

    A widget normally must setFocusPolicy() to something other than
    \c Qt::NoFocus in order to receive focus events. (Note that the
    application programmer can call setFocus() on any widget, even
    those that do not normally accept focus.)

    The default implementation updates the widget (except for windows
    that do not specify a focusPolicy()).

    \sa focusOutEvent(), setFocusPolicy(), keyPressEvent(),
    keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusInEvent(QFocusEvent *)
{
    if (focusPolicy() != Qt::NoFocus || !isWindow()) {
        update();
    }
}

/*!
    \fn void QWidget::focusOutEvent(QFocusEvent *event)

    This event handler can be reimplemented in a subclass to receive
    keyboard focus events (focus lost) for the widget. The events is
    passed in the \a event parameter.

    A widget normally must setFocusPolicy() to something other than
    \c Qt::NoFocus in order to receive focus events. (Note that the
    application programmer can call setFocus() on any widget, even
    those that do not normally accept focus.)

    The default implementation updates the widget (except for windows
    that do not specify a focusPolicy()).

    \sa focusInEvent(), setFocusPolicy(), keyPressEvent(),
    keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusOutEvent(QFocusEvent *)
{
    if (focusPolicy() != Qt::NoFocus || !isWindow())
        update();
}

/*!
    \fn void QWidget::enterEvent(QEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget enter events which are passed in the \a event parameter.

    An event is sent to the widget when the mouse cursor enters the
    widget.

    \sa leaveEvent(), mouseMoveEvent(), event()
*/

void QWidget::enterEvent(QEvent *)
{
}

/*!
    \fn void QWidget::leaveEvent(QEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget leave events which are passed in the \a event parameter.

    A leave event is sent to the widget when the mouse cursor leaves
    the widget.

    \sa enterEvent(), mouseMoveEvent(), event()
*/

void QWidget::leaveEvent(QEvent *)
{
}

/*!
    \fn void QWidget::paintEvent(QPaintEvent *event)

    This event handler can be reimplemented in a subclass to receive
    paint events which are passed in the \a event parameter.

    A paint event is a request to repaint all or part of the widget.
    It can happen as a result of repaint() or update(), or because the
    widget was obscured and has now been uncovered, or for many other
    reasons.

    Many widgets can simply repaint their entire surface when asked
    to, but some slow widgets need to optimize by painting only the
    requested region: QPaintEvent::region(). This speed optimization
    does not change the result, as painting is clipped to that region
    during event processing. QListView and QTableView do this, for
    example.

    Qt also tries to speed up painting by merging multiple paint
    events into one. When update() is called several times or the
    window system sends several paint events, Qt merges these events
    into one event with a larger region (see QRegion::unite()).
    repaint() does not permit this optimization, so we suggest using
    update() when possible.

    When the paint event occurs, the update region has normally been
    erased, so that you're painting on the widget's background.

    The background can be set using setBackgroundRole() and
    setPalette().

    From Qt 4.0, QWidget automatically double-buffers its painting, so
    there's no need to write double-buffering code in paintEvent() to
    avoid flicker.

    \sa event(), repaint(), update(), QPainter, QPixmap, QPaintEvent
*/

void QWidget::paintEvent(QPaintEvent *)
{
}


/*!
    \fn void QWidget::moveEvent(QMoveEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget move events which are passed in the \a event parameter.
    When the widget receives this event, it is already at the new
    position.

    The old position is accessible through QMoveEvent::oldPos().

    \sa resizeEvent(), event(), move(), QMoveEvent
*/

void QWidget::moveEvent(QMoveEvent *)
{
}


/*!
    \fn void QWidget::resizeEvent(QResizeEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget resize events which are passed in the \a event parameter.
    When resizeEvent() is called, the widget already has its new
    geometry. The old size is accessible through
    QResizeEvent::oldSize().

    The widget will be erased and receive a paint event immediately
    after processing the resize event. No drawing need be (or should
    be) done inside this handler.


    \sa moveEvent(), event(), resize(), QResizeEvent, paintEvent()
*/

void QWidget::resizeEvent(QResizeEvent *)
{
}

/*!
    \fn void QWidget::actionEvent(QActionEvent *event)

    This event handler is called with the given \a event whenever the
    widget's actions are changed.

    \sa addAction(), insertAction(), removeAction(), actions(), QActionEvent
*/
void QWidget::actionEvent(QActionEvent *)
{

}

/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive widget close events.

    \sa event(), hide(), close(), QCloseEvent
*/

void QWidget::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
}


/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive widget context menu events.

    The handler is called when the widget's \l contextMenuPolicy is
    Qt::DefaultContextMenu.

    The default implementation calls e->ignore(), which rejects the
    context event. See the \l QContextMenuEvent documentation for
    more details.

    \sa event(), QContextMenuEvent customContextMenuRequested()
*/

void QWidget::contextMenuEvent(QContextMenuEvent *e)
{
    e->ignore();
}


/*!
    This event handler, for event \a e, can be reimplemented in a
    subclass to receive Input Method composition events. This handler
    is called when the state of the input method changes.

    The default implementation calls e->ignore(), which rejects the
    Input Method event. See the \l QInputMethodEvent documentation for more
    details.

    \sa event(), QInputMethodEvent
*/
void QWidget::inputMethodEvent(QInputMethodEvent *e)
{
    e->ignore();
}

/*!
    This method is only relevant for input widgets. It is used by the
    input method to query a set of properties of the widget to be
    able to support complex input method operations as support for
    surrounding text and reconversions.

    \a query specifies which property is queried.

    \sa Qt::ImQueryProperty QInputMethodEvent QInputContext
*/
QVariant QWidget::inputMethodQuery(Qt::InputMethodQuery query) const
{
    switch(query) {
    case Qt::ImMicroFocus:
        return QRect(width()/2, 0, 1, height());
    case Qt::ImFont:
        return font();
    default:
        return QVariant();
    }
}

#ifndef QT_NO_DRAGANDDROP

/*!
    \fn void QWidget::dragEnterEvent(QDragEnterEvent *event)

    This event handler is called when a drag is in progress and the
    mouse enters this widget. The event is passed in the \a event parameter.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QTextDrag, QImageDrag, QDragEnterEvent
*/
void QWidget::dragEnterEvent(QDragEnterEvent *)
{
}

/*!
    \fn void QWidget::dragMoveEvent(QDragMoveEvent *event)

    This event handler is called if a drag is in progress, and when
    any of the following conditions occurs: the cursor enters this widget,
    the cursor moves within this widget, or a modifier key is pressed on
    the keyboard while this widget has the focus. The event is passed
    in the \a event parameter.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QTextDrag, QImageDrag, QDragMoveEvent
*/
void QWidget::dragMoveEvent(QDragMoveEvent *)
{
}

/*!
    \fn void QWidget::dragLeaveEvent(QDragLeaveEvent *event)

    This event handler is called when a drag is in progress and the
    mouse leaves this widget. The event is passed in the \a event
    parameter.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QTextDrag, QImageDrag, QDragLeaveEvent
*/
void QWidget::dragLeaveEvent(QDragLeaveEvent *)
{
}

/*!
    \fn void QWidget::dropEvent(QDropEvent *event)

    This event handler is called when the drag is dropped on this
    widget which are passed in the \a event parameter.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QTextDrag, QImageDrag, QDropEvent
*/
void QWidget::dropEvent(QDropEvent *)
{
}

#endif // QT_NO_DRAGANDDROP

/*!
    \fn void QWidget::showEvent(QShowEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget show events which are passed in the \a event parameter.

    Non-spontaneous show events are sent to widgets immediately
    before they are shown. The spontaneous show events of windows are
    delivered afterwards.

    Note: A widget receives spontaneous show and hide events when its
    mapping status is changed by the window system, e.g. a spontaneous
    hide event when the user minimizes the window, and a spontaneous
    show event when the window is restored again. After receiving a
    spontaneous hide event, a widget is still considered visible in
    the sense of isVisible().

    \sa visible, event(), QShowEvent
*/
void QWidget::showEvent(QShowEvent *)
{
}

/*!
    \fn void QWidget::hideEvent(QHideEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget hide events. The event is passed in the \a event parameter.

    Hide events are sent to widgets immediately after they have been
    hidden.

    Note: A widget receives spontaneous show and hide events when its
    mapping status is changed by the window system, e.g. a spontaneous
    hide event when the user minimizes the window, and a spontaneous
    show event when the window is restored again. After receiving a
    spontaneous hide event, a widget is still considered visible in
    the sense of isVisible().

    \sa visible, event(), QHideEvent
*/
void QWidget::hideEvent(QHideEvent *)
{
}

/*
    \fn QWidget::x11Event(MSG *)

    This special event handler can be reimplemented in a subclass to
    receive native X11 events.

    In your reimplementation of this function, if you want to stop the
    event being handled by Qt, return true. If you return false, this
    native event is passed back to Qt, which translates the event into
    a Qt event and sends it to the widget.

    \warning This function is not portable.

    \sa QApplication::x11EventFilter()
*/


#if defined(Q_WS_MAC)

/*!
    \fn bool QWidget::macEvent(EventHandlerCallRef caller, EventRef event)

    This special event handler can be reimplemented in a subclass to
    receive native Macintosh events which are passed from the \a
    caller with the event details in the \a event parameter.

    In your reimplementation of this function, if you want to stop the
    event being handled by Qt, return true. If you return false, this
    native event is passed back to Qt, which translates the event into
    a Qt event and sends it to the widget.

    \warning This function is not portable.

    \sa QApplication::macEventFilter()
*/

bool QWidget::macEvent(EventHandlerCallRef, EventRef)
{
    return false;
}

#endif
#if defined(Q_WS_WIN)

/*!
    This special event handler can be reimplemented in a subclass to
    receive native Windows events which are passed in the \a message
    parameter.

    In your reimplementation of this function, if you want to stop the
    event being handled by Qt, return true and set \a result to the value
    that the window procedure should return. If you return false, this
    native event is passed back to Qt, which translates the event into
    a Qt event and sends it to the widget.

    \warning This function is not portable.

    \sa QApplication::winEventFilter()
*/
bool QWidget::winEvent(MSG *message, long *result)
{
    Q_UNUSED(message);
    Q_UNUSED(result);
    return false;
}

#endif
#if defined(Q_WS_X11)

/*!
    \fn bool QWidget::x11Event(XEvent *event)

    This special event handler can be reimplemented in a subclass to
    receive native X11 events which are passed in the \a event
    parameter.

    In your reimplementation of this function, if you want to stop the
    event being handled by Qt, return true. If you return false, this
    native event is passed back to Qt, which translates the event into
    a Qt event and sends it to the widget.

    \warning This function is not portable.

    \sa QApplication::x11EventFilter()
*/
bool QWidget::x11Event(XEvent *)
{
    return false;
}

#endif
#if defined(Q_WS_QWS)

/*!
    \fn bool QWidget::qwsEvent(QWSEvent *event)

    This special event handler can be reimplemented in a subclass to
    receive native Qt/Embedded events which are passed in the \a event
    parameter.

    In your reimplementation of this function, if you want to stop the
    event being handled by Qt, return true. If you return false, this
    native event is passed back to Qt, which translates the event into
    a Qt event and sends it to the widget.

    \warning This function is not portable.

    \sa QApplication::qwsEventFilter()
*/
bool QWidget::qwsEvent(QWSEvent *)
{
    return false;
}

#endif


/*!
    Ensures delayed initialization of a widget and its children.

    This function will be called \e after a widget has been fully
    created and \e before it is shown the very first time.

    Polishing is useful for final initialization which depends on
    having an instantiated widget. This is something a constructor
    cannot guarantee since the initialization of the subclasses might
    not be finished.

    For widgets, this function makes sure the widget has a proper font
    and palette and QApplication::polish() has been called.

    If you need to change some settings when a widget is polished,
    use the Polish event delivered to event().

    \sa event(), QApplication::polish()
*/
void QWidget::ensurePolished() const
{
    Q_D(const QWidget);

    const QMetaObject *m = metaObject();
    if (m == d->polished)
        return;
    d->polished = m;

    QEvent e(QEvent::Polish);
    QCoreApplication::sendEvent(const_cast<QWidget *>(this), &e);

    // polish children after 'this'
    QList<QObject*> children = d->children;
    for (int i = 0; i < children.size(); ++i) {
        QObject *o = children.at(i);
        if(!o->isWidgetType())
            continue;
        if (QWidget *w = qobject_cast<QWidget *>(o))
            w->ensurePolished();
    }

    if (d->parent && d->sendChildEvents) {
        QChildEvent e(QEvent::ChildPolished, const_cast<QWidget *>(this));
        QCoreApplication::sendEvent(d->parent, &e);
    }
}

/*!
    Returns the mask currently set on a widget. If no mask is set the
    return value will be an empty region.

    \sa setMask(), clearMask(), QRegion::isEmpty()
*/
QRegion QWidget::mask() const
{
    Q_D(const QWidget);
    return d->extra ? d->extra->mask : QRegion();
}

#ifndef QT_NO_LAYOUT
/*!
    Returns the layout engine that manages the geometry of this
    widget's children.

    If the widget does not have a layout, layout() returns 0.

    \sa  sizePolicy()
*/
QLayout* QWidget::layout() const
{
    return d_func()->layout;
}
#endif


/*!
    \property QWidget::sizePolicy
    \brief the default layout behavior of the widget

    If there is a QLayout that manages this widget's children, the
    size policy specified by that layout is used. If there is no such
    QLayout, the result of this function is used.

    The default policy is Preferred/Preferred, which means that the
    widget can be freely resized, but prefers to be the size
    sizeHint() returns. Button-like widgets set the size policy to
    specify that they may stretch horizontally, but are fixed
    vertically. The same applies to lineedit controls (such as
    QLineEdit, QSpinBox or an editable QComboBox) and other
    horizontally orientated widgets (such as QProgressBar).
    QToolButton's are normally square, so they allow growth in both
    directions. Widgets that support different directions (such as
    QSlider, QScrollBar or QHeader) specify stretching in the
    respective direction only. Widgets that can provide scrollbars
    (usually subclasses of QScrollView) tend to specify that they can
    use additional space, and that they can make do with less than
    sizeHint().

    \sa sizeHint() QLayout QSizePolicy updateGeometry()
*/
QSizePolicy QWidget::sizePolicy() const
{
    Q_D(const QWidget);
    // GCC 2.95.x on FreeBSD can't handle this as a turnary operator.
    if (d->extra)
        return d->extra->size_policy;
    return QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void QWidget::setSizePolicy(QSizePolicy policy)
{
    setAttribute(Qt::WA_WState_OwnSizePolicy);
    if (policy == sizePolicy())
        return;

    Q_D(QWidget);
    d->createExtra();
    d->extra->size_policy = policy;
    updateGeometry();
}

/*!
    \fn void QWidget::setSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical)
    \overload

    Sets the size policy of the widget to \a horizontal and \a
    vertical, with standard stretch and no height-for-width.

    \sa QSizePolicy::QSizePolicy()
*/

/*!
    Returns the preferred height for this widget, given the width \a w.

    If this widget has a layout, the default implementation returns
    the layout's preferred height.  if there is no layout, the default
    implementation returns -1 indicating that the preferred height
    does not depend on the width.
*/

int QWidget::heightForWidth(int w) const
{
#ifndef QT_NO_LAYOUT
    if (layout() && layout()->hasHeightForWidth())
        return layout()->totalHeightForWidth(w);
#endif
    return -1;
}

/*!
    Returns the visible child widget at pixel position (\a{x}, \a{y})
    in the widget's own coordinate system. If there is no visible
    child widget at the specified position, returns 0.
*/

/*!
    \overload

    Returns the visible child widget at point \a p in the widget's own
    coordinate system.
*/

QWidget *QWidget::childAt(const QPoint &p) const
{
    if (!rect().contains(p))
        return 0;
    Q_D(const QWidget);
    for (int i = d->children.size(); i > 0 ;) {
        --i;
        QWidget *w = qobject_cast<QWidget *>(d->children.at(i));
        if (w && !w->isWindow() && !w->isExplicitlyHidden() && w->geometry().contains(p)) {
            if (QWidget *t = w->childAt(p.x() - w->x(), p.y() - w->y()))
                return t;
            // if WMouseNoMask is set the widget mask is ignored, if
            // the widget has no mask then the WMouseNoMask flag has no
            // effect
            if (w->testAttribute(Qt::WA_MouseNoMask) || w->mask().contains(p)
                || w->mask().isEmpty())
                return w;
        }
    }
    return 0;
}


/*!
    Notifies the layout system that this widget has changed and may
    need to change geometry.

    Call this function if the sizeHint() or sizePolicy() have changed.

    For explicitly hidden widgets, updateGeometry() is a no-op. The
    layout system will be notified as soon as the widget is shown.
*/

void QWidget::updateGeometry()
{
#ifndef QT_NO_LAYOUT
    if (!isWindow() && !isExplicitlyHidden() && parentWidget()) {
        if (parentWidget()->d_func()->layout)
            parentWidget()->d_func()->layout->update();
        else if (parentWidget()->isVisible())
            QApplication::postEvent(parentWidget(), new QEvent(QEvent::LayoutRequest));
    }
#endif
}

/*! \property QWidget::windowFlags

    Window flags are a combination of a window type (e.g. Qt::Widget,
    or Qt::Window) and zero or more hints to the window system
    (e.g. Qt::FramelessWindowHint).

    \sa windowType()
 */


void QWidget::setWindowFlags(Qt::WindowFlags f)
{
    if (!(data->window_flags & Qt::Window) && !(f & Qt::Window))
        data->window_flags = f;
    else
        setParent(parentWidget(), f);
}

void QWidget::overrideWindowFlags(Qt::WindowFlags f)
{
    data->window_flags = f;
}

/*! \fn Qt::WindowType windowType() const

    Returns the window type of this widget. This is identical to
    windowFlags() & Qt::WindowType_Mask.

    \sa windowFlags
*/

/*!

    Sets the parent of the widget to \a parent. The widget is moved
    to position (0,0) in its new parent.

    If the new parent widget is in a different window, the
    reparented widget and its children are appended to the end of the
    \link setFocusPolicy() tab chain \endlink of the new parent
    widget, in the same internal order as before. If one of the moved
    widgets had keyboard focus, setParent() calls clearFocus() for that
    widget.

    If the new parent widget is in the same window as the
    old parent, setting the parent doesn't change the tab order or
    keyboard focus.

    If the "new" parent widget is the old parent widget, this function
    does nothing.

    \warning It is extremely unlikely that you will ever need this
    function. If you have a widget that changes its content
    dynamically, it is far easier to use \l QWidgetStack or \l
    QWizard.

*/
void QWidget::setParent(QWidget *parent)
{
    if (parent == parentWidget())
        return;
    setParent((QWidget*)parent, windowFlags() & ~Qt::WindowType_Mask);
}

/*!
    \overload

    This function also takes widget flags, \a f as an argument.

    \sa getWFlags()
*/

void QWidget::setParent(QWidget *parent, Qt::WFlags f)
{
    Q_D(QWidget);
    bool resized = testAttribute(Qt::WA_Resized);
    QWidget *oldtlw = window();
    d->setParent_sys(parent, f);
    d->reparentFocusWidgets(oldtlw);
    setAttribute(Qt::WA_Resized, resized);
    d->resolveFont();
    d->resolvePalette();
    d->resolveLayoutDirection();
    if (parent && d->sendChildEvents && d->polished) {
        QChildEvent e(QEvent::ChildPolished, this);
        QCoreApplication::sendEvent(parent, &e);
    }
    QEvent e(QEvent::ParentChange);
    QApplication::sendEvent(this, &e);
}

/*!
    Repaints the widget directly by calling paintEvent() immediately,
    unless updates are disabled or the widget is hidden.

    We suggest only using repaint() if you need an immediate repaint,
    for example during animation. In almost all circumstances update()
    is better, as it permits Qt to optimize for speed and minimize
    flicker.

    \warning If you call repaint() in a function which may itself be
    called from paintEvent(), you may get infinite recursion. The
    update() function never causes recursion.

    \sa update(), paintEvent(), setUpdatesEnabled()
*/

void QWidget::repaint()
{
    Q_D(QWidget);
#if defined(Q_WS_X11)
//     d->removePendingPaintEvents(); // ### this is far too slow to go in
#endif
    repaint(d->clipRect());
}

/*! \overload

    This version repaints a rectangle (\a x, \a y, \a w, \a h) inside
    the widget.

    If \a w is negative, it is replaced with \c{width() - x}, and if
    \a h is negative, it is replaced width \c{height() - y}.
*/
void QWidget::repaint(int x, int y, int w, int h)
{
    if (x > data->crect.width() || y > data->crect.height())
        return;

    Q_D(QWidget);
    if (w < 0)
        w = data->crect.width()  - x;
    if (h < 0)
        h = data->crect.height() - y;
    repaint(d->clipRect().intersect(QRect(x, y, w, h)));
}

/*! \overload

    This version repaints a rectangle \a r inside the widget.
*/
void QWidget::repaint(const QRect &r)
{
    Q_D(QWidget);
    repaint(QRegion(d->clipRect().intersect(r)));
}

/*! \fn void QWidget::repaint(const QRegion &rgn)
    \overload

    This version repaints a region \a rgn inside the widget.
*/

/*! \fn void QWidget::update()
    Updates the widget unless updates are disabled or the widget is
    hidden.

    This function does not cause an immediate repaint; instead it
    schedules a paint event for processing when Qt returns to the main
    event loop. This permits Qt to optimize for more speed and less
    flicker than a call to repaint() does.

    Calling update() several times normally results in just one
    paintEvent() call.

    Qt normally erases the widget's area before the paintEvent() call.
    If the \c Qt::WRepaintNoErase widget flag is set, the widget is
    responsible for painting all its pixels itself.

    \sa repaint() paintEvent(), setUpdatesEnabled()
*/

/*! \fn void QWidget::update(int x, int y, int w, int h)
    \overload

    This version updates a rectangle (\a x, \a y, \a w, \a h) inside
    the widget.
*/


/*!
    \fn void QWidget::update(const QRect &r)
    \overload

    This version updates a rectangle \a r inside the widget.
*/

/*! \fn void QWidget::update(const QRegion &rgn)
    \overload

    This version repaints a region \a rgn inside the widget.
*/

#ifdef QT3_SUPPORT
/*!
    Clear the rectangle at point (\a x, \a y) of width \a w and height
    \a h.

    \warning This is best done in a paintEvent().
*/
void QWidget::erase_helper(int x, int y, int w, int h)
{
    if (testAttribute(Qt::WA_NoSystemBackground) || testAttribute(Qt::WA_UpdatesDisabled) ||  !testAttribute(Qt::WA_WState_Visible))
        return;
    if (w < 0)
        w = data->crect.width()  - x;
    if (h < 0)
        h = data->crect.height() - y;
    if (w != 0 && h != 0) {
        QPainter p(this);
        p.eraseRect(QRect(x, y, w, h));
    }
}

/*!
    \overload

    Clear the given region, \a rgn.

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your erasing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/
void QWidget::erase(const QRegion& rgn)
{
    if (testAttribute(Qt::WA_NoSystemBackground) || testAttribute(Qt::WA_UpdatesDisabled) || !testAttribute(Qt::WA_WState_Visible))
        return;

    QPainter p(this);
    p.setClipRegion(rgn);
    p.eraseRect(rgn.boundingRect());
}

void QWidget::drawText_helper(int x, int y, const QString &str)
{
    if(!testAttribute(Qt::WA_WState_Visible))
        return;
    QPainter paint(this);
    paint.drawText(x, y, str);
}


/*!
    Closes the widget.

    Use the no-argument overload instead.
*/
bool QWidget::close(bool alsoDelete)
{
    QPointer<QWidget> that = this;
    bool accepted = close();
    if (alsoDelete && accepted && that)
        deleteLater();
    return accepted;
}

void QWidget::setIcon(const QPixmap &i)
{
    setWindowIcon(i);
}

/*!
    Return's the widget's icon.

    Use windowIconText() instead().
*/
const QPixmap *QWidget::icon() const
{
    Q_D(const QWidget);
    return (d->extra && d->extra->topextra) ? d->extra->topextra->iconPixmap : 0;
}

#endif // QT3_SUPPORT

/*!
    Sets the attribute \a attribute on this widget if \a on is true;
    otherwise clears the attribute.

    \sa testAttribute()
*/
void QWidget::setAttribute(Qt::WidgetAttribute attribute, bool on)
{
    Q_D(QWidget);
    Q_ASSERT_X(sizeof(d->high_attributes)*8 >= (Qt::WA_AttributeCount - sizeof(uint)*8),
               "QWidget::setAttribute(WidgetAttribute, bool)",
               "QWidgetPrivate::high_attributes[] too small to contain all attributes in WidgetAttribute");
    if (attribute < int(8*sizeof(uint))) {
        if (on)
            data->widget_attributes |= (1<<attribute);
        else
            data->widget_attributes &= ~(1<<attribute);
    } else {
        const int x = attribute - 8*sizeof(uint);
        const int int_off = x / (8*sizeof(uint));
        if (on)
            d->high_attributes[int_off] |= (1<<(x-(int_off*8*sizeof(uint))));
        else
            d->high_attributes[int_off] &= ~(1<<(x-(int_off*8*sizeof(uint))));
    }
    switch (attribute) {
    case Qt::WA_NoChildEventsForParent:
        d->sendChildEvents = !on;
        break;
    case Qt::WA_MacMetalStyle:
#ifdef Q_WS_MAC
        extern void qt_mac_update_metal_style(QWidget*); //qwidget_mac.cpp
        qt_mac_update_metal_style(this);
#endif
        break;
    case Qt::WA_MouseTracking: {
        QEvent e(QEvent::MouseTrackingChange);
        QApplication::sendEvent(this, &e);
        break; }
    case Qt::WA_NoSystemBackground:
    case Qt::WA_UpdatesDisabled:
        d->updateSystemBackground();
        break;
    case Qt::WA_ContentsPropagated:
        if (isVisible())
            d->updatePropagatedBackground();
    default:
        break;
    }
}

/*! \fn bool QWidget::testAttribute(Qt::WidgetAttribute attribute) const

  Returns true if attribute \a attribute is set on this widget;
  otherwise returns false.

  \sa setAttribute()
 */
bool QWidget::testAttribute_helper(Qt::WidgetAttribute attribute) const
{
    Q_D(const QWidget);
    const int x = attribute - 8*sizeof(uint);
    const int int_off = x / (8*sizeof(uint));
    return (d->high_attributes[int_off] & (1<<(x-(int_off*8*sizeof(uint)))));
}

/*!
  \property QWidget::windowOpacity

  \brief The level of opacity for the window.

  The valid range of opacity is from 1.0 (completely opaque) to
  0.0 (completely transparent).

  By default the value of this property is 1.0.

  This feature is only present on Mac OS X and Windows 2000 and up.

  \warning Changing this property from opaque to transparent might issue a
  paint event that needs to be processed before the window is displayed
  correctly. This affects mainly the use of QPixmap::grabWindow(). Also note
  that semi-transparent windows update and resize significantly slower than
  opaque windows.
*/

/*!
  \property QWidget::windowModified

  \brief If the window is in a modified state.

  A modified window is a window whose content has changed but has not
  been saved to disk.  This flag will have different effects varied by
  the platform. On Mac OS X the close button will have a modified
  look, on Windows will have an '*' (asterisk) appended to its
  titlebar text.

  This feature is only present on Mac OS X and Windows.
*/

/*!
  \property QWidget::toolTip

  \brief the widget's tooltip

  \sa QToolTip statusTip whatsThis
*/
void QWidget::setToolTip(const QString &s)
{
    Q_D(QWidget);
    d->toolTip = s;
}

QString QWidget::toolTip() const
{
    Q_D(const QWidget);
    return d->toolTip;
}

/*!
  \property QWidget::statusTip

  \brief the widget's status tip

  \sa toolTip whatsThis
*/
void QWidget::setStatusTip(const QString &s)
{
    Q_D(QWidget);
    d->statusTip = s;
}

QString QWidget::statusTip() const
{
    Q_D(const QWidget);
    return d->statusTip;
}

/*!
  \property QWidget::whatsThis

  \brief the widget's What's This help text.

  \sa QWhatsThis QWidget::toolTip QWidget::statusTip
*/
void QWidget::setWhatsThis(const QString &s)
{
    Q_D(QWidget);
    d->whatsThis = s;
}

QString QWidget::whatsThis() const
{
    Q_D(const QWidget);
    return d->whatsThis;
}

#ifndef QT_NO_ACCESSIBILITY
/*!
  \property QWidget::accessibleName

  \brief the widget's name as seen by assistive technologies

  \sa QAccessibleInterface::text
*/
void QWidget::setAccessibleName(const QString &name)
{
    Q_D(QWidget);
    d->accessibleName = name;
}

QString QWidget::accessibleName() const
{
    Q_D(const QWidget);
    return d->accessibleName;
}

/*!
  \property QWidget::accessibleDescription

  \brief the widget's description as seen by assistive technologies

  \sa QAccessibleInterface::text
*/
void QWidget::setAccessibleDescription(const QString &description)
{
    Q_D(QWidget);
    d->accessibleDescription = description;
}

QString QWidget::accessibleDescription() const
{
    Q_D(const QWidget);
    return d->accessibleDescription;
}
#endif

/*!
    Adds a shortcut to Qt's shortcut system that watches for the given
    \a key sequence in the given \a context. If the \a context is not
    \c OnApplication, the shortcut is local to this widget; otherwise
    it applies to the application as a whole.

    If the same \a key sequence has been grabbed by several widgets,
    when the \a key sequence occurs a QEvent::Shortcut event is sent
    to all the widgets to which it applies in a non-deterministic
    order, but with the ``ambiguous'' flag set to true.

    \warning You should not normally need to use this function;
    instead create \l{QAction}s with the shortcut key sequences you
    require (if you also want equivalent menu options and toolbar
    buttons), or create \l{QShortcut}s if you just need key sequences.
    Both QAction and QShortcut handle all the event filtering for you,
    and provide signals which are triggered when the user triggers the
    key sequence, so are much easier to use than this low-level
    function.

    \sa releaseShortcut() setShortcutEnabled()
*/
int QWidget::grabShortcut(const QKeySequence &key, Qt::ShortcutContext context)
{
    Q_ASSERT(qApp);
    if (key.isEmpty())
        return 0;
    setAttribute(Qt::WA_GrabbedShortcut);
    return qApp->d_func()->shortcutMap.addShortcut(this, key, context);
}


/*!
    Removes the shortcut with the given \a id from Qt's shortcut
    system. The widget will no longer receive QEvent::Shortcut events
    for the shortcut's key sequence (unless it has other shortcuts
    with the same key sequence).

    \warning You should not normally need to use this function since
    Qt's shortcut system removes shortcuts automatically when their
    parent widget is destroyed. It is best to use QAction or
    QShortcut to handle shortcuts, since they are easier to use than
    this low-level function. Note also that this is an expensive
    operation.

    \sa grabShortcut() setShortcutEnabled()
*/
void QWidget::releaseShortcut(int id)
{
    Q_ASSERT(qApp);
    if (id)
        qApp->d_func()->shortcutMap.removeShortcut(id, this, 0);
}

/*!
    If \a enable is true, the shortcut with the given \a id is
    enabled; otherwise the shortcut is disabled.

    \warning You should not normally need to use this function since
    Qt's shortcut system enables/disables shortcuts automatically as
    widgets become hidden/visible and gain or lose focus. It is best
    to use QAction or QShortcut to handle shortcuts, since they are
    easier to use than this low-level function.

    \sa grabShortcut() releaseShortcut()
*/
void QWidget::setShortcutEnabled(int id, bool enable)
{
    Q_ASSERT(qApp);
    if (id)
        qApp->d_func()->shortcutMap.setShortcutEnabled(enable, id, this, 0);
}


void QWidget::updateMicroFocus()
{
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
    QInputContext *ic = inputContext();
    if (ic)
        ic->update();
#endif
#ifndef QT_NO_ACCESSIBILITY
    // ##### is this correct
    QAccessible::updateAccessibility(this, 0, QAccessible::StateChanged);
#endif
}


#if defined (Q_WS_WIN)
/*!
    Returns the window system handle of the widget, for low-level
    access. Using this function is not portable.

    An HDC aquired with getDC() has to be released with releaseDC().

    \warning Using this function is not portable.
*/
HDC QWidget::getDC() const
{
    Q_D(const QWidget);
    return (HDC) d->hd;
}

/*!
    Releases the HDC \a hdc aquired by a previous call to getDC().

    \warning Using this function is not portable.
*/
void QWidget::releaseDC(HDC hdc) const
{
    Q_D(const QWidget);
    Q_ASSERT(d->hd == hdc);
}
#else
/*!
    Returns the window system handle of the widget, for low-level
    access. Using this function is not portable.

    The HANDLE type varies with platform; see \c qwindowdefs.h for
    details.
*/
Qt::HANDLE QWidget::handle() const
{
    Q_D(const QWidget);
    return d->hd;
}
#endif


/*!
    Raises this widget to the top of the parent widget's stack.

    After this call the widget will be visually in front of any
    overlapping sibling widgets.

    \sa lower(), stackUnder()
*/

void QWidget::raise()
{
    Q_D(QWidget);
    QWidget *p = parentWidget();
    int from;
    if (p && (from = p->d_func()->children.indexOf(this)) >= 0)
        p->d_func()->children.move(from, p->d_func()->children.size() - 1);
    d->raise_sys();

    QEvent e(QEvent::ZOrderChange);
    QApplication::sendEvent(this, &e);
}

/*!
    Lowers the widget to the bottom of the parent widget's stack.

    After this call the widget will be visually behind (and therefore
    obscured by) any overlapping sibling widgets.

    \sa raise(), stackUnder()
*/

void QWidget::lower()
{
    Q_D(QWidget);
    QWidget *p = parentWidget();
    int from;
    if (p && (from = p->d_func()->children.indexOf(this)) >= 0)
        p->d_func()->children.move(from, 0);
    d->lower_sys();

    QEvent e(QEvent::ZOrderChange);
    QApplication::sendEvent(this, &e);
}


/*!
    Places the widget under \a w in the parent widget's stack.

    To make this work, the widget itself and \a w must be siblings.

    \sa raise(), lower()
*/
void QWidget::stackUnder(QWidget* w)
{
    Q_D(QWidget);
    QWidget *p = parentWidget();
    int from;
    int to;
    if (!w || isWindow() || p != w->parentWidget() || this == w)
        return;
    if (p && (to = p->d_func()->children.indexOf(w)) >= 0 && (from = p->d_func()->children.indexOf(this)) >= 0) {
        if (from < to)
            --to;
        p->d_func()->children.move(from, to);
    }
    d->stackUnder_sys(w);

    QEvent e(QEvent::ZOrderChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::styleChange(QStyle&) { }
void QWidget::enabledChange(bool) { }  // compat
void QWidget::paletteChange(const QPalette &) { }  // compat
void QWidget::fontChange(const QFont &) { }  // compat
void QWidget::windowActivationChange(bool) { }  // compat
void QWidget::languageChange() { }  // compat


/*!
    \property QWidget::visibleRect
    \brief holds the widget's visible rectangle

    \compat
*/

/*!
    \enum QWidget::BackgroundOrigin

    \compat

    \value WidgetOrigin
    \value ParentOrigin
    \value WindowOrigin
    \value AncestorOrigin

*/

/*!
    \fn bool QWidget::isVisibleToTLW() const

    Use isVisible() instead.
*/

/*!
    \fn void QWidget::iconify()

    Use showMinimized() instead.
*/

/*!
    \fn void QWidget::constPolish() const

    Use ensurePolished() instead.
*/

/*!
    \fn void QWidget::reparent(QWidget *parent, Qt::WFlags f, const QPoint &p, bool showIt)

    Use setParent() to change the parent or the widget's widget flags;
    use move() to move the widget, and use show() to show the widget.
*/

/*!
    \fn void QWidget::reparent(QWidget *parent, const QPoint &p, bool showIt)

    Use setParent() to change the parent; use move() to move the
    widget, and use show() to show the widget.
*/

/*!
    \fn void QWidget::recreate(QWidget *parent, Qt::WFlags f, const QPoint & p, bool showIt)

    Use setParent() to change the parent or the widget's widget flags;
    use move() to move the widget, and use show() to show the widget.
*/

/*!
    \fn bool QWidget::hasMouse() const

    Use testAttribute(Qt::WA_UnderMouse) instead.
*/

/*!
    \fn bool QWidget::ownCursor() const

    Use testAttribute(Qt::WA_SetCursor) instead.
*/

/*!
    \fn bool QWidget::ownFont() const

    Use testAttribute(Qt::WA_SetFont) instead.
*/

/*!
    \fn void QWidget::unsetFont()

    Use setFont(QFont()) instead.
*/

/*!
    \fn bool QWidget::ownPalette() const

    Use testAttribute(Qt::WA_SetPalette) instead.
*/

/*!
    \fn void QWidget::unsetPalette()

    Use setPalette(QPalette()) instead.
*/

/*!
    \fn void QWidget::setEraseColor(const QColor &color)

    Use the palette instead.

    \oldcode
    widget->setEraseColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setErasePixmap(const QPixmap &pixmap)

    Use the palette instead.

    \oldcode
    widget->setErasePixmap(pixmap);
    \newcode
    QPalette palette;
    palette.setBrush(widget->backgroundRole(), QBrush(pixmap));
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setPaletteForegroundColor(const QColor &color)

    Use the palette directly.

    \oldcode
    widget->setPaletteForegroundColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->foregroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setPaletteBackgroundColor(const QColor &color)

    Use the palette directly.

    \oldcode
    widget->setPaletteBackgroundColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setPaletteBackgroundPixmap(const QPixmap &pixmap)

    Use the palette directly.

    \oldcode
    widget->setPaletteBackgroundPixmap(pixmap);
    \newcode
    QPalette palette;
    palette.setBrush(widget->backgroundRole(), QBrush(pixmap));
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setBackgroundPixmap(const QPixmap &pixmap)

    Use the palette instead.

    \oldcode
    widget->setBackgroundPixmap(pixmap);
    \newcode
    QPalette palette;
    palette.setBrush(widget->backgroundRole(), QBrush(pixmap));
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setBackgroundColor(const QColor &color)

    Use the palette instead.

    \oldcode
    widget->setBackgroundColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn QColorGroup QWidget::colorGroup() const

    Use QColorGroup(palette()) instead.
*/

/*!
    \fn QWidget *QWidget::parentWidget(bool sameWindow) const

    Use the no-argument overload instead.
*/

/*!
    \fn void QWidget::setKeyCompression(bool b)

    Use setAttribute(Qt::WA_KeyCompression, b) instead.
*/

/*!
    \fn void QWidget::setFont(const QFont &f, bool b)

    Use the single-argument overload instead.
*/

/*!
    \fn void QWidget::setPalette(const QPalette &p, bool b)

    Use the single-argument overload instead.
*/

/*!
    \fn void QWidget::setBackgroundOrigin(BackgroundOrigin background)

    \obsolete
*/

/*!
    \fn BackgroundOrigin QWidget::backgroundOrigin() const

    \obsolete

    Always returns \c WindowOrigin.
*/

/*!
    \fn QPoint QWidget::backgroundOffset() const

    \obsolete

    Always returns QPoint().
*/

/*!
    \fn void QWidget::repaint(bool b)

    Use the no-argument overload instead.
*/

/*!
    \fn void QWidget::repaint(int x, int y, int w, int h, bool b)

    Use the four-argument overload instead.
*/

/*!
    \fn void QWidget::repaint(const QRect &r, bool b)

    Use the single rect-argument overload instead.
*/

/*!
    \fn void QWidget::repaint(const QRegion &rgn, bool b)

    Use the single region-argument overload instead.
*/

/*!
    \fn void QWidget::erase()

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your erasing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::erase(int x, int y, int w, int h)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your erasing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::erase(const QRect &rect)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your erasing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::drawText(const QPoint &p, const QString &s)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your drawing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::drawText(int x, int y, const QString &s)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your drawing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn QWidget *QWidget::childAt(const QPoint &p, bool includeThis) const

    Use the single point argument overload instead.
*/

/*!
    \fn void QWidget::setCaption(const QString &c)

    Use setWindowTitle() instead.
*/

/*!
    \fn void QWidget::setIcon(const QPixmap &i)

    Use setWindowIcon() instead.
*/

/*!
    \fn void QWidget::setIconText(const QString &it)

    Use setWindowIconText() instead.
*/

/*!
    \fn QString QWidget::caption() const

    Use windowTitle() instead.
*/

/*!
    \fn QString QWidget::iconText() const

    Use windowIconText() instead().
*/

/*!
    \fn bool QWidget::isTopLevel() const
    \obsolete

    Use isWindow() instead.
*/

/*!
    \fn bool QWidget::isRightToLeft() const
    \internal
*/

/*!
    \fn bool QWidget::isLeftToRight() const
    \internal
*/

/*!
    \fn void QWidget::setInputMethodEnabled(bool enabled)

    Use setAttribute(Qt::WA_InputMethodEnabled, \a enabled) instead.
*/

/*!
    \fn bool QWidget::isInputMethodEnabled() const

    Use testAttribute(Qt::WA_InputMethodEnabled) instead.
*/

/*!
    \fn void QWidget::setActiveWindow()

    Use activateWindow() instead.
*/

/*!
    \fn bool QWidget::isHidden() const

    Use isExplicitlyHidden() instead.
*/

/*!
    \fn bool QWidget::isShown() const

    Use !isExplicitlyHidden() instead (notice the exclamation mark).
*/

/*!
    \fn bool QWidget::isDialog() const

    Use windowType() == Qt::Dialog instead.
*/

/*!
    \fn bool QWidget::isPopup() const

    Use windowType() == Qt::Popup instead.
*/

/*!
    \fn bool QWidget::isDesktop() const

    Use windowType() == Qt::Desktop instead.
*/

/*!
    \fn void QWidget::polish()

    Use ensurePolished() instead.
*/

/*!
    \fn QWidget *QWidget::childAt(int x, int y, bool includeThis) const

    Use the childAt() overload that doesn't have an \a includeThis parameter.

    \oldcode
        return widget->childAt(x, y, true);
    \newcode
        QWidget *child = widget->childAt(x, y, true);
        if (child)
            return child;
        if (widget->rect().contains(x, y))
            return widget;
    \endcode
*/

#include "moc_qwidget.cpp"
