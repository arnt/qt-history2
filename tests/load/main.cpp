#include <qapplication.h>
#include <qnetwork.h>
#include "load.h"

main( int argc, char** argv )
{
    QApplication app(argc,argv);

    qInitNetworkProtocols();

    QString url;

    url = argv[1];

    Load ice(url);
    app.setMainWidget(&ice);
    ice.show();

    return app.exec();
}
