/****************************************************************************
** $Id: $
**
** Implementation of Qt/FB central server
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

#include "qwindowsystem_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsutils_qws.h"
#include "qwscursor_qws.h"
#include "qwsdisplay_qws.h"
#include "qwsmouse_qws.h"
#include "qcopchannel_qws.h"

#include "qapplication.h"
#include "qsocketnotifier.h"
#include "qpointarray.h" //cursor test code
#include "qimage.h"
#include "qcursor.h"
#include "qgfx_qws.h"
#include "qwindowdefs.h"
#include "qlock_qws.h"
#include "qwsregionmanager_qws.h"
#include "qqueue.h"
#include "qfile.h"
#include "qtimer.h"
#include "qpen.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef QT_NO_QWS_MULTIPROCESS
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/param.h>
#include <sys/mount.h>
#endif
#include <signal.h>
#include <fcntl.h>

#ifndef QT_NO_SOUND
#ifdef QT_USE_OLD_QWS_SOUND
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#else
#include "qsoundqss_qws.h"
#endif
#endif

#include <qgfx_qws.h>

QWSServer *qwsServer=0;

extern char *qws_display_spec;
extern void qt_init_display(); //qapplication_qws.cpp
extern QString qws_qtePipeFilename();

extern void qt_client_enqueue(const QWSEvent *); //qapplication_qws.cpp
typedef void MoveRegionF( const QWSRegionMoveCommand*);
typedef void RequestRegionF( int, QRegion );
typedef void SetAltitudeF( const QWSChangeAltitudeCommand* );
extern QQueue<QWSCommand> *qt_get_server_queue();

static QRect maxwindow_rect;
static const char *defaultMouse = "Auto";
static const char *defaultKeyboard = "TTY";
static int qws_keyModifiers = 0;

static int get_object_id()
{
    static int next=1000;
    return next++;
}


//#define QWS_REGION_DEBUG

/*!
  \class QWSWindow qwindowsystem_qws.h
  \ingroup qws
  \brief The QWSWindow class provides server-specific functionality in Qt/Embedded.

  When you run a Qt/Embedded application, it either runs as a server
  or connects to an existing server. If it runs as a server, some additional
  functionality is provided by the QWSServer class.

  This class maintains information about each window and allows operations
  to be performed on the windows.

  You can get the window's name(), caption() and winId(), along with
  the client() that owns the window.

  The region the window wants to draw on is returned by requested();
  the region that the window is allowed to draw on is returned by
  allocation().

  The visibility of the window can be determined using isVisible(),
  isPartiallyObscured() and isFullyObscured(). Visibility can be
  changed using raise(), lower(), show(), hide() and
  setActiveWindow().


*/

/*!
  \fn int QWSWindow::winId() const

  Returns the Id of this window.
*/

/*!
  \fn const QString &QWSWindow::name() const

  Returns the name of this window.
*/

/*!
  \fn const QString &QWSWindow::caption() const

  Returns this window's caption.
*/

/*!
  \fn QWSClient* QWSWindow::client() const

  Returns the QWSClient that owns this window.
*/

/*!
  \fn QRegion QWSWindow::requested() const

  Returns the region that the window has requested to draw onto including
  any window decorations.

  \sa allocation()
*/

/*!
  \fn QRegion QWSWindow::allocation() const

  Returns the region that the window is allowed to draw onto including
  any window decorations but excluding regions covered by other windows.

  \sa requested()
*/

/*!
  \fn bool QWSWindow::isVisible() const

  Returns TRUE if the window is visible; otherwise returns FALSE.
*/

/*!
  \fn bool QWSWindow::isPartiallyObscured() const

  Returns TRUE is the window is partially obsured by another window or
  by the bounds of the screen; otherwise returns FALSE.
*/

/*!
  \fn bool QWSWindow::isFullyObscured() const

  Returns TRUE is the window is completely obsured by another window or
  by the bounds of the screen; otherwise returns FALSE.
*/

/*!
  Raises the window above all other windows except "Stay on top" windows.
*/
void QWSWindow::raise()
{
    qwsServer->raiseWindow( this );
}

/*!
  Lowers the window below other windows.
*/
void QWSWindow::lower()
{
    qwsServer->lowerWindow( this );
}

/*!
  Shows the window.
*/
void QWSWindow::show()
{
    operation( QWSWindowOperationEvent::Show );
}

/*!
  Hides the window.
*/
void QWSWindow::hide()
{
    operation( QWSWindowOperationEvent::Hide );
}

/*!
  Make this the active window (i.e. sets the keyboard focus to this
  window).
*/
void QWSWindow::setActiveWindow()
{
    qwsServer->setFocus( this, TRUE );
}

void QWSWindow::setName( const QString &n )
{
    rgnName = n;
}

void QWSWindow::setCaption( const QString &c )
{
    rgnCaption = c;
}

/*!
  \internal
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
  \internal
  Removes \a r from the window's allocated region
*/
void QWSWindow::removeAllocation(QWSRegionManager *rm, QRegion r)
{
    QRegion nr = allocated_region - r;
    if ( nr != allocated_region ) {
	allocated_region = nr;
	rm->set( alloc_region_idx, allocated_region );
	modified = TRUE;
    } else if ( needAck ) {
	// set our region dirty anyway
	rm->markUpdated( alloc_region_idx );
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

void QWSWindow::operation( QWSWindowOperationEvent::Operation o )
{
    QWSWindowOperationEvent event;
    event.simpleData.window = id;
    event.simpleData.op = o;
    c->sendEvent( &event );
}

/*!
    Destructor.
*/
QWSWindow::~QWSWindow()
{
}


/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/
//always use frame buffer
QWSClient::QWSClient( QObject* parent, int socket )
    : QObject( parent), s(socket), command(0)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if ( socket == -1 ) {
	csocket = 0;
	isClosed = FALSE;
    } else {
	csocket = new QWSSocket( this );
	csocket->setSocket(socket);
	isClosed = FALSE;

	csocket->flush();

	connect( csocket, SIGNAL(readyRead()), this, SIGNAL(readyRead()) );
	connect( csocket, SIGNAL(connectionClosed()), this, SLOT(closeHandler()) );
	connect( csocket, SIGNAL(error(int)), this, SLOT(errorHandler(int)) );
    }
#else
    isClosed = FALSE;
#endif //QT_NO_QWS_MULTIPROCESS
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
#ifndef QT_NO_QWS_MULTIPROCESS
    switch( err ) {
    case QWSSocket::ErrConnectionRefused:
	s = "Connection Refused";
	break;
    case QWSSocket::ErrHostNotFound:
	s = "Host Not Found";
	break;
    case QWSSocket::ErrSocketRead:
	s = "Socket Read";
	break;
    }
#endif
    //qDebug( "Client %p error %d (%s)", this, err, s.ascii() );
    isClosed = TRUE;
#ifndef QT_NO_QWS_MULTIPROCESS
    if ( csocket )
	csocket->flush(); //####We need to clean out the pipes, this in not the the way.
#endif
    emit connectionClosed();
}

int QWSClient::socket() const
{
    return s;
}

void QWSClient::sendEvent( QWSEvent* event )
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if ( csocket ) {
	event->write( csocket );
	csocket->flush();
    }
    else
#endif
    {
	qt_client_enqueue( event );
    }
}

