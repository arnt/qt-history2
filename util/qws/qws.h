/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.h#13 $
**
** Definition of Qt/FB central server classes
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

#ifndef QTFB_H
#define QTFB_H

#include <qserversocket.h>
#include <qmap.h>
#include <qdatetime.h>

#include "qwsproperty.h"
#include "qwscommand.h"

const int QTFB_PORT=0x4642; // FB

class QWSClient;

class QWSWindow
{
    friend class QWSServer;
public:
    QWSWindow(int i, QWSClient* c) : id(i), client(c) { }

    int winId() const { return id; }
    bool forClient(const QWSClient* c) const { return client==c; }
    QRegion allocation() const { return allocated_region; }

    void addAllocation( QRegion );
    bool removeAllocation( QRegion );

private:
    int id;
    QWSClient* client;

    QRegion requested_region;
    QRegion allocated_region;
};

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

class QWSServer : public QServerSocket
{
    Q_OBJECT

public:
    QWSServer( bool fake=FALSE, QObject *parent=0, const char *name=0 );
    ~QWSServer();
    void newConnection( int socket );

    uchar* frameBuffer() { return framebuffer; }

    void sendMouseEvent(const QPoint& pos, int state);
    void sendPropertyNotifyEvent( int property, int state );
    QWSPropertyManager *properties() {
	return &propertyManager;
    }

private:
    void invokeCreate( QWSCreateCommand *cmd, QWSClient *client );
    void invokeRegion( QWSRegionCommand *cmd, QWSClient *client );
    void invokeAddProperty( QWSAddPropertyCommand *cmd );
    void invokeSetProperty( QWSSetPropertyCommand *cmd );
    void invokeRemoveProperty( QWSRemovePropertyCommand *cmd );
    void invokeGetProperty( QWSGetPropertyCommand *cmd, QWSClient *client );
    void invokeSetSelectionOwner( QWSSetSelectionOwnerCommand *cmd );
    void invokeConvertSelection( QWSConvertSelectionCommand *cmd );

private slots:
    void doClient();
    void readMouseData();

private:
    void handleMouseData();
    typedef QMapIterator<int,QWSClient*> ClientIterator;
    typedef QMap<int,QWSClient*> ClientMap;

    int shmid;
    uchar* framebuffer;
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

    int mouseFD;
    int mouseIdx;
    uchar *mouseBuf;
    int mouseX, mouseY;

    // Window management
    QList<QWSWindow> windows; // first=topmost
    QWSWindow* newWindow(int id, QWSClient* client);
    QWSWindow* findWindow(int windowid, QWSClient* client);
    void setWindowRegion(QWSWindow*, QRegion r);
    int pending_region_acks;
};



/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

class QWSClient : public QSocket
{
    friend class QWSServer;
public:
    QWSClient( int socket, int shmid );

    int socket() const;

    void sendMouseEvent(const QPoint& pos, int state);
    void sendPropertyNotifyEvent( int property, int state );
    void sendPropertyReplyEvent( int property, int len, char *data );
    void sendSelectionClearEvent( int windowid );
    void sendSelectionRequestEvent( QWSConvertSelectionCommand *cmd, int windowid );
    QWSCommand* readMoreCommand();
    void writeRegion( QRegion reg );

private:
    int s; // XXX QSocket::d::socket->socket() is this value
    QTime timer;
    QWSCommand* command;
};

#endif
