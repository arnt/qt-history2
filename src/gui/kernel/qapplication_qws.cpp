/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qglobal.h"
#include "qcursor.h"
#include "qapplication.h"
#include "private/qapplication_p.h"
#include "qwidget.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qtextcodec.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qsocketnotifier.h"
#include "qsessionmanager.h"
#include "qclipboard.h"
#include "qbitmap.h"
#include "qwssocket_qws.h"
#include "qtransportauth_qws.h"
#include "private/qtransportauth_qws_p.h"
#include "qwsevent_qws.h"
#include "private/qwscommand_qws_p.h"
#include "qwsproperty_qws.h"
#include "qscreen_qws.h"
#include "qcopchannel_qws.h"
#include "private/qlock_p.h"
#include "private/qwslock_p.h"
//#include "qmemorymanager_qws.h"
#include "qwsmanager_qws.h"
//#include "qwsregionmanager_qws.h"
#include "qwindowsystem_qws.h"
#include "private/qwindowsystem_p.h"

#include "qwsdisplay_qws.h"
#include "private/qwsdisplay_qws_p.h"
#include "private/qwsinputcontext_p.h"
#include "qfile.h"
#include "qhash.h"
#include "qdesktopwidget.h"
#include "qcolormap.h"
#include "private/qcursor_p.h"
#include "qsettings.h"
#include "qdebug.h"
#include "qeventdispatcher_qws_p.h"
#include "private/qwidget_p.h"
#include "private/qbackingstore_p.h"
#include "private/qwindowsurface_qws_p.h"
#include "private/qfont_p.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef QT_NO_QWS_MULTIPROCESS
#ifdef QT_NO_QSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#ifndef Q_OS_DARWIN
# include <sys/sem.h>
#endif
#include <sys/socket.h>
#else
#include "private/qsharedmemory_p.h"
#endif
#endif

#ifndef QT_NO_DIRECTPAINTER
class QDirectPainter;
extern void qt_directpainter_region(QDirectPainter *dp, const QRegion &alloc, int type);
#ifndef QT_NO_QWSEMBEDWIDGET
extern void qt_directpainter_embedevent(QDirectPainter *dp,
                                        const QWSEmbedEvent *e);
#endif
#endif // QT_NO_DIRECTPAINTER

const int qwsSharedRamSize = 1 * 1024; // misc data, written by server, read by clients

extern QApplication::Type qt_appType;
extern bool qt_app_has_font;

//these used to be environment variables, they are initialized from
//environment variables in

bool qws_savefonts = false;
bool qws_screen_is_interlaced=false; //### should be detected
bool qws_shared_memory = false;
bool qws_sw_cursor = true;
bool qws_accel = true;            // ### never set
QByteArray qws_display_spec(":0");
Q_GUI_EXPORT int qws_display_id = 0;
int qws_client_id = 0;
QWidget *qt_pressGrab = 0;
QWidget *qt_mouseGrb = 0;
int *qt_last_x = 0;
int *qt_last_y = 0;

static int mouse_x_root = -1;
static int mouse_y_root = -1;
static int mouse_state = 0;

int qt_servershmid = -1;

bool qws_overrideCursor = false;
#ifndef QT_NO_QWS_MANAGER
#include "qdecorationfactory_qws.h"
static QDecoration *qws_decoration = 0;
#endif

#if defined(QT_DEBUG)
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

// Get the name of the directory where Qtopia Core temporary data should
// live.
QString qws_dataDir()
{
    static QString result;
    if (!result.isEmpty())
        return result;
    QByteArray dataDir = QString("/tmp/qtembedded-%1").arg(qws_display_id).toLocal8Bit();
    if (mkdir(dataDir, 0700)) {
        if (errno != EEXIST) {
            qFatal("Cannot create Qtopia Core data directory: %s", dataDir.constData());
        }
    }

    struct stat buf;
    if (lstat(dataDir, &buf))
        qFatal("stat failed for Qtopia Core data directory: %s", dataDir.constData());

    if (!S_ISDIR(buf.st_mode))
        qFatal("%s is not a directory", dataDir.constData());
    if (buf.st_uid != getuid())
        qFatal("Qtopia Core data directory is not owned by user %d", getuid());

    if ((buf.st_mode & 0677) != 0600)
        qFatal("Qtopia Core data directory has incorrect permissions: %s", dataDir.constData());
    dataDir += "/";

    result = QString(dataDir);
    return result;
}

// Get the filename of the pipe Qtopia Core uses for server/client comms
Q_GUI_EXPORT QString qws_qtePipeFilename()
{
    return (qws_dataDir() + QString(QTE_PIPE).arg(qws_display_id));
}

static void setMaxWindowRect(const QRect &rect)
{
    const QList<QScreen*> subScreens = qt_screen->subScreens();
    QScreen *screen = qt_screen;
    for (int i = 0; i < subScreens.size(); ++i) {
        if (subScreens.at(i)->region().contains(rect)) {
            screen = subScreens.at(i);
            break;
        }
    }

    const QSize devSize(screen->deviceWidth(), screen->deviceHeight());
    QRect r = screen->mapFromDevice(rect, devSize);

    QApplicationPrivate *ap = QApplicationPrivate::instance();
    ap->setMaxWindowRect(screen, r);
}

void QApplicationPrivate::setMaxWindowRect(const QScreen *screen,
                                           const QRect &rect)
{
    if (maxWindowRects.value(screen) == rect)
        return;

    maxWindowRects[screen] = rect;

    // Re-resize any maximized windows
    QWidgetList l = QApplication::topLevelWidgets();
    for (int i = 0; i < l.size(); ++i) {
        QWidget *w = l.at(i);
        QScreen *s = w->d_func()->getScreen();
        if (w->isVisible() && w->isMaximized() && s == screen) {
            w->showNormal(); //#### flicker
            w->showMaximized();
        }
    }
}

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/


static QString appName;                          // application name
static const char *appFont = 0;                  // application font
static const char *appBGCol = 0;                 // application bg color
static const char *appFGCol = 0;                 // application fg color
static const char *appBTNCol = 0;                // application btn color
static const char *mwGeometry = 0;               // main widget geometry
static const char *mwTitle = 0;                  // main widget title
//static bool mwIconic = false;                  // main widget iconified

static bool app_do_modal = false;                // modal mode
Q_GUI_EXPORT QWSDisplay *qt_fbdpy = 0;                        // QWS `display'
QLock *QWSDisplay::lock = 0;

static int mouseButtonPressed = 0;               // last mouse button pressed
static int mouseButtonPressTime = 0;             // when was a button pressed
static short mouseXPos, mouseYPos;               // mouse position in act window

extern QWidgetList *qt_modal_stack;              // stack of modal widgets

static QWidget *popupButtonFocus = 0;
static QWidget *popupOfPopupButtonFocus = 0;
static bool popupCloseDownMode = false;
static bool popupGrabOk;
static QPointer<QWidget> *mouseInWidget = 0;

static bool sm_blockUserInput = false;           // session management

QWidget *qt_button_down = 0;                     // widget got last button-down
WId qt_last_cursor = 0xffffffff;                 // Was -1, but WIds are unsigned

class QWSMouseEvent;
class QWSKeyEvent;

class QETWidget : public QWidget                 // event translator widget
{
public:
    bool translateMouseEvent(const QWSMouseEvent *, int oldstate);
    bool translateKeyEvent(const QWSKeyEvent *, bool grab);
    bool translateRegionEvent(const QWSRegionEvent *);
#ifndef QT_NO_QWSEMBEDWIDGET
    void translateEmbedEvent(const QWSEmbedEvent *event);
#endif
    bool translateWheelEvent(const QWSMouseEvent *me);
    void repaintDecoration(QRegion r, bool post);
    void updateRegion();

    bool raiseOnClick()
    {
        // With limited windowmanagement/taskbar/etc., raising big windows
        // (eg. spreadsheet) over the top of everything else (eg. calculator)
        // is just annoying.
        return !isMaximized() && !isFullScreen();
    }
};

void QApplicationPrivate::createEventDispatcher()
{
    Q_Q(QApplication);
    eventDispatcher = (q->type() != QApplication::Tty
                       ? new QEventDispatcherQWS(q)
                       : new QEventDispatcherUNIX(q));
}

// Single-process stuff. This should maybe move into qwindowsystem_qws.cpp

static bool qws_single_process;
static QList<QWSEvent*> incoming;
static QList<QWSCommand*> outgoing;

void qt_client_enqueue(const QWSEvent *event)
{
    QWSEvent *copy = QWSEvent::factory(event->type);
    copy->copyFrom(event);
    incoming.append(copy);
}

QList<QWSCommand*> *qt_get_server_queue()
{
    return &outgoing;
}

void qt_server_enqueue(const QWSCommand *command)
{
    QWSCommand *copy = QWSCommand::factory(command->type);
    copy->copyFrom(command);
    outgoing.append(copy);
}

QWSDisplay::Data::Data(QObject* parent, bool singleProcess)
{
#ifdef QT_NO_QWS_MULTIPROCESS
    Q_UNUSED(parent);
    Q_UNUSED(singleProcess);
#else
    if (singleProcess)
        csocket = 0;
    else {
        csocket = new QWSSocket(parent);
        QObject::connect(csocket, SIGNAL(disconnected()),
                         qApp, SLOT(quit()));
    }
    clientLock = 0;
#endif
    init();
}

QWSDisplay::Data::~Data()
{
//        delete rgnMan; rgnMan = 0;
//        delete memorymanager; memorymanager = 0;
    qt_screen->disconnect();
    delete qt_screen; qt_screen = 0;
#ifndef QT_NO_QWS_CURSOR
    delete qt_screencursor; qt_screencursor = 0;
#endif
#ifndef QT_NO_QWS_MULTIPROCESS
    shm.detach();
    if (csocket) {
        QWSCommand shutdownCmd(QWSCommand::Shutdown, 0, 0);
        shutdownCmd.write(csocket);
        csocket->flush(); // may be pending QCop message, eg.
        delete csocket;
    }
    delete clientLock;
    clientLock = 0;
#endif
    delete connected_event;
    delete mouse_event;
    delete current_event;
    qDeleteAll(queue);
#ifndef QT_NO_COP
    delete qcop_response;
#endif
}

bool QWSDisplay::Data::lockClient(QWSLock::LockType type, int timeout)
{
    return !clientLock || clientLock->lock(type, timeout);
}

void QWSDisplay::Data::unlockClient(QWSLock::LockType type)
{
    if (clientLock) clientLock->unlock(type);
}

bool QWSDisplay::Data::waitClient(QWSLock::LockType type, int timeout)
{
    return !clientLock || clientLock->wait(type, timeout);
}

QWSLock* QWSDisplay::Data::getClientLock()
{
    return clientLock;
}

void QWSDisplay::Data::flush()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (csocket) {
        csocket->waitForReadyRead(0);
        csocket->flush();
   }
#endif
}

#if 0
void QWSDisplay::Data::debugQueue() {
    for (int i = 0; i < queue.size(); ++i) {
        QWSEvent *e = queue.at(i);
        qDebug( "   ev %d type %d sl %d rl %d", i, e->type, e->simpleLen, e->rawLen);
    }
}
#endif

bool QWSDisplay::Data::queueNotEmpty()
{
    return mouse_event/*||region_event*/||queue.count() > 0;
}
QWSEvent* QWSDisplay::Data::dequeue()
{
    QWSEvent *r=0;
    if (queue.count()) {
        r = queue.first(); queue.removeFirst();
    } else if (mouse_event) {
        r = mouse_event;
        mouse_event = 0;
#ifdef QAPPLICATION_EXTRA_DEBUG
        mouse_event_count = 0;
#endif
#if 0
    } else {
        r = region_event;
        region_event = 0;
#endif
    }
    return r;
}

QWSEvent* QWSDisplay::Data::peek()
{
    return queue.first();
}

bool QWSDisplay::Data::directServerConnection()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    return csocket == 0;
#else
    return true;
#endif
}

#if 0
void QWSDisplay::Data::offsetPendingExpose(int, const QPoint &);
void QWSDisplay::Data::translateExpose(QWSRegionModifiedEvent *re, const QPoint &p)
{
    for (int i = 0; i < re->simpleData.nrectangles; i++)
        re->rectangles[i].translate(p.x(), p.y());
}
#endif
void QWSDisplay::Data::create(int n)
{
    QWSCreateCommand cmd(n);
    sendCommand(cmd);
}

void QWSDisplay::Data::flushCommands()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if  (csocket)
        csocket->flush();
#endif
}

void QWSDisplay::Data::sendCommand(QWSCommand & cmd)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if  (csocket)
        cmd.write(csocket);
    else
#endif
        qt_server_enqueue(&cmd);
}

void QWSDisplay::Data::sendSynchronousCommand(QWSCommand & cmd)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if  (csocket) {
        lockClient(QWSLock::Communication);
        cmd.write(csocket);
        csocket->waitForBytesWritten();
        waitClient(QWSLock::Communication);
    } else
#endif
        qt_server_enqueue(&cmd);
}

int QWSDisplay::Data::takeId()
{
    if (unused_identifiers.count() == 10)
        create(15);
    if (unused_identifiers.count() == 0)
        waitForCreation();
    return unused_identifiers.takeFirst();
}

void QWSDisplay::Data::setMouseFilter(void (*filter)(QWSMouseEvent*))
{
    mouseFilter = filter;
}

QWSLock* QWSDisplay::Data::clientLock = 0;

