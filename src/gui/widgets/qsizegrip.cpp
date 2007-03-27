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
#include <QtGui/qabstractscrollarea.h>

#define SZ_SIZEBOTTOMRIGHT  0xf008
#define SZ_SIZEBOTTOMLEFT   0xf007
#define SZ_SIZETOPLEFT      0xf004
#define SZ_SIZETOPRIGHT     0xf005

static QWidget *qt_sizegrip_topLevelWidget(QWidget* w)
{
    while (w && !w->isWindow() && w->windowType() != Qt::SubWindow)
        w = w->parentWidget();
    return w;
}

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
    Qt::Corner m_corner;
    bool gotMousePress;
#ifdef Q_WS_MAC
    void updateMacSizer(bool hide) const;
#endif
    Qt::Corner corner() const;
    inline bool atBottom() const
    {
        return m_corner == Qt::BottomRightCorner || m_corner == Qt::BottomLeftCorner;
    }

    inline bool atLeft() const
    {
        return m_corner == Qt::BottomLeftCorner || m_corner == Qt::TopLeftCorner;
    }
};

#ifdef Q_WS_MAC
void QSizeGripPrivate::updateMacSizer(bool hide) const
{
    Q_Q(const QSizeGrip);
    if (QApplication::closingDown() || !q->parentWidget())
        return;
    QWidget *topLevelWindow = qt_sizegrip_topLevelWidget(const_cast<QSizeGrip *>(q));
    if(topLevelWindow && topLevelWindow->isWindow())
        QWidgetPrivate::qt_mac_update_sizer(topLevelWindow, hide ? -1 : 1);
}
#endif

Qt::Corner QSizeGripPrivate::corner() const
{
    Q_Q(const QSizeGrip);
    QWidget *tlw = qt_sizegrip_topLevelWidget(const_cast<QSizeGrip *>(q));
    const QPoint sizeGripPos = q->mapTo(tlw, QPoint(0, 0));
    const QPoint globalPos = tlw->mapFromGlobal(q->mapToGlobal(QPoint(0, 0)));
    bool isAtBottom = sizeGripPos.y() >= tlw->height() / 2;
    bool isAtLeft = sizeGripPos.x() <= tlw->width() / 2;
    if (isAtLeft)
        return isAtBottom ? Qt::BottomLeftCorner : Qt::TopLeftCorner;
    else
        return isAtBottom ? Qt::BottomRightCorner : Qt::TopRightCorner;
}

