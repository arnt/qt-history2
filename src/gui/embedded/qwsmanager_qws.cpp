/****************************************************************************
**
** Implementation of Qt/Embedded window manager.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
#include "qaccel.h"
#include "qstyle.h"
#include "qwidget.h"
#include "qpopupmenu.h"
#include "qpainter.h"
#include "qregion.h"
#include "qevent.h"
#include "qcursor.h"
#include "qgfx_qws.h"
#include "qwsdisplay_qws.h"
#include "qwsregionmanager_qws.h"
#include "qwsdefaultdecoration_qws.h"
#include "qevent.h"
#include "qdesktopwidget.h"

#include <private/qwidget_p.h>

#include "qpaintengine_qws.h"

enum WMStyle {
    Default_WMStyle = 1, /* Starting at zero stuffs up menus */
    KDE_WMStyle,
    KDE2_WMStyle,
    BeOS_WMStyle,
    Windows_WMStyle,
    Hydro_WMStyle,
};


#ifndef QT_NO_QWS_WINDOWS_WM_STYLE
#include "qwswindowsdecoration_qws.h"
QWSDefaultDecoration *new_Windows_WMDecorations() { return new QWSWindowsDecoration(); }
#endif // QT_NO_QWS_WINDOWS_WM_STYLE
#ifndef QT_NO_QWS_KDE_WM_STYLE
#include "qwskdedecoration_qws.h"
QWSDefaultDecoration *new_KDE_WMDecorations() { return new QWSKDEDecoration(); }
#endif // QT_NO_QWS_KDE_WM_STYLE
#ifndef QT_NO_QWS_KDE2_WM_STYLE
#include "qwskde2decoration_qws.h"
QWSDefaultDecoration *new_KDE2_WMDecorations() { return new QWSKDE2Decoration(); }
#endif // QT_NO_QWS_KDE2_WM_STYLE
#ifndef QT_NO_QWS_BEOS_WM_STYLE
#include "qwsbeosdecoration_qws.h"
QWSDefaultDecoration *new_BeOS_WMDecorations() { return new QWSBeOSDecoration(); }
#endif // QT_NO_QWS_BEOS_WM_STYLE
#ifndef QT_NO_QWS_HYDRO_WM_STYLE
#include "qwshydrodecoration_qws.h"
QWSDefaultDecoration *new_Hydro_WMDecorations() { return new QWSHydroDecoration(); }
#endif // QT_NO_QWS_HYDRO_WM_STYLE

#include "qwsdefaultdecoration_qws.h"
QWSDefaultDecoration *new_Default_WMDecorations() { return new QWSDefaultDecoration(); }
#define d d_func()
#define q q_func()


struct WMStyleFactoryItem {
        WMStyle WMStyleType;
        QString WMStyleName;
        QWSDefaultDecoration *(*new_WMDecorations)();
} WMStyleList[] = {
#ifndef QT_NO_QWS_WINDOWS_WM_STYLE
    { Windows_WMStyle, QT_TRANSLATE_NOOP("QWSDecoration", "Windows"), new_Windows_WMDecorations },
#endif // QT_NO_QWS_WINDOWS_WM_STYLE
#ifndef QT_NO_QWS_KDE_WM_STYLE
    { KDE_WMStyle, QT_TRANSLATE_NOOP("QWSDecoration", "KDE"), new_KDE_WMDecorations },
#endif // QT_NO_QWS_KDE_WM_STYLE
#ifndef QT_NO_QWS_KDE2_WM_STYLE
    { KDE2_WMStyle, QT_TRANSLATE_NOOP("QWSDecoration", "KDE2"), new_KDE2_WMDecorations },
#endif // QT_NO_QWS_KDE2_WM_STYLE
#ifndef QT_NO_QWS_BEOS_WM_STYLE
    { BeOS_WMStyle, QT_TRANSLATE_NOOP("QWSDecoration", "BeOS"), new_BeOS_WMDecorations },
#endif // QT_NO_QWS_BEOS_WM_STYLE
#ifndef QT_NO_QWS_HYDRO_WM_STYLE
    { Hydro_WMStyle, QT_TRANSLATE_NOOP("QWSDecoration", "Hydro"), new_Hydro_WMDecorations },
#endif // QT_NO_QWS_HYDRO_WM_STYLE

    { Default_WMStyle, QT_TRANSLATE_NOOP("QWSDecoration", "Default"), new_Default_WMDecorations },
    { Default_WMStyle, QString::null, NULL }
};


QWSDecoration *QWSManager::newDefaultDecoration()
{
    return new QWSDefaultDecoration;
}


