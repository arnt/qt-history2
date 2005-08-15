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
#include "qcolormap.h"
#include "qdebug.h"

extern bool qt_reuse_double_buffer; // declared in qapplication_x11.cpp

#include <private/qpixmap_p.h>
#include <private/qpaintengine_x11_p.h>
#include "qt_x11_p.h"
#include "qx11info_x11.h"

#include <stdlib.h>

// defined in qapplication_x11.cpp
bool qt_wstate_iconified(WId);
void qt_updated_rootinfo();


#if !defined(QT_NO_IM)
#include "qinputcontext.h"
#include "qinputcontextfactory.h"
#endif

#include "qwidget_p.h"

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

Q_GUI_EXPORT void qt_x11_wait_for_window_manager(QWidget* w)
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
    if (!w->isVisible()) // not managed by the window manager
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

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_Q(QWidget);
    Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    if (type == Qt::ToolTip)
        flags |= Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint;

    bool topLevel = (flags & Qt::Window);
    bool popup = (type == Qt::Popup);
    bool dialog = (type == Qt::Dialog
                   || type == Qt::Sheet
                   || (flags & Qt::MSWindowsFixedSizeDialogHint));
    bool desktop = (type == Qt::Desktop);
    bool tool = (type == Qt::Tool || type == Qt::SplashScreen
                 || type == Qt::ToolTip || type == Qt::Drawer);

    bool customize =  (flags & (
                                Qt::X11BypassWindowManagerHint
                                | Qt::FramelessWindowHint
                                | Qt::WindowTitleHint
                                | Qt::WindowSystemMenuHint
                                | Qt::WindowMinimizeButtonHint
                                | Qt::WindowMaximizeButtonHint
                                | Qt::WindowContextHelpButtonHint
                                ));


    if(topLevel && parentWidget) { // if our parent stays on top, so must we
        QWidget *ptl = parentWidget->window();
        if(ptl && (ptl->windowFlags() & Qt::WindowStaysOnTopHint))
            flags |= Qt::WindowStaysOnTopHint;
    }

    Window parentw, destroyw = 0;
    WId id;

    // always initialize
    if (!window)
        initializeWindow = true;

    if (desktop &&
        qt_x11_create_desktop_on_screen >= 0 &&
        qt_x11_create_desktop_on_screen != xinfo.screen()) {
        // desktop on a certain screen other than the default requested
        QX11InfoData *xd = &X11->screens[qt_x11_create_desktop_on_screen];
        xinfo.setX11Data(xd);
    } else if (parentWidget && parentWidget->d_func()->xinfo.screen() != xinfo.screen()) {
        xinfo = parentWidget->d_func()->xinfo;
    }

    //get display, screen number, root window and desktop geometry for
    //the current screen
    Display *dpy = X11->display;
    int scr = xinfo.screen();
    Window root_win = RootWindow(dpy, scr);
    int sw = DisplayWidth(dpy,scr);
    int sh = DisplayHeight(dpy,scr);

    if (desktop) {                                // desktop widget
        dialog = popup = false;                        // force these flags off
        q->data->crect.setRect(0, 0, sw, sh);
    } else if (topLevel) {                        // calc pos/size from screen
        q->data->crect.setRect(sw/4, 3*sh/10, sw/2, 4*sh/10);
    } else {                                        // child widget
        q->data->crect.setRect(0, 0, 100, 30);
    }

    parentw = topLevel ? root_win : parentWidget->winId();

    XSetWindowAttributes wsa;

    if (window) {                                // override the old window
        if (destroyOldWindow)
            destroyw = data.winid;
        id = window;
        setWinId(window);
        XWindowAttributes a;
        XGetWindowAttributes(dpy, window, &a);
        q->data->crect.setRect(a.x, a.y, a.width, a.height);

        if (a.map_state == IsUnmapped)
            q->setAttribute(Qt::WA_WState_Visible, false);
        else
            q->setAttribute(Qt::WA_WState_Visible);

        QX11InfoData* xd = xinfo.getX11Data(true);

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
                             XVisualIDFromVisual((Visual *) QX11Info::appVisual(xinfo.screen())));
        xd->colormap = a.colormap;
        xd->defaultColormap = (a.colormap == QX11Info::appColormap(xinfo.screen()));
        xinfo.setX11Data(xd);
    } else if (desktop) {                        // desktop widget
        id = (WId)parentw;                        // id = root window
//         QWidget *otherDesktop = find(id);        // is there another desktop?
//         if (otherDesktop && otherDesktop->testWFlags(Qt::WPaintDesktop)) {
//             otherDesktop->d->setWinId(0);        // remove id from widget mapper
//             d->setWinId(id);                     // make sure otherDesktop is
//             otherDesktop->d->setWinId(id);       // found first
//         } else {
        setWinId(id);
//         }
    } else {
        if (xinfo.defaultVisual() && xinfo.defaultColormap()) {
            id = (WId)qt_XCreateSimpleWindow(q, dpy, parentw,
                                             q->data->crect.left(), q->data->crect.top(),
                                             q->data->crect.width(), q->data->crect.height(),
                                             0,
                                             BlackPixel(dpy, xinfo.screen()),
                                             WhitePixel(dpy, xinfo.screen()));
        } else {
            wsa.background_pixel = WhitePixel(dpy, xinfo.screen());
            wsa.border_pixel = BlackPixel(dpy, xinfo.screen());
            wsa.colormap = xinfo.colormap();
            id = (WId)qt_XCreateWindow(q, dpy, parentw,
                                       data.crect.left(), data.crect.top(),
                                       data.crect.width(), data.crect.height(),
                                       0, xinfo.depth(), InputOutput,
                                       (Visual *) xinfo.visual(),
                                       CWBackPixel|CWBorderPixel|CWColormap,
                                       &wsa);
        }

        setWinId(id);                                // set widget id/handle + hd
    }

