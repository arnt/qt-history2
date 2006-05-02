/****************************************************************************
 * **
 * ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include "qsystemtrayicon.h"
#include "qsystemtrayicon_p.h"

#include "qmenu.h"
#include "qevent.h"
#include "qpoint.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qpainterpath.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qgridlayout.h"
#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qbitmap.h"

/*!
    \class QSystemTrayIcon
    \brief The QSystemTrayIcon class provides a way to add an entry to the system tray.

    Modern operating systems usually provide a special area on the desktop,
    called \e{system tray} or \e{notification area}, where applications can
    display icons and short messages.

    \img system-tray.png The system tray on Windows XP

    The system tray is supported on the following platforms:

    \list
    \o All versions of Windows.
    \o On X11, all window managers that implement the freedesktop.org system
       tray specification, including recent versions of KDE and GNOME.
    \omit
    \o On Mac OS X, when the \c growl program is running.
    \endomit
    \endlist

    To check whether a system tray is present on the user's desktop,
    call the static function QSystemTrayIcon::isSystemTrayAvailable(). If the system tray 
    is currently unavailable but becomes available later, QSystemTrayIcon will automatically add an 
    entry in the system tray if it is visible().

    To add a system tray entry, create a QSystemTrayIcon object, call setContextMenu()
    to provide a context menu for the icon, and call show() to make it visible in the system
    tray. Status notification messages ("balloon messages") can be displayed at any time using
    showMessage().
    
    The activated() signal is emitted when the users clicks on the icon. (More precisely,
    on Windows, it is emitted when the left mouse button is released; on X11, when the
    left mouse button is pressed.) 
*/

/*!
    Creates a QSystemTrayIcon object. \a parent is propagated
    to the QObject constructor. The icon is initially invisible.

    \sa visible
*/
QSystemTrayIcon::QSystemTrayIcon(QObject *parent)
: QObject(*new QSystemTrayIconPrivate(), parent)
{
}

/*!
    Removes the icon from the system tray and frees all allocated resources.
*/
QSystemTrayIcon::~QSystemTrayIcon()
{
    Q_D(QSystemTrayIcon);
    d->remove();
}

/*!
    The menu will pop up when the user requests the context menu for the system tray entry
    by clicking the mouse button.
*/
void QSystemTrayIcon::setContextMenu(QMenu *menu)
{
    Q_D(QSystemTrayIcon);
    d->menu = menu;
}

/*!
    Returns the current context menu for the system tray entry.
*/
QMenu* QSystemTrayIcon::contextMenu() const
{
    Q_D(const QSystemTrayIcon);
    return d->menu;
}

/*!
    \property QSystemTrayIcon::icon
    \brief the system tray icon.

    On Windows, the system tray icon size is 16x16. On X11, the preferred size 22x22 and
    will be scaled as necessary.
*/
void QSystemTrayIcon::setIcon(const QIcon &icon)
{
    Q_D(QSystemTrayIcon);
    d->icon = icon;
    d->updateIcon();
}

QIcon QSystemTrayIcon::icon() const
{
    Q_D(const QSystemTrayIcon);
    return d->icon;
}

/*!
    \property QSystemTrayIcon::toolTip
    \brief the tooltip for the system tray entry
  
    On some systems, the tooltip's length is limited. The tooltip will be truncated if necessary.
*/
void QSystemTrayIcon::setToolTip(const QString &tooltip)
{
    Q_D(QSystemTrayIcon);
    d->toolTip = tooltip;
    d->updateToolTip();
}

QString QSystemTrayIcon::toolTip() const
{
    Q_D(const QSystemTrayIcon);
    return d->toolTip;
}

/*!
    \fn void QSystemTrayIcon::show()
    Shows the icon in the system tray.

    \sa hide(), visible
*/

/*!
    \fn void QSystemTrayIcon::hide()
    Hides the system tray entry.

    \sa show(), visible
*/


/*!
    \property QSystemTrayIcon::visible
    \brief whether the system tray entry is visible

    Calling setVisible(true) or show() makes the system tray entry visible. Calling 
    setVisible(false) or hide() hides the system tray entry.
*/
void QSystemTrayIcon::setVisible(bool visible)
{
    Q_D(QSystemTrayIcon);
    if (visible == d->visible)
        return;
    d->visible = visible;
    if (d->visible)
        d->install();
    else
        d->remove();
}

