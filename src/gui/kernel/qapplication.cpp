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

#include "qaccessible.h"
#include "qapplication.h"
#include "qcleanuphandler.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qdir.h"
#include "qdragobject.h"
#include "qevent.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qguieventloop.h"
#include "qhash.h"
#include "qhash.h"
#include "qlayout.h"
#include "qmessagebox.h"
#include "qsessionmanager.h"
#include "qstyle.h"
#include "qstylefactory.h"
#include "qtextcodec.h"
#include "qtranslator.h"
#include "qvariant.h"
#include "qwidget.h"

#include <private/qeventloop_p.h>


#include <qthread.h>
#include <private/qthread_p.h>

#ifdef Q_WS_WIN
#include "qinputcontext_p.h"
#endif
#include <private/qfontdata_p.h>

#include <stdlib.h>

extern void qt_call_post_routines();

#include "qapplication_p.h"
#include "qwidget_p.h"
#define d d_func()
#define q q_func()


QApplication::Type qt_appType=QApplication::Tty;

QApplicationPrivate::QApplicationPrivate(int &argc, char **argv, QApplication::Type type)
    : QCoreApplicationPrivate(argc, argv)
{
    qt_appType = type;

#ifndef QT_NO_SESSIONMANAGER
    is_session_restored = false;
#endif

#ifdef QT_COMPAT
    qt_compat_used = 0;
    qt_compat_resolved = 0;
    qt_tryAccelEvent = 0;
    qt_tryComposeUnicode = 0;
    qt_dispatchAccelEvent = 0;
#endif
}

void QApplicationPrivate::createEventLoop()
{ eventLoop = q->type() != QApplication::Tty ? new QGuiEventLoop(q) : new QEventLoop(q); }



/*!
  \class QApplication qapplication.h
  \brief The QApplication class manages the GUI application's control
  flow and main settings.

  \ingroup application
  \mainclass

  It contains the main event loop, where all events from the window
  system and other sources are processed and dispatched. It also
  handles the application's initialization and finalization, and
  provides session management. It also handles most system-wide and
  application-wide settings.

  For any GUI application that uses Qt, there is precisely one
  QApplication object, no matter whether the application has 0, 1, 2
  or more windows at any time.

  The QApplication object is accessible through the instance()
  function. (In earlier Qt versions the qApp global was used instead
  of instance().)

  QApplication's main areas of responsibility are:
  \list

  \i It initializes the application with the user's desktop settings
  such as palette(), font() and doubleClickInterval(). It keeps track
  of these properties in case the user changes the desktop globally, for
  example through some kind of control panel.

  \i It performs event handling, meaning that it receives events
  from the underlying window system and dispatches them to the relevant
  widgets. By using sendEvent() and postEvent() you can send your own
  events to widgets.

  \i It parses common command line arguments and sets its internal
  state accordingly. See the \link QApplication::QApplication()
  constructor documentation\endlink below for more details about this.

  \i It defines the application's look and feel, which is
  encapsulated in a QStyle object. This can be changed at runtime
  with setStyle().

  \i It specifies how the application is to allocate colors.
  See setColorSpec() for details.

  \i It provides localization of strings that are visible to the user
  via translate().

  \i It provides some magical objects like the desktop() and the
  clipboard().

  \i It knows about the application's windows. You can ask which
  widget is at a certain position using widgetAt(), get a list of
  topLevelWidgets() and closeAllWindows(), etc.

  \i It manages the application's mouse cursor handling,
  see setOverrideCursor()

  \i On the X window system, it provides functions to flush and sync
  the communication stream, see flushX() and syncX().

  \i It provides support for sophisticated \link
  session.html session management \endlink. This makes it possible
  for applications to terminate gracefully when the user logs out, to
  cancel a shutdown process if termination isn't possible and even to
  preserve the entire application's state for a future session. See
  isSessionRestored(), sessionId() and commitData() and saveState()
  for details.

  \endlist

  Since the QApplication object does so much initialization, it
  \e{must} be created before any other objects related to the user
  interface are created.

  Since it also deals with common command line arguments, it is
  usually a good idea to create it \e before any interpretation or
  modification of \c argv is done in the application itself. (Note
  also that for X11, setMainWidget() may change the main widget
  according to the \c -geometry option. To preserve this
  functionality, you must set your defaults before setMainWidget() and
  any overrides after.)

  \table
    \header \i21 Groups of functions
    \row
     \i System settings
     \i
        desktopSettingsAware(),
        setDesktopSettingsAware(),
        cursorFlashTime(),
        setCursorFlashTime(),
        doubleClickInterval(),
        setDoubleClickInterval(),
        wheelScrollLines(),
        setWheelScrollLines(),
        palette(),
        setPalette(),
        font(),
        setFont(),
        fontMetrics().

    \row
     \i Event handling
     \i
        exec(),
        processEvents(),
        enter_loop(),
        exit_loop(),
        exit(),
        quit().
        sendEvent(),
        postEvent(),
        sendPostedEvents(),
        removePostedEvents(),
        hasPendingEvents(),
        notify(),
        macEventFilter(),
        qwsEventFilter(),
        x11EventFilter(),
        x11ProcessEvent(),
        winEventFilter().

    \row
     \i GUI Styles
     \i
        style(),
        setStyle(),
        polish().

    \row
     \i Color usage
     \i
        colorSpec(),
        setColorSpec(),
        qwsSetCustomColors().

    \row
     \i Text handling
     \i
        installTranslator(),
        removeTranslator()
        translate().

    \row
     \i Widgets
     \i
        mainWidget(),
        setMainWidget(),
        allWidgets(),
        topLevelWidgets(),
        desktop(),
        activePopupWidget(),
        activeModalWidget(),
        clipboard(),
        focusWidget(),
        winFocus(),
        activeWindow(),
        widgetAt().

    \row
     \i Advanced cursor handling
     \i
        overrideCursor(),
        setOverrideCursor(),
        restoreOverrideCursor().

    \row
     \i X Window System synchronization
     \i
        flushX(),
        syncX().

    \row
     \i Session management
     \i
        isSessionRestored(),
        sessionId(),
        commitData(),
        saveState().

    \row
    \i Threading
    \i
        lock(), unlock(), locked(), tryLock(),

    \row
     \i Miscellaneous
     \i
        closeAllWindows(),
        startingUp(),
        closingDown(),
        type().
  \endtable

  \e {Non-GUI programs:} While Qt is not optimized or
  designed for writing non-GUI programs, it's possible to use
  \link tools.html some of its classes \endlink without creating a
  QApplication. This can be useful if you wish to share code between
  a non-GUI server and a GUI client.

  \headerfile qnamespace.h
  \headerfile qwindowdefs.h
  \headerfile qglobal.h
*/

/*!
    \enum QApplication::Type

    \value Tty a console application
    \value GuiClient a GUI client application
    \value GuiServer a GUI server application
*/

/*!
    \enum QApplication::ColorSpec

    \value NormalColor the default color allocation policy
    \value CustomColor the same as NormalColor for X11; allocates colors
    to a palette on demand under Windows
    \value ManyColor the right choice for applications that use thousands of
    colors

    See setColorSpec() for full details.
*/

/*!
    \fn QWidget *QApplication::topLevelAt(const QPoint &p)

    Returns the top level widget at the point \a p.
*/
/*!
    \fn QWidget *QApplication::topLevelAt(int x, int y)

    \overload

    Returns the top level widget at the point (\a{x}, \a{y}).
*/


/*
  The qt_init() and qt_cleanup() functions are implemented in the
  qapplication_xyz.cpp file.
*/

void qt_init(QApplicationPrivate *priv, int type
#ifdef Q_WS_X11
              , Display *display = 0, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0
#endif
   );
void qt_cleanup();
bool qt_tryModalHelper(QWidget *widget, QWidget **rettop);

Qt::MouseButtons QApplicationPrivate::mouse_buttons = Qt::NoButton;
Qt::KeyboardModifiers QApplicationPrivate::modifier_buttons = Qt::NoModifier;

QStyle *QApplicationPrivate::app_style = 0;        // default application style
bool qt_explicit_app_style = false; // style explicitly set by programmer

int QApplicationPrivate::app_cspec = QApplication::NormalColor;
#ifndef QT_NO_PALETTE
QPalette *QApplicationPrivate::app_pal = 0;        // default application palette
#endif
QFont *QApplicationPrivate::app_font = 0;        // default application font
bool qt_app_has_font = false;
QPixmap *QApplicationPrivate::app_icon = 0;
QWidget *QApplicationPrivate::main_widget = 0;        // main application widget
QWidget *QApplicationPrivate::focus_widget = 0;        // has keyboard input focus
QWidget *QApplicationPrivate::active_window = 0;        // toplevel with keyboard focus
bool QApplicationPrivate::obey_desktop_settings = true;        // use winsys resources
int QApplicationPrivate::cursor_flash_time = 1000;        // text caret flash time
int QApplicationPrivate::mouse_double_click_time = 400;        // mouse dbl click limit
#ifndef QT_NO_WHEELEVENT
int QApplicationPrivate::wheel_scroll_lines = 3;                // number of lines to scroll
#endif
bool qt_is_gui_used;
bool Q_GUI_EXPORT qt_tab_all_widgets = true;
QRect qt_maxWindowRect;
static int drag_time = 500;
static int drag_distance = 4;
static bool reverse_layout = false;
QSize QApplicationPrivate::app_strut = QSize(0,0); // no default application strut
bool QApplicationPrivate::animate_ui = true;
bool QApplicationPrivate::animate_menu = false;
bool QApplicationPrivate::fade_menu = false;
bool QApplicationPrivate::animate_combo = false;
bool QApplicationPrivate::animate_tooltip = false;
bool QApplicationPrivate::fade_tooltip = false;
bool QApplicationPrivate::animate_toolbox = false;
bool QApplicationPrivate::widgetCount = false;

#if defined(QT_TABLET_SUPPORT)
bool chokeMouse = false;
#endif


int qt_double_buffer_timer = 0;

void qt_setMaxWindowRect(const QRect& r)
{
    qt_maxWindowRect = r;
    // Re-resize any maximized windows
    QWidgetList l = QApplication::topLevelWidgets();
    for (int i = 0; i < l.size(); ++i) {
        QWidget *w = l.at(i);
        if (w->isVisible() && w->isMaximized()) {
            w->showMaximized();
            w->showNormal(); //#### flicker
        }
    }
}

/*!
    \fn qAddPostRoutine(QtCleanUpFunction ptr)

    \relates QApplication

    Adds a global routine that will be called from the QApplication
    destructor. This function is normally used to add cleanup routines
    for program-wide functionality.

    The function specified by \a ptr should take no arguments and should
    return nothing. For example:

    \code
    static int *global_ptr = 0;

    static void cleanup_ptr()
    {
        delete [] global_ptr;
        global_ptr = 0;
    }

    void init_ptr()
    {
        global_ptr = new int[100];        // allocate data
        qAddPostRoutine(cleanup_ptr);        // delete later
    }
    \endcode

    Note that for an application- or module-wide cleanup,
    qAddPostRoutine() is often not suitable. For example, if the
    program is split into dynamically loaded modules, the relevant
    module may be unloaded long before the QApplication destructor is
    called.

    For modules and libraries, using a reference-counted
    initialization manager or Qt's parent-child deletion mechanism may
    be better. Here is an example of a private class which uses the
    parent-child mechanism to call a cleanup function at the right
    time:

    \code
    class MyPrivateInitStuff: public QObject {
    private:
        MyPrivateInitStuff(QObject *parent): QObject(parent) {
            // initialization goes here
        }
        MyPrivateInitStuff *p;

    public:
        static MyPrivateInitStuff *initStuff(QObject *parent) {
            if (!p)
                p = new MyPrivateInitStuff(parent);
            return p;
        }

        ~MyPrivateInitStuff() {
            // cleanup (the "post routine") goes here
        }
    }
    \endcode

    By selecting the right parent widget/object, this can often be
    made to clean up the module's data at the exactly the right
    moment.
*/

// ######## move to QApplicationPrivate
// Default application palettes and fonts (per widget type)

typedef QHash<QByteArray, QPalette> PaletteHash;
Q_GLOBAL_STATIC(PaletteHash, app_palettes)
PaletteHash *qt_app_palettes_hash()
{
    return app_palettes();
}

typedef QHash<QByteArray, QFont> FontHash;
Q_GLOBAL_STATIC(FontHash, app_fonts)
FontHash *qt_app_fonts_hash()
{
    return app_fonts();
}

QWidgetList *QApplicationPrivate::popupWidgets = 0;        // has keyboard input focus

