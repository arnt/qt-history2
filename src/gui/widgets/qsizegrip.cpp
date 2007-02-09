/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
#include "qdebug.h"
#include <QDesktopWidget>

#if defined(Q_WS_X11)
#include <private/qt_x11_p.h>
#elif defined (Q_WS_WIN)
#include "qt_windows.h"
#endif
#ifdef Q_WS_MAC
#include <private/qt_mac_p.h>
#endif

#include <private/qwidget_p.h>

#define SZ_SIZEBOTTOMRIGHT  0xf008
#define SZ_SIZEBOTTOMLEFT   0xf007
#define SZ_SIZETOPLEFT      0xf004
#define SZ_SIZETOPRIGHT     0xf005

class QSizeGripPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QSizeGrip)
public:
    void init();
    QPoint p;
    QRect r;
    int d;
    int dxMax;
    int dyMax;
    bool hiddenByUser;
    bool atBottom;
    bool gotMousePress;
};

static QWidget *qt_sizegrip_topLevelWidget(QWidget* w)
{
    while (w && !w->isWindow() && w->windowType() != Qt::SubWindow)
        w = w->parentWidget();
    return w;
}

static bool qt_sizegrip_atBottom(QWidget* sg)
{
    QWidget *tlw = qt_sizegrip_topLevelWidget(sg);
    return tlw->mapFromGlobal(sg->mapToGlobal(QPoint(0, 0))).y() >= (tlw->height() / 2);
}

/*!
    \class QSizeGrip

    \brief The QSizeGrip class provides a corner-grip for resizing a top-level window.

    \ingroup application
    \ingroup basic
    \ingroup appearance

    This widget works like the standard Windows resize handle. In the
    X11 version this resize handle generally works differently from
    the one provided by the system if the X11 window manager does not
    support necessary modern post-ICCCM specifications.

    Put this widget anywhere in a widget tree and the user can use it
    to resize the top-level window. Generally, this should be in the
    lower right-hand corner. Note that QStatusBar already uses this
    widget, so if you have a status bar (e.g. you are using
    QMainWindow), then you don't need to use this widget explicitly.

    On some platforms the sizegrip automatically hides itself when the
    window is shown full screen or maximised.

    \table 50%
    \row \o \inlineimage plastique-sizegrip.png Screenshot of a Plastique style size grip
    \o A size grip widget at the bottom-right corner of a main window, shown in the
    \l{Plastique Style Widget Gallery}{Plastique widget style}.
    \endtable

    The QSizeGrip class inherits QWidget and reimplements the \l
    {QWidget::mousePressEvent()}{mousePressEvent()} and \l
    {QWidget::mouseMoveEvent()}{mouseMoveEvent()} functions to feature
    the resize functionality, and the \l
    {QWidget::paintEvent()}{paintEvent()} function to render the
    sizegrip widget.

    \sa QStatusBar QWidget::windowState()
*/


/*!
    Constructs a resize corner as a child widget of  the given \a
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

    Constructs a resize corner with the given \a name, as a child
    widget of the given \a parent.
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
    dxMax = 0;
    dyMax = 0;
    hiddenByUser = false;
    atBottom = qt_sizegrip_atBottom(q);
    gotMousePress = false;

#ifndef QT_NO_CURSOR
#ifndef Q_WS_MAC
    q->setCursor(q->isRightToLeft() ^ atBottom ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
#endif
#endif
    q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    QWidget *tlw = qt_sizegrip_topLevelWidget(q);
    tlw->installEventFilter(q);
}


/*!
    Destroys this size grip.
*/
QSizeGrip::~QSizeGrip()
{
}

