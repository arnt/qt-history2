#include <qhttp.h>
#include <qapplication.h>
#include <qfile.h>
#include <qurl.h>

#include <stdlib.h>
#include <stdio.h>

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
    
    QFile file;
    file.open( IO_WriteOnly, stdout );
	
    QFile in;
    in.open( IO_ReadOnly, stdin );

    QHttpClient* client = new QHttpClient;
    client->setDevice( &file );
    QHttpRequestHeader header( "POST", url.path() );
    header.setContentType( "text/plain" );

    client->request( url.host(), ( url.port() != -1 ? url.port() : 80 ), header, &in );

    QObject::connect( client, SIGNAL( finished() ), qApp, SLOT( quit() ) );

    app.exec();

    file.close();
    in.close();
}
