#ifndef MAIN_H
#define MAIN_H

#include <qobject.h>
#include <qhttp.h>

class TestServer : public QHttpServer
{
    Q_OBJECT
public:
    TestServer( int port = 0, QObject* parent = 0, const char* name = 0 );

protected:
    void newConnection( int socket );
};

class TestConnection : public QHttpConnection
{
    Q_OBJECT
public:
    TestConnection( int socket, QObject* parent = 0, const char* name = 0 );
    ~TestConnection();

protected:
    void request( const QHttpRequestHeader& header, const QByteArray& data );
};

#endif
