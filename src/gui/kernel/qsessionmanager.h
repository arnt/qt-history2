/****************************************************************************
**
** Definition of QSessionManager class.
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

#ifndef QSESSIONMANAGER_H
#define QSESSIONMANAGER_H

#ifndef QT_H
#include "qobject.h"
#include "qwindowdefs.h"
#include "qstring.h"
#include "qstringlist.h"
#endif // QT_H
#ifndef QT_NO_SESSIONMANAGER

class QSessionManagerPrivate;

class Q_GUI_EXPORT  QSessionManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSessionManager)
    QSessionManager(QApplication *app, QString &id, QString &key);
    ~QSessionManager();
public:
    QString sessionId() const;
    QString sessionKey() const;
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
    void* handle() const;
#endif

    bool allowsInteraction();
    bool allowsErrorInteraction();
    void release();

    void cancel();

    enum RestartHint {
        RestartIfRunning,
        RestartAnyway,
        RestartImmediately,
        RestartNever
    };
    void setRestartHint(RestartHint);
    RestartHint restartHint() const;

    void setRestartCommand(const QStringList&);
    QStringList restartCommand() const;
    void setDiscardCommand(const QStringList&);
    QStringList discardCommand() const;

    void setManagerProperty(const QString& name, const QString& value);
    void setManagerProperty(const QString& name, const QStringList& value);

    bool isPhase2() const;
    void requestPhase2();

private:
    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QBaseApplication;
};

#endif // QT_NO_SESSIONMANAGER
#endif // QSESSIONMANAGER_H
