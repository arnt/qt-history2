/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#23 $
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

#define LINUX_MOUSE
#ifdef LINUX_MOUSE
//mouse stuff:
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
static const char *mouseDev = "/dev/mouse";
static const int mouseBufSize = 100;
#endif

static int SWIDTH=640;
static int SHEIGHT=480;

static int get_object_id()
{
    static int next=1000;
    return next++;
}


/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/
//### shmid < 0 means use frame buffer
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
    header.fbid = shmid < 0 ? 0 : -1; //### always use FB 0
    writeBlock((char*)&header,sizeof(header));

    // Send some objects - client process probably wants some
    QWSCreationEvent event;
    event.type = QWSEvent::Creation;
    for (int i=0; i<10; i++) {
	event.objectid = get_object_id();
	writeBlock( (char*)&event, sizeof(event) );
    }

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
    flush();
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
	rectangle.y = r[i].y();
	rectangle.width = r[i].width();
	rectangle.height = r[i].height();
	writeBlock( (char*)&rectangle, sizeof(rectangle) );
    }
    flush();
}

void QWSClient::sendPropertyNotifyEvent( int property, int state )
{
    QWSPropertyNotifyEvent event;
    event.type = QWSEvent::PropertyNotify;
    event.window = 0; // not used yet
    event.property = property;
    event.state = state;
    writeBlock( (char*)&event, sizeof( event ) );
    flush();
}

void QWSClient::sendPropertyReplyEvent( int property, int len, char *data )
{
    QWSPropertyReplyEvent event;
    event.type = QWSEvent::PropertyReply;
    event.window = 0; // not used yet
    event.property = property;
    event.len = len;
    writeBlock( (char*)&event, sizeof( event ) - sizeof( event.data ) );
    if ( len > 0 )
	writeBlock( data, len );
    flush();
}

void QWSClient::sendSelectionClearEvent( int windowid )
{
    QWSSelectionClearEvent event;
    event.type = QWSEvent::SelectionClear;
    event.window = windowid;
    writeBlock( (char*)&event, sizeof( event ) );
    flush();
}

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

QWSServer::QWSServer( bool fake, QObject *parent=0, const char *name=0 ) :
    QServerSocket(QTFB_PORT,parent,name),
    mouseBuf(0), pending_region_acks(0)
{
    if ( fake ) {
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
    } else {
	shmid = -1; //let client do all FB handling.
#ifdef LINUX_MOUSE
	if ((mouseFD = open( mouseDev, O_RDWR | O_NDELAY)) < 0) {
	    printf( "Cannot open %s (%s)\n", (const char*)mouseDev,
		    strerror(errno));
	    exit(1);
	}
	mouseBuf = new uchar[mouseBufSize];
	mouseIdx = 0;
	mouseX = 500;
	mouseY = 300;
	QSocketNotifier *sn = new QSocketNotifier( mouseFD,
						   QSocketNotifier::Read,
						   this );
	connect( sn, SIGNAL(activated(int)),this, SLOT(readMouseData()) );
#endif


    }

    // no selection yet
    selectionOwner.windowid = -1;
    selectionOwner.time.set( -1, -1, -1, -1 );
    
    if ( !start() )
	qFatal("Failed to bind to port %d",QTFB_PORT);
}

QWSServer::~QWSServer()
{
    // XXX destroy all clients
    if ( mouseBuf )
	delete[] mouseBuf; //??? is delete[] 0 safe?
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
	    case QWSCommand::GetProperty:
		command = new QWSGetPropertyCommand;
		break;
	    case QWSCommand::SetSelectionOwner:
		command = new QWSSetSelectionOwnerCommand;
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
	case QWSCommand::GetProperty:
	    invokeGetProperty( (QWSGetPropertyCommand*)command, client );
	    break;
	case QWSCommand::SetSelectionOwner:
	    invokeSetSelectionOwner( (QWSSetSelectionOwnerCommand*)command );
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

void QWSServer::sendPropertyNotifyEvent( int property, int state )
{
    for ( ClientIterator it = client.begin(); it != client.end(); ++it )
	( *it )->sendPropertyNotifyEvent( property, state );
}

void QWSServer::invokeCreate( QWSCreateCommand *, QWSClient *client )
{
    qDebug( "QWSServer::invokeCreate" );
    QWSCreationEvent event;
    event.type = QWSEvent::Creation;
    event.objectid = get_object_id();
    client->writeBlock( (char*)&event, sizeof(event) );
    client->flush();
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
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, client);
    if ( !changingw ) {
	qWarning("Invalue window handle %08x",cmd->simpleData.windowid);
	return;
    }
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
    if ( properties()->setProperty( cmd->simpleData.windowid,
				    cmd->simpleData.property,
				    cmd->simpleData.mode,
				    cmd->data,
				    cmd->rawLen ) ) {
	qDebug( "setting property successful" );
	sendPropertyNotifyEvent( cmd->simpleData.property,
				 QWSEvent::PropertyNewValue );
   } else
 	qDebug( "setting property failed" );
}

void QWSServer::invokeRemoveProperty( QWSRemovePropertyCommand *cmd )
{
    qDebug( "QWSServer::invokeRemoveProperty %d %d", cmd->simpleData.windowid,
	    cmd->simpleData.property );
    if ( properties()->removeProperty( cmd->simpleData.windowid,
				       cmd->simpleData.property ) ) {
 	qDebug( "remove property successful" );
	sendPropertyNotifyEvent( cmd->simpleData.property,
				 QWSEvent::PropertyDeleted );
    } else
 	qDebug( "removing property failed" );
}

