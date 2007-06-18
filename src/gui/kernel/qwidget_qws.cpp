/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcursor.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qhash.h"
#include "qstack.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include "qinputcontext.h"
#include "qdesktopwidget.h"

#include "qwsdisplay_qws.h"
#include "private/qwsdisplay_qws_p.h"
#include "qscreen_qws.h"
#include "qwsmanager_qws.h"
#include <private/qwsmanager_p.h>
#include <private/qbackingstore_p.h>
#include <private/qwindowsurface_qws_p.h>
#include <private/qwslock_p.h>
#include "qpaintengine.h"

#include "qdebug.h"

#include "qwidget_p.h"

extern int *qt_last_x;
extern int *qt_last_y;
extern WId qt_last_cursor;
extern bool qws_overrideCursor;
extern QWidget *qt_pressGrab;
extern QWidget *qt_mouseGrb;

static QWidget *keyboardGrb = 0;

static int takeLocalId()
{
    static int n=-1000;
    return --n;
}

class QWSServer;
extern QWSServer *qwsServer;

static inline bool isServerProcess()
{
    return (qwsServer != 0);
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool /*destroyOldWindow*/)
{
    Q_Q(QWidget);
    Qt::WindowType type = q->windowType();
    Qt::WindowFlags flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    data.alloc_region_index = -1;

    // we don't have a "Drawer" window type
    if (type == Qt::Drawer) {
        type = Qt::Widget;
        flags &= ~Qt::WindowType_Mask;
    }


    bool topLevel = (flags & Qt::Window);
    bool popup = (type == Qt::Popup);
    bool dialog = (type == Qt::Dialog
                   || type == Qt::Sheet
                   || (flags & Qt::MSWindowsFixedSizeDialogHint));
    bool desktop = (type == Qt::Desktop);
    bool tool = (type == Qt::Tool || type == Qt::SplashScreen || type == Qt::ToolTip);

#ifndef QT_NO_WARNING_OUTPUT
    static bool toolWarningShown = false;
    if (!toolWarningShown && type == Qt::Tool && !(flags & Qt::FramelessWindowHint)) {
        qWarning("Qtopia Core " QT_VERSION_STR " does not support tool windows with frames.\n"
                 "This behavior will change in a later release. To ensure compatibility with\n"
                 "future versions, use (Qt::Tool | Qt::FramelessWindowHint).");
        toolWarningShown = true;
    }
#endif

    // a popup stays on top
    if (popup)
        flags |= Qt::WindowStaysOnTopHint;

    WId           id;
    QWSDisplay* dpy = QWidget::qwsDisplay();

    if (!window)                                // always initialize
        initializeWindow = true;

    if(topLevel && parentWidget) { // if our parent stays on top, so must we
        QWidget *ptl = parentWidget->window();
        if(ptl && (ptl->windowFlags() & Qt::WindowStaysOnTopHint))
            flags |= Qt::WindowStaysOnTopHint;
    }

    int sw = dpy->width();
    int sh = dpy->height();
    if (desktop) {                                // desktop widget
        dialog = popup = false;                        // force these flags off
        data.crect.setRect(0, 0, sw, sh);
    } else if (topLevel && !q->testAttribute(Qt::WA_Resized)) {
        data.crect.setSize(QSize(sw/2, 4*sh/10));
    }

    if (window) {                                // override the old window
        id = window;
        setWinId(window);
    } else if (desktop) {                        // desktop widget
        id = (WId)-2;                                // id = root window
#if 0
        QWidget *otherDesktop = q->find(id);        // is there another desktop?
        if (otherDesktop && otherDesktop->testWFlags(Qt::WPaintDesktop)) {
            otherDesktop->d_func()->setWinId(0);        // remove id from widget mapper
            setWinId(id);                        // make sure otherDesktop is
            otherDesktop->d_func()->setWinId(id);        //   found first
        } else
#endif
        {
            setWinId(id);
        }
    } else {
        id = topLevel ? dpy->takeId() : takeLocalId();
        setWinId(id);                                // set widget id/handle + hd
    }


    bool hasFrame = true;
    if (topLevel) {
        if (desktop || popup || tool)
            hasFrame = false;
        else
            hasFrame = !(flags & Qt::FramelessWindowHint);
    }
    if (desktop) {
        q->setAttribute(Qt::WA_WState_Visible);
    } else if (topLevel) {                        // set X cursor
        //QCursor *oc = QApplication::overrideCursor();
        if (initializeWindow) {
            //XXX XDefineCursor(dpy, winid, oc ? oc->handle() : cursor().handle());
        }
        QWidget::qwsDisplay()->nameRegion(q->internalWinId(), q->objectName(), q->windowTitle());
    }

    if (topLevel) {
        createTLExtra();
        QTLWExtra *topextra = extra->topextra;
#ifndef QT_NO_QWS_MANAGER
        if (hasFrame) {
            // get size of wm decoration and make the old crect the new frect
            QRect cr = data.crect;
            QRegion r = QApplication::qwsDecoration().region(q, cr) | cr;
            QRect br(r.boundingRect());
            topextra->frameStrut.setCoords(cr.x() - br.x(),
                                                  cr.y() - br.y(),
                                                  br.right() - cr.right(),
                                                  br.bottom() - cr.bottom());
            if (!q->testAttribute(Qt::WA_Moved) || topextra->posFromMove)
                data.crect.translate(topextra->frameStrut.left(), topextra->frameStrut.top());
            if (!topData()->qwsManager) {
                topData()->qwsManager = new QWSManager(q);
                if(q->data->window_state & ~Qt::WindowActive == Qt::WindowMaximized)
                    topData()->qwsManager->maximize();
            }

        } else if (topData()->qwsManager) {
            delete topData()->qwsManager;
            topData()->qwsManager = 0;
            data.crect.translate(-topextra->frameStrut.left(), -topextra->frameStrut.top());
            topextra->frameStrut.setCoords(0, 0, 0, 0);
        }
#endif
        if (!topextra->caption.isEmpty())
            setWindowTitle_helper(topextra->caption);

        //XXX If we are session managed, inform the window manager about it
    } else {
        if (extra && extra->topextra)        { // already allocated due to reparent?
            extra->topextra->frameStrut.setCoords(0, 0, 0, 0);
        }
        //updateRequestedRegion(mapToGlobal(QPoint(0,0)));
    }
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    Q_D(QWidget);

    if (!isWindow() && parentWidget())
        parentWidget()->d_func()->invalidateBuffer(geometry());

    d->deactivateWidgetCleanup();
    if (testAttribute(Qt::WA_WState_Created)) {
        setAttribute(Qt::WA_WState_Created, false);
        QObjectList childObjects =  children();
        for (int i = 0; i < childObjects.size(); ++i) {
            QObject *obj = childObjects.at(i);
            if (obj->isWidgetType())
                static_cast<QWidget*>(obj)->destroy(destroySubWindows,
                                                     destroySubWindows);
        }
        releaseMouse();
        if (qt_pressGrab == this)
          qt_pressGrab = 0;

        if (keyboardGrb == this)
            releaseKeyboard();
        if (testAttribute(Qt::WA_ShowModal))                // just be sure we leave modal
            QApplicationPrivate::leaveModal(this);
        else if ((windowType() == Qt::Popup))
            qApp->d_func()->closePopup(this);

        if (d->ic) {
            delete d->ic;
            d->ic =0;
        } else {
            // release previous focus information participating with
            // preedit preservation of qic -- while we still have a winId
            QInputContext *qic = inputContext();
            if (qic)
                qic->widgetDestroyed(this);
        }

        if ((windowType() == Qt::Desktop)) {
        } else {
            if (parentWidget() && parentWidget()->testAttribute(Qt::WA_WState_Created)) {
                d->hide_sys();
            }
            if (destroyWindow && isWindow()) {
                d->extra->topextra->backingStore->windowSurface->setGeometry(QRect());
                qwsDisplay()->destroyRegion(internalWinId());
            }
        }
        d->setWinId(0);
    }
}