#ifndef QT_NO_QWS_MULTIPROCESS
void Q_GUI_EXPORT qt_app_reinit( const QString& newAppName )
{
    qt_fbdpy->d->reinit( newAppName );
}

#endif

class QDesktopWidget;
extern QDesktopWidget *qt_desktopWidget;

#ifndef QT_NO_QWS_MULTIPROCESS
void QWSDisplay::Data::reinit( const QString& newAppName )
{
    Q_ASSERT(csocket);

    delete connected_event;
    connected_event = 0;
//    region_ack = 0;
    delete mouse_event;
    mouse_event = 0;
//    region_event = 0;
    region_offset_window = 0;
#ifndef QT_NO_COP
    delete qcop_response;
    qcop_response = 0;
#endif
    delete current_event;
    current_event = 0;
#ifdef QAPPLICATION_EXTRA_DEBUG
    mouse_event_count = 0;
#endif
    mouseFilter = 0;

    qt_desktopWidget = 0;
    delete QWSDisplay::Data::clientLock;
    QWSDisplay::Data::clientLock = 0;

    QString pipe = qws_qtePipeFilename();

    // QWS client
    // Cleanup all cached ids
    unused_identifiers.clear();
    delete csocket;

    appName = newAppName;
    qApp->setObjectName( appName );

    csocket = new QWSSocket();
    QObject::connect(csocket, SIGNAL(disconnected()),
                     qApp, SLOT(quit()));
    csocket->connectToLocalFile(pipe);

    QWSDisplay::Data::clientLock = new QWSLock();

    QWSIdentifyCommand cmd;
    cmd.setId(appName, QWSDisplay::Data::clientLock->id());

#ifndef QT_NO_SXE
    QTransportAuth *a = QTransportAuth::getInstance();
    QTransportAuth::Data *d = a->connectTransport(
            QTransportAuth::UnixStreamSock |
            QTransportAuth::Trusted,
            csocket->socketDescriptor());
    QAuthDevice *ad = a->authBuf( d, csocket );
    ad->setClient( reinterpret_cast<void*>(csocket) );

    cmd.write(ad);
#else
    cmd.write(csocket);
#endif

    // wait for connect confirmation
    waitForConnection();

    qws_client_id = connected_event->simpleData.clientId;

    if (!QWSDisplay::initLock(pipe, false))
        qFatal("Cannot get display lock");

    if (shm.attach(connected_event->simpleData.servershmid)) {
        sharedRam = static_cast<uchar *>(shm.address());
        QScreen *s = qt_get_screen(qws_display_id, qws_display_spec.constData());
        sharedRamSize += s->memoryNeeded(qws_display_spec.constData());
    } else {
        perror("QWSDisplay::Data::init");
        qFatal("Client can't attach to main ram memory.");
    }

    qApp->desktop();

    // We wait for creation mainly so that we can process important
    // initialization events such as MaxWindowRect that are sent
    // before object id creation.  Waiting here avoids later window
    // resizing since we have the MWR before windows are displayed.
    waitForCreation();

    sharedRamSize -= sizeof(int);
    qt_last_x = reinterpret_cast<int *>(sharedRam + sharedRamSize);
    sharedRamSize -= sizeof(int);
    qt_last_y = reinterpret_cast<int *>(sharedRam + sharedRamSize);

#ifndef QT_NO_COP
    QCopChannel::reregisterAll();
#endif
    csocket->flush();
}
#endif

void QWSDisplay::Data::init()
{
    connected_event = 0;
//    region_ack = 0;
    mouse_event = 0;
    mouse_state = -1;
    mouse_winid = 0;
//    region_event = 0;
    region_offset_window = 0;
#ifndef QT_NO_COP
    qcop_response = 0;
#endif
    current_event = 0;
#ifdef QAPPLICATION_EXTRA_DEBUG
    mouse_event_count = 0;
#endif
    mouseFilter = 0;

    QString pipe = qws_qtePipeFilename();

    sharedRamSize = qwsSharedRamSize;

#ifndef QT_NO_QWS_MULTIPROCESS
    if (csocket)    {
        // QWS client

        connectToPipe();

        QWSDisplay::Data::clientLock = new QWSLock();

        QWSIdentifyCommand cmd;
        cmd.setId(appName, QWSDisplay::Data::clientLock->id());
#ifndef QT_NO_SXE
        QTransportAuth *a = QTransportAuth::getInstance();
        QTransportAuth::Data *d = a->connectTransport(
                QTransportAuth::UnixStreamSock |
                QTransportAuth::Trusted,
                csocket->socketDescriptor());
        QAuthDevice *ad = a->authBuf( d, csocket );
        ad->setClient( reinterpret_cast<void*>(csocket) );
        cmd.write(ad);
#else
        cmd.write(csocket);
#endif

        // create(30); // not necessary, server will send ids anyway
        waitForConnection();

        qws_client_id = connected_event->simpleData.clientId;

        // now we want to get the exact display spec to use if we haven't
        // specified anything.
        if (qws_display_spec.at(0) == ':')
            qws_display_spec = connected_event->display;

        if (!QWSDisplay::initLock(pipe, false))
            qFatal("Cannot get display lock");

        if (shm.attach(connected_event->simpleData.servershmid)) {
            sharedRam = static_cast<uchar *>(shm.address());
            QScreen *s = qt_get_screen(qws_display_id, qws_display_spec.constData());
            sharedRamSize += s->memoryNeeded(qws_display_spec.constData());
        } else {
            perror("QWSDisplay::Data::init");
            qFatal("Client can't attach to main ram memory.");
        }

        // We wait for creation mainly so that we can process important
        // initialization events such as MaxWindowRect that are sent
        // before object id creation.  Waiting here avoids later window
        // resizing since we have the MWR before windows are displayed.
        waitForCreation();
    } else
#endif
    {
        create(30);

        // QWS server
        if (!QWSDisplay::initLock(pipe, true))
            qFatal("Cannot get display lock");

        QScreen *s = qt_get_screen(qws_display_id, qws_display_spec.constData());
        sharedRamSize += s->memoryNeeded(qws_display_spec.constData());

#ifndef QT_NO_QWS_MULTIPROCESS

        if (!shm.create(sharedRamSize)) {
            perror("Cannot create main ram shared memory\n");
            qFatal("Unable to allocate %d bytes of shared memory", sharedRamSize);
        }
        qt_servershmid = shm.id();
        sharedRam = static_cast<uchar *>(shm.address());
#else
        sharedRam=static_cast<uchar *>(malloc(sharedRamSize));
#endif
        // Need to zero index count at end of block, might as well zero
        // the rest too
        memset(sharedRam,0,sharedRamSize);

        QWSIdentifyCommand cmd;
        qt_server_enqueue(&cmd);
    }

    // Allow some memory for the graphics driver too
    //### Note that sharedRamSize() has side effects; it must be called
    //### once, and only once, and before initDevice()
    sharedRamSize -= qt_screen->sharedRamSize(sharedRam+sharedRamSize);

#ifndef QT_NO_QWS_MULTIPROCESS
    if(!csocket)
#endif
    {
        //QWS server process
        qt_screen->initDevice();
    }

    sharedRamSize -= sizeof(int);
    qt_last_x = reinterpret_cast<int *>(sharedRam + sharedRamSize);
    sharedRamSize -= sizeof(int);
    qt_last_y = reinterpret_cast<int *>(sharedRam + sharedRamSize);

    /* Initialise framebuffer memory manager */
    /* Add 4k for luck and to avoid clobbering hardware cursor */
//    int screensize=qt_screen->screenSize();
//     memorymanager=new QMemoryManager(qt_screen->base()+screensize+4096,
//         qt_screen->totalSize()-(screensize+4096),0);

// #ifndef QT_NO_QWS_MULTIPROCESS
//     rgnMan = new QWSRegionManager(pipe, csocket);
// #else
//     rgnMan = new QWSRegionManager(pipe, 0); //####### not necessary
// #endif
#ifndef QT_NO_QWS_MULTIPROCESS
    if (csocket)
        csocket->flush();
#endif
}


QWSEvent* QWSDisplay::Data::readMore()
{
#ifdef QT_NO_QWS_MULTIPROCESS
    return incoming.isEmpty() ? 0 : incoming.takeFirst();
#else
    if (!csocket)
        return incoming.isEmpty() ? 0 : incoming.takeFirst();
    // read next event
    if (!current_event) {
        int event_type = qws_read_uint(csocket);

        if (event_type >= 0) {
            current_event = QWSEvent::factory(event_type);
        }
    }

    if (current_event) {
        if (current_event->read(csocket)) {
            // Finished reading a whole event.
            QWSEvent* result = current_event;
            current_event = 0;
            return result;
        }
    }

    // Not finished reading a whole event.
    return 0;
#endif
}


void QWSDisplay::Data::fillQueue()
{
    QWSServer::processEventQueue();
    QWSEvent *e = readMore();
    while (e) {
        if (e->type == QWSEvent::Connected) {
            connected_event = static_cast<QWSConnectedEvent *>(e);
            return;
        } else if (e->type == QWSEvent::Creation) {
            QWSCreationEvent *ce = static_cast<QWSCreationEvent*>(e);
            int id = ce->simpleData.objectid;
            int count = ce->simpleData.count;
            for (int i = 0; i < count; ++i)
                unused_identifiers.append(id++);
            delete e;
        } else if (e->type == QWSEvent::Mouse) {
            if (!qt_screen) {
                delete e;
            } else {
                QWSMouseEvent *me = static_cast<QWSMouseEvent*>(e);
                if (mouseFilter)
                    mouseFilter(me);
#ifdef QAPPLICATION_EXTRA_DEBUG
                static const char *defaultAction= "INITIAL";
                const char * action = defaultAction;
#endif
                delete mouse_event;
                if (mouse_winid != me->window ()
                    || mouse_state != me->simpleData.state) {
                        queue.append(me);
                        mouse_winid = me->window();
                        mouse_state = me->simpleData.state;
                        mouse_event = 0;
#ifdef QAPPLICATION_EXTRA_DEBUG
                        mouse_event_count = 0;
                        action = "ENQUEUE";
#endif
                } else {
#ifdef QAPPLICATION_EXTRA_DEBUG
                    if (mouse_event)
                        action = "COMPRESS";
                    mouse_event_count++;
#endif
                    mouse_event = me;
                }
#ifdef QAPPLICATION_EXTRA_DEBUG
                if (me->simpleData.state !=0 || action != defaultAction || mouse_event_count != 0)
                    qDebug("fillQueue %s (%d,%d), state %x win %d count %d", action,
                           me->simpleData.x_root, me->simpleData.y_root, me->simpleData.state,
                           me->window(), mouse_event_count);
#endif
            }
#if 0
        } else if (e->type == QWSEvent::RegionModified) {
            QWSRegionModifiedEvent *re = static_cast<QWSRegionModifiedEvent *>(e);
            if (re->simpleData.is_ack) {
                region_ack = re;
                region_offset = QPoint();
                region_offset_window = 0;
            } else {
                if (region_offset_window == re->window() && !region_offset.isNull()) {
//                    qDebug("Rgn Adjust a %d, %d", region_offset.x(), region_offset.y());
                    translateExpose(re, region_offset);
                }
                if (!region_event || re->window() == region_event->window()) {
                    if (region_event) {
                        QRegion r1;
                        r1.setRects(re->rectangles, re->simpleData.nrectangles);
                        QRegion r2;
                        r2.setRects(region_event->rectangles,
                                region_event->simpleData.nrectangles);
                        QRegion ur(r1 + r2);
                        region_event->setData(reinterpret_cast<const char *>(ur.rects().constData()),
                                ur.rects().count() * sizeof(QRect), true);
                        region_event->simpleData.nrectangles = ur.rects().count();
                        delete e;
                    } else {
                        region_event = re;
                    }
                } else {
                    queue.append(e);
                }
            }
#endif // 0 (RegionModified)
        } else if (e->type==QWSEvent::MaxWindowRect && qt_screen) {
            // Process this ASAP, in case new widgets are created (startup)
            setMaxWindowRect((static_cast<QWSMaxWindowRectEvent*>(e))->simpleData.rect);
            delete e;
#ifndef QT_NO_COP
        } else if (e->type == QWSEvent::QCopMessage) {
            QWSQCopMessageEvent *pe = static_cast<QWSQCopMessageEvent*>(e);
            if (pe->simpleData.is_response) {
                qcop_response = pe;
            } else {
                queue.append(e);
            }
#endif
        } else {
            queue.append(e);
        }
        //debugQueue();
        e = readMore();
    }
}
#if 0
void QWSDisplay::Data::offsetPendingExpose(int window, const QPoint &offset)
{
    if (offset.isNull())
        return;

    region_offset = offset;
    region_offset_window = window;
    for (int i = 0; i < queue.size(); ++i) {
        QWSEvent *e = queue.at(i);
        if (e->type == QWSEvent::RegionModified) {
            QWSRegionModifiedEvent *re = static_cast<QWSRegionModifiedEvent *>(e);
            if (!re->simpleData.is_ack && region_offset_window == re->window()) {
//                qDebug("Rgn Adjust b %d, %d", region_offset.x(), region_offset.y());
                translateExpose(re, region_offset);
            }
        }
    }

    if (region_event && region_offset_window == region_event->window()) {
//        qDebug("Rgn Adjust c %d, %d", region_offset.x(), region_offset.y());
        translateExpose(region_event, region_offset);
    }
}
#endif

