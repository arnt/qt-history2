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

#include "qwsmanager_qws.h"

#ifndef QT_NO_QWS_MANAGER

#include "qdrawutil.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qwidget.h"
#include "qmenu.h"
#include "qpainter.h"
#include "private/qpainter_p.h"
#include "qregion.h"
#include "qevent.h"
#include "qcursor.h"
#include "qwsdisplay_qws.h"
#include "qdesktopwidget.h"

#include <private/qwidget_p.h>
#include <private/qwidget_qws_p.h>

#include "qdecorationfactory_qws.h"

#include "qlayout.h"

#include "qwsmanager_p.h"

#include <qdebug.h>

QWidget *QWSManagerPrivate::active = 0;
QPoint QWSManagerPrivate::mousePos;


QWSManagerPrivate::QWSManagerPrivate()
    : QObjectPrivate(), activeRegion(QDecoration::None), managed(0), popup(0),
      previousRegionType(0), previousRegionRepainted(false)
{
    cached_region.regionType = 0;
}

QRegion &QWSManager::cachedRegion()
{
    return d_func()->cached_region.region;
}


QWSManager::QWSManager(QWidget *w)
    : QObject(*new QWSManagerPrivate, (QObject*)0)
{
    d_func()->managed = w;

}

QWSManager::~QWSManager()
{
    Q_D(QWSManager);
#ifndef QT_NO_POPUPMENU
    if (d->popup)
        delete d->popup;
#endif
    if (d->managed == QWSManagerPrivate::active)
        QWSManagerPrivate::active = 0;
}

QWidget *QWSManager::widget()
{
    Q_D(QWSManager);
    return d->managed;
}

QWidget *QWSManager::grabbedMouse()
{
    return QWSManagerPrivate::active;
}

QRegion QWSManager::region()
{
    Q_D(QWSManager);
    return QApplication::qwsDecoration().region(d->managed, d->managed->geometry());
}

bool QWSManager::event(QEvent *e)
{
    if (QObject::event(e))
        return true;

    switch (e->type()) {
        case QEvent::MouseMove:
            mouseMoveEvent((QMouseEvent*)e);
            break;

        case QEvent::MouseButtonPress:
            mousePressEvent((QMouseEvent*)e);
            break;

        case QEvent::MouseButtonRelease:
            mouseReleaseEvent((QMouseEvent*)e);
            break;

        case QEvent::MouseButtonDblClick:
            mouseDoubleClickEvent((QMouseEvent*)e);
            break;

        case QEvent::Paint:
            paintEvent((QPaintEvent*)e);
            break;

        default:
            return false;
            break;
    }

    return true;
}

void QWSManager::mousePressEvent(QMouseEvent *e)
{
    Q_D(QWSManager);
    d->mousePos = e->globalPos();
    d->activeRegion = QApplication::qwsDecoration().regionAt(d->managed, d->mousePos);
    if(d->cached_region.regionType)
        d->previousRegionRepainted |= repaintRegion(d->cached_region.regionType, QDecoration::Pressed);

    if (d->activeRegion == QDecoration::Menu)
        menu(d->managed->geometry().topLeft());
    if (d->activeRegion != QDecoration::None &&
         d->activeRegion != QDecoration::Menu) {
        d->active = d->managed;
        d->managed->grabMouse();
    }
    if (d->activeRegion != QDecoration::None &&
         d->activeRegion != QDecoration::Close &&
         d->activeRegion != QDecoration::Minimize &&
         d->activeRegion != QDecoration::Menu) {
        d->managed->raise();
    }

    if (e->button() == Qt::RightButton) {
        menu(e->globalPos());
    }
}

void QWSManager::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QWSManager);
    d->managed->releaseMouse();
    if (d->cached_region.regionType && d->previousRegionRepainted && QApplication::mouseButtons() == 0) {
        bool doesHover = repaintRegion(d->cached_region.regionType, QDecoration::Hover);
        if (!doesHover) {
            repaintRegion(d->cached_region.regionType, QDecoration::Normal);
            d->previousRegionRepainted = false;
        }
    }

    if (e->button() == Qt::LeftButton) {
        //handleMove();
        int itm = QApplication::qwsDecoration().regionAt(d->managed, e->globalPos());
        int activatedItem = d->activeRegion;
        d->activeRegion = QDecoration::None;
        d->active = 0;
        if (activatedItem == itm)
            QApplication::qwsDecoration().regionClicked(d->managed, itm);
    } else if (d->activeRegion == QDecoration::None) {
        d->active = 0;
    }
}

void QWSManager::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_D(QWSManager);
    if (e->button() == Qt::LeftButton)
        QApplication::qwsDecoration().regionDoubleClicked(d->managed,
            QApplication::qwsDecoration().regionAt(d->managed, e->globalPos()));
}

