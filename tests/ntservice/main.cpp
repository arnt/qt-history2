#include "qtservice.h"
#include <qapplication.h>
#include <qwidget.h>
#include <qmessagebox.h>

class AService : public QObject, public QtService
{
public:
    AService( const QString &name )
	: QtService( name, TRUE, TRUE )
    {
    }

protected:
    int run( int argc, char **argv );
    void quit();
    void pause();
    void resume();
};

int AService::run( int argc, char **argv )
{
    QApplication app( argc, argv );
    QWidget widget;
    widget.show();
    app.setMainWidget( &widget );
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
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
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
	    } else if ( a == "-v" || a == "-version" ) {
		QString infostr = QString( "The service\n\t%1\n\t%2\n\nis %4 and %5" )
				     .arg( service.serviceName() )
				     .arg( service.filePath() )
				     .arg( service.isInstalled() ? "installed" : "not installed" )
				     .arg( service.isRunning() ? "running" : "not running" );
		qDebug( infostr.latin1() );
	    } else if ( a == "-e" || a == "-exec" ) {
		service.exec( argc - 2, &argv[2] );
	    } else if ( a == "-s" || a == "-stop" ) {
		service.stop();
	    } else {
		qDebug( "<service> -[i|u|v]\n\n"
			"\t-i(nstall)\t: Install the service\n"
			"\t-u(ninstall)\t: Uninstall the service\n"
			"\t-e(xec)\t\t: Execute the service.\n"
			"\t\t\t  If the service is not installed, run it as a regular program\n"
			"\t-s(top)\t\t: Stop the service.\n"
			"\t-v(ersion)\t: Print version and status information\n" );
	    }
	} else {
	    if ( !service.start() )
		qDebug( "The service %s could not start", service.serviceName().latin1() );
	}
    } else {
	service.exec( argc, argv );
    }

    return 0;
}
