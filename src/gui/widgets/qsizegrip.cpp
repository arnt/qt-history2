/****************************************************************************
**
** Implementation of QSizeGrip class.
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
#elif defined(Q_WS_MAC)
#include <private/qwidget_p.h>
#endif


static QWidget *qt_sizegrip_topLevelWidget(QWidget* w)
{
    QWidget *p = w->parentWidget();
    while (!w->isTopLevel() && p && !p->inherits("QWorkspace")) {
        w = p;
        p = p->parentWidget();
    }
    return w;
}

static QWidget* qt_sizegrip_workspace(QWidget* w)
{
    while (w && !w->inherits("QWorkspace")) {
        if (w->isTopLevel())
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

    <img src=qsizegrip-m.png> <img src=qsizegrip-w.png>

    \sa QStatusBar
*/


/*!
    Constructs a resize corner called \a name, as a child widget of \a
    parent.
*/
QSizeGrip::QSizeGrip(QWidget * parent, const char* name)
    : QWidget(parent, name)
{
#ifndef QT_NO_CURSOR
#ifndef Q_WS_MAC
    if (QApplication::reverseLayout())
        setCursor(Qt::SizeBDiagCursor);
    else
        setCursor(Qt::SizeFDiagCursor);
#endif
#endif
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
#if defined(Q_WS_X11)
    if (!qt_sizegrip_workspace(this)) {
        WId id = winId();
        XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
                        ATOM(_QT_SIZEGRIP), XA_WINDOW, 32, PropModeReplace,
                        (unsigned char *)&id, 1);
    }
#endif
    tlw = qt_sizegrip_topLevelWidget(this);
    if (tlw)
        tlw->installEventFilter(this);
    installEventFilter(this); //for binary compatibility fix in 4.0 with an event() ###
}


/*!
    Destroys the size grip.
*/
QSizeGrip::~QSizeGrip()
{
#if defined(Q_WS_X11)
    if (!QApplication::closingDown() && parentWidget()) {
        WId id = XNone;
        XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
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
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::Style_Default;
    return (style().sizeFromContents(QStyle::CT_SizeGrip, &opt, QSize(13, 13), fontMetrics(), this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    Paints the resize grip. Resize grips are usually rendered as small
    diagonal textured lines in the lower-right corner. The event is in
    \a e.
*/
void QSizeGrip::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setClipRegion(e->region());
    QStyleOption opt(0, QStyleOption::Default);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::Style_Default;
    style().drawPrimitive(QStyle::PE_SizeGrip, &opt, &painter, this);
}

/*!
    Primes the resize operation. The event is in \a e.
*/
void QSizeGrip::mousePressEvent(QMouseEvent * e)
{
    p = e->globalPos();
    s = qt_sizegrip_topLevelWidget(this)->size();
}


/*!
    Resizes the top-level widget containing this widget. The event is
    in \a e.
*/
void QSizeGrip::mouseMoveEvent(QMouseEvent * e)
{
    if (e->state() != Qt::LeftButton)
        return;

    QWidget* tlw = qt_sizegrip_topLevelWidget(this);
    if (tlw->testWState(Qt::WState_ConfigPending))
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
    int h = np.y() - p.y() + s.height();

    if (QApplication::reverseLayout())
        w = s.width() - (np.x() - p.x());
    else
        w = np.x() - p.x() + s.width();

    if (w < 1)
        w = 1;
    if (h < 1)
        h = 1;
    QSize ms(tlw->minimumSizeHint());
    ms = ms.expandedTo(minimumSize());
    if (w < ms.width())
        w = ms.width();
    if (h < ms.height())
        h = ms.height();

    if (QApplication::reverseLayout()) {
        if (tlw->isTopLevel()) {
            int x = tlw->geometry().x() + (np.x()-p.x());
            int y = tlw->geometry().y();
            tlw->setGeometry(x,y,w,h);
        } else {
            tlw->resize(w, h);
            if (tlw->size() == QSize(w,h))
                tlw->move(tlw->x() + (np.x()-p.x()), tlw->y());
        }
    } else {
        tlw->resize(w, h);
    }
#ifdef Q_WS_WIN
    MSG msg;
    while(PeekMessage(&msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
      ;
#endif
    QApplication::syncX();

    if (QApplication::reverseLayout() && tlw->size() == QSize(w,h)) {
        s.rwidth() = tlw->size().width();
        p.rx() = np.x();
    }
}

/*! \reimp */
bool QSizeGrip::eventFilter(QObject *o, QEvent *e)
{
    if (o == tlw) {
        switch (e->type()) {
#ifndef Q_WS_MAC
        /* The size grip goes no where on Mac OS X when you maximize!  --Sam */
        case QEvent::ShowMaximized:
#endif
        case QEvent::ShowFullScreen:
            hide();
            break;
        case QEvent::ShowNormal:
            show();
            break;
        default:
            break;
        }
    } else if(o == this) {
        switch(e->type()) {
#if defined(Q_WS_MAC)
        case QEvent::Hide:
        case QEvent::Show:
            if(!QApplication::closingDown() && parentWidget() && !qt_sizegrip_workspace(this)) {
                if(QWidget *w = qt_sizegrip_topLevelWidget(this)) {
                    if(w->isTopLevel())
                        QWidgetPrivate::qt_mac_update_sizer(w, e->type() == QEvent::Hide ? -1 : 1);
                }
            }
            break;
#endif
        case QEvent::Resize: {
            QPointArray pa(3);
            if (!QApplication::reverseLayout()) {
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
    }
    return false;
}

#endif //QT_NO_SIZEGRIP
