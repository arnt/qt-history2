#include "plugmainwindow.h"
#include "qwidgetfactory.h"
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
    if ( p == "centralWidget" ) { // fake a property
	QWidget* w = QWidgetFactory::create( v.toString(), mainWindow() );
	delete mainWindow()->centralWidget();
	if ( w ) {
	    mainWindow()->setCentralWidget( w );
	} else {
	    QWidget* label = QWidgetFactory::create( "QLabel", mainWindow() );
	    label->setProperty( "text", QString("Don't know \"%1\"").arg( v.toString() ) );
	    mainWindow()->setCentralWidget( label );	    
	}
    } else {
	mainWindow()->setProperty( p, v );
    }
}

void PlugMainWindowInterface::requestProperty( const QCString& p, QVariant& v )
{
    if ( p == "mainWindow" ) // fake a property (hacky and probably unsafe)
	v = QVariant( (uint)mainWindow() );
    else
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
