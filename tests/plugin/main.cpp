#include "plugmainwindow.h"

#include <qvariant.h>

QApplicationInterface* PlugApplication::requestApplicationInterface( const QCString& request )
{
    if ( request == "PlugMainWindowInterface" )
	return appIface ? appIface : ( appIface = new QApplicationInterface( qApp->mainWidget() ) );
    else
	return QApplication::requestApplicationInterface( request );
}

int main( int argc, char** argv )
{
    PlugApplication app( argc, argv );
    PlugMainWindow mw;

    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
