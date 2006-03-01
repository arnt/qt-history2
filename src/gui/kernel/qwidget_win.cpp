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
#include "qbitmap.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qimage.h"
#include "qlayout.h"
#include "qlibrary.h"
#include "qpainter.h"
#include "qstack.h"
#include "qt_windows.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "private/qbackingstore_p.h"

#include <qdebug.h>

#include <private/qapplication_p.h>
#include <private/qwininputcontext_p.h>
#include <private/qpaintengine_raster_p.h>

typedef BOOL    (WINAPI *PtrSetLayeredWindowAttributes)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
static PtrSetLayeredWindowAttributes ptrSetLayeredWindowAttributes = 0;
#define Q_WS_EX_LAYERED           0x00080000 // copied from WS_EX_LAYERED in winuser.h
#define Q_LWA_ALPHA               0x00000002 // copied from LWA_ALPHA in winuser.h

#ifdef Q_OS_TEMP
#include "sip.h"
#endif

#if defined(QT_NON_COMMERCIAL)
#include "qnc_win.h"
#endif

#if !defined(WS_EX_TOOLWINDOW)
#define WS_EX_TOOLWINDOW 0x00000080
#endif

#if !defined(GWLP_WNDPROC)
#define GWLP_WNDPROC GWL_WNDPROC
#endif

//#define TABLET_DEBUG
#define PACKETDATA  (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE \
                     | PK_ORIENTATION | PK_CURSOR | PK_Z)
#define PACKETMODE  0
#include <wintab.h>
#include <pktdef.h>

typedef HCTX        (API *PtrWTOpen)(HWND, LPLOGCONTEXT, BOOL);
typedef BOOL        (API *PtrWTClose)(HCTX);
typedef UINT        (API *PtrWTInfo)(UINT, UINT, LPVOID);
typedef BOOL        (API *PtrWTEnable)(HCTX, BOOL);
typedef BOOL        (API *PtrWTOverlap)(HCTX, BOOL);
typedef int        (API *PtrWTPacketsGet)(HCTX, int, LPVOID);
typedef BOOL        (API *PtrWTGet)(HCTX, LPLOGCONTEXT);
typedef int     (API *PtrWTQueueSizeGet)(HCTX);
typedef BOOL    (API *PtrWTQueueSizeSet)(HCTX, int);

static PtrWTOpen ptrWTOpen = 0;
static PtrWTClose ptrWTClose = 0;
static PtrWTInfo ptrWTInfo = 0;
static PtrWTQueueSizeGet ptrWTQueueSizeGet = 0;
static PtrWTQueueSizeSet ptrWTQueueSizeSet = 0;
static void init_wintab_functions();
static void qt_tablet_init();
static void qt_tablet_cleanup();
extern HCTX qt_tablet_context;
extern bool qt_tablet_tilt_support;

static QWidget *qt_tablet_widget = 0;

extern bool qt_is_gui_used;
static void init_wintab_functions()
{
    if (!qt_is_gui_used)
        return;
    QLibrary library("wintab32");
    QT_WA({
        ptrWTOpen = (PtrWTOpen)library.resolve("WTOpenW");
        ptrWTInfo = (PtrWTInfo)library.resolve("WTInfoW");
    } , {
        ptrWTOpen = (PtrWTOpen)library.resolve("WTOpenA");
        ptrWTInfo = (PtrWTInfo)library.resolve("WTInfoA");
    });

    ptrWTClose = (PtrWTClose)library.resolve("WTClose");
    ptrWTQueueSizeGet = (PtrWTQueueSizeGet)library.resolve("WTQueueSizeGet");
    ptrWTQueueSizeSet = (PtrWTQueueSizeSet)library.resolve("WTQueueSizeSet");
}

static void qt_tablet_init()
{
    static bool firstTime = true;
    if (!firstTime)
        return;
    firstTime = false;
    qt_tablet_widget = new QWidget(0);
    qt_tablet_widget->setObjectName(QLatin1String("Qt internal tablet widget"));
    LOGCONTEXT lcMine;
    qAddPostRoutine(qt_tablet_cleanup);
    struct tagAXIS tpOri[3];
    init_wintab_functions();
    if (ptrWTInfo && ptrWTOpen && ptrWTQueueSizeGet && ptrWTQueueSizeSet) {
        // make sure we have WinTab
        if (!ptrWTInfo(0, 0, NULL)) {
#ifdef TABLET_DEBUG
            qWarning("QWidget: Wintab services not available");
#endif
            return;
        }

        // some tablets don't support tilt, check if it is possible,
        qt_tablet_tilt_support = ptrWTInfo(WTI_DEVICES, DVC_ORIENTATION, &tpOri);
        if (qt_tablet_tilt_support) {
            // check for azimuth and altitude
            qt_tablet_tilt_support = tpOri[0].axResolution && tpOri[1].axResolution;
        }
        // build our context from the default context
        ptrWTInfo(WTI_DEFSYSCTX, 0, &lcMine);
        // Go for the raw coordinates, the tablet event will return good stuff
        lcMine.lcOptions |= CXO_MESSAGES | CXO_CSRMESSAGES;
        lcMine.lcPktData = PACKETDATA;
        lcMine.lcPktMode = PACKETMODE;
        lcMine.lcMoveMask = PACKETDATA;
        lcMine.lcOutOrgX = 0;
        lcMine.lcOutExtX = lcMine.lcInExtX;
        lcMine.lcOutOrgY = 0;
        lcMine.lcOutExtY = -lcMine.lcInExtY;
        qt_tablet_context = ptrWTOpen(qt_tablet_widget->winId(), &lcMine, true);
#ifdef TABLET_DEBUG
        qDebug("Tablet is %p", qt_tablet_context);
#endif
        if (!qt_tablet_context) {
#ifdef TABLET_DEBUG
            qWarning("QWidget: Failed to open the tablet");
#endif
            return;
        }
        // Set the size of the Packet Queue to the correct size...
        int currSize = ptrWTQueueSizeGet(qt_tablet_context);
        if (!ptrWTQueueSizeSet(qt_tablet_context, QT_TABLET_NPACKETQSIZE)) {
            // Ideally one might want to use a smaller
            // multiple, but for now, since we managed to destroy
            // the existing Q with the previous call, set it back
            // to the other size, which should work.  If not,
            // there will be trouble.
            if (!ptrWTQueueSizeSet(qt_tablet_context, currSize)) {
                Q_ASSERT_X(0, "Qt::Internal", "There is no packet queue for"
                         " the tablet. The tablet will not work");
            }
        }
    }
}

