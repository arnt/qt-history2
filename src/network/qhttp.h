/****************************************************************************
** $Id: //depot/qt/main/src/network/qhttp.h#15 $
**
** Definition of QHtpp and related classes.
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
#include "qserversocket.h"
#include "qnetworkprotocol.h"
#endif // QT_H

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

class QSocket;
class QTimerEvent;
class QTextStream;
class QIODevice;

class QHttpHeaderPrivate;
class QHttpReplyHeaderPrivate;
class QHttpRequestHeaderPrivate;
class QHttpClientPrivate;
class QHttpServerPrivate;
class QHttpConnectionPrivate;
class QHttpPrivate;


class Q_EXPORT QHttpHeader
{
public:
    QHttpHeader();
    QHttpHeader( const QHttpHeader& header );
    QHttpHeader( const QString& str );
    virtual ~QHttpHeader();

    QHttpHeader& operator=( const QHttpHeader& h );

    QString value( const QString& key ) const;
    QStringList keys() const;
    bool hasKey( const QString& key ) const;

    void setValue( const QString& key, const QString& value );
    void removeValue( const QString& key );

    uint contentLength() const;
    QString contentType() const;
    void setContentLength( int len );
    void setContentType( const QString& type );

    enum Connection { Close, KeepAlive };
    void setConnection( Connection );
    Connection connection() const;

    virtual QString toString() const;

    bool isValid() const;

    QTextStream& read( QTextStream& );
    QTextStream& write( QTextStream& ) const;

protected:
    virtual bool parseLine( const QString& line, int number );

    void parse( const QString& str );

private:
    QHttpHeaderPrivate *d;
    bool m_bValid;
};


class Q_EXPORT QHttpReplyHeader : public QHttpHeader
{
public:
    QHttpReplyHeader();
    QHttpReplyHeader( int code, const QString& text = QString::null, int version = 10 );
    QHttpReplyHeader( const QHttpReplyHeader& header );
    QHttpReplyHeader( const QString& str );

    void setReply( int code, const QString& text = QString::null, int version = 10 );
    int replyCode() const;
    QString replyText() const;
    int version() const;
    bool hasAutoContentLength() const;

    virtual QString toString() const;

protected:
    virtual bool parseLine( const QString& line, int number );

private:
    QHttpReplyHeaderPrivate *d;
    int m_code;
    QString m_text;
    int m_version;
};


class Q_EXPORT QHttpRequestHeader : public QHttpHeader
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
    QHttpRequestHeaderPrivate *d;
    QString m_method;
    QString m_path;
    int m_version;
};


class Q_EXPORT QHttpClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY( State state READ state )
    Q_ENUMS( State )
    //Q_PROPERTY( QIODevice* device READ device WRITE setDevice )

public:
    enum State { Closed, Connecting, Sending, Reading, Alive, Idle };

    QHttpClient( QObject* parent = 0, const char* name = 0 );
    ~QHttpClient();

    virtual bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const char* data, uint size );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const QByteArray& data );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const QCString& data );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, QIODevice* device );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header );

    void close();

    State state() const;
    void setDevice( QIODevice* );
    QIODevice* device() const;

signals:
    void reply( const QHttpReplyHeader& repl, const QByteArray& data );
    void reply( const QHttpReplyHeader& repl, const QIODevice* device );
    void replyChunk( const QHttpReplyHeader& repl, const QByteArray& data );
    void replyHeader( const QHttpReplyHeader& repl );
    void requestFailed();
    void finished();
    
protected:
    void timerEvent( QTimerEvent * );

private slots:
    void readyRead();
    void connected();
    void error( int );
    void closed();
    void bytesWritten( int );

private:
    void killIdleTimer();

    QHttpClientPrivate *d;

    QSocket* m_socket;
    QByteArray m_buffer;
    uint m_bytesRead;
    QHttpRequestHeader m_header;
    State m_state;
    bool m_readHeader;
    QHttpReplyHeader m_reply;

    int m_idleTimer;

    QIODevice* m_device;
    QIODevice* m_postDevice;
};


class Q_EXPORT QHttpServer : public QServerSocket
{
    Q_OBJECT

public:
    QHttpServer( int port = 0, QObject* parent = 0, const char* name = 0 );

private:
    QHttpServerPrivate *d;
};


class Q_EXPORT QHttpConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY( State state READ state )
    Q_ENUMS( State )

public:
    enum State { Created, Reading, Waiting, Writing, Alive, Closed };

    QHttpConnection( int socket, QObject* parent = 0, const char* name = 0 );
    ~QHttpConnection();

    State state() const;

    void reply( const QHttpReplyHeader& repl, const QCString& data );
    void reply( const QHttpReplyHeader& repl, const QByteArray& data );
    virtual void reply( const QHttpReplyHeader& repl, const char* data, uint size );

    void allowKeepAlive( bool );
    bool isKeepAliveAllowed() const;
    void setKeepAliveTimeout( int timeout );
    int keepAliveTimeout() const;

signals:
    void replyFinished();
    void replyFailed();

protected:
    virtual void request( const QHttpRequestHeader& header, const QByteArray& data ) = 0;
    virtual void error( int );
    void close();
    void timerEvent( QTimerEvent * );

private slots:
    void readyRead();
    void bytesWritten( int );
    void closed();
    void socketError( int );

private:
    QHttpConnectionPrivate *d;

    QSocket* m_socket;
    int m_bytesToWrite;
    int m_bytesToRead;
    QHttpRequestHeader m_header;
    QByteArray m_buffer;
    State m_state;
    bool m_readHeader;
    int m_killTimer;
    bool m_allowKeepAlive;
    int m_keepAliveTimeout;
};


class QHttp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QHttp();
    virtual ~QHttp();

    int supportedOperations() const;

protected:
    void operationGet( QNetworkOperation *op );
    void operationPut( QNetworkOperation *op );

//    QSocket *commandSocket;
//    bool connectionReady, passiveMode;

private:
#if 0
    bool checkConnection( QNetworkOperation *op );
#endif
//    void close();

protected slots:
    void reply( const QHttpReplyHeader & rep, const QByteArray & dataA );
    void requestFinished();
//    void hostFound();
//    void connected();
//    void closed();
//    void readyRead();

private:
    QHttpPrivate *d;
    QHttpClient *client;
    int bytesRead;
    enum Operation { NoOp, Get, Put };
    Operation operation;
};


QTextStream& operator>>( QTextStream&, QHttpRequestHeader& );
QTextStream& operator<<( QTextStream&, const QHttpRequestHeader& );

QTextStream& operator>>( QTextStream&, QHttpReplyHeader& );
QTextStream& operator<<( QTextStream&, const QHttpReplyHeader& );

#endif

#endif