void QWSClient::sendConnectedEvent( const char *display_spec )
{
    QWSConnectedEvent event;
    event.simpleData.window = 0;
    event.simpleData.len = strlen( display_spec ) + 1;
    char * tmp=(char *)display_spec;
    event.setData( tmp, event.simpleData.len );
    sendEvent( &event );
}

void QWSClient::sendMaxWindowRectEvent()
{
    QWSMaxWindowRectEvent event;
    event.simpleData.window = 0;
    event.simpleData.rect = maxwindow_rect;
    sendEvent(&event);
}

void QWSClient::sendRegionModifyEvent( int winid, QRegion exposed, bool ack )
{
    QWSRegionModifiedEvent event;
    event.simpleData.window = winid;
    event.simpleData.nrectangles = exposed.rects().count();
    event.simpleData.is_ack = ack;
    event.setData( (char *)exposed.rects().data(),
		    exposed.rects().count() * sizeof(QRect), FALSE );

//    qDebug( "Sending %d %d rects ack: %d", winid, event.simpleData.nrectangles, ack );
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

#ifndef QT_NO_SOUND
#ifdef QT_USE_OLD_QWS_SOUND

	/*
	***
	****
*************
**********
**********  WARNING:  This code is obsoleted by tests/qsound,
**********            which will soobn be used instead.
*************
	****
	***
	*/

struct QRiffChunk {
    char id[4];
    Q_UINT32 size;
    char data[4/*size*/];
};

//#define QT_QWS_SOUND_8BIT
static const int sound_fragment_size = 8;
static const int sound_stereo = 0;
static const int sound_speed = 11025;

static const int sound_buffer_size=1<<sound_fragment_size;
class QWSSoundServerBucket {
public:
    QWSSoundServerBucket(QIODevice* d)
    {
	dev = d;
	out = 0;
	chunk_remaining = 0;
	available = readSamples(data,sound_buffer_size); // preload some
    }
    ~QWSSoundServerBucket()
    {
	delete dev;
    }
    int max() const
    {
	return available;
    }
    void add(int* mix, int count)
    {
	Q_ASSERT(available>=count);
	available -= count;
	while ( count-- ) {
	    *mix++ += ((int)data[out] - 128)*128;
	    if ( ++out == sound_buffer_size )
		out = 0;
	}
    }
    void rewind(int count)
    {
	Q_ASSERT(count<sound_buffer_size);
	out = (out + (sound_buffer_size-count))%sound_buffer_size;
	available += count;
    }
    bool refill()
    {
	int toread = sound_buffer_size - available;
	int in = (out + available)%sound_buffer_size;
	int a = sound_buffer_size-in; if ( a > toread ) a = toread;
	int rd = a ? readSamples(data+in,a) : 0;
	if ( rd < 0 )
	    return FALSE; // ############
	int b = toread - rd;
	if ( b ) {
	    int r = readSamples(data,b);
	    if ( r > 0 )
		rd += r;
	}
	available += rd;
	return rd > 0;
    }
private:
    int readSamples(uchar* dst, int count)
    {
	if ( chunk_remaining < 0 )
	    return 0; // in error state
	for ( ;; ) {
	    if ( chunk_remaining > 0 ) {
		if ( count > chunk_remaining )
		    count = chunk_remaining;
		chunk_remaining -= count;
		return dev->readBlock((char*)dst, count);
	    } else {
		chunk_remaining = -1;
		// Keep reading chunks...
		const int n = sizeof(chunk)-sizeof(chunk.data);
		if ( dev->readBlock((char*)&chunk,n) != n ) {
		    return 0;
		}
		if ( qstrncmp(chunk.id,"data",4) == 0 ) {
		    chunk_remaining = chunk.size;
		} else if ( qstrncmp(chunk.id,"RIFF",4) == 0 ) {
		    char d[4];
		    dev->readBlock(d,4);
		    if ( qstrncmp(d,"WAVE",4) != 0 ) {
			return 0;
		    }
		} else if ( qstrncmp(chunk.id,"fmt ",4) == 0 ) {
		    struct {
			#define WAVE_FORMAT_PCM 1
			Q_INT16 formatTag;
			Q_INT16 channels;
			Q_INT32 samplesPerSec;
			Q_INT32 avgBytesPerSec;
			Q_INT16 blockAlign;
		    } chunkdata;
		    if ( dev->readBlock((char*)&chunkdata,sizeof(chunkdata)) != sizeof(chunkdata) ) {
			qDebug("WAV file: UNSUPPORTED SIZE");
			return 0;
		    }
		    if ( chunkdata.formatTag != WAVE_FORMAT_PCM ) {
			qDebug("WAV file: UNSUPPORTED FORMAT");
			return 0;
		    }
		    if ( chunkdata.channels != sound_stereo+1 ) {
			qDebug("WAV file: UNSUPPORTED CHANNELS");
			return 0;
		    }
		    /* Ignore
		    if ( chunkdata.samplesPerSec != sound_speed ) {
			return 0;
		    }
		    */
		} else {
		    // ignored chunk
		    if ( !dev->at(dev->at()+chunk.size) ) {
			return 0;
		    }
		}
	    }
	}
    }
    QRiffChunk chunk;
    int chunk_remaining;

    QIODevice* dev;
    uchar data[sound_buffer_size];
    int available,out;
};

class QWSSoundServerData {
public:
    QWSSoundServerData(QWSSoundServer* s)
    {
	active.setAutoDelete(TRUE);
	sn = 0;
	server = s;
    }

    void feedDevice(int fd)
    {
	QWSSoundServerBucket* bucket;
	int available = sound_buffer_size;
	int n = 0;
	QPtrListIterator<QWSSoundServerBucket> it(active);
	for (; (bucket = *it);) {
	    ++it;
	    int m = bucket->max();
	    if ( m ) {
		if ( m < available )
		    available = m;
		n++;
	    } else {
		active.removeRef(bucket);
	    }
	}
	if ( n ) {
	    int data[sound_buffer_size];
	    for (int i=0; i<available; i++)
		data[i]=0;
	    for (bucket = active.first(); bucket; bucket = active.next()) {
		bucket->add(data,available);
	    }
#ifdef QT_QWS_SOUND_8BIT
	    signed char d8[sound_buffer_size];
	    for (int i=0; i<available; i++) {
		int t = data[i] / 1; // ######### configurable
		if ( t > 127 ) t = 127;
		if ( t < -128 ) t = -128;
		d8[i] = (signed char)t;
	    }
	    int w = ::write(fd,d8,available);
#else
	    short d16[sound_buffer_size];
	    for (int i=0; i<available; i++) {
		int t = data[i]; // ######### configurable
		if ( t > 32767 ) t = 32767;
		if ( t < -32768 ) t = -32768;
		d16[i] = (short)t;
		//d16[i] = ((t&0xff)<<8)|((t>>8)&0xff);
	    }
	    int w = ::write(fd,(char*)d16,available*2)/2;
#endif
	    if ( w < 0 )
		return; // ##############
	    int rw = available - w;
	    if ( rw ) {
		for (bucket = active.first(); bucket; bucket = active.next()) {
		    bucket->rewind(rw);
		    bucket->refill();
		}
	    }
	    for (bucket = active.first(); bucket; bucket = active.next()) {
		if ( !bucket->refill() ) {
		    //active.remove(bucket);
		}
	    }
	} else {
	    closeDevice();
	}
    }

    void playFile(const QString& filename)
    {
	QFile* f = new QFile(filename);
	f->open(IO_ReadOnly);
	active.append(new QWSSoundServerBucket(f));
	openDevice();
    }

private:
    void openDevice()
    {
	if ( !sn ) {
	    int fd = ::open("/dev/dsp",O_RDWR);
	    if ( fd < 0 ) {
		// For debugging purposes - defined QT_NO_SOUND if you
		// don't have sound hardware!
		fd = ::open("/tmp/dsp",O_WRONLY);
	    }

	    // Setup soundcard at 16 bit mono
	    int v;
	    v=0x00040000+sound_fragment_size; ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &v);
#ifdef QT_QWS_SOUND_8BIT
	    v=AFMT_U8; ioctl(fd, SNDCTL_DSP_SETFMT, &v);
#else
	    v=AFMT_S16_LE; ioctl(fd, SNDCTL_DSP_SETFMT, &v);
#endif
	    v=sound_stereo; ioctl(fd, SNDCTL_DSP_STEREO, &v);
	    v=sound_speed; ioctl(fd, SNDCTL_DSP_SPEED, &v);

	    sn = new QSocketNotifier(fd,QSocketNotifier::Write,server);
	    QObject::connect(sn,SIGNAL(activated(int)),server,SLOT(feedDevice(int)));
	}
    }

    void closeDevice()
    {
	if ( sn ) {
	    ::close(sn->socket());
	    delete sn;
	    sn = 0;
	}
    }

    QPtrList<QWSSoundServerBucket> active;
    QSocketNotifier* sn;
    QWSSoundServer* server;
};