#ifndef QT_NO_XRENDER
    if (picture) {
        XRenderFreePicture(X11->display, picture);
        picture = 0;
    }

    if (X11->use_xrender) {
        picture = XRenderCreatePicture(dpy, id, XRenderFindVisualFormat (dpy, (Visual *) xinfo.visual()), 0, 0);
    }
#endif // QT_NO_XRENDER

    // NET window types
    long net_wintypes[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int curr_wintype = 0;

    QtMWMHints mwmhints;
    mwmhints.flags = 0L;
    mwmhints.functions = MWM_FUNC_ALL;
    mwmhints.decorations = MWM_DECOR_ALL;
    mwmhints.input_mode = 0L;
    mwmhints.status = 0L;

    if (topLevel) {
        ulong wsa_mask = 0;

        if (customize) {
            mwmhints.decorations = 0L;
            mwmhints.flags |= MWM_HINTS_DECORATIONS;

            // All these buttons depend on the system menu, so we enable it
            if (flags & (Qt::WindowMinimizeButtonHint
                         | Qt::WindowMaximizeButtonHint
                         | Qt::WindowContextHelpButtonHint))
                flags |= Qt::WindowSystemMenuHint;
            if (flags & Qt::FramelessWindowHint) {
                // override netwm type - quick and easy for KDE noborder
                net_wintypes[curr_wintype++] = ATOM(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE);
            } else {
                mwmhints.decorations |= MWM_DECOR_BORDER;
                mwmhints.decorations |= MWM_DECOR_RESIZEH;

                if (flags & Qt::WindowTitleHint)
                    mwmhints.decorations |= MWM_DECOR_TITLE;

                if (flags & Qt::WindowSystemMenuHint)
                    mwmhints.decorations |= MWM_DECOR_MENU;

                if (flags & Qt::WindowMinimizeButtonHint)
                    mwmhints.decorations |= MWM_DECOR_MINIMIZE;

                if (flags & Qt::WindowMaximizeButtonHint)
                    mwmhints.decorations |= MWM_DECOR_MAXIMIZE;
            }
        } else if (desktop || popup) {
        } else if (dialog) {
            flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint;
        } else if (type == Qt::SplashScreen) {
            if (qt_net_supports(ATOM(_NET_WM_WINDOW_TYPE_SPLASH))) {
                flags &= ~Qt::X11BypassWindowManagerHint;
                net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_SPLASH);
            } else {
                flags |= Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint;
            }
        } else if (type == Qt::Tool || type == Qt::Drawer) {
            flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
        } else {
            flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint;
        }

        if (tool) {
            wsa.save_under = True;
            wsa_mask |= CWSaveUnder;
        }
        // ### need a better way to do this
        if (q->inherits("QMenu")) {
            // menu netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_MENU);
        } else if (q->inherits("QToolBar")) {
            // toolbar netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_TOOLBAR);
        } else if (type == Qt::Tool || type == Qt::Drawer) {
            // utility netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_UTILITY);
        }

        if (dialog) // dialog netwm type
            net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_DIALOG);
        // normal netwm type - default
        net_wintypes[curr_wintype++] = ATOM(_NET_WM_WINDOW_TYPE_NORMAL);

        if (flags & Qt::X11BypassWindowManagerHint) {
            wsa.override_redirect = True;
            wsa_mask |= CWOverrideRedirect;
        }

        if (wsa_mask && initializeWindow)
            XChangeWindowAttributes(dpy, id, wsa_mask, &wsa);
    } else if (!customize) {
        flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;
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
        QWidget *p = parentWidget;
        if (p)
            p = p->window();

        if (dialog || tool) {
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
        size_hints.x = data.crect.left();
        size_hints.y = data.crect.top();
        size_hints.width = data.crect.width();
        size_hints.height = data.crect.height();
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
        class_hint.res_class = const_cast<char *>(QX11Info::appClass());   // application class

        XSetWMProperties(dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints, &class_hint);

        XResizeWindow(dpy, id, data.crect.width(), data.crect.height());
        XStoreName(dpy, id, appName.data());
        Atom protocols[4];
        int n = 0;
        protocols[n++] = ATOM(WM_DELETE_WINDOW);        // support del window protocol
        protocols[n++] = ATOM(WM_TAKE_FOCUS);                // support take focus window protocol
        protocols[n++] = ATOM(_NET_WM_PING);                // support _NET_WM_PING protocol
        if (flags & Qt::WindowContextHelpButtonHint)
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
        data.fstrut_dirty = 1;

        // declare the widget's object name as window role
        QByteArray objName = q->objectName().toLocal8Bit();
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
        data.fstrut_dirty = 0;
    }

    if (initializeWindow) {
        // don't erase when resizing
        wsa.bit_gravity = QApplication::isRightToLeft() ? NorthEastGravity : NorthWestGravity;
        XChangeWindowAttributes(dpy, id, CWBitGravity, &wsa);
    }

    // set X11 event mask
    if (desktop) {
//         QWidget* main_desktop = find(id);
//         if (main_desktop->testWFlags(Qt::WPaintDesktop))
//             XSelectInput(dpy, id, stdDesktopEventMask | ExposureMask);
//         else
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
        q->setAttribute(Qt::WA_WState_Visible);
    } else if (topLevel) {                        // set X cursor
        q->setAttribute(Qt::WA_SetCursor);
        if (initializeWindow)
            qt_x11_enforce_cursor(q);
    }

    if (destroyw)
        qt_XDestroyWindow(q, dpy, destroyw);

    // newly created windows are positioned at the window system's
    // (0,0) position. If the parent uses wrect mapping to expand the
    // coordinate system, we must also adjust this widget's window
    // system position
    if (!topLevel && !parentWidget->data->wrect.topLeft().isNull())
        setWSGeometry();

#if !defined(QT_NO_IM)
    ic = 0;
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
    Q_D(QWidget);
    d->deactivateWidgetCleanup();
    if (testAttribute(Qt::WA_WState_Created)) {
        setAttribute(Qt::WA_WState_Created, false);
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
        if (isWindow())
            X11->deferred_map.removeAll(this);
        if (testAttribute(Qt::WA_ShowModal))                // just be sure we leave modal
            QApplicationPrivate::leaveModal(this);
        else if ((windowType() == Qt::Popup))
            qApp->d_func()->closePopup(this);

#ifndef QT_NO_XRENDER
        if (d->picture) {
            if (destroyWindow)
                XRenderFreePicture(X11->display, d->picture);
            d->picture = 0;
        }
#endif // QT_NO_XRENDER

        if ((windowType() == Qt::Desktop)) {
            if (acceptDrops())
                X11->dndEnable(this, false);
        } else {
            if (destroyWindow)
                qt_XDestroyWindow(this, X11->display, data->winid);
        }
        d->setWinId(0);

        extern void qPRCleanup(QWidget *widget); // from qapplication_x11.cpp
        if (testAttribute(Qt::WA_WState_Reparented))
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
    Q_Q(QWidget);
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
    WId old_winid = data.winid;
    if ((q->windowType() == Qt::Desktop))
        old_winid = 0;
    setWinId(0);

    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    XUnmapWindow(X11->display, old_winid);
    XReparentWindow(X11->display, old_winid, RootWindow(X11->display, xinfo.screen()), 0, 0);

    if (q->isWindow() || !parent) { // we are toplevel, or reparenting to toplevel
        QTLWExtra *top = topData();
        top->parentWinId = 0;
        // zero the frame strut and mark it dirty
        top->fleft = top->fright = top->ftop = top->fbottom = 0;
        data.fstrut_dirty = true;
    }

    QObjectPrivate::setParent_helper(parent);
    bool     enable = q->isEnabled();                // remember status
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize    s            = q->size();
    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);

    data.window_flags = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    q->create();
    if (q->isWindow() || (!parent || parent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    QObjectList chlist = q->children();
    for (int i = 0; i < chlist.size(); ++i) { // reparent children
        QObject *obj = chlist.at(i);
        if (obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if (xinfo.screen() != w->d_func()->xinfo.screen()) {
                // ### force setParent() to not shortcut out (because
                // ### we're setting the parent to the current parent)
                w->d_func()->parent = 0;
                w->setParent(q);
            } else if (!w->isWindow()) {
                XReparentWindow(X11->display, w->winId(), q->winId(),
                                w->geometry().x(), w->geometry().y());
            } else if ((w->windowType() == Qt::Popup)
                       || (w->windowFlags() & Qt::MSWindowsFixedSizeDialogHint)
                       || (w->windowType() == Qt::Dialog)
                       || (w->windowType() == Qt::SplashScreen)
                       || (w->windowType() == Qt::ToolTip)
                       || (w->windowType() == Qt::Tool)
                       || (w->windowType() == Qt::Drawer)
                       || (w->windowType() == Qt::Sheet)) {
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

    if (q->isWindow()) {
        uint window_state = data.window_state;
        const QRect r = topData()->normalGeometry;
        q->setGeometry(0, 0, s.width(), s.height());
        data.window_state = window_state;
        topData()->normalGeometry = r;
        if (!extra->topextra->caption.isEmpty())
            setWindowTitle_helper(extra->topextra->caption);
    } else {
        q->setGeometry(0, 0, s.width(), s.height());
    }

    setEnabled_helper(enable); //preserving WA_ForceDisabled

    q->setFocusPolicy(fp);
    if (extra && !extra->mask.isEmpty())
        q->setMask(extra->mask);
    if (old_winid)
        qt_XDestroyWindow(q, X11->display, old_winid);
    if (setcurs)
        q->setCursor(oldcurs);

    // re-register dnd
    if (oldparent)
        oldparent->d_func()->checkChildrenDnd();

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
    Q_D(const QWidget);
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
    Q_D(const QWidget);
    int           x, y;
    Window child;
    XTranslateCoordinates(X11->display,
                          QApplication::desktop()->screen(d->xinfo.screen())->winId(),
                          winId(), pos.x(), pos.y(), &x, &y, &child);
    return d->mapFromWS(QPoint(x, y));
}

void QWidgetPrivate::updateSystemBackground()
{
    Q_Q(QWidget);
    QBrush brush = q->palette().brush(QPalette::Active, q->backgroundRole());
    Qt::WindowType type = q->windowType();
    if (brush.style() == Qt::NoBrush
        || q->testAttribute(Qt::WA_NoSystemBackground)
        || q->testAttribute(Qt::WA_UpdatesDisabled)
        || type == Qt::Popup || type == Qt::ToolTip
        )
        XSetWindowBackgroundPixmap(X11->display, q->winId(), XNone);
    else if (isBackgroundInherited())
        XSetWindowBackgroundPixmap(X11->display, q->winId(), ParentRelative);
    else if (brush.style() == Qt::TexturePattern)
        XSetWindowBackgroundPixmap(X11->display, q->winId(),
                                   brush.texture().data->x11ConvertToDefaultDepth());
    else
        XSetWindowBackground(X11->display, q->winId(),
                             QColormap::instance(xinfo.screen()).pixel(brush.color()));
}

void QWidget::setCursor(const QCursor &cursor)
{
    Q_D(QWidget);
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
    Q_D(QWidget);
    if (d->extra) {
        delete d->extra->curs;
        d->extra->curs = 0;
    }
    if (!isWindow())
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

void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
    Q_Q(QWidget);
    XSetWMName(X11->display, q->winId(), qstring_to_xtp(caption));

    QByteArray net_wm_name = caption.toUtf8();
    XChangeProperty(X11->display, q->winId(), ATOM(_NET_WM_NAME), ATOM(UTF8_STRING), 8,
                    PropModeReplace, (unsigned char *)net_wm_name.data(), net_wm_name.size());
}

void QWidgetPrivate::setWindowIcon_sys()
{
    Q_Q(QWidget);
    QTLWExtra *topData = extra->topextra;
    if (topData->iconPixmap)
        // already been set
        return;

    XWMHints *h = XGetWMHints(X11->display, q->winId());
    XWMHints wm_hints;
    if (!h) {
        h = &wm_hints;
        h->flags = 0;
    }

    QIcon icon = q->windowIcon();
    if (!icon.isNull()) {
        QSize size = icon.actualSize(QSize(64, 64));
        QPixmap pixmap = icon.pixmap(size);
        /*
          if the app is not using the default visual, convert the icon
          to 1bpp as stated in the ICCCM section 4.1.2.4; otherwise,
          create the icon pixmap in the default depth (even though
          this violates the ICCCM)
        */
        if (!QX11Info::appDefaultVisual(xinfo.screen())
            || !QX11Info::appDefaultColormap(xinfo.screen())) {
            // non-default visual/colormap, use 1bpp bitmap
            topData->iconPixmap = new QBitmap(pixmap);
            h->icon_pixmap = topData->iconPixmap->handle();
        } else {
            // default depth, use a normal pixmap (even though this
            // violates the ICCCM)
            topData->iconPixmap = new QPixmap(pixmap);
            h->icon_pixmap = topData->iconPixmap->data->x11ConvertToDefaultDepth();
        }
        h->flags |= IconPixmapHint;

        QBitmap mask = topData->iconPixmap->mask();
        if (!mask.isNull()) {
            if (!topData->iconMask)
                topData->iconMask = new QBitmap;
            *topData->iconMask = mask;
            h->icon_mask = topData->iconMask->handle();
            h->flags |= IconMaskHint;
        }
    } else {
        h->flags &= ~(IconPixmapHint | IconMaskHint);
    }

    XSetWMHints(X11->display, q->winId(), h);
    if (h != &wm_hints)
        XFree((char *)h);
}

void QWidgetPrivate::setWindowIconText_sys(const QString &iconText)
{
    Q_Q(QWidget);
    XSetWMIconName(X11->display, q->winId(), qstring_to_xtp(iconText));

    QByteArray icon_name = iconText.toUtf8();
    XChangeProperty(X11->display, q->winId(), ATOM(_NET_WM_ICON_NAME), ATOM(UTF8_STRING), 8,
                    PropModeReplace, (unsigned char *) icon_name.constData(), icon_name.size());
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
        XUngrabPointer(X11->display, X11->time);
        XFlush(X11->display);
        mouseGrb = 0;
    }
}

/*!
    Grabs the keyboard input.

    This widget receives all keyboard events until releaseKeyboard()
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
    window.  It will change the color of the task bar entry to indicate
    that the window has changed in some way. This is because Microsoft
    do not allow an application to interrupt what the user is currently
    doing in another application.

    \sa isActiveWindow(), window(), show()
*/

void QWidget::activateWindow()
{
    Q_D(QWidget);
    QWidget *tlw = window();
    if (tlw->isVisible() && !tlw->d_func()->topData()->embedded && !X11->deferred_map.contains(tlw)) {
        XSetInputFocus(X11->display, tlw->winId(), XRevertToParent, X11->time);
 	d->focusInputContext();
    }
}


void QWidget::update()
{
    Q_D(QWidget);
    if (isVisible() && updatesEnabled()) {
//         d->removePendingPaintEvents(); // ### this is far too slow to go in
        d->invalidated_region = d->clipRect();
        QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void QWidget::update(const QRegion &rgn)
{
    Q_D(QWidget);
    if (isVisible() && updatesEnabled()) {
        d->invalidated_region |= (rgn & d->clipRect());
        QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void QWidget::update(const QRect &r)
{
    Q_D(QWidget);
    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
    if (w && h && isVisible() && updatesEnabled()) {
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

    Qt::HANDLE hd, picture;
    int screen, depth;
    int width, height;
};

static QX11DoubleBuffer *qt_x11_global_double_buffer = 0;
static bool qt_x11_global_double_buffer_active = false;
static bool qt_x11_enable_global_db = true;

/*!
    This function can be used to enable/disable the global double
    buffering under X11.
*/

Q_GUI_EXPORT void qt_x11_set_global_double_buffer(bool enable)
{
    qt_x11_enable_global_db = enable;
}

static void qt_discard_double_buffer(QX11DoubleBuffer **db)
{
    if (!*db) return;

    XFreePixmap(X11->display, (*db)->hd);
#ifndef QT_NO_XRENDER
    if (X11->use_xrender)
        XRenderFreePicture(X11->display, (*db)->picture);
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
    if (*db != qt_x11_global_double_buffer) {
        // qDebug("--> discarding temporary double buffer");
        qt_discard_double_buffer(db);
    } else {
        // qDebug("--> global double buffer unused");
	qt_x11_global_double_buffer_active = false;
    }
}

static QX11DoubleBuffer *qt_x11_create_double_buffer(int screen, int depth, int width, int height)
{
    QX11DoubleBuffer *db = new QX11DoubleBuffer;
    db->hd = XCreatePixmap(X11->display, RootWindow(X11->display, screen), width, height, depth);
    db->picture = 0;
#ifndef QT_NO_XRENDER
    if (X11->use_xrender)
        db->picture = XRenderCreatePicture(X11->display, db->hd,
                                           XRenderFindVisualFormat(X11->display, (Visual *) QX11Info::appVisual()), 0, 0);
#endif
    db->screen = screen;
    db->depth = depth;
    db->width = width;
    db->height = height;
    return db;
}

static
void qt_x11_get_double_buffer(QX11DoubleBuffer **db, int screen, int depth, int width, int height)
{
    if (!qt_reuse_double_buffer || qt_x11_global_double_buffer_active) {
        // qDebug("<-- creating temporary double buffer");
        *db = qt_x11_create_double_buffer(screen, depth, width, height);
	return;
    }

    // qDebug("<-- using global double buffer");
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

 	width  = qMax(qt_x11_global_double_buffer->width, width);
 	height = qMax(qt_x11_global_double_buffer->height, height);

        qt_discard_double_buffer();
    }

    qt_x11_global_double_buffer = *db = qt_x11_create_double_buffer(screen, depth, width, height);
}

void QWidget::repaint(const QRegion& rgn)
{
    Q_D(QWidget);
    if (!isVisible() || !updatesEnabled() || !testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;

    if (testAttribute(Qt::WA_WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");


    if (!d->invalidated_region.isEmpty())
        d->invalidated_region -= rgn;

    setAttribute(Qt::WA_WState_InPaintEvent);

    QRect br = rgn.boundingRect();
    QVector<QRect> rects = rgn.rects();
    QRect brWS = d->mapToWS(br);

    bool double_buffer = (!testAttribute(Qt::WA_PaintOnScreen)
                          && !testAttribute(Qt::WA_NoSystemBackground)
                          && br.width()  <= QX11DoubleBuffer::MaxWidth
                          && br.height() <= QX11DoubleBuffer::MaxHeight
                          && !QPainter::redirected(this)
                          && qt_x11_enable_global_db);

    bool do_system_clip = !double_buffer && (rects.size() > 1 || (br != QRect(0,0,data->crect.width(),data->crect.height())));

    Qt::HANDLE old_hd = d->hd;
    Qt::HANDLE old_picture = d->picture;

    QPoint redirectionOffset;
    QX11DoubleBuffer *qDoubleBuffer = 0;
    if (double_buffer) {
        qt_x11_get_double_buffer(&qDoubleBuffer, d->xinfo.screen(), d->xinfo.depth(),
                                 br.width(), br.height());

	d->hd = qDoubleBuffer->hd;
	d->picture = qDoubleBuffer->picture;
        redirectionOffset = br.topLeft();
    } else {
        redirectionOffset = data->wrect.topLeft();
    }

    if (!redirectionOffset.isNull())
        QPainter::setRedirected(this, this, redirectionOffset);

    QPaintEngine *engine = paintEngine();

    if (engine && do_system_clip) {
        if (redirectionOffset.isNull()) {
            engine->setSystemClip(rgn);
        } else {
            QRegion redirectedRegion(rgn);
            redirectedRegion.translate(-redirectionOffset);
            engine->setSystemClip(redirectedRegion);
        }
    }

    QPaintEvent e(rgn);
    if (engine
        && !testAttribute(Qt::WA_NoBackground)
        && !testAttribute(Qt::WA_NoSystemBackground)) {
        d->composeBackground(br);
#ifdef QT3_SUPPORT
        e.setErased(true);
#endif
    }
    QApplication::sendSpontaneousEvent(this, &e);

    if (engine && do_system_clip)
        engine->setSystemClip(QRegion());

    if (!redirectionOffset.isNull())
        QPainter::restoreRedirected(this);

    if (double_buffer) {
        GC gc = XCreateGC(d->xinfo.display(), d->hd, 0, 0);
        if (testAttribute(Qt::WA_PaintUnclipped))
	    XSetSubwindowMode(X11->display, gc, IncludeInferiors);
        for (int i = 0; i < rects.size(); ++i) {
            QRect rr = d->mapToWS(rects.at(i));
            XCopyArea(X11->display, d->hd, winId(), gc,
                      rr.x() - brWS.x(), rr.y() - brWS.y(),
                      rr.width(), rr.height(),
                      rr.x(), rr.y());
        }
        XFreeGC(d->xinfo.display(), gc);

        d->hd = old_hd;
        d->picture = old_picture;

	qt_x11_release_double_buffer(&qDoubleBuffer);

        if (!QApplicationPrivate::active_window) {
            extern int qt_double_buffer_timer;
            if (qt_double_buffer_timer)
                qApp->killTimer(qt_double_buffer_timer);
            qt_double_buffer_timer = qApp->startTimer(500);
        }
    }

    // Clean out the temporary engine if used...
    if (d->extraPaintEngine) {
        delete d->extraPaintEngine;
        d->extraPaintEngine = 0;
    }

    setAttribute(Qt::WA_WState_InPaintEvent, false);
    if(!testAttribute(Qt::WA_PaintOutsidePaintEvent) && paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

    if (testAttribute(Qt::WA_ContentsPropagated))
        d->updatePropagatedBackground(&rgn);
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Q_D(QWidget);
    bool needShow = false;
    Qt::WindowStates oldstate = windowState();
    if (oldstate == newstate)
        return;
    if (isWindow()) {
        // Ensure the initial size is valid, since we store it as normalGeometry below.
        if (!testAttribute(Qt::WA_Resized) && !isVisible())
            adjustSize();

        QTLWExtra *top = d->topData();

        if ((oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized)) {
            if (qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_HORZ))
                && qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_VERT))) {
                if ((newstate & Qt::WindowMaximized) && !(oldstate & Qt::WindowFullScreen))
                    top->normalGeometry = geometry();
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
                if (newstate & Qt::WindowFullScreen)
                    top->normalGeometry = geometry();
                qt_change_net_wm_state(this, (newstate & Qt::WindowFullScreen),
                                       ATOM(_NET_WM_STATE_FULLSCREEN));
            } else {
                needShow = isVisible();

                if (newstate & Qt::WindowFullScreen) {
                    d->updateFrameStrut();
                    const QRect normalGeometry = geometry();
                    const QPoint fullScreenOffset = QPoint(top->fleft, top->ftop);

                    top->savedFlags = windowFlags();
                    setParent(0, Qt::Window | Qt::FramelessWindowHint);
                    const QRect r = top->normalGeometry;
                    setGeometry(qApp->desktop()->screenGeometry(this));
                    top->normalGeometry = r;

                    if (top->normalGeometry.width() < 0) {
                        top->normalGeometry = normalGeometry;
                        top->fullScreenOffset = fullScreenOffset;
                    }
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
                        setGeometry(top->normalGeometry.adjusted(-top->fullScreenOffset.x(),
                                                                 -top->fullScreenOffset.y(),
                                                                 -top->fullScreenOffset.x(),
                                                                 -top->fullScreenOffset.y()));
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

    data->window_state = newstate;

    if (needShow)
        show();

    if (newstate & Qt::WindowActive)
        activateWindow();

    QWindowStateChangeEvent e(oldstate);
    QApplication::sendEvent(this, &e);
}

/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);
    if (q->isWindow()) {
        XWMHints *h = XGetWMHints(X11->display, q->winId());
        XWMHints  wm_hints;
        bool got_hints = h != 0;
        if (!got_hints) {
            h = &wm_hints;
            h->flags = 0;
        }
        h->initial_state = q->isMinimized() ? IconicState : NormalState;
        h->flags |= StateHint;
        XSetWMHints(X11->display, q->winId(), h);
        if (got_hints)
            XFree((char *)h);

        // update _MOTIF_WM_HINTS
        QtMWMHints mwmhints = GetMWMHints(X11->display, q->winId());

        if (q->testAttribute(Qt::WA_ShowModal)) {
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

        Qt::WindowFlags flags = q->windowFlags();
        if (flags & Qt::WindowStaysOnTopHint) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_ABOVE);
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_STAYS_ON_TOP);
        }
        if (q->isFullScreen()) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_FULLSCREEN);
        }
        if (q->isMaximized()) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
        }
        if (q->testAttribute(Qt::WA_ShowModal)) {
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
            qt_x11_wait_for_window_manager(q);

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
            data.window_state = data.window_state | Qt::WindowMaximized;

            return;
        }

        if (q->isFullScreen() && !qt_net_supports(ATOM(_NET_WM_STATE_FULLSCREEN))) {
            XMapWindow(X11->display, q->winId());
            qt_x11_wait_for_window_manager(q);
            return;
        }
    }

    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;
    q->setAttribute(Qt::WA_Mapped);

    if (!q->isWindow()
        && (q->testAttribute(Qt::WA_NoBackground)
            || q->palette().brush(q->backgroundRole()).style() == Qt::LinearGradientPattern)) {
        XSetWindowBackgroundPixmap(X11->display, q->winId(), XNone);
        XMapWindow(X11->display, q->winId());
        return;
    }
    XMapWindow(X11->display, q->winId());
    
    // Freedesktop.org Startup Notification
    if (X11->startupId && q->isWindow()) {
        QByteArray message("remove: ID=");
        message.append(X11->startupId);
        sendStartupMessage(message.constData());
        X11->startupId = 0;
    }
}

/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidgetPrivate::sendStartupMessage(const char *message) const
{
    Q_Q(const QWidget);

    if (!message)
        return;

    XEvent xevent; 
    xevent.xclient.type = ClientMessage;
    xevent.xclient.message_type = ATOM(_NET_STARTUP_INFO_BEGIN);
    xevent.xclient.display = X11->display;
    xevent.xclient.window = q->winId();
    xevent.xclient.format = 8;

    Window rootWindow = RootWindow(X11->display, DefaultScreen(X11->display));
    uint sent = 0;
    uint length = strlen(message) + 1;
    do {
        if (sent == 20) 
            xevent.xclient.message_type = ATOM(_NET_STARTUP_INFO);
        
        for (uint i = 0; i < 20 && i + sent <= length; i++)
            xevent.xclient.data.b[i] = message[i + sent++]; 
            
        XSendEvent(X11->display, rootWindow, false, PropertyChangeMask, &xevent);
    } while (sent <= length); 
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidgetPrivate::hide_sys()
{
    Q_Q(QWidget);
    deactivateWidgetCleanup();
    if (q->isWindow()) {
        X11->deferred_map.removeAll(q);
        if (q->winId()) // in nsplugin, may be 0
            XWithdrawWindow(X11->display, q->winId(), xinfo.screen());

        QTLWExtra *top = topData();
        data.crect.moveTopLeft(QPoint(data.crect.x() - top->fleft,
                                          data.crect.y() - top->ftop));

        // zero the frame strut and mark it dirty
        top->fleft = top->fright = top->ftop = top->fbottom = 0;
        data.fstrut_dirty = true;

        XFlush(X11->display);
    } else {
        q->setAttribute(Qt::WA_Mapped, false);
        if (q->winId()) // in nsplugin, may be 0
            XUnmapWindow(X11->display, q->winId());
    }
}

void QWidgetPrivate::raise_sys()
{
    Q_Q(QWidget);
    XRaiseWindow(X11->display, q->winId());
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    XLowerWindow(X11->display, q->winId());
}

void QWidgetPrivate::stackUnder_sys(QWidget* w)
{
    Q_Q(QWidget);
    Window stack[2];
    stack[0] = w->winId();;
    stack[1] = q->winId();
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
void QWidgetPrivate::setWSGeometry(bool dontShow)
{
    Q_Q(QWidget);

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
        } else if (!q->isHidden()) {
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
            if (!w->isWindow())
                w->d_func()->setWSGeometry(jump);
        }
    }

    // move ourselves to the new position and map (if necessary) after
    // the movement. Rationale: moving unmapped windows is much faster
    // than moving mapped windows
    if (jump) //avoid flicker when jumping
        XSetWindowBackgroundPixmap(dpy, data.winid, XNone);
    XMoveResizeWindow(dpy, data.winid, xrect.x(), xrect.y(), xrect.width(), xrect.height());

    //to avoid flicker, we have to show children after the helper widget has moved
    if (jump) {
        for (int i = 0; i < children.size(); ++i) {
            QObject *object = children.at(i);
            if (object->isWidgetType()) {
                QWidget *w = static_cast<QWidget *>(object);
                if (!w->testAttribute(Qt::WA_OutsideWSRange) && !w->testAttribute(Qt::WA_Mapped) && !w->isHidden()) {
                    w->setAttribute(Qt::WA_Mapped);
                    XMapWindow(dpy, w->data->winid);
                }
            }
        }
    }


    if  (jump)
        XClearArea(dpy, data.winid, 0, 0, wrect.width(), wrect.height(), True);

    if (mapWindow && !dontShow) {
            q->setAttribute(Qt::WA_Mapped);
            XMapWindow(dpy, data.winid);
    }
}

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    Display *dpy = X11->display;

    if ((q->windowType() == Qt::Desktop))
        return;
    if (q->isWindow()) {
        if (!qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_VERT))
            && !qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_HORZ)))
            data.window_state &= ~Qt::WindowMaximized;
        if (!qt_net_supports(ATOM(_NET_WM_STATE_FULLSCREEN)))
            data.window_state &= ~Qt::WindowFullScreen;
        topData()->normalGeometry = QRect(0,0,-1,-1);
        w = qMax(1, w);
        h = qMax(1, h);
    } else {
        uint s = data.window_state;
        s &= ~(Qt::WindowMaximized | Qt::WindowFullScreen);
        data.window_state = s;
    }
    if (extra) {                                // any size restrictions?
        w = qMin(w,extra->maxw);
        h = qMin(h,extra->maxh);
        w = qMax(w,extra->minw);
        h = qMax(h,extra->minh);
    }
    QPoint oldPos(q->pos());
    QSize oldSize(q->size());
    QRect oldGeom(data.crect);
    QRect  r(x, y, w, h);

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if (!q->isWindow() && oldGeom == r)
        return;

    data.crect = r;
    bool isResize = q->size() != oldSize;

    if (q->isWindow()) {
        if (isMove)
            topData()->uspos = 1;
        if (isResize)
            topData()->ussize = 1;
        do_size_hints(q, extra);
        if (isMove) {
            if (! qt_broken_wm)
                // pos() is right according to ICCCM 4.1.5
                XMoveResizeWindow(dpy, data.winid, q->pos().x(), q->pos().y(), w, h);
            else
                // work around 4Dwm's incompliance with ICCCM 4.1.5
                XMoveResizeWindow(dpy, data.winid, x, y, w, h);
        } else if (isResize)
            XResizeWindow(dpy, data.winid, w, h);
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
                QMoveEvent e(data.crect.topLeft(), oldGeom.topLeft());
                QApplication::sendEvent(q, &e);
            }
        }
        if (isResize) {
            // set config pending only on resize, see qapplication_x11.cpp, translateConfigEvent()
            q->setAttribute(Qt::WA_WState_ConfigPending);

            QResizeEvent e(q->size(), oldSize);
            QApplication::sendEvent(q, &e);
        }
    } else {
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}

