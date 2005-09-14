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

    //setup the engine
    QPaintEngine *pe = widget->paintEngine();
    pe->setSystemClip(toBePainted);

    {
        QPainter p(widget);
        p.setClipRegion(toBePainted);
        p.fillRect(widget->rect(), Qt::yellow);
        p.end();
    }

    //restore
    widget->setAttribute(Qt::WA_PaintUnclipped, paintUnclipped);
    pe->setSystemClip(QRegion());

    //flush
    if (pe->type() == QPaintEngine::Raster) {
        QRasterPaintEngine *rpe = static_cast<QRasterPaintEngine *>(pe);
        rpe->flush(widget, QPoint());
    }
    QApplication::syncX();

#if defined(Q_OS_UNIX)
    ::usleep(80000);
#elif defined(Q_OS_WIN)
    ::Sleep(25);
#endif
    return true;
}

void qt_syncBackingStore(QRegion rgn, QWidget *widget)
{
    if (!QWidgetBackingStore::paintOnScreen(widget)) {
        QWidget *tlw = widget->window();
        tlw->d_func()->topData()->backingStore->cleanRegion(rgn, widget);
    } else {
        widget->repaint(rgn);
    }
}

#ifdef Q_WS_X11
void qt_syncBackingStore(QWidget *widget)
{
    const QRegion dirty =  widget->d_func()->dirtyOnScreen;
    QWidget *tlw = widget->window();
    if (!QWidgetBackingStore::paintOnScreen(widget))
        tlw->d_func()->topData()->backingStore->cleanRegion(dirty, widget);
    else
        widget->repaint(dirty);
}

//special case hack for config events where sychronisity means that some
//repaints happen inbetween resizes
void qt_syncBackingStore_x11ConfigEvent(QWidget *widget)
{
    if(QWidgetBackingStore::paintOnScreen(widget))
        widget->repaint();
}
#endif

QWidgetBackingStore::QWidgetBackingStore(QWidget *t) : tlw(t)
#ifdef Q_WS_WIN
                                                     , buffer(t)
#endif
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
}

void QWidgetBackingStore::cleanRegion(const QRegion &rgn, QWidget *widget)
{
    if (!widget->isVisible() || !widget->updatesEnabled() || !tlw->testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;

    if(!QWidgetBackingStore::paintOnScreen(widget)) {
        QRegion toClean;

        if (buffer.size() != tlw->size()) {
#if defined(Q_WS_X11)
            extern int qt_x11_preferred_pixmap_depth;
            int old_qt_x11_preferred_pixmap_depth = qt_x11_preferred_pixmap_depth;
            qt_x11_preferred_pixmap_depth = widget->x11Info().depth();
            buffer = QPixmap(tlw->size());
            qt_x11_preferred_pixmap_depth = old_qt_x11_preferred_pixmap_depth;
#elif defined(Q_WS_WIN)
            if (buffer.paintEngine())
                ((QRasterPaintEngine *)buffer.paintEngine())->releaseBuffer();
#endif
            toClean = QRegion(0, 0, tlw->width(), tlw->height());
        } else {
            toClean = dirty;
        }

        if(!toClean.isEmpty()) {
            cleanBuffer(toClean, tlw, QPoint(0, 0), Recursive|AsRoot);
            dirty -= toClean;
        }

        QRegion toFlush = rgn;
        toFlush.translate(widget->mapTo(tlw, QPoint(0, 0)));
        cleanScreen(toFlush, tlw, QPoint(0, 0), Recursive);
    }
}

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

        bool flushed = qt_flushPaint(widget, toBePainted);

        //clip away the new area
        QPainter::setRedirected(widget, &buffer, -offset);
        QRegion wrgn = toBePainted;
        wrgn.translate(offset);
        buffer.paintEngine()->setSystemClip(wrgn);
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
        buffer.paintEngine()->setSystemClip(QRegion());
        QPainter::restoreRedirected(widget);

        widget->setAttribute(Qt::WA_WState_InPaintEvent, false);
        if(!widget->testAttribute(Qt::WA_PaintOutsidePaintEvent) && widget->paintingActive())
            qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

        if (flushed)
            cleanScreen(toBePainted, widget, offset, 0);
    } else if(widget == tlw) {
            QPainter p(&buffer);
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
        d->composeBackground(rgn.boundingRect());

        //actually send the paint event
        setAttribute(Qt::WA_PendingUpdate, false);
        QPaintEvent e(rgn);
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

