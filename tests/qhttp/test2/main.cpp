#include <qapplication.h>
#include <qhttp.h>

#include "main.h"

TestServer::TestServer( int port, QObject* parent, const char* name )
    : QHttpServer( port, parent, name )
{
}

void TestServer::newConnection( int socket )
{
    (void)new TestConnection( socket, this );
}

TestConnection::TestConnection( int socket, QObject* parent, const char* name )
    : QHttpConnection( socket, parent, name )
{
    allowKeepAlive( FALSE );
}

TestConnection::~TestConnection()
{
}

void TestConnection::request( const QHttpRequestHeader& header, const QByteArray& )
{
    qDebug( "Request=%s", header.toString().latin1() );

    QHttpReplyHeader repl( 200, "OK" );
    QCString str( "<H1>Alles wird gut</H1>\n" );
    repl.setContentType( "text/html" );

    reply( repl, str );
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    TestServer* t = new TestServer;
    qDebug("PORT=%i", t->port() );

    return app.exec();
}
