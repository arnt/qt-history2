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

#include "qpaintengine_qws.h"
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
    static QPoint mousePressPos;
    static QPoint lastMousePos;
};
QWidget *QWSManagerPrivate::active = 0;
QPoint QWSManagerPrivate::mousePressPos;
QPoint QWSManagerPrivate::lastMousePos;


QWSManagerPrivate::QWSManagerPrivate()
    : QObjectPrivate(), activeRegion(QDecoration::None), managed(0), popup(0)
{
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
    d->mousePressPos = e->globalPos();
    d->lastMousePos = d->mousePressPos;
    d->activeRegion = QApplication::qwsDecoration().regionAt(d->managed, d->mousePressPos);
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
    if (e->button() == Qt::LeftButton) {
        //handleMove();
        int itm = QApplication::qwsDecoration().regionAt(d->managed, e->globalPos());
        int activatedItem = d->activeRegion;
        d->activeRegion = QDecoration::None;
        d->active = 0;
        switch (activatedItem) {
            case QDecoration::Close:
                if (itm == QDecoration::Close) {
                    close();
                    return;
                }
                break;
            case QDecoration::Minimize:
                if (itm == QDecoration::Minimize)
                    minimize();
                break;
            case QDecoration::Maximize:
                if (itm == QDecoration::Maximize)
                    toggleMaximize();
                break;
            default:
                break;
        }
    } else if (d->activeRegion == QDecoration::None) {
        d->active = 0;
    }
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
#ifndef QT_NO_CURSOR
    // cursor
    QWSDisplay *qwsd = QApplication::desktop()->qwsDisplay();

    int reg = d->activeRegion;
    if (d->activeRegion == QDecoration::None && !QWidget::mouseGrabber())
        reg = QApplication::qwsDecoration().regionAt(d->managed, e->globalPos());

    qwsd->selectCursor(d->managed, regionToShape(reg));
#endif //QT_NO_CURSOR
    // resize/move regions

    // don't allow dragging to where the user probably cannot click!
    QPoint g = e->globalPos();
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

    handleMove(g);
}

void QWSManager::handleMove(const QPoint &g)
{
    if (g == d->lastMousePos)
        return;
    d->lastMousePos = g;

    if ( d->managed->isMaximized() )
        return;

    int x = d->managed->geometry().x();
    int y = d->managed->geometry().y();
    int w = d->managed->width();
    int h = d->managed->height();

    QRect geom(d->managed->geometry());

    if (d->activeRegion == QDecoration::Title) {
        QPoint delta = g - d->mousePressPos;
        geom = QRect(x + delta.x(), y + delta.y(), w, h);
        d->mousePressPos = g;
    } else {
        bool keepTop = true;
        bool keepLeft = true;
        switch (d->activeRegion) {
        case QDecoration::Top:
            geom.setTop(g.y());
            keepTop = false;
            break;
        case QDecoration::Bottom:
            geom.setBottom(g.y());
            keepTop = true;
            break;
        case QDecoration::Left:
            geom.setLeft(g.x());
            keepLeft = false;
            break;
        case QDecoration::Right:
            geom.setRight(g.x());
            keepLeft = true;
            break;
        case QDecoration::TopRight:
            geom.setTopRight(g);
            keepLeft = true;
            keepTop = false;
            break;
        case QDecoration::TopLeft:
            geom.setTopLeft(g);
            keepLeft = false;
            keepTop = false;
            break;
        case QDecoration::BottomLeft:
            geom.setBottomLeft(g);
            keepLeft = false;
            keepTop = true;
            break;
        case QDecoration::BottomRight:
            geom.setBottomRight(g);
            keepLeft = true;
            keepTop = true;
            break;
        default:
            return;
        }

        QSize newSize = QLayout::closestAcceptableSize(d->managed, geom.size());

        if (keepTop)
            geom.setHeight(newSize.height());
        else
            geom.setTop(geom.top() - (newSize.height() - geom.height()));

        if (keepLeft)
            geom.setWidth(newSize.width());
        else
            geom.setLeft(geom.left() - (newSize.width() - geom.width()));
    }




    if (geom != d->managed->geometry()) {
        QApplication::sendPostedEvents();
        d->managed->setGeometry(geom);
    }
}

