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
#include "qcursor.h"
#include "qinputcontext.h"
#include "qdesktopwidget.h"
#include "qcleanuphandler.h"

#include "qwsdisplay_qws.h"
#include "qscreen_qws.h"
#include "qwsmanager_qws.h"
#include "qwsregionmanager_qws.h"

#include <private/qpaintengine_qws_p.h> //### this one goes away very soon

#include <qgfxraster_qws.h>


#include "qdebug.h"

#include "qwidget_p.h"
#define d d_func()
#define q q_func()

extern int *qt_last_x;
extern int *qt_last_y;
extern WId qt_last_cursor;
extern bool qws_overrideCursor;
extern QWidget *qt_pressGrab;
extern QWidget *qt_mouseGrb;

extern QRect qt_maxWindowRect;

extern bool qt_override_paint_on_screen;

extern void qwsUpdateActivePainters();

static QWidget *keyboardGrb = 0;

static int takeLocalId()
{
    static int n=-1000;
    return --n;
}

// This repaints all children within a widget.

static void paint_children(QWidget * p,const QRegion& r, bool update)
{
    if(!p)
        return;
    QObjectList childObjects=p->children();
    for (int i = 0; i < childObjects.size(); ++i) {
        QObject * o = childObjects.at(i);

        if(o->isWidgetType()) {
                QWidget *w = static_cast<QWidget *>(o);
                if (w->testAttribute(Qt::WA_WState_Visible)) {
                    QRegion wr(QRegion(w->geometry()) & r);
                    if (!wr.isEmpty()) {
                        wr.translate(-w->x(),-w->y());
                        if (update || w->testAttribute(Qt::WA_WState_InPaintEvent))
                            w->update(wr);
                        else
                            w->repaint(wr);
                        paint_children(w,wr,update);
                    }
                }
            }
    }

}

// Paint the widget and its children

static void paint_hierarchy(QWidget *w, bool update)
{
    if (w && w->testAttribute(Qt::WA_WState_Visible)) {
        if (update)
            w->update(w->rect());
        else
            w->repaint(w->rect());

        QObjectList childObjects = w->children();
        for (int i = 0; i < childObjects.size(); ++i) {
            QObject *o = childObjects.at(i);
            if(o->isWidgetType())
                paint_hierarchy(static_cast<QWidget *>(o),update);
        }
    }
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool /*destroyOldWindow*/)
{
    Qt::WindowType type = q->windowType();
    Qt::WindowFlags &flags = data.window_flags;
    QWidget *parentWidget = q->parentWidget();

    data.alloc_region_index = -1;
    data.alloc_region_revision = -1;
    isSettingGeometry = false;
    data.overlapping_children = -1;

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
            otherDesktop->d->setWinId(0);        // remove id from widget mapper
            setWinId(id);                        // make sure otherDesktop is
            otherDesktop->d->setWinId(id);        //   found first
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

    data.alloc_region_dirty=false;
    data.paintable_region_dirty=false;


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

        // If we are session managed, inform the window manager about it
        if (extra && !extra->mask.isEmpty()) {
            data.req_region = extra->mask;
            data.req_region.translate(data.crect.x(),data.crect.y());
            data.req_region &= data.crect; //??? this is optional
        } else {
            data.req_region = data.crect;
        }
        data.req_region = qt_screen->mapToDevice(data.req_region, QSize(qt_screen->width(), qt_screen->height()));
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
            qApp->closePopup(this);
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
            oldparent->d->setChildrenAllocatedDirty();
            oldparent->data->paintable_region_dirty = true;
        }
        if (newparent) {
            newparent->d->setChildrenAllocatedDirty();
            newparent->data->paintable_region_dirty = true;
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
        extra->topextra->caption = QString::null;
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
    d->createExtra();
    delete d->extra->curs;
    d->extra->curs = new QCursor(cursor);
    setAttribute(Qt::WA_SetCursor);
    if (isVisible())
        d->updateCursor(d->paintableRegion());
}

void QWidget::unsetCursor()
{
    if (d->extra) {
        delete d->extra->curs;
        d->extra->curs = 0;
    }
    setAttribute(Qt::WA_SetCursor, false);
    if (isVisible())
        d->updateCursor(d->paintableRegion());
}
#endif //QT_NO_CURSOR

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

