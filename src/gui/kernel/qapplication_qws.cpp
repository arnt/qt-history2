/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Started with from qapplication_x11.cpp,v 2.399 1999/10/22 14:39:33

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
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsproperty_qws.h"
#include "qscreen_qws.h"
#include "qcopchannel_qws.h"
#include "private/qlock_p.h"
#include "qmemorymanager_qws.h"
#include "qwsmanager_qws.h"
//#include "qwsregionmanager_qws.h"
#include "qwindowsystem_qws.h"
#include "qwsdisplay_qws.h"
#include "private/qwsinputcontext_p.h"
#include "qfile.h"
#include "qhash.h"
#include "qdesktopwidget.h"
#include "qcolormap.h"
#include <private/qcursor_p.h>

#include "qsettings.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
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

#include "qeventdispatcher_qws_p.h"

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <sys/time.h>

#include "private/qwidget_p.h"

const int qwsSharedRamSize = 100 * 1024;
                          //Small amount to fit on small devices.

extern void qt_qws_set_max_window_rect(const QRect& r);
extern QRect qt_maxWindowRect;

static bool servermaxrect=false; // set to true once.


extern QApplication::Type qt_appType;
extern bool qt_app_has_font;

//these used to be environment variables, they are initialized from
//environment variables in

bool qws_savefonts = false;
bool qws_screen_is_interlaced=false; //### should be detected
bool qws_shared_memory = false;
bool qws_sw_cursor = true;
bool qws_accel = true;            // ### never set
const char *qws_display_spec = ":0";
int qws_display_id = 0;
int qws_client_id = 0;
QWidget *qt_pressGrab = 0;
QWidget *qt_mouseGrb = 0;
int *qt_last_x = 0;
int *qt_last_y = 0;

bool qt_override_paint_on_screen = false;


static int mouse_x_root = -1;
static int mouse_y_root = -1;
static int mouse_state = 0;

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

// Get the name of the directory where Qt/Embedded temporary data should
// live.
QString qws_dataDir()
{
    QByteArray username = "unknown";
    const char *logname = qgetenv("LOGNAME");
    if (logname)
        username = logname;

    QByteArray dataDir = "/tmp/qtembedded-" + username;
    if (mkdir(dataDir, 0700)) {
        if (errno != EEXIST) {
            qFatal("Cannot create Qt/Embedded data directory: %s", dataDir.constData());
        }
    }

    struct stat buf;
    if (lstat(dataDir, &buf))
        qFatal("stat failed for Qt/Embedded data directory: %s", dataDir.constData());

    if (!S_ISDIR(buf.st_mode))
        qFatal("%s is not a directory", dataDir.constData());
    if (buf.st_uid != getuid())
        qFatal("Qt/Embedded data directory is not owned by user %d", getuid());

    if ((buf.st_mode & 0677) != 0600)
        qFatal("Qt/Embedded data directory has incorrect permissions: %s", dataDir.constData());
    dataDir += "/";

    return QString(dataDir);
}

// Get the filename of the pipe Qt/Embedded uses for server/client comms
QString qws_qtePipeFilename()
{
    return (qws_dataDir() + QString(QTE_PIPE).arg(qws_display_id));
}