QDesktopWidget *qt_desktopWidget = 0;                // root window widgets
#ifndef QT_NO_CLIPBOARD
QClipboard              *qt_clipboard = 0;        // global clipboard object
#endif
QWidgetList * qt_modal_stack=0;                // stack of modal widgets

#ifndef QT_NO_PALETTE
QPalette *qt_std_pal = 0;

void qt_create_std_palette()
{
    if (qt_std_pal)
        delete qt_std_pal;

    QColor standardLightGray(192, 192, 192);
    QColor light(standardLightGray.light());
    QColor dark(standardLightGray.dark());
    qt_std_pal =
        new QPalette(Qt::black, standardLightGray, light, dark, Qt::gray, Qt::black, Qt::white);
    qt_std_pal->setBrush(QPalette::Disabled, QPalette::Foreground, Qt::darkGray);
    qt_std_pal->setBrush(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    qt_std_pal->setBrush(QPalette::Disabled, QPalette::Base, qt_std_pal->background());
}

static void qt_fix_tooltips()
{
    // No resources for this yet (unlike on Windows).
    QPalette pal(Qt::black, QColor(255,255,220),
                  QColor(96,96,96), Qt::black, Qt::black,
                  Qt::black, QColor(255,255,220));
    QApplication::setPalette(pal, "QTipLabel");
}
#endif

/*!
    \internal
*/
void QApplication::process_cmdline()
{
    // process platform-indep command line
    if (!qt_is_gui_used || !d->argc)
        return;

    int argc = d->argc;
    char **argv = d->argv;
    int i, j;

    j = 1;
    for (i=1; i<argc; i++) {
        if (argv[i] && *argv[i] != '-') {
            argv[j++] = argv[i];
            continue;
        }
        QByteArray arg = argv[i];
        arg = arg;
        QString s;
        if (arg == "-qdevel" || arg == "-qdebug") {
            // obsolete argument
        } else if (arg.indexOf("-style=", 0) != -1) {
            s = arg.right(arg.length() - 7).toLower();
        } else if (arg == "-style" && i < argc-1) {
            s = argv[++i];
            s = s.toLower();
#ifndef QT_NO_SESSIONMANAGER
        } else if (arg == "-session" && i < argc-1) {
            ++i;
            if (argv[i] && *argv[i]) {
                d->session_id = QString::fromLatin1(argv[i]);
                int p = d->session_id.indexOf('_');
                if (p >= 0) {
                    d->session_key = d->session_id.mid(p +1);
                    d->session_id = d->session_id.left(p);
                }
                d->is_session_restored = true;
            }
#endif
        } else if (qstrcmp(arg, "-reverse") == 0) {
            setReverseLayout(true);
        } else if (qstrcmp(arg, "-widgetcount") == 0) {
            QApplicationPrivate::widgetCount = true;
        } else {
            argv[j++] = argv[i];
        }
#ifndef QT_NO_STYLE
        if (!s.isEmpty()) {
            setStyle(s);
        }
#endif
    }

    if(j < argc) {
        argv[j] = 0;
        d->argc = j;
    }
}

/*!
  Initializes the window system and constructs an application object
  with \a argc command line arguments in \a argv.

  The global \c qApp pointer refers to this application object. Only
  one application object should be created.

  This application object must be constructed before any \link
  QPaintDevice paint devices\endlink (including widgets, pixmaps, bitmaps
  etc.).

  Note that \a argc and \a argv might be changed. Qt removes command
  line arguments that it recognizes. The original \a argc and \a argv
  can be accessed later with \c qApp->argc() and \c qApp->argv().
  The documentation for argv() contains a detailed description of how
  to process command line arguments.

  Qt debugging options (not available if Qt was compiled without the
  QT_DEBUG flag defined):
  \list
  \i -nograb, tells Qt that it must never grab the mouse or the keyboard.
  \i -dograb (only under X11), running under a debugger can cause
  an implicit -nograb, use -dograb to override.
  \i -sync (only under X11), switches to synchronous mode for
        debugging.
  \endlist

  See \link debug.html Debugging Techniques \endlink for a more
  detailed explanation.

  All Qt programs automatically support the following command line options:
  \list
  \i -style= \e style, sets the application GUI style. Possible values
       are \c motif, \c windows, and \c platinum. If you compiled Qt
       with additional styles or have additional styles as plugins these
       will be available to the \c -style command line option.
  \i -style \e style, is the same as listed above.
  \i -session= \e session, restores the application from an earlier
       \link session.html session \endlink.
  \i -session \e session, is the same as listed above.
  \i -widgetcount, prints debug message at the end about number of widgets left
       undestroyed and maximum number of widgets existed at the same time
  \endlist

  The X11 version of Qt also supports some traditional X11
  command line options:
  \list
  \i -display \e display, sets the X display (default is $DISPLAY).
  \i -geometry \e geometry, sets the client geometry of the
        \link setMainWidget() main widget\endlink.
  \i -fn or \c -font \e font, defines the application font. The
  font should be specified using an X logical font description.
  \i -bg or \c -background \e color, sets the default background color
        and an application palette (light and dark shades are calculated).
  \i -fg or \c -foreground \e color, sets the default foreground color.
  \i -btn or \c -button \e color, sets the default button color.
  \i -name \e name, sets the application name.
  \i -title \e title, sets the application title (caption).
  \i -visual \c TrueColor, forces the application to use a TrueColor visual
       on an 8-bit display.
  \i -ncols \e count, limits the number of colors allocated in the
       color cube on an 8-bit display, if the application is using the
       \c QApplication::ManyColor color specification. If \e count is
       216 then a 6x6x6 color cube is used (i.e. 6 levels of red, 6 of green,
       and 6 of blue); for other values, a cube
       approximately proportional to a 2x3x1 cube is used.
  \i -cmap, causes the application to install a private color map
       on an 8-bit display.
  \endlist

  \sa argc(), argv()
*/

QApplication::QApplication(int &argc, char **argv)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GuiClient))
{ construct(); }


/*!
  Constructs an application object with \a argc command line arguments
  in \a argv. If \a GUIenabled is true, a GUI application is
  constructed, otherwise a non-GUI (console) application is created.

  Set \a GUIenabled to false for programs without a graphical user
  interface that should be able to run without a window system.

  On X11, the window system is initialized if \a GUIenabled is true.
  If \a GUIenabled is false, the application does not connect to the
  X-server.
  On Windows and Macintosh, currently the window system is always
  initialized, regardless of the value of GUIenabled. This may change in
  future versions of Qt.

  The following example shows how to create an application that
  uses a graphical interface when available.
  \code
  int main(int argc, char **argv)
  {
#ifdef Q_WS_X11
    bool useGUI = getenv("DISPLAY") != 0;
#else
    bool useGUI = true;
#endif
    QApplication app(argc, argv, useGUI);

    if (useGUI) {
       //start GUI version
       ...
    } else {
       //start non-GUI version
       ...
    }
    return app.exec();
  }
\endcode
*/

QApplication::QApplication(int &argc, char **argv, bool GUIenabled )
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GUIenabled ? GuiClient : Tty))
{ construct(); }

/*!
  Constructs an application object with \a argc command line arguments
  in \a argv.

  For Qt/Embedded, passing \c QApplication::GuiServer for \a type
  makes this application the server (equivalent to running with the
  -qws option).
*/
QApplication::QApplication(int &argc, char **argv, Type type)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, type))
{ construct(); }

/*!
    \internal
*/
void QApplication::construct()
{
    qt_is_gui_used = (qt_appType != Tty);
    qt_init(d, qt_appType);   // Must be called before initialize()
    process_cmdline();
    initialize();
    if (qt_is_gui_used)
        qt_maxWindowRect = desktop()->rect();
    eventLoop()->appStartingUp();
}

#if defined(Q_WS_X11)
// ### a string literal is a cont char*
// ### using it as a char* is wrong and could lead to segfaults
// ### if aargv is modified someday
// ########## make it work with argc == argv == 0
static int aargc = 1;
static char *aargv[] = { (char*)"unknown", 0 };

/*!
  Create an application, given an already open display \a dpy. If \a
  visual and \a colormap are non-zero, the application will use those as
  the default Visual and Colormap contexts.

  \warning Qt only supports TrueColor visuals at depths higher than 8
  bits-per-pixel.

  This is available only on X11.
*/
QApplication::QApplication(Display* dpy, Qt::HANDLE visual, Qt::HANDLE colormap)
    : QCoreApplication(*new QApplicationPrivate(aargc, aargv, GuiClient))
{
    qt_is_gui_used = true;
    // ... no command line.

    if (! dpy)
        qWarning("QApplication: invalid Display* argument.");

    qt_init(d, GuiClient, dpy, visual, colormap);

    initialize();

    if (qt_is_gui_used)
        qt_maxWindowRect = desktop()->rect();
    eventLoop()->appStartingUp();
}

/*!
  Create an application, given an already open display \a dpy and using
  \a argc command line arguments in \a argv. If \a
  visual and \a colormap are non-zero, the application will use those as
  the default Visual and Colormap contexts.

  \warning Qt only supports TrueColor visuals at depths higher than 8
  bits-per-pixel.

  This is available only on X11.

*/
QApplication::QApplication(Display *dpy, int argc, char **argv,
                           Qt::HANDLE visual, Qt::HANDLE colormap)
    : QCoreApplication(*new QApplicationPrivate(argc, argv, GuiClient))
{
    qt_is_gui_used = true;

    if (! dpy)
        qWarning("QApplication: invalid Display* argument.");
    qt_init(d, GuiClient, dpy, visual, colormap);

    process_cmdline();
    initialize();

    if (qt_is_gui_used)
        qt_maxWindowRect = desktop()->rect();
    eventLoop()->appStartingUp();
}


#endif // Q_WS_X11


/*!
  Initializes the QApplication object, called from the constructors.
*/

void QApplication::initialize()
{
    QWidget::mapper = new QWidgetMapper;
#ifndef QT_NO_PALETTE
    (void) palette();  // trigger creation of application palette
#endif
#ifndef QT_NO_VARIANT
    // trigger registering of QVariant's GUI types
    const QVariant v = QVariant(int(QMetaType::Int), (void *)0);
    Q_UNUSED(v)
#endif
    is_app_running = true; // no longer starting up

#ifndef QT_NO_SESSIONMANAGER
    // connect to the session manager
    d->session_manager = new QSessionManager(this, d->session_id, d->session_key);
#endif

}

/*!
    Returns the type of application, Tty, GuiClient or GuiServer.
*/

QApplication::Type QApplication::type() const
{
    return qt_appType;
}

/*****************************************************************************
  Functions returning the active popup and modal widgets.
 *****************************************************************************/

/*!
  Returns the active popup widget.

  A popup widget is a special top level widget that sets the \c
  Qt::WType_Popup widget flag, e.g. the QPopupMenu widget. When the
  application opens a popup widget, all events are sent to the popup.
  Normal widgets and modal widgets cannot be accessed before the popup
  widget is closed.

  Only other popup widgets may be opened when a popup widget is shown.
  The popup widgets are organized in a stack. This function returns
  the active popup widget at the top of the stack.

  \sa activeModalWidget(), topLevelWidgets()
*/

QWidget *QApplication::activePopupWidget()
{
    return QApplicationPrivate::popupWidgets && !QApplicationPrivate::popupWidgets->isEmpty() ?
        QApplicationPrivate::popupWidgets->last() : 0;
}


/*!
  Returns the active modal widget.

  A modal widget is a special top level widget which is a subclass of
  QDialog that specifies the modal parameter of the constructor as
  true. A modal widget must be closed before the user can continue
  with other parts of the program.

  Modal widgets are organized in a stack. This function returns
  the active modal widget at the top of the stack.

  \sa activePopupWidget(), topLevelWidgets()
*/

QWidget *QApplication::activeModalWidget()
{
    return qt_modal_stack && !qt_modal_stack->isEmpty() ? qt_modal_stack->first() : 0;
}

/*!
  Cleans up any window system resources that were allocated by this
  application. Sets the global variable \c qApp to 0.
*/