static void qt_tablet_cleanup()
{
    if (ptrWTClose)
        ptrWTClose(qt_tablet_context);
    delete qt_tablet_widget;
    qt_tablet_widget = 0;
}

const QString qt_reg_winclass(Qt::WFlags flags);                // defined in qapplication_win.cpp
void            qt_olednd_unregister(QWidget* widget, QOleDropTarget *dst); // dnd_win
QOleDropTarget* qt_olednd_register(QWidget* widget);

extern bool qt_nograb();
extern HRGN qt_win_bitmapToRegion(const QBitmap& bitmap);

static QWidget *mouseGrb    = 0;
static QCursor *mouseGrbCur = 0;
static QWidget *keyboardGrb = 0;
static HHOOK   journalRec  = 0;

extern "C" LRESULT CALLBACK QtWndProc(HWND, UINT, WPARAM, LPARAM);

#define XCOORD_MAX 32767
#define WRECT_MAX 16383

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_Q(QWidget);
    static int sw = -1, sh = -1;

    Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;

    bool topLevel = (flags & Qt::Window);
    bool popup = (type == Qt::Popup);
    bool dialog = (type == Qt::Dialog
                   || type == Qt::Sheet
                   || (flags & Qt::MSWindowsFixedSizeDialogHint));
    bool desktop = (type == Qt::Desktop);
    bool tool = (type == Qt::Tool || type == Qt::Drawer);

    bool customize =  (flags & (
                                Qt::X11BypassWindowManagerHint
                                | Qt::FramelessWindowHint
                                | Qt::WindowTitleHint
                                | Qt::WindowSystemMenuHint
                                | Qt::WindowMinimizeButtonHint
                                | Qt::WindowMaximizeButtonHint
                                | Qt::WindowContextHelpButtonHint
                                ));
    HINSTANCE appinst  = qWinAppInst();
    HWND parentw, destroyw = 0;
    WId id;

    QString windowClassName = qt_reg_winclass(q->windowFlags());

    if (!window)                                // always initialize
        initializeWindow = true;

    if (popup)
        flags |= Qt::WindowStaysOnTopHint; // a popup stays on top

    if (sw < 0) {                                // get the (primary) screen size
        sw = GetSystemMetrics(SM_CXSCREEN);
        sh = GetSystemMetrics(SM_CYSCREEN);
    }

    if (desktop) {                                // desktop widget
        popup = false;                                // force this flags off
#ifndef Q_OS_TEMP
        if (QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95)
            data.crect.setRect(GetSystemMetrics(76 /* SM_XVIRTUALSCREEN  */), GetSystemMetrics(77 /* SM_YVIRTUALSCREEN  */),
                           GetSystemMetrics(78 /* SM_CXVIRTUALSCREEN */), GetSystemMetrics(79 /* SM_CYVIRTUALSCREEN */));
        else
#endif
            data.crect.setRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }

    parentw = q->parentWidget() ? q->parentWidget()->winId() : 0;

#ifdef UNICODE
    QString title;
    const TCHAR *ttitle = 0;
