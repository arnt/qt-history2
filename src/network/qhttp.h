/****************************************************************************
** $Id$
**
** Definition of QHttp and related classes.
**
** Created : 970521
**
** Copyright (C) 1997-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QHTTP_H
#define QHTTP_H

#ifndef QT_H
#include "qobject.h"
#include "qnetworkprotocol.h"
#endif // QT_H

#if !defined( QT_MODULE_NETWORK ) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_NETWORK )
#define QM_EXPORT_HTTP
#else
#define QM_EXPORT_HTTP Q_EXPORT
#endif

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

class QSocket;
class QTimerEvent;
class QTextStream;
class QIODevice;

class QHttpPrivate;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QM_EXPORT_HTTP QMap<QString, QString>;
// MOC_SKIP_END
#endif

class QM_EXPORT_HTTP QHttpHeader
{
public:
    QHttpHeader();
    QHttpHeader( const QHttpHeader& header );
    QHttpHeader( const QString& str );
    virtual ~QHttpHeader();

    QHttpHeader& operator=( const QHttpHeader& h );

    QString value( const QString& key ) const;
    void setValue( const QString& key, const QString& value );
    void removeValue( const QString& key );

    QStringList keys() const;
    bool hasKey( const QString& key ) const;

    bool hasContentLength() const;
    uint contentLength() const;
    void setContentLength( int len );

    bool hasContentType() const;
    QString contentType() const;
    void setContentType( const QString& type );

    virtual QString toString() const;
    bool isValid() const;

    virtual int majorVersion() const = 0;
    virtual int minorVersion() const = 0;

protected:
    virtual bool parseLine( const QString& line, int number );
    bool parse( const QString& str );

private:
    QMap<QString,QString> values;
    bool valid;
};

class QM_EXPORT_HTTP QHttpResponseHeader : public QHttpHeader
{
private:
    QHttpResponseHeader( int code, const QString& text = QString::null, int majorVer = 1, int minorVer = 1 );
    QHttpResponseHeader( const QString& str );

    void setStatusLine( int code, const QString& text = QString::null, int majorVer = 1, int minorVer = 1 );

public:
    QHttpResponseHeader();
    QHttpResponseHeader( const QHttpResponseHeader& header );

    int statusCode() const;
    QString reasonPhrase() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine( const QString& line, int number );

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
    QHttpRequestHeader( const QString& method, const QString& path, int majorVer = 1, int minorVer = 1 );
    QHttpRequestHeader( const QHttpRequestHeader& header );
    QHttpRequestHeader( const QString& str );

    void setRequest( const QString& method, const QString& path, int majorVer = 1, int minorVer = 1 );

    QString method() const;
    QString path() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine( const QString& line, int number );

private:
    QString m;
    QString p;
    int majVer;
    int minVer;
};

class QM_EXPORT_HTTP QHttp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QHttp();
    QHttp( QObject* parent, const char* name = 0 ); // ### Qt 4.0: join the two constructors
    QHttp( const QString &hostname, Q_UINT16 port=80, QObject* parent=0, const char* name = 0 );
    virtual ~QHttp();

    int supportedOperations() const;

protected:
    void operationGet( QNetworkOperation *op );
    void operationPut( QNetworkOperation *op );

private slots:
    void clientReply( const QHttpResponseHeader &rep, const QByteArray & dataA );
    void clientFinishedSuccess();
    void clientFinishedError( const QString &detail, int );
    void clientConnected();
    void clientClosed();
    void clientHostFound();

private:
    QHttpPrivate *d;
    void *unused; // ### Qt 4.0: remove this (in for binary compatibility)
    int bytesRead;

    // new API
public:
    enum State { Unconnected, HostLookup, Connecting, Sending, Reading, Connected, Closing };
    enum Error {
	NoError,
	UnknownError,
	ConnectionRefused,
	HostNotFound,
	UnexpectedClose,
	InvalidResponseHeader,
	WrongContentLength
    };

    void setHost(const QString &hostname, Q_UINT16 port=80 );

    bool request( const QHttpRequestHeader& header, const char* data, uint size );
    bool request( const QHttpRequestHeader& header, const QByteArray& data );
    bool request( const QHttpRequestHeader& header, const QCString& data );
    bool request( const QHttpRequestHeader& header, QIODevice* device );
    bool request( const QHttpRequestHeader& header );

    void close();

    State state() const;
    void setDevice( QIODevice* );
    QIODevice* device() const;

signals:
    void response( const QHttpResponseHeader& resp, const QByteArray& data );
    void response( const QHttpResponseHeader& resp, const QIODevice* device );
    void responseChunk( const QHttpResponseHeader& resp, const QByteArray& data );
    void responseHeader( const QHttpResponseHeader& resp );

    void finishedError( const QString& detail, int error );
    void finishedSuccess();

    // informational
    void connected();
    void closed();
    void hostFound();

protected:
    void timerEvent( QTimerEvent * );

private slots:
    void slotReadyRead();
    void slotConnected();
    void slotError( int );
    void slotClosed();
    void slotBytesWritten( int );

private:
    void killIdleTimer();

    void init();
};

#endif
#endif
