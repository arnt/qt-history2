#include <qcomponentfactory.h>

#include "../interfaces/printinterface.h"

void main()
{
    QInterfacePtr<PrintInterface> iface;

    if ( QComponentFactory::createInstance( "Qt.Example1", IID_Print, (QUnknownInterface**)&iface ) == QS_OK )
	iface->sayHello();

    iface = 0;

    if ( QComponentFactory::createInstance( "Qt.Example2", IID_Print, (QUnknownInterface**)&iface ) == QS_OK )
	iface->sayHello();
}