QWSSoundServer::QWSSoundServer(QObject* parent) :
    QObject(parent)
{
    d = new QWSSoundServerData(this);
}

QWSSoundServer::~QWSSoundServer()
{
    delete d;
}

void QWSSoundServer::playFile(const QString& filename)
{
    d->playFile(filename);
}

void QWSSoundServer::feedDevice(int fd)
{
    d->feedDevice(fd);
}

#endif
#endif

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

/*!
  \class QWSServer qwindowsystem_qws.h
  \ingroup qws
  \brief The QWSServer class provides server-specific functionality in Qt/Embedded.

  When you run a Qt/Embedded application, it either runs as a server
  or connects to an existing server. If it runs as a server, some additional
  operations are provided via the QWSServer class.

  This class is instantiated by QApplication for Qt/Embedded server processes.
  You should never construct this class yourself.

  A pointer to the QWSServer instance can be obtained via the global
  qwsServer variable.

  The mouse and keyboard devices can be opened with openMouse() and
  openKeyboard(). (Close them with closeMouse() and closeKeyboard().)

  The display is refreshed with refresh(), and painting can be enabled
  or disabled with enablePainting().

  Obtain the list of client windows with clientWindows() and find out
  which window is at a particular point with windowAt().

  Many static functions are provided, for example,
  setKeyboardFilter(), setKeyboardHandler(), setDefaultKeyboard() and
  setDefaultMouse().

  The size of the window rectangle can be set with setMaxWindowRect(),
  and the desktop's background can be set with setDesktopBackground().

  The screen saver is controlled with setScreenSaverInterval() and
  screenSaverActivate().
*/

/*!
  \fn const QList<QWSWindow> &QWSServer::clientWindows()

  Returns the list of top-level windows.  This list will change as
  applications add and remove wigdets so it should not be stored for future
  use.  The windows are sorted in stacking order from top-most to
  bottom-most.
*/

/*!
  Construct a QWSServer class with parent \a parent, called \a name
  and flags \a flags.

  \warning This class is instantiated by QApplication for Qt/Embedded
  server processes. You should never construct this class yourself.
*/

QWSServer::QWSServer( int flags, QObject *parent, const char *name ) :
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSServerSocket(qws_qtePipeFilename(),16,parent,name),
#else
    QObject( parent, name ),
#endif
    disablePainting(false)
{
    Q_ASSERT( !qwsServer );
    qwsServer = this;

#ifndef QT_NO_QWS_MULTIPROCESS
    QString pipe = qws_qtePipeFilename();

    if ( !ok() ) {
	perror("Error");
	qFatal("Failed to bind to %s", pipe.latin1() );
    } else {
	struct linger tmp;
	tmp.l_onoff=1;
	tmp.l_linger=0;
	setsockopt(socket(),SOL_SOCKET,SO_LINGER,(char *)&tmp,sizeof(tmp));
    }

    signal(SIGPIPE, ignoreSignal); //we get it when we read
#endif
    focusw = 0;
    mouseGrabber = 0;
    mouseGrabbing = FALSE;
#ifndef QT_NO_QWS_CURSOR
    cursorNeedsUpdate=FALSE;
    nextCursor = 0;
#endif

#ifndef QT_NO_QWS_MULTIPROCESS

    if ( !geteuid() ) {
#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS)
	if( mount(0,"/var/shm", "shm", 0, 0) ) {
	    /* This just confuses people with 2.2 kernels
	    if ( errno != EBUSY )
		qDebug("Failed mounting shm fs on /var/shm: %s",strerror(errno));
	    */
	}
#endif
    }
#endif

    // no selection yet
    selectionOwner.windowid = -1;
    selectionOwner.time.set( -1, -1, -1, -1 );

    openDisplay();

    screensavertimer = new QTimer(this);
    connect( screensavertimer, SIGNAL(timeout()), this, SLOT(screenSaverSleep()) );
    screensaverinterval = 0;
    screenSaverWake();

    // input devices
    if ( !(flags&DisableMouse) ) {
	openMouse();
    }
    initializeCursor();

#ifndef QT_NO_QWS_KEYBOARD
    if ( !(flags&DisableKeyboard) ) {
	openKeyboard();
    }
#endif
    if ( !bgColor )
	bgColor = new QColor( 0x20, 0xb0, 0x50 );
    screenRegion = QRegion( 0, 0, swidth, sheight );
    paintBackground( screenRegion );

    client[-1] = new QWSClient( this, -1 );

#ifndef QT_NO_SOUND
    soundserver = new QWSSoundServer(this);
#endif
}

