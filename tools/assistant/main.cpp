/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mainwindow.h"
#include "helpdialogimpl.h"
#include "config.h"

#include <qapplication.h>
#include <qserversocket.h>
#include <qsocket.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qsettings.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qguardedptr.h>
#include <stdlib.h>
#include <stdio.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#ifdef Q_WS_WIN
#define INDEX_CHECK( text ) if( i+1 >= argc ) { QMessageBox::information( 0, "Qt Assistant", text ); return 1; }
#else
#define INDEX_CHECK( text ) if( i+1 >= argc ) { fprintf( stderr, text "\n" ); return 1; }
#endif

static bool allowFirstRun = TRUE;

class AssistantSocket : public QSocket
{
    Q_OBJECT
public:
    AssistantSocket( int sock, QObject *parent = 0 );
    ~AssistantSocket() {}

signals:
    void showLinkRequest( const QString& );

private slots:
    void readClient();
    void connectionClosed();
};


class AssistantServer : public QServerSocket
{
    Q_OBJECT
public:
    AssistantServer( QObject* parent = 0 );
    void newConnection( int socket );
    Q_UINT16 getPort() const;

signals:
    void showLinkRequest( const QString& );
    void newConnect();

private:
    Q_UINT16 p;
};


AssistantSocket::AssistantSocket( int sock, QObject *parent )
    : QSocket( parent, 0 )
{
    connect( this, SIGNAL( readyRead() ),
	     SLOT( readClient() ) );
    connect( this, SIGNAL( connectionClosed() ),
	     SLOT( connectionClosed() ) );
    setSocket( sock );
}

void AssistantSocket::readClient()
{
    QString link = QString::null;
    while ( canReadLine() )
	link = readLine();
    if ( !link.isNull() ) {
	link = link.replace( "\n", "" );
	link = link.replace( "\r", "" );
	emit showLinkRequest( link );
    }
}

void AssistantSocket::connectionClosed()
{
    delete this;
}

AssistantServer::AssistantServer( QObject *parent )
    : QServerSocket( 0x7f000001, 0, 1, parent )
{
    if ( !ok() ) {
	QMessageBox::critical( 0, tr( "Qt Assistant" ),
		tr( "Failed to bind to port %1" ).arg( port() ) );
        exit( 1 );
    }
    p = port();
}

Q_UINT16 AssistantServer::getPort() const
{
    return p;
}

void AssistantServer::newConnection( int socket )
{
    AssistantSocket *as = new AssistantSocket( socket, this );
    connect( as, SIGNAL( showLinkRequest( const QString& ) ),
	     this, SIGNAL( showLinkRequest( const QString& ) ) );
    emit newConnect();
}