/*!
  \reimp
*/
QSize QSizeGrip::sizeHint() const
{
    QStyleOption opt(0);
    opt.init(this);
    return (style()->sizeFromContents(QStyle::CT_SizeGrip, &opt, QSize(13, 13), this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    Paints the resize grip.

    Resize grips are usually rendered as small diagonal textured lines
    in the lower-right corner. The paint event is passed in the \a
    event parameter.
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
    \fn void QSizeGrip::mousePressEvent(QMouseEvent * event)

    Receives the mouse press events for the widget, and primes the
    resize operation. The mouse press event is passed in the \a event
    parameter.
*/
void QSizeGrip::mousePressEvent(QMouseEvent * e)
{
    QWidget *tlw = qt_sizegrip_topLevelWidget(this);

#ifdef Q_WS_X11
    if (e->button() == Qt::LeftButton
        && X11->isSupportedByWM(ATOM(_NET_WM_MOVERESIZE))
        && tlw->isWindow()) {
        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.message_type = ATOM(_NET_WM_MOVERESIZE);
        xev.xclient.display = X11->display;
        xev.xclient.window = tlw->winId();
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = e->globalPos().x();
        xev.xclient.data.l[1] = e->globalPos().y();
        xev.xclient.data.l[2] = (QApplication::layoutDirection() == Qt::RightToLeft) ? 6 : 4; // bottomleft/bottomright
        xev.xclient.data.l[3] = Button1;
        xev.xclient.data.l[4] = 0;
        XUngrabPointer(X11->display, X11->time);
        XSendEvent(X11->display, QX11Info::appRootWindow(x11Info().screen()), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &xev);
        return;
    }
#endif

    Q_D(QSizeGrip);
    d->gotMousePress = true;
    d->p = e->globalPos();
    d->r = tlw->geometry();

    // Find available desktop/workspace geometry.
    QRect availableGeometry;
    if (tlw->isWindow())
        availableGeometry = QApplication::desktop()->availableGeometry(tlw);
    else
        availableGeometry = tlw->parentWidget()->contentsRect();

    // Find frame geometries, title bar height, and decoration sizes.
    const QRect frameGeometry = tlw->frameGeometry();
    const int titleBarHeight = qMax(tlw->geometry().y() - frameGeometry.y(), 0);
    const int bottomDecoration = qMax(frameGeometry.height() - tlw->height() - titleBarHeight, 0);
    const int leftRightDecoration = qMax((frameGeometry.width() - tlw->width()) / 2, 0);

    // Determine dyMax depending on whether the sizegrip is at the bottom
    // of the widget or not.
    if (d->atBottom)
        d->dyMax = availableGeometry.bottom() - d->r.bottom() - bottomDecoration;
    else
        d->dyMax = availableGeometry.y() - d->r.y() + titleBarHeight;

    // In RTL mode, the size grip is to the left; find dxMax from the desktop/workspace
    // geometry, the size grip geometry and the width of the decoration.
    if (isRightToLeft())
        d->dxMax = availableGeometry.x() - d->r.x() + leftRightDecoration;
    else
        d->dxMax = availableGeometry.right() - d->r.right() - leftRightDecoration;
}


/*!
    \fn void QSizeGrip::mouseMoveEvent(QMouseEvent * event)
    Resizes the top-level widget containing this widget. The mouse
    move event is passed in the \a event parameter.
*/
void QSizeGrip::mouseMoveEvent(QMouseEvent * e)
{
    QWidget* tlw = qt_sizegrip_topLevelWidget(this);

#ifdef Q_WS_X11
    if (e->button() == Qt::LeftButton
        && X11->isSupportedByWM(ATOM(_NET_WM_MOVERESIZE))
        && tlw->isTopLevel())
        return;
#endif

    if (e->buttons() != Qt::LeftButton)
        return;

    Q_D(QSizeGrip);
    if (d->gotMousePress == false)
        return;

    if (tlw->testAttribute(Qt::WA_WState_ConfigPending))
        return;

    QPoint np(e->globalPos());

    if (!tlw->isWindow()) {
        QWidget* ws = tlw->parentWidget();
        QPoint tmp(ws->mapFromGlobal(np));
        if (tmp.x() > ws->width())
            tmp.setX(ws->width());
        if (tmp.y() > ws->height())
            tmp.setY(ws->height());
        np = ws->mapToGlobal(tmp);
    }

    // Don't extend beyond the available geometry; bound to dyMax and dxMax.
    QSize ns;
    if (d->atBottom)
        ns.rheight() = d->r.height() + qMin(np.y() - d->p.y(), d->dyMax);
    else
        ns.rheight() = d->r.height() - qMax(np.y() - d->p.y(), d->dyMax);

    if (isRightToLeft())
        ns.rwidth() = d->r.width() - qMax(np.x() - d->p.x(), d->dxMax);
    else
        ns.rwidth() = d->r.width() + qMin(np.x() - d->p.x(), d->dxMax);

    ns = ns.expandedTo(tlw->minimumSize()).expandedTo(tlw->minimumSizeHint()).boundedTo(tlw->maximumSize());

    QPoint p;
    QRect nr(p, ns);
    if (d->atBottom) {
        if (isRightToLeft())
            nr.moveTopRight(d->r.topRight());
        else
            nr.moveTopLeft(d->r.topLeft());
    } else {
        if (isRightToLeft())
            nr.moveBottomRight(d->r.bottomRight());
        else
            nr.moveBottomLeft(d->r.bottomLeft());
    }

    tlw->setGeometry(nr);

#ifdef Q_WS_WIN
    MSG msg;
    while(PeekMessage(&msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
      ;
#endif
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
    switch(e->type()) {
#if defined(Q_WS_MAC)
    case QEvent::Hide:
    case QEvent::Show:
        d->atBottom = qt_sizegrip_atBottom(this);
        if(!QApplication::closingDown() && parentWidget()) {
            if(QWidget *w = qt_sizegrip_topLevelWidget(this)) {
                if(w->isWindow())
                    QWidgetPrivate::qt_mac_update_sizer(w, e->type() == QEvent::Hide ? -1 : 1);
            }
        }
        break;
#endif
    case QEvent::MouseButtonRelease:
        d->gotMousePress = false;
        d->p = QPoint();
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

#ifdef Q_WS_WIN
/*! \reimp */
bool QSizeGrip::winEvent( MSG *m, long *result )
{
    if (m->message == WM_LBUTTONDOWN) {
        // toplevel windows use the native size grip on Windows
        if (QWidget *w = qt_sizegrip_topLevelWidget(this)) {
            if (w->isWindow()) {
                PostMessage(w->winId(), WM_SYSCOMMAND,
                            qApp->isLeftToRight() ? SZ_SIZEBOTTOMRIGHT : SZ_SIZEBOTTOMLEFT, 0);
                return true;
            }
        }
    }
    return QWidget::winEvent(m, result);
}
#endif

#endif //QT_NO_SIZEGRIP
