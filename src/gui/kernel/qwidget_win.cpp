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
#include "qpainter.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qlayout.h"
#include "qt_windows.h"
#include "qcursor.h"
#include <private/qapplication_p.h>
#include <private/qinputcontext_p.h>
#include "qevent.h"
#include "qstack.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "qlibrary.h"
#include "qdesktopwidget.h"
#include <private/qpaintengine_win_p.h>
#include "qcleanuphandler.h"

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

#include "qwidget_p.h"

#if !defined(WS_EX_TOOLWINDOW)
#define WS_EX_TOOLWINDOW 0x00000080
#endif

#if !defined(GWLP_WNDPROC)
#define GWLP_WNDPROC GWL_WNDPROC
#endif

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

extern void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

#define d d_func()
#define q q_func()

#define XCOORD_MAX 32767
#define WRECT_MAX 16383

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
    if (testAttribute(Qt::WA_WState_Created) && window == 0)
        return;
    setAttribute(Qt::WA_WState_Created);                        // set created flag

    if (!parentWidget() || parentWidget()->isDesktop())
        setWFlags(Qt::WType_TopLevel);                // top-level widget

    static int sw = -1, sh = -1;

    bool topLevel = testWFlags(Qt::WType_TopLevel);
    bool popup = testWFlags(Qt::WType_Popup);
    bool dialog = testWFlags(Qt::WType_Dialog);
    bool desktop  = testWFlags(Qt::WType_Desktop);
    HINSTANCE appinst  = qWinAppInst();
    HWND   parentw, destroyw = 0;
    WId           id;

    QString windowClassName = qt_reg_winclass(getWFlags());

    if (!window)                                // always initialize
        initializeWindow = true;

    if (popup)
        setWFlags(Qt::WStyle_StaysOnTop); // a popup stays on top

    if (sw < 0) {                                // get the (primary) screen size
        sw = GetSystemMetrics(SM_CXSCREEN);
        sh = GetSystemMetrics(SM_CYSCREEN);
    }

    if (dialog || popup || desktop) {                // these are top-level, too
        topLevel = true;
        setWFlags(Qt::WType_TopLevel);
    }

    if (desktop) {                                // desktop widget
        popup = false;                                // force this flags off
#ifndef Q_OS_TEMP
        if (QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95)
            data->crect.setRect(GetSystemMetrics(76 /* SM_XVIRTUALSCREEN  */), GetSystemMetrics(77 /* SM_YVIRTUALSCREEN  */),
                           GetSystemMetrics(78 /* SM_CXVIRTUALSCREEN */), GetSystemMetrics(79 /* SM_CYVIRTUALSCREEN */));
        else
#endif
            data->crect.setRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }

    parentw = parentWidget() ? parentWidget()->winId() : 0;

#ifdef UNICODE
    QString title;
    const TCHAR *ttitle = 0;
#endif
    QByteArray title95;
    int         style = WS_CHILD;
    int         exsty = WS_EX_NOPARENTNOTIFY;

    if (window) {
        style = GetWindowLongA(window, GWL_STYLE);
        if (!style)
            qErrnoWarning("QWidget::create: GetWindowLong failed");
        topLevel = false; // #### needed for some IE plugins??
    } else if (popup) {
        style = WS_POPUP;
    } else if (!topLevel) {
        if (!testWFlags(Qt::WStyle_Customize))
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_MinMax | Qt::WStyle_SysMenu );
    } else if (!desktop) {
        if (testWFlags(Qt::WStyle_Customize)) {
            if (testWFlags(Qt::WStyle_NormalBorder|Qt::WStyle_DialogBorder) == 0) {
                style = WS_POPUP;                // no border
            } else {
                style = 0;
            }
        } else {
            style = WS_OVERLAPPED;
            if (testWFlags(Qt::WType_Dialog))
#ifndef Q_OS_TEMP
                setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu | Qt::WStyle_ContextHelp);
            else
                setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_MinMax | Qt::WStyle_SysMenu );
        }
#else
                setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu);
            else
                setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title);
        }
