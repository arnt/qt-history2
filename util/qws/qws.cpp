/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#1 $
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

#include "qtfb.h"
#include <qapplication.h>
#include <qwidget.h>
#include <qimage.h>
#include <qsocket.h>
#include <qdatetime.h>

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

class QtFBClient : public QSocket {
    int s; // XXX QSocket::d::socket->socket() is this value
    QDataStream stream;
    QTime timer;

public:
    QtFBClient( int socket, int shmid ) :
	QSocket(socket),
	s(socket),
	stream(this)
    {
	stream.setByteOrder(QDataStream::LittleEndian); // XXX per client
	stream << SWIDTH << SHEIGHT << 32 << shmid;
	stream.device()->flush();
    }

    int socket() const { return s; }

    void sendMouseEvent(const QPoint& pos, int state)
    {
	int window = 0; // not used yet
	int time=timer.elapsed();
	stream << INT8('M') << window << (int)pos.x() << (int)pos.y() << state << time;
	stream.device()->flush();
    }
};

struct NewWindowStruct {
    static void invoke(QtFBClient*)
    {
	qFatal("Not implemented");
    }
    ushort x, y, w, h;
    ushort flags;
};

static struct Command {
    void (*invoke)(QtFBClient*);
    int size;
    char cmd;
} command[] = {
    { &NewWindowStruct::invoke, sizeof(NewWindowStruct), 'N' }
};


QtFBServer::QtFBServer( QObject *parent=0, const char *name=0 ) :
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

QtFBServer::~QtFBServer()
{
    // XXX destroy all clients
}

void QtFBServer::newConnection( int socket )
{
    qDebug("New client...");
    client[socket] = new QtFBClient(socket,shmid);
    connect( client[socket], SIGNAL(readyRead()),
	     this, SLOT(doClient()) );
}

void QtFBServer::doClient()
{
    QtFBClient* c = (QtFBClient*)sender();
    int cmd = c->getch();
    for (int i=0; i<int(sizeof(command)/sizeof(Command)); i++) {
	if ( cmd == command[i].cmd ) {
	    command[i].invoke(c);
	    goto validcmd;
	}
    }
    qWarning("Protocol error - disconnecting client");
    client[c->socket()] = 0;
    delete c;
    return;

validcmd:
    ;
}

void QtFBServer::sendMouseEvent(const QPoint& pos, int state)
{
    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendMouseEvent(pos,state);
}



class Main : public QWidget {
    QImage img;
    QtFBServer server;

public:
    Main() :
	server( this )
    {
	img = QImage( server.frameBuffer(),
	    SWIDTH, SHEIGHT, 32, 0, 0, QImage::BigEndian );

	setMouseTracking(TRUE);
	resize(SWIDTH,SHEIGHT);
	startTimer(500);
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
	server.sendMouseEvent(e->pos(), e->stateAfter());
    }

    void paintEvent(QPaintEvent* e)
    {
	QRect r = e->rect();
	bitBlt(this, 0, 0, &img, r.x(), r.y(), r.width(), r.height(),
	    OrderedDither);
    }
};

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Main m;
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}