QWidget *QWSManager::active = 0;
QPoint QWSManager::mousePos;

QWSManager::QWSManager(QWidget *w)
    : activeRegion(QWSDecoration::None), managed(w), popup(0)
{
    dx = 0;
    dy = 0;

    menuBtn = new QWSButton(this, QWSDecoration::Menu);
    closeBtn = new QWSButton(this, QWSDecoration::Close);
    minimizeBtn = new QWSButton(this, QWSDecoration::Minimize);
    maximizeBtn = new QWSButton(this, QWSDecoration::Maximize, true);
}

QWSManager::~QWSManager()
{
#ifndef QT_NO_POPUPMENU
    if (popup)
        delete popup;
#endif
    delete menuBtn;
    delete closeBtn;
    delete minimizeBtn;
    delete maximizeBtn;

}

QRegion QWSManager::region()
{
    return QApplication::qwsDecoration().region(managed, managed->geometry());
}

QWSDecoration::Region QWSManager::pointInRegion(const QPoint &p)
{
    QWSDecoration &dec = QApplication::qwsDecoration();
    QRect rect(managed->geometry());

    for (int i = QWSDecoration::LastRegion; i >= QWSDecoration::Title; i--) {
        if (dec.region(managed, rect, (QWSDecoration::Region)i).contains(p))
            return (QWSDecoration::Region)i;
    }

    return QWSDecoration::None;
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

    mousePos = e->globalPos();
    dx = 0;
    dy = 0;
    activeRegion = pointInRegion(mousePos);
    switch (activeRegion) {
    case QWSDecoration::Menu:
        menu(managed->geometry().topLeft());
        break;
    case QWSDecoration::Close:
        setClicked(closeBtn, true);
        break;
    case QWSDecoration::Minimize:
        setClicked(minimizeBtn, true);
        break;
    case QWSDecoration::Maximize:
        setClicked(maximizeBtn, true);
        break;
    default:
        break;
    }
    if (activeRegion != QWSDecoration::None &&
         activeRegion != QWSDecoration::Menu) {
        active = managed;
        managed->grabMouse();
    }
    if (activeRegion != QWSDecoration::None &&
         activeRegion != QWSDecoration::Close &&
         activeRegion != QWSDecoration::Minimize &&
         activeRegion != QWSDecoration::Menu) {
        managed->raise();
    }
    if (e->button() == Qt::RightButton) {
        menu(e->globalPos());
    }
}

void QWSManager::mouseReleaseEvent(QMouseEvent *e)
{
    managed->releaseMouse();
    if (e->button() == Qt::LeftButton) {
        handleMove();
        mousePos = e->globalPos();
        QWSDecoration::Region rgn = pointInRegion(e->globalPos());
        QWSDecoration::Region activatedRegion = activeRegion;
        activeRegion = QWSDecoration::None;
        active = 0;
        switch (activatedRegion) {
            case QWSDecoration::Close:
                setClicked(closeBtn, false);
                if (rgn == QWSDecoration::Close) {
                    close();
                    return;
                }
                break;
            case QWSDecoration::Minimize:
                setClicked(minimizeBtn, false);
                if (rgn == QWSDecoration::Minimize)
                    minimize();
                break;
            case QWSDecoration::Maximize:
                setClicked(maximizeBtn, false);
                if (rgn == QWSDecoration::Maximize)
                    toggleMaximize();
                break;
            default:
                break;
        }
    } else if (activeRegion == QWSDecoration::None) {
        active = 0;
    }
}

void QWSManager::mouseMoveEvent(QMouseEvent *e)
{
#ifndef QT_NO_CURSOR
    static Qt::CursorShape shape[] = { Qt::ArrowCursor, ArrowCursor, ArrowCursor,
                            Qt::SizeVerCursor, SizeVerCursor, Qt::SizeHorCursor,
                            Qt::SizeHorCursor, Qt::SizeFDiagCursor, Qt::SizeBDiagCursor,
                            Qt::SizeBDiagCursor, Qt::SizeFDiagCursor, Qt::ArrowCursor,
                            Qt::ArrowCursor, ArrowCursor, ArrowCursor, ArrowCursor};

    // cursor
    QWSDisplay *qwsd = QApplication::desktop()->qwsDisplay();
    if (activeRegion == QWSDecoration::None)
    {
        if (!QWidget::mouseGrabber()) {
            QWSDecoration::Region r = pointInRegion(e->globalPos());
            qwsd->selectCursor(managed, shape[r]);
        }
    } else
        qwsd->selectCursor(managed, shape[activeRegion]);
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

    dx = g.x() - mousePos.x();
    dy = g.y() - mousePos.y();

    handleMove();

    // button regions
    QWSDecoration::Region r = pointInRegion(e->globalPos());
    setMouseOver(menuBtn, r == QWSDecoration::Menu);
    setMouseOver(closeBtn, r == QWSDecoration::Close);
    setMouseOver(minimizeBtn, r == QWSDecoration::Minimize);
    setMouseOver(maximizeBtn, r == QWSDecoration::Maximize);
}

