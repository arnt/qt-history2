#include <qapplication.h>
#include "packageclient.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv, false );

    if ( a.argc() != 4 ) {
	qFatal( "Three arguments expected" );
	return -1;
    }

    new PackageClient( a.argv()[1], a.argv()[2], a.argv()[3] );

    a.exec();
};
