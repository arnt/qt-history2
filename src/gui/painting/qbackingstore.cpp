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
    static char checked_env = -1;
    if(checked_env == -1)
        checked_env = (qgetenv("QT_NO_BACKINGSTORE") == "1") ? 1 : 0;

    bool ret = false;
    if(checked_env == 1)
        ret = false;
    else
        ret = !tlw->testAttribute(Qt::WA_PaintOnScreen);
    return ret;
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
        QRegion area = widget->d_func()->clipRegion();
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
        if(QWExtra *extra = widget->d_func()->extraData()) {
            if(!extra->mask.isEmpty())
                wrgn &= extra->mask;
        }
        updateWidget_sys(wrgn, widget);
        wrgn.translate(widget->mapTo(tlw, QPoint(0, 0)));
    }
    dirty += wrgn;
}

void QWidgetBackingStore::paintBuffer(QWidget *widget, const QPoint &offset, uint flags)
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
        GC gc = XCreateGC(widget->d_func()->xinfo.display(), buffer.handle(), 0, 0);
        QPoint tl = widget->d_func()->mapToWS(offset);
        XCopyArea(X11->display, buffer.handle(), widget->handle(), gc,
                  tl.x(), tl.y(), widget->width(), widget->height(),
                  0, 0);
        XFreeGC(widget->d_func()->xinfo.display(), gc);
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
                if(!child->isWindow())
                    paintBuffer(child, offset+child->pos(), flags & ~(AsRoot));
            }
        }
    }
}

void QWidgetBackingStore::cleanRegion(const QRegion &rgn, QWidget *widget)
{
    if(!isBuffered())
        return;

    QRegion toClean;
    QPoint tlwOffset(0, 0);
    if(buffer.size() != tlw->size()) {
#if defined(Q_WS_X11)
        extern int qt_x11_preferred_pixmap_depth;
        int old_qt_x11_preferred_pixmap_depth = qt_x11_preferred_pixmap_depth;
        qt_x11_preferred_pixmap_depth = widget->x11Info().depth();
        buffer = QPixmap(tlw->size());
        qt_x11_preferred_pixmap_depth = old_qt_x11_preferred_pixmap_depth;
#endif
        toClean = QRegion(0, 0, tlw->width(), tlw->height());
        widget = tlw;
    } else if(widget && widget != tlw) {
        QWidget *background = 0;
        for(QWidget *w = widget; w; w = w->parentWidget()) {
            if(w->isWindow() || isOpaque(w)) {
                background = w;
                break;
            }
        }
        if(background) {
            tlwOffset = background->mapTo(tlw, QPoint(0, 0));

            QRegion req(rgn);
            if(widget != background)
                req.translate(widget->mapTo(background, QPoint(0, 0)));
            toClean = req;

            QRegion dirtyTrans(dirty);
            dirtyTrans.translate(-tlwOffset);
            toClean &= dirtyTrans;

            toClean &= background->rect();

            widget = background;
        }
    } else {
        widget = tlw;
        toClean = dirty;
    }

    if(!toClean.isEmpty()) {
        {
            QRegion cleaned = toClean;
            cleaned.translate(tlwOffset);
            dirty -= cleaned;
        }
        paintWidget(toClean, widget, tlwOffset, Recursive|PaintSym|AsRoot);
#ifdef Q_WS_X11
        QStack<QWidget*> widgets;
        widgets.push(tlw);
        while(!widgets.isEmpty()) {
            QWidget *widget = widgets.pop();
            widget->d_func()->updateSystemBackground();
            QObjectList children = widget->children();
            for(int i = 0; i < children.size(); ++i) {
                if(QWidget *child = qobject_cast<QWidget*>(children.at(i))) {
                    if(!child->isWindow())
                        widgets.push(child);
                }
            }
        }
#else
    } else if(widget && widget != tlw) {
        QWidget *background = 0;
        for(QWidget *w = widget; w; w = w->parentWidget()) {
            if(w->isWindow() || isOpaque(w)) {
                background = w;
                break;
            }
        }
        if(background)
            paintBuffer(background, background->mapTo(tlw, QPoint(0, 0)), AsRoot|Recursive);
    } else {
        paintBuffer(tlw, QPoint(0, 0), AsRoot|Recursive);
#endif
    }
}

