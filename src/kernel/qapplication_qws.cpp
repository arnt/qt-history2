/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_qws.cpp#8 $
**
** Implementation of Qt/FB startup routines and event handling
**
** Created : 991025
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

// Started with from qapplication_x11.cpp,v 2.399 1999/10/22 14:39:33

#define select		_qt_hide_select
#define gettimeofday	_qt_hide_gettimeofday

#include "qglobal.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qtextcodec.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qsocketnotifier.h"
#include "qsessionmanager.h"
#include "qvaluelist.h"
#include "qdict.h"
#include "qqueue.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qbitmap.h"
#include "qwssocket_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsproperty_qws.h"
#include "qgfx_qws.h"
#include "qfontmanager_qws.h"
#include "qlock_qws.h"
#include "qmemorymanager_qws.h"
#include "qwsmanager_qws.h"
#include "qwsregionmanager_qws.h"
#include "qwindowsystem_qws.h"
#include "qwsdisplay_qws.h"

#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdlib.h>
#ifdef QT_SM_SUPPORT
#include <pwd.h>
#endif
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#define	 GC GC_QQQ

#if defined(_OS_LINUX_) && defined(DEBUG)
#include "qfile.h"
#include <unistd.h>
#endif

#if defined(_OS_IRIX_)
#include <bstring.h>
#endif

#include <sys/time.h>

#if defined(_OS_AIX_) && defined(_CC_GNU_)
#include <sys/select.h>
#include <unistd.h>
#endif

#if defined(_OS_QNX_)
#include <sys/select.h>
#endif


QLock *QWSDisplay::lock = 0;



//these used to be environment variables, they are initialized from
//environment variables in

bool qws_smoothfonts = TRUE;
bool qws_savefonts = FALSE;
bool qws_shared_memory = FALSE;
bool qws_sw_cursor = TRUE;
#ifndef QT_NO_QWS_MANAGER
static QWSDecoration *qws_decoration = 0;
#endif
#undef gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );
#undef select
extern "C" int select( int, void *, void *, void *, struct timeval * );

#ifdef DEBUG
/*
extern "C" void dumpmem(const char* m)
{
    static int init=0;
    static int prev=0;
    FILE* f = fopen("/proc/meminfo","r");
    //    char line[100];
    int total=0,used=0,free=0,shared=0,buffers=0,cached=0;
    fscanf(f,"%*[^M]Mem: %d %d %d %d %d %d",&total,&used,&free,&shared,&buffers,&cached);
    used -= buffers + cached;
    if (!init) {
        init=used;
    } else {
        printf("%40s: %+8d = %8d\n",m,used-init-prev,used-init);
        prev = used-init;
    }
    fclose(f);
}
*/
#endif

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

class QWSDisplay;

static char    *appName;			// application name
static char    *appFont		= 0;		// application font
static char    *appBGCol	= 0;		// application bg color
static char    *appFGCol	= 0;		// application fg color
static char    *appBTNCol	= 0;		// application btn color
static char    *mwGeometry	= 0;		// main widget geometry
static char    *mwTitle		= 0;		// main widget title
//static bool	mwIconic	= FALSE;	// main widget iconified

static bool	app_do_modal	= FALSE;	// modal mode
QWSDisplay*	qt_fbdpy = 0;			// QWS `display'
static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static int	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse position in act window

static QWidgetList *modal_stack  = 0;		// stack of modal widgets
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
static bool	    popupGrabOk;

static bool sm_blockUserInput = FALSE;		// session management

// one day in the future we will be able to have static objects in libraries....
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups


typedef void  (*VFPTR)();
typedef QList<void> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

static void	initTimers();
static void	cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval        *qt_wait_timer();
int	        qt_activate_timers();

QObject	       *qt_clipboard = 0;

QWidget	       *qt_button_down	 = 0;		// widget got last button-down
static WId	qt_last_cursor = 0xffffffff;  // Was -1, but WIds are unsigned

extern bool qt_is_gui_used; // qwidget.cpp

class QWSMouseEvent;
class QWSKeyEvent;

class QETWidget : public QWidget		// event translator widget
{
public:
    void setWState( WFlags f )		{ QWidget::setWState(f); }
    void clearWState( WFlags f )	{ QWidget::clearWState(f); }
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
    bool translateMouseEvent( const QWSMouseEvent * );
    bool dispatchMouseEvent( const QWSMouseEvent * );

    bool translateKeyEvent( const QWSKeyEvent *, bool grab );
    bool translateRegionModifiedEvent( const QWSRegionModifiedEvent * );
    bool translateWheelEvent( int global_x, int global_y, int delta, int state );
    void repaintHierarchy(QRegion r);
    void repaintDecoration(QRegion r);
    void restrictRegion( QRegion r );
};



// Single-process stuff. This should maybe move into qwindowsystem_qws.cpp

static bool qws_single_process;
static QQueue<QWSEvent> incoming;
static QQueue<QWSCommand> outgoing;

void qt_client_enqueue(const QWSEvent *event )
{
    QWSEvent *copy = QWSEvent::factory( event->type );
    copy->copyFrom( event );
    incoming.enqueue( copy );
}

QQueue<QWSCommand> *qt_get_server_queue()
{
    return &outgoing;
}

void qt_server_enqueue( const QWSCommand *command )
{
    QWSCommand *copy = QWSCommand::factory( command->type );
    copy->copyFrom( command );
    outgoing.enqueue( copy );
}

class QWSDisplayData {
public:
    QWSDisplayData( QObject* parent, bool singleProcess = FALSE )
    {
	if ( singleProcess )
	    csocket = 0;
	else
	    csocket = new QWSSocket(parent);
	init();
    }

    //####public data members

    QWSRegionManager *rgnMan;

private:
    QWSSocket *csocket;
    QList<QWSEvent> queue;
    uchar* offscreenaddress;

    QWSMouseEvent* mouse_event;
    QWSRegionModifiedEvent *region_event;
    QWSRegionModifiedEvent *region_ack;
    QWSEvent* current_event;
    QValueList<int> unused_identifiers;

    enum { VariableEvent=-1 };
public:
    bool queueNotEmpty()
    {
	return mouse_event||region_event||queue.count() > 0;
    }
    QWSEvent *dequeue()
    {
	QWSEvent *r;
	if ( queue.count() ) {
	    r = queue.first(); queue.removeFirst();
	} else if ( mouse_event ) {
	    r = mouse_event;
	    mouse_event = 0;
	} else {
	    r = region_event;
	    region_event = 0;
	}
	return r;
    }

    QWSEvent *peek() {
	return queue.first();
    }
    bool directServerConnection() { return csocket == 0; }

    void fillQueue();
    void waitForRegionAck();
    void waitForCreation();
    void init();
    void create()
    {
	QWSCreateCommand cmd;
	if  ( csocket )
	    cmd.write( csocket );
	else
	    qt_server_enqueue( &cmd );
    }

    void sendCommand( QWSCommand & cmd )
    {
	if  ( csocket )
	    cmd.write( csocket );
	else
	    qt_server_enqueue( &cmd );
    }


    QWSEvent *readMore();



    int takeId()
    {
	// top up bag
	create();
	if ( !unused_identifiers.count() ) {
	    // We have to wait!
	    for (int o=0; o<30; o++)
		create();
	    waitForCreation();
	}
	QValueList<int>::Iterator head = unused_identifiers.begin();
	int i = *head;
	unused_identifiers.remove(head);
	return i;
    }
};



