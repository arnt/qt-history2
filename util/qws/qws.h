/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.h#13 $
**
** Definition of Qt/FB central server classes
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

#ifndef QTFB_H
#define QTFB_H

#include <qserversocket.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qqueue.h>
#include <qlist.h>

#include "qwsproperty.h"
#include "qwscommand.h"

const int QTFB_PORT=0x4642; // FB

class QWSClient;

class QWSWindow
{
    friend class QWSServer;
public:
    QWSWindow(int i, QWSClient* client) : id(i), c(client), pending_acks(0)
	{ }

    int winId() const { return id; }
    bool forClient(const QWSClient* cl) const { return cl==c; }
    QWSClient* client() const { return c; }
    QRegion allocation() const { return allocated_region; }

    void addAllocation( QRegion, bool isAck = FALSE );
    bool removeAllocation( QRegion );

private:
    int id;
    QWSClient* c;
    short int pending_acks;
    QRegion requested_region;
    QRegion allocated_region;
};

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

struct QWSCommandStruct;

class QWSServer : private QServerSocket
{
    Q_OBJECT

public:
    QWSServer( int swidth=0, int sheight=0, bool simulate = FALSE,
	       QObject *parent=0, const char *name=0 );
    ~QWSServer();
    void newConnection( int socket );

    uchar* frameBuffer() { return framebuffer; }

    void sendKeyEvent(int unicode, int modifiers, bool isPress,
		      bool autoRepeat);
    void sendMouseEvent(const QPoint& pos, int state);
    void sendPropertyNotifyEvent( int property, int state );
    QWSPropertyManager *properties() {
	return &propertyManager;
    }

    QWSWindow *windowAt( const QPoint& pos );

    // For debugging only at this time
    QList<QWSWindow> clientWindows() { return windows; }

private:
    void invokeCreate( QWSCreateCommand *cmd, QWSClient *client );
    void invokeRegion( QWSRegionCommand *cmd, QWSClient *client );
    void invokeSetAltitude( QWSChangeAltitudeCommand *cmd, QWSClient *client );
    void invokeAddProperty( QWSAddPropertyCommand *cmd );
    void invokeSetProperty( QWSSetPropertyCommand *cmd );
    void invokeRemoveProperty( QWSRemovePropertyCommand *cmd );
    void invokeGetProperty( QWSGetPropertyCommand *cmd, QWSClient *client );
    void invokeSetSelectionOwner( QWSSetSelectionOwnerCommand *cmd );
    void invokeConvertSelection( QWSConvertSelectionCommand *cmd );

    void initIO();
    void handleMouseData();

    void showCursor();
    void paintServerRegion();
    void paintBackground( QRegion );

private slots:
    void clientClosed();
    void doClient();
    void readMouseData();
    void readKeyboardData();

private:
    typedef QMapIterator<int,QWSClient*> ClientIterator;
    typedef QMap<int,QWSClient*> ClientMap;

    int shmid;
    int ramid;
    uchar* framebuffer;
    uchar* sharedram;
    int offscreen;
    int offscreenlen;
    int fblen;
    int ramlen;

    ClientMap client;
    QWSPropertyManager propertyManager;
    struct SelectionOwner {
	int windowid;
	struct Time {
	    void set( int h, int m, int s, int s2 ) {
		hour = h; minute = m; sec = s; ms = s2;
	    }
	    int hour, minute, sec, ms;
	} time;
    } selectionOwner;
    QTime timer;

    QWSWindow *mouseGrabber;
    bool mouseGrabbing;
    int swidth, sheight;
    int mouseFD;
    int kbdFD;
    int mouseIdx;
    uchar *mouseBuf;
    QPoint mousePos;
    QPoint cursorPos;
    bool cursorNeedsUpdate;
    QRegion serverRegion;

    QQueue<QWSCommandStruct> commandQueue;
    QRegion pendingAllocation;
    QRegion pendingRegion;
    int pendingWindex;

    // Window management
    QList<QWSWindow> windows; // first=topmost
    QWSWindow* newWindow(int id, QWSClient* client);
    QWSWindow* findWindow(int windowid, QWSClient* client);
    void setWindowRegion(QWSWindow*, QRegion r );
    void raiseWindow( QWSWindow *, int = 0);
    void lowerWindow( QWSWindow *, int = -1);
    void exposeRegion( QRegion );
    void givePendingRegion();
    int pending_region_acks;
};



/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/
class QWSMouseEvent;

class QWSClient : private QSocket
{
    Q_OBJECT
    //  friend class QWSServer;
public:
    QWSClient( QObject* parent, int socket, int shmid, int swidth, int sheight,
	       int ramid, int fblen, int offscreen, int offscreenlen);
    ~QWSClient();

    int socket() const;

    QObject* asQObject() { return (QObject*)this; } //### private inheritance 
    
    void sendSimpleEvent( void* event, uint size );
    void sendRegionAddEvent( int winid, bool ack, QRegion );
    void sendRegionRemoveEvent( int winid, int eventid, QRegion );
    void sendMouseEvent( const QWSMouseEvent& );
    void sendPropertyNotifyEvent( int property, int state );
    void sendPropertyReplyEvent( int property, int len, char *data );
    void sendSelectionClearEvent( int windowid );
    void sendSelectionRequestEvent( QWSConvertSelectionCommand *cmd, int windowid );
    QWSCommand* readMoreCommand();
    void writeRegion( QRegion reg );
signals:
    void connectionClosed();
private slots:
    void closeHandler();
    void errorHandler( int );
private:
    int s; // XXX QSocket::d::socket->socket() is this value
    QWSCommand* command;
    uint isClosed : 1;
};

#endif