/*!
  Destruct QWSServer
*/
QWSServer::~QWSServer()
{
    // destroy all clients
    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	delete *it;
    delete bgColor;
    bgColor = 0;
    closeDisplay();
    closeMouse();
#ifndef QT_NO_QWS_KEYBOARD
    closeKeyboard();
#endif
}

#ifndef QT_NO_QWS_MULTIPROCESS
/*!
  \internal
*/
void QWSServer::newConnection( int socket )
{
    client[socket] = new QWSClient(this,socket);
    connect( client[socket], SIGNAL(readyRead()),
	     this, SLOT(doClient()) );
    connect( client[socket], SIGNAL(connectionClosed()),
	     this, SLOT(clientClosed()) );

    client[socket]->sendConnectedEvent( qws_display_spec );

    if ( !maxwindow_rect.isEmpty() )
	client[socket]->sendMaxWindowRectEvent();

    // pre-provide some object id's
    for (int i=0; i<20; i++)
	invokeCreate(0,client[socket]);
}

/*!
  \internal
*/
void QWSServer::clientClosed()
{
    QWSClient* cl = (QWSClient*)sender();

    // Remove any queued commands for this client
    QPtrListIterator<QWSCommandStruct> it(commandQueue);
    for (QWSCommandStruct* cs = it.current(); (cs=*it); ++it ) {
	if ( cs->client == cl ) {
	    commandQueue.removeRef(cs);
	    delete cs;
	}
    }

#ifndef QT_NO_COP
    // Enfore unsubscription from all channels.
    QCopChannel::detach( cl );
#endif

    QRegion exposed;
    {
	// Shut down all windows for this client
	QPtrListIterator<QWSWindow> it( windows );
	QWSWindow* w;
	while (( w = it.current() )) {
	    ++it;
	    if ( w->forClient(cl) )
		w->shuttingDown();
	}
    }
    {
	// Delete all windows for this client
	QPtrListIterator<QWSWindow> it( windows );
	QWSWindow* w;
	while (( w = it.current() )) {
	    ++it;
	    if ( w->forClient(cl) ) {
		if ( mouseGrabber == w ) {
		    mouseGrabber = 0;
		    mouseGrabbing = FALSE;
#ifndef QT_NO_QWS_CURSOR
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
#ifndef QT_NO_QWS_PROPERTIES
		manager()->removeProperties( w->winId() );
#endif
		emit windowEvent( w, Destroy );
		delete w; //windows is not auto-delete
	    }
	}
    }
    client.remove( cl->socket() );
    delete cl;
    exposeRegion( exposed );
    syncRegions();
}
#endif //QT_NO_QWS_MULTIPROCESS


QWSCommand* QWSClient::readMoreCommand()
{
#ifndef QT_NO_QWS_MULTIPROCESS
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
    }
    else
#endif
    {
	return qt_get_server_queue()->dequeue();
    }

}


/*!
  \internal
*/
void QWSServer::processEventQueue()
{
    if ( qwsServer )
	qwsServer->doClient( qwsServer->client[-1] );
}


#ifndef QT_NO_QWS_MULTIPROCESS
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
#endif

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
	case QWSCommand::RegionName:
	    invokeRegionName( (QWSRegionNameCommand*)cs->command, cs->client );
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
#ifndef QT_NO_QWS_CURSOR
	case QWSCommand::DefineCursor:
	    invokeDefineCursor( (QWSDefineCursorCommand*)cs->command, cs->client );
	    break;
	case QWSCommand::SelectCursor:
	    invokeSelectCursor( (QWSSelectCursorCommand*)cs->command, cs->client );
	    break;
#endif
	case QWSCommand::GrabMouse:
	    invokeGrabMouse( (QWSGrabMouseCommand*)cs->command, cs->client );
	    break;
#ifndef QT_NO_SOUND
	case QWSCommand::PlaySound:
	    invokePlaySound( (QWSPlaySoundCommand*)cs->command, cs->client );
	    break;
#endif
#ifndef QT_NO_COP
	case QWSCommand::QCopRegisterChannel:
	    invokeRegisterChannel( (QWSQCopRegisterChannelCommand*)cs->command,
				   cs->client );
	    break;
	case QWSCommand::QCopSend:
	    invokeQCopSend( (QWSQCopSendCommand*)cs->command, cs->client );
	    break;
#endif
#ifndef QT_NO_QWS_REPEATER
	case QWSCommand::RepaintRegion:
	    invokeRepaintRegion((QWSRepaintRegionCommand*)cs->command,
				cs->client);
	    break;
#endif
	}
	delete cs->command;
	delete cs;
#ifndef QT_NO_QWS_CURSOR
	if (cursorNeedsUpdate)
	    showCursor();
#endif
    }
}


void QWSServer::showCursor()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->show();
#endif
}

void QWSServer::hideCursor()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
}

/*!
    If \a e is TRUE, painting on the display is enabled; if \a e is
    FALSE, painting is disabled.
*/

void QWSServer::enablePainting(bool e)
{
// ### don't like this
    if (e)
    {
	disablePainting = false;
	setWindowRegion( 0, QRegion() );
	showCursor();
	syncRegions();
    }
    else
    {
	disablePainting = true;
	hideCursor();
	setWindowRegion( 0, QRegion(0,0,swidth,sheight) );
	syncRegions();
    }
}

/*!
  Refreshes the entire display.
*/
void QWSServer::refresh()
{
    exposeRegion( QRegion(0,0,swidth,sheight) );
    syncRegions();
}

#ifndef QT_NO_QWS_REPEATER
/*!
    \overload
  Refreshes the region \a r.
*/
void QWSServer::refresh(QRegion & r)
{
    exposeRegion(r);
    syncRegions();
}
#endif

/*!
  Sets the area of the screen which Qt/Embedded applications
  will consider to be the maximum area to use for windows to \a r.

  \sa QWidget::showMaximized()
*/

void QWSServer::setMaxWindowRect(const QRect& r)
{
    QRect tr = qt_screen->mapToDevice(r,
	QSize(qt_screen->width(),qt_screen->height()));
    if ( maxwindow_rect != tr ) {
	maxwindow_rect = tr;
	qwsServer->sendMaxWindowRectEvents();
    }
}

/*!
  \internal
*/
void QWSServer::sendMaxWindowRectEvents()
{
    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendMaxWindowRectEvent();
}

/*!
  Set the mouse driver \a m to use if $QWS_MOUSE_PROTO is not defined.
  The default is platform-dependent.
*/
void QWSServer::setDefaultMouse( const char *m )
{
    defaultMouse = m;
}

