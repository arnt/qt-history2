/****************************************************************************
**
** Implementation of QWidget and QWindow classes for X11.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include "qdatetime.h"
#include "qcursor.h"
#include "qstack.h"
#include "qcleanuphandler.h"

// Paint event clipping magic
extern void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

#include "qpaintengine_x11.h"
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


#ifndef QT_NO_XIM
#include "qinputcontext_p.h"
#endif

#include "qwidget_p.h"
#define d d_func()
#define q q_func()

#define XCOORD_MAX 32767
#define WRECT_MAX 8191

extern bool qt_dnd_enable(QWidget* w, bool on);
extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;

// defined in qapplication_x11.cpp
extern Time qt_x_time;
extern Time qt_x_user_time;

int qt_x11_create_desktop_on_screen = -1;


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

extern bool qt_broken_wm;

// defined in qapplication_x11.cpp
extern bool qt_net_supports(Atom);

#if defined (QT_TABLET_SUPPORT)
extern XDevice *devStylus;
extern XDevice *devEraser;
extern XEventClass event_list_stylus[7];
extern XEventClass event_list_eraser[7];
extern int qt_curr_events_stylus;
extern int qt_curr_events_eraser;
#endif

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
        XCreateSimpleWindow(QX11Info::appDisplay(),
                             QX11Info::appRootWindow(),
                             0, 0, 1, 1, 0, 0, 0);

    // set client leader property to itself
    XChangeProperty(QX11Info::appDisplay(),
                     X11->wm_client_leader, ATOM(WM_CLIENT_LEADER),
                     XA_WINDOW, 32, PropModeReplace,
                     (unsigned char *)&X11->wm_client_leader, 1);

    // If we are session managed, inform the window manager about it
    QByteArray session = qApp->sessionId().toLatin1();
    if (!session.isEmpty()) {
        XChangeProperty(QX11Info::appDisplay(),
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
            XDefineCursor(w->x11Info()->display(), w->winId(), oc->handle());
        } else if (w->isEnabled()) {
            XDefineCursor(w->x11Info()->display(), w->winId(), w->cursor().handle());
        } else {
            // enforce the windows behavior of clearing the cursor on
            // disabled widgets
            XDefineCursor(w->x11Info()->display(), w->winId(), XNone);
        }
    } else {
        XDefineCursor(w->x11Info()->display(), w->winId(), XNone);
    }
}

Q_GUI_EXPORT void qt_wait_for_window_manager(QWidget* w)
{
    QApplication::flush();
    XEvent ev;
    QTime t;
    t.start();
    while (!XCheckTypedWindowEvent(w->x11Info()->display(), w->winId(), ReparentNotify, &ev)) {
        if (XCheckTypedWindowEvent(w->x11Info()->display(), w->winId(), MapNotify, &ev))
            break;
        if (t.elapsed() > 500)
            return; // give up, no event available
        qApp->syncX(); // non-busy wait
    }
    qApp->x11ProcessEvent(&ev);
    if (XCheckTypedWindowEvent(w->x11Info()->display(), w->winId(), ConfigureNotify, &ev))
        qApp->x11ProcessEvent(&ev);
}

static void qt_net_change_wm_state(const QWidget* w, bool set, Atom one, Atom two = 0)
{
    if (w->isShown()) {
        // managed by WM
        XEvent e;
        e.xclient.type = ClientMessage;
        e.xclient.message_type = ATOM(_NET_WM_STATE);
        e.xclient.display = w->x11Info()->display();
        e.xclient.window = w->winId();
        e.xclient.format = 32;
        e.xclient.data.l[0] = set ? 1 : 0;
        e.xclient.data.l[1] = one;
        e.xclient.data.l[2] = two;
        e.xclient.data.l[3] = 0;
        e.xclient.data.l[4] = 0;
        XSendEvent(w->x11Info()->display(), RootWindow(w->x11Info()->display(), w->x11Info()->screen()),
                   False, (SubstructureNotifyMask|SubstructureRedirectMask), &e);
    } else {
        Atom ret;
        int format = 0, status;
        unsigned char *data = 0;
        unsigned long nitems = 0, after = 0;
        Atom *old_states = 0;
        status = XGetWindowProperty(w->x11Info()->display(), w->winId(),
                                    ATOM(_NET_WM_STATE), 0, 1024, False,
                                    XA_ATOM, &ret, &format, &nitems,
                                    &after, &data);
        if (status == Success && ret == XA_ATOM && format == 32 && nitems > 0)
            old_states = (Atom *) data;
        else
            nitems = 0;

        Atom *new_states = new Atom[nitems + 2];
        int i, j = 0;
        for (i = 0; i < (int)nitems; ++i) {
            if (old_states[i] && old_states[i] != one && old_states[i] != two)
                new_states[j++] = old_states[i];
        }

        if (set) {
            if (one) new_states[j++] = one;
            if (two) new_states[j++] = two;
        }

        if (j)
            XChangeProperty(w->x11Info()->display(), w->winId(), ATOM(_NET_WM_STATE), XA_ATOM, 32,
                            PropModeReplace, (uchar *) new_states, j);
        else
            XDeleteProperty(w->x11Info()->display(), w->winId(), ATOM(_NET_WM_STATE));

        delete [] new_states;
        if (data) XFree(data);
    }
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

    if (!d->xinfo)
        d->xinfo = new QX11Info;

    if (desktop &&
        qt_x11_create_desktop_on_screen >= 0 &&
        qt_x11_create_desktop_on_screen != d->xinfo->screen()) {
        // desktop on a certain screen other than the default requested
        QX11InfoData *xd = d->xinfo->getX11Data(true);
        xd->x_screen = qt_x11_create_desktop_on_screen;
        xd->x_depth = QX11Info::appDepth(xd->x_screen);
        xd->x_cells = QX11Info::appCells(xd->x_screen);
        xd->x_colormap = QX11Info::appColormap(xd->x_screen);
        xd->x_defcolormap = QX11Info::appDefaultColormap(xd->x_screen);
        xd->x_visual = QX11Info::appVisual(xd->x_screen);
        xd->x_defvisual = QX11Info::appDefaultVisual(xd->x_screen);
        d->xinfo->setX11Data(xd);
    } else if (parentWidget() &&  parentWidget()->d->xinfo->screen() != d->xinfo->screen()) {
        // if we have a parent widget, move to its screen if necessary
        QX11InfoData* xd = d->xinfo->getX11Data(true);
        xd->x_screen = parentWidget()->d->xinfo->screen();
        xd->x_depth = QX11Info::appDepth(xd->x_screen);
        xd->x_cells = QX11Info::appCells(xd->x_screen);
        xd->x_colormap = QX11Info::appColormap(xd->x_screen);
        xd->x_defcolormap = QX11Info::appDefaultColormap(xd->x_screen);
        xd->x_visual = QX11Info::appVisual(xd->x_screen);
        xd->x_defvisual = QX11Info::appDefaultVisual(xd->x_screen);
        d->xinfo->setX11Data(xd);
    }

    //get display, screen number, root window and desktop geometry for
    //the current screen
    Display *dpy = d->xinfo->display();
    int scr = d->xinfo->screen();
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
        setWinId(window);
        XWindowAttributes a;
        XGetWindowAttributes(dpy, window, &a);
        data->crect.setRect(a.x, a.y, a.width, a.height);

        if (a.map_state == IsUnmapped)
            clearWState(Qt::WState_Visible);
        else
            setWState(Qt::WState_Visible);

        QX11InfoData* xd = d->xinfo->getX11Data(true);

        // find which screen the window is on...
        xd->x_screen = QX11Info::appScreen(); // by default, use the default :)
        int i;
        for (i = 0; i < ScreenCount(dpy); i++) {
            if (RootWindow(dpy, i) == a.root) {
                xd->x_screen = i;
                break;
            }
        }

        xd->x_depth = a.depth;
        xd->x_cells = DisplayCells(dpy, xd->x_screen);
        xd->x_visual = a.visual;
        xd->x_defvisual = (XVisualIDFromVisual((Visual *) a.visual) ==
                           XVisualIDFromVisual((Visual *) QX11Info::appVisual(d->xinfo->screen())));
        xd->x_colormap = a.colormap;
        xd->x_defcolormap = (a.colormap == QX11Info::appColormap(d->xinfo->screen()));
        d->xinfo->setX11Data(xd);
    } else if (desktop) {                        // desktop widget
        id = (WId)parentw;                        // id = root window
        QWidget *otherDesktop = find(id);        // is there another desktop?
        if (otherDesktop && otherDesktop->testWFlags(Qt::WPaintDesktop)) {
            otherDesktop->setWinId(0);        // remove id from widget mapper
            setWinId(id);                        // make sure otherDesktop is
            otherDesktop->setWinId(id);        //   found first
        } else {
            setWinId(id);
        }
    } else {
        if (d->xinfo->defaultVisual() && d->xinfo->defaultColormap()) {
            id = (WId)qt_XCreateSimpleWindow(this, dpy, parentw,
                                             data->crect.left(), data->crect.top(),
                                             data->crect.width(), data->crect.height(),
                                             0,
                                             QColor(Qt::black).pixel(d->xinfo->screen()),
                                             QColor(Qt::white).pixel(d->xinfo->screen()));
        } else {
            wsa.background_pixel = QColor(Qt::white).pixel(d->xinfo->screen());
            wsa.border_pixel = QColor(Qt::black).pixel(d->xinfo->screen());
            wsa.colormap = d->xinfo->colormap();
            id = (WId)qt_XCreateWindow(this, dpy, parentw,
                                       data->crect.left(), data->crect.top(),
                                       data->crect.width(), data->crect.height(),
                                       0, d->xinfo->depth(), InputOutput,
                                       (Visual *) d->xinfo->visual(),
                                       CWBackPixel|CWBorderPixel|CWColormap,
                                       &wsa);
        }

        setWinId(id);                                // set widget id/handle + hd
    }

#ifndef QT_NO_XFT
    if (d->xft_hd) {
        XftDrawDestroy((XftDraw *) d->xft_hd);
        d->xft_hd = 0;
    }

    if (X11->has_xft) {
        d->xft_hd = (Qt::HANDLE)
                    XftDrawCreate(dpy, id, (Visual *) d->xinfo->visual(), d->xinfo->colormap());
    }
#endif // QT_NO_XFT

    // NET window types
    long net_wintypes[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int curr_wintype = 0;

    // NET window states
    long net_winstates[6] = { 0, 0, 0, 0, 0, 0 };
    int curr_winstate = 0;

    struct {
        ulong flags, functions, decorations;
        long input_mode;
        ulong status;
    } mwmhints;

    mwmhints.flags = mwmhints.functions = 0L;
    mwmhints.decorations = (1L << 0); // MWM_DECOR_ALL
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
            mwmhints.flags |= (1L << 1); // MWM_HINTS_DECORATIONS

            if (testWFlags(Qt::WStyle_NoBorder)) {
                // override netwm type - quick and easy for KDE noborder
                net_wintypes[curr_wintype++] = ATOM(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE);
            } else {
                if (testWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_DialogBorder)) {
                    mwmhints.decorations |= (1L << 1); // MWM_DECOR_BORDER
                    mwmhints.decorations |= (1L << 2); //  MWM_DECOR_RESIZEH
                }

                if (testWFlags(Qt::WStyle_Title))
                    mwmhints.decorations |= (1L << 3); // MWM_DECOR_TITLE

                if (testWFlags(Qt::WStyle_SysMenu))
                    mwmhints.decorations |= (1L << 4); // MWM_DECOR_MENU

                if (testWFlags(Qt::WStyle_Minimize))
                    mwmhints.decorations |= (1L << 5); // MWM_DECOR_MINIMIZE

                if (testWFlags(Qt::WStyle_Maximize))
                    mwmhints.decorations |= (1L << 6); // MWM_DECOR_MAXIMIZE
            }

            if (testWFlags(Qt::WStyle_Tool)) {
                wsa.save_under = True;
                wsa_mask |= CWSaveUnder;
            }
        } else if (testWFlags(Qt::WType_Dialog)) {
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu | Qt::WStyle_ContextHelp);
        } else {
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_MinMax | Qt::WStyle_SysMenu);

            // maximized netwm state
            if (testWState(Qt::WState_Maximized)) {
                net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
                net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
            }
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

        // stays on top
        if (testWFlags(Qt::WStyle_StaysOnTop)) {
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_ABOVE);
            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_STAYS_ON_TOP);
        }

        if (testWFlags(Qt::WShowModal)) {
            mwmhints.input_mode = 3L; // MWM_INPUT_FULL_APPLICATION_MODAL
            mwmhints.flags |= (1L << 2); // MWM_HINTS_INPUT_MODE

            net_winstates[curr_winstate++] = ATOM(_NET_WM_STATE_MODAL);
        }

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
        QWidget *p = parentWidget();        // real parent
        if (p)
            p = p->topLevelWidget();

        if (dialog || testWFlags(Qt::WStyle_DialogBorder) || testWFlags(Qt::WStyle_Tool)) {
            if (p)
                XSetTransientForHint(dpy, id, p->winId());
            else                                // application-modal
                XSetTransientForHint(dpy, id, root_win);
        }

        // find the real client leader, i.e. a toplevel without parent
        while (p && p->parentWidget())
            p = p->parentWidget()->topLevelWidget();

        XSizeHints size_hints;
        size_hints.flags = USSize | PSize | PWinGravity;
        size_hints.x = data->crect.left();
        size_hints.y = data->crect.top();
        size_hints.width = data->crect.width();
        size_hints.height = data->crect.height();
        size_hints.win_gravity =
            QApplication::reverseLayout() ? NorthEastGravity : NorthWestGravity;

        XWMHints wm_hints;                        // window manager hints
        wm_hints.input = True;
        wm_hints.initial_state = NormalState;
        wm_hints.flags = InputHint | StateHint;

        if (!X11->wm_client_leader)
            create_wm_client_leader();

        wm_hints.window_group = X11->wm_client_leader;
        wm_hints.flags |= WindowGroupHint;

        XClassHint class_hint;
        class_hint.res_name = (char *) qAppName(); // application name
        class_hint.res_class = (char *) qAppClass();        // application class

        XSetWMProperties(dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints, &class_hint);

        XResizeWindow(dpy, id, data->crect.width(), data->crect.height());
        XStoreName(dpy, id, qAppName());
        Atom protocols[4];
        int n = 0;
        protocols[n++] = ATOM(WM_DELETE_WINDOW);        // support del window protocol
        protocols[n++] = ATOM(WM_TAKE_FOCUS);                // support take focus window protocol
        protocols[n++] = ATOM(_NET_WM_PING);                // support _NET_WM_PING protocol
        if (testWFlags(Qt::WStyle_ContextHelp))
            protocols[n++] = ATOM(_NET_WM_CONTEXT_HELP);
        XSetWMProtocols(dpy, id, protocols, n);

        // set mwm hints
        if (mwmhints.flags != 0l)
            XChangeProperty(dpy, id, ATOM(_MOTIF_WM_HINTS), ATOM(_MOTIF_WM_HINTS), 32,
                            PropModeReplace, (unsigned char *) &mwmhints, 5);
        else
            XDeleteProperty(dpy, id, ATOM(_MOTIF_WM_HINTS));

        // set _NET_WM_WINDOW_TYPE
        if (curr_wintype > 0)
            XChangeProperty(dpy, id, ATOM(_NET_WM_WINDOW_TYPE), XA_ATOM, 32, PropModeReplace,
                            (unsigned char *) net_wintypes, curr_wintype);
        else
            XDeleteProperty(dpy, id, ATOM(_NET_WM_WINDOW_TYPE));

        // set _NET_WM_WINDOW_STATE
        if (curr_winstate > 0)
            XChangeProperty(dpy, id, ATOM(_NET_WM_STATE), XA_ATOM, 32, PropModeReplace,
                            (unsigned char *) net_winstates, curr_winstate);
        else
            XDeleteProperty(dpy, id, ATOM(_NET_WM_STATE));

        // set _NET_WM_PID
        long curr_pid = getpid();
        XChangeProperty(dpy, id, ATOM(_NET_WM_PID), XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char *) &curr_pid, 1);

        // when we create a toplevel widget, the frame strut should be dirty
        data->fstrut_dirty = 1;

        // declare the widget's object name as window role
        XChangeProperty(dpy, id,
                        ATOM(WM_WINDOW_ROLE), XA_STRING, 8, PropModeReplace,
                        (unsigned char *)objectName().local8Bit(), qstrlen(objectName().local8Bit()));

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
        wsa.bit_gravity = QApplication::reverseLayout() ? NorthEastGravity : NorthWestGravity;
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
#if defined (QT_TABLET_SUPPORT)
        if (devStylus != NULL)
            XSelectExtensionEvent(dpy, id, event_list_stylus, qt_curr_events_stylus);
        if (devEraser != NULL)
            XSelectExtensionEvent(dpy, id, event_list_eraser, qt_curr_events_eraser);
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
    deactivateWidgetCleanup();
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
                qt_dnd_enable(this, false);
        } else {
            if (destroyWindow)
                qt_XDestroyWindow(this, d->xinfo->display(), data->winid);
        }
        setWinId(0);

        extern void qPRCleanup(QWidget *widget); // from qapplication_x11.cpp
        if (testWState(Qt::WState_Reparented))
            qPRCleanup(this);
        delete d->xinfo;
        d->xinfo = 0;
    }
}

void QWidget::reparent_sys(QWidget *parent, Qt::WFlags f, const QPoint &p, bool showIt)
{
    extern void qPRCreate(const QWidget *, Window);

    QCursor oldcurs;
    bool setcurs = testAttribute(Qt::WA_SetCursor);
    if (setcurs) {
        oldcurs = cursor();
        unsetCursor();
    }

    // dnd unregister (we will register again below)
    bool accept_drops = acceptDrops();
    setAcceptDrops(false);

    QWidget* oldtlw = topLevelWidget();
    QWidget *oldparent = parentWidget();
    WId old_winid = data->winid;
    if (testWFlags(Qt::WType_Desktop))
        old_winid = 0;
    setWinId(0);

    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    XUnmapWindow(d->xinfo->display(), old_winid);
    XReparentWindow(d->xinfo->display(), old_winid,
                     RootWindow(d->xinfo->display(), d->xinfo->screen()), 0, 0);

    if (isTopLevel()) {
        // input contexts are associated with toplevel widgets, so we need
        // destroy the context here.  if we are reparenting back to toplevel,
        // then we will have another context created, otherwise we will
        // use our new toplevel's context
        d->destroyInputContext();
    }

    if (isTopLevel() || !parent) // we are toplevel, or reparenting to toplevel
        d->topData()->parentWinId = 0;

    QObject::setParent_helper(parent);
    bool     enable = isEnabled();                // remember status
    Qt::FocusPolicy fp = focusPolicy();
    QSize    s            = size();
    QString capt = windowTitle();
    data->widget_flags = f;
    clearWState(Qt::WState_Created | Qt::WState_Visible | Qt::WState_Hidden | Qt::WState_ExplicitShowHide);
    create();
    if (isTopLevel() || (!parent || parent->isVisible()))
        setWState(Qt::WState_Hidden);

    QObjectList chlist = children();
    for (int i = 0; i < chlist.size(); ++i) { // reparent children
        QObject *obj = chlist.at(i);
        if (obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if (!w->isTopLevel()) {
                XReparentWindow(d->xinfo->display(), w->winId(), winId(),
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
                XUnmapWindow(w->d->xinfo->display(), w->winId());
                XSetTransientForHint(w->d->xinfo->display(), w->winId(), winId());
                QApplication::postEvent(w, new QEvent(QEvent::ShowWindowRequest));
            }
        }
    }
    qPRCreate(this, old_winid);
    d->updateSystemBackground();

    if (isTopLevel()) {
        uint save_state = data->widget_state & (Qt::WState_Maximized | Qt::WState_FullScreen);
        const QRect r = d->topData()->normalGeometry;
        setGeometry(p.x(), p.y(), s.width(), s.height());
        data->widget_state |= save_state;
        d->topData()->normalGeometry = r;
    } else {
        setGeometry(p.x(), p.y(), s.width(), s.height());
    }

    setEnabled(enable);
    setFocusPolicy(fp);
    if (!capt.isNull()) {
        d->extra->topextra->caption = QString::null;
        setWindowTitle(capt);
    }
    if (showIt)
        show();
    if (old_winid)
        qt_XDestroyWindow(this, d->xinfo->display(), old_winid);
    if (setcurs)
        setCursor(oldcurs);

    reparentFocusWidgets(oldtlw);

    // re-register dnd
    if (oldparent)
        oldparent->d->checkChildrenDnd();

    if (accept_drops)
        setAcceptDrops(true);
    else {
        d->checkChildrenDnd();
        d->topData()->dnd = 0;
        qt_dnd_enable(this, (d->extra && d->extra->children_use_dnd));
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
    XTranslateCoordinates(d->xinfo->display(), winId(),
                          QApplication::desktop()->screen(d->xinfo->screen())->winId(),
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
    XTranslateCoordinates(d->xinfo->display(),
                          QApplication::desktop()->screen(d->xinfo->screen())->winId(),
                          winId(), pos.x(), pos.y(), &x, &y, &child);
    return d->mapFromWS(QPoint(x, y));
}

/*!
    When a widget gets focus, it should call setMicroFocusHint() with
    some appropriate position and size, \a x, \a y, \a width and \a
    height. This has no \e visual effect, it just provides hints to
    any system-specific input handling tools.

    The \a text argument should be true if this is a position for text
    input.

    In the Windows version of Qt, this method sets the system caret,
    which is used for user Accessibility focus handling.  If \a text
    is true, it also sets the IME composition window in Far East Asian
    language input systems.

    In the X11 version of Qt, if \a text is true, this method sets the
    XIM "spot" point for complex language input handling.

    The font \a f is a rendering hint to the currently active input method.
    If \a f is 0 the widget's font is used.

    \sa microFocusHint()
*/
void QWidget::setMicroFocusHint(int x, int y, int width, int height,
                                bool text, QFont *f)
{
#ifndef QT_NO_XIM
    if (text) {
        QWidget* tlw = topLevelWidget();
        QTLWExtra *topdata = tlw->d->topData();

        // trigger input context creation if it hasn't happened already
        d->createInputContext();
        QInputContext *qic = (QInputContext *) topdata->xic;

        if (X11->xim && qic) {
            QPoint p(x, y);
            QPoint p2 = mapTo(topLevelWidget(), QPoint(0, 0));
            p = mapTo(topLevelWidget(), p);
            qic->setXFontSet(f ? *f : data->fnt);
            qic->setComposePosition(p.x(), p.y() + height);
            qic->setComposeArea(p2.x(), p2.y(), this->width(), this->height());
        }
    }
#endif

    if (QRect(x, y, width, height) != microFocusHint()) {
        d->createExtra();
        d->extraData()->micro_focus_hint.setRect(x, y, width, height);
    }
}


