#include <qapplication.h>
#include <qnetwork.h>
#include "load.h"

main( int argc, char** argv )
{
    QApplication app(argc,argv);

    qInitNetworkProtocols();

    QString url;

    if ( argc > 1 )
	url = argv[1];
    else // ##### name server trouble...
	//url = "http://ice.trolltech.com.au/cgi-bin/isdninfo";
	url = "http://203.46.225.145/cgi-bin/isdninfo";

    Load ice(url);
    app.setMainWidget(&ice);
    ice.show();

    return app.exec();
}
