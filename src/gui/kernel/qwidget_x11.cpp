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

#include "qevent.h"
#include "qwidget.h"
#include "qdesktopwidget.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qnamespace.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include "qdatetime.h"
#include "qcursor.h"
#include "qstack.h"
#include "qcleanuphandler.h"
#include "qcolormap.h"

// Paint event clipping magic
extern void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

extern bool qt_has_accelerated_xrender; // declared in qapplication_x11.cpp

#include <private/qpaintengine_x11_p.h>
#include "qt_x11_p.h"
#include "qx11info_x11.h"

#include <stdlib.h>

// defined in qapplication_x11.cpp
bool qt_wstate_iconified(WId);
void qt_updated_rootinfo();

// from qpaintengine_x11.cpp
extern void qt_erase_background(QPaintDevice *pd, int screen,
                                int x, int y, int width, int height,
                                const QBrush &brush, int offx, int offy);


#if !defined(QT_NO_IM)
#include "qinputcontext.h"
#include "qinputcontextfactory.h"
#endif

#include "qwidget_p.h"
#define d d_func()
#define q q_func()

#define XCOORD_MAX 32767
#define WRECT_MAX 8191

extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;


int qt_x11_create_desktop_on_screen = -1;


// MWM support
struct QtMWMHints {
    ulong flags, functions, decorations;
    long input_mode;
    ulong status;
};

enum {
    MWM_HINTS_FUNCTIONS   = (1L << 0),

    MWM_FUNC_ALL      = (1L << 0),
    MWM_FUNC_RESIZE   = (1L << 1),
    MWM_FUNC_MOVE     = (1L << 2),
    MWM_FUNC_MINIMIZE = (1L << 3),
    MWM_FUNC_MAXIMIZE = (1L << 4),
    MWM_FUNC_CLOSE    = (1L << 5),

    MWM_HINTS_DECORATIONS = (1L << 1),

    MWM_DECOR_ALL      = (1L << 0),
    MWM_DECOR_BORDER   = (1L << 1),
    MWM_DECOR_RESIZEH  = (1L << 2),
    MWM_DECOR_TITLE    = (1L << 3),
    MWM_DECOR_MENU     = (1L << 4),
    MWM_DECOR_MINIMIZE = (1L << 5),
    MWM_DECOR_MAXIMIZE = (1L << 6),

    MWM_HINTS_INPUT_MODE = (1L << 2),

    MWM_INPUT_FULL_APPLICATION_MODAL    = 3L
};


static QtMWMHints GetMWMHints(Display *display, Window window)
{
    QtMWMHints mwmhints;

    Atom type;
    int format;
    ulong nitems, bytesLeft;
    uchar *data = 0;
    if ((XGetWindowProperty(display, window, ATOM(_MOTIF_WM_HINTS), 0, 5, false,
                            ATOM(_MOTIF_WM_HINTS), &type, &format, &nitems, &bytesLeft,
                            &data) == Success)
        && (type == ATOM(_MOTIF_WM_HINTS)
            && format == 32
            && nitems >= 5)) {
        mwmhints = *(reinterpret_cast<QtMWMHints *>(data));
    } else {
        mwmhints.flags = 0L;
        mwmhints.functions = MWM_FUNC_ALL;
        mwmhints.decorations = MWM_DECOR_ALL;
        mwmhints.input_mode = 0L;
        mwmhints.status = 0L;
    }

    if (data)
        XFree(data);

    return mwmhints;
}


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

extern bool qt_broken_wm;

// defined in qapplication_x11.cpp
extern bool qt_net_supports(Atom);

const uint stdWidgetEventMask =                        // X event mask
        (uint)(
            KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask |
            KeymapStateMask |
            ButtonMotionMask | PointerMotionMask |
            EnterWindowMask | LeaveWindowMask |
            FocusChangeMask |
            ExposureMask |
            PropertyChangeMask |
            StructureNotifyMask
       );

const uint stdDesktopEventMask =                        // X event mask
       (uint)(
           KeymapStateMask |
           EnterWindowMask | LeaveWindowMask |
           PropertyChangeMask
      );


/*
  The qt_ functions below are implemented in qwidgetcreate_x11.cpp.
*/

Window qt_XCreateWindow(const QWidget *creator,
                         Display *display, Window parent,
                         int x, int y, uint w, uint h,
                         int borderwidth, int depth,
                         uint windowclass, Visual *visual,
                         ulong valuemask, XSetWindowAttributes *attributes);
Window qt_XCreateSimpleWindow(const QWidget *creator,
                               Display *display, Window parent,
                               int x, int y, uint w, uint h, int borderwidth,
                               ulong border, ulong background);
void qt_XDestroyWindow(const QWidget *destroyer,
                        Display *display, Window window);


static void qt_insert_sip(QWidget* scrolled_widget, int dx, int dy)
{
    QX11Data::ScrollInProgress sip = { X11->sip_serial++, scrolled_widget, dx, dy };
    X11->sip_list.append(sip);

    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = scrolled_widget->winId();
    client_message.format = 32;
    client_message.message_type = ATOM(_QT_SCROLL_DONE);
    client_message.data.l[0] = sip.id;

    XSendEvent(X11->display, scrolled_widget->winId(), False, NoEventMask,
        (XEvent*)&client_message);
}

static int qt_sip_count(QWidget* scrolled_widget)
{
    int sips=0;

    for (int i = 0; i < X11->sip_list.size(); ++i) {
        const QX11Data::ScrollInProgress &sip = X11->sip_list.at(i);
        if (sip.scrolled_widget == scrolled_widget)
            sips++;
    }

    return sips;
}


static void create_wm_client_leader()
{
    if (X11->wm_client_leader) return;

    X11->wm_client_leader =
        XCreateSimpleWindow(X11->display,
                             QX11Info::appRootWindow(),
                             0, 0, 1, 1, 0, 0, 0);

    // set client leader property to itself
    XChangeProperty(X11->display,
                     X11->wm_client_leader, ATOM(WM_CLIENT_LEADER),
                     XA_WINDOW, 32, PropModeReplace,
                     (unsigned char *)&X11->wm_client_leader, 1);

    // If we are session managed, inform the window manager about it
    QByteArray session = qApp->sessionId().toLatin1();
    if (!session.isEmpty()) {
        XChangeProperty(X11->display,
                         X11->wm_client_leader, ATOM(SM_CLIENT_ID),
                         XA_STRING, 8, PropModeReplace,
                         (unsigned char *)session.data(), session.size());
    }
}


Q_GUI_EXPORT void qt_x11_enforce_cursor(QWidget * w)
{
    if (w->testAttribute(Qt::WA_SetCursor)) {
        QCursor *oc = QApplication::overrideCursor();
        if (oc) {
            XDefineCursor(X11->display, w->winId(), oc->handle());
        } else if (w->isEnabled()) {
            XDefineCursor(X11->display, w->winId(), w->cursor().handle());
        } else {
            // enforce the windows behavior of clearing the cursor on
            // disabled widgets
            XDefineCursor(X11->display, w->winId(), XNone);
        }
    } else {
        XDefineCursor(X11->display, w->winId(), XNone);
    }
}

Q_GUI_EXPORT void qt_wait_for_window_manager(QWidget* w)
{
    QApplication::flush();
    XEvent ev;
    QTime t;
    t.start();
    while (!XCheckTypedWindowEvent(X11->display, w->winId(), ReparentNotify, &ev)) {
        if (XCheckTypedWindowEvent(X11->display, w->winId(), MapNotify, &ev))
            break;
        if (t.elapsed() > 500)
            return; // give up, no event available
        qApp->syncX(); // non-busy wait
    }
    qApp->x11ProcessEvent(&ev);
    if (XCheckTypedWindowEvent(X11->display, w->winId(), ConfigureNotify, &ev))
        qApp->x11ProcessEvent(&ev);
}