#ifndef QT_NO_QWS_MULTIPROCESS
void QWSDisplay::Data::connectToPipe()
{
    Q_ASSERT(csocket);

    const QString pipe = qws_qtePipeFilename();
    int i = 0;
    while (!csocket->connectToLocalFile(pipe)) {
        if (++i > 5) {
            qWarning("No Qtopia Core server appears to be running.");
            qWarning("If you want to run this program as a server,");
            qWarning("add the \"-qws\" command-line option.");
            exit(1);
        }
        sleep(1);
    }
}

void QWSDisplay::Data::waitForConnection()
{
    connected_event = 0;

    for (int i = 0; i < 5; i++) {
        fillQueue();
        if (connected_event)
            return;
        csocket->flush();
        csocket->waitForReadyRead(1000);
    }

    csocket->flush();
    if (!connected_event)
        qFatal("Did not receive a connection event from the qws server");
}

#if 0
void QWSDisplay::Data::waitForRegionAck()
{
    for (;;) {
        fillQueue();
        if (region_ack)
            break;
#ifndef QT_NO_QWS_MULTIPROCESS
        if (csocket) {
            csocket->flush();
            csocket->waitForReadyRead(1000);
            if (csocket->state() != QAbstractSocket::ConnectedState)
                return;
        }
#endif
    }
    queue.prepend(region_ack);
    region_ack = 0;
}
#endif
#endif // QT_NO_QWS_MULTIPROCESS

void QWSDisplay::Data::waitForCreation()
{
    fillQueue();
#ifndef QT_NO_QWS_MULTIPROCESS
    while (unused_identifiers.count() == 0) {
        if (csocket) {
            csocket->flush();
            csocket->waitForReadyRead(1000);
        }
        fillQueue();
    }
#endif
}

#ifndef QT_NO_COP
void QWSDisplay::Data::waitForQCopResponse()
{
    for (;;) {
        fillQueue();
        if (qcop_response)
            break;
#ifndef QT_NO_QWS_MULTIPROCESS
        if (csocket) {
            csocket->flush();
            csocket->waitForReadyRead(1000);
        }
#endif
    }
    queue.prepend(qcop_response);
    qcop_response = 0;
}
#endif


/*!
    \class QWSDisplay qwsdisplay_qws.h
    \brief The QWSDisplay class provides a display for QWS; it is an internal class.

    \internal

    \ingroup qws
*/

QWSDisplay::QWSDisplay()
{
    d = new Data(0, qws_single_process);
}

QWSDisplay::~QWSDisplay()
{
    delete d;
    delete lock;
    lock = 0;
}

bool QWSDisplay::grabbed()
{
    return lock->locked();
}

void QWSDisplay::grab()
{
    lock->lock(QLock::Read);
}

void QWSDisplay::grab(bool write)
{
    lock->lock(write ? QLock::Write : QLock::Read);

}
void QWSDisplay::ungrab()
{
    lock->unlock();
}

#if 0
QWSRegionManager *QWSDisplay::regionManager() const
{
    return d->rgnMan;
}
#endif

bool QWSDisplay::eventPending() const
{
#ifndef QT_NO_QWS_MULTIPROCESS
    d->flush();
#endif
    d->fillQueue();
    return d->queueNotEmpty();
}


/*
  Caller must delete return value!
 */
QWSEvent *QWSDisplay::getEvent()
{
    d->fillQueue();
    Q_ASSERT(d->queueNotEmpty());
    QWSEvent* e = d->dequeue();

    return e;
}

uchar* QWSDisplay::frameBuffer() const { return qt_screen->base(); }
int QWSDisplay::width() const { return qt_screen->width(); }
int QWSDisplay::height() const { return qt_screen->height(); }
int QWSDisplay::depth() const { return qt_screen->depth(); }
int QWSDisplay::pixmapDepth() const { return qt_screen->pixmapDepth(); }
bool QWSDisplay::supportsDepth(int depth) const { return qt_screen->supportsDepth(depth); }
uchar *QWSDisplay::sharedRam() const { return d->sharedRam; }
int QWSDisplay::sharedRamSize() const { return d->sharedRamSize; }


void QWSDisplay::addProperty(int winId, int property)
{
    QWSAddPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand(cmd);
}

void QWSDisplay::setProperty(int winId, int property, int mode, const QByteArray &data)
{
    QWSSetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    cmd.simpleData.mode = mode;
    cmd.setData(data.constData(), data.size());
    d->sendCommand(cmd);
}

void QWSDisplay::setProperty(int winId, int property, int mode,
                              const char * data)
{
    QWSSetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    cmd.simpleData.mode = mode;
    cmd.setData(data, strlen(data));
    d->sendCommand(cmd);
}

void QWSDisplay::removeProperty(int winId, int property)
{
    QWSRemovePropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand(cmd);
}

/*
    It is the caller's responsibility to delete[] \a data.
 */
bool QWSDisplay::getProperty(int winId, int property, char *&data, int &len)
{
    QWSGetPropertyCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;
    d->sendCommand(cmd);

    getPropertyLen = -2;
    getPropertyData = 0;

    while (getPropertyLen == -2)
        qApp->processEvents(); //########## USE an ACK event instead. That's dangerous!

    len = getPropertyLen;
    data = getPropertyData;

    getPropertyLen = -2;
    getPropertyData = 0;

    return len != -1;
}

void QWSDisplay::setAltitude(int winId, int alt, bool fixed)
{
    QWSChangeAltitudeCommand cmd;
#ifdef QT_DEBUG
    memset(cmd.simpleDataPtr, 0, sizeof(cmd.simpleData)); //shut up Valgrind
#endif
    cmd.simpleData.windowid = winId;
    cmd.simpleData.altitude = QWSChangeAltitudeCommand::Altitude(alt);
    cmd.simpleData.fixed = fixed;
    if (d->directServerConnection()) {
        qwsServer->d_func()->set_altitude(&cmd);
    } else {
        d->sendSynchronousCommand(cmd);
    }
}

void QWSDisplay::setOpacity(int winId, int opacity)
{
    QWSSetOpacityCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.opacity = opacity;
    if (d->directServerConnection()) {
        qwsServer->d_func()->set_opacity(&cmd);
    } else {
        d->sendCommand(cmd);
    }
}



void QWSDisplay::requestFocus(int winId, bool get)
{
    QWSRequestFocusCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.flag = get;
    if (d->directServerConnection())
        qwsServer->d_func()->request_focus(&cmd);
    else
        d->sendCommand(cmd);
}

void QWSDisplay::setIdentity(const QString &appName)
{
    QWSIdentifyCommand cmd;
    cmd.setId(appName, QWSDisplay::Data::clientLock ? QWSDisplay::Data::clientLock->id() : -1 );
    if (d->directServerConnection())
        qwsServer->d_func()->set_identity(&cmd);
    else
        d->sendCommand(cmd);
}

void QWSDisplay::nameRegion(int winId, const QString& n, const QString &c)
{
    QWSRegionNameCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.setName(n, c);
    if (d->directServerConnection())
        qwsServer->d_func()->name_region(&cmd);
    else
        d->sendCommand(cmd);
}

void QWSDisplay::requestRegion(int winId, const QString &surfaceKey,
                               const QByteArray &surfaceData,
                               const QRegion &region)
{
    if (d->directServerConnection()) {
        qwsServer->d_func()->request_region(winId, surfaceKey,
                                            surfaceData, region);
    } else {
        QWSRegionCommand cmd;
        cmd.setData(winId, surfaceKey, surfaceData, region);
        d->sendSynchronousCommand(cmd);
    }
}

void QWSDisplay::repaintRegion(int winId, bool opaque, QRegion r)
{
    if (d->directServerConnection()) {
        qwsServer->d_func()->repaint_region(winId, opaque, r);
    } else {
        QVector<QRect> ra = r.rects();

        /*
          for (int i = 0; i < ra.size(); i++) {
          QRect r(ra[i]);
          qDebug("rect: %d %d %d %d", r.x(), r.y(), r.right(), r.bottom());
          }
        */

        QWSRepaintRegionCommand cmd;
    /* XXX QWSRegionCommand is padded out in a compiler dependent way.
       Zeroed out to avoid valgrind reporting uninitialized memory usage.
       */
#ifdef QT_DEBUG
        memset(cmd.simpleDataPtr, 0, sizeof(cmd.simpleData)); //shut up Valgrind
#endif
        cmd.simpleData.windowid = winId;
        cmd.simpleData.opaque = opaque;
        cmd.simpleData.nrectangles = ra.count();
        cmd.setData(reinterpret_cast<const char *>(ra.constData()),
                    ra.count() * sizeof(QRect), false);

        d->sendSynchronousCommand(cmd);
    }
}


void QWSDisplay::moveRegion(int winId, int dx, int dy)
{
    QWSRegionMoveCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.dx = dx;
    cmd.simpleData.dy = dy;

    if (d->directServerConnection()) {
        qwsServer->d_func()->move_region(&cmd);
    } else {
        d->sendSynchronousCommand(cmd);
    }
//    d->offsetPendingExpose(winId, QPoint(cmd.simpleData.dx, cmd.simpleData.dy));
}

void QWSDisplay::destroyRegion(int winId)
{
    QWSRegionDestroyCommand cmd;
    cmd.simpleData.windowid = winId;
    if (d->directServerConnection()) {
        qwsServer->d_func()->destroy_region(&cmd);
    } else {
        d->sendCommand(cmd);
    }
}

#ifndef QT_NO_QWS_INPUTMETHODS

void QWSDisplay::sendIMUpdate(int type, int winId, int widgetid)
{
    QWSIMUpdateCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.widgetid = widgetid;

    cmd.simpleData.type = type;

      if (d->directServerConnection()) {
        qwsServer->d_func()->im_update(&cmd);
    } else {
        d->sendCommand(cmd);
    }
}

void QWSDisplay::sendIMResponse(int winId, int property, const QVariant &result)
{
    QWSIMResponseCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.property = property;

    cmd.setResult(result);

    if (d->directServerConnection()) {
        qwsServer->d_func()->im_response(&cmd);
    } else {
        d->sendCommand(cmd);
    }
}

void QWSDisplay::resetIM()
{
    sendIMUpdate(QWSInputMethod::Reset, -1, -1);
}

void QWSDisplay::sendIMMouseEvent(int index, bool isPress)
{
    QWSIMMouseCommand cmd;
    cmd.simpleData.index = index;
    cmd.simpleData.state = isPress ? QWSServer::MousePress : QWSServer::MouseRelease;
    if (d->directServerConnection()) {
        qwsServer->d_func()->send_im_mouse(&cmd);
    } else {
        d->sendCommand(cmd);
    }
}

#endif

int QWSDisplay::takeId()
{
    return d->takeId();
}

bool QWSDisplay::initLock(const QString &filename, bool create)
{
    if (!lock) {
        lock = new QLock(filename, 'd', create);

        if (!lock->isValid()) {
            delete lock;
            lock = 0;
            return false;
        }
    }

    return true;
}

void QWSDisplay::setSelectionOwner(int winId, const QTime &time)
{
    QWSSetSelectionOwnerCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.hour = time.hour();
    cmd.simpleData.minute = time.minute();
    cmd.simpleData.sec = time.second();
    cmd.simpleData.ms = time.msec();
    d->sendCommand(cmd);
}

void QWSDisplay::convertSelection(int winId, int selectionProperty, const QString &mimeTypes)
{
#ifdef QT_NO_QWS_PROPERTIES
    Q_UNUSED(mimeTypes);
#else
    // ### we need the atom/property thingy like in X here
    addProperty(winId, QT_QWS_PROPERTY_CONVERTSELECTION);
    setProperty(winId, QT_QWS_PROPERTY_CONVERTSELECTION,
                 int(QWSPropertyManager::PropReplace), mimeTypes.toLatin1());
#endif
    QWSConvertSelectionCommand cmd;
    cmd.simpleData.requestor = winId;
    cmd.simpleData.selection = selectionProperty;
    cmd.simpleData.mimeTypes = QT_QWS_PROPERTY_CONVERTSELECTION;
    d->sendCommand(cmd);
}

void QWSDisplay::defineCursor(int id, const QBitmap &curs, const QBitmap &mask,
                            int hotX, int hotY)
{
    QImage cursImg = curs.toImage();
    QImage maskImg = mask.toImage();

    QWSDefineCursorCommand cmd;
    cmd.simpleData.width = curs.width();
    cmd.simpleData.height = curs.height();
    cmd.simpleData.hotX = hotX;
    cmd.simpleData.hotY = hotY;
    cmd.simpleData.id = id;


    // must copy each scanline since there might be gaps between them
    const int height = curs.height();
    const int width = curs.width();
    const int dst_bpl = (width + 7) / 8;

    int dataLen = dst_bpl * height;
    uchar *data = new uchar[dataLen*2];
    uchar *dst = data;

    int src_bpl = cursImg.bytesPerLine();
    const uchar *cursSrc = cursImg.bits();
    for (int i = 0; i < height; ++i) {
        memcpy(dst, cursSrc + i*src_bpl, dst_bpl);
        dst += dst_bpl;
    }

    src_bpl = maskImg.bytesPerLine();
    const uchar *maskSrc = maskImg.bits();
    for (int i = 0; i < height; ++i) {
        memcpy(dst, maskSrc + i*src_bpl, dst_bpl);
        dst += dst_bpl;
    }

    cmd.setData(reinterpret_cast<char*>(data), dataLen*2);
    delete [] data;
    d->sendCommand(cmd);
}

