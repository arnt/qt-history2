#include <qapplication.h>
#include "qss.h"

main(int argc, char** argv)
{
    QApplication app(argc,argv);
    if ( argc > 1 ) {
	// Client
	QWSSoundClient client;
	QObject::connect(&client,SIGNAL(connectionClosed()),qApp,SLOT(quit()));
	QObject::connect(&client,SIGNAL(delayedCloseFinished()),qApp,SLOT(quit()));
	for (int a=1; a<argc; a++) {
	    client.play(argv[a]);
	}
	client.close();
	return app.exec();
    } else {
	// Server
	QWSSoundServer qss;
	return app.exec();
    }
}