bool QSystemTrayIcon::isVisible() const
{
    Q_D(const QSystemTrayIcon);
    return d->visible;
}

/*!
  \reimp
*/
bool QSystemTrayIcon::event(QEvent *e)
{
    return QObject::event(e);
}

/*!
    \fn void QSystemTrayIcon::activated(const QPoint &globalPos)

    This signal is emitted when the user activates the system tray icon
    with \a globalPos being the global mouse position at that moment.
  
    The mouse button that activates a system tray entry is dependant on the platform
    On Window, this is a left button click. On X11, this is triggered when the
    left button is pressed.

    \sa clicked(), doubleClicked()
*/

/*!
    \fn void QSystemTrayIcon::clicked(const QPoint &globalPos, Qt::MouseButton button)

    This signal is emitted when the \a button is clicked (i.e pressed down then
    released while the mouse cursor is inside the icon) with \a globalPos being the 
    global mouse position at that moment.

    \sa activated(), doubleClicked(), messageClicked()
*/

/*!
    \fn void QSystemTrayIcon::doubleClicked(const QPoint &globalPos)

    This signal is emitted when the user double clicks the system tray
    icon with the left mouse button, with \a globalPos being the global mouse
    position at that moment.

    \sa clicked(), activated()
*/

/*!
    \fn void QSystemTrayIcon::messageClicked()

    This signal is emitted when the message displayed using showMessage()
    was clicked by the user.
*/


/*!
    Returns true if the system tray is available; otherwise returns
    false.

    If the system tray is currently unavailable but becomes available later,
    QSystemTrayIcon will automatically add an entry in the system tray if it is visible().
*/

bool QSystemTrayIcon::isSystemTrayAvailable()
{
    return QSystemTrayIconPrivate::isSystemTrayAvailable();
}

/*!
    Shows a balloon message for the entry with title \a title,
    message \a msg and icon \a icon for \a msecs milliseconds.

    The messageClicked() signal is emitted when the message was clicked
    by the user.

    Note that display of messages are system dependant and may
    not appear at all. Hence, it should not be relied upon as the
    sole means for providing critical information.

    \sa showMessage()
  */
void QSystemTrayIcon::showMessage(const QString& title, const QString& msg, 
                            QSystemTrayIcon::MessageIcon icon, int msecs)
{
    Q_D(QSystemTrayIcon);
    d->showMessage(title, msg, icon, msecs); 
}

//////////////////////////////////////////////////////////////////////
static QBalloonTip *theSolitaryBalloonTip = 0;

void QBalloonTip::showBalloon(QSystemTrayIcon::MessageIcon icon, const QString& title,
                              const QString& message, QSystemTrayIcon *trayIcon,
                              const QPoint& pos, int timeout, bool showArrow)
{
    hideBalloon();
    theSolitaryBalloonTip = new QBalloonTip(icon, title, message, trayIcon);
    if (timeout < 0)
        timeout = 10000; //10 sec default
    theSolitaryBalloonTip->balloon(pos, timeout, showArrow);
}

void QBalloonTip::hideBalloon()
{
    if (!theSolitaryBalloonTip) 
        return;
    theSolitaryBalloonTip->hide();
    delete theSolitaryBalloonTip;
    theSolitaryBalloonTip = 0;
}

QBalloonTip::QBalloonTip(QSystemTrayIcon::MessageIcon icon, const QString& title, 
                         const QString& message, QSystemTrayIcon *ti)
    : QWidget(0, Qt::ToolTip), trayIcon(ti), timerId(-1)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(ti, SIGNAL(destroyed()), this, SLOT(close()));

    QLabel *titleLabel = new QLabel;
    titleLabel->installEventFilter(this);
    titleLabel->setText(title);
    QFont f = titleLabel->font();
    f.setBold(true);
    titleLabel->setFont(f);
    titleLabel->setTextFormat(Qt::PlainText); // to maintain compat with windows

    QPushButton *closeButton = new QPushButton;
    closeButton->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
    closeButton->setIconSize(QSize(18, 18));
    closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    closeButton->setFixedSize(18, 18);
    QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QLabel *msgLabel = new QLabel;
    msgLabel->installEventFilter(this);
    msgLabel->setText(message);
    msgLabel->setTextFormat(Qt::PlainText);
    msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QStyle::StandardPixmap p;
    switch (icon) {
    case QSystemTrayIcon::Warning:
        p = QStyle::SP_MessageBoxWarning;
        break;
    case QSystemTrayIcon::Critical:
        p = QStyle::SP_MessageBoxCritical;
        break;
    case QSystemTrayIcon::Information:
        p = QStyle::SP_MessageBoxInformation;
        break;
    default:
        break;
    }
    QLabel *iconLabel = new QLabel;
    iconLabel->setPixmap(style()->standardIcon(p).pixmap(15, 15));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel->setMargin(2);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(iconLabel, 0, 0);
    layout->addWidget(titleLabel, 0, 1);
    layout->addWidget(closeButton, 0, 2);
    layout->addWidget(msgLabel, 1, 0, 1, 3);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setMargin(3);
    layout->setSpacing(1);
    setLayout(layout);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xe1));
    setPalette(pal);
}