#endif
    QByteArray title95;
    int style = WS_CHILD;
    int exsty = WS_EX_NOPARENTNOTIFY;

    if (window) {
        style = GetWindowLongA(window, GWL_STYLE);
        if (!style)
            qErrnoWarning("QWidget::create: GetWindowLong failed");
        topLevel = false; // #### needed for some IE plugins??
    } else if (popup || (type == Qt::ToolTip) || (type == Qt::SplashScreen)) {
        style = WS_POPUP;
    } else if (!topLevel) {
        if (!customize)
            flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;
    } else if (!desktop) {
        if (customize) {
            if (flags & Qt::FramelessWindowHint)
                style = WS_POPUP;                // no border
            else
                style = 0;

            // All these buttons depend on the system menu, so we enable it
            if (flags & (Qt::WindowMinimizeButtonHint
                         | Qt::WindowMaximizeButtonHint
                         | Qt::WindowContextHelpButtonHint))
                flags |= Qt::WindowSystemMenuHint;

        } else {
            style = WS_OVERLAPPED;
#ifndef Q_OS_TEMP
            if (type == Qt::Dialog)
                flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowContextHelpButtonHint;
            else if (type == Qt::Tool)
                flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint;
            else
                flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;
#else
            if (type == Qt::Dialog)
                flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowContextHelpButtonHint;
            else if (type == Qt::Tool)
                flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint;
            else
                flags |= Qt::WindowTitleHint;
#endif
        }
    }
    if (!desktop) {
        // if (!testAttribute(Qt::WA_PaintUnclipped))
        // ### Commented out for now as it causes some problems, but
        // this should be correct anyway, so dig some more into this
        style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
        if (topLevel) {
            if ((type == Qt::Window || dialog || tool)) {
                if (!(flags & Qt::FramelessWindowHint)) {
                    if ((type == Qt::Window || dialog) && !(flags & Qt::MSWindowsFixedSizeDialogHint)) {
                        style |= WS_THICKFRAME;
                        if(!(flags &
                            ( Qt::WindowSystemMenuHint
                            | Qt::WindowTitleHint
                            | Qt::WindowMinMaxButtonsHint
                            | Qt::WindowContextHelpButtonHint)))
                            style |= WS_POPUP;
                    } else {
                        style |= WS_POPUP | WS_DLGFRAME;
                    }
                }
                if (flags & Qt::WindowTitleHint)
                    style |= WS_CAPTION;
                if (flags & Qt::WindowSystemMenuHint)
                    style |= WS_SYSMENU;
                if (flags & Qt::WindowMinimizeButtonHint)
                    style |= WS_MINIMIZEBOX;
                if (flags & Qt::WindowMaximizeButtonHint)
                    style |= WS_MAXIMIZEBOX;
                if (tool)
                    exsty |= WS_EX_TOOLWINDOW;
                if (flags & Qt::WindowContextHelpButtonHint)
                    exsty |= WS_EX_CONTEXTHELP;
            } else {
                 exsty |= WS_EX_TOOLWINDOW;
            }
        }
    }

    if (flags & Qt::WindowTitleHint) {
        QT_WA({
            title = q->isWindow() ? qAppName() : q->objectName();
            ttitle = (TCHAR*)title.utf16();
        } , {
            title95 = q->isWindow() ? qAppName().toLocal8Bit() : q->objectName().toLatin1();
        });
    }

    // The Qt::WA_WState_Created flag is checked by translateConfigEvent() in
    // qapplication_win.cpp. We switch it off temporarily to avoid move
    // and resize events during creationt
    q->setAttribute(Qt::WA_WState_Created, false);

    if (window) {                                // override the old window
        if (destroyOldWindow)
            destroyw = data.winid;
        id = window;
        setWinId(window);
        LONG res = SetWindowLongA(window, GWL_STYLE, style);
        if (!res)
            qErrnoWarning("QWidget::create: Failed to set window style");
#ifdef _WIN64
        res = SetWindowLongPtrA( window, GWLP_WNDPROC, (LONG_PTR)QtWndProc );
#else
        res = SetWindowLongA( window, GWL_WNDPROC, (LONG)QtWndProc );
#endif
        if (!res)
            qErrnoWarning("QWidget::create: Failed to set window procedure");
    } else if (desktop) {                        // desktop widget
#ifndef Q_OS_TEMP
        id = GetDesktopWindow();
//         QWidget *otherDesktop = QWidget::find(id);        // is there another desktop?
//         if (otherDesktop && otherDesktop->testWFlags(Qt::WPaintDesktop)) {
//             otherDesktop->d_func()->setWinId(0);        // remove id from widget mapper
//             d->setWinId(id);                     // make sure otherDesktop is
//             otherDesktop->d_func()->setWinId(id);       //   found first
//         } else {
            setWinId(id);
//         }
#endif
    } else if (topLevel) {                       // create top-level widget
        if (popup)
            parentw = 0;

#ifdef Q_OS_TEMP

        const TCHAR *cname = windowClassName.utf16();

        id = CreateWindowEx(exsty, cname, ttitle, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parentw, 0, appinst, 0);
#else

        QT_WA({
            const TCHAR *cname = (TCHAR*)windowClassName.utf16();
            id = CreateWindowEx(exsty, cname, ttitle, style,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                parentw, 0, appinst, 0);
        } , {
            id = CreateWindowExA(exsty, windowClassName.toLatin1(), title95, style,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 parentw, 0, appinst, 0);
        });

#endif

        if (!id)
            qErrnoWarning("QWidget::create: Failed to create window");
        setWinId(id);
        if ((flags & Qt::WindowStaysOnTopHint) || (type == Qt::ToolTip))
            SetWindowPos(id, HWND_TOPMOST, 0, 0, 100, 100, SWP_NOACTIVATE);
    } else {                                        // create child widget
        QT_WA({
            const TCHAR *cname = (TCHAR*)windowClassName.utf16();
            id = CreateWindowEx(exsty, cname, ttitle, style, 0, 0, 100, 30,
                            parentw, NULL, appinst, NULL);
        } , {
            id = CreateWindowExA(exsty, windowClassName.toLatin1(), title95, style, 0, 0, 100, 30,
                            parentw, NULL, appinst, NULL);
        });
        if (!id)
            qErrnoWarning("QWidget::create: Failed to create window");
        SetWindowPos(id, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        setWinId(id);
    }

    if (desktop) {
        q->setAttribute(Qt::WA_WState_Visible);
    } else {
        RECT  fr, cr;
        GetWindowRect(id, &fr);                // update rects
        GetClientRect(id, &cr);
        if (cr.top == cr.bottom && cr.left == cr.right) {
            if (initializeWindow) {
                int x, y, w, h;
                if (topLevel) {
                    x = sw/4;
                    y = 3*sh/10;
                    w = sw/2;
                    h = 4*sh/10;
                } else {
                    x = y = 0;
                    w = 100;
                    h = 30;
                }
                MoveWindow(q->winId(), x, y, w, h, TRUE);
            }
            GetWindowRect(id, &fr);                // update rects
            GetClientRect(id, &cr);
        }
        if (topLevel){
            // one cannot trust cr.left and cr.top, use a correction POINT instead
            POINT pt;
            pt.x = 0;
            pt.y = 0;
            ClientToScreen(id, &pt);
            data.crect = QRect(QPoint(pt.x, pt.y),
                           QPoint(pt.x+cr.right, pt.y+cr.bottom));

            QTLWExtra *top = topData();
            top->ftop = data.crect.top() - fr.top;
            top->fleft = data.crect.left() - fr.left;
            top->fbottom = fr.bottom - data.crect.bottom();
            top->fright = fr.right - data.crect.right();
            data.fstrut_dirty = false;

            createTLExtra();
        } else {
            data.crect.setCoords(cr.left, cr.top, cr.right, cr.bottom);
            // in case extra data already exists (eg. reparent()).  Set it.
        }
    }

    q->setAttribute(Qt::WA_WState_Created);                // accept move/resize events
    hd = 0;                                        // no display context

    if (window) {                                // got window from outside
        if (IsWindowVisible(window))
            q->setAttribute(Qt::WA_WState_Visible);
        else
            q->setAttribute(Qt::WA_WState_Visible, false);
    }

#if defined(QT_NON_COMMERCIAL)
    QT_NC_WIDGET_CREATE
#endif

    if (destroyw) {
        DestroyWindow(destroyw);
    }

    QWinInputContext::enable(q, q->testAttribute(Qt::WA_InputMethodEnabled) & q->isEnabled());
    if (q != qt_tablet_widget && QWidgetPrivate::mapper)
        qt_tablet_init();
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    Q_D(QWidget);
    if (QWidget *p = parentWidget())
        p->d_func()->invalidateBuffer(geometry());
    d->deactivateWidgetCleanup();
    if (testAttribute(Qt::WA_WState_Created)) {
        setAttribute(Qt::WA_WState_Created, false);
        for(int i = 0; i < d->children.size(); ++i) { // destroy all widget children
            register QObject *obj = d->children.at(i);
            if (obj->isWidgetType())
                ((QWidget*)obj)->destroy(destroySubWindows,
                                         destroySubWindows);
        }
        if (mouseGrb == this)
            releaseMouse();
        if (keyboardGrb == this)
            releaseKeyboard();
        if (testAttribute(Qt::WA_ShowModal))                // just be sure we leave modal
            QApplicationPrivate::leaveModal(this);
        else if ((windowType() == Qt::Popup))
            qApp->d_func()->closePopup(this);
        if (destroyWindow && !(windowType() == Qt::Desktop)) {
            DestroyWindow(winId());
        }
        d->setWinId(0);
    }
}

