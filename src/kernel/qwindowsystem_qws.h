/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
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

#ifndef QT_H
#include <qserversocket.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qimage.h>

#include "qwsproperty.h"
#include "qwscommand.h"
#include "qwsevent.h"
#endif // QT_H

struct SWCursorData;
class QWSCursor;
class QWSClient;
class QWSRegionManager;
class QGfx;

class QWSWindow
{
    friend class QWSServer;
public:
    QWSWindow(int i, QWSClient* client)
	: id(i), alloc_region_idx(-1), modified(FALSE), needAck(FALSE),
	    onTop(FALSE), c(client), last_focus_time(0)
	{ }

    int winId() const { return id; }
    bool forClient(const QWSClient* cl) const { return cl==c; }
    QWSClient* client() const { return c; }
    QRegion allocation() const { return allocated_region; }
    bool hidden() const { return requested_region.isEmpty(); }
    bool partiallyObscured() const { return requested_region!=allocated_region; }
    bool fullyObscured() const { return allocated_region.isEmpty(); }

    void addAllocation( QWSRegionManager *, QRegion );
    void removeAllocation( QWSRegionManager *, QRegion );

    int  allocationIndex() const { return alloc_region_idx; }
    void setAllocationIndex( int i ) { alloc_region_idx = i; }
    void updateAllocation();

    void setNeedAck( bool n ) { needAck = n; }

    void focus(bool get);
    int focusPriority() const { return last_focus_time; }
    void shuttingDown() { last_focus_time=0; }

private:
    int id;
    int alloc_region_idx;
    bool modified;
    bool needAck;
    bool onTop;
    QWSClient* c;
    QRegion requested_region;
    QRegion allocated_region;
    QRegion exposed;
    int last_focus_time;
};

class MouseHandler : public QObject {
    Q_OBJECT
public:
    MouseHandler();
    ~MouseHandler();
signals:
    void mouseChanged(const QPoint& pos, int bstate);
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
    QWSServer( int flags = 0, QObject *parent=0, const char *name=0 );
    ~QWSServer();
    void newConnection( int socket );

    enum ServerFlags { DisableKeyboard = 0x01,
		       DisableMouse = 0x02,
		       DisableAccel = 0x04 };

    
    enum GUIMode { NoGui = FALSE, NormalGUI = TRUE, Server };
    
    static void sendKeyEvent(int unicode, int modifiers, bool isPress,
			     bool autoRepeat);
    static void processKeyEvent(int unicode, int modifiers, bool isPress,
				bool autoRepeat);

    typedef struct KeyMap {
	int  key_code;
	ushort unicode;
	ushort shift_unicode;
	ushort ctrl_unicode;
    };


    static const KeyMap *keyMap();
    
    class KeyboardFilter
    {
    public:
	virtual bool filter(int unicode, int modifiers, bool isPress,
		      bool autoRepeat)=0;
    };
    
    static void setKeyboardFilter( KeyboardFilter *f );
    
    void sendMouseEvent(const QPoint& pos, int state);
    void sendPropertyNotifyEvent( int property, int state );
    QWSPropertyManager *manager() {
	return &propertyManager;
    }

    QWSWindow *windowAt( const QPoint& pos );

    // For debugging only at this time
    QList<QWSWindow> clientWindows() { return windows; }

    void openMouse();
    void closeMouse();

    void refresh();
    void enablePainting(bool);

    // ### a pixmap would be nice, but qws can't load them
    void setBackgroundImage( const QImage &img );

    static void processEventQueue();

    static void move_region( const QWSRegionMoveCommand * );
    static void set_altitude( const QWSChangeAltitudeCommand * );
    static void request_region( int, QRegion );
    static void startup(int flags);
    static void closedown();

    static void emergency_cleanup();
    