void QWSDisplay::destroyCursor(int id)
{
    QWSDefineCursorCommand cmd;
    cmd.simpleData.width = 0;
    cmd.simpleData.height = 0;
    cmd.simpleData.hotX = 0;
    cmd.simpleData.hotY = 0;
    cmd.simpleData.id = id;
    cmd.setData(0, 0);

    d->sendCommand(cmd);
}

#ifndef QT_NO_SOUND
void QWSDisplay::playSoundFile(const QString& f)
{
    QWSPlaySoundCommand cmd;
    cmd.setFileName(f);
    d->sendCommand(cmd);
}
#endif

#ifndef QT_NO_COP
void QWSDisplay::registerChannel(const QString& channel)
{
    QWSQCopRegisterChannelCommand reg;
    reg.setChannel(channel);
    qt_fbdpy->d->sendCommand(reg);
}

void QWSDisplay::sendMessage(const QString &channel, const QString &msg,
                   const QByteArray &data)
{
    QWSQCopSendCommand com;
    com.setMessage(channel, msg, data);
    qt_fbdpy->d->sendCommand(com);
}

void QWSDisplay::flushCommands()
{
    qt_fbdpy->d->flushCommands();
}

/*
  caller deletes result
*/
QWSQCopMessageEvent* QWSDisplay::waitForQCopResponse()
{
    qt_fbdpy->d->waitForQCopResponse();
    QWSQCopMessageEvent *e = static_cast<QWSQCopMessageEvent*>(qt_fbdpy->d->dequeue());
    Q_ASSERT(e->type == QWSEvent::QCopMessage);
    return e;
}
#endif

void QWSDisplay::sendFontCommand(int type, const QByteArray &fontName)
{
    QWSFontCommand cmd;
    cmd.simpleData.type = type;
    cmd.setFontName(fontName);
    d->sendCommand(cmd);
}

void QWSDisplay::setWindowCaption(QWidget *w, const QString &c)
{
    if (w->isWindow()) {
        nameRegion(w->internalWinId(), w->objectName(), c);
        static_cast<QETWidget *>(w)->repaintDecoration(qApp->desktop()->rect(), true);
    }
}

void QWSDisplay::selectCursor(QWidget *w, unsigned int cursId)
{
    if (cursId != qt_last_cursor)
    {
        QWidget *top = w->window();
        qt_last_cursor = cursId;
        QWSSelectCursorCommand cmd;
        cmd.simpleData.windowid = top->internalWinId();
        cmd.simpleData.id = cursId;
        d->sendCommand(cmd);
        d->flush();
    }
}

void QWSDisplay::setCursorPosition(int x, int y)
{
    QWSPositionCursorCommand cmd;
    cmd.simpleData.newX = x;
    cmd.simpleData.newY = y;
    d->sendCommand(cmd);
    d->flush();
}

void QWSDisplay::grabMouse(QWidget *w, bool grab)
{
    QWidget *top = w->window();
    QWSGrabMouseCommand cmd;
#ifdef QT_DEBUG
    memset(cmd.simpleDataPtr, 0, sizeof(cmd.simpleData)); //shut up Valgrind
#endif
    cmd.simpleData.windowid = top->winId();
    cmd.simpleData.grab = grab;
    d->sendCommand(cmd);
    d->flush();
}

void QWSDisplay::grabKeyboard(QWidget *w, bool grab)
{
    QWidget *top = w->window();
    QWSGrabKeyboardCommand cmd;
#ifdef QT_DEBUG
    memset(cmd.simpleDataPtr, 0, sizeof(cmd.simpleData)); //shut up Valgrind
#endif
    cmd.simpleData.windowid = top->winId();
    cmd.simpleData.grab = grab;
    d->sendCommand(cmd);
    d->flush();
}

QList<QWSWindowInfo> QWSDisplay::windowList()
{
    QList<QWSWindowInfo> ret;
    if(d->directServerConnection()) {
        QList<QWSInternalWindowInfo*> * qin=QWSServer::windowList();
        for (int i = 0; i < qin->count(); ++i) {
            QWSInternalWindowInfo * qwi = qin->at(i);
            QWSWindowInfo tmp;
            tmp.winid = qwi->winid;
            tmp.clientid = qwi->clientid;
            tmp.name = QString(qwi->name);
            ret.append(tmp);
        }
        qDeleteAll(*qin);
        delete qin;
    }
    return ret;
}

int QWSDisplay::windowAt(const QPoint &p)
{
    //### currently only implemented for the server process
    int ret = 0;
    if(d->directServerConnection()) {
        QWSWindow *win = qwsServer->windowAt(p);
        if (win)
            return win->winId();
    }
    return ret;
}

void QWSDisplay::setRawMouseEventFilter(void (*filter)(QWSMouseEvent *))
{
    if (qt_fbdpy)
        qt_fbdpy->d->setMouseFilter(filter);
}

#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
#ifdef QT_QWS_TRANSFORMED
extern void qws_setScreenTransformation(int);
extern void qws_mapPixmaps(bool from);
#endif

void QWSDisplay::setTransformation(int t)
{
#ifdef QT_QWS_TRANSFORMED

    QApplicationPrivate *ap = QApplicationPrivate::instance();
    const QRect maxWindowRect = ap->maxWindowRect(qt_screen);
    bool isFullScreen = maxWindowRect == QRect(0, 0, qt_screen->width(), qt_screen->height());

    QPixmapCache::clear();
    QFontCache::instance->clear();
    qws_mapPixmaps(true);
    qws_setScreenTransformation(t);
    qws_mapPixmaps(false);

    if (qt_fbdpy->d_func()->directServerConnection()) {
        qwsServer->d_func()->resetEngine();
        qwsServer->d_func()->refresh();
    }

    QSize olds = qApp->desktop()->size();
    qApp->desktop()->resize(qt_screen->width(), qt_screen->height());
    // ## why post the resize event?
    qApp->postEvent(qApp->desktop(), new QResizeEvent(qApp->desktop()->size(), olds));
    emit QApplication::desktop()->resized(0);

    QWidgetList  list = QApplication::topLevelWidgets();
    for (int i = list.size()-1; i >= 0; --i) {
        QWidget *w = (QWidget*)list[i];

        if ((w->windowType() == Qt::Desktop)) {
            //nothing
        } else if (w->testAttribute(Qt::WA_WState_FullScreen)) {
            w->resize(qt_screen->width(), qt_screen->height());
        } else {
            QETWidget *etw = static_cast<QETWidget*>(w);
            etw->updateRegion();
            if (etw->isVisible()) {
                etw->repaintHierarchy(etw->geometry(), true);
                etw->repaintDecoration(qApp->desktop()->rect(), true);
            }
        }
    }


    // only update the mwr if it is full screen.
    if (isFullScreen) {
        QApplicationPriate *ap = QApplicationPrivate::instance();
        ap->setMaxWindowRect(qt_screen,
                             QRect(0,0, qt_screen->width(), qt_screen->height()));
    }

#endif
}
#endif

static bool qt_try_modal(QWidget *, QWSEvent *);

/*****************************************************************************
  qt_init() - initializes Qt/FB
 *****************************************************************************/

static void qt_set_qws_resources()

{
    if (QApplication::desktopSettingsAware())
        QApplicationPrivate::qws_apply_settings();

    if (appFont)
        QApplication::setFont(QFont(appFont));

    if (appBGCol || appBTNCol || appFGCol) {
        (void) QApplication::style();  // trigger creation of application style and system palettes
        QColor btn;
        QColor bg;
        QColor fg;
        if (appBGCol)
            bg = QColor(appBGCol);
        else
            bg = QApplicationPrivate::sys_pal->color(QPalette::Window);
        if (appFGCol)
            fg = QColor(appFGCol);
        else
            fg = QApplicationPrivate::sys_pal->color(QPalette::WindowText);
        if (appBTNCol)
            btn = QColor(appBTNCol);
        else
            btn = QApplicationPrivate::sys_pal->color(QPalette::Button);

        int h,s,v;
        fg.getHsv(&h,&s,&v);
        QColor base = Qt::white;
        bool bright_mode = false;
        if (v >= 255-50) {
            base = btn.dark(150);
            bright_mode = true;
        }

        QPalette pal(fg, btn, btn.light(), btn.dark(), btn.dark(150), fg, Qt::white, base, bg);
        if (bright_mode) {
            pal.setColor(QPalette::HighlightedText, base);
            pal.setColor(QPalette::Highlight, Qt::white);
        } else {
            pal.setColor(QPalette::HighlightedText, Qt::white);
            pal.setColor(QPalette::Highlight, Qt::darkBlue);
        }
        QColor disabled((fg.red()   + btn.red())  / 2,
                        (fg.green() + btn.green())/ 2,
                        (fg.blue()  + btn.blue()) / 2);
        pal.setColorGroup(QPalette::Disabled, disabled, btn, btn.light(125),
                          btn.dark(), btn.dark(150), disabled, Qt::white, Qt::white, bg);
        if (bright_mode) {
            pal.setColor(QPalette::Disabled, QPalette::HighlightedText, base);
            pal.setColor(QPalette::Disabled, QPalette::Highlight, Qt::white);
        } else {
            pal.setColor(QPalette::Disabled, QPalette::HighlightedText, Qt::white);
            pal.setColor(QPalette::Disabled, QPalette::Highlight, Qt::darkBlue);
        }
        QApplicationPrivate::setSystemPalette(pal);

    }
}

void QApplicationPrivate::initializeWidgetPaletteHash()
{
}

/*! \internal
    apply the settings to the application
*/
bool QApplicationPrivate::qws_apply_settings()
{
#ifndef QT_NO_SETTINGS
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));

    QStringList strlist;
    int i;
    QPalette pal(Qt::black);
    int groupCount = 0;
    strlist = settings.value(QLatin1String("Palette/active")).toStringList();
    if (strlist.count() == QPalette::NColorRoles) {
        ++groupCount;
        for (i = 0; i < QPalette::NColorRoles; i++)
            pal.setColor(QPalette::Active, (QPalette::ColorRole) i,
                         QColor(strlist[i]));
    }
    strlist = settings.value(QLatin1String("Palette/inactive")).toStringList();
    if (strlist.count() == QPalette::NColorRoles) {
        ++groupCount;
        for (i = 0; i < QPalette::NColorRoles; i++)
            pal.setColor(QPalette::Inactive, (QPalette::ColorRole) i,
                         QColor(strlist[i]));
    }
    strlist = settings.value(QLatin1String("Palette/disabled")).toStringList();
    if (strlist.count() == QPalette::NColorRoles) {
        ++groupCount;
        for (i = 0; i < QPalette::NColorRoles; i++)
            pal.setColor(QPalette::Disabled, (QPalette::ColorRole) i,
                         QColor(strlist[i]));
    }


    if (groupCount == QPalette::NColorGroups)
        QApplicationPrivate::setSystemPalette(pal);

    if (!qt_app_has_font) {
        QFont font(QApplication::font());
        QString str = settings.value(QLatin1String("font")).toString();
        if (!str.isEmpty()) {
            font.fromString(str);
            if (font != QApplication::font())
                QApplication::setFont(font);
        }
    }

    // read library (ie. plugin) path list
    QString libpathkey =
        QString::fromLatin1("%1.%2/libraryPath")
        .arg(QT_VERSION >> 16)
        .arg((QT_VERSION & 0xff00) >> 8);
    QStringList pathlist = settings.value(libpathkey).toString().split(QLatin1Char(':'));
#ifndef QT_NO_LIBRARY
    if (! pathlist.isEmpty()) {
        QStringList::ConstIterator it = pathlist.constBegin();
        while (it != pathlist.constEnd())
            QApplication::addLibraryPath(*it++);
    }
#endif

    // read new QStyle
    QString stylename = settings.value(QLatin1String("style")).toString();
    if (QCoreApplication::startingUp()) {
        if (!stylename.isEmpty() && !QApplicationPrivate::styleOverride)
            QApplicationPrivate::styleOverride = new QString(stylename);
    } else {
        QApplication::setStyle(stylename);
    }

    int num =
        settings.value(QLatin1String("doubleClickInterval"),
                       QApplication::doubleClickInterval()).toInt();
    QApplication::setDoubleClickInterval(num);

    num =
        settings.value(QLatin1String("cursorFlashTime"),
                       QApplication::cursorFlashTime()).toInt();
    QApplication::setCursorFlashTime(num);

#ifndef QT_NO_WHEELEVENT
    num =
        settings.value(QLatin1String("wheelScrollLines"),
                       QApplication::wheelScrollLines()).toInt();
    QApplication::setWheelScrollLines(num);
#endif

    QString colorspec = settings.value(QLatin1String("colorSpec"),
                                       QVariant(QLatin1String("default"))).toString();
    if (colorspec == QLatin1String("normal"))
        QApplication::setColorSpec(QApplication::NormalColor);
    else if (colorspec == QLatin1String("custom"))
        QApplication::setColorSpec(QApplication::CustomColor);
    else if (colorspec == QLatin1String("many"))
        QApplication::setColorSpec(QApplication::ManyColor);
    else if (colorspec != QLatin1String("default"))
        colorspec = QLatin1String("default");

#ifndef QT_NO_TEXTCODEC
    QString defaultcodec = settings.value(QLatin1String("defaultCodec"),
                                          QVariant(QLatin1String("none"))).toString();
    if (defaultcodec != QLatin1String("none")) {
        QTextCodec *codec = QTextCodec::codecForName(defaultcodec.toLatin1());
        if (codec)
            QTextCodec::setCodecForTr(codec);
    }
