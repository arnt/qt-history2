/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#15 $
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
#include "qwsutils.h"

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
    s(socket),
    command(0)
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

void QWSClient::writeRegion( QRegion reg )
{
    // XXX when the protocol is finalized, we should
    // XXX make it possible to write reg.rects directly
    QArray<QRect> r = reg.rects();
    if ( r.size() == 0 )
	return;
    struct {
        int x, y, width, height;
    } rectangle;
    for (uint i=0; i<r.size(); i++) {
	rectangle.x = r[i].x();
	rectangle.y = r[i].x();
	rectangle.width = r[i].width();
	rectangle.height = r[i].height();
	writeBlock( (char*)&rectangle, sizeof(rectangle) );
    }
}

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

QWSServer::QWSServer( QObject *parent=0, const char *name=0 ) :
    QServerSocket(QTFB_PORT,parent,name),
    pending_region_acks(0)
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

QWSCommand* QWSClient::readMoreCommand()
{
    // read next command
    if ( !command ) {
	int command_type = qws_read_uint( this );

	if ( command_type>=0 ) {
	    switch ( command_type ) {
	    case QWSCommand::Create:
		command = new QWSCreateCommand;
		break;
	    case QWSCommand::Region:
		command = new QWSRegionCommand;
		break;
	    case QWSCommand::AddProperty:
		command = new QWSAddPropertyCommand;
		break;
	    case QWSCommand::SetProperty:
		command = new QWSSetPropertyCommand;
		break;
	    case QWSCommand::RemoveProperty:
		command = new QWSRemovePropertyCommand;
		break;
	    default:
		qDebug( "QWSClient::readMoreCommand() : Protocol error - got %08x!", command_type );
	    }
	}
    }

    if ( command ) {
	if ( command->read(this) ) {
	    // Finished reading a whole command.
	    QWSCommand* result = command;
	    command = 0;
	    return result;
	}
    }

    // Not finished reading a whole command.
    return 0;
}

void QWSServer::doClient()
{
    QWSClient* client = (QWSClient*)sender();
    QWSCommand* command=client->readMoreCommand();
    while ( command ) {
	switch ( command->type ) {
	case QWSCommand::Create:
	    invokeCreate( (QWSCreateCommand*)command, client );
	    break;
	case QWSCommand::Region:
	    invokeRegion( (QWSRegionCommand*)command, client );
	    break;
	case QWSCommand::AddProperty:
	    invokeAddProperty( (QWSAddPropertyCommand*)command );
	    break;
	case QWSCommand::SetProperty:
	    invokeSetProperty( (QWSSetPropertyCommand*)command );
	    break;
	case QWSCommand::RemoveProperty:
	    invokeRemoveProperty( (QWSRemovePropertyCommand*)command );
	    break;
	}

	delete command;

	// Try for some more...
	command=client->readMoreCommand();
    }
}

void QWSServer::sendMouseEvent(const QPoint& pos, int state)
{
    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendMouseEvent(pos,state);
}


static int get_object_id()
{
    static int next=1000;
    return next++;
}

void QWSServer::invokeCreate( QWSCreateCommand *, QWSClient *client )
{
    qDebug( "QWSServer::invokeCreate" );
    QWSCreationEvent event;
    event.type = QWSEvent::Creation;
    event.objectid = get_object_id();
    client->writeBlock( (char*)&event, sizeof(event) );
}

void QWSServer::invokeRegion( QWSRegionCommand *cmd, QWSClient *client )
{
    qDebug( "QWSServer::invokeRegion" );
    QWSRegionCommand::Rectangle *rects = 
	(QWSRegionCommand::Rectangle*)cmd->rectangles;
    // XXX would be much faster to build the region directly
    QRegion region;
    for ( int i = 0; i < cmd->simpleData.nrectangles; ++i ) {
	QRect rc(rects[ i ].x, rects[ i ].y, rects[ i ].width, rects[ i ].height);
	region |= rc;
	QWSRegionCommand::Rectangle r = rects[ i ];
	qDebug( "    rect: %d %d %d %d", r.x, r.y, r.width, r.height );
    }
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid);
    if ( !changingw ) return;
    if ( !changingw->forClient(client) ) {
	qWarning("Disabled: clients changing other client's window region");
	return;
    }
    setWindowRegion( changingw, region );
}