void QWSDisplayData::init()
{
    region_ack = 0;
    mouse_event = 0;
    region_event = 0;
    current_event = 0;

    if ( csocket )
	csocket->connectToLocalFile(QTE_PIPE);

    if ( !QWSDisplay::initLock( "/dev/fb0" ) )
	qFatal( "Cannot get display lock" );

    key_t memkey =  ftok( "/dev/fb0", 'm' );
    //    qDebug( "Trying shmid %x", memkey );


    int ramid=shmget(memkey,0,0);
    if ( ramid == -1 ) {
	perror( "Cannot find main shared memory" );
	exit(1);
    }
    struct shmid_ds shminfo;
    if ( shmctl( ramid, IPC_STAT, &shminfo ) == -1 )
	qFatal( "Cannot get main ram memory status" );
    int ramsize=shminfo.shm_segsz;

    offscreenaddress=(uchar *)shmat(ramid,0,0);
    if(offscreenaddress==(uchar *)-1) {
	perror("Can't attach to main ram memory.");
	exit(1);
    }

    qt_probe_bus();

    int screensize=qt_screen->screenSize();

    int mouseoffset = 0;

#ifndef QT_NO_QWS_CURSOR
    mouseoffset=qt_screen->initCursor(offscreenaddress + ramsize);
#endif

    ramsize-=mouseoffset;

    /* Initialise framebuffer memory manager */
    /* Add 4k for luck and to avoid clobbering hardware cursor */
    memorymanager=new QMemoryManager(qt_screen->base()+screensize+4096,
	qt_screen->totalSize()-(screensize+4096),0);

    rgnMan = new QWSRegionManager( TRUE );

    // Create some object ID's in advance.
    // XXX server should just send some
    for (int o=0; o<10; o++)
	create();
    if ( csocket )
	csocket->flush();
}


QWSEvent* QWSDisplayData::readMore()
{
    if ( !csocket )
	return incoming.dequeue();
    // read next event
    if ( !current_event ) {
	int event_type = qws_read_uint( csocket );

	if ( event_type >= 0 ) {
	    current_event = QWSEvent::factory( event_type );
	}
    }

    if ( current_event ) {
	if ( current_event->read( csocket ) ) {
	    // Finished reading a whole event.
	    QWSEvent* result = current_event;
	    current_event = 0;
	    return result;
	}
    }

    // Not finished reading a whole event.
    return 0;
}


void QWSDisplayData::fillQueue()
{
    QWSServer::processEventQueue();
    QWSEvent *e = readMore();
    while ( e ) {
  	if ( e->type == QWSEvent::Creation ) {
	    QWSCreationEvent *ce = (QWSCreationEvent*)e;
	    unused_identifiers.append(ce->simpleData.objectid);
	    delete e;
	} else if ( e->type == QWSEvent::Mouse ) {
	    if ( mouse_event ) {
		if ( (mouse_event->window() != e->window ()
		      || mouse_event->simpleData.state !=
		      ((QWSMouseEvent*)e)->simpleData.state )) {
		    queue.append( mouse_event );
		} else {
		    delete mouse_event;
		}
	    }
	    mouse_event = (QWSMouseEvent*)e;
	} else if ( e->type == QWSEvent::RegionModified ) {
	    QWSRegionModifiedEvent *re = (QWSRegionModifiedEvent *)e;
	    if ( re->simpleData.is_ack ) {
		region_ack = re;
	    } else if ( (!region_event || re->window() == region_event->window() ) ) {
		if ( region_event ) {
		    QRegion r1;
		    r1.setRects( re->rectangles, re->simpleData.nrectangles );
		    QRegion r2;
		    r2.setRects( region_event->rectangles,
				 region_event->simpleData.nrectangles );
		    QRegion ur = r1 + r2;
		    region_event->setData( (char *)ur.rects().data(),
					    ur.rects().count() * sizeof(QRect), TRUE );
		    region_event->simpleData.nrectangles = ur.rects().count();
		    delete e;
		} else {
		    region_event = re;
		}
	    } else {
		queue.append(e);
	    }
	} else {
	    queue.append(e);
	}
	e = readMore();
    }
}



void QWSDisplayData::waitForRegionAck()
{
    if ( csocket )
	csocket->flush();
    while ( 1 ) {
	fillQueue();
	if ( region_ack )
	    break;
	if ( csocket )
	    csocket->waitForMore(1000);
    }
    queue.prepend(region_ack);
    region_ack = 0;
}

void QWSDisplayData::waitForCreation()
{
    if ( csocket )
	csocket->flush();
    fillQueue();
    while ( unused_identifiers.count() == 0 ) {
	if ( csocket )
	    csocket->waitForMore(1000);
	fillQueue();
    }
}


QWSDisplay::QWSDisplay()
{
    d = new QWSDisplayData( 0, qws_single_process );
}

QGfx * QWSDisplay::screenGfx()
{
    // We need something a little cleverer here when we get to hardware
    // acceleration
    return qt_screen->screenGfx();
}

QWSRegionManager *QWSDisplay::regionManager()
{
    return d->rgnMan;
}

bool QWSDisplay::eventPending() const
{
    d->fillQueue();
    return d->queueNotEmpty();
}


/*
  Caller must delete return value!
 */
QWSEvent*  QWSDisplay::getEvent()
{
    d->fillQueue();
    ASSERT(d->queueNotEmpty());
    QWSEvent* e = d->dequeue();

    return e;
}

uchar* QWSDisplay::frameBuffer() const { return qt_screen->base(); }
int QWSDisplay::width() const { return qt_screen->width(); }
int QWSDisplay::height() const { return qt_screen->height(); }
int QWSDisplay::depth() const { return qt_screen->depth(); }
int QWSDisplay::greenDepth() const { return qt_screen->depth()==16 ? 6 : 8; }

void QWSDisplay::addProperty( int winId, int property )
{
    QWSAddPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand( cmd );
}

void QWSDisplay::setProperty( int winId, int property, int mode, const QByteArray &data )
{
    QWSSetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    cmd.simpleData.mode = mode;
    cmd.setData( data.data(), data.size() );
    d->sendCommand( cmd );
}

void QWSDisplay::removeProperty( int winId, int property )
{
    QWSRemovePropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand( cmd );
}

bool QWSDisplay::getProperty( int winId, int property, char *&data, int &len )
{
    QWSGetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand( cmd );

    getPropertyLen = -2;
    getPropertyData = 0;

    while ( getPropertyLen == -2 )
	qApp->processEvents();

    len = getPropertyLen;
    data = getPropertyData;

    getPropertyLen = -2;
    getPropertyData = 0;

    return len != -1;
}

void QWSDisplay::setAltitude(int winId, int alt, bool fixed )
{
    QWSChangeAltitudeCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.altitude = alt;
    cmd.simpleData.fixed = fixed;
    if ( d->directServerConnection() ) {
	QWSServer::set_altitude( &cmd );
    } else {
	d->sendCommand( cmd );
    }
    d->waitForRegionAck();
}

void QWSDisplay::requestFocus(int winId, bool get)
{
    QWSRequestFocusCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.flag = get;
    d->sendCommand( cmd );
}

//### should change signature to requestRegion(QWidget*,QRegion)
void QWSDisplay::requestRegion(int winId, QRegion r)
{
    if ( d->directServerConnection() ) {
	QWSServer::request_region( winId, r );
    } else {
	//by sending the event, I promise not to paint outside the region
	QETWidget *widget = (QETWidget*)QWidget::find( (WId)winId );
	widget->restrictRegion( r ); //### probably not needed anymore.

	QArray<QRect> ra = r.rects();

	/*
	  for ( int i = 0; i < ra.size(); i++ ) {
	  QRect r( ra[i] );
	  qDebug("rect: %d %d %d %d", r.x(), r.y(), r.right(), r.bottom() );
	  }
	*/

	QWSRegionCommand cmd;
	cmd.simpleData.windowid = winId;
	cmd.simpleData.nrectangles = ra.count();
	cmd.setData( (char *)ra.data(), ra.count() * sizeof(QRect), FALSE);
	d->sendCommand( cmd );
    }
    if ( !r.isEmpty() )
	d->waitForRegionAck();
}

void QWSDisplay::moveRegion( int winId, int dx, int dy )
{
    //UNUSED QETWidget *widget = (QETWidget*)QWidget::find( (WId)winId );

    QWSRegionMoveCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.dx = dx;
    cmd.simpleData.dy = dy;

    if ( d->directServerConnection() ) {
	QWSServer::move_region( &cmd );
    } else {
	d->sendCommand( cmd );
    }
    d->waitForRegionAck();
}
void QWSDisplay::destroyRegion( int winId )
{
    QWSRegionDestroyCommand cmd;
    cmd.simpleData.windowid = winId;
    d->sendCommand( cmd );
}

int QWSDisplay::takeId()
{
    return d->takeId();
}

