#include "plugmainwindow.h"
#include "qapplicationinterfaces.h"

QApplicationInterface* PlugApplication::requestInterface( const QCString& request )
{
    if ( request == "PlugMainWindowInterface" )
	return mwIface ? mwIface : ( mwIface = new PlugMainWindowInterface );
    else
	return QApplication::requestInterface( request );
}

int main( int argc, char** argv )
{
    PlugApplication app( argc, argv );
    PlugMainWindow mw;

    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