/*!
    \class QSizeGrip

    \brief The QSizeGrip class provides a resize handle for resizing top-level windows.

    \ingroup application
    \ingroup basic
    \ingroup appearance

    This widget works like the standard Windows resize handle. In the
    X11 version this resize handle generally works differently from
    the one provided by the system if the X11 window manager does not
    support necessary modern post-ICCCM specifications.

    Put this widget anywhere in a widget tree and the user can use it
    to resize the top-level window or any widget with the Qt::SubWindow
    flag set. Generally, this should be in the lower right-hand corner.
    Note that QStatusBar already uses this widget, so if you have a
    status bar (e.g., you are using QMainWindow), then you don't need
    to use this widget explicitly.

    On some platforms the size grip automatically hides itself when the
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
    size grip widget.

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
    m_corner = corner();
    gotMousePress = false;

#if !defined(QT_NO_CURSOR) && !defined(Q_WS_MAC)
    q->setCursor(m_corner == Qt::TopLeftCorner || m_corner == Qt::BottomRightCorner
                 ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
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
    QPainter painter(this);
    QStyleOptionSizeGrip opt;
    opt.init(this);
    opt.corner = d->m_corner;
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
    if (e->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(e);
        return;
    }

    Q_D(QSizeGrip);
    QWidget *tlw = qt_sizegrip_topLevelWidget(this);
    d->p = e->globalPos();
    d->gotMousePress = true;
    d->r = tlw->geometry();

#ifdef Q_WS_X11
    // Use a native X11 sizegrip for "real" top-level windows if supported.
    if (tlw->isWindow() && X11->isSupportedByWM(ATOM(_NET_WM_MOVERESIZE))) {
        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.message_type = ATOM(_NET_WM_MOVERESIZE);
        xev.xclient.display = X11->display;
        xev.xclient.window = tlw->winId();
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = e->globalPos().x();
        xev.xclient.data.l[1] = e->globalPos().y();
        if (d->atBottom())
            xev.xclient.data.l[2] = d->atLeft() ? 6 : 4; // bottomleft/bottomright
        else
            xev.xclient.data.l[2] = d->atLeft() ? 0 : 2; // topleft/topright
        xev.xclient.data.l[3] = Button1;
        xev.xclient.data.l[4] = 0;
        XUngrabPointer(X11->display, X11->time);
        XSendEvent(X11->display, QX11Info::appRootWindow(x11Info().screen()), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &xev);
        return;
    }
#endif // Q_WS_X11

    // Find available desktop/workspace geometry.
    QRect availableGeometry;
    bool hasVerticalSizeConstraint = true;
    bool hasHorizontalSizeConstraint = true;
    if (tlw->isWindow())
        availableGeometry = QApplication::desktop()->availableGeometry(tlw);
    else {
        const QWidget *tlwParent = tlw->parentWidget();
        // Check if tlw is inside QAbstractScrollArea/QScrollArea.
        // If that's the case tlw->parentWidget() will return the viewport
        // and tlw->parentWidget()->parentWidget() will return the scroll area.
        QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(tlwParent->parentWidget());
        if (scrollArea) {
            hasHorizontalSizeConstraint = scrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
            hasVerticalSizeConstraint = scrollArea->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
        }
        availableGeometry = tlwParent->contentsRect();
    }

    // Find frame geometries, title bar height, and decoration sizes.
    const QRect frameGeometry = tlw->frameGeometry();
    const int titleBarHeight = qMax(tlw->geometry().y() - frameGeometry.y(), 0);
    const int bottomDecoration = qMax(frameGeometry.height() - tlw->height() - titleBarHeight, 0);
    const int leftRightDecoration = qMax((frameGeometry.width() - tlw->width()) / 2, 0);

    // Determine dyMax depending on whether the sizegrip is at the bottom
    // of the widget or not.
    if (d->atBottom()) {
        if (hasVerticalSizeConstraint)
            d->dyMax = availableGeometry.bottom() - d->r.bottom() - bottomDecoration;
        else
            d->dyMax = INT_MAX;
    } else {
        if (hasVerticalSizeConstraint)
            d->dyMax = availableGeometry.y() - d->r.y() + titleBarHeight;
        else
            d->dyMax = -INT_MAX;
    }

    // In RTL mode, the size grip is to the left; find dxMax from the desktop/workspace
    // geometry, the size grip geometry and the width of the decoration.
    if (d->atLeft()) {
        if (hasHorizontalSizeConstraint)
            d->dxMax = availableGeometry.x() - d->r.x() + leftRightDecoration;
        else
            d->dxMax = -INT_MAX;
    } else {
        if (hasHorizontalSizeConstraint)
            d->dxMax = availableGeometry.right() - d->r.right() - leftRightDecoration;
        else
            d->dxMax = INT_MAX;
    }
}


/*!
    \fn void QSizeGrip::mouseMoveEvent(QMouseEvent * event)
    Resizes the top-level widget containing this widget. The mouse
    move event is passed in the \a event parameter.
*/
void QSizeGrip::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() != Qt::LeftButton) {
        QWidget::mouseMoveEvent(e);
        return;
    }

    Q_D(QSizeGrip);
    QWidget* tlw = qt_sizegrip_topLevelWidget(this);
    if (!d->gotMousePress || tlw->testAttribute(Qt::WA_WState_ConfigPending))
        return;

#ifdef Q_WS_X11
    if (tlw->isWindow() && X11->isSupportedByWM(ATOM(_NET_WM_MOVERESIZE))
        && tlw->isTopLevel())
        return;
#endif
#ifdef Q_WS_WIN
    if (tlw->isWindow() && ::GetSystemMenu(tlw->winId(), FALSE) != 0) {
        MSG msg;
        while(PeekMessage(&msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE));
        return;
    }