bool QWSDisplay::initLock( const QString &filename, bool create )
{
    if ( !lock ) {
	lock = new QLock( filename, 'd', create );

	if ( !lock->isValid() ) {
	    delete lock;
	    lock = 0;
	    return FALSE;
	}
    }

    return TRUE;
}

void QWSDisplay::setSelectionOwner( int winId, const QTime &time )
{
    QWSSetSelectionOwnerCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.hour = time.hour();
    cmd.simpleData.minute = time.minute();
    cmd.simpleData.sec = time.second();
    cmd.simpleData.ms = time.msec();
    d->sendCommand( cmd );
}

void QWSDisplay::convertSelection( int winId, int selectionProperty, const QString &mimeTypes )
{
#ifndef QT_NO_QWS_PROPERTIES
    // ### we need the atom/property thingy like in X here
    addProperty( winId, 999 );
    setProperty( winId, 999,
		 (int)QWSPropertyManager::PropReplace, QCString( mimeTypes.latin1() ) );
#endif
    QWSConvertSelectionCommand cmd;
    cmd.simpleData.requestor = winId;
    cmd.simpleData.selection = selectionProperty;
    cmd.simpleData.mimeTypes = 999;
    d->sendCommand( cmd );
}

void QWSDisplay::defineCursor(int id, const QBitmap &curs, const QBitmap &mask,
			    int hotX, int hotY)
{
    QImage cursImg = curs.convertToImage();
    QImage maskImg = mask.convertToImage();

    QWSDefineCursorCommand cmd;
    cmd.simpleData.width = curs.width();
    cmd.simpleData.height = curs.height();
    cmd.simpleData.hotX = hotX;
    cmd.simpleData.hotY = hotY;
    cmd.simpleData.id = id;

    int dataLen = cursImg.numBytes();

    unsigned char *data = new unsigned char [dataLen*2];
    memcpy(data, cursImg.bits(), dataLen);
    memcpy(data + dataLen, maskImg.bits(), dataLen);

    cmd.setData( (char*)data, dataLen*2 );
    delete [] data;
    d->sendCommand( cmd );
}

#ifndef QT_NO_SOUND
void QWSDisplay::playSoundFile(const QString& f)
{
    QWSPlaySoundCommand cmd;
    cmd.setFilename(f);
    d->sendCommand( cmd );
}
#endif

void QWSDisplay::selectCursor( QWidget *w, unsigned int cursId )
{
    if (cursId != qt_last_cursor)
    {
	QWidget *top = w->topLevelWidget();
	qt_last_cursor = cursId;
	QWSSelectCursorCommand cmd;
	cmd.simpleData.windowid = top->winId();
	cmd.simpleData.id = cursId;
	d->sendCommand( cmd );
    }
}

void QWSDisplay::grabMouse( QWidget *w, bool grab )
{
    QWidget *top = w->topLevelWidget();
    QWSGrabMouseCommand cmd;
    cmd.simpleData.windowid = top->winId();
    cmd.simpleData.grab = grab;
    d->sendCommand( cmd );
}


static bool	qt_try_modal( QWidget *, QWSEvent * );

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

#ifndef QT_NO_PALETTE
// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();
#endif

/*****************************************************************************
  qt_init() - initializes Qt/FB
 *****************************************************************************/



static void qt_set_qws_resources()

{
#ifndef QT_NO_PALETTE
    if ( !qt_std_pal )
	qt_create_std_palette();
    if ( appFont )
	QApplication::setFont( QFont(appFont) );

    if ( appBGCol || appBTNCol || appFGCol ) {


    	QColor btn;
	QColor bg;
	QColor fg;
	if ( appBGCol )
	    bg = QColor( appBGCol );
	else
	    bg = qt_std_pal->normal().background();
	if ( appFGCol )
	    fg = QColor( appFGCol );
	else
	    fg = qt_std_pal->normal().foreground();
	if ( appBTNCol )
	    btn = QColor( appBTNCol);
	else
	    btn = qt_std_pal->normal().button();
	int h,s,v;
	fg.hsv(&h,&s,&v);
	QColor base = Qt::white;
	bool bright_mode = FALSE;
	if (v >= 255-50) {
	    base = btn.dark(150);
	    bright_mode = TRUE;
	}

	QColorGroup cg( fg, btn, btn.light(),
			btn.dark(), btn.dark(150), fg, Qt::white, base, bg );
	if (bright_mode) {
	    cg.setColor( QColorGroup::HighlightedText, base );
	    cg.setColor( QColorGroup::Highlight, Qt::white );
	}
	else {
	    cg.setColor( QColorGroup::HighlightedText, Qt::white );
	    cg.setColor( QColorGroup::Highlight, Qt::darkBlue );
	}
	QColor disabled( (fg.red()+btn.red())/2,
			 (fg.green()+btn.green())/2,
			 (fg.blue()+btn.blue())/2);
	QColorGroup dcg( disabled, btn, btn.light( 125 ), btn.dark(), btn.dark(150),
			 disabled, Qt::white, Qt::white, bg );
	QPalette pal( cg, dcg, cg );
	if ( pal != *qt_std_pal && pal != QApplication::palette() )
	    QApplication::setPalette( pal, TRUE );
	*qt_std_pal = pal;
    }
#endif // QT_NO_PALETTE
}

static void init_display()
{
    if ( qt_fbdpy ) return; // workaround server==client case

    // Connect to FB server

    qt_fbdpy = new QWSDisplay;

    // Get display parameters

    // Set paintdevice parameters

    // XXX initial info sent from server

    // Misc. initialization

    QColor::initialize();
    QFont::initialize();
#ifndef QT_NO_CURSOR
    QCursor::initialize();
#endif
    QPainter::initialize();
    QFontManager::initialize();
#ifndef QT_NO_QWS_MANAGER
    qws_decoration = new QWSDefaultDecoration;
#endif

    qApp->setName( appName );

    QFont f;
    f = QFont( "helvetica", 10 );
    QApplication::setFont( f );

    qt_set_qws_resources();

}

void qt_init_display()
{
    qt_is_gui_used = TRUE;
    qws_single_process = TRUE;
    init_display();
}


void qt_init( int *argcptr, char **argv, QApplication::Type type )
{
    if ( type == QApplication::GuiServer )
	qt_is_gui_used = FALSE; //we'll turn it on in a second
    qws_smoothfonts = getenv("QWS_NO_SMOOTH_FONTS") == 0;
    qws_sw_cursor = getenv("QWS_SW_CURSOR") != 0;

    //qws_savefonts = getenv("QWS_SAVEFONTS") != 0;
    //qws_shared_memory = getenv("QWS_NOSHARED") == 0;

    int flags = 0;
    char *p;
    int argc = *argcptr;
    int j;

    if ( getenv("QWS_NOACCEL") )
	flags |= QWSServer::DisableAccel;

    // Set application name

    p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

    // Get command line params

    j = 1;
    for ( int i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( arg == "-fn" || arg == "-font" ) {
	    if ( ++i < argc )
		appFont = argv[i];
	} else if ( arg == "-bg" || arg == "-background" ) {
	    if ( ++i < argc )
		appBGCol = argv[i];
	} else if ( arg == "-btn" || arg == "-button" ) {
	    if ( ++i < argc )
		appBTNCol = argv[i];
	} else if ( arg == "-fg" || arg == "-foreground" ) {
	    if ( ++i < argc )
		appFGCol = argv[i];
	} else if ( arg == "-name" ) {
	    if ( ++i < argc )
		appName = argv[i];
	} else if ( arg == "-title" ) {
	    if ( ++i < argc )
		mwTitle = argv[i];
	} else if ( arg == "-geometry" ) {
	    if ( ++i < argc )
		mwGeometry = argv[i];
	} else if ( arg == "-shared" ) {
	    qws_shared_memory = TRUE;
	} else if ( arg == "-noshared" ) {
	    qws_shared_memory = FALSE;
	} else if ( arg == "-accel" ) {
	    flags &= ~QWSServer::DisableAccel;
	} else if ( arg == "-noaccel" ) {
	    flags |= QWSServer::DisableAccel;
	} else if ( arg == "-smoothfonts" ) {
	    qws_smoothfonts = TRUE;
	} else if ( arg == "-nosmoothfonts" ) {
	    qws_smoothfonts = FALSE;
	} else if ( arg == "-savefonts" ) {
	    qws_savefonts = TRUE;
	} else if ( arg == "-nosavefonts" ) {
	    qws_savefonts = FALSE;
	} else if ( arg == "-swcursor" ) {
	    qws_sw_cursor = TRUE;
	} else if ( arg == "-noswcursor" ) {
	    qws_sw_cursor = FALSE;
	} else if ( arg == "-keyboard" ) {
	    flags &= ~QWSServer::DisableKeyboard;
	} else if ( arg == "-nokeyboard" ) {
	    flags |= QWSServer::DisableKeyboard;
	} else if ( arg == "-mouse" ) {
	    flags &= ~QWSServer::DisableMouse;
	} else if ( arg == "-nomouse" ) {
	    flags |= QWSServer::DisableMouse;
	} else if ( arg == "-qws" ) {
	    type = QApplication::GuiServer;
	} else {
	    argv[j++] = argv[i];
	}
    }

    *argcptr = j;

    gettimeofday( &watchtime, 0 );

    if ( type == QApplication::GuiServer ) {
	qws_single_process = TRUE;
	QWSServer::startup(flags);
    }

    if( qt_is_gui_used )
	init_display();
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( postRList ) {
	VFPTR f = (VFPTR)postRList->first();
	while ( f ) {				// call post routines
	    (*f)();
	    postRList->remove();
	    f = (VFPTR)postRList->first();
	}
	delete postRList;
	postRList = 0;
    }

    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
#ifndef QT_NO_CURSOR
    QCursor::cleanup();
#endif
    QFont::cleanup();
    QColor::cleanup();
    QFontManager::cleanup();

    if ( qws_single_process )
	QWSServer::closedown();
    if ( qt_is_gui_used ) {
	delete qt_fbdpy;
    }
    qt_fbdpy = 0;

    delete activeBeforePopup;
    activeBeforePopup = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qAddPostRoutine( Q_CleanUpFunction p )
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	CHECK_PTR( postRList );
    }
    postRList->insert( 0, (void *)p );		// store at list head
}