static inline Qt::CursorShape regionToShape(int region)
{
    if (region == QDecoration::None)
        return Qt::ArrowCursor;

    static struct {
        int region;
        Qt::CursorShape shape;
    } r2s[] = {
        { QDecoration::TopLeft,     Qt::SizeFDiagCursor },
        { QDecoration::Top,         Qt::SizeVerCursor},
        { QDecoration::TopRight,    Qt::SizeBDiagCursor},
        { QDecoration::Left,        Qt::SizeHorCursor},
        { QDecoration::Right,       Qt::SizeHorCursor},
        { QDecoration::BottomLeft,  Qt::SizeBDiagCursor},
        { QDecoration::Bottom,      Qt::SizeVerCursor},
        { QDecoration::BottomRight, Qt::SizeFDiagCursor},
        { QDecoration::None,        Qt::ArrowCursor}
    };

    int i = 0;
    while (region != r2s[i].region && r2s[i].region)
        ++i;
    return r2s[i].shape;
}

void QWSManager::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QWSManager);
    if (d->newCachedRegion(e->globalPos())) {
        if(d->previousRegionType && d->previousRegionRepainted)
            repaintRegion(d->previousRegionType, QDecoration::Normal);
        if(d->cached_region.regionType) {
            d->previousRegionRepainted = repaintRegion(d->cached_region.regionType, QDecoration::Hover);
        }
    }


#ifndef QT_NO_CURSOR
    QWSDisplay *qwsd = QApplication::desktop()->qwsDisplay();
    qwsd->selectCursor(d->managed, regionToShape(d->cachedRegionAt()));
#endif //QT_NO_CURSOR

    if (d->activeRegion)
        handleMove(e->globalPos());
}

void QWSManager::handleMove(QPoint g)
{
    Q_D(QWSManager);
    // don't allow dragging to where the user probably cannot click!
    extern QRect qt_maxWindowRect;
    if (qt_maxWindowRect.isValid()) {
        if (g.x() < qt_maxWindowRect.x())
            g.setX(qt_maxWindowRect.x());
        if (g.y() < qt_maxWindowRect.y())
            g.setY(qt_maxWindowRect.y());
        if (g.x() > qt_maxWindowRect.right())
            g.setX(qt_maxWindowRect.right());
        if (g.y() > qt_maxWindowRect.bottom())
            g.setY(qt_maxWindowRect.bottom());
    }

    if (g == d->mousePos)
        return;

    if ( d->managed->isMaximized() )
        return;

    int x = d->managed->geometry().x();
    int y = d->managed->geometry().y();
    int w = d->managed->width();
    int h = d->managed->height();

    QRect geom(d->managed->geometry());

    QPoint delta = g - d->mousePos;
    d->mousePos = g;

    if (d->activeRegion == QDecoration::Title) {
        geom = QRect(x + delta.x(), y + delta.y(), w, h);
    } else {
        bool keepTop = true;
        bool keepLeft = true;
        switch (d->activeRegion) {
        case QDecoration::Top:
            geom.setTop(geom.top() + delta.y());
            keepTop = false;
            break;
        case QDecoration::Bottom:
            geom.setBottom(geom.bottom() + delta.y());
            keepTop = true;
            break;
        case QDecoration::Left:
            geom.setLeft(geom.left() + delta.x());
            keepLeft = false;
            break;
        case QDecoration::Right:
            geom.setRight(geom.right() + delta.x());
            keepLeft = true;
            break;
        case QDecoration::TopRight:
            geom.setTopRight(geom.topRight() + delta);
            keepLeft = true;
            keepTop = false;
            break;
        case QDecoration::TopLeft:
            geom.setTopLeft(geom.topLeft() + delta);
            keepLeft = false;
            keepTop = false;
            break;
        case QDecoration::BottomLeft:
            geom.setBottomLeft(geom.bottomLeft() + delta);
            keepLeft = false;
            keepTop = true;
            break;
        case QDecoration::BottomRight:
            geom.setBottomRight(geom.bottomRight() + delta);
            keepLeft = true;
            keepTop = true;
            break;
        default:
            return;
        }

        QSize newSize = QLayout::closestAcceptableSize(d->managed, geom.size());

        int dx = newSize.width() - geom.width();
        int dy = newSize.height() - geom.height();

        if (keepTop) {
            geom.setBottom(geom.bottom() + dy);
            d->mousePos.ry() += dy;
        } else {
            geom.setTop(geom.top() - dy);
            d->mousePos.ry() -= dy;
        }
        if (keepLeft) {
            geom.setRight(geom.right() + dx);
            d->mousePos.rx() += dx;
        } else {
            geom.setLeft(geom.left() - dx);
            d->mousePos.rx() -= dx;
        }
    }
    if (geom != d->managed->geometry()) {
        QApplication::sendPostedEvents();
        d->managed->setGeometry(geom);
    }
}

void QWSManager::paintEvent(QPaintEvent *)
{
    repaintRegion(QDecoration::All, QDecoration::Normal);
}

