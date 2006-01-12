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

#ifndef QSOCKS5SOCKETENGINE_P_H
#define QSOCKS5SOCKETENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qabstractsocketengine_p.h"
#include "qnetworkproxy.h"

#ifndef QT_NO_SOCKS5

class QSocks5SocketEnginePrivate;

class Q_INTERNAL_EXPORT QSocks5SocketEngine : public QAbstractSocketEngine
{
    Q_OBJECT
public:
    QSocks5SocketEngine(QObject *parent = 0);
    ~QSocks5SocketEngine();

    bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    bool initialize(int socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState);

    void setProxy(const QNetworkProxy &networkProxy);

    int socketDescriptor() const;

    bool isValid() const;

    bool connectToHost(const QHostAddress &address, quint16 port);
    bool bind(const QHostAddress &address, quint16 port);
    bool listen();
    int accept();
    void close();

    qint64 bytesAvailable() const;

    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);

#ifndef QT_NO_UDPSOCKET
    qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0,
        quint16 *port = 0);
    qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr,
        quint16 port);
    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
#endif // QT_NO_UDPSOCKET

    int option(SocketOption option) const;
    bool setOption(SocketOption option, int value);

    bool waitForRead(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForWrite(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
                            bool checkRead, bool checkWrite,
                            int msecs = 30000, bool *timedOut = 0) const;

    bool isReadNotificationEnabled() const;
    void setReadNotificationEnabled(bool enable);
    bool isWriteNotificationEnabled() const;
    void setWriteNotificationEnabled(bool enable);
    bool isExceptionNotificationEnabled() const;
    void setExceptionNotificationEnabled(bool enable);

private:
    Q_DECLARE_PRIVATE(QSocks5SocketEngine)
    Q_DISABLE_COPY(QSocks5SocketEngine)
    Q_PRIVATE_SLOT(d_func(), void controlSocketConnected())
    Q_PRIVATE_SLOT(d_func(), void controlSocketReadNotification())
    Q_PRIVATE_SLOT(d_func(), void controlSocketError(QAbstractSocket::SocketError))
#ifndef QT_NO_UDPSOCKET
    Q_PRIVATE_SLOT(d_func(), void udpSocketReadNotification())
#endif
    Q_PRIVATE_SLOT(d_func(), void controlSocketBytesWritten())
    Q_PRIVATE_SLOT(d_func(), void emitPendingReadNotification())
    Q_PRIVATE_SLOT(d_func(), void emitPendingWriteNotification())
    Q_PRIVATE_SLOT(d_func(), void controlSocketDisconnected())
    Q_PRIVATE_SLOT(d_func(), void controlSocketStateChanged(QAbstractSocket::SocketState))

};


class QTcpSocket;

class QSocks5Authenticator
{
public:
    QSocks5Authenticator();
    virtual ~QSocks5Authenticator();
    virtual char methodId();
    virtual bool beginAuthenticate(QTcpSocket *socket, bool *completed);
    virtual bool continueAuthenticate(QTcpSocket *socket, bool *completed);

    virtual bool seal(const QByteArray buf, QByteArray *sealedBuf);
    virtual bool unSeal(const QByteArray sealedBuf, QByteArray *buf);
    virtual bool unSeal(QTcpSocket *sealedSocket, QByteArray *buf);

    virtual QString errorString() { return QString(); }
};

class QSocks5PasswordAuthenticator : public QSocks5Authenticator
{
public:
    QSocks5PasswordAuthenticator(const QString &userName, const QString &password);
    char methodId();
    bool beginAuthenticate(QTcpSocket *socket, bool *completed);
    bool continueAuthenticate(QTcpSocket *socket, bool *completed);

    QString errorString();

private:
    QString userName;
    QString password;
};

struct QSocks5Data;
struct QSocks5ConnectData;
struct QSocks5UdpAssociateData;
struct QSocks5BindData;

class QSocks5SocketEnginePrivate : public QAbstractSocketEnginePrivate
{
    Q_DECLARE_PUBLIC(QSocks5SocketEngine)
public:
    QSocks5SocketEnginePrivate();
    ~QSocks5SocketEnginePrivate();

   enum Socks5State
    {
        unInitialized = 0,
        ConnectError,
        AuthenticationMethodsSent,
        Authenticating,
        AuthenticatingError,
        RequestMethodSent,
        RequestSuccess,
        RequestError,
        Connected,
        BindSuccess,
        BindError,
        ControlSocketError,
        SocksError
    };
    Socks5State socks5State;

    enum Socks5Mode
    {
        NoMode,
        ConnectMode,
        BindMode,
        UdpAssociateMode
    };
    Socks5Mode mode;

    enum Socks5Error
    {
        SocksFailure = 0x01,
        ConnectionNotAllowed = 0x02,
        NetworkUnreachable = 0x03,
        HostUnreachable = 0x04,
        ConnectionRefused = 0x05,
        TTLExpired = 0x06,
        CommandNotSupported = 0x07,
        AddressTypeNotSupported = 0x08,
        LastKnownError = AddressTypeNotSupported,
        UnknownError
    };
    Socks5Error socks5Error;
    QString socks5ErrorString;

    void initialize(Socks5Mode socks5Mode);

    void parseAuthenticationMethodReply();
    void parseAuthenticatingReply();
    void sendRequestMethod();
    void parseRequestMethodReply();
    void parseNewConnection();

    void controlSocketConnected();
    void controlSocketReadNotification();
    void controlSocketError(QAbstractSocket::SocketError);
#ifndef QT_NO_UDPSOCKET
    void checkForDatagrams() const;
    void udpSocketReadNotification();
#endif
    void controlSocketBytesWritten();
    void controlSocketDisconnected();
    void controlSocketStateChanged(QAbstractSocket::SocketState);

    QNetworkProxy proxyInfo;

    bool readNotificationEnabled, writeNotificationEnabled, exceptNotificationEnabled;

    int socketDescriptor;

    QSocks5Data *data;
    QSocks5ConnectData *connectData;
#ifndef QT_NO_UDPSOCKET
    QSocks5UdpAssociateData *udpData;
#endif
    QSocks5BindData *bindData;

    mutable bool readNotificationActivated;
    mutable bool writeNotificationActivated;

    bool readNotificationPending;
    void emitPendingReadNotification();
    void emitReadNotification();
    bool writeNotificationPending;
    void emitPendingWriteNotification();
    void emitWriteNotification();
};

class Q_INTERNAL_EXPORT QSocks5SocketEngineHandler : public QSocketEngineHandler
{
public:
    virtual QAbstractSocketEngine *createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent);
    virtual QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent);
};

#endif // QT_NO_SOCKS5
#endif // QSOCKS5SOCKETENGINE_H