void QWidgetPrivate::setParent_sys(QWidget *newparent, Qt::WindowFlags f)
{
    Q_Q(QWidget);
    bool wasCreated = q->testAttribute(Qt::WA_WState_Created);
     if (q->isVisible() && q->parentWidget() && parent != q->parentWidget())
        q->parentWidget()->d_func()->invalidateBuffer(q->geometry());
#ifndef QT_NO_CURSOR
    QCursor oldcurs;
    bool setcurs=q->testAttribute(Qt::WA_SetCursor);
    if (setcurs) {
        oldcurs = q->cursor();
        q->unsetCursor();
    }
#endif

    WId old_winid = data.winid;
    if ((q->windowType() == Qt::Desktop))
        old_winid = 0;

    if (!q->isWindow() && q->parentWidget() && q->parentWidget()->testAttribute(Qt::WA_WState_Created))
        hide_sys();

    setWinId(0);

    if (parent != newparent) {
        QWidget *oldparent = q->parentWidget();
        QObjectPrivate::setParent_helper(newparent);
        if (oldparent) {
//            oldparent->d_func()->setChildrenAllocatedDirty();
//            oldparent->data->paintable_region_dirty = true;
        }
        if (newparent) {
//            newparent->d_func()->setChildrenAllocatedDirty();
//            newparent->data->paintable_region_dirty = true;
            //@@@@@@@
        }
    }
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize    s            = q->size();
    //QBrush   bgc    = background();                        // save colors
    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);

    data.window_flags = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    adjustFlags(data.window_flags, q);
    // keep compatibility with previous versions, we need to preserve the created state
    // (but we recreate the winId for the widget being reparented, again for compability)
    if (wasCreated || (!q->isWindow() && newparent->testAttribute(Qt::WA_WState_Created)))
        createWinId();
    if (q->isWindow() || (!newparent || newparent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    if (q->isWindow()) {
        QRect fs = frameStrut();
        data.crect = QRect(fs.left(), fs.top(), s.width(), s.height());
        if ((data.window_flags & Qt::FramelessWindowHint) && extra && extra->topextra)
            extra->topextra->frameStrut.setCoords(0, 0, 0, 0);
    } else {
        data.crect = QRect(0, 0, s.width(), s.height());
    }

    q->setFocusPolicy(fp);
    if (extra && !extra->mask.isEmpty()) {
        QRegion r = extra->mask;
        extra->mask = QRegion();
        q->setMask(r);
    }
    if ((int)old_winid > 0) {
        QWidget::qwsDisplay()->destroyRegion(old_winid);
        extra->topextra->backingStore->windowSurface->setGeometry(QRect());
    }
#ifndef QT_NO_CURSOR
    if (setcurs) {
        q->setCursor(oldcurs);
    }
#endif
}


QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    int           x=pos.x(), y=pos.y();
    const QWidget* w = this;
    while (w) {
        x += w->data->crect.x();
        y += w->data->crect.y();
        w = w->isWindow() ? 0 : w->parentWidget();
    }
    return QPoint(x, y);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    int           x=pos.x(), y=pos.y();
    const QWidget* w = this;
    while (w) {
        x -= w->data->crect.x();
        y -= w->data->crect.y();
        w = w->isWindow() ? 0 : w->parentWidget();
    }
    return QPoint(x, y);
}

