/****************************************************************************
**
** Implementation of Qt/FB central server.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qws.h"
#include "qwsevent.h"
#include "qwscommand.h"
#include "qwsutils.h"
#include "qwsaccel.h"
#include "qws_cursor.h"

#include <qapplication.h>
#include <qpointarray.h> //cursor test code
#include <qimage.h>
#include <qcursor.h>
#include <qgfx.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

const int sharedRamSize=1000000;

static int get_object_id()
{
    static int next=1000;
    return next++;
}

static int semset = 0;
static key_t semkey = 0;

/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/
//### shmid < 0 means use frame buffer
QWSClient::QWSClient( QObject* parent, int socket, int shmid,
		      int swidth, int sheight, int sdepth,
		      int ramid, int fblen, int offscreen, int offscreenlen) :
    QSocket(parent),
    s(socket),
    command(0)
{
    setSocket(socket);
    isClosed = FALSE;
    QWSHeader header;
    header.width = swidth;
    header.height = sheight;
    header.depth = sdepth;
    header.semkey = semkey;
    header.shmid = shmid;
    header.ramid = ramid;
    header.ramlen= sharedRamSize;
    header.fblen = fblen;
    header.offscreen = offscreen;
    header.offscreenlen = offscreenlen;
    header.fbid = shmid < 0 ? 0 : -1; //### always use FB 0
    writeBlock((char*)&header,sizeof(header));

    // Send some objects - client process probably wants some
    QWSCreationEvent event;
    for (int i=0; i<10; i++) {
	event.simpleData.objectid = get_object_id();
	event.write( this );
    }

    flush();

    connect( this, SIGNAL(closed()), this, SLOT(closeHandler()) );
    connect( this, SIGNAL(error(int)), this, SLOT(errorHandler(int)) );
}

QWSClient::~QWSClient()
{
}


void QWSClient::closeHandler()
{
    qDebug( "Client %p closed", this );
    isClosed = TRUE;
    emit connectionClosed();
}

void QWSClient::errorHandler( int err )
{
    QString s = "Unknown";
    switch( err ) {
    case ErrConnectionRefused:
	s = "Connection Refused";
	break;
    case ErrHostNotFound:
	s = "Host Not Found";
	break;
    case ErrSocketRead:
	s = "Socket Read";
	break;
    }
    qDebug( "Client %p error %d (%s)", this, err, s.ascii() );
    isClosed = TRUE;
    flush(); //####We need to clean out the pipes, this in not the the way.
    emit connectionClosed();
}

int QWSClient::socket() const
{
    return s;
}

void QWSClient::sendEvent( QWSEvent* event )
{
    event->write( this );
    flush();
}


static void copyRects( QRegion reg, void *data )
{
    // XXX when the protocol is finalized, we should
    // XXX make it possible to write reg.rects directly
    QMemArray<QRect> ra = reg.rects();
    if ( ra.size() == 0 )
	return;
    QWSRegionRemoveEvent::Rectangle *r=(QWSRegionRemoveEvent::Rectangle*)data;
    for (uint i=0; i<ra.size(); i++) {
	r[i].x = ra[i].x();
	r[i].y = ra[i].y();
	r[i].width = ra[i].width();
	r[i].height = ra[i].height();
#ifdef QWS_REGION_DEBUG
	qDebug( "   writeRegion rect[%d] = (%d,%d) %dx%d", i,
	     r[i].x,r[i].y,r[i].width,r[i].height);
#endif
    }
}


void QWSClient::sendRegionAddEvent( int winid, bool ack, QRegion r )
{
    QWSRegionAddEvent event;
    event.simpleData.window = winid;
    event.simpleData.is_ack = ack;
    event.simpleData.nrectangles = r.rects().count(); // XXX MAJOR WASTAGE
    event.allocateRects( event.simpleData.nrectangles );
#ifdef QWS_REGION_DEBUG
    qDebug("Add region (%d rects) to %p/%d", event.simpleData.nrectangles, c, id);

#endif
    copyRects( r, event.rectangles );
    event.write( this );
    flush();
}

void QWSClient::sendRegionRemoveEvent( int winid, int eventid, QRegion r )
{
    QWSRegionRemoveEvent event;
    event.simpleData.eventid = eventid;
    event.simpleData.window = winid;
    event.simpleData.nrectangles = r.rects().count(); // XXX MAJOR WASTAGE
    event.allocateRects( event.simpleData.nrectangles );
#ifdef QWS_REGION_DEBUG
    qDebug("Remove region (%d rects) from %p/%d", event.simpleData.nrectangles, c, id);
#endif
    copyRects( r, event.rectangles );
    event.write( this );
    flush();
}


void QWSClient::sendPropertyNotifyEvent( int property, int state )
{
    QWSPropertyNotifyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.state = state;
    event.write( this );
    flush();
}

void QWSClient::sendPropertyReplyEvent( int property, int len, char *data )
{
    QWSPropertyReplyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.len = len;
    event.setData( data, len );
    event.write( this );
    flush();
}

void QWSClient::sendSelectionClearEvent( int windowid )
{
    QWSSelectionClearEvent event;
    event.simpleData.window = windowid;
    event.write( this );
    flush();
}

void QWSClient::sendSelectionRequestEvent( QWSConvertSelectionCommand *cmd, int windowid )
{
    QWSSelectionRequestEvent event;
    event.simpleData.window = windowid;
    event.simpleData.requestor = cmd->simpleData.requestor;
    event.simpleData.property = cmd->simpleData.selection;
    event.simpleData.mimeTypes = cmd->simpleData.mimeTypes;
    event.write( this );
    flush();
}

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/


struct QWSCommandStruct
{
    QWSCommandStruct( QWSCommand *c, QWSClient *cl ) :command(c),client(cl){}
    QWSCommand *command;
    QWSClient *client;
};


/*
  INVARIANT: pending_region_acks is sum of the pending_acks
  of the QWSWindow objects in the windows list.

  QWSServer maintains the invariant.
*/

