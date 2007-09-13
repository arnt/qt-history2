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

#ifndef QSYSTEMTRAYICON_H
#define QSYSTEMTRAYICON_H

#include <QtCore/qobject.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QtGui/qicon.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QSystemTrayIconPrivate;

class QMenu;
class QEvent;
class QWheelEvent;
class QMouseEvent;
class QPoint;

class Q_GUI_EXPORT QSystemTrayIcon : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible DESIGNABLE false)

public:
    QSystemTrayIcon(QObject *parent = 0);
    QSystemTrayIcon(const QIcon &icon, QObject *parent = 0);
    ~QSystemTrayIcon();

    enum ActivationReason {
        Unknown,
        Context,
        DoubleClick,
        Trigger,
        MiddleClick
    };

#ifndef QT_NO_MENU
    void setContextMenu(QMenu *menu);
    QMenu *contextMenu() const;
#endif

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString toolTip() const;
    void setToolTip(const QString &tip);

    static bool isSystemTrayAvailable();
    static bool supportsMessages();

    enum MessageIcon { NoIcon, Information, Warning, Critical };
    void showMessage(const QString &title, const QString &msg,
                     MessageIcon icon = Information, int msecs = 10000);

    QRect geometry() const;
    bool isVisible() const;

public Q_SLOTS:
    void setVisible(bool visible);
    inline void show() { setVisible(true); }
    inline void hide() { setVisible(false); }

Q_SIGNALS:
    void activated(QSystemTrayIcon::ActivationReason reason);
    void messageClicked();

protected:
    bool event(QEvent *event);

private:
    Q_DISABLE_COPY(QSystemTrayIcon)
    Q_DECLARE_PRIVATE(QSystemTrayIcon)

    friend class QSystemTrayIconSys;
    friend class QBalloonTip;
    friend void qtsystray_sendActivated(QSystemTrayIcon *, int);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_SYSTEMTRAYICON
#endif // QSYSTEMTRAYICON_H