char *qAppName()				// get application name
{
    return appName;
}

/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

/* Copyright notice for ReadInteger and parseGeometry

Copyright (c) 1985, 1986, 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/
/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged.
 */

static int
ReadInteger(char *string, char **NextString)
{
    register int Result = 0;
    int Sign = 1;

    if (*string == '+')
	string++;
    else if (*string == '-')
    {
	string++;
	Sign = -1;
    }
    for (; (*string >= '0') && (*string <= '9'); string++)
    {
	Result = (Result * 10) + (*string - '0');
    }
    *NextString = string;
    if (Sign >= 0)
	return (Result);
    else
	return (-Result);
}

static int parseGeometry( const char* string,
			  int* x, int* y, int* width, int* height )
{
	int mask = NoValue;
	register char *strind;
	unsigned int tempWidth=0, tempHeight=0;
	int tempX=0, tempY=0;
	char *nextCharacter;

	if ( (string == NULL) || (*string == '\0')) return(mask);
	if (*string == '=')
		string++;  /* ignore possible '=' at beg of geometry spec */

	strind = (char *)string;
	if (*strind != '+' && *strind != '-' && *strind != 'x') {
		tempWidth = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= WidthValue;
	}

	if (*strind == 'x' || *strind == 'X') {
		strind++;
		tempHeight = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= HeightValue;
	}

	if ((*strind == '+') || (*strind == '-')) {
		if (*strind == '-') {
  			strind++;
			tempX = -ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return (0);
			strind = nextCharacter;
			mask |= XNegative;

		}
		else
		{	strind++;
			tempX = ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return(0);
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-')) {
			if (*strind == '-') {
				strind++;
				tempY = -ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
				mask |= YNegative;

			}
			else
			{
				strind++;
				tempY = ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
			}
			mask |= YValue;
		}
	}

	/* If strind isn't at the end of the string the it's an invalid
		geometry specification. */

	if (*strind != '\0') return (0);

	if (mask & XValue)
	    *x = tempX;
 	if (mask & YValue)
	    *y = tempY;
	if (mask & WidthValue)
            *width = tempWidth;
	if (mask & HeightValue)
            *height = tempHeight;
	return (mask);
}


void QApplication::setMainWidget( QWidget *mainWidget )
{
    extern int qwidget_tlw_gravity;		// in qwidget_qws.cpp

    main_widget = mainWidget;
    if ( main_widget ) {			// give WM command line
	if ( mwTitle ) {
	    // XXX
	}
	if ( mwGeometry ) {			// parse geometry
	    int x, y;
	    int w, h;
	    int m = parseGeometry( mwGeometry, &x, &y, &w, &h );
	    QSize minSize = main_widget->minimumSize();
	    QSize maxSize = main_widget->maximumSize();
	    if ( (m & XValue) == 0 )
		x = main_widget->geometry().x();
	    if ( (m & YValue) == 0 )
		y = main_widget->geometry().y();
	    if ( (m & WidthValue) == 0 )
		w = main_widget->width();
	    if ( (m & HeightValue) == 0 )
		h = main_widget->height();
	    w = QMIN(w,maxSize.width());
	    h = QMIN(h,maxSize.height());
	    w = QMAX(w,minSize.width());
	    h = QMAX(h,minSize.height());
	    if ( (m & XNegative) ) {
		x = desktop()->width()  + x - w;
		qwidget_tlw_gravity = 3;
	    }
	    if ( (m & YNegative) ) {
		y = desktop()->height() + y - h;
		qwidget_tlw_gravity = (m & XNegative) ? 9 : 7;
	    }
	    main_widget->setGeometry( x, y, w, h );
	}
    }
}


/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/
#ifndef QT_NO_CURSOR
typedef QList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;
void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
    QWidget *w = QWidget::mouseGrabber();
    if ( !w )
	w = desktop();
    QPaintDevice::qwsDisplay()->selectCursor(w, (int)app_cursor->handle());
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    QWidget *w = QWidget::mouseGrabber();
    if ( !w )
	w = desktop();
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
	QPaintDevice::qwsDisplay()->selectCursor(w, ArrowCursor);
    } else
	QPaintDevice::qwsDisplay()->selectCursor(w, (int)app_cursor->handle());
}
#endif// QT_NO_CURSOR


void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
	tellAllWidgets = (++app_tracking == 1);
    } else {
	tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {
	    if ( app_tracking > 0 ) {		// switch on
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( TRUE );
		    w->clearWState(WState_MouseTracking);
		}
	    } else {				// switch off
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setWState(WState_MouseTracking);
		    w->setMouseTracking( FALSE );
		}
	    }
	    ++it;
	}
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos );

static QWidget *findWidget( const QObjectList& list, const QPoint &pos, bool rec )
{
    QWidget *w;
    QObjectListIt it( list );
    it.toLast();
    while ( it.current() ) {
	if ( it.current()->isWidgetType() ) {
	    w = (QWidget*)it.current();
	    if ( w->isVisible() && w->geometry().contains(pos) ) {
		if ( !rec )
		    return w;
		QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		return c ? c : w;
	    }
	}
	--it;
    }
    return 0;
}

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	return findWidget( *p->children(), pos, TRUE );
    }
    return 0;
}

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    // XXX not a fast function...
    QWidgetList *list = topLevelWidgets();

    QPoint pos(x,y);

    if ( list ) {
	QWidget *w;
	QWidgetListIt it( *list );
	it.toLast();
	while ( it.current() ) {
	    w = (QWidget*)it.current();
	    if ( w->isVisible() && w->geometry().contains(pos) ) {
		if ( !child )
		    return w;
		QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		return c ? c : w;
	    }
	    --it;
	}
	delete list;
	return 0;
    } else {
	return 0;
    }
}

void QApplication::beep()
{
}



/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

struct QSockNot {
    QObject *obj;
    int	     fd;
    fd_set  *queue;
};

typedef QList<QSockNot> QSNList;
typedef QListIterator<QSockNot> QSNListIt;

static int	sn_highest = -1;
static QSNList *sn_read	   = 0;
static QSNList *sn_write   = 0;
static QSNList *sn_except  = 0;

