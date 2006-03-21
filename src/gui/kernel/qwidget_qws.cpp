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
#include "private/qwsdisplay_qws_p.h"
#include "qscreen_qws.h"
#include "qwsmanager_qws.h"
#include <private/qwsmanager_p.h>
#include <private/qbackingstore_p.h>
#include <private/qwslock_p.h>
#include "qpaintengine.h"

#include "qdebug.h"

#include "qwidget_p.h"
#include "qwidget_qws_p.h"

extern int *qt_last_x;
extern int *qt_last_y;
extern WId qt_last_cursor;
extern bool qws_overrideCursor;
extern QWidget *qt_pressGrab;
extern QWidget *qt_mouseGrb;

extern QRect qt_maxWindowRect;

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
        QWidget::qwsDisplay()->nameRegion(q->winId(), q->objectName(), q->windowTitle());
    }

    if (topLevel) {
        createTLExtra();
#ifndef QT_NO_QWS_MANAGER
        if (hasFrame) {
            // get size of wm decoration and make the old crect the new frect
            QRect cr = data.crect;
            QRegion r = QApplication::qwsDecoration().region(q, cr) | cr;
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
        if ((windowType() == Qt::Desktop)) {
        } else {
            if (parentWidget() && parentWidget()->testAttribute(Qt::WA_WState_Created)) {
                d->hide_sys();
            }
            if (destroyWindow && isWindow()) {
                qwsDisplay()->destroyRegion(winId());
                d->extra->topextra->backingStore->buffer.detach();
            }
        }
        d->setWinId(0);
    }
}


