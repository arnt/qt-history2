/****************************************************************************
**
** Definition of QSocketDevice class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSOCKETDEVICE_H
#define QSOCKETDEVICE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qhostaddress.h" // for int-to-QHostAddress conversion
#endif // QT_H

#if !defined(QT_MODULE_NETWORK) || defined(QT_LICENSE_PROFESSIONAL) || defined(QT_INTERNAL_NETWORK)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

#ifndef QT_NO_NETWORK
class QSocketDevicePrivate;

class QM_EXPORT_NETWORK QSocketDevice : public QIODevice
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

    bool open(int mode);
    void close();
    void flush();

    Offset size() const;
    Offset at() const;
    bool at(Offset);
    bool atEnd() const;

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

    Q_LONG bytesAvailable() const;
    Q_LONG waitForMore(int msecs, bool *timeout=0) const;
    Q_LONG readBlock(char *data, Q_ULONG maxlen);
    Q_LONG writeBlock(const char *data, Q_ULONG len);
    virtual Q_LONG writeBlock(const char *data, Q_ULONG len, const QHostAddress & host,
                              Q_UINT16 port);

    int getch();
    int putch(int);
    int ungetch(int);

    Q_UINT16 port() const;
    Q_UINT16 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

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

protected:
    void setError(Error err);

private:
#if defined(Q_DISABLE_COPY)
    QSocketDevice(const QSocketDevice &);
    QSocketDevice &operator=(const QSocketDevice &);
#endif
};

#endif // QT_NO_NETWORK
#endif // QSOCKETDEVICE_H