bool QWidgetBackingStore::isOpaque(const QWidget *widget)
{
    if (!widget->testAttribute(Qt::WA_NoBackground) && !widget->testAttribute(Qt::WA_NoSystemBackground)) {
        const QPalette &pal = widget->palette();
        QPalette::ColorRole bg = widget->backgroundRole();
        QBrush bgBrush = pal.brush(bg);
        return (bgBrush.style() != Qt::NoBrush && bgBrush.isOpaque() &&
                ((widget->isWindow() || widget->windowType() == Qt::SubWindow)
                 || (widget->d_func()->bg_role != QPalette::NoRole || (pal.resolve() & (1<<bg)))));
    }
    return false;
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
    if(!widget->isVisible() || widget->isHidden() ||
       !widget->updatesEnabled() || !widget->testAttribute(Qt::WA_Mapped))
       return;

    //update the "in paint event" flag
    if (widget->testAttribute(Qt::WA_WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");
    widget->setAttribute(Qt::WA_WState_InPaintEvent);

    //clip away the new area
    if(isBuffered()) {
        QPainter::setRedirected(widget, &buffer, -offset);
        QRegion wrgn = rgn;
        wrgn.translate(offset);
        buffer.paintEngine()->setSystemClip(wrgn);
#ifdef Q_WS_WIN
        QRasterPaintEngine *engine = (QRasterPaintEngine*) buffer.paintEngine();
        HDC engine_dc = engine->getDC();
	SelectClipRgn(engine_dc, wrgn.handle());
        engine->releaseDC(engine_dc);
#endif
    } else {
        buffer.paintEngine()->setSystemClip(rgn);
   }

    //paint the background
    QWidget *background = 0;
    if(hasBackground(widget)) {
        background = widget;
    } else if(flags & AsRoot) {
        for(QWidget *w = widget; w; w = w->parentWidget()) {
            if(w->isWindow() || hasBackground(w)) {
                background = w;
                break;
            }
        }
    }
    if(background) {
        QPainter p(widget); // We shall use it only once
        p.fillRect(widget->rect(), background->palette().brush(background->backgroundRole()));
    }

#if 0
    qDebug() << "painting" << widget << "opaque ==" << hasBackground(widget) << "background ==" << background;
    qDebug() << "clipping to" << rgn << "location == " << offset
             << "geometry ==" << QRect(widget->mapTo(tlw, QPoint(0, 0)), widget->size());
    fflush(stderr);
#endif

    //actually send the paint event
    widget->setAttribute(Qt::WA_PendingUpdate, false);
    paintWidget_sys(rgn, widget);

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

    if((flags & PaintSym) && isBuffered())
        paintBuffer(widget, offset, 0);

    //be recursive
    if(flags & Recursive) {
        const QObjectList children = widget->children();
        for(int i = 0; i < children.size(); ++i) {
            if(QWidget *child = qobject_cast<QWidget*>(children.at(i))) {
                if(!child->isWindow() && widget->isVisible() &&
                   widget->updatesEnabled() && widget->testAttribute(Qt::WA_Mapped)) {
                    QRegion subRegion(rgn);
                    subRegion.translate(-child->pos());
                    subRegion &= child->rect();
                    if(QWExtra *extra = child->d_func()->extraData()) {
                        if(!extra->mask.isEmpty())
                            subRegion &= extra->mask;
                    }
                    if(!subRegion.isEmpty())
                        paintWidget(subRegion, child, offset+child->pos(), flags & ~(AsRoot));
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