void QWidgetPrivate::reparentChildren()
{
    Q_Q(QWidget);
    QObjectList chlist = q->children();
    for(int i = 0; i < chlist.size(); ++i) { // reparent children
        QObject *obj = chlist.at(i);
        if (obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if ((w->windowType() == Qt::Popup)) {
                ;
            } else if (w->isWindow()) {
                bool showIt = w->isVisible();
                QPoint old_pos = w->pos();
                w->setParent(q, w->windowFlags());
                w->move(old_pos);
                if (showIt)
                    w->show();
            } else {
                w->d_func()->invalidateBuffer(w->rect());
                SetParent(w->winId(), q->winId());
                w->d_func()->reparentChildren();
            }
        }
    }
}

void QWidgetPrivate::setParent_sys(QWidget *parent, Qt::WFlags f)
{
    Q_Q(QWidget);

    if (q->isVisible() && q->parentWidget() && parent != q->parentWidget())
        q->parentWidget()->d_func()->invalidateBuffer(q->geometry());

    WId old_winid = data.winid;
    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    if (q->isVisible()) {
        ShowWindow(data.winid, SW_HIDE);
        SetParent(data.winid, 0);
    }

    if (q->testAttribute(Qt::WA_DropSiteRegistered))
        q->setAttribute(Qt::WA_DropSiteRegistered, false); // ole dnd unregister (we will register again below)

    if ((q->windowType() == Qt::Desktop))
        old_winid = 0;
    setWinId(0);

    QObjectPrivate::setParent_helper(parent);
    bool enable = q->isEnabled();                // remember status
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize s = q->size();
    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);

    data.window_flags = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    q->create();

    if (q->isWindow() || (!parent || parent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    reparentChildren();

    q->resize(s);
    setEnabled_helper(enable); //preserving WA_ForceDisabled
    q->setFocusPolicy(fp);
    if (extra && !extra->mask.isEmpty()) {
        QRegion r = extra->mask;
        extra->mask = QRegion();
        q->setMask(r);
    }
    if (extra && extra->topextra && !extra->topextra->caption.isEmpty())
        setWindowTitle_helper(extra->topextra->caption);
    if (old_winid)
        DestroyWindow(old_winid);

    if (q->testAttribute(Qt::WA_AcceptDrops)
        || (!q->isWindow() && q->parentWidget() && q->parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)))
        q->setAttribute(Qt::WA_DropSiteRegistered, true);


#ifdef Q_OS_TEMP
    // Show borderless toplevel windows in tasklist & NavBar
    if (!parent) {
        QString txt = windowTitle().isEmpty()?qAppName():windowTitle();
        SetWindowText(winId(), (TCHAR*)txt.utf16());
    }
#endif
    invalidateBuffer(q->rect());
}


QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!isVisible() || isMinimized())
        return mapTo(window(), pos) + window()->pos() +
        (window()->geometry().topLeft() - window()->frameGeometry().topLeft());
    POINT p;
    QPoint tmp = d->mapToWS(pos);
    p.x = tmp.x();
    p.y = tmp.y();
    ClientToScreen(winId(), &p);
    return QPoint(p.x, p.y);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    Q_D(const QWidget);
    if (!isVisible() || isMinimized())
        return mapFrom(window(), pos - window()->pos());
    POINT p;
    p.x = pos.x();
    p.y = pos.y();
    ScreenToClient(winId(), &p);
    return d->mapFromWS(QPoint(p.x, p.y));
}

void QWidgetPrivate::updateSystemBackground() {}

extern void qt_win_set_cursor(QWidget *, const QCursor &); // qapplication_win.cpp

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
    qt_win_set_cursor(this, QWidget::cursor());
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
    qt_win_set_cursor(this, cursor());
}

void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
    Q_Q(QWidget);
    QT_WA({
        SetWindowText(q->winId(), (TCHAR*)caption.utf16());
    } , {
        SetWindowTextA(q->winId(), caption.toLocal8Bit());
    });
}

/*
  Create an icon mask the way Windows wants it using CreateBitmap.
*/

HBITMAP qt_createIconMask(const QBitmap &bitmap)
{
    QImage bm = bitmap.toImage().convertToFormat(QImage::Format_Mono);
    int w = bm.width();
    int h = bm.height();
    int bpl = ((w+15)/16)*2;                        // bpl, 16 bit alignment
    uchar *bits = new uchar[bpl*h];
    bm.invertPixels();
    for (int y=0; y<h; y++)
        memcpy(bits+y*bpl, bm.scanLine(y), bpl);
    HBITMAP hbm = CreateBitmap(w, h, 1, 1, bits);
    delete [] bits;
    return hbm;
}

HICON qt_createIcon(QIcon icon, int xSize, int ySize, QPixmap **cache)
{
    HICON result = 0;
    if (!icon.isNull()) { // valid icon
        QSize size = icon.actualSize(QSize(xSize, ySize));
        QPixmap pm = icon.pixmap(size);
        if (pm.isNull())
            return 0;

        QBitmap mask = pm.mask();
        if (mask.isNull()) {
            mask = QBitmap(pm.size());
            mask.fill(Qt::color1);
        }

        HBITMAP im = qt_createIconMask(mask);
        ICONINFO ii;
        ii.fIcon    = true;
        ii.hbmMask  = im;
        ii.hbmColor = pm.toWinHBITMAP(QPixmap::PremultipliedAlpha);
        ii.xHotspot = 0;
        ii.yHotspot = 0;
        result = CreateIconIndirect(&ii);

        if (cache)
            *cache = new QPixmap(pm);;
        DeleteObject(ii.hbmColor);
        DeleteObject(im);
    }
    return result;
}

void QWidgetPrivate::setWindowIcon_sys(bool forceReset)
{
    Q_Q(QWidget);
    if (extra->topextra->iconPixmap && !forceReset)
        // already been set
        return;

    QTLWExtra* x = extra->topextra;
    if (x->winIconBig) {
        DestroyIcon(x->winIconBig);
        x->winIconBig = 0;
        DestroyIcon(x->winIconSmall);
        x->winIconSmall = 0;
    }

    x->winIconSmall = qt_createIcon(q->windowIcon(),
                                    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                    &(x->iconPixmap));
    x->winIconBig = qt_createIcon(q->windowIcon(),
                                  GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
                                  &(x->iconPixmap));
    if (!x->winIconBig)
        x->winIconBig = x->winIconSmall;

    SendMessageA(q->winId(), WM_SETICON, 0 /* ICON_SMALL */, (LPARAM)x->winIconSmall);
    SendMessageA(q->winId(), WM_SETICON, 1 /* ICON_BIG */, (LPARAM)x->winIconBig);
}


void QWidgetPrivate::setWindowIconText_sys(const QString &iconText)
{
    Q_UNUSED(iconText);
}


QCursor *qt_grab_cursor()
{
    return mouseGrbCur;
}

