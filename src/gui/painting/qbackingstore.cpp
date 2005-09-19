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
#include <qglobal.h>
#include <qapplication.h>
#ifdef Q_WS_WIN
# include "qt_windows.h"
# include <private/qpaintengine_raster_p.h>
#endif
#include "qbackingstore_p.h"
#include "private/qwidget_p.h"
#include <qdebug.h>
#include <qstack.h>
#include <qevent.h>
#ifdef Q_WS_X11
# include "private/qt_x11_p.h"
#endif

#ifdef Q_WS_QWS
#include <qwsdisplay_qws.h>
#include <qapplication.h>
#include <qwsmanager_qws.h>
#endif

/*****************************************************************************
  Top Level Window backing store
 *****************************************************************************/

extern bool qt_sendSpontaneousEvent(QObject*, QEvent*); // qapplication_xxx.cpp

bool QWidgetBackingStore::paintOnScreen(QWidget *w)
{
    if (w && (w->testAttribute(Qt::WA_PaintOnScreen) || w->testAttribute(Qt::WA_NoSystemBackground)))
        return true;
    static signed char checked_env = -1;
    if(checked_env == -1)
        checked_env = (qgetenv("QT_ONSCREEN_PAINT") == "1") ? 1 : 0;

    return (checked_env == 1);
}

#ifdef Q_WS_QWS
static int yWinId = 0;

static void createYellowThing()
{
    if (yWinId == 0) {
        yWinId = QWidget::qwsDisplay()->takeId();
        QWidget::qwsDisplay()->nameRegion(yWinId, "Debug flush paint", "Silly yellow thing");
        QWidget::qwsDisplay()->setAltitude(yWinId, 1, true);
    }
}

static bool qt_flushUpdate(const QRegion &globalRgnHack)
{
    static int checked_env = -1;
    if(checked_env == -1) {
        checked_env = qgetenv("QT_FLUSH_UPDATE").toInt();
        if (checked_env != 0)
            createYellowThing();
    }

    if (checked_env == 0)
        return false;


    QRegion globalRgn = globalRgnHack.boundingRect().adjusted(-10,-10,10,10);


    QWidget::qwsDisplay()->requestRegion(yWinId, -1, false, globalRgn);
    QWidget::qwsDisplay()->setAltitude(yWinId, 1, true);
    QWidget::qwsDisplay()->repaintRegion(yWinId, false, globalRgn);

    ::usleep(10000*checked_env);
    QWidget::qwsDisplay()->requestRegion(yWinId, -1, false, QRegion());
    ::usleep(10000*checked_env);
    return true;
}

static bool qt_flushPaint(QWidget *widget, const QRegion &toBePainted)
{
    static int checked_env = -1;
    if(checked_env == -1) {
        checked_env = qgetenv("QT_FLUSH_PAINT").toInt();
        if (checked_env != 0)
            createYellowThing();
    }

    if (checked_env == 0)
        return false;

    QRegion globalRgn = toBePainted;
    globalRgn.translate(widget->mapToGlobal(QPoint()));

    QWidget::qwsDisplay()->requestRegion(yWinId, -1, false, globalRgn);
    QWidget::qwsDisplay()->repaintRegion(yWinId, false, globalRgn);

    ::usleep(20000*checked_env);
    QWidget::qwsDisplay()->requestRegion(yWinId, -1, false, QRegion());
    return true;
}
#else
static bool qt_flushPaint(QWidget *widget, const QRegion &toBePainted)
{
    static signed char checked_env = -1;
    if(checked_env == -1)
        checked_env = (qgetenv("QT_FLUSH_PAINT") == "1") ? 1 : 0;

    if (checked_env != 1)
        return false;

    //flags to fool painter
   bool paintUnclipped = widget->testAttribute(Qt::WA_PaintUnclipped);
    if (!QWidgetBackingStore::paintOnScreen(widget))
        widget->setAttribute(Qt::WA_PaintUnclipped);
#ifndef Q_WS_QWS
    //setup the engine
    QPaintEngine *pe = widget->paintEngine();
    pe->setSystemClip(toBePainted);
#endif
    {
        QPainter p(widget);
        p.setClipRegion(toBePainted);
        p.fillRect(widget->rect(), Qt::yellow);
        p.end();
    }

    //restore
    widget->setAttribute(Qt::WA_PaintUnclipped, paintUnclipped);
#ifndef Q_WS_QWS
    pe->setSystemClip(QRegion());

    //flush
    if (pe->type() == QPaintEngine::Raster) {
        QRasterPaintEngine *rpe = static_cast<QRasterPaintEngine *>(pe);
        rpe->flush(widget, QPoint());
    }
    QApplication::syncX();
#endif
#if defined(Q_OS_UNIX)
    ::usleep(20000);
#elif defined(Q_OS_WIN)
    ::Sleep(25);
#endif
    return true;
}
#endif