static void qt_change_net_wm_state(const QWidget* w, bool set, Atom one, Atom two = 0)
{
    if (!w->isShown()) // not managed by the window manager
        return;

    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = ATOM(_NET_WM_STATE);
    e.xclient.display = X11->display;
    e.xclient.window = w->winId();
    e.xclient.format = 32;
    e.xclient.data.l[0] = set ? 1 : 0;
    e.xclient.data.l[1] = one;
    e.xclient.data.l[2] = two;
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;
    XSendEvent(X11->display, RootWindow(X11->display, w->x11Info().screen()),
               false, (SubstructureNotifyMask | SubstructureRedirectMask), &e);
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
    if (testWState(Qt::WState_Created) && window == 0)
        return;

    // set created flag
    setWState(Qt::WState_Created);

    bool popup = testWFlags(Qt::WType_Popup);
    bool dialog = testWFlags(Qt::WType_Dialog);
    bool desktop = testWFlags(Qt::WType_Desktop);

    // top-level widget
    if (!parentWidget() || parentWidget()->isDesktop())
        setWFlags(Qt::WType_TopLevel);

    // these are top-level, too
    if (dialog || popup || desktop || testWFlags(Qt::WStyle_Splash))
        setWFlags(Qt::WType_TopLevel);

    // a popup stays on top
    if (popup)
        setWFlags(Qt::WStyle_StaysOnTop);

    bool topLevel = testWFlags(Qt::WType_TopLevel);
    Window parentw, destroyw = 0;
    WId           id;

    // always initialize
    if (!window)
        initializeWindow = true;

    if (desktop &&
        qt_x11_create_desktop_on_screen >= 0 &&
        qt_x11_create_desktop_on_screen != d->xinfo.screen()) {
        // desktop on a certain screen other than the default requested
        QX11InfoData *xd = &X11->screens[qt_x11_create_desktop_on_screen];
        d->xinfo.setX11Data(xd);
    } else if (parentWidget() &&  parentWidget()->d->xinfo.screen() != d->xinfo.screen()) {
        d->xinfo = parentWidget()->d->xinfo;
    }

    //get display, screen number, root window and desktop geometry for
    //the current screen
    Display *dpy = X11->display;
    int scr = d->xinfo.screen();
    Window root_win = RootWindow(dpy, scr);
    int sw = DisplayWidth(dpy,scr);
    int sh = DisplayHeight(dpy,scr);

    if (desktop) {                                // desktop widget
        dialog = popup = false;                        // force these flags off
        data->crect.setRect(0, 0, sw, sh);
    } else if (topLevel) {                        // calc pos/size from screen
        data->crect.setRect(sw/4, 3*sh/10, sw/2, 4*sh/10);
    } else {                                        // child widget
        data->crect.setRect(0, 0, 100, 30);
    }

    parentw = topLevel ? root_win : parentWidget()->winId();

    XSetWindowAttributes wsa;

    if (window) {                                // override the old window
        if (destroyOldWindow)
            destroyw = data->winid;
        id = window;
        d->setWinId(window);
        XWindowAttributes a;
        XGetWindowAttributes(dpy, window, &a);
        data->crect.setRect(a.x, a.y, a.width, a.height);

        if (a.map_state == IsUnmapped)
            clearWState(Qt::WState_Visible);
        else
            setWState(Qt::WState_Visible);

        QX11InfoData* xd = d->xinfo.getX11Data(true);

        // find which screen the window is on...
        xd->screen = QX11Info::appScreen(); // by default, use the default :)
        int i;
        for (i = 0; i < ScreenCount(dpy); i++) {
            if (RootWindow(dpy, i) == a.root) {
                xd->screen = i;
                break;
            }
        }

        xd->depth = a.depth;
        xd->cells = DisplayCells(dpy, xd->screen);
        xd->visual = a.visual;
        xd->defaultVisual = (XVisualIDFromVisual((Visual *) a.visual) ==
                             XVisualIDFromVisual((Visual *) QX11Info::appVisual(d->xinfo.screen())));
        xd->colormap = a.colormap;
        xd->defaultColormap = (a.colormap == QX11Info::appColormap(d->xinfo.screen()));
        d->xinfo.setX11Data(xd);
    } else if (desktop) {                        // desktop widget
        id = (WId)parentw;                        // id = root window
        QWidget *otherDesktop = find(id);        // is there another desktop?
        if (otherDesktop && otherDesktop->testWFlags(Qt::WPaintDesktop)) {
            otherDesktop->d->setWinId(0);        // remove id from widget mapper
            d->setWinId(id);                     // make sure otherDesktop is
            otherDesktop->d->setWinId(id);       // found first
        } else {
            d->setWinId(id);
        }
    } else {
        if (d->xinfo.defaultVisual() && d->xinfo.defaultColormap()) {
            id = (WId)qt_XCreateSimpleWindow(this, dpy, parentw,
                                             data->crect.left(), data->crect.top(),
                                             data->crect.width(), data->crect.height(),
                                             0,
                                             BlackPixel(dpy, d->xinfo.screen()),
                                             WhitePixel(dpy, d->xinfo.screen()));
        } else {
            wsa.background_pixel = WhitePixel(dpy, d->xinfo.screen());
            wsa.border_pixel = BlackPixel(dpy, d->xinfo.screen());
            wsa.colormap = d->xinfo.colormap();
            id = (WId)qt_XCreateWindow(this, dpy, parentw,
                                       data->crect.left(), data->crect.top(),
                                       data->crect.width(), data->crect.height(),
                                       0, d->xinfo.depth(), InputOutput,
                                       (Visual *) d->xinfo.visual(),
                                       CWBackPixel|CWBorderPixel|CWColormap,
                                       &wsa);
        }

        d->setWinId(id);                                // set widget id/handle + hd
    }

#ifndef QT_NO_XFT
    if (d->xft_hd) {
        XftDrawDestroy((XftDraw *) d->xft_hd);
        d->xft_hd = 0;
    }

    if (X11->has_xft) {
        d->xft_hd = (Qt::HANDLE)
                    XftDrawCreate(dpy, id, (Visual *) d->xinfo.visual(), d->xinfo.colormap());
    }
#endif // QT_NO_XFT

    // NET window types
    long net_wintypes[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int curr_wintype = 0;

    QtMWMHints mwmhints;
    mwmhints.flags = 0L;
    mwmhints.functions = MWM_FUNC_ALL;
    mwmhints.decorations = MWM_DECOR_ALL;
    mwmhints.input_mode = 0L;
    mwmhints.status = 0L;

    if (topLevel && ! (desktop || popup)) {
        ulong wsa_mask = 0;

        if (testWFlags(Qt::WStyle_Splash)) {
            if (qt_net_supports(ATOM(_NET_WM_WINDOW_TYPE_SPLASH))) {
                clearWFlags(Qt::WX11BypassWM);
                net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_SPLASH);
            } else {
                setWFlags(Qt::WX11BypassWM | Qt::WStyle_Tool | Qt::WStyle_NoBorder);
            }
        }
        if (testWFlags(Qt::WStyle_Customize)) {
            mwmhints.decorations = 0L;
            mwmhints.flags |= MWM_HINTS_DECORATIONS;

            if (testWFlags(Qt::WStyle_NoBorder)) {
                // override netwm type - quick and easy for KDE noborder
                net_wintypes[curr_wintype++] = ATOM(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE);
            } else {
                if (testWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_DialogBorder)) {
                    mwmhints.decorations |= MWM_DECOR_BORDER;
                    mwmhints.decorations |= MWM_DECOR_RESIZEH;
                }

                if (testWFlags(Qt::WStyle_Title))
                    mwmhints.decorations |= MWM_DECOR_TITLE;

                if (testWFlags(Qt::WStyle_SysMenu))
                    mwmhints.decorations |= MWM_DECOR_MENU;

                if (testWFlags(Qt::WStyle_Minimize))
                    mwmhints.decorations |= MWM_DECOR_MINIMIZE;

                if (testWFlags(Qt::WStyle_Maximize))
                    mwmhints.decorations |= MWM_DECOR_MAXIMIZE;
            }

            if (testWFlags(Qt::WStyle_Tool)) {
                wsa.save_under = True;
                wsa_mask |= CWSaveUnder;
            }
        } else if (testWFlags(Qt::WType_Dialog)) {
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu | Qt::WStyle_ContextHelp);
        } else {
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_MinMax | Qt::WStyle_SysMenu);
        }

        // ### need a better way to do this
        if (inherits("QMenu")) {
            // menu netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_MENU);
        } else if (inherits("QToolBar")) {
            // toolbar netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_TOOLBAR);
        } else if (testWFlags(Qt::WStyle_Customize) && testWFlags(Qt::WStyle_Tool)) {
            // utility netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_UTILITY);
        }

        if (dialog) // dialog netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_DIALOG);
        // normal netwm type - default
        net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_NORMAL);

        if (testWFlags(Qt::WX11BypassWM)) {
            wsa.override_redirect = True;
            wsa_mask |= CWOverrideRedirect;
        }

        if (wsa_mask && initializeWindow)
            XChangeWindowAttributes(dpy, id, wsa_mask, &wsa);
    } else {
        if (! testWFlags(Qt::WStyle_Customize))
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title |
                      Qt::WStyle_MinMax | Qt::WStyle_SysMenu);
    }


    if (!initializeWindow) {
        // do no initialization
    } else if (popup) {                        // popup widget
        wsa.override_redirect = True;
        wsa.save_under = True;
        XChangeWindowAttributes(dpy, id, CWOverrideRedirect | CWSaveUnder,
                                &wsa);
    } else if (topLevel && !desktop) {        // top-level widget
        if (!X11->wm_client_leader)
            create_wm_client_leader();

        // real parent
        QWidget *p = parentWidget();
        if (p)
            p = p->topLevelWidget();

        if (dialog || testWFlags(Qt::WStyle_DialogBorder) || testWFlags(Qt::WStyle_Tool)) {
            if (p) {
                // transient for window
                XSetTransientForHint(dpy, id, p->winId());
            } else {
                // transient for group
                XSetTransientForHint(dpy, id, X11->wm_client_leader);
            }
        }

        XSizeHints size_hints;
        size_hints.flags = USSize | PSize | PWinGravity;
        size_hints.x = data->crect.left();
        size_hints.y = data->crect.top();
        size_hints.width = data->crect.width();
        size_hints.height = data->crect.height();
        size_hints.win_gravity =
            QApplication::isRightToLeft() ? NorthEastGravity : NorthWestGravity;

        XWMHints wm_hints;                        // window manager hints
        wm_hints.flags = InputHint | StateHint | WindowGroupHint;
        wm_hints.input = True;
        wm_hints.initial_state = NormalState;
        wm_hints.window_group = X11->wm_client_leader;

        XClassHint class_hint;
        QByteArray appName = qAppName().toLatin1();
        class_hint.res_name = appName.data(); // application name
        class_hint.res_class = const_cast<char *>(qAppClass());   // application class

        XSetWMProperties(dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints, &class_hint);

        XResizeWindow(dpy, id, data->crect.width(), data->crect.height());
        XStoreName(dpy, id, appName.data());
        Atom protocols[4];
        int n = 0;
        protocols[n++] = ATOM(WM_DELETE_WINDOW);        // support del window protocol
        protocols[n++] = ATOM(WM_TAKE_FOCUS);                // support take focus window protocol
        protocols[n++] = ATOM(_NET_WM_PING);                // support _NET_WM_PING protocol
        if (testWFlags(Qt::WStyle_ContextHelp))
            protocols[n++] = ATOM(_NET_WM_CONTEXT_HELP);
        XSetWMProtocols(dpy, id, protocols, n);

        // set mwm hints
        if (mwmhints.flags != 0l) {
            XChangeProperty(dpy, id, ATOM(_MOTIF_WM_HINTS), ATOM(_MOTIF_WM_HINTS), 32,
                            PropModeReplace, (unsigned char *) &mwmhints, 5);
        } else {
            XDeleteProperty(dpy, id, ATOM(_MOTIF_WM_HINTS));
        }

        // set _NET_WM_WINDOW_TYPE
        if (curr_wintype > 0)
            XChangeProperty(dpy, id, ATOM(_NET_WM_WINDOW_TYPE), XA_ATOM, 32, PropModeReplace,
                            (unsigned char *) net_wintypes, curr_wintype);
        else
            XDeleteProperty(dpy, id, ATOM(_NET_WM_WINDOW_TYPE));

        // set _NET_WM_PID
        long curr_pid = getpid();
        XChangeProperty(dpy, id, ATOM(_NET_WM_PID), XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char *) &curr_pid, 1);

        // when we create a toplevel widget, the frame strut should be dirty
        data->fstrut_dirty = 1;

        // declare the widget's object name as window role
        QByteArray objName = objectName().toLocal8Bit();
        XChangeProperty(dpy, id,
                        ATOM(WM_WINDOW_ROLE), XA_STRING, 8, PropModeReplace,
                        (unsigned char *)objName.constData(), objName.length());

        // set client leader property
        XChangeProperty(dpy, id, ATOM(WM_CLIENT_LEADER),
                        XA_WINDOW, 32, PropModeReplace,
                        (unsigned char *)&X11->wm_client_leader, 1);
    } else {
        // non-toplevel widgets don't have a frame, so no need to
        // update the strut
        data->fstrut_dirty = 0;
    }

    if (initializeWindow) {
        // don't erase when resizing
        wsa.bit_gravity = QApplication::isRightToLeft() ? NorthEastGravity : NorthWestGravity;
        XChangeWindowAttributes(dpy, id, CWBitGravity, &wsa);
    }

    // set X11 event mask
    if (desktop) {
        QWidget* main_desktop = find(id);
        if (main_desktop->testWFlags(Qt::WPaintDesktop))
            XSelectInput(dpy, id, stdDesktopEventMask | ExposureMask);
        else
            XSelectInput(dpy, id, stdDesktopEventMask);
    } else {
        XSelectInput(dpy, id, stdWidgetEventMask);
#if !defined (QT_NO_TABLET_SUPPORT)
        TabletDeviceDataList *tablet_list = qt_tablet_devices();
        for (int i = 0; i < tablet_list->size(); ++i) {
            TabletDeviceData tablet = tablet_list->at(i);
            XSelectExtensionEvent(dpy, id, reinterpret_cast<XEventClass*>(tablet.eventList),
                                  tablet.eventCount);
        }
#endif
    }

    if (desktop) {
        setWState(Qt::WState_Visible);
    } else if (topLevel) {                        // set X cursor
        setAttribute(Qt::WA_SetCursor);
        if (initializeWindow)
            qt_x11_enforce_cursor(this);
    }

    if (destroyw)
        qt_XDestroyWindow(this, dpy, destroyw);

    // newly created windows are positioned at the window system's
    // (0,0) position. If the parent uses wrect mapping to expand the
    // coordinate system, we must also adjust this widget's window
    // system position
    if (!topLevel && !parentWidget()->data->wrect.topLeft().isNull())
        d->setWSGeometry();