/*!
  Set the keyboard driver to \a k, e.g. if $QWS_KEYBOARD is not defined.
  The default is platform-dependant.
*/

void QWSServer::setDefaultKeyboard( const char *k )
{
    defaultKeyboard = k;
}

/*!
  Send a mouse event. \a pos is the screen position where the mouse
  event occurred and \a state is a mask indicating which buttons are
  pressed.
*/

void QWSServer::sendMouseEvent(const QPoint& pos, int state)
{
    qwsServer->showCursor();

    qwsServer->screenSaverWake();

    mousePosition = pos;

    QWSMouseEvent event;

    //If grabbing window disappears, grab is still active until
    //after mouse release.
    QWSWindow *win = qwsServer->mouseGrabber ? qwsServer->mouseGrabber : qwsServer->windowAt( pos );
    event.simpleData.window = win ? win->id : 0;

#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->move(pos.x(),pos.y());

    // Arrow cursor over desktop
    if (!win) {
	if ( !qwsServer->mouseGrabber )
	    qwsServer->setCursor(QWSCursor::systemCursor(ArrowCursor));
	else
	    qwsServer->nextCursor = QWSCursor::systemCursor(ArrowCursor);
    }
#endif

    if ( state && !qwsServer->mouseGrabbing ) {
	qwsServer->mouseGrabber = win;
    }

    event.simpleData.x_root=pos.x();
    event.simpleData.y_root=pos.y();
    event.simpleData.state=state | qws_keyModifiers;
    event.simpleData.time=qwsServer->timer.elapsed();

    for (ClientIterator it = qwsServer->client.begin(); it != qwsServer->client.end(); ++it )
	(*it)->sendEvent( &event );

    if ( !state && !qwsServer->mouseGrabbing ) {
#ifndef QT_NO_QWS_CURSOR
	if (qwsServer->mouseGrabber && qwsServer->nextCursor) {
	    qwsServer->setCursor(qwsServer->nextCursor);
	    qwsServer->nextCursor = 0;
	}
#endif
	qwsServer->mouseGrabber = 0;
    }
}

/*!
  Returns the primary mouse handler.
*/
QWSMouseHandler *QWSServer::mouseHandler()
{
    return qwsServer->mousehandlers.first();
}

// called by QWSMouseHandler constructor, not user code.
/*!
  \internal
*/
void QWSServer::setMouseHandler(QWSMouseHandler* mh)
{
    qwsServer->mousehandlers.prepend(mh);
}

/*!
  \internal
*/
QPtrList<QWSInternalWindowInfo> * QWSServer::windowList()
{
    QPtrList<QWSInternalWindowInfo> * ret=new QPtrList<QWSInternalWindowInfo>;
    ret->setAutoDelete(true);
    QWSWindow * window;
    for(window=qwsServer->windows.first();window!=0;
	window=qwsServer->windows.next()) {
	QWSInternalWindowInfo * qwi=new QWSInternalWindowInfo();
	qwi->winid=window->winId();
	qwi->clientid=(unsigned int)window->client();
#ifndef QT_NO_QWS_PROPERTIES
	char * name;
	int len;
	qwsServer->propertyManager.getProperty(qwi->winid,
					       QT_QWS_PROPERTY_WINDOWNAME,
					       name,len);
	if(name) {
	    char * buf=(char *)malloc(len+2);
	    strncpy(buf,name,len);
	    buf[len]=0;
	    qwi->name=buf;
	    free(buf);
	} else {
	    qwi->name="unknown";
	}
#else
	qwi->name="unknown";
#endif
	ret->append(qwi);
    }

    return ret;
}

#ifndef QT_NO_COP
/*!
  \internal
*/
void QWSServer::sendQCopEvent( QWSClient *c, const QCString &ch,
			       const QCString &msg, const QByteArray &data,
			       bool response )
{
    Q_ASSERT( c );

    QWSQCopMessageEvent event;
    event.simpleData.is_response = response;
    event.simpleData.lchannel = ch.length();
    event.simpleData.lmessage = msg.length();
    event.simpleData.ldata = data.size();
    int l = event.simpleData.lchannel + event.simpleData.lmessage +
	    event.simpleData.ldata;

    // combine channel, message and data into one block of raw bytes
    QByteArray raw( l );
    char *d = (char*)raw.data();
    memcpy( d, ch.data(), event.simpleData.lchannel );
    d += event.simpleData.lchannel;
    memcpy( d, msg.data(), event.simpleData.lmessage );
    d += event.simpleData.lmessage;
    memcpy( d, data.data(), event.simpleData.ldata );

    event.setData( raw.data(), l );

    c->sendEvent( &event );
}
#endif

/*!
  Returns the window containing the point \a pos or 0 if there is no window
  under the point.
*/

QWSWindow *QWSServer::windowAt( const QPoint& pos )
{
    for (uint i=0; i<windows.count(); i++) {
	QWSWindow* w = windows.at(i);
	if ( w->requested_region.contains( pos ) )
	    return w;
    }
    return 0;
}

static int keyUnicode(int keycode)
{
    const QWSServer::KeyMap *km = QWSServer::keyMap();
    while (km->key_code) {
	if ( km->key_code == keycode ) {
	    return km->unicode;
	}
	++km;
    }
    return 0xffff;
}

/*!
  Send a key event. You can use this to send key events generated by
  "virtual keyboards". \a unicode is the unicode value of the key to
  send, \a keycode the Qt keycode (e.g. Key_Left), \a modifiers
  indicates whether, Shift/Alt/Ctrl keys are pressed, \a isPress
  is TRUE if this is a key down event and FALSE if it's a key up event,
  and \a autoRepeat is TRUE if this is an autorepeat event (i.e. the
  user has held the key down and this is the second or subsequent
  key event being sent).
*/

void QWSServer::sendKeyEvent(int unicode, int keycode, int modifiers, bool isPress,
  bool autoRepeat)
{
    if ( isPress ) {
#ifdef QT_QWS_IPAQ
	// Temporary power control (should be done externally).
	if ( keycode == Key_SysReq )
	    QWSServer::screenSaverActivate( !QWSServer::screenSaverActive() );
	else
#endif
	qwsServer->screenSaverWake();
    }

    qws_keyModifiers = modifiers;

    QWSKeyEvent event;
    event.simpleData.window = qwsServer->focusw ? qwsServer->focusw->winId() : 0;
    event.simpleData.unicode = unicode < 0 ? keyUnicode(keycode) : unicode;
    event.simpleData.keycode = keycode;
    event.simpleData.modifiers = modifiers;
    event.simpleData.is_press = isPress;
    event.simpleData.is_auto_repeat = autoRepeat;

    for (ClientIterator it = qwsServer->client.begin(); it != qwsServer->client.end(); ++it ) {
	(*it)->sendEvent(&event);
    }
}
#ifndef QT_NO_QWS_PROPERTIES
/*!
  \internal
*/
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