// The procedure does nothing, but is required for mousegrabbing to work
LRESULT CALLBACK qJournalRecordProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#ifndef Q_OS_TEMP
    return CallNextHookEx(journalRec, nCode, wParam, lParam);
#else
    return 0;
#endif
}

void QWidget::grabMouse()
{
    if (!qt_nograb()) {
        if (mouseGrb)
            mouseGrb->releaseMouse();
#ifndef Q_OS_TEMP
        journalRec = SetWindowsHookExA(WH_JOURNALRECORD, (HOOKPROC)qJournalRecordProc, GetModuleHandleA(0), 0);
#endif
        SetCapture(winId());
        mouseGrb = this;
    }
}

void QWidget::grabMouse(const QCursor &cursor)
{
    if (!qt_nograb()) {
        if (mouseGrb)
            mouseGrb->releaseMouse();
#ifndef Q_OS_TEMP
        journalRec = SetWindowsHookExA(WH_JOURNALRECORD, (HOOKPROC)qJournalRecordProc, GetModuleHandleA(0), 0);
#endif
        SetCapture(winId());
        mouseGrbCur = new QCursor(cursor);
        SetCursor(mouseGrbCur->handle());
        mouseGrb = this;
    }
}

void QWidget::releaseMouse()
{
    if (!qt_nograb() && mouseGrb == this) {
        ReleaseCapture();
        if (journalRec) {
#ifndef Q_OS_TEMP
            UnhookWindowsHookEx(journalRec);
#endif
            journalRec = 0;
        }
        if (mouseGrbCur) {
            delete mouseGrbCur;
            mouseGrbCur = 0;
        }
        mouseGrb = 0;
    }
}

void QWidget::grabKeyboard()
{
    if (!qt_nograb()) {
        if (keyboardGrb)
            keyboardGrb->releaseKeyboard();
        keyboardGrb = this;
    }
}

void QWidget::releaseKeyboard()
{
    if (!qt_nograb() && keyboardGrb == this)
        keyboardGrb = 0;
}


QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}

void QWidget::activateWindow()
{
    SetForegroundWindow(window()->winId());
}

extern UINT WM_QT_REPAINT;

void QWidgetPrivate::dirtyWidget_sys(const QRegion &rgn)
{
    Q_Q(QWidget);
    if (!rgn.isEmpty()) {
        InvalidateRgn(q->winId(), rgn.handle(), FALSE);
        // check if this is the first call to dirty a previously clean widget
        if (!q->testAttribute(Qt::WA_PendingUpdate)) {
            q->setAttribute(Qt::WA_PendingUpdate);
            QT_WA( {
                PostMessageW(q->winId(), WM_QT_REPAINT, 0, 0);
            }, {
                PostMessageA(q->winId(), WM_QT_REPAINT, 0, 0);
            } );
        }
    }
}

void QWidgetPrivate::cleanWidget_sys(const QRegion& rgn)
{
    Q_Q(QWidget);
    ValidateRgn(q->winId(),rgn.handle());
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Q_D(QWidget);
    Qt::WindowStates oldstate = windowState();
    if (oldstate == newstate)
        return;

    int max = SW_MAXIMIZE;
    int min = SW_MINIMIZE;
    int normal = SW_SHOWNOACTIVATE;
    if (newstate & Qt::WindowActive) {
        max = SW_SHOWMAXIMIZED;
        min = SW_SHOWMINIMIZED;
        normal = SW_SHOWNORMAL;
    }

    if (isWindow()) {

        // Ensure the initial size is valid, since we store it as normalGeometry below.
        if (!testAttribute(Qt::WA_Resized) && !isVisible())
            adjustSize();

        if ((oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized)) {
            if (newstate & Qt::WindowMaximized && !(oldstate & Qt::WindowFullScreen))
                d->topData()->normalGeometry = geometry();
            if (isVisible() && !(newstate & Qt::WindowMinimized)) {
                ShowWindow(winId(), (newstate & Qt::WindowMaximized) ? max : normal);
                if (!(newstate & Qt::WindowFullScreen)) {
                    QRect r = d->topData()->normalGeometry;
                    if (!(newstate & Qt::WindowMaximized) && r.width() >= 0) {
                        if (pos() != r.topLeft() || size() !=r.size()) {
                            d->topData()->normalGeometry = QRect(0,0,-1,-1);
                            setGeometry(r);
                        }
                    }
                } else {
                    d->updateFrameStrut();
                }
            }
        }

        if ((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            if (newstate & Qt::WindowFullScreen) {
                if (d->topData()->normalGeometry.width() < 0 && !(oldstate & Qt::WindowMaximized))
                    d->topData()->normalGeometry = geometry();
                d->topData()->savedFlags = GetWindowLongA(winId(), GWL_STYLE);
                UINT style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP;
                if (isVisible())
                    style |= WS_VISIBLE;
                SetWindowLongA(winId(), GWL_STYLE, style);
                QRect r = qApp->desktop()->screenGeometry(this);
                UINT swpf = SWP_FRAMECHANGED;
                if (newstate & Qt::WindowActive)
                    swpf |= SWP_NOACTIVATE;

                SetWindowPos(winId(), HWND_TOP, r.left(), r.top(), r.width(), r.height(), swpf);
                d->updateFrameStrut();
            } else {
                UINT style = d->topData()->savedFlags;
                if (isVisible())
                    style |= WS_VISIBLE;
                SetWindowLongA(winId(), GWL_STYLE, style);

                UINT swpf = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE;
                if (newstate & Qt::WindowActive)
                    swpf |= SWP_NOACTIVATE;
                SetWindowPos(winId(), 0, 0, 0, 0, 0, swpf);
                d->updateFrameStrut();

                // preserve maximized state
                if (isVisible())
                    ShowWindow(winId(), (newstate & Qt::WindowMaximized) ? max : normal);

                if (!(newstate & Qt::WindowMaximized)) {
                    QRect r = d->topData()->normalGeometry;
                    d->topData()->normalGeometry = QRect(0,0,-1,-1);
                    if (r.isValid())
                        setGeometry(r);
                }
            }
        }

        if ((oldstate & Qt::WindowMinimized) != (newstate & Qt::WindowMinimized)) {
            if (isVisible())
                ShowWindow(winId(), (newstate & Qt::WindowMinimized) ? min :
                                    (newstate & Qt::WindowMaximized) ? max : normal);
        }
    }


    data->window_state = newstate;

    QWindowStateChangeEvent e(oldstate);
    QApplication::sendEvent(this, &e);
}


/*
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidgetPrivate::hide_sys()
{
    Q_Q(QWidget);
    deactivateWidgetCleanup();
    ShowWindow(q->winId(), SW_HIDE);
    if(!q->isWindow())
        invalidateBuffer(q->rect());
    else if (!QWidgetBackingStore::paintOnScreen(q))
        extra->topextra->backingStore->releaseBuffer(); // release backing store buffer on hide
}


#ifndef Q_OS_TEMP // ------------------------------------------------

/*
  \internal
  Platform-specific part of QWidget::show().
*/
void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);
#if defined(QT_NON_COMMERCIAL)
    QT_NC_SHOW_WINDOW