#if !defined(QT_NO_IM)
    d->ic = 0;
#endif
}


/*!
    Frees up window system resources. Destroys the widget window if \a
    destroyWindow is true.

    destroy() calls itself recursively for all the child widgets,
    passing \a destroySubWindows for the \a destroyWindow parameter.
    To have more control over destruction of subwidgets, destroy
    subwidgets selectively first.

    This function is usually called from the QWidget destructor.
*/

void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    d->deactivateWidgetCleanup();
    if (testWState(Qt::WState_Created)) {
        clearWState(Qt::WState_Created);
        QObjectList childs = children();
        for (int i = 0; i < childs.size(); ++i) { // destroy all widget children
            register QObject *obj = childs.at(i);
            if (obj->isWidgetType())
                static_cast<QWidget*>(obj)->destroy(destroySubWindows,
                                                    destroySubWindows);
        }
        if (mouseGrb == this)
            releaseMouse();
        if (keyboardGrb == this)
            releaseKeyboard();
        if (isTopLevel())
            X11->deferred_map.removeAll(this);
        if (testWFlags(Qt::WShowModal))                // just be sure we leave modal
            qt_leave_modal(this);
        else if (testWFlags(Qt::WType_Popup))
            qApp->closePopup(this);

#ifndef QT_NO_XFT
        if (d->xft_hd) {
            if (destroyWindow)
                XftDrawDestroy((XftDraw *) d->xft_hd);
            else
                free((void*) d->xft_hd);
            d->xft_hd = 0;
        }
#endif // QT_NO_XFT

        if (testWFlags(Qt::WType_Desktop)) {
            if (acceptDrops())
                X11->dndEnable(this, false);
        } else {
            if (destroyWindow)
                qt_XDestroyWindow(this, X11->display, data->winid);
        }
        d->setWinId(0);

        extern void qPRCleanup(QWidget *widget); // from qapplication_x11.cpp
        if (testWState(Qt::WState_Reparented))
            qPRCleanup(this);

	if(d->ic) {
	    delete d->ic;
	} else {
	    // release previous focus information participating with
	    // preedit preservation of qic
	    QInputContext *qic = inputContext();
	    if (qic)
		qic->widgetDestroyed(this);
	}
    }
}

void QWidgetPrivate::setParent_sys(QWidget *parent, Qt::WFlags f)
{
    extern void qPRCreate(const QWidget *, Window);

    QCursor oldcurs;
    bool setcurs = q->testAttribute(Qt::WA_SetCursor);
    if (setcurs) {
        oldcurs = q->cursor();
        q->unsetCursor();
    }

    // dnd unregister (we will register again below)
    bool accept_drops = q->acceptDrops();
    q->setAcceptDrops(false);

    QWidget *oldparent = q->parentWidget();
    WId old_winid = q->data->winid;
    if (q->testWFlags(Qt::WType_Desktop))
        old_winid = 0;
    setWinId(0);

    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    XUnmapWindow(X11->display, old_winid);
    XReparentWindow(X11->display, old_winid, RootWindow(X11->display, xinfo.screen()), 0, 0);

    if (q->isTopLevel() || !parent) // we are toplevel, or reparenting to toplevel
        topData()->parentWinId = 0;

    QObjectPrivate::setParent_helper(parent);
    bool     enable = q->isEnabled();                // remember status
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize    s            = q->size();
    QString capt = q->windowTitle();
    q->data->widget_flags = f;
    q->clearWState(Qt::WState_Created | Qt::WState_Visible | Qt::WState_Hidden | Qt::WState_ExplicitShowHide);
    q->create();
    if (q->isTopLevel() || (!parent || parent->isVisible()))
        q->setWState(Qt::WState_Hidden);

    QObjectList chlist = q->children();
    for (int i = 0; i < chlist.size(); ++i) { // reparent children
        QObject *obj = chlist.at(i);
        if (obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if (!w->isTopLevel()) {
                XReparentWindow(X11->display, w->winId(), q->winId(),
                                w->geometry().x(), w->geometry().y());
            } else if (w->isPopup()
                        || w->testWFlags(Qt::WStyle_DialogBorder)
                        || w->testWFlags(Qt::WType_Dialog)
                        || w->testWFlags(Qt::WStyle_Tool)) {
                /*
                  when reparenting toplevel windows with toplevel-transient children,
                  we need to make sure that the window manager gets the updated
                  WM_TRANSIENT_FOR information... unfortunately, some window managers
                  don't handle changing WM_TRANSIENT_FOR before the toplevel window is
                  visible, so we unmap and remap all toplevel-transient children *after*
                  the toplevel parent has been mapped.  thankfully, this is easy in Qt :)
                */
                XUnmapWindow(X11->display, w->winId());
                XSetTransientForHint(X11->display, w->winId(), q->winId());
                QApplication::postEvent(w, new QEvent(QEvent::ShowWindowRequest));
            }
        }
    }
    qPRCreate(q, old_winid);
    updateSystemBackground();

    if (q->isTopLevel()) {
        uint save_state = q->data->widget_state & (Qt::WState_Maximized | Qt::WState_FullScreen);
        const QRect r = topData()->normalGeometry;
        q->setGeometry(0, 0, s.width(), s.height());
        q->data->widget_state |= save_state;
        topData()->normalGeometry = r;
    } else {
        q->setGeometry(0, 0, s.width(), s.height());
    }

    q->setEnabled(enable);
    q->setFocusPolicy(fp);
    if (!capt.isNull()) {
        extra->topextra->caption = QString::null;
        q->setWindowTitle(capt);
    }
    if (old_winid)
        qt_XDestroyWindow(q, X11->display, old_winid);
    if (setcurs)
        q->setCursor(oldcurs);

    // re-register dnd
    if (oldparent)
        oldparent->d->checkChildrenDnd();

    if (accept_drops)
        q->setAcceptDrops(true);
    else {
        checkChildrenDnd();
        topData()->dnd = 0;
        X11->dndEnable(q, (extra && extra->children_use_dnd));
    }
}


/*!
    Translates the widget coordinate \a pos to global screen
    coordinates. For example, \c{mapToGlobal(QPoint(0,0))} would give
    the global coordinates of the top-left pixel of the widget.

    \sa mapFromGlobal() mapTo() mapToParent()
*/

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    int           x, y;
    Window child;
    QPoint p = d->mapToWS(pos);
    XTranslateCoordinates(X11->display, winId(),
                          QApplication::desktop()->screen(d->xinfo.screen())->winId(),
                          p.x(), p.y(), &x, &y, &child);
    return QPoint(x, y);
}

/*!
    Translates the global screen coordinate \a pos to widget
    coordinates.

    \sa mapToGlobal() mapFrom() mapFromParent()
*/

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    int           x, y;
    Window child;
    XTranslateCoordinates(X11->display,
                          QApplication::desktop()->screen(d->xinfo.screen())->winId(),
                          winId(), pos.x(), pos.y(), &x, &y, &child);
    return d->mapFromWS(QPoint(x, y));
}

void QWidgetPrivate::setFont_sys(QFont *)
{
    // Nothing
}

void QWidgetPrivate::updateSystemBackground()
{
    QBrush brush = q->palette().brush(q->backgroundRole());
    if (brush.style() == Qt::NoBrush || q->testAttribute(Qt::WA_NoSystemBackground)) {
        XSetWindowBackgroundPixmap(X11->display, q->winId(), XNone);
    } else if (!brush.texture().isNull()) {
        QPixmap pix = brush.texture();
        if (data.wrect.isValid() && !isBackgroundInherited()) {
            int xoff = data.wrect.x() % pix.width();
            int yoff = data.wrect.y() % pix.height();
            if (xoff || yoff) {
                QPixmap newPix(pix.size(), pix.depth());
                qt_erase_background(&newPix, newPix.x11Info().screen(), 0,0,pix.width(), pix.height(),
                                    pix, xoff, yoff);
                pix = newPix;
            }
        }
        XSetWindowBackgroundPixmap(X11->display, q->winId(),
                                   isBackgroundInherited()
                                   ? ParentRelative
                                   : pix.handle());
    } else {
        XSetWindowBackground(X11->display, q->winId(),
                             QColormap::instance(xinfo.screen()).pixel(brush.color()));
    }
}

void QWidget::setCursor(const QCursor &cursor)
{
    if (cursor.shape() != Qt::ArrowCursor
        || (d->extra && d->extra->curs)) {
        d->createExtra();
        delete d->extra->curs;
        d->extra->curs = new QCursor(cursor);
    }
    setAttribute(Qt::WA_SetCursor);
    qt_x11_enforce_cursor(this);
    XFlush(X11->display);
}

void QWidget::unsetCursor()
{
    if (d->extra) {
        delete d->extra->curs;
        d->extra->curs = 0;
    }
    if (!isTopLevel())
        setAttribute(Qt::WA_SetCursor, false);
    qt_x11_enforce_cursor(this);
    XFlush(X11->display);
}

static XTextProperty*
qstring_to_xtp(const QString& s)
{
    static XTextProperty tp = { 0, 0, 0, 0 };
    static bool free_prop = true; // we can't free tp.value in case it references
    // the data of the static QCString below.
    if (tp.value) {
        if (free_prop)
            XFree(tp.value);
        tp.value = 0;
        free_prop = true;
    }

    static const QTextCodec* mapper = QTextCodec::codecForLocale();
    int errCode = 0;
    if (mapper) {
        QByteArray mapped = mapper->fromUnicode(s);
        char* tl[2];
        tl[0] = mapped.data();
        tl[1] = 0;
        errCode = XmbTextListToTextProperty(X11->display, tl, 1, XStdICCTextStyle, &tp);
#if defined(QT_DEBUG)
        if (errCode < 0)
            qDebug("qstring_to_xtp result code %d", errCode);
#endif
    }
    if (!mapper || errCode < 0) {
        static QByteArray qcs;
        qcs = s.toAscii();
        tp.value = (uchar*)qcs.data();
        tp.encoding = XA_STRING;
        tp.format = 8;
        tp.nitems = qcs.length();
        free_prop = false;
    }

    // ### If we knew WM could understand unicode, we could use
    // ### a much simpler, cheaper encoding...
    /*
        tp.value = (XChar2b*)s.unicode();
        tp.encoding = XA_UNICODE; // wish
        tp.format = 16;
        tp.nitems = s.length();
    */

    return &tp;
}

void QWidget::setWindowModified(bool mod)
{
    setAttribute(Qt::WA_WindowModified, mod);
    QEvent e(QEvent::ModifiedChange);
    QApplication::sendEvent(this, &e);
}