void QWSServer::invokeRegionName( QWSRegionNameCommand *cmd, QWSClient *client )
{
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, client);
    changingw->setName( cmd->name );
    changingw->setCaption( cmd->caption );
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
    bool isShow = !changingw->isVisible() && !region.isEmpty();
    setWindowRegion( changingw, region );
    syncRegions( changingw );
    if ( isShow )
	emit windowEvent( changingw, Show );
    if ( !region.isEmpty() )
	emit windowEvent( changingw, Geometry );
    else
	emit windowEvent( changingw, Hide );
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
    emit windowEvent( changingw, Geometry );
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
    if ( focusw == changingw ) {
	changingw->shuttingDown();
	setFocus(changingw,FALSE);
    }
#ifndef QT_NO_QWS_PROPERTIES
    manager()->removeProperties( changingw->winId() );
#endif
    emit windowEvent( changingw, Destroy );
    delete changingw;
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

    if ( alt < -1 || alt > 1 ) {
	qWarning( "Only lower, raise, and stays-on-top supported" );
	return;
    }

    QWSWindow* changingw = findWindow(winId, client);
    if ( !changingw ) {
	qWarning("Invalid window handle %08x", winId);
	return;
    }

    changingw->setNeedAck( TRUE );

    if ( fixed && alt == 1)
	changingw->onTop = TRUE;
    if ( alt < 0 )
	lowerWindow( changingw, alt );
    else
	raiseWindow( changingw, alt );

    if ( !changingw->forClient(client) ) {
	refresh();
    }

}
#ifndef QT_NO_QWS_PROPERTIES
void QWSServer::invokeAddProperty( QWSAddPropertyCommand *cmd )
{
    manager()->addProperty( cmd->simpleData.windowid, cmd->simpleData.property );
}

void QWSServer::invokeSetProperty( QWSSetPropertyCommand *cmd )
{
    if ( manager()->setProperty( cmd->simpleData.windowid,
				    cmd->simpleData.property,
				    cmd->simpleData.mode,
				    cmd->data,
				    cmd->rawLen ) ) {
	sendPropertyNotifyEvent( cmd->simpleData.property,
				 QWSPropertyNotifyEvent::PropertyNewValue );
   }
}

void QWSServer::invokeRemoveProperty( QWSRemovePropertyCommand *cmd )
{
    if ( manager()->removeProperty( cmd->simpleData.windowid,
				       cmd->simpleData.property ) ) {
	sendPropertyNotifyEvent( cmd->simpleData.property,
				 QWSPropertyNotifyEvent::PropertyDeleted );
    }
}

