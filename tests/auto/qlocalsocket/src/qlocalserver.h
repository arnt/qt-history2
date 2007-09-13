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

#ifndef QLOCALSERVER_H
#define QLOCALSERVER_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

#ifndef QT_NO_LOCALSERVER

QT_DECLARE_CLASS(QLocalSocket)
QT_DECLARE_CLASS(QLocalServerPrivate)

class QLocalServer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QLocalServer)

Q_SIGNALS:
    void newConnection();

public:
    enum LocalServerError
    {
        NoError,
        NameError,
        PermissionDeniedError,
        AddressInUseError,
        UnknownError
    };

    QLocalServer(QObject *parent = 0);
    ~QLocalServer();

    bool close();
    QString errorString() const;
    virtual bool hasPendingConnections() const;
    bool isListening() const;
    bool listen(const QString &name);
    int maxPendingConnections() const;
    virtual QLocalSocket *nextPendingConnection();
    QString serverName() const;
    LocalServerError serverError() const;
    void setMaxPendingConnections(int numConnections);
    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);

protected:
    virtual void incomingConnection(int socketDescriptor);

private:
    Q_DISABLE_COPY(QLocalServer)
#ifdef Q_OS_WIN
    Q_PRIVATE_SLOT(d_func(), void _q_openSocket(int handle))
    Q_PRIVATE_SLOT(d_func(), void _q_stoppedListening())
#else
    Q_PRIVATE_SLOT(d_func(), void _q_socketActivated())
#endif
};

#endif // QT_NO_LOCALSERVER

QT_END_HEADER

#endif // QLOCALSERVER_H