void QWidgetPrivate::setConstraints_sys()
{
    Q_Q(QWidget);
    do_size_hints(q, extra);
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
    Q_D(QWidget);
    if (!updatesEnabled() && children().size() == 0)
        return;
    bool valid_rect = r.isValid();
    bool just_update = qAbs(dx) > width() || qAbs(dy) > height();
    QRect sr = valid_rect ? r : d->clipRect();
    if (just_update) {
        update();
    } else if (!valid_rect){
        d->invalidated_region.translate(dx, dy);
    }

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
    // Want expose events
    if (w > 0 && h > 0 && !just_update) {
        GC gc = XCreateGC(dpy, winId(), 0, 0);
        XSetGraphicsExposures(dpy, gc, True);
        XCopyArea(dpy, winId(), winId(), gc, x1, y1, w, h, x2, y2);
        XFreeGC(dpy, gc);
    }

    if (!valid_rect && !d->children.isEmpty()) {        // scroll children
        QPoint pd(dx, dy);
        for (int i = 0; i < d->children.size(); ++i) { // move all children
            register QObject *object = d->children.at(i);
            if (object->isWidgetType()) {
                QWidget *w = static_cast<QWidget *>(object);
                if (!w->isWindow())
                    w->move(w->pos() + pd);
            }
        }
    }

    if (just_update)
        return;

    // Don't let the server be bogged-down with repaint events
    bool repaint_immediately = (qt_sip_count(this) < 3 && !testAttribute(Qt::WA_WState_InPaintEvent));

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
    Q_D(const QWidget);
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
    extra->topextra->iconMask = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    // don't destroy input context here. it will be destroyed in
    // QWidget::destroy() destroyInputContext();
    delete extra->topextra->iconMask;
    extra->topextra->iconMask = 0;
}

