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
#include "qstyle.h"
#include "qwidget.h"
#include "qmenu.h"
#include "qpainter.h"
#include "private/qpainter_p.h"
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


class QWSManagerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWSManager)
public:
    QWSManagerPrivate();

    QWSDecoration::Region activeRegion;
    QWidget *managed;
    QMenu *popup;
    QWSButton *menuBtn;
    QWSButton *closeBtn;
    QWSButton *minimizeBtn;
    QWSButton *maximizeBtn;

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

    int dx;
    int dy;

    static QWidget *active;
    static QPoint mousePos;
};
QWidget *QWSManagerPrivate::active = 0;
QPoint QWSManagerPrivate::mousePos;


QWSManagerPrivate::QWSManagerPrivate()
    : QObjectPrivate(), activeRegion(QWSDecoration::None), managed(0), popup(0), menuBtn(0),
      closeBtn(0), minimizeBtn(0), maximizeBtn(0), dx(0), dy(0)
{
}

#define d d_func()
#define q q_func()


QWSManager::QWSManager(QWidget *w)
    : QObject(*new QWSManagerPrivate, (QObject*)0)
{
    d->managed = w;
    d->menuBtn = new QWSButton(this, QWSDecoration::Menu);
    d->closeBtn = new QWSButton(this, QWSDecoration::Close);
    d->minimizeBtn = new QWSButton(this, QWSDecoration::Minimize);
    d->maximizeBtn = new QWSButton(this, QWSDecoration::Maximize, true);
}

