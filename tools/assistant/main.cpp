/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mainwindow.h"
#include "helpdialog.h"
#include "config.h"

#include <qapplication.h>
#include <qserversocket.h>
#include <qsocket.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qpointer.h>
#include <stdlib.h>
#include <stdio.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#ifdef Q_WS_WIN
#define INDEX_CHECK( text ) if( i+1 >= argc ) { QMessageBox::information( 0, "Qt Assistant", text ); return 1; }
#else
#define INDEX_CHECK( text ) if( i+1 >= argc ) { fprintf( stderr, text "\n" ); return 1; }
#endif

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
    : QSocket( parent )
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
        link = link.replace(QLatin1String("\n"), QLatin1String(""));
        link = link.replace(QLatin1String("\r"), QLatin1String(""));
        emit showLinkRequest( link );
    }
}

void AssistantSocket::connectionClosed()
{
    delete this;
}

AssistantServer::AssistantServer( QObject *parent )
    : QServerSocket( QHostAddress::LocalhostAddress, 0, 1, parent )
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
        QString arg = QString::fromAscii(argv[1]);
        arg = arg.toLower();
        if ( arg == QLatin1String("-addcontentfile")
            || arg == QLatin1String("-removecontentfile")
#ifndef Q_WS_WIN
            || arg == QLatin1String("-help")
#endif
            )
            withGUI = FALSE;
    }
    QApplication a(argc, argv, withGUI);

    QString resourceDir;
    AssistantServer *as = 0;
    QStringList catlist;
    QString file, profileName, aDocPath;
    bool server = FALSE;
    bool hideSidebar = FALSE;
    if ( argc == 2 ) {
        if ( (argv[1])[0] != '-' )
            file = QString::fromUtf8(argv[1]);
    }
    if ( file.isEmpty() ) {
        for ( int i = 1; i < argc; i++ ) {
            QString opt = QString::fromAscii(argv[i]).toLower();
            if ( opt == QLatin1String("-file") ) {
                INDEX_CHECK( "Missing file argument!" );
                i++;
                file = QFile::decodeName(argv[i]);
            } else if ( opt == QLatin1String("-server") ) {
                server = TRUE;
            } else if ( opt == QLatin1String("-profile") ) {
                INDEX_CHECK( "Missing profile argument!" );
                profileName = QFile::decodeName(argv[++i]);
            } else if ( opt == QLatin1String("-addcontentfile") ) {
                INDEX_CHECK( "Missing content file!" );
                Config *c = Config::loadConfig( QString::null );
                QFileInfo file( QFile::decodeName(argv[i+1]) );
                if( !file.exists() ) {
                    fprintf( stderr, "Could not locate content file: '%s'\n",
                             file.absoluteFilePath().latin1() );
                    fflush( stderr );
                    return 1;
                }
                DocuParser *parser = DocuParser::createParser( file.absoluteFilePath() );
                if( parser ) {
                    QFile f( QFile::decodeName(argv[i+1]) );
                    if( !parser->parse( &f ) ) {
                        fprintf( stderr, "Failed to parse file: '%s'\n, ",
                                 file.absoluteFilePath().latin1() );
                        fflush( stderr );
                        return 1;
                    }
                    parser->addTo( c->profile() );
                    c->setDocRebuild( TRUE );
                    c->save();
                }
                return 0;
            } else if ( opt == QLatin1String("-removecontentfile") ) {
                INDEX_CHECK( "Missing content file!" );
                Config *c = Config::loadConfig( QString::null );
                Profile *profile = c->profile();
                QStringList entries = profile->docs.find(QString::fromAscii(argv[i+1]));
                if (entries.count() == 0) {
                    fprintf(stderr, "Could not locate content file: '%s'\n",
                            argv[i+1]);
                    fflush(stderr);
                    return 1;
                } else if (entries.count() > 1) {
                    fprintf(stderr, "More than one entry matching file name found, "
                        "please specify full path to file");
                    fflush(stderr);
                    return 1;
                } else {
                    QFileInfo file(entries[0]);
                    if( !file.exists() ) {
                        fprintf( stderr, "Could not locate content file: '%s'\n",
                            file.absoluteFilePath().latin1() );
                        fflush( stderr );
                        return 1;
                    }
                    profile->removeDocFileEntry( file.absoluteFilePath() );
                    c->setDocRebuild( TRUE );
                    c->save();
                }
                return 0;
            } else if ( opt == QLatin1String("-hidesidebar") ) {
                hideSidebar = TRUE;
            } else if ( opt == QLatin1String("-help") ) {
                QString helpText = QLatin1String( "Usage: assistant [option]\n"
                                  "Options:\n"
                                  " -file Filename             assistant opens the specified file\n"
                                  " -server                    reads commands from a socket after\n"
                                  "                            assistant has started\n"
                                  " -profile fileName          starts assistant and displays the\n"
                                  "                            profile specified in the file fileName.\n"
                                  " -addContentFile file       adds the content file 'file' to the set of\n"
                                  "                            documentation available by default\n"
                                  " -removeContentFile file    removes the content file 'file' from the\n"
                                  "                            documentation available by default\n"
                                  " -hideSidebar               assistant will hide the sidebar.\n"
                                  " -help                      shows this help.");
#ifdef Q_WS_WIN
                QMessageBox::information( 0, QLatin1String("Qt Assistant"), 
                    QLatin1String("<pre>") + helpText + QLatin1String("</pre>") );
#else
                printf( "%s\n", helpText.latin1() );
#endif
                exit( 0 );
            } else if ( opt == QLatin1String("-resourcedir") ) {
                INDEX_CHECK( "Missing resource directory argument!" );
                resourceDir = QFile::decodeName( argv[++i] );
            } else {
                fprintf( stderr, "Unrecognized option '%s'. Try -help to get help.\n",
                         argv[i] );
                fflush( stderr );
            }
        }
    }

    if( resourceDir.isNull() )
        resourceDir = QFile::decodeName(qInstallPathTranslations());

    QTranslator translator( 0 );
    translator.load( QLatin1String("assistant_") + QLatin1String(QTextCodec::locale()), resourceDir );
    a.installTranslator( &translator );

    QTranslator qtTranslator( 0 );
    qtTranslator.load( QLatin1String("qt_") + QLatin1String(QTextCodec::locale()), resourceDir );
    a.installTranslator( &qtTranslator );

    Config *conf = Config::loadConfig( profileName );
    if ( !conf ) {
        fprintf( stderr, "Profile '%s' does not exist!\n", profileName.latin1() );
        fflush( stderr );
        return -1;
    }

    bool max = conf->isMaximized();
    QStringList links = conf->source();
    conf->hideSideBar( hideSidebar );

    QPointer<MainWindow> mw = new MainWindow();
    mw->setObjectName(QLatin1String("Assistant"));

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

    if ( !file.isEmpty() )
        mw->showLink( file );
    else if ( file.isEmpty() )
        mw->showLinks( links );

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    int appExec = a.exec();
    delete (MainWindow*)mw;
    return appExec;
}

#include "main.moc"
