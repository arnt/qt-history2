#include <qapplication.h>
#include <qserversocket.h>
#include <qsocket.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qsettings.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <stdlib.h>
#include <iostream.h>

#include "assistant.h"
#include "mainwindow.h"
#include "docuparser.h"

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

    QString title = handler.getDocumentationTitle();
    if ( title.isEmpty() )
	title = fi.absFilePath();
    addItemToList( "/Qt Assistant/3.1/AdditionalDocFiles/", fi.absFilePath() );
    addItemToList( "/Qt Assistant/3.1/AdditionalDocTitles/", title );
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


#if defined(Q_OS_MACX)
#include <stdlib.h>
#include <qdir.h>
#endif

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    AssistantServer *as = 0;
    QStringList catlist;
    QString file = "";
    bool server = FALSE;
    if ( a.argc() == 2 ) {
	if ( (a.argv()[1])[0] != '-' )
	    file = a.argv()[1];
    }
    if ( file.isEmpty() ) {
	for ( int i = 1; i < a.argc(); i++ ) {
	    if ( QString( a.argv()[i] ) == "-file" ) {
		i++;
		file = a.argv()[i];
	    } else if ( QString( a.argv()[i] ) == "-server" ) {
	        server = TRUE;
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
		printf( " -server              reads commands from a socket after\n" );
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

    QString keybase("/Qt Assistant/3.1/");
    QSettings *config = new QSettings();
    config->insertSearchPath( QSettings::Windows, "/Trolltech" );
    if( !catlist.isEmpty() ) {
	config->writeEntry( keybase + "CategoriesSelected/", catlist );
    }
    bool max = config->readBoolEntry( keybase  + "GeometryMaximized", FALSE );
    QString link = config->readEntry( keybase + "Source", "" );

#ifndef QT_PALMTOPCENTER_DOCS
    bool firstRun = config->readBoolEntry( keybase + "FirstRun", TRUE );
    if ( firstRun ) {
	QString path = QString( qInstallPathDocs() ) + "/html/";
	QStringList lst;
	lst.append( path + "qt.xml" );
	lst.append( path + "designer.xml" );
	lst.append( path + "assistant.xml" );
	lst.append( path + "linguist.xml" );
	lst.append( path + "qmake.xml" );
	config->writeEntry( keybase + "AdditionalDocFiles", lst );
	lst.clear();
	lst << "Qt Reference Documentation" << "Qt Designer Manual";
	lst << "Qt Assistant Manual" << "Qt Linguist Manual" << "qmake User Guide";
	config->writeEntry( keybase + "AdditionalDocTitles", lst );
	lst.clear();
	lst << "qt" << "qt/reference" << "qt/designer" << "qt/assistant" << "qt/linguist" << "qt/qmake";
	config->writeEntry( keybase + "CategoriesAvailable", lst );
	lst.prepend( "all" );
	config->writeEntry( keybase + "CategoriesSelected", lst );
	config->writeEntry( keybase + "FirstRun", FALSE );
	config->writeEntry( keybase + "NewDoc", TRUE );
    }
#endif

    delete config;
    config = 0;

    MainWindow *mw = new MainWindow( 0, "Assistant" );

    if ( server ) {
	as = new AssistantServer();
	cout << as->port() << endl;
	cout.flush();
	as->connect( as, SIGNAL( showLinkRequest( const QString& ) ),
		mw, SLOT( showLinkFromClient( const QString& ) ) );
    }

    if ( max )
	mw->showMaximized();
    else
	mw->show();

    qApp->processEvents();

    if ( !server ) {
	if ( !file.isEmpty() )
	    mw->showLink( file );
	else if ( file.isEmpty() )
	    mw->showLink( link );
    }
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    return a.exec();
}

#include "main.moc"
