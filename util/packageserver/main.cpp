#include <qapplication.h>
#include "packageserver.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    new PackageServer();
    a.exec();
}
