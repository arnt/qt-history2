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
#include "qscreen_qws.h"
#include "qwsmanager_qws.h"
#include <private/qwsmanager_p.h>
//#include "qwsregionmanager_qws.h"
#include "qpaintengine.h"

#include <qgfxraster_qws.h>


#include "qdebug.h"

#include "qwidget_p.h"
#include "qwidget_qws_p.h"


#include <sys/types.h>
#include <sys/shm.h>

extern int *qt_last_x;
extern int *qt_last_y;
extern WId qt_last_cursor;
extern bool qws_overrideCursor;
extern QWidget *qt_pressGrab;
extern QWidget *qt_mouseGrb;

extern QRect qt_maxWindowRect;

extern bool qt_override_paint_on_screen;

static QWidget *keyboardGrb = 0;

static int takeLocalId()
{
    static int n=-1000;
    return --n;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool /*destroyOldWindow*/)
{
    Q_Q(QWidget);
    Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
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

    bool customize =  (flags & (
                                Qt::X11BypassWindowManagerHint
                                | Qt::FramelessWindowHint
                                | Qt::WindowTitleHint
                                | Qt::WindowSystemMenuHint
                                | Qt::WindowMinimizeButtonHint
                                | Qt::WindowMaximizeButtonHint
                                | Qt::WindowContextHelpButtonHint
                                ));

    // a popup stays on top
    if (popup)
        flags |= Qt::WindowStaysOnTopHint;

    WId           id;
    QWSDisplay* dpy = q->qwsDisplay();

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
    } else if (topLevel) {                        // calc pos/size from screen
        data.crect.setRect(0, 0, sw/2, 4*sh/10);
    } else {                                        // child widget
        data.crect.setRect(0, 0, 100, 30);
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
        if (desktop || popup || tool) {
            hasFrame = false;
        } else if (customize) {
            hasFrame = !(flags & Qt::FramelessWindowHint);
        } else if (dialog) {
            flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowContextHelpButtonHint;
        } else {
            flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;
        }
    } else  if (!customize) {
        flags |= Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint;
    }

    q->setAttribute(Qt::WA_MouseTracking, true);
    q->setMouseTracking(false);                        // also sets event mask
    if (desktop) {
        q->setAttribute(Qt::WA_WState_Visible);
    } else if (topLevel) {                        // set X cursor
        //QCursor *oc = QApplication::overrideCursor();
        if (initializeWindow) {
            //XXX XDefineCursor(dpy, winid, oc ? oc->handle() : cursor().handle());
        }
        q->setAttribute(Qt::WA_SetCursor);
#ifndef QT_NO_WIDGET_TOPEXTRA
        q->qwsDisplay()->nameRegion(q->winId(), q->objectName(), q->windowTitle());
#else
        q->qwsDisplay()->nameRegion(q->winId(), q->objectName(), QString::null);
#endif
    }

    if (topLevel) {
#ifndef QT_NO_WIDGET_TOPEXTRA
        createTLExtra();
#endif
#ifndef QT_NO_QWS_MANAGER
        if (hasFrame) {
            // get size of wm decoration and make the old crect the new frect
            QRect cr = data.crect;
            QRegion r = QApplication::qwsDecoration().region(q, cr);
            QRect br(r.boundingRect());
            extra->topextra->fleft = cr.x() - br.x();
            extra->topextra->ftop = cr.y() - br.y();
            extra->topextra->fright = br.right() - cr.right();
            extra->topextra->fbottom = br.bottom() - cr.bottom();
            data.crect.adjust(extra->topextra->fleft, extra->topextra->ftop,
                                  -extra->topextra->fright, -extra->topextra->fbottom);
            topData()->qwsManager = new QWSManager(q);
        } else if (topData()->qwsManager) {
            delete topData()->qwsManager;
            topData()->qwsManager = 0;
            data.crect.translate(-extra->topextra->fleft, -extra->topextra->ftop);
            extra->topextra->fleft = extra->topextra->ftop =
                extra->topextra->fright = extra->topextra->fbottom = 0;
        }
#endif
        // declare the widget's object name as window role

        qt_fbdpy->addProperty(id,QT_QWS_PROPERTY_WINDOWNAME);
        qt_fbdpy->setProperty(id,QT_QWS_PROPERTY_WINDOWNAME,0,q->objectName().toLatin1());

        //XXX If we are session managed, inform the window manager about it
    } else {
        if (extra && extra->topextra)        { // already allocated due to reparent?
            extra->topextra->fleft = 0;
            extra->topextra->ftop = 0;
            extra->topextra->fright = 0;
            extra->topextra->fbottom = 0;
        }
        //updateRequestedRegion(mapToGlobal(QPoint(0,0)));
    }
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    Q_D(QWidget);
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
        if ((windowType() == Qt::Desktop)) {
        } else {
            if (parentWidget() && parentWidget()->testAttribute(Qt::WA_WState_Created)) {
                d->hide_sys();
            }
            if (destroyWindow && isWindow())
                qwsDisplay()->destroyRegion(winId());
        }
        d->setWinId(0);
    }
}