#if 0 // #####
void QWidget::setMicroFocusHint(int x, int y, int width, int height,
                                 bool text, QFont *)
{
    if (QRect(x, y, width, height) != microFocusHint()) {
        d->createExtra();
        d->extra->micro_focus_hint.setRect(x, y, width, height);
    }
#ifndef QT_NO_QWS_INPUTMETHODS
    if (text) {
        QWidget *tlw = window();
        int winid = tlw->internalWinId();
        QPoint p(x, y + height);
        QPoint gp = mapToGlobal(p);

        QRect r = QRect(mapToGlobal(QPoint(0,0)),
                         size());

        r.setBottom(tlw->geometry().bottom());

        //qDebug("QWidget::setMicroFocusHint %d %d %d %d", r.x(),
        //        r.y(), r.width(), r.height());
        QInputContext::setMicroFocusWidget(this);

        qwsDisplay()->setIMInfo(winid, gp.x(), gp.y(), r);

        //send font info,  ###if necessary
        qwsDisplay()->setInputFont(winid, font());
    }
#endif
}
#endif

void QWidgetPrivate::updateSystemBackground() {}

#ifndef QT_NO_CURSOR
void QWidgetPrivate::setCursor_sys(const QCursor &cursor)
{
    Q_UNUSED(cursor);
    Q_Q(QWidget);
    if (q->isVisible())
        updateCursor();
}

void QWidgetPrivate::unsetCursor_sys()
{
    Q_Q(QWidget);
    if (q->isVisible())
        updateCursor();
}
#endif //QT_NO_CURSOR

void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
    Q_Q(QWidget);
    QWidget::qwsDisplay()->setWindowCaption(q, caption);
}

