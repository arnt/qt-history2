/****************************************************************************
**
** Definition of QSocket class.
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

#ifndef QSOCKET_H
#define QSOCKET_H

#ifndef QT_H
#include "qobject.h"
#include "qiodevice.h"
#include "qresolver.h"
#include "qhostaddress.h" // int->QHostAddress conversion
#endif // QT_H

#if !defined(QT_MODULE_NETWORK) || defined(QT_LICENSE_PROFESSIONAL) || defined(QT_INTERNAL_NETWORK)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

#ifndef QT_NO_NETWORK
class QSocketPrivate;
class QSocketDevice;

class QM_EXPORT_NETWORK QSocket : public QObject, public QIODevice
{
    Q_OBJECT
public:
    enum Error {
        ErrConnectionRefused,
        ErrHostNotFound,
        ErrSocketRead
    };

    QSocket(QObject *parent=0, const char *name=0);
    virtual ~QSocket();

    enum State {
        Idle,
        HostLookup,
        Connecting,
        Connected,
#ifdef QT_COMPAT
        Connection = Connected,
#endif
        Closing
    };
    State state() const;

    int socket() const;
    virtual void setSocket(int);

    QSocketDevice *socketDevice();
    virtual void setSocketDevice(QSocketDevice *);

#ifndef QT_NO_DNS
    virtual void connectToHost(const QString &host, Q_UINT16 port);
#endif
    QString peerName() const;

    // Implementation of QIODevice abstract virtual functions
    bool open(int mode);
    void close();
    void flush();
    Offset size() const;
    Offset at() const;
    bool at(Offset);
    bool atEnd() const;

    Q_ULONG bytesAvailable() const; // ### QIODevice::Offset instead?
    Q_ULONG waitForMore(int msecs, bool *timeout = 0) const;
    Q_ULONG bytesToWrite() const;
    void clearPendingData();

    Q_LONG readBlock(char *data, Q_ULONG maxlen);
    Q_LONG writeBlock(const char *data, Q_ULONG len);
    Q_LONG readLine(char *data, Q_ULONG maxlen);

    int getch();
    int putch(int);
    int ungetch(int);

    bool canReadLine() const;
    virtual QString readLine();

    Q_UINT16 port() const;
    Q_UINT16 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

    void setReadBufferSize(Q_ULONG);
    Q_ULONG readBufferSize() const;

signals:
    void hostFound();
    void connected();
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void bytesWritten(int nbytes);
    void error(int);

protected slots:
    virtual void sn_read(bool force = false);
    virtual void sn_write();

private:
#if defined(Q_DISABLE_COPY)
    QSocket(const QSocket &);
    QSocket &operator=(const QSocket &);
#endif

    Q_DECLARE_PRIVATE(QSocket);
    Q_PRIVATE_SLOT(void tryConnecting(const QResolverHostInfo &))
    Q_PRIVATE_SLOT(void emitErrorConnectionRefused())

    QIODevicePrivate *d_ptr;
};

#endif //QT_NO_NETWORK
#endif // QSOCKET_H