#endif
        // workaround for some versions of Windows
        if (testWFlags(Qt::WStyle_MinMax))
            clearWFlags(Qt::WStyle_ContextHelp);
    }
    if (!desktop) {
        // if (!testAttribute(Qt::WA_PaintUnclipped))
        // ### Commented out for now as it causes some problems, but
        // this should be correct anyway, so dig some more into this
            style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
        if (topLevel) {
#ifndef Q_OS_TEMP
            if (testWFlags(Qt::WStyle_NormalBorder)) {
                style |= WS_THICKFRAME;
                if(!testWFlags(Qt::WStyle_Title | Qt::WStyle_SysMenu | Qt::WStyle_Minimize | Qt::WStyle_Maximize | Qt::WStyle_ContextHelp))
                    style |= WS_POPUP;
            } else if (testWFlags(Qt::WStyle_DialogBorder))
                style |= WS_POPUP | WS_DLGFRAME;
#else
            if (testWFlags(Qt::WStyle_DialogBorder))
                style |= WS_POPUP;
#endif
            if (testWFlags(Qt::WStyle_Title))
                style |= WS_CAPTION;
            if (testWFlags(Qt::WStyle_SysMenu))
                style |= WS_SYSMENU;
            if (testWFlags(Qt::WStyle_Minimize))
                style |= WS_MINIMIZEBOX;
            if (testWFlags(Qt::WStyle_Maximize))
                style |= WS_MAXIMIZEBOX;
            if (testWFlags(Qt::WStyle_Tool) | testWFlags(Qt::WType_Popup))
                exsty |= WS_EX_TOOLWINDOW;
            if (testWFlags(Qt::WStyle_ContextHelp))
                exsty |= WS_EX_CONTEXTHELP;
        }
    }
    if (testWFlags(Qt::WStyle_Title)) {
        QT_WA({
            title = isTopLevel() ? qAppName() : objectName();
            ttitle = (TCHAR*)title.utf16();
        } , {
            title95 = isTopLevel() ? qAppName().toLocal8Bit() : objectName().toLatin1();
        });
    }

        // The Qt::WA_WState_Created flag is checked by translateConfigEvent() in
        // qapplication_win.cpp. We switch it off temporarily to avoid move
        // and resize events during creation
    setAttribute(Qt::WA_WState_Created, false);

    if (window) {                                // override the old window
        if (destroyOldWindow)
            destroyw = data->winid;
        id = window;
        d->setWinId(window);
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
        QWidget *otherDesktop = find(id);        // is there another desktop?
        if (otherDesktop && otherDesktop->testWFlags(Qt::WPaintDesktop)) {
            otherDesktop->d->setWinId(0);        // remove id from widget mapper
            d->setWinId(id);                     // make sure otherDesktop is
            otherDesktop->d->setWinId(id);       //   found first
        } else {
            d->setWinId(id);
        }
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
        d->setWinId(id);
        if (testWFlags(Qt::WStyle_StaysOnTop))
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
        d->setWinId(id);
    }

    if (desktop) {
        setAttribute(Qt::WA_WState_Visible);
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
                MoveWindow(winId(), x, y, w, h, true);
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
            data->crect = QRect(QPoint(pt.x, pt.y),
                           QPoint(pt.x+cr.right, pt.y+cr.bottom));

            QTLWExtra *top = d->topData();
            top->ftop = data->crect.top() - fr.top;
            top->fleft = data->crect.left() - fr.left;
            top->fbottom = fr.bottom - data->crect.bottom();
            top->fright = fr.right - data->crect.right();
            data->fstrut_dirty = false;

            d->createTLExtra();
        } else {
            data->crect.setCoords(cr.left, cr.top, cr.right, cr.bottom);
            // in case extra data already exists (eg. reparent()).  Set it.
        }
    }

    setAttribute(Qt::WA_WState_Created);                // accept move/resize events
    d->hd = 0;                                        // no display context

    if (window) {                                // got window from outside
        if (IsWindowVisible(window))
            setAttribute(Qt::WA_WState_Visible);
        else
            setAttribute(Qt::WA_WState_Visible, false);
    }

#if defined(QT_NON_COMMERCIAL)
    QT_NC_WIDGET_CREATE
#endif

    if (destroyw) {
        DestroyWindow(destroyw);
    }

    d->setFont_sys();
    QInputContext::enable(this, data->im_enabled & isEnabled());
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
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
        if (testWFlags(Qt::WShowModal))                // just be sure we leave modal
            qt_leave_modal(this);
        else if (testWFlags(Qt::WType_Popup))
            qApp->closePopup(this);
        if (destroyWindow && !testWFlags(Qt::WType_Desktop)) {
            DestroyWindow(winId());
        }
        d->setWinId(0);
    }
}

void QWidgetPrivate::reparentChildren()
{
    QObjectList chlist = q->children();
    for(int i = 0; i < chlist.size(); ++i) { // reparent children
        QObject *obj = chlist.at(i);
        if (obj->isWidgetType()) {
            QWidget *w = (QWidget *)obj;
            if (w->isPopup()) {
                ;
            } else if (w->isTopLevel()) {
                bool showIt = w->isShown();
                QPoint old_pos = w->pos();
                w->setParent(q, w->getWFlags());
                w->move(old_pos);
                if (showIt)
                    w->show();
            } else {
                SetParent(w->winId(), q->winId());
                w->d->reparentChildren();
            }
        }
    }
}

