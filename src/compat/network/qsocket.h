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

#include "qobject.h"
#include "qiodevice.h"
#include "qdns.h"
#include "qhostaddress.h" // int->QHostAddress conversion

class QSocketPrivate;
class QSocketDevice;

class Q_COMPAT_EXPORT QSocket : public QObject, public QIODevice
{
    Q_OBJECT
public:
    enum Error {
        ErrConnectionRefused,
        ErrHostNotFound,
        ErrSocketRead
    };

    QSocket(QObject *parent = 0);
    QSocket(QObject *parent, const char *name);
    virtual ~QSocket();

    enum State {
        Idle,
        HostLookup,
        Connecting,
        Connected,
        Connection = Connected,
        Closing
    };
    State state() const;

    int socket() const;
    virtual void setSocket(int);

    QSocketDevice *socketDevice();
    virtual void setSocketDevice(QSocketDevice *);

    virtual void connectToHost(const QString &host, Q_UINT16 port);

    QString peerName() const;

    Q_ULONG bytesAvailable() const; // ### QIODevice::Offset instead?
    Q_ULONG waitForMore(int msecs, bool *timeout = 0) const;
    Q_ULONG bytesToWrite() const;
    void clearPendingData();

    virtual Q_LLONG readLine(char *data, Q_LLONG maxlen);
    bool canReadLine() const;
    virtual QByteArray readLine();

    Q_UINT16 port() const;
    Q_UINT16 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

    void setReadBufferSize(Q_ULONG);
    Q_ULONG readBufferSize() const;

    bool isOpen() const { return state() == Connected; }
    virtual bool atEnd() const;
    virtual void flush();
    virtual void close();
    virtual Q_LLONG size() const;
    virtual Q_LLONG at() const;
    virtual bool seek(Q_LLONG off);
    virtual Q_LLONG read(char *data, Q_LLONG maxlen);
    virtual Q_LLONG write(const char *data, Q_LLONG len);
    virtual int ungetch(int);

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
    Q_DISABLE_COPY(QSocket)
    Q_DECLARE_PRIVATE(QSocket)
    Q_PRIVATE_SLOT(d, void tryConnecting(const QDnsHostInfo &))
    Q_PRIVATE_SLOT(d, void connectToNextAddress())

    QIODevicePrivate *d_ptr;
};

#endif