#endif

    int w = settings.value(QLatin1String("globalStrut/width")).toInt();
    int h = settings.value(QLatin1String("globalStrut/height")).toInt();
    QSize strut(w, h);
    if (strut.isValid())
        QApplication::setGlobalStrut(strut);

    QStringList effects = settings.value(QLatin1String("GUIEffects")).toStringList();
    QApplication::setEffectEnabled(Qt::UI_General,
                                   effects.contains(QLatin1String("general")));
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu,
                                   effects.contains(QLatin1String("animatemenu")));
    QApplication::setEffectEnabled(Qt::UI_FadeMenu,
                                   effects.contains(QLatin1String("fademenu")));
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo,
                                   effects.contains(QLatin1String("animatecombo")));
    QApplication::setEffectEnabled(Qt::UI_AnimateTooltip,
                                   effects.contains(QLatin1String("animatetooltip")));
    QApplication::setEffectEnabled(Qt::UI_FadeTooltip,
                                   effects.contains(QLatin1String("fadetooltip")));
    QApplication::setEffectEnabled(Qt::UI_AnimateToolBox,
                                   effects.contains(QLatin1String("animatetoolbox")));

    settings.beginGroup(QLatin1String("Font Substitutions"));
    QStringList fontsubs = settings.childKeys();
    if (!fontsubs.isEmpty()) {
        QStringList::Iterator it = fontsubs.begin();
        for (; it != fontsubs.end(); ++it) {
            QString fam = *it;
            QStringList subs = settings.value(fam).toStringList();
            QFont::insertSubstitutions(fam, subs);
        }
    }
    settings.endGroup();

    settings.endGroup(); // Qt

    return true;
#else
    return false;
#endif // QT_NO_SETTINGS
}



static void init_display()
{
    if (qt_fbdpy) return; // workaround server==client case

    // Connect to FB server
    qt_fbdpy = new QWSDisplay();

    // Get display parameters
    // Set paintdevice parameters
    // XXX initial info sent from server
    // Misc. initialization

    QColormap::initialize();
    QFont::initialize();
#ifndef QT_NO_CURSOR
    QCursorData::initialize();
#endif

    qApp->setObjectName(appName);

    if (!QApplicationPrivate::app_font) {
        QFont f;
        f = QFont(QLatin1String("helvetica"), 10);
        QApplication::setFont(f);
    }
    qt_set_qws_resources();
}

void qt_init_display()
{
    qt_is_gui_used = true;
    qws_single_process = true;
    init_display();
}

static bool read_bool_env_var(const char *var, bool defaultvalue)
{
    // returns true if env variable is set to non-zero
    // returns false if env var is set to zero
    // returns defaultvalue if env var not set
    char *x = ::getenv(var);
    return (x && *x) ? (strcmp(x,"0") != 0) : defaultvalue;
}

void qt_init(QApplicationPrivate *priv, int type)
{
    if (type == QApplication::GuiServer)
        qt_is_gui_used = false; //we'll turn it on in a second
    qws_sw_cursor = read_bool_env_var("QWS_SW_CURSOR",qws_sw_cursor);
    qws_screen_is_interlaced = read_bool_env_var("QWS_INTERLACE",false);

    const char *display = ::getenv("QWS_DISPLAY");
    if (display)
        qws_display_spec = display; // since we setenv later!

    //qws_savefonts = qgetenv("QWS_SAVEFONTS") != 0;
    //qws_shared_memory = qgetenv("QWS_NOSHARED") == 0;

    int flags = 0;
    char *p;
    int argc = priv->argc;
    char **argv = priv->argv;
    int j;

    // Set application name

    if (argv && *argv) { //apparently, we allow people to pass 0 on the other platforms
        p = strrchr(argv[0], '/');
        appName = p ? p + 1 : argv[0];
    }

    // Get command line params

    j = argc ? 1 : 0;
    QString decoration;
    for (int i=1; i<argc; i++) {
        if (argv[i] && *argv[i] != '-') {
            argv[j++] = argv[i];
            continue;
        }
        QByteArray arg = argv[i];
        if (arg == "-fn" || arg == "-font") {
            if (++i < argc)
                appFont = argv[i];
        } else if (arg == "-bg" || arg == "-background") {
            if (++i < argc)
                appBGCol = argv[i];
        } else if (arg == "-btn" || arg == "-button") {
            if (++i < argc)
                appBTNCol = argv[i];
        } else if (arg == "-fg" || arg == "-foreground") {
            if (++i < argc)
                appFGCol = argv[i];
        } else if (arg == "-name") {
            if (++i < argc)
                appName = argv[i];
        } else if (arg == "-title") {
            if (++i < argc)
                mwTitle = argv[i];
        } else if (arg == "-geometry") {
            if (++i < argc)
                mwGeometry = argv[i];
        } else if (arg == "-shared") {
            qws_shared_memory = true;
        } else if (arg == "-noshared") {
            qws_shared_memory = false;
        } else if (arg == "-savefonts") {
            qws_savefonts = true;
        } else if (arg == "-nosavefonts") {
            qws_savefonts = false;
        } else if (arg == "-swcursor") {
            qws_sw_cursor = true;
        } else if (arg == "-noswcursor") {
            qws_sw_cursor = false;
        } else if (arg == "-keyboard") {
            flags &= ~QWSServer::DisableKeyboard;
        } else if (arg == "-nokeyboard") {
            flags |= QWSServer::DisableKeyboard;
        } else if (arg == "-mouse") {
            flags &= ~QWSServer::DisableMouse;
        } else if (arg == "-nomouse") {
            flags |= QWSServer::DisableMouse;
        } else if (arg == "-qws") {
            type = QApplication::GuiServer;
        } else if (arg == "-interlaced") {
            qws_screen_is_interlaced = true;
        } else if (arg == "-display") {
            if (++i < argc)
                qws_display_spec = argv[i];
        } else if (arg == "-decoration") {
            if (++i < argc)
                decoration = argv[i];
        } else {
            argv[j++] = argv[i];
        }
    }
    if(j < priv->argc) {
        priv->argv[j] = 0;
        priv->argc = j;
    }

    mouseInWidget = new QPointer<QWidget>;

    const QString disp(qws_display_spec);
    QRegExp regexp(":(\\d+)$");
    if (regexp.lastIndexIn(disp) != -1) {
        const QString capture = regexp.cap(1);
        bool ok = false;
        int id = capture.toInt(&ok);
        if (ok)
            qws_display_id = id;
    }

    if (type == QApplication::GuiServer) {
        qt_appType = QApplication::Type(type);
        qws_single_process = true;
        QWSServer::startup(flags);
        setenv("QWS_DISPLAY", qws_display_spec.constData(), 0);
    }

    if(qt_is_gui_used) {
        init_display();
#ifndef QT_NO_QWS_MANAGER
        if (decoration.isEmpty()) {
#ifndef QT_NO_QWS_DECORATION_STYLED
            decoration = QString::fromLatin1("styled");
#else
            decoration = QString::fromLatin1("default");
#endif
        }
        qws_decoration = QApplication::qwsSetDecoration(decoration);
#ifndef QT_NO_QWS_DECORATION_DEFAULT
        if (!qws_decoration)
            QApplication::qwsSetDecoration(new QDecorationDefault);
#endif
#endif // QT_NO_QWS_MANAGER
#ifndef QT_NO_QWS_INPUTMETHODS
        qApp->setInputContext(new QWSInputContext);
#endif
    }

/*### convert interlace style
    if (qws_screen_is_interlaced)
        QApplication::setStyle(new QInterlaceStyle);
*/
}

bool qt_sendSpontaneousEvent(QObject *obj, QEvent *event)
{
    return QCoreApplication::sendSpontaneousEvent(obj, event);
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    QPixmapCache::clear();
#ifndef QT_NO_CURSOR
    QCursorData::cleanup();
#endif
    QFont::cleanup();
    QColormap::cleanup();

    if (qws_single_process) {
        QWSServer::closedown();
    }

    qDeleteAll(outgoing);
    outgoing.clear();
    qDeleteAll(incoming);
    incoming.clear();

    if (qt_is_gui_used) {
        delete qt_fbdpy;
    }
    qt_fbdpy = 0;

#ifndef QT_NO_QWS_MANAGER
    delete qws_decoration;
    qws_decoration = 0;
#endif

    delete mouseInWidget;
    mouseInWidget = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

QString QApplicationPrivate::appName() const // get application name
{
    return ::appName;
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
 *   were actually found in the string. For each value found,
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
        return Result;
    else
        return -Result;
}

static int parseGeometry(const char* string,
                          int* x, int* y, int* width, int* height)
{
        int mask = NoValue;
        register char *strind;
        unsigned int tempWidth=0, tempHeight=0;
        int tempX=0, tempY=0;
        char *nextCharacter;

        if (!string || (*string == '\0')) return mask;
        if (*string == '=')
                string++;  /* ignore possible '=' at beg of geometry spec */

        strind = const_cast<char *>(string);
        if (*strind != '+' && *strind != '-' && *strind != 'x') {
                tempWidth = ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter)
                    return 0;
                strind = nextCharacter;
                mask |= WidthValue;
        }

        if (*strind == 'x' || *strind == 'X') {
                strind++;
                tempHeight = ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter)
                    return 0;
                strind = nextCharacter;
                mask |= HeightValue;
        }

        if ((*strind == '+') || (*strind == '-')) {
                if (*strind == '-') {
                        strind++;
                        tempX = -ReadInteger(strind, &nextCharacter);
                        if (strind == nextCharacter)
                            return 0;
                        strind = nextCharacter;
                        mask |= XNegative;

                }
                else
                {        strind++;
                        tempX = ReadInteger(strind, &nextCharacter);
                        if (strind == nextCharacter)
                            return 0;
                        strind = nextCharacter;
                }
                mask |= XValue;
                if ((*strind == '+') || (*strind == '-')) {
                        if (*strind == '-') {
                                strind++;
                                tempY = -ReadInteger(strind, &nextCharacter);
                                if (strind == nextCharacter)
                                    return 0;
                                strind = nextCharacter;
                                mask |= YNegative;

                        }
                        else
                        {
                                strind++;
                                tempY = ReadInteger(strind, &nextCharacter);
                                if (strind == nextCharacter)
                                    return 0;
                                strind = nextCharacter;
                        }
                        mask |= YValue;
                }
        }

        /* If strind isn't at the end of the string then it's an invalid
                geometry specification. */

        if (*strind != '\0') return 0;

        if (mask & XValue)
            *x = tempX;
        if (mask & YValue)
            *y = tempY;
        if (mask & WidthValue)
            *width = tempWidth;
        if (mask & HeightValue)
            *height = tempHeight;
        return mask;
}

#ifdef QT3_SUPPORT
void QApplication::setMainWidget(QWidget *mainWidget)
{
    QApplicationPrivate::main_widget = mainWidget;
    if (QApplicationPrivate::main_widget) // give WM command line
        QApplicationPrivate::applyQWSSpecificCommandLineArguments(QApplicationPrivate::main_widget);
}
#endif

void QApplicationPrivate::applyQWSSpecificCommandLineArguments(QWidget *main_widget)
{
    static bool beenHereDoneThat = false;
    if (beenHereDoneThat)
        return;
    beenHereDoneThat = true;
    if (qApp->windowIcon().isNull() && main_widget->testAttribute(Qt::WA_SetWindowIcon))
        qApp->setWindowIcon(main_widget->windowIcon());
    if (mwTitle) //  && main_widget->windowTitle().isEmpty())
        main_widget->setWindowTitle(mwTitle);
    if (mwGeometry) { // parse geometry
        int x = 0;
        int y = 0;
        int w, h;
        int m = parseGeometry(mwGeometry, &x, &y, &w, &h);
        QSize minSize = main_widget->minimumSize();
        QSize maxSize = main_widget->maximumSize();
        if ((m & XValue) == 0)
            x = main_widget->geometry().x();
        if ((m & YValue) == 0)
            y = main_widget->geometry().y();
        if ((m & WidthValue) == 0)
            w = main_widget->width();
        if ((m & HeightValue) == 0)
            h = main_widget->height();
        w = qMin(w,maxSize.width());
        h = qMin(h,maxSize.height());
        w = qMax(w,minSize.width());
        h = qMax(h,minSize.height());
        if ((m & XNegative)) {
            x = qApp->desktop()->width()  + x - w;
            x -= (main_widget->frameGeometry().width() - main_widget->width()) / 2;
        } else {
            x += (main_widget->geometry().x() - main_widget->x());
        }
        if ((m & YNegative)) {
            y = qApp->desktop()->height() + y - h;
        } else {
            y += (main_widget->geometry().y() - main_widget->y());
        }

        main_widget->setGeometry(x, y, w, h);
    }
}

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/
#ifndef QT_NO_CURSOR
void QApplication::setOverrideCursor(const QCursor &cursor)
{
    qApp->d_func()->cursor_list.prepend(cursor);

    QWidget *w = QWidget::mouseGrabber();
    if (!w && qt_last_x)
        w = topLevelAt(*qt_last_x, *qt_last_y);
    if (!w)
        w = desktop();
    QPaintDevice::qwsDisplay()->selectCursor(w, qApp->d_func()->cursor_list.first().handle());
}

