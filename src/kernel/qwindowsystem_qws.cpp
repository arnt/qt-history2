/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.cpp#4 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#include "qwindowsystem_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsutils_qws.h"
#include "qwscursor_qws.h"
#include "qwsdisplay_qws.h"

#include <qapplication.h>
#include <qpointarray.h> //cursor test code
#include <qimage.h>
#include <qcursor.h>
#include <qgfx_qws.h>
#include <qwindowdefs.h>
#include <qlock_qws.h>
#include <qwsregionmanager_qws.h>
#include <qqueue.h>

#include <qpen.h>


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#include <qgfx_qws.h>

extern void qt_init_display(); //qapplication_qws.cpp

extern void qt_client_enqueue(const QWSEvent *); //qapplication_qws.cpp
typedef void MoveRegionF( const QWSRegionMoveCommand*);
typedef void RequestRegionF( int, QRegion );
typedef void SetAltitudeF( const QWSChangeAltitudeCommand* );
extern QQueue<QWSCommand> *qt_get_server_queue();




//const int sharedRamSize=4*1024*1024; // SHMMAX
const int sharedRamSize=32*1024; // Small amount to fit on small devices.

static int get_object_id()
{
    static int next=1000;
    return next++;
}


//#define QWS_REGION_DEBUG


/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/
//always use frame buffer
QWSClient::QWSClient( QObject* parent, int socket ) 
    : QObject( parent), s(socket), command(0)
{
    if ( socket == -1 ) {
	csocket = 0;
	isClosed = FALSE;
	
    } else {
	csocket = new QSocket( this );
	csocket->setSocket(socket,FALSE);
	isClosed = FALSE;

	// Send some objects - client process probably wants some
	QWSCreationEvent event;
	for (int i=0; i<10; i++) {
	    event.simpleData.objectid = get_object_id();
	    event.write( csocket );
	}

	csocket->flush();

	connect( csocket, SIGNAL(readyRead()), this, SIGNAL(readyRead()) );
	connect( csocket, SIGNAL(closed()), this, SLOT(closeHandler()) );
	connect( csocket, SIGNAL(error(int)), this, SLOT(errorHandler(int)) );
    }
}

QWSClient::~QWSClient()
{
}


void QWSClient::closeHandler()
{
    isClosed = TRUE;
    emit connectionClosed();
}

void QWSClient::errorHandler( int err )
{
    QString s = "Unknown";
    switch( err ) {
    case QSocket::ErrConnectionRefused:
	s = "Connection Refused";
	break;
    case QSocket::ErrHostNotFound:
	s = "Host Not Found";
	break;
    case QSocket::ErrSocketRead:
	s = "Socket Read";
	break;
    }
    qDebug( "Client %p error %d (%s)", this, err, s.ascii() );
    isClosed = TRUE;
    if ( csocket )
	csocket->flush(); //####We need to clean out the pipes, this in not the the way.
    emit connectionClosed();
}

int QWSClient::socket() const
{
    return s;
}

void QWSClient::sendEvent( QWSEvent* event )
{
    if ( csocket ) {
	event->write( csocket );
	csocket->flush();
    } else {
	qt_client_enqueue( event );
    }
}

void QWSClient::sendRegionModifyEvent( int winid, QRegion exposed, bool ack )
{
    QWSRegionModifiedEvent event;
    event.simpleData.window = winid;
    event.simpleData.nrectangles = exposed.rects().count();
    event.simpleData.is_ack = ack;
    event.setData( (char *)exposed.rects().data(),
		    exposed.rects().count() * sizeof(QRect), FALSE );

    sendEvent( &event );
}

#ifndef QT_NO_QWS_PROPERTIES
void QWSClient::sendPropertyNotifyEvent( int property, int state )
{
    QWSPropertyNotifyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.state = state;
    sendEvent( &event );
}

void QWSClient::sendPropertyReplyEvent( int property, int len, char *data )
{
    QWSPropertyReplyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.len = len;
    event.setData( data, len );
    sendEvent( &event );
}
#endif //QT_NO_QWS_PROPERTIES
void QWSClient::sendSelectionClearEvent( int windowid )
{
    QWSSelectionClearEvent event;
    event.simpleData.window = windowid;
    sendEvent( &event );
}