bool QWidget::isWindowModified() const
{
    return testAttribute(Qt::WA_WindowModified);
}

void QWidget::setWindowTitle(const QString &caption)
{
    if (QWidget::windowTitle() == caption)
        return;

    d->topData()->caption = caption;
    XSetWMName(X11->display, winId(), qstring_to_xtp(caption));

    QByteArray net_wm_name = caption.toUtf8();
    XChangeProperty(X11->display, winId(), ATOM(_NET_WM_NAME), ATOM(UTF8_STRING), 8,
                    PropModeReplace, (unsigned char *)net_wm_name.data(), net_wm_name.size());

    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::setWindowIcon_sys(const QPixmap &pixmap)
{
    if (d->extra && d->extra->topextra) {
        delete d->extra->topextra->icon;
        d->extra->topextra->icon = 0;
    }

    XWMHints *h = XGetWMHints(X11->display, q->winId());
    XWMHints wm_hints;
    if (!h) {
        h = &wm_hints;
        h->flags = 0;
    }

    if (!pixmap.isNull()) {
        QPixmap* pm = new QPixmap(pixmap);
        if (!pm->mask())
            pm->setMask(pm->createHeuristicMask()); // may do detach()

        d->createTLExtra();
        d->extra->topextra->icon = pm;

        h->icon_pixmap = pm->handle();
        h->flags |= IconPixmapHint;
        if (pm->mask()) {
            h->icon_mask = pm->mask()->handle();
            h->flags |= IconMaskHint;
        }
    } else {
        h->flags &= ~(IconPixmapHint | IconMaskHint);
    }

    XSetWMHints(X11->display, q->winId(), h);
    if (h != &wm_hints)
        XFree((char *)h);
}

void QWidget::setWindowIconText(const QString &iconText)
{
    d->createTLExtra();
    d->extra->topextra->iconText = iconText;

    XSetWMIconName(X11->display, winId(), qstring_to_xtp(iconText));

    QByteArray icon_name = iconText.toUtf8();
    XChangeProperty(X11->display, winId(), ATOM(_NET_WM_ICON_NAME), ATOM(UTF8_STRING), 8,
                    PropModeReplace, (unsigned char *) icon_name.data(), icon_name.size());

    QEvent e(QEvent::IconTextChange);
    QApplication::sendEvent(this, &e);
}


/*!
    Grabs the mouse input.

    This widget receives all mouse events until releaseMouse() is
    called; other widgets get no mouse events at all. Keyboard
    events are not affected. Use grabKeyboard() if you want to grab
    that.

    \warning Bugs in mouse-grabbing applications very often lock the
    terminal. Use this function with extreme caution, and consider
    using the \c -nograb command line option while debugging.

    It is almost never necessary to grab the mouse when using Qt, as
    Qt grabs and releases it sensibly. In particular, Qt grabs the
    mouse when a mouse button is pressed and keeps it until the last
    button is released.

    Note that only visible widgets can grab mouse input. If
    isVisible() returns false for a widget, that widget cannot call
    grabMouse().

    \sa releaseMouse() grabKeyboard() releaseKeyboard() grabKeyboard()
    focusWidget()
*/

void QWidget::grabMouse()
{
    if (isVisible() && !qt_nograb()) {
        if (mouseGrb)
            mouseGrb->releaseMouse();
#ifndef QT_NO_DEBUG
        int status =
#endif
            XGrabPointer(X11->display, winId(), False,
                          (uint)(ButtonPressMask | ButtonReleaseMask |
                                  PointerMotionMask | EnterWindowMask |
                                  LeaveWindowMask),
                          GrabModeAsync, GrabModeAsync,
                          XNone, XNone, X11->time);
#ifndef QT_NO_DEBUG
        if (status) {
            const char *s =
                status == GrabNotViewable ? "\"GrabNotViewable\"" :
                status == AlreadyGrabbed  ? "\"AlreadyGrabbed\"" :
                status == GrabFrozen      ? "\"GrabFrozen\"" :
                status == GrabInvalidTime ? "\"GrabInvalidTime\"" :
                "<?>";
            qWarning("Grabbing the mouse failed with %s", s);
        }
#endif
        mouseGrb = this;
    }
}

/*!
    \overload

    Grabs the mouse input and changes the cursor shape.

    The cursor will assume shape \a cursor (for as long as the mouse
    focus is grabbed) and this widget will be the only one to receive
    mouse events until releaseMouse() is called().

    \warning Grabbing the mouse might lock the terminal.

    \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/

void QWidget::grabMouse(const QCursor &cursor)
{
    if (!qt_nograb()) {
        if (mouseGrb)
            mouseGrb->releaseMouse();
#ifndef QT_NO_DEBUG
        int status =
#endif
        XGrabPointer(X11->display, winId(), False,
                      (uint)(ButtonPressMask | ButtonReleaseMask |
                             PointerMotionMask | EnterWindowMask | LeaveWindowMask),
                      GrabModeAsync, GrabModeAsync,
                      XNone, cursor.handle(), X11->time);
#ifndef QT_NO_DEBUG
        if (status) {
            const char *s =
                status == GrabNotViewable ? "\"GrabNotViewable\"" :
                status == AlreadyGrabbed  ? "\"AlreadyGrabbed\"" :
                status == GrabFrozen      ? "\"GrabFrozen\"" :
                status == GrabInvalidTime ? "\"GrabInvalidTime\"" :
                                            "<?>";
            qWarning("Grabbing the mouse failed with %s", s);
        }
#endif
        mouseGrb = this;
    }
}

/*!
    Releases the mouse grab.

    \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::releaseMouse()
{
    if (!qt_nograb() && mouseGrb == this) {
        XUngrabPointer(X11->display,  X11->time);
        XFlush(X11->display);
        mouseGrb = 0;
    }
}

/*!
    Grabs the keyboard input.

    This widget reveives all keyboard events until releaseKeyboard()
    is called; other widgets get no keyboard events at all. Mouse
    events are not affected. Use grabMouse() if you want to grab that.

    The focus widget is not affected, except that it doesn't receive
    any keyboard events. setFocus() moves the focus as usual, but the
    new focus widget receives keyboard events only after
    releaseKeyboard() is called.

    If a different widget is currently grabbing keyboard input, that
    widget's grab is released first.

    \sa releaseKeyboard() grabMouse() releaseMouse() focusWidget()
*/

void QWidget::grabKeyboard()
{
    if (!qt_nograb()) {
        if (keyboardGrb)
            keyboardGrb->releaseKeyboard();
        XGrabKeyboard(X11->display, data->winid, False, GrabModeAsync, GrabModeAsync,
                       X11->time);
        keyboardGrb = this;
    }
}

/*!
    Releases the keyboard grab.

    \sa grabKeyboard(), grabMouse(), releaseMouse()
*/

void QWidget::releaseKeyboard()
{
    if (!qt_nograb() && keyboardGrb == this) {
        XUngrabKeyboard(X11->display, X11->time);
        keyboardGrb = 0;
    }
}


/*!
    Returns the widget that is currently grabbing the mouse input.

    If no widget in this application is currently grabbing the mouse,
    0 is returned.

    \sa grabMouse(), keyboardGrabber()
*/

QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

/*!
    Returns the widget that is currently grabbing the keyboard input.

    If no widget in this application is currently grabbing the
    keyboard, 0 is returned.

    \sa grabMouse(), mouseGrabber()
*/

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}

/*!
    Sets the top-level widget containing this widget to be the active
    window.

    An active window is a visible top-level window that has the
    keyboard input focus.

    This function performs the same operation as clicking the mouse on
    the title bar of a top-level window. On X11, the result depends on
    the Window Manager. If you want to ensure that the window is
    stacked on top as well you should also call raise(). Note that the
    window must be visible, otherwise activateWindow() has no effect.

    On Windows, if you are calling this when the application is not
    currently the active one then it will not make it the active
    window.  It will flash the task bar entry blue to indicate that
    the window has done something. This is because Microsoft do not
    allow an application to interrupt what the user is currently doing
    in another application.

    \sa isActiveWindow(), topLevelWidget(), show()
*/

void QWidget::activateWindow()
{
    QWidget *tlw = topLevelWidget();
    if (tlw->isVisible() && !tlw->d->topData()->embedded && !X11->deferred_map.contains(tlw)) {
        XSetInputFocus(X11->display, tlw->winId(), XRevertToParent, X11->time);
 	d->focusInputContext();
    }
}


