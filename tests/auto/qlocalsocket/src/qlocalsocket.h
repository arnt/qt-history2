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

#ifndef QLOCALSOCKET_H
#define QLOCALSOCKET_H

#include <QtCore/qiodevice.h>

QT_BEGIN_HEADER

#ifndef QT_NO_LOCALSOCKET

class QLocalSocketPrivate;

class QLocalSocket : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QLocalSocket)

public:
    enum LocalSocketError
    {
        ConnectionRefusedError,
        RemoteClosedError,
        NotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,
        DatagramTooLargeError,
        ConnectionError,
        UnsupportedSocketOperationError,
        UnknownSocketError
    };

    enum LocalSocketState
    {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };

    QLocalSocket(QObject *parent = 0);
    ~QLocalSocket();

    void connectToName(const QString &name, OpenMode openMode = ReadWrite);
    QString peerName() const;
    void disconnectSocket();

    void abort();
    virtual qint64 bytesAvailable() const;
    virtual qint64 bytesToWrite() const;
    virtual bool canReadLine() const;
    virtual void close();
    void disconnectFromName();
    LocalSocketError error() const;
    bool flush();
    bool isValid() const;
    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);

    bool setSocketDescriptor(int socketDescriptor,
                             LocalSocketState socketState = ConnectedState, OpenMode openMode = ReadWrite);
    int socketDescriptor() const;

    LocalSocketState state() const;
    bool waitForBytesWritten(int msecs);
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);
    virtual bool waitForReadyRead(int msecs = 30000);

Q_SIGNALS:
    void connected();
    void disconnected();
    void error(QLocalSocket::LocalSocketError socketError);
    void stateChanged(QLocalSocket::LocalSocketState socketState);

protected:
    virtual qint64 readData(char*, qint64);
    virtual qint64 writeData(const char*, qint64);

private:
    Q_DISABLE_COPY(QLocalSocket)
#ifdef Q_OS_WIN
    Q_PRIVATE_SLOT(d_func(), void _q_notified());
#else
    Q_PRIVATE_SLOT(d_func(), void _q_stateChanged(QAbstractSocket::SocketState));
    Q_PRIVATE_SLOT(d_func(), void _q_connectToSocket());
#endif
};

#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QLocalSocket::LocalSocketError);
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QLocalSocket::LocalSocketState);
#endif

#endif // QT_NO_LOCALSOCKET

QT_END_HEADER

#endif // QLOCALSOCKET_H