#endif

    QPoint np(e->globalPos());

    // Don't extend beyond the available geometry; bound to dyMax and dxMax.
    QSize ns;
    if (d->atBottom())
        ns.rheight() = d->r.height() + qMin(np.y() - d->p.y(), d->dyMax);
    else
        ns.rheight() = d->r.height() - qMax(np.y() - d->p.y(), d->dyMax);

    if (d->atLeft())
        ns.rwidth() = d->r.width() - qMax(np.x() - d->p.x(), d->dxMax);
    else
        ns.rwidth() = d->r.width() + qMin(np.x() - d->p.x(), d->dxMax);

    ns = ns.expandedTo(tlw->minimumSize()).expandedTo(tlw->minimumSizeHint()).boundedTo(tlw->maximumSize());

    QPoint p;
    QRect nr(p, ns);
    if (d->atBottom()) {
        if (d->atLeft())
            nr.moveTopRight(d->r.topRight());
        else
            nr.moveTopLeft(d->r.topLeft());
    } else {
        if (d->atLeft())
            nr.moveBottomRight(d->r.bottomRight());
        else
            nr.moveBottomLeft(d->r.bottomLeft());
    }

    tlw->setGeometry(nr);
}

/*!
  \reimp
*/
void QSizeGrip::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton) {
        Q_D(QSizeGrip);
        d->gotMousePress = false;
        d->p = QPoint();
    } else {
        QWidget::mouseReleaseEvent(mouseEvent);
    }
}

/*!
  \reimp
*/
void QSizeGrip::moveEvent(QMoveEvent * /*moveEvent*/)
{
    Q_D(QSizeGrip);
    // We're inside a resize operation; no update necessary.
    if (!d->p.isNull())
        return;

    d->m_corner = d->corner();
#if !defined(QT_NO_CURSOR) && !defined(Q_WS_MAC)
    setCursor(d->m_corner == Qt::TopLeftCorner || d->m_corner == Qt::BottomRightCorner
              ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
#endif
}

/*!
  \reimp
*/
void QSizeGrip::showEvent(QShowEvent *showEvent)
{
#ifdef Q_WS_MAC
    d_func()->updateMacSizer(false);
#endif
    QWidget::showEvent(showEvent);
}

/*!
  \reimp
*/
void QSizeGrip::hideEvent(QHideEvent *hideEvent)
{
#ifdef Q_WS_MAC
    d_func()->updateMacSizer(true);
#endif
    QWidget::hideEvent(hideEvent);
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
    if (d->hiddenByUser || e->type() != QEvent::WindowStateChange)
        return QWidget::eventFilter(o, e);
    QWidget *tlw = qt_sizegrip_topLevelWidget(this);
    if (o != tlw)
        return QWidget::eventFilter(o, e);
    QWidget::setVisible((tlw->windowState() &
                             (Qt::WindowFullScreen
#ifndef Q_WS_MAC
                              | Qt::WindowMaximized
#endif
                              ))==0);
    return false;
}

bool QSizeGrip::event(QEvent *event)
{
    return QWidget::event(event);
}

#ifdef Q_WS_WIN
/*! \reimp */
bool QSizeGrip::winEvent( MSG *m, long *result )
{
    if (m->message != WM_LBUTTONDOWN)
        return QWidget::winEvent(m, result);

    // toplevel windows use the native size grip on Windows
    QWidget *w = qt_sizegrip_topLevelWidget(this);
    if (!w->isWindow())
        return QWidget::winEvent(m, result);

    Q_D(QSizeGrip);
    if (d->atBottom()) {
        PostMessage(w->winId(), WM_SYSCOMMAND, d->atLeft() ? SZ_SIZEBOTTOMLEFT
                                                           : SZ_SIZEBOTTOMRIGHT, 0);
    } else {
        PostMessage(w->winId(), WM_SYSCOMMAND, d->atLeft() ? SZ_SIZETOPLEFT
                                                           : SZ_SIZETOPRIGHT, 0);
    }
    return true;
}
#endif

#endif //QT_NO_SIZEGRIP
