/****************************************************************************
**
** Definition of QHttp and related classes.
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

#ifndef QHTTP_H
#define QHTTP_H

#ifndef QT_H
#include "qobject.h"
#include "qstringlist.h"
#include "qmap.h"
#endif // QT_H

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_HTTP
#define QM_TEMPLATE_EXTERN_HTTP
#else
#define QM_EXPORT_HTTP Q_NETWORK_EXPORT
#endif

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

class QSocket;
class QTimerEvent;
class QTextStream;
class QIODevice;

class QHttpPrivate;
class QHttpRequest;

class QM_EXPORT_HTTP QHttpHeader
{
public:
    QHttpHeader();
    QHttpHeader(const QHttpHeader& header);
    QHttpHeader(const QString& str);
    virtual ~QHttpHeader();

    QHttpHeader& operator=(const QHttpHeader& h);

    QString value(const QString& key) const;
    void setValue(const QString& key, const QString& value);
    void removeValue(const QString& key);

    QStringList keys() const;
    bool hasKey(const QString& key) const;

    bool hasContentLength() const;
    uint contentLength() const;
    void setContentLength(int len);

    bool hasContentType() const;
    QString contentType() const;
    void setContentType(const QString& type);

    virtual QString toString() const;
    bool isValid() const;

    virtual int majorVersion() const = 0;
    virtual int minorVersion() const = 0;

protected:
    virtual bool parseLine(const QString& line, int number);
    bool parse(const QString& str);
    void setValid(bool);

private:
    QMap<QString,QString> values;
    bool valid;
};

class QM_EXPORT_HTTP QHttpResponseHeader : public QHttpHeader
{
private:
    QHttpResponseHeader(int code, const QString& text = QString::null, int majorVer = 1, int minorVer = 1);
    QHttpResponseHeader(const QString& str);

    void setStatusLine(int code, const QString& text = QString::null, int majorVer = 1, int minorVer = 1);

public:
    QHttpResponseHeader();
    QHttpResponseHeader(const QHttpResponseHeader& header);

    int statusCode() const;
    QString reasonPhrase() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString& line, int number);

private:
    int statCode;
    QString reasonPhr;
    int majVer;
    int minVer;

    friend class QHttp;
};

class QM_EXPORT_HTTP QHttpRequestHeader : public QHttpHeader
{
public:
    QHttpRequestHeader();
    QHttpRequestHeader(const QString& method, const QString& path, int majorVer = 1, int minorVer = 1);
    QHttpRequestHeader(const QHttpRequestHeader& header);
    QHttpRequestHeader(const QString& str);

    void setRequest(const QString& method, const QString& path, int majorVer = 1, int minorVer = 1);

    QString method() const;
    QString path() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString& line, int number);

private:
    QString m;
    QString p;
    int majVer;
    int minVer;
};

class QM_EXPORT_HTTP QHttp : public QObject
{
    Q_OBJECT

public:
    QHttp(QObject* parent = 0, const char* name = 0);
    QHttp(const QString &hostname, Q_UINT16 port=80, QObject* parent=0, const char* name = 0);
    virtual ~QHttp();

    int supportedOperations() const;

    enum State { Unconnected, HostLookup, Connecting, Sending, Reading, Connected, Closing };
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

    int setHost(const QString &hostname, Q_UINT16 port=80);
    int setSocket(QSocket *socket);

    int get(const QString& path, QIODevice* to=0);
    int post(const QString& path, QIODevice* data, QIODevice* to=0 );
    int post(const QString& path, const QByteArray& data, QIODevice* to=0);
    int head(const QString& path);
    int request(const QHttpRequestHeader &header, QIODevice *device=0, QIODevice *to=0);
    int request(const QHttpRequestHeader &header, const QByteArray &data, QIODevice *to=0);

    int closeConnection();
    int close();

    Q_ULONG bytesAvailable() const;
    Q_LONG readBlock(char *data, Q_ULONG maxlen);
    QByteArray readAll();

    int currentId() const;
    QIODevice* currentSourceDevice() const;
    QIODevice* currentDestinationDevice() const;
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
    void responseHeaderReceived(const QHttpResponseHeader& resp);
    void readyRead(const QHttpResponseHeader& resp);
    void dataSendProgress(int, int);
    void dataReadProgress(int, int);

    void requestStarted(int);
    void requestFinished(int, bool);
    void done(bool);

protected:
    void timerEvent(QTimerEvent *);

private slots:
    void startNextRequest();
    void slotReadyRead();
    void slotConnected();
    void slotError(int);
    void slotClosed();
    void slotBytesWritten(int);

private:
    QHttpPrivate *d;

    int addRequest(QHttpRequest *);
    void sendRequest();
    void finishedWithSuccess();
    void finishedWithError(const QString& detail, int errorCode);

    void killIdleTimer();

    void init();
    void setState(int);
    void closeConn();
    void setSock(QSocket *socket);

    friend class QHttpNormalRequest;
    friend class QHttpSetHostRequest;
    friend class QHttpSetSocketRequest;
    friend class QHttpCloseRequest;
    friend class QHttpPGHRequest;
};

#endif
#endif
