#include "plugmainwindow.h"
#include "qwidgetfactory.h"
#include "qapplicationinterface.h"
#include <qvariant.h>

QApplicationInterface* PlugApplication::requestApplicationInterface( const QCString& request )
{
    if ( request == "PlugMainWindowInterface" )
	return mwIface ? mwIface : ( mwIface = new PlugMainWindowInterface( qApp->mainWidget() ) );
    else
	return QApplication::requestApplicationInterface( request );
}

QStrList PlugApplication::queryInterfaceList() const
{
    QStrList list;
    list.append( "PlugMainWindowInterface" );

    return list;
}

PlugMainWindowInterface::PlugMainWindowInterface( QObject *o )
    : QApplicationInterface( o ) 
{
}

void PlugMainWindowInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    if ( p == "centralWidget" ) { // fake a property
	PlugMainWindow* mw = (PlugMainWindow*)object();
	QWidget* w = QWidgetFactory::create( v.toString(), mw );
	delete mw->centralWidget();
	if ( w ) {
	    mw->setCentralWidget( w );
	} else {
	    QWidget* label = QWidgetFactory::create( "QLabel", mw );
	    label->setProperty( "text", QString("Don't know \"%1\"").arg( v.toString() ) );
	    mw->setCentralWidget( label );
	}
    } else {
	QApplicationInterface::requestSetProperty( p, v );
    }
}

void PlugMainWindowInterface::requestProperty( const QCString& p, QVariant& v )
{
    if ( p == "mainWindow" ) // fake a property (hacky and probably unsafe)
	v = QVariant( (uint)object() );
    else
	QApplicationInterface::requestProperty( p, v );
}

int main( int argc, char** argv )
{
    PlugApplication app( argc, argv );
    PlugMainWindow mw;

    app.setMainWidget( &mw );
    mw.show();

    return app.exec();
}