void QWidgetPrivate::setParent_sys(QWidget *newparent, Qt::WFlags f)
{
    Q_Q(QWidget);
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
    bool     enable = q->isEnabled();                // remember status
    Qt::FocusPolicy fp = q->focusPolicy();
    QSize    s            = q->size();
    //QBrush   bgc    = background();                        // save colors
    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);

    data.window_flags = f;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    q->create();
    if (q->isWindow() || (!newparent || newparent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    if (q->isWindow())
        q->setGeometry(extra->topextra->fleft, extra->topextra->ftop,
                       s.width(), s.height());
    else
        q->setGeometry(0, 0, s.width(), s.height());

    if (q->isWindow()) {
        if (!extra->topextra->caption.isEmpty())
            setWindowTitle_helper(extra->topextra->caption);
    }

    setEnabled_helper(enable); //preserving WA_ForceDisabled
    q->setFocusPolicy(fp);
    if (extra && !extra->mask.isEmpty()) {
        QRegion r = extra->mask;
        extra->mask = QRegion();
        q->setMask(r);
    }
    if ((int)old_winid > 0) {
        QWidget::qwsDisplay()->destroyRegion(old_winid);
        extra->topextra->backingStore->buffer.detach();
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
        int winid = tlw->winId();
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
        qwsDisplay()->requestFocus(tlw->winId(), true);
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

    wbs->dirty_on_screen += wrgn;

    QApplication::postEvent(tlw, new QEvent(QEvent::UpdateRequest));
}

void QWidgetPrivate::cleanWidget_sys(const QRegion& rgn)
{
    Q_Q(QWidget);
    QWidget *tlw = q->window();
    QWidgetBackingStore *wbs = tlw->d_func()->topData()->backingStore;
    QRegion wrgn(rgn);
    if (tlw != q) {
        QPoint offs(q->mapTo(tlw, QPoint()));
        wrgn.translate(offs);
    }

    wbs->dirty_on_screen -= wrgn;
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
    QWidget::qwsDisplay()->repaintRegion(win->data->winid, opaque, globalrgn);
}

//#define QT_SHAREDMEM_DEBUG

QWSBackingStore::QWSBackingStore()
{
    isSharedMemory = false;
    memLock = 0;
    ownsMemory = false;
}

QWSBackingStore::~QWSBackingStore()
{
    detach();
}

void QWSBackingStore::blit(const QRect &r, const QPoint &p)
{
    int lineskip = img.bytesPerLine();
    int depth = img.depth() >> 3;

    const uchar *src;
    uchar *dest;

    if (r.top() < p.y()) {
        // backwards vertically
        src = mem + r.bottom()*lineskip + r.left()*depth;
        dest = mem + (p.y()+r.height()-1)*lineskip + p.x()*depth;
        lineskip = -lineskip;
    } else {
        src = mem + r.top()*lineskip + r.left()*depth;
        dest = mem + p.y()*lineskip + p.x()*depth;
    }

    const int w = r.width();
    int h = r.height();
    const int bytes = w * depth;
    lock();
    do {
        ::memmove(dest, src, bytes);
        dest += lineskip;
        src += lineskip;
    } while (--h);
    unlock();
}

QWSMemId QWSBackingStore::memoryId() const
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (isSharedMemory)
        return shm.id();
    else
#endif
        return mem;
}

void QWSBackingStore::detach()
{
#ifdef QT_SHAREDMEM_DEBUG
    qDebug() << "QWSBackingStore::detach shmid" << shm.id() << "shmaddr" << shm.address();
#endif
    img = QImage();
#ifndef QT_NO_QWS_MULTIPROCESS
    if (isSharedMemory)
        shm.detach();
    else
#endif
        if (ownsMemory)
            delete[] mem;
    isSharedMemory = false;
    ownsMemory = false;
    mem = 0;
}



bool QWSBackingStore::createIfNecessary(QWidget *tlw)
{
    QRegion tlwRegion = tlw->geometry();
#ifndef QT_NO_QWS_MANAGER
    QTLWExtra *topextra = tlw->d_func()->extra->topextra;
    if (topextra->qwsManager)
        tlwRegion += topextra->qwsManager->region();
#endif
    if (!tlw->d_func()->extra->mask.isEmpty())
        tlwRegion &= tlw->d_func()->extra->mask.translated(tlw->geometry().topLeft());
    QSize tlwSize = tlwRegion.boundingRect().size();


    QBrush bgBrush = tlw->palette().brush(tlw->backgroundRole());
    bool opaque = bgBrush.style() == Qt::NoBrush || bgBrush.isOpaque();

    QImage::Format imageFormat = (opaque && qt_screen->depth() == 16) ? QImage::Format_RGB16 : QImage::Format_ARGB32_Premultiplied;

#ifdef EXPERIMENTAL_ONSCREEN_PAINT
    bool useBS = !opaque || tlw->windowOpacity() != qreal(1.0);
#else
    bool useBS = true;
#endif
    bool doCreate = false;
    if (useBS)
        doCreate = tlwSize != size() || imageFormat != img.format() || _windowType == NoBS; // ...
    else
        doCreate = _windowType != NoBS;

    if (doCreate) {
        create(tlwSize, imageFormat, useBS ? int(opaque) : int(NoBS));
         QWidget::qwsDisplay()->requestRegion(tlw->data->winid,
                                              memoryId(),
                                              _windowType, tlwRegion, imageFormat);
         offs = tlw->geometry().topLeft() - tlwRegion.boundingRect().topLeft();
    }

    if (windowType() == QWSBackingStore::NoBS)
        offs = tlw->geometry().topLeft();

    return doCreate;
}


void QWSBackingStore::create(QSize s, QImage::Format imageFormat, int windowType)
{
    if (size() == s && imageFormat == img.format() && windowType == _windowType)
        return;
    detach();
    _windowType = windowType;

    if (_windowType == QWSBackingStore::NoBS) {
        mem = qt_screen->base();
        img = QImage(mem, qt_screen->width(), qt_screen->height(), imageFormat);
        return;
    }
    if (s.isNull()) {
#ifdef QT_SHAREDMEM_DEBUG
        qDebug() << "QWSBackingStore::create null size";
#endif
        return;
    }
    const int bytes_per_pixel = imageFormat == QImage::Format_RGB16 ? 2 : 4;
    int extradatasize = 0;//2 * sizeof(int); //store height and width ???
    int bpl = (s.width() * bytes_per_pixel + 3) & ~3;
    int datasize = bpl * s.height() + extradatasize;

    if (isServerProcess()) { // I'm the server process
        mem = new uchar[datasize];
        isSharedMemory = false;
        ownsMemory = true;
#ifndef QT_NO_QWS_MULTIPROCESS
    } else {
        isSharedMemory = true;
        ownsMemory = false;
        if (!shm.create(datasize)) {
            perror("QWSBackingStore::create allocating shared memory");
            qFatal("Error creating shared memory of size %d", datasize);
        }
        mem = static_cast<uchar*>(shm.address());
        memLock = QWSDisplay::Data::getClientLock();
#endif
    }

    img = QImage(mem + extradatasize, s.width(), s.height(), imageFormat);
#ifdef QT_SHAREDMEM_DEBUG
    qDebug() << "QWSBackingStore::create size" << s << "shmid" << shm.id() << "shmaddr" << shm.address() << "imageFormat" << imageFormat;
#endif
}

#ifndef QT_NO_QWS_MULTIPROCESS
void QWSBackingStore::attach(QWSMemId id, QSize s, QImage::Format imageFormat, int windowType)
{
    if (shm.id() == id && s == size() && _windowType == windowType)
        return;
    detach();
    _windowType = windowType;
    if (windowType == QWSBackingStore::NoBS) {
        mem  = qt_screen->base();
        return;
    }
    if (s.isNull())
        return;
    if (windowType == YellowThing) {
        // QT_FLUSH_PAINT
        //detach has already set everything
        return;
    }
    if (!shm.attach(id)) {
        perror("QWSBackingStore::attach attaching to shared memory");
        qWarning("QWSBackingStore::attach: Error attaching to shared memory 0x%x of size %d",
                 int(id), s.width() * s.height());
        return;
    }
    isSharedMemory = true;
    ownsMemory = false;
    mem = static_cast<uchar*>(shm.address());

    int extradatasize = 0;
    img = QImage(mem + extradatasize, s.width(), s.height(), imageFormat);
#ifdef QT_SHAREDMEM_DEBUG
    qDebug() << "QWSBackingStore::attach size" << s << "shmid" << shm.id() << "shmaddr" << shm.address() << "imageFormat" << imageFormat;
#endif
}
#endif // QT_NO_QWS_MULTIPROCESS

void QWSBackingStore::setMemory(QWSMemId id, const QSize &s, QImage::Format imageFormat, int windowType)
{
    _windowType = windowType;
    if (_windowType == QWSBackingStore::NoBS) {
        //qDebug("QWSBackingStore::setMemory");
        mem = qt_screen->base();

        img = QImage(mem, qt_screen->width(), qt_screen->height(), imageFormat);
        isSharedMemory = false;
        ownsMemory = false;
        return;
    }
    if (windowType == YellowThing) {
        // Handle QT_FLUSH_PAINT
        mem = 0;
        img = QImage();
        isSharedMemory = false;
        ownsMemory = false;
        return;
    }
    mem = id;
    img = QImage(mem, s.width(), s.height(), imageFormat);
    isSharedMemory = false;
    ownsMemory = false;
#ifdef QT_SHAREDMEM_DEBUG
    qDebug() << "QWSBackingStore::setMemory size" << s << "shmid" << shm.id() << "shmaddr" << shm.address() << "imageFormat" << imageFormat;
#endif
}

bool QWSBackingStore::lock(int timeout)
{
    return !memLock || memLock->lock(QWSLock::BackingStore, timeout);
}

void QWSBackingStore::unlock()
{
    if (memLock)
        memLock->unlock(QWSLock::BackingStore);
}

bool QWSBackingStore::wait(int timeout)
{
    return !memLock || memLock->wait(QWSLock::BackingStore, timeout);
}

/*!
    Set the lock to be used for backingstore synchronization.
    The backingstore will not take ownership of the lock.
*/
void QWSBackingStore::setLock(QWSLock *l)
{
    memLock = l;
}

void QWidgetPrivate::show_sys()
{
    Q_Q(QWidget);
    q->setAttribute(Qt::WA_Mapped);
    if (q->isWindow()) {
        QRegion r = localRequestedRegion();
#ifndef QT_NO_QWS_MANAGER
        if (extra && extra->topextra && extra->topextra->qwsManager) {
            r.translate(data.crect.topLeft());
            r += extra->topextra->qwsManager->region();
            r.translate(-data.crect.topLeft());
        }
#endif
        invalidateBuffer(r);
        if (q->windowType() != Qt::Popup
            && q->windowType() != Qt::Tool
            && q->windowType() != Qt::ToolTip ) {
            QWidget::qwsDisplay()->requestFocus(data.winid,true);
        }
        QWidget::qwsDisplay()->setAltitude(data.winid,
                                     (q->windowFlags() & Qt::WindowStaysOnTopHint) ? 1 : 0, true);

    }

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
        QWidget::qwsDisplay()->setAltitude(q->winId(), 0);
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
    } else if (QWidget *p = q->parentWidget()) {
        p->update(q->geometry());
    }
}