QWSManager::~QWSManager()
{
#ifndef QT_NO_POPUPMENU
    if (d->popup)
        delete d->popup;
#endif
    delete d->menuBtn;
    delete d->closeBtn;
    delete d->minimizeBtn;
    delete d->maximizeBtn;

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

QWSDecoration::Region QWSManager::pointInRegion(const QPoint &p)
{
    QWSDecoration &dec = QApplication::qwsDecoration();
    QRect rect(d->managed->geometry());

    for (int i = QWSDecoration::LastRegion; i >= QWSDecoration::Title; i--) {
        if (dec.region(d->managed, rect, (QWSDecoration::Region)i).contains(p))
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

    d->mousePos = e->globalPos();
    d->dx = 0;
    d->dy = 0;
    d->activeRegion = pointInRegion(d->mousePos);
    switch (d->activeRegion) {
    case QWSDecoration::Menu:
        menu(d->managed->geometry().topLeft());
        break;
    case QWSDecoration::Close:
        setClicked(d->closeBtn, true);
        break;
    case QWSDecoration::Minimize:
        setClicked(d->minimizeBtn, true);
        break;
    case QWSDecoration::Maximize:
        setClicked(d->maximizeBtn, true);
        break;
    default:
        break;
    }
    if (d->activeRegion != QWSDecoration::None &&
         d->activeRegion != QWSDecoration::Menu) {
        d->active = d->managed;
        d->managed->grabMouse();
    }
    if (d->activeRegion != QWSDecoration::None &&
         d->activeRegion != QWSDecoration::Close &&
         d->activeRegion != QWSDecoration::Minimize &&
         d->activeRegion != QWSDecoration::Menu) {
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
        handleMove();
        d->mousePos = e->globalPos();
        QWSDecoration::Region rgn = pointInRegion(e->globalPos());
        QWSDecoration::Region activatedRegion = d->activeRegion;
        d->activeRegion = QWSDecoration::None;
        d->active = 0;
        switch (activatedRegion) {
            case QWSDecoration::Close:
                setClicked(d->closeBtn, false);
                if (rgn == QWSDecoration::Close) {
                    close();
                    return;
                }
                break;
            case QWSDecoration::Minimize:
                setClicked(d->minimizeBtn, false);
                if (rgn == QWSDecoration::Minimize)
                    minimize();
                break;
            case QWSDecoration::Maximize:
                setClicked(d->maximizeBtn, false);
                if (rgn == QWSDecoration::Maximize)
                    toggleMaximize();
                break;
            default:
                break;
        }
    } else if (d->activeRegion == QWSDecoration::None) {
        d->active = 0;
    }
}

void QWSManager::mouseMoveEvent(QMouseEvent *e)
{
#ifndef QT_NO_CURSOR
    static Qt::CursorShape shape[] = { Qt::ArrowCursor, Qt::ArrowCursor, Qt::ArrowCursor,
                            Qt::SizeVerCursor, Qt::SizeVerCursor, Qt::SizeHorCursor,
                            Qt::SizeHorCursor, Qt::SizeFDiagCursor, Qt::SizeBDiagCursor,
                            Qt::SizeBDiagCursor, Qt::SizeFDiagCursor, Qt::ArrowCursor,
                            Qt::ArrowCursor, Qt::ArrowCursor, Qt::ArrowCursor, Qt::ArrowCursor};

    // cursor
    QWSDisplay *qwsd = QApplication::desktop()->qwsDisplay();
    if (d->activeRegion == QWSDecoration::None)
    {
        if (!QWidget::mouseGrabber()) {
            QWSDecoration::Region r = pointInRegion(e->globalPos());
            qwsd->selectCursor(d->managed, shape[r]);
        }
    } else
        qwsd->selectCursor(d->managed, shape[d->activeRegion]);
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

    d->dx = g.x() - d->mousePos.x();
    d->dy = g.y() - d->mousePos.y();

    handleMove();

    // button regions
    QWSDecoration::Region r = pointInRegion(e->globalPos());
    setMouseOver(d->menuBtn, r == QWSDecoration::Menu);
    setMouseOver(d->closeBtn, r == QWSDecoration::Close);
    setMouseOver(d->minimizeBtn, r == QWSDecoration::Minimize);
    setMouseOver(d->maximizeBtn, r == QWSDecoration::Maximize);
}

void QWSManager::handleMove()
{
    if (!d->dx && !d->dy)
        return;

    int x = d->managed->geometry().x();
    int y = d->managed->geometry().y();
    int w = d->managed->width();
    int h = d->managed->height();

    QRect geom(d->managed->geometry());

    switch (d->activeRegion) {
        case QWSDecoration::Title:
            geom = QRect(x + d->dx, y + d->dy, w, h);
            break;
        case QWSDecoration::Top:
            geom = QRect(x, y + d->dy, w, h - d->dy);
            break;
        case QWSDecoration::Bottom:
            geom = QRect(x, y, w, h + d->dy);
            break;
        case QWSDecoration::Left:
            geom = QRect(x + d->dx, y, w - d->dx, h);
            break;
        case QWSDecoration::Right:
            geom = QRect(x, y, w + d->dx, h);
            break;
        case QWSDecoration::TopRight:
            geom = QRect(x, y + d->dy, w + d->dx, h - d->dy);
            break;
        case QWSDecoration::TopLeft:
            geom = QRect(x + d->dx, y + d->dy, w - d->dx, h - d->dy);
            break;
        case QWSDecoration::BottomLeft:
            geom = QRect(x + d->dx, y, w - d->dx, h + d->dy);
            break;
        case QWSDecoration::BottomRight:
            geom = QRect(x, y, w + d->dx, h + d->dy);
            break;
        default:
            return;
    }

    if (geom.width() >= d->managed->minimumWidth()
            && geom.width() <= d->managed->maximumWidth()) {
        d->mousePos.setX(d->mousePos.x() + d->dx);
    } else if (geom.width() < d->managed->minimumWidth()) {
        if (x != geom.x()) {
            geom.setX(x+(w-d->managed->minimumWidth()));
            d->mousePos.setX(geom.x());
        } else {
            d->mousePos.setX(x+d->managed->minimumWidth());
        }
        geom.setWidth(d->managed->minimumWidth());
    } else {
        geom.setX(x);
        geom.setWidth(w);
    }
    if (geom.height() >= d->managed->minimumHeight()
            && geom.height() <= d->managed->maximumHeight()) {
        d->mousePos.setY(d->mousePos.y() + d->dy);
    } else if (geom.height() < d->managed->minimumHeight()) {
        if (y != geom.y()) {
            geom.setY(y+(h-d->managed->minimumHeight()));
            d->mousePos.setY(d->mousePos.y()+(geom.y()-y));
        } else {
            d->mousePos.setY(y+d->managed->minimumHeight());
        }
        geom.setHeight(d->managed->minimumHeight());
    } else {
        geom.setY(y);
        geom.setHeight(h);
    }

    if (geom != d->managed->geometry()) {
        QApplication::sendPostedEvents();
        d->managed->setGeometry(geom);
    }

    d->dx = 0;
    d->dy = 0;
}

void QWSManager::paintEvent(QPaintEvent *)
{
    if (!d->managed->isVisible())
        return;
    QWSDecoration &dec = QApplication::qwsDecoration();
    if (d->managed->testWState(Qt::WState_InPaintEvent))
        qWarning("QWSManager::paintEvent() recursive paint event detected");
    d->managed->setWState(Qt::WState_InPaintEvent);
    QPainter painter(d->managed);

    // Adjust our widget region to contain the window
    // manager decoration instead of the widget itself.
    QRegion r = d->managed->d->topData()->decor_allocated_region;
    int rgnIdx = d->managed->data->alloc_region_index;

    QGfx *gfx = static_cast<QWSPaintEngine *>(painter.d->engine)->gfx();
    if (rgnIdx >= 0) {
        QRegion newRegion;
        bool changed = false;
        QWSDisplay::grab();
        const int *rgnRev = qt_fbdpy->regionManager()->revision(rgnIdx);
        if (d->managed->data->alloc_region_revision != *rgnRev) {
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

    painter.setClipRegion(dec.region(d->managed, d->managed->rect()));
    dec.paint(&painter, d->managed);
    painter.setClipRegion(dec.region(d->managed, d->managed->rect()));
    dec.paintButton(&painter, d->managed, QWSDecoration::Menu, d->menuBtn->state());
    dec.paintButton(&painter, d->managed, QWSDecoration::Close, d->closeBtn->state());
    dec.paintButton(&painter, d->managed, QWSDecoration::Minimize, d->minimizeBtn->state());
    dec.paintButton(&painter, d->managed, QWSDecoration::Maximize, d->maximizeBtn->state());
    d->managed->clearWState(Qt::WState_InPaintEvent);
}

void QWSManager::menu(const QPoint &pos)
{
#ifndef QT_NO_POPUPMENU
    if (!d->popup) {
        // Basic window operation menu
        d->popup = new QMenu();
        d->menuActions[QWSManagerPrivate::NormalizeAction] = new QAction(qApp->translate("QWSDecoration",  "&Restore"));
        d->menuActions[QWSManagerPrivate::TitleAction] = new QAction(qApp->translate("QWSDecoration",  "&Move"));
        d->menuActions[QWSManagerPrivate::BottomRightAction] = new QAction(qApp->translate("QWSDecoration",  "&Size"));
        d->menuActions[QWSManagerPrivate::MinimizeAction] = new QAction(qApp->translate("QWSDecoration",  "Mi&nimize"));
        d->menuActions[QWSManagerPrivate::MaximizeAction] = new QAction(qApp->translate("QWSDecoration",  "Ma&ximize"));
        d->menuActions[QWSManagerPrivate::CloseAction] = new QAction(qApp->translate("QWSDecoration",  "Close"));

        d->popup->addAction(d->menuActions[QWSManagerPrivate::NormalizeAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::TitleAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::BottomRightAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::MinimizeAction]);
        d->popup->addAction(d->menuActions[QWSManagerPrivate::MaximizeAction]);
        d->popup->addSeparator();
        d->popup->addAction(d->menuActions[QWSManagerPrivate::CloseAction]);
        connect(d->popup, SIGNAL(activated(QAction*)), SLOT(menuActivated(QAction*)));

        // Add Style menu
        QMenu *styleMenu = new QMenu();
        for (int i = 0; !WMStyleList[i].WMStyleName.isEmpty(); i++)
            styleMenu->addAction(qApp->translate("QWSDecoration", WMStyleList[i].WMStyleName.latin1()));
        styleMenu->addSeparator();
        styleMenu->addAction("foobar");
        connect(styleMenu, SIGNAL(activated(QAction*)), this, SLOT(styleMenuActivated(QAction*)));
        d->popup->addSeparator();
        d->popup->addMenu(tr("Style"), styleMenu);
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

void QWSManager::styleMenuActivated(QAction *item)
{
    for (int i = 0; !WMStyleList[i].WMStyleName.isEmpty(); i++) {
        if (item->text() == qApp->translate("QWSDecoration", WMStyleList[i].WMStyleName.latin1()))
            qApp->qwsSetDecoration(WMStyleList[i].new_WMDecorations());
    }

    // Force a repaint of the WM regions
    const QSize s = d->managed->size();
    d->managed->resize(s.width() + 1, s.height());
    d->managed->resize(s.width(), s.height());
}

void QWSManager::menuActivated(QAction *item)
{
    if (item == d->menuActions[QWSManagerPrivate::CloseAction]) {
        close();
    } else if (item == d->menuActions[QWSManagerPrivate::MinimizeAction]) {
        minimize();
    } else if (item == d->menuActions[QWSManagerPrivate::MaximizeAction]
               || item == d->menuActions[QWSManagerPrivate::NormalizeAction]) {
        toggleMaximize();
    } else if (item == d->menuActions[QWSManagerPrivate::TitleAction]) {
        d->mousePos = QCursor::pos();
        d->activeRegion = QWSDecoration::Title;
        d->active = d->managed;
        d->managed->grabMouse();
    } else if (item == d->menuActions[QWSManagerPrivate::BottomRightAction]) {
        d->mousePos = QCursor::pos();
        d->activeRegion = QWSDecoration::BottomRight;
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
    setOn(d->maximizeBtn,true);
}

void QWSManager::toggleMaximize()
{
    if (!d->managed->isMaximized()) {
        d->managed->showMaximized();
        setOn(d->maximizeBtn,true);
    } else {
        d->managed->showNormal();
        setOn(d->maximizeBtn,false);
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
    if (!d->managed->isVisible())
        return;
    QWSDecoration &dec = QApplication::qwsDecoration();
    if (d->managed->testWState(Qt::WState_InPaintEvent))
        qWarning("QWSManager::repaintButton() recursive paint event detected");
    d->managed->setWState(Qt::WState_InPaintEvent);
    //### This isn't really inside a paint event
    //optimization instead of calling paintEvent()
    QPainter painter(d->managed);
    painter.setClipRegion(dec.region(d->managed, d->managed->rect()));
    dec.paintButton(&painter, d->managed, b->type(), b->state());
    d->managed->clearWState(Qt::WState_InPaintEvent);
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