void qt_syncBackingStore(QRegion rgn, QWidget *widget)
{
    if (!QWidgetBackingStore::paintOnScreen(widget)) {
        QWidget *tlw = widget->window();
        tlw->d_func()->topData()->backingStore->cleanRegion(rgn, widget);
    } else {
        widget->repaint(rgn);
    }
}

#if defined(Q_WS_X11)
void qt_syncBackingStore(QWidget *widget)
{
    const QRegion dirty =  widget->d_func()->dirtyOnScreen;
    QWidget *tlw = widget->window();
    if (!QWidgetBackingStore::paintOnScreen(widget))
        tlw->d_func()->topData()->backingStore->cleanRegion(dirty, widget);
    else
        widget->repaint(dirty);
}
#elif defined(Q_WS_QWS)
void qt_syncBackingStore(QWidget *widget)
{
    QWidget *tlw = widget->window();
    QTLWExtra *topData = tlw->d_func()->topData();

//    QRegion toClean = tlw->rect();
    QRegion toClean = topData->backingStore->dirty;

#if 0
    qDebug() << "qt_syncBackingStore" << tlw << tlw->rect();
    qDebug() << "dirty ==" << topData->backingStore->dirty;
#endif
    if (!toClean.isEmpty())
        topData->backingStore->cleanRegion(toClean, tlw);
}

#endif

QWidgetBackingStore::QWidgetBackingStore(QWidget *t) : tlw(t)
#ifdef Q_WS_WIN
                                                     , buffer(t)
#endif
                                                     , dirtyBufferSize(false)
{

}

QWidgetBackingStore::~QWidgetBackingStore()
{

}

void QWidgetBackingStore::scrollRegion(const QRegion &rgn, int dx, int dy, QWidget *widget)
{
    QRegion wrgn(rgn);
#if 1
    QRegion area = widget->d_func()->clipRect();
    if(area.isEmpty())
        return;
    wrgn &= area;

    QRegion newrgn(rgn);
    newrgn.translate(dx, dy);
    wrgn += newrgn;
#else
    QRegion area = widget->d_func()->clipRegion();
    if(area.isEmpty())
        return;
    wrgn &= area;

    QRegion newrgn(rgn);
    newrgn.translate(dx, dy);

    if(isOpaque(widget))
        wrgn ^= newrgn;
    else
        wrgn += newrgn;

    if(QWExtra *extra = widget->d_func()->extraData()) {
        if(!extra->mask.isEmpty())
            wrgn &= extra->mask;
    }
    QPoint pos(widget->mapTo(tlw, QPoint(0, 0)));
    //wrgn.translate(pos);
#if defined(Q_WS_WIN)
    QRasterPaintEngine *engine = (QRasterPaintEngine*) buffer.paintEngine();
    HDC engine_dc = engine->getDC();
    BitBlt(engine_dc, pos.x()+dx, pos.y()+dy, widget->width(), widget->height(),
           engine_dc, pos.x(), pos.y(), SRCCOPY);
    engine->releaseDC(engine_dc);
#elif defined(Q_WS_X11)
    GC gc = XCreateGC(widget->d_func()->xinfo.display(), buffer.handle(), 0, 0);
    XCopyArea(X11->display, buffer.handle(), buffer.handle(), gc,
              pos.x(), pos.y(), widget->width(), widget->height(),
              pos.x()+dx, pos.y()+dy);
    XFreeGC(widget->d_func()->xinfo.display(), gc);
#endif
}
#endif

    dirtyRegion(wrgn, widget);
}

