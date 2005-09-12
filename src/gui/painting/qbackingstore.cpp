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
#ifdef Q_WS_X11
# include "private/qt_x11_p.h"
#endif

/*****************************************************************************
  Top Level Window backing store
 *****************************************************************************/

static bool qt_bufferBackingStore(const QWidget *tlw)
{
    return true;
    static signed char checked_env = -1;
    if(checked_env == -1)
        checked_env = (qgetenv("QT_NO_BACKINGSTORE") == "1") ? 1 : 0;

    bool ret = false;
    if(checked_env == 1)
        ret = false;
    else
        ret = !tlw->testAttribute(Qt::WA_PaintOnScreen);
    return ret;
}

static bool qt_flushPaint()
{
    static signed char checked_env = -1;
    if(checked_env == -1)
        checked_env = (qgetenv("QT_FLUSH_PAINT") == "1") ? 1 : 0;

    return (checked_env == 1);
}

void qt_syncBackingStores()
{
    QWidgetList tlws = QApplication::topLevelWidgets();
    for(int i = 0; i < tlws.size(); ++i) {
        QWidget *tlw = tlws.at(i);
        if(qt_bufferBackingStore(tlw))
            qt_syncBackingStore(tlw->d_func()->topData()->backingStore->dirty, tlw);
    }
}

void qt_syncBackingStore(QRegion rgn, QWidget *widget)
{
#ifdef Q_WS_WIN
    ValidateRgn(widget->winId(), rgn.handle());
#endif
    QWidget *tlw = widget->window();

    if(qt_bufferBackingStore(tlw))
        tlw->d_func()->topData()->backingStore->cleanRegion(rgn, widget);
    else
        tlw->d_func()->topData()->backingStore->paintWidget(rgn, widget,
                                                            widget->mapTo(tlw, QPoint(0, 0)),
                                                            QWidgetBackingStore::AsRoot);
}

QWidgetBackingStore::QWidgetBackingStore(QWidget *t) : tlw(t)
#ifdef Q_WS_WIN
                                                     , buffer(t)
#endif
{

}

QWidgetBackingStore::~QWidgetBackingStore()
{

}

bool QWidgetBackingStore::isBuffered() const
{
    return qt_bufferBackingStore(tlw);
}

