#include "plugmainwindow.h"

#include <qvariant.h>

QApplicationInterface* PlugApplication::requestApplicationInterface()
{
    return appIface ? appIface : ( appIface = new PlugApplicationInterface );
}

PlugApplicationInterface::PlugApplicationInterface()
{
    iMainWindow = 0;
}

PlugApplicationInterface::~PlugApplicationInterface()
{
    delete iMainWindow;
}

QComponentInterface* PlugApplicationInterface::requestInterface( const QCString& request )
{
    if ( request == "PlugMainWindowInterface" ) {
	return iMainWindow ? iMainWindow : (iMainWindow = new QComponentInterface( (PlugMainWindow*)qApp->mainWidget() ) );
    }
    return 0;
}

int main( int argc, char** argv )
{
    PlugApplication app( argc, argv );
    PlugMainWindow mw;

    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
