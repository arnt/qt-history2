/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of Qt/FB central server classes
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTFB_H
#define QTFB_H

#ifndef QT_H
#include <qwssocket_qws.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qimage.h>

#include "qwsproperty_qws.h"
#include "qwscommand_qws.h"
#include "qwsevent_qws.h"
#include "qkeyboard_qws.h"
#endif // QT_H

struct SWCursorData;
class QWSCursor;
class QWSClient;
class QWSRegionManager;
class QGfx;

class QWSInternalWindowInfo
{

public:

    int winid;
    unsigned int clientid;
    QString name;   // Corresponds to QObject name of top-level widget

};

class QWSWindow
{
    friend class QWSServer;
public:
    QWSWindow(int i, QWSClient* client)
	: id(i), alloc_region_idx(-1), modified(FALSE), needAck(FALSE),
	    onTop(FALSE), c(client), last_focus_time(0)
	{ }
    ~QWSWindow();

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
    void setAllocationIndex( int i ) { alloc_region_idx = i; modified = TRUE; }
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
#ifndef QT_NO_SOUND
class QWSSoundServer;
#ifdef QT_USE_OLD_QWS_SOUND
class QWSSoundServerData;

class QWSSoundServer : public QObject {
    Q_OBJECT
public:
    QWSSoundServer(QObject* parent);
    ~QWSSoundServer();
    void playFile(const QString& filename);
private slots:
    void feedDevice(int fd);
private:
    QWSSoundServerData* d;
};
#endif
#endif

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

class QWSMouseHandler;
struct QWSCommandStruct;
#ifndef QT_NO_QWS_MULTIPROCESS
class QWSServer : private QWSServerSocket
#else
class QWSServer : private QObject
#endif
{
    friend class QCopChannel;
    friend class QWSMouseHandler;
    Q_OBJECT

public:
    QWSServer( int flags = 0, QObject *parent=0, const char *name=0 );
    ~QWSServer();
#ifndef QT_NO_QWS_MULTIPROCESS
    void newConnection( int socket );
#endif
    enum ServerFlags { DisableKeyboard = 0x01,
		       DisableMouse = 0x02 };


    enum GUIMode { NoGui = FALSE, NormalGUI = TRUE, Server };

    static void sendKeyEvent(int unicode, int keycode, int modifiers, bool isPress,
			     bool autoRepeat);
    static void processKeyEvent(int unicode, int keycode, int modifiers, bool isPress,
				bool autoRepeat);

    typedef struct KeyMap {
	ushort key_code;
	ushort unicode;
	ushort shift_unicode;
	ushort ctrl_unicode;
    };


    static const KeyMap *keyMap();
#ifndef QT_NO_QWS_KEYBOARD
    class KeyboardFilter
    {
    public:
	virtual bool filter(int unicode, int keycode, int modifiers, bool isPress,
		      bool autoRepeat)=0;
    };

    static void setKeyboardFilter( KeyboardFilter *f );
#endif
    static void setDefaultMouse( const char * );
    static void setDefaultKeyboard( const char * );
    static void setMaxWindowRect(const QRect&);
    static void sendMaxWindowRectEvents();
    static void sendMouseEvent(const QPoint& pos, int state);

    static void setDesktopBackground( const QImage &img );
    static void setDesktopBackground( const QColor & );
    static QWSMouseHandler *mouseHandler();
    static void setMouseHandler(QWSMouseHandler*);
#ifndef QT_NO_QWS_KEYBOARD    
    static QWSKeyboardHandler* keyboardHandler();
    static void setKeyboardHandler(QWSKeyboardHandler* kh);
#endif

    static QList<QWSInternalWindowInfo> * windowList();

    void sendPropertyNotifyEvent( int property, int state );
#ifndef QT_NO_QWS_PROPERTIES
    QWSPropertyManager *manager() {
	return &propertyManager;
    }
#endif
#ifndef QT_NO_COP
    static void sendQCopEvent( QWSClient *c, const QCString &ch,
			       const QCString &msg, const QByteArray &data,
			       bool response = FALSE );
#endif
    QWSWindow *windowAt( const QPoint& pos );

    // For debugging only at this time
    QList<QWSWindow> clientWindows() { return windows; }

    void openMouse();
    void closeMouse();
#ifndef QT_NO_QWS_KEYBOARD
    void openKeyboard();
    void closeKeyboard();
#endif
    void refresh();
    void enablePainting(bool);

    static void processEventQueue();
    static void setScreenSaverInterval(int);
    static bool screenSaverActive();
    static void screenSaverActivate(bool);

public:
    static void move_region( const QWSRegionMoveCommand * );
    static void set_altitude( const QWSChangeAltitudeCommand * );
    static void request_region( int, QRegion );
    static void startup( int flags );
    static void closedown();

    static void emergency_cleanup();

    static QPoint mousePosition;

private:
    static QWSServer *qwsServer; //there can be only one
    static QColor *bgColor;
    static QImage *bgImage;

private:
    void invokeCreate( QWSCreateCommand *cmd, QWSClient *client );
    void invokeRegion( QWSRegionCommand *cmd, QWSClient *client );
    void invokeRegionMove( const QWSRegionMoveCommand *cmd, QWSClient *client );
    void invokeRegionDestroy( QWSRegionDestroyCommand *cmd, QWSClient *client );
    void invokeSetAltitude( const QWSChangeAltitudeCommand *cmd, QWSClient *client );
#ifndef QT_NO_QWS_PROPERTIES
    void invokeAddProperty( QWSAddPropertyCommand *cmd );
    void invokeSetProperty( QWSSetPropertyCommand *cmd );
    void invokeRemoveProperty( QWSRemovePropertyCommand *cmd );
    void invokeGetProperty( QWSGetPropertyCommand *cmd, QWSClient *client );
#endif //QT_NO_QWS_PROPERTIES
    void invokeSetSelectionOwner( QWSSetSelectionOwnerCommand *cmd );
    void invokeConvertSelection( QWSConvertSelectionCommand *cmd );
    void invokeSetFocus( QWSRequestFocusCommand *cmd, QWSClient *client );

    void initIO();
    void setFocus( QWSWindow*, bool gain );
#ifndef QT_NO_QWS_CURSOR
    void invokeDefineCursor( QWSDefineCursorCommand *cmd, QWSClient *client );
    void invokeSelectCursor( QWSSelectCursorCommand *cmd, QWSClient *client );
#endif
    void invokeGrabMouse( QWSGrabMouseCommand *cmd, QWSClient *client );
#ifndef QT_NO_SOUND
    void invokePlaySound( QWSPlaySoundCommand *cmd, QWSClient *client );
#endif
#ifndef QT_NO_COP
    void invokeRegisterChannel( QWSQCopRegisterChannelCommand *cmd,
				QWSClient *client );
    void invokeQCopSend( QWSQCopSendCommand *cmd, QWSClient *client );

#endif

    QWSMouseHandler* newMouseHandler(const QString& spec);
#ifndef QT_NO_QWS_KEYBOARD
    QWSKeyboardHandler* newKeyboardHandler(const QString& spec);
#endif
    void openDisplay();
    void closeDisplay();

    void showCursor();
    void hideCursor();
    void initializeCursor();
    void paintServerRegion();
    void paintBackground( QRegion );
    void refreshBackground();

private slots:
#ifndef QT_NO_QWS_MULTIPROCESS
    void clientClosed();
    void doClient();
#endif
    void setMouse(const QPoint& pos,int bstate);

    void screenSaverWake();
    void screenSaverSleep();

private:
    void doClient( QWSClient * );
    typedef QMapIterator<int,QWSClient*> ClientIterator;
    typedef QMap<int,QWSClient*> ClientMap;

    uchar* sharedram;
    int ramlen;

    QGfx *gfx;

    ClientMap client;
#ifndef QT_NO_QWS_PROPERTIES
    QWSPropertyManager propertyManager;
#endif
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
    QTimer* screensavertimer;
    int screensaverinterval;

    QWSWindow *focusw;
    QWSWindow *mouseGrabber;
    bool mouseGrabbing;
    int swidth, sheight, sdepth;
#ifndef QT_NO_QWS_CURSOR
    bool cursorNeedsUpdate;
    QWSCursor *cursor;	    // cursor currently shown
    QWSCursor *nextCursor;  // cursor to show once grabbing is off
#endif
    QRegion screenRegion;   // the entire display region
    QRegion serverRegion;
    QRegion dirtyBackground;
    bool disablePainting;
    QList<QWSMouseHandler> mousehandlers;
#ifndef QT_NO_QWS_KEYBOARD
    QList<QWSKeyboardHandler> keyboardhandlers;
#endif

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

    // multimedia
#ifndef QT_NO_SOUND
    QWSSoundServer *soundserver;
#endif
#ifndef QT_NO_COP
    QMap<QString, QList<QWSClient> > channels;
#endif
};



/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

struct QWSMouseEvent;

typedef QMap<int, QWSCursor*> QWSCursorMap;

class QWSClient : public QObject
{
    Q_OBJECT
public:
    QWSClient( QObject* parent, int socket );
    ~QWSClient();

    int socket() const;

    void sendEvent( QWSEvent* event );
    void sendConnectedEvent( const char *display_spec );
    void sendMaxWindowRectEvent();
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
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSSocket *csocket;
#endif
    QWSCommand* command;
    uint isClosed : 1;
};

#endif
