#include <qapplication.h>
#include <qtextstream.h>
#include <qhttp.h>
#include <qurl.h>
#include <qfile.h>

#include "main.h"


TestClient::TestClient()
{
    m_client = new QHttpClient( this );

    connect( m_client, SIGNAL(response(const QHttpResponseHeader&,const QByteArray&)),
	    this, SLOT(reply(const QHttpResponseHeader&, const QByteArray&)) );
    connect( m_client, SIGNAL( finished() ),
	    qApp, SLOT( quit() ) );
}

void TestClient::get( const QString& host, int port, const QString& path )
{
    QHttpRequestHeader r1( "GET", path );
    m_client->request( host, port, r1, QByteArray() );
}

void TestClient::reply( const QHttpResponseHeader& repl, const QByteArray& data )
{
    qDebug( "Response=%s", repl.toString().latin1() );

    QFile file( "data" );
    file.open( IO_WriteOnly );
    file.writeBlock( data );

    qDebug( "---- Wrote %i bytes to file '%s'\n", data.size(), file.name().latin1() );
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    if ( argc != 2 )
    {
	fprintf( stderr, "Syntax: %s url\n", argv[0] );
	return 1;
    }

    QUrl url( argv[1] );
    if ( !url.isValid() )
    {
	fprintf( stderr, "Invalid URL\n" );
	return 2;
    }

    if ( url.protocol() != "http" )
    {
	fprintf( stderr, "Only the http protocol is supported\n" );
	return 3;
    }

    TestClient t;
    t.get( url.host(), url.port() != -1 ? url.port() : 80, url.path() );

    return app.exec();
}
