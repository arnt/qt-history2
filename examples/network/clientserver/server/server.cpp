/****************************************************************************
** $Id: //depot/qt/main/examples/clientserver/server/server.cpp#1 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qsocket.h>
#include <qserversocket.h>
#include <qapplication.h>
#include <qvbox.h>
#include <qtextview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>

#include <stdlib.h>


/*
  The ClientSocket class provides a socket that is connected with a client.
  For every client that connects to the server, the server creates a new
  instance of this class.
*/
class ClientSocket : public QSocket
{
    Q_OBJECT
public:
    ClientSocket( int sock, QObject *parent=0, const char *name=0 ) :
	QSocket( parent, name )
    {
	line = 0;
	connect( this, SIGNAL(readyRead()), SLOT(readClient()) );
	connect( this, SIGNAL(connectionClosed()), SLOT(connectionClosed()) );
	setSocket( sock );
    }

    ~ClientSocket()
    {
    }

private slots:
    void readClient()
    {
	while ( canReadLine() ) {
	    QTextStream os( this );
	    os << line << ": " << readLine();
	    line++;
	}
    }

    void connectionClosed()
    {
	delete this;
    }

private:
    int line;
};


/*
  The SimpleServer class handles new connections to the server. For every
  client that connects, it creates a new ClientSocket -- that instance is now
  responsible for the communication with that client.
*/
class SimpleServer : public QServerSocket
{
    Q_OBJECT
public:
    SimpleServer( QObject* parent=0 ) :
	QServerSocket( 4242, 0, parent )
    {
	if ( !ok() ) {
	    qWarning("Failed to bind to port 4242");
	    exit(1);
	}
    }

    ~SimpleServer()
    {
    }

    void newConnection( int socket )
    {
	(void)new ClientSocket( socket, this );
	emit newConnect();
    }

signals:
    void newConnect();
};


/*
  The ServerInfo class provides a small GUI for the server. It also creates the
  SimpleServer and as a result the server.
*/
class ServerInfo : public QVBox
{
    Q_OBJECT
public:
    ServerInfo()
    {
	SimpleServer *server = new SimpleServer( this );

	QString itext = QString(
		"This is a small server example.\n"
		"Connect with the client now."
		);
	QLabel *lb = new QLabel( itext, this );
	lb->setAlignment( AlignHCenter );
	infoText = new QTextView( this );
	QPushButton *quit = new QPushButton( "Quit" , this );

	connect( server, SIGNAL(newConnect()), SLOT(newConnect()) );
	connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
    }

    ~ServerInfo()
    {
    }

private slots:
    void newConnect()
    {
	infoText->append( "New connection\n" );
    }

private:
    QTextView *infoText;
};


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    ServerInfo info;
    app.setMainWidget( &info );
    info.show();
    return app.exec();
}

#include "server.moc"