    void openKeyboard();
    void closeKeyboard();
    

private:
    static QWSServer *qwsServer; //there can be only one
    
private:
    void invokeCreate( QWSCreateCommand *cmd, QWSClient *client );
    void invokeRegion( QWSRegionCommand *cmd, QWSClient *client );
    void invokeRegionMove( const QWSRegionMoveCommand *cmd, QWSClient *client );
    void invokeRegionDestroy( QWSRegionDestroyCommand *cmd, QWSClient *client );
    void invokeSetAltitude( const QWSChangeAltitudeCommand *cmd, QWSClient *client );
    void invokeAddProperty( QWSAddPropertyCommand *cmd );
    void invokeSetProperty( QWSSetPropertyCommand *cmd );
    void invokeRemoveProperty( QWSRemovePropertyCommand *cmd );
    void invokeGetProperty( QWSGetPropertyCommand *cmd, QWSClient *client );
    void invokeSetSelectionOwner( QWSSetSelectionOwnerCommand *cmd );
    void invokeConvertSelection( QWSConvertSelectionCommand *cmd );
    void invokeSetFocus( QWSRequestFocusCommand *cmd, QWSClient *client );

    void initIO();
    void setFocus( QWSWindow*, bool gain );
    void invokeDefineCursor( QWSDefineCursorCommand *cmd, QWSClient *client );
    void invokeSelectCursor( QWSSelectCursorCommand *cmd, QWSClient *client );
    void invokeGrabMouse( QWSGrabMouseCommand *cmd, QWSClient *client );

    MouseHandler* newMouseHandler(const QString& spec);


    void openDisplay();
    void closeDisplay();

    void showCursor();
    void initializeCursor();
    void paintServerRegion();
    void paintBackground( QRegion );

private slots:
    void clientClosed();
    void doClient();
    void setMouse(const QPoint& pos,int bstate);

    void readKeyboardData();

private:
    void doClient( QWSClient * );
    typedef QMapIterator<int,QWSClient*> ClientIterator;
    typedef QMap<int,QWSClient*> ClientMap;

    uchar* sharedram;
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
    QPoint cursorPos;
    bool cursorNeedsUpdate;
    QWSCursor *cursor;	    // cursor currently shown
    QWSCursor *nextCursor;  // cursor to show once grabbing is off
    QRegion screenRegion;   // the entire display region
    QRegion serverRegion;
    QRegion dirtyBackground;
    bool disablePainting;
    QList<MouseHandler> mousehandlers;
    QImage bgImage;

    QList<QWSCommandStruct> commandQueue;
    QWSRegionManager *rgnMan;

    // Window management
    QList<QWSWindow> windows; // first=topmost
    QWSWindow* newWindow(int id, QWSClient* client);
    QWSWindow* findWindow(int windowid, QWSClient* client);
    void moveWindowRegion(QWSWindow*, int dx, int dy );
    void setWindowRegion(QWSWindow*, QRegion r );
    void raiseWindow( QWSWindow *, int = 0);
    void lowerWindow( QWSWindow *, int = -1);
    void exposeRegion( QRegion , int index = 0 );
    void notifyModified( QWSWindow *active = 0 );
    void syncRegions( QWSWindow *active = 0 );

    void setCursor(QWSCursor *curs);
};



/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/
class QWSMouseEvent;

typedef QMap<int, QWSCursor*> QWSCursorMap;

class QWSClient : public QObject
{
    Q_OBJECT
public:
    QWSClient( QObject* parent, int socket );
    ~QWSClient();

    int socket() const;

    void sendEvent( QWSEvent* event );
    void sendRegionModifyEvent( int winid, QRegion exposed, bool ack );
    void sendFocusEvent( int winid, bool get );
    void sendPropertyNotifyEvent( int property, int state );
    void sendPropertyReplyEvent( int property, int len, char *data );
    void sendSelectionClearEvent( int windowid );
    void sendSelectionRequestEvent( QWSConvertSelectionCommand *cmd, int windowid );
    QWSCommand* readMoreCommand();

    QWSCursorMap cursors;	// cursors defined by this client
signals:
    void connectionClosed();
    void readyRead();
private slots:
    void closeHandler();
    void errorHandler( int );
private:
    int s; // XXX csocket->d->socket->socket() is this value
    QSocket *csocket;
    QWSCommand* command;
    uint isClosed : 1;
};

#endif
