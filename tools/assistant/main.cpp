#include <qapplication.h>
#include "mainwindow.h"
#include <qserversocket.h>
#include <qsocket.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qsettings.h>
#include <qdir.h>
#include <stdlib.h>
#include "assistant.h"
#include "docuparser.h"

const int server_port = 7358;


class Socket : public QSocket
{
    Q_OBJECT

public:
    Socket( QObject *parent );
    ~Socket();

    void start();

private slots:
    void startOwnWindow();
    void sendFile();

};

class ServerSocket : public QServerSocket
{
    Q_OBJECT
public:
    ServerSocket( MainWindow *mw );
    ~ServerSocket();
    void newConnection( int );

private slots:
    void dataReceived();

private:
    QPtrList<QSocket> connections;
    MainWindow *mainWindow;

};


Socket::Socket( QObject *parent )
    : QSocket( parent )
{
    connect( this, SIGNAL( error( int ) ), this, SLOT( startOwnWindow() ) );
    connect( this, SIGNAL( connected() ), this, SLOT( sendFile() ) );
    connect( this, SIGNAL( bytesWritten( int ) ), qApp, SLOT( quit() ) );
}

Socket::~Socket()
{
}

void Socket::start()
{
    connectToHost( "127.0.0.1", server_port );
}

void Socket::startOwnWindow()
{
    MainWindow * mw = new MainWindow;

    QString keybase("/Qt Assistant/3.1/");
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    if ( config.readBoolEntry( keybase  + "GeometryMaximized", FALSE ) )
	mw->showMaximized();
    else
	mw->show();

    QString s = qApp->argv()[ 1 ];
    if ( s.left( 2 ) == "d:" )
	s.remove( 0, 2 );
    if ( !s.isEmpty() )
	mw->showLink( s, "" );
    (void)new ServerSocket( mw );
}

void Socket::sendFile()
{
    QString s = qApp->argv()[ 1 ];
    if ( s.left( 2 ) == "d:" ) {
	s.remove( 0, 2 );
	s += "\n";
	writeBlock( s.latin1(), s.length() );
    } else {
	s += "\n";
	writeBlock( s.latin1(), s.length() );
    }
}




ServerSocket::ServerSocket( MainWindow *mw )
    : QServerSocket( server_port, 0, mw ), mainWindow( mw )
{
    connections.setAutoDelete( TRUE );
}

ServerSocket::~ServerSocket()
{
    for ( QSocket *s = connections.first(); s; s = connections.next() )
	s->close();
}

void ServerSocket::newConnection( int socket )
{
    QSocket *s = new QSocket( this );
    s->setSocket( socket );
    connect( s, SIGNAL( readyRead() ),
	     this, SLOT( dataReceived() ) );
    connections.append( s );
}

void ServerSocket::dataReceived()
{
    QSocket *s = (QSocket*)sender();
    if ( !s || !s->inherits( "QSocket" ) )
	return;
    if ( !s->canReadLine() )
	return;
    QString line = s->readLine();
    line = line.simplifyWhiteSpace();
#if 0
    if ( line == "assistant" ) {
	Assistant *a = new Assistant( 0 );
	a->resize( 300, 600 );
	a->show();
	return;
    }
#endif
    mainWindow->showLink( line, "" );
    mainWindow->show();
    mainWindow->raise();
}

class StdInParser : public QSocket
{
    Q_OBJECT
public:
    StdInParser( MainWindow *aWindow, QObject *parent=0, const char *name = 0 );

public slots:
    void readIn();

private:
    MainWindow *mw;
};

StdInParser::StdInParser( MainWindow *aWindow, QObject *parent, const char *name )
    :QSocket( parent, name )
{
    mw = aWindow;
    connect( this, SIGNAL(readyRead()), this, SLOT(readIn()) );
    setSocket(0);
}

void StdInParser::readIn()
{
    QString data = readLine().simplifyWhiteSpace();
    if ( '-' != data[0] ) {
	mw->showLink( data, "" );
    } else {
	if ( -1 != data.find("find") ) {
	    mw->find();
	} else if ( -1 != data.find("goHome") ) {
	    mw->goHome();
	} else if ( -1 != data.find("hide") ) {
	    mw->hide();
	    return;
	} else if ( -1 != data.find("print") ) {
	    mw->print();
	} else if ( -1 != data.find("raise") ) {
	    mw->raise();
	} else if ( -1 != data.find("saveSettings") ) {
	    mw->saveSettings();
	} else if ( -1 != data.find("setupBookmarkMenu") ) {
	    mw->setupBookmarkMenu();
	} else if ( -1 != data.find("show") ) {
	    mw->show();
	} else if ( -1 != data.find("showBookmark") ) {
	    mw->showBookmark( data.remove(0, data.findRev(' ')).toInt() );
	} else if ( -1 != data.find("showDesignerHelp") ) {
	    mw->showDesignerHelp();
	} else if ( -1 != data.find("showLinguistHelp") ) {
	    mw->showLinguistHelp();
	} else if ( -1 != data.find("showQtHelp") ) {
	    mw->showQtHelp();
	} else if ( -1 != data.find("showSettingsDialog") ) {
	    mw->showSettingsDialog();
	} else if ( -1 != data.find("updateBookmarkMenu") ) {
	    mw->updateBookmarkMenu();
	}
    }
    mw->show();
    mw->raise();
}