void QWidgetPrivate::setWindowIcon_sys(bool /*forceReset*/)
{
#if 0
     QTLWExtra* x = d->topData();
     delete x->icon;
     x->icon = 0;
    QBitmap mask;
    if (unscaledPixmap.isNull()) {
    } else {
        QImage unscaledIcon = unscaledPixmap.toImage();
        QPixmap pixmap =
            QPixmap::fromImage(unscaledIcon.scale(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        x->icon = new QPixmap(pixmap);
        mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
    }
#endif
}

void QWidgetPrivate::setWindowIconText_sys(const QString &iconText)
{
    Q_UNUSED(iconText);
}

void QWidget::grabMouse()
{
    if (qt_mouseGrb)
        qt_mouseGrb->releaseMouse();

    qwsDisplay()->grabMouse(this,true);

    qt_mouseGrb = this;
    qt_pressGrab = 0;
}

#ifndef QT_NO_CURSOR
void QWidget::grabMouse(const QCursor &cursor)
{
    if (qt_mouseGrb)
        qt_mouseGrb->releaseMouse();

    qwsDisplay()->grabMouse(this,true);
    qwsDisplay()->selectCursor(this, cursor.handle());
    qt_mouseGrb = this;
    qt_pressGrab = 0;
}
#endif

void QWidget::releaseMouse()
{
    if (qt_mouseGrb == this) {
        qwsDisplay()->grabMouse(this,false);
        qt_mouseGrb = 0;
    }
}

void QWidget::grabKeyboard()
{
    if (keyboardGrb)
        keyboardGrb->releaseKeyboard();
    qwsDisplay()->grabKeyboard(this, true);
    keyboardGrb = this;
}

void QWidget::releaseKeyboard()
{
    if (keyboardGrb == this) {
        qwsDisplay()->grabKeyboard(this, false);
        keyboardGrb = 0;
    }
}


QWidget *QWidget::mouseGrabber()
{
    if (qt_mouseGrb)
        return qt_mouseGrb;
    return qt_pressGrab;
}


QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}

void QWidget::activateWindow()
{
    QWidget *tlw = window();
    if (tlw->isVisible()) {
        Q_ASSERT(tlw->testAttribute(Qt::WA_WState_Created));
        qwsDisplay()->requestFocus(tlw->internalWinId(), true);
    }
}

void QWidgetPrivate::dirtyWidget_sys(const QRegion &rgn)
{
    Q_Q(QWidget);
    QWidget *tlw = q->window();

    QWidgetBackingStore *wbs = tlw->d_func()->topData()->backingStore;
    QRegion wrgn(rgn);

    if (tlw != q) {
        QPoint offs(q->mapTo(tlw, QPoint()));
        wrgn.translate(offs);
    }

    QWSWindowSurface *surface = static_cast<QWSWindowSurface*>(wbs->windowSurface);
    if (surface) {
        surface->setDirty(wrgn);


#ifdef Q_BACKINGSTORE_SUBSURFACES
        // dirty on all subsurfaces...

        QList<QWindowSurface*> subSurfaces = wbs->subSurfaces;
        // XXX: hw: only if region intersects any of the subsurfaces
        for (int i = 0; i < subSurfaces.size(); ++i) {
            QWSWindowSurface *s = static_cast<QWSWindowSurface*>(subSurfaces.at(i));
            QPoint p = s->window()->mapTo(tlw, QPoint()); // must use widget?
            s->setDirty(wrgn.translated(-p));
        }
#endif // Q_BACKINGSTORE_SUBSURFACES
    }
}

void QWidgetPrivate::cleanWidget_sys(const QRegion& rgn)
{
    Q_UNUSED(rgn);
}



/*
  Should we require that  q is a toplevel window ???

  Used by QWSManager
 */
void QWidgetPrivate::blitToScreen(const QRegion &globalrgn)
{
    Q_Q(QWidget);
    QWidget *win = q->window();
    QBrush bgBrush = win->palette().brush(win->backgroundRole());
    bool opaque = bgBrush.style() == Qt::NoBrush || bgBrush.isOpaque();
    QWidget::qwsDisplay()->repaintRegion(win->data->winid, win->windowFlags(), opaque, globalrgn);
}

void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);
    q->setAttribute(Qt::WA_Mapped);
    if (q->isWindow()) {

        if (QWindowSurface *surface = q->windowSurface()) {
            const QRect frameRect = q->frameGeometry();
            if (surface->geometry() != frameRect)
                surface->setGeometry(frameRect);
        }

        QRegion r = localRequestedRegion();
#ifndef QT_NO_QWS_MANAGER
        if (extra && extra->topextra && extra->topextra->qwsManager) {
            r.translate(data.crect.topLeft());
            r += extra->topextra->qwsManager->region();
            r.translate(-data.crect.topLeft());
        }
#endif
        data.fstrut_dirty = true;
        invalidateBuffer(r);
        if (!q->testAttribute(Qt::WA_ShowWithoutActivating)
            && q->windowType() != Qt::Popup
            && q->windowType() != Qt::Tool
            && q->windowType() != Qt::ToolTip) {
            QWidget::qwsDisplay()->requestFocus(data.winid,true);
        }
	bool staysontop =
            (q->windowFlags() & Qt::WindowStaysOnTopHint)
            || (q->windowState() & Qt::WindowFullScreen)
            || q->windowType() == Qt::Popup;
        QWSChangeAltitudeCommand::Altitude altitude;
        altitude = staysontop ? QWSChangeAltitudeCommand::StaysOnTop : QWSChangeAltitudeCommand::Raise;
        QWidget::qwsDisplay()->setAltitude(data.winid, altitude, true);
    }
