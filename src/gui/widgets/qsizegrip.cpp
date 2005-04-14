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
    QWidget *tlw;
    bool hiddenByUser;
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
    setObjectName(name);
    d->init();
}
#endif

void QSizeGripPrivate::init()
{
    Q_Q(QSizeGrip);
    hiddenByUser = false;
#ifndef QT_NO_CURSOR
#ifndef Q_WS_MAC
    q->setCursor(q->isRightToLeft() ? Qt::SizeBDiagCursor : Qt::SizeFDiagCursor);
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
    tlw = qt_sizegrip_topLevelWidget(q);
    if (tlw)
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
    \a e.
*/
void QSizeGrip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QStyleOption opt(0);
    opt.init(this);
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

    int w;
    int h = np.y() - d->p.y() + d->s.height();

    if (isRightToLeft())
        w = d->s.width() - (np.x() - d->p.x());
    else
        w = np.x() - d->p.x() + d->s.width();

    if (w < 1)
        w = 1;
    if (h < 1)
        h = 1;
    QSize ms(tlw->minimumSize());
    ms = ms.expandedTo(minimumSize());
    if (w < ms.width())
        w = ms.width();
    if (h < ms.height())
        h = ms.height();

    if (isRightToLeft()) {
        d->tlw->resize(w, h);
        if (d->tlw->size() == QSize(w, h))
            d->tlw->move(tlw->x() + (np.x() - d->p.x()), tlw->y());
    } else {
        d->tlw->resize(w, h);
    }
#ifdef Q_WS_WIN
    MSG msg;
    while(PeekMessage(&msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
      ;
#endif
    QApplication::syncX();

    if (isRightToLeft() && d->tlw->size() == QSize(w,h)) {
        d->s.rwidth() = tlw->size().width();
        d->p.rx() = np.x();
    }
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
    if (!d->hiddenByUser && e->type() == QEvent::WindowStateChange && o == d->tlw) {
        QWidget::setVisible((d->tlw->windowState() &
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
    switch(e->type()) {
#if defined(Q_WS_MAC)
    case QEvent::Hide:
    case QEvent::Show:
        if(!QApplication::closingDown() && parentWidget() && !qt_sizegrip_workspace(this)) {
            if(QWidget *w = qt_sizegrip_topLevelWidget(this)) {
                if(w->isWindow())
                    QWidgetPrivate::qt_mac_update_sizer(w, e->type() == QEvent::Hide ? -1 : 1);
            }
        }
        break;
#endif
    case QEvent::Resize: {
        QPolygon pa(3);
        if (isLeftToRight()) {
            pa.setPoint(0, width() + 1, 0);
            pa.setPoint(1, width() + 1, height() + 1);
            pa.setPoint(2, 0, height());
        } else {
            pa.setPoint(0, 0, 0);
            pa.setPoint(1, width() + 1, height() + 1);
            pa.setPoint(2, 0, height() + 1);
        }
        clearMask();
        setMask(QRegion(pa));
        break;
    }
    default:
        break;
    }
    return QWidget::event(e);
}

#endif //QT_NO_SIZEGRIP