void QWidgetPrivate::setParent_sys(QWidget *newparent, Qt::WFlags f)
{
    Q_Q(QWidget);
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
    bool     enable = q->isEnabled();                // remember status
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize    s            = q->size();
    //QBrush   bgc    = background();                        // save colors
#ifndef QT_NO_WIDGET_TOPEXTRA
    QString capt = q->windowTitle();
#endif
    data.window_flags = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
    q->create();
    if (q->isWindow() || (!newparent || newparent->isVisible()))
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setGeometry(0, 0, s.width(), s.height());
    q->setEnabled(enable);
    q->setFocusPolicy(fp);
#ifndef QT_NO_WIDGET_TOPEXTRA
    if (!capt.isNull()) {
        extra->topextra->caption.clear();
        q->setWindowTitle(capt);
    }
#endif
    if ((int)old_winid > 0)
        q->qwsDisplay()->destroyRegion(old_winid);
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
#ifndef QT_NO_QWS_IM
    if (text) {
        QWidget *tlw = window();
        int winid = tlw->winId();
        QPoint p(x, y + height);
        QPoint gp = mapToGlobal(p);

        QRect r = QRect(mapToGlobal(QPoint(0,0)),
                         size());

        r.setBottom(tlw->geometry().bottom());

        //qDebug("QWidget::setMicroFocusHint %d %d %d %d", r.x(),
        //        r.y(),  r.width(), r.height());
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

void QWidget::setCursor(const QCursor &cursor)
{
    Q_D(QWidget);
    d->createExtra();
    delete d->extra->curs;
    d->extra->curs = new QCursor(cursor);
    setAttribute(Qt::WA_SetCursor);
//    if (isVisible())
//        d->updateCursor(d->paintableRegion());
    //@@@@@@ cursor stuff
}

void QWidget::unsetCursor()
{
    Q_D(QWidget);
    if (d->extra) {
        delete d->extra->curs;
        d->extra->curs = 0;
    }
    setAttribute(Qt::WA_SetCursor, false);
//    if (isVisible())
//        d->updateCursor(d->paintableRegion());
    //@@@@@@ cursor stuff
}
#endif //QT_NO_CURSOR

#ifndef QT_NO_WIDGET_TOPEXTRA
void QWidgetPrivate::setWindowTitle_sys(const QString &caption)
{
    Q_Q(QWidget);
    q->qwsDisplay()->setWindowCaption(q, caption);
}
#endif

#ifndef QT_NO_WIDGET_TOPEXTRA
void QWidgetPrivate::setWindowIcon_sys()
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
#ifndef QT_NO_IMAGE_SMOOTHSCALE
            QPixmap::fromImage(unscaledIcon.scale(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
#else
            QPixmap::fromImage(unscaledIcon);
#endif
        x->icon = new QPixmap(pixmap);
        mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
    }
#endif
}


void QWidget::setWindowIconText(const QString &iconText)
{
    Q_D(QWidget);
    d->createTLExtra();
    d->extra->topextra->iconText = iconText;
    QEvent e(QEvent::IconTextChange);
    QApplication::sendEvent(this, &e);
}
#endif

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
        qwsDisplay()->requestFocus(tlw->winId(), true);
    }
}


