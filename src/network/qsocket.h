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

#ifndef QSOCKET_H
#define QSOCKET_H

#ifndef QT_H
#include "qobject.h"
#include "qiodevice.h"
#include "qioengine.h"
#include "qdns.h"
#include "qhostaddress.h" // int->QHostAddress conversion
#endif // QT_H

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

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

    QSocket(QObject *parent = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QSocket(QObject *parent, const char *name);
#endif
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

    virtual void connectToHost(const QString &host, Q_UINT16 port);

    QString peerName() const;

    virtual QIOEngine *ioEngine() const;

    Q_ULONG bytesAvailable() const; // ### QIODevice::Offset instead?
    Q_ULONG waitForMore(int msecs, bool *timeout = 0) const;
    Q_ULONG bytesToWrite() const;
    void clearPendingData();

#ifdef Q_NO_USING_KEYWORD
    inline Q_LONG readLine(char *data, Q_LONG maxlen) { return QIODevice::readLine(data, maxlen); }
#else
    using QIODevice::readLine;
#endif
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

    Q_DECLARE_PRIVATE(QSocket)
    Q_PRIVATE_SLOT(d, void tryConnecting(const QDnsHostInfo &))
    Q_PRIVATE_SLOT(d, void connectToNextAddress())

    QIODevicePrivate *d_ptr;
    friend class QSocketEngine;
};

#endif
