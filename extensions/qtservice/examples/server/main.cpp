/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <stdlib.h>
#include <qsocket.h>
#include <qregexp.h>
#include <qserversocket.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qtservice.h>

// HttpDaemon is the the class that implements the simple HTTP server.
class HttpDaemon : public QServerSocket
{
    Q_OBJECT
public:
    HttpDaemon( QObject* parent=0 ) :
	QServerSocket(8080,1,parent)
    {
	if ( !ok() ) {
	    qService->reportEvent( "Failed to bind to port 8080", QtService::Error );
	    exit( 1 );
	}
    }

    void newConnection( int socket )
    {
	// When a new client connects, the server constructs a QSocket and all
	// communication with the client is done over this QSocket. QSocket
	// works asynchronouslyl, this means that all the communication is done
	// in the two slots readClient() and discardClient().
	QSocket* s = new QSocket( this );
	connect( s, SIGNAL(readyRead()), this, SLOT(readClient()) );
	connect( s, SIGNAL(delayedCloseFinished()), this, SLOT(discardClient()) );
	s->setSocket( socket );
	
	qService->reportEvent( "New Connection" );
    }

private slots:
    void readClient()
    {
	// This slot is called when the client sent data to the server. The
	// server looks if it was a get request and sends a very simple HTML
	// document back.
	QSocket* socket = (QSocket*)sender();
	if ( socket->canReadLine() ) {
	    QStringList tokens = QStringList::split( QRegExp("[ \r\n][ \r\n]*"), socket->readLine() );
	    if ( tokens[0] == "GET" ) {
		QTextStream os( socket );
		os.setEncoding( QTextStream::UnicodeUTF8 );
		os << "HTTP/1.0 200 Ok\r\n"
		    "Content-Type: text/html; charset=\"utf-8\"\r\n"
		    "\r\n"
		    "<h1>Nothing to see here</h1>\n";
		socket->close();
		
		qService->reportEvent( "Wrote to client" );
	    }
	}
    }
    void discardClient()
    {
	QSocket* socket = (QSocket*)sender();
	delete socket;
	
	qService->reportEvent( "Connection closed" );
    }
};

class HttpService : public QtService
{
public:
    HttpService()
	: QtService( "Qt HTTP Daemon", TRUE, FALSE )
    {
	qService = this;
    }

protected:
    int run( int argc, char **argv )
    {
	QApplication app( argc, argv, FALSE );

	HttpDaemon daemon;
	return app.exec();
    }
    void stop()
    {
	qApp->quit();
    }
};

void main( int argc, char **argv )
{
    HttpService service;

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
}

#include "main.moc"