int main( int argc, char ** argv )
{
    bool withGUI = TRUE;
    if ( argc > 1 ) {
	QString arg( argv[1] );
	arg = arg.lower();
#ifndef Q_WS_WIN
	if ( arg == "-addprofile" ||
	     arg == "-installqtdoc" ||
	     arg == "-help" )
	    withGUI = FALSE;
#endif
    }
    QApplication a( argc, argv, withGUI );

    QString resourceDir;
    AssistantServer *as = 0;
    QStringList catlist;
    QString file, profileName, aDocPath;
    bool server = FALSE;
    bool hideSidebar = FALSE;
    bool startClean = FALSE;
    if ( argc == 2 ) {
	if ( (argv[1])[0] != '-' )
	    file = argv[1];
    }
    if ( file.isEmpty() ) {
	for ( int i = 1; i < argc; i++ ) {
	    if ( QString( argv[i] ).lower() == "-file" ) {
		INDEX_CHECK( "Missing file argument!" );
		i++;
		file = argv[i];
	    } else if ( QString( argv[i] ).lower() == "-assistanthelppath" ) {
		INDEX_CHECK( "Missing path of assisntant help files!" );
		aDocPath = argv[++i];
	    } else if ( QString( argv[i] ).lower() == "-server" ) {
	        server = TRUE;
	    //} else if ( QString( argv[i] ).lower() == "-disablefirstrun" ) {
	    //    allowFirstRun = FALSE;
	    } else if ( QString( argv[i] ).lower() == "-startclean" ) {
		startClean = TRUE;
	    } else if ( QString( argv[i] ).lower() == "-addprofile" ) {
		INDEX_CHECK( "Missing profile argument!" );
		QString path = "";
		i++;
		if( i+1 < argc )
		    path = QString( argv[i+1] );
		if ( !Config::addProfile( QString( argv[i] ), path ) )
		    exit( 1 );
		exit( 0 );
	    } else if ( QString( argv[i] ).lower() == "-installqtdoc" ) {
		if ( !Config::addProfile( "default", QString::null ) )
		    exit( 1 );
		exit( 0 );
	    } else if ( QString( argv[i] ).lower() == "-profile" ) {
		INDEX_CHECK( "Missing profile argument!" );
		profileName = argv[++i];
	    } else if ( QString( argv[i] ).lower() == "-hidesidebar" ) {
		hideSidebar = TRUE;
	    } else if ( QString( argv[i] ).lower() == "-help" ) {
		QString helpText( "Usage: assistant [option]\n"
				  "Options:\n"
				  " -file Filename             assistant opens the specified file\n"
				  //" -disableFirstRun         assistant will not try to register defaults.\n"
				  " -server                    reads commands from a socket after\n"
				  "                            assistant has started\n"
				  " -profile Name              starts assistant and displays the\n"
				  "                            profile Name.\n"
				  " -addProfile File [Path]    adds the profile defined in File.\n"
				  "                            Specify the location of the content\n"
				  "                            files via Path. Otherwise, the base\n"
				  "                            path of the profile is taken.\n"
				  "                            For further informations have a look\n"
				  "                            at the assistant online help.\n"
				  " -installQtDoc              installs the Qt profile. This option is\n"
				  "                            only necessary if assistants first installed\n"
				  "                            profile was another than the Qt one.\n"
				  "                            If assistant is run the first time without\n"
				  "                            any argument this is called by default.\n"
				  " -assistantHelpPath [Path]  location of the help files for how to use\n"
				  "                            Qt Assistant.\n"
				  " -hideSidebar               assistant will hide the sidebar.\n"
				  " -help                      shows this help.");
#ifdef Q_WS_WIN
		QMessageBox::information( 0, "Qt Assistant", "<pre>" + helpText + "</pre>" );
#else
		printf( "%s\n", helpText.latin1() );
#endif
		exit( 0 );
	    } else if ( QString( argv[i] ).lower() == "-resourcedir" ) {
		INDEX_CHECK( "Missing resource directory argument!" );
		resourceDir = QString( argv[++i] );
	    } else {
		qFatal( "Unrecognized option '%s'. Try -help to get help.\n", argv[i] );
	    }
	}
    }

    if( resourceDir.isNull() )
	resourceDir = qInstallPathTranslations();
    if( !QFile::exists( resourceDir ) ) 
	fprintf( stderr, "Resource file directory '%s' does not exist!\n", resourceDir.latin1() );

    QTranslator translator( 0 );
    translator.load( QString("assistant_") + QTextCodec::locale(), resourceDir );
    a.installTranslator( &translator );

    QTranslator qtTranslator( 0 );
    qtTranslator.load( QString("qt_") + QTextCodec::locale(), resourceDir );
    a.installTranslator( &qtTranslator );

    Config *conf = new Config( profileName );
    if ( !conf->validProfileName() ) {
	qFatal( "Profile '%s' does not exist!\n", profileName.latin1() );
	return -1;
    }

    bool max = conf->isMaximized();
    QString link = conf->source();
    conf->hideSideBar( hideSidebar );

    if( !aDocPath.isNull() ) {
	conf->setAssistantDocPath( aDocPath );
    }

    QGuardedPtr<MainWindow> mw = new MainWindow( 0, "Assistant" );

    if ( server ) {
	as = new AssistantServer();
	printf("%d\n", as->port() );
	fflush( stdout );
	as->connect( as, SIGNAL( showLinkRequest( const QString& ) ),
		     mw, SLOT( showLinkFromClient( const QString& ) ) );
    }

    if ( max )
	mw->showMaximized();
    else
	mw->show();

    if ( !server ) {
	if ( !file.isEmpty() )
	    mw->showLink( file );
	else if ( file.isEmpty() )
	    mw->showLink( link );
    }

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    int appExec = a.exec();
    delete mw;
    return appExec;
}

#include "main.moc"