void QWidgetBackingStore::scrollRegion(const QRegion &rgn, int dx, int dy, QWidget *widget)
{
    if(!isBuffered())
        return;

    QRegion wrgn(rgn);
#if 1
    if(widget) {
        QRegion area = widget->d_func()->clipRect();
        if(area.isEmpty())
            return;
        wrgn &= area;

        QRegion newrgn(rgn);
        newrgn.translate(dx, dy);
        wrgn += newrgn;
    }
#else
    if(widget) {
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
    if(!isBuffered())
        return;

    QRegion wrgn(rgn);
    if(widget) {
        Q_ASSERT(widget->window() == tlw);
        if(!widget->isVisible() || !widget->updatesEnabled())
            return;
        wrgn &= widget->d_func()->clipRect();
#if 0
        if(QWExtra *extra = widget->d_func()->extraData()) {
            if(!extra->mask.isEmpty())
                wrgn &= extra->mask;
        }
#endif
        updateWidget_sys(wrgn, widget);
        wrgn.translate(widget->mapTo(tlw, QPoint(0, 0)));
    }
    dirty += wrgn;
}

void QWidgetBackingStore::paintBuffer(const QRegion &rgn, QWidget *widget, const QPoint &offset, uint flags)
{
    if(!isBuffered())
        return;

    //update the in paint event flag
    if (widget->testAttribute(Qt::WA_WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");
    widget->setAttribute(Qt::WA_WState_InPaintEvent);

    { //flush the pixmap
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

    //restore
    widget->setAttribute(Qt::WA_WState_InPaintEvent, false);
    if(!widget->testAttribute(Qt::WA_PaintOutsidePaintEvent) && widget->paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

    if(flags & Recursive) {
        const QObjectList children = widget->children();
        for(int i = 0; i < children.size(); ++i) {
            if(QWidget *child = qobject_cast<QWidget*>(children.at(i))) {
                if(!child->isWindow()) {
                    QRegion childRegion(rgn);
                    childRegion.translate(-child->pos());
                    childRegion &= child->d_func()->clipRect();
                    if(!childRegion.isEmpty())
                        paintBuffer(childRegion, child, offset+child->pos(), flags & ~(AsRoot));
                }
            }
        }
    }
}

void QWidgetBackingStore::cleanRegion(const QRegion &rgn, QWidget *widget)
{
    if(!isBuffered())
        return;

    if (!widget->isVisible() || !widget->updatesEnabled() || !tlw->testAttribute(Qt::WA_Mapped) || rgn.isEmpty())
        return;

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

    QRegion toFlush = rgn;
    toFlush.translate(widget->mapTo(tlw, QPoint(0, 0)));
    if(!toClean.isEmpty()) {
        paintWidget(toClean, tlw, QPoint(0, 0), Recursive|Flush|AsRoot);
        toFlush -= toClean;
        dirty -= toClean;
    }

    if(!toFlush.isEmpty())
        paintBuffer(toFlush, tlw, QPoint(0, 0), AsRoot|Recursive);
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

void QWidgetBackingStore::paintWidget(const QRegion &rgn, QWidget *widget, const QPoint &offset, uint flags)
{
    if (!widget->isVisible() || !widget->updatesEnabled() || rgn.isEmpty())
        return;
    QRegion toBePainted  = (rgn & widget->d_func()->visibleRegion());
    if (!toBePainted.isEmpty()) {

        //update the "in paint event" flag
        if (widget->testAttribute(Qt::WA_WState_InPaintEvent))
            qWarning("QWidget::repaint: recursive repaint detected.");
        widget->setAttribute(Qt::WA_WState_InPaintEvent);

        if (qt_flushPaint()) {
            QApplication::flush();

            QPainter p(widget);
            p.setClipRegion(toBePainted);
            p.fillRect(widget->rect(), Qt::yellow);
            p.end();

            QPaintEngine *pe = widget->paintEngine();
            QRasterPaintEngine *rpe = 0;
            if (pe->type() == QPaintEngine::Raster) {
                rpe = (QRasterPaintEngine *) pe;
                rpe->setSystemClip(toBePainted);
                rpe->flush(widget, QPoint());
                rpe->setSystemClip(QRegion());
            }

#if defined(Q_OS_UNIX)
            ::usleep(20000);
#elif defined(Q_OS_WIN)
            ::Sleep(25);
#endif
        }

        //clip away the new area
        if(isBuffered()) {
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
        }

        //paint the background
        if(hasBackground(widget) || (flags & AsRoot)) {
            QPainter p(widget);

            QBrush bg_brush = widget->palette().brush(widget->backgroundRole());
            p.fillRect(widget->rect(), bg_brush);
        }

#if 0
        qDebug() << "painting" << widget << "opaque ==" << hasBackground(widget);
        qDebug() << "clipping to" << toBePainted << "location == " << offset
                 << "geometry ==" << QRect(widget->mapTo(tlw, QPoint(0, 0)), widget->size());
        fflush(stderr);
#endif

        //actually send the paint event
        widget->setAttribute(Qt::WA_PendingUpdate, false);
        paintWidget_sys(toBePainted, widget);

        //restore
        if(isBuffered()) {
            buffer.paintEngine()->setSystemClip(QRegion());
            QPainter::restoreRedirected(widget);
        } else {
            QPaintEngine *engine = widget->paintEngine();
#ifdef Q_WS_WIN
            if (engine && engine->type() == QPaintEngine::Raster) {
                bool tmp_dc = !widget->d_func()->hd;
                if (tmp_dc)
                    widget->d_func()->hd = GetDC(widget->winId());

                static_cast<QRasterPaintEngine *>(engine)->flush(widget, QPoint(0, 0));

                if (tmp_dc) {
                    ReleaseDC(widget->winId(), (HDC)widget->d_func()->hd);
                    widget->d_func()->hd = 0;
                }
            }
#endif

            engine->setSystemClip(QRegion());
        }
        widget->setAttribute(Qt::WA_WState_InPaintEvent, false);
        if(!widget->testAttribute(Qt::WA_PaintOutsidePaintEvent) && widget->paintingActive())
            qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

        if((flags & Flush) && isBuffered())
            paintBuffer(toBePainted, widget, offset, 0);
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
                        paintWidget(childRegion, child, offset+child->pos(), flags & ~(AsRoot));
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
    if(x->backingStore->isBuffered())
        x->backingStore->scrollRegion(rgn, dx, dy, q);
}

void QWidgetPrivate::invalidateBuffer(const QRegion &rgn)
{
    if(qApp && qApp->closingDown())
        return;
    Q_Q(QWidget);
    QWidget *tlw = q->window();
    QTLWExtra* x = tlw->d_func()->topData();
    if(x->backingStore->isBuffered())
        x->backingStore->dirtyRegion(rgn, q);
    else
        x->backingStore->updateWidget_sys(rgn, q);
}

void QWidget::repaint(const QRegion& rgn)
{
    QWidget *tlw = window();
    if (!isVisible() || !updatesEnabled() || rgn.isEmpty())
        return;
    QTLWExtra* x = tlw->d_func()->topData();
    if(x->backingStore->isBuffered()) {
        x->backingStore->dirtyRegion(rgn, this);
        x->backingStore->cleanRegion(rgn, this);
    } else {
        x->backingStore->paintWidget(rgn, this, mapTo(tlw, QPoint(0, 0)),
                                     QWidgetBackingStore::AsRoot);
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
    if(x->backingStore->isBuffered())
        x->backingStore->dirtyRegion(rgn, this);
    else
        x->backingStore->updateWidget_sys(rgn, this);
}

