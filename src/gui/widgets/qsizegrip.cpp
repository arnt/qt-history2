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

#include "qsizegrip.h"

#ifndef QT_NO_SIZEGRIP

#include "qapplication.h"
#include "qevent.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"

#if defined(Q_WS_X11)
#include <private/qt_x11_p.h>
#elif defined (Q_WS_WIN)
#include "qt_windows.h"
#endif

#include <private/qwidget_p.h>

class QSizeGripPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QSizeGrip)
public:
    void init();
    QPoint p;
    QSize s;
    int d;
    bool hiddenByUser;
    bool atBottom;
};

static QWidget *qt_sizegrip_topLevelWidget(QWidget* w)
{
    QWidget *p = w->parentWidget();
    while (!w->isWindow() && p && !p->inherits("QWorkspace")) {
        w = p;
        p = p->parentWidget();
    }
    return w;
}

static QWidget* qt_sizegrip_workspace(QWidget* w)
{
    while (w && !w->inherits("QWorkspace")) {
        if (w->isWindow())
            return 0;
        w = w->parentWidget();
    }
    return w;
}

static bool qt_sizegrip_atBottom(QWidget* sg)
{
    QWidget *tlw = qt_sizegrip_topLevelWidget(sg);
    return tlw->mapFromGlobal(sg->mapToGlobal(QPoint(0, 0))).y() >= (tlw->height() / 2);
}

/*!
    \class QSizeGrip qsizegrip.h

    \brief The QSizeGrip class provides a corner-grip for resizing a top-level window.

    \ingroup application
    \ingroup basic
    \ingroup appearance

    This widget works like the standard Windows resize handle. In the
    X11 version this resize handle generally works differently from
    the one provided by the system; we hope to reduce this difference
    in the future.

    Put this widget anywhere in a widget tree and the user can use it
    to resize the top-level window. Generally, this should be in the
    lower right-hand corner. Note that QStatusBar already uses this
    widget, so if you have a status bar (e.g. you are using
    QMainWindow), then you don't need to use this widget explicitly.

    On some platforms the sizegrip automatically hides itself when the
    window is shown full screen or maximised.

    \inlineimage qsizegrip-m.png Screenshot in Motif style
    \inlineimage qsizegrip-w.png Screenshot in Windows style

    \sa QStatusBar QWidget::windowState()
*/


/*!
    Constructs a resize corner as a child widget of \a
    parent.
*/
QSizeGrip::QSizeGrip(QWidget * parent)
    : QWidget(*new QSizeGripPrivate, parent, 0)
{
    Q_D(QSizeGrip);
    d->init();
}

#ifdef QT3_SUPPORT
/*!
  \obsolete

    Constructs a resize corner called \a name, as a child widget of \a
    parent.
*/
QSizeGrip::QSizeGrip(QWidget * parent, const char* name)
    : QWidget(*new QSizeGripPrivate, parent, 0)
{
    Q_D(QSizeGrip);
    setObjectName(QString::fromAscii(name));
    d->init();
}
#endif