#ifdef Q_BACKINGSTORE_SUBSURFACES
    else if (q->windowSurface()) {
        QWSWindowSurface *surface;
        surface = static_cast<QWSWindowSurface*>(q->windowSurface());
        const QPoint p = q->mapToGlobal(QPoint());
        surface->setGeometry(QRect(p, q->size()));
    }
#endif

    if (!q->window()->data->in_show) {
         invalidateBuffer(q->rect());
    }
}


void QWidgetPrivate::hide_sys()
{
    Q_Q(QWidget);
    deactivateWidgetCleanup();

    if (q->isWindow()) {
        q->releaseMouse();
//        requestWindowRegion(QRegion());

        extra->topextra->backingStore->releaseBuffer();


        QWidget::qwsDisplay()->requestFocus(data.winid,false);
    } else {
        QWidget *p = q->parentWidget();
        if (p &&p->isVisible()) {
            invalidateBuffer(q->rect());
        }
    }
}



static Qt::WindowStates effectiveState(Qt::WindowStates state)
 {
     if (state & Qt::WindowMinimized)
         return Qt::WindowMinimized;
     else if (state & Qt::WindowFullScreen)
         return Qt::WindowFullScreen;
     else if (state & Qt::WindowMaximized)
         return Qt::WindowMaximized;
     return Qt::WindowNoState;
 }

void QWidgetPrivate::setMaxWindowState_helper()
{
    // in_set_window_state is usually set in setWindowState(), but this
    // function is used in other functions as well
    // (e.g QApplicationPrivate::setMaxWindowRect())
    const uint old_state = data.in_set_window_state;
    data.in_set_window_state = 1;

#ifndef QT_NO_QWS_MANAGER
    if (extra && extra->topextra && extra->topextra->qwsManager)
        extra->topextra->qwsManager->maximize();
    else
#endif
    {
        Q_Q(QWidget);
        const QApplicationPrivate *ap = QApplicationPrivate::instance();
        const QRect maxWindowRect = ap->maxWindowRect(getScreen());
        q->setGeometry(maxWindowRect);
    }
    data.in_set_window_state = old_state;
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Q_D(QWidget);
    Qt::WindowStates oldstate = windowState();
    if (oldstate == newstate)
        return;
    if (isWindow() && !testAttribute(Qt::WA_WState_Created))
        create();

    data->window_state = newstate;
    data->in_set_window_state = 1;
    bool needShow = false;
    Qt::WindowStates newEffectiveState = effectiveState(newstate);
    Qt::WindowStates oldEffectiveState = effectiveState(oldstate);
    if (isWindow() && newEffectiveState != oldEffectiveState) {
        d->createTLExtra();
        if (oldEffectiveState == Qt::WindowNoState) { //normal
            d->topData()->normalGeometry = geometry();
        } else if (oldEffectiveState == Qt::WindowFullScreen) {
            setParent(0, d->topData()->savedFlags);
            needShow = true;
        } else if (oldEffectiveState == Qt::WindowMinimized) {
            needShow = true;
        }

        if (newEffectiveState == Qt::WindowMinimized) {
            //### not ideal...
            hide();
            needShow = false;
        } else if (newEffectiveState == Qt::WindowFullScreen) {
            d->topData()->savedFlags = windowFlags();
            setParent(0, Qt::FramelessWindowHint | (windowFlags() & Qt::WindowStaysOnTopHint));
            const QRect screen = qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(this));
            move(screen.topLeft());
            resize(screen.size());
            raise();
            needShow = true;
        } else if (newEffectiveState == Qt::WindowMaximized) {
            createWinId();
            d->setMaxWindowState_helper();
        } else { //normal
            QRect r = d->topData()->normalGeometry;
            if (r.width() >= 0) {
                d->topData()->normalGeometry = QRect(0,0,-1,-1);
                setGeometry(r);
            }
        }
    }
    data->in_set_window_state = 0;

    if (needShow)
        show();

    if (newstate & Qt::WindowActive)
        activateWindow();

    QWindowStateChangeEvent e(oldstate);
    QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::setFocus_sys()
{

}

