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

#ifndef QHTTP_H
#define QHTTP_H

#include "QtCore/qobject.h"
#include "QtCore/qstringlist.h"
#include "QtCore/qmap.h"

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

class QTcpSocket;
class QTimerEvent;
class QIODevice;

class QHttpPrivate;

class QHttpHeaderPrivate;
class Q_NETWORK_EXPORT QHttpHeader
{
public:
    QHttpHeader();
    QHttpHeader(const QHttpHeader &header);
    QHttpHeader(const QString &str);
    virtual ~QHttpHeader();

    QHttpHeader &operator=(const QHttpHeader &h);

    QString value(const QString &key) const;
    void setValue(const QString &key, const QString &value);
    void removeValue(const QString &key);

    QStringList keys() const;
    bool hasKey(const QString &key) const;

    bool hasContentLength() const;
    uint contentLength() const;
    void setContentLength(int len);

    bool hasContentType() const;
    QString contentType() const;
    void setContentType(const QString &type);

    virtual QString toString() const;
    bool isValid() const;

    virtual int majorVersion() const = 0;
    virtual int minorVersion() const = 0;

protected:
    virtual bool parseLine(const QString &line, int number);
    bool parse(const QString &str);
    void setValid(bool);

    QHttpHeader(QHttpHeaderPrivate &dd, const QString &str = QString());
    QHttpHeader(QHttpHeaderPrivate &dd, const QHttpHeader &header);
    QHttpHeaderPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QHttpHeader)
};

class QHttpResponseHeaderPrivate;
class Q_NETWORK_EXPORT QHttpResponseHeader : public QHttpHeader
{
private:
    QHttpResponseHeader(int code, const QString &text = QString::null, int majorVer = 1, int minorVer = 1);
    QHttpResponseHeader(const QString &str);

    void setStatusLine(int code, const QString &text = QString::null, int majorVer = 1, int minorVer = 1);

public:
    QHttpResponseHeader();
    QHttpResponseHeader(const QHttpResponseHeader &header);
    QHttpResponseHeader &operator=(const QHttpResponseHeader &header);

    int statusCode() const;
    QString reasonPhrase() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString &line, int number);

private:
    Q_DECLARE_PRIVATE(QHttpResponseHeader)
    friend class QHttpPrivate;
};

class QHttpRequestHeaderPrivate;
class Q_NETWORK_EXPORT QHttpRequestHeader : public QHttpHeader
{
public:
    QHttpRequestHeader();
    QHttpRequestHeader(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);
    QHttpRequestHeader(const QHttpRequestHeader &header);
    QHttpRequestHeader(const QString &str);
    QHttpRequestHeader &operator=(const QHttpRequestHeader &header);

    void setRequest(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);

    QString method() const;
    QString path() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString &line, int number);

private:
    Q_DECLARE_PRIVATE(QHttpRequestHeader)
};

class Q_NETWORK_EXPORT QHttp : public QObject
{
    Q_OBJECT

public:
    explicit QHttp(QObject *parent = 0);
    QHttp(const QString &hostname, Q_UINT16 port = 80, QObject *parent = 0);
    virtual ~QHttp();

    int supportedOperations() const;

    enum State {
        Unconnected,
        HostLookup,
        Connecting,
        Sending,
        Reading,
        Connected,
        Closing
    };
    enum Error {
        NoError,
        UnknownError,
        HostNotFound,
        ConnectionRefused,
        UnexpectedClose,
        InvalidResponseHeader,
        WrongContentLength,
        Aborted
    };

    int setHost(const QString &hostname, Q_UINT16 port = 80);
    int setSocket(QTcpSocket *socket);
    int setUser(const QString &username, const QString &password = QString());

    int setProxy(const QString &host, int port,
                 const QString &username = QString(),
                 const QString &password = QString());

    int get(const QString &path, QIODevice *to=0);
    int post(const QString &path, QIODevice *data, QIODevice *to=0 );
    int post(const QString &path, const QByteArray &data, QIODevice *to=0);
    int head(const QString &path);
    int request(const QHttpRequestHeader &header, QIODevice *device=0, QIODevice *to=0);
    int request(const QHttpRequestHeader &header, const QByteArray &data, QIODevice *to=0);

    int closeConnection();
    int close();

    Q_LONGLONG bytesAvailable() const;
#ifdef QT_COMPAT
    inline QT_COMPAT Q_LONG readBlock(char *data, Q_ULONG maxlen)
    { return Q_LONG(read(data, Q_LONGLONG(maxlen))); }
#endif
    Q_LONGLONG read(char *data, Q_LONGLONG maxlen);
    QByteArray readAll();

    int currentId() const;
    QIODevice *currentSourceDevice() const;
    QIODevice *currentDestinationDevice() const;
    QHttpRequestHeader currentRequest() const;
    bool hasPendingRequests() const;
    void clearPendingRequests();

    State state() const;

    Error error() const;
    QString errorString() const;

public slots:
    void abort();

signals:
    void stateChanged(int);
    void responseHeaderReceived(const QHttpResponseHeader &resp);
    void readyRead(const QHttpResponseHeader &resp);
    void dataSendProgress(int, int);
    void dataReadProgress(int, int);

    void requestStarted(int);
    void requestFinished(int, bool);
    void done(bool);

protected:
    void timerEvent(QTimerEvent *);

private:
    Q_DISABLE_COPY(QHttp)
    Q_DECLARE_PRIVATE(QHttp)

    Q_PRIVATE_SLOT(d, void startNextRequest())
    Q_PRIVATE_SLOT(d, void slotReadyRead())
    Q_PRIVATE_SLOT(d, void slotConnected())
    Q_PRIVATE_SLOT(d, void slotError(QTcpSocket::SocketError))
    Q_PRIVATE_SLOT(d, void slotClosed())
    Q_PRIVATE_SLOT(d, void slotBytesWritten(Q_LONGLONG numBytes))

    friend class QHttpNormalRequest;
    friend class QHttpSetHostRequest;
    friend class QHttpSetSocketRequest;
    friend class QHttpSetUserRequest;
    friend class QHttpSetProxyRequest;
    friend class QHttpCloseRequest;
    friend class QHttpPGHRequest;
};

#endif

#endif // QHTTP_H