/*
   examine the children of our parent up the tree and set the
   children_use_dnd extra data appropriately... this is used to keep DND enabled
   for widgets that are reparented and don't have DND enabled, BUT *DO* have
   children (or children of children ...) with DND enabled...
*/
void QWidgetPrivate::checkChildrenDnd()
{
    Q_Q(QWidget);
    QWidget *widget = q;
    while (widget && !(widget->windowType() == Qt::Desktop)) {
        // note: this isn't done for the desktop widget
        bool children_use_dnd = false;
        for (int i = 0; i < widget->d_func()->children.size(); ++i) {
            const QObject *object = widget->d_func()->children.at(i);
            if (object->isWidgetType()) {
                const QWidget *child = static_cast<const QWidget *>(object);
                children_use_dnd = (children_use_dnd ||
                                    child->acceptDrops() ||
                                    (child->d_func()->extra &&
                                     child->d_func()->extra->children_use_dnd));
            }
        }

        widget->d_func()->createExtra();
        widget->d_func()->extra->children_use_dnd = children_use_dnd;

        widget = widget->parentWidget();
    }
}

bool QWidgetPrivate::setAcceptDrops_sys(bool on)
{
    bool ok = X11->dndEnable(q_func(), on);
    checkChildrenDnd(); // ## ???
    return ok;
}