void QWSManager::paintEvent(QPaintEvent *)
{
    if (!d->managed->isVisible())
        return;
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
    dec.paint(&painter, d->managed);
    d->managed->clearWState(Qt::WState_InPaintEvent);
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_POPUPMENU
    if (!d->popup) {
        // Basic window operation menu
        d->popup = new QMenu();
        d->menuActions[QWSManagerPrivate::NormalizeAction] = new QAction(qApp->translate("QDecoration",  "&Restore"));
        d->menuActions[QWSManagerPrivate::TitleAction] = new QAction(qApp->translate("QDecoration",  "&Move"));
        d->menuActions[QWSManagerPrivate::BottomRightAction] = new QAction(qApp->translate("QDecoration",  "&Size"));
        d->menuActions[QWSManagerPrivate::MinimizeAction] = new QAction(qApp->translate("QDecoration",  "Mi&nimize"));
        d->menuActions[QWSManagerPrivate::MaximizeAction] = new QAction(qApp->translate("QDecoration",  "Ma&ximize"));
        d->menuActions[QWSManagerPrivate::CloseAction] = new QAction(qApp->translate("QDecoration",  "Close"));

        d->popup->addAction(d->menuActions[QWSManagerPrivate::NormalizeAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::TitleAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::BottomRightAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::MinimizeAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::MaximizeAction]);
        d->popup->addSeparator();
        d->popup->addAction(d->menuActions[QWSManagerPrivate::CloseAction]);
        connect(d->popup, SIGNAL(triggered(QAction*)), SLOT(menuTriggered(QAction*)));

        // Add Style menu
        QMenu *styleMenu = new QMenu("Style");
/*
        for (int i = 0; !WMStyleList[i].WMStyleName.isEmpty(); i++)
            styleMenu->addAction(qApp->translate("QDecoration", WMStyleList[i].WMStyleName.latin1()));
        connect(styleMenu, SIGNAL(triggered(QAction*)), this, SLOT(styleMenuTriggered(QAction*)));
        d->popup->addSeparator();
        d->popup->addAction(styleMenu->menuAction());
*/
    }

    d->menuActions[QWSManagerPrivate::MaximizeAction]->setEnabled(!d->managed->isMaximized());
    d->menuActions[QWSManagerPrivate::NormalizeAction]->setEnabled(d->managed->isMaximized());
    d->popup->popup(pos);
#endif
}

#include <qcdestyle.h>
#include <qcommonstyle.h>
#include <qcompactstyle.h>
#include <qmotifplusstyle.h>
#include <qmotifstyle.h>
#include <qplatinumstyle.h>
#include <qsgistyle.h>
#include <qwindowsstyle.h>

void QWSManager::styleMenuTriggered(QAction *item)
{
/*
    for (int i = 0; !WMStyleList[i].WMStyleName.isEmpty(); i++) {
        if (item->text() == qApp->translate("QDecoration", WMStyleList[i].WMStyleName.latin1()))
            qApp->qwsSetDecoration(WMStyleList[i].new_WMDecorations());
    }
*/
    // Force a repaint of the WM regions
    const QSize s = d->managed->size();
    d->managed->resize(s.width() + 1, s.height());
    d->managed->resize(s.width(), s.height());
}

void QWSManager::menuTriggered(QAction *item)
{
    if (item == d->menuActions[QWSManagerPrivate::CloseAction]) {
        close();
    } else if (item == d->menuActions[QWSManagerPrivate::MinimizeAction]) {
        minimize();
    } else if (item == d->menuActions[QWSManagerPrivate::MaximizeAction]
               || item == d->menuActions[QWSManagerPrivate::NormalizeAction]) {
        toggleMaximize();
    } else if (item == d->menuActions[QWSManagerPrivate::TitleAction]) {
        d->mousePressPos = QCursor::pos();
        d->activeRegion = QDecoration::Title;
        d->active = d->managed;
        d->managed->grabMouse();
    } else if (item == d->menuActions[QWSManagerPrivate::BottomRightAction]) {
        d->activeRegion = QDecoration::BottomRight;
        d->active = d->managed;
        d->managed->grabMouse();
    } else {
        qWarning("QWSManager: Unknown menu option");
    }
}

void QWSManager::close()
{
    d->active = 0;
    QApplication::qwsDecoration().close(d->managed);
}

void QWSManager::minimize()
{
    QApplication::qwsDecoration().minimize(d->managed);
}

void QWSManager::maximize()
{
    QApplication::qwsDecoration().maximize(d->managed);
}

void QWSManager::toggleMaximize()
{
    if (!d->managed->isMaximized()) {
        d->managed->showMaximized();
    } else {
        d->managed->showNormal();
    }
}

#endif //QT_NO_QWS_MANAGER