void QWSServer::invokeGetProperty( QWSGetPropertyCommand *cmd, QWSClient *client )
{
    qDebug( "QWSServer::invokeGetProperty %d %d", cmd->simpleData.windowid,
	    cmd->simpleData.property );
    char *data;
    int len;
    if ( properties()->getProperty( cmd->simpleData.windowid,
				    cmd->simpleData.property,
				    data, len ) ) {
 	qDebug( "get property successful" );
	client->sendPropertyReplyEvent( cmd->simpleData.property, len, data );
	delete [] data;
    } else {
 	qDebug( "get property failed" );
	client->sendPropertyReplyEvent( cmd->simpleData.property, -1, 0 );
	delete [] data;
    }
}

void QWSServer::invokeSetSelectionOwner( QWSSetSelectionOwnerCommand *cmd )
{
    qDebug( "QWSServer::invokeSetSelectionOwner" );
    
    SelectionOwner so;
    so.windowid = cmd->simpleData.windowid;
    so.time.set( cmd->simpleData.hour, cmd->simpleData.minute,
		 cmd->simpleData.sec, cmd->simpleData.ms );

    if ( selectionOwner.windowid != -1 ) {
	QWSWindow *win = findWindow( selectionOwner.windowid, 0 );
	if ( win )
	    win->client->sendSelectionClearEvent( selectionOwner.windowid );
	else
	    qDebug( "couldn't find window %d", selectionOwner.windowid );
    }
    
    selectionOwner = so;
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

QWSWindow* QWSServer::newWindow(int id, QWSClient* client)
{
    // Make a new window, put it on top.
    QWSWindow* w = new QWSWindow(id,client);
    windows.prepend(w);
    return w;
}

QWSWindow* QWSServer::findWindow(int windowid, QWSClient* client)
{
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w->winId() == windowid )
	    return w;
    }
    if ( client )
	return newWindow(windowid,client);
    else
	return 0;
}

/*!
  Changes the requested region of window \a windowid to \a r,
  sends appropriate region change events to all appropriate
  clients, and waits for all required acknowledgements.
*/
void QWSServer::setWindowRegion(QWSWindow* changingw, QRegion r)
{
    qDebug("setWindowRegion");

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
	    if ( r.isEmpty() ) {
		break; // Nothing left for deeper windows
	    }
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

/*
  mouseIdx is the number of bytes in the buffer (aka the first free
  position). handleMouseData() moves any data it doesn't use to
  the beginning of the buffer, and updates mouseIdx.
 */

void QWSServer::readMouseData()
{
#ifdef LINUX_MOUSE
    int n;
    do {
	n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx );
	if ( n > 0 ) {
	    mouseIdx += n;
	    handleMouseData();
	}
    } while ( n > 0 );
#endif
}




/*
  This implements the Logitech MouseMan(Plus) protocol,
  wheel not yet supported.
*/

void QWSServer::handleMouseData()
{
    static const int screen_width = 1024; //#####
    static const int screen_height = 768; //#####
    static const int accel_limit = 5;
    static const int accel = 2;


    //    printf( "handleMouseData mouseIdx=%d\n", mouseIdx );


    int idx = 0;

    while ( mouseIdx-idx >= 3 ) {
	int bstate = 0;
	uchar *mb = mouseBuf+idx;

	if (mb[0] & 0x01)
	    bstate |= Qt::LeftButton;
	if (mb[0] & 0x02)
	    bstate |= Qt::RightButton;
	if (mb[0] & 0x04)
	    bstate |= Qt::MidButton;

	int overflow = (mb[0]>>6 )& 0x03;
	//### wheel events signalled with overflow bit, ignore for now
	int dx,dy;
	if ( !overflow ) {
	    bool xs = mb[0] & 0x10;
	    bool ys = mb[0] & 0x20;

	    dx = xs ? mb[1]-256 : mb[1];
	    dy = ys ? mb[2]-256 : mb[2];
	    if ( QABS(dx) > accel_limit || QABS(dy) > accel_limit ) {
		dx *= accel;
		dy *= accel;
	    }
	    mouseX += dx;
	    mouseY -= dy; // swap coordinate system

	    mouseX = QMIN( QMAX( mouseX, 0 ), screen_width );
	    mouseY = QMIN( QMAX( mouseY, 0 ), screen_height );


	    sendMouseEvent( QPoint(mouseX,mouseY), bstate );
	}
	idx += 3;


#if 0 //debug
	const char *b1 = (mb[0] & 0x01) ? "b1":"  ";//left
	const char *b2 = (mb[0] & 0x02) ? "b2":"  ";//right
	const char *b3 = (mb[0] & 0x04) ? "b3":"  ";//mid


	printf( "(%2d) %02x %02x %02x ", idx, mb[0],mb[1],mb[2] );


	if ( overflow )
	    printf( "Overflow%d %s %s %s  (%4d,%4d)\n", overflow,
		    b1, b2, b3, mouseX, mouseY );
	else
	    printf( "%s %s %s (%+3d,%+3d)  (%4d,%4d)\n",
		    b1, b2, b3, dx, dy, mouseX, mouseY );
#endif
    }

    int surplus = mouseIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	mouseBuf[i] = mouseBuf[idx+i];
    mouseIdx = surplus;


    //printf( "exit handleMouseData mouseIdx=%d\n", mouseIdx );


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
	    server = new QWSServer( TRUE, this );
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

    bool useGUI = getenv( "DISPLAY" ) != 0;

    QApplication app(argc, argv, useGUI);

    if ( argc > 1 ) {
	refresh_delay = atoi(argv[1]);
    }

    if ( useGUI ) {
	Main *m = new Main;
	app.setMainWidget(m);
	m->serve(refresh_delay);
	m->show();
    } else {
	(void)new QWSServer;
    }
    return app.exec();
}
