#include "plugmainwindow.h"
#include "qapplicationinterfaces.h"

QApplicationInterface* PlugApplication::requestApplicationInterface( const QCString& request )
{
    if ( request == "QActionInterface" )
	return mwIface ? mwIface : ( mwIface = new PlugMainWindowInterface );
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
