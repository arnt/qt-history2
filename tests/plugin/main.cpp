#include "plugmainwindow.h"
#include "qapplicationinterfaces.h"
#include <qvariant.h>

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

void PlugMainWindowInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    mainWindow()->setProperty( p, v );
}

void PlugMainWindowInterface::requestProperty( const QCString& p, QVariant& v )
{
    if ( p == "mainWindow" ) {
	v = QVariant( (uint)mainWindow() );
    }    
    v = mainWindow()->property( p );
}

PlugMainWindow* PlugMainWindowInterface::mainWindow() const
{
    return (PlugMainWindow*)qApp->mainWidget();
}

int main( int argc, char** argv )
{
    PlugApplication app( argc, argv );
    PlugMainWindow mw;

    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