void QApplication::restoreOverrideCursor()
{
    if (qApp->d_func()->cursor_list.isEmpty())
        return;
    qApp->d_func()->cursor_list.removeFirst();

    QWidget *w = QWidget::mouseGrabber();
    if (!w && qt_last_x)
        w = topLevelAt(*qt_last_x, *qt_last_y);
    if (!w)
        w = desktop();

    int cursor_handle = Qt::ArrowCursor;
    if (qApp->d_func()->cursor_list.isEmpty()) {
        qws_overrideCursor = false;
        QWidget *upw = QApplication::widgetAt(*qt_last_x, *qt_last_y);
        if (upw)
            cursor_handle = upw->cursor().handle();
    } else {
        cursor_handle = qApp->d_func()->cursor_list.first().handle();
    }
    QPaintDevice::qwsDisplay()->selectCursor(w, cursor_handle);
}
#endif// QT_NO_CURSOR



/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

/*!
    \internal
*/
QWidget *QApplicationPrivate::findWidget(const QObjectList& list,
                                   const QPoint &pos, bool rec)
{
    QWidget *w;

    for (int i = list.size()-1; i >= 0; --i) {
        if (list.at(i)->isWidgetType()) {
          w = static_cast<QWidget*>(list.at(i));
            if (w->isVisible() && !w->testAttribute(Qt::WA_TransparentForMouseEvents) &&  w->geometry().contains(pos)
                && (!w->d_func()->extra || w->d_func()->extra->mask.isEmpty() ||  w->d_func()->extra->mask.contains(pos - w->geometry().topLeft()) )) {
                if (!rec)
                    return w;
                QWidget *c = w->childAt(w->mapFromParent(pos));
                return c ? c : w;
            }
        }
    }
    return 0;
}


QWidget *QApplication::topLevelAt(const QPoint &pos)
{
    //### QWSDisplay::windowAt() is currently only implemented in the server process
    int winId = QPaintDevice::qwsDisplay()->windowAt(pos);
    if (winId !=0)
        return QWidget::find(winId);

#if 1
    // fallback implementation for client processes
//### This is slightly wrong: we have no guarantee that the list is in
//### stacking order, so if the topmost window is transparent, we may
//### return the wrong widget

    QWidgetList list = topLevelWidgets();
    for (int i = list.size()-1; i >= 0; --i) {
        QWidget *w = list[i];
        if (w != QApplication::desktop() &&
             w->isVisible() && w->d_func()->localAllocatedRegion().contains(w->mapFromParent(pos))
            )
            return w;
    }
#endif
    return 0;
}

void QApplication::beep()
{
}

/*!
    \internal
*/
int QApplication::qwsProcessEvent(QWSEvent* event)
{
    Q_D(QApplication);
    int oldstate = -1;
    bool isMove = false;
    if (event->type == QWSEvent::Mouse) {
        QWSMouseEvent::SimpleData &mouse = event->asMouse()->simpleData;
        isMove = mouse_x_root != mouse.x_root || mouse_y_root != mouse.y_root;
        oldstate = mouse_state;
        mouse_x_root = mouse.x_root;
        mouse_y_root = mouse.y_root;
        mouse_state = mouse.state;
    }

    long unused;
    if (filterEvent(event, &unused))                  // send through app filter
        return 1;

    if (qwsEventFilter(event))                        // send through app filter
        return 1;


#ifndef QT_NO_QWS_PROPERTIES
    if (event->type == QWSEvent::PropertyNotify) {
        QWSPropertyNotifyEvent *e = static_cast<QWSPropertyNotifyEvent*>(event);
        if (e->simpleData.property == 424242) {       // Clipboard
#ifndef QT_NO_CLIPBOARD
            if (qt_clipboard) {
                QClipboardEvent e(reinterpret_cast<QEventPrivate*>(event));
                QApplication::sendEvent(qt_clipboard, &e);
            }
#endif
        }
    } else if (event->type == QWSEvent::PropertyReply) {
        QWSPropertyReplyEvent *e = static_cast<QWSPropertyReplyEvent*>(event);
        int len = e->simpleData.len;
        char *data;
        if (len <= 0) {
            data = 0;
        } else {
            data = new char[len];
            memcpy(data, e->data, len) ;
        }
        QPaintDevice::qwsDisplay()->getPropertyLen = len;
        QPaintDevice::qwsDisplay()->getPropertyData = data;
    }
#endif //QT_NO_QWS_PROPERTIES
#ifndef QT_NO_COP
    else if (event->type == QWSEvent::QCopMessage) {
        QWSQCopMessageEvent *e = static_cast<QWSQCopMessageEvent*>(event);
        QCopChannel::sendLocally(e->channel, e->message, e->data);
        return 0;
    }
#endif
#if !defined(QT_NO_QWS_QPF2)
    else if (event->type == QWSEvent::Font) {
        QWSFontEvent *e = static_cast<QWSFontEvent *>(event);
        if (e->simpleData.type == QWSFontEvent::FontRemoved
            && QFontCache::instance) {
            QFontCache::instance->removeEngineForFont(e->fontName);
        }
    }
#endif

    QPointer<QETWidget> widget = static_cast<QETWidget*>(QWidget::find(WId(event->window())));

#ifndef QT_NO_DIRECTPAINTER
    if (!widget && d->directPainters) {
        QDirectPainter *dp = d->directPainters->value(WId(event->window()));
        if (dp == 0) {
        } else if (event->type == QWSEvent::Region) {
            QWSRegionEvent *e = static_cast<QWSRegionEvent*>(event);
            QRegion reg;
            reg.setRects(e->rectangles, e->simpleData.nrectangles);
            qt_directpainter_region(dp, reg, e->simpleData.type);
            return 1;
#ifndef QT_NO_QWSEMBEDWIDGET
        } else if (event->type == QWSEvent::Embed) {
            QWSEmbedEvent *e = static_cast<QWSEmbedEvent*>(event);
            qt_directpainter_embedevent(dp, e);
            return 1;
 #endif // QT_NO_QWSEMBEDWIDGET
        }
    }
#endif // QT_NO_DIRECTPAINTER

#ifndef QT_NO_QWS_MANAGER
    if (d->last_manager && event->type == QWSEvent::Mouse) {
        QPoint pos(event->asMouse()->simpleData.x_root, event->asMouse()->simpleData.y_root);
        if (!d->last_manager->cachedRegion().contains(pos)) {
            // MouseEvent not yet delivered, so QCursor::pos() is not yet updated, sending 2 x pos
            QMouseEvent outside(QEvent::MouseMove, pos, pos, Qt::NoButton, 0, 0);
            QApplication::sendSpontaneousEvent(d->last_manager, &outside);
            d->last_manager = 0;
        }
    }
#endif // QT_NO_QWS_MANAGER

    QETWidget *keywidget=0;
    bool grabbed=false;
    if (event->type==QWSEvent::Key || event->type == QWSEvent::IMEvent || event->type == QWSEvent::IMQuery) {
        keywidget = static_cast<QETWidget*>(QWidget::keyboardGrabber());
        if (keywidget) {
            grabbed = true;
        } else {
            if (QWidget *popup = QApplication::activePopupWidget()) {
                if (popup->focusWidget())
                    keywidget = static_cast<QETWidget*>(popup->focusWidget());
                else
                    keywidget = static_cast<QETWidget*>(popup);
            } else if (QApplicationPrivate::focus_widget && QApplicationPrivate::focus_widget->isVisible())
                keywidget = static_cast<QETWidget*>(QApplicationPrivate::focus_widget);
            else if (widget)
                keywidget = static_cast<QETWidget*>(widget->window());
        }
    } else if (event->type==QWSEvent::MaxWindowRect) {
        QRect r = static_cast<QWSMaxWindowRectEvent*>(event)->simpleData.rect;
        setMaxWindowRect(r);
        return 0;
    } else if (widget && event->type==QWSEvent::Mouse) {
        // The mouse event is to one of my top-level widgets
        // which one?
        const int btnMask = Qt::LeftButton | Qt::RightButton | Qt::MidButton;
        QPoint p(event->asMouse()->simpleData.x_root,
                 event->asMouse()->simpleData.y_root);
        int mouseButtonState = event->asMouse()->simpleData.state & btnMask;
        static int btnstate = 0;

        QETWidget *w = static_cast<QETWidget*>(QWidget::mouseGrabber());
        if (w && !mouseButtonState && qt_pressGrab == w)
            qt_pressGrab = 0;
#ifndef QT_NO_QWS_MANAGER
        if (!w)
            w = static_cast<QETWidget*>(QWSManager::grabbedMouse());
#endif
        if (w) {
            // Our mouse is grabbed - send it.
            widget = w;
            btnstate = mouseButtonState;
        } else {
            static QWidget *gw = 0;
            // Three jobs to do here:
            // 1. find the child widget this event belongs to.
            // 2. make sure the cursor is correct.
            // 3. handle implicit mouse grab due to button press.
            w = widget; // w is the widget the cursor is in.

            //### ??? alloc_region
            //#### why should we get events outside alloc_region ????
            if (1 /*widget->data->alloc_region.contains(dp) */) {
                // Find the child widget that the cursor is in.
                w = static_cast<QETWidget*>(widget->childAt(widget->mapFromParent(p)));
                if (!w)
                    w = widget;
#ifndef QT_NO_CURSOR
                // Update Cursor.
                if (!gw || gw != w || qt_last_cursor == 0xffffffff) {
                    QCursor *curs = 0;
                    if (!qApp->d_func()->cursor_list.isEmpty())
                        curs = &qApp->d_func()->cursor_list.first();
                    else if (w->d_func()->extraData())
                        curs = w->d_func()->extraData()->curs;
                    QWidget *pw = w;
                    // If this widget has no cursor set, try parent.
                    while (!curs) {
                        pw = pw->parentWidget();
                        if (!pw)
                            break;
                        if (pw->d_func()->extraData())
                            curs = pw->d_func()->extraData()->curs;
                    }
                    if (!qws_overrideCursor) {
                        if (curs)
                            QPaintDevice::qwsDisplay()->selectCursor(widget, curs->handle());
                        else
                            QPaintDevice::qwsDisplay()->selectCursor(widget, Qt::ArrowCursor);
                    }
                }
#endif
                gw = w;
            } else {
                // This event is not for any of our widgets
                gw = 0;
            }
            if (mouseButtonState && !btnstate) {
                // The server has grabbed the mouse for us.
                // Remember which of my widgets has it.
                qt_pressGrab = w;
                if (!widget->isActiveWindow() &&
                    (!app_do_modal || QApplication::activeModalWidget() == widget) &&
                    !((widget->windowFlags() & Qt::FramelessWindowHint) || (widget->windowType() == Qt::Tool))) {
                    widget->activateWindow();
                    if (widget->raiseOnClick())
                        widget->raise();
                }
            }
            btnstate = mouseButtonState;
            widget = w;
        }
    }

    if (!widget) {                                // don't know this window
        if (!QWidget::mouseGrabber()
#ifndef QT_NO_QWS_MANAGER
            && !QWSManager::grabbedMouse()
#endif
            ) {
            qt_last_cursor = 0xffffffff; // cursor can be changed by another application
        }

        QWidget* popup = QApplication::activePopupWidget();
        if (popup) {

            /*
              That is more than suboptimal. The real solution should
              do some keyevent and buttonevent translation, so that
              the popup still continues to work as the user expects.
              Unfortunately this translation is currently only
              possible with a known widget. I'll change that soon
              (Matthias).
            */

            // Danger - make sure we don't lock the server
            switch (event->type) {
            case QWSEvent::Mouse:
            case QWSEvent::Key:
                do {
                    popup->close();
                } while ((popup = qApp->activePopupWidget()));
                return 1;
            }
        }
        if (event->type == QWSEvent::Mouse && *mouseInWidget) {
            QApplicationPrivate::dispatchEnterLeave(0, *mouseInWidget);
            (*mouseInWidget) = 0;
        }
        return -1;
    }

    if (app_do_modal)                                // modal event handling
        if (!qt_try_modal(widget, event)) {
            return 1;
        }

    if (widget->qwsEvent(event))                // send through widget filter
        return 1;
    switch (event->type) {

    case QWSEvent::Mouse: {                        // mouse event
        QWSMouseEvent *me = event->asMouse();
        QWSMouseEvent::SimpleData &mouse = me->simpleData;

        //  Translate a QWS event into separate move
        // and press/release events
        // Beware of reentrancy: we can enter a modal state
        // inside translateMouseEvent

        if (isMove) {
            QWSMouseEvent move = *me;
            move.simpleData.state = oldstate;
            widget->translateMouseEvent(&move, oldstate);
        }
        if ((mouse.state&Qt::MouseButtonMask) != (oldstate&Qt::MouseButtonMask)) {
            widget->translateMouseEvent(me, oldstate);
        }

        if (mouse.delta != 0)
            widget->translateWheelEvent(me);

        if (qt_button_down && (mouse_state & Qt::MouseButtonMask) == 0)
            qt_button_down = 0;

        break;
    }
    case QWSEvent::Key:                                // keyboard event
        if (keywidget) // should always exist
            keywidget->translateKeyEvent(static_cast<QWSKeyEvent*>(event), grabbed);
        break;

#ifndef QT_NO_QWS_INPUTMETHODS
    case QWSEvent::IMEvent:
        if (keywidget) // should always exist
            QWSInputContext::translateIMEvent(keywidget, static_cast<QWSIMEvent*>(event));
        break;

    case QWSEvent::IMQuery:
        if (keywidget) // should always exist
            QWSInputContext::translateIMQueryEvent(keywidget, static_cast<QWSIMQueryEvent*>(event));
        break;

    case QWSEvent::IMInit:
        QWSInputContext::translateIMInitEvent(static_cast<QWSIMInitEvent*>(event));
        break;
#endif
    case QWSEvent::Region:
        widget->translateRegionEvent(static_cast<QWSRegionEvent*>(event));
        break;
    case QWSEvent::Focus:
        if ((static_cast<QWSFocusEvent*>(event))->simpleData.get_focus) {
            if (widget == static_cast<QWidget *>(desktop()))
                return true; // not interesting
            if (activeWindow() != widget) {
                setActiveWindow(widget);
                if (QApplicationPrivate::active_window)
                    static_cast<QETWidget *>(QApplicationPrivate::active_window)->repaintDecoration(desktop()->rect(), false);
                if (widget && !d->inPopupMode()) {
                    QWidget *w = widget->focusWidget();
                    while (w && w->focusProxy())
                        w = w->focusProxy();
                    if (w && (w->focusPolicy() != Qt::NoFocus))
                        w->setFocus();
                    else
                        widget->QWidget::focusNextPrevChild(true);
                    if (!QApplicationPrivate::focus_widget) {
                        if (widget->focusWidget())
                            widget->focusWidget()->setFocus();
                        else
                            widget->window()->setFocus();
                    }
                }
            }
        } else {        // lost focus
            if (widget == static_cast<QWidget *>(desktop()))
                return true; // not interesting
            if (QApplicationPrivate::focus_widget) {
                QETWidget *old = static_cast<QETWidget *>(QApplicationPrivate::active_window);
                setActiveWindow(0);
                qt_last_cursor = 0xffffffff;
                //QApplicationPrivate::active_window = 0;
                if (old)
                    old->repaintDecoration(desktop()->rect(), false);
                /* activateWindow() sends focus events
                   QApplication::setFocusWidget(0);
                */
            }
        }
        break;

    case QWSEvent::WindowOperation:
        if (static_cast<QWidget *>(widget) == desktop())
            return true;
        switch ((static_cast<QWSWindowOperationEvent *>(event))->simpleData.op) {
        case QWSWindowOperationEvent::Show:
            widget->show();
            break;
        case QWSWindowOperationEvent::Hide:
            widget->hide();
            break;
        case QWSWindowOperationEvent::ShowMaximized:
            widget->showMaximized();
            break;
        case QWSWindowOperationEvent::ShowMinimized:
            widget->showMinimized();
            break;
        case QWSWindowOperationEvent::ShowNormal:
            widget->showNormal();
            break;
        case QWSWindowOperationEvent::Close:
            widget->d_func()->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
            break;
        }
        break;
#ifndef QT_NO_QWSEMBEDWIDGET
    case QWSEvent::Embed:
        widget->translateEmbedEvent(static_cast<QWSEmbedEvent*>(event));
        break;
#endif
    default:
        break;
    }

    return 0;
}