#endif
    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;
    q->setAttribute(Qt::WA_Mapped);

    int sm = SW_SHOWNORMAL;
    bool fakedMaximize = false;
    if (q->isWindow()) {
        if (q->isMinimized())
            sm = SW_SHOWMINIMIZED;
        else if (q->isMaximized()) {
            sm = SW_SHOWMAXIMIZED;
            // Windows will not behave correctly when we try to maximize a window which does not
            // have minimize nor maximize buttons in the window frame. Windows would then ignore
            // non-available geometry, and rather maximize the widget to the full screen, minus the
            // window frame (caption). So, we do a trick here, by adding a maximize button before
            // maximizing the widget, and then remove the maximize button afterwards.
            Qt::WindowFlags &flags = data.window_flags;
            if (flags & Qt::WindowTitleHint &&
                !(flags & (Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint))) {
                fakedMaximize = TRUE;
                int style = GetWindowLong(q->winId(), GWL_STYLE);
                SetWindowLong(q->winId(), GWL_STYLE, style | WS_MAXIMIZEBOX);
            }
        }
    }
    if ((q->windowType() == Qt::Popup) || q->windowType() == Qt::ToolTip)
        sm = SW_SHOWNOACTIVATE;

    ShowWindow(q->winId(), sm);

    if (fakedMaximize) {
        int style = GetWindowLong(q->winId(), GWL_STYLE);
        SetWindowLong(q->winId(), GWL_STYLE, style & ~WS_MAXIMIZEBOX);
        SetWindowPos(q->winId(), 0, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER
                     | SWP_FRAMECHANGED);
    }

    if (IsIconic(q->winId()))
        data.window_state |= Qt::WindowMinimized;
    if (IsZoomed(q->winId()))
        data.window_state |= Qt::WindowMaximized;

    UpdateWindow(q->winId());
    invalidateBuffer(q->rect());
}

#else // Q_OS_TEMP --------------------------------------------------
# if defined(WIN32_PLATFORM_PSPC) && (WIN32_PLATFORM_PSPC < 310)
#  define SHFS_SHOWTASKBAR            0x0001
#  define SHFS_SHOWSIPBUTTON          0x0004
   extern "C" BOOL __stdcall SHFullScreen(HWND hwndRequester, DWORD dwState);
# else
#  include <aygshell.h>
# endif

void QWidget::show_sys()
{
    Q_D(QWidget);
    if (testAttribute(Qt::WA_OutsideWSRange))
        return;
    setAttribute(Qt::WA_Mapped);
    uint sm = SW_SHOW;
    if (isWindow()) {
        switch (d->topData()->showMode) {
        case 1:
            sm = SW_HIDE;
            break;
        case 2:
            {
                int scrnum = qApp->desktop()->screenNumber(this);
                setGeometry(qApp->desktop()->availableGeometry(scrnum));
            }
            // Fall-through
        default:
            sm = SW_SHOW;
            break;
        }
        d->topData()->showMode = 0; // reset
    }

    if ((windowType() == Qt::Tool) || (windowType() == Qt::Popup) || windowType() == Qt::ToolTip)
        sm = SW_SHOWNOACTIVATE;

    ShowWindow(winId(), sm);
    if (isWindow() && sm == SW_SHOW)
        SetForegroundWindow(winId());
    UpdateWindow(winId());
    if(!q->isWindow())
        invalidateBuffer(q->rect());
}

#endif // Q_OS_TEMP -------------------------------------------------