static fd_set	sn_readfds;			// fd set for reading
static fd_set	sn_writefds;			// fd set for writing
static fd_set	sn_exceptfds;			// fd set for exceptions
static fd_set	sn_queued_read;
static fd_set	sn_queued_write;
static fd_set	sn_queued_except;

static struct SN_Type {
    QSNList **list;
    fd_set   *fdspec;
    fd_set   *fdres;
    fd_set   *queue;
} sn_vec[3] = {
    { &sn_read,	  &sn_readfds,	 &app_readfds,   &sn_queued_read },
    { &sn_write,  &sn_writefds,	 &app_writefds,  &sn_queued_write },
    { &sn_except, &sn_exceptfds, &app_exceptfds, &sn_queued_except } };


static QSNList *sn_act_list = 0;


static void sn_cleanup()
{
    delete sn_act_list;
    sn_act_list = 0;
    for ( int i=0; i<3; i++ ) {
	delete *sn_vec[i].list;
	*sn_vec[i].list = 0;
    }
}


static void sn_init()
{
    if ( !sn_act_list ) {
	sn_act_list = new QSNList;
	CHECK_PTR( sn_act_list );
	qAddPostRoutine( sn_cleanup );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNList  *list = *sn_vec[type].list;
    fd_set   *fds  =  sn_vec[type].fdspec;
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( !list ) {
	    sn_init();
	    list = new QSNList;			// create new list
	    CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
	sn->queue = sn_vec[type].queue;
	if ( list->isEmpty() ) {
	    list->insert( 0, sn );
	} else {				// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(CHECK_STATE)
	    if ( p && p->fd == sockfd ) {
		static const char *t[] = { "read", "write", "exception" };
		qWarning( "QSocketNotifier: Multiple socket notifiers for "
			 "same socket %d and type %s", sockfd, t[type] );
	    }
#endif
	    if ( p )
		list->insert( list->at(), sn );
	    else
		list->append( sn );
	}
	FD_SET( sockfd, fds );
	sn_highest = QMAX(sn_highest,sockfd);

    } else {					// disable notifier

	if ( list == 0 )
	    return FALSE;			// no such fd set
	QSockNot *sn = list->first();
	while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
	    sn = list->next();
	if ( !sn )				// not found
	    return FALSE;
	FD_CLR( sockfd, fds );			// clear fd bit
	FD_CLR( sockfd, sn->queue );
	if ( sn_act_list )
	    sn_act_list->removeRef( sn );	// remove from activation list
	list->remove();				// remove notifier found above
	if ( sn_highest == sockfd ) {		// find highest fd
	    sn_highest = -1;
	    for ( int i=0; i<3; i++ ) {
		if ( *sn_vec[i].list && (*sn_vec[i].list)->count() )
		    sn_highest = QMAX(sn_highest,  // list is fd-sorted
				      (*sn_vec[i].list)->getFirst()->fd);
	    }
	}
    }

    return TRUE;
}


//
// We choose a random activation order to be more fair under high load.
// If a constant order is used and a peer early in the list can
// saturate the IO, it might grab our attention completely.
// Also, if we're using a straight list, the callback routines may
// delete other entries from the list before those other entries are
// processed.
//

static int sn_activate()
{
    if ( !sn_act_list )
	sn_init();
    int i, n_act = 0;
    for ( i=0; i<3; i++ ) {			// for each list...
	if ( *sn_vec[i].list ) {		// any entries?
	    QSNList  *list = *sn_vec[i].list;
	    fd_set   *fds  = sn_vec[i].fdres;
	    QSockNot *sn   = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, fds ) &&	// store away for activation
		     !FD_ISSET( sn->fd, sn->queue ) ) {
		    sn_act_list->insert( (rand() & 0xff) %
					 (sn_act_list->count()+1),
					 sn );
		    FD_SET( sn->fd, sn->queue );
		}
		sn = list->next();
	    }
	}
    }
    if ( sn_act_list->count() > 0 ) {		// activate entries
	QEvent event( QEvent::SockAct );
	QSNListIt it( *sn_act_list );
	QSockNot *sn;
	while ( (sn=it.current()) ) {
	    ++it;
	    sn_act_list->removeRef( sn );
	    if ( FD_ISSET(sn->fd, sn->queue) ) {
		FD_CLR( sn->fd, sn->queue );
		QApplication::sendEvent( sn->obj, &event );
		n_act++;
	    }
	}
    }
    return n_act;
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

int QApplication::exec()
{
    quit_now = FALSE;
    quit_code = 0;
    enter_loop();
    return quit_code;
}


bool QApplication::processNextEvent( bool canWait )
{
    int	   nevents = 0;

    if (qt_is_gui_used ) {
	sendPostedEvents();

	while ( qt_fbdpy->eventPending() ) {	// also flushes output buffer
	    if ( app_exit_loop )		// quit between events
		return FALSE;
	    QWSEvent *event = qt_fbdpy->getEvent();	// get next event
	    nevents++;

	    bool ret = qwsProcessEvent( event ) == 1;
	    delete event;
	    if ( ret )
		return TRUE;
	}
    }
    if ( app_exit_loop )			// break immediately
	return FALSE;

    sendPostedEvents();

    if ( !outgoing.isEmpty() )
	QWSServer::processEventQueue();

    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or X event
    if ( !incoming.isEmpty() )
	canWait = FALSE;
    if ( !canWait ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }
    if ( sn_highest >= 0 ) {			// has socket notifier(s)
	if ( sn_read )
	    app_readfds = sn_readfds;
	else
	    FD_ZERO( &app_readfds );
	if ( sn_write )
	    app_writefds = sn_writefds;
	if ( sn_except )
	    app_exceptfds = sn_exceptfds;
    } else {
	FD_ZERO( &app_readfds );
    }

    int nsel;

#if defined(_OS_WIN32_)
#define FDCAST (fd_set*)
#else
#define FDCAST (void*)
#endif

    nsel = select( sn_highest+1,
		   FDCAST (&app_readfds),
		   FDCAST (sn_write  ? &app_writefds  : 0),
		   FDCAST (sn_except ? &app_exceptfds : 0),
		   tm );
#undef FDCAST

    if ( nsel == -1 ) {
	if ( errno == EINTR || errno == EAGAIN ) {
	    errno = 0;
	    return (nevents > 0);
	} else {
	    // select error
	}
    } else if ( nsel > 0 && sn_highest >= 0 ) {
	nevents += sn_activate();
    }

    nevents += qt_activate_timers();		// activate timers

    return (nevents > 0);
}