void QWidget::update()
{
    if ((data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) == Qt::WState_Visible) {
//         d->removePendingPaintEvents(); // ### this is far too slow to go in
        d->invalidated_region = d->clipRect();
        QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void QWidget::update(const QRegion &rgn)
{
    if ((data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) == Qt::WState_Visible) {
        d->invalidated_region |= (rgn & d->clipRect());
        QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void QWidget::update(const QRect &r)
{
    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
    if (w && h && (data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) == Qt::WState_Visible) {
        if (w < 0)
            w = data->crect.width()  - x;
        if (h < 0)
            h = data->crect.height() - y;
        if (w != 0 && h != 0) {
            d->invalidated_region |= (d->clipRect() & QRect(x, y, w, h));
            QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
        }
    }
}

struct QX11DoubleBuffer
{
    enum {
        MaxWidth  = SHRT_MAX,
        MaxHeight = SHRT_MAX
    };

    Qt::HANDLE hd, xft_hd;
    int screen, depth;
    int width, height;
};

static QX11DoubleBuffer *qt_x11_global_double_buffer = 0;
static bool qt_x11_global_double_buffer_active = false;

static void qt_discard_double_buffer(QX11DoubleBuffer **db)
{
    if (!*db) return;

    XFreePixmap(X11->display, (*db)->hd);
#ifndef QT_NO_XFT
    if (X11->has_xft)
        XftDrawDestroy((XftDraw *) (*db)->xft_hd);
#endif
    delete *db;
    *db = 0;
}

void qt_discard_double_buffer()
{
    qt_discard_double_buffer(&qt_x11_global_double_buffer);
}

static void qt_x11_release_double_buffer(QX11DoubleBuffer **db)
{
    if (*db != qt_x11_global_double_buffer)
        qt_discard_double_buffer(db);
    else
	qt_x11_global_double_buffer_active = false;
}

static QX11DoubleBuffer *qt_x11_create_double_buffer(Qt::HANDLE hd, int screen, int depth, int width, int height)
{
    QX11DoubleBuffer *db = new QX11DoubleBuffer;
    db->hd = XCreatePixmap(X11->display, hd, width, height, depth);
    db->xft_hd = 0;
#ifndef QT_NO_XFT
    if (X11->has_xft) {
        db->xft_hd =
            (Qt::HANDLE) XftDrawCreate(X11->display,
                                       db->hd,
                                       (Visual *) QX11Info::appVisual(),
                                       QX11Info::appColormap());
    }
#endif
    db->screen = screen;
    db->depth = depth;
    db->width = width;
    db->height = height;
    return db;
}

static
void qt_x11_get_double_buffer(QX11DoubleBuffer **db, Qt::HANDLE hd, int screen, int depth, int width, int height)
{
    if (!qt_has_accelerated_xrender)
	qt_x11_global_double_buffer_active = true;

    if (qt_x11_global_double_buffer_active) {
        *db = qt_x11_create_double_buffer(hd, screen, depth, width, height);
	return;
    }

    if (qt_has_accelerated_xrender)
	qt_x11_global_double_buffer_active = true;

    // the db should consist of 128x128 chunks
    width  = qMin(((width / 128) + 1) * 128, (int)QX11DoubleBuffer::MaxWidth);
    height = qMin(((height / 128) + 1) * 128, (int)QX11DoubleBuffer::MaxHeight);

    if (qt_x11_global_double_buffer) {
        if (qt_x11_global_double_buffer->screen == screen
            && qt_x11_global_double_buffer->depth == depth
            && qt_x11_global_double_buffer->width >= width
            && qt_x11_global_double_buffer->height >= height) {
            *db = qt_x11_global_double_buffer;
            return;
        }

 	width  = qMax(qt_x11_global_double_buffer->width,  width);
 	height = qMax(qt_x11_global_double_buffer->height, height);

        qt_discard_double_buffer();
    }

    qt_x11_global_double_buffer = *db = qt_x11_create_double_buffer(hd, screen, depth, width, height);
}

void QWidget::repaint(const QRegion& rgn)
{
    if (testWState(Qt::WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");

    if ((data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) != Qt::WState_Visible
         || !testAttribute(Qt::WA_Mapped))
        return;

    if (rgn.isEmpty())
        return;

    if (!d->invalidated_region.isEmpty())
        d->invalidated_region -= rgn;

    setWState(Qt::WState_InPaintEvent);

    QRect br = rgn.boundingRect();
    QRect brWS = d->mapToWS(br);
    bool do_clipping = (br != QRect(0,0,data->crect.width(),data->crect.height()));

    bool double_buffer = (!testAttribute(Qt::WA_PaintOnScreen)
                          && !testAttribute(Qt::WA_NoSystemBackground)
                          && br.width()  <= QX11DoubleBuffer::MaxWidth
                          && br.height() <= QX11DoubleBuffer::MaxHeight
                          && !QPainter::redirected(this));

    Qt::HANDLE old_hd = d->hd;
    Qt::HANDLE old_xft_hd = d->xft_hd;

    QPoint redirectionOffset;
    QX11DoubleBuffer *qDoubleBuffer = 0;
    if (double_buffer) {
        qt_x11_get_double_buffer(&qDoubleBuffer, d->hd, d->xinfo.screen(), d->xinfo.depth(),
                                 br.width(), br.height());

	d->hd = qDoubleBuffer->hd;
	d->xft_hd = qDoubleBuffer->xft_hd;
        redirectionOffset = br.topLeft();
    } else {
        redirectionOffset = data->wrect.topLeft();
    }

    if (!redirectionOffset.isNull())
        QPainter::setRedirected(this, this, redirectionOffset);

    if (do_clipping) {
        if (redirectionOffset.isNull()) {
            qt_set_paintevent_clipping(this, rgn);
        } else {
            QRegion redirectedRegion(rgn);
            redirectedRegion.translate(-redirectionOffset);
            qt_set_paintevent_clipping(this, redirectedRegion);
        }
    }

    if (!testAttribute(Qt::WA_NoBackground) && !testAttribute(Qt::WA_NoSystemBackground) ) {
        QPoint offset;
        QStack<QWidget*> parents;
        QWidget *w = q;
        while (w->d->isBackgroundInherited()) {
            offset += w->pos();
            w = w->parentWidget();
            parents += w;
        }

        if (double_buffer) {
            qt_erase_background(q, q->d->xinfo.screen(),
                                br.x() - redirectionOffset.x(), br.y() - redirectionOffset.y(),
                                br.width(), br.height(), data->pal.brush(w->d->bg_role),
                                br.x() + offset.x(), br.y() + offset.y());
        } else {
            QVector<QRect> rects = rgn.rects();
            for (int i = 0; i < rects.size(); ++i) {
                QRect rr = d->mapToWS(rects[i]);
		if (data->pal.brush(w->d->bg_role).style() == Qt::LinearGradientPattern) {
		    QPainter p(q);
		    p.fillRect(rr, data->pal.brush(w->d->bg_role));
		} else {
		    XClearArea(X11->display, q->winId(), rr.x(), rr.y(), rr.width(), rr.height(), False);
		}
            }
        }

        if (parents.size()) {
            w = parents.pop();
            for (;;) {
                if (w->testAttribute(Qt::WA_ContentsPropagated)) {
                    QPainter::setRedirected(w, q, offset + redirectionOffset);
                    QRect rr = d->clipRect();
                    rr.translate(offset);
                    QPaintEvent e(rr);
                    bool was_in_paint_event = w->testWState(Qt::WState_InPaintEvent);
                    w->setWState(Qt::WState_InPaintEvent);
                    QApplication::sendEvent(w, &e);
                    if(!was_in_paint_event) {
                        w->clearWState(Qt::WState_InPaintEvent);
                        if(!w->testAttribute(Qt::WA_PaintOutsidePaintEvent) && w->paintingActive())
                            qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");
                    }
                    QPainter::restoreRedirected(w);
                }
                if (parents.size() == 0)
                    break;
                w = parents.pop();
                offset -= w->pos();
            }
        }
    }

    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent(this, &e);

    if (do_clipping)
        qt_clear_paintevent_clipping();

    if (!redirectionOffset.isNull())
        QPainter::restoreRedirected(this);

    if (double_buffer) {
        GC gc = qt_xget_temp_gc(d->xinfo.screen(), false);
        QVector<QRect> rects = rgn.rects();
	if (testAttribute(Qt::WA_PaintUnclipped))
	    XSetSubwindowMode(X11->display, gc, IncludeInferiors);
        for (int i = 0; i < rects.size(); ++i) {
            QRect rr = d->mapToWS(rects[i]);
            XCopyArea(X11->display, d->hd, winId(), gc,
                      rr.x() - brWS.x(), rr.y() - brWS.y(),
                      rr.width(), rr.height(),
                      rr.x(), rr.y());
        }

        d->hd = old_hd;
        d->xft_hd = old_xft_hd;

	qt_x11_release_double_buffer(&qDoubleBuffer);

        if (!QApplicationPrivate::active_window) {
            extern int qt_double_buffer_timer;
            if (qt_double_buffer_timer)
                qApp->killTimer(qt_double_buffer_timer);
            qt_double_buffer_timer = qApp->startTimer(500);
        }
    }

    clearWState(Qt::WState_InPaintEvent);
    if(!testAttribute(Qt::WA_PaintOutsidePaintEvent) && paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

    if (testAttribute(Qt::WA_ContentsPropagated))
        d->updatePropagatedBackground(&rgn);
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    bool needShow = false;
    Qt::WindowStates oldstate = windowState();
    if (isTopLevel()) {
        // Ensure the initial size is valid, since we store it as normalGeometry below.
        if (!testAttribute(Qt::WA_Resized) && !isVisible())
            adjustSize();

        QTLWExtra *top = d->topData();

        if ((oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized)) {
            if (qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_HORZ))
                && qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_VERT))) {
                qt_change_net_wm_state(this, (newstate & Qt::WindowMaximized),
                                       ATOM(_NET_WM_STATE_MAXIMIZED_HORZ),
                                       ATOM(_NET_WM_STATE_MAXIMIZED_VERT));
            } else if (! (newstate & Qt::WindowFullScreen)) {
                if (newstate & Qt::WindowMaximized) {
                    // save original geometry
                    const QRect normalGeometry = geometry();

                    if (isVisible()) {
                        d->updateFrameStrut();
                        const QRect maxRect = QApplication::desktop()->availableGeometry(this);
                        const QRect r = top->normalGeometry;
                        setGeometry(maxRect.x() + top->fleft,
                                    maxRect.y() + top->ftop,
                                    maxRect.width() - top->fleft - top->fright,
                                    maxRect.height() - top->ftop - top->fbottom);
                        top->normalGeometry = r;
                    }

                    if (top->normalGeometry.width() < 0)
                        top->normalGeometry = normalGeometry;
                } else {
                    // restore original geometry
                    setGeometry(top->normalGeometry);
                }
            }
        }

        if ((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            if (qt_net_supports(ATOM(_NET_WM_STATE_FULLSCREEN))) {
                qt_change_net_wm_state(this, (newstate & Qt::WindowFullScreen),
                                       ATOM(_NET_WM_STATE_FULLSCREEN));
            } else {
                needShow = isVisible();

                if (newstate & Qt::WindowFullScreen) {
                    const QRect normalGeometry = QRect(pos(), size());

                    top->savedFlags = getWFlags();
                    setParent(0, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_NoBorder |
                              // preserve some widget flags
                              (getWFlags() & 0xffff0000));

                    const QRect r = top->normalGeometry;
                    setGeometry(qApp->desktop()->screenGeometry(this));
                    top->normalGeometry = r;

                    if (top->normalGeometry.width() < 0)
                        top->normalGeometry = normalGeometry;
                } else {
                    setParent(0, top->savedFlags);

                    if (newstate & Qt::WindowMaximized) {
                        // from fullscreen to maximized
                        d->updateFrameStrut();
                        const QRect maxRect = QApplication::desktop()->availableGeometry(this);
                        const QRect r = top->normalGeometry;
                        setGeometry(maxRect.x() + top->fleft,
                                    maxRect.y() + top->ftop,
                                    maxRect.width() - top->fleft - top->fright,
                                    maxRect.height() - top->ftop - top->fbottom);
                        top->normalGeometry = r;
                    } else {
                        // restore original geometry
                        setGeometry(top->normalGeometry);
                    }
                }
            }
        }

        if ((oldstate & Qt::WindowMinimized) != (newstate & Qt::WindowMinimized)) {
            if (isVisible()) {
                if (newstate & Qt::WindowMinimized) {
                    XEvent e;
                    e.xclient.type = ClientMessage;
                    e.xclient.message_type = ATOM(WM_CHANGE_STATE);
                    e.xclient.display = X11->display;
                    e.xclient.window = data->winid;
                    e.xclient.format = 32;
                    e.xclient.data.l[0] = IconicState;
                    e.xclient.data.l[1] = 0;
                    e.xclient.data.l[2] = 0;
                    e.xclient.data.l[3] = 0;
                    e.xclient.data.l[4] = 0;
                    XSendEvent(X11->display,
                               RootWindow(X11->display,d->xinfo.screen()),
                               False, (SubstructureNotifyMask|SubstructureRedirectMask), &e);
                } else {
                    setAttribute(Qt::WA_Mapped);
                    XMapWindow(X11->display, winId());
                }
            }

            needShow = false;
        }
    }

    data->widget_state &= ~(Qt::WState_Minimized | Qt::WState_Maximized | Qt::WState_FullScreen);
    if (newstate & Qt::WindowMinimized)
        data->widget_state |= Qt::WState_Minimized;
    if (newstate & Qt::WindowMaximized)
        data->widget_state |= Qt::WState_Maximized;
    if (newstate & Qt::WindowFullScreen)
        data->widget_state |= Qt::WState_FullScreen;

    if (needShow)
        show();

    if (newstate & Qt::WindowActive)
        activateWindow();

    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}

/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidgetPrivate::show_sys()
{
    if (q->isTopLevel()) {
        XWMHints *h = XGetWMHints(X11->display, q->winId());
        XWMHints  wm_hints;
        bool got_hints = h != 0;
        if (!got_hints) {
            h = &wm_hints;
            h->flags = 0;
        }
        h->initial_state = q->testWState(Qt::WState_Minimized) ? IconicState : NormalState;
        h->flags |= StateHint;
        XSetWMHints(X11->display, q->winId(), h);
        if (got_hints)
            XFree((char *)h);

        // update _MOTIF_WM_HINTS
        QtMWMHints mwmhints = GetMWMHints(X11->display, q->winId());

        if (q->testWFlags(Qt::WShowModal)) {
            mwmhints.input_mode = MWM_INPUT_FULL_APPLICATION_MODAL;
            mwmhints.flags |= MWM_HINTS_INPUT_MODE;
        }

        if (q->minimumSize() == q->maximumSize()) {
            // fixed size, remove the resize handle (since mwm/dtwm
            // isn't smart enough to do it itself)
            mwmhints.flags |= MWM_HINTS_FUNCTIONS;
            if (mwmhints.functions == MWM_FUNC_ALL) {
                mwmhints.functions = (MWM_FUNC_MOVE
                                      | MWM_FUNC_MINIMIZE
                                      | MWM_FUNC_MAXIMIZE
                                      | MWM_FUNC_CLOSE);
            } else {
                mwmhints.functions &= ~MWM_FUNC_RESIZE;
            }

            mwmhints.flags |= MWM_HINTS_DECORATIONS;
            if (mwmhints.decorations == MWM_DECOR_ALL) {
                mwmhints.decorations = (MWM_DECOR_BORDER
                                        | MWM_DECOR_RESIZEH
                                        | MWM_DECOR_TITLE
                                        | MWM_DECOR_MENU
                                        | MWM_DECOR_MINIMIZE
                                        | MWM_DECOR_MAXIMIZE);
            } else {
                mwmhints.decorations &= ~MWM_DECOR_RESIZEH;
            }
        }

        if (mwmhints.flags != 0l) {
            XChangeProperty(X11->display, q->winId(), ATOM(_MOTIF_WM_HINTS),
                            ATOM(_MOTIF_WM_HINTS), 32, PropModeReplace,
                            (unsigned char *) &mwmhints, 5);
        } else {
            XDeleteProperty(X11->display, q->winId(), ATOM(_MOTIF_WM_HINTS));
        }

        // set _NET_WM_STATE
        Atom net_winstates[6] = { 0, 0, 0, 0, 0, 0 };
        int curr_winstate = 0;

        if (q->testWFlags(Qt::WStyle_StaysOnTop)) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_ABOVE);
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_STAYS_ON_TOP);
        }
        if (q->testWState(Qt::WState_FullScreen)) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_FULLSCREEN);
        }
        if (q->testWState(Qt::WState_Maximized)) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
        }
        if (q->testWFlags(Qt::WShowModal)) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MODAL);
        }

        if (curr_winstate > 0) {
            XChangeProperty(X11->display, q->winId(), ATOM(_NET_WM_STATE), XA_ATOM,
                            32, PropModeReplace, (unsigned char *) net_winstates, curr_winstate);
        } else {
            XDeleteProperty(X11->display, q->winId(), ATOM(_NET_WM_STATE));
        }

        // set _NET_WM_USER_TIME
        if (X11->userTime != CurrentTime) {
            XChangeProperty(X11->display, q->winId(), ATOM(_NET_WM_USER_TIME), XA_CARDINAL,
                            32, PropModeReplace, (unsigned char *) &X11->userTime, 1);
        }

        if (!topData()->embedded
            && topData()->parentWinId
            && topData()->parentWinId != QX11Info::appRootWindow(xinfo.screen())
            && !q->isMinimized()) {
            X11->deferred_map.append(q);
            return;
        }

        if (q->isMaximized() && !q->isFullScreen()
            && !(qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_HORZ))
                 && qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_VERT)))) {
	    XMapWindow(X11->display, q->winId());
 	    qt_wait_for_window_manager(q);

 	    // if the wm was not smart enough to adjust our size, do that manually
 	    updateFrameStrut();
	    QRect maxRect = QApplication::desktop()->availableGeometry(q);

 	    QTLWExtra *top = topData();
 	    QRect normalRect = top->normalGeometry;

 	    q->setGeometry(maxRect.x() + top->fleft,
			maxRect.y() + top->ftop,
 			maxRect.width() - top->fleft - top->fright,
 			maxRect.height() - top->ftop - top->fbottom);

	    // restore the original normalGeometry
 	    top->normalGeometry = normalRect;
 	    // internalSetGeometry() clears the maximized flag... make sure we set it back
 	    q->setWState(Qt::WState_Maximized);

 	    return;
        }

	if (q->isFullScreen() && !qt_net_supports(ATOM(_NET_WM_STATE_FULLSCREEN))) {
 	    XMapWindow(X11->display, q->winId());
 	    qt_wait_for_window_manager(q);
	    return;
	}
    }

    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;
    q->setAttribute(Qt::WA_Mapped);

    if (!q->isTopLevel()
        && (q->testAttribute(Qt::WA_NoBackground)
            || q->palette().brush(q->backgroundRole()).style() == Qt::LinearGradientPattern)) {
        XSetWindowBackgroundPixmap(X11->display, q->winId(), XNone);
        XMapWindow(X11->display, q->winId());
        updateSystemBackground();
        return;
    }
    XMapWindow(X11->display, q->winId());
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidgetPrivate::hide_sys()
{
    q->clearWState(Qt::WState_Exposed);
    deactivateWidgetCleanup();
    if (q->isTopLevel()) {
        X11->deferred_map.removeAll(q);
        if (q->winId()) // in nsplugin, may be 0
            XWithdrawWindow(X11->display, q->winId(), xinfo.screen());

        QTLWExtra *top = topData();
        q->data->crect.moveTopLeft(QPoint(q->data->crect.x() - top->fleft,
                                          q->data->crect.y() - top->ftop));

        // zero the frame strut and mark it dirty
        top->fleft = top->fright = top->ftop = top->fbottom = 0;
        q->data->fstrut_dirty = true;

        XFlush(X11->display);
    } else {
        q->setAttribute(Qt::WA_Mapped, false);
        if (q->winId()) // in nsplugin, may be 0
            XUnmapWindow(X11->display, q->winId());
    }
}