void QWidgetBackingStore::dirtyRegion(const QRegion &rgn, QWidget *widget)
{
    QRegion wrgn(rgn);
    Q_ASSERT(widget->window() == tlw);
    if(!widget->isVisible() || !widget->updatesEnabled())
        return;
    wrgn &= widget->d_func()->clipRect();
    widget->d_func()->dirtyWidget_sys(wrgn);
    if(!QWidgetBackingStore::paintOnScreen(widget)) {
        wrgn.translate(widget->mapTo(tlw, QPoint(0, 0)));
        dirty += wrgn;
    }
}

void QWidgetBackingStore::cleanScreen(const QRegion &rgn, QWidget *widget, const QPoint &offset, uint flags)
{
    if (rgn.isEmpty())
        return;
#ifdef Q_WS_QWS
    Q_UNUSED(offset);
    QWidget *win = widget->window();
    QBrush bgBrush = win->palette().brush(win->backgroundRole());
    bool opaque = bgBrush.style() == Qt::NoBrush || bgBrush.isOpaque();
    QRegion globalrgn = rgn;
    globalrgn.translate(win->geometry().topLeft());
    win->qwsDisplay()->repaintRegion(win->data->winid, opaque, globalrgn);

    qt_flushUpdate(globalrgn);
#else
    if (!QWidgetBackingStore::paintOnScreen(widget)){
        widget->d_func()->cleanWidget_sys(rgn);
#if defined(Q_WS_WIN)
        QRasterPaintEngine *engine = (QRasterPaintEngine*) buffer.paintEngine();
        HDC engine_dc = engine->getDC();
        HDC widget_dc = (HDC) widget->d_func()->hd;
        bool tmp_widget_dc = false;
        if (!widget_dc) {
            widget_dc = GetDC(widget->winId());
            tmp_widget_dc = true;
        }
        BitBlt(widget_dc, 0, 0, widget->width(), widget->height(),
               engine_dc, offset.x(), offset.y(), SRCCOPY);
        if (tmp_widget_dc)
            ReleaseDC(widget->winId(), widget_dc);
        engine->releaseDC(engine_dc);
#elif defined(Q_WS_X11)
        extern void *qt_getClipRects(const QRegion &r, int &num); // in qpaintengine_x11.cpp
        GC gc = XCreateGC(X11->display, buffer.handle(), 0, 0);
        int num;
        XRectangle *rects = (XRectangle *)qt_getClipRects(rgn, num);
        XSetClipRectangles(X11->display, gc, 0, 0, rects, num, YXBanded );
        XSetGraphicsExposures(X11->display, gc, False);
//         XFillRectangle(X11->display, widget->handle(), gc, 0, 0, widget->width(), widget->height());
        XCopyArea(X11->display, buffer.handle(), widget->handle(), gc,
                  offset.x(), offset.y(), widget->width(), widget->height(), 0, 0);
        XFreeGC(X11->display, gc);
#endif
    }

    if(flags & Recursive) {
        const QObjectList children = widget->children();
        for(int i = 0; i < children.size(); ++i) {
            if(QWidget *child = qobject_cast<QWidget*>(children.at(i))) {
                if(!child->isWindow()) {
                    QRegion childRegion(rgn);
                    childRegion.translate(-child->pos());
                    childRegion &= child->d_func()->clipRect();
                    if(!childRegion.isEmpty())
                        cleanScreen(childRegion, child, offset+child->pos(), flags);
                }
            }
        }
    }
#endif
}