QApplication::~QApplication()
{
    qt_call_post_routines();

#ifndef QT_NO_CLIPBOARD
    // flush clipboard contents
    if (qt_clipboard) {
        QCustomEvent event(QEvent::Clipboard);
        QApplication::sendEvent(qt_clipboard, &event);
    }
#endif

    eventLoop()->appClosingDown();

    delete qt_desktopWidget;
    qt_desktopWidget = 0;
    is_app_closing = true;

#ifndef QT_NO_CLIPBOARD
    delete qt_clipboard;
    qt_clipboard = 0;
#endif

    // delete widget mapper
    if (!QWidget::mapper)                                // already gone
        return;
    QWidgetMapper * myMapper = QWidget::mapper;
    QWidget::mapper = 0;
    for (QWidgetMapper::Iterator it = myMapper->begin(); it != myMapper->end(); ++it) {
        register QWidget *w = *it;
        if (!w->parent())                        // widget is a parent
            w->destroy(true, true);
    }
    delete myMapper;

#ifndef QT_NO_PALETTE
    delete qt_std_pal;
    qt_std_pal = 0;
    delete QApplicationPrivate::app_pal;
    QApplicationPrivate::app_pal = 0;
    app_palettes()->clear();
#endif
    delete QApplicationPrivate::app_font;
    QApplicationPrivate::app_font = 0;
    app_fonts()->clear();
#ifndef QT_NO_STYLE
    delete QApplicationPrivate::app_style;
    QApplicationPrivate::app_style = 0;
#endif
    delete QApplicationPrivate::app_icon;
    QApplicationPrivate::app_icon = 0;
#ifndef QT_NO_CURSOR
    d->cursor_list.clear();
#endif

#ifndef QT_NO_DRAGANDDROP
    extern QDragManager *qt_dnd_manager;
    delete qt_dnd_manager;
#endif

    qt_cleanup();

    if (QApplicationPrivate::widgetCount)
        qDebug("Widgets left: %i    Max widgets: %i \n", QWidget::instanceCounter, QWidget::maxInstances);
#ifndef QT_NO_SESSIONMANAGER
    delete d->session_manager;
    d->session_manager = 0;
#endif //QT_NO_SESSIONMANAGER

    qt_explicit_app_style = false;
    qt_app_has_font = false;
    QApplicationPrivate::obey_desktop_settings = true;
    QApplicationPrivate::cursor_flash_time = 1000;
    QApplicationPrivate::mouse_double_click_time = 400;
#ifndef QT_NO_WHEELEVENT
    QApplicationPrivate::wheel_scroll_lines = 3;
#endif
    drag_time = 500;
    drag_distance = 4;
    reverse_layout = false;
    QApplicationPrivate::app_strut = QSize(0, 0);
    QApplicationPrivate::animate_ui = true;
    QApplicationPrivate::animate_menu = false;
    QApplicationPrivate::fade_menu = false;
    QApplicationPrivate::animate_combo = false;
    QApplicationPrivate::animate_tooltip = false;
    QApplicationPrivate::fade_tooltip = false;
    QApplicationPrivate::widgetCount = false;
}


/*!
    Returns a pointer to the widget at global screen position
    (\a{x}, \a{y}), or 0 if there is no Qt widget there.

    This function is normally rather slow.

    \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/
QWidget *QApplication::widgetAt(int x, int y)
{
    QWidget *ret = widgetAt_sys(x, y);
    if(ret && ret->testAttribute(Qt::WA_TransparentForMouseEvents)) {
        //shoot a hole in the widget and try once again
        QRegion oldmask = ret->mask();
        QPoint wpoint = ret->mapFromGlobal(QPoint(x, y));
        QRegion newmask = oldmask - QRegion(wpoint.x(), wpoint.y(), 1, 1);
        ret->setMask(newmask);
        QWidget *poke = widgetAt_sys(x, y);
        ret->setMask(oldmask);
        ret = poke;
    }
    return ret;
}


/*!
    \fn void QApplication::setArgs(int argc, char **argv)
    \internal
*/



/*!
    \internal
*/
bool QApplication::compressEvent(QEvent *event, QObject *receiver, QPostEventList *postedEvents)
{
    if ((event->type() == QEvent::UpdateRequest
#ifdef QT_COMPAT
          || event->type() == QEvent::LayoutHint
#endif
          || event->type() == QEvent::LayoutRequest
          || event->type() == QEvent::Resize
          || event->type() == QEvent::Move
#ifdef Q_WS_QWS
          || event->type() == QEvent::QWSUpdate
#endif
          || event->type() == QEvent::LanguageChange
	  || event->type() == QEvent::IMCompose)) {
        for (int i = 0; i < postedEvents->size(); ++i) {
            const QPostEvent &cur = postedEvents->at(i);
            if (cur.receiver != receiver || cur.event == 0 || cur.event->type() != event->type())
                continue;
            if (cur.event->type() == QEvent::LayoutRequest
#ifdef QT_COMPAT
                 || cur.event->type() == QEvent::LayoutHint
#endif
                 || cur.event->type() == QEvent::UpdateRequest) {
                ;
            }
            else if (cur.event->type() == QEvent::Resize) {
                ((QResizeEvent *)(cur.event))->s = ((QResizeEvent *)event)->s;
            } else if (cur.event->type() == QEvent::Move) {
                ((QMoveEvent *)(cur.event))->p = ((QMoveEvent *)event)->p;
#ifdef Q_WS_QWS
            } else if (cur.event->type() == QEvent::QWSUpdate) {
                QPaintEvent * p = (QPaintEvent*)(cur.event);
                p->reg = p->reg.unite(((QPaintEvent *)event)->reg);
                p->rec = p->rec.unite(((QPaintEvent *)event)->rec);
#endif
            } else if (cur.event->type() == QEvent::LanguageChange) {
                ;
	    } else if ( cur.event->type() == QEvent::IMCompose ) {
                *(QIMEvent *)(cur.event) = *(QIMEvent *)event;
            } else {
                continue;
            }
            return true;
        }
    }
    return false;
}

#ifndef QT_NO_STYLE

static QString *qt_style_override = 0;

/*!
  Returns the application's style object.

  \sa setStyle(), QStyle
*/
QStyle *QApplication::style()
{
#ifndef QT_NO_STYLE
    if (QApplicationPrivate::app_style)
        return QApplicationPrivate::app_style;
    if (!qt_is_gui_used)
        qFatal("No style available in non-gui applications!");

#if defined(Q_WS_X11)
    if(!qt_style_override)
        x11_initialize_style(); // run-time search for default style
#endif
    if (!QApplicationPrivate::app_style) {
        // Compile-time search for default style
        //
        QString style;
        if (qt_style_override) {
            style = *qt_style_override;
            delete qt_style_override;
            qt_style_override = 0;
        } else {
#  if defined(Q_WS_WIN) && defined(Q_OS_TEMP)
            style = "PocketPC";
#elif defined(Q_WS_WIN)
            if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
                style = "WindowsXP";
            else
                style = "Windows";                // default styles for Windows
#elif defined(Q_WS_X11) && defined(Q_OS_SOLARIS)
            style = "CDE";                        // default style for X11 on Solaris
#elif defined(Q_WS_X11) && defined(Q_OS_IRIX)
            style = "SGI";                        // default style for X11 on IRIX
#elif defined(Q_WS_X11)
                style = "Motif";                // default style for X11
#elif defined(Q_WS_MAC)
                style = "Macintosh";                // default style for all Mac's
#elif defined(Q_WS_QWS)
            style = "Compact";                // default style for small devices
#endif
        }
        QApplicationPrivate::app_style = QStyleFactory::create(style);
        if (!QApplicationPrivate::app_style &&  // platform default style not available, try alternatives
            !(QApplicationPrivate::app_style = QStyleFactory::create("Windows")) &&
            !(QApplicationPrivate::app_style = QStyleFactory::create("Platinum")) &&
            !(QApplicationPrivate::app_style = QStyleFactory::create("MotifPlus")) &&
            !(QApplicationPrivate::app_style = QStyleFactory::create("Motif")) &&
            !(QApplicationPrivate::app_style = QStyleFactory::create("CDE")) &&
            !(QApplicationPrivate::app_style = QStyleFactory::create("Mac")) &&
            !(QApplicationPrivate::app_style = QStyleFactory::create("SGI")) &&
            !(QApplicationPrivate::app_style = QStyleFactory::create("Compact"))
            && (QStyleFactory::keys().isEmpty() ||
                !(QApplicationPrivate::app_style = QStyleFactory::create(QStyleFactory::keys()[0])))
       )
            qFatal("No %s style available!", style.latin1());
    }

    QPalette app_pal_copy (*QApplicationPrivate::app_pal);
    QApplicationPrivate::app_style->polish(*QApplicationPrivate::app_pal);

    if (is_app_running && !is_app_closing && (*QApplicationPrivate::app_pal != app_pal_copy)) {
        QEvent e(QEvent::ApplicationPaletteChange);
        for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin(); it != QWidget::mapper->constEnd(); ++it) {
            register QWidget *w = *it;
            sendEvent(w, &e);
        }
    }

    QApplicationPrivate::app_style->polish(qApp);
#endif
    return QApplicationPrivate::app_style;
}

/*!
  Sets the application's GUI style to \a style. Ownership of the style
  object is transferred to QApplication, so QApplication will delete
  the style object on application exit or when a new style is set.

  Example usage:
  \code
    QApplication::setStyle(new QWindowsStyle);
  \endcode

  When switching application styles, the color palette is set back to
  the initial colors or the system defaults. This is necessary since
  certain styles have to adapt the color palette to be fully
  style-guide compliant.

  \sa style(), QStyle, setPalette(), desktopSettingsAware()
*/
void QApplication::setStyle(QStyle *style)
{
    if (!style)
        return;

    QStyle* old = QApplicationPrivate::app_style;
    QApplicationPrivate::app_style = style;
#ifdef Q_WS_X11
    qt_explicit_app_style = true;
#endif // Q_WS_X11

    if (startingUp()) {
        delete old;
        return;
    }

    // clean up the old style
    if (old) {
        if (is_app_running && !is_app_closing) {
            for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin(); it != QWidget::mapper->constEnd(); ++it) {
                register QWidget *w = *it;
                if (!w->testWFlags(Qt::WType_Desktop) &&        // except desktop
                     w->testWState(Qt::WState_Polished)) { // has been polished
                    old->unPolish(w);
                }
            }
        }
        old->unPolish(qApp);
    }

    // take care of possible palette requirements of certain gui
    // styles. Do it before polishing the application since the style
    // might call QApplication::setStyle() itself
    if (!qt_std_pal)
        qt_create_std_palette();
    QPalette tmpPal = *qt_std_pal;
    setPalette(tmpPal);

    // initialize the application with the new style
    QApplicationPrivate::app_style->polish(qApp);

    // re-polish existing widgets if necessary
    if (old) {
        if (is_app_running && !is_app_closing) {
            for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin(); it != QWidget::mapper->constEnd(); ++it) {
                register QWidget *w = *it;
                if (!w->testWFlags(Qt::WType_Desktop)) {        // except desktop
                    if (w->testWState(Qt::WState_Polished))
                        QApplicationPrivate::app_style->polish(w);                // repolish
                    QEvent e(QEvent::StyleChange);
                    QApplication::sendEvent(w, &e);
#ifdef QT_COMPAT
                    w->styleChange(*old);
#endif
                    if (w->isVisible())
                        w->update();
                }
            }
        }
        delete old;
    }
}

/*!
  \overload

  Requests a QStyle object for \a style from the QStyleFactory.

  The string must be one of the QStyleFactory::keys(), typically one
  of "windows", "motif", "cde", "motifplus", "platinum", "sgi" and
  "compact". Depending on the platform, "windowsxp", "aqua" or
  "macintosh" may be available.

  A later call to the QApplication constructor will override the
  requested style when a "-style" option is passed in as a commandline
  parameter.

  Returns 0 if an unknown \a style is passed, otherwise the QStyle object
  returned is set as the application's GUI style.
*/
QStyle* QApplication::setStyle(const QString& style)
{
#ifdef Q_WS_X11
    qt_explicit_app_style = true;
#endif // Q_WS_X11

    if (startingUp()) {
        if(qt_style_override)
            *qt_style_override = style;
        else
            qt_style_override = new QString(style);
        return 0;
    }
    QStyle *s = QStyleFactory::create(style);
    if (!s)
        return 0;

    setStyle(s);
    return s;
}

#endif


/*!
  Returns the color specification.
  \sa QApplication::setColorSpec()
 */

int QApplication::colorSpec()
{
    return QApplicationPrivate::app_cspec;
}

