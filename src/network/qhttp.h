#ifndef VV_HTTP_H
#define VV_HTTP_H

#ifndef QT_H
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qcstring.h>
#include <qbuffer.h>
#include <qserversocket.h>
#endif // QT_H

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

class QSocket;
class QTimerEvent;
class QTextStream;
class QIODevice;

class QHttpHeaderPrivate;

class Q_EXPORT QHttpHeader
{
public:
    QHttpHeader();
    QHttpHeader( const QHttpHeader& header );
    QHttpHeader( const QString& str );
    virtual ~QHttpHeader();

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

    /**
     * @return TRUE if the server did not specify a size of the reply data.
     *              This is only possible if the server set the connection mode
     *              to Close.
     */
    bool hasAutoContentLength() const;

    virtual QString toString() const;

protected:
    virtual bool parseLine( const QString& line, int number );

private:
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
    QString m_method;
    QString m_path;
    int m_version;
};

class Q_EXPORT QHttpClient : public QObject
{
    Q_OBJECT
public:
    /**
     * Closed: The connection was just closed, but still one can not make new
     *         requests using this client. Better wait for Idle.
     * Connecting: A request was issued and the client is looking up ip addresses
     *             or connecting to the remote host.
     * Sending: The client is sending its request to the server.
     * Reading: The client has sent its request and is reading the servers
     *          response.
     * Alive: The connection to the host is open. It is possible to make new requests.
     * Idle: There is no open connection. It is possible to make new requests.
     */
    enum State { Closed, Connecting, Sending, Reading, Alive, Idle };

    QHttpClient( QObject* parent = 0, const char* name = 0 );
    ~QHttpClient();

    /**
     * Call this method to make a POST request.
     *
     * Call this method after the client was created or after the @ref #idle signal
     * was emitted.
     *
     * On other occasions the function returns FALSE to indicate that it can
     * not issue an request currently.
     */
    virtual bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const char* data, uint size );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const QByteArray& data );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, const QCString& data );
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header, QIODevice* device );
    /**
     * Call this method to make a GET request.
     */
    bool request( const QString& hostname, int port, const QHttpRequestHeader& header );

    /**
     * Force a close of the conection. This will abort a running request.
     *
     * Do not call @ref #request in
     * response to this signal. Instead wait for @ref #idle.
     */
    void close();

    State state() const;

    /**
     * If a device was set, then all data read by the QHttpClient - but not the
     * HTTP headers - are written to this device.
     *
     * The device must be opened for writing.
     *
     * Setting the device to 0 means that subsequently read data will be
     * read into memory. By default QHttpClient reads into memory.
     *
     * Setting a device makes sense when downloading very big chunks of data.
     */
    void setDevice( QIODevice* );
    QIODevice* device();

signals:
    void reply( const QHttpReplyHeader& repl, const QByteArray& data );
    void reply( const QHttpReplyHeader& repl, const QIODevice* device );
    void replyHeader( const QHttpReplyHeader& repl );
    void requestFailed();
    void idle();
    
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

/**
 * A class that accepts connections.
 *
 * You must overload the void newConnection( int ) method.
 * Usually your implementation will create a derived object
 * of @ref #QHttpConnection to handle the connection.
 */
class Q_EXPORT QHttpServer : public QServerSocket
{
    Q_OBJECT
public:
    QHttpServer( int port = 0, QObject* parent = 0, const char* name = 0 );
};

class Q_EXPORT QHttpConnection : public QObject
{
    Q_OBJECT
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

QTextStream& operator>>( QTextStream&, QHttpRequestHeader& );
QTextStream& operator<<( QTextStream&, const QHttpRequestHeader& );

QTextStream& operator>>( QTextStream&, QHttpReplyHeader& );
QTextStream& operator<<( QTextStream&, const QHttpReplyHeader& );

#endif

#endif