/*!
    Raises this widget to the top of the parent widget's stack.

    After this call the widget will be visually in front of any
    overlapping sibling widgets.

    \sa lower(), stackUnder()
*/

void QWidget::raise()
{
    QWidget *p = parentWidget();
    int from;
    if (p && (from = p->d->children.indexOf(this)) >= 0)
        p->d->children.move(from, p->d->children.size() - 1);
    XRaiseWindow(X11->display, winId());
}

/*!
    Lowers the widget to the bottom of the parent widget's stack.

    After this call the widget will be visually behind (and therefore
    obscured by) any overlapping sibling widgets.

    \sa raise(), stackUnder()
*/

void QWidget::lower()
{
    QWidget *p = parentWidget();
    int from;
    if (p && (from = p->d->children.indexOf(this)) >= 0)
        p->d->children.move(from, 0);
    XLowerWindow(X11->display, winId());
}


/*!
    Places the widget under \a w in the parent widget's stack.

    To make this work, the widget itself and \a w must be siblings.

    \sa raise(), lower()
*/
void QWidget::stackUnder(QWidget* w)
{
    QWidget *p = parentWidget();
    int from;
    int to;
    if (!w || isTopLevel() || p != w->parentWidget() || this == w)
        return;
    if (p && (to = p->d->children.indexOf(w)) >= 0 && (from = p->d->children.indexOf(this)) >= 0) {
        if (from < to)
            --to;
        p->d->children.move(from, to);
    }
    Window stack[2];
    stack[0] = w->winId();;
    stack[1] = winId();
    XRestackWindows(X11->display, stack, 2);
}


static void do_size_hints(QWidget* widget, QWExtra *x)
{
    XSizeHints s;
    s.flags = 0;
    if (x) {
        s.x = widget->x();
        s.y = widget->y();
        s.width = widget->width();
        s.height = widget->height();
        if (x->minw > 0 || x->minh > 0) {
            // add minimum size hints
            s.flags |= PMinSize;
            s.min_width  = qMin(XCOORD_MAX, x->minw);
            s.min_height = qMin(XCOORD_MAX, x->minh);
        }
        if (x->maxw < QWIDGETSIZE_MAX || x->maxh < QWIDGETSIZE_MAX) {
            // add maximum size hints
            s.flags |= PMaxSize;
            s.max_width  = qMin(XCOORD_MAX, x->maxw);
            s.max_height = qMin(XCOORD_MAX, x->maxh);
        }
        if (x->topextra &&
            (x->topextra->incw > 0 || x->topextra->inch > 0)) {
            // add resize increment hints
            s.flags |= PResizeInc | PBaseSize;
            s.width_inc = x->topextra->incw;
            s.height_inc = x->topextra->inch;
            s.base_width = x->topextra->basew;
            s.base_height = x->topextra->baseh;
        }
        if (x->topextra && x->topextra->uspos) {
            // user (i.e. command-line) specified position
            s.flags |= USPosition;
            s.flags |= PPosition;
        }
        if (x->topextra && x->topextra->ussize) {
            // user (i.e. command-line) specified size
            s.flags |= USSize;
            s.flags |= PSize;
        }
    }
    s.flags |= PWinGravity;
    s.win_gravity = NorthWestGravity;
    XSetWMNormalHints(X11->display, widget->winId(), &s);
}


/*
  Helper function for non-toplevel widgets. Helps to map Qt's 32bit
  coordinate system to X11's 16bit coordinate system.

  Sets the geometry of the widget to data.crect, but clipped to sizes
  that X can handle. Unmaps widgets that are completely outside the
  valid range.

  Maintains data.wrect, which is the geometry of the X widget,
  measured in this widget's coordinate system.

  if the parent is not clipped, parentWRect is empty, otherwise
  parentWRect is the geometry of the parent's X rect, measured in
  parent's coord sys
 */