/*!
  Sets the color specification for the application to \a spec.

  The color specification controls how the application allocates colors
  when run on a display with a limited amount of colors, e.g. 8 bit / 256
  color displays.

  The color specification must be set before you create the QApplication
  object.

  The options are:
  \list
  \i QApplication::NormalColor.
    This is the default color allocation strategy. Use this option if
    your application uses buttons, menus, texts and pixmaps with few
    colors. With this option, the application uses system global
    colors. This works fine for most applications under X11, but on
    Windows machines it may cause dithering of non-standard colors.
  \i QApplication::CustomColor.
    Use this option if your application needs a small number of custom
    colors. On X11, this option is the same as NormalColor. On Windows, Qt
    creates a Windows palette, and allocates colors to it on demand.
  \i QApplication::ManyColor.
    Use this option if your application is very color hungry
    (e.g. it requires thousands of colors).
    Under X11 the effect is:
    \list
    \i For 256-color displays which have at best a 256 color true
       color visual, the default visual is used, and colors are
       allocated from a color cube. The color cube is the 6x6x6 (216
       color) "Web palette" (the red, green, and blue components
       always have one of the following values: 0x00, 0x33, 0x66,
       0x99, 0xCC, or 0xFF), but the number of colors can be changed
       by the \e -ncols option. The user can force the application to
       use the true color visual with the \link
       QApplication::QApplication() -visual \endlink option.
    \i For 256-color displays which have a true color visual with more
       than 256 colors, use that visual. Silicon Graphics X servers
       have this feature, for example. They provide an 8 bit visual
       by default but can deliver true color when asked.
    \endlist
    On Windows, Qt creates a Windows palette, and fills it with a color cube.
  \endlist

  Be aware that the CustomColor and ManyColor choices may lead to colormap
  flashing: The foreground application gets (most) of the available
  colors, while the background windows will look less attractive.

  Example:
  \code
  int main(int argc, char **argv)
  {
      QApplication::setColorSpec(QApplication::ManyColor);
      QApplication a(argc, argv);
      ...
  }
  \endcode

  QColor provides more functionality for controlling color allocation and
  freeing up certain colors. See QColor::enterAllocContext() for more
  information.

  To check what mode you end up with, call QColor::numBitPlanes() once
  the QApplication object exists. A value greater than 8 (typically
  16, 24 or 32) means true color.

  \sa colorSpec(), QColor::numBitPlanes(), QColor::enterAllocContext() */

void QApplication::setColorSpec(int spec)
{
    if (qApp)
        qWarning("QApplication::setColorSpec: This function must be "
                 "called before the QApplication object is created");
    QApplicationPrivate::app_cspec = spec;
}

/*!
  Returns the application's global strut.

  The strut is a size object whose dimensions are the minimum that any
  GUI element that the user can interact with should have. For example
  no button should be resized to be smaller than the global strut size.

  \sa setGlobalStrut()
*/

QSize QApplication::globalStrut()
{
    return QApplicationPrivate::app_strut;
}

/*!
  Sets the application's global strut to \a strut.

  The strut is a size object whose dimensions are the minimum that any
  GUI element that the user can interact with should have. For example
  no button should be resized to be smaller than the global strut size.

  The strut size should be considered when reimplementing GUI controls
  that may be used on touch-screens or similar IO-devices.

  Example:
  \code
  QSize& WidgetClass::sizeHint() const
  {
      return QSize(80, 25).expandedTo(QApplication::globalStrut());
  }
  \endcode

  \sa globalStrut()
*/

void QApplication::setGlobalStrut(const QSize& strut)
{
    QApplicationPrivate::app_strut = strut;
}

#ifndef QT_NO_PALETTE
/*!
    Returns the application palette.

    \sa setPalette(), QWidget::palette()
*/
QPalette QApplication::palette()
{
    if (!qApp)
        qWarning("QApplication::palette: This function can only be "
                  "called after the QApplication object has been created");
    if (!QApplicationPrivate::app_pal) {
        if (!qt_std_pal)
            qt_create_std_palette();
        QApplicationPrivate::app_pal = new QPalette(*qt_std_pal);
        qt_fix_tooltips();
    }

    return *QApplicationPrivate::app_pal;
}

/*!
    \overload

    If a widget is passed in \a w, the default palette for the
    widget's class is returned. This may or may not be the application
    palette. In most cases there isn't a special palette for certain
    types of widgets, but one notable exception is the popup menu
    under Windows, if the user has defined a special background color
    for menus in the display settings.

    \sa setPalette(), QWidget::palette()
*/
QPalette QApplication::palette(const QWidget* w)
{
    PaletteHash *hash = app_palettes();
    if (w && hash && hash->size()) {
        QHash<QByteArray, QPalette>::ConstIterator it = hash->find(w->metaObject()->className());
        if (it != hash->constEnd())
            return *it;
        for (it = hash->constBegin(); it != hash->constEnd(); ++it) {
            if (w->inherits(it.key()))
                return it.value();
        }
    }
    return palette();
}

/*!
    \overload

    Returns the palette for widgets of the given \a className.

    \sa setPalette(), QWidget::palette()
*/
QPalette QApplication::palette(const char *className)
{
    if (!QApplicationPrivate::app_pal)
        palette();
    PaletteHash *hash = app_palettes();
    if (className && hash && hash->size()) {
        QHash<QByteArray, QPalette>::ConstIterator it = hash->find(className);
        if (it != hash->constEnd())
            return *it;
    }
    return *QApplicationPrivate::app_pal;
}

/*!
  Changes the default application palette to \a palette.

  If \a className is passed, the change applies only to widgets that
  inherit \a className (as reported by QObject::inherits()). If
  \a className is left 0, the change affects all widgets, thus overriding
  any previously set class specific palettes.

  The palette may be changed according to the current GUI style in
  QStyle::polish().

  \sa QWidget::setPalette(), palette(), QStyle::polish()
*/

void QApplication::setPalette(const QPalette &palette, const char* className)
{
    QPalette pal = palette;

#ifndef QT_NO_STYLE
    if (!startingUp()) // on startup this has been done already
        qApp->style()->polish(pal);        // NB: non-const reference
#endif
    bool all = false;
    PaletteHash *hash = app_palettes();
    if (!className) {
        if (!QApplicationPrivate::app_pal)
            QApplicationPrivate::app_pal = new QPalette(pal);
        else
            *QApplicationPrivate::app_pal = pal;
        if (hash && hash->size()) {
            all = true;
            hash->clear();
        }
        qt_fix_tooltips();
    } else if (hash) {
        hash->insert(className, pal);
    }

    if (is_app_running && !is_app_closing) {
        QEvent e(QEvent::ApplicationPaletteChange);
        for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin();
             it != QWidget::mapper->constEnd(); ++it) {
            register QWidget *w = *it;
            if (all || (!className && w->isTopLevel()) || w->inherits(className)) // matching class
                sendEvent(w, &e);
        }
    }
}

#endif // QT_NO_PALETTE

/*!
  Returns the default font for the widget \a w, or the default
  application font if \a w is 0.

  \sa setFont(), fontMetrics(), QWidget::font()
*/

QFont QApplication::font(const QWidget *w)
{
    FontHash *hash = app_fonts();
    if (w && hash  && hash->size()) {
        QHash<QByteArray, QFont>::ConstIterator it =
            hash->find(w->metaObject()->className());
        if (it != hash->constEnd())
            return it.value();
        for (it = hash->begin(); it != hash->end(); ++it) {
            if (w->inherits(it.key()))
                return it.value();
        }
    }
    if (!QApplicationPrivate::app_font)
        QApplicationPrivate::app_font = new QFont("Helvetica");
    return *QApplicationPrivate::app_font;
}

/*! Changes the default application font to \a font.  If \a className
  is passed, the change applies only to classes that inherit \a
  className (as reported by QObject::inherits()).

  On application start-up, the default font depends on the window
  system. It can vary depending on both the window system version and
  the locale. This function lets you override the default font; but
  overriding may be a bad idea because, for example, some locales need
  extra-large fonts to support their special characters.

  \sa font(), fontMetrics(), QWidget::setFont()
*/

void QApplication::setFont(const QFont &font, const char* className)
{
    bool all = false;
    FontHash *hash = app_fonts();
    if (!className) {
        qt_app_has_font = true;
        if (!QApplicationPrivate::app_font)
            QApplicationPrivate::app_font = new QFont(font);
        else
            *QApplicationPrivate::app_font = font;
        if (hash && hash->size()) {
            all = true;
            hash->clear();
        }
        // ### qt_fix_tooltips() ?
    } else if (hash) {
        hash->insert(className, font);
    }
    if (is_app_running && !is_app_closing) {
        QEvent e(QEvent::ApplicationFontChange);
        for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin();
             it != QWidget::mapper->constEnd(); ++it) {
            register QWidget *w = *it;
            if (all || (!className && w->isTopLevel()) || w->inherits(className)) // matching class
                sendEvent(w, &e);
        }
    }
}

/*!
  Returns the default window icon.

  \sa setWindowIcon()
 */
const QPixmap &QApplication::windowIcon()
{
    if (!QApplicationPrivate::app_icon)
        QApplicationPrivate::app_icon = new QPixmap();
    return *QApplicationPrivate::app_icon;
}

/*!
  Sets the default window icon to \a pixmap.

  \sa windowIcon()
 */
void QApplication::setWindowIcon(const QPixmap &pixmap)
{
    if (!QApplicationPrivate::app_icon)
        QApplicationPrivate::app_icon = new QPixmap(pixmap);
    else
        *QApplicationPrivate::app_icon = pixmap;
    if (is_app_running && !is_app_closing) {
#ifdef Q_WS_MAC
        void qt_mac_set_app_icon(const QPixmap &); //qapplication_mac.cpp
        qt_mac_set_app_icon(pixmap);
#endif
        QEvent e(QEvent::ApplicationWindowIconChange);
        for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin();
             it != QWidget::mapper->constEnd(); ++it) {
            register QWidget *w = *it;
            if (w->isTopLevel())
                sendEvent(w, &e);
        }
    }
}


/*!
  Initialization of the appearance of the widget \a w \e before it is first
  shown.

  Usually widgets call this automatically when they are polished. It
  may be used to do some style-based central customization of widgets.

  Note that you are not limited to the public functions of QWidget.
  Instead, based on meta information like QObject::className() you are
  able to customize any kind of widget.

  \sa QStyle::polish(), setPalette(), setFont()
*/

void QApplication::polish(QWidget *w)
{
#ifndef QT_NO_STYLE
    w->style()->polish(w);
#endif
    w->setWState(Qt::WState_Polished);
}


/*!
  Returns a list of the top level widgets in the application.

  The list is empty (QList::isEmpty()) if there are no top level
  widgets.

  Note that some of the top level widgets may be hidden, for example
  the tooltip if no tooltip is currently shown.

  Example:
  \code
    // Show all hidden top level widgets.
    foreach(QWidget *w, QApplication::topLevelWidgets()) {
        if (w->isHidden())
            w->show();
    }
  \endcode

  \sa allWidgets(), QWidget::isTopLevel(), QWidget::isHidden(),
      QList::isEmpty()
*/
QWidgetList QApplication::topLevelWidgets()
{
    QWidgetList list;
    if (QWidget::mapper) {
        for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin();
             it != QWidget::mapper->constEnd(); ++it) {
            QWidget *w = *it;
            if (w->isTopLevel())
                list.append(w);
        }
    }
    return list;
}

/*!
  Returns a list of all the widgets in the application.

  The list is empty (QList::isEmpty()) if there are no widgets.

  Note that some of the widgets may be hidden.

  Example that updates all widgets:
  \code
    foreach(QWidget *w, QApplication::allWidgets())
        w->update();
  \endcode

  \sa topLevelWidgets(), QWidget::isVisible(), QList::isEmpty(),
*/

QWidgetList QApplication::allWidgets()
{
    QWidgetList list;
    if (QWidget::mapper) {
        for (QWidgetMapper::ConstIterator it = QWidget::mapper->constBegin();
             it != QWidget::mapper->constEnd(); ++it)
            list.append(*it);
    }
    return list;
}

/*!
  Returns the application widget that has the keyboard input focus, or
  0 if no widget in this application has the focus.

  \sa QWidget::setFocus(), QWidget::hasFocus(), activeWindow()
*/

QWidget *QApplication::focusWidget() const
{
    return QApplicationPrivate::focus_widget;
}

void QApplication::setFocusWidget(QWidget *focus)
{
    QApplicationPrivate::focus_widget = focus;
}


/*!
  Returns the application top-level window that has the keyboard input
  focus, or 0 if no application window has the focus. Note that
  there might be an activeWindow() even if there is no focusWidget(),
  for example if no widget in that window accepts key events.

  \sa QWidget::setFocus(), QWidget::hasFocus(), focusWidget()
*/

QWidget *QApplication::activeWindow() const
{
    return QApplicationPrivate::active_window;
}

/*!
  Returns display (screen) font metrics for the application font.

  \sa font(), setFont(), QWidget::fontMetrics(), QPainter::fontMetrics()
*/

QFontMetrics QApplication::fontMetrics()
{
    return desktop()->fontMetrics();
}