void QWidget::update()
{
    if (isVisible() && updatesEnabled())
        QApplication::postEvent(this, new QWSUpdateEvent(visibleRegion()));
}

void QWidget::update(const QRegion &rgn)
{
    if (isVisible() && updatesEnabled())
         QApplication::postEvent(this, new QWSUpdateEvent(rgn & visibleRegion()));
}

void QWidget::update(const QRect &r)
{
    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
    if (w && h && isVisible() && updatesEnabled()) {
        if (w < 0)
            w = data->crect.width()  - x;
        if (h < 0)
            h = data->crect.height() - y;
        if (w != 0 && h != 0)
            QApplication::postEvent(this,
                    new QWSUpdateEvent(visibleRegion().intersect(QRect(x, y, w, h))));
    }
}


/*
  Should we require that  q is a toplevel window ???
 */
void QWidgetPrivate::bltToScreen(const QRegion &globalrgn)
{
    Q_Q(QWidget);
//    qDebug("QWidgetPrivate::bltToScreen");
    QWidget *win = q->window();
    QBrush bgBrush = win->palette().brush(win->backgroundRole());
    bool opaque = bgBrush.style() == Qt::NoBrush || bgBrush.isOpaque();
    q->qwsDisplay()->repaintRegion(win->data->winid, opaque, globalrgn);
}


/*
  rgn is in parent's coordinates (same as geometry())
 */
void QWidgetPrivate::paintHierarchy(const QRegion &rgn)
{
    Q_Q(QWidget);
#if 0 //DEBUG
    static bool painting = false;
    bool outermost = !painting;
    if (outermost)
        qDebug(">>>>> paintHierarchy %p START", q);
    painting = true;
#endif

    QRegion myrgn(rgn & q->geometry());
    if (myrgn.isEmpty())
        return;

    myrgn.translate(-q->geometry().topLeft());
    doPaint(myrgn);
    for (int i = 0; i < children.size(); ++i) {
        register QObject *obj=children.at(i);
        if (obj->isWidgetType()) {
            QWidget* w = static_cast<QWidget*>(obj);
            if (w->isVisible() && !w->isWindow())
                w->d_func()->paintHierarchy(myrgn);
        }
    }
#if 0 //DEBUG
    if (outermost) {
        qDebug("<<<<<< paintHierarchy %p END", q);
        painting = false;
    }
#endif
}



/*
  rgn is in my coordinates (same as rect())
*/