bool QWSManagerPrivate::doPaint(int decorationRegion, QDecoration::DecorationState state)
{
    bool result = false;
    if (!managed->isVisible())
        return result;
    QDecoration &dec = QApplication::qwsDecoration();
    if (managed->testAttribute(Qt::WA_WState_InPaintEvent))
        qWarning("QWSManager::paintEvent() recursive paint event detected");
//    d->managed->setAttribute(Qt::WA_WState_InPaintEvent);

    QTLWExtra *topextra = managed->d_func()->extra->topextra;
    QPixmap *buf = topextra->backingStore->pixmap();
    if (buf->isNull()) {
//        qDebug("QWSManager::doPaint empty buf");
        managed->setAttribute(Qt::WA_WState_InPaintEvent, false);
        return false;
    }
    QPainter painter;
    painter.begin(buf);
    QRegion clipRgn = dec.region(managed, managed->rect().translated(-topextra->backingStoreOffset ));
#if 0
    qDebug() << "QWSManagerPrivate::doPaint"
             << "rect" << managed->rect()
             << "backingStoreOffset" << topextra->backingStoreOffset
             << "clipRgn" << clipRgn;
#endif
    painter.setClipRegion(clipRgn);
    painter.translate(-topextra->backingStoreOffset);

    result = dec.paint(&painter, managed, decorationRegion, state);
    painter.end();

    managed->setAttribute(Qt::WA_WState_InPaintEvent, false);
    return result;
}

bool QWSManager::repaintRegion(int decorationRegion, QDecoration::DecorationState state)
{
    Q_D(QWSManager);
    //### lock backing store
    bool result = d->doPaint(decorationRegion, state);
    //### unlock
    QTLWExtra *topextra = d->managed->d_func()->extra->topextra;
    QDecoration &dec = QApplication::qwsDecoration();
    //### copies too much, but we don't know here what has actually been changed
    if (result) {
        d->managed->d_func()->bltToScreen(dec.region(d->managed, d->managed->geometry()));
    }
    return result;
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_POPUPMENU
    Q_D(QWSManager);
    if (d->popup)
        delete d->popup;

    // Basic window operation menu
    d->popup = new QMenu();
    QApplication::qwsDecoration().buildSysMenu(d->managed, d->popup);
    connect(d->popup, SIGNAL(triggered(QAction*)), SLOT(menuTriggered(QAction*)));

    d->popup->popup(pos);
#endif
}

void QWSManager::menuTriggered(QAction *action)
{
    Q_D(QWSManager);
    QApplication::qwsDecoration().menuTriggered(d->managed, action);
    d->popup->deleteLater();
    d->popup = 0;
}

void QWSManager::startMove()
{
    Q_D(QWSManager);
    d->mousePos = QCursor::pos();
    d->activeRegion = QDecoration::Title;
    d->active = d->managed;
    d->managed->grabMouse();
}

void QWSManager::startResize()
{
    Q_D(QWSManager);
    d->activeRegion = QDecoration::BottomRight;
    d->active = d->managed;
    d->managed->grabMouse();
}

void QWSManager::maximize()
{
    Q_D(QWSManager);
    // find out how much space the decoration needs
    extern QRect qt_maxWindowRect;
    QRect desk = qt_maxWindowRect;
    QRect dummy(0, 0, 1, 1);
    QRect nr;
    QRegion r = QApplication::qwsDecoration().region(d->managed, dummy);
    if (r.isEmpty()) {
        nr = desk;
    } else {
        QRect rect = r.boundingRect();
        nr = QRect(desk.x()-rect.x(), desk.y()-rect.y(),
                desk.width() - (rect.width()==1 ? 0 : rect.width()), // ==1 -> dummy
                desk.height() - (rect.height()==1 ? 0 : rect.height()));
    }
    d->managed->setGeometry(nr);
}

bool QWSManagerPrivate::newCachedRegion(const QPoint &pos)
{
    // Check if anything has changed that would affect the region caching
    if (managed->windowFlags() == cached_region.windowFlags
        && managed->geometry() == cached_region.windowGeometry
        && cached_region.region.contains(pos))
        return false;

    // Update the cached region
    int reg = QApplication::qwsDecoration().regionAt(managed, pos);
    if (QWidget::mouseGrabber())
        reg = QDecoration::None;

    previousRegionType = cached_region.regionType;
    cached_region.regionType = reg;
    cached_region.region = QApplication::qwsDecoration().region(managed, managed->geometry(),
                                                                reg);
    cached_region.windowFlags = managed->windowFlags();
    cached_region.windowGeometry = managed->geometry();
//    QRect rec = cached_region.region.boundingRect();
//    qDebug("Updated cached region: 0x%04x (%d, %d)  (%d, %d,  %d, %d)",
//           reg, pos.x(), pos.y(), rec.x(), rec.y(), rec.right(), rec.bottom());
    return true;
}

#endif //QT_NO_QWS_MANAGER
