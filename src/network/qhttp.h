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

    QTextStream& read( QTextStream& );
    QTextStream& write( QTextStream& ) const;

protected:
    virtual bool parseLine( const QString& line, int number );

    void parse( const QString& str );

private:
    QMap<QString,QString> m_values;
    bool m_bValid;
};

class QM_EXPORT_HTTP QHttpResponseHeader : public QHttpHeader
{
public:
    QHttpResponseHeader();
    QHttpResponseHeader( int code, const QString& text = QString::null, int version = 10 );
    QHttpResponseHeader( const QHttpResponseHeader& header );
    QHttpResponseHeader( const QString& str );

    void setStatusLine( int code, const QString& text = QString::null, int version = 10 );
    int statusCode() const;
    QString reasonPhrase() const;
    int version() const;

    virtual QString toString() const;

protected:
    virtual bool parseLine( const QString& line, int number );

private:
    int m_code;
    QString m_text;
    int m_version;
};

class QM_EXPORT_HTTP QHttpRequestHeader : public QHttpHeader
{
public:
    QHttpRequestHeader();
    QHttpRequestHeader( const QString& method, const QString& path, int version = 10 );
    QHttpRequestHeader( const QHttpRequestHeader& header );
    QHttpRequestHeader( const QString& str );

    void setRequest( const QString& method, const QString& path, int version = 10 );
    QString method() const;
    QString path() const;
    int version();

    virtual QString toString() const;

protected:
    virtual bool parseLine( const QString& line, int number );

private:
    QString m_method;
    QString m_path;
    int m_version;
};

class QM_EXPORT_HTTP QHttpClient : public QObject
{
    Q_OBJECT

public:
    enum State { Closed, Connecting, Sending, Reading, Alive, Idle };
    enum Error {
	UnknownError,
	ConnectionRefused,
	HostNotFound,
	UnexpectedClose,
	InvalidResponseHeader,
	WrongContentLength
    };

    QHttpClient( QObject* parent = 0, const char* name = 0 );
    ~QHttpClient();

    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const char* data, uint size );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const QByteArray& data );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const QCString& data );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, QIODevice* device );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header );

    void close();

    State state() const;
    void setDevice( QIODevice* );
    QIODevice* device() const;

signals:
    void response( const QHttpResponseHeader& repl, const QByteArray& data );
    void response( const QHttpResponseHeader& repl, const QIODevice* device );
    void responseChunk( const QHttpResponseHeader& repl, const QByteArray& data );
    void responseHeader( const QHttpResponseHeader& repl );

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

    QSocket* m_socket;
    QByteArray m_buffer;
    uint m_bytesRead;
    QHttpRequestHeader m_header;
    State m_state;
    bool m_readHeader;
    QHttpResponseHeader m_response;

    int m_idleTimer;

    QIODevice* m_device;
    QIODevice* m_postDevice;
};

class QM_EXPORT_HTTP QHttp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QHttp();
    virtual ~QHttp();

    int supportedOperations() const;

protected:
    void operationGet( QNetworkOperation *op );
    void operationPut( QNetworkOperation *op );

private slots:
    void reply( const QHttpResponseHeader & rep, const QByteArray & dataA );
    void finishedSuccess();
    void finishedError( const QString &detail, int );
    void connected();
    void closed();
    void hostFound();

private:
    QHttpPrivate *d;
    QHttpClient *client;
    int bytesRead;
};


#if 0
QM_EXPORT_HTTP QTextStream& operator>>( QTextStream&, QHttpRequestHeader& );
QM_EXPORT_HTTP QTextStream& operator<<( QTextStream&, const QHttpRequestHeader& );

QM_EXPORT_HTTP QTextStream& operator>>( QTextStream&, QHttpResponseHeader& );
QM_EXPORT_HTTP QTextStream& operator<<( QTextStream&, const QHttpResponseHeader& );
#endif

#endif
#endif