void QWidgetBackingStore::cleanRegion(const QRegion &rgn, QWidget *widget)
{
    if (!widget->isVisible() || !widget->updatesEnabled() || !tlw->testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;

    if(!QWidgetBackingStore::paintOnScreen(widget)) {
        QRegion toClean;

#ifdef Q_WS_QWS
        //QWExtra *extra = tlw->d_func()->extra;
        QRect tlwFrame = tlw->frameGeometry();
        QSize tlwSize = tlwFrame.size();
#else
        QSize tlwSize = tlw->size();
#endif
        if (buffer.size() != tlwSize || dirtyBufferSize) {
#if defined(Q_WS_X11)
            extern int qt_x11_preferred_pixmap_depth;
            int old_qt_x11_preferred_pixmap_depth = qt_x11_preferred_pixmap_depth;
            qt_x11_preferred_pixmap_depth = widget->x11Info().depth();
            buffer = QPixmap(tlwSize);
            qt_x11_preferred_pixmap_depth = old_qt_x11_preferred_pixmap_depth;
#elif defined(Q_WS_WIN)
            if (buffer.paintEngine())
                ((QRasterPaintEngine *)buffer.paintEngine())->releaseBuffer();
#elif defined(Q_WS_QWS)
            QRegion tlwRegion = tlwFrame;
            tlwOffset = tlw->geometry().topLeft() - tlwFrame.topLeft();
            if (!tlw->d_func()->extra->mask.isEmpty()) {
                tlwRegion = tlw->d_func()->extra->mask;
                tlwRegion.translate(tlw->geometry().topLeft());
                tlwRegion &= tlwFrame;
                tlwSize = tlwRegion.boundingRect().size();
                tlwOffset = tlw->geometry().topLeft() - tlwRegion.boundingRect().topLeft();
            }
            buffer.create(tlwSize);
            QBrush bgBrush = tlw->palette().brush(tlw->backgroundRole());
            bool opaque = bgBrush.style() == Qt::NoBrush || bgBrush.isOpaque();
            QWidget::qwsDisplay()->requestRegion(tlw->data->winid, buffer.memoryId(), opaque, tlwRegion);
            QTLWExtra *topextra = tlw->d_func()->extra->topextra;
            if (topextra->qwsManager)
                QApplication::postEvent(topextra->qwsManager, new QPaintEvent(tlwFrame));
#endif
            toClean = QRegion(0, 0, tlw->width(), tlw->height());
            dirtyBufferSize = false;
        } else {
            toClean = dirty;
        }

#ifdef Q_WS_QWS
        QRegion toFlush = toClean; //??? correct as long as we don't have fast scroll
#endif
        if(!toClean.isEmpty()) {
#ifdef Q_WS_QWS
            buffer.lock();
#endif
            cleanBuffer(toClean, tlw, tlwOffset, Recursive|AsRoot);
#ifdef Q_WS_QWS
            buffer.unlock();
#endif
            dirty -= toClean;
        }
#ifndef Q_WS_QWS
        QRegion toFlush = rgn;
#endif
        toFlush.translate(widget->mapTo(tlw, QPoint()));
        cleanScreen(toFlush, tlw, tlwOffset, Recursive);
    }
}

#ifdef Q_WS_QWS
void QWidgetBackingStore::releaseBuffer()
{
    buffer.detach();
    QWidget::qwsDisplay()->requestRegion(tlw->data->winid, 0, true, QRegion(0));
}
#endif


bool QWidgetBackingStore::isOpaque(const QWidget *widget)
{
    return widget->d_func()->isOpaque();
}

bool QWidgetBackingStore::hasBackground(const QWidget *widget)
{
    if (!widget->testAttribute(Qt::WA_NoBackground) && !widget->testAttribute(Qt::WA_NoSystemBackground)) {
        const QPalette &pal = widget->palette();
        QPalette::ColorRole bg = widget->backgroundRole();
        QBrush bgBrush = pal.brush(bg);
        return (bgBrush.style() != Qt::NoBrush &&
                ((widget->isWindow() || widget->windowType() == Qt::SubWindow)
                 || (widget->d_func()->bg_role != QPalette::NoRole || (pal.resolve() & (1<<bg)))));
    }
    return false;
}

void QWidgetBackingStore::cleanBuffer(const QRegion &rgn, QWidget *widget, const QPoint &offset, uint flags)
{
    if (!widget->isVisible() || !widget->updatesEnabled() || rgn.isEmpty())
        return;

    QRegion toBePainted  = (rgn & widget->d_func()->visibleRegion());
    if (!QWidgetBackingStore::paintOnScreen(widget) && !toBePainted.isEmpty()) {
        //update the "in paint event" flag
        if (widget->testAttribute(Qt::WA_WState_InPaintEvent))
            qWarning("QWidget::repaint: recursive repaint detected.");
        widget->setAttribute(Qt::WA_WState_InPaintEvent);

        //clip away the new area
#ifdef Q_WS_QWS
        QPainter::setRedirected(widget, buffer.pixmap(), -offset); //###
        bool flushed = qt_flushPaint(widget, toBePainted);
#else
        bool flushed = qt_flushPaint(widget, toBePainted);
        QPainter::setRedirected(widget, &buffer, -offset);
#endif
        QRegion wrgn = toBePainted;
        wrgn.translate(offset);
#ifdef Q_WS_QWS
        buffer.pixmap()->paintEngine()->setSystemClip(wrgn); //###
#else
        buffer.paintEngine()->setSystemClip(wrgn);
#endif
#ifdef Q_WS_WIN
        QRasterPaintEngine *engine = (QRasterPaintEngine*) buffer.paintEngine();
        HDC engine_dc = engine->getDC();
        SelectClipRgn(engine_dc, wrgn.handle());
        engine->releaseDC(engine_dc);
#endif

        //paint the background
        if((flags & AsRoot) || hasBackground(widget)) {
            QPainter p(widget);
            const QBrush bg = widget->palette().brush(widget->backgroundRole());
#ifdef Q_WS_QWS
            if (flags & AsRoot)
                p.setCompositionMode(QPainter::CompositionMode_Source); //copy alpha straight in
#endif
            if (bg.style() == Qt::TexturePattern)
                p.drawTiledPixmap(widget->rect(), bg.texture());
            else
                p.fillRect(widget->rect(), bg);
        }

#if 0
        qDebug() << "painting" << widget << "opaque ==" << hasBackground(widget);
        qDebug() << "clipping to" << toBePainted << "location == " << offset
                 << "geometry ==" << QRect(widget->mapTo(tlw, QPoint(0, 0)), widget->size());
        fflush(stderr);
#endif

        //actually send the paint event
        widget->setAttribute(Qt::WA_PendingUpdate, false);
        QPaintEvent e(toBePainted);
        qt_sendSpontaneousEvent(widget, &e);

        //restore
#ifdef Q_WS_QWS
        buffer.pixmap()->paintEngine()->setSystemClip(QRegion()); //###
#else
        buffer.paintEngine()->setSystemClip(QRegion());
#endif
        QPainter::restoreRedirected(widget);

        widget->setAttribute(Qt::WA_WState_InPaintEvent, false);
        if(!widget->testAttribute(Qt::WA_PaintOutsidePaintEvent) && widget->paintingActive())
            qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

        if (flushed)
            cleanScreen(toBePainted, widget, offset, 0);
    } else if(widget == tlw) {
#ifdef Q_WS_QWS
            QPainter p(buffer.pixmap());
#else
            QPainter p(&buffer);
#endif
            p.setClipRegion(toBePainted);
            const QBrush bg = widget->palette().brush(widget->backgroundRole());
            if (bg.style() == Qt::TexturePattern)
                p.drawTiledPixmap(widget->rect(), bg.texture());
            else
                p.fillRect(widget->rect(), bg);
    }

    //be recursive
    if(flags & Recursive) {
        const QObjectList children = widget->children();
        for(int i = 0; i < children.size(); ++i) {
            if(QWidget *child = qobject_cast<QWidget*>(children.at(i))) {
                if(!child->isWindow() && child->isVisible() && child->updatesEnabled()) {
                    QRegion childRegion(rgn);
                    childRegion.translate(-child->pos());
                    childRegion &= child->d_func()->clipRect();
                    if(QWExtra *extra = child->d_func()->extraData()) {
                        if(!extra->mask.isEmpty())
                         childRegion &= extra->mask;
                    }
                    if(!childRegion.isEmpty())
                        cleanBuffer(childRegion, child, offset+child->pos(), flags & ~(AsRoot));
                }
            }
        }
    }
}

/* cross-platform QWidget code */
void QWidgetPrivate::scrollBuffer(const QRegion &rgn, int dx, int dy)
{
    Q_Q(QWidget);
    QWidget *tlw = q->window();
    QTLWExtra* x = tlw->d_func()->topData();
    x->backingStore->scrollRegion(rgn, dx, dy, q);
}

void QWidgetPrivate::invalidateBuffer(const QRegion &rgn)
{
    if(qApp && qApp->closingDown())
        return;
    Q_Q(QWidget);
    QWidget *tlw = q->window();
    QTLWExtra* x = tlw->d_func()->topData();
    x->backingStore->dirtyRegion(rgn, q);
}

void QWidget::repaint(const QRegion& rgn)
{
    if (!isVisible() || !updatesEnabled() || rgn.isEmpty())
        return;
    if (!QWidgetBackingStore::paintOnScreen(this)) {
        QWidget *tlw = window();
        QTLWExtra* x = tlw->d_func()->topData();
        x->backingStore->dirtyRegion(rgn, this);
        x->backingStore->cleanRegion(rgn, this);
    } else {
        Q_D(QWidget);
        d->cleanWidget_sys(rgn);

        //update the "in paint event" flag
        if (testAttribute(Qt::WA_WState_InPaintEvent))
            qWarning("QWidget::repaint: recursive repaint detected.");
        setAttribute(Qt::WA_WState_InPaintEvent);

        qt_flushPaint(this, rgn);

        QPaintEngine *engine = paintEngine();
        engine->setSystemClip(rgn);

        //paint the background
        QPaintEvent e(rgn);
        if (!testAttribute(Qt::WA_NoBackground)
            && !testAttribute(Qt::WA_NoSystemBackground)) {
            d->composeBackground(rgn.boundingRect());
#ifdef QT3_SUPPORT
            e.setErased(true);
#endif
        }

        //actually send the paint event
        setAttribute(Qt::WA_PendingUpdate, false);
        QApplication::sendSpontaneousEvent(this, &e);

#ifdef Q_WS_WIN
        if (engine && engine->type() == QPaintEngine::Raster) {
            bool tmp_dc = !d->hd;
            if (tmp_dc)
                d->hd = GetDC(winId());
            static_cast<QRasterPaintEngine *>(engine)->flush(this, QPoint(0, 0));
            if (tmp_dc) {
                ReleaseDC(winId(), (HDC)d->hd);
                d->hd = 0;
            }
        }
#endif
        engine->setSystemClip(QRegion());

        setAttribute(Qt::WA_WState_InPaintEvent, false);
        if(!testAttribute(Qt::WA_PaintOutsidePaintEvent) && paintingActive())
            qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");
    }
}

void QWidget::update()
{
    update(rect());
}

void QWidget::update(const QRect &r)
{
    update(QRegion(r));
}

void QWidget::update(const QRegion& rgn)
{
    if(!isVisible() || !updatesEnabled() || rgn.isEmpty())
        return;
    setAttribute(Qt::WA_PendingUpdate);

    QWidget *tlw = window();
    QTLWExtra* x = tlw->d_func()->topData();
    x->backingStore->dirtyRegion(rgn, this);
}

