/****************************************************************************
**
** Definition of Qt/FB central server classes.
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

#ifndef QTFB_H
#define QTFB_H

#include <qserversocket.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qptrqueue.h>
#include <qptrlist.h>

#include "qwsproperty.h"
#include "qwscommand.h"
#include "qwsevent.h"

const int QTFB_PORT=0x4642; // FB

class QWSCursor;
class QWSClient;
class QGfx;

class QWSWindow
{
    friend class QWSServer;
public:
    QWSWindow(int i, QWSClient* client) : id(i), c(client), pending_acks(0),
	    last_focus_time(0)
	{ }

    int winId() const { return id; }
    bool forClient(const QWSClient* cl) const { return cl==c; }
    QWSClient* client() const { return c; }
    QRegion allocation() const { return allocated_region; }
    bool hidden() const { return requested_region.isEmpty(); }
    bool partiallyObscured() const { return requested_region!=allocated_region; }
    bool fullyObscured() const { return allocated_region.isEmpty(); }

    void addAllocation( QRegion, bool isAck = FALSE );
    bool removeAllocation( QRegion );

    void focus(bool get);
    int focusPriority() const { return last_focus_time; }

private:
    int id;
    QWSClient* c;
    short int pending_acks;
    QRegion requested_region;
    QRegion allocated_region;
    int last_focus_time;
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
    QWSServer( int swidth=0, int sheight=0, int simulate_depth = 0,
	       int flags = 0, QObject *parent=0, const char *name=0 );
    ~QWSServer();
    void newConnection( int socket );

    enum ServerFlags { DisableKeyboard = 0x01,
		       DisableMouse = 0x02,
		       DisableAccel = 0x04 };

    uchar* frameBuffer() { return framebuffer; }

    void sendKeyEvent(int unicode, int modifiers, bool isPress,
		      bool autoRepeat);
    void setMouse(const QPoint& pos,int bstate);
    void sendMouseEvent(const QPoint& pos, int state);
    void sendPropertyNotifyEvent( int property, int state );
    QWSPropertyManager *properties() {
	return &propertyManager;
    }

    QWSWindow *windowAt( const QPoint& pos );

    // For debugging only at this time
    QPtrList<QWSWindow> clientWindows() { return windows; }

    void openMouse();
    void closeMouse();

    void refresh();
    void enablePainting(bool);

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
    void invokeSetFocus( QWSRequestFocusCommand *cmd, QWSClient *client );

    void setFocus( QWSWindow*, bool gain );

    void initIO();
    void invokeDefineCursor( QWSDefineCursorCommand *cmd, QWSClient *client );
    void invokeSelectCursor( QWSSelectCursorCommand *cmd, QWSClient *client );

    void handleMouseData();

    void openKeyboard();
    void closeKeyboard();

    void openDisplay();
    void closeDisplay();

    void showCursor();
    void initializeCursor();
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

    QGfx *gfx;

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

    QWSWindow *focusw;
    QWSWindow *mouseGrabber;
    bool mouseGrabbing;
    int swidth, sheight, sdepth;
    QPoint mousePos;
    QPoint cursorPos;
    bool cursorNeedsUpdate;
    QWSCursor *cursor;
    QRegion serverRegion;
    bool disablePainting;

    QPtrQueue<QWSCommandStruct> commandQueue;
    QRegion pendingAllocation;
    QRegion pendingRegion;
    int pendingWindex;

    // Window management
    QPtrList<QWSWindow> windows; // first=topmost
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

typedef QMap<int, QWSCursor*> QWSCursorMap;

class QWSClient : private QSocket
{
    Q_OBJECT
    //  friend class QWSServer;
public:
    QWSClient( QObject* parent, int socket, int shmid,
		int swidth, int sheight, int sdepth,
		int ramid, int fblen, int offscreen, int offscreenlen);
    ~QWSClient();

    int socket() const;

    QObject* asQObject() { return (QObject*)this; } //### private inheritance

    void sendEvent( QWSEvent* event );
    void sendRegionAddEvent( int winid, bool ack, QRegion );
    void sendRegionRemoveEvent( int winid, int eventid, QRegion );
    void sendFocusEvent( int winid, bool get );
    void sendPropertyNotifyEvent( int property, int state );
    void sendPropertyReplyEvent( int property, int len, char *data );
    void sendSelectionClearEvent( int windowid );
    void sendSelectionRequestEvent( QWSConvertSelectionCommand *cmd, int windowid );
    QWSCommand* readMoreCommand();

    QWSCursorMap cursors;	// cursors defined by this client
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
