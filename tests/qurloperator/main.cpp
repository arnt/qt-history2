#include <qapplication.h>
#include <qnetwork.h>

#include "mainview.h"

int main( int argc, char** argv )
{
    QApplication a( argc, argv );

    qInitNetworkProtocols();

    MainView mv;
    mv.show();

    a.setMainWidget( &mv );

    return a.exec();
}