/*!
    \fn bool QApplication::qwsEventFilter(QWSEvent *event)

    This virtual function is only implemented under Qtopia Core.

    If you create an application that inherits QApplication and
    reimplement this function, you get direct access to all QWS (Q
    Window System) events that the are received from the QWS master
    process. The events are passed in the \a event parameter.

    Return true if you want to stop the event from being processed.
    Return false for normal event dispatching. The default
    implementation returns false.
*/
bool QApplication::qwsEventFilter(QWSEvent *)
{
    return false;
}

/*!
    Set Qtopia Core custom color table.

    Qtopia Core on 8-bpp displays allocates a standard 216 color cube.
    The remaining 40 colors may be used by setting a custom color
    table in the QWS master process before any clients connect.

    \a colorTable is an array of up to 40 custom colors. \a start is
    the starting index (0-39) and \a numColors is the number of colors
    to be set (1-40).

    This method is non-portable. It is available \e only in
    Qtopia Core.
*/
void QApplication::qwsSetCustomColors(QRgb *colorTable, int start, int numColors)
{
    if (start < 0 || start > 39) {
        qWarning("QApplication::qwsSetCustomColors: start < 0 || start > 39");
        return;
    }
    if (start + numColors > 40) {
        numColors = 40 - start;
        qWarning("QApplication::qwsSetCustomColors: Too many colors");
    }
    start += 216;
    for (int i = 0; i < numColors; i++) {
        qt_screen->set(start + i, qRed(colorTable[i]), qGreen(colorTable[i]),
                        qBlue(colorTable[i]));
    }
}

#ifndef QT_NO_QWS_MANAGER
/*!
    Return the QWSDecoration used for decorating windows.

    \warning This method is non-portable. It is only available in
    Qtopia Core.

    \sa QDecoration
*/
QDecoration &QApplication::qwsDecoration()
{
    return *qws_decoration;
}

/*!
    \fn void QApplication::qwsSetDecoration(QDecoration *decoration)

    Set the QWSDecoration derived class to use for decorating the
    Qtopia Core windows to \a decoration.

    This method is non-portable. It is only available in Qtopia Core.

    \sa QDecoration
*/
void QApplication::qwsSetDecoration(QDecoration *dec)
{
    if (dec) {
        delete qws_decoration;
        qws_decoration = dec;
        QWidgetList widgets = topLevelWidgets();
        for (int i = 0; i < widgets.size(); ++i) {
            QWidget *w = widgets[i];
            if (w->isVisible() && w != desktop()) {
                static_cast<QETWidget *>(w)->updateRegion();
                static_cast<QETWidget *>(w)->repaintDecoration(desktop()->rect(), false);
                if (w->isMaximized())
                    w->showMaximized();
            }
        }
    }
}

/*!
  \overload

  Requests a QDecoration object for \a decoration from the QDecorationFactory.

  The string must be one of the QDecorationFactory::keys(). Keys are
  case insensitive.

  A later call to the QApplication constructor will override the
  requested style when a "-style" option is passed in as a commandline
  parameter.

  Returns 0 if an unknown \a decoration is passed, otherwise the QStyle object
  returned is set as the application's GUI style.
*/
QDecoration* QApplication::qwsSetDecoration(const QString &decoration)
{
    QDecoration *decore = QDecorationFactory::create(decoration);
    if (!decore)
        return 0;

    qwsSetDecoration(decore);
    return decore;
}

#endif

bool QApplicationPrivate::modalState()
{
    return app_do_modal;
}

void QApplicationPrivate::enterModal_sys(QWidget *widget)
{
    if (!qt_modal_stack)
        qt_modal_stack = new QWidgetList;
    qt_modal_stack->insert(0, widget);
    app_do_modal = true;
}

void QApplicationPrivate::leaveModal_sys(QWidget *widget)
{
    if (qt_modal_stack && qt_modal_stack->removeAll(widget)) {
        if (qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
        }
    }
    app_do_modal = qt_modal_stack != 0;
}

static bool qt_try_modal(QWidget *widget, QWSEvent *event)
{
    QWidget * top = 0;

    if (QApplicationPrivate::tryModalHelper(widget, &top))
        return true;

    bool block_event  = false;
    bool paint_event = false;

    switch (event->type) {
        case QWSEvent::Focus:
            if (!static_cast<QWSFocusEvent*>(event)->simpleData.get_focus)
                break;
            // drop through
        case QWSEvent::Mouse:                        // disallow mouse/key events
        case QWSEvent::Key:
            block_event         = true;
            break;
#if 0
    case QWSEvent::RegionModified:
            paint_event = true;
            break;
#endif
    }

    if (top->parentWidget() == 0 && (block_event || paint_event))
        top->raise();

    return !block_event;
}

static int openPopupCount = 0;
void QApplicationPrivate::openPopup(QWidget *popup)
{
    openPopupCount++;
    if (!popupWidgets) {                        // create list
        popupWidgets = new QWidgetList;

        /* only grab if you are the first/parent popup */
        QPaintDevice::qwsDisplay()->grabMouse(popup,true);
        QPaintDevice::qwsDisplay()->grabKeyboard(popup,true);
        popupGrabOk = true;
    }
    popupWidgets->append(popup);                // add to end of list

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    if (popup->focusWidget()) {
        popup->focusWidget()->setFocus(Qt::PopupFocusReason);
    } else if (popupWidgets->count() == 1) { // this was the first popup
        if (QWidget *fw = QApplication::focusWidget()) {
#ifdef QT_KEYPAD_NAVIGATION
            if (QApplication::keypadNavigationEnabled()) {
//                qDebug() << "openPopup() - check focus";
                if (fw->hasEditFocus()) {
//                    qDebug() << "openPopup() - save old" << fw;
                    oldEditFocus = fw;
                    fw->setEditFocus(false);
                } else {
//                    qDebug() << "openPopup() - reset old";
                    oldEditFocus = 0;
                }
            }
#endif
            QFocusEvent e(QEvent::FocusOut, Qt::PopupFocusReason);
            QApplication::sendEvent(fw, &e);
        }
    }
}