void QWidgetPrivate::raise_sys()
{
    Q_Q(QWidget);
    SetWindowPos(q->winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    if(!q->isWindow())
        invalidateBuffer(q->rect());
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    SetWindowPos(q->winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    invalidateBuffer(q->rect());
}

void QWidgetPrivate::stackUnder_sys(QWidget* w)
{
    Q_Q(QWidget);
    SetWindowPos(q->winId(), w->winId() , 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    invalidateBuffer(q->rect());
}


/*
  Helper function for non-toplevel widgets. Helps to map Qt's 32bit
  coordinate system to Windpws's 16bit coordinate system.

  This code is duplicated from the X11 code, so any changes there
  should also (most likely) be reflected here.

  (In all comments below: s/X/Windows/g)
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
                MoveWindow(q->winId(), xrect.x(), xrect.y(), xrect.width(), xrect.height(), true);
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
            ShowWindow(q->winId(), SW_HIDE);
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
    for (int i = 0; i < children.size(); ++i) {
        QObject *object = children.at(i);
        if (object->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(object);
            if (!w->isWindow())
                w->d_func()->setWSGeometry();
        }
    }

    // move ourselves to the new position and map (if necessary) after
    // the movement. Rationale: moving unmapped windows is much faster
    // than moving mapped windows
    MoveWindow(q->winId(), xrect.x(), xrect.y(), xrect.width(), xrect.height(), !jump);
    if (mapWindow && !dontShow) {
        q->setAttribute(Qt::WA_Mapped);
        ShowWindow(q->winId(), SW_SHOWNOACTIVATE);
    }

    if (jump)
        InvalidateRect(q->winId(), 0, false);

}

//
// The internal qWinRequestConfig, defined in qapplication_win.cpp, stores move,
// resize and setGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig(WId, int, int, int, int, int);

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    if (extra) {                                // any size restrictions?
        w = qMin(w,extra->maxw);
        h = qMin(h,extra->maxh);
        w = qMax(w,extra->minw);
        h = qMax(h,extra->minh);
    }
    if (q->isWindow()) {
        topData()->normalGeometry = QRect(0, 0, -1, -1);
        w = qMax(1, w);
        h = qMax(1, h);
    }

    QSize  oldSize(q->size());
    QPoint oldPos(q->pos());

    if (!q->isWindow())
        isMove = (data.crect.topLeft() != QPoint(x, y));
    bool isResize = w != oldSize.width() || h != oldSize.height();

    if (!isMove && !isResize)
        return;

    if (isResize && !q->testAttribute(Qt::WA_StaticContents))
        ValidateRgn(q->winId(), 0);

    if (isResize)
        data.window_state &= ~Qt::WindowMaximized;

    if (data.window_state & Qt::WindowFullScreen) {
        QTLWExtra *top = topData();

        if (q->isWindow()) {
            // We need to update these flags when we remove the full screen state
            // or the frame will not be updated
            UINT style = top->savedFlags;
            if (q->isVisible())
                style |= WS_VISIBLE;
            SetWindowLongA(q->winId(), GWL_STYLE, style);

            UINT swpf = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE;
            if (data.window_state & Qt::WindowActive)
                swpf |= SWP_NOACTIVATE;
            SetWindowPos(q->winId(), 0, 0, 0, 0, 0, swpf);

            // We also need to update the frame strut, but since updateFrameStrut
            // just returns when the window is hidden we need to do this explicitly.
            // More pasted code, ick.
            RECT  fr, cr;
            GetWindowRect(q->winId(), &fr);
            GetClientRect(q->winId(), &cr);

            POINT pt;
            pt.x = 0;
            pt.y = 0;

            ClientToScreen(q->winId(), &pt);
            q->data->crect = QRect(QPoint(pt.x, pt.y),
                                   QPoint(pt.x + cr.right, pt.y + cr.bottom));

            top->ftop = data.crect.top() - fr.top;
            top->fleft = data.crect.left() - fr.left;
            top->fbottom = fr.bottom - data.crect.bottom();
            top->fright = fr.right - data.crect.right();
        }
        data.window_state &= ~Qt::WindowFullScreen;
        topData()->savedFlags = 0;
    }

    if (q->testAttribute(Qt::WA_WState_ConfigPending)) {        // processing config event
        qWinRequestConfig(q->winId(), isMove ? 2 : 1, x, y, w, h);
    } else {
        q->setAttribute(Qt::WA_WState_ConfigPending);
        if (q->windowType() == Qt::Desktop) {
            data.crect.setRect(x, y, w, h);
        } else if (q->isWindow()) {
            QRect fr(q->frameGeometry());
            if (extra) {
                fr.setLeft(fr.left() + x - data.crect.left());
                fr.setTop(fr.top() + y - data.crect.top());
                fr.setRight(fr.right() + (x + w - 1) - data.crect.right());
                fr.setBottom(fr.bottom() + (y + h - 1) - data.crect.bottom());
            }
            MoveWindow(q->winId(), fr.x(), fr.y(), fr.width(), fr.height(), true);
            if (!q->isVisible())
                InvalidateRect(q->winId(), 0, FALSE);
            RECT rect;
            GetClientRect(q->winId(), &rect);
    	    data.crect.setRect(x, y, rect.right - rect.left, rect.bottom - rect.top);
        } else {
            QRect oldGeom(data.crect);
            data.crect.setRect(x, y, w, h);
            if (q->isVisible()) {
                if (!isResize) {
                    moveRect(QRect(oldPos, oldSize), x - oldGeom.x(), y - oldGeom.y());
                } else {
                    invalidateBuffer(q->rect());
                    QRegion oldRegion(QRect(oldPos, oldSize));
                    if (!q->mask().isEmpty())
                        oldRegion &= q->mask().translated(oldPos);
                    q->parentWidget()->d_func()->invalidateBuffer(oldRegion);
                }
            }
            setWSGeometry();
        }
        q->setAttribute(Qt::WA_WState_ConfigPending, false);
    }

    if (q->isWindow() && q->isVisible() && isResize) {
        invalidateBuffer(q->rect()); //after the resize
    }

    // Process events immediately rather than in translateConfigEvent to
    // avoid windows message process delay.
    if (q->isVisible()) {
        if (isMove && q->pos() != oldPos) {
            QMoveEvent e(q->pos(), oldPos);
            QApplication::sendEvent(q, &e);
        }
        if (isResize) {
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
}

void QWidget::scroll(int dx, int dy)
{
    if (!updatesEnabled() && children().size() == 0)
        return;
    if (dx == 0 && dy == 0)
        return;

    Q_D(QWidget);
    d->scrollChildren(dx, dy);

    if (!QWidgetBackingStore::paintOnScreen(this)) {
        d->scrollRect(rect(), dx, dy);
    } else {
        UINT flags = SW_INVALIDATE;
        if (!testAttribute(Qt::WA_OpaquePaintEvent))
            flags |= SW_ERASE;
        ScrollWindowEx(winId(), dx, dy, 0, 0, 0, 0, flags);
        d->scrollRect(rect(), dx, dy);
        UpdateWindow(winId());
    }
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    if (!updatesEnabled())
        return;
    if (dx == 0 && dy == 0)
        return;

    Q_D(QWidget);

    if (!QWidgetBackingStore::paintOnScreen(this)) {
        d->scrollRect(rect(), dx, dy);
    } else {
        RECT wr;
        wr.top = r.top();
        wr.left = r.left();
        wr.bottom = r.bottom()+1;
        wr.right = r.right()+1;

        UINT flags = SW_INVALIDATE;
        if (!testAttribute(Qt::WA_OpaquePaintEvent))
            flags |= SW_ERASE;
        ScrollWindowEx(winId(), dx, dy, &wr, &wr, 0, 0, flags);
        d->scrollRect(rect(), dx, dy);
        UpdateWindow(winId());
    }
}

extern Q_GUI_EXPORT HDC qt_win_display_dc();

int QWidget::metric(PaintDeviceMetric m) const
{
    Q_D(const QWidget);
    int val;
    if (m == PdmWidth) {
        val = data->crect.width();
    } else if (m == PdmHeight) {
        val = data->crect.height();
    } else {
        HDC gdc = qt_win_display_dc();
        switch (m) {
        case PdmDpiX:
        case PdmPhysicalDpiX:
            val = GetDeviceCaps(gdc, LOGPIXELSX);
            break;
        case PdmDpiY:
        case PdmPhysicalDpiY:
            val = GetDeviceCaps(gdc, LOGPIXELSY);
            break;
        case PdmWidthMM:
            val = data->crect.width()
                    * GetDeviceCaps(gdc, HORZSIZE)
                    / GetDeviceCaps(gdc, HORZRES);
            break;
        case PdmHeightMM:
            val = data->crect.height()
                    * GetDeviceCaps(gdc, VERTSIZE)
                    / GetDeviceCaps(gdc, VERTRES);
            break;
        case PdmNumColors:
            if (GetDeviceCaps(gdc, RASTERCAPS) & RC_PALETTE)
                val = GetDeviceCaps(gdc, SIZEPALETTE);
            else {
                int bpp = GetDeviceCaps((HDC)d->hd, BITSPIXEL);
                if(bpp==32)
                    val = INT_MAX;
                else if(bpp<=8)
                    val = GetDeviceCaps((HDC)d->hd, NUMCOLORS);
                else
                    val = 1 << (bpp * GetDeviceCaps((HDC)d->hd, PLANES));
            }
            break;
        case PdmDepth:
            val = GetDeviceCaps(gdc, BITSPIXEL);
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
    extra->dropTarget = 0;
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->winIconSmall = 0;
    extra->topextra->winIconBig = 0;
    extra->topextra->backingStore = new QWidgetBackingStore(q_func());
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if (extra->topextra->winIconSmall)
        DestroyIcon(extra->topextra->winIconSmall);
    if (extra->topextra->winIconBig)
        DestroyIcon(extra->topextra->winIconBig);
    delete extra->topextra->backingStore;
}

void QWidgetPrivate::registerDropSite(bool on)
{
    Q_Q(QWidget);
    // Enablement is defined by d->extra->dropTarget != 0.
    if (on) {
        // Turn on.
        createExtra();
        QWExtra *extra = extraData();
        if (!extra->dropTarget)
            extra->dropTarget = qt_olednd_register(q);
    } else {
        // Turn off.
        QWExtra *extra = extraData();
        if (extra && extra->dropTarget) {
            qt_olednd_unregister(q, extra->dropTarget);
            extra->dropTarget = 0;
        }
    }
}

void QWidget::setMask(const QRegion &region)
{
    Q_D(QWidget);
    d->createExtra();
    if (region == d->extra->mask)
        return;
#ifndef QT_NO_BACKINGSTORE
    QRegion parentR;
    if (!isWindow())
        parentR = d->extra->mask.isEmpty() ? QRegion(rect()) : d->extra->mask ;
#endif

    d->extra->mask = region;

    // Since SetWindowRegion takes ownership, and we need to translate,
    // we take a copy.
    HRGN wr = CreateRectRgn(0,0,0,0);
    CombineRgn(wr, region.handle(), 0, RGN_COPY);

    int fleft = 0, ftop = 0;
    if (isWindow()) {
        ftop = d->topData()->ftop;
        fleft = d->topData()->fleft;
    }
    OffsetRgn(wr, fleft, ftop);
#ifndef QT_NO_BACKINGSTORE
    if (isVisible()) {
        if (!isWindow()) {
            parentR += d->extra->mask;
            parentWidget()->update(parentR.translated(geometry().topLeft()));
        }
        if (!testAttribute(Qt::WA_PaintOnScreen))
            update();
    }
#endif
    SetWindowRgn(winId(), wr, true);
}

void QWidget::setMask(const QBitmap &bitmap)
{
    QRegion region(bitmap);
    setMask(region);
}

void QWidget::clearMask()
{
    Q_D(QWidget);
    d->createExtra();
    if(QWExtra *extra = d->extraData())
        extra->mask = QRegion();
    SetWindowRgn(winId(), 0, true);
}

void QWidgetPrivate::updateFrameStrut() const
{
    Q_Q(const QWidget);
    if (!q->isVisible() || (q->windowType() == Qt::Desktop)) {
        q->data->fstrut_dirty = q->isVisible();
        return;
    }

    RECT  fr, cr;
    GetWindowRect(q->winId(), &fr);
    GetClientRect(q->winId(), &cr);

    POINT pt;
    pt.x = 0;
    pt.y = 0;

    ClientToScreen(q->winId(), &pt);
    q->data->crect = QRect(QPoint(pt.x, pt.y),
                         QPoint(pt.x + cr.right, pt.y + cr.bottom));

    QTLWExtra *top = topData();
    top->ftop = data.crect.top() - fr.top;
    top->fleft = data.crect.left() - fr.left;
    top->fbottom = fr.bottom - data.crect.bottom();
    top->fright = fr.right - data.crect.right();

    q->data->fstrut_dirty = false;
}

void QWidget::setWindowOpacity(qreal level)
{
    Q_D(QWidget);
    if(!isWindow())
        return;

    static bool function_resolved = false;
    if (!function_resolved) {
        ptrSetLayeredWindowAttributes =
            (PtrSetLayeredWindowAttributes) QLibrary::resolve("user32",
                                                              "SetLayeredWindowAttributes");
        function_resolved = true;
    }

    if (!ptrSetLayeredWindowAttributes)
        return;

    level = qMin<qreal>(qMax<qreal>(level, 0), 1.0);
    int wl = GetWindowLongA(winId(), GWL_EXSTYLE);

    if (level != 1.0) {
        if ((wl&Q_WS_EX_LAYERED) == 0)
            SetWindowLongA(winId(), GWL_EXSTYLE, wl|Q_WS_EX_LAYERED);
    } else if (wl&Q_WS_EX_LAYERED) {
        SetWindowLongA(winId(), GWL_EXSTYLE, wl & ~Q_WS_EX_LAYERED);
    }

    bool exposeTrick = (level != 1.0) && (d->topData()->opacity == 1.0) && isVisible();
    if (exposeTrick)
        d->hide_sys(); // Work around Windows (non-)expose bug
    (*ptrSetLayeredWindowAttributes)(winId(), 0, (int)(level * 255), Q_LWA_ALPHA);
    if (exposeTrick)
        d->show_sys(); // Work around Windows (non-)expose bug
    d->topData()->opacity = (uchar)(level * 255);
}

qreal QWidget::windowOpacity() const
{
    return isWindow() ? (d_func()->topData()->opacity / 255.0) : 0.0;
}

class QGlobalRasterPaintEngine: public QRasterPaintEngine
{
public:
    inline QGlobalRasterPaintEngine() : QRasterPaintEngine() { setFlushOnEnd(false); }
    Q_GLOBAL_STATIC(QGlobalRasterPaintEngine, instance)
};

QPaintEngine *QWidget::paintEngine() const
{
    Q_D(const QWidget);
    QPaintEngine *globalEngine = QGlobalRasterPaintEngine::instance();
    if (globalEngine->isActive()) {
        if (d->extraPaintEngine)
            return d->extraPaintEngine;
        QRasterPaintEngine *engine = new QRasterPaintEngine();
        engine->setFlushOnEnd(false);
        const_cast<QWidget *>(this)->d_func()->extraPaintEngine = engine;
        return d->extraPaintEngine;
    }
    return globalEngine;
}

void QWidgetPrivate::setModal_sys()
{
}