void QWidgetPrivate::lower_sys()
{
    Q_Q(QWidget);
    if (q->isWindow()) {
        QWidget::qwsDisplay()->setAltitude(data.winid, -1);
    } else if (QWidget *p = q->parentWidget()) {
        p->update(q->geometry());
    }
}

void QWidgetPrivate::stackUnder_sys(QWidget*)
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
                QWidget::qwsDisplay()->moveRegion(data.winid, x - oldp.x(), y - oldp.y());
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
                    QWSBackingStore *bs = &topextra->backingStore->buffer;
                    if (bs->windowType() == QWSBackingStore::NoBS) {
                        QWidget::qwsDisplay()->requestRegion(data.winid, bs->memoryId(), bs->windowType(),
                                                             myregion, bs->image().format());
#ifndef QT_NO_QWS_MANAGER
                        if (topextra->qwsManager)
                            topextra->qwsManager->d_func()->dirtyRegion(QDecoration::All,
                                                            QDecoration::Normal);
#endif
                    }
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
                invalidateBuffer(q->rect()); //###
            } else {
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

    topextra->inPaintTransaction = inTransaction;
}

void QWidgetPrivate::setConstraints_sys()
{
}


void QWidget::scroll(int dx, int dy)
{

    Q_D(QWidget);
    if (!updatesEnabled() && children().size() == 0)
        return;
    if (dx == 0 && dy == 0)
        return;
    d->scrollChildren(dx, dy);
    d->scrollRect(rect(), dx, dy);
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
   Q_D(QWidget);
    if (!updatesEnabled() && children().size() == 0)
        return;
    if (dx == 0 && dy == 0)
        return;
    d->scrollRect(r, dx, dy);
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
    extra->topextra->backingStore = new QWidgetBackingStore(q_func());
}