/*!
  Closes all top-level windows.

  This function is particularly useful for applications with many
  top-level windows. It could, for example, be connected to a "Quit"
  entry in the file menu as shown in the following code example:

  \code
    // the "Quit" menu entry should try to close all windows
    QMenu* file = new Menu(this);
    file->addAction("&Quit", qApp, SLOT(closeAllWindows()), Qt::CTRL+Qt::Key_Q);

    // when the last window is closed, the application should quit
    connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
  \endcode

  The windows are closed in random order, until one window does not
  accept the close event.

  \sa QWidget::close(), QWidget::closeEvent(), lastWindowClosed(),
  quit(), topLevelWidgets(), QWidget::isTopLevel()

 */
void QApplication::closeAllWindows()
{
    bool did_close = true;
    QWidget *w;
    while((w = activeModalWidget()) && did_close) {
        if(w->isHidden())
            break;
        did_close = w->close();
    }
    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; i < list.size(); ++i) {
        w = list.at(i);
        if (!w->isHidden()) {
            did_close = w->close();
            list = QApplication::topLevelWidgets();
            i = -1;
        }
    }
}

/*!
    Displays a simple message box about Qt. The message includes the
    version number of Qt being used by the application.

    This is useful for inclusion in the Help menu of an application.
    See the examples/menu/menu.cpp example.

    This function is a convenience slot for QMessageBox::aboutQt().
*/
void QApplication::aboutQt()
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::aboutQt(mainWidget());
#endif // QT_NO_MESSAGEBOX
}


/*!
  \fn void QApplication::lastWindowClosed()

  This signal is emitted when the user has closed the last
  top level window.

  The signal is very useful when your application has many top level
  widgets but no main widget. You can then connect it to the quit()
  slot.

  For convenience, this signal is \e not emitted for transient top level
  widgets such as popup menus and dialogs.

  \sa mainWidget(), topLevelWidgets(), QWidget::isTopLevel(), QWidget::close()
*/

#ifndef QT_NO_TRANSLATION
static bool qt_detectRTLLanguage()
{
    return QApplication::tr("QT_LAYOUT_DIRECTION",
            "Translate this string to the string 'LTR' in left-to-right"
            " languages or to 'RTL' in right-to-left languages (such as Hebrew"
            " and Arabic) to get proper widget layout.") == "RTL";
}
#endif

/*
  \fn bool QApplication::sendEvent(QObject *receiver, QEvent *event)

  Sends event \a event directly to receiver \a receiver, using the
  notify() function. Returns the value that was returned from the event
  handler.

    The event is \e not deleted when the event has been sent. The normal
    approach is to create the event on the stack, e.g.
    \code
    QMouseEvent me(QEvent::MouseButtonPress, pos, 0, 0);
    QApplication::sendEvent(mainWindow, &me);
    \endcode
    If you create the event on the heap you must delete it.

  \sa postEvent(), notify()
*/


/*!\reimp

*/
bool QApplication::event(QEvent *e)
{
    if(e->type() == QEvent::Close) {
        QCloseEvent *ce = (QCloseEvent*)e;
        ce->accept();
        closeAllWindows();

        QWidgetList list = topLevelWidgets();
        for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (!w->isHidden() && !w->isDesktop() && !w->isPopup() &&
                 (!w->isDialog() || !w->parentWidget())) {
                ce->ignore();
                break;
            }
        }
        if(ce->isAccepted())
            return true;
    } else if(e->type() == QEvent::LanguageChange) {
#ifndef QT_NO_TRANSLATION
        setReverseLayout(qt_detectRTLLanguage());
#endif
        QWidgetList list = topLevelWidgets();
        for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (!w->isDesktop())
                postEvent(w, new QEvent(QEvent::LanguageChange));
        }
    } else if (e->type() == QEvent::Timer) {
        QTimerEvent *te = static_cast<QTimerEvent*>(e);
        Q_ASSERT(te != 0);
        if (te->timerId() == qt_double_buffer_timer) {
            if (!QApplicationPrivate::active_window) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
                extern void qt_discard_double_buffer();
                qt_discard_double_buffer();
#endif
            }

            killTimer(qt_double_buffer_timer);
            qt_double_buffer_timer = 0;
            return true;
        } else if (te->timerId() == d->toolTipWakeUp.timerId()) {
            d->toolTipWakeUp.stop();
            d->toolTipFallAsleep.start(2000, this);
            if (d->toolTipWidget) {
                QHelpEvent e(QEvent::ToolTip, d->toolTipPos, d->toolTipGlobalPos);
                QApplication::sendEvent(d->toolTipWidget, &e);
            }
        } else if (te->timerId() == d->toolTipFallAsleep.timerId()) {
            d->toolTipFallAsleep.stop();
        }
    }
    return QCoreApplication::event(e);
}
#if !defined(Q_WS_X11)

// The doc and X implementation of this function is in qapplication_x11.cpp

void QApplication::syncX()        {}                // do nothing

#endif

#if defined(Q_OS_CYGWIN)
/*!
  \fn Qt::WindowsVersion QApplication::winVersion()

  Returns the version of the Windows operating system that is running:

  \table
  \header \i Version \i OS
  \row \i Qt::WV_95 \i Windows 95
  \row \i Qt::WV_98 \i Windows 98
  \row \i Qt::WV_Me \i Windows Me
  \row \i Qt::WV_NT \i Windows NT 4.x
  \row \i Qt::WV_2000 \i Windows 2000 (NT5)
  \row \i Qt::WV_XP \i Windows XP
  \row \i Qt::WV_2003 \i Windows Server 2003 family
  \row \i Qt::WV_CE \i Windows CE
  \row \i Qt::WV_CENET \i Windows CE.NET
  \endtable

  \warning This function is only implemented for the Windows version
  of Qt.
*/
#endif

/*!
  Sets the active window to the \a act widget in response to a system
  event. The function is called from the platform specific event
  handlers.

  \warning This function does \e not set the keyboard focus to the
  active widget. Call QWidget::setActiveWindow() instead.

  It sets the activeWindow() and focusWidget() attributes and sends
  proper WindowActivate/WindowDeactivate and FocusIn/FocusOut events
  to all appropriate widgets. The window will then be painted in
  active state (e.g. cursors in line edits will blink), and it will
  have tool tips enabled.

  \sa activeWindow(), QWidget::setActiveWindow()
 */
void QApplication::setActiveWindow(QWidget* act)
{
    QWidget* window = act?act->topLevelWidget():0;

    if (QApplicationPrivate::active_window == window)
        return;

    // first the activation/deactivation events
    QEvent ae(QEvent::ActivationChange);
    if (QApplicationPrivate::active_window) {
        QWidgetList deacts;
#ifndef QT_NO_STYLE
        if (style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, QApplicationPrivate::active_window)) {
            QWidgetList list = topLevelWidgets();
            for (int i = 0; i < list.size(); ++i) {
                QWidget *w = list.at(i);
                if (w->isVisible() && w->isActiveWindow())
                    deacts.append(w);
            }
        } else
#endif
            deacts.append(QApplicationPrivate::active_window);
        QApplicationPrivate::active_window = 0;
        QEvent e(QEvent::WindowDeactivate);
        for(int i = 0; i < deacts.size(); ++i) {
            QWidget *w = deacts.at(i);
            sendSpontaneousEvent(w, &e);
            sendSpontaneousEvent(w, &ae);
        }
    }

    QApplicationPrivate::active_window = window;
    if (QApplicationPrivate::active_window) {
        QEvent e(QEvent::WindowActivate);
        QWidgetList acts;
#ifndef QT_NO_STYLE
        if (style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, QApplicationPrivate::active_window)) {
            QWidgetList list = topLevelWidgets();
            for (int i = 0; i < list.size(); ++i) {
                QWidget *w = list.at(i);
                if (w->isVisible() && w->isActiveWindow())
                    acts.append(w);
            }
        } else
#endif
            acts.append(QApplicationPrivate::active_window);
        for (int i = 0; i < acts.size(); ++i) {
            QWidget *w = acts.at(i);
            sendSpontaneousEvent(w, &e);
            sendSpontaneousEvent(w, &ae);
        }
    }

    // then focus events
    QFocusEvent::setReason(QFocusEvent::ActiveWindow);
    if (!QApplicationPrivate::active_window && QApplicationPrivate::focus_widget) {
        QFocusEvent out(QEvent::FocusOut);
        QWidget *tmp = QApplicationPrivate::focus_widget;
        QApplicationPrivate::focus_widget = 0;
#ifdef Q_WS_WIN
        QInputContext::accept(tmp);
#elif defined(Q_WS_X11)
	tmp->d->unfocusInputContext();
#endif
        sendSpontaneousEvent(tmp, &out);
    } else if (QApplicationPrivate::active_window) {
        QWidget *w = QApplicationPrivate::active_window->focusWidget();
        if (w /*&& w->focusPolicy() != QWidget::NoFocus*/)
            w->setFocus();
        else
            QApplicationPrivate::active_window->focusNextPrevChild(true);
    }
    QFocusEvent::resetReason();

    if (!QApplicationPrivate::active_window) {
        // (re)start the timer to discard the global double buffer
        // while the application doesn't have any active windows
        if (qt_double_buffer_timer)
            killTimer(qt_double_buffer_timer);
        qt_double_buffer_timer = startTimer(500);
    }
}


/*!\internal

  Creates the proper Enter/Leave event when widget \a enter is entered
  and widget \a leave is left.
 */
void qt_dispatchEnterLeave(QWidget* enter, QWidget* leave) {
#if 0
    if (leave) {
        QEvent e(QEvent::Leave);
        QApplication::sendEvent(leave, & e);
    }
    if (enter) {
        QEvent e(QEvent::Enter);
        QApplication::sendEvent(enter, & e);
    }
    return;
#endif

    QWidget* w ;
    if (!enter && !leave)
        return;
    QWidgetList leaveList;
    QWidgetList enterList;

    bool sameWindow = leave && enter && leave->topLevelWidget() == enter->topLevelWidget();
    if (leave && !sameWindow) {
        w = leave;
        do {
            leaveList.append(w);
        } while (!w->isTopLevel() && (w = w->parentWidget()));
    }
    if (enter && !sameWindow) {
        w = enter;
        do {
            enterList.prepend(w);
        } while (!w->isTopLevel() && (w = w->parentWidget()));
    }
    if (sameWindow) {
        int enterDepth = 0;
        int leaveDepth = 0;
        w = enter;
        while (!w->isTopLevel() && (w = w->parentWidget()))
            enterDepth++;
        w = leave;
        while (!w->isTopLevel() && (w = w->parentWidget()))
            leaveDepth++;
        QWidget* wenter = enter;
        QWidget* wleave = leave;
        while (enterDepth > leaveDepth) {
            wenter = wenter->parentWidget();
            enterDepth--;
        }
        while (leaveDepth > enterDepth) {
            wleave = wleave->parentWidget();
            leaveDepth--;
        }
        while (!wenter->isTopLevel() && wenter != wleave) {
            wenter = wenter->parentWidget();
            wleave = wleave->parentWidget();
        }

        w = leave;
        while (w != wleave) {
            leaveList.append(w);
            w = w->parentWidget();
        }
        w = enter;
        while (w != wenter) {
            enterList.prepend(w);
            w = w->parentWidget();
        }
    }

    QEvent leaveEvent(QEvent::Leave);
    for (int i = 0; i < leaveList.size(); ++i) {
        w = leaveList.at(i);
        if (!qApp->activeModalWidget() || qt_tryModalHelper(w, 0))
            QApplication::sendEvent(w, &leaveEvent);
    }
    QEvent enterEvent(QEvent::Enter);
    for (int i = 0; i < enterList.size(); ++i) {
        w = enterList.at(i);
        if (!qApp->activeModalWidget() || qt_tryModalHelper(w, 0))
            QApplication::sendEvent(w, &enterEvent);
    }
}


#ifdef Q_WS_MAC
extern QWidget *qt_tryModalHelperMac(QWidget * top); //qapplication_mac.cpp
#endif


/*!\internal

  Called from qapplication_\e{platform}.cpp, returns true
  if the widget should accept the event.
 */
bool qt_tryModalHelper(QWidget *widget, QWidget **rettop) {
    QWidget *modal=0, *top=QApplication::activeModalWidget();
    if (rettop) *rettop = top;

    if (qApp->activePopupWidget())
        return true;

#ifdef Q_WS_MAC
    top = qt_tryModalHelperMac(top);
    if (rettop) *rettop = top;
#endif

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if (widget->testWFlags(Qt::WShowModal))        // widget is modal
        modal = widget;
    if (!top || modal == top)                        // don't block event
        return true;

    QWidget * p = widget->parentWidget(); // Check if the active modal widget is a parent of our widget
    while (p) {
        if (p == top)
            return true;
        p = p->parentWidget();
    }

    while (groupLeader && !groupLeader->testWFlags(Qt::WGroupLeader))
        groupLeader = groupLeader->parentWidget();

    if (groupLeader) {
        // Does groupLeader have a child in qt_modal_stack?
        bool unrelated = true;
        for (int i = 0; unrelated && i < qt_modal_stack->size(); ++i) {
            modal = qt_modal_stack->at(i);
            QWidget* p = modal->parentWidget();
            while (p && p != groupLeader && !p->testWFlags(Qt::WGroupLeader)) {
                p = p->parentWidget();
            }
            if (p == groupLeader) unrelated = false;
        }

        if (unrelated)
            return true;                // don't block event
    }
    return false;
}