#ifndef QT_NO_WIDGET_TOPEXTRA
void QWidget::setWindowTitle(const QString &caption)
{
    if (d->extra && d->extra->topextra && d->extra->topextra->caption == caption)
        return; // for less flicker
    d->createTLExtra();
    d->extra->topextra->caption = caption;
    qwsDisplay()->setWindowCaption(this, caption);
    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

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

#ifdef QT_QWS_NO_BACKING_STORE
struct QWSDoubleBuffer
{
    enum {
        MaxWidth = SHRT_MAX,
        MaxHeight = SHRT_MAX
    };

    QPixmap *hd;
    int depth;
};

static QWSDoubleBuffer *qt_global_double_buffer = 0;
static bool qt_global_double_buffer_active = false;

static void qt_discard_double_buffer(QWSDoubleBuffer **db)
{
    if (!*db)
        return;

    delete (*db)->hd;
    delete *db;
    *db = 0;
}

void qt_discard_double_buffer()
{
    qt_discard_double_buffer(&qt_global_double_buffer);
}

static QWSDoubleBuffer *qt_qws_create_double_buffer(int width, int height, int depth)
{
    QWSDoubleBuffer *db = new QWSDoubleBuffer;
    db->depth = depth;
    db->hd = new QPixmap(width, height, db->depth);
    Q_ASSERT(db->hd);
    return db;
}

static void qt_qws_get_double_buffer(QWSDoubleBuffer **db, int width, int height, int depth)
{
    // the db should consist of 128x128 chunks
    width  = qMin(((width / 128) + 1) * 128, int(QWSDoubleBuffer::MaxWidth));
    height = qMin(((height / 128) + 1) * 128, int(QWSDoubleBuffer::MaxHeight));

    if (qt_global_double_buffer_active) {
        *db = qt_qws_create_double_buffer(width, height, depth);
        return;
    }

    qt_global_double_buffer_active = true;

    if (qt_global_double_buffer) {
        if (qt_global_double_buffer->hd->width() >= width
            && qt_global_double_buffer->hd->height() >= height) {
            *db = qt_global_double_buffer;
            return;
        }

        width  = qMax(qt_global_double_buffer->hd->width(), width);
        height = qMax(qt_global_double_buffer->hd->height(), height);

        qt_discard_double_buffer(&qt_global_double_buffer);
    }

    qt_global_double_buffer = qt_qws_create_double_buffer(width, height, depth);
    *db = qt_global_double_buffer;
};

static void qt_qws_release_double_buffer(QWSDoubleBuffer **db)
{
    if (*db != qt_global_double_buffer)
        qt_discard_double_buffer(db);
    else
        qt_global_double_buffer_active = false;
}

#endif


#ifndef QT_QWS_NO_BACKING_STORE
void QWidgetPrivate::bltToScreen(const QRegion &globalrgn)
{
    static QGfx *gfx = 0;
    if ( !gfx)
        gfx = qt_screen->screenGfx();


//take the logic from QWSPaintEngine::begin
    QWidget *win = q->window();

//     QPoint globalpos=q->mapToGlobal(QPoint(0,0));
//     QRegion cliprgn = rgn;
//     cliprgn.translate(globalpos);

    QTLWExtra *topextra = win->d->extra->topextra;
    QPixmap *buf = &topextra->backingStore;
    QPoint bsoffs = topextra->backingStoreOffset;

    QPoint topLeft = win->pos();

    gfx->setWidgetDeviceRegion(globalrgn);

    gfx->setClipRegion(QRegion(), Qt::NoClip);


    gfx->setSource(buf);
    gfx->setAlphaType(QGfx::IgnoreAlpha);

    gfx->blt(topLeft.x(),topLeft.y(), buf->width(), buf->height(), 0, 0);
}
#endif


void QWidgetPrivate::paintHierarchy()
{
#if 0 //DEBUG
    static bool painting = false;
    bool outermost = !painting;
    if (outermost)
        qDebug(">>>>> paintHierarchy %p START", q);
    painting = true;
#endif
    doPaint(q->rect());

    for (int i = 0; i < children.size(); ++i) {
        register QObject *obj=children.at(i);
        if (obj->isWidgetType()) {
            QWidget* w = static_cast<QWidget*>(obj);
            if (w->isVisible() && !w->isWindow())
                w->d->paintHierarchy();
        }
    }
#if 0 //DEBUG
    if (outermost) {
        qDebug("<<<<<< paintHierarchy %p END", q);
        painting = false;
    }
#endif
}





void QWidget::repaint(const QRegion& rgn)
{
    if (!isVisible() || !updatesEnabled() || !testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;
#ifdef QT_QWS_NO_BACKING_STORE
    d->doPaint(rgn);
#else
    QRegion globalrgn = rgn;
    QPoint globalPos = mapToGlobal(QPoint(0,0));
    globalrgn.translate(globalPos);
    globalrgn &= d->paintableRegion();

    QRegion repaintRgn = globalrgn;
    repaintRgn.translate(-globalPos);

    d->doPaint(repaintRgn);

    d->bltToScreen(globalrgn);
#endif

    if (testAttribute(Qt::WA_ContentsPropagated))
        d->updatePropagatedBackground(&rgn);
}

void QWidgetPrivate::doPaint(const QRegion &rgn)
{

    qDebug("doPaint %p child of %p", q, q->parentWidget());


    if (q->testAttribute(Qt::WA_WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");

    q->setAttribute(Qt::WA_WState_InPaintEvent);

    QRect br = rgn.boundingRect();
    bool do_clipping = (br != QRect(0, 0, data.crect.width(), data.crect.height()));
#ifdef QT_QWS_NO_BACKING_STORE
    bool double_buffer = !qt_override_paint_on_screen &&
                         (!testAttribute(Qt::WA_PaintOnScreen)
                          && !testAttribute(Qt::WA_NoSystemBackground)
                          && br.width()  <= QWSDoubleBuffer::MaxWidth
                          && br.height() <= QWSDoubleBuffer::MaxHeight
                          && !QPainter::redirected(this));

    QPoint redirectionOffset;
    QWSDoubleBuffer *qDoubleBuffer = 0;
    if (double_buffer) {
        qt_qws_get_double_buffer(&qDoubleBuffer, br.width(), br.height(), QPixmap::defaultDepth());
        redirectionOffset = br.topLeft();
        QPainter::setRedirected(this, qDoubleBuffer->hd, redirectionOffset);
    }
#else
    QWidget *tlw = q->window();
    QTLWExtra *topextra = tlw->d->extra->topextra;
    QPoint redirectionOffset = topextra->backingStoreOffset + q->mapFrom(tlw,QPoint(0,0));
    QPainter::setRedirected(q, &topextra->backingStore, redirectionOffset);
#endif

    QPainter p; // We'll use it several times
#ifdef QT_QWS_NO_BACKING_STORE
    // Set clipping
    if (do_clipping) {
        if (redirectionOffset.isNull()) {
            paintEngine()->setSystemClip(rgn);
        } else {
            QRegion redirectionRegion(rgn);
            redirectionRegion.translate(-redirectionOffset);
            paintEngine()->setSystemClip(redirectionRegion);
        }
    }
#else
    //###### clipping
    QRegion clipRegion(rgn);
    clipRegion.translate(-redirectionOffset);
    topextra->backingStore.paintEngine()->setSystemClip(clipRegion);

#endif
    if (!q->testAttribute(Qt::WA_NoBackground) && !q->testAttribute(Qt::WA_NoSystemBackground))
        composeBackground(br);

    // Send paint event to self
    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent(q, &e);
#ifdef QT_QWS_NO_BACKING_STORE
    // Clear the clipping again
    if (do_clipping)
        paintEngine()->setSystemClip(QRegion());
#else
    topextra->backingStore.paintEngine()->setSystemClip(QRegion());
#endif

#ifndef QT_QWS_NO_BACKING_STORE
    QPainter::restoreRedirected(q);
#else
    // Flush double buffer, if used
    if (double_buffer) {
        QPainter::restoreRedirected(this);

        p.begin(this);
        QVector<QRect> rects = rgn.rects();
        for (int i = 0; i < rects.size(); ++i) {
            QRect rr = rects.at(i);
            p.drawPixmap(rr.topLeft(), *(qDoubleBuffer->hd),
                         QRect(rr.topLeft() - redirectionOffset, rr.size()));
        }
        p.end();

        qt_qws_release_double_buffer(&qDoubleBuffer);

        // Delete double buffer if not used within timeout
        if (!QApplicationPrivate::active_window) {
            extern int qt_double_buffer_timer;
            if (qt_double_buffer_timer)
                qApp->killTimer(qt_double_buffer_timer);
            qt_double_buffer_timer = qApp->startTimer(500);
        }
    }
#endif

    // Clean out the temporary engine if used...
    if (extraPaintEngine) {
        delete extraPaintEngine;
        extraPaintEngine = 0;
    }

    q->setAttribute(Qt::WA_WState_InPaintEvent, false);

    if(!q->testAttribute(Qt::WA_PaintOutsidePaintEvent) && q->paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");
}


void QWidgetPrivate::requestWindowRegion(const QRegion &r)
{
    q->qwsDisplay()->requestRegion(data.winid, r);
#ifndef QT_QWS_NO_BACKING_STORE
    Q_ASSERT(extra && extra->topextra);
    QRect br = r.boundingRect();
    if (extra->topextra->backingStore.size() != br.size()) {
        extra->topextra->backingStore = QPixmap(br.size());
#if 0 //DEBUG
        extra->topextra->backingStore.fill(QColor(Qt::yellow));
        qDebug() << "backingStore size" << br.size() << "offset" << br.topLeft() - q->geometry().topLeft();
#endif
    }
    extra->topextra->backingStoreOffset = br.topLeft() - q->geometry().topLeft();
    paintHierarchy();
#endif
}

void QWidgetPrivate::show_sys()
{
    if (q->isWindow()) {
        updateRequestedRegion(q->mapToGlobal(QPoint(0,0)));
        QRegion r(data.req_region);
#ifndef QT_NO_QWS_MANAGER
        if (extra && extra->topextra && extra->topextra->qwsManager) {
            QRegion wmr = extra->topextra->qwsManager->region();
            wmr = qt_screen->mapToDevice(wmr, QSize(qt_screen->width(), qt_screen->height()));
            r += wmr;
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
        updateRequestedRegion(q->mapToGlobal(QPoint(0,0)));
        QWidget *p = q->parentWidget();
        p->d->setChildrenAllocatedDirty(q->geometry(), q);
        p->data->paintable_region_dirty = true;
        p->data->overlapping_children = -1;
        paint_hierarchy(q, true);
    }
}


void QWidgetPrivate::hide_sys()
{
    deactivateWidgetCleanup();

    if (data.req_region.isEmpty())        // Already invisible?
        return;

    if (q->isWindow()) {
        q->releaseMouse();
        requestWindowRegion(QRegion());
        q->qwsDisplay()->requestFocus(data.winid,false);
    } else {
        QWidget *p = q->parentWidget();
        if (p) {
            p->d->setChildrenAllocatedDirty(q->geometry(), q);
            p->data->paintable_region_dirty = true;
            if (p->data->overlapping_children)
                p->data->overlapping_children = -1;
            if (p->isVisible()) {
                p->update(q->geometry());
                paint_children(p,q->geometry(),true);
            }
        }
    }
    updateRequestedRegion(q->mapToGlobal(QPoint(0,0)));
}


void QWidget::setWindowState(Qt::WindowStates newstate)
{
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

    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::raise_sys()
{
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
        p->d->setChildrenAllocatedDirty(q->geometry(), q);
        paint_hierarchy(q, true);
    }
}

void QWidgetPrivate::lower_sys()
{
    if (q->isWindow()) {
        q->qwsDisplay()->setAltitude(data.winid, -1);
    } else if (QWidget *p = q->parentWidget()) {
        p->d->setChildrenAllocatedDirty(q->geometry());
        paint_children(p, q->geometry(),true);
    }
}

void QWidgetPrivate::stackUnder_sys(QWidget* w)
{
    if (QWidget *p = q->parentWidget()) {
        // #### excessive repaints
        p->d->setChildrenAllocatedDirty();
        paint_children(p, q->geometry(), true);
        paint_children(p, w->geometry(), true);
    }
}


void QWidgetPrivate::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    if (d->extra) {                                // any size restrictions?
        w = qMin(w,d->extra->maxw);
        h = qMin(h,d->extra->maxh);
        w = qMax(w,d->extra->minw);
        h = qMax(h,d->extra->minh);
    }
    if (q->isWindow()) {
        w = qMax(1, w);
        h = qMax(1, h);
    }

    QPoint oldp = q->geometry().topLeft();
    QSize olds = q->size();
    QRect r(x, y, w, h);

    bool isResize = olds != r.size();

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if (r.size() == olds && oldp == r.topLeft())
        return;

    QRegion oldAlloc;
    if (!q->isWindow() && isMove && (w==olds.width() && h==olds.height())) {
        oldAlloc = allocatedRegion();
    }

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

    if (q->isWindow()) {
        //### ConfigPending not implemented, do we need it?
        //setAttribute(Qt::WA_WState_ConfigPending);
        if (isMove && (w==olds.width() && h==olds.height())) {
            // just need to translate current region
            QSize s(qt_screen->width(), qt_screen->height());
            QPoint td1 = qt_screen->mapToDevice(QPoint(0,0), s);
            QPoint td2 = qt_screen->mapToDevice(QPoint(x - oldp.x(),y - oldp.y()), s);
            QPoint dd = QPoint(td2.x()-td1.x(), td2.y()-td1.y());
            data.req_region.translate(dd.x(), dd.y());
        } else {
            if (d->extra && !d->extra->mask.isEmpty()) {
                data.req_region = d->extra->mask;
                data.req_region.translate(data.crect.x(),data.crect.y());
                data.req_region &= data.crect; //??? this is optional
            } else {
                data.req_region = data.crect;
            }
            data.req_region = qt_screen->mapToDevice(data.req_region, QSize(qt_screen->width(), qt_screen->height()));
        }
        if (q->isVisible()) {
            if (isMove && !isResize && data.alloc_region_index >= 0) {
                q->qwsDisplay()->moveRegion(data.winid, x - oldp.x(), y - oldp.y());
                d->setChildrenAllocatedDirty();
            } else {
                QRegion rgn(data.req_region);
#ifndef QT_NO_QWS_MANAGER
                if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
                    QRegion wmr = d->extra->topextra->qwsManager->region();
                    wmr = qt_screen->mapToDevice(wmr, QSize(qt_screen->width(), qt_screen->height()));
                    rgn += wmr;
                }
#endif
                requestWindowRegion(rgn);
                if (d->extra && d->extra->topextra) {
                    QRect br(rgn.boundingRect());
                    br = qt_screen->mapFromDevice(br, QSize(qt_screen->deviceWidth(), qt_screen->deviceHeight()));
                    d->extra->topextra->fleft = data.crect.x()-br.x();
                    d->extra->topextra->ftop = data.crect.y()-br.y();
                    d->extra->topextra->fright = br.right()-data.crect.right();
                    d->extra->topextra->fbottom = br.bottom()-data.crect.bottom();
                }
            }
        }
    }

    if (q->isVisible()) {
        d->isSettingGeometry = true;
        if (isMove) {
            QMoveEvent e(q->pos(), oldPos);
            QApplication::sendEvent(q, &e);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager)
                QApplication::sendEvent(d->extra->topextra->qwsManager, &e);
#endif
        }
        if (isResize) {
            QResizeEvent e(r.size(), olds);
            QApplication::sendEvent(q, &e);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
                QResizeEvent e(r.size(), olds);
                QApplication::sendEvent(d->topData()->qwsManager, &e);
            }
#endif
        }

        updateRequestedRegion(q->mapToGlobal(QPoint(0,0)));

        QWidget *p = q->parentWidget();
        if (!q->isWindow() || isResize) {
            if (p && !q->isWindow()) {
                p->data->paintable_region_dirty = true;
                QRegion oldr(QRect(oldp, olds));
                dirtyChildren.translate(x, y);
                if (p->d->isSettingGeometry) {
                    if (oldp != r.topLeft()) {
                        QRegion upd((QRegion(r) | oldr) & p->rect());
                        dirtyChildren |= upd;
                    } else {
                        dirtyChildren |= QRegion(r) - oldr;
                        q->update(q->rect());
                    }
                    p->d->dirtyChildren |= dirtyChildren;
                } else {
                    QRegion upd((QRegion(r) | oldr) & p->rect());
                    dirtyChildren |= upd;
                    QRegion paintRegion = dirtyChildren;
#define FAST_WIDGET_MOVE
#ifdef FAST_WIDGET_MOVE
                    if (isMove && (w==olds.width() && h==olds.height())) {
                        QSize s(qt_screen->width(), qt_screen->height());

                        QPoint td1 = qt_screen->mapToDevice(QPoint(0,0), s);
                        QPoint td2 = qt_screen->mapToDevice(QPoint(x - oldp.x(),y - oldp.y()), s);
                        QPoint dd = QPoint(td2.x()-td1.x(), td2.y()-td1.y());
                        oldAlloc.translate(dd.x(), dd.y());

                        QRegion alloc(allocatedRegion());

                        QRegion scrollRegion(alloc & oldAlloc);
                        if (!scrollRegion.isEmpty()) {
                            bool was_unclipped = p->testAttribute(Qt::WA_PaintUnclipped);
                            p->setAttribute(Qt::WA_PaintUnclipped);

                            QWSPaintEngine * engine=static_cast<QWSPaintEngine*>(p->paintEngine());
                            engine->begin(p);

                            engine->setClipDeviceRegion(scrollRegion);
                            engine->scroll(x,y,w,h,oldp.x(),oldp.y());
                            engine->end();
                            if (!was_unclipped)
                                p->setAttribute(Qt::WA_PaintUnclipped,false);

                            QSize ds(qt_screen->deviceWidth(), qt_screen->deviceHeight());
                            scrollRegion = qt_screen->mapFromDevice(scrollRegion, ds);
                            QPoint gp = p->mapToGlobal(QPoint(0,0));
                            scrollRegion.translate(-gp.x(), -gp.y());
                            paintRegion -= scrollRegion;
                        }
                    }
#endif
                    if (!oldr.isEmpty())
                        p->update(oldr);
                    p->d->setChildrenAllocatedDirty(dirtyChildren, q);
                    qwsUpdateActivePainters();
                    paint_children(p, paintRegion, isResize);
                }
                p->data->overlapping_children = -1;
            } else {
                if (oldp != r.topLeft()) {
                    qwsUpdateActivePainters();
                    paint_hierarchy(q, true);
                } else {
                    d->setChildrenAllocatedDirty(dirtyChildren);
                    qwsUpdateActivePainters();
                    QApplication::postEvent(q, new QWSUpdateEvent(q->rect()));
                    paint_children(q, dirtyChildren, true);
                }
            }
        } else {
            qwsUpdateActivePainters();
        }
#ifndef QT_NO_QWS_MANAGER
        if (isResize && d->extra && d->extra->topextra && d->extra->topextra->qwsManager)
            QApplication::postEvent(d->topData()->qwsManager, new QPaintEvent(q->visibleRegion()));
#endif
        d->isSettingGeometry = false;
        dirtyChildren = QRegion();
    } else {
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}


void QWidget::setMinimumSize(int minw, int minh)
{
    if (!qt_maxWindowRect.isEmpty()) {
        // This is really just a work-around. Layout shouldn't be asking
        // for minimum sizes bigger than the screen.
        if (minw > qt_maxWindowRect.width())
            minw = qt_maxWindowRect.width();
        if (minh > qt_maxWindowRect.height())
            minh = qt_maxWindowRect.height();
    }

    if (minw < 0 || minh < 0)
        qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
    d->createExtra();
    if (d->extra->minw == minw && d->extra->minh == minh)
        return;
    d->extra->minw = minw;
    d->extra->minh = minh;
    if (minw > width() || minh > height())
        resize(qMax(minw,width()), qMax(minh,height()));
    updateGeometry();
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
    if (maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
        qWarning("QWidget::setMaximumSize: (%s/%s) "
                "The largest allowed size is (%d,%d)",
                 objectName().isEmpty() ? "unnamed" : objectName().toLatin1().constData(),
                 metaObject()->className(), QWIDGETSIZE_MAX,
                QWIDGETSIZE_MAX);
        maxw = qMin(maxw, QWIDGETSIZE_MAX);
        maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if (maxw < 0 || maxh < 0) {
        qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                 objectName().isEmpty() ? "unnamed" : objectName().toLatin1().constData(),
                 metaObject()->className(), maxw, maxh);
        maxw = qMax(maxw, 0);
        maxh = qMax(maxh, 0);
    }
    d->createExtra();
    if (d->extra->maxw == maxw && d->extra->maxh == maxh)
        return;
    d->extra->maxw = maxw;
    d->extra->maxh = maxh;
    if (maxw < width() || maxh < height())
        resize(qMin(maxw,width()), qMin(maxh,height()));
    updateGeometry();
}

void QWidget::setSizeIncrement(int w, int h)
{
    d->createTLExtra();
    QTLWExtra* x = d->extra->topextra;
    if (x->incw == w && x->inch == h)
        return;
    x->incw = w;
    x->inch = h;
    if (isWindow()) {
        // XXX ...
    }
}

void QWidget::setBaseSize(int basew, int baseh)
{
    d->createTLExtra();
    QTLWExtra* x = d->extra->topextra;
    if (x->basew == basew && x->baseh == baseh)
        return;
    x->basew = basew;
    x->baseh = baseh;
    if (isWindow()) {
        // XXX
    }
}

void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    if (!updatesEnabled() && children().size() == 0)
        return;
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
                w->d->updateRequestedRegion(gpos + w->pos());
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

bool QWidget::acceptDrops() const
{
    return testAttribute(Qt::WA_WState_DND);
}

void QWidget::setAcceptDrops(bool on)
{
    if (!!testAttribute(Qt::WA_WState_DND) != on) {
        if (1/*XXX qt_xdnd_enable(this, on)*/) {
            if (on)
                setAttribute(Qt::WA_WState_DND);
            else
                setAttribute(Qt::WA_WState_DND, false);
        }
    }
}

void QWidgetPrivate::updateOverlappingChildren() const
{
    if (data.overlapping_children != -1 || d->isSettingGeometry)
        return;

    QRegion r;
    for (int i = 0; i < children.size(); ++i) {
        QObject *ch = children.at(i);
            if (ch->isWidgetType() && !((QWidget*)ch)->isWindow()) {
                QWidget *w = (QWidget *)ch;
                if (w->isVisible()) {
                    QRegion rr(w->data->req_region);
                    QRegion ir = r & rr;
                    if (!ir.isEmpty()) {
                        data.overlapping_children = 1;
                        return;
                    }
                    r |= rr;
                }
            }
    }

    data.overlapping_children = 0;
}

void QWidgetPrivate::updateRequestedRegion(const QPoint &gpos)
{
    if (!q->isWindow()) {
        if (!q->testAttribute(Qt::WA_WState_Visible)) {
            data.req_region = QRegion();
        } else {
            data.req_region = QRect(gpos,data.crect.size());
            if (d->extra && !d->extra->mask.isEmpty()) {
                QRegion maskr = d->extra->mask;
                maskr.translate(gpos.x(), gpos.y());
                data.req_region &= maskr;
            }
            data.req_region = qt_screen->mapToDevice(data.req_region, QSize(qt_screen->width(), qt_screen->height()));
        }
    }

    for (int i = 0; i < children.size(); ++i) {
        QObject *ch = children.at(i);
            if (ch->isWidgetType() && !((QWidget*)ch)->isWindow()) {
                QWidget *w = static_cast<QWidget *>(ch);
                w->d->updateRequestedRegion(gpos + w->pos());
            }
    }

}

QRegion QWidgetPrivate::requestedRegion() const
{
    return data.req_region;
}

void QWidgetPrivate::setChildrenAllocatedDirty()
{
    for (int i = 0; i < children.size(); ++i) {
        QObject *ch = children.at(i);
        if (ch->isWidgetType()) {
            static_cast<QWidget *>(ch)->data->alloc_region_dirty = true;
        }
    }
}

void QWidgetPrivate::setChildrenAllocatedDirty(const QRegion &r, const QWidget *dirty)
{
    for (int i = 0; i < children.size(); ++i) {
        QObject *ch = children.at(i);
        if (ch->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(ch);
            if (r.boundingRect().intersects(w->geometry()))
                w->data->alloc_region_dirty = true;
            if (w == dirty)
                break;
        }
    }
}

// check my hierarchy for dirty allocated regions
bool QWidgetPrivate::isAllocatedRegionDirty() const
{
    if (q->isWindow())
        return false;

    if (data.alloc_region_dirty)
        return true;

    return q->parentWidget()->d->isAllocatedRegionDirty();
}

inline bool QRect::intersects(const QRect &r) const
{
    return (qMax(x1, r.x1) <= qMin(x2, r.x2) &&
             qMax(y1, r.y1) <= qMin(y2, r.y2));
}

QRegion QWidgetPrivate::allocatedRegion() const
{
    if (q->isVisible()) {
        if (q->isWindow()) {
            return data.alloc_region;
        } else {
            if (isAllocatedRegionDirty()) {
                QRegion r(data.req_region);
                QWidgetPrivate *pwd = q->parentWidget()->d;
                r &= pwd->allocatedRegion();
                pwd->updateOverlappingChildren();
                if (pwd->data.overlapping_children) {
                    QObjectList siblings = pwd->children;
                    bool clip=false;
                    for (int i = 0; i < siblings.size(); ++i) {
                        QObject *ch = siblings.at(i);
                        if (ch->isWidgetType()) {
                            QWidget *w = static_cast<QWidget*>(ch);
                            if (w == q)
                                clip=true;
                            else if (clip && !w->isWindow() && w->isVisible()) {
                                if (w->geometry().intersects(q->geometry()))
                                        r -= w->data->req_region;
                            }
                        }
                    }

                }

                // if I'm dirty, so are my chlidren.
                for (int i = 0; i < children.size(); ++i) {
                    QObject *ch = children.at(i);
                    if (ch->isWidgetType()) {
                        QWidget *w = static_cast<QWidget *>(ch);
                        if (!w->isWindow())
                            w->data->alloc_region_dirty = true;
                    }
                }

                data.alloc_region = r;
                data.alloc_region_dirty = false;
                data.paintable_region_dirty = true;
            }
            return data.alloc_region;
        }
    } else {
        return QRegion();
    }
}

QRegion QWidgetPrivate::paintableRegion() const
{
    if (q->isVisible()) {
        if (data.paintable_region_dirty || isAllocatedRegionDirty()) {
            data.paintable_region = allocatedRegion();
            for (int i = 0; i < children.size(); ++i) {
                QObject *ch = children.at(i);
                if (ch->isWidgetType()) {
                    QWidget *w = static_cast<QWidget *>(ch);
                    if (!w->isWindow() && w->isVisible())
                        data.paintable_region -= w->data->req_region;
                }
            }

            data.paintable_region_dirty = false;
#ifndef QT_NO_CURSOR
            // The change in paintable region may have result in the
            // cursor now being within my region.
            updateCursor(data.paintable_region);
#endif
        }
        if (!q->isWindow())
            return data.paintable_region;
        else {
            QRegion r(data.paintable_region);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra)
                r += d->extra->topextra->decor_allocated_region;
#endif
            return r;
        }
    }

    return QRegion();
}

void QWidget::setMask(const QRegion& region)
{
    d->createExtra();

    if (region == d->extra->mask)
        return;

    data->alloc_region_dirty = true;

    d->extra->mask = region;

    if (isWindow()) {
        if (!region.isEmpty()) {
            data->req_region = d->extra->mask;
            data->req_region.translate(data->crect.x(),data->crect.y()); //###expensive?
            data->req_region &= data->crect; //??? this is optional
        } else
            data->req_region = QRegion(data->crect);
        data->req_region = qt_screen->mapToDevice(data->req_region, QSize(qt_screen->width(), qt_screen->height()));
    }
    if (isVisible()) {
        if (isWindow()) {
            QRegion rgn(data->req_region);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
                QRegion wmr = d->extra->topextra->qwsManager->region();
                wmr = qt_screen->mapToDevice(wmr, QSize(qt_screen->width(), qt_screen->height()));
                rgn += wmr;
            }
#endif
            d->requestWindowRegion(rgn);
        } else {
            d->updateRequestedRegion(mapToGlobal(QPoint(0,0)));
            parentWidget()->data->paintable_region_dirty = true;
            parentWidget()->repaint(geometry());
            paint_children(parentWidget(),geometry(),true);
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
    if (qt_last_x && (!QWidget::mouseGrabber() || QWidget::mouseGrabber() == q) &&
            qt_last_cursor != (WId)q->cursor().handle() && !qws_overrideCursor) {
        QSize s(qt_screen->width(), qt_screen->height());
        QPoint pos = qt_screen->mapToDevice(QPoint(*qt_last_x, *qt_last_y), s);
        if (r.contains(pos))
            q->qwsDisplay()->selectCursor(const_cast<QWidget*>(q), q->cursor().handle());
    }
}
#endif


void QWidget::setWindowOpacity(qreal)
{
}

qreal QWidget::windowOpacity() const
{
    return 1.0;
}

static QSingleCleanupHandler<QWSPaintEngine> qt_paintengine_cleanup_handler;
static QWSPaintEngine *qt_widget_paintengine = 0;
/*!
    Returns the widget's paint engine. (This defaults to the
    QQWSPaintEngine.)
*/
QPaintEngine *QWidget::paintEngine() const
{
    qWarning("QWidget::paintEngine() should no longer be called");

    if (!qt_widget_paintengine) {
        qt_widget_paintengine = new QWSPaintEngine();
        qt_paintengine_cleanup_handler.set(&qt_widget_paintengine);
    }
    if (qt_widget_paintengine->isActive()) {
        if (d->extraPaintEngine)
            return d->extraPaintEngine;
        const_cast<QWidget *>(this)->d->extraPaintEngine = new QWSPaintEngine();
        return d->extraPaintEngine;
    }
    return qt_widget_paintengine;
}
