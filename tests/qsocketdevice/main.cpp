#include <qapplication.h>

#include "some.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QString host = "127.0.0.1";
    QString port = "2323";

    if ( argc > 1 ) {
	host = argv[1];
    }
    if ( argc > 2 ) {
	port = argv[2];
    }

    Some some( host, port.toUInt() );
    QObject::connect( &some, SIGNAL(quitted()),
	    &a, SLOT(quit()) );

    a.setMainWidget( &some );
    some.show();
    return a.exec();
}