void QWSManager::handleMove()
{
    if (!dx && !dy)
        return;

    int x = managed->geometry().x();
    int y = managed->geometry().y();
    int w = managed->width();
    int h = managed->height();

    QRect geom(managed->geometry());

    switch (activeRegion) {
        case QWSDecoration::Title:
            geom = QRect(x + dx, y + dy, w, h);
            break;
        case QWSDecoration::Top:
            geom = QRect(x, y + dy, w, h - dy);
            break;
        case QWSDecoration::Bottom:
            geom = QRect(x, y, w, h + dy);
            break;
        case QWSDecoration::Left:
            geom = QRect(x + dx, y, w - dx, h);
            break;
        case QWSDecoration::Right:
            geom = QRect(x, y, w + dx, h);
            break;
        case QWSDecoration::TopRight:
            geom = QRect(x, y + dy, w + dx, h - dy);
            break;
        case QWSDecoration::TopLeft:
            geom = QRect(x + dx, y + dy, w - dx, h - dy);
            break;
        case QWSDecoration::BottomLeft:
            geom = QRect(x + dx, y, w - dx, h + dy);
            break;
        case QWSDecoration::BottomRight:
            geom = QRect(x, y, w + dx, h + dy);
            break;
        default:
            return;
    }

    if (geom.width() >= managed->minimumWidth()
            && geom.width() <= managed->maximumWidth()) {
        mousePos.setX(mousePos.x() + dx);
    } else if (geom.width() < managed->minimumWidth()) {
        if (x != geom.x()) {
            geom.setX(x+(w-managed->minimumWidth()));
            mousePos.setX(geom.x());
        } else {
            mousePos.setX(x+managed->minimumWidth());
        }
        geom.setWidth(managed->minimumWidth());
    } else {
        geom.setX(x);
        geom.setWidth(w);
    }
    if (geom.height() >= managed->minimumHeight()
            && geom.height() <= managed->maximumHeight()) {
        mousePos.setY(mousePos.y() + dy);
    } else if (geom.height() < managed->minimumHeight()) {
        if (y != geom.y()) {
            geom.setY(y+(h-managed->minimumHeight()));
            mousePos.setY(mousePos.y()+(geom.y()-y));
        } else {
            mousePos.setY(y+managed->minimumHeight());
        }
        geom.setHeight(managed->minimumHeight());
    } else {
        geom.setY(y);
        geom.setHeight(h);
    }

    if (geom != managed->geometry()) {
        QApplication::sendPostedEvents();
        managed->setGeometry(geom);
    }

    dx = 0;
    dy = 0;
}

