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

#ifndef QSOCKETDEVICE_H
#define QSOCKETDEVICE_H

#include "qiodevice.h"
#include "qhostaddress.h" // for int-to-QHostAddress conversion

class QSocketDevicePrivate;

class Q_COMPAT_EXPORT QSocketDevice : public QIODevice
{
    Q_DECLARE_PRIVATE(QSocketDevice)
public:
    enum Type { Stream, Datagram };
    enum Protocol { IPv4, IPv6, Unknown };

    QSocketDevice(Type type = Stream);
    QSocketDevice(Type type, Protocol protocol, int dummy);
    QSocketDevice(int socket, Type type);
    virtual ~QSocketDevice();

    bool isValid() const;
    Type type() const;
    Protocol protocol() const;

    int socket() const;
    virtual void setSocket(int socket, Type type);

    bool blocking() const;
    virtual void setBlocking(bool);

    bool addressReusable() const;
    virtual void setAddressReusable(bool);

    int receiveBufferSize() const;
    virtual void setReceiveBufferSize(uint);
    int sendBufferSize() const;
    virtual void setSendBufferSize(uint);

    virtual bool connect(const QHostAddress &, Q_UINT16);

    virtual bool bind(const QHostAddress &, Q_UINT16);
    virtual bool listen(int backlog);
    virtual int accept();

#ifdef QT_COMPAT
#ifdef Q_NO_USING_KEYWORD
    inline QT_COMPAT Q_LONG writeBlock(const char *data, Q_LONG len)
    { return QIODevice::writeBlock(data, len); }
    inline QT_COMPAT Q_LONG writeBlock(const QByteArray &data)
    { return QIODevice::writeBlock(data); }
#else
    using QIODevice::writeBlock;
#endif
    inline QT_COMPAT Q_LONG writeBlock(const char *data, Q_LONG len,
                                        const QHostAddress & host, Q_UINT16 port)
    { return write(data, len, host, port); }
#endif
    virtual Q_LLONG write(const char *data, Q_LLONG len);
    virtual Q_LLONG write(const char *data, Q_LLONG len, const QHostAddress & host, Q_UINT16 port);

    Q_LONG bytesAvailable() const;
    Q_LONG waitForMore(int msecs, bool *timeout=0) const;
    Q_UINT16 port() const;
    Q_UINT16 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

    virtual int ungetch(int) { return -1; }
    virtual void close();
    virtual Q_LLONG size() const;
    virtual Q_LLONG at() const;
    virtual bool seek(Q_LLONG off);
    virtual Q_LLONG read(char *data, Q_LLONG maxlen);

    enum Error {
        NoError,
        AlreadyBound,
        Inaccessible,
        NoResources,
        InternalError,
#ifdef QT_COMPAT
        Bug = InternalError,
#endif
        Impossible,
        NoFiles,
        ConnectionRefused,
        NetworkFailure,
        UnknownError
    };
    Error error() const;

    void setError(Error err);

private:
    Q_DISABLE_COPY(QSocketDevice)

    friend class QSocketDeviceEngine;
};

class QSocketDeviceEnginePrivate;

#endif