QWSServer::QWSServer( int sw, int sh, int simulate_depth, int flags,
		      QObject *parent, const char *name ) :
    QServerSocket(QTFB_PORT,16,parent,name),
    disablePainting(FALSE), pending_region_acks(0)
{
    focusw = 0;
    mouseGrabber = 0;
    mouseGrabbing = FALSE;
    cursorNeedsUpdate=FALSE;
    cursorPos = QPoint(-1,-1); //no software cursor
    swidth = sw;
    sheight = sh;
    sdepth = simulate_depth;

    semkey = ftok("/dev/fb0",'q');
    semset = semget(semkey,1,IPC_CREAT|0777);//###SEMAPHORE_BUG
    if ( semset == -1 ) {
	perror("Cannot create semaphore for /dev/fb0");
	exit(1);
    }
    int arg = 1;
    semctl(semset,0,SETVAL,arg);
    //    semctl(semset,1,SETVAL,arg); //###SEMAPHORE_BUG

    if ( simulate_depth ) {
	// Simulate card with 8 megs of memory or so
	shmid = shmget(IPC_PRIVATE,/* swidth*sheight*sizeof(QRgb) */
		       8000000,
		       IPC_CREAT|IPC_EXCL|0666);
	fblen=swidth*sheight*simulate_depth/8;
	offscreen=fblen+4;
	offscreenlen=(8000000)-(fblen+4);
	if ( shmid < 0 )
	    perror("Cannot allocate shared memory.  Server already running?");
	framebuffer = (uchar*)shmat( shmid, 0, 0 );
	if ( framebuffer == (uchar*)-1 )
	    perror("Cannot attach to shared memory.");
	int e=shmctl(shmid, IPC_RMID, 0);
	if ( e<0 )
	    perror("shmctl IPC_RMID");
    } else {
	// No spare graphics memory, which is untrue...
	offscreen=0;
	offscreenlen=0;
	shmid = -1; //let client do all FB handling.

	// initialise devices
	if ( !(flags&DisableKeyboard) )
	    openKeyboard();
	openDisplay();
	if ( !(flags&DisableMouse) ) {
	    openMouse();
	    initializeCursor();
	    cursor = QWSCursor::systemCursor(ArrowCursor);
	    setMouse(QPoint(swidth/2, sheight/2), 0);
	}
    }

    // Now we allocate a chunk of main ram for font cache and any
    // other appropriate purposes

    ramlen=sharedRamSize;        // 1 megabyte (ish) of main ram
    ramid=shmget(IPC_PRIVATE,sharedRamSize,IPC_CREAT|IPC_EXCL|0666);
    if(ramid<0) {
	perror("Cannot allocate main ram shared memory\n");
    }
    sharedram=(uchar *)shmat(ramid,0,0);
    if(sharedram==(uchar *)-1) {
	perror("Cannot attach to main ram shared memory\n");
    }
    // Need to zero index count at end of block, might as well zero
    // the rest too
    memset(sharedram,0,sharedRamSize);
    int e=shmctl(ramid,IPC_RMID,0);
    if(e<0)
	perror("shmctl main ram IPC_RMID");

    // no selection yet
    selectionOwner.windowid = -1;
    selectionOwner.time.set( -1, -1, -1, -1 );

    paintBackground( QRegion(0,0,swidth, sheight) );
}

