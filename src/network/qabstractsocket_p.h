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

#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtNetwork/qabstractsocket.h"
#include "QtCore/qbytearray.h"
#include "QtCore/qlist.h"
#include "QtCore/qtimer.h"
#include "private/qringbuffer_p.h"
#include "private/qiodevice_p.h"
#include "private/qnativesocketengine_p.h"
#include "qnetworkproxy.h"

QT_BEGIN_NAMESPACE

class QHostInfo;

class QAbstractSocketPrivate : public QIODevicePrivate, public QAbstractSocketEngineReceiver
{
    Q_DECLARE_PUBLIC(QAbstractSocket)
public:
    QAbstractSocketPrivate();
    virtual ~QAbstractSocketPrivate();

    // from QAbstractSocketEngineReceiver
    inline void readNotification() { canReadNotification(); }
    inline void writeNotification() { canWriteNotification(); }
    inline void exceptionNotification() {}
    inline void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) {
        Q_Q(QAbstractSocket);
        q->proxyAuthenticationRequired(proxy, authenticator);
    }

    bool canReadNotification();
    bool canWriteNotification();

    // slots
    void _q_connectToNextAddress();
    void _q_startConnecting(const QHostInfo &hostInfo);
    void _q_testConnection();
    void _q_abortConnectionAttempt();

    bool readSocketNotifierCalled;
    bool readSocketNotifierState;
    bool readSocketNotifierStateSet;

    bool emittedReadyRead;
    bool emittedBytesWritten;

    bool closeCalled;

    QString hostName;
    quint16 port;
    QHostAddress host;
    QList<QHostAddress> addresses;

    quint16 localPort;
    quint16 peerPort;
    QHostAddress localAddress;
    QHostAddress peerAddress;
    QString peerName;

    QAbstractSocketEngine *socketEngine;
    int cachedSocketDescriptor;

    void resetSocketLayer();
    bool flush();

    bool initSocketLayer(const QHostAddress &host, QAbstractSocket::SocketType socketType);
    void fetchConnectionParameters();
    void setupSocketNotifiers();
    bool readFromSocket();

#ifdef Q_OS_LINUX
    qint64 addToBytesAvailable;
#endif
    qint64 readBufferMaxSize;
    QRingBuffer readBuffer;
    QRingBuffer writeBuffer;

    bool isBuffered;
    int blockingTimeout;

    QTimer *connectTimer;
    int connectTimeElapsed;

    int hostLookupId;

    QAbstractSocket::SocketType socketType;
    QAbstractSocket::SocketState state;

    QAbstractSocket::SocketError socketError;

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy *proxy;
#endif
};

QT_END_NAMESPACE

#endif // QABSTRACTSOCKET_P_H