int QApplication::qwsProcessEvent( QWSEvent* event )
{
    if ( qwsEventFilter(event) )			// send through app filter
	return 1;
#ifndef QT_NO_QWS_PROPERTIES
    if ( event->type == QWSEvent::PropertyNotify ) {
	//QWSPropertyNotifyEvent *e = (QWSPropertyNotifyEvent*)event;
    } else if ( event->type == QWSEvent::PropertyReply ) {
	QWSPropertyReplyEvent *e = (QWSPropertyReplyEvent*)event;
	QPaintDevice::qwsDisplay()->getPropertyLen = e->simpleData.len;
	QPaintDevice::qwsDisplay()->getPropertyData = e->data;
    }
#endif //QT_NO_QWS_PROPERTIES
    QETWidget *widget = (QETWidget*)QWidget::find( (WId)event->window() );

    QETWidget *keywidget=0;
    bool grabbed=FALSE;
    if ( event->type==QWSEvent::Key ) {
	keywidget = (QETWidget*)QWidget::keyboardGrabber();
	if ( keywidget ) {
	    grabbed = TRUE;
	} else {
	    if ( focus_widget )
		keywidget = (QETWidget*)focus_widget;
	    else if ( widget )
		keywidget = (QETWidget*)widget->topLevelWidget();
	}
    } else if ( widget && event->type==QWSEvent::Mouse ) {
	// The mouse event is to one of my top-level widgets
	// which one?
	QPoint p(event->asMouse()->simpleData.x_root,
		 event->asMouse()->simpleData.y_root);

	QETWidget *w = (QETWidget*)QWidget::mouseGrabber();
#ifndef QT_NO_QWS_MANAGER
	if ( !w )
	    w = (QETWidget*)QWSManager::grabbedMouse();
	QWSManager *wm = widget->topData()->qwsManager;
#endif
	if (w) {
	    widget = w;
	} else
#ifndef QT_NO_QWS_MANAGER
	    if (!wm || !(wm->region().contains(p)))
#endif
		{
	    static int btnstate = 0;
	    static QWidget *gw = 0;
	    w = (QETWidget*)findChildWidget(widget, widget->mapFromParent(p));
	    w = w ? (QETWidget*)w : widget;
	    if ( event->asMouse()->simpleData.state && !btnstate ) {
		btnstate = event->asMouse()->simpleData.state;
		gw = w;
	    } else if ( !event->asMouse()->simpleData.state && btnstate ) {
		btnstate = 0;
		gw = 0;
	    }
#ifndef QT_NO_CURSOR
	    if ( !gw || gw == w ) {
		QCursor *curs = app_cursor;
		if (!curs && w->extraData())
		    curs = w->extraData()->curs;
		if (curs) {
		    QPaintDevice::qwsDisplay()->selectCursor(widget, (int)curs->handle());
		}
		else {
		    QPaintDevice::qwsDisplay()->selectCursor(widget, ArrowCursor);
		}
	    }
#endif
	    widget = w;
	}
    }

    if ( !widget ) {				// don't know this window
	qt_last_cursor = 0xffffffff; // cursor can be changed by another application

	QWidget* popup = QApplication::activePopupWidget();
	if ( popup ) {

	    /*
	      That is more than suboptimal. The real solution should
	      do some keyevent and buttonevent translation, so that
	      the popup still continues to work as the user expects.
	      Unfortunately this translation is currently only
	      possible with a known widget. I'll change that soon
	      (Matthias).
	     */

	    // Danger - make sure we don't lock the server
	    switch ( event->type ) {
	    case QWSEvent::Mouse:
	    case QWSEvent::Key:
		do {
		    popup->close();
		} while ( (popup = qApp->activePopupWidget()) );
		return 1;
	    }
	}
	return -1;
    }

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, event) ) {
	    return 1;
	}

    if ( widget->qwsEvent(event) )		// send through widget filter
	return 1;

    switch ( event->type ) {

    case QWSEvent::Mouse:			// mouse event
	widget->translateMouseEvent( event->asMouse() );
	break;

    case QWSEvent::Key:				// keyboard event
	if ( keywidget ) // should always exist
	    keywidget->translateKeyEvent( (QWSKeyEvent*)event, grabbed );
	break;

    case QWSEvent::RegionModified:
	widget->translateRegionModifiedEvent( (QWSRegionModifiedEvent*)event );
	break;

    case QWSEvent::Focus:
	if ( ((QWSFocusEvent*)event)->simpleData.get_focus ) {
	    if ( widget == desktop() )
		return TRUE; // not interesting
	    if ( inPopupMode() ) // some delayed focus event to ignore
		break;
	    active_window = widget->topLevelWidget();
	    setActiveWindow(active_window);
	    ((QETWidget *)active_window)->repaintDecoration(desktop()->rect());

	    QWidget *w = widget->focusWidget();
	    while ( w && w->focusProxy() )
		w = w->focusProxy();
	    if ( w && w->isFocusEnabled() )
		w->setFocus();
	    else
		widget->focusNextPrevChild( TRUE );
	    if ( !focus_widget ) {
		if ( widget->focusWidget() )
		    widget->focusWidget()->setFocus();
		else
		    widget->topLevelWidget()->setFocus();
	    }
	} else {	// lost focus
	    if ( widget == desktop() )
		return TRUE; // not interesting
	    if ( focus_widget && !inPopupMode() ) {
		QETWidget *old = (QETWidget *)active_window;
		setActiveWindow(0);
		//active_window = 0;
		if (old)
		    old->repaintDecoration(desktop()->rect());
		/* setActiveWindow() sends focus events
		QFocusEvent out( QEvent::FocusOut );
		QWidget *widget = focus_widget;
		focus_widget = 0;
		QApplication::sendEvent( widget, &out );
		*/
	    }
	}
	break;
    default:
	break;
    }

    return 0;
}

void QApplication::processEvents( int maxtime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !app_exit_loop && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}


/*!
  This virtual function is only implemented under Qt/Embedded.

  If you create an application that inherits QApplication and
  reimplement this function, you get direct access to all QWS
  (Q Window System) events that the are received from the QWS
  master process.

  Return TRUE if you want to stop the event from being processed, or
  return FALSE for normal event dispatching.
*/
bool QApplication::qwsEventFilter( QWSEvent * )
{
    return FALSE;
}

#ifndef QT_NO_QWS_MANAGER
/*!
  Return the QWSDecoration used for decorating windows.

  This method is non-portable.  It is available \e only in Qt/Embedded.

  \sa QWSDecoration
*/
QWSDecoration &QApplication::qwsDecoration()
{
    return *qws_decoration;
}

/*!
  Set the QWSDecoration derived class to use for decorating the Qt/Embedded
  windows.

  This method is non-portable.  It is available \e only in Qt/Embedded.

  \sa QWSDecoration
*/
void QApplication::qwsSetDecoration( QWSDecoration *d )
{
    if ( d ) {
	delete qws_decoration;
	qws_decoration = d;
    }
}
#endif

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !modal_stack ) {			// create modal stack
	modal_stack = new QWidgetList;
	CHECK_PTR( modal_stack );
    }
    modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( modal_stack && modal_stack->removeRef(widget) ) {
	if ( modal_stack->isEmpty() ) {
	    delete modal_stack;
	    modal_stack = 0;
	}
    }
    app_do_modal = modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, QWSEvent *event )
{
    if ( qApp->activePopupWidget() )
	return TRUE;

    widget = widget->topLevelWidget();
    // a bit of a hack: use WStyle_Tool as a general ignore-modality
    // flag, also for complex widgets with children.
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=modal_stack->getFirst();

    if ( widget->testWFlags(Qt::WType_Modal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )			// don't block event
	return TRUE;

#ifdef ALLOW_NON_APPLICATION_MODAL
    if ( top && top->parentWidget() ) {
	// Not application-modal
	// Does widget have a child in modal_stack?
	bool unrelated = TRUE;
	modal = modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != widget ) {
		p = p->parentWidget();
	    }
	    modal = modal_stack->next();
	    if ( p ) unrelated = FALSE;
	}
	if ( unrelated ) return TRUE;		// don't block event
    }
#endif

    bool block_event  = FALSE;
    bool paint_event = FALSE;

    switch ( event->type ) {
	case QWSEvent::Mouse:			// disallow mouse/key events
	case QWSEvent::Key:
	case QWSEvent::Focus:
	    block_event	 = TRUE;
	    break;
	case QWSEvent::RegionModified:
	    paint_event = TRUE;
	    break;
    }

    if ( top->parentWidget() == 0 && (block_event || paint_event) )
	top->raise();

    return !block_event;
}


void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 ){ // grab mouse/keyboard
	// XXX grab keyboard
    }

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    active_window = popup;
    if (active_window->focusWidget())
	active_window->focusWidget()->setFocus();
    else
	active_window->setFocus();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;

    popupWidgets->removeRef( popup );
    if (popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( popupGrabOk ) {	// grabbing not disabled
	    // XXX ungrab keyboard
	}
	active_window = (*activeBeforePopup);
	// restore the former active window immediately, although
	// we'll get a focusIn later
	if ( active_window )
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
    }
     else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	 active_window = popupWidgets->getLast();
	 if (active_window->focusWidget())
	     active_window->focusWidget()->setFocus();
	 else
	     active_window->setFocus();
     }
}


//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef QList<TimerInfo> TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list


//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline timeval &operator+=( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ( (t1.tv_usec += t2.tv_usec) >= 1000000 ) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
}

static inline timeval operator+( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000 ) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}

static inline timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}


//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//

static int allocTimerId()			// find avail timer identifier
{
    int i = timerBitVec->size()-1;
    while ( i >= 0 && (*timerBitVec)[i] )
	i--;
    if ( i < 0 ) {
	i = timerBitVec->size();
	timerBitVec->resize( 4 * i );
	for( int j=timerBitVec->size()-1; j > i; j-- )
	    timerBitVec->clearBit( j );
    }
    timerBitVec->setBit( i );
    return i+1;
}