/*!
  Returns the desktop widget (also called the root window).

  The desktop widget is useful for obtaining the size of the screen.
  It may also be possible to draw on the desktop. We recommend against
  assuming that it's possible to draw on the desktop, since this does
  not work on all operating systems.

  \code
    QDesktopWidget *d = QApplication::desktop();
    int w = d->width();            // returns desktop width
    int h = d->height();    // returns desktop height
  \endcode
*/

QDesktopWidget *QApplication::desktop()
{
    if (!qt_desktopWidget || // not created yet
         !qt_desktopWidget->isDesktop()) { // reparented away
        qt_desktopWidget = new QDesktopWidget();
    }
    return qt_desktopWidget;
}

#ifndef QT_NO_CLIPBOARD
/*!
  Returns a pointer to the application global clipboard.
*/
QClipboard *QApplication::clipboard()
{
    if (qt_clipboard == 0) {
        qt_clipboard = new QClipboard(0);
    }
    return qt_clipboard;
}
#endif // QT_NO_CLIPBOARD

/*!
  By default, Qt will try to use the current standard colors, fonts
  etc., from the underlying window system's desktop settings,
  and use them for all relevant widgets. This behavior can be switched off
  by calling this function with \a on set to false.

  This static function must be called before creating the QApplication
  object, like this:

  \code
  int main(int argc, char** argv) {
    QApplication::setDesktopSettingsAware(false); // I know better than the user
    QApplication myApp(argc, argv); // Use default fonts & colors
    ...
  }
  \endcode

  \sa desktopSettingsAware()
*/

void QApplication::setDesktopSettingsAware(bool on)
{
    QApplicationPrivate::obey_desktop_settings = on;
}

/*!
  Returns the value set by setDesktopSettingsAware(); by default true.

  \sa setDesktopSettingsAware()
*/

bool QApplication::desktopSettingsAware()
{
    return QApplicationPrivate::obey_desktop_settings;
}

/*!
  Returns the current state of the modifier keys on the keyboard. The
  current state is updated sychronously as the event queue is emptied
  of events that will spontaneously change the keyboard state
  (QEvent::KeyPress and QEvent::KeyRelease events).

  It should be noted this may not reflect the actual keys held on the
  input device at the time of calling but rather the modifiers as
  last reported in one of the above events. If no keys are being held
  Qt::NoModifier is returned.

  \sa QApplication::mouseButtons(), Qt::KeyboardModifiers
*/

Qt::KeyboardModifiers QApplication::keyboardModifiers()
{
    return QApplicationPrivate::modifier_buttons;
}

/*!
  Returns the current state of the buttons on the mouse. The current
  state is updated syncronously as the event queue is emptied of
  events that will spontaneously change the mouse state
  (QEvent::MousePress and QEvent::MouseRelease events).

  It should be noted this may not reflect the actual buttons held on
  theinput device at the time of calling but rather the mouse buttons
  as last reported in one of the above events. If no mouse buttons rae
  being held Qt::NoButton is returned.

  \sa QApplication::keyboardModifiers(), Qt::MouseButtons
*/

Qt::MouseButtons QApplication::mouseButtons()
{
    return QApplicationPrivate::mouse_buttons;
}

/*!
  \fn bool QApplication::isSessionRestored() const

  Returns true if the application has been restored from an earlier
  \link session.html session\endlink; otherwise returns false.

  \sa sessionId(), commitData(), saveState()
*/


/*!
  \fn QString QApplication::sessionId() const

  Returns the current \link session.html session's\endlink identifier.

  If the application has been restored from an earlier session, this
  identifier is the same as it was in that previous session.

  The session identifier is guaranteed to be unique both for different
  applications and for different instances of the same application.

  \sa isSessionRestored(), sessionKey(), commitData(), saveState()
 */

/*!
  \fn QString QApplication::sessionKey() const

  Returns the session key in the current \link session.html
  session\endlink.

  If the application has been restored from an earlier session, this
  key is the same as it was when the previous session ended.

  The session key changes with every call of commitData() or
  saveState().

  \sa isSessionRestored(), sessionId(), commitData(), saveState()
 */
#ifndef QT_NO_SESSIONMANAGER
bool QApplication::isSessionRestored() const
{
    return d->is_session_restored;
}

QString QApplication::sessionId() const
{
    return d->session_id;
}

QString QApplication::sessionKey() const
{
    return d->session_key;
}
#endif



/*!
  \fn void QApplication::commitData(QSessionManager& sm)

  This function deals with \link session.html session
  management\endlink. It is invoked when the QSessionManager wants the
  application to commit all its data.

  Usually this means saving all open files, after getting
  permission from the user. Furthermore you may want to provide a means
  by which the user can cancel the shutdown.

  Note that you should not exit the application within this function.
  Instead, the session manager may or may not do this afterwards,
  depending on the context.

  \warning Within this function, no user interaction is possible, \e
  unless you ask the session manager \a sm for explicit permission.
  See QSessionManager::allowsInteraction() and
  QSessionManager::allowsErrorInteraction() for details and example
  usage.

  The default implementation requests interaction and sends a close
  event to all visible top level widgets. If any event was
  rejected, the shutdown is canceled.

  \sa isSessionRestored(), sessionId(), saveState(), \link session.html the Session Management overview\endlink
*/
#ifndef QT_NO_SESSIONMANAGER
void QApplication::commitData(QSessionManager& sm )
{

    if (sm.allowsInteraction()) {
        QWidgetList done;
        QWidgetList list = QApplication::topLevelWidgets();
        bool cancelled = false;
        for (int i = 0; !cancelled && i < list.size(); ++i) {
            QWidget* w = list.at(i);
            if (!w->isHidden()) {
                QCloseEvent e;
                sendEvent(w, &e);
                cancelled = !e.isAccepted();
                if (!cancelled)
                    done.append(w);
                list = QApplication::topLevelWidgets();
                i = -1;
            }
            while (i < list.size()-1 && done.contains(list.at(i-1)))
                ++i;
        }
        if (cancelled)
            sm.cancel();
    }
}


/*!
  \fn void QApplication::saveState(QSessionManager& sm)

  This function deals with \link session.html session
  management\endlink. It is invoked when the
  \link QSessionManager session manager \endlink wants the application
  to preserve its state for a future session.

  For example, a text editor would create a temporary file that
  includes the current contents of its edit buffers, the location of
  the cursor and other aspects of the current editing session.

  Note that you should never exit the application within this
  function. Instead, the session manager may or may not do this
  afterwards, depending on the context. Futhermore, most session
  managers will very likely request a saved state immediately after
  the application has been started. This permits the session manager
  to learn about the application's restart policy.

  \warning Within this function, no user interaction is possible, \e
  unless you ask the session manager \a sm for explicit permission.
  See QSessionManager::allowsInteraction() and
  QSessionManager::allowsErrorInteraction() for details.

  \sa isSessionRestored(), sessionId(), commitData(), \link session.html the Session Management overview\endlink
*/

void QApplication::saveState(QSessionManager& /* sm */)
{
}
#endif //QT_NO_SESSIONMANAGER
/*!
  Sets the time after which a drag should start to \a ms ms.

  \sa startDragTime()
*/

void QApplication::setStartDragTime(int ms)
{
    drag_time = ms;
}

/*!
  If you support drag and drop in you application and a drag should
  start after a mouse click and after a certain time elapsed, you
  should use the value which this method returns as the delay (in ms).

  Qt also uses this delay internally, e.g. in QTextEdit and QLineEdit,
  for starting a drag.

  The default value is 500 ms.

  \sa setStartDragTime(), startDragDistance()
*/

int QApplication::startDragTime()
{
    return drag_time;
}

/*!
  Sets the distance after which a drag should start to \a l pixels.

  \sa startDragDistance()
*/

void QApplication::setStartDragDistance(int l)
{
    drag_distance = l;
}

/*!
  If you support drag and drop in you application and a drag should
  start after a mouse click and after moving the mouse a certain
  distance, you should use the value which this method returns as the
  distance.

  For example, if the mouse position of the click is stored in \c
  startPos and the current position (e.g. in the mouse move event) is
  \c currPos, you can find out if a drag should be started with code
  like this:
  \code
  if ((startPos - currPos).manhattanLength() >
       QApplication::startDragDistance())
    startTheDrag();
  \endcode

  Qt uses this value internally, e.g. in QFileDialog.

  The default value is 4 pixels.

  \sa setStartDragDistance(), startDragTime(), QPoint::manhattanLength()
*/

int QApplication::startDragDistance()
{
    return drag_distance;
}

/*!
  If \a b is true, all dialogs and widgets will be laid out in a
  mirrored fashion, as required by right to left languages such as
  Arabic and Hebrew. If \a b is false, dialogs and widgets are laid
  out left to right.

  Changing this flag in runtime does not cause a relayout of already
  instantiated widgets.

  \sa reverseLayout()
*/
void QApplication::setReverseLayout(bool b)
{
    if (reverse_layout == b)
        return;

    reverse_layout = b;

    QWidgetList list = topLevelWidgets();
    for (int i = 0; i < list.size(); ++i) {
        QWidget *w = list.at(i);
        postEvent(w, new QEvent(QEvent::LayoutDirectionChange));
    }
}

/*!
    Returns true if all dialogs and widgets will be laid out in a
    mirrored (right to left) fashion. Returns false if dialogs and
    widgets will be laid out left to right.

  \sa setReverseLayout()
*/
bool QApplication::reverseLayout()
{
    return reverse_layout;
}


/*!
    \fn QCursor *QApplication::overrideCursor()

    Returns the active application override cursor.

    This function returns 0 if no application cursor has been defined
    (i.e. the internal cursor stack is empty).

    \sa setOverrideCursor(), restoreOverrideCursor()
*/
#ifndef QT_NO_CURSOR
QCursor *QApplication::overrideCursor()
{
    return qApp->d->cursor_list.isEmpty() ? 0 : &qApp->d->cursor_list.first();
}
#endif

/*! \reimp
 */
int QApplication::exec()
{
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::setRootObject(this);
#endif
    return QCoreApplication::exec();
}

/*! \reimp
 */
