#include <qapplication.h>
#include "controlcentral.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    ControlCentral central;

    a.setMainWidget( &central );
    if ( argc > 1 ) {
	QStringList files;
	for ( int i=1; i<argc; i++ ) {
	    files.append( argv[i] );
	}
	central.show( &files );
    } else {
	central.show();
    }
    return a.exec();
}