void QWSServer::invokeGetProperty( QWSGetPropertyCommand *cmd, QWSClient *client )
{
    char *data;
    int len;

    if ( manager()->getProperty( cmd->simpleData.windowid,
				    cmd->simpleData.property,
				    data, len ) ) {
	client->sendPropertyReplyEvent( cmd->simpleData.property, len, data );
    } else {
	client->sendPropertyReplyEvent( cmd->simpleData.property, -1, 0 );
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

#ifndef QT_NO_QWS_CURSOR
void QWSServer::invokeDefineCursor( QWSDefineCursorCommand *cmd, QWSClient *client )
{
    if (cmd->simpleData.height > 64 || cmd->simpleData.width > 64) {
	qDebug("Cannot define cursor size > 64x64");
	return;
    }

    int dataLen = cmd->simpleData.height * ((cmd->simpleData.width+7) / 8);

    QWSCursor *curs = new QWSCursor( cmd->data, cmd->data + dataLen,
				cmd->simpleData.width, cmd->simpleData.height,
				cmd->simpleData.hotX, cmd->simpleData.hotY);

    client->cursors.insert(cmd->simpleData.id, curs);
}

void QWSServer::invokeSelectCursor( QWSSelectCursorCommand *cmd, QWSClient *client )
{
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
}
#endif

void QWSServer::invokeGrabMouse( QWSGrabMouseCommand *cmd, QWSClient *client )
{
    QWSWindow* win = findWindow(cmd->simpleData.windowid, client);

    if ( cmd->simpleData.grab ) {
	if ( !mouseGrabber || ( mouseGrabber->client() == client ) ) {
	    mouseGrabbing = TRUE;
	    mouseGrabber = win;
	}
    } else {
#ifndef QT_NO_QWS_CURSOR
	if (nextCursor) {
	    // Not grabbing -> set the correct cursor
	    setCursor(nextCursor);
	    nextCursor = 0;
	}
#endif
	mouseGrabbing = FALSE;
	mouseGrabber = 0;
    }
}

#ifndef QT_NO_SOUND
void QWSServer::invokePlaySound( QWSPlaySoundCommand *cmd, QWSClient * )
{
    soundserver->playFile(cmd->filename);
}
#endif

#ifndef QT_NO_COP
void QWSServer::invokeRegisterChannel( QWSQCopRegisterChannelCommand *cmd,
				       QWSClient *client )
{
    QCopChannel::registerChannel( cmd->channel, client );
}

void QWSServer::invokeQCopSend( QWSQCopSendCommand *cmd, QWSClient *client )
{
    QCopChannel::answer( client, cmd->channel, cmd->message, cmd->data );
}

#endif

#ifndef QT_NO_QWS_REPEATER
void QWSServer::invokeRepaintRegion(QWSRepaintRegionCommand * cmd,
				    QWSClient * client)
{
    QRegion r1;
    r1.setRects(cmd->rectangles,cmd->simpleData.numrects);
    refresh(r1);
}
#endif

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
    emit windowEvent( w, Create );
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
	rgnMan->commit();
	changingw->updateAllocation(); // still need ack
	return;
    }

    int windowPos = 0;

    //change position in list:
    QWSWindow *w = windows.first();
    while ( w ) {
	if ( w == changingw ) {
	    windowPos = windows.at();
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

    if ( windowPos != windows.at() ) {
	// window changed position
	setWindowRegion( changingw, changingw->requested_region );
    }
    syncRegions( changingw );
    emit windowEvent( changingw, Raise );
}

void QWSServer::lowerWindow( QWSWindow *changingw, int )
{
    if ( changingw == windows.last() ) {
	rgnMan->commit();
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
    emit windowEvent( changingw, Lower );
}

void QWSServer::moveWindowRegion( QWSWindow *changingw, int dx, int dy )
{
    if ( !changingw ) return;

    QRegion oldAlloc( changingw->allocation() );
    oldAlloc.translate( dx, dy );
    QRegion newRegion( changingw->requested_region );
    newRegion.translate( dx, dy );
/*
    for ( int i = 0; i < oldAlloc.rects().count(); i++ )
	qDebug( "oldAlloc %d, %d %dx%d",
	    oldAlloc.rects()[i].x(),
	    oldAlloc.rects()[i].y(),
	    oldAlloc.rects()[i].width(),
	    oldAlloc.rects()[i].height() );
*/
    setWindowRegion( changingw, newRegion );
/*
    for ( int i = 0; i < changingw->allocation().rects().count(); i++ )
	qDebug( "newAlloc %d, %d %dx%d",
	    changingw->allocation().rects()[i].x(),
	    changingw->allocation().rects()[i].y(),
	    changingw->allocation().rects()[i].width(),
	    changingw->allocation().rects()[i].height() );
*/
    // add exposed areas
    changingw->exposed = changingw->allocation() - oldAlloc;

    QWSDisplay::grab( TRUE );
    rgnMan->commit();

    // safe to blt now
    QRegion cr( changingw->allocation() );
    cr &= oldAlloc;

    QSize s = QSize(swidth, sheight);
    cr = qt_screen->mapFromDevice( cr, s );
    QPoint p1 = qt_screen->mapFromDevice( QPoint(0, 0), s );
    QPoint p2 = qt_screen->mapFromDevice( QPoint(dx, dy), s );

    QRect br( cr.boundingRect() );
    gfx->setClipRegion( cr );
    gfx->scroll( br.x(), br.y(), br.width(), br.height(),
		 br.x() - (p2.x() - p1.x()), br.y() - (p2.y() - p1.y()) );
    gfx->setClipRegion( screenRegion );
    QWSDisplay::ungrab();
/*
    for ( int i = 0; i < changingw->exposed.rects().count(); i++ )
	qDebug( "svr exposed: %d, %d %dx%d",
	    changingw->exposed.rects()[i].x(),
	    changingw->exposed.rects()[i].y(),
	    changingw->exposed.rects()[i].width(),
	    changingw->exposed.rects()[i].height() );
*/
    notifyModified( changingw );
    paintBackground( dirtyBackground );
    dirtyBackground = QRegion();
}

/*!
  Changes the requested region of window \a changingw to \a r,
  sends appropriate region change events to all appropriate
  clients, and waits for all required acknowledgements.

  If \a changingw is 0, the server's reserved region is changed.
*/
void QWSServer::setWindowRegion( QWSWindow* changingw, QRegion r )
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
	changingw->addAllocation( rgnMan, extra_allocation & screenRegion );
    else if ( !disablePainting )
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

/*!
  Closes the pointer device(s).
*/
void QWSServer::closeMouse()
{
    mousehandlers.setAutoDelete(TRUE);
    mousehandlers.clear();
}

/*!
  Opens the mouse device(s).
*/
void QWSServer::openMouse()
{
    QString mice = getenv("QWS_MOUSE_PROTO");
    if ( mice.isEmpty() ) {
#if defined(QT_QWS_CASSIOPEIA)
	mice = "TPanel:/dev/tpanel";
#elif !defined(QT_NO_QWS_VFB)
	extern bool qvfbEnabled;
	if ( qvfbEnabled )
	    mice = "QVFbMouse";
#endif
	if ( mice.isEmpty() )
	    mice = defaultMouse;
    }
    closeMouse();
    if ( mice == "None" )
	return;
#ifndef QT_NO_STRINGLIST
    QStringList mouse = QStringList::split(" ",mice);
    for (QStringList::Iterator m=mouse.begin(); m!=mouse.end(); ++m) {
	(void)newMouseHandler(*m);
    }
#else
    (void)newMouseHandler(mice); //Assume only one
#endif
}

#ifndef QT_NO_QWS_KEYBOARD

/*!
  Closes keyboard device(s).
*/
void QWSServer::closeKeyboard()
{
    keyboardhandlers.setAutoDelete(TRUE);
    keyboardhandlers.clear();
}

/*!
  Returns the primary keyboard handler.
*/
QWSKeyboardHandler* QWSServer::keyboardHandler()
{
    return qwsServer->keyboardhandlers.first();
}

/*!
  Sets the primary keyboard handler to \a kh.
*/
void QWSServer::setKeyboardHandler(QWSKeyboardHandler* kh)
{
    qwsServer->keyboardhandlers.prepend(kh);
}

/*!
  Opens the keyboard device(s).
*/
void QWSServer::openKeyboard()
{
    QString keyboards = getenv("QWS_KEYBOARD");
    if ( keyboards.isEmpty() ) {
#if defined( QT_QWS_CASSIOPEIA )
	keyboards = "Buttons";
#elif !defined(QT_NO_QWS_VFB)
	extern bool qvfbEnabled;
	if ( qvfbEnabled )
	    keyboards = "QVFbKeyboard";
#endif
	if ( keyboards.isEmpty() ) {
	    keyboards = defaultKeyboard;	// last resort
	}
    }
    closeKeyboard();
    if ( keyboards == "None" )
	return;
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

#endif //QT_NO_QWS_KEYBOARD

QPoint QWSServer::mousePosition;
QColor *QWSServer::bgColor = 0;
QImage *QWSServer::bgImage = 0;

void QWSServer::move_region( const QWSRegionMoveCommand *cmd )
{
    QWSClient *serverClient = client[-1];
    invokeRegionMove( cmd, serverClient );
}

void QWSServer::set_altitude( const QWSChangeAltitudeCommand *cmd )
{
    QWSClient *serverClient = client[-1];
    invokeSetAltitude( cmd, serverClient );
}


void QWSServer::request_region( int wid, QRegion region )
{
    QWSClient *serverClient = client[-1];
    QWSWindow* changingw = findWindow( wid, serverClient );
    if ( !region.isEmpty() )
	changingw->setNeedAck( TRUE );
    bool isShow = !changingw->isVisible() && !region.isEmpty();
    setWindowRegion( changingw, region );
    syncRegions( changingw );
    if ( isShow )
	emit windowEvent( changingw, Show );
    if ( !region.isEmpty() )
	emit windowEvent( changingw, Geometry );
    else
	emit windowEvent( changingw, Hide );
    if ( focusw == changingw && region.isEmpty() )
	setFocus(changingw,FALSE);
}


void QWSServer::openDisplay()
{
    qt_init_display();

    rgnMan = qt_fbdpy->regionManager();
    swidth = qt_screen->deviceWidth();
    sheight = qt_screen->deviceHeight();
    gfx = qt_screen->screenGfx();
}


void QWSServer::closeDisplay()
{
    qt_screen->shutdownDevice();
}


void QWSServer::paintServerRegion()
{
}

void QWSServer::paintBackground( QRegion r )
{
    if ( bgImage && bgImage->isNull() )
	return;
    if ( !r.isEmpty() ) {
	Q_ASSERT ( qt_fbdpy );

	r = qt_screen->mapFromDevice( r, QSize(swidth, sheight) );

	gfx->setClipRegion( r );
	QRect br( r.boundingRect() );
	if ( !bgImage ) {
	    gfx->setBrush(QBrush( *bgColor ));
	    gfx->fillRect( br.x(), br.y(), br.width(), br.height() );
	} else {
	    gfx->setSource( bgImage );
	    gfx->setBrushOffset( br.x(), br.y() );
	    gfx->tiledBlt( br.x(), br.y(), br.width(), br.height() );
	}
	gfx->setClipRegion( QRegion() );
    }
}


void QWSServer::refreshBackground()
{
    QRegion r(0, 0, swidth, sheight);
    for (uint i=0; i<windows.count(); i++) {
	if ( r.isEmpty() )
	    return; // Nothing left for deeper windows
	QWSWindow* w = windows.at(i);
	r -= w->allocation();
    }
    paintBackground( r );
}


/*!
  Sets the image \a img to be used as the background in the absence of
  obscuring windows.
*/

void QWSServer::setDesktopBackground( const QImage &img )
{

    if ( !bgImage )
	bgImage = new QImage( img );
    else
	*bgImage = img;

    if ( qwsServer )
	qwsServer->refreshBackground();
}

/*!
  \overload
  Sets the color \a c to be used as the background in the absence of obscuring
  windows.
*/

void QWSServer::setDesktopBackground( const QColor &c )
{
    if ( !bgColor )
	bgColor = new QColor( c );
    else
	*bgColor = c;

    if ( bgImage ) {
	delete bgImage;
	bgImage = 0;
    }

    if ( qwsServer )
	qwsServer->refreshBackground();
}

/*!
  \internal
 */
void QWSServer::startup(int flags)
{
    if ( qwsServer )
	return;
    unlink( qws_qtePipeFilename().latin1() );
    (void)new QWSServer(flags);
}


/*!
  \internal
*/

void QWSServer::closedown()
{
    unlink( qws_qtePipeFilename().latin1() );
    delete qwsServer;
    qwsServer = 0;
}


void QWSServer::emergency_cleanup()
{
#ifndef QT_NO_QWS_KEYBOARD
    if ( qwsServer )
	qwsServer->closeKeyboard();
#endif
}

#ifndef QT_NO_QWS_KEYBOARD
static QWSServer::KeyboardFilter *keyFilter;

/*!
  \internal
*/
void QWSServer::processKeyEvent(int unicode, int keycode, int modifiers, bool isPress,
  bool autoRepeat)
{
    if ( !keyFilter ||
	 !keyFilter->filter( unicode, keycode, modifiers, isPress, autoRepeat ) )
	sendKeyEvent( unicode, keycode, modifiers, isPress, autoRepeat );
}

/*!
  Sets a filter \a f to be invoked for all key events from physical keyboard
  drivers (events sent via processKeyEvent()).
  The filter is not invoked for keys generated by virtual keyboard
  drivers (events sent via sendKeyEvent()).
*/
void QWSServer::setKeyboardFilter( KeyboardFilter *f )
{
    keyFilter = f;
}
#endif

/*!
  Sets the timeout for the screensaver to \a ms milliseconds. A setting
  of zero turns off the screensaver.
*/
void QWSServer::setScreenSaverInterval(int ms)
{
    if ( ms < qwsServer->screensaverinterval )
	qwsServer->screenSaverWake();
    qwsServer->screensaverinterval = ms;
    if ( !qwsServer->screensaverinterval )
	if ( !qwsServer->screensavertimer->isActive() )
	    qt_screen->blank(FALSE);
}

extern bool qt_disable_lowpriority_timers;

void QWSServer::screenSaverWake()
{
    if ( screensaverinterval ) {
	if ( !screensavertimer->isActive() )
	    qt_screen->blank(FALSE);
	screensavertimer->start(screensaverinterval,TRUE);
    }
    qt_disable_lowpriority_timers=FALSE;
}

void QWSServer::screenSaverSleep()
{
    qt_screen->blank(TRUE);
    screensavertimer->stop();
    qt_disable_lowpriority_timers=TRUE;
}

/*!
  Returns TRUE if the screensaver is active (i.e. the screen is
  blanked); otherwise returns FALSE.
*/
bool QWSServer::screenSaverActive()
{
    return qwsServer->screensaverinterval
	&& !qwsServer->screensavertimer->isActive();
}

/*!
    If \a activate is TRUE the screensaver is activated immediately;
    if \a activate is FALSE the screensaver is deactivated.
*/
void QWSServer::screenSaverActivate(bool activate)
{
    if ( activate )
	qwsServer->screenSaverSleep();
    else
	qwsServer->screenSaverWake();
}

/*!
\fn  QWSWindow::QWSWindow(int i, QWSClient * client)

  Constructs a new top-level window, associated with the client \a client
  and giving it the id \a i.
*/

/*!
\fn QWSServer::manager()

  Returns the QWSPropertyManager, which is used for implementing
  X11-style window properties.
*/

/*!
  \fn QWSServer::windowEvent(QWSWindow * w, QWSServer::WindowEvent e)

This signal is emitted whenever something happens to a top level
window (e.g. it's created or destroyed). \a w is the window
to which the event of type \a e has occurred.
*/

/*!
  \fn QWSServer::keyMap()

Returns the keyboard mapping table used to convert keyboard
scancodes to Qt keycodes and unicode values. It's used by the keyboard
driver in qkeyboard_qws.cpp.
*/

/*!
  \enum QWSServer::ServerFlags
This enum is used to pass various options to the window system server.
Currently defined are:

\value DisableKeyboard Ignore all keyboard input.
\value DisableMouse Ignore all mouse input.
*/

/*!
  \enum QWSServer::GUIMode
This determines what sort of QWS server to create:

\value NoGui This is used for non-graphical Qt applications.
\value NormalGUI A normal Qt/Embedded application (not the server).
\value Server A Qt/Embedded server (e.g. if -qws has been specified
  on the command line.
*/

/*!
  \class QWSServer::KeyMap
  \brief The QWSServer::KeyMap class is used for mapping scancodes.

  The KeyMap structure records an individual KeyMap entry in the
  array used to map keyboard scancodes to Qt key codes and unicode
  values.
*/

/*!
  \class QWSServer::KeyboardFilter
  \brief The QWSServer::KeyboardFilter class provides a global keyboard
  event filter.

  The KeyboardFilter class is used to implement a global, low-level
  filter on key events in the Qt/Embedded server application; this
  can be used to implement things like APM (advanced power management)
  suspend from a button without having to filter for it in all
  applications.
*/

/*!
  \enum QWSServer::WindowEvent
This specifies what sort of event has occurred to a top level window:

\value Create A new window has been created (QWidget constructor).
\value Destroy The window has been closed and deleted (QWidget destructor).
\value Hide The window has been hidden with QWidget::hide().
\value Show The window has been shown with QWidget::show() or similar.
\value Raise The window has been raised to the top of the desktop.
\value Lower The window has been lowered.
\value Geometry The window has changed size or position.
*/