void QWidgetPrivate::raise_sys()
{
    Q_Q(QWidget);
    //@@@ transaction
    if (q->isWindow()) {
        Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
        QWidget::qwsDisplay()->setAltitude(q->internalWinId(),
                                           QWSChangeAltitudeCommand::Raise);
        // XXX: subsurfaces?
#ifdef QT_NO_WINDOWGROUPHINT
#else
        QObjectList childObjects =  q->children();
        if (!childObjects.isEmpty()) {
            QWidgetList toraise;
            for (int i = 0; i < childObjects.size(); ++i) {
                QObject *obj = childObjects.at(i);
                if (obj->isWidgetType()) {
                    QWidget* w = static_cast<QWidget*>(obj);
                    if (w->isWindow())
                        toraise.append(w);
                }
            }

            for (int i = 0; i < toraise.size(); ++i) {
                QWidget *w = toraise.at(i);
                if (w->isVisible())
                    w->raise();
            }
        }
#endif // QT_NO_WINDOWGROUPHINT
    } else {
        setDirtyOpaqueRegion();
        q->update(q->rect());
    }
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    if (q->isWindow()) {
        Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));
        QWidget::qwsDisplay()->setAltitude(data.winid,
                                           QWSChangeAltitudeCommand::Lower);
    } else if (QWidget *p = q->parentWidget()) {
        setDirtyOpaqueRegion();
        p->d_func()->invalidateBuffer(q->geometry());
    }
}

void QWidgetPrivate::stackUnder_sys(QWidget*)
{
    Q_Q(QWidget);
    if (QWidget *p = q->parentWidget()) {
        setDirtyOpaqueRegion();
        p->d_func()->invalidateBuffer(q->geometry());
    }
}

void QWidgetPrivate::moveSurface(QWindowSurface *surface, const QPoint &offset)
{
    QWSWindowSurface *s = static_cast<QWSWindowSurface*>(surface);
    if (!s->move(offset))
        s->invalidateBuffer();

    QWSDisplay::instance()->moveRegion(s->winId(), offset.x(), offset.y());
}

void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    Q_Q(QWidget);
    if (extra) {                                // any size restrictions?
        w = qMin(w,extra->maxw);
        h = qMin(h,extra->maxh);
        w = qMax(w,extra->minw);
        h = qMax(h,extra->minh);
    }

    QPoint oldp = q->geometry().topLeft();
    QSize olds = q->size();
    QRect r(x, y, w, h);

    bool isResize = olds != r.size();
    isMove = oldp != r.topLeft(); //### why do we have isMove as a parameter?

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if (r.size() == olds && oldp == r.topLeft())
        return;

    if (!data.in_set_window_state) {
        q->data->window_state &= ~Qt::WindowMaximized;
        q->data->window_state &= ~Qt::WindowFullScreen;
        if (q->isWindow())
            topData()->normalGeometry = QRect(0, 0, -1, -1);
    }
    QPoint oldPos = q->pos();
    data.crect = r;

    if ((q->windowType() == Qt::Desktop))
        return;

    bool inTransaction = false;
    QTLWExtra *topextra = q->window()->d_func()->maybeTopData();
    if (topextra) {
        inTransaction = topextra->inPaintTransaction;
        topextra->inPaintTransaction = true;
    }
    if (q->isVisible()) {

        bool toplevelMove = false;
        QWSWindowSurface *surface = 0;

        if (q->isWindow()) {
            //### ConfigPending not implemented, do we need it?
            //setAttribute(Qt::WA_WState_ConfigPending);
            const QWidgetBackingStore *bs = maybeBackingStore();
            if (bs)
                surface = static_cast<QWSWindowSurface*>(bs->windowSurface);
            if (isMove && !isResize && surface) {
                const QPoint offset(x - oldp.x(), y - oldp.y());
                moveSurface(surface, offset);
                toplevelMove = true; //server moves window, but we must send moveEvent, which might trigger painting

#ifdef Q_BACKINGSTORE_SUBSURFACES
                QList<QWindowSurface*> surfaces = bs->subSurfaces;
                for (int i = 0; i < surfaces.size(); ++i)
                    moveSurface(surfaces.at(i), offset);
#endif
            } else {
                    updateFrameStrut();
            }
        }


        //### must have frame geometry correct before sending move/resize events
        if (isMove) {
            QMoveEvent e(q->pos(), oldPos);
            QApplication::sendEvent(q, &e);
        }
        if (isResize) {
            QResizeEvent e(r.size(), olds);
            QApplication::sendEvent(q, &e);
        }
        if (!toplevelMove) {
            if (q->isWindow()) {
                if (surface)
                    surface->setGeometry(q->frameGeometry());
                else
                    invalidateBuffer(q->rect()); //###

#ifdef Q_BACKINGSTORE_SUBSURFACES
                // XXX: should not resize subsurfaces. Children within a layout
                // will be resized automatically while children with a static
                // geometry should get a new clip region instead.
                const QRect clipRect = q->geometry();
                QWidgetBackingStore *bs = maybeBackingStore();
                QList<QWindowSurface*> surfaces = bs->subSurfaces;
                for (int i = 0; i < surfaces.size(); ++i) {
                    QWSWindowSurface *s = static_cast<QWSWindowSurface*>(surfaces.at(i));
                    QRect srect = s->geometry();
                    s->setGeometry(clipRect & srect);
                }
#endif
            }
#ifdef Q_BACKINGSTORE_SUBSURFACES
            // XXX: merge this case with the isWindow() case
            else if (maybeTopData() && maybeTopData()->windowSurface) {
                QWSWindowSurface *surface;
                surface = static_cast<QWSWindowSurface*>(q->windowSurface());
                if (isMove && !isResize) {
                    moveSurface(surface, QPoint(x - oldp.x(), y - oldp.y()));
                } else {
                    const QPoint p = q->mapToGlobal(QPoint());
                    surface->setGeometry(QRect(p, QSize(w, h)));
                }
            }
#endif
            else {
                if (isMove && !isResize) {
                    moveRect(QRect(oldPos, olds), x - oldPos.x(), y - oldPos.y());
                } else {
                    q->parentWidget()->d_func()->invalidateBuffer(QRect(oldPos, olds));
                    invalidateBuffer(q->rect());
                    //TODO: handle static contents
                }
            }
        }
    } else { // not visible
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }
    if (topextra)
        topextra->inPaintTransaction = inTransaction;
}

