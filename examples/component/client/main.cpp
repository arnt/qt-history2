#include <qcomponentfactory.h>

#include "../interfaces/printinterface.h"

int main( int argc, char **argv )
{
    QInterfacePtr<PrintInterface> iface;

    if ( QComponentFactory::createInstance( "Qt.Example", IID_Print, (QUnknownInterface**)&iface ) == QS_OK ) {
	iface->sayHello();
    }

    return 0;
}
