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
#include "qcursor.h"

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
#include "qwsregionmanager_qws.h"
#include "qevent.h"
#include "qdesktopwidget.h"

#include <private/qwidget_p.h>

#include <private/qpaintengine_qws_p.h>
#include "qdecorationfactory_qws.h"

#include "qlayout.h"

#define d d_func()
#define q q_func()


class QWSManagerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWSManager)
public:
    QWSManagerPrivate();

    int activeRegion;
    QWidget *managed;
    QMenu *popup;

    enum MenuAction {
        NormalizeAction,
        TitleAction,
        BottomRightAction,
        MinimizeAction,
        MaximizeAction,
        CloseAction,
        LastMenuAction
    };
    QAction *menuActions[LastMenuAction];

    static QWidget *active;
    static QPoint mousePos;

    // Region caching to avoid getting a regiontype's
    // QRegion for each mouse move event
    int previousRegionType;
    bool previousRegionRepainted; // Hover/Press handled
    struct RegionCaching {
        int regionType;
        QRegion region;
        Qt::WFlags windowFlags;
        QRect windowGeometry;
    } cached_region;

    bool newCachedRegion(const QPoint &pos);
    int cachedRegionAt()
    { return cached_region.regionType; }
};

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
    return d->cached_region.region;
}


#define d d_func()
#define q q_func()


QWSManager::QWSManager(QWidget *w)
    : QObject(*new QWSManagerPrivate, (QObject*)0)
{
    d->managed = w;

}

QWSManager::~QWSManager()
{
#ifndef QT_NO_POPUPMENU
    if (d->popup)
        delete d->popup;
#endif
    if (d->managed == QWSManagerPrivate::active)
        QWSManagerPrivate::active = 0;
}

QWidget *QWSManager::widget()
{
    return d->managed;
}

QWidget *QWSManager::grabbedMouse()
{
    return QWSManagerPrivate::active;
}

QRegion QWSManager::region()
{
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

bool QWSManager::repaintRegion(int decorationRegion, QDecoration::DecorationState state)
{
    bool result = false;
    if (!d->managed->isVisible())
        return result;
    QDecoration &dec = QApplication::qwsDecoration();
    if (d->managed->testWState(Qt::WState_InPaintEvent))
        qWarning("QWSManager::paintEvent() recursive paint event detected");
    d->managed->setWState(Qt::WState_InPaintEvent);
    QPainter painter(d->managed);

    // Adjust our widget region to contain the window
    // manager decoration instead of the widget itself.
    QRegion r = d->managed->d->topData()->decor_allocated_region;
    int rgnIdx = d->managed->data->alloc_region_index;

    QWSPaintEngine *pe = static_cast<QWSPaintEngine *>(painter.d->engine);
    if (rgnIdx >= 0) {
        QRegion newRegion;
        bool changed = false;
        QWSDisplay::grab();
        const int *rgnRev = qt_fbdpy->regionManager()->revision(rgnIdx);
        if (d->managed->data->alloc_region_revision != *rgnRev) {
             newRegion = qt_fbdpy->regionManager()->region(rgnIdx);
             changed = true;
        }
        pe->setGlobalRegionIndex(rgnIdx);
        QWSDisplay::ungrab();
        if (changed) {
            r &= newRegion;
        }
    }
    pe->setWidgetDeviceRegion(r);

    painter.setClipRegion(dec.region(d->managed, d->managed->rect()));
    result = dec.paint(&painter, d->managed, decorationRegion, state);

    d->managed->clearWState(Qt::WState_InPaintEvent);
    return result;
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_POPUPMENU
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
    QApplication::qwsDecoration().menuTriggered(d->managed, action);
    d->popup->deleteLater();
    d->popup = 0;
}

void QWSManager::startMove()
{
    d->mousePos = QCursor::pos();
    d->activeRegion = QDecoration::Title;
    d->active = d->managed;
    d->managed->grabMouse();
}

void QWSManager::startResize()
{
    d->activeRegion = QDecoration::BottomRight;
    d->active = d->managed;
    d->managed->grabMouse();
}

void QWSManager::maximize()
{
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
    if (managed->getWFlags() == cached_region.windowFlags
        && managed->geometry() == cached_region.windowGeometry
        && cached_region.region.contains(pos))
        return false;

    // Update the cached region
    int reg = QApplication::qwsDecoration().regionAt(d->managed, pos);
    if (QWidget::mouseGrabber())
        reg = QDecoration::None;

    previousRegionType = cached_region.regionType;
    cached_region.regionType = reg;
    cached_region.region = QApplication::qwsDecoration().region(d->managed, d->managed->geometry(),
                                                                reg);
    cached_region.windowFlags = managed->getWFlags();
    cached_region.windowGeometry = managed->geometry();
//    QRect rec = d->cached_region.region.boundingRect();
//    qDebug("Updated cached region: 0x%04x (%d, %d)  (%d, %d,  %d, %d)",
//           reg, pos.x(), pos.y(), rec.x(), rec.y(), rec.right(), rec.bottom());
    return true;
}

#endif //QT_NO_QWS_MANAGER