QWSServer::~QWSServer()
{
    // XXX destroy all clients
    closeDisplay();
    closeKeyboard();
    closeMouse();
}

void QWSServer::newConnection( int socket )
{
    qDebug("New client...");
    client[socket] = new QWSClient(this,socket,shmid,swidth,sheight,sdepth,
				   ramid,fblen,offscreen,offscreenlen);

    connect( client[socket]->asQObject(), SIGNAL(readyRead()),
	     this, SLOT(doClient()) );
    connect( client[socket]->asQObject(), SIGNAL(connectionClosed()),
	     this, SLOT(clientClosed()) );
}

void QWSServer::clientClosed()
{
    qDebug( "QWSServer::clientClosed()" );
    QWSClient* cl = (QWSClient*)sender();

    QRegion exposed;
    QPtrListIterator<QWSWindow> it( windows );
    QWSWindow* w;
    while (( w = it.current() )) {
	++it;
	if ( w->forClient(cl) ) {
	    if ( mouseGrabber == w )
		mouseGrabber = 0;
	    if ( focusw == w )
		focusw = 0;
	    exposed += w->allocation();
	    pending_region_acks -= w->pending_acks;
	    windows.removeRef(w);
	    delete w; //windows is not auto-delete
	}
    }
    client.remove( cl->socket() );
    delete cl;

    exposeRegion( exposed );
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
	    case QWSCommand::RegionAck:
		command = new QWSRegionAckCommand;
		break;
	    case QWSCommand::RequestFocus:
		command = new QWSRequestFocusCommand;
		break;
	    case QWSCommand::ChangeAltitude:
		command = new QWSChangeAltitudeCommand;
		break;
	     case QWSCommand::DefineCursor:
		command = new QWSDefineCursorCommand;
		break;
	     case QWSCommand::SelectCursor:
		command = new QWSSelectCursorCommand;
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
    static bool active = FALSE;
    if (active) {
	qDebug( "QWSServer::doClient() reentrant call, ignoring" );
	return;
    }
    active = TRUE;
    QWSClient* client = (QWSClient*)sender();
    QWSCommand* command=client->readMoreCommand();


    while ( command ) {
	if ( command->type == QWSCommand::RegionAck ) {
	    int id = ((QWSRegionAckCommand*)command)->simpleData.eventid;
	    QWSWindow *w = findWindow( id, 0 );
	    if ( w ) {
		w->pending_acks--;
		pending_region_acks--;
	    } else {
		qWarning( "Unexpected ack from client %p/%d, ignored",
			  client, id );
	    }
#ifdef QWS_REGION_DEBUG
	    qDebug( "QWSCommand::RegionAck from %p pending:%d",
		    client, pending_region_acks);
#endif
	    if ( pending_region_acks == 0 ) {
		givePendingRegion();
	    }
	    delete command;
	} else {
	    QWSCommandStruct *cs = new QWSCommandStruct( command, client );
	    commandQueue.enqueue( cs );
	}
	// Try for some more...
	command=client->readMoreCommand();
    }


    while ( pending_region_acks == 0 && !commandQueue.isEmpty() ) {
	QWSCommandStruct *cs = commandQueue.dequeue();
	switch ( cs->command->type ) {
	case QWSCommand::Create:
	    invokeCreate( (QWSCreateCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::Region:
	    invokeRegion( (QWSRegionCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::AddProperty:
	    invokeAddProperty( (QWSAddPropertyCommand*)cs->command );
	    break;
	case QWSCommand::SetProperty:
	    invokeSetProperty( (QWSSetPropertyCommand*)cs->command );
	    break;
	case QWSCommand::RemoveProperty:
	    invokeRemoveProperty( (QWSRemovePropertyCommand*)cs->command );
	    break;
	case QWSCommand::GetProperty:
	    invokeGetProperty( (QWSGetPropertyCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::SetSelectionOwner:
	    invokeSetSelectionOwner( (QWSSetSelectionOwnerCommand*)cs->command );
	    break;
	case QWSCommand::RegionAck:
	    qWarning( "QWSServer::doClient() uncaught RegionAck" );
	    break;
	case QWSCommand::RequestFocus:
	    invokeSetFocus( (QWSRequestFocusCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::ChangeAltitude:
	    invokeSetAltitude( (QWSChangeAltitudeCommand*)cs->command,
			       cs->client );
	    break;
	case QWSCommand::DefineCursor:
	    invokeDefineCursor( (QWSDefineCursorCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::SelectCursor:
	    invokeSelectCursor( (QWSSelectCursorCommand*)cs->command, cs->client );
	    break;

	}
	delete cs->command;
	delete cs;
	if (cursorNeedsUpdate)
	    showCursor();
    }
    active = FALSE;
}


void QWSServer::showCursor()
{
    if(probed_card)
	return;

    if ( pending_region_acks != 0 ) {
	cursorNeedsUpdate = TRUE;
	return;
    }
    cursorNeedsUpdate = FALSE;
    if ( cursorPos == mousePos )
	return;
    cursorPos = mousePos;

/*
    //##### hardcoded region
   QPolygon a;
   a.setPoints(  7,
		 0,0,
		 6,0,
		 4,2,
		 14,12,
		 12,14,
		 2,4,
		 0,6 );
    a.translate( cursorPos.x()-1, cursorPos.y()-1 );
    setWindowRegion( 0, QRegion(a) & QRegion(0,0,swidth,sheight) );
*/

    if ( cursor ) {
	QRegion r( cursor->region() );
	r.translate( cursorPos.x()-1, cursorPos.y()-1 );
	r.translate( -cursor->hotSpot().x(), -cursor->hotSpot().y() );
	setWindowRegion( 0, r & QRegion(0,0,swidth,sheight) );
    }
}

// ### don't like this
void QWSServer::enablePainting(bool e)
{
    if (e)
    {
	disablePainting = FALSE;
	setWindowRegion( 0, QRegion() );
    }
    else
    {
	disablePainting = TRUE;
	setWindowRegion( 0, QRegion(0,0,swidth,sheight) );
    }
}

void QWSServer::refresh()
{
    exposeRegion( QRegion(0,0,swidth,sheight) );
}


void QWSServer::sendMouseEvent(const QPoint& pos, int state)
{
    showCursor();

    if ( mouseGrabbing && state == 0 ) {
	//Note that if a window disappears, mouseGrabbing may be
	//true while mouseGrabber is 0
	mouseGrabbing = FALSE;
	mouseGrabber = 0;
    }

    if ( mouseGrabbing && state == 0 ) {
	//Note that if a window disappears, mouseGrabbing may be
	//true while mouseGrabber is 0
	mouseGrabbing = FALSE;
	mouseGrabber = 0;
    }


    QWSMouseEvent event;

    QWSWindow *win = mouseGrabbing ? mouseGrabber : windowAt( pos );
    event.simpleData.window = win ? win->id : 0;

    if ( !mouseGrabbing && win && state != 0 ) {
	//button has been pressed
	mouseGrabbing = TRUE;
	mouseGrabber = win;
    }
    event.simpleData.x_root=pos.x();
    event.simpleData.y_root=pos.y();
    event.simpleData.state=state;
    event.simpleData.time=timer.elapsed();

    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendEvent( &event );
}


QWSWindow *QWSServer::windowAt( const QPoint& pos )
{
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w->requested_region.contains( pos ) )
	    return w;
    }
    return 0;
}

void QWSServer::sendKeyEvent(int unicode, int modifiers, bool isPress,
  bool autoRepeat)
{
    QWSKeyEvent event;
    event.simpleData.window = focusw ? focusw->winId() : 0;
    event.simpleData.unicode = unicode;
    event.simpleData.modifiers = modifiers;
    event.simpleData.is_press = isPress;
    event.simpleData.is_auto_repeat = autoRepeat;

    for (ClientIterator it = client.begin(); it != client.end(); ++it ) {
	(*it)->sendEvent(&event);
    }
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
    event.simpleData.objectid = get_object_id();
    client->sendEvent( &event );
}

void QWSServer::invokeRegion( QWSRegionCommand *cmd, QWSClient *client )
{
#ifdef QWS_REGION_DEBUG
    qDebug( "QWSServer::invokeRegion %d rects (%d)",
	    cmd->simpleData.nrectangles, cmd->simpleData.windowid );
#endif
    QWSRegionCommand::Rectangle *rects =
	(QWSRegionCommand::Rectangle*)cmd->rectangles;
    // XXX would be much faster to build the region directly
    QRegion region;
    for ( int i = 0; i < cmd->simpleData.nrectangles; ++i ) {
	QRect rc(rects[ i ].x, rects[ i ].y, rects[ i ].width, rects[ i ].height);
	region |= rc;
	QWSRegionCommand::Rectangle r = rects[ i ];
#ifdef QWS_REGION_DEBUG
	qDebug( "    rect: %d %d %d %d", r.x, r.y, r.width, r.height );
#endif
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
    if ( focusw == changingw && region.isEmpty() )
	setFocus(changingw,FALSE);
}


void QWSServer::invokeSetFocus( QWSRequestFocusCommand *cmd, QWSClient *client )
{
    int winId = cmd->simpleData.windowid;
    int gain = cmd->simpleData.flag;

    if ( gain != 0 && gain != 1 ) {
	qWarning( "Only 0(lose) and 1(gain) supported" );
	return;
    }

    QWSWindow* changingw = findWindow(winId, client);

    if ( !changingw->forClient(client) ) {
       qWarning("Disabled: clients changing other client's focus");
	return;
    }

    setFocus(changingw, gain);
}

void QWSServer::setFocus( QWSWindow* changingw, bool gain )
{
    if ( gain ) {
	if ( focusw != changingw ) {
	    if ( focusw ) focusw->focus(0);
	    focusw = changingw;
	    focusw->focus(1);
	}
    } else if ( focusw == changingw ) {
	changingw->focus(0);
	focusw = 0;
	// pass focus to window which most recently got it...
	QWSWindow* bestw=0;
	for (uint i=0; i<windows.count(); i++) {
	    QWSWindow* w = windows.at(i);
	    if ( bestw != changingw &&
		    !w->hidden() &&
		    (!bestw || bestw->focusPriority() < w->focusPriority()) )
		bestw = w;
	}
	if ( !bestw )
	    bestw = changingw; // must be the only one
	focusw = bestw;
	focusw->focus(1);
    }
}

void QWSServer::invokeSetAltitude( QWSChangeAltitudeCommand *cmd,
				   QWSClient *client )
{
    int winId = cmd->simpleData.windowid;
    int alt = cmd->simpleData.altitude;
#if 0
    qDebug( "QWSServer::invokeSetAltitude winId %d alt %d)", winId, alt );
#endif

    if ( alt != 0 && alt != -1 ) {
	qWarning( "Only raise() and lower() supported" );
	return;
    }

    QWSWindow* changingw = findWindow(winId, client);
    if ( !changingw ) {
	qWarning("Invalid window handle %08x", winId);
	return;
    }
    if ( !changingw->forClient(client) ) {
       qWarning("Disabled: clients changing other client's altitude");
	return;
     }
    if ( alt < 0 )
	lowerWindow( changingw, alt );
    else
	raiseWindow( changingw, alt );
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
				 QWSPropertyNotifyEvent::PropertyNewValue );
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
				 QWSPropertyNotifyEvent::PropertyDeleted );
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
	    win->client()->sendSelectionClearEvent( selectionOwner.windowid );
	else
	    qDebug( "couldn't find window %d", selectionOwner.windowid );
    }

    selectionOwner = so;
}

void QWSServer::invokeConvertSelection( QWSConvertSelectionCommand *cmd )
{
    qDebug( "QWSServer::invokeConvertSelection" );

    if ( selectionOwner.windowid != -1 ) {
	QWSWindow *win = findWindow( selectionOwner.windowid, 0 );
	if ( win )
	    win->client()->sendSelectionRequestEvent( cmd, selectionOwner.windowid );
	else
	    qDebug( "couldn't find window %d", selectionOwner.windowid );
    }
}

void QWSServer::invokeDefineCursor( QWSDefineCursorCommand *cmd, QWSClient *client )
{
    int dataLen = cmd->simpleData.height * ((cmd->simpleData.width+7) / 8);

    qDebug( "QWSServer::invokeDefineCursor %d %d (%d)",
	    cmd->simpleData.width, cmd->simpleData.height, dataLen );

    QWSCursor *curs = new QWSCursor( cmd->data, cmd->data + dataLen,
				cmd->simpleData.width, cmd->simpleData.height,
				cmd->simpleData.hotX, cmd->simpleData.hotY);

    client->cursors.insert(cmd->simpleData.id, curs);
}

void QWSServer::invokeSelectCursor( QWSSelectCursorCommand *cmd, QWSClient *client )
{
    int id = cmd->simpleData.id;
//    qDebug( "QWSServer::invokeSelectCursor %d", id);
    cursor = 0;
    if (id <= LastCursor) {
	cursor = QWSCursor::systemCursor(id);
    }
    else {
	QWSCursorMap cursMap = client->cursors;
	QWSCursorMap::Iterator it = cursMap.find(id);
	if (it != cursMap.end()) {
	    cursor = it.data();
	}
	else {
	    cursor = QWSCursor::systemCursor(ArrowCursor);
	}
    }

    // ### set cursor in hardware

    cursorNeedsUpdate = TRUE;
}

/*!
  Adds \a r to the window's allocated region.

  If \a isAck is TRUE the event passed to the client has the \c is_ack
  flag set; ie. the event is in response to a Region command for this
  window.
*/
void QWSWindow::addAllocation( QRegion r, bool isAck )
{
    QRegion added = r & requested_region;
    allocated_region |= added;
    if ( isAck || !added.isEmpty() )
	c->sendRegionAddEvent( id, isAck, added );
}

bool QWSWindow::removeAllocation(QRegion r)
{
    QRegion nr = allocated_region - r;
    if ( allocated_region != nr ) {
	allocated_region = nr;

	//static int not_used_yet = 0; // protocol doesn't use this.
	//##### using window ID for event ID is a hack! reconsider
	c->sendRegionRemoveEvent( id, id, r );

	return TRUE; // ack required
    }
    return FALSE;
}

static int global_focus_time_counter=0;

void QWSWindow::focus(bool get)
{
    qDebug( "QWSWindow::focus %s %d", get?"on":"off", id );
    if ( get )
	last_focus_time = global_focus_time_counter++;
    QWSFocusEvent event;
    event.simpleData.window = id;
    event.simpleData.get_focus = get;
    c->sendEvent( &event );
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




void QWSServer::raiseWindow( QWSWindow *changingw, int )
{
    //change position in list:
    QWSWindow *w = windows.first();
    while ( w ) {
	if ( w == changingw ) {
	    windows.take();
	    windows.prepend( changingw );
	    break;
	}
	w = windows.next();
    }
    setWindowRegion( changingw, changingw->requested_region );
}

void QWSServer::lowerWindow( QWSWindow *changingw, int )
{
    //lower: must remove region from window first.
    QRegion visible;
    visible = changingw->allocation();
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w != changingw )
	    visible = visible - w->requested_region;
	if ( visible.isEmpty() )
	    break; //widget will be totally hidden;
    }
    qDebug( "lower: %p, %d, %d", changingw, changingw->winId(),
	    visible.rects().count() );
    QRegion exposed = changingw->allocation() - visible;

    //the exposed region comes from changingw, so either we have to
    //wait for acks from changingw, or we don't have to do anything
    if ( changingw->removeAllocation( exposed )  ) {
	Q_ASSERT( !pending_region_acks );
	changingw->pending_acks++;
	pendingRegion = exposed;
	pending_region_acks++;
	pendingAllocation = QRegion();
	pendingWindex = -1; //slight hack, will repaint server region
    }

    //change position in list:

    QWSWindow *w = windows.first();
    while ( w ) {
	if ( w == changingw ) {
	    windows.take();
	    windows.append( changingw );
	    break;
	}
	w = windows.next();
    }
}

/*!
  Changes the requested region of window \a changingw to \a r,
  sends appropriate region change events to all appropriate
  clients, and waits for all required acknowledgements.

  If \a changingw is 0, the server's reserved region is changed.
  If \a onlyAllocate is TRUE, the requested region is not changed, only
  the allocated region. Be careful using this option, it is only really
  useful if the windows list changes.
*/
void QWSServer::setWindowRegion(QWSWindow* changingw, QRegion r )
{
#ifdef QWS_REGION_DEBUG
    qDebug("setWindowRegion %d", changingw ? changingw->winId() : -1 );
#endif

    QRegion exposed;
    if (changingw) {
	changingw->requested_region = r;
	r = r - serverRegion;
	exposed = changingw->allocation() - r;
    } else {
	exposed = serverRegion-r;
	serverRegion = r;
    }
    QRegion allocation;
    int windex = -1;


    // First, take the region away from whichever windows currently have it...

    if ( changingw && changingw->removeAllocation(exposed) ) {
	changingw->pending_acks++;
	pending_region_acks++;
    }

    bool deeper = changingw == 0;;
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w == changingw ) {
	    windex = i;
	    allocation = r;
	    deeper = TRUE;
	} else if ( deeper ) {
	    if ( w->removeAllocation(r) ) {
		w->pending_acks++;
		pending_region_acks++;
	    }
	    r -= w->allocation();
	} else {
	    r -= w->allocation();
	}
	if ( r.isEmpty() ) {
	    break; // Nothing left for deeper windows
	}
    }

    // The region to give to the window:
    pendingAllocation = allocation;

    // Wait for acknowledgements if pending_region_acks > 0
    // otherwise do it straight away.

    pendingWindex = windex;
    pendingRegion = exposed;

    if ( pending_region_acks == 0 )
	givePendingRegion();

}

void QWSServer::exposeRegion( QRegion r )
{
    for (uint i=0; i<windows.count(); i++) {
	if ( r.isEmpty() )
	    return; // Nothing left for deeper windows
	QWSWindow* w = windows.at(i);
	w->addAllocation( r );
	r -= w->allocation();
    }
    paintBackground( r );
}

void QWSServer::givePendingRegion()
{
    QRegion exposed = pendingRegion;
    // Finally, give anything exposed...

    if ( pendingWindex >= 0 ) {
	QWSWindow* changingw = windows.at( pendingWindex );
	Q_ASSERT( changingw );
	changingw->addAllocation( pendingAllocation, TRUE );
    } else if (!disablePainting) {
	paintServerRegion();
    }
    for (uint i=pendingWindex+1; i<windows.count(); i++) {
	if ( exposed.isEmpty() )
	    return; // Nothing left for deeper windows
	QWSWindow* w = windows.at(i);
	w->addAllocation(exposed);
	exposed -= w->allocation();
    }
    paintBackground( exposed );
}

void QWSServer::setMouse(const QPoint& p,int bstate)
{
    mousePos.setX( qMin( qMax( p.x(), 0 ), swidth ) );
    mousePos.setY( qMin( qMax( p.y(), 0 ), sheight ) );
    if (probed_card) {
	probed_card->move_cursor(mousePos.x(), mousePos.y());
    }
    sendMouseEvent( mousePos, bstate );
    if ( mousePos == QPoint(0,0) && bstate==7 )
	qApp->quit();
}
