#include <qcomponentfactory.h>

int main( int argc, char **argv )
{
    QInterfacePtr<QUnknownInterface> iface;

    if ( QComponentFactory::createInstance( "Qt.Example", IID_QUnknown, &iface ) == QS_OK ) {
	qDebug( "Component has been loaded" );
    }

    return 0;
}