void QWidgetPrivate::setParent_sys(QWidget *parent, Qt::WFlags f)
{
    WId old_winid = data.winid;
    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    if (q->isVisible()) {
        ShowWindow(data.winid, SW_HIDE);
        SetParent(data.winid, 0);
    }

    bool accept_drops = q->acceptDrops();
    if (accept_drops)
        q->setAcceptDrops(false); // ole dnd unregister (we will register again below)
    if (q->testWFlags(Qt::WType_Desktop))
        old_winid = 0;
    setWinId(0);

    QObjectPrivate::setParent_helper(parent);
    bool enable = q->isEnabled();                // remember status
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize s = q->size();
    QString capt = q->windowTitle();
    data.window_type = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
    q->create();

    if (q->isTopLevel() || (!parent || parent->isVisible()))
        q->setAttribute(Qt::WA_WState_Hidden);

    d->reparentChildren();

    q->setGeometry(0, 0, s.width(), s.height());
    q->setEnabled(enable);
    q->setFocusPolicy(fp);
    if (!capt.isNull()) {
        d->extra->topextra->caption = QString::null;
        q->setWindowTitle(capt);
    }
    if (old_winid)
        DestroyWindow(old_winid);

    if (accept_drops)
        q->setAcceptDrops(true);

#ifdef Q_OS_TEMP
    // Show borderless toplevel windows in tasklist & NavBar
    if (!parent) {
        QString txt = windowTitle().isEmpty()?qAppName():windowTitle();
        SetWindowText(winId(), (TCHAR*)txt.utf16());
    }
#endif
}


QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    if (!isVisible() || isMinimized())
        return mapTo(topLevelWidget(), pos) + topLevelWidget()->pos() +
        (topLevelWidget()->geometry().topLeft() - topLevelWidget()->frameGeometry().topLeft());
    POINT p;
    QPoint tmp = d->mapToWS(pos);
    p.x = tmp.x();
    p.y = tmp.y();
    ClientToScreen(winId(), &p);
    return QPoint(p.x, p.y);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    if (!isVisible() || isMinimized())
        return mapFrom(topLevelWidget(), pos - topLevelWidget()->pos());
    POINT p;
    p.x = pos.x();
    p.y = pos.y();
    ScreenToClient(winId(), &p);
    return d->mapFromWS(QPoint(p.x, p.y));
}


void QWidgetPrivate::setFont_sys(QFont *f)
{
    QInputContext::setFont(q, (f ? *f : q->font()));
}

void QWidget::resetInputContext()
{
    if (!hasFocus())
        return;
    QInputContext::accept();
}

void QWidgetPrivate::updateSystemBackground() {}

extern void qt_set_cursor(QWidget *, const QCursor &); // qapplication_win.cpp

void QWidget::setCursor(const QCursor &cursor)
{
    if (cursor.shape() != Qt::ArrowCursor
        || (d->extra && d->extra->curs)) {
        d->createExtra();
        delete d->extra->curs;
        d->extra->curs = new QCursor(cursor);
    }
    setAttribute(Qt::WA_SetCursor);
    qt_set_cursor(this, QWidget::cursor());
}

void QWidget::unsetCursor()
{
    if (d->extra) {
        delete d->extra->curs;
        d->extra->curs = 0;
    }
    if (!isTopLevel())
        setAttribute(Qt::WA_SetCursor, false);
    qt_set_cursor(this, cursor());
}