void QWidgetPrivate::setConstraints_sys()
{
}

QScreen* QWidgetPrivate::getScreen() const
{
    Q_Q(const QWidget);

    const QList<QScreen*> subScreens = qt_screen->subScreens();
    if (subScreens.isEmpty())
        return qt_screen;

    const int screen = QApplication::desktop()->screenNumber(q);

    return qt_screen->subScreens().at(screen < 0 ? 0 : screen);
}

void QWidgetPrivate::scroll_sys(int dx, int dy)
{
    Q_Q(QWidget);
    scrollChildren(dx, dy);
    scrollRect(q->rect(), dx, dy);
}

void QWidgetPrivate::scroll_sys(int dx, int dy, const QRect &r)
{
    scrollRect(r, dx, dy);
}

int QWidget::metric(PaintDeviceMetric m) const
{
    Q_D(const QWidget);

    int val;
    if (m == PdmWidth) {
        val = data->crect.width();
    } else if (m == PdmWidthMM) {
        const QScreen *screen = d->getScreen();
        val = data->crect.width() * screen->physicalWidth() / screen->width();
    } else if (m == PdmHeight) {
        val = data->crect.height();
    } else if (m == PdmHeightMM) {
        const QScreen *screen = d->getScreen();
        val = data->crect.height() * screen->physicalHeight() / screen->height();
    } else if (m == PdmDepth) {
        return qwsDisplay()->depth();
    } else if (m == PdmDpiX || m == PdmPhysicalDpiX) {
        const QScreen *screen = d->getScreen();
        return qRound(screen->width() / double(screen->physicalWidth() / 25.4));
    } else if (m == PdmDpiY || m == PdmPhysicalDpiY) {
        const QScreen *screen = d->getScreen();
        return qRound(screen->height() / double(screen->physicalHeight() / 25.4));
    } else {
        val = QPaintDevice::metric(m);// XXX
    }
    return val;
}

void QWidgetPrivate::createSysExtra()
{
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->backingStore = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    delete extra->topextra->backingStore;
    extra->topextra->backingStore = 0;
}

void QWidgetPrivate::registerDropSite(bool on)
{
    Q_UNUSED(on);
}

QRegion QWidgetPrivate::localRequestedRegion() const
{
    Q_Q(const QWidget);
    QRegion r(q->rect());
    if (extra && !extra->mask.isEmpty())
        r &= extra->mask;

    return r;
}

QRegion QWidgetPrivate::localAllocatedRegion() const
{
    Q_Q(const QWidget);

    QWidgetBackingStore *wbs = q->window()->d_func()->maybeBackingStore();

    QWindowSurface *ws = wbs ? wbs->windowSurface : 0;
    if (!ws)
        return QRegion();
    QRegion r = static_cast<QWSWindowSurface*>(ws)->clipRegion();
    if (!q->isWindow()) {
        QPoint off = q->mapTo(q->window(), QPoint());
        r &= localRequestedRegion().translated(off);
        r.translate(-off);
    }
    return r;
}