QBalloonTip::~QBalloonTip()
{
    theSolitaryBalloonTip = 0;
}

void QBalloonTip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), pixmap);
}

void QBalloonTip::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void QBalloonTip::balloon(const QPoint& pos, int msecs, bool showArrow)
{
    QRect scr = QApplication::desktop()->screenGeometry();
    QSize sh = sizeHint();
    const int border = 1;
    const int ah = 18, ao = 18, aw = 18, rc = 7;
    bool arrowAtTop = (pos.y() + sh.height() + ah < scr.height());
    bool arrowAtLeft = (pos.x() + sh.width() - ao < scr.width());
    setContentsMargins(border + 3,  border + (arrowAtTop ? ah : 0) + 2, border + 3, border + (arrowAtTop ? 0 : ah) + 2);
    updateGeometry();
    sh  = sizeHint();

    int ml, mr, mt, mb;
    pixmap = QPixmap(sizeHint());
    pixmap.fill(Qt::white);
    if (!arrowAtTop) {
        ml = mt = 0;
        mr = pixmap.width() - 1;
        mb = pixmap.height() - ah - 1;
    } else {
        ml = 0;
        mt = ah;
        mr = pixmap.width() - 1;
        mb = pixmap.height() - 1;
    }

    QPainterPath path;
    path.moveTo(ml + rc, mt);
    if (arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(ml + ao, mt);
            path.lineTo(ml + ao, mt - ah);
            path.lineTo(ml + ao + aw, mt);
        }
        move(qMax(pos.x() - ao, scr.left() + 2), pos.y());
    } else if (arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(mr - ao - aw, mt);
            path.lineTo(mr - ao, mt - ah);
            path.lineTo(mr - ao, mt);
        }
        move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2), pos.y());
    }
    path.lineTo(mr - rc, mt);
    path.arcTo(QRect(mr - rc*2, mt, rc*2, rc*2), 90, -90);
    path.lineTo(mr, mb - rc);
    path.arcTo(QRect(mr - rc*2, mb - rc*2, rc*2, rc*2), 0, -90);
    if (!arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(mr - ao, mb);
            path.lineTo(mr - ao, mb + ah);
            path.lineTo(mr - ao - aw, mb);
        }
        move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2),
             pos.y() - sh.height());
    } else if (!arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(ao + aw, mb);
            path.lineTo(ao, mb + ah);
            path.lineTo(ao, mb);
        }
        move(qMax(pos.x() - ao, scr.x() + 2), pos.y() - sh.height());
    }
    path.lineTo(ml + rc, mb);
    path.arcTo(QRect(ml, mb - rc*2, rc*2, rc*2), -90, -90);
    path.lineTo(ml, mt + rc);
    path.arcTo(QRect(ml, mt, rc*2, rc*2), 180, -90);
    
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, border));
    painter.setBrush(palette().color(QPalette::Window));
    painter.drawPath(path);
    setMask(pixmap.createHeuristicMask());
    if (msecs > 0)
        timerId = startTimer(msecs);
    show();
}

void QBalloonTip::mousePressEvent(QMouseEvent *e)
{
    close();
    if(e->button() == Qt::LeftButton)
        emit trayIcon->messageClicked();
}

void QBalloonTip::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timerId) {
        killTimer(timerId);
        if (!underMouse())
            close();
        return;
    }
    QWidget::timerEvent(e);
}

bool QBalloonTip::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress)
        mousePressEvent(static_cast<QMouseEvent *>(e));
    return QWidget::eventFilter(o, e);
}