void QWidget::repaint(const QRegion& rgn)
{
    if (!isVisible() || !updatesEnabled() || !testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;
    QRegion globalrgn = rgn;
    QPoint globalPos = mapToGlobal(QPoint(0,0));
    globalrgn.translate(globalPos);

    QWSBackingStore *bs = window()->d_func()->extra->topextra->backingStore;
    bs->lock(true);
    window()->d_func()->paintHierarchy(globalrgn); //optimizable...
    bs->unlock();
    window()->d_func()->bltToScreen(globalrgn);
}

/*
  rgn is in my coordinates (same as rect())
*/

void QWidgetPrivate::doPaint(const QRegion &rgn)
{

//    qDebug("doPaint %p child of %p", q, q->parentWidget());
    Q_Q(QWidget);

    if (q->testAttribute(Qt::WA_WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");

    q->setAttribute(Qt::WA_WState_InPaintEvent);

    QWidget *tlw = q->window();
    QTLWExtra *topextra = tlw->d_func()->extra->topextra;
    if (!topextra || !topextra->backingStore) {
        qWarning("QWidgetPrivate::doPaint no backingStore");
        return;
    }
    QPixmap *bs = topextra->backingStore->pixmap();
    QPoint redirectionOffset = topextra->backingStoreOffset + q->mapFrom(tlw,QPoint(0,0));

    QPainter::setRedirected(q, bs, redirectionOffset);

    QRegion clipRegion(rgn);
    clipRegion.translate(-redirectionOffset);
    bs->paintEngine()->setSystemClip(clipRegion);

    if (!q->testAttribute(Qt::WA_NoBackground) && !q->testAttribute(Qt::WA_NoSystemBackground)) {
        const QPalette &pal = q->palette();
        QPalette::ColorRole bg = q->backgroundRole();
        QBrush bgBrush = pal.brush(bg);
        //##### put in an isBackgroundSpecified() function ???
        bool hasBackground = (q->isWindow() || q->windowType() == Qt::SubWindow)
                             || (bg_role != QPalette::NoRole || (pal.resolve() & (1<<bg))) ;

        if (hasBackground && bgBrush.style() != Qt::NoBrush) {
            QPainter p(q); // We shall use it only once
            if (q->isWindow())
                p.setCompositionMode(QPainter::CompositionMode_Source); //copy alpha straight in
            p.fillRect(q->rect(), bgBrush);
        }
    }
    // Send paint event to self
    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent(q, &e);

    // Clear the clipping again
    bs->paintEngine()->setSystemClip(QRegion());

    QPainter::restoreRedirected(q);

    // Clean out the temporary engine if used...
    if (extraPaintEngine) {
        delete extraPaintEngine;
        extraPaintEngine = 0;
    }

    q->setAttribute(Qt::WA_WState_InPaintEvent, false);

    if(!q->testAttribute(Qt::WA_PaintOutsidePaintEvent) && q->paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");
}

//#define QT_SHAREDMEM_DEBUG

QWSBackingStore::QWSBackingStore()
    :pix(0), shmid(-1), shmaddr(0)
{
}

QWSBackingStore::~QWSBackingStore()
{
    detach();
}

QSize QWSBackingStore::size() const
{
    return pix ? pix->size() : QSize(0,0);
}

QPixmap *QWSBackingStore::pixmap()
{
    return pix;
}
void QWSBackingStore::detach()
{
#ifdef QT_SHAREDMEM_DEBUG
    qDebug() << "QWSBackingStore::detach shmid" << shmid << "shmaddr" << shmaddr;
#endif
    delete pix;
    pix =0;
    if (shmid != -1) {
        //we don't IPC_RMID it here; we do it in attach() instead. Otherwise the server could get a
        //region event with a shmid that's already deleted
        shmdt(shmaddr);
        shmid = -1;
        shmaddr = 0;
    }
}

void QWSBackingStore::create(QSize s)
{
    if (size() == s) {
        return;
    }
    detach();
    pix = new QPixmap;
    if (s.isNull()) {
#ifdef QT_SHAREDMEM_DEBUG
        qDebug() << "QWSBackingStore::create null size";
#endif
        return;
    }
    int extradatasize = 0;//2 * sizeof(int); //store height and width ???
    int datasize = 4 * s.width() * s.height() + extradatasize;
    shmid = shmget(IPC_PRIVATE, datasize,  IPC_CREAT|0600);
    shmaddr = shmat(shmid,0,0);
    QImage img(static_cast<uchar*>(shmaddr)+extradatasize, s.width(), s.height(),
               QImage::Format_ARGB32_Premultiplied);
    *pix = QPixmap::fromImage(img);
#ifdef QT_SHAREDMEM_DEBUG
    qDebug() << "QWSBackingStore::create size" << s << "shmid" << shmid << "shmaddr" << shmaddr;
#endif
}

void QWSBackingStore::attach(int id, QSize s)
{
    if (shmid == id && s == size())
        return;
    detach();
    shmid = id;
    pix = new QPixmap;
    if (id == -1) {
#ifdef QT_SHAREDMEM_DEBUG
        qDebug() << "QWSBackingStore::attach no id, size" << s;
#endif
        return;
    }
    shmaddr = shmat(shmid,0,0);
    if (shmaddr == (void*)-1) {
#ifdef QT_SHAREDMEM_DEBUG
        qDebug() << "QWSBackingStore::attach COULD NOT ATTACH TO SHARED MEMORY size" << s << "shmid" << shmid;
#endif
        return;
    }
    // IPC_RMID only marks for deletion, so we know that the segment will be removed on exit
    // we do it in attach instead of create in order to allow on-demand (single use) backing store
    shmctl(shmid, IPC_RMID, 0);

    int extradatasize = 0;
    QImage img(static_cast<uchar*>(shmaddr)+extradatasize, s.width(), s.height(),
               QImage::Format_ARGB32_Premultiplied);
    *pix = QPixmap::fromImage(img);
#ifdef QT_SHAREDMEM_DEBUG
    qDebug() << "QWSBackingStore::attach size" << s << "shmid" << shmid << "shmaddr" << shmaddr;
#endif
}


// TODO: one lock per buffer (or client)
// also: no need to lock in server
void QWSBackingStore::lock(bool write)
{
    QWSDisplay::grab(write);
}

void QWSBackingStore::unlock()
{
    QWSDisplay::ungrab();
}

void QWidgetPrivate::requestWindowRegion(const QRegion &r)
{
    Q_Q(QWidget);
    QRegion deviceregion = qt_screen->mapToDevice(r, QSize(qt_screen->width(), qt_screen->height()));
    Q_ASSERT(extra && extra->topextra);
    QRect br = r.boundingRect();
    if (!extra->topextra->backingStore)
        extra->topextra->backingStore = new QWSBackingStore;

    QWSBackingStore *bs = extra->topextra->backingStore;

    if (bs->size() != br.size()) {
        bs->create(br.size());
    }
    extra->topextra->backingStoreOffset = br.topLeft() - q->geometry().topLeft();
    paintHierarchy(r);
#ifndef QT_NO_QWS_MANAGER
    if (extra && extra->topextra && extra->topextra->qwsManager) {
        extra->topextra->qwsManager->d_func()->doPaint();
    }
#endif

    QBrush bgBrush = q->palette().brush(q->backgroundRole());
    bool opaque = bgBrush.style() == Qt::NoBrush || bgBrush.isOpaque(); //### duplicated in bltToScreen


    q->qwsDisplay()->requestRegion(data.winid, bs->memoryId(), opaque, deviceregion);
}

void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);
    if (q->isWindow()) {
        QRegion r = localRequestedRegion();
        r.translate(data.crect.topLeft());
#ifndef QT_NO_QWS_MANAGER
        if (extra && extra->topextra && extra->topextra->qwsManager) {
            r += extra->topextra->qwsManager->region();
        }
#endif
        requestWindowRegion(r);
        if (q->windowType() != Qt::Popup
            && q->windowType() != Qt::Tool
            && q->windowType() != Qt::ToolTip ) {
            q->qwsDisplay()->requestFocus(data.winid,true);
        }
        q->qwsDisplay()->setAltitude(data.winid,
                                     (q->windowFlags() & Qt::WindowStaysOnTopHint) ? 1 : 0, true);

    } else if (!q->window()->data->in_show) {
        q->update(); //#####@@@@@@
    }
}


void QWidgetPrivate::hide_sys()
{
    Q_Q(QWidget);
    deactivateWidgetCleanup();

    if (q->isWindow()) {
        q->releaseMouse();
        requestWindowRegion(QRegion());
        q->qwsDisplay()->requestFocus(data.winid,false);
    } else {
        QWidget *p = q->parentWidget();
        if (p &&p->isVisible()) {
            p->update(q->geometry()); //@@@ ???
        }
    }
}


void QWidget::setWindowState(Qt::WindowStates newstate)
{
    Q_D(QWidget);
    Qt::WindowStates oldstate = windowState();
    if (oldstate == newstate)
        return;
    data->window_state = newstate;
    data->in_set_window_state = 1;

    bool needShow = false;
    if (isWindow() && newstate != oldstate) {
        d->createTLExtra();
        if (oldstate == Qt::WindowNoState) { //normal
            d->topData()->normalGeometry = geometry();
        } else if (oldstate & Qt::WindowFullScreen) {
            setParent(0, d->topData()->savedFlags);
            needShow = true;
        } else if (oldstate & Qt::WindowMinimized) {
            needShow = true;
        }

        if (newstate & Qt::WindowMinimized) {
            //### not ideal...
            hide();
            needShow = false;
        } else if (newstate & Qt::WindowFullScreen) {
            d->topData()->savedFlags = windowFlags();
            setParent(0, Qt::FramelessWindowHint);
            const QRect screen = qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(this));
            move(screen.topLeft());
            resize(screen.size());
            raise();
            needShow = true;
        } else if (newstate & Qt::WindowMaximized) {
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager)
                d->extra->topextra->qwsManager->maximize();
            else
#endif
                setGeometry(qt_maxWindowRect);
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

void QWidgetPrivate::raise_sys()
{
    Q_Q(QWidget);
    //@@@ transaction
    if (q->isWindow()) {
#ifdef QT_NO_WINDOWGROUPHINT
        qwsDisplay()->setAltitude(q->winId(), 0);
#else
        q->qwsDisplay()->setAltitude(q->winId(), 0);

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
    } else if (QWidget *p = q->parentWidget()) {
        p->update(q->geometry());
    }
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    if (q->isWindow()) {
        q->qwsDisplay()->setAltitude(data.winid, -1);
    } else if (QWidget *p = q->parentWidget()) {
        p->update(q->geometry());
    }
}

void QWidgetPrivate::stackUnder_sys(QWidget* w)
{
    Q_Q(QWidget);
    if (QWidget *p = q->parentWidget()) {
        p->update(q->geometry());
    }
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
    if (q->isWindow()) {
        w = qMax(1, w);
        h = qMax(1, h);
    }

    QPoint oldp = q->geometry().topLeft();
    QSize olds = q->size();
    QRect r(x, y, w, h);

    bool isResize = olds != r.size();
    isMove = oldp != r.topLeft(); //###### why do we have isMove as a parameter ??????

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

    QTLWExtra *topextra = q->window()->d_func()->extra->topextra;
    bool inTransaction = topextra->inPaintTransaction;
    topextra->inPaintTransaction = true;

    if (q->isVisible()) {

        QRegion myregion;
        bool toplevelMove = false;
        if (q->isWindow()) {
            //### ConfigPending not implemented, do we need it?
            //setAttribute(Qt::WA_WState_ConfigPending);
            if (isMove && !isResize) {
                q->qwsDisplay()->moveRegion(data.winid, x - oldp.x(), y - oldp.y());
                toplevelMove = true; //server moves window, but we must send moveEvent, which might trigger painting
            } else {
                myregion = localRequestedRegion();
                myregion.translate(x,y);
#ifndef QT_NO_QWS_MANAGER
                if (topextra && topextra->qwsManager) {
                    myregion += topextra->qwsManager->region();
                }
#endif
                if (topextra) {
                    QRect br(myregion.boundingRect());
                    topextra->fleft = data.crect.x()-br.x();
                    topextra->ftop = data.crect.y()-br.y();
                    topextra->fright = br.right()-data.crect.right();
                    topextra->fbottom = br.bottom()-data.crect.bottom();
                }
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
                requestWindowRegion(myregion); //paints to BS ### should it ???
                topextra->dirtyRegion |= myregion;
            } else {
                QWidget *p = q->parentWidget();
                QRegion dirty(QRect(p->mapToGlobal(data.crect.topLeft()), data.crect.size()));
                dirty |= QRect(p->mapToGlobal(oldp), olds);
                topextra->dirtyRegion |= dirty;
            }
        }

        if (!inTransaction && !topextra->dirtyRegion.isEmpty()) {
            if (!q->isWindow()) {
                topextra->backingStore->lock(true);
                q->window()->d_func()->paintHierarchy(topextra->dirtyRegion);
                topextra->backingStore->unlock();
            }
            bltToScreen(topextra->dirtyRegion);
            topextra->dirtyRegion = QRegion();
        }
    } else { // not visible
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }

    if (!inTransaction)
        topextra->inPaintTransaction = false;
}

/*
//##### #define FAST_WIDGET_MOVE
#ifdef FAST_WIDGET_MOVE
//                    if (isMove && (w==olds.width() && h==olds.height())) {
//                        QSize s(qt_screen->width(), qt_screen->height());
//
//                        QPoint td1 = qt_screen->mapToDevice(QPoint(0,0), s);
//                        QPoint td2 = qt_screen->mapToDevice(QPoint(x - oldp.x(),y - oldp.y()), s);
//                        QPoint dd = QPoint(td2.x()-td1.x(), td2.y()-td1.y());
//                        oldAlloc.translate(dd.x(), dd.y());
//
//                        QRegion alloc(allocatedRegion());
//
//                        QRegion scrollRegion(alloc & oldAlloc);
//                        if (!scrollRegion.isEmpty()) {
//                            bool was_unclipped = p->testAttribute(Qt::WA_PaintUnclipped);
//                            p->setAttribute(Qt::WA_PaintUnclipped);
//
//                            QWSPaintEngine * engine=static_cast<QWSPaintEngine*>(p->paintEngine());
//                            engine->begin(p);
//
//                            engine->setClipDeviceRegion(scrollRegion);
//                            engine->scroll(x,y,w,h,oldp.x(),oldp.y());
//                            engine->end();
//                            if (!was_unclipped)
//                                p->setAttribute(Qt::WA_PaintUnclipped,false);
//
//                            QSize ds(qt_screen->deviceWidth(), qt_screen->deviceHeight());
//                            scrollRegion = qt_screen->mapFromDevice(scrollRegion, ds);
//                            QPoint gp = p->mapToGlobal(QPoint(0,0));
//                            scrollRegion.translate(-gp.x(), -gp.y());
//                            paintRegion -= scrollRegion;
//                        }
//                    }
#endif
*/

void QWidgetPrivate::setConstraints_sys()
{
}


void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    //@@@@
    qDebug("QWidget::scroll ### not implemented ###");
    if (!updatesEnabled() && children().size() == 0)
        return;
#if 0
    bool valid_rect = r.isValid();
    QRect sr = valid_rect?r:rect();
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

    QSize s(qt_screen->width(), qt_screen->height());
    QRegion alloc = valid_rect ? d->paintableRegion() : d->allocatedRegion();

    QRegion dAlloc = alloc;
    QPoint td1 = qt_screen->mapToDevice(QPoint(0,0), s);
    QPoint td2 = qt_screen->mapToDevice(QPoint(dx,dy), s);
    dAlloc.translate(td2.x()-td1.x(), td2.y()-td1.y());

    QRegion scrollRegion(alloc & dAlloc);

    if (w > 0 && h > 0) {
        bool was_unclipped = testAttribute(Qt::WA_PaintUnclipped);
        setAttribute(Qt::WA_PaintUnclipped);

        QWSPaintEngine * engine=static_cast<QWSPaintEngine*>(paintEngine());
        engine->begin(this);

        engine->setClipDeviceRegion(scrollRegion);
        engine->scroll(x2,y2,w,h,x1,y1);
        engine->end();
        if (!was_unclipped)
            setAttribute(Qt::WA_PaintUnclipped,false);
    }
    data->paintable_region_dirty = true;

    QPoint gpos = mapToGlobal(QPoint());

    if (!valid_rect && children().size() > 0) {        // scroll children
        d->setChildrenAllocatedDirty();
        QPoint pd(dx, dy);
        QObjectList childObjects = children();
        for (int i = 0; i < childObjects.size(); ++i) { // move all children
            QObject *object = childObjects.at(i);
            if (object->isWidgetType()) {
                QWidget *w = static_cast<QWidget *>(object);
                QPoint oldp = w->pos();
                QRect  r(w->pos() + pd, w->size());
                w->data->crect = r;
//                w->d_func()->updateRequestedRegion(gpos + w->pos());
                QMoveEvent e(r.topLeft(), oldp);
                QApplication::sendEvent(w, &e);
            }
        }
    }

    QSize ds(qt_screen->deviceWidth(), qt_screen->deviceHeight());
    scrollRegion = qt_screen->mapFromDevice(scrollRegion, ds);
    scrollRegion.translate(-gpos.x(), -gpos.y());

    QRegion update(sr);
    update -= scrollRegion;
    if (dx) {
        int x = x2 == sr.x() ? sr.x()+w : sr.x();
        update |= QRect(x, sr.y(), qAbs(dx), sr.height());
    }
    if (dy) {
        int y = y2 == sr.y() ? sr.y()+h : sr.y();
        update |= QRect(sr.x(), y, sr.width(), qAbs(dy));
    }
    repaint(update);
    if (!valid_rect)
        paint_children(this, update, false);
#endif
}


int QWidget::metric(PaintDeviceMetric m) const
{
    int val;
    if (m == PdmWidth) {
        val = data->crect.width();
    } else if (m == PdmWidthMM) {
        // 75 dpi is 3dpmm
        val = (data->crect.width()*100)/288;
    } else if (m == PdmHeight) {
        val = data->crect.height();
    } else if (m == PdmHeightMM) {
        val = (data->crect.height()*100)/288;
    } else if (m == PdmDepth) {
        return qwsDisplay()->depth();
    } else if (m == PdmDpiX || m == PdmPhysicalDpiX) {
        return 72;
    } else if (m == PdmDpiY || m == PdmPhysicalDpiY) {
        return 72;
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
}

void QWidgetPrivate::deleteTLSysExtra()
{
}

bool QWidgetPrivate::setAcceptDrops_sys(bool on)
{
    Q_UNUSED(on);
    return true;
}

QRegion QWidgetPrivate::localRequestedRegion() const
{
    Q_Q(const QWidget);
    QRegion r(q->rect());
    if (extra && !extra->mask.isEmpty())
        r &= extra->mask;

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

    d->extra->mask = region;

    if (isVisible()) {
        if (isWindow()) {
            QRegion rgn = d->localRequestedRegion();
            rgn.translate(geometry().topLeft());
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
                rgn += d->extra->topextra->qwsManager->region();
            }
#endif
            d->requestWindowRegion(rgn);
        } else {
            update(); //@@@ ??? should do parent update of oldmask | newmask ....
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

/*!
    \internal
*/
const unsigned char * QWidget::qwsScanLine(int i) const
{
    // Should add widget x() here, maybe
    unsigned char * base=qwsDisplay()->frameBuffer();
    if(base)
        base+=i*qwsBytesPerLine();
    return base;
}

/*!
    \internal
*/
int QWidget::qwsBytesPerLine() const
{
    return qt_screen->linestep();
}

void QWidgetPrivate::updateFrameStrut() const
{
    Q_Q(const QWidget);
    QWidget *that = const_cast<QWidget *>(q);

    if(!q->isVisible() || (q->windowType() == Qt::Desktop)) {
        that->data->fstrut_dirty = q->isVisible();
        return;
    }

    //FIXME: need to fill in frame strut info
}

#ifndef QT_NO_CURSOR
void QWidgetPrivate::updateCursor(const QRegion &r) const
{
    Q_Q(const QWidget);
    //@@@ region stuff must be redone
    if (qt_last_x && (!QWidget::mouseGrabber() || QWidget::mouseGrabber() == q) &&
            qt_last_cursor != (WId)q->cursor().handle() && !qws_overrideCursor) {
        QSize s(qt_screen->width(), qt_screen->height());
        QPoint pos = qt_screen->mapToDevice(QPoint(*qt_last_x, *qt_last_y), s);
        if (r.contains(pos))
            q->qwsDisplay()->selectCursor(const_cast<QWidget*>(q), q->cursor().handle());
    }
}
#endif


void QWidget::setWindowOpacity(qreal level)
{
    Q_D(QWidget);
    level = qMin<qreal>(qMax(level, 0.0), 1.0);
    uchar opacity = uchar(level * 255);
    d->topData()->opacity = opacity;
    if (isWindow() && isVisible())
        qwsDisplay()->setOpacity(data->winid, opacity);
}

qreal QWidget::windowOpacity() const
{
    return isWindow() ? ((QWidget*)this)->d_func()->topData()->opacity / 255.0 : 0.0;
}

//static QSingleCleanupHandler<QWSPaintEngine> qt_paintengine_cleanup_handler;
//static QWSPaintEngine *qt_widget_paintengine = 0;
/*!
    Returns the widget's paint engine.
*/
QPaintEngine *QWidget::paintEngine() const
{
    qWarning("QWidget::paintEngine() should no longer be called");
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