void QWidget::setWindowModified(bool mod)
{
    setAttribute(Qt::WA_WindowModified, mod);
    {
        QString caption = QWidget::windowTitle();
#if defined(QT_NON_COMMERCIAL)
        QT_NC_CAPTION
#else
            QString cap = caption;
#endif
        if(mod)
            cap += " *";
        QT_WA({
            SetWindowText(winId(), (TCHAR*)cap.utf16());
        } , {
            SetWindowTextA(winId(), cap.toLocal8Bit());
        });
    }
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
        return; // for less flicker
    d->topData()->caption = caption;

#if defined(QT_NON_COMMERCIAL)
    QT_NC_CAPTION
#else
    QString cap = caption;
#endif
    if(isWindowModified())
        cap += " *";
    QT_WA({
        SetWindowText(winId(), (TCHAR*)cap.utf16());
    } , {
        SetWindowTextA(winId(), cap.toLocal8Bit());
    });

    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

/*
  Create an icon mask the way Windows wants it using CreateBitmap.
*/

HBITMAP qt_createIconMask(const QBitmap &bitmap)
{
    QImage bm = bitmap.toImage();
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


void QWidgetPrivate::setWindowIcon_sys(const QPixmap &pixmap)
{
    QTLWExtra* x = d->topData();
    delete x->icon;
    x->icon = 0;
    if (x->winIcon) {
        DestroyIcon(x->winIcon);
        x->winIcon = 0;
    }
    if (!pixmap.isNull()) {                        // valid icon
        QPixmap pm(pixmap.size(), pixmap.depth(), QPixmap::NormalOptim);
        QBitmap mask(pixmap.size(), false, QPixmap::NormalOptim);
        if (pixmap.mask()) {
            mask.fill(Qt::color0);
            QPainter maskPainter(&mask);
            maskPainter.drawPixmap(0, 0, *pixmap.mask());
        } else {
            mask.fill(Qt::color1);
        }
        QPainter painter(&pm);
        painter.drawPixmap(0, 0, pixmap);
        painter.end();
        HBITMAP im = qt_createIconMask(mask);
        ICONINFO ii;
        ii.fIcon    = true;
        ii.hbmMask  = im;
        ii.hbmColor = pm.hbm();
        ii.xHotspot = 0;
        ii.yHotspot = 0;
        x->icon = new QPixmap(pixmap);
        x->winIcon = CreateIconIndirect(&ii);
        DeleteObject(im);
    }
    SendMessageA(q->winId(), WM_SETICON, 0, /* ICON_SMALL */
                  (long)x->winIcon);
    SendMessageA(q->winId(), WM_SETICON, 1, /* ICON_BIG */
                  (long)x->winIcon);

}


void QWidget::setWindowIconText(const QString &iconText)
{
    d->topData()->iconText = iconText;
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
    SetForegroundWindow(topLevelWidget()->winId());
}


void QWidget::update()
{
    if (isVisible() && isUpdatesEnabled()) {
        InvalidateRect(winId(), 0, false);
        setAttribute(Qt::WA_PendingUpdate);
    }
}

void QWidget::update(const QRegion &rgn)
{
    if (isVisible() && isUpdatesEnabled()) {
        if (!rgn.isEmpty()) {
            InvalidateRgn(winId(), rgn.handle(), false);
            setAttribute(Qt::WA_PendingUpdate);
        }
    }
}

void QWidget::update(const QRect &r)
{
    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
    if (w && h && isVisible() && isUpdatesEnabled()) {
        RECT r;
        r.left = x;
        r.top  = y;
        if (w < 0)
            r.right = data->crect.width();
        else
            r.right = x + w;
        if (h < 0)
            r.bottom = data->crect.height();
        else
            r.bottom = y + h;
        InvalidateRect(winId(), &r, false);
        setAttribute(Qt::WA_PendingUpdate);
    }
}

struct QWinDoubleBuffer
{
    enum {
        MaxWidth = SHRT_MAX,
        MaxHeight = SHRT_MAX
    };

    HDC hdc;
    HBITMAP hbm;
    int width;
    int height;
};

static QWinDoubleBuffer *qt_global_double_buffer = 0;
static bool qt_global_double_buffer_active = false;

static void qt_discard_double_buffer(QWinDoubleBuffer **db)
{
    if (!*db)
        return;

    DeleteDC((*db)->hdc);
    DeleteObject((*db)->hbm);

    delete *db;
    *db = 0;
}

void qt_discard_double_buffer()
{
    qt_discard_double_buffer(&qt_global_double_buffer);
}

static QWinDoubleBuffer *qt_win_create_double_buffer(int width, int height)
{
    QWinDoubleBuffer *db = new QWinDoubleBuffer;
    db->hdc = CreateCompatibleDC(qt_display_dc());
    db->hbm = CreateCompatibleBitmap(qt_display_dc(), width, height);
    db->width = width;
    db->height = height;
    Q_ASSERT(db->hdc);
    Q_ASSERT(db->hbm);
    bool success = SelectObject(db->hdc, db->hbm);
    Q_ASSERT(success);
    Q_UNUSED(success); // --release warning
    SelectClipRgn(db->hdc, 0);
    return db;
}

static void qt_win_get_double_buffer(QWinDoubleBuffer **db, int width, int height)
{
    // the db should consist of 128x128 chunks
    width  = qMin(((width / 128) + 1) * 128, (int)QWinDoubleBuffer::MaxWidth);
    height = qMin(((height / 128) + 1) * 128, (int)QWinDoubleBuffer::MaxHeight);

    if (qt_global_double_buffer_active) {
        *db = qt_win_create_double_buffer(width, height);
        return;
    }

    qt_global_double_buffer_active = true;

    if (qt_global_double_buffer) {
        if (qt_global_double_buffer->width >= width
            && qt_global_double_buffer->height >= height) {
            *db = qt_global_double_buffer;
            SelectClipRgn((*db)->hdc, 0);
            return;
        }

        width  = qMax(qt_global_double_buffer->width,  width);
        height = qMax(qt_global_double_buffer->height, height);

        qt_discard_double_buffer(&qt_global_double_buffer);
    }

    qt_global_double_buffer = qt_win_create_double_buffer(width, height);
    *db = qt_global_double_buffer;
};

static void qt_win_release_double_buffer(QWinDoubleBuffer **db)
{
    if (*db != qt_global_double_buffer)
        qt_discard_double_buffer(db);
    else
        qt_global_double_buffer_active = false;
}

extern void qt_erase_background(HDC, int, int, int, int, const QBrush &, int, int, QWidget *);

void QWidget::repaint(const QRegion& rgn)
{
    if (!isVisible() || !isUpdatesEnabled() || !testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;

    setAttribute(Qt::WA_PendingUpdate, false);
    if (testAttribute(Qt::WA_WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");

    ValidateRgn(winId(),rgn.handle());

    setAttribute(Qt::WA_WState_InPaintEvent);

    QRect br = rgn.boundingRect();
    QRect brWS = d->mapToWS(br);
    bool do_clipping = (br != QRect(0, 0, data->crect.width(), data->crect.height()));
    bool double_buffer = (!testAttribute(Qt::WA_PaintOnScreen)
                          && !testAttribute(Qt::WA_NoSystemBackground)
                          && br.width()  <= QWinDoubleBuffer::MaxWidth
                          && br.height() <= QWinDoubleBuffer::MaxHeight
                          && !QPainter::redirected(this));
    bool tmphdc = !d->hd;
    if (tmphdc)
        d->hd = GetDC(winId());
    HDC old_dc = (HDC)d->hd;

    QPoint redirectionOffset;

    QWinDoubleBuffer *qDoubleBuffer = 0;
    if (double_buffer) {
        qt_win_get_double_buffer( &qDoubleBuffer, br.width(), br.height());
        d->hd = qDoubleBuffer->hdc;
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
            qt_erase_background((HDC)d->hd, br.x()-redirectionOffset.x(), br.y()-redirectionOffset.y(),
                                br.width(), br.height(),
                                palette().brush(w->d->bg_role),
                                br.x() + offset.x(), br.y() + offset.y(),
                                this);
        } else {
            QRegion mappedRegion(rgn);
            mappedRegion.translate(-data->wrect.topLeft());
            SelectClipRgn((HDC)d->hd, mappedRegion.handle());
            QSize bgsize = data->wrect.isValid() ? data->wrect.size() : data->crect.size();
            // ### This triggers bitblt on entire area. Potentially a lot. Clip here too!
            qt_erase_background((HDC)d->hd, 0, 0, bgsize.width(), bgsize.height(),
                                palette().brush(w->d->bg_role), offset.x(), offset.y(),
                                this);
        }

        if (parents.size()) {
            w = parents.pop();
            for (;;) {
                if (w->testAttribute(Qt::WA_ContentsPropagated)) {
                    QPainter::setRedirected(w, q, offset);
                    QRect rr = d->clipRect();
                    rr.translate(offset);
                    QPaintEvent e(rr);
                    bool was_in_paint_event = w->testAttribute(Qt::WA_WState_InPaintEvent);
                    w->setAttribute(Qt::WA_WState_InPaintEvent);
                    QApplication::sendEvent(w, &e);
                    if(!was_in_paint_event) {
                        w->setAttribute(Qt::WA_WState_InPaintEvent, false);
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
        SelectClipRgn((HDC)d->hd, 0);
    }

    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent(this, &e);

    if (do_clipping)
        qt_clear_paintevent_clipping();

    if (!redirectionOffset.isNull())
        QPainter::restoreRedirected(this);



    if (double_buffer) {
        QVector<QRect> rects = rgn.rects();
        for (int i=0; i<rects.size(); ++i) {
            QRect rr = d->mapToWS(rects.at(i));
            BitBlt(old_dc,
                   rr.x(), rr.y(),
                   rr.width(), rr.height(),
                   (HDC)d->hd,
                   rr.x()-brWS.x(), rr.y()-brWS.y(),
                   SRCCOPY);
        }

        d->hd = old_dc;

        qt_win_release_double_buffer(&qDoubleBuffer);

        // Start timer to kill global double buffer.
        if (!qApp->activeWindow()) {
            extern int qt_double_buffer_timer;
            if (qt_double_buffer_timer)
                qApp->killTimer(qt_double_buffer_timer);
            qt_double_buffer_timer = qApp->startTimer(500);
        }
    }

    if (tmphdc) {
        ReleaseDC(winId(), (HDC)d->hd);
        d->hd = 0;
    }

    setAttribute(Qt::WA_WState_InPaintEvent, false);
    if(!testAttribute(Qt::WA_PaintOutsidePaintEvent) && paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

    if (testAttribute(Qt::WA_ContentsPropagated))
        d->updatePropagatedBackground(&rgn);
}


void QWidget::setWindowState(Qt::WindowStates newstate)
{
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

    if (isTopLevel()) {

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

    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}


/*
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidgetPrivate::hide_sys()
{
    deactivateWidgetCleanup();
    ShowWindow(q->winId(), SW_HIDE);
}


#ifndef Q_OS_TEMP // ------------------------------------------------

/*
  \internal
  Platform-specific part of QWidget::show().
*/
void QWidgetPrivate::show_sys()
{
#if defined(QT_NON_COMMERCIAL)
    QT_NC_SHOW_WINDOW
#endif
    if (q->testAttribute(Qt::WA_OutsideWSRange))
        return;
    q->setAttribute(Qt::WA_Mapped);

    int sm = SW_SHOWNORMAL;
    if (q->isTopLevel()) {
        if (q->isMinimized())
            sm = SW_SHOWMINIMIZED;
        else if (q->isMinimized())
            sm = SW_SHOWMAXIMIZED;
    }
    if (q->testWFlags(Qt::WStyle_Tool) || q->isPopup())
        sm = SW_SHOWNOACTIVATE;

    ShowWindow(q->winId(), sm);
    if (IsIconic(q->winId()))
        data.window_state |= Qt::WindowMinimized;
    if (IsZoomed(q->winId()))
        data.window_state |= Qt::WindowMaximized;

    UpdateWindow(q->winId());
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
    if (testAttribute(Qt::WA_OutsideWSRange))
        return;
    setAttribute(Qt::WA_Mapped);
    uint sm = SW_SHOW;
    if (isTopLevel()) {
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

    if (testWFlags(Qt::WStyle_Tool) || isPopup())
        sm = SW_SHOWNOACTIVATE;

    ShowWindow(winId(), sm);
    if (isTopLevel() && sm == SW_SHOW)
        SetForegroundWindow(winId());
    UpdateWindow(winId());
}


#if 0
void QWidget::showMinimized()
{
    if (isTopLevel()) {
        if (d->topData()->fullscreen) {
            reparent(0, d->topData()->savedFlags, d->topData()->normalGeometry.topLeft());
            d->topData()->fullscreen = 0;
            SHFullScreen(winId(), SHFS_SHOWTASKBAR | SHFS_SHOWSIPBUTTON);
        }
        d->topData()->showMode = 1;
    }

    if (isVisible())
        show_sys();
    else
        show();

    QEvent e(QEvent::ShowMinimized);
    QApplication::sendEvent(this, &e);
    data.window_state &= ~Qt::WindowMaximized;
    data.window_state |= Qt::WindowMinimized;
}


void QWidget::showMaximized()
{
    if (isTopLevel()) {
        if (d->topData()->normalGeometry.width() < 0) {
            d->topData()->savedFlags = getWFlags();
            d->topData()->normalGeometry = geometry();
            reparent(0,
                      Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_NoBorder |
                      (getWFlags() & 0xffff0000), // preserve some widget flags
                      d->topData()->normalGeometry.topLeft());
            d->topData()->fullscreen = 0;
            SHFullScreen(winId(), SHFS_SHOWTASKBAR | SHFS_SHOWSIPBUTTON);
        }
        d->topData()->showMode = 2;
    }

    if (isVisible())
        show_sys();
    else
        show();

    QEvent e(QEvent::ShowMaximized);
    QApplication::sendEvent(this, &e);
    data.window_state &= ~Qt::WindowMinimized;
    data.window_state |= Qt::WindowMaximized;
}


void QWidget::showNormal()
{
    d->topData()->showMode = 0;
    if (isTopLevel()) {
        if (d->topData()->normalGeometry.width() > 0) {
            int val = d->topData()->savedFlags;
            int style = WS_OVERLAPPED,
                exsty = 0;

            style |= (val & Qt::WStyle_DialogBorder ? WS_POPUP : 0);
            style |= (val & Qt::WStyle_Title        ? WS_CAPTION : 0);
            style |= (val & Qt::WStyle_SysMenu        ? WS_SYSMENU : 0);
            style |= (val & Qt::WStyle_Minimize        ? WS_MINIMIZEBOX : 0);
            style |= (val & Qt::WStyle_Maximize        ? WS_MAXIMIZEBOX : 0);
            exsty |= (val & Qt::WStyle_Tool                ? WS_EX_TOOLWINDOW : 0);
            exsty |= (val & Qt::WType_Popup                ? WS_EX_TOOLWINDOW : 0);
            exsty |= (val & Qt::WStyle_ContextHelp        ? WS_EX_CONTEXTHELP : 0);

            SetWindowLong(winId(), GWL_STYLE, style);
            if (exsty)
                SetWindowLong(winId(), GWL_EXSTYLE, exsty);
            // flush Window style cache
            SHFullScreen(winId(), SHFS_SHOWTASKBAR | SHFS_SHOWSIPBUTTON);
            SetWindowPos(winId(), HWND_TOP, 0, 0, 200, 200, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

            d->topData()->fullscreen = 0;
            QRect r = d->topData()->normalGeometry;
            if (r.width() >= 0) {
                // the widget has been maximized
                d->topData()->normalGeometry = QRect(0,0,-1,-1);
                resize(r.size());
                move(r.topLeft());
            }
        }
    }
    show();
    ShowWindow(winId(), SW_SHOWNORMAL);

    if (d->extra && d->extra->topextra)
        d->extra->topextra->fullscreen = 0;
    QEvent e(QEvent::ShowNormal);
    QApplication::sendEvent(this, &e);
    data.window_state &= ~Qt::WindowMaximized;
    data.window_state &= ~Qt::WindowMinimized;
}
#endif // 0

#endif // Q_OS_TEMP -------------------------------------------------

void QWidgetPrivate::raise_sys()
{
    uint f = (q->isPopup() || q->testWFlags(Qt::WStyle_Tool)) ? SWP_NOACTIVATE : 0;
    SetWindowPos(q->winId(), HWND_TOP, 0, 0, 0, 0, f | SWP_NOMOVE | SWP_NOSIZE);
}

void QWidgetPrivate::lower_sys()
{
    uint f = (q->isPopup() || q->testWFlags(Qt::WStyle_Tool)) ? SWP_NOACTIVATE : 0;
    SetWindowPos(q->winId(), HWND_BOTTOM, 0, 0, 0, 0, f | SWP_NOMOVE |
                  SWP_NOSIZE);
}

void QWidgetPrivate::stackUnder_sys(QWidget* w)
{
    SetWindowPos(q->winId(), w->winId() , 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}


/*
  Helper function for non-toplevel widgets. Helps to map Qt's 32bit
  coordinate system to Windpws's 16bit coordinate system.

  This code is duplicated from the X11 code, so any changes there
  should also (most likely) be reflected here.

  (In all comments below: s/X/Windows/g)
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
        } else if (q->isShown()) {
            mapWindow = true;
        }
    }

    if (outsideRange)
        return;


    // and now recursively for all children...
    data.wrect = wrect;
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
    MoveWindow(q->winId(), xrect.x(), xrect.y(), xrect.width(), xrect.height(), true);
    if (mapWindow) {
            q->setAttribute(Qt::WA_Mapped);
            ShowWindow(q->winId(), SW_SHOWNOACTIVATE);
    }

}

//
// The internal qWinRequestConfig, defined in qapplication_win.cpp, stores move,
// resize and setGeometry requests for a widget that is already
// processing a config event. The purpose is to avoid recursion.
//
void qWinRequestConfig(WId, int, int, int, int, int);

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    if (d->extra) {                                // any size restrictions?
        w = qMin(w,d->extra->maxw);
        h = qMin(h,d->extra->maxh);
        w = qMax(w,d->extra->minw);
        h = qMax(h,d->extra->minh);
    }
    if (q->isTopLevel()) {
        d->topData()->normalGeometry = QRect(0, 0, -1, -1);
        w = qMax(1, w);
        h = qMax(1, h);
    }

    QSize  oldSize(q->size());
    QPoint oldPos(q->pos());

    if (!q->isTopLevel())
        isMove = (data.crect.topLeft() != QPoint(x, y));
    bool isResize = w != oldSize.width() || h != oldSize.height();

    if (!isMove && !isResize)
        return;

    if (isResize) {
        if (!q->testAttribute(Qt::WA_StaticContents))
            ValidateRgn(q->winId(), 0);
        else if (!q->testAttribute(Qt::WA_WState_InPaintEvent)) {
            QRegion region(0, 0, 1, 1);
            int result = GetUpdateRgn(q->winId(), region.handle(), false);
            if (result != NULLREGION && result != ERROR)
                q->repaint(region);
        }
    }

    if (isResize)
        data.window_state &= ~Qt::WindowMaximized;
    data.window_state &= ~Qt::WindowFullScreen;
    if (q->testAttribute(Qt::WA_WState_ConfigPending)) {        // processing config event
        qWinRequestConfig(q->winId(), isMove ? 2 : 1, x, y, w, h);
    } else {
        q->setAttribute(Qt::WA_WState_ConfigPending);
        if (q->isTopLevel()) {
            QRect fr(q->frameGeometry());
            if (d->extra) {
                fr.setLeft(fr.left() + x - data.crect.left());
                fr.setTop(fr.top() + y - data.crect.top());
                fr.setRight(fr.right() + (x + w - 1) - data.crect.right());
                fr.setBottom(fr.bottom() + (y + h - 1) - data.crect.bottom());
            }
            MoveWindow(q->winId(), fr.x(), fr.y(), fr.width(), fr.height(), true);
            RECT rect;
            GetClientRect(q->winId(), &rect);
	    data.crect.setRect(x, y, rect.right - rect.left, rect.bottom - rect.top);
        } else {
            data.crect.setRect(x, y, w, h);
            d->setWSGeometry();
        }
        q->setAttribute(Qt::WA_WState_ConfigPending, false);
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
            if (!q->testAttribute(Qt::WA_StaticContents))
                q->testAttribute(Qt::WA_WState_InPaintEvent)?q->update():q->repaint();
        }
    } else {
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}


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
    updateGeometry();
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
    if (maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
        qWarning("QWidget::setMaximumSize: The largest allowed size is (%d,%d)",
                 QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        maxw = qMin(maxw, QWIDGETSIZE_MAX);
        maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if (maxw < 0 || maxh < 0) {
        qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                 objectName().isEmpty() ? "unnamed" : objectName().toLatin1(),
                 metaObject()->className(), maxw, maxh);
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
    updateGeometry();
}

void QWidget::setSizeIncrement(int w, int h)
{
    d->createTLExtra();
    d->extra->topextra->incw = w;
    d->extra->topextra->inch = h;
}

void QWidget::setBaseSize(int w, int h)
{
    d->createTLExtra();
    d->extra->topextra->basew = w;
    d->extra->topextra->baseh = h;
}

void QWidget::scroll(int dx, int dy)
{
    if (!isUpdatesEnabled() && children().size() == 0)
        return;
    UINT flags = SW_INVALIDATE | SW_SCROLLCHILDREN;
    if (!testAttribute(Qt::WA_NoBackground))
        flags |= SW_ERASE;

    ScrollWindowEx(winId(), dx, dy, 0, 0, 0, 0, flags);
    UpdateWindow(winId());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    if (!isUpdatesEnabled())
        return;
    UINT flags = SW_INVALIDATE;
    if (!testAttribute(Qt::WA_NoBackground))
        flags |= SW_ERASE;

    RECT wr;
    wr.top = r.top();
    wr.left = r.left();
    wr.bottom = r.bottom()+1;
    wr.right = r.right()+1;
    ScrollWindowEx(winId(), dx, dy, &wr, &wr, 0, 0, flags);
    UpdateWindow(winId());
}

extern Q_GUI_EXPORT HDC qt_display_dc();

int QWidget::metric(PaintDeviceMetric m) const
{
    int val;
    if (m == PdmWidth) {
        val = data->crect.width();
    } else if (m == PdmHeight) {
        val = data->crect.height();
    } else {
        HDC gdc = qt_display_dc();
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
    q->setAcceptDrops(false);
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->winIcon = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if (extra->topextra->winIcon)
        DestroyIcon(extra->topextra->winIcon);
}


bool QWidget::acceptDrops() const
{
    return (d->extra && d->extra->dropTarget);
}

void QWidget::setAcceptDrops(bool on)
{
    // Enablement is defined by d->extra->dropTarget != 0.

    if (on) {
        // Turn on.
        d->createExtra();
        QWExtra *extra = d->extraData();
        if (!extra->dropTarget)
            extra->dropTarget = qt_olednd_register(this);
    } else {
        // Turn off.
        QWExtra *extra = d->extraData();
        if (extra && extra->dropTarget) {
            qt_olednd_unregister(this, extra->dropTarget);
            extra->dropTarget = 0;
        }
    }
}

void QWidget::setMask(const QRegion &region)
{
    d->createExtra();
    if(QWExtra *extra = d->extraData())
        extra->mask = region;

    // Since SetWindowRegion takes ownership, and we need to translate,
    // we take a copy.
    HRGN wr = CreateRectRgn(0,0,0,0);
    CombineRgn(wr, region.handle(), 0, RGN_COPY);

    int fleft = 0, ftop = 0;
    if (isTopLevel()) {
        ftop = d->topData()->ftop;
        fleft = d->topData()->fleft;
    }
    OffsetRgn(wr, fleft, ftop);
    SetWindowRgn(winId(), wr, true);
}

void QWidget::setMask(const QBitmap &bitmap)
{
    d->createExtra();
    if(QWExtra *extra = d->extraData())
        extra->mask = QRegion(bitmap);

    HRGN wr = qt_win_bitmapToRegion(bitmap);

    int fleft = 0, ftop = 0;
    if (isTopLevel()) {
        ftop = d->topData()->ftop;
        fleft = d->topData()->fleft;
    }
    OffsetRgn(wr, fleft, ftop);
    SetWindowRgn(winId(), wr, true);
}

void QWidget::clearMask()
{
    d->createExtra();
    if(QWExtra *extra = d->extraData())
        extra->mask = QRegion();
    SetWindowRgn(winId(), 0, true);
}

void QWidgetPrivate::updateFrameStrut() const
{
    if (!q->isVisible() || q->isDesktop()) {
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
    if(!isTopLevel())
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

    level = qMin(qMax(level, 0), 1.0);
    int wl = GetWindowLongA(winId(), GWL_EXSTYLE);

    if (level != 1.0) {
        if ((wl&Q_WS_EX_LAYERED) == 0)
            SetWindowLongA(winId(), GWL_EXSTYLE, wl|Q_WS_EX_LAYERED);
    } else if (wl&Q_WS_EX_LAYERED) {
        SetWindowLongA(winId(), GWL_EXSTYLE, wl & ~Q_WS_EX_LAYERED);
    }

    (*ptrSetLayeredWindowAttributes)(winId(), 0, (int)(level * 255), Q_LWA_ALPHA);
    d->topData()->opacity = (uchar)(level * 255);
}

qreal QWidget::windowOpacity() const
{
    return isTopLevel() ? (d->topData()->opacity / 255.0) : 0.0;
}

static QSingleCleanupHandler<QWin32PaintEngine> qt_paintengine_cleanup_handler;
static QWin32PaintEngine *qt_widget_paintengine = 0;
QPaintEngine *QWidget::paintEngine() const
{
    if (!qt_widget_paintengine) {
        qt_widget_paintengine = new QWin32PaintEngine();
        qt_paintengine_cleanup_handler.set(&qt_widget_paintengine);
    }

    if (qt_widget_paintengine->isActive()) {
        QPaintEngine *extraEngine = new QWin32PaintEngine();
        extraEngine->setAutoDestruct(true);
        return extraEngine;
    }

    return qt_widget_paintengine;
}