void QWidgetPrivate::setFont_sys(QFont *)
{
    // Nothing
}

void QWidgetPrivate::updateSystemBackground()
{
    QBrush brush = q->palette().brush(q->backgroundRole());
    if (brush.style() == Qt::NoBrush || q->testAttribute(Qt::WA_NoSystemBackground)) {
        XSetWindowBackgroundPixmap(xinfo->display(), q->winId(), XNone);
    } else if (brush.pixmap()) {
        QPixmap pix = *brush.pixmap();
        if (data.wrect.isValid() && !pix.isNull() && !isBackgroundInherited()) {
            int xoff = data.wrect.x() % pix.width();
            int yoff = data.wrect.y() % pix.height();
            if (xoff || yoff) {
                QPixmap newPix(pix.size(), pix.depth());
                qt_erase_background(&newPix, newPix.x11Info()->screen(), 0,0,pix.width(), pix.height(),
                                    pix, xoff, yoff);
                pix = newPix;
            }
        }
        XSetWindowBackgroundPixmap(xinfo->display(), q->winId(),
                                   isBackgroundInherited()
                                   ? ParentRelative
                                   : pix.handle());
    } else {
        XSetWindowBackground(xinfo->display(), q->winId(), brush.color().pixel(xinfo->screen()));
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
    XFlush(d->xinfo->display());
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
    XFlush(d->xinfo->display());
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
        errCode = XmbTextListToTextProperty(QX11Info::appDisplay(),
                                             tl, 1, XStdICCTextStyle, &tp);
#if defined(QT_DEBUG)
        if (errCode < 0)
            qDebug("qstring_to_xtp result code %d", errCode);
#endif
    }
    if (!mapper || errCode < 0) {
        static QByteArray qcs;
        qcs = s.ascii();
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
    XSetWMName(d->xinfo->display(), winId(), qstring_to_xtp(caption));

    QByteArray net_wm_name = caption.toUtf8();
    XChangeProperty(d->xinfo->display(), winId(), ATOM(_NET_WM_NAME), ATOM(UTF8_STRING), 8,
                    PropModeReplace, (unsigned char *)net_wm_name.data(), net_wm_name.size());

    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::setWindowIcon(const QPixmap &pixmap)
{
    if (d->extra && d->extra->topextra) {
        delete d->extra->topextra->icon;
        d->extra->topextra->icon = 0;
    } else {
        d->createTLExtra();
    }
    Pixmap icon_pixmap = 0;
    Pixmap mask_pixmap = 0;
    if (!pixmap.isNull()) {
        QPixmap* pm = new QPixmap(pixmap);
        d->extra->topextra->icon = pm;
        if (!pm->mask())
            pm->setMask(pm->createHeuristicMask()); // may do detach()
        icon_pixmap = pm->handle();
        if (pm->mask())
            mask_pixmap = pm->mask()->handle();
    }
    XWMHints *h = XGetWMHints(d->xinfo->display(), winId());
    XWMHints  wm_hints;
    bool got_hints = h != 0;
    if (!got_hints) {
        h = &wm_hints;
        h->flags = 0;
    }
    h->icon_pixmap = icon_pixmap;
    h->icon_mask = mask_pixmap;
    h->flags |= IconPixmapHint | IconMaskHint;
    XSetWMHints(d->xinfo->display(), winId(), h);
    if (got_hints)
        XFree((char *)h);
    QEvent e(QEvent::WindowIconChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::setWindowIconText(const QString &iconText)
{
    d->createTLExtra();
    d->extra->topextra->iconText = iconText;

    XSetWMIconName(d->xinfo->display(), winId(), qstring_to_xtp(iconText));

    QByteArray icon_name = iconText.toUtf8();
    XChangeProperty(d->xinfo->display(), winId(), ATOM(_NET_WM_ICON_NAME), ATOM(UTF8_STRING), 8,
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
            XGrabPointer(d->xinfo->display(), winId(), False,
                          (uint)(ButtonPressMask | ButtonReleaseMask |
                                  PointerMotionMask | EnterWindowMask |
                                  LeaveWindowMask),
                          GrabModeAsync, GrabModeAsync,
                          XNone, XNone, qt_x_time);
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
        XGrabPointer(d->xinfo->display(), winId(), False,
                      (uint)(ButtonPressMask | ButtonReleaseMask |
                             PointerMotionMask | EnterWindowMask | LeaveWindowMask),
                      GrabModeAsync, GrabModeAsync,
                      XNone, cursor.handle(), qt_x_time);
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
        XUngrabPointer(d->xinfo->display(),  qt_x_time);
        XFlush(d->xinfo->display());
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
        XGrabKeyboard(d->xinfo->display(), data->winid, False, GrabModeAsync, GrabModeAsync,
                       qt_x_time);
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
        XUngrabKeyboard(d->xinfo->display(), qt_x_time);
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
    window must be visible, otherwise setActiveWindow() has no effect.

    On Windows, if you are calling this when the application is not
    currently the active one then it will not make it the active
    window.  It will flash the task bar entry blue to indicate that
    the window has done something. This is because Microsoft do not
    allow an application to interrupt what the user is currently doing
    in another application.

    \sa isActiveWindow(), topLevelWidget(), show()
*/

void QWidget::setActiveWindow()
{
    QWidget *tlw = topLevelWidget();
    if (tlw->isVisible() && !tlw->d->topData()->embedded && !X11->deferred_map.contains(tlw)) {
        XSetInputFocus(d->xinfo->display(), tlw->winId(), XRevertToNone, qt_x_time);

#ifndef QT_NO_XIM
        // trigger input context creation if it hasn't happened already
        d->createInputContext();

        if (tlw->d->topData()->xic) {
            QInputContext *qic = (QInputContext *) tlw->d->topData()->xic;
            qic->setFocus();
        }
#endif
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

void QWidget::update(int x, int y, int w, int h)
{
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
static QX11DoubleBuffer *global_double_buffer = 0;

void qt_discard_double_buffer()
{
    if (!global_double_buffer) return;

    XFreePixmap(QX11Info::appDisplay(), global_double_buffer->hd);
#ifndef QT_NO_XFT
    if (X11->use_xrender && X11->has_xft)
        XftDrawDestroy((XftDraw *) global_double_buffer->xft_hd);
#endif

    delete global_double_buffer;
    global_double_buffer = 0;
}

static
void qt_x11_get_double_buffer(Qt::HANDLE &hd, Qt::HANDLE &xft_hd,
                              int screen, int depth, int width, int height)
{
    // the db should consist of 128x128 chunks
    width  = qMin(((width / 128) + 1) * 128, (int)QX11DoubleBuffer::MaxWidth);
    height = qMin(((height / 128) + 1) * 128, (int)QX11DoubleBuffer::MaxHeight);

    if (global_double_buffer) {
        if (global_double_buffer->screen == screen
            && global_double_buffer->depth == depth
            && global_double_buffer->width >= width
            && global_double_buffer->height >= height) {
            hd = global_double_buffer->hd;
            xft_hd = global_double_buffer->xft_hd;
            return;
        }

        width  = qMax(global_double_buffer->width,  width);
        height = qMax(global_double_buffer->height, height);

        qt_discard_double_buffer();
    }

    global_double_buffer = new QX11DoubleBuffer;
    global_double_buffer->hd =
        XCreatePixmap(QX11Info::appDisplay(), hd, width, height, depth);

#ifndef QT_NO_XFT
    if (X11->use_xrender && X11->has_xft)
        global_double_buffer->xft_hd =
            (Qt::HANDLE) XftDrawCreate(QX11Info::appDisplay(),
                                       global_double_buffer->hd,
                                       (Visual *) QX11Info::appVisual(),
                                       QX11Info::appColormap());
#endif

    global_double_buffer->screen = screen;
    global_double_buffer->depth = depth;
    global_double_buffer->width = width;
    global_double_buffer->height = height;

    hd = global_double_buffer->hd;
    xft_hd = global_double_buffer->xft_hd;
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
                          && br.width()  <= QX11DoubleBuffer::MaxWidth
                          && br.height() <= QX11DoubleBuffer::MaxHeight
                          && !QPainter::redirected(this));

    Qt::HANDLE old_hd = d->hd;
    Qt::HANDLE old_xft_hd = d->xft_hd;

    QPoint redirectionOffset;

    if (double_buffer) {
        qt_x11_get_double_buffer(d->hd, d->xft_hd, d->xinfo->screen(), d->xinfo->depth(),
                                 br.width(), br.height());
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

    if (testAttribute(Qt::WA_NoSystemBackground)) {
        if (double_buffer && !testAttribute(Qt::WA_NoBackground)) {
            GC gc = qt_xget_temp_gc(d->xinfo->screen(), false);
	    if (testAttribute(Qt::WA_PaintUnclipped))
		XSetSubwindowMode(d->xinfo->display(), gc, IncludeInferiors);
            XCopyArea(d->xinfo->display(), winId(), d->hd, gc,
                      brWS.x(), brWS.y(), brWS.width(), brWS.height(), 0, 0);
        }
    } else if (!testAttribute(Qt::WA_NoBackground)) {
        QPoint offset;
        QStack<QWidget*> parents;
        QWidget *w = q;
        while (w->d->isBackgroundInherited()) {
            offset += w->pos();
            w = w->parentWidget();
            parents += w;
        }

        if (double_buffer) {
            qt_erase_background(q, q->d->xinfo->screen(),
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
		    XClearArea(q->d->xinfo->display(), q->winId(),
			       rr.x(), rr.y(), rr.width(), rr.height(), False);
		}
            }
        }

        if (parents.size()) {
            w = parents.pop();
            for (;;) {
                if (w->testAttribute(Qt::WA_ContentsPropagated)) {
                    QPainter::setRedirected(w, q, offset + redirectionOffset);
                    QRect rr = d->clipRect();
                    rr.moveBy(offset);
                    QPaintEvent e(rr);
                    bool was_in_paint_event = w->testWState(Qt::WState_InPaintEvent);
                    w->setWState(Qt::WState_InPaintEvent);
                    QApplication::sendEvent(w, &e);
                    if(!was_in_paint_event) {
                        w->clearWState(Qt::WState_InPaintEvent);
                        if(w->paintingActive())
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
        GC gc = qt_xget_temp_gc(d->xinfo->screen(), false);
        QVector<QRect> rects = rgn.rects();
	if (testAttribute(Qt::WA_PaintUnclipped))
	    XSetSubwindowMode(d->xinfo->display(), gc, IncludeInferiors);
        for (int i = 0; i < rects.size(); ++i) {
            QRect rr = d->mapToWS(rects[i]);
            XCopyArea(d->xinfo->display(), d->hd, winId(), gc,
                      rr.x() - brWS.x(), rr.y() - brWS.y(),
                      rr.width(), rr.height(),
                      rr.x(), rr.y());
        }

        d->hd = old_hd;
        d->xft_hd = old_xft_hd;

        if (!qApp->active_window) {
            extern int qt_double_buffer_timer;
            if (qt_double_buffer_timer)
                qApp->killTimer(qt_double_buffer_timer);
            qt_double_buffer_timer = qApp->startTimer(500);
        }
    }

    clearWState(Qt::WState_InPaintEvent);
    if(paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

    if (testAttribute(Qt::WA_ContentsPropagated))
        d->updatePropagatedBackground(&rgn);
}

void QWidget::setWindowState(uint newstate)
{
    bool needShow = false;
    uint oldstate = windowState();
    if (isTopLevel()) {
        QTLWExtra *top = d->topData();

        if ((oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized)) {
            if (qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_HORZ))
                && qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_VERT))) {
                qt_net_change_wm_state(this, (newstate & Qt::WindowMaximized),
                                       ATOM(_NET_WM_STATE_MAXIMIZED_HORZ),
                                       ATOM(_NET_WM_STATE_MAXIMIZED_VERT));
            } else if (! (newstate & Qt::WindowFullScreen)) {
                if (newstate & Qt::WindowMaximized) {
                    // save original geometry
                    const QRect normalGeometry = geometry();

                    if (isVisible()) {
                        updateFrameStrut();
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
                qt_net_change_wm_state(this, (newstate & Qt::WindowFullScreen),
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
                        updateFrameStrut();
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
                    e.xclient.display = d->xinfo->display();
                    e.xclient.window = data->winid;
                    e.xclient.format = 32;
                    e.xclient.data.l[0] = IconicState;
                    e.xclient.data.l[1] = 0;
                    e.xclient.data.l[2] = 0;
                    e.xclient.data.l[3] = 0;
                    e.xclient.data.l[4] = 0;
                    XSendEvent(d->xinfo->display(),
                               RootWindow(d->xinfo->display(),d->xinfo->screen()),
                               False, (SubstructureNotifyMask|SubstructureRedirectMask), &e);
                } else {
                    setAttribute(Qt::WA_Mapped);
                    XMapWindow(d->xinfo->display(), winId());
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
        setActiveWindow();

    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}

/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::show_sys()
{
    if (isTopLevel() ) {
        XWMHints *h = XGetWMHints(d->xinfo->display(), winId());
        XWMHints  wm_hints;
        bool got_hints = h != 0;
        if (!got_hints) {
            h = &wm_hints;
            h->flags = 0;
        }
        h->initial_state = testWState(Qt::WState_Minimized) ? IconicState : NormalState;
        h->flags |= StateHint;
        XSetWMHints(d->xinfo->display(), winId(), h);
        if (got_hints)
            XFree((char *)h);

        if (qt_x_user_time != CurrentTime) {
            XChangeProperty(d->xinfo->display(), winId(), ATOM(_NET_WM_USER_TIME), XA_CARDINAL,
                            32, PropModeReplace, (unsigned char *) &qt_x_user_time, 1);
        }

        if (!d->topData()->embedded
            && d->topData()->parentWinId
            && d->topData()->parentWinId != QX11Info::appRootWindow(d->xinfo->screen())
            && !isMinimized()) {
            X11->deferred_map.append(this);
            return;
        }

        if (isMaximized() && !isFullScreen()
            && !(qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_HORZ))
                 && qt_net_supports(ATOM(_NET_WM_STATE_MAXIMIZED_VERT)))) {
	    XMapWindow( x11Info()->display(), winId() );
 	    qt_wait_for_window_manager(this);

 	    // if the wm was not smart enough to adjust our size, do that manually
 	    updateFrameStrut();
	    QRect maxRect = QApplication::desktop()->availableGeometry(this);

 	    QTLWExtra *top = d->topData();
 	    QRect normalRect = top->normalGeometry;

 	    setGeometry(maxRect.x() + top->fleft,
			maxRect.y() + top->ftop,
 			maxRect.width() - top->fleft - top->fright,
 			maxRect.height() - top->ftop - top->fbottom);

	    // restore the original normalGeometry
 	    top->normalGeometry = normalRect;
 	    // internalSetGeometry() clears the maximized flag... make sure we set it back
 	    setWState(Qt::WState_Maximized);

 	    return;
        }

	if (isFullScreen() && !qt_net_supports(ATOM(_NET_WM_STATE_FULLSCREEN))) {
 	    XMapWindow(x11Info()->display(), winId());
 	    qt_wait_for_window_manager(this);
	    return;
	}
    }

    if (testAttribute(Qt::WA_OutsideWSRange))
        return;
    setAttribute(Qt::WA_Mapped);
    XMapWindow(d->xinfo->display(), winId());
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hide_sys()
{
    clearWState(Qt::WState_Exposed);
    deactivateWidgetCleanup();
    if (isTopLevel()) {
        X11->deferred_map.removeAll(this);
        if (winId()) // in nsplugin, may be 0
            XWithdrawWindow(d->xinfo->display(), winId(), d->xinfo->screen());

        QTLWExtra *top = d->topData();
        data->crect.moveTopLeft(QPoint(data->crect.x() - top->fleft, data->crect.y() - top->ftop));

        // zero the frame strut and mark it dirty
        top->fleft = top->fright = top->ftop = top->fbottom = 0;
        data->fstrut_dirty = true;

        XFlush(d->xinfo->display());
    } else {
        setAttribute(Qt::WA_Mapped, false);
        if (winId()) // in nsplugin, may be 0
            XUnmapWindow(d->xinfo->display(), winId());
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
    XRaiseWindow(d->xinfo->display(), winId());
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
    XLowerWindow(d->xinfo->display(), winId());
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
    XRestackWindows(d->xinfo->display(), stack, 2);
}



/*
  The global variable qt_widget_tlw_gravity defines the window gravity of
  the next top level window to be created. We do this when setting the
  main widget's geometry and the "-geometry" command line option contains
  a negative position.
*/

int qt_widget_tlw_gravity = -1;

static void do_size_hints(QWidget* widget, QWExtra *x)
{
    if (qt_widget_tlw_gravity == -1)
        qt_widget_tlw_gravity = QApplication::reverseLayout() ? NorthEastGravity: NorthWestGravity;
    XSizeHints s;
    s.flags = 0;
    if (x) {
        s.x = widget->x();
        s.y = widget->y();
        s.width = widget->width();
        s.height = widget->height();
        if (x->minw > 0 || x->minh > 0) {        // add minimum size hints
            s.flags |= PMinSize;
            s.min_width  = qMin(XCOORD_MAX, x->minw);
            s.min_height = qMin(XCOORD_MAX, x->minh);
        }
        s.flags |= PMaxSize;                // add maximum size hints
        s.max_width  = qMin(XCOORD_MAX, x->maxw);
        s.max_height = qMin(XCOORD_MAX, x->maxh);
        if (x->topextra &&
           (x->topextra->incw > 0 || x->topextra->inch > 0))
        {                                        // add resize increment hints
            s.flags |= PResizeInc | PBaseSize;
            s.width_inc = x->topextra->incw;
            s.height_inc = x->topextra->inch;
            s.base_width = x->topextra->basew;
            s.base_height = x->topextra->baseh;
        }

        if (x->topextra && x->topextra->uspos) {
            s.flags |= USPosition;
            s.flags |= PPosition;
        }
        if (x->topextra && x->topextra->ussize) {
            s.flags |= USSize;
            s.flags |= PSize;
        }
    }
    s.flags |= PWinGravity;
    s.win_gravity = qt_widget_tlw_gravity;        // usually NorthWest
    // reset in case it was set
    qt_widget_tlw_gravity = QApplication::reverseLayout() ? NorthEastGravity: NorthWestGravity;
    XSetWMNormalHints(widget->x11Info()->display(), widget->winId(), &s);
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
    Display *dpy = xinfo->display();
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
            wrect.moveBy(-data.crect.topLeft());
        }
        //translate from parent's Qt coords to parent's X coords
        xrect.moveBy(-parentWRect.topLeft());

    } else {
        // parent is not clipped, we may or may not have to clip

        if (data.wrect.isValid()) {
            // This is where the main optimization is: we are already
            // clipped, and if our clip is still valid, we can just
            // move our window, and do not need to move or clip
            // children

            QRect vrect = xrect & q->parentWidget()->rect();
            vrect.moveBy(-data.crect.topLeft()); //the part of me that's visible through parent, in my Qt coords
            if (data.wrect.contains(vrect)) {
                xrect = data.wrect;
                xrect.moveBy(data.crect.topLeft());
                XMoveResizeWindow(dpy, data.winid, xrect.x(), xrect.y(), xrect.width(), xrect.height());
                return;
            }
        }

        if (!validRange.contains(xrect)) {
            // we are too big, and must clip
            xrect &=wrectRange;
            wrect = xrect;
            wrect.moveBy(-data.crect.topLeft());
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

void QWidget::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Display *dpy = d->xinfo->display();

    if (testWFlags(Qt::WType_Desktop))
        return;
    clearWState(Qt::WState_Maximized);
    clearWState(Qt::WState_FullScreen);
    if (d->extra) {                                // any size restrictions?
        w = qMin(w,d->extra->maxw);
        h = qMin(h,d->extra->maxh);
        w = qMax(w,d->extra->minw);
        h = qMax(h,d->extra->minh);
    }
    if (isTopLevel()) {
        d->topData()->normalGeometry = QRect(0,0,-1,-1);
        w = qMax(1, w);
        h = qMax(1, h);
    }
    QPoint oldPos(pos());
    QSize oldSize(size());
    QRect oldGeom(data->crect);
    QRect  r(x, y, w, h);

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if (!isTopLevel() && oldGeom == r)
        return;

    data->crect = r;
    bool isResize = size() != oldSize;

    if (isTopLevel()) {
        if (isMove)
            d->topData()->uspos = 1;
        if (isResize)
            d->topData()->ussize = 1;
        do_size_hints(this, d->extra);
        if (isMove) {
            if (! qt_broken_wm)
                // pos() is right according to ICCCM 4.1.5
                XMoveResizeWindow(dpy, data->winid, pos().x(), pos().y(), w, h);
            else
                // work around 4Dwm's incompliance with ICCCM 4.1.5
                XMoveResizeWindow(dpy, data->winid, x, y, w, h);
        } else if (isResize)
            XResizeWindow(dpy, data->winid, w, h);
    } else {
        d->setWSGeometry();
    }

    if (isVisible()) {
        if (isMove && pos() != oldPos) {
            if (! qt_broken_wm) {
                // pos() is right according to ICCCM 4.1.5
                QMoveEvent e(pos(), oldPos);
                QApplication::sendEvent(this, &e);
            } else {
                // work around 4Dwm's incompliance with ICCCM 4.1.5
                QMoveEvent e(data->crect.topLeft(), oldGeom.topLeft());
                QApplication::sendEvent(this, &e);
            }
        }
        if (isResize) {
            // set config pending only on resize, see qapplication_x11.cpp, translateConfigEvent()
            setWState(Qt::WState_ConfigPending);

            QResizeEvent e(size(), oldSize);
            QApplication::sendEvent(this, &e);

            // Process events immediately rather than in
            // translateConfigEvent to avoid message process delay.
            if (!testAttribute(Qt::WA_StaticContents))
                testWState(Qt::WState_InPaintEvent)?update():repaint();
        }
    } else {
        if (isMove && pos() != oldPos)
            setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            setAttribute(Qt::WA_PendingResizeEvent, true);
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
        resize(qMax(minw,width()), qMax(minh,height()));
        setAttribute(Qt::WA_Resized, resized); //not a user resize
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
                 objectName().local8Bit(), metaObject()->className(), QWIDGETSIZE_MAX,
                QWIDGETSIZE_MAX);
        maxw = qMin(maxw, QWIDGETSIZE_MAX);
        maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if (maxw < 0 || maxh < 0) {
        qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                objectName().local8Bit(), metaObject()->className(), maxw, maxh);
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
    bool just_update = QABS(dx) > width() || QABS(dy) > height();
    if (just_update)
        update();
    QRect sr = valid_rect?r:clipRegion().boundingRect();
    int x1, y1, x2, y2, w=sr.width(), h=sr.height();
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

    Display *dpy = d->xinfo->display();
    GC gc = qt_xget_readonly_gc(d->xinfo->screen(), false);
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
            repaint(x, sr.y(), QABS(dx), sr.height());
        else
            XClearArea(dpy, data->winid, x, sr.y(), QABS(dx), sr.height(), True);
    }
    if (dy) {
        int y = y2 == sr.y() ? sr.y()+h : sr.y();
        if (repaint_immediately)
            repaint(sr.x(), y, sr.width(), QABS(dy));
        else
            XClearArea(dpy, data->winid, sr.x(), y, sr.width(), QABS(dy), True);
    }

    qt_insert_sip(this, dx, dy); // #### ignores r
}

/*!
    Internal implementation of the virtual QPaintDevice::metric()
    function.

    Use the QPaintDeviceMetrics class instead.

    \a m is the metric to get.
*/

int QWidget::metric(int m) const
{
    int val;
    if (m == QPaintDeviceMetrics::PdmWidth) {
        val = data->crect.width();
    } else if (m == QPaintDeviceMetrics::PdmHeight) {
        val = data->crect.height();
    } else {
        Display *dpy = d->xinfo->display();
        int scr = d->xinfo->screen();
        switch (m) {
            case QPaintDeviceMetrics::PdmDpiX:
            case QPaintDeviceMetrics::PdmPhysicalDpiX:
                val = QX11Info::appDpiX(scr);
                break;
            case QPaintDeviceMetrics::PdmDpiY:
            case QPaintDeviceMetrics::PdmPhysicalDpiY:
                val = QX11Info::appDpiY(scr);
                break;
            case QPaintDeviceMetrics::PdmWidthMM:
                val = (DisplayWidthMM(dpy,scr)*data->crect.width())/
                      DisplayWidth(dpy,scr);
                break;
            case QPaintDeviceMetrics::PdmHeightMM:
                val = (DisplayHeightMM(dpy,scr)*data->crect.height())/
                      DisplayHeight(dpy,scr);
                break;
            case QPaintDeviceMetrics::PdmNumColors:
                val = d->xinfo->cells();
                break;
            case QPaintDeviceMetrics::PdmDepth:
                val = d->xinfo->depth();
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
    // created lazily
    extra->topextra->xic = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    destroyInputContext();
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
        if (qt_dnd_enable(this, on)) {
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

    XShapeCombineRegion(d->xinfo->display(), winId(), ShapeBounding, 0, 0,
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
    if (bm.x11Info()->screen() != d->xinfo->screen())
        bm.x11SetScreen(d->xinfo->screen());
    XShapeCombineMask(d->xinfo->display(), winId(), ShapeBounding, 0, 0,
                       bm.handle(), ShapeSet);
}

/*!
    Removes any mask set by setMask().

    \sa setMask()
*/

void QWidget::clearMask()
{
    XShapeCombineMask(d->xinfo->display(), winId(), ShapeBounding, 0, 0,
                       XNone, ShapeSet);
}

/*!
  \internal

  Computes the frame rectangle when needed.  This is an internal function, you
  should never call this.
*/

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this;

    if (! isVisible() || isDesktop()) {
        data->fstrut_dirty = (! isVisible());
        return;
    }

    Atom type_ret;
    Window l = winId(), w = winId(), p, r; // target window, it's parent, root
    Window *c;
    int i_unused;
    unsigned int nc;
    unsigned char *data_ret;
    unsigned long l_unused;

    while (XQueryTree(QX11Info::appDisplay(), w, &r, &p, &c, &nc)) {
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
            (XGetWindowProperty(QX11Info::appDisplay(), p,
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
    if (XTranslateCoordinates(QX11Info::appDisplay(), l, w,
                              0, 0, &transx, &transy, &p) &&
        XGetWindowAttributes(QX11Info::appDisplay(), w, &wattr)) {
        QTLWExtra *top = that->d->topData();
        top->fleft = transx;
        top->ftop = transy;
        top->fright = wattr.width - data->crect.width() - top->fleft;
        top->fbottom = wattr.height - data->crect.height() - top->ftop;

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

    data->fstrut_dirty = 0;
}


void QWidgetPrivate::createInputContext()
{
    QWidget *tlw = q->topLevelWidget();
    QTLWExtra *topdata = tlw->d->topData();

#ifndef QT_NO_XIM
    if (X11->xim) {
        if (! topdata->xic) {
            QInputContext *qic = new QInputContext(tlw);
            topdata->xic = (void *) qic;
        }
    } else
#endif // QT_NO_XIM
        {
            // qDebug("QWidget::createInputContext: no xim");
            topdata->xic = 0;
        }
}


void QWidgetPrivate::destroyInputContext()
{
#ifndef QT_NO_XIM
    QInputContext *qic = (QInputContext *) d->extra->topextra->xic;
    delete qic;
#endif // QT_NO_XIM
    d->extra->topextra->xic = 0;
}


/*!
    This function is called when the user finishes input composition,
    e.g. changes focus to another widget, moves the cursor, etc.
*/
void QWidget::resetInputContext()
{
#ifndef QT_NO_XIM
    if (X11->xim_style & XIMPreeditCallbacks) {
        QWidget *tlw = topLevelWidget();
        QTLWExtra *topdata = tlw->d->topData();

        // trigger input context creation if it hasn't happened already
        d->createInputContext();

        if (topdata->xic) {
            QInputContext *qic = (QInputContext *) topdata->xic;
            qic->reset();
        }
    }
#endif // QT_NO_XIM
}


void QWidgetPrivate::focusInputContext()
{
#ifndef QT_NO_XIM
    QWidget *tlw = q->topLevelWidget();
    QTLWExtra *topdata = tlw->d->topData();

    // trigger input context creation if it hasn't happened already
    createInputContext();

    if (topdata->xic) {
        QInputContext *qic = (QInputContext *) topdata->xic;
        qic->setFocus();
    }
#endif // QT_NO_XIM
}

void QWidget::setWindowOpacity(double)
{
}

double QWidget::windowOpacity() const
{
    return 1.0;
}

/*!
    \internal
*/
QX11Info *QWidget::x11Info() const
{
    return d->xinfo;
}

void QWidgetPrivate::setWindowRole(const char *role)
{
    XChangeProperty(q->d->xinfo->display(), q->winId(),
                    ATOM(WM_WINDOW_ROLE), XA_STRING, 8, PropModeReplace,
                    (unsigned char *)role, qstrlen(role));
}

static QSingleCleanupHandler<QX11PaintEngine> qt_paintengine_cleanup_handler;
static QX11PaintEngine *qt_widget_paintengine = 0;

QPaintEngine *QWidget::paintEngine() const
{
    if (!qt_widget_paintengine) {
        qt_widget_paintengine = new QX11PaintEngine(const_cast<QWidget *>(this));
        qt_paintengine_cleanup_handler.set(&qt_widget_paintengine);
    }
    return qt_widget_paintengine;
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

