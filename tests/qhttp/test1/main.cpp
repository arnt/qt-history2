#include <qapplication.h>
#include <qtextstream.h>
#include <qhttp.h>

#include <stdio.h>

#include "main.h"


TestClient::TestClient()
{
    m_client = new QHttpClient( this );
    connect( m_client, SIGNAL(reply(const QHttpReplyHeader&,const QByteArray&)),
	     this, SLOT(reply(const QHttpReplyHeader&, const QByteArray&)) );
}

void TestClient::get( const QString& host, int port, const QString& path )
{
    QHttpRequestHeader r1( "GET", path );
    m_client->request( host, port, r1, QByteArray() );
}

void TestClient::reply( const QHttpReplyHeader& repl, const QByteArray& data )
{
    qDebug( "Reply=%s", repl.toString().latin1() );

    QByteArray tmp = data;
    tmp.detach();
    tmp.resize( tmp.size() + 1 );
    tmp[ tmp.size() - 1 ] = 0;
    QString str( tmp );

    qDebug( "---- Read %i bytes\n%s", data.size(), str.latin1() );
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QHttpRequestHeader r1( "GET", "/public/welcome.html" );
    r1.setValue( "dummy", "myvalue" );
    r1.setContentType( "text/html" );
    r1.setContentLength( 167 );

    qDebug( "Request1=%s", r1.toString().latin1() );

    QTextStream ts( stdout, IO_WriteOnly );
    ts << r1;
    
    QHttpRequestHeader r2( r1.toString() );
    qDebug( "Request2=%s", r2.toString().latin1() );

    QHttpReplyHeader a1( 200, "OK" );
    a1.setValue( "dummy", "myvalue" );
    a1.setContentType( "text/html" );
    a1.setContentLength( 167 );

    qDebug( "Reply1=%s", a1.toString().latin1() );
    ts << a1;
    
    QHttpReplyHeader a2( a1.toString() );
    qDebug( "Reply2=%s", a2.toString().latin1() );

    TestClient* t = new TestClient;
    t->get( "127.0.0.1", 80, "/" );

    return app.exec();
}
