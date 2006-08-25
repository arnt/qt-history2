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

#ifndef QSYSTEMTRAYICON_P_H
#define QSYSTEMTRAYICON_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qsystemtrayicon.h"
#include "private/qobject_p.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include "QtGui/qmenu.h"
#include "QtGui/qpixmap.h"
#include "QtCore/qstring.h"
#include "QtCore/qpointer.h"

class QSystemTrayIconSys;
class QToolButton;
class QLabel;

class QSystemTrayIconPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSystemTrayIcon)

public:
    QSystemTrayIconPrivate() : sys(0), visible(false) { }

    void install_sys();
    void remove_sys();
    void updateIcon_sys();
    void updateToolTip_sys();
    void updateMenu_sys();
    QPoint globalPos_sys() const;
    void showMessage_sys(const QString &msg, const QString &title, QSystemTrayIcon::MessageIcon icon, int secs);
    static bool isSystemTrayAvailable_sys();

    QPointer<QMenu> menu;
    QIcon icon;
    QString toolTip;
    QSystemTrayIconSys *sys;
    bool visible;
};

class QBalloonTip : public QWidget
{
public:
    static void showBalloon(QSystemTrayIcon::MessageIcon icon, const QString& title,
                            const QString& msg, QSystemTrayIcon *trayIcon,
                            const QPoint& pos, int timeout, bool showArrow = true);
    static void hideBalloon();

private:
    QBalloonTip(QSystemTrayIcon::MessageIcon icon, const QString& title,
                const QString& msg, QSystemTrayIcon *trayIcon);
    ~QBalloonTip();
    void balloon(const QPoint&, int, bool);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);

private:
    QSystemTrayIcon *trayIcon;
    QPixmap pixmap;
    int timerId;
};

#if defined(Q_WS_X11)
#include <QtCore/qcoreapplication.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

class QSystemTrayIconSys : public QWidget
{
    friend class QSystemTrayIconPrivate;

public:
    QSystemTrayIconSys(QSystemTrayIcon *q);
    ~QSystemTrayIconSys();
    enum {
        SYSTEM_TRAY_REQUEST_DOCK = 0,
        SYSTEM_TRAY_BEGIN_MESSAGE = 1,
        SYSTEM_TRAY_CANCEL_MESSAGE =2
    };

    void addToTray();
    void updateIcon();

    // QObject::event is public but QWidget's ::event() re-implementation
    // is protected ;(
    inline bool deliverToolTipEvent(QEvent *e)
    { return QWidget::event(e); }

    static Window sysTrayWindow;
    static QList<QSystemTrayIconSys *> trayIcons;
    static QCoreApplication::EventFilter oldEventFilter;
    static bool sysTrayTracker(void *message, long *result);
    static Window locateSystemTray();
    static Atom sysTraySelection;

protected:
    void paintEvent(QPaintEvent *pe);
    void resizeEvent(QResizeEvent *re);
    bool x11Event(XEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    bool event(QEvent *e);

private:
    QSystemTrayIcon *q;
    QPixmap cachedPixmap;
};
#endif // Q_WS_X11

#endif // QT_NO_SYSTEMTRAYICON
#endif // QSYSTEMTRAYICON_P_H