extern void qt_qws_set_max_window_rect(const QRect&);
static void setMaxWindowRect(const QRect& r)
{
    QRect tr = qt_screen->mapFromDevice(r,
        qt_screen->mapToDevice(QSize(qt_screen->width(),qt_screen->height())));
    qt_qws_set_max_window_rect(tr);
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
QWSDisplay *qt_fbdpy = 0;                        // QWS `display'
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
    bool translateRegionModifiedEvent(const QWSRegionModifiedEvent *);
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

class QWSDisplay::Data {
public:
    Data(QObject* parent, bool singleProcess = false)
    {
#ifndef QT_NO_QWS_MULTIPROCESS
        if (singleProcess)
            csocket = 0;
        else {
            csocket = new QWSSocket(parent);
            QObject::connect(csocket, SIGNAL(disconnected()),
                              qApp, SLOT(quit()));
        }
#endif
        init();
    }

    ~Data()
    {
//        delete rgnMan; rgnMan = 0;
        delete memorymanager; memorymanager = 0;
        qt_screen->disconnect();
        delete qt_screen; qt_screen = 0;
#ifndef QT_NO_QWS_CURSOR
        delete qt_screencursor; qt_screencursor = 0;
#endif
#ifndef QT_NO_QWS_MULTIPROCESS
        shm.detach();
        if (!csocket && ramid != -1) {
            shm.destroy();
        }
        if (csocket) {
            csocket->flush(); // may be pending QCop message, eg.
            delete csocket;
        }
        delete connected_event;
#endif
    }

    void flush()
    {
#ifndef QT_NO_QWS_MULTIPROCESS
        if (csocket)
            csocket->flush();
#endif
    }

    //####public data members

//    QWSRegionManager *rgnMan;
    uchar *sharedRam;
#if !defined(Q_NO_QSHM) && !defined(QT_NO_QWS_MULTIPROCESS)
    QSharedMemory shm;
#endif
    int sharedRamSize;
    int ramid;

private:
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSSocket *csocket;
#endif
    QList<QWSEvent*> queue;

#if 0
    void debugQueue() {
            for (int i = 0; i < queue.size(); ++i) {
                QWSEvent *e = queue.at(i);
                qDebug( "   ev %d type %d sl %d rl %d", i, e->type, e->simpleLen, e->rawLen);
            }
    }
#endif

    QWSConnectedEvent* connected_event;
    QWSMouseEvent* mouse_event;
    QWSRegionModifiedEvent *region_event;
    QWSRegionModifiedEvent *region_ack;
    QPoint region_offset;
    int region_offset_window;
#ifndef QT_NO_COP
    QWSQCopMessageEvent *qcop_response;
#endif
    QWSEvent* current_event;
    QList<int> unused_identifiers;
    int mouse_event_count;
    void (*mouseFilter)(QWSMouseEvent *);

    enum { VariableEvent=-1 };
public:
    bool queueNotEmpty()
    {
        return mouse_event||region_event||queue.count() > 0;
    }
    QWSEvent *dequeue()
    {
        QWSEvent *r;
        if (queue.count()) {
            r = queue.first(); queue.removeFirst();
        } else if (mouse_event) {
            r = mouse_event;
            mouse_event = 0;
            mouse_event_count = 0;
        } else {
            r = region_event;
            region_event = 0;
        }
        return r;
    }

    QWSEvent *peek() {
        return queue.first();
    }
#ifndef QT_NO_QWS_MULTIPROCESS
    bool directServerConnection() { return csocket == 0; }
#else
    bool directServerConnection() { return true; }
#endif
    void fillQueue();
    void waitForConnection();
//    void waitForRegionAck();
    void waitForCreation();
#ifndef QT_NO_COP
    void waitForQCopResponse();
#endif
    void offsetPendingExpose(int, const QPoint &);
    void translateExpose(QWSRegionModifiedEvent *re, const QPoint &p)
    {
        for (int i = 0; i < re->simpleData.nrectangles; i++)
            re->rectangles[i].translate(p.x(), p.y());
    }
    void init();
    void create()
    {
        QWSCreateCommand cmd;
#ifndef QT_NO_QWS_MULTIPROCESS
        if  (csocket)
            cmd.write(csocket);
        else
#endif
            qt_server_enqueue(&cmd);
    }

    void sendCommand(QWSCommand & cmd)
    {
#ifndef QT_NO_QWS_MULTIPROCESS
        if  (csocket)
            cmd.write(csocket);
        else
#endif
            qt_server_enqueue(&cmd);
    }


    QWSEvent *readMore();

    int takeId()
    {
        // top up bag
        create();
        if (!unused_identifiers.count()) {
            // We have to wait!
            for (int o=0; o<30; o++)
                create();
            waitForCreation();
        }
        int i = unused_identifiers.first();
        unused_identifiers.removeFirst();
        return i;
    }

    void setMouseFilter(void (*filter)(QWSMouseEvent*))
    {
        mouseFilter = filter;
    }
};

void QWSDisplay::Data::init()
{
    connected_event = 0;
    region_ack = 0;
    mouse_event = 0;
    region_event = 0;
    region_offset_window = 0;
#ifndef QT_NO_COP
    qcop_response = 0;
#endif
    current_event = 0;
    mouse_event_count = 0;
    ramid = -1;
    mouseFilter = 0;

    QString pipe = qws_qtePipeFilename();

    sharedRamSize = qwsSharedRamSize;

#ifndef QT_NO_QWS_MULTIPROCESS
    if (csocket)    {
        // QWS client
        csocket->connectToLocalFile(pipe);
        QWSIdentifyCommand cmd;
        cmd.setId(appName);
#ifndef QT_NO_QWS_MULTIPROCESS
        if  (csocket)
            cmd.write(csocket);
        else
#endif
            qt_server_enqueue(&cmd);

        // wait for connect confirmation
        waitForConnection();

        qws_client_id = connected_event->simpleData.clientId;

        // now we want to get the exact display spec to use if we haven't
        // specified anything.
        if (qws_display_spec[0] == ':')
            qws_display_spec = strdup(connected_event->display); //###memory leak

        if (!QWSDisplay::initLock(pipe, false))
            qFatal("Cannot get display lock");

        shm = QSharedMemory(0,pipe,'m');
        if (shm.create() && shm.attach()) {
            QScreen *s = qt_get_screen(qws_display_id, qws_display_spec);
            sharedRamSize += s->memoryNeeded(qws_display_spec);
        } else {
            perror("Can't attach to main ram memory.");
            exit(1);
        }
        sharedRam = static_cast<uchar *>(shm.base());
    } else
#endif
    {

        // QWS server
        if (!QWSDisplay::initLock(pipe, true))
            qFatal("Cannot get display lock");

        QScreen *s = qt_get_screen(qws_display_id, qws_display_spec);
        sharedRamSize += s->memoryNeeded(qws_display_spec);

#ifndef QT_NO_QWS_MULTIPROCESS

        shm = QSharedMemory(sharedRamSize,pipe, 'm');
        if (!shm.create())
            perror("Cannot create main ram shared memory\n");
        if (!shm.attach())
            perror("Cannot attach to main ram shared memory\n");
        sharedRam = static_cast<uchar *>(shm.base());
#else
        sharedRam=static_cast<uchar *>(malloc(sharedRamSize));
#endif
        // Need to zero index count at end of block, might as well zero
        // the rest too
        memset(sharedRam,0,sharedRamSize);

        QWSIdentifyCommand cmd;
        cmd.setId(appName);
        qt_server_enqueue(&cmd);
    }
    setMaxWindowRect(QRect(0,0,qt_screen->width(),qt_screen->height()));
    int mouseoffset = 0;

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

#ifndef QT_NO_QWS_CURSOR
    mouseoffset=qt_screen->initCursor(sharedRam + sharedRamSize,
#ifndef QT_NO_QWS_MULTIPROCESS
                                      !csocket
#else
                                      true
#endif
                                     );
#endif

    sharedRamSize -= mouseoffset;
    sharedRamSize -= sizeof(int);
    qt_last_x = reinterpret_cast<int *>(sharedRam + sharedRamSize);
    sharedRamSize -= sizeof(int);
    qt_last_y = reinterpret_cast<int *>(sharedRam + sharedRamSize);

    /* Initialise framebuffer memory manager */
    /* Add 4k for luck and to avoid clobbering hardware cursor */
    int screensize=qt_screen->screenSize();
    memorymanager=new QMemoryManager(qt_screen->base()+screensize+4096,
        qt_screen->totalSize()-(screensize+4096),0);

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
    return incoming ? incoming.takeFirst() : 0;
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
            unused_identifiers.append(ce->simpleData.objectid);
            delete e;
        } else if (e->type == QWSEvent::Mouse) {
            if (!qt_screen) {
                delete e;
            } else {
                QWSMouseEvent *me = static_cast<QWSMouseEvent*>(e);
                if (mouseFilter)
                    mouseFilter(me);
                if (mouse_event) {
                    if ((mouse_event->window() != e->window ()
                          || mouse_event->simpleData.state !=
                          me->simpleData.state)) {
                        queue.append(mouse_event);
                        mouse_event_count = 0;
                    } else if (mouse_event_count == 1) {
                        // make sure the position of the press is not
                        // compressed away.
                        queue.append(mouse_event);
                    } else {
                        delete mouse_event;
                    }
                }
                QSize s(qt_screen->deviceWidth(), qt_screen->deviceHeight());
                QPoint p(me->simpleData.x_root, me->simpleData.y_root);
                p = qt_screen->mapFromDevice(p, s);
                me->simpleData.x_root = p.x();
                me->simpleData.y_root = p.y();
                mouse_event = me;
                mouse_event_count++;
            }
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
                        region_event->setData(reinterpret_cast<char *>(ur.rects().data()),
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
        } else if (e->type==QWSEvent::MaxWindowRect && !servermaxrect && qt_screen) {
            // Process this ASAP, in case new widgets are created (startup)
            servermaxrect=true;
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


void QWSDisplay::Data::waitForConnection()
{
    fillQueue();
#ifndef QT_NO_QWS_MULTIPROCESS
    for (int i = 0; i < 5; i++) {
        fillQueue();
        if (connected_event)
            return;
        if (csocket) {
            csocket->flush();
            csocket->waitForReadyRead(2000);
//            qDebug( "waitForReadyRead %d bytesAvailable %lld", i, csocket->bytesAvailable() );
        }
        usleep(50000);
        //sleep(1);
        fillQueue();
    }
#else
    if (connected_event)
        return;
#endif
    qWarning("No Qt/Embedded server appears to be running.");
    qWarning("If you want to run this program as a server,");
    qWarning("add the \"-qws\" command-line option.");
    exit(1);
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

void QWSDisplay::Data::waitForCreation()
{
    fillQueue();
    while (unused_identifiers.count() == 0) {
#ifndef QT_NO_QWS_MULTIPROCESS
        if (csocket) {
            csocket->flush();
            csocket->waitForReadyRead(1000);
        }
#endif
        fillQueue();
    }
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
    QByteArray f___ = data; //#########
    cmd.setData(f___.data(), f___.size());
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

#ifndef QT_NO_QWS_REPEATER
void QWSDisplay::repaintRegion(QRegion & r)
{
    QWSRepaintRegionCommand cmd;
    cmd.simpleData.winId = -1;
    cmd.simpleData.nrectangles=r.rects().count();
    cmd.setData((char *)r.rects().data(),
                 r.rects().count() * sizeof(QRect), false);
    d->sendCommand(cmd);
}
#endif

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
    cmd.simpleData.altitude = alt;
    cmd.simpleData.fixed = fixed;
    if (d->directServerConnection()) {
        qwsServer->set_altitude(&cmd);
    } else {
        d->sendCommand(cmd);
    }
//    d->waitForRegionAck();
}

void QWSDisplay::setOpacity(int winId, int opacity)
{
    QWSSetOpacityCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.opacity = opacity;
    if (d->directServerConnection()) {
        qwsServer->set_opacity(&cmd);
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
        qwsServer->request_focus(&cmd);
    else
        d->sendCommand(cmd);
}

void QWSDisplay::setIdentity(const QString &appName)
{
    QWSIdentifyCommand cmd;
    cmd.setId(appName);
    if (d->directServerConnection())
        qwsServer->set_identity(&cmd);
    else
        d->sendCommand(cmd);
}

void QWSDisplay::nameRegion(int winId, const QString& n, const QString &c)
{
    QWSRegionNameCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.setName(n, c);
    if (d->directServerConnection())
        qwsServer->name_region(&cmd);
    else
        d->sendCommand(cmd);
}

void QWSDisplay::requestRegion(int winId, int shmid, bool opaque, QRegion r)
{
    if (d->directServerConnection()) {
        qwsServer->request_region(winId, shmid, opaque, r);
    } else {
        QVector<QRect> ra = r.rects();

        /*
          for (int i = 0; i < ra.size(); i++) {
          QRect r(ra[i]);
          qDebug("rect: %d %d %d %d", r.x(), r.y(), r.right(), r.bottom());
          }
        */

        QWSRegionCommand cmd;
        cmd.simpleData.windowid = winId;
        cmd.simpleData.opaque = opaque;
        cmd.simpleData.shmid = shmid;
        cmd.simpleData.nrectangles = ra.count();
        cmd.setData(reinterpret_cast<char *>(ra.data()), ra.count() * sizeof(QRect), false);
        d->sendCommand(cmd);
    }

    // if (!r.isEmpty())
    //    d->waitForRegionAck();
}

void QWSDisplay::repaintRegion(int winId, bool opaque, QRegion r)
{
    if (d->directServerConnection()) {
        qwsServer->repaint_region(winId, opaque, r);
    } else {
        QVector<QRect> ra = r.rects();

        /*
          for (int i = 0; i < ra.size(); i++) {
          QRect r(ra[i]);
          qDebug("rect: %d %d %d %d", r.x(), r.y(), r.right(), r.bottom());
          }
        */

        QWSRepaintRegionCommand cmd;
        cmd.simpleData.windowid = winId;
        cmd.simpleData.opaque = opaque;
        cmd.simpleData.nrectangles = ra.count();
        cmd.setData(reinterpret_cast<char *>(ra.data()), ra.count() * sizeof(QRect), false);
        d->sendCommand(cmd);
    }
}


void QWSDisplay::moveRegion(int winId, int dx, int dy)
{
    //UNUSED QETWidget *widget = (QETWidget*)QWidget::find((WId)winId);

    QPoint p1 = qt_screen->mapToDevice(QPoint(0, 0),
                    QSize(qt_screen->width(), qt_screen->height()));
    QPoint p2 = qt_screen->mapToDevice(QPoint(dx, dy),
                    QSize(qt_screen->width(), qt_screen->height()));

    QWSRegionMoveCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.dx = p2.x() - p1.x();
    cmd.simpleData.dy = p2.y() - p1.y();

    if (d->directServerConnection()) {
        qwsServer->move_region(&cmd);
    } else {
        d->sendCommand(cmd);
    }
    d->offsetPendingExpose(winId, QPoint(cmd.simpleData.dx, cmd.simpleData.dy));
//    d->waitForRegionAck();
}

void QWSDisplay::destroyRegion(int winId)
{
    QWSRegionDestroyCommand cmd;
    cmd.simpleData.windowid = winId;
    if (d->directServerConnection()) {
        qwsServer->destroy_region(&cmd);
    } else {
        d->sendCommand(cmd);
    }
}

#ifndef QT_NO_QWS_IM

void QWSDisplay::sendIMUpdate(int type, int winId, int widgetid)
{
    QWSIMUpdateCommand cmd;
    cmd.simpleData.windowid = winId;
    cmd.simpleData.windowid = widgetid;

    cmd.simpleData.type = type;

      if (d->directServerConnection()) {
        qwsServer->im_update(&cmd);
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
        qwsServer->im_response(&cmd);
    } else {
        d->sendCommand(cmd);
    }
}

void QWSDisplay::resetIM()
{
    sendIMUpdate(QWSIMUpdateCommand::Reset, -1, -1);
}

void QWSDisplay::sendIMMouseEvent(int index, bool isPress)
{
    QWSIMMouseCommand cmd;
    cmd.simpleData.index = index;
    cmd.simpleData.state = isPress ? QWSServer::MousePress : QWSServer::MouseRelease;
    if (d->directServerConnection()) {
        qwsServer->send_im_mouse(&cmd);
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
#ifndef QT_NO_QWS_PROPERTIES
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

    int dataLen = cursImg.numBytes();

    unsigned char *data = new unsigned char [dataLen*2];
    memcpy(data, cursImg.bits(), dataLen);
    memcpy(data + dataLen, maskImg.bits(), dataLen);

    cmd.setData(reinterpret_cast<char*>(data), dataLen*2);
    delete [] data;
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


void QWSDisplay::setWindowCaption(QWidget *w, const QString &c)
{
    if (w->isWindow()) {
        nameRegion(w->winId(), w->objectName(), c);
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
        cmd.simpleData.windowid = top->winId();
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

void QWSDisplay::setRawMouseEventFilter(void (*filter)(QWSMouseEvent *))
{
    if (qt_fbdpy)
        qt_fbdpy->d->setMouseFilter(filter);
}

#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
#ifndef QT_NO_QWS_TRANSFORMED
extern void qws_setScreenTransformation(int);
extern void qws_mapPixmaps(bool from);
#endif

void QWSDisplay::setTransformation(int t)
{
#ifndef QT_NO_QWS_TRANSFORMED

    bool isFullScreen = qt_maxWindowRect == QRect(0, 0, qt_screen->width(), qt_screen->height());

    QPixmapCache::clear();
    QFontCache::instance->clear();
    qws_mapPixmaps(true);
    qws_setScreenTransformation(t);
    qws_mapPixmaps(false);

    if (qt_fbdpy->d_func()->directServerConnection()) {
        qwsServer->resetEngine();
        qwsServer->refresh();
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
    if (isFullScreen)
        qt_qws_set_max_window_rect(QRect(0,0, qt_screen->width(), qt_screen->height()));

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
            bg = QApplicationPrivate::sys_pal->color(QPalette::Background);
        if (appFGCol)
            fg = QColor(appFGCol);
        else
            fg = QApplicationPrivate::sys_pal->color(QPalette::Foreground);
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




/*! \internal
    apply the settings to the application
*/
bool QApplicationPrivate::qws_apply_settings()
{

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
        QString(QLatin1String("%1.%2/libraryPath"))
        .arg(QT_VERSION >> 16)
        .arg((QT_VERSION & 0xff00) >> 8);
    QStringList pathlist = settings.value(libpathkey).toString().split(QLatin1Char(':'));
    if (! pathlist.isEmpty()) {
        QStringList::ConstIterator it = pathlist.begin();
        while (it != pathlist.end())
            QApplication::addLibraryPath(*it++);
    }

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

    num =
        settings.value(QLatin1String("wheelScrollLines"),
                       QApplication::wheelScrollLines()).toInt();
    QApplication::setWheelScrollLines(num);

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

    QString defaultcodec = settings.value(QLatin1String("defaultCodec"),
                                          QVariant(QLatin1String("none"))).toString();
    if (defaultcodec != QLatin1String("none")) {
        QTextCodec *codec = QTextCodec::codecForName(defaultcodec.toLatin1());
        if (codec)
            QTextCodec::setCodecForTr(codec);
    }

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
    QStringList fontsubs = settings.childGroups();
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
}





static void init_display()
{
    if (qt_fbdpy) return; // workaround server==client case

    setlocale(LC_ALL, "");                // use correct char set mapping
    setlocale(LC_NUMERIC, "C");        // make sprintf()/scanf() work

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
        f = QFont("helvetica", 10);
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
    char *x = qgetenv(var);
    return (x && *x) ? (strcmp(x,"0") != 0) : defaultvalue;
}

void qt_init(QApplicationPrivate *priv, int type)
{
    if (type == QApplication::GuiServer)
        qt_is_gui_used = false; //we'll turn it on in a second
    qws_sw_cursor = read_bool_env_var("QWS_SW_CURSOR",qws_sw_cursor);
    qws_screen_is_interlaced = read_bool_env_var("QWS_INTERLACE",false);

    const char *display = qgetenv("QWS_DISPLAY");
    if (display)
        qws_display_spec = strdup(display); // since we setenv later!

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
        } else if (arg == "-paint-on-screen") {
            qt_override_paint_on_screen = true;
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


    priv->argc = j;

    mouseInWidget = new QPointer<QWidget>;

    //We only support 10 displays, so the string should be ".*:[0-9]"
    //    QRegExp r(":[0-9]");  // only supports 10 displays
    QString disp(qws_display_spec);
    //    int m = r.match(QString(qws_display_spec) , 0, &len);
    if (disp[disp.length()-2] == ':' &&
         disp[disp.length()-1].digitValue() >= 0) {
        qws_display_id = disp[disp.length()-1].digitValue();
    }

    if (type == QApplication::GuiServer) {
        qt_appType = QApplication::Type(type);
        qws_single_process = true;
        QWSServer::startup(flags);
        setenv("QWS_DISPLAY", qws_display_spec, 0);
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
        if (!qws_decoration)
            QApplication::qwsSetDecoration(new QDecorationDefault);
#endif
#ifndef QT_NO_QWS_IM
        qApp->setInputContext(new QWSInputContext);
#endif
    }

#ifndef QT_NO_STYLE_INTERLACE
/*### convert interlace style
    if (qws_screen_is_interlaced)
        QApplication::setStyle(new QInterlaceStyle);
*/
#endif
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

QString qAppName() // get application name
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

        /* If strind isn't at the end of the string the it's an invalid
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
    if (QApplicationPrivate::main_widget) {                        // give WM command line
        if (windowIcon().isNull() && QApplicationPrivate::main_widget->testAttribute(Qt::WA_SetWindowIcon))
            setWindowIcon(QApplicationPrivate::main_widget->windowIcon());
        if (mwTitle) {
            // XXX
        }
        if (mwGeometry) {                        // parse geometry
            int x, y;
            int w, h;
            int m = parseGeometry(mwGeometry, &x, &y, &w, &h);
            QSize minSize = QApplicationPrivate::main_widget->minimumSize();
            QSize maxSize = QApplicationPrivate::main_widget->maximumSize();
            if ((m & XValue) == 0)
                x = QApplicationPrivate::main_widget->geometry().x();
            if ((m & YValue) == 0)
                y = QApplicationPrivate::main_widget->geometry().y();
            if ((m & WidthValue) == 0)
                w = QApplicationPrivate::main_widget->width();
            if ((m & HeightValue) == 0)
                h = QApplicationPrivate::main_widget->height();
            w = qMin(w,maxSize.width());
            h = qMin(h,maxSize.height());
            w = qMax(w,minSize.width());
            h = qMax(h,minSize.height());
            if ((m & XNegative)) {
                x = desktop()->width()  + x - w;
            }
            if ((m & YNegative)) {
                y = desktop()->height() + y - h;
            }
            QApplicationPrivate::main_widget->setGeometry(x, y, w, h);
        }
    }
}
#endif

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
        QWidget *upw = QApplicationPrivate::widgetAt_sys(*qt_last_x, *qt_last_y);
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
                QWidget *c = findChildWidget(w, w->mapFromParent(pos));
                return c ? c : w;
            }
        }
    }
    return 0;
}

/*!
    Recursively searches all of \a p's child widgets and returns the
    most nested widget that contains point \a pos.
*/
QWidget *QApplicationPrivate::findChildWidget(const QWidget *p, const QPoint &pos)
{
    return findWidget(p->children(), pos, true);
}

QWidget *QApplication::topLevelAt(const QPoint &pos)
{

#warning "allocatedRegion problem"
    QWidgetList list = topLevelWidgets();

    for (int i = list.size()-1; i >= 0; --i) {
        QWidget *w = list[i];
        if (w != QApplication::desktop() &&
             w->isVisible() && w->geometry().contains(pos)
            // && w->d_func()->allocatedRegion().contains(qt_screen->mapToDevice(w->mapToGlobal(w->mapFromParent(pos)), QSize(qt_screen->width(), qt_screen->height())))
            )
            return w;
    }
    return 0;
}

QWidget *QApplicationPrivate::widgetAt_sys(int x, int y)
{
    // XXX not a fast function...
    QWidget *tlw = QApplication::topLevelAt(x, y);
    if (!tlw)
        return 0;
    QPoint pos(x,y);
    QWidget *c = findChildWidget(tlw, tlw->mapFromParent(pos));
    return c ? c : tlw;
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
    if (event->type == QWSEvent::QCopMessage) {
        QWSQCopMessageEvent *e = static_cast<QWSQCopMessageEvent*>(event);
        QCopChannel::sendLocally(e->channel, e->message, e->data);
        return 0;
    }
#endif

    QETWidget *widget = static_cast<QETWidget*>(QWidget::find(WId(event->window())));
    if (d->last_manager && event->type == QWSEvent::Mouse) {
        QPoint pos(event->asMouse()->simpleData.x_root, event->asMouse()->simpleData.y_root);
        if (!d->last_manager->cachedRegion().contains(pos)) {
            // MouseEvent not yet delivered, so QCursor::pos() is not yet updated, sending 2 x pos
            QMouseEvent outside(QEvent::MouseMove, pos, pos, Qt::NoButton, 0, 0);
            QApplication::sendSpontaneousEvent(d->last_manager, &outside);
            d->last_manager = 0;
        }
    }

    QETWidget *keywidget=0;
    bool grabbed=false;
    if (event->type==QWSEvent::Key || event->type == QWSEvent::IMEvent || event->type == QWSEvent::IMQuery) {
        keywidget = static_cast<QETWidget*>(QWidget::keyboardGrabber());
        if (keywidget) {
            grabbed = true;
        } else {
            if (QApplicationPrivate::focus_widget && QApplicationPrivate::focus_widget->isVisible())
                keywidget = static_cast<QETWidget*>(QApplicationPrivate::focus_widget);
            else if (widget)
                keywidget = static_cast<QETWidget*>(widget->window());
        }
    } else if (event->type==QWSEvent::MaxWindowRect) {
        servermaxrect=true;
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
            QSize s(qt_screen->width(), qt_screen->height());
            QPoint dp = qt_screen->mapToDevice(p, s);
#warning "more alloc_region trouble"
            //#### why should we get events outside alloc_region ????
            if (1 /*widget->data->alloc_region.contains(dp) */) {
                // Find the child widget that the cursor is in.
                w = static_cast<QETWidget*>(QApplicationPrivate::findChildWidget(widget, widget->mapFromParent(p)));
                w = w ? static_cast<QETWidget*>(w) : widget;
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

#ifndef QT_NO_QWS_IM
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

    case QWSEvent::RegionModified:
        widget->translateRegionModifiedEvent(static_cast<QWSRegionModifiedEvent*>(event));
        break;

    case QWSEvent::Focus:
        if ((static_cast<QWSFocusEvent*>(event))->simpleData.get_focus) {
            if (widget == static_cast<QWidget *>(desktop()))
                return true; // not interesting
            if (d->inPopupMode()) {
               //  just some delayed focus event to ignore
                break;
            }
            if (activeWindow() != widget) {
                setActiveWindow(widget);
                static_cast<QETWidget *>(QApplicationPrivate::active_window)->repaintDecoration(desktop()->rect(), false);

                QWidget *w = widget->focusWidget();
                while (w && w->focusProxy())
                    w = w->focusProxy();
                if (w && (w->focusPolicy() != Qt::NoFocus))
                    w->setFocus();
                else
                    widget->focusNextPrevChild(true);
                if (!QApplicationPrivate::focus_widget) {
                    if (widget->focusWidget())
                        widget->focusWidget()->setFocus();
                    else
                        widget->window()->setFocus();
                }
            }
        } else {        // lost focus
            if (widget == static_cast<QWidget *>(desktop()))
                return true; // not interesting
            if (QApplicationPrivate::focus_widget && !d->inPopupMode()) {
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
    default:
        break;
    }

    return 0;
}

/*!
    \fn bool QApplication::qwsEventFilter(QWSEvent *event)

    \warning This virtual function is only implemented under Qt/Embedded.

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
    Set Qt/Embedded custom color table.

    Qt/Embedded on 8-bpp displays allocates a standard 216 color cube.
    The remaining 40 colors may be used by setting a custom color
    table in the QWS master process before any clients connect.

    \a colorTable is an array of up to 40 custom colors. \a start is
    the starting index (0-39) and \a numColors is the number of colors
    to be set (1-40).

    This method is non-portable. It is available \e only in
    Qt/Embedded.
*/
void QApplication::qwsSetCustomColors(QRgb *colorTable, int start, int numColors)
{
    if (start < 0 || start > 39) {
        qWarning("QApplication::qwsSetCustomColors - start < 0 || start > 39");
        return;
    }
    if (start + numColors > 40) {
        numColors = 40 - start;
        qWarning("QApplication::qwsSetCustomColors - too many colors");
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

    This method is non-portable. It is available \e only in Qt/Embedded.

    \sa QDecoration
*/
QDecoration &QApplication::qwsDecoration()
{
    return *qws_decoration;
}

/*!
    Set the QWSDecoration derived class to use for decorating the
    Qt/Embedded windows to \a dec.

    This method is non-portable. It is available \e only in
    Qt/Embedded.

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

  The string must be one of the QStyleFactory::keys(), typically one
  of "windows", "motif", "cde", "motifplus", "platinum", "sgi" and
  "compact". Depending on the platform, "windowsxp", "aqua" or
  "macintosh" may be available.

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

void QApplicationPrivate::enterModal(QWidget *widget)
{
    if (!qt_modal_stack) {                        // create modal stack
        qt_modal_stack = new QWidgetList;
    }
    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }

    qt_modal_stack->insert(0, widget);
    app_do_modal = true;
}


void QApplicationPrivate::leaveModal(QWidget *widget)
{
    if (qt_modal_stack && qt_modal_stack->removeAll(widget)) {
        if (qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
        }
    }
    app_do_modal = qt_modal_stack != 0;

    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }
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
        case QWSEvent::RegionModified:
            paint_event = true;
            break;
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
    if (qt_last_x) {
        *qt_last_x=mouse.x_root;
        *qt_last_y=mouse.y_root;
    }
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

#ifndef QT_NO_QWS_IM
                //############ We used to do a QInputContext::reset(oldFocus);
                // when we changed the focus widget. See change 93389 for where the
                // focus code went. The IM code was (after testing for ClickToFocus):
                //if (mouse.state&button && w != QInputContext::microFocusWidget()) //button press
                //        QInputContext::reset(oldFocus);

#endif
                if (mouse.state&button) { //button press
                    qt_button_down = QApplicationPrivate::findChildWidget(this, pos);        //magic for masked widgets
                    if (!qt_button_down || !qt_button_down->testAttribute(Qt::WA_MouseNoMask))
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
#warning "even more alloc_region trouble"
            if ((w->windowType() == Qt::Popup) && w->d_func()->localRequestedRegion().contains(globalPos - w->geometry().topLeft())) { //was alloc_region
                popup = w;
                break;
            }
        }
        pos = popup->mapFromGlobal(globalPos);
        bool releaseAfter = false;
        QWidget *popupChild  = QApplicationPrivate::findChildWidget(popup, pos);
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

#if defined QT3_SUPPORT && !defined(QT_NO_ACCEL)
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


void QETWidget::repaintDecoration(QRegion r, bool post)
{
#ifndef QT_NO_QWS_MANAGER
    //please note that qwsManager is a QObject, not a QWidget.
    //therefore, normal ways of painting do not work.
    // However, it does listen to paint events.

    Q_D(QWidget);
    if (isWindow() && d->topData()->qwsManager) {
        r &= d->topData()->qwsManager->region();
        if (!r.isEmpty()) {
            r.translate(-data->crect.x(),-data->crect.y());
            if (post) {
                //### why not use r here as well?
                QApplication::postEvent(d->topData()->qwsManager, new QPaintEvent(visibleRegion()));
            } else {
                QPaintEvent e(r&visibleRegion());
                QApplication::sendEvent(d->topData()->qwsManager, &e);
            }
        }
    }
#endif
}

void QETWidget::updateRegion()
{
    qWarning("QETWidget::updateRegion() not implemented");
#if 0
    if ((windowType() == Qt::Desktop))
       return;
    if (!isVisible())
        return;
    QRegion r;
    if (d->extra && !d->extra->mask.isEmpty()) {
       r = d->extra->mask;
       r &= rect();
       r.translate(data->crect.x(),data->crect.y());
    } else {
       r = data->crect;
    }
#ifndef QT_NO_QWS_MANAGER
    if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager)
        r += d->extra->topextra->qwsManager->region();;
#endif

    r = qt_screen->mapToDevice(r, QSize(qt_screen->width(), qt_screen->height()));

    qwsDisplay()->requestRegion(winId(), r);
#endif
}


bool QETWidget::translateRegionModifiedEvent(const QWSRegionModifiedEvent *event)
{
    qDebug("QETWidget::translateRegionModifiedEvent");
#if 0
    QWSRegionManager *rgnMan = qt_fbdpy->regionManager();

    if (data->alloc_region_index < 0) {
        data->alloc_region_index = rgnMan->find(winId());

        if (data->alloc_region_index < 0) {
            return false;
        }
    }

    if (event->simpleData.nrectangles)
    {
        QRegion exposed;
        exposed.setRects(event->rectangles, event->simpleData.nrectangles);
        QSize s(qt_screen->deviceWidth(), qt_screen->deviceHeight());
        exposed = qt_screen->mapFromDevice(exposed, s);
/*
        for (int i = 0; i < event->simpleData.nrectangles; i++)
            qDebug("exposed: %d, %d %d x %d",
                event->rectangles[i].x(),
                event->rectangles[i].y(),
                event->rectangles[i].width(),
                event->rectangles[i].height());
*/
        /////////// repaintDecoration(exposed, false);

//### change 25062: don't leave WM decorations behind when resizing
//#ifndef QT_NO_QWS_MANAGER
//        exposed |= extraExposed;
//#endif
//### change 53580: but only for resizes
//    qws_regionRequest = false;

        d->bltToScreen(exposed);
    }
#endif
    return true;
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