inline bool QRect::intersects(const QRect &r) const
{
    return (qMax(x1, r.x1) <= qMin(x2, r.x2) &&
             qMax(y1, r.y1) <= qMin(y2, r.y2));
}

void QWidget::setMask(const QRegion& region)
{
    Q_D(QWidget);
    d->createExtra();

    if (region == d->extra->mask)
        return;

    QRegion parentR;
    if (!isWindow())
        parentR = d->extra->mask.isEmpty() ? QRegion(rect()) : d->extra->mask ;

    d->extra->mask = region;

    if (isVisible()) {
        if (isWindow()) {
            d->data.fstrut_dirty = true;
            d->invalidateBuffer(rect());
            QWindowSurface *surface = d->extra->topextra->backingStore->windowSurface;
            if (surface) {
                // QWSWindowSurface::setGeometry() returns without doing anything
                // if old geom  == new geom. Therefore, we need to reset the old value.
                surface->QWindowSurface::setGeometry(QRect());
                surface->setGeometry(frameGeometry());
            }
        } else {
            parentR += d->extra->mask;
            parentWidget()->update(parentR.translated(geometry().topLeft()));
            update();
        }
    }
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
    setMask(QRegion());
}

void QWidgetPrivate::updateFrameStrut()
{
    Q_Q(QWidget);

    if(!q->isVisible() || (q->windowType() == Qt::Desktop)) {
        data.fstrut_dirty = q->isVisible();
        return;
    }

#ifndef QT_NO_QWS_MANAGER
    if (extra && extra->topextra && extra->topextra->qwsManager) {
        QTLWExtra *topextra = extra->topextra;
        const QRect oldFrameStrut = topextra->frameStrut;
        const QRect contents = data.crect;
        QRegion r = localRequestedRegion().translated(contents.topLeft());
        r += extra->topextra->qwsManager->region();
        const QRect frame = r.boundingRect();

        topextra->frameStrut.setCoords(contents.left() - frame.left(),
                                       contents.top() - frame.top(),
                                       frame.right() - contents.right(),
                                       frame.bottom() - contents.bottom());
        topextra->qwsManager->repaintRegion(QDecoration::All, QDecoration::Normal);
    }
#endif
    data.fstrut_dirty = false;
}

#ifndef QT_NO_CURSOR
void QWidgetPrivate::updateCursor() const
{
    Q_Q(const QWidget);

    if (QApplication::overrideCursor())
        return;

    if (qt_last_x
        && (!QWidget::mouseGrabber() || QWidget::mouseGrabber() == q)
        && qt_last_cursor != (WId)q->cursor().handle())
    {
        const QPoint pos(*qt_last_x, *qt_last_y);
        const QPoint offset = q->mapToGlobal(QPoint());
        if (!localAllocatedRegion().contains(pos - offset))
            return;

        const QWidget *w = q->childAt(q->mapFromGlobal(pos));
        if (!w || w->cursor().handle() == q->cursor().handle())
            QWidget::qwsDisplay()->selectCursor(const_cast<QWidget*>(q),
                                                q->cursor().handle());
    }
}
#endif


void QWidget::setWindowOpacity(qreal level)
{
    if (!isWindow())
        return;

    Q_D(QWidget);
    level = qBound(qreal(0), level, qreal(1));
    uchar opacity = uchar(level * 255);
    d->topData()->opacity = opacity;
    d->createWinId();
    qwsDisplay()->setOpacity(data->winid, opacity);
}

qreal QWidget::windowOpacity() const
{
    return isWindow() ? ((QWidget*)this)->d_func()->topData()->opacity / 255.0 : 1.0;
}

//static QSingleCleanupHandler<QWSPaintEngine> qt_paintengine_cleanup_handler;
//static QWSPaintEngine *qt_widget_paintengine = 0;
/*!
    Returns the widget's paint engine.
*/
QPaintEngine *QWidget::paintEngine() const
{
    qWarning("QWidget::paintEngine: Should no longer be called");
    return 0; //##### @@@
//     if (!qt_widget_paintengine) {
//         qt_widget_paintengine = new QRasterPaintEngine();
//         qt_paintengine_cleanup_handler.set(&qt_widget_paintengine);
//     }
//     if (qt_widget_paintengine->isActive()) {
//         if (d->extraPaintEngine)
//             return d->extraPaintEngine;
//         const_cast<QWidget *>(this)->d_func()->extraPaintEngine = new QRasterPaintEngine();
//         return d->extraPaintEngine;
//     }
//    return qt_widget_paintengine;
}

void QWidgetPrivate::setModal_sys()
{
}

