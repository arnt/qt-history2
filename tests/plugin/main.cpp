#include "plugmainwindow.h"
#include "qapplicationinterfaces.h"

QApplicationInterface* PlugApplication::requestApplicationInterface( const QCString& request )
{
    if ( request == "PlugMainWindowInterface" )
	return mwIface ? mwIface : ( mwIface = new PlugMainWindowInterface );
    else
	return QApplication::requestApplicationInterface( request );
}

QStrList PlugApplication::queryInterfaceList() const
{
    QStrList list;
    list.append( "PlugMainWindowInterface" );

    return list;
}

int main( int argc, char** argv )
{
    PlugApplication app( argc, argv );
    PlugMainWindow mw;

    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