class AddDoc
{
public:
    AddDoc();
    bool addDocFile( const QString &file );
private:
    void addItemToList( const QString &rcEntry, const QString &item );
};

AddDoc::AddDoc()
{
}

bool AddDoc::addDocFile( const QString &file )
{
    QFileInfo fi( file );
    if ( !fi.isReadable() ) {
	printf( "error: file %s is not readable!\n\n", file.latin1() );
	return FALSE;
    }

    addItemToList( "/Qt Assistant/3.1/AdditionalDocFiles/", fi.absFilePath() );

    DocuParser handler;

    QFile f( file );
    QXmlInputSource source( f );
    QXmlSimpleReader reader;
    reader.setContentHandler( &handler );
    reader.setErrorHandler( &handler );
    bool ok = reader.parse( source );
    f.close();
    if ( !ok ) {
	QString afp = fi.absFilePath();
	printf( "error: file %s has a wrong format!\n\n", afp.latin1() );
	return FALSE;
    }
    if ( handler.getCategory().isEmpty() )
	return TRUE;    
    
    addItemToList( "/Qt Assistant/3.1/CategoriesAvailable/", handler.getCategory() );
    addItemToList( "/Qt Assistant/3.1/CategoriesSelected/", handler.getCategory() );

    return TRUE;
}

void AddDoc::addItemToList( const QString &rcEntry, const QString &item )
{
    QSettings settings;
    QStringList list = settings.readListEntry( rcEntry );
    QStringList::iterator it = list.begin();
    for ( ; it != list.end(); ++it ) {
	if ( item.lower() == (*it).lower() )
	    return;
    }
    list << item;
    settings.writeEntry( rcEntry, list );
    settings.writeEntry( "/Qt Assistant/3.1/NewDoc/", TRUE );
}


#ifdef Q_OS_MACX
#include <stdlib.h>
#include <qdir.h>
#endif

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    StdInParser *commandInput = 0;

    QStringList catlist;
    QString file = "";
    bool parsestdin = FALSE;
    if ( a.argc() == 2 ) {
	if ( (a.argv()[1])[0] != '-' )
	    file = a.argv()[1];
    }
    if ( file.isEmpty() ) {
	for ( int i = 1; i < a.argc(); i++ ) {
	    if ( QString( a.argv()[i] ) == "-file" ) {
		i++;
		file = a.argv()[i];
	    } else if ( QString( a.argv()[i] ) == "-stdin" ) {
	        parsestdin = TRUE;
	    } else if ( QString( a.argv()[i] ).lower() == "-category" ) {
		i++;
		catlist << QString(a.argv()[i]).lower();
	    } else if ( QString( a.argv()[i] ).lower() == "-addcontentfile" ) {
		i++;
		AddDoc ad;
		if ( !ad.addDocFile( a.argv()[i] ) )
		    exit( 1 );
		exit( 0 );
	    } else if ( QString( a.argv()[i] ) == "-help" ) {
		printf( "Usage: assistant [option]\n" );
		printf( "Options:\n" );
		printf( " -file Filename       assistant opens the specified file\n" );
		printf( " -category Category   displays all documentations which\n" );
		printf( "                      belong to this category. This\n" );
		printf( "                      option can be set serveral times\n" );
		printf( " -stdin               reads commands from stdin after\n" );
		printf( "                      assistant has started\n" );
		printf( " -addContentFile File adds the documentation found in the\n" );
		printf( "                      specified file. Make sure that this\n" );
		printf( "                      file has the right format. For further\n" );
		printf( "                      informations have a look at the\n" );
		printf( "                      assistant online help.\n" );
		printf( " -help                shows this help\n" );
		exit( 0 );
	    }
	    else {
		printf( "Wrong options! Try -help to get help.\n" );
		exit( 1 );
	    }
	}
    }
    if ( a.argc() >= 1 || file.left( 2 ) != "d:" ) {
	QString keybase("/Qt Assistant/3.1/");
	QSettings *config = new QSettings();
	config->insertSearchPath( QSettings::Windows, "/Trolltech" );
	if( !catlist.isEmpty() ) {
	    config->writeEntry( "/Qt Assistant/3.1/CategoriesSelected/", catlist );
	}
	bool max = config->readBoolEntry( keybase  + "GeometryMaximized", FALSE ) ;
	delete config;
	config = 0;
	MainWindow * mw = new MainWindow(0, "Assistant" );

	if ( max )
	    mw->showMaximized();
	else
	    mw->show();

	if ( file.left( 2 ) == "d:" )
	    file.remove( 0, 2 );
	if ( parsestdin )
	    commandInput = new StdInParser(mw, &a);
	else if ( !file.isEmpty() )
	    mw->showLink( file, "" );

    } else {
	Socket *s = new Socket( 0 );
	s->start();
    }

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    // Now we need to setup read from stdin...

    return a.exec();
}

#include "main.moc"