void QWidgetPrivate::setWSGeometry()
{

    /*
      There are up to four different coordinate systems here:
      Qt coordinate system for this widget.
      X coordinate system for this widget (relative to wrect).
      Qt coordinate system for parent
      X coordinate system for parent (relative to parent's wrect).
     */
    Display *dpy = xinfo.display();
    QRect validRange(-XCOORD_MAX,-XCOORD_MAX, 2*XCOORD_MAX, 2*XCOORD_MAX);
    QRect wrectRange(-WRECT_MAX,-WRECT_MAX, 2*WRECT_MAX, 2*WRECT_MAX);
    QRect wrect;
    //xrect is the X geometry of my X widget. (starts out in  parent's Qt coord sys, and ends up in parent's X coord sys)
    QRect xrect = data.crect;

    QRect parentWRect = q->parentWidget()->data->wrect;

    if (parentWRect.isValid()) {
        // parent is clipped, and we have to clip to the same limit as parent
        if (!parentWRect.contains(xrect)) {
            xrect &= parentWRect;
            wrect = xrect;
            //translate from parent's to my Qt coord sys
            wrect.translate(-data.crect.topLeft());
        }
        //translate from parent's Qt coords to parent's X coords
        xrect.translate(-parentWRect.topLeft());

    } else {
        // parent is not clipped, we may or may not have to clip

        if (data.wrect.isValid()) {
            // This is where the main optimization is: we are already
            // clipped, and if our clip is still valid, we can just
            // move our window, and do not need to move or clip
            // children

            QRect vrect = xrect & q->parentWidget()->rect();
            vrect.translate(-data.crect.topLeft()); //the part of me that's visible through parent, in my Qt coords
            if (data.wrect.contains(vrect)) {
                xrect = data.wrect;
                xrect.translate(data.crect.topLeft());
                XMoveResizeWindow(dpy, data.winid, xrect.x(), xrect.y(), xrect.width(), xrect.height());
                return;
            }
        }

        if (!validRange.contains(xrect)) {
            // we are too big, and must clip
            xrect &=wrectRange;
            wrect = xrect;
            wrect.translate(-data.crect.topLeft());
            //parent's X coord system is equal to parent's Qt coord
            //sys, so we don't need to map xrect.
        }

    }

    // unmap if we are outside the valid window system coord system
    bool outsideRange = !xrect.isValid();
    bool mapWindow = false;
    if (q->testAttribute(Qt::WA_OutsideWSRange) != outsideRange) {
        q->setAttribute(Qt::WA_OutsideWSRange, outsideRange);
        if (outsideRange) {
            XUnmapWindow(dpy, data.winid);
            q->setAttribute(Qt::WA_Mapped, false);
        } else if (q->isShown()) {
            mapWindow = true;
        }
    }

    if (outsideRange)
        return;

    bool jump = (data.wrect != wrect);
    data.wrect = wrect;


    // and now recursively for all children...
    // ### can be optimized
    for (int i = 0; i < children.size(); ++i) {
        QObject *object = children.at(i);
        if (object->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(object);
            if (!w->isTopLevel())
                w->d->setWSGeometry();
        }
    }

    // move ourselves to the new position and map (if necessary) after
    // the movement. Rationale: moving unmapped windows is much faster
    // than moving mapped windows
    if (jump) //avoid flicker when jumping
        XSetWindowBackgroundPixmap(dpy, data.winid, XNone);
    XMoveResizeWindow(dpy, data.winid, xrect.x(), xrect.y(), xrect.width(), xrect.height());
    if  (jump) {
        updateSystemBackground();
        XClearArea(dpy, data.winid, 0, 0, wrect.width(), wrect.height(), True);
    }
    if (mapWindow) {
            q->setAttribute(Qt::WA_Mapped);
            XMapWindow(dpy, data.winid);
    }
}

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Display *dpy = X11->display;

    if (q->testWFlags(Qt::WType_Desktop))
        return;
    if (q->isTopLevel()) {
        if (!qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_VERT))
            && !qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_HORZ)))
            q->clearWState(Qt::WState_Maximized);
        if (!qt_net_supports(ATOM(_NET_WM_STATE_FULLSCREEN)))
            q->clearWState(Qt::WState_FullScreen);
        topData()->normalGeometry = QRect(0,0,-1,-1);
        w = qMax(1, w);
        h = qMax(1, h);
    } else {
        q->clearWState(Qt::WState_Maximized);
        q->clearWState(Qt::WState_FullScreen);
    }
    if (extra) {                                // any size restrictions?
        w = qMin(w,extra->maxw);
        h = qMin(h,extra->maxh);
        w = qMax(w,extra->minw);
        h = qMax(h,extra->minh);
    }
    QPoint oldPos(q->pos());
    QSize oldSize(q->size());
    QRect oldGeom(q->data->crect);
    QRect  r(x, y, w, h);

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if (!q->isTopLevel() && oldGeom == r)
        return;

    q->data->crect = r;
    bool isResize = q->size() != oldSize;

    if (q->isTopLevel()) {
        if (isMove)
            topData()->uspos = 1;
        if (isResize)
            topData()->ussize = 1;
        do_size_hints(q, extra);
        if (isMove) {
            if (! qt_broken_wm)
                // pos() is right according to ICCCM 4.1.5
                XMoveResizeWindow(dpy, q->data->winid, q->pos().x(), q->pos().y(), w, h);
            else
                // work around 4Dwm's incompliance with ICCCM 4.1.5
                XMoveResizeWindow(dpy, q->data->winid, x, y, w, h);
        } else if (isResize)
            XResizeWindow(dpy, q->data->winid, w, h);
    } else {
        setWSGeometry();
    }

    if (q->isVisible()) {
        if (isMove && q->pos() != oldPos) {
            if (! qt_broken_wm) {
                // pos() is right according to ICCCM 4.1.5
                QMoveEvent e(q->pos(), oldPos);
                QApplication::sendEvent(q, &e);
            } else {
                // work around 4Dwm's incompliance with ICCCM 4.1.5
                QMoveEvent e(q->data->crect.topLeft(), oldGeom.topLeft());
                QApplication::sendEvent(q, &e);
            }
        }
        if (isResize) {
            // set config pending only on resize, see qapplication_x11.cpp, translateConfigEvent()
            q->setWState(Qt::WState_ConfigPending);

            QResizeEvent e(q->size(), oldSize);
            QApplication::sendEvent(q, &e);

            // Process events immediately rather than in
            // translateConfigEvent to avoid message process delay.
            if (!q->testAttribute(Qt::WA_StaticContents))
                q->testWState(Qt::WState_InPaintEvent)?q->update():q->repaint();
        }
    } else {
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}


/*!
    \overload

    This function corresponds to setMinimumSize(QSize(minw, minh)).
    Sets the minimum width to \a minw and the minimum height to \a
    minh.
*/

void QWidget::setMinimumSize(int minw, int minh)
{
    if (minw < 0 || minh < 0)
        qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
    d->createExtra();
    if (d->extra->minw == minw && d->extra->minh == minh)
        return;
    d->extra->minw = minw;
    d->extra->minh = minh;
    if (minw > width() || minh > height()) {
        bool resized = testAttribute(Qt::WA_Resized);
        int maximized = testWState(Qt::WState_Maximized);
        resize(qMax(minw,width()), qMax(minh,height()));
        setAttribute(Qt::WA_Resized, resized); //not a user resize
        setWState(QFlag(maximized));
    }
    if (testWFlags(Qt::WType_TopLevel))
        do_size_hints(this, d->extra);
    updateGeometry();
}

/*!
    \overload

    This function corresponds to setMaximumSize(QSize(\a maxw, \a
    maxh)). Sets the maximum width to \a maxw and the maximum height
    to \a maxh.
*/
void QWidget::setMaximumSize(int maxw, int maxh)
{
    if (maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
        qWarning("QWidget::setMaximumSize: (%s/%s) "
                "The largest allowed size is (%d,%d)",
                 objectName().toLocal8Bit().data(), metaObject()->className(), QWIDGETSIZE_MAX,
                QWIDGETSIZE_MAX);
        maxw = qMin(maxw, QWIDGETSIZE_MAX);
        maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if (maxw < 0 || maxh < 0) {
        qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                objectName().toLocal8Bit().data(), metaObject()->className(), maxw, maxh);
        maxw = qMax(maxw, 0);
        maxh = qMax(maxh, 0);
    }
    d->createExtra();
    if (d->extra->maxw == maxw && d->extra->maxh == maxh)
        return;
    d->extra->maxw = maxw;
    d->extra->maxh = maxh;
    if (maxw < width() || maxh < height()) {
        bool resized = testAttribute(Qt::WA_Resized);
        resize(qMin(maxw,width()), qMin(maxh,height()));
        setAttribute(Qt::WA_Resized, resized); //not a user resize
    }
    if (testWFlags(Qt::WType_TopLevel))
        do_size_hints(this, d->extra);
    updateGeometry();
}

/*!
    \overload

    Sets the x (width) size increment to \a w and the y (height) size
    increment to \a h.
*/
void QWidget::setSizeIncrement(int w, int h)
{
    QTLWExtra* x = d->topData();
    if (x->incw == w && x->inch == h)
        return;
    x->incw = w;
    x->inch = h;
    if (testWFlags(Qt::WType_TopLevel))
        do_size_hints(this, d->extra);
}

/*!
    \overload

    This corresponds to setBaseSize(QSize(\a basew, \a baseh)). Sets
    the widgets base size to width \a basew and height \a baseh.
*/
void QWidget::setBaseSize(int basew, int baseh)
{
    d->createTLExtra();
    QTLWExtra* x = d->topData();
    if (x->basew == basew && x->baseh == baseh)
        return;
    x->basew = basew;
    x->baseh = baseh;
    if (testWFlags(Qt::WType_TopLevel))
        do_size_hints(this, d->extra);
}
/*!
    Scrolls the widget including its children \a dx pixels to the
    right and \a dy downwards. Both \a dx and \a dy may be negative.

    After scrolling, scroll() sends a paint event for the the part
    that is read but not written. For example, when scrolling 10
    pixels rightwards, the leftmost ten pixels of the widget need
    repainting. The paint event may be delivered immediately or later,
    depending on some heuristics (note that you might have to force
    processing of paint events using QApplication::sendPostedEvents()
    when using scroll() and move() in combination).

    \sa QScrollView bitBlt()
*/

void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

/*!
    \overload

    This version only scrolls \a r and does not move the children of
    the widget.

    If \a r is empty or invalid, the result is undefined.

    \sa QScrollView bitBlt()
*/
void QWidget::scroll(int dx, int dy, const QRect& r)
{
    if (testWState(Qt::WState_BlockUpdates) && d->children.isEmpty())
        return;
    bool valid_rect = r.isValid();
    bool just_update = qAbs(dx) > width() || qAbs(dy) > height();
    if (just_update)
        update();
    QRect sr = valid_rect ? r : visibleRegion().boundingRect();
    int x1, y1, x2, y2, w = sr.width(), h = sr.height();
    if (dx > 0) {
        x1 = sr.x();
        x2 = x1+dx;
        w -= dx;
    } else {
        x2 = sr.x();
        x1 = x2-dx;
        w += dx;
    }
    if (dy > 0) {
        y1 = sr.y();
        y2 = y1+dy;
        h -= dy;
    } else {
        y2 = sr.y();
        y1 = y2-dy;
        h += dy;
    }

    if (dx == 0 && dy == 0)
        return;

    Display *dpy = X11->display;
    GC gc = qt_xget_readonly_gc(d->xinfo.screen(), false);
    // Want expose events
    if (w > 0 && h > 0 && !just_update) {
        XSetGraphicsExposures(dpy, gc, True);
        XCopyArea(dpy, winId(), winId(), gc, x1, y1, w, h, x2, y2);
        XSetGraphicsExposures(dpy, gc, False);
    }

    if (!valid_rect && !d->children.isEmpty()) {        // scroll children
        QPoint pd(dx, dy);
        for (int i = 0; i < d->children.size(); ++i) { // move all children
            register QObject *object = d->children.at(i);
            if (object->isWidgetType()) {
                QWidget *w = static_cast<QWidget *>(object);
                if (!w->isTopLevel())
                    w->move(w->pos() + pd);
            }
        }
    }

    if (just_update)
        return;

    // Don't let the server be bogged-down with repaint events
    bool repaint_immediately = (qt_sip_count(this) < 3 && !testWState(Qt::WState_InPaintEvent));

    if (dx) {
        int x = x2 == sr.x() ? sr.x()+w : sr.x();
        if (repaint_immediately)
            repaint(x, sr.y(), qAbs(dx), sr.height());
        else
            XClearArea(dpy, data->winid, x, sr.y(), qAbs(dx), sr.height(), True);
    }
    if (dy) {
        int y = y2 == sr.y() ? sr.y()+h : sr.y();
        if (repaint_immediately)
            repaint(sr.x(), y, sr.width(), qAbs(dy));
        else
            XClearArea(dpy, data->winid, sr.x(), y, sr.width(), qAbs(dy), True);
    }

    qt_insert_sip(this, dx, dy); // #### ignores r
}

