#include <qtservice.h>
#include <qapplication.h>
#include <qlabel.h>

class InteractiveService : public QtService
{
public:
    InteractiveService( const QString &name );

protected:
    int run( int argc, char **argv );
    void stop();
    void pause();
    void resume();

private:
    QLabel *gui;
};

InteractiveService::InteractiveService( const QString &name )
    : QtService( name, TRUE, TRUE ), gui( 0 )
{
}

int InteractiveService::run( int argc, char **argv )
{
    QApplication app( argc, argv );

    if ( !isRunning() ) {
	gui = new QLabel( "Running standalone!", 0, "gui" );
	app.setMainWidget( gui );
    } else {
	gui = new QLabel( "Running as a service!", 0, "gui", Qt::WStyle_Customize | Qt::WStyle_NoBorder );
    }
    gui->move( app.desktop()->availableGeometry().topLeft() );
    gui->show();

    return app.exec();
}

void InteractiveService::stop()
{
    if ( qApp )
	qApp->quit();
}

void InteractiveService::pause()
{
    if ( gui )
	gui->hide();
}

void InteractiveService::resume()
{
    if ( gui )
	gui->show();
}

int main( int argc, char **argv )
{
    InteractiveService service( QString("Qt Interactive Service ") );
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	if ( argc > 1 ) {
	    QString a( argv[1] );
	    if ( a == "-i" || a == "-install" ) {
		if ( !service.isInstalled() ) {
		    if ( !service.install() )
			qWarning( "The service %s could not be installed", service.serviceName().latin1() );
		} else {
		    qWarning( "The service %s is already installed", service.serviceName().latin1() );
		}
	    } else if ( a == "-u" || a == "-uninstall" ) {
		if ( service.isInstalled() ) {
		    if ( !service.uninstall() )
			qWarning( "The service %s could not be uninstalled", service.serviceName().latin1() );
		} else {
		    qWarning( "The service %s is not installed", service.serviceName().latin1() );
		}
	    } else if ( a == "-v" || a == "-version" ) {
		QString infostr = QString( "The service\n\t%1\n\t%2\n\nis %4 and %5" )
				     .arg( service.serviceName() )
				     .arg( service.filePath() )
				     .arg( service.isInstalled() ? "installed" : "not installed" )
				     .arg( service.isRunning() ? "running" : "not running" );
		qWarning( infostr.latin1() );
	    } else if ( a == "-e" || a == "-exec" ) {
		service.tryStart( argc - 2, argv + 2 );
	    } else if ( a == "-s" || a == "-stop" ) {
		service.tryStop();
	    } else {
		qWarning( "<service> -[i|u|e|s|v]\n\n"
			"\t-i(nstall)\t: Install the service\n"
			"\t-u(ninstall)\t: Uninstall the service\n"
			"\t-e(xec)\t\t: Execute the service.\n"
			"\t\t\t  If the service is not installed, run it as a regular program\n"
			"\t-s(top)\t\t: Stop the service.\n"
			"\t-v(ersion)\t: Print version and status information\n" );
	    }
	} else {
	    if ( !service.start() )
		qWarning( "The service %s could not start", service.serviceName().latin1() );
	}
    } else {
	service.tryStart( argc, argv );
    }

    return 0;
}
