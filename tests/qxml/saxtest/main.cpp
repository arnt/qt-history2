#include <qapplication.h>

#include "controlcentral.h"

//
// main
//
int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    ControlCentral central;

    a.setMainWidget( &central );
    if ( argc > 1 ) {
	bool noGui = FALSE;
	QStringList files;
	for ( int i=1; i<argc; i++ ) {
	    if ( qstrcmp( argv[i], "-nogui" ) == 0 ) {
		noGui = TRUE;
	    } else {
		files.append( argv[i] );
	    }
	}
	central.show( &files );
	if ( noGui ) {
	    a.quit();
	    return 0;
	}
    } else {
	central.show();
    }
    return a.exec();
}