bool QApplication::notify(QObject *receiver, QEvent *e)
{
    // no events are delivered after ~QCoreApplication() has started
    if (is_app_closing)
        return true;

    if (receiver == 0) {                        // serious error
        qWarning("QApplication::notify: Unexpected null receiver");
        return true;
    }

    Q_ASSERT_X(QThread::currentQThread() == receiver->thread(),
               "QApplication::sendEvent",
               QString("Cannot send events to objects owned by a different thread (%1).  "
                       "Receiver '%2' (of type '%3') was created in thread %4")
               .arg(QString::number((ulong) QThread::currentThread(), 16))
               .arg(receiver->objectName())
               .arg(receiver->metaObject()->className())
               .arg(QString::number((ulong) receiver->thread(), 16))
               .latin1());

#ifdef QT_COMPAT
    if (e->type() == QEvent::ChildRemoved && receiver->d->postedChildInsertedEvents)
       d->removePostedChildInsertedEvents(receiver, static_cast<QChildEvent *>(e)->child());
#endif // QT_COMPAT

    // capture the current mouse/keyboard state
    if(e->spontaneous()) {
        if(e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
            Qt::KeyboardModifier modif = Qt::NoModifier;
            switch(static_cast<QKeyEvent*>(e)->key()) {
            case Qt::Key_Shift:
                modif = Qt::ShiftModifier;
                break;
            case Qt::Key_Control:
                modif = Qt::ControlModifier;
                break;
            case Qt::Key_Meta:
                modif = Qt::MetaModifier;
                break;
            case Qt::Key_Alt:
                modif = Qt::AltModifier;
                break;
            case Qt::Key_NumLock:
                modif = Qt::KeypadModifier;
                break;
            default:
                break;
            }
            if(modif != Qt::NoModifier) {
                if(e->type() == QEvent::KeyPress)
                    QApplicationPrivate::modifier_buttons |= modif;
                else
                    QApplicationPrivate::modifier_buttons &= ~modif;
            }
        } else if(e->type() == QEvent::MouseButtonPress
                  || e->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent*>(e);
            if(me->type() == QEvent::MouseButtonPress)
                QApplicationPrivate::mouse_buttons |= me->button();
            else
                QApplicationPrivate::mouse_buttons &= ~me->button();
        }
    }

    // User input and window activation makes tooltips sleep
    switch (e->type()) {
    case QEvent::ActivationChange:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::FocusOut:
    case QEvent::FocusIn:
        d->toolTipWakeUp.stop();
        d->toolTipFallAsleep.stop();
        break;
    default:
        break;
    }

    bool res = false;
    if (!receiver->isWidgetType()) {
        res = notify_helper(receiver, e);
    } else switch (e->type()) {
#if defined QT_COMPAT && !defined(QT_NO_ACCEL)
    case QEvent::Accel:
    {
        if (d->use_compat()) {
            QKeyEvent* key = (QKeyEvent*) e;
            res = notify_helper(receiver, e);

            if (!res && !key->isAccepted())
                res = d->qt_dispatchAccelEvent((QWidget*)receiver, key);

            // next lines are for compatibility with Qt <= 3.0.x: old
            // QAccel was listening on toplevel widgets
            if (!res && !key->isAccepted() && !((QWidget*)receiver)->isTopLevel())
                res = notify_helper(((QWidget*)receiver)->topLevelWidget(), e);
        }
        break;
    }
#endif //QT_COMPAT && !QT_NO_ACCEL
    case QEvent::ShortcutOverride:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        {
        QWidget* w = static_cast<QWidget*>(receiver);
        QKeyEvent* key = static_cast<QKeyEvent*>(e);
#if defined QT_COMPAT && !defined(QT_NO_ACCEL)
        if (d->use_compat() && d->qt_tryComposeUnicode(w, key))
            break;
#endif
        // Try looking for a Shortcut before sending key events
        if (key->type()==QEvent::KeyPress)
            if (res = qApp->d->shortcutMap.tryShortcutEvent(w, key))
                return res;

        bool def = key->isAccepted();
        while (w) {
            if (def)
                key->accept();
            else
                key->ignore();
            res = notify_helper(w, e);
            if (res || key->isAccepted())
                break;
            if (w->isTopLevel() || !w->parentWidget()
                || (w->testAttribute(Qt::WA_CompositeChild)
                    && w->parentWidget()->testAttribute(Qt::WA_CompositeParent))
               )
                break;
            w = w->parentWidget();
        }
    }
    break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    {
        QWidget* w = (QWidget*)receiver;
        QMouseEvent* mouse = (QMouseEvent*) e;
        QPoint relpos = mouse->pos();

        if (e->spontaneous()) {
            while (w->testAttribute(Qt::WA_CompositeChild)
                   && w->parentWidget()
                   && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)) {
                relpos += w->pos();
                w = w->parentWidget();
            }

            if (e->type() == QEvent::MouseButtonPress) {
                QWidget *fw = w;
                while (fw) {
                    if (fw->isEnabled() && (fw->focusPolicy() & Qt::ClickFocus)) {
                        QFocusEvent::setReason(QFocusEvent::Mouse);
                        fw->setFocus();
                        QFocusEvent::resetReason();
                        break;
                    }
                    if (fw->isTopLevel())
                        break;
                    fw = fw->parentWidget();
                }
            }

            if (e->type() == QEvent::MouseMove) {
                d->toolTipWidget = w;
                d->toolTipPos = relpos;
                d->toolTipGlobalPos = mouse->globalPos();
                d->toolTipWakeUp.start(d->toolTipFallAsleep.isActive()?1:700, this);
            }
        }

        while (w) {
            QMouseEvent me(mouse->type(), relpos, mouse->globalPos(), mouse->button(), mouse->buttons(),
                           mouse->modifiers());
            me.spont = mouse->spontaneous();
            // throw away any mouse-tracking-only mouse events
            if (!w->hasMouseTracking()
                && mouse->type() == QEvent::MouseMove && mouse->buttons() == 0) {
                // but still send them through all application event filters (normally done by notify_helper)
                for (int i = 0; i < d->eventFilters.size(); ++i) {
                    register QObject *obj = d->eventFilters.at(i);
                    if (obj && obj->eventFilter(w, w == receiver ? mouse : &me))
                        break;
                }
                res = true;
                break;
            }
            w->setAttribute(Qt::WA_NoMouseReplay, false);
            res = notify_helper(w, w == receiver ? mouse : &me);
            e->spont = false;
            if (res || w->isTopLevel() || w->testWFlags(Qt::WNoMousePropagation)
                || (w->testAttribute(Qt::WA_CompositeChild)
                    && w->parentWidget()
                    && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)))
                break;

            relpos += w->pos();
            w = w->parentWidget();
        }
        if (res)
            mouse->accept();
        else
            mouse->ignore();
    }
    break;
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
    {
        QWidget* w = (QWidget*)receiver;
        QWheelEvent* wheel = (QWheelEvent*) e;
        QPoint relpos = wheel->pos();

        if (e->spontaneous()) {
            while (w->testAttribute(Qt::WA_CompositeChild)
                   && w->parentWidget()
                   && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)) {
                relpos += w->pos();
                w = w->parentWidget();
            }

            QWidget *fw = w;
            while (fw) {
                if (fw->isEnabled() && (fw->focusPolicy() & Qt::WheelFocus) == Qt::WheelFocus) {
                    QFocusEvent::setReason(QFocusEvent::Mouse);
                    fw->setFocus();
                    QFocusEvent::resetReason();
                    break;
                }
                if (fw->isTopLevel())
                    break;
                fw = fw->parentWidget();
            }
        }

        while (w) {
            QWheelEvent we(relpos, wheel->globalPos(), wheel->delta(), wheel->buttons(),
                           wheel->modifiers(), wheel->orientation());
            we.spont = wheel->spontaneous();
            res = notify_helper(w,  w == receiver ? wheel : &we);
            e->spont = false;
            if (res || w->isTopLevel() || w->testWFlags(Qt::WNoMousePropagation)
                || (w->testAttribute(Qt::WA_CompositeChild)
                    && w->parentWidget()
                    && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)))
                break;

            relpos += w->pos();
            w = w->parentWidget();
        }
        if (res)
            wheel->accept();
        else
            wheel->ignore();
    }
    break;
#endif
    case QEvent::ContextMenu:
    {
        QWidget* w = (QWidget*)receiver;
        QContextMenuEvent *context = (QContextMenuEvent*) e;
        QPoint relpos = context->pos();
        if (e->spontaneous())
            while (w->testAttribute(Qt::WA_CompositeChild)
                   && w->parentWidget()
                   && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)) {
                relpos += w->pos();
                w = w->parentWidget();
            }

        while (w) {
            QContextMenuEvent ce(context->reason(), relpos, context->globalPos());
            ce.spont = e->spontaneous();
            res = notify_helper(w,  w == receiver ? context : &ce);
            e->spont = false;

            if (res || w->isTopLevel() || w->testWFlags(Qt::WNoMousePropagation)
                || (w->testAttribute(Qt::WA_CompositeChild)
                    && w->parentWidget()
                    && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)))
                break;

            relpos += w->pos();
            w = w->parentWidget();
        }
        if (res)
            context->accept();
        else
            context->ignore();
    }
    break;
#if defined (QT_TABLET_SUPPORT)
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    {
        QWidget *w = (QWidget*)receiver;
        QTabletEvent *tablet = (QTabletEvent*)e;
        QPoint relpos = tablet->pos();
        if (e->spontaneous()) {
            while (w->testAttribute(Qt::WA_CompositeChild)
                   && w->parentWidget()
                   && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)) {
                relpos += w->pos();
                w = w->parentWidget();
            }
        }

        while (w) {
            QTabletEvent te(tablet->type(), tablet->pos(), tablet->globalPos(), tablet->hiResPos(),
                            tablet->minHiResX(), tablet->maxHiResX(), tablet->minHiResY(), tablet->maxHiResY(),
                            tablet->device(), tablet->pressure(), tablet->minPressure(), tablet->maxPressure(),
                            tablet->xTilt(), tablet->yTilt(), tablet->modifiers(), tablet->uniqueId());
            te.spont = e->spontaneous();
            res = notify_helper(w, w == receiver ? tablet : &te);
            e->spont = false;
            if (res || w->isTopLevel() || w->testWFlags(Qt::WNoMousePropagation)
                || (w->testAttribute(Qt::WA_CompositeChild)
                    && w->parentWidget()
                    && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)))
                break;

            relpos += w->pos();
            w = w->parentWidget();
        }
        if (res)
            tablet->accept();
        else
            tablet->ignore();
        chokeMouse = tablet->isAccepted();
    }
    break;
#endif

    case QEvent::ToolTip:
    case QEvent::WhatsThis:
    {
        QWidget* w = (QWidget*)receiver;
        QHelpEvent *help = (QHelpEvent*) e;
        QPoint relpos = help->pos();
        if (e->spontaneous())
            while (w->testAttribute(Qt::WA_CompositeChild)
                   && w->parentWidget()
                   && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)) {
                relpos += w->pos();
                w = w->parentWidget();
            }

        while (w) {
            QHelpEvent he(help->type(), relpos, help->globalPos());
            he.spont = e->spontaneous();
            res = notify_helper(w,  w == receiver ? help : &he);
            e->spont = false;

            if (res || w->isTopLevel()
                || (w->testAttribute(Qt::WA_CompositeChild)
                    && w->parentWidget()
                    && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)))
                break;

            relpos += w->pos();
            w = w->parentWidget();
        }
    }
    break;

    case QEvent::StatusTip:
    case QEvent::WhatsThisClicked:
    {
        QWidget *w = (QWidget*)receiver;
        while (w) {
            res = notify_helper(w, e);
            if (res || w->isTopLevel()
                || (w->testAttribute(Qt::WA_CompositeChild)
                    && w->parentWidget()
                    && w->parentWidget()->testAttribute(Qt::WA_CompositeParent)))
                break;
            w = w->parentWidget();
        }
    }
    break;

    default:
        res = notify_helper(receiver, e);
        break;
    }

    return res;
}

bool QApplication::notify_helper(QObject *receiver, QEvent * e)
{
    // send to all application event filters
    for (int i = 0; i < d->eventFilters.size(); ++i) {
        register QObject *obj = d->eventFilters.at(i);
        if (obj && obj->eventFilter(receiver,e))
            return true;
    }

    bool consumed = false;
    bool handled = false;

    if (receiver->isWidgetType()) {
        QWidget *widget = (QWidget*)receiver;

        // toggle HasMouse widget state on enter and leave
        if (e->type() == QEvent::Enter || e->type() == QEvent::DragEnter)
            widget->setAttribute(Qt::WA_UnderMouse, true);
        else if (e->type() == QEvent::Leave || e->type() == QEvent::DragLeave)
            widget->setAttribute(Qt::WA_UnderMouse, false);

        if (QLayout *layout=widget->d->layout) {
            layout->widgetEvent(e);
        }


        // throw away mouse events to disabled widgets
        if (!widget->isEnabled()) {
            switch(e->type()) {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseMove:
                ((QMouseEvent*) e)->ignore();
                consumed = true;
                handled = true;
                break;
#ifndef QT_NO_DRAGANDDROP
            case QEvent::DragEnter:
            case QEvent::DragMove:
                ((QDragMoveEvent*) e)->ignore();
                handled = true;
                break;
            case QEvent::DragLeave:
            case QEvent::DragResponse:
                handled = true;
                break;
            case QEvent::Drop:
                ((QDropEvent*) e)->ignore();
                handled = true;
                break;
#endif
#ifndef QT_NO_WHEELEVENT
            case QEvent::Wheel:
                ((QWheelEvent*) e)->ignore();
                handled = true;
                break;
#endif
            case QEvent::ContextMenu:
                ((QContextMenuEvent*) e)->ignore();
                handled = true;
                break;
            default:
                break;
            }
        }

    }

    if (!handled) {
        // send to all receiver event filters
        if (receiver != this) {
            for (int i = 0; i < receiver->d->eventFilters.size(); ++i) {
                register QObject *obj = receiver->d->eventFilters.at(i);
                if (obj && obj->eventFilter(receiver,e))
                    return true;
            }
        }
        consumed = receiver->event(e);
    }
    e->spont = false;
    return consumed;
}


