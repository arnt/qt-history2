/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#9 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qws.h"
#include "qwsevent.h"
#include "qwscommand.h"

#include <qapplication.h>
#include <qwidget.h>
#include <qimage.h>
#include <qsocket.h>
#include <qdatetime.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static int SWIDTH=640;
static int SHEIGHT=480;

// make a unique window id
static int get_window_id()
{
    static int win_id = 0;
    return ++win_id;
}

/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

QWSClient::QWSClient( int socket, int shmid ) :
    QSocket(socket),
    s(socket)
{
    QWSHeader header;
    header.width = SWIDTH;
    header.height = SHEIGHT;
    header.depth = 32;
    header.shmid = shmid;
    writeBlock((char*)&header,sizeof(header));
    flush();
}

int QWSClient::socket() const
{
    return s;
}

void QWSClient::sendMouseEvent(const QPoint& pos, int state)
{
    QWSMouseEvent event;
    event.type = QWSEvent::Mouse;
    event.zero_window = 0; // not used yet
    event.x_root=pos.x();
    event.y_root=pos.y();
    event.state=state;
    event.time=timer.elapsed();
    writeBlock((char*)&event,sizeof(event));
}

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

QWSServer::QWSServer( QObject *parent=0, const char *name=0 ) :
    QServerSocket(QTFB_PORT,parent,name)
{
    shmid = shmget(IPC_PRIVATE, SWIDTH*SHEIGHT*sizeof(QRgb),
			IPC_CREAT|IPC_EXCL|0666);
    if ( shmid < 0 )
	perror("Cannot allocate shared memory.  Server already running?");
    framebuffer = (uchar*)shmat( shmid, 0, 0 );
    if ( framebuffer == (uchar*)-1 )
	perror("Cannot attach to shared memory.");
    int e=shmctl(shmid, IPC_RMID, 0);
    if ( e<0 )
	perror("shmctl IPC_RMID");

    if ( !start() )
	qFatal("Failed to bind to port %d",QTFB_PORT);
}

QWSServer::~QWSServer()
{
    // XXX destroy all clients
}

void QWSServer::newConnection( int socket )
{
    qDebug("New client...");
    client[socket] = new QWSClient(socket,shmid);
    connect( client[socket], SIGNAL(readyRead()),
	     this, SLOT(doClient()) );
}

void QWSServer::doClient()
{
    QWSClient* c = (QWSClient*)sender();
    int command_type = qws_read_uint( c );
    QWSCommand *command = QWSCommand::getCommand( (QWSCommand::Type)command_type, this, c );
    if ( !command ) {
	qWarning( "Protocol error - got: %d", command_type );
	return;
    }

    command->readData();
    command->execute();
}

void QWSServer::sendMouseEvent(const QPoint& pos, int state)
{
    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendMouseEvent(pos,state);
}



class Main : public QWidget {
    QImage img;
    QWSServer *server;

public:
    Main() :
	server( 0 )
    {
	resize(SWIDTH,SHEIGHT);
	setMouseTracking(TRUE);
    }

    void serve(int refresh_delay)
    {
	if ( !server ) {
	    setFixedSize(size()); // Allow -geometry to set it, but then freeze.
	    SWIDTH = width();
	    SHEIGHT = width();
	    server = new QWSServer(this);
	    img = QImage( server->frameBuffer(),
			SWIDTH, SHEIGHT, 32, 0, 0, QImage::BigEndian );
	    startTimer(refresh_delay);
	}
    }

    void timerEvent(QTimerEvent*)
    {
	repaint(FALSE);
    }

    void mousePressEvent(QMouseEvent* e)
    {
	sendMouseEvent(e);
    }
    void mouseReleaseEvent(QMouseEvent* e)
    {
	sendMouseEvent(e);
    }
    void mouseMoveEvent(QMouseEvent* e)
    {
	sendMouseEvent(e);
    }

    void sendMouseEvent(QMouseEvent* e)
    {
	server->sendMouseEvent(e->pos(), e->stateAfter());
    }

    void paintEvent(QPaintEvent* e)
    {
	QRect r = e->rect();
	bitBlt(this, r.x(), r.y(), &img, r.x(), r.y(), r.width(), r.height(),
	    OrderedDither);
    }
};

main(int argc, char** argv)
{
    int refresh_delay=500;

    QApplication app(argc, argv);

    if ( argc > 1 ) {
	refresh_delay = atoi(argv[1]);
    }

    qwsRegisterCommands();

    Main m;
    app.setMainWidget(&m);
    m.serve(refresh_delay);
    m.show();

    return app.exec();
}

static ushort hex_ushort_to_int( ushort c )
{
    if ( c >= 'A' && c <= 'F')
	return c - 'A' + 10;
    if ( c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if ( c >= '0' && c <= '9')
	return c - '0';
    return 0;
}

static int hex_to_int( char *array )
{
    return ( 16 * 16 * 16 * hex_ushort_to_int( array[ 0 ] ) +
	     16 * 16 * hex_ushort_to_int( array[ 1 ] ) +
	     16 * hex_ushort_to_int( array[ 2 ] ) + 
	     hex_ushort_to_int( array[ 3 ] ) );
}

int qws_read_uint( QSocket *socket )
{
    if ( !socket )
	return -1;

    int i;
    socket->readBlock( (char*)&i, sizeof( int ) );
    
    return hex_to_int( (char*)&i );
}
