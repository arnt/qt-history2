#include "qtservice.h"
#include <qapplication.h>
#include <qwidget.h>

class AService : public QtService
{
public:
    AService( const QString &name )
	: QtService( name, TRUE )
    {
    }

protected:
    int run( int argc, char **argv );
    void quit();
    void pause();
    void resume();

private:
    bool running;
};

int AService::run( int argc, char **argv )
{
    QApplication app( argc, argv );
    QWidget widget;
    app.setMainWidget( &widget );
    widget.show();

    return app.exec();
}

void AService::quit()
{
    if ( qApp )
	qApp->quit();
}

void AService::pause()
{
    qApp->mainWidget()->hide();
}

void AService::resume()
{
    qApp->mainWidget()->show();
}

int main( int argc, char **argv )
{
    AService service( QString("Qt Service ") + QT_VERSION_STR );

    if ( argc > 1 ) {
	QString a( argv[1] );
	if ( a == "-i" || a == "-install" ) {
	    if ( !service.isInstalled() ) {
		if ( !service.install() )
		    qDebug( "The service %s could not be installed", service.serviceName().latin1() );
	    } else {
		qDebug( "The service %s is already installed", service.serviceName().latin1() );
	    }
	} else if ( a == "-u" || a == "-uninstall" ) {
	    if ( service.isInstalled() ) {
		if ( !service.uninstall() )
		    qDebug( "The service %s could not be uninstalled", service.serviceName().latin1() );
	    } else {
		qDebug( "The service %s is not installed", service.serviceName().latin1() );
	    }
	} else if ( a == "-v" ) {
	    QString infostr = QString( "The service\n\t%1\n\t%2\n\nis %4 and %5" )
				 .arg( service.serviceName() )
				 .arg( service.filePath() )
				 .arg( service.isInstalled() ? "installed" : "not installed" )
				 .arg( service.isRunning() ? "running" : "not running" );
	    qDebug( infostr.latin1() );
	} else {
	    qDebug( "<service> -[i|u|v]\n\n"
		    "\t-i: Install the service\n"
		    "\t-u: Uninstall the service\n"
		    "\t-v: Print version information\n" );
	}
    } else {
	if ( !service.start() )
	    qDebug( "The service %s could not start", service.serviceName().latin1() );
    }

    return service.isRunning();
}