static void insertTimer( const TimerInfo *ti )	// insert timer info into list
{
    TimerInfo *t = timerList->first();
    int index = 0;
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
	t = timerList->next();
	index++;
    }
    timerList->insert( index, ti );		// inserts sorted
}

static inline void getTime( timeval &t )	// get time of day
{
    gettimeofday( &t, 0 );
    while ( t.tv_usec >= 1000000 ) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}

static void repairTimer( const timeval &time )	// repair broken timer
{
    if ( !timerList )				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}


//
// Timer activation functions (called from the event loop)
//

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.
*/

timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if ( timerList && timerList->count() ) {	// there are waiting timers
	getTime( currentTime );
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if ( currentTime < t->timeout ) {	// time to wait
	    tm = t->timeout - currentTime;
	} else {
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	return &tm;
    }
    return 0;					// no timers
}

/*
  Activates the timer events that have expired. Returns the number of timers
  (not 0-timer) that were activated.
*/

int qt_activate_timers()
{
    if ( !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    int n_act = 0;
    register TimerInfo *t;
    while ( maxcount-- ) {			// avoid starvation
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if ( !t || currentTime < t->timeout )	// no timer has expired
	    break;
	timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
    }
    return n_act;
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	timerList = 0;
	delete timerBitVec;
	timerBitVec = 0;
    }
}


//
// Main timer functions for starting and killing timers
//

int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 ||
	 id > (int)timerBitVec->size() || !obj )// cannot create timer
	return 0;
    timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    CHECK_PTR( t );
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    return id;
}

bool qKillTimer( int id )
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 ||
	 id > (int)timerBitVec->size() || !timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	timerBitVec->clearBit( id-1 );		// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    timerBitVec->clearBit( t->id-1 );
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}


/*****************************************************************************
  Event translation; translates FB events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// FB doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

static int translateButtonState( int s )
{
    // No translation required at this time
    return s;
}

// Needed for QCursor::pos

int qt_last_x;
int qt_last_y;
static const int AnyButton = (Qt::LeftButton | Qt::MidButton | Qt::RightButton );


//The logic got a little too complex, so I split the function in
//two. This one translates a QWS event into separate move
//and press/release events
//There is code duplication for now, I don't want to rock the boat
//too much at this stage.
bool QETWidget::translateMouseEvent( const QWSMouseEvent *event )
{
    static int old_x_root = -1;
    static int old_y_root = -1;
    static int old_state = 0;
    const QWSMouseEvent::SimpleData &mouse = event->simpleData;

    //since we compress mouse events with the same state, the
    //press/release will have been at the end of the move

    if ( mouse.x_root != old_x_root || mouse.y_root != old_y_root ) {
	old_x_root = mouse.x_root;
	old_y_root = mouse.y_root;
	QWSMouseEvent move = *event;
	move.simpleData.state = old_state;
	dispatchMouseEvent( &move );
    }

    if ( (mouse.state&AnyButton) != (old_state&AnyButton) ) {
	old_state = mouse.state;
	dispatchMouseEvent( event );
    }
    //Must ensure that mouse release event gets to the widget
    //that got the mouse press
    if ( qt_button_down && ( mouse.state & AnyButton ) == 0 )
	qt_button_down = 0;

    return TRUE;
}

bool QETWidget::dispatchMouseEvent( const QWSMouseEvent *event )
{
    static bool manualGrab = FALSE;
    QPoint pos;
    QPoint globalPos;
    int	   button = 0;
    int	   state;

    static int old_x_root = -1;
    static int old_y_root = -1;
    static int old_state = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;
    const QWSMouseEvent::SimpleData &mouse = event->simpleData;
    pos = mapFromGlobal(QPoint( mouse.x_root, mouse.y_root ));
    qt_last_x=mouse.x_root;
    qt_last_y=mouse.y_root;
    globalPos.rx() = mouse.x_root;
    globalPos.ry() = mouse.y_root;
    state = translateButtonState( mouse.state );

    //    while (1) { // Extract move and press/release from one QWSEvent
	QEvent::Type type = QEvent::None;

	if ( mouse.x_root != old_x_root || mouse.y_root != old_y_root ) {
	    old_x_root = mouse.x_root;
	    old_y_root = mouse.y_root;
	    // mouse move
	    // XXX compress motion events
	    type = QEvent::MouseMove;
	} else if ( 0 ) {
	    type = QEvent::MouseMove;
	    if ( !qt_button_down )
		state = state & ~AnyButton;
	} else if ( (mouse.state&AnyButton) != (old_state&AnyButton) ) {
	    for ( button = LeftButton; !type && button <= MidButton; button<<=1 ) {
		if ( (mouse.state&button) != (old_state&button) ) {
		    // button press or release
		    if ( isEnabled() ) {
			QWidget* w = this;
			while ( w->focusProxy() )
			    w = w->focusProxy();
			if ( w->focusPolicy() & QWidget::ClickFocus ) {
			    w->setFocus();
			}
		    }
		    if ( mouse.state&button ) { //button press
			qt_button_down = findChildWidget( this, pos );	//magic for masked widgets
			if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
			    qt_button_down = this;
			if ( /*XXX mouseActWindow == this &&*/
			     mouseButtonPressed == button &&
			     (long)mouse.time -(long)mouseButtonPressTime
				   < QApplication::doubleClickInterval() &&
			     QABS(mouse.x_root - mouseXPos) < 5 &&
			     QABS(mouse.y_root - mouseYPos) < 5 ) {
			    type = QEvent::MouseButtonDblClick;
			    mouseButtonPressTime -= 2000;	// no double-click next time
			} else {
			    type = QEvent::MouseButtonPress;
			    mouseButtonPressTime = mouse.time;
			}
			mouseButtonPressed = button; 	// save event params for
			mouseXPos = globalPos.x();		// future double click tests
			mouseYPos = globalPos.y();
		    } else {				// mouse button released
			if ( manualGrab ) {			// release manual grab
			    manualGrab = FALSE;
			    // XXX XUngrabPointer( x11Display(), CurrentTime );
			}

			type = QEvent::MouseButtonRelease;
		    }
		    old_state ^= button;
		}
	    }
	    button >>= 1;
	}
	//XXX mouseActWindow = winId();			// save some event params
	mouseButtonState = state;

	if ( type == 0 )				// event consumed
	    return FALSE; //EXIT in the normal case

	if ( qApp->inPopupMode() ) {			// in popup mode
	    QWidget *popup = qApp->activePopupWidget();
	    if ( popup != this ) {
		if ( testWFlags(WType_Popup) && rect().contains(pos) )
		    popup = this;
		else				// send to last popup
		    pos = popup->mapFromGlobal( globalPos );
	    }
	    bool releaseAfter = FALSE;
	    QWidget *popupChild  = findChildWidget( popup, pos );
	    QWidget *popupTarget = popupChild ? popupChild : popup;

	    if (popup != popupOfPopupButtonFocus){
		popupButtonFocus = 0;
		popupOfPopupButtonFocus = 0;
	    }

	    if ( !popupTarget->isEnabled() )
		return FALSE; //EXIT special case

	    switch ( type ) {
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonDblClick:
		    popupButtonFocus = popupChild;
		    popupOfPopupButtonFocus = popup;
		    break;
		case QEvent::MouseButtonRelease:
		    releaseAfter = TRUE;
		    break;
		default:
		    break;				// nothing for mouse move
	    }

	    if ( popupButtonFocus ) {
		QMouseEvent e( type, popupButtonFocus->mapFromGlobal(globalPos),
			       globalPos, button, state );
		QApplication::sendEvent( popupButtonFocus, &e );
		if ( releaseAfter ) {
		    popupButtonFocus = 0;
		    popupOfPopupButtonFocus = 0;
		}
	    } else if ( popupChild ) {
		QMouseEvent e( type, popupChild->mapFromGlobal(globalPos),
			       globalPos, button, state );
		QApplication::sendEvent( popupChild, &e );
	    } else {
		QMouseEvent e( type, pos, globalPos, button, state );
		QApplication::sendEvent( popupChild ? popupChild : popup, &e );
	    }

	    if ( releaseAfter )
		qt_button_down = 0;

	    // XXX WWA: don't understand
	    if ( qApp->inPopupMode() ) {			// still in popup mode
		//XXX if ( popupGrabOk )
		    //XXX XAllowEvents( x11Display(), SyncPointer, CurrentTime );
	    } else {
		/* XXX
		if ( type != QEvent::MouseButtonRelease && state != 0 &&
		     QWidget::find((WId)mouseActWindow) ) {
		    manualGrab = TRUE;		// need to manually grab
		    XGrabPointer( x11Display(), mouseActWindow, FALSE,
				  (uint)(ButtonPressMask | ButtonReleaseMask |
				  ButtonMotionMask |
				  EnterWindowMask | LeaveWindowMask),
				  GrabModeAsync, GrabModeAsync,
				  None, None, CurrentTime );
		}
		*/
	    }

	} else { //qApp not in popup mode
	    QWidget *widget = this;
	    QWidget *w = QWidget::mouseGrabber();
	    if ( !w && qt_button_down )
		w = qt_button_down;
	    if ( w && w != this ) {
		widget = w;
		pos = mapToGlobal( pos );
		pos = w->mapFromGlobal( pos );
	    }

	    if ( popupCloseDownMode ) {
		popupCloseDownMode = FALSE;
		if ( testWFlags(WType_Popup) )	// ignore replayed event
		    return TRUE; //EXIT
	    }

	    if ( type == QEvent::MouseButtonRelease &&
		 (state & (~button) & ( LeftButton |
					MidButton |
					RightButton)) == 0 ) {
		qt_button_down = 0;
	    }

	    QMouseEvent e( type, pos, globalPos, button, state );
#ifndef QT_NO_QWS_MANAGER
	    if (widget->isTopLevel() && widget->topData()->qwsManager
		&& (widget->topData()->qwsManager->region().contains(globalPos)
		    || (QWSManager::grabbedMouse() && QWidget::mouseGrabber())) ) {
		QApplication::sendEvent( widget->topData()->qwsManager, &e );
	    } else
#endif
		{
		QApplication::sendEvent( widget, &e );
	    }
	}
	// }
    return TRUE;
}