void QWidgetPrivate::deleteTLSysExtra()
{
    delete extra->topextra->backingStore;
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
            QTLWExtra *topextra = d->extra->topextra;
            QRegion myregion = d->localRequestedRegion();
            myregion.translate(geometry().topLeft());
            if (topextra) {
#ifndef QT_NO_QWS_MANAGER
                if (topextra->qwsManager)
                    myregion += topextra->qwsManager->region();
#endif
                QRect br(myregion.boundingRect());
                topextra->fleft = d->data.crect.x()-br.x();
                topextra->ftop = d->data.crect.y()-br.y();
                topextra->fright = br.right()-d->data.crect.right();
                topextra->fbottom = br.bottom()-d->data.crect.bottom();
            }
            d->invalidateBuffer(rect());
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

void QWidgetPrivate::updateFrameStrut() const
{
#if 0
    Q_Q(const QWidget);

    if(!q->isVisible() || (q->windowType() == Qt::Desktop)) {
        data.fstrut_dirty = q->isVisible();
        return;
    }
#ifndef QT_NO_QWS_MANAGER
    QRegion r = localRequestedRegion();
    r.translate(data.crect.topLeft());
    if (extra && extra->topextra && extra->topextra->qwsManager) {
        r += extra->topextra->qwsManager->region();
    }
    QRect contents = geometry();
    QRect frame = r.boundingRect();
    top->fleft = contents.left() - frame.left();
    top->ftop = contents.top() - frame.top();
    top->fright = frame.right() - contents.right();
    top->fbottom = frame.bottom() - contents.bottom();
#endif
    data.fstrut_dirty = false;
#endif
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
            QWidget::qwsDisplay()->selectCursor(const_cast<QWidget*>(q), q->cursor().handle());
    }
}
#endif


void QWidget::setWindowOpacity(qreal level)
{
    Q_D(QWidget);
    level = qMin<qreal>(qMax(level, qreal(0.0)), qreal(1.0));
    uchar opacity = uchar(level * 255);
    d->topData()->opacity = opacity;
    if (isWindow())
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