void QApplicationPrivate::closePopup(QWidget *popup)
{
    if (!popupWidgets)
        return;

    popupWidgets->removeAll(popup);
    if (popup == popupOfPopupButtonFocus) {
        popupButtonFocus = 0;
        popupOfPopupButtonFocus = 0;
    }
    if (popupWidgets->count() == 0) {                // this was the last popup
        popupCloseDownMode = true;                // control mouse events
        delete popupWidgets;
        popupWidgets = 0;
        if (popupGrabOk) {        // grabbing not disabled
            QPaintDevice::qwsDisplay()->grabMouse(popup,false);
            QPaintDevice::qwsDisplay()->grabKeyboard(popup,false);
            popupGrabOk = false;
            // XXX ungrab keyboard
        }
        if (active_window) {
            if (QWidget *fw = active_window->focusWidget()) {
                if (fw != QApplication::focusWidget()) {
                    fw->setFocus(Qt::PopupFocusReason);
                } else {
                    QFocusEvent e(QEvent::FocusIn, Qt::PopupFocusReason);
                    QApplication::sendEvent(fw, &e);
#ifdef QT_KEYPAD_NAVIGATION
                    if (QApplication::keypadNavigationEnabled() && fw == oldEditFocus)
                        fw->setEditFocus(true);
#endif
                }
            }
        }
    } else {
        // popups are not focus-handled by the window system (the
        // first popup grabbed the keyboard), so we have to do that
        // manually: A popup was closed, so the previous popup gets
        // the focus.
        QWidget* aw = popupWidgets->last();
        if (QWidget *fw = aw->focusWidget())
            fw->setFocus(Qt::PopupFocusReason);
    }
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


// Needed for QCursor::pos

static const int AnyButton = (Qt::LeftButton | Qt::MidButton | Qt::RightButton);



//
// Wheel event translation
//
bool QETWidget::translateWheelEvent(const QWSMouseEvent *me)
{
#ifdef QT_NO_WHEELEVENT
    Q_UNUSED(me);
    return false;
#else
    const QWSMouseEvent::SimpleData &mouse = me->simpleData;

    // Figure out wheeling direction:
    //    Horizontal wheel w/o Alt
    // OR Vertical wheel   w/  Alt  ==> Horizontal wheeling
    //    ..all other permutations  ==> Vertical wheeling
    int axis = mouse.delta / 120; // WHEEL_DELTA?
    Qt::Orientation orient = ((axis == 2 || axis == -2) && (mouse.state & Qt::AltModifier == 0))
                             ||((axis == 1 || axis == -1) && mouse.state & Qt::AltModifier)
                             ? Qt::Horizontal : Qt::Vertical;

    QPoint mousePoint = QPoint(mouse.x_root, mouse.y_root);

    // send the event to the widget or its ancestors
    QWidget* popup = qApp->activePopupWidget();
    if (popup && window() != popup)
        popup->close();
    QWheelEvent we(mapFromGlobal(mousePoint), mousePoint, mouse.delta,
                   Qt::MouseButtons(mouse.state & Qt::MouseButtonMask),
                   Qt::KeyboardModifiers(mouse.state & Qt::KeyboardModifierMask), orient);
    if (QApplication::sendSpontaneousEvent(this, &we))
        return true;

    // send the event to the widget that has the focus or its ancestors, if different
    QWidget *w = this;
    if (w != qApp->focusWidget() && (w = qApp->focusWidget())) {
        QWidget* popup = qApp->activePopupWidget();
        if (popup && w != popup)
            popup->hide();
        if (QApplication::sendSpontaneousEvent(w, &we))
            return true;
    }
    return false;
#endif
}

bool QETWidget::translateMouseEvent(const QWSMouseEvent *event, int prevstate)
{
    static bool manualGrab = false;
    QPoint pos;
    QPoint globalPos;
    int button = 0;

    if (sm_blockUserInput) // block user interaction during session management
        return true;
    const QWSMouseEvent::SimpleData &mouse = event->simpleData;
    pos = mapFromGlobal(QPoint(mouse.x_root, mouse.y_root));
//     if (qt_last_x) {
//         *qt_last_x=mouse.x_root;
//         *qt_last_y=mouse.y_root;
//     }
    globalPos.rx() = mouse.x_root;
    globalPos.ry() = mouse.y_root;

    QEvent::Type type = QEvent::None;

    Qt::MouseButtons buttonstate = Qt::MouseButtons(mouse.state & Qt::MouseButtonMask);
    Qt::KeyboardModifiers keystate = Qt::KeyboardModifiers(mouse.state & Qt::KeyboardModifierMask);

    if (mouse.state == prevstate) {
        // mouse move
        type = QEvent::MouseMove;
    } else if ((mouse.state&AnyButton) != (prevstate&AnyButton)) {
        Qt::MouseButtons current_buttons = Qt::MouseButtons(prevstate&Qt::MouseButtonMask);
        for (button = Qt::LeftButton; !type && button <= Qt::MidButton; button<<=1) {
            if ((mouse.state&button) != (current_buttons&button)) {
                // button press or release
                current_buttons = Qt::MouseButtons(current_buttons ^ button);

#ifndef QT_NO_QWS_INPUTMETHODS
                //############ We used to do a QInputContext::reset(oldFocus);
                // when we changed the focus widget. See change 93389 for where the
                // focus code went. The IM code was (after testing for ClickToFocus):
                //if (mouse.state&button && w != QInputContext::microFocusWidget()) //button press
                //        QInputContext::reset(oldFocus);

#endif
                if (mouse.state&button) { //button press
                    qt_button_down = childAt(pos);
                    if (!qt_button_down)
                        qt_button_down = this;
                    if (/*XXX mouseActWindow == this &&*/
                        mouseButtonPressed == button &&
                        long(mouse.time) -long(mouseButtonPressTime)
                            < QApplication::doubleClickInterval() &&
                        qAbs(mouse.x_root - mouseXPos) < 5 &&
                        qAbs(mouse.y_root - mouseYPos) < 5) {
                        type = QEvent::MouseButtonDblClick;
                        mouseButtonPressTime -= 2000;        // no double-click next time
                    } else {
                        type = QEvent::MouseButtonPress;
                        mouseButtonPressTime = mouse.time;
                    }
                    mouseButtonPressed = button;        // save event params for
                    mouseXPos = globalPos.x();                // future double click tests
                    mouseYPos = globalPos.y();
                } else {                                // mouse button released
                    if (manualGrab) {                        // release manual grab
                        manualGrab = false;
                        // XXX XUngrabPointer(x11Display(), CurrentTime);
                    }

                    type = QEvent::MouseButtonRelease;
                }
            }
        }
        button >>= 1;
    }
    //XXX mouseActWindow = winId();                        // save some event params

    if (type == 0) {                                // event consumed
        return false; //EXIT in the normal case
    }

    if (qApp->d_func()->inPopupMode()) {                        // in popup mode
        QWidget *popup = qApp->activePopupWidget();
        // in X11, this would be the window we are over.
        // in QWS this is the top level popup.  to allow mouse
        // events to other widgets, need to go through qApp->QApplicationPrivate::popupWidgets.
        QSize s(qt_screen->width(), qt_screen->height());
        for (int i = 0; i < QApplicationPrivate::popupWidgets->size(); ++i) {
            QWidget *w = QApplicationPrivate::popupWidgets->at(i);

            if ((w->windowType() == Qt::Popup) && w->d_func()->localAllocatedRegion().contains(globalPos - w->geometry().topLeft()))
            {
                popup = w;
                break;
            }
        }
        pos = popup->mapFromGlobal(globalPos);
        bool releaseAfter = false;
        QWidget *popupChild  = popup->childAt(pos);
        QWidget *popupTarget = popupChild ? popupChild : popup;

        if (popup != popupOfPopupButtonFocus){
            popupButtonFocus = 0;
            popupOfPopupButtonFocus = 0;
        }

        if (!popupTarget->isEnabled()) {
            return false; //EXIT special case
        }

        switch (type) {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonDblClick:
                popupButtonFocus = popupChild;
                popupOfPopupButtonFocus = popup;
                break;
            case QEvent::MouseButtonRelease:
                releaseAfter = true;
                break;
            default:
                break;                                // nothing for mouse move
        }

        int oldOpenPopupCount = openPopupCount;

        if (popupButtonFocus) {
            QMouseEvent e(type, popupButtonFocus->mapFromGlobal(globalPos),
                        globalPos, Qt::MouseButton(button), buttonstate, keystate);
            QApplication::sendSpontaneousEvent(popupButtonFocus, & e);
            if (releaseAfter) {
                popupButtonFocus = 0;
                popupOfPopupButtonFocus = 0;
            }
        } else if (popupChild) {
            QMouseEvent e(type, popupChild->mapFromGlobal(globalPos),
                        globalPos, Qt::MouseButton(button), buttonstate, keystate);
            QApplication::sendSpontaneousEvent(popupChild, & e);
        } else {
            QMouseEvent e(type, pos, globalPos, Qt::MouseButton(button), buttonstate, keystate);
            QApplication::sendSpontaneousEvent(popupChild ? popupChild : popup, & e);
        }
        if (type == QEvent::MouseButtonPress && button == Qt::RightButton && (openPopupCount == oldOpenPopupCount)) {
            QWidget *popupEvent = popup;
            if(popupButtonFocus)
                popupEvent = popupButtonFocus;
            else if(popupChild)
                popupEvent = popupChild;
            QContextMenuEvent e(QContextMenuEvent::Mouse, pos, globalPos);
            QApplication::sendSpontaneousEvent(popupEvent, &e);
        }

        if (releaseAfter)
            qt_button_down = 0;

    } else { //qApp not in popup mode
        QWidget *widget = this;
        QWidget *w = QWidget::mouseGrabber();
        if (!w && qt_button_down)
            w = qt_button_down;
        if (w && w != this) {
            widget = w;
            pos = mapToGlobal(pos);
            pos = w->mapFromGlobal(pos);
        }

        if (popupCloseDownMode) {
            popupCloseDownMode = false;
            if ((windowType() == Qt::Popup))        // ignore replayed event
                return true; //EXIT
        }

        if (type == QEvent::MouseButtonRelease &&
            (mouse.state & (~button) & (Qt::LeftButton |
                                    Qt::MidButton |
                                    Qt::RightButton)) == 0) {
            qt_button_down = 0;
        }

        int oldOpenPopupCount = openPopupCount;

        QMouseEvent e(type, pos, globalPos, Qt::MouseButton(button), buttonstate, keystate);
#ifndef QT_NO_QWS_MANAGER
        if (widget->isWindow() && widget->d_func()->topData()->qwsManager
            && (widget->d_func()->topData()->qwsManager->region().contains(globalPos)
                || QWSManager::grabbedMouse() )) {
            if ((*mouseInWidget)) {
                QApplicationPrivate::dispatchEnterLeave(0, *mouseInWidget);
                (*mouseInWidget) = 0;
            }
            QApplication::sendSpontaneousEvent(widget->d_func()->topData()->qwsManager, &e);
            qApp->d_func()->last_manager = widget->d_func()->topData()->qwsManager;
        } else
#endif
        {
            if (widget != (*mouseInWidget)) {
                QApplicationPrivate::dispatchEnterLeave(widget, *mouseInWidget);
                (*mouseInWidget) = widget;
            }
            QApplication::sendSpontaneousEvent(widget, &e);
        }
        if (type == QEvent::MouseButtonPress && button == Qt::RightButton && (openPopupCount == oldOpenPopupCount)) {
            QContextMenuEvent e(QContextMenuEvent::Mouse, pos, globalPos);
            QApplication::sendSpontaneousEvent(widget, &e);
        }
    }
    return true;
}


bool QETWidget::translateKeyEvent(const QWSKeyEvent *event, bool grab) /* grab is used in the #ifdef */
{
    int code = -1;
    Qt::KeyboardModifiers state = event->simpleData.modifiers;

    if (sm_blockUserInput) // block user interaction during session management
        return true;

    if (!isEnabled())
        return true;

    QEvent::Type type = event->simpleData.is_press ?
                        QEvent::KeyPress : QEvent::KeyRelease;
    bool autor = event->simpleData.is_auto_repeat;
    QString text;
    char ascii = 0;
    if (event->simpleData.unicode) {
        QChar ch(event->simpleData.unicode);
        if (ch.unicode() != 0xffff)
            text += ch;
        ascii = ch.toLatin1();
    }
    code = event->simpleData.keycode;

#if defined QT3_SUPPORT && !defined(QT_NO_SHORTCUT)
    if (type == QEvent::KeyPress && !grab
        && static_cast<QApplicationPrivate*>(qApp->d_ptr)->use_compat()) {
        // send accel events if the keyboard is not grabbed
        QKeyEvent a(type, code, state, text, autor, int(text.length()));
        if (static_cast<QApplicationPrivate*>(qApp->d_ptr)->qt_tryAccelEvent(this, &a))
            return true;
    }
#else
    Q_UNUSED(grab);
#endif
    if (!text.isEmpty() && testAttribute(Qt::WA_KeyCompression)) {
        // the widget wants key compression so it gets it

        // XXX not implemented
    }

    QKeyEvent e(type, code, state, text, autor, int(text.length()));
    return QApplication::sendSpontaneousEvent(this, &e);
}

bool QETWidget::translateRegionEvent(const QWSRegionEvent *event)
{
    Q_D(QWidget);

    QWidget *win = QWidget::find(WId(event->window()));
    if (!win)
        return true;

    QWidgetBackingStore *bs = d->topData()->backingStore;
    QWSWindowSurface *surface = static_cast<QWSWindowSurface*>(bs->windowSurface);

    QRegion region;
    region.setRects(event->rectangles, event->simpleData.nrectangles);

    switch (event->simpleData.type) {
    case QWSRegionEvent::Allocation:
        region.translate(-win->geometry().topLeft());
        surface->setClipRegion(region);
        break;
    case QWSRegionEvent::Request:
        win->setGeometry(region.boundingRect());
        break;
    default:
        break;
    }

    return true;
}

#ifndef QT_NO_QWSEMBEDWIDGET
void QETWidget::translateEmbedEvent(const QWSEmbedEvent *event)
{
    if (event->simpleData.type | QWSEmbedEvent::Region) {
        const QRegion region = event->region;
        setGeometry(region.boundingRect());
        setVisible(!region.isEmpty());
    }
}
#endif // QT_NO_QWSEMBEDWIDGET

void QETWidget::repaintDecoration(QRegion r, bool post)
{
    Q_UNUSED(post);
#ifdef QT_NO_QWS_MANAGER
    Q_UNUSED(r);
#else
    //please note that qwsManager is a QObject, not a QWidget.
    //therefore, normal ways of painting do not work.
    // However, it does listen to paint events.

    Q_D(QWidget);
    if (isWindow() && d->topData()->qwsManager && isVisible()) {
        QWSManager *manager = d->topData()->qwsManager;
        r &= manager->region();
        if (!r.isEmpty())
            manager->repaintRegion(QDecoration::All, QDecoration::Normal);
    }
#endif
}

void QETWidget::updateRegion()
{
    Q_D(QWidget);

    QTLWExtra *topextra = d->extra->topextra;
    QRegion myregion = d->localRequestedRegion();
    myregion.translate(geometry().topLeft());

#ifndef QT_NO_QWS_MANAGER
    QWSManager *manager = topextra->qwsManager;
    if (manager)
        myregion += manager->region();
#endif

    QRect br(myregion.boundingRect());
    topextra->frameStrut.setCoords(d->data.crect.x() - br.x(),
                                   d->data.crect.y() - br.y(),
                                   br.right() - d->data.crect.right(),
                                   br.bottom() - d->data.crect.bottom());
}

void  QApplication::setCursorFlashTime(int msecs)
{
    QApplicationPrivate::cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    return QApplicationPrivate::cursor_flash_time;
}

void QApplication::setDoubleClickInterval(int ms)
{
    QApplicationPrivate::mouse_double_click_time = ms;
}

int QApplication::doubleClickInterval()
{
    return QApplicationPrivate::mouse_double_click_time;
}

void QApplication::setKeyboardInputInterval(int ms)
{
    QApplicationPrivate::keyboard_input_time = ms;
}

int QApplication::keyboardInputInterval()
{
    return QApplicationPrivate::keyboard_input_time;
}

#ifndef QT_NO_WHEELEVENT
void QApplication::setWheelScrollLines(int lines)
{
    QApplicationPrivate::wheel_scroll_lines = lines;
}

int QApplication::wheelScrollLines()
{
    return QApplicationPrivate::wheel_scroll_lines;
}
#endif

void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
    switch (effect) {
    case Qt::UI_AnimateMenu:
        QApplicationPrivate::animate_menu = enable;
        break;
    case Qt::UI_FadeMenu:
        if (enable)
            QApplicationPrivate::animate_menu = true;
        QApplicationPrivate::fade_menu = enable;
        break;
    case Qt::UI_AnimateCombo:
        QApplicationPrivate::animate_combo = enable;
        break;
    case Qt::UI_AnimateTooltip:
        QApplicationPrivate::animate_tooltip = enable;
        break;
    case Qt::UI_FadeTooltip:
        if (enable)
            QApplicationPrivate::animate_tooltip = true;
        QApplicationPrivate::fade_tooltip = enable;
        break;
    case Qt::UI_AnimateToolBox:
        QApplicationPrivate::animate_toolbox = enable;
        break;
    default:
        QApplicationPrivate::animate_ui = enable;
        break;
    }
}

bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
    if (QColormap::instance().depth() < 16 || !QApplicationPrivate::animate_ui)
        return false;

    switch(effect) {
    case Qt::UI_AnimateMenu:
        return QApplicationPrivate::animate_menu;
    case Qt::UI_FadeMenu:
        return QApplicationPrivate::fade_menu;
    case Qt::UI_AnimateCombo:
        return QApplicationPrivate::animate_combo;
    case Qt::UI_AnimateTooltip:
        return QApplicationPrivate::animate_tooltip;
    case Qt::UI_FadeTooltip:
        return QApplicationPrivate::fade_tooltip;
    case Qt::UI_AnimateToolBox:
        return QApplicationPrivate::animate_toolbox;
    default:
        return QApplicationPrivate::animate_ui;
    }
}

void QApplication::setArgs(int c, char **v)
{
    Q_D(QApplication);
    d->argc = c;
    d->argv = v;
}
