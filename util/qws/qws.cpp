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
#include <qpointarray.h> //cursor test code

#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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
QWSClient::QWSClient( int socket, int shmid, int swidth, int sheight ) :
    QSocket(socket),
    s(socket),
    command(0)
{
    QWSHeader header;
    header.width = swidth;
    header.height = sheight;
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

void QWSClient::sendMouseEvent( const QWSMouseEvent &event )
{
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
#if 1//DEBUG
     qDebug( "   writeRegion rect[%d] = (%d,%d) %dx%d", i,
	     rectangle.x,rectangle.y,rectangle.width,rectangle.height);

     
#endif
	
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

void QWSClient::sendSelectionRequestEvent( QWSConvertSelectionCommand *cmd, int windowid )
{
    QWSSelectionRequestEvent event;
    event.type = QWSEvent::SelectionRequest;
    event.window = windowid;
    event.requestor = cmd->simpleData.requestor;
    event.property = cmd->simpleData.selection;
    event.mimeTypes = cmd->simpleData.mimeTypes;
    writeBlock( (char*)&event, sizeof( event ) );
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

QWSServer::QWSServer( int sw, int sh, QObject *parent=0, const char *name=0 ) :
    QServerSocket(QTFB_PORT,16,parent,name),
    mouseBuf(0), pending_region_acks(0)
{
    cursorNeedsUpdate=FALSE;
    cursorPos = QPoint(-1,-1); //no software cursor
    swidth = sw;
    sheight = sh;

    if ( swidth && sheight ) {
	shmid = shmget(IPC_PRIVATE, swidth*sheight*sizeof(QRgb),
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
	initIO(); //device specific init
    }

    // no selection yet
    selectionOwner.windowid = -1;
    selectionOwner.time.set( -1, -1, -1, -1 );
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
    client[socket] = new QWSClient(socket,shmid,swidth,sheight);
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
	    case QWSCommand::RegionAck:
		command = new QWSRegionAckCommand;
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
	    pending_region_acks--;
	    qDebug( "QWSCommand::RegionAck from %p pending:%d", 
		    client, pending_region_acks);
	    if ( pending_region_acks == 0 ) {
		givePendingRegion();
	    }
	    delete command;
	} else {
	    /*
	    if ( command->type == QWSCommand::Region ) {
		QWSRegionCommand *cmd = (QWSRegionCommand*)command;
		QWSWindow* w = findWindow(cmd->simpleData.windowid, client);
		w->region_request_count++;
		qDebug( "Window %p, region_request_count %d", w, 
			w->region_request_count); 
			}*/
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
	    //{	
	    //	  QWSRegionCommand *cmd = (QWSRegionCommand*)cs->command;
	    //	  QWSWindow* w = findWindow(cmd->simpleData.windowid,
	    //				    cs->client);
	    //	  if ( --w->region_request_count == 0 )
	    invokeRegion( (QWSRegionCommand*)cs->command, cs->client );
	    //}
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
    if ( pending_region_acks != 0 ) {
	cursorNeedsUpdate = TRUE;
	return;
    }
    cursorNeedsUpdate = FALSE;
    if ( cursorPos == mousePos )
	return;
    cursorPos = mousePos;
    //##### hardcoded region
   QPointArray a;
   a.setPoints(  7, 
		 0,0,
		 6,0,
		 4,2,
		 14,12,
		 12,14,
		 2,4,
		 0,6 );
    a.translate( cursorPos.x(), cursorPos.y() );
    setWindowRegion( 0, QRegion(a) );
}

void QWSServer::sendMouseEvent(const QPoint& pos, int state)
{
    showCursor();
    
    
    QWSMouseEvent event;
    event.type = QWSEvent::Mouse;

    QWSWindow *win = windowAt( pos );
    event.window = win ? win->id : 0;

    event.x_root=pos.x();
    event.y_root=pos.y();
    event.state=state;
    event.time=timer.elapsed(); 

    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendMouseEvent( event );
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
    event.type = QWSEvent::Key;
    event.window = 0; //##### not used yet
    event.unicode = unicode;
    event.modifiers = modifiers;
    event.is_press = isPress;
    event.is_auto_repeat = autoRepeat;
    
    for (ClientIterator it = client.begin(); it != client.end(); ++it ) {
	(*it)->writeBlock((char*)&event,sizeof(event));
	(*it)->flush();
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
    
void QWSWindow::addAllocation(QRegion r)
{
    QRegion added = r & requested_region;
    allocated_region |= added;

    QWSRegionAddEvent event;
    event.type = QWSEvent::RegionAdd;
    event.window = id;
    event.nrectangles = added.rects().count(); // XXX MAJOR WASTAGE
    c->writeBlock( (char*)&event, sizeof(event)-sizeof(event.rectangles) );
#if 1//DEBUG
qDebug("Add region (%d rects) to %p/%d",event.nrectangles, c,id);
#endif
    c->writeRegion( added );
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
	c->writeBlock( (char*)&event, sizeof(event)-sizeof(event.rectangles) );

#if 1 //DEBUG
qDebug("Remove region (%d rects) from %p/%d", event.nrectangles, c, id);

#endif 
	
	c->writeRegion( r );
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
  Changes the requested region of window \a changingw to \a r,
  sends appropriate region change events to all appropriate
  clients, and waits for all required acknowledgements.
  
  
  If \a changingw is 0, the server's reserved region is changed.
*/
void QWSServer::setWindowRegion(QWSWindow* changingw, QRegion r)
{
    qDebug("setWindowRegion");

    if (changingw)
	changingw->requested_region = r;

    QRegion allocation;
    QRegion exposed = changingw ? changingw->allocation() - r : serverRegion-r;
    int windex = -1;

    // First, take the region away from whichever windows currently have it...
    if ( changingw == 0 )
	serverRegion = r;
    bool deeper = changingw == 0;;
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

    // The region to give to the window:
    pendingAllocation = allocation;

    // Wait for acknowledgements if pending_region_acks > 0
    // otherwise do it straight away.
    
    pendingWindex = windex;
    pendingRegion = exposed;

    if ( pending_region_acks == 0 )
	    givePendingRegion();
    
}

void QWSServer::givePendingRegion()
{    
    QRegion exposed = pendingRegion;
    // Finally, give anything exposed...
    
    if ( pendingWindex >= 0 ) {
	QWSWindow* changingw = windows.at( pendingWindex );
	ASSERT( changingw );
	changingw->addAllocation(pendingAllocation); 
    } else {
	paintServerRegion();
    }
    for (uint i=pendingWindex+1; i<windows.count(); i++) {
	if ( exposed.isEmpty() )
	    return; // Nothing left for deeper windows
	QWSWindow* w = windows.at(i);
	w->addAllocation(exposed);
	exposed -= w->allocation();
    }
}