/*!
  \class QSessionManager qsessionmanager.h
  \brief The QSessionManager class provides access to the session manager.

  \ingroup application
  \ingroup environment

  The session manager is responsible for session management, most
  importantly for interruption and resumption. A "session" is a kind
  of record of the state of the system, e.g. which applications were
  run at start up and which applications are currently running. The
  session manager is used to save the session, e.g. when the machine
  is shut down; and to restore a session, e.g. when the machine is
  started up. Use QSettings to save and restore an individual
  application's settings, e.g. window positions, recently used files,
  etc.

  QSessionManager provides an interface between the application and
  the session manager so that the program can work well with the
  session manager. In Qt, session management requests for action
  are handled by the two virtual functions QApplication::commitData()
  and QApplication::saveState(). Both provide a reference to
  a session manager object as argument, to allow the application
  to communicate with the session manager.

  During a session management action (i.e. within commitData() and
  saveState()), no user interaction is possible \e unless the
  application got explicit permission from the session manager. You
  ask for permission by calling allowsInteraction() or, if it's really
  urgent, allowsErrorInteraction(). Qt does not enforce this, but the
  session manager may.

  You can try to abort the shutdown process by calling cancel(). The
  default commitData() function does this if some top-level window
  rejected its closeEvent().

  For sophisticated session managers provided on Unix/X11, QSessionManager
  offers further possibilites to fine-tune an application's session
  management behavior: setRestartCommand(), setDiscardCommand(),
  setRestartHint(), setProperty(), requestPhase2(). See the respective
  function descriptions for further details.
*/

/*! \enum QSessionManager::RestartHint

  This enum type defines the circumstances under which this
  application wants to be restarted by the session manager. The
  current values are

  \value RestartIfRunning  if the application is still running when
  the session is shut down, it wants to be restarted at the start of
  the next session.

  \value RestartAnyway  the application wants to be started at the
  start of the next session, no matter what. (This is useful for
  utilities that run just after startup and then quit.)

  \value RestartImmediately  the application wants to be started
  immediately whenever it is not running.

  \value RestartNever  the application does not want to be restarted
  automatically.

  The default hint is \c RestartIfRunning.
*/


/*!
  \fn QString QSessionManager::sessionId() const

  Returns the identifier of the current session.

  If the application has been restored from an earlier session, this
  identifier is the same as it was in that earlier session.

  \sa sessionKey(), QApplication::sessionId()
 */

/*!
  \fn QString QSessionManager::sessionKey() const

  Returns the session key in the current session.

  If the application has been restored from an earlier session, this
  key is the same as it was when the previous session ended.

  The session key changes with every call of commitData() or
  saveState().

  \sa sessionId(), QApplication::sessionKey()
 */

/*!
    \fn void* QSessionManager::handle() const

    \internal
*/

/*!
  \fn bool QSessionManager::allowsInteraction()

  Asks the session manager for permission to interact with the
  user. Returns true if interaction is permitted; otherwise
  returns false.

  The rationale behind this mechanism is to make it possible to
  synchronize user interaction during a shutdown. Advanced session
  managers may ask all applications simultaneously to commit their
  data, resulting in a much faster shutdown.

  When the interaction is completed we strongly recommend releasing the
  user interaction semaphore with a call to release(). This way, other
  applications may get the chance to interact with the user while your
  application is still busy saving data. (The semaphore is implicitly
  released when the application exits.)

  If the user decides to cancel the shutdown process during the
  interaction phase, you must tell the session manager that this has
  happened by calling cancel().

  Here's an example of how an application's QApplication::commitData()
  might be implemented:

\code
void MyApplication::commitData(QSessionManager& sm) {
    if (sm.allowsInteraction()) {
        switch (QMessageBox::warning(
                    yourMainWindow,
                    tr("Application Name"),
                    tr("Save changes to document Foo?"),
                    tr("&Yes"),
                    tr("&No"),
                    tr("Cancel"),
                    0, 2)) {
        case 0: // yes
            sm.release();
            // save document here; if saving fails, call sm.cancel()
            break;
        case 1: // continue without saving
            break;
        default: // cancel
            sm.cancel();
            break;
        }
    } else {
        // we did not get permission to interact, then
        // do something reasonable instead.
    }
}
\endcode

  If an error occurred within the application while saving its data,
  you may want to try allowsErrorInteraction() instead.

  \sa QApplication::commitData(), release(), cancel()
*/


/*!
  \fn bool QSessionManager::allowsErrorInteraction()

  This is similar to allowsInteraction(), but also tells the session
  manager that an error occurred. Session managers may give error
  interaction request higher priority, which means that it is more likely
  that an error interaction is permitted. However, you are still not
  guaranteed that the session manager will allow interaction.

  \sa allowsInteraction(), release(), cancel()
*/

/*!
  \fn void QSessionManager::release()

  Releases the session manager's interaction semaphore after an
  interaction phase.

  \sa allowsInteraction(), allowsErrorInteraction()
*/

/*!
  \fn void QSessionManager::cancel()

  Tells the session manager to cancel the shutdown process.  Applications
  should not call this function without first asking the user.

  \sa allowsInteraction(), allowsErrorInteraction()

*/

/*!
  \fn void QSessionManager::setRestartHint(RestartHint hint)

  Sets the application's restart hint to \a hint. On application
  startup the hint is set to \c RestartIfRunning.

  Note that these flags are only hints, a session manager may or may
  not respect them.

  We recommend setting the restart hint in QApplication::saveState()
  because most session managers perform a checkpoint shortly after an
  application's startup.

  \sa restartHint()
*/

/*!
  \fn QSessionManager::RestartHint QSessionManager::restartHint() const

  Returns the application's current restart hint. The default is
  \c RestartIfRunning.

  \sa setRestartHint()
*/

/*!
  \fn void QSessionManager::setRestartCommand(const QStringList& command)

  If the session manager is capable of restoring sessions it will
  execute \a command in order to restore the application. The command
  defaults to

  \code
        appname -session id
  \endcode

  The \c -session option is mandatory; otherwise QApplication cannot
  tell whether it has been restored or what the current session
  identifier is. See QApplication::isSessionRestored() and
  QApplication::sessionId() for details.

  If your application is very simple, it may be possible to store the
  entire application state in additional command line options. This
  is usually a very bad idea because command lines are often limited
  to a few hundred bytes. Instead, use QSettings, or temporary files
  or a database for this purpose. By marking the data with the unique
  sessionId(), you will be able to restore the application in a future
  session.

  \sa restartCommand(), setDiscardCommand(), setRestartHint()
*/

/*!
  \fn QStringList QSessionManager::restartCommand() const

  Returns the currently set restart command.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = mySession.restartCommand();
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

  \sa setRestartCommand(), restartHint()
*/

/*!
  \fn void QSessionManager::setDiscardCommand(const QStringList& list)

  Sets the discard command to the given \a list.

  \sa discardCommand(), setRestartCommand()
*/


/*!
  \fn QStringList QSessionManager::discardCommand() const

  Returns the currently set discard command.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = mySession.discardCommand();
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

  \sa setDiscardCommand(), restartCommand(), setRestartCommand()
*/

/*!
  \fn void QSessionManager::setManagerProperty(const QString &name, const QString &value)
  \overload

  Low-level write access to the application's identification and state
  records are kept in the session manager.

    The property called \a name has its value set to the string \a value.
*/

/*!
  \fn void QSessionManager::setManagerProperty(const QString& name,
                                                const QStringList& value)

  Low-level write access to the application's identification and state
  record are kept in the session manager.

    The property called \a name has its value set to the string list \a value.
*/

/*!
  \fn bool QSessionManager::isPhase2() const

  Returns true if the session manager is currently performing a second
  session management phase; otherwise returns false.

  \sa requestPhase2()
*/

/*!
  \fn void QSessionManager::requestPhase2()

  Requests a second session management phase for the application. The
  application may then return immediately from the
  QApplication::commitData() or QApplication::saveState() function,
  and they will be called again once most or all other applications have
  finished their session management.

  The two phases are useful for applications such as the X11 window manager
  that need to store information about another application's windows
  and therefore have to wait until these applications have completed their
  respective session management tasks.

  Note that if another application has requested a second phase it
  may get called before, simultaneously with, or after your
  application's second phase.

  \sa isPhase2()
*/

/*!
  \fn Qt::Alignment QApplication::horizontalAlignment(Qt::Alignment align)

  Strips out vertical alignment flags and transforms an
  alignment \a align of Qt::AlignAuto into Qt::AlignLeft or
  Qt::AlignRight according to the language used. The other horizontal
  alignment flags are left untouched.
*/

/*****************************************************************************
  Stubbed session management support
 *****************************************************************************/
#ifndef QT_NO_SESSIONMANAGER
#if defined(QT_NO_SM_SUPPORT) || defined(Q_WS_WIN) || defined(Q_WS_MAC) || defined(Q_WS_QWS)

class QSessionManagerPrivate : public QObjectPrivate
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QString sessionKey;
    QSessionManager::RestartHint restartHint;
};

QSessionManager* qt_session_manager_self = 0;
QSessionManager::QSessionManager(QApplication * app, QString &id, QString &key)
    : QObject(*new QSessionManagerPrivate, app)
{
    setObjectName("qt_sessionmanager");
    qt_session_manager_self = this;
#if defined(Q_WS_WIN) && !defined(Q_OS_TEMP)
    wchar_t guidstr[40];
    GUID guid;
    CoCreateGuid(&guid);
    StringFromGUID2(guid, guidstr, 40);
    id = QString::fromUtf16((ushort*)guidstr);
    CoCreateGuid(&guid);
    StringFromGUID2(guid, guidstr, 40);
    key = QString::fromUtf16((ushort*)guidstr);
#endif
    d->sessionId = id;
    d->sessionKey = key;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    qt_session_manager_self = 0;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

QString QSessionManager::sessionKey() const
{
    return d->sessionKey;
}


#if defined(Q_WS_X11) || defined(Q_WS_MAC)
void* QSessionManager::handle() const
{
    return 0;
}
#endif

#if !defined(Q_WS_WIN)
bool QSessionManager::allowsInteraction()
{
    return true;
}

bool QSessionManager::allowsErrorInteraction()
{
    return true;
}
void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}
#endif


void QSessionManager::setRestartHint(QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand(const QStringList& command)
{
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand(const QStringList& command)
{
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    return d->discardCommand;
}

void QSessionManager::setManagerProperty(const QString&, const QString&)
{
}

void QSessionManager::setManagerProperty(const QString&, const QStringList&)
{
}

bool QSessionManager::isPhase2() const
{
    return false;
}

void QSessionManager::requestPhase2()
{
}

#endif // QT_NO_SM_SUPPORT
#endif //QT_NO_SESSIONMANAGER

/*!
    \enum QApplication::ColorMode

    \compat

    \value NormalColors
    \value CustomColors
*/

/*!
    \fn Qt::MacintoshVersion QApplication::macVersion()

    Use QSysInfo::MacintoshVersion instead.
*/

/*!
    \fn ColorMode QApplication::colorMode()

    Use ColorMode(colorSpec()) instead.
*/

/*!
    \fn void QApplication::setColorMode(ColorMode mode)

    Use setColorSpec(ColorSpec(mode)) instead.
*/

/*!
    \fn bool QApplication::hasGlobalMouseTracking()

    Always returns true.
###
*/

/*!
    \fn void QApplication::setGlobalMouseTracking(bool b)

###
*/

/*!
    \fn void QApplication::flushX()

    Use flush() instead.
*/

/*!
    \fn void QApplication::setWinStyleHighlightColor(const QColor &c)

    Use the palette instead.

    \oldcode
    app.setWinStyleHighlightColor(color);
    \newcode
    QPalette palette(qApp->palette());
    palette.setColor(QPalette::Highlight, color);
    qApp->setPalette(palette);
    \endcode
*/

/*!
    \fn void QApplication::setPalette(const QPalette &pal, bool b, const char* className = 0)

    Use the two-argument overload instead.
*/

/*!
    \fn void QApplication::setFont(const QFont &font, bool b, const char* className = 0)

    Use the two-argument overload instead.
*/

/*!
    \fn const QColor &QApplication::winStyleHighlightColor()

    Use qApp->palette().color(QPalette::Active, QPalette::Highlight)
    instead.
*/

/*!
    \fn QWidget *QApplication::widgetAt(int x, int y, bool child)

    Use the two-argument widgetAt() overload to get the child widget.
    To get the top-level widget do this:
    \code
    QWidget *widget = qApp->widgetAt(x, y);
    if (widget)
        widget = widget->topLevelWidget();
    \endcode
*/

/*!
    \fn QWidget *QApplication::widgetAt(const QPoint &point, bool child)

    Use the single-argument widgetAt() overload to get the child widget.
    To get the top-level widget do this:
    \code
    QWidget *widget = qApp->widgetAt(point);
    if (widget)
        widget = widget->topLevelWidget();
    \endcode
*/

QWidget *QApplication::mainWidget() const
{
    return QApplicationPrivate::main_widget;
}

bool QApplication::inPopupMode() const
{
    return QApplicationPrivate::popupWidgets != 0;
}