/*!
    Internal implementation of the virtual QPaintDevice::metric()
    function.

    \a m is the metric to get.
*/

int QWidget::metric(PaintDeviceMetric m) const
{
    int val;
    if (m == PdmWidth) {
        val = data->crect.width();
    } else if (m == PdmHeight) {
        val = data->crect.height();
    } else {
        Display *dpy = X11->display;
        int scr = d->xinfo.screen();
        switch (m) {
            case PdmDpiX:
            case PdmPhysicalDpiX:
                val = QX11Info::appDpiX(scr);
                break;
            case PdmDpiY:
            case PdmPhysicalDpiY:
                val = QX11Info::appDpiY(scr);
                break;
            case PdmWidthMM:
                val = (DisplayWidthMM(dpy,scr)*data->crect.width())/
                      DisplayWidth(dpy,scr);
                break;
            case PdmHeightMM:
                val = (DisplayHeightMM(dpy,scr)*data->crect.height())/
                      DisplayHeight(dpy,scr);
                break;
            case PdmNumColors:
                val = d->xinfo.cells();
                break;
            case PdmDepth:
                val = d->xinfo.depth();
                break;
            default:
                val = 0;
                qWarning("QWidget::metric: Invalid metric command");
        }
    }
    return val;
}

void QWidgetPrivate::createSysExtra()
{
    extra->xDndProxy = 0;
    extra->children_use_dnd = false;
    extra->compress_events = true;
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
}

void QWidgetPrivate::deleteTLSysExtra()
{
    // don't destroy input context here. it will be destroyed in
    // QWidget::destroy() destroyInputContext();
}

/*
   examine the children of our parent up the tree and set the
   children_use_dnd extra data appropriately... this is used to keep DND enabled
   for widgets that are reparented and don't have DND enabled, BUT *DO* have
   children (or children of children ...) with DND enabled...
*/
void QWidgetPrivate::checkChildrenDnd()
{
    QWidget *widget = q;
    while (widget && !widget->isDesktop()) {
        // note: this isn't done for the desktop widget
        bool children_use_dnd = false;
        for (int i = 0; i < widget->d->children.size(); ++i) {
            const QObject *object = widget->d->children.at(i);
            if (object->isWidgetType()) {
                const QWidget *child = static_cast<const QWidget *>(object);
                children_use_dnd = (children_use_dnd ||
                                    child->acceptDrops() ||
                                    (child->d->extra &&
                                     child->d->extra->children_use_dnd));
            }
        }

        widget->d->createExtra();
        widget->d->extra->children_use_dnd = children_use_dnd;

        widget = widget->parentWidget();
    }
}

/*!
    \property QWidget::acceptDrops
    \brief whether drop events are enabled for this widget

    Setting this property to true announces to the system that this
    widget \e may be able to accept drop events.

    If the widget is the desktop (QWidget::isDesktop()), this may
    fail if another application is using the desktop; you can call
    acceptDrops() to test if this occurs.

    \warning
    Do not modify this property in a Drag&Drop event handler.
*/
bool QWidget::acceptDrops() const
{
    return testWState(Qt::WState_DND);
}

void QWidget::setAcceptDrops(bool on)
{
    if ((bool)testWState(Qt::WState_DND) != on) {
        if (X11->dndEnable(this, on)) {
            if (on)
                setWState(Qt::WState_DND);
            else
                clearWState(Qt::WState_DND);
        }

        d->checkChildrenDnd();
    }
}

/*!
    \overload

    Causes only the parts of the widget which overlap \a region to be
    visible. If the region includes pixels outside the rect() of the
    widget, window system controls in that area may or may not be
    visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    \sa setMask(), clearMask()
*/

void QWidget::setMask(const QRegion& region)
{
    d->createExtra();
    if(QWExtra *extra = d->extraData())
        extra->mask = region;

    XShapeCombineRegion(X11->display, winId(), ShapeBounding, 0, 0,
                         region.handle(), ShapeSet);
}

/*!
    Causes only the pixels of the widget for which \a bitmap has a
    corresponding 1 bit to be visible. If the region includes pixels
    outside the rect() of the widget, window system controls in that
    area may or may not be visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    See \c examples/tux for an example of masking for transparency.

    \sa setMask(), clearMask()
*/

void QWidget::setMask(const QBitmap &bitmap)
{
    d->createExtra();
    if(QWExtra *extra = d->extraData())
        extra->mask = QRegion(bitmap);

    QBitmap bm = bitmap;
    if (bm.x11Info().screen() != d->xinfo.screen())
        bm.x11SetScreen(d->xinfo.screen());
    XShapeCombineMask(X11->display, winId(), ShapeBounding, 0, 0,
                       bm.handle(), ShapeSet);
}

/*!
    Removes any mask set by setMask().

    \sa setMask()
*/

void QWidget::clearMask()
{
    XShapeCombineMask(X11->display, winId(), ShapeBounding, 0, 0,
                       XNone, ShapeSet);
}

/*!
  \internal

  Computes the frame rectangle when needed.  This is an internal function, you
  should never call this.
*/

void QWidgetPrivate::updateFrameStrut() const
{
    if (! q->isVisible() || q->isDesktop()) {
        q->data->fstrut_dirty = (!q->isVisible());
        return;
    }

    Atom type_ret;
    Window l = q->winId(), w = l, p, r; // target window, it's parent, root
    Window *c;
    int i_unused;
    unsigned int nc;
    unsigned char *data_ret;
    unsigned long l_unused;

    while (XQueryTree(X11->display, w, &r, &p, &c, &nc)) {
        if (c && nc > 0)
            XFree(c);

        if (! p) {
            qWarning("QWidget::updateFrameStrut(): ERROR - no parent");
            return;
        }

        // if the parent window is the root window, an Enlightenment virtual root or
        // a NET WM virtual root window, stop here
        data_ret = 0;
        if (p == r ||
            (XGetWindowProperty(X11->display, p,
                                ATOM(ENLIGHTENMENT_DESKTOP), 0, 1, False, XA_CARDINAL,
                                &type_ret, &i_unused, &l_unused, &l_unused,
                                &data_ret) == Success &&
             type_ret == XA_CARDINAL)) {
            if (data_ret)
                XFree(data_ret);

            break;
        } else if (qt_net_supports(ATOM(_NET_VIRTUAL_ROOTS)) && X11->net_virtual_root_list) {
            int i = 0;
            while (X11->net_virtual_root_list[i] != 0) {
                if (X11->net_virtual_root_list[i++] == p)
                    break;
            }
        }

        l = w;
        w = p;
    }

    // we have our window
    int transx, transy;
    XWindowAttributes wattr;
    if (XTranslateCoordinates(X11->display, l, w,
                              0, 0, &transx, &transy, &p) &&
        XGetWindowAttributes(X11->display, w, &wattr)) {
        QTLWExtra *top = topData();
        top->fleft = transx;
        top->ftop = transy;
        top->fright = wattr.width - q->data->crect.width() - top->fleft;
        top->fbottom = wattr.height - q->data->crect.height() - top->ftop;

        // add the border_width for the window managers frame... some window managers
        // do not use a border_width of zero for their frames, and if we the left and
        // top strut, we ensure that pos() is absolutely correct.  frameGeometry()
        // will still be incorrect though... perhaps i should have foffset as well, to
        // indicate the frame offset (equal to the border_width on X).
        // - Brad
        top->fleft += wattr.border_width;
        top->fright += wattr.border_width;
        top->ftop += wattr.border_width;
        top->fbottom += wattr.border_width;
    }

    q->data->fstrut_dirty = 0;
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
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        return 0;

    if (d->ic)
        return d->ic;
    return qApp->inputContext();
}


/*!
  This function sets an input context specified by \a
  identifierName on this widget.

  \sa inputContext()
*/
void QWidget::setInputContext( const QString& identifierName )
{
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        return;
    if (d->ic)
	delete d->ic;
    // an input context that has the identifierName is generated.
    d->ic = QInputContextFactory::create(identifierName, this);
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
    QInputContext *qic = q->inputContext();
    if( qic )
	qic->reset();
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
    QInputContext *qic = q->inputContext();
    if (qic) {
	if(qic->focusWidget() != q)
	    qic->setFocusWidget(q);
    }
#endif // QT_NO_IM
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
    QInputContext *qic = q->inputContext();
    if ( qic ) {
	qic->setFocusWidget( 0 );
    }
#endif // QT_NO_IM
}

void QWidget::setWindowOpacity(qreal)
{
}

qreal QWidget::windowOpacity() const
{
    return 1.0;
}

/*!
    \internal
*/
const QX11Info &QWidget::x11Info() const
{
    return d->xinfo;
}

void QWidgetPrivate::setWindowRole(const char *role)
{
    XChangeProperty(X11->display, q->winId(),
                    ATOM(WM_WINDOW_ROLE), XA_STRING, 8, PropModeReplace,
                    (unsigned char *)role, qstrlen(role));
}

Q_GLOBAL_STATIC(QX11PaintEngine, qt_widget_paintengine)
QPaintEngine *QWidget::paintEngine() const
{
    if (qt_widget_paintengine()->isActive()) {
        QPaintEngine *engine = new QX11PaintEngine();
        engine->setAutoDestruct(true);
        return engine;
    }
    return qt_widget_paintengine();
}

/*!
    Returns the Xft picture handle of the widget for XRender
    support. Use of this function is not portable. This function will
    return 0 if XRender support is not compiled into Qt, if the
    XRender extension is not supported on the X11 display, or if the
    handle could not be created.
*/
Qt::HANDLE QWidget::xftPictureHandle() const
{
#ifndef QT_NO_XFT
    return d->xft_hd ? XftDrawPicture((XftDraw *) d->xft_hd) : 0;
#else
    return 0;
#endif // QT_NO_XFT
}

/*!
    Returns the Xft draw handle of the widget for XRender
    support. Use of this function is not portable. This function will
    return 0 if XRender support is not compiled into Qt, if the
    XRender extension is not supported on the X11 display, or if the
    handle could not be created.
*/
Qt::HANDLE QWidget::xftDrawHandle() const
{
    return d->xft_hd;
}