bool QETWidget::translateKeyEvent( const QWSKeyEvent *event, bool grab )
{
    int	   code = -1;
    int	   state = event->simpleData.modifiers;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    if ( !isEnabled() )
	return TRUE;

    QEvent::Type type = event->simpleData.is_press ?
	QEvent::KeyPress : QEvent::KeyRelease;
    bool    autor = event->simpleData.is_auto_repeat;
    QString text;
    char   ascii = 0;
    if ( event->simpleData.unicode ) {
	QChar ch(event->simpleData.unicode );
	if ( ch.unicode() != 0xffff )
	    text += ch;
	ascii = ch.latin1();
    }
    code = event->simpleData.keycode;

    bool isAccel = FALSE;
    if (!grab) { // test for accel if the keyboard is not grabbed
	QKeyEvent a( QEvent::AccelAvailable, code, ascii, state, text, FALSE,
		     int(text.length()) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	isAccel = a.isAccepted();
    }

    if ( !isAccel && !text.isEmpty() && testWState(WState_CompressKeys) ) {
	// the widget wants key compression so it gets it

	// XXX not implemented
    }


    // process accelerates before popups
    QKeyEvent e( type, code, ascii, state, text, autor,
		 int(text.length()) );
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel event to tlw if the keyboard is not grabbed
	QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor,
		     int(text.length()) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	if ( a.isAccepted() )
	    return TRUE;
    }
    return QApplication::sendEvent( this, &e );
}

void QETWidget::repaintHierarchy(QRegion r)
{
    r &= geometry();
    if (r.isEmpty())
	return;
    r.translate(-x(),-y());

    if ( !testWFlags( WRepaintNoErase ) )
	erase(r);

    QPaintEvent e( r );
    setWState( WState_InPaintEvent );
    qt_set_paintevent_clipping( this, r);
    QApplication::sendEvent( this, &e );
    qt_clear_paintevent_clipping();
    clearWState( WState_InPaintEvent );

    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( obj->isWidgetType() ) {
		QETWidget* w = (QETWidget*)obj;
		w->repaintHierarchy(r);
	    }
	}
    }
}

void QETWidget::repaintDecoration(QRegion r)
{
#ifndef QT_NO_QWS_MANAGER
    if ( testWFlags(WType_TopLevel) && topData()->qwsManager) {
	r &= topData()->qwsManager->region();
	r.translate(-x(),-y());
	QPaintEvent e(r, FALSE);
	setWState( WState_InPaintEvent );
	qt_set_paintevent_clipping( this, r );
	QApplication::sendEvent(topData()->qwsManager, &e );
	qt_clear_paintevent_clipping();
	clearWState( WState_InPaintEvent );
    }
#endif
}


void QETWidget::restrictRegion( QRegion r )
{
    QRegion totalr = alloc_region;
#ifndef QT_NO_QWS_MANAGER
    if ( testWFlags(WType_TopLevel) && topData()->qwsManager ) {
	totalr += topData()->decor_allocated_region;
	totalr &= r;
	topData()->decor_allocated_region = totalr & topData()->qwsManager->region();
    } else
#endif
	{
	totalr &= r;
    }

    alloc_region = totalr & req_region;
}

bool QETWidget::translateRegionModifiedEvent( const QWSRegionModifiedEvent *event )
{
    QWSRegionManager *rgnMan = qt_fbdpy->regionManager();

    if ( alloc_region_index < 0 ) {
	alloc_region_index = rgnMan->find( winId() );

	if ( alloc_region_index < 0 ) {
	    qFatal( "Cannot find region for window %d", winId() );
	}
    }

    QWSDisplay::grab();
    int revision = *rgnMan->revision( alloc_region_index );
    if ( revision != alloc_region_revision ) {
	alloc_region_revision = revision;
	QRegion newRegion = rgnMan->region( alloc_region_index );
	QWSDisplay::ungrab();
#ifndef QT_NO_QWS_MANAGER
	if ( testWFlags(WType_TopLevel) && topData()->qwsManager ) {
	    QRegion mr(topData()->qwsManager->region());
	    topData()->decor_allocated_region = newRegion & mr;
	    newRegion -= mr;
	}
#endif
	alloc_region = newRegion;

	// set children's allocated region dirty
	const QObjectList *c = children();
	if ( c ) {
	    QObjectListIt it(*c);
	    QObject* ch;
	    while ((ch=it.current())) {
		++it;
		if ( ch->isWidgetType() )
		    ((QWidget *)ch)->alloc_region_dirty = TRUE;
	    }
	}
    } else {
	QWSDisplay::ungrab();
    }

    if ( event->simpleData.nrectangles )
    {
	QRegion exposed;
	exposed.setRects( event->rectangles, event->simpleData.nrectangles );
/*
	for ( int i = 0; i < event->simpleData.nrectangles; i++ )
	    qDebug( "exposed: %d, %d %dx%d",
		event->rectangles[i].x(),
		event->rectangles[i].y(),
		event->rectangles[i].width(),
		event->rectangles[i].height() );
*/
	repaintDecoration( exposed );
	repaintHierarchy( exposed );
    }

    return TRUE;
}


void  QApplication::setCursorFlashTime( int msecs )
{
    cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

void QApplication::setDoubleClickInterval( int ms )
{
    mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}



/*****************************************************************************
  Session management support (-D QT_SM_SUPPORT to enable it)
 *****************************************************************************/
#ifndef QT_NO_SESSIONMANAGER

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData;
    d->sessionId = session;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

bool QSessionManager::allowsInteraction()
{
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return TRUE;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}

void QSessionManager::setRestartHint( QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand( const QStringList& command)
{
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand( const QStringList& command)
{
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    return d->discardCommand;
}

void QSessionManager::setProperty( const QString&, const QString&)
{
}

void QSessionManager::setProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return FALSE;
}

void QSessionManager::requestPhase2()
{
}
#endif //QT_NO_SESSIONMANAGER
// Need to add some sort of implementation here?

void QApplication::setWheelScrollLines(int)
{
}

int QApplication::wheelScrollLines()
{
    return 0;
}