/*!
    \overload

    Causes only the parts of the widget which overlap \a region to be
    visible. If the region includes pixels outside the rect() of the
    widget, window system controls in that area may or may not be
    visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.
*/

void QWidget::setMask(const QRegion& region)
{
    Q_D(QWidget);
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

    \omit
    See \c examples/tux for an example of masking for transparency.
    \endomit

    The following code shows how an image with an alpha channel can be
    used to generate a mask for a widget:

    \quotefromfile snippets/widget-mask/main.cpp
    \skipto QLabel
    \printuntil setMask

    The label shown by this code is masked using the image it contains,
    giving the appearance that an irregularly-shaped image is being drawn
    directly onto the screen.

    \sa clearMask()
*/

void QWidget::setMask(const QBitmap &bitmap)
{
    Q_D(QWidget);
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
    Q_Q(const QWidget);
    if (! q->isVisible() || (q->windowType() == Qt::Desktop)) {
        data.fstrut_dirty = (!q->isVisible());
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
        top->fright = wattr.width - data.crect.width() - top->fleft;
        top->fbottom = wattr.height - data.crect.height() - top->ftop;

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

   data.fstrut_dirty = 0;
}

void QWidget::setWindowOpacity(qreal opacity)
{
    if (isTopLevel()) {
        ulong value = ulong(opacity * 0xffffffff);
        XChangeProperty(QX11Info::display(), winId(), ATOM(_NET_WM_WINDOW_OPACITY), XA_CARDINAL,
                        32, PropModeReplace, (uchar*)&value, 1);
    }
}

qreal QWidget::windowOpacity() const
{
    Q_D(const QWidget);
    if (isTopLevel()) {
        QTLWExtra *topData = d->topData();
        return double(topData->opacity) / 255.;
    } else
        return 1.0;
}

/*!
    Returns information about the configuration of the X display used to display
    the widget.

    \warning This function is only available on X11.
*/
const QX11Info &QWidget::x11Info() const
{
    Q_D(const QWidget);
    return d->xinfo;
}

void QWidgetPrivate::setWindowRole(const char *role)
{
    Q_Q(QWidget);
    XChangeProperty(X11->display, q->winId(),
                    ATOM(WM_WINDOW_ROLE), XA_STRING, 8, PropModeReplace,
                    (unsigned char *)role, qstrlen(role));
}

Q_GLOBAL_STATIC(QX11PaintEngine, qt_widget_paintengine)
QPaintEngine *QWidget::paintEngine() const
{
    Q_D(const QWidget);
    if (qt_widget_paintengine()->isActive()) {
        if (d->extraPaintEngine)
            return d->extraPaintEngine;
        QWidget *self = const_cast<QWidget *>(this);
        self->d_func()->extraPaintEngine = new QX11PaintEngine();
        return d->extraPaintEngine;
    }
    return qt_widget_paintengine();
}

/*!
    Returns the X11 Picture handle of the widget for XRender
    support. Use of this function is not portable. This function will
    return 0 if XRender support is not compiled into Qt, if the
    XRender extension is not supported on the X11 display, or if the
    handle could not be created.
*/
Qt::HANDLE QWidget::x11PictureHandle() const
{
#ifndef QT_NO_XRENDER
    Q_D(const QWidget);
    return d->picture;
#else
    return 0;
#endif // QT_NO_XRENDER
}

#ifndef QT_NO_XRENDER
XRenderColor QX11Data::preMultiply(const QColor &c)
{
    XRenderColor color;
    const uint A = c.alpha(),
               R = c.red(),
               G = c.green(),
               B = c.blue();
    color.alpha = (A | A << 8);
    color.red   = (R | R << 8) * color.alpha / 0x10000;
    color.green = (G | G << 8) * color.alpha / 0x10000;
    color.blue  = (B | B << 8) * color.alpha / 0x10000;
    return color;
}
Picture QX11Data::getSolidFill(int screen, const QColor &c)
{
    if (!X11->use_xrender)
        return XNone;

    XRenderColor color = preMultiply(c);
    for (int i = 0; i < X11->solid_fill_count; ++i) {
        if (X11->solid_fills[i].screen == screen
            && X11->solid_fills[i].color.alpha == color.alpha
            && X11->solid_fills[i].color.red == color.red
            && X11->solid_fills[i].color.green == color.green
            && X11->solid_fills[i].color.blue == color.blue)
            return X11->solid_fills[i].picture;
    }
    // none found, replace one
    int i = rand() % 16;

    if (X11->solid_fills[i].screen != screen && X11->solid_fills[i].picture) {
	XRenderFreePicture (X11->display, X11->solid_fills[i].picture);
	X11->solid_fills[i].picture = 0;
    }

    if (!X11->solid_fills[i].picture) {
	Pixmap pixmap = XCreatePixmap (X11->display, RootWindow (X11->display, screen), 1, 1, 32);
        XRenderPictureAttributes attrs;
	attrs.repeat = True;
	X11->solid_fills[i].picture = XRenderCreatePicture (X11->display, pixmap,
                                                            XRenderFindStandardFormat(X11->display, PictStandardARGB32),
                                                            CPRepeat, &attrs);
	XFreePixmap (X11->display, pixmap);
    }

    X11->solid_fills[i].color = color;
    X11->solid_fills[i].screen = screen;
    XRenderFillRectangle (X11->display, PictOpSrc, X11->solid_fills[i].picture, &color, 0, 0, 1, 1);
    return X11->solid_fills[i].picture;
}
#endif