void QWSClient::sendSelectionRequestEvent( QWSConvertSelectionCommand *cmd, int windowid )
{
    QWSSelectionRequestEvent event;
    event.simpleData.window = windowid;
    event.simpleData.requestor = cmd->simpleData.requestor;
    event.simpleData.property = cmd->simpleData.selection;
    event.simpleData.mimeTypes = cmd->simpleData.mimeTypes;
    sendEvent( &event );
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




static void ignoreSignal( int )
{
}


/*
  INVARIANT: pending_region_acks is sum of the pending_acks
  of the QWSWindow objects in the windows list.

  QWSServer maintains the invariant.
*/

QWSServer::QWSServer( int flags,
		      QObject *parent, const char *name ) :
    QServerSocket(QTE_PIPE,16,parent,name),
    disablePainting(false)
{
    ASSERT( !qwsServer );
    qwsServer = this;

    if ( !ok() ) {
	perror("Error");
	qWarning("Failed to bind to %s", QTE_PIPE );
    } else {
	struct linger tmp;
	tmp.l_onoff=1;
	tmp.l_linger=0;
	setsockopt(socket(),SOL_SOCKET,SO_LINGER,(char *)&tmp,sizeof(tmp));
    }

    signal(SIGPIPE, ignoreSignal); //we get it when we read

    
    focusw = 0;
    mouseGrabber = 0;
    mouseGrabbing = FALSE;
    cursorNeedsUpdate=FALSE;
    cursorPos = QPoint(-1,-1); //no software cursor
    nextCursor = 0;

    if ( !geteuid() ) {
	if(mount(0,"/var/shm","shm",0,0)) {
	    if ( errno != EBUSY )
		qDebug("Failed mounting shm fs on /var/shm: %s",strerror(errno));
	}
    }

    if ( !QWSDisplay::initLock( "/dev/fb0", TRUE ) )
	qFatal( "Cannot get display lock" );

    openDisplay();

    rgnMan = new QWSRegionManager( FALSE );

    // Now we allocate a chunk of main ram for font cache and any
    // other appropriate purposes

    ramlen=sharedRamSize;        // 1 megabyte (ish) of main ram

    key_t memkey = ftok( "/dev/fb0", 'm' );
    int ramid=shmget(memkey,sharedRamSize,IPC_CREAT|0666);
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
    //    int e=shmctl(ramid,IPC_RMID,0);  //############
    //    if(e<0)
    //	perror("shmctl main ram IPC_RMID");

    // no selection yet
    selectionOwner.windowid = -1;
    selectionOwner.time.set( -1, -1, -1, -1 );

    // input devices
    if ( !(flags&DisableMouse) ) {
	openMouse();
	initializeCursor();
    }

    
    if ( !(flags&DisableKeyboard) ) {
	openKeyboard();
    }

    screenRegion = QRegion( 0, 0, swidth, sheight );
    paintBackground( screenRegion );

    qt_init_display();
    client[-1] = new QWSClient( this, -1 );
}

QWSServer::~QWSServer()
{
    // destroy all clients
    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	delete *it;
    delete rgnMan;
    closeDisplay();
    closeMouse();
    closeKeyboard();
}

void QWSServer::newConnection( int socket )
{
    client[socket] = new QWSClient(this,socket);
    connect( client[socket], SIGNAL(readyRead()),
	     this, SLOT(doClient()) );
    connect( client[socket], SIGNAL(connectionClosed()),
	     this, SLOT(clientClosed()) );

    // pre-provide some object id's
    for (int i=0; i<20; i++)
	invokeCreate(0,client[socket]);
}

void QWSServer::clientClosed()
{
    QWSClient* cl = (QWSClient*)sender();

    // Remove any queued commands for this client
    QListIterator<QWSCommandStruct> it(commandQueue);
    for (QWSCommandStruct* cs = it.current(); (cs=*it); ++it ) {
	if ( cs->client == cl ) {
	    commandQueue.removeRef(cs);
	    delete cs;
	}
    }

    QRegion exposed;
    {
	// Shut down all windows for this client
	QListIterator<QWSWindow> it( windows );
	QWSWindow* w;
	while (( w = it.current() )) {
	    ++it;
	    if ( w->forClient(cl) )
		w->shuttingDown();
	}
    }
    {
	// Delete all windows for this client
	QListIterator<QWSWindow> it( windows );
	QWSWindow* w;
	while (( w = it.current() )) {
	    ++it;
	    if ( w->forClient(cl) ) {
		if ( mouseGrabber == w ) {
		    mouseGrabber = 0;
		    mouseGrabbing = FALSE;
#ifndef QT_NO_CURSOR
		    if (nextCursor) {
			// Not grabbing -> set the correct cursor
			setCursor(nextCursor);
			nextCursor = 0;
		    }
#endif
		}
		exposed += w->allocation();
		rgnMan->remove( w->allocationIndex() );
		if ( focusw == w )
		    setFocus(focusw,0);
		windows.removeRef(w);
		delete w; //windows is not auto-delete
	    }
	}
    }
    client.remove( cl->socket() );
    delete cl;
    exposeRegion( exposed );
    syncRegions();
}



QWSCommand* QWSClient::readMoreCommand()
{
    if ( csocket ) {
	// read next command
	if ( !command ) {
	    int command_type = qws_read_uint( csocket );

	    if ( command_type>=0 ) {
		command = QWSCommand::factory( command_type );
	    }
	}

	if ( command ) {
	    if ( command->read( csocket ) ) {
		// Finished reading a whole command.
		QWSCommand* result = command;
		command = 0;
		return result;
	    }
	}

	// Not finished reading a whole command.
	return 0;
    } else {
	return qt_get_server_queue()->dequeue();
    }
}



void QWSServer::processEventQueue()
{
    if ( qwsServer )
	qwsServer->doClient( qwsServer->client[-1] );
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
    doClient( client );
    active = FALSE;
}
 
void QWSServer::doClient( QWSClient *client )
{
    QWSCommand* command=client->readMoreCommand();
    
    while ( command ) {
	QWSCommandStruct *cs = new QWSCommandStruct( command, client );
	commandQueue.append( cs );
	// Try for some more...
	command=client->readMoreCommand();
    }


    while ( !commandQueue.isEmpty() ) {
	commandQueue.first();
	QWSCommandStruct *cs = commandQueue.take();
	switch ( cs->command->type ) {
	case QWSCommand::Create:
	    invokeCreate( (QWSCreateCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::Region:
	    invokeRegion( (QWSRegionCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::RegionMove:
	    invokeRegionMove( (QWSRegionMoveCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::RegionDestroy:
	    invokeRegionDestroy( (QWSRegionDestroyCommand*)cs->command, cs->client );
	    break;
#ifndef QT_NO_QWS_PROPERTIES
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
#endif
	case QWSCommand::SetSelectionOwner:
	    invokeSetSelectionOwner( (QWSSetSelectionOwnerCommand*)cs->command );
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
	case QWSCommand::GrabMouse:
	    invokeGrabMouse( (QWSGrabMouseCommand*)cs->command, cs->client );
	    break;

	}
	delete cs->command;
	delete cs;
	if (cursorNeedsUpdate)
	    showCursor();
    }
}


void QWSServer::showCursor()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->show();
#endif
}

// ### don't like this
void QWSServer::enablePainting(bool e)
{
    if (e)
    {
	disablePainting = false;
	setWindowRegion( 0, QRegion() );
	syncRegions();
    }
    else
    {
	disablePainting = true;
	setWindowRegion( 0, QRegion(0,0,swidth,sheight) );
	syncRegions();
    }
}

void QWSServer::setBackgroundImage( const QImage &img )
{
    bgImage = img;

    QRegion r(0, 0, swidth, sheight);
    for (uint i=0; i<windows.count(); i++) {
	if ( r.isEmpty() )
	    return; // Nothing left for deeper windows
	QWSWindow* w = windows.at(i);
	r -= w->allocation();
    }
    paintBackground( r );
}

void QWSServer::refresh()
{
    exposeRegion( QRegion(0,0,swidth,sheight) );
    syncRegions();
}


void QWSServer::sendMouseEvent(const QPoint& pos, int state)
{
    showCursor();

    QWSMouseEvent event;

    //If grabbing window disappears, grab is still active until
    //after mouse release.
    QWSWindow *win = mouseGrabber ? mouseGrabber : windowAt( pos );
    event.simpleData.window = win ? win->id : 0;

#ifndef QT_NO_QWS_CURSOR
    // Arrow cursor over desktop
    if (!win) {
	if ( !mouseGrabber )
	    setCursor(QWSCursor::systemCursor(ArrowCursor));
	else
	    nextCursor = QWSCursor::systemCursor(ArrowCursor);
    }
#endif

    if ( state && !mouseGrabbing ) {
	mouseGrabber = win;
    }

    event.simpleData.x_root=pos.x();
    event.simpleData.y_root=pos.y();
    event.simpleData.state=state;
    event.simpleData.time=timer.elapsed();

    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendEvent( &event );

    if ( !state && !mouseGrabbing ) {
#ifndef QT_NO_CURSOR
	if (mouseGrabber && nextCursor) {
	    setCursor(nextCursor);
	    nextCursor = 0;
	}
#endif
	mouseGrabber = 0;
    }
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


void QWSServer::sendKeyEvent(int unicode, int keycode, int modifiers, bool isPress,
  bool autoRepeat)
{
    QWSKeyEvent event;
    event.simpleData.window = qwsServer->focusw ? qwsServer->focusw->winId() : 0;
    event.simpleData.unicode = unicode;
    event.simpleData.keycode = keycode;
    event.simpleData.modifiers = modifiers;
    event.simpleData.is_press = isPress;
    event.simpleData.is_auto_repeat = autoRepeat;

    for (ClientIterator it = qwsServer->client.begin(); it != qwsServer->client.end(); ++it ) {
	(*it)->sendEvent(&event);
    }
}
#ifndef QT_NO_QWS_PROPERTIES
void QWSServer::sendPropertyNotifyEvent( int property, int state )
{
    for ( ClientIterator it = client.begin(); it != client.end(); ++it )
	( *it )->sendPropertyNotifyEvent( property, state );
}
#endif
void QWSServer::invokeCreate( QWSCreateCommand *, QWSClient *client )
{
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

    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, client);
    if ( !changingw ) {
	qWarning("Invalue window handle %08x",cmd->simpleData.windowid);
	return;
    }
    if ( !changingw->forClient(client) ) {
	qWarning("Disabled: clients changing other client's window region");
	return;
    }

    QRegion region;
    region.setRects(cmd->rectangles, cmd->simpleData.nrectangles);

    if ( !region.isEmpty() )
	changingw->setNeedAck( TRUE );
    setWindowRegion( changingw, region );
    syncRegions( changingw );
    if ( focusw == changingw && region.isEmpty() )
	setFocus(changingw,FALSE);
}

void QWSServer::invokeRegionMove( const QWSRegionMoveCommand *cmd, QWSClient *client )
{
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, client);
    if ( !changingw ) {
	qWarning("Invalue window handle %08x",cmd->simpleData.windowid);
	return;
    }
    if ( !changingw->forClient(client) ) {
	qWarning("Disabled: clients changing other client's window region");
	return;
    }

    changingw->setNeedAck( TRUE );
    moveWindowRegion( changingw, cmd->simpleData.dx, cmd->simpleData.dy );
}

void QWSServer::invokeRegionDestroy( QWSRegionDestroyCommand *cmd, QWSClient *client )
{
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, client);
    if ( !changingw ) {
	qWarning("Invalue window handle %08x",cmd->simpleData.windowid);
	return;
    }
    if ( !changingw->forClient(client) ) {
	qWarning("Disabled: clients changing other client's window region");
	return;
    }

    setWindowRegion( changingw, QRegion() );
    rgnMan->remove( changingw->allocationIndex() );
    QWSWindow *w = windows.first();
    while ( w ) {
	if ( w == changingw ) {
	    windows.take();
	    break;
	}
	w = windows.next();
    }
    syncRegions();
    if ( focusw == changingw )
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
	    if ( w != changingw && !w->hidden() &&
		    (!bestw || bestw->focusPriority() < w->focusPriority()) )
		bestw = w;
	}
	if ( !bestw && changingw->focusPriority() ) { // accept focus back?
	    bestw = changingw; // must be the only one
	}
	focusw = bestw;
	if ( focusw )
	    focusw->focus(1);
    }
}

void QWSServer::invokeSetAltitude( const QWSChangeAltitudeCommand *cmd,
				   QWSClient *client )
{
    int winId = cmd->simpleData.windowid;
    int alt = cmd->simpleData.altitude;
    bool fixed = cmd->simpleData.fixed;
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
    changingw->setNeedAck( TRUE );
    if ( fixed && alt == 0)
	changingw->onTop = TRUE;
    if ( alt < 0 )
	lowerWindow( changingw, alt );
    else
	raiseWindow( changingw, alt );
}
#ifndef QT_NO_QWS_PROPERTIES
void QWSServer::invokeAddProperty( QWSAddPropertyCommand *cmd )
{
    qDebug( "QWSServer::invokeAddProperty %d %d", cmd->simpleData.windowid,
	    cmd->simpleData.property );
    if ( manager()->addProperty( cmd->simpleData.windowid, cmd->simpleData.property ) )
 	qDebug( "add property successful" );
    else
 	qDebug( "adding property failed" );
}

void QWSServer::invokeSetProperty( QWSSetPropertyCommand *cmd )
{
    qDebug( "QWSServer::invokeSetProperty %d %d %d %s",
	    cmd->simpleData.windowid, cmd->simpleData.property,
	    cmd->simpleData.mode, cmd->data );
    if ( manager()->setProperty( cmd->simpleData.windowid,
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
    if ( manager()->removeProperty( cmd->simpleData.windowid,
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
    if ( manager()->getProperty( cmd->simpleData.windowid,
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
#endif //QT_NO_QWS_PROPERTIES

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
#ifndef QT_NO_QWS_CURSOR
    if (cmd->simpleData.height > 64 || cmd->simpleData.width > 64) {
	qDebug("Cannot define cursor size > 64x64");
	return;
    }

    int dataLen = cmd->simpleData.height * ((cmd->simpleData.width+7) / 8);

    QWSCursor *curs = new QWSCursor( cmd->data, cmd->data + dataLen,
				cmd->simpleData.width, cmd->simpleData.height,
				cmd->simpleData.hotX, cmd->simpleData.hotY);

    client->cursors.insert(cmd->simpleData.id, curs);
#endif
}

void QWSServer::invokeSelectCursor( QWSSelectCursorCommand *cmd, QWSClient *client )
{
#ifndef QT_NO_QWS_CURSOR
    int id = cmd->simpleData.id;
    QWSCursor *curs = 0;
    if (id <= LastCursor) {
        curs = QWSCursor::systemCursor(id);
    }
    else {
	QWSCursorMap cursMap = client->cursors;
	QWSCursorMap::Iterator it = cursMap.find(id);
	if (it != cursMap.end()) {
	    curs = it.data();
	}
    }
    if (curs == 0) {
	curs = QWSCursor::systemCursor(ArrowCursor);
    }

    if (mouseGrabber) {
	// If the mouse is being grabbed, we don't want just anyone to
        // be able to change the cursor.  We do want the cursor to be set
	// correctly once mouse grabbing is stopped though.
	QWSWindow* win = findWindow(cmd->simpleData.windowid, client);
	if (win != mouseGrabber)
	    nextCursor = curs;
	else
	    setCursor(curs);
    }
    else
	setCursor(curs);

    cursorNeedsUpdate = true;
#endif
}

void QWSServer::invokeGrabMouse( QWSGrabMouseCommand *cmd, QWSClient *client )
{
    QWSWindow* win = findWindow(cmd->simpleData.windowid, client);
    if ( mouseGrabbing ) {
	if (win != mouseGrabber) {
	    qWarning("Mouse already grabbed by another window");
	    return;
	}

	if ( !cmd->simpleData.grab ) {
	    mouseGrabbing = FALSE;
	    mouseGrabber = 0;
#ifndef QT_NO_CURSOR
	    if (nextCursor) {
		// Not grabbing -> set the correct cursor
		setCursor(nextCursor);
		nextCursor = 0;
	    }
#endif
	}
    } else if ( cmd->simpleData.grab ) {
	mouseGrabbing = TRUE;
	mouseGrabber = win;
    }
}

/*!
  Adds \a r to the window's allocated region.
*/
void QWSWindow::addAllocation( QWSRegionManager *rm, QRegion r )
{
    QRegion added = r & requested_region;
    if ( !added.isEmpty() ) {
	allocated_region |= added;
	exposed |= added;
	rm->set( alloc_region_idx, allocated_region );
	modified = TRUE;
    }
}

/*!
  Removes \a r from the window's allocated region
*/
void QWSWindow::removeAllocation(QWSRegionManager *rm, QRegion r)
{
    QRegion nr = allocated_region - r;
    if ( nr != allocated_region ) {
	allocated_region = nr;
	rm->set( alloc_region_idx, allocated_region );
	modified = TRUE;
    }
}

void QWSWindow::updateAllocation()
{
    if ( modified || needAck) {
	c->sendRegionModifyEvent( id, exposed, needAck );
	exposed = QRegion();
	modified = FALSE;
	needAck = FALSE;
    }
}

static int global_focus_time_counter=100;

void QWSWindow::focus(bool get)
{
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
    int idx = rgnMan->add( id, QRegion() );
    w->setAllocationIndex( idx );
    // insert after "stays on top" windows
    QWSWindow *win = windows.first();
    bool added = FALSE;
    while ( win ) {
	if ( !win->onTop ) {
	    windows.insert( windows.at(), w );
	    added = TRUE;
	    break;
	}
	win = windows.next();
    }
    if ( !added )
	windows.append( w );
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
    if ( changingw == windows.first() ) {
	changingw->updateAllocation(); // still need ack
	return;
    }

    //change position in list:
    QWSWindow *w = windows.first();
    while ( w ) {
	if ( w == changingw ) {
	    windows.take();
	    break;
	}
	w = windows.next();
    }

    if ( changingw->onTop )
	windows.prepend( changingw );
    else {
	// insert after "stays on top" windows
	bool in = FALSE;
	w = windows.first();
	while ( w ) {
	    if ( !w->onTop ) {
		windows.insert( windows.at(), changingw );
		in = TRUE;
		break;
	    }
	    w = windows.next();
	}
	if ( !in )
	    windows.append( changingw );
    }

    setWindowRegion( changingw, changingw->requested_region );
    syncRegions( changingw );
}

void QWSServer::lowerWindow( QWSWindow *changingw, int )
{
    if ( changingw == windows.last() ) {
	changingw->updateAllocation(); // still need ack
	return;
    }

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
    QRegion exposed = changingw->allocation() - visible;

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

    changingw->removeAllocation( rgnMan, exposed );
    exposeRegion( exposed, 0 );
    syncRegions( changingw );
}

void QWSServer::moveWindowRegion( QWSWindow *changingw, int dx, int dy )
{
    if ( !changingw ) return;

    QRegion oldAlloc( changingw->allocation() );
    oldAlloc.translate( dx, dy );
    QRegion newRegion( changingw->requested_region );
    newRegion.translate( dx, dy );

    setWindowRegion( changingw, newRegion );

    // add exposed areas
    changingw->exposed = changingw->allocation() - oldAlloc;

    QWSDisplay::grab( TRUE );
    rgnMan->commit();

    // safe to blt now
    QRegion cr( changingw->allocation() );
    cr &= oldAlloc;
    QRect br( cr.boundingRect() );
    gfx->setClipRegion( cr );
    gfx->scroll( br.x(), br.y(), br.width(), br.height(),
		 br.x() - dx, br.y() - dy );
    gfx->setClipRegion( QRegion( 0, 0, swidth, sheight ) );
    QWSDisplay::ungrab();

    notifyModified( changingw );
    paintBackground( dirtyBackground );
    dirtyBackground = QRegion();
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
    QRegion extra_allocation;
    int windex = -1;


    if ( changingw )
	changingw->removeAllocation( rgnMan, exposed );

    // Go through the higher windows and calculate the reqion that we will
    // end up with.
    // Then continue with the deeper windows, taking the requested region
    bool deeper = changingw == 0;
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w == changingw ) {
	    windex = i;
	    extra_allocation = r - w->allocation();
	    deeper = TRUE;
	} else if ( deeper ) {
	    w->removeAllocation(rgnMan, r);
	    r -= w->allocation();
	} else {
	    //higher windows
	    r -= w->allocation();
	}
	if ( r.isEmpty() ) {
	    break; // Nothing left for deeper windows
	}
    }

    if ( changingw && !changingw->requested_region.isEmpty() )
	changingw->addAllocation( rgnMan, extra_allocation & screenRegion);
    else if (!disablePainting)
	paintServerRegion();

    exposeRegion( exposed, windex+1 );
}

void QWSServer::exposeRegion( QRegion r, int start )
{
    r &= screenRegion;

    for (uint i=start; i<windows.count(); i++) {
	if ( r.isEmpty() )
	    break; // Nothing left for deeper windows
	QWSWindow* w = windows.at(i);
	w->addAllocation( rgnMan, r );
	r -= w->allocation();
    }
    dirtyBackground |= r;
}

void QWSServer::notifyModified( QWSWindow *active )
{
    // notify active window first
    if ( active )
	active->updateAllocation();

    // now the rest
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	w->updateAllocation();
    }
}

void QWSServer::syncRegions( QWSWindow *active )
{
    rgnMan->commit();
    notifyModified( active );
    paintBackground( dirtyBackground );
    dirtyBackground = QRegion();
}

QMouseHandler::QMouseHandler()
{
}

QMouseHandler::~QMouseHandler()
{
}

void QWSServer::closeMouse()
{
    mousehandlers.setAutoDelete(TRUE);
    mousehandlers.clear();
}

void QWSServer::openMouse()
{
    QString mice = getenv("QWS_MOUSE_PROTO");
    if ( mice.isEmpty() ) {
#if defined(__MIPSEL__)
	mice = "TPanel:/dev/tpanel";
#elif !defined(QT_NO_QWS_VFB)
	extern bool qvfbEnabled;
	if ( qvfbEnabled )
	    mice = "QVFbMouse";
#endif
	if ( mice.isEmpty() ) {
	    mice = "MouseMan:/dev/mouse";   // last resort
	}
	qDebug("Assuming mouse %s", mice.latin1() );
    }
    closeMouse();
#ifndef QT_NO_STRINGLIST
    QStringList mouse = QStringList::split(" ",mice);
    for (QStringList::Iterator m=mouse.begin(); m!=mouse.end(); ++m) {
	QMouseHandler* mh = newMouseHandler(*m);
	connect(mh, SIGNAL(mouseChanged(const QPoint&,int)),
		this, SLOT(setMouse(const QPoint&,int)));
	mousehandlers.append(mh);
    }
#else
    QMouseHandler* mh = newMouseHandler(mice); //Assume only one
    connect(mh, SIGNAL(mouseChanged(const QPoint&,int)),
	    this, SLOT(setMouse(const QPoint&,int)));
    mousehandlers.append(mh);
    
#endif    
}

QWSKeyboardHandler::QWSKeyboardHandler()
{
}

QWSKeyboardHandler::~QWSKeyboardHandler()
{
}

void QWSServer::closeKeyboard()
{
    keyboardhandlers.setAutoDelete(TRUE);
    keyboardhandlers.clear();
}

void QWSServer::openKeyboard()
{
    QString keyboards = getenv("QWS_KEYBOARD");
    if ( keyboards.isEmpty() ) {
#ifdef __MIPSEL__
	keyboards = "Buttons";
#elif !defined(QT_NO_QWS_VFB)
	extern bool qvfbEnabled;
	if ( qvfbEnabled )
	    keyboards = "QVFbKeyboard";
#endif
	if ( keyboards.isEmpty() ) {
	    keyboards = "TTY";	// last resort
	}
    }
    closeKeyboard();
#ifndef QT_NO_STRINGLIST
    QStringList keyboard = QStringList::split(" ",keyboards);
    for (QStringList::Iterator k=keyboard.begin(); k!=keyboard.end(); ++k) {
	QWSKeyboardHandler* kh = newKeyboardHandler(*k);
	keyboardhandlers.append(kh);
    }
#else
    QWSKeyboardHandler* kh = newKeyboardHandler(keyboards); //assume only one
    keyboardhandlers.append(kh);
#endif
}

QWSServer *QWSServer::qwsServer=0; //there can be only one

void QWSServer::move_region( const QWSRegionMoveCommand *cmd )
{
    QWSClient *serverClient = qwsServer->client[-1];
    qwsServer->invokeRegionMove( cmd, serverClient );
}

void QWSServer::set_altitude( const QWSChangeAltitudeCommand *cmd )
{
    QWSClient *serverClient = qwsServer->client[-1];
    qwsServer->invokeSetAltitude( cmd, serverClient );
}


void QWSServer::request_region( int wid, QRegion region )
{
    QWSClient *serverClient = qwsServer->client[-1];
    QWSWindow* changingw = qwsServer->findWindow( wid, serverClient );
    if ( !region.isEmpty() )
	changingw->setNeedAck( TRUE );
    qwsServer->setWindowRegion( changingw, region );
    qwsServer->syncRegions( changingw );
    if ( qwsServer->focusw == changingw && region.isEmpty() )
	qwsServer->setFocus(changingw,FALSE);
}




//previously in qws_linuxfb.cpp:


static bool fb_open = FALSE;

void QWSServer::openDisplay()
{
    fb_open=TRUE;

    QScreen * s=qt_probe_bus();
    s->initCard();

    swidth=s->width();
    sheight=s->height();
    gfx=s->screenGfx();
}


void QWSServer::closeDisplay()
{
    qt_screen->shutdownCard();
}


void QWSServer::paintServerRegion()
{
}

void QWSServer::paintBackground( QRegion r )
{
    if ( !r.isEmpty() ) {
	ASSERT ( fb_open );

	gfx->setClipRegion( r );
	QRect br( r.boundingRect() );
	if ( bgImage.isNull() ) {
	    /*
	    uint col = 0x20b050;
	    if (qt_screen->depth() == 16) {
		//### manual color allocation
		int r = (col & 0xff0000) >> 16;
		int g = (col & 0x00ff00) >> 8;
		int b = (col & 0x0000ff);
		r=r >> 3;
		g=g >> 2;
		b=b >> 3;
		r=r << 11;
		g=g << 5;
		col = r | g | b;
	    }
	    gfx->setBrush(QBrush(QColor(col, col)));
	    */
	    QColor col(0x20, 0xb0, 0x50);
	    gfx->setBrush(QBrush(col));
	    gfx->setPen(QPen::NoPen);
	    gfx->drawRect( br.x(), br.y(), br.width(), br.height() );
	} else {
	    gfx->setSource( &bgImage );
	    gfx->tiledBlt( br.x(), br.y(), br.width(), br.height() );
	}
	gfx->setClipRegion( QRegion() );
    }
}


/*!
  Start the server
 */

void QWSServer::startup(int flags)
{
    if ( qwsServer )
	return;
    unlink( QTE_PIPE );
    (void)new QWSServer(flags);

}


/*!
  Close down the server
*/

void QWSServer::closedown()
{
    unlink( QTE_PIPE );
    delete qwsServer;
}


void QWSServer::emergency_cleanup()
{
    if ( qwsServer )
	qwsServer->closeKeyboard();
}


static QWSServer::KeyboardFilter *keyFilter;

void QWSServer::processKeyEvent(int unicode, int keycode, int modifiers, bool isPress,
  bool autoRepeat)
{
    if ( !keyFilter ||
	 !keyFilter->filter( unicode, keycode, modifiers, isPress, autoRepeat ) )
	sendKeyEvent( unicode, keycode, modifiers, isPress, autoRepeat );
}


void QWSServer::setKeyboardFilter( KeyboardFilter *f )
{
    keyFilter = f;
}
