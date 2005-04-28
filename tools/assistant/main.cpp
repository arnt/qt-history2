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

#include <qtcpserver.h>
#include <qtcpsocket.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qpointer.h>
#include <stdlib.h>
#include <stdio.h>
#include <qtranslator.h>
#include <qlocale.h>
#include <qlibraryinfo.h>

#ifdef Q_WS_WIN
#define INDEX_CHECK( text ) if( i+1 >= argc ) { QMessageBox::information( 0, "Qt Assistant", text ); return 1; }
#else
#define INDEX_CHECK( text ) if( i+1 >= argc ) { fprintf( stderr, text "\n" ); return 1; }
#endif


class AssistantSocket : public QTcpSocket
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


class AssistantServer : public QTcpServer
{
    Q_OBJECT
public:
    AssistantServer( QObject* parent = 0 );
    quint16 getPort() const;

signals:
    void showLinkRequest( const QString& );
    void newConnect();

public slots:
    virtual void incomingConnection( int socket );

private:
    quint16 p;
};


AssistantSocket::AssistantSocket( int sock, QObject *parent )
    : QTcpSocket( parent )
{
    connect( this, SIGNAL(readyRead()), SLOT(readClient()) );
    connect( this, SIGNAL(disconnected()), SLOT(connectionClosed()) );
    setSocketDescriptor( sock );
}

void AssistantSocket::readClient()
{
    QString link = QString();
    while ( canReadLine() )
        link = readLine();
    if ( !link.isNull() ) {
        link = link.replace(QLatin1String("\n"), QLatin1String(""));
        link = link.replace(QLatin1String("\r"), QLatin1String(""));
        QFileInfo fi(link);
        link = fi.absoluteFilePath();
        emit showLinkRequest( link );
    }
}

void AssistantSocket::connectionClosed()
{
    deleteLater();
}

AssistantServer::AssistantServer( QObject *parent )
    : QTcpServer( parent )
{
    listen(QHostAddress::LocalHost, 0);
    if ( !isListening() ) {
        QMessageBox::critical( 0, tr( "Qt Assistant" ),
                tr( "Failed to bind to port %1" ).arg( serverPort() ) );
        exit( 1 );
    }
    p = serverPort();
}

quint16 AssistantServer::getPort() const
{
    return p;
}

void AssistantServer::incomingConnection( int socket )
{
    AssistantSocket *as = new AssistantSocket( socket, this );
    connect( as, SIGNAL(showLinkRequest(QString)),
             this, SIGNAL(showLinkRequest(QString)) );
    emit newConnect();
}

int main( int argc, char ** argv )
{
    bool withGUI = true;
    if ( argc > 1 ) {
        QString arg = QString::fromAscii(argv[1]);
        arg = arg.toLower();
        if ( arg == QLatin1String("-addcontentfile")
            || arg == QLatin1String("-removecontentfile")
#ifndef Q_WS_WIN
            || arg == QLatin1String("-help")
#endif
            )
            withGUI = false;
    }
    QApplication a(argc, argv, withGUI);
    a.setOrganizationName("Trolltech");
    a.setApplicationName("Assistant");

    QString resourceDir;
    AssistantServer *as = 0;
    QStringList catlist;
    QString file, profileName, aDocPath;
    bool server = false;
    bool hideSidebar = false;
    if ( argc == 2 ) {
        if ( (argv[1])[0] != '-' ) {
            file = QString::fromUtf8(argv[1]);
            QFileInfo fi(file);
            file = fi.absoluteFilePath();
            file = MainWindow::urlifyFileName(file);
        }
    }
    if ( file.isEmpty() ) {
        for ( int i = 1; i < argc; i++ ) {
            QString opt = QString::fromAscii(argv[i]).toLower();
            if ( opt == QLatin1String("-file") ) {
                INDEX_CHECK( "Missing file argument!" );
                i++;
                file = QFile::decodeName(argv[i]);
            } else if ( opt == QLatin1String("-server") ) {
                server = true;
            } else if ( opt == QLatin1String("-profile") ) {
                INDEX_CHECK( "Missing profile argument!" );
                profileName = QFile::decodeName(argv[++i]);
            } else if ( opt == QLatin1String("-addcontentfile") ) {
                INDEX_CHECK( "Missing content file!" );
                Config *c = Config::loadConfig( QString() );
                QFileInfo file( QFile::decodeName(argv[i+1]) );
                if( !file.exists() ) {
                    fprintf( stderr, "Could not locate content file: '%s'\n",
                             file.absoluteFilePath().toLatin1().constData() );
                    fflush( stderr );
                    return 1;
                }
                DocuParser *parser = DocuParser::createParser( file.absoluteFilePath() );
                if( parser ) {
                    QFile f( QFile::decodeName(argv[i+1]) );
                    if( !parser->parse( &f ) ) {
                        fprintf( stderr, "Failed to parse file: '%s'\n, ",
                                 file.absoluteFilePath().toLatin1().constData() );
                        fflush( stderr );
                        return 1;
                    }
                    parser->addTo( c->profile() );
                    c->setDocRebuild( true );
                    c->save();
                }
                return 0;
            } else if ( opt == QLatin1String("-removecontentfile") ) {
                INDEX_CHECK( "Missing content file!" );
                Config *c = Config::loadConfig( QString() );
                Profile *profile = c->profile();
                QStringList entries = profile->docs.filter(QString::fromAscii(argv[i+1]));
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
                            file.absoluteFilePath().toLatin1().constData() );
                        fflush( stderr );
                        return 1;
                    }
                    profile->removeDocFileEntry( file.absoluteFilePath() );
                    c->setDocRebuild( true );
                    c->save();
                }
                return 0;
            } else if ( opt == QLatin1String("-hidesidebar") ) {
                hideSidebar = true;
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
                printf( "%s\n", helpText.toLatin1().constData() );
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
        resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    QTranslator translator( 0 );
    translator.load( QLatin1String("assistant_") + QLocale::system().name().toLower(), resourceDir );
    a.installTranslator( &translator );

    QTranslator qtTranslator( 0 );
    qtTranslator.load( QLatin1String("qt_") + QLocale::system().name().toLower(), resourceDir );
    a.installTranslator( &qtTranslator );

    Config *conf = Config::loadConfig( profileName );
    if ( !conf ) {
        fprintf( stderr, "Profile '%s' does not exist!\n", profileName.toLatin1().constData() );
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
        printf("%d\n", as->serverPort() );
        fflush( stdout );
        as->connect( as, SIGNAL(showLinkRequest(QString)),
                     mw, SLOT(showLinkFromClient(QString)) );
    }

    if ( max )
        mw->showMaximized();
    else
        mw->show();

    if ( !file.isEmpty() ) {
        mw->showLink( MainWindow::urlifyFileName(file) );
    } else if ( file.isEmpty() )
        mw->showLinks( links );

    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );

    int appExec = a.exec();
    delete (MainWindow*)mw;
    return appExec;
}

#include "main.moc"