void QSizeGripPrivate::init()
{
    Q_Q(QSizeGrip);
    hiddenByUser = false;
    atBottom = qt_sizegrip_atBottom(q);
#ifndef QT_NO_CURSOR
#ifndef Q_WS_MAC
    q->setCursor(q->isRightToLeft() ^ atBottom ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
#endif
#endif
    q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
#if defined(Q_WS_X11)
    if (!qt_sizegrip_workspace(q)) {
        WId id = q->winId();
        XChangeProperty(X11->display, q->window()->winId(),
                        ATOM(_QT_SIZEGRIP), XA_WINDOW, 32, PropModeReplace,
                        (unsigned char *)&id, 1);
    }
#endif
    QWidget *tlw = qt_sizegrip_topLevelWidget(q);
    tlw->installEventFilter(q);
}


/*!
    Destroys the size grip.
*/
QSizeGrip::~QSizeGrip()
{
#if defined(Q_WS_X11)
    if (!QApplication::closingDown() && parentWidget()) {
        WId id = XNone;
        XChangeProperty(X11->display, window()->winId(),
                        ATOM(_QT_SIZEGRIP), XA_WINDOW, 32, PropModeReplace,
                        (unsigned char *)&id, 1);
    }
#endif
}

/*!
    Returns the size grip's size hint; this is a small size.
*/
QSize QSizeGrip::sizeHint() const
{
    QStyleOption opt(0);
    opt.init(this);
    return (style()->sizeFromContents(QStyle::CT_SizeGrip, &opt, QSize(13, 13), this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    Paints the resize grip. Resize grips are usually rendered as small
    diagonal textured lines in the lower-right corner. The event is in
    \a event.
*/
void QSizeGrip::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    Q_D(QSizeGrip);
    if (d->p.isNull()) { // update "bottomness" unless we are resizing
#ifndef QT_NO_CURSOR
#ifndef Q_WS_MAC
        if (qt_sizegrip_atBottom(this) != d->atBottom) {
            d->atBottom = !d->atBottom;
            setCursor(isRightToLeft() ^ d->atBottom ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
        }
#endif
#endif
    }
    QPainter painter(this);
    QStyleOptionSizeGrip opt;
    opt.init(this);
    if (isRightToLeft())
        opt.corner = d->atBottom ? Qt::BottomLeftCorner : Qt::TopLeftCorner;
    else
        opt.corner = d->atBottom ? Qt::BottomRightCorner : Qt::TopRightCorner;
    style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
}

/*!
    Primes the resize operation. The event is in \a e.
*/
void QSizeGrip::mousePressEvent(QMouseEvent * e)
{
    Q_D(QSizeGrip);
    d->p = e->globalPos();
    d->s = qt_sizegrip_topLevelWidget(this)->size();
}


/*!
    Resizes the top-level widget containing this widget. The event is
    in \a e.
*/
void QSizeGrip::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() != Qt::LeftButton)
        return;

    Q_D(QSizeGrip);
    QWidget* tlw = qt_sizegrip_topLevelWidget(this);
    if (tlw->testAttribute(Qt::WA_WState_ConfigPending))
        return;

    QPoint np(e->globalPos());

    QWidget* ws = qt_sizegrip_workspace(this);
    if (ws) {
        QPoint tmp(ws->mapFromGlobal(np));
        if (tmp.x() > ws->width())
            tmp.setX(ws->width());
        if (tmp.y() > ws->height())
            tmp.setY(ws->height());
        np = ws->mapToGlobal(tmp);
    }

    QSize ns;
    if (d->atBottom)
        ns.rheight() = np.y() - d->p.y() + d->s.height();
    else
        ns.rheight() = d->s.height() - (np.y() - d->p.y());

    if (isRightToLeft())
        ns.rwidth() = d->s.width() - (np.x() - d->p.x());
    else
        ns.rwidth() = np.x() - d->p.x() + d->s.width();

    ns = ns.expandedTo(tlw->minimumSize()).expandedTo(tlw->minimumSizeHint()).boundedTo(tlw->maximumSize());

    QRect g = tlw->geometry();
    if (d->atBottom) {
        if (isRightToLeft()) {
            tlw->setGeometry(g.right() - ns.width() + 1, g.y(), ns.width(), ns.height());
            int width = tlw->width();
            if (width != ns.width()) // check if the platform was able to set the requested geometry
                tlw->setGeometry(g.right() - width + 1, g.y(), width, tlw->height());
        } else
            tlw->resize(ns);
    } else {
        if (isRightToLeft())
            tlw->setGeometry(g.right() - ns.width() + 1, g.bottom() - ns.height() + 1, 
                             ns.width(), ns.height());
        else
            tlw->setGeometry(g.x(), g.bottom() - ns.height() + 1, ns.width(), ns.height());
    }
#ifdef Q_WS_WIN
    MSG msg;
    while(PeekMessage(&msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
      ;
#endif
    QApplication::syncX();
}

/*! \reimp */
void QSizeGrip::setVisible(bool visible)
{
    Q_D(QSizeGrip);
    d->hiddenByUser = !visible;
    QWidget::setVisible(visible);
}


/*! \reimp */
bool QSizeGrip::eventFilter(QObject *o, QEvent *e)
{
    Q_D(QSizeGrip);
    QWidget *tlw = qt_sizegrip_topLevelWidget(this);
    if (!d->hiddenByUser && e->type() == QEvent::WindowStateChange && o == tlw) {
        QWidget::setVisible((tlw->windowState() &
                             (Qt::WindowFullScreen
#ifndef Q_WS_MAC
                              | Qt::WindowMaximized
#endif
                                 ))==0);
    }
    return false;
}

/*! \reimp */
bool QSizeGrip::event(QEvent *e)
{
    Q_D(QSizeGrip);
#if defined(Q_WS_MAC)
    switch(e->type()) {
    case QEvent::Hide:
    case QEvent::Show:
        if(!QApplication::closingDown() && parentWidget() && !qt_sizegrip_workspace(this)) {
            if(QWidget *w = qt_sizegrip_topLevelWidget(this)) {
                if(w->isWindow())
                    QWidgetPrivate::qt_mac_update_sizer(w, e->type() == QEvent::Hide ? -1 : 1);
            }
        }
        break;
    default:
        break;
    }
#endif
    switch(e->type()) {
        case QEvent::MouseButtonRelease:
            d->p = QPoint();
            break;
    }
    return QWidget::event(e);
}

#endif //QT_NO_SIZEGRIP