void QWSManager::paintEvent(QPaintEvent *)
{
    if (!managed->isVisible())
        return;
    QWSDecoration &dec = QApplication::qwsDecoration();
    if (managed->testWState(Qt::WState_InPaintEvent))
        qWarning("QWSManager::paintEvent() recursive paint event detected");
    managed->setWState(Qt::WState_InPaintEvent);
    QPainter painter(managed);

    // Adjust our widget region to contain the window
    // manager decoration instead of the widget itself.
    QRegion r = managed->d->topData()->decor_allocated_region;
    int rgnIdx = managed->data->alloc_region_index;

    QGfx *gfx = static_cast<QWSPaintEngine *>(painter.device()->engine())->gfx();
    if (rgnIdx >= 0) {
        QRegion newRegion;
        bool changed = false;
        QWSDisplay::grab();
        const int *rgnRev = qt_fbdpy->regionManager()->revision(rgnIdx);
        if (managed->data->alloc_region_revision != *rgnRev) {
             newRegion = qt_fbdpy->regionManager()->region(rgnIdx);
             changed = true;
        }
        gfx->setGlobalRegionIndex(rgnIdx);
        QWSDisplay::ungrab();
        if (changed) {
            r &= newRegion;
        }
    }
    gfx->setWidgetDeviceRegion(r);

    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paint(&painter, managed);
    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paintButton(&painter, managed, QWSDecoration::Menu, menuBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Close, closeBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Minimize, minimizeBtn->state());
    dec.paintButton(&painter, managed, QWSDecoration::Maximize, maximizeBtn->state());
    managed->clearWState(Qt::WState_InPaintEvent);
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_POPUPMENU
    if (!popup) {
        popup = QApplication::qwsDecoration().menu(managed, managed->pos());

        // Add Style menu
        QPopupMenu *styleMenu = new QPopupMenu();
        for (int i = 0; !WMStyleList[i].WMStyleName.isEmpty(); i++)
            styleMenu->insertItem(qApp->translate("QWSDecoration", WMStyleList[i].WMStyleName.latin1()), WMStyleList[i].WMStyleType);
        styleMenu->connect(styleMenu, SIGNAL(activated(int)), this, SLOT(styleMenuActivated(int)));
//        popup->insertSeparator();
//        popup->insertItem(tr("Style"), styleMenu);

        connect(popup, SIGNAL(activated(int)), SLOT(menuActivated(int)));
    }
    popup->setItemEnabled(QWSDecoration::Maximize, !managed->isMaximized());
    popup->setItemEnabled(QWSDecoration::Normalize, managed->isMaximized());
    popup->popup(pos);
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

void QWSManager::styleMenuActivated(int id)
{
    for (int i = 0; !WMStyleList[i].WMStyleName.isEmpty(); i++) {
        if (id == WMStyleList[i].WMStyleType) {
            qApp->qwsSetDecoration(WMStyleList[i].new_WMDecorations());
        }
    }

    // Force a repaint of the WM regions
    const QSize s = managed->size();
    managed->resize(s.width() + 1, s.height());
    managed->resize(s.width(), s.height());
}

void QWSManager::menuActivated(int id)
{
    switch (id) {
        case QWSDecoration::Close:
            close();
            return;
        case QWSDecoration::Minimize:
            minimize();
            break;
        case QWSDecoration::Maximize:
        case QWSDecoration::Normalize:
            toggleMaximize();
            break;
        case QWSDecoration::Title:
            mousePos = QCursor::pos();
            activeRegion = QWSDecoration::Title;
            active = managed;
            managed->grabMouse();
            break;
        case QWSDecoration::BottomRight:
            mousePos = QCursor::pos();
            activeRegion = QWSDecoration::BottomRight;
            active = managed;
            managed->grabMouse();
            break;
        default:
            break;
    }
}

void QWSManager::close()
{
    active = 0;
    QApplication::qwsDecoration().close(managed);
}

void QWSManager::minimize()
{
    QApplication::qwsDecoration().minimize(managed);
}


void QWSManager::maximize()
{
    QApplication::qwsDecoration().maximize(managed);
    setOn(maximizeBtn,true);
}

void QWSManager::toggleMaximize()
{
    if (!managed->isMaximized()) {
        managed->showMaximized();
        setOn(maximizeBtn,true);
    } else {
        managed->showNormal();
        setOn(maximizeBtn,false);
    }
}



void QWSManager::setMouseOver(QWSButton *b, bool m)
{
    if (b->setMouseOver(m))
        repaintButton(b);
}
void QWSManager::setClicked(QWSButton *b, bool c)
{
    if (b->setClicked(c))
        repaintButton(b);
}
void QWSManager::setOn(QWSButton *b, bool o)
{
    if (b->setOn(o))
        repaintButton(b);
}

void QWSManager::repaintButton(QWSButton *b)
{
    if (!managed->isVisible())
        return;
    QWSDecoration &dec = QApplication::qwsDecoration();
    if (managed->testWState(Qt::WState_InPaintEvent))
        qWarning("QWSManager::repaintButton() recursive paint event detected");
    managed->setWState(Qt::WState_InPaintEvent);
#warning "This isn't really inside a paint event. Please fix"
    QPainter painter(managed);
    painter.setClipRegion(dec.region(managed, managed->rect()));
    dec.paintButton(&painter, managed, b->type(), b->state());
    managed->clearWState(Qt::WState_InPaintEvent);
}



/*
*/
QWSButton::QWSButton(QWSManager *m, QWSDecoration::Region t, bool tb)
    : flags(0), toggle(tb), typ(t), manager(m)
{
}

bool QWSButton::setMouseOver(bool m)
{
    int s = state();
    if (m) flags |= MouseOver;
    else flags &= ~MouseOver;
    return (state() != s);
}

bool QWSButton::setClicked(bool c)
{
    int s = state();
    if (c) flags |= Clicked;
    else flags &= ~Clicked;
    return (state() != s);
}

bool QWSButton::setOn(bool o)
{
    int s = state();
    if (o) flags |= On;
    else flags &= ~On;
    return (state() != s);
}

#endif //QT_NO_QWS_MANAGER