void QWSServer::invokeAddProperty( QWSAddPropertyCommand *cmd )
{
    qDebug( "QWSServer::invokeAddProperty %d %d", cmd->simpleData.windowid,
	    cmd->simpleData.property );
    if ( properties()->addProperty( cmd->simpleData.windowid, cmd->simpleData.property ) )
 	qDebug( "add property successful" );
    else
 	qDebug( "adding property failed" );
}

void QWSServer::invokeSetProperty( QWSSetPropertyCommand *cmd )
{
    qDebug( "QWSServer::invokeSetProperty %d %d %d %s",
	    cmd->simpleData.windowid, cmd->simpleData.property,
	    cmd->simpleData.mode, cmd->data );
    QCString ba( cmd->rawLen );
    ba = cmd->data;
    if ( properties()->setProperty( cmd->simpleData.windowid,
				    cmd->simpleData.property,
				    cmd->simpleData.mode,
				    ba ) )
 	qDebug( "set property successful" );
    else
 	qDebug( "setting property failed" );
}

void QWSServer::invokeRemoveProperty( QWSRemovePropertyCommand *cmd )
{
    qDebug( "QWSServer::invokeRemoveProperty %d %d", cmd->simpleData.windowid,
	    cmd->simpleData.property );
    if ( properties()->removeProperty( cmd->simpleData.windowid, cmd->simpleData.property ) )
 	qDebug( "remove property successful" );
    else
 	qDebug( "removing property failed" );
}


void QWSWindow::addAllocation(QRegion r)
{
    allocated_region |= r;

    QWSRegionAddEvent event;
    event.type = QWSEvent::RegionAdd;
    event.window = id;
    event.nrectangles = r.rects().count(); // XXX MAJOR WASTAGE
    client->writeBlock( (char*)&event, sizeof(event)-sizeof(event.rectangles) );
    client->writeRegion( r );
}

bool QWSWindow::removeAllocation(QRegion r)
{
    QRegion nr = allocated_region - r;
    if ( allocated_region != nr ) {
	allocated_region = nr;

	static int not_used_yet = 0; // protocol doesn't use this.

	QWSRegionRemoveEvent event;
	event.type = QWSEvent::RegionRemove;
	event.eventid = not_used_yet++;;
	event.window = id;
	event.nrectangles = r.rects().count(); // XXX MAJOR WASTAGE
	client->writeBlock( (char*)&event, sizeof(event)-sizeof(event.rectangles) );
	client->writeRegion( r );

	return TRUE; // ack required
    }
    return FALSE;
}

void QWSServer::newWindow(int id, QWSClient* client)
{
    // Make a new window, put it on top.
    QWSWindow* w = new QWSWindow(id,client);
    windows.prepend(w);
}

QWSWindow* QWSServer::findWindow(int windowid)
{
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w->winId() == windowid )
	    return w;
    }
    return 0;
}

/*!
  Changes the requested region of window \a windowid to \a r,
  sends appropriate region change events to all appropriate
  clients, and waits for all required acknowledgements.
*/
void QWSServer::setWindowRegion(QWSWindow* changingw, QRegion r)
{
    QRegion allocation;
    QRegion exposed = changingw->allocation() - r;
    uint windex;

    // First, take the region away from whichever windows currently have it...
    bool deeper = FALSE;
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w == changingw ) {
	    windex = i;
	    if ( w->removeAllocation(exposed) )
		pending_region_acks++;
	    allocation = r;
	    deeper = TRUE;
	} else if ( deeper ) {
	    if ( w->removeAllocation(r) )
		pending_region_acks++;
	    r -= w->allocation();
	} else {
	    r -= w->allocation();
	    if ( r.isEmpty() )
		return; // Nothing left for deeper windows
	}
    }

    // Give the region to the window...
    changingw->addAllocation(allocation);

    // Wait for acknowledgements

    // Then, give anything exposed...
    for (uint i=windex+1; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	w->addAllocation(exposed);
	exposed -= w->allocation();
	if ( exposed.isEmpty() )
	    return; // Nothing left for deeper windows
    }
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

    Main m;
    app.setMainWidget(&m);
    m.serve(refresh_delay);
    m.show();

    return app.exec();
}
