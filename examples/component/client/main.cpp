#include <qcomponentfactory.h>

#include "../interfaces/printinterface.h"

int main()
{
    QInterfacePtr<PrintInterface> iface;

    if ( QComponentFactory::createInstance( "Qt.Example", IID_Print, (QUnknownInterface**)&iface ) == QS_OK )
	iface->sayHello();

    if ( QComponentFactory::createInstance( "Qt.AnotherExample.1", IID_Print, (QUnknownInterface**)&iface ) == QS_OK )
	iface->sayHello();

    return 0;
}
