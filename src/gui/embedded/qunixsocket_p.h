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

#ifndef _QUNIXSOCKET_H_
#define _QUNIXSOCKET_H_

#include <QtNetwork/qabstractsocket.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

extern "C" {
#include <sys/types.h>
};

class QUnixSocketRights;
class QUnixSocketRightsPrivate;
class QUnixSocketPrivate;
class QUnixSocketMessagePrivate;

class QUnixSocketRights {
public:
    QUnixSocketRights(int);
    ~QUnixSocketRights();

    QUnixSocketRights(const QUnixSocketRights &);
    QUnixSocketRights & operator=(const QUnixSocketRights &);

    bool isValid() const;

    int dupFd() const;
    int peekFd() const;

private:
    friend class QUnixSocket;
    QUnixSocketRights(int,int);
    QSharedDataPointer<QUnixSocketRightsPrivate> d;
};

class QUnixSocketMessage {
public:
    QUnixSocketMessage();
    QUnixSocketMessage(const QByteArray &);
    QUnixSocketMessage(const QByteArray &, const QList<QUnixSocketRights> &);
    QUnixSocketMessage(const QUnixSocketMessage &);
    QUnixSocketMessage & operator=(const QUnixSocketMessage &);
    ~QUnixSocketMessage();

    void setBytes(const QByteArray &);
    void setRights(const QList<QUnixSocketRights> &);

    const QList<QUnixSocketRights> & rights() const;
    bool rightsWereTruncated() const;

    const QByteArray & bytes() const;

    pid_t processId() const;
    uid_t userId() const;
    gid_t groupId() const;

    void setProcessId(pid_t);
    void setUserId(uid_t);
    void setGroupId(gid_t);

    bool isValid() const;
private:
    friend class QUnixSocket;
    friend class QUnixSocketPrivate;
    QSharedDataPointer<QUnixSocketMessagePrivate> d;
};

class QUnixSocket : public QIODevice
{
    Q_OBJECT
public:
    QUnixSocket(QObject * = 0);
    QUnixSocket(qint64, qint64, QObject * = 0);
    virtual ~QUnixSocket();

    enum SocketState {
        UnconnectedState = QAbstractSocket::UnconnectedState,
        HostLookupState = QAbstractSocket::HostLookupState,
        ConnectingState = QAbstractSocket::ConnectingState,
        ConnectedState = QAbstractSocket::ConnectedState,
        BoundState = QAbstractSocket::BoundState,
        ClosingState = QAbstractSocket::ClosingState,
        ListeningState = QAbstractSocket::ListeningState,
    };

    enum SocketError { NoError, InvalidPath, ResourceError,
                       NonexistentPath, ConnectionRefused, UnknownError,
                       ReadFailure, WriteFailure };

    bool connect(const QByteArray & path);
    bool setSocketDescriptor(int socketDescriptor);
    int socketDescriptor() const;
    void abort();
    void close();

    void flush();

    SocketError error() const;

    SocketState state() const;
    QByteArray address() const;

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;

    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);
    qint64 rightsBufferSize() const;
    void setRightsBufferSize(qint64 size);

    qint64 write(const char * data, qint64 maxSize)
    { return QIODevice::write(data, maxSize); }
    qint64 write(const QByteArray & byteArray)
    { return QIODevice::write(byteArray); }
    qint64 read(char * data, qint64 maxSize)
    { return QIODevice::read(data, maxSize); }
    QByteArray read(qint64 maxSize)
    { return QIODevice::read(maxSize); }

    qint64 write(const QUnixSocketMessage &);
    QUnixSocketMessage read();

    virtual bool isSequential() const;
    virtual bool waitForReadyRead(int msec = 300);
    virtual bool waitForBytesWritten(int msec = 300);

signals:
    void stateChanged(SocketState socketState);

protected:
    virtual qint64 readData(char * data, qint64 maxSize);
    virtual qint64 writeData (const char * data, qint64 maxSize);

private:
    QUnixSocket(const QUnixSocket &);
    QUnixSocket & operator=(const QUnixSocket &);

    QUnixSocketPrivate * d;
};

#endif // _QUNIXSOCKET_H_

