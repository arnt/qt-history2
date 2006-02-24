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

#include "qwindowsystem_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws_p.h"
#include "qtransportauth_qws_p.h"
#include "qwsutils_qws.h"
#include "qwscursor_qws.h"
#include "qwsdisplay_qws.h"
#include "qmouse_qws.h"
#include "qcopchannel_qws.h"

#include "qapplication.h"
#include "qsocketnotifier.h"
#include "qpolygon.h"
#include "qimage.h"
#include "qcursor.h"
#include <private/qpaintengine_raster_p.h>
#include "qscreen_qws.h"
#include "qwindowdefs.h"
#include "private/qlock_p.h"
#include "qwslock_p.h"
#include "qfile.h"
#include "qtimer.h"
#include "qpen.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qinputcontext.h"
#include "qpainter.h"

#ifdef QTRANSPORTAUTH_DEBUG
#include <qdebug.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef QT_NO_QWS_MULTIPROCESS
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#ifndef Q_OS_DARWIN
# include <sys/sem.h>
#endif
#include <sys/param.h>
#include <sys/mount.h>
#endif
#include <signal.h>
#include <fcntl.h>

#if !defined(QT_NO_SOUND) && !defined(Q_OS_DARWIN)
#ifdef QT_USE_OLD_QWS_SOUND
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#else
#include "qsoundqss_qws.h"
#endif
#endif

#include "qkbddriverfactory_qws.h"
#include "qmousedriverfactory_qws.h"

#include <qbuffer.h>

#include <private/qwidget_qws_p.h>

#include "qwindowsystem_p.h"

#define EXTERNAL_SOUND_SERVER

extern void qt_qws_set_max_window_rect(const QRect& r);

QWSServer Q_GUI_EXPORT *qwsServer=0;
static QWSServerPrivate *qwsServerPrivate=0;

QWSScreenSaver::~QWSScreenSaver()
{
}

extern QByteArray qws_display_spec;
extern void qt_init_display(); //qapplication_qws.cpp
extern QString qws_qtePipeFilename();

extern void qt_client_enqueue(const QWSEvent *); //qapplication_qws.cpp
typedef void MoveRegionF(const QWSRegionMoveCommand*);
typedef void RequestRegionF(int, QRegion);
typedef void SetAltitudeF(const QWSChangeAltitudeCommand*);
extern QList<QWSCommand*> *qt_get_server_queue();

static QRect maxwindow_rect;
static const char *defaultMouse =
#if defined(QT_QWS_CASSIOPEIA) || defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX) || defined(QT_QWS_YOPY) || defined(QWS_CUSTOMTOUCHPANEL)
    "TPanel"
#else
    "Auto"
#endif
#if defined(QT_QWS_CASSIOPEIA)
    "/dev/tpanel"
#endif
    ;
static const char *defaultKeyboard = "TTY";
static int qws_keyModifiers = 0;

static QWSWindow *keyboardGrabber;
static bool keyboardGrabbing;

static int get_object_id(int count = 1)
{
    static int next=1000;
    int n = next;
    next += count;
    return n;
}
#ifndef QT_NO_QWS_INPUTMETHODS
static QWSInputMethod *current_IM = 0;

static QWSWindow *current_IM_composing_win = 0;
static int current_IM_winId = -1;
static bool force_reject_strokeIM = false;
#endif


//#define QWS_REGION_DEBUG

/*!
    \class QWSScreenSaver
    \ingroup qws
*/

/*!
    \class QWSWindow qwindowsystem_qws.h
    \brief The QWSWindow class provides server-specific functionality in Qtopia Core.

    \ingroup qws

    When you run a Qtopia Core application, it either runs as a server
    or connects to an existing server. If it runs as a server, some
    additional functionality is provided by the QWSServer class.

    This class maintains information about each window and allows
    operations to be performed on the windows.

    You can get the window's name(), caption() and winId(), along with
    the client() that owns the window.

    The region the window wants to draw on is returned by requestedRegion().

    The visibility of the window can be determined using
    isVisible(). Visibility can be changed using raise(), lower(), show(),
    hide() and setActiveWindow().
*/

/*!
    \fn int QWSWindow::winId() const

    Returns the window's Id.
*/

/*!
    \fn const QString &QWSWindow::name() const

    Returns the window's name.
*/

/*!
    \fn const QString &QWSWindow::caption() const

    Returns the window's caption.
*/

/*!
    \fn QWSClient* QWSWindow::client() const

    Returns the QWSClient that owns this window.
*/

/*!
    \fn QRegion QWSWindow::requestedRegion() const

    Returns the region that the window has requested to draw onto,
    including any window decorations.
    \omit
    \sa allocatedRegion()
    \endomit
*/

/* NO DOC
    \fn QRegion QWSWindow::allocatedRegion() const

    Returns the region that the window is allowed to draw onto,
    including any window decorations but excluding regions covered by
    other windows.

    \sa requestedRegion()
*/

/*!
    \fn bool QWSWindow::isVisible() const

    Returns true if the window is visible; otherwise returns false.
*/

/* NO DOC
    \fn bool QWSWindow::isPartiallyObscured() const

    Returns true if the window is partially obsured by another window
    or by the bounds of the screen; otherwise returns false.
*/

/*!
    \fn bool QWSWindow::isFullyObscured() const

    Returns true if the window is completely obsured by another window
    or by the bounds of the screen; otherwise returns false.
*/

QWSWindow::QWSWindow(int i, QWSClient* client)
        : id(i), modified(false),
          onTop(false), c(client), last_focus_time(0), _opacity(255), opaque(true), d(0)
{
    _backingStore = new QWSBackingStore;
}

/*!
    Raises the window above all other windows except "Stay on top" windows.
*/
void QWSWindow::raise()
{
    qwsServerPrivate->raiseWindow(this);
}

/*!
    Lowers the window below other windows.
*/
void QWSWindow::lower()
{
    qwsServerPrivate->lowerWindow(this);
}

/*!
    Shows the window.
*/
void QWSWindow::show()
{
    operation(QWSWindowOperationEvent::Show);
}

/*!
    Hides the window.
*/
void QWSWindow::hide()
{
    operation(QWSWindowOperationEvent::Hide);
}

/*!
    Make this the active window (i.e. sets the keyboard focus to this
    window).
*/
void QWSWindow::setActiveWindow()
{
    qwsServerPrivate->setFocus(this, true);
}

void QWSWindow::setName(const QString &n)
{
    rgnName = n;
}

/*!
  Sets the window's caption to \a c.
*/
void QWSWindow::setCaption(const QString &c)
{
    rgnCaption = c;
}


static int global_focus_time_counter=100;

void QWSWindow::focus(bool get)
{
    if (get)
        last_focus_time = global_focus_time_counter++;
    QWSFocusEvent event;
    event.simpleData.window = id;
    event.simpleData.get_focus = get;
    c->sendEvent(&event);
}

void QWSWindow::operation(QWSWindowOperationEvent::Operation o)
{
    QWSWindowOperationEvent event;
    event.simpleData.window = id;
    event.simpleData.op = o;
    c->sendEvent(&event);
}

/*!
    Destructor.
*/
QWSWindow::~QWSWindow()
{
#ifndef QT_NO_QWS_INPUTMETHODS
    if (current_IM_composing_win == this)
        current_IM_composing_win = 0;
#endif
    delete _backingStore;
}


/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

class QWSClientPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWSClient)

public:
    QWSClientPrivate();
    ~QWSClientPrivate();

    void setLockId(int id);
    void unlockCommunication();

private:
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSLock *clientLock;
#endif
    friend class QWSServerPrivate;
};

QWSClientPrivate::QWSClientPrivate()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    clientLock = 0;
#endif
}

QWSClientPrivate::~QWSClientPrivate()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    delete clientLock;
#endif
}

void QWSClientPrivate::setLockId(int id)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    clientLock = new QWSLock(id);
#endif
}

void QWSClientPrivate::unlockCommunication()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (clientLock)
        clientLock->unlock(QWSLock::Communication);
#endif
}

/*!
    \class QWSClient
    \ingroup qws
*/

//always use frame buffer
QWSClient::QWSClient(QObject* parent, QWS_SOCK_BASE* sock, int id)
    : QObject(*new QWSClientPrivate, parent), command(0), cid(id)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    csocket = 0;
    if (!sock) {
        socketDescriptor = -1;
        isClosed = false;
    } else {
        csocket = static_cast<QWSSocket*>(sock); //###
        isClosed = false;

        csocket->flush();
        socketDescriptor = csocket->socketDescriptor();
        connect(csocket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
        connect(csocket, SIGNAL(disconnected()), this, SLOT(closeHandler()));
        connect(csocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorHandler()));
    }
#else
    isClosed = false;
#endif //QT_NO_QWS_MULTIPROCESS
}

QWSClient::~QWSClient()
{
    qDeleteAll(cursors);
    delete command;
}

void QWSClient::setIdentity(const QString& i)
{
    id = i;
}

void QWSClient::closeHandler()
{
    isClosed = true;
    emit connectionClosed();
}

void QWSClient::errorHandler()
{
#ifdef QWS_SOCKET_DEBUG
    qDebug("Client %p error %s", this, csocket ? csocket->errorString().toLatin1().constData() : "(no socket)");
#endif
    isClosed = true;
//####Do we need to clean out the pipes?

    emit connectionClosed();
}


int QWSClient::socket() const
{
    return socketDescriptor;
}

void QWSClient::sendEvent(QWSEvent* event)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (csocket) {
        // qDebug() << "QWSClient::sendEvent type " << event->type << " socket state " << csocket->state();
        if ((QAbstractSocket::SocketState)(csocket->state()) == QAbstractSocket::ConnectedState) {
            event->write(csocket);
        }
    }
    else
#endif
    {
        qt_client_enqueue(event);
    }
}


void QWSClient::sendRegionEvent(int winid, QRegion rgn, int type)
{
    QWSRegionEvent event;
    event.simpleData.window = winid;
    event.simpleData.nrectangles = rgn.rects().count();
    event.simpleData.type = type;
    QVector<QRect> rects = rgn.rects();
    event.setData((char *)rects.constData(),
                    rects.count() * sizeof(QRect), false);

//    qDebug() << "Sending Region event to" << winid << "rgn" << rgn << "type" << type;

    sendEvent(&event);
}

extern int qt_servershmid;

void QWSClient::sendConnectedEvent(const char *display_spec)
{
    QWSConnectedEvent event;
    event.simpleData.window = 0;
    event.simpleData.len = strlen(display_spec) + 1;
    event.simpleData.clientId = cid;
    event.simpleData.servershmid = qt_servershmid;
    char * tmp=(char *)display_spec;
    event.setData(tmp, event.simpleData.len);
    sendEvent(&event);
}

void QWSClient::sendMaxWindowRectEvent()
{
    QWSMaxWindowRectEvent event;
    event.simpleData.window = 0;
    event.simpleData.rect = maxwindow_rect;
    sendEvent(&event);
}


#ifndef QT_NO_QWS_PROPERTIES
void QWSClient::sendPropertyNotifyEvent(int property, int state)
{
    QWSPropertyNotifyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.state = state;
    sendEvent(&event);
}

void QWSClient::sendPropertyReplyEvent(int property, int len, char *data)
{
    QWSPropertyReplyEvent event;
    event.simpleData.window = 0; // not used yet
    event.simpleData.property = property;
    event.simpleData.len = len;
    event.setData(data, len);
    sendEvent(&event);
}
#endif //QT_NO_QWS_PROPERTIES
void QWSClient::sendSelectionClearEvent(int windowid)
{
    QWSSelectionClearEvent event;
    event.simpleData.window = windowid;
    sendEvent(&event);
}

void QWSClient::sendSelectionRequestEvent(QWSConvertSelectionCommand *cmd, int windowid)
{
    QWSSelectionRequestEvent event;
    event.simpleData.window = windowid;
    event.simpleData.requestor = cmd->simpleData.requestor;
    event.simpleData.property = cmd->simpleData.selection;
    event.simpleData.mimeTypes = cmd->simpleData.mimeTypes;
    sendEvent(&event);
}


/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

/*!
    \class QWSServer qwindowsystem_qws.h
    \brief The QWSServer class provides server-specific functionality in Qtopia Core.

    \ingroup qws

    When you run a Qtopia Core application, it either runs as a server
    or connects to an existing server. If it runs as a server, some
    additional operations are provided by this class.

    This class is instantiated by QApplication for Qtopia Core server
    processes. You should never construct this class yourself.

    A pointer to the QWSServer instance can be obtained via the global
    \c qwsServer variable.

    The mouse and keyboard devices can be opened with openMouse() and
    openKeyboard(). (Close them with closeMouse() and
    closeKeyboard().)

    The display is refreshed with refresh(), and painting can be
    enabled or disabled with enablePainting().

    Obtain the list of client windows with clientWindows() and find
    out which window is at a particular point with windowAt().

    Many static functions are provided, for example,
    setKeyboardFilter(), setKeyboardHandler(), setDefaultKeyboard()
    and setDefaultMouse().

    The size of the window rectangle can be set with
    setMaxWindowRect(), and the desktop's background can be set with
    setDesktopBackground().

    The screen saver is controlled with setScreenSaverInterval() and
    screenSaverActivate().
*/

/*!
    \enum QWSServer::IMState

    \internal

    \value InputMethodPreedit Composing.
    \value InputMethodCommit Finished composing.
    \omitvalue InputMethodCommitToPrev
*/

/*!
    \fn void QWSServer::markedText(const QString &text)

    This signal is emitted whenever some text is selected. The
    selection is passed in \a text.
*/

/*!
    \enum QWSServer::IMMouse

    \internal
*/

/*!
    \fn void QWSServer::setOverrideKeys(const KeyOverride *keyOveride)

    \internal

    Sets the override keys to \a keyOveride.
*/

/*!
    \fn const QList<QWSWindow*> &QWSServer::clientWindows()

    Returns the list of top-level windows. This list will change as
    applications add and remove widgets so it should not be stored for
    future use. The windows are sorted in stacking order from
    top-most to bottom-most.
*/

/*!
    \fn void QWSServer::newChannel(const QString& channel)

    This signal is emitted when the QCopChannel \a channel is created.
*/

/*!
    \fn void QWSServer::removedChannel(const QString& channel)

    This signal is emitted immediately after the QCopChannel \a
    channel is destroyed. Note that a channel is not destroyed until
    all its listeners have unregistered.
*/

/*!
    \fn QWSServer::QWSServer(int flags, QObject *parent)

    Construct a QWSServer object.

    \warning This class is instantiated by QApplication for
    Qtopia Core server processes. You should never construct this
    class yourself.

    The \a flags are used for keyboard and mouse setting. The server's
    parent is \a parent.
*/

/*!
    \fn static QWSServer* QWSServer::instance()

    Returns a pointer to the application's QWSServer instance. The pointer
    will be 0 if the application is not the window server, e.g.
    QApplication::type() != QApplication::GuiServer.
*/

struct QWSCommandStruct
{
    QWSCommandStruct(QWSCommand *c, QWSClient *cl) :command(c),client(cl){}
    ~QWSCommandStruct() { delete command; }

    QWSCommand *command;
    QWSClient *client;

};

QWSServer::QWSServer(int flags, QObject *parent) :
    QObject(*new QWSServerPrivate, parent)
{
    Q_D(QWSServer);
    d->initServer(flags);
}

#ifdef QT3_SUPPORT
/*!
    Use the two-argument overload and call setObjectName() instead.
*/
QWSServer::QWSServer(int flags, QObject *parent, const char *name) :
    QObject(*new QWSServerPrivate, parent)
{
    Q_D(QWSServer);
    setObjectName(QString::fromAscii(name));
    d->initServer(flags);
}
#endif


static void ignoreSignal(int) {} // Used to eat SIGPIPE signals below

void QWSServerPrivate::initServer(int flags)
{
    Q_Q(QWSServer);
    Q_ASSERT(!qwsServer);
    qwsServer = q;
    qwsServerPrivate = this;
    disablePainting = false;
#ifndef QT_NO_QWS_MULTIPROCESS
    ssocket = new QWSServerSocket(qws_qtePipeFilename(), q);
    QObject::connect(ssocket, SIGNAL(newConnection()), q, SLOT(newConnection()));

    if ( !ssocket->isListening()) {
        perror("Error");
        qFatal("Failed to bind to %s", qws_qtePipeFilename().toLatin1().constData());
    }

    struct linger tmp;
    tmp.l_onoff=1;
    tmp.l_linger=0;
    setsockopt(ssocket->socketDescriptor(),SOL_SOCKET,SO_LINGER,(char *)&tmp,sizeof(tmp));


    signal(SIGPIPE, ignoreSignal); //we get it when we read
#endif
    focusw = 0;
    mouseGrabber = 0;
    mouseGrabbing = false;
    keyboardGrabber = 0;
    keyboardGrabbing = false;
#ifndef QT_NO_QWS_CURSOR
    haveviscurs = false;
    cursor = 0;
    nextCursor = 0;
#endif

#ifndef QT_NO_QWS_MULTIPROCESS

    if (!geteuid()) {
#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_DARWIN)
        if(mount(0,"/var/shm", "shm", 0, 0)) {
            /* This just confuses people with 2.2 kernels
            if (errno != EBUSY)
                qDebug("Failed mounting shm fs on /var/shm: %s",strerror(errno));
            */
        }
#endif
    }
#endif

    // no selection yet
    selectionOwner.windowid = -1;
    selectionOwner.time.set(-1, -1, -1, -1);

    openDisplay();

    screensavertimer = new QTimer(q);
    screensavertimer->setSingleShot(true);
    QObject::connect(screensavertimer, SIGNAL(timeout()), q, SLOT(screenSaverTimeout()));
    screenSaverWake();

    clientMap[-1] = new QWSClient(q, 0, 0);

    if (!bgBrush)
        bgBrush = new QBrush(QColor(0x20, 0xb0, 0x50));

    initializeCursor();

    qt_screen->exposeRegion(QRect(0,0,qt_screen->width(), qt_screen->height()), 0);

    // input devices
    if (!(flags&QWSServer::DisableMouse)) {
        q->openMouse();
    }
#ifndef QT_NO_QWS_KEYBOARD
    if (!(flags&QWSServer::DisableKeyboard)) {
        q->openKeyboard();
    }
#endif

#if !defined(QT_NO_SOUND) && !defined(EXTERNAL_SOUND_SERVER) && !defined(Q_OS_DARWIN)
    soundserver = new QWSSoundServer(q);
#endif
}

/*!
    Destruct QWSServer
*/
QWSServer::~QWSServer()
{
    closeMouse();
#ifndef QT_NO_QWS_KEYBOARD
    closeKeyboard();
#endif
}


const QList<QWSWindow*> &QWSServer::clientWindows()
{
    Q_D(QWSServer);
    return d->windows;
}

/*!
  \internal
*/
void QWSServerPrivate::releaseMouse(QWSWindow* w)
{
    if (w && mouseGrabber == w) {
        mouseGrabber = 0;
        mouseGrabbing = false;
#ifndef QT_NO_QWS_CURSOR
        if (nextCursor) {
            // Not grabbing -> set the correct cursor
            setCursor(nextCursor);
            nextCursor = 0;
        }
#endif
    }
}

/*!
  \internal
*/
void QWSServerPrivate::releaseKeyboard(QWSWindow* w)
{
    if (keyboardGrabber == w) {
        keyboardGrabber = 0;
        keyboardGrabbing = false;
    }
}

void QWSServerPrivate::handleWindowClose(QWSWindow *w)
{
    w->shuttingDown();
    if (focusw == w)
        setFocus(w,false);
    if (mouseGrabber == w)
        releaseMouse(w);
    if (keyboardGrabber == w)
        releaseKeyboard(w);
}


#ifndef QT_NO_QWS_MULTIPROCESS
/*!
  \internal
*/
void QWSServerPrivate::newConnection()
{
    Q_Q(QWSServer);
    while (QWS_SOCK_BASE *sock = ssocket->nextPendingConnection()) {
        int socket = sock->socketDescriptor();

        QWSClient *client = new QWSClient(q,sock, get_object_id());
        clientMap[socket] = client;

#ifndef QT_NO_SXV
#ifdef QTRANSPORTAUTH_DEBUG
        qDebug( "Transport auth connected: unix stream socket %d", socket );
#endif
        // get a handle to the per-process authentication service
        QTransportAuth *a = QTransportAuth::getInstance();

        // assert that this transport is trusted
        QTransportAuth::Data *d = a->connectTransport(
                QTransportAuth::UnixStreamSock |
                QTransportAuth::Trusted, socket );

        QAuthDevice *ad = a->recvBuf( d, sock );
        ad->setClient(client);

        QObject::connect(ad, SIGNAL(readyRead()),
                q, SLOT(doClient()));

        QObject::connect(client, SIGNAL(connectionClosed()),
                q, SLOT(clientClosed()));
#else
        QObject::connect(client, SIGNAL(readyRead()),
                         q, SLOT(doClient()));
        QObject::connect(client, SIGNAL(connectionClosed()),
                         q, SLOT(clientClosed()));
#endif // QT_NO_SXV

        client->sendConnectedEvent(qws_display_spec.constData());

        if (!maxwindow_rect.isEmpty() && clientMap.contains(socket))
            client->sendMaxWindowRectEvent();

        // pre-provide some object id's
        QWSCreateCommand cmd(30);
        invokeCreate(&cmd, client);
    }
}
/*!
  \internal
*/
void QWSServerPrivate::clientClosed()
{
    Q_Q(QWSServer);
    QWSClient* cl = (QWSClient*)q->sender();

    // Remove any queued commands for this client
    int i = 0;
    while (i < commandQueue.size()) {
        QWSCommandStruct *cs = commandQueue.at(i);
        if (cs->client == cl) {
            commandQueue.removeAt(i);
            delete cs;
        } else {
            ++i;
        }
    }

#ifndef QT_NO_COP
    // Enfore unsubscription from all channels.
    QCopChannel::detach(cl);
#endif

    // Shut down all windows for this client
    for (int i = 0; i < windows.size(); ++i) {
        QWSWindow* w = windows.at(i);
        if (w->forClient(cl))
            w->shuttingDown();
    }

    // Delete all windows for this client
    QRegion exposed;
    i = 0;
    while (i < windows.size()) {
        QWSWindow* w = windows.at(i);
        if (w->forClient(cl)) {
            w->c = 0; //so we don't send events to it anymore
            releaseMouse(w);
            releaseKeyboard(w);
            exposed += w->requestedRegion(); //### too much, but how often do we do this...
//                rgnMan->remove(w->allocationIndex());
            if (focusw == w)
                setFocus(focusw,0);
            if (mouseGrabber == w)
                releaseMouse(w);
            windows.takeAt(i);
            if (i < nReserved)
                --nReserved;
#ifndef QT_NO_QWS_PROPERTIES
            propertyManager.removeProperties(w->winId());
#endif
            emit q->windowEvent(w, QWSServer::Destroy);
            deletedWindows.append(w);
        } else {
            ++i;
        }
    }
    if (deletedWindows.count())
        QTimer::singleShot(0, q, SLOT(deleteWindowsLater()));

    //qDebug("removing client %d with socket %d", cl->clientId(), cl->socket());
    clientMap.remove(cl->socket());
    if (cl == cursorClient)
        cursorClient = 0;
    if (qt_screen->clearCacheFunc)
        (qt_screen->clearCacheFunc)(qt_screen, cl->clientId());  // remove any remaining cache entries.
    cl->deleteLater();

    exposeRegion(exposed);
//    syncRegions();
}

void QWSServerPrivate::deleteWindowsLater()
{
    qDeleteAll(deletedWindows);
    deletedWindows.clear();
}

#endif //QT_NO_QWS_MULTIPROCESS

QWSCommand* QWSClient::readMoreCommand()
{
    QIODevice *socket = 0;
#ifndef QT_NO_SXV
    if (socketDescriptor != -1)  // not server socket
        socket = QTransportAuth::getInstance()->passThroughByClient( this );
#if QTRANSPORTAUTH_DEBUG
    if (socket) {
        char displaybuf[1024];
        qint64 bytes = socket->bytesAvailable();
        if ( bytes > 511 ) bytes = 511;
        hexstring( displaybuf, ((unsigned char *)(reinterpret_cast<QAuthDevice*>(socket)->buffer().constData())), bytes );
        qDebug( "readMoreCommand: %lli bytes - %s", socket->bytesAvailable(), displaybuf );
    }
#endif
#endif // QT_NO_SXV

#ifndef QT_NO_QWS_MULTIPROCESS
    if (!socket)
        socket = csocket;   // server socket
    if (socket) {
        // read next command
        if (!command) {
            int command_type = qws_read_uint(socket);

            if (command_type >= 0)
                command = QWSCommand::factory(command_type);
        }
        if (command) {
            if (command->read(socket)) {
                // Finished reading a whole command.
                QWSCommand* result = command;
                command = 0;
                return result;
            }
        }

        // Not finished reading a whole command.
        return 0;
    } else
#endif // QT_NO_QWS_MULTIPROCESS
    {
        QList<QWSCommand*> *serverQueue = qt_get_server_queue();
        return serverQueue->isEmpty() ? 0 : serverQueue->takeFirst();
    }
}


/*!
  \internal
*/
void QWSServer::processEventQueue()
{
    if (qwsServerPrivate)
        qwsServerPrivate->doClient(qwsServerPrivate->clientMap.value(-1));
}


#ifndef QT_NO_QWS_MULTIPROCESS
void QWSServerPrivate::doClient()
{
    Q_Q(QWSServer);
    static bool active = false;
    if (active) {
        qDebug("QWSServer::doClient() reentrant call, ignoring");
        return;
    }
    active = true;

    QWSClient* client;
#ifndef QT_NO_SXV
    QAuthDevice *ad = qobject_cast<QAuthDevice*>(q->sender());
    if (ad)
        client = ad->client();
    else
#endif
        client = (QWSClient*)q->sender();
    doClient(client);
    active = false;
}
#endif // QT_NO_QWS_MULTIPROCESS

void QWSServerPrivate::doClient(QWSClient *client)
{
    QWSCommand* command=client->readMoreCommand();

    while (command) {
        QWSCommandStruct *cs = new QWSCommandStruct(command, client);
        commandQueue.append(cs);
        // Try for some more...
        command=client->readMoreCommand();
    }

    while (!commandQueue.isEmpty()) {
        QWSCommandStruct *cs = commandQueue.takeAt(0);
        switch (cs->command->type) {
        case QWSCommand::Identify:
            invokeIdentify((QWSIdentifyCommand*)cs->command, cs->client);
            break;
        case QWSCommand::Create:
            invokeCreate((QWSCreateCommand*)cs->command, cs->client);
            break;
        case QWSCommand::RegionName:
            invokeRegionName((QWSRegionNameCommand*)cs->command, cs->client);
            break;
        case QWSCommand::Region:
            invokeRegion((QWSRegionCommand*)cs->command, cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
        case QWSCommand::RegionMove:
            invokeRegionMove((QWSRegionMoveCommand*)cs->command, cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
        case QWSCommand::RegionDestroy:
            invokeRegionDestroy((QWSRegionDestroyCommand*)cs->command, cs->client);
            break;
#ifndef QT_NO_QWS_PROPERTIES
        case QWSCommand::AddProperty:
            invokeAddProperty((QWSAddPropertyCommand*)cs->command);
            break;
        case QWSCommand::SetProperty:
            invokeSetProperty((QWSSetPropertyCommand*)cs->command);
            break;
        case QWSCommand::RemoveProperty:
            invokeRemoveProperty((QWSRemovePropertyCommand*)cs->command);
            break;
        case QWSCommand::GetProperty:
            invokeGetProperty((QWSGetPropertyCommand*)cs->command, cs->client);
            break;
#endif
        case QWSCommand::SetSelectionOwner:
            invokeSetSelectionOwner((QWSSetSelectionOwnerCommand*)cs->command);
            break;
        case QWSCommand::RequestFocus:
            invokeSetFocus((QWSRequestFocusCommand*)cs->command, cs->client);
            break;
        case QWSCommand::ChangeAltitude:
            invokeSetAltitude((QWSChangeAltitudeCommand*)cs->command,
                               cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
        case QWSCommand::SetOpacity:
            invokeSetOpacity((QWSSetOpacityCommand*)cs->command,
                               cs->client);
            break;

#ifndef QT_NO_QWS_CURSOR
        case QWSCommand::DefineCursor:
            invokeDefineCursor((QWSDefineCursorCommand*)cs->command, cs->client);
            break;
        case QWSCommand::SelectCursor:
            invokeSelectCursor((QWSSelectCursorCommand*)cs->command, cs->client);
            break;
        case QWSCommand::PositionCursor:
            invokePositionCursor((QWSPositionCursorCommand*)cs->command, cs->client);
            break;
#endif
        case QWSCommand::GrabMouse:
            invokeGrabMouse((QWSGrabMouseCommand*)cs->command, cs->client);
            break;
        case QWSCommand::GrabKeyboard:
            invokeGrabKeyboard((QWSGrabKeyboardCommand*)cs->command, cs->client);
            break;
#if !defined(QT_NO_SOUND) && !defined(Q_OS_DARWIN)
        case QWSCommand::PlaySound:
            invokePlaySound((QWSPlaySoundCommand*)cs->command, cs->client);
            break;
#endif
#ifndef QT_NO_COP
        case QWSCommand::QCopRegisterChannel:
            invokeRegisterChannel((QWSQCopRegisterChannelCommand*)cs->command,
                                   cs->client);
            break;
        case QWSCommand::QCopSend:
            invokeQCopSend((QWSQCopSendCommand*)cs->command, cs->client);
            break;
#endif
#ifndef QT_NO_QWS_INPUTMETHODS
        case QWSCommand::IMUpdate:
            invokeIMUpdate((QWSIMUpdateCommand*)cs->command, cs->client);
            break;
        case QWSCommand::IMResponse:
            invokeIMResponse((QWSIMResponseCommand*)cs->command, cs->client);
            break;
        case QWSCommand::IMMouse:
            {
                if (current_IM) {
                    QWSIMMouseCommand *cmd = (QWSIMMouseCommand *) cs->command;
                    current_IM->mouseHandler(cmd->simpleData.index,
                                              cmd->simpleData.state);
                }
            }
            break;
#endif
        case QWSCommand::RepaintRegion:
            invokeRepaintRegion((QWSRepaintRegionCommand*)cs->command,
                                cs->client);
            cs->client->d_func()->unlockCommunication();
            break;
        }
        delete cs;
    }
}


void QWSServerPrivate::showCursor()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->show();
#endif
}

void QWSServerPrivate::hideCursor()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
}

/*!
    If \a e is true, painting on the display is enabled; if \a e is
    false, painting is disabled.
*/
void QWSServer::enablePainting(bool e)
{
    Q_D(QWSServer);
    // ### don't like this
    if (e)
    {
        d->disablePainting = false;
        d->setWindowRegion(0, QRegion());
        d->showCursor();
    }
    else
    {
        d->disablePainting = true;
        d->hideCursor();
        d->setWindowRegion(0, QRegion(0,0,d->swidth,d->sheight));
    }
}

/*!
    Refreshes the entire display.
*/
void QWSServer::refresh()
{
    Q_D(QWSServer);
    d->exposeRegion(QRegion(0, 0, d->swidth, d->sheight));
//    syncRegions();
}

/*!
    \overload

    Refreshes the region \a r.
*/
void QWSServer::refresh(QRegion & r)
{
    Q_D(QWSServer);
    d->exposeRegion(r);
//    syncRegions();
}

/*!
    Sets the area of the screen which Qtopia Core applications will
    consider to be the maximum area to use for windows to \a r.

    \sa QWidget::showMaximized()
*/
void QWSServer::setMaxWindowRect(const QRect& r)
{
    QRect tr = qt_screen->mapToDevice(r,
        QSize(qt_screen->width(),qt_screen->height()));
    if (maxwindow_rect != tr) {
        maxwindow_rect = tr;
        qwsServerPrivate->sendMaxWindowRectEvents();
    }
}

/*!
  \internal
*/
void QWSServerPrivate::sendMaxWindowRectEvents()
{
    for (ClientIterator it = clientMap.begin(); it != clientMap.end(); ++it)
        (*it)->sendMaxWindowRectEvent();
}

/*!
    Set the mouse driver \a m to use if \c $QWS_MOUSE_PROTO is not
    defined. The default is platform-dependent.
*/
void QWSServer::setDefaultMouse(const char *m)
{
    defaultMouse = m;
}

/*!
    Set the keyboard driver to \a k, e.g. if \c $QWS_KEYBOARD is not
    defined. The default is platform-dependent.
*/
void QWSServer::setDefaultKeyboard(const char *k)
{
    defaultKeyboard = k;
}

#ifndef QT_NO_QWS_CURSOR
static bool prevWin;
#endif


extern int *qt_last_x,*qt_last_y;


/*!
  \internal

  Send a mouse event. \a pos is the screen position where the mouse
  event occurred and \a state is a mask indicating which buttons are
  pressed.
*/
void QWSServer::sendMouseEvent(const QPoint& pos, int state, int wheel)
{
    //const int btnMask = Qt::LeftButton | Qt::RightButton | Qt::MidButton;
    qwsServerPrivate->showCursor();

    if (state)
        qwsServerPrivate->screenSaverWake();


    QPoint tpos;
    // transformations
    if (qt_screen->isTransformed()) {
	QSize s = QSize(qt_screen->deviceWidth(), qt_screen->deviceHeight());
	tpos = qt_screen->mapFromDevice(pos, s);
    } else {
	tpos = pos;
    }
#ifndef QT_NO_QWS_INPUTMETHODS
    const int btnMask = Qt::LeftButton | Qt::RightButton | Qt::MidButton;
    int stroke_count; // number of strokes to keep shown.
    if (force_reject_strokeIM || qwsServerPrivate->mouseGrabber
	    || !current_IM) {
	stroke_count = 0;
    } else {
	stroke_count = current_IM->filter(tpos, state, wheel);
    }

    if (stroke_count == 0) {
	if (state&btnMask)
	    force_reject_strokeIM = true;
        QWSServerPrivate::sendMouseEventUnfiltered(pos, state);
    }
    // stop force reject after stroke ends.
    if (state&btnMask && force_reject_strokeIM)
	force_reject_strokeIM = false;
    // on end of stroke, force_rejct
    // and once a stroke is rejected, do not try again till pen is lifted
#else
    QWSServerPrivate::sendMouseEventUnfiltered(tpos, state, wheel);
#endif // end QT_NO_QWS_FSIM
}

void QWSServerPrivate::sendMouseEventUnfiltered(const QPoint &pos, int state, int wheel)
{
    const int btnMask = Qt::LeftButton | Qt::RightButton | Qt::MidButton;
    if (qt_last_x) {
         *qt_last_x = pos.x();
         *qt_last_y = pos.y();
    }
    QWSServer::mousePosition = pos;
    qwsServerPrivate->mouseState = state;

    QWSMouseEvent event;

    //If grabbing window disappears, grab is still active until
    //after mouse release.
    QWSWindow *win = qwsServerPrivate->mouseGrabber ? qwsServerPrivate->mouseGrabber : qwsServer->windowAt(pos);
    event.simpleData.window = win ? win->id : 0;

#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->move(pos.x(),pos.y());

    // Arrow cursor over desktop
    // prevWin remembers if the last event was over a window
    if (!win && prevWin) {
        if (!qwsServerPrivate->mouseGrabber)
            qwsServerPrivate->setCursor(QWSCursor::systemCursor(Qt::ArrowCursor));
        else
            qwsServerPrivate->nextCursor = QWSCursor::systemCursor(Qt::ArrowCursor);
        prevWin = false;
    }
    // reset prevWin
    if (win && !prevWin)
        prevWin = true;
#endif

    if ((state&btnMask) && !qwsServerPrivate->mouseGrabbing) {
        qwsServerPrivate->mouseGrabber = win;
    }

    event.simpleData.x_root=pos.x();
    event.simpleData.y_root=pos.y();
    event.simpleData.state=state | qws_keyModifiers;
    event.simpleData.delta = wheel;
    event.simpleData.time=qwsServerPrivate->timer.elapsed();

    QWSClient *serverClient = qwsServerPrivate->clientMap.value(-1);
    QWSClient *winClient = win ? win->client() : 0;

#ifndef QT_NO_QWS_INPUTMETHODS
    //tell the input method if we click on a different window that is not IM transparent

    static int oldstate = 0;
    bool isPress = state > oldstate;
    oldstate = state;
    if (isPress && current_IM && current_IM_winId != -1) {
        QWSWindow *kbw = keyboardGrabber ? keyboardGrabber :
                         qwsServerPrivate->focusw;

        QWidget *target = winClient == serverClient ?
                          QApplication::widgetAt(pos) : 0;
        if (kbw != win && (!target || !(target->testAttribute(Qt::WA_InputMethodTransparent))))
            current_IM->mouseHandler(-1, QWSServer::MouseOutside);
    }
#endif

    if (serverClient)
       serverClient->sendEvent(&event);
    if (winClient && winClient != serverClient)
       winClient->sendEvent(&event);

    // Make sure that if we leave a window, that window gets one last mouse
    // event so that it knows the mouse has left.
    QWSClient *oldClient = qwsServer->d_func()->cursorClient;
    if (oldClient && oldClient != winClient && oldClient != serverClient)
        oldClient->sendEvent(&event);

    qwsServer->d_func()->cursorClient = winClient;

    if (!(state&btnMask) && !qwsServerPrivate->mouseGrabbing)
        qwsServerPrivate->releaseMouse(qwsServerPrivate->mouseGrabber);
}

/*!
    Returns the primary mouse handler.
*/
QWSMouseHandler *QWSServer::mouseHandler()
{
    if (qwsServerPrivate->mousehandlers.empty())
        return 0;
    return qwsServerPrivate->mousehandlers.first();
}

// called by QWSMouseHandler constructor, not user code.
/*!
  \internal
*/
void QWSServer::setMouseHandler(QWSMouseHandler* mh)
{
    qwsServerPrivate->mousehandlers.prepend(mh);
}

/*!
  \internal

  Caller owns data in list, and must delete contents
*/
QList<QWSInternalWindowInfo*> * QWSServer::windowList()
{
    QList<QWSInternalWindowInfo*> * ret=new QList<QWSInternalWindowInfo*>;
    for (int i=0; i < qwsServerPrivate->windows.size(); ++i) {
        QWSWindow *window = qwsServerPrivate->windows.at(i);
        QWSInternalWindowInfo * qwi=new QWSInternalWindowInfo();
        qwi->winid=window->winId();
        qwi->clientid=window->client()->clientId();
#ifndef QT_NO_QWS_PROPERTIES
        char * name;
        int len;
        qwsServerPrivate->propertyManager.getProperty(qwi->winid,
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
void QWSServerPrivate::sendQCopEvent(QWSClient *c, const QString &ch,
                               const QString &msg, const QByteArray &data,
                               bool response)
{
    Q_ASSERT(c);

    QWSQCopMessageEvent event;
    event.simpleData.is_response = response;
    event.simpleData.lchannel = ch.length();
    event.simpleData.lmessage = msg.length();
    event.simpleData.ldata = data.size();
    int l = event.simpleData.lchannel + event.simpleData.lmessage +
            event.simpleData.ldata;

    // combine channel, message and data into one block of raw bytes
    QByteArray raw(l, 0);
    char *d = (char*)raw.data();
    memcpy(d, ch.toLatin1().constData(), event.simpleData.lchannel);
    d += event.simpleData.lchannel;
    memcpy(d, msg.toLatin1().constData(), event.simpleData.lmessage);
    d += event.simpleData.lmessage;
    memcpy(d, data.data(), event.simpleData.ldata);

    event.setData(raw.data(), l);

    c->sendEvent(&event);
}
#endif

/*!
    Returns the window containing the point \a pos or 0 if there is no
    window under the point.
*/
QWSWindow *QWSServer::windowAt(const QPoint& pos)
{
    Q_D(QWSServer);
    for (int i=0; i<d->windows.size(); ++i) {
        QWSWindow* w = d->windows.at(i);
        if (w->requested_region.contains(pos))
            return w;
    }
    return 0;
}

static int keyUnicode(int keycode)
{
    int code = 0xffff;

    if (keycode >= Qt::Key_A && keycode <= Qt::Key_Z)
        code = keycode - Qt::Key_A + 'a';
    else if (keycode >= Qt::Key_0 && keycode <= Qt::Key_9)
        code = keycode - Qt::Key_0 + '0';

    return code;
}

/*!
    Send a key event. You can use this to send key events generated by
    "virtual keyboards".

    \table
    \header \o Parameter \o Description
    \row
        \o \a unicode
        \o The unicode value of the key to send
    \row
        \o \a keycode
        \o The Qt keycode value as defined by the Qt::Key enum.
    \row
        \o \a modifiers
        \o An OR combination of Qt::KeyboardModifier values, indicating whether
            Shift/Alt/Ctrl keys are pressed.
    \row
        \o \a isPress
        \o True if this is a key down event; otherwise false.
    \row
        \o \a autoRepeat
        \o True if this event is caused by auto repeat (i.e. the
            user has held the key down and this is the second or subsequent
            key event being sent); otherwise false.
    \endtable
*/
void QWSServer::sendKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                             bool isPress, bool autoRepeat)
{
    qws_keyModifiers = modifiers;

    if (isPress) {
        if (keycode != Qt::Key_F34 && keycode != Qt::Key_F35)
            qwsServerPrivate->screenSaverWake();
    }

#ifndef QT_NO_QWS_INPUTMETHODS

    if (!current_IM || !current_IM->filter(unicode, keycode, modifiers, isPress, autoRepeat))
        QWSServerPrivate::sendKeyEventUnfiltered(unicode, keycode, modifiers, isPress, autoRepeat);
#else
    QWSServerPrivate::sendKeyEventUnfiltered(unicode, keycode, modifiers, isPress, autoRepeat);
#endif
}

void QWSServerPrivate::sendKeyEventUnfiltered(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                       bool isPress, bool autoRepeat)
{

    QWSKeyEvent event;
    QWSWindow *win = keyboardGrabber ? keyboardGrabber :
        qwsServerPrivate->focusw;

    event.simpleData.window = win ? win->winId() : 0;

    event.simpleData.unicode =
#ifndef QT_NO_QWS_KEYBOARD
        unicode < 0 ? keyUnicode(keycode) :
#endif
        unicode;
    event.simpleData.keycode = keycode;
    event.simpleData.modifiers = modifiers;
    event.simpleData.is_press = isPress;
    event.simpleData.is_auto_repeat = autoRepeat;

    for (ClientIterator it = qwsServerPrivate->clientMap.begin(); it != qwsServerPrivate->clientMap.end(); ++it)
        (*it)->sendEvent(&event);
}

/*!
    \internal
*/
void QWSServer::beginDisplayReconfigure()
{
    qwsServer->enablePainting(false);
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
    QWSDisplay::grab(true);
    qt_screen->disconnect();
}

/*!
    \internal
*/
void QWSServer::endDisplayReconfigure()
{
    qt_screen->connect(QString());
    qwsServerPrivate->swidth = qt_screen->deviceWidth();
    qwsServerPrivate->sheight = qt_screen->deviceHeight();

    QWSDisplay::ungrab();
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->show();
#endif
    qt_qws_set_max_window_rect(QRect(0, 0, qt_screen->deviceWidth(), qt_screen->deviceHeight()));
    QSize olds = qApp->desktop()->size();
    qApp->desktop()->resize(qt_screen->width(), qt_screen->height());
    qApp->postEvent(qApp->desktop(), new QResizeEvent(qApp->desktop()->size(), olds));
    qwsServer->enablePainting(true);
    qwsServer->refresh();
    qDebug("Desktop size: %dx%d", qApp->desktop()->width(), qApp->desktop()->height());
}

void QWSServerPrivate::resetEngine()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
    qt_screencursor->show();
#endif
}


#ifndef QT_NO_QWS_CURSOR
/*!
    If \a vis is true, makes the cursor visible; if \a vis is false,
    makes the cursor invisible.

    \sa isCursorVisible()
*/
void QWSServer::setCursorVisible(bool vis)
{
    if (qwsServerPrivate && qwsServerPrivate->haveviscurs != vis) {
        QWSCursor* c = qwsServerPrivate->cursor;
        qwsServerPrivate->setCursor(QWSCursor::systemCursor(Qt::BlankCursor));
        qwsServerPrivate->haveviscurs = vis;
        qwsServerPrivate->setCursor(c);
    }
}

/*!
    Returns true if the cursor is visible; otherwise returns false.

    \sa setCursorVisible()
*/
bool QWSServer::isCursorVisible()
{
    return qwsServerPrivate ? qwsServerPrivate->haveviscurs : true;
}
#endif

#ifndef QT_NO_QWS_INPUTMETHODS


/*!
    This function sends the input method event \a ime to the server.

    If there is a window currently in compose mode, the event is sent
    to that window. Otherwise, the event is sent to the current focus window.
*/


void QWSServer::sendIMEvent(const QInputMethodEvent *ime)
{
    QWSIMEvent event;

    QWSWindow *win = keyboardGrabber ? keyboardGrabber :
                     qwsServerPrivate->focusw;

    //if currently composing then event must go to the composing window

    if (current_IM_composing_win)
        win = current_IM_composing_win;

    event.simpleData.window = win ? win->winId() : 0;
    event.simpleData.replaceFrom = ime->replacementStart();;
    event.simpleData.replaceLength = ime->replacementLength();

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);

    out << ime->preeditString();
    out << ime->commitString();

    const QList<QInputMethodEvent::Attribute> &attributes = ime->attributes();
    for (int i = 0; i < attributes.count(); ++i) {
        const QInputMethodEvent::Attribute &a = attributes.at(i);
        out << a.type << a.start << a.length << a.value;
    }
    event.setData(buffer.data(), buffer.size());
    QWSClient *serverClient = qwsServerPrivate->clientMap.value(-1);
    if (serverClient)
        serverClient->sendEvent(&event);
    if (win && win->client() && win->client() != serverClient)
        win->client()->sendEvent(&event);

    current_IM_composing_win = ime->preeditString().isEmpty() ? 0 : win;
    current_IM_winId = win->winId();
}


/*!
  Sends an input method query for the specified \a property.

  You must reimplement the virtual function QWSInputMethod::queryResponse()
  in a subclass of QWSInputMethod if you want to receive responses to
  input method queries.
*/
void QWSServer::sendIMQuery(int property)
{
    QWSIMQueryEvent event;

    QWSWindow *win = keyboardGrabber ? keyboardGrabber :
                     qwsServerPrivate->focusw;
    if (current_IM_composing_win)
        win = current_IM_composing_win;

    event.simpleData.window = win ? win->winId() : 0;
    event.simpleData.property = property;
    if (win && win->client())
        win->client()->sendEvent(&event);
}



/*!
    \internal

    Sets the current input method to \a im.
*/
void QWSServer::setCurrentInputMethod(QWSInputMethod *im)
{
    if (current_IM)
        current_IM->reset(); //??? send an update event instead ?
    current_IM = im;
}

/*!
    \fn static void QWSServer::resetInputMethod()

    \internal
*/

#endif //QT_NO_QWS_INPUTMETHODS

#ifndef QT_NO_QWS_PROPERTIES
/*!
  \internal
*/
void QWSServer::sendPropertyNotifyEvent(int property, int state)
{
    Q_D(QWSServer);
    QWSServerPrivate::ClientIterator it = d->clientMap.begin();
    while (it != d->clientMap.end()) {
        QWSClient *cl = *it;
        ++it;
        cl->sendPropertyNotifyEvent(property, state);
    }
}
#endif

void QWSServerPrivate::invokeIdentify(const QWSIdentifyCommand *cmd, QWSClient *client)
{
    client->setIdentity(cmd->id);
#ifndef QT_NO_QWS_MULTIPROCESS
    if (client->clientId() > 0)
        client->d_func()->setLockId(cmd->simpleData.idLock);
#endif
}

void QWSServerPrivate::invokeCreate(QWSCreateCommand *cmd, QWSClient *client)
{
    QWSCreationEvent event;
    event.simpleData.objectid = get_object_id(cmd->count);
    event.simpleData.count = cmd->count;
    client->sendEvent(&event);
}

void QWSServerPrivate::invokeRegionName(const QWSRegionNameCommand *cmd, QWSClient *client)
{
    Q_Q(QWSServer);
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, client);
    if (changingw) {
        changingw->setName(cmd->name);
        changingw->setCaption(cmd->caption);
        emit q->windowEvent(changingw, QWSServer::Name);
    }
}

void QWSServerPrivate::invokeRegion(QWSRegionCommand *cmd, QWSClient *client)
{
#ifdef QWS_REGION_DEBUG
    qDebug("QWSServer::invokeRegion %d rects (%d)",
            cmd->simpleData.nrectangles, cmd->simpleData.windowid);
#endif

    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, 0);
    if (!changingw) {
        qWarning("Invalid window handle %08x",cmd->simpleData.windowid);
        return;
    }
    if (!changingw->forClient(client)) {
        qWarning("Disabled: clients changing other client's window region");
        return;
    }

    QRegion region;
    region.setRects(cmd->rectangles, cmd->simpleData.nrectangles);

    request_region(cmd->simpleData.windowid, cmd->simpleData.memoryid,
                   cmd->simpleData.windowtype, region, (QImage::Format)cmd->simpleData.imgFormat, changingw);
}

void QWSServerPrivate::invokeRegionMove(const QWSRegionMoveCommand *cmd, QWSClient *client)
{
    Q_Q(QWSServer);
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, 0);
    if (!changingw) {
        qWarning("invokeRegionMove: Invalid window handle %d",cmd->simpleData.windowid);
        return;
    }
    if (!changingw->forClient(client)) {
        qWarning("Disabled: clients changing other client's window region");
        return;
    }

//    changingw->setNeedAck(true);
    moveWindowRegion(changingw, cmd->simpleData.dx, cmd->simpleData.dy);
    emit q->windowEvent(changingw, QWSServer::Geometry);
}

void QWSServerPrivate::invokeRegionDestroy(const QWSRegionDestroyCommand *cmd, QWSClient *client)
{
    Q_Q(QWSServer);
    QWSWindow* changingw = findWindow(cmd->simpleData.windowid, 0);
    if (!changingw) {
        qWarning("invokeRegionDestroy: Invalid window handle %d",cmd->simpleData.windowid);
        return;
    }
    if (!changingw->forClient(client)) {
        qWarning("Disabled: clients changing other client's window region");
        return;
    }

    setWindowRegion(changingw, QRegion());
//    rgnMan->remove(changingw->allocationIndex());
    for (int i = 0; i < windows.size(); ++i) {
        if (windows.at(i) == changingw) {
            windows.takeAt(i);
            if (i < nReserved)
                --nReserved;
            break;
        }
    }
//    syncRegions();
    handleWindowClose(changingw);
#ifndef QT_NO_QWS_PROPERTIES
    propertyManager.removeProperties(changingw->winId());
#endif
    emit q->windowEvent(changingw, QWSServer::Destroy);
    delete changingw;
}

void QWSServerPrivate::invokeSetFocus(const QWSRequestFocusCommand *cmd, QWSClient *client)
{
    int winId = cmd->simpleData.windowid;
    int gain = cmd->simpleData.flag;

    if (gain != 0 && gain != 1) {
        qWarning("Only 0(lose) and 1(gain) supported");
        return;
    }

    QWSWindow* changingw = findWindow(winId, 0);
    if (!changingw)
        return;

    if (!changingw->forClient(client)) {
       qWarning("Disabled: clients changing other client's focus");
        return;
    }

    setFocus(changingw, gain);
}

void QWSServerPrivate::setFocus(QWSWindow* changingw, bool gain)
{
    Q_Q(QWSServer);
#ifndef QT_NO_QWS_INPUTMETHODS
    /*
      This is the logic:
      QWSWindow *loser = 0;
      if (gain && focusw != changingw)
         loser = focusw;
      else if (!gain && focusw == changingw)
         loser = focusw;
      But these five lines can be reduced to one:
    */
    if (current_IM) {
        QWSWindow *loser =  (!gain == (focusw==changingw)) ? focusw : 0;
        if (loser && loser->winId() == current_IM_winId)
            current_IM->updateHandler(QWSInputMethod::FocusOut);
    }
#endif
    if (gain) {
        if (focusw != changingw) {
            if (focusw) focusw->focus(0);
            focusw = changingw;
            focusw->focus(1);
            emit q->windowEvent(focusw, QWSServer::Active);
        }
    } else if (focusw == changingw) {
        if (changingw->client())
            changingw->focus(0);
        focusw = 0;
        // pass focus to window which most recently got it...
        QWSWindow* bestw=0;
        for (int i=0; i<windows.size(); ++i) {
            QWSWindow* w = windows.at(i);
            if (w != changingw && !w->hidden() &&
                    (!bestw || bestw->focusPriority() < w->focusPriority()))
                bestw = w;
        }
        if (!bestw && changingw->focusPriority()) { // accept focus back?
            bestw = changingw; // must be the only one
        }
        focusw = bestw;
        if (focusw)
            focusw->focus(1);
        emit q->windowEvent(focusw, QWSServer::Active);
    }
}



void QWSServerPrivate::invokeSetOpacity(const QWSSetOpacityCommand *cmd, QWSClient *client)
{
    Q_UNUSED( client );
    int winId = cmd->simpleData.windowid;
    int opacity = cmd->simpleData.opacity;

    QWSWindow* changingw = findWindow(winId, 0);

    if (!changingw) {
        qWarning("invokeSetOpacity: Invalid window handle %d", winId);
        return;
    }

    int altitude = windows.indexOf(changingw);
    changingw->_opacity = opacity;
    exposeRegion(changingw->requested_region, altitude);
}

void QWSServerPrivate::invokeSetAltitude(const QWSChangeAltitudeCommand *cmd,
                                   QWSClient *client)
{
    Q_UNUSED( client );
    int winId = cmd->simpleData.windowid;
    int alt = cmd->simpleData.altitude;
    bool fixed = cmd->simpleData.fixed;
#if 0
    qDebug("QWSServer::invokeSetAltitude winId %d alt %d)", winId, alt);
#endif

    if (alt < -1 || alt > 1) {
        qWarning("QWSServer::invokeSetAltitude Only lower, raise and stays-on-top supported");
        return;
    }

    QWSWindow* changingw = findWindow(winId, 0);
    if (!changingw) {
        qWarning("invokeSetAltitude: Invalid window handle %d", winId);
        return;
    }

    if (fixed && alt >= 1) {
        changingw->onTop = true;
    }
    if (alt < 0)
        lowerWindow(changingw, alt);
    else
        raiseWindow(changingw, alt);

//      if (!changingw->forClient(client)) {
//         refresh();
//     }
}

#ifndef QT_NO_QWS_PROPERTIES
void QWSServerPrivate::invokeAddProperty(QWSAddPropertyCommand *cmd)
{
    propertyManager.addProperty(cmd->simpleData.windowid, cmd->simpleData.property);
}

void QWSServerPrivate::invokeSetProperty(QWSSetPropertyCommand *cmd)
{
    Q_Q(QWSServer);
    if (propertyManager.setProperty(cmd->simpleData.windowid,
                                    cmd->simpleData.property,
                                    cmd->simpleData.mode,
                                    cmd->data,
                                    cmd->rawLen)) {
        q->sendPropertyNotifyEvent(cmd->simpleData.property,
                                 QWSPropertyNotifyEvent::PropertyNewValue);
#ifndef QT_NO_QWS_INPUTMETHODS
        if (cmd->simpleData.property == QT_QWS_PROPERTY_MARKEDTEXT) {
            QString s((const QChar*)cmd->data, cmd->rawLen/2);
            emit q->markedText(s);
        }
#endif
    }
}

void QWSServerPrivate::invokeRemoveProperty(QWSRemovePropertyCommand *cmd)
{
    Q_Q(QWSServer);
    if (propertyManager.removeProperty(cmd->simpleData.windowid,
                                       cmd->simpleData.property)) {
        q->sendPropertyNotifyEvent(cmd->simpleData.property,
                                 QWSPropertyNotifyEvent::PropertyDeleted);
    }
}

void QWSServerPrivate::invokeGetProperty(QWSGetPropertyCommand *cmd, QWSClient *client)
{
    char *data;
    int len;

    if (propertyManager.getProperty(cmd->simpleData.windowid,
                                    cmd->simpleData.property,
                                    data, len)) {
        client->sendPropertyReplyEvent(cmd->simpleData.property, len, data);
    } else {
        client->sendPropertyReplyEvent(cmd->simpleData.property, -1, 0);
    }
}
#endif //QT_NO_QWS_PROPERTIES

void QWSServerPrivate::invokeSetSelectionOwner(QWSSetSelectionOwnerCommand *cmd)
{
    qDebug("QWSServer::invokeSetSelectionOwner");

    SelectionOwner so;
    so.windowid = cmd->simpleData.windowid;
    so.time.set(cmd->simpleData.hour, cmd->simpleData.minute,
                 cmd->simpleData.sec, cmd->simpleData.ms);

    if (selectionOwner.windowid != -1) {
        QWSWindow *win = findWindow(selectionOwner.windowid, 0);
        if (win)
            win->client()->sendSelectionClearEvent(selectionOwner.windowid);
        else
            qDebug("couldn't find window %d", selectionOwner.windowid);
    }

    selectionOwner = so;
}

void QWSServerPrivate::invokeConvertSelection(QWSConvertSelectionCommand *cmd)
{
    qDebug("QWSServer::invokeConvertSelection");

    if (selectionOwner.windowid != -1) {
        QWSWindow *win = findWindow(selectionOwner.windowid, 0);
        if (win)
            win->client()->sendSelectionRequestEvent(cmd, selectionOwner.windowid);
        else
            qDebug("couldn't find window %d", selectionOwner.windowid);
    }
}

#ifndef QT_NO_QWS_CURSOR
void QWSServerPrivate::invokeDefineCursor(QWSDefineCursorCommand *cmd, QWSClient *client)
{
    if (cmd->simpleData.height > 64 || cmd->simpleData.width > 64) {
        qDebug("Cannot define cursor size > 64x64");
        return;
    }

    delete client->cursors.take(cmd->simpleData.id);

    int dataLen = cmd->simpleData.height * ((cmd->simpleData.width+7) / 8);

    if (dataLen > 0 && cmd->data) {
        QWSCursor *curs = new QWSCursor(cmd->data, cmd->data + dataLen,
                                        cmd->simpleData.width, cmd->simpleData.height,
                                        cmd->simpleData.hotX, cmd->simpleData.hotY);
        client->cursors.insert(cmd->simpleData.id, curs);
    }
}

void QWSServerPrivate::invokeSelectCursor(QWSSelectCursorCommand *cmd, QWSClient *client)
{
    int id = cmd->simpleData.id;
    QWSCursor *curs = 0;
    if (id <= Qt::LastCursor) {
        curs = QWSCursor::systemCursor(id);
    }
    else {
        QWSCursorMap cursMap = client->cursors;
        QWSCursorMap::Iterator it = cursMap.find(id);
        if (it != cursMap.end()) {
            curs = it.value();
        }
    }
    if (curs == 0) {
        curs = QWSCursor::systemCursor(Qt::ArrowCursor);
    }

    QWSWindow* win = findWindow(cmd->simpleData.windowid, 0);
    if (mouseGrabber) {
        // If the mouse is being grabbed, we don't want just anyone to
        // be able to change the cursor.  We do want the cursor to be set
        // correctly once mouse grabbing is stopped though.
        if (win != mouseGrabber)
            nextCursor = curs;
        else
            setCursor(curs);
    } else if (win && win->requestedRegion().contains(QWSServer::mousePosition)) { //##################### cursor
        // A non-grabbing window can only set the cursor shape if the
        // cursor is within its allocated region.
        setCursor(curs);
    }
}

void QWSServerPrivate::invokePositionCursor(QWSPositionCursorCommand *cmd, QWSClient *)
{
    Q_Q(QWSServer);
    QPoint newPos(cmd->simpleData.newX, cmd->simpleData.newY);
    if (newPos != QWSServer::mousePosition)
        q->sendMouseEvent(newPos, qwsServer->d_func()->mouseState);
}
#endif

void QWSServerPrivate::invokeGrabMouse(QWSGrabMouseCommand *cmd, QWSClient *client)
{
    QWSWindow* win = findWindow(cmd->simpleData.windowid, 0);
    if (!win)
        return;

    if (cmd->simpleData.grab) {
        if (!mouseGrabber || mouseGrabber->client() == client) {
            mouseGrabbing = true;
            mouseGrabber = win;
        }
    } else {
        releaseMouse(mouseGrabber);
    }
}

void QWSServerPrivate::invokeGrabKeyboard(QWSGrabKeyboardCommand *cmd, QWSClient *client)
{
    QWSWindow* win = findWindow(cmd->simpleData.windowid, 0);
    if (!win)
        return;

    if (cmd->simpleData.grab) {
        if (!keyboardGrabber || (keyboardGrabber->client() == client)) {
            keyboardGrabbing = true;
            keyboardGrabber = win;
        }
    } else {
        releaseKeyboard(keyboardGrabber);
    }
}

#if !defined(QT_NO_SOUND)
void QWSServerPrivate::invokePlaySound(QWSPlaySoundCommand *cmd, QWSClient *)
{
#if !defined(EXTERNAL_SOUND_SERVER) && !defined(Q_OS_DARWIN)
    soundserver->playFile( 1, cmd->filename );
#else
    Q_UNUSED(cmd);
#endif
}
#endif

#ifndef QT_NO_COP
void QWSServerPrivate::invokeRegisterChannel(QWSQCopRegisterChannelCommand *cmd,
                                       QWSClient *client)
{
  // QCopChannel will force us to emit the newChannel signal if this channel
  // didn't already exist.
  QCopChannel::registerChannel(cmd->channel, client);
}

void QWSServerPrivate::invokeQCopSend(QWSQCopSendCommand *cmd, QWSClient *client)
{
    QCopChannel::answer(client, cmd->channel, cmd->message, cmd->data);
}

#endif

#ifndef QT_NO_QWS_INPUTMETHODS
void QWSServer::resetInputMethod()
{
    if (current_IM && qwsServer) {
      current_IM->reset();
    }
}

void QWSServerPrivate::invokeIMResponse(const QWSIMResponseCommand *cmd,
                                 QWSClient *)
{
    if (current_IM)
        current_IM->queryResponse(cmd->simpleData.property, cmd->result);
}

void QWSServerPrivate::invokeIMUpdate(const QWSIMUpdateCommand *cmd,
                                 QWSClient *)
{
    if (cmd->simpleData.type == QWSInputMethod::FocusIn)
        current_IM_winId = cmd->simpleData.windowid;

    if (current_IM && current_IM_winId == cmd->simpleData.windowid)
        current_IM->updateHandler(cmd->simpleData.type);
}

#endif

void QWSServerPrivate::invokeRepaintRegion(QWSRepaintRegionCommand * cmd,
                                    QWSClient *)
{
    QRegion r;
    r.setRects(cmd->rectangles,cmd->simpleData.nrectangles);
    repaint_region(cmd->simpleData.windowid, cmd->simpleData.opaque, r);
}

QWSWindow* QWSServerPrivate::newWindow(int id, QWSClient* client)
{
    Q_Q(QWSServer);
    // Make a new window, put it on top.
    QWSWindow* w = new QWSWindow(id,client);

    // insert after "stays on top" windows
    bool added = false;
    for (int i = nReserved; i < windows.size(); ++i) {
        QWSWindow *win = windows.at(i);
        if (!win->onTop) {
            windows.insert(i, w);
            added = true;
            break;
        }
    }
    if (!added)
        windows.append(w);
    emit q->windowEvent(w, QWSServer::Create);
    return w;
}

QWSWindow* QWSServerPrivate::findWindow(int windowid, QWSClient* client)
{
    for (int i=0; i<windows.size(); ++i) {
        QWSWindow* w = windows.at(i);
        if (w->winId() == windowid)
            return w;
    }
    if (client)
        return newWindow(windowid,client);
    else
        return 0;
}


void QWSServerPrivate::raiseWindow(QWSWindow *changingw, int /*alt*/)
{
    Q_Q(QWSServer);
    if (changingw == windows.first())
        return;

    int windowPos = 0;

    //change position in list:
    for (int i = 0; i < windows.size(); ++i) {
        QWSWindow *w = windows.at(i);
        if (w == changingw) {
            windowPos = i;
            windows.takeAt(i);
            break;
        }
    }

    int newPos = -1;
    if (changingw->onTop) {
        windows.insert(nReserved, changingw);
        newPos = nReserved;
    } else {
        // insert after "stays on top" windows
        bool in = false;
        for (int i = nReserved; i < windows.size(); ++i) {
            QWSWindow *w = windows.at(i);
            if (!w->onTop) {
                windows.insert(i, changingw);
                in = true;
                newPos = i;
                break;
            }
        }
        if (!in) {
            windows.append(changingw);
            newPos = windows.size()-1;
        }
    }

    if (windowPos != newPos) {
        exposeRegion(changingw->requestedRegion(), newPos); //### exposes too much, including what was already visible
    }
//    syncRegions(changingw);
    emit q->windowEvent(changingw, QWSServer::Raise);
}

void QWSServerPrivate::lowerWindow(QWSWindow *changingw, int /*alt*/)
{
    Q_Q(QWSServer);
    if (changingw == windows.last())
        return;

    int i = windows.indexOf(changingw);
    windows.move(i,windows.size()-1);

    QRegion exposed = changingw->requestedRegion(); //### exposes too much, including what was already visible
    exposeRegion(exposed, i);
    emit q->windowEvent(changingw, QWSServer::Lower);
}

void QWSServerPrivate::moveWindowRegion(QWSWindow *changingw, int dx, int dy)
{
    if (!changingw) return;

    QRegion oldRegion(changingw->requested_region);
    changingw->requested_region.translate(dx, dy);

    int idx = windows.indexOf(changingw);
    exposeRegion(oldRegion + changingw->requested_region, idx);
}

/*!
    Changes the requested region of window \a changingw to \a r
    If \a changingw is 0, the server's reserved region is changed.
*/
void QWSServerPrivate::setWindowRegion(QWSWindow* changingw, QRegion r)
{
    if (!changingw) {
        qWarning("Not implemented in this release");
        return;
    }

    if (changingw->requested_region == r)
        return;
    QRegion oldRegion(changingw->requested_region);
    changingw->requested_region = r;

    int idx = windows.indexOf(changingw);
    exposeRegion(oldRegion - changingw->requested_region, idx);
}


void QWSServerPrivate::exposeRegion(QRegion r, int changing)
{
    qt_screen->exposeRegion(r, qMax(nReserved, changing));
}

/*!
    Closes the pointer device(s).
*/
void QWSServer::closeMouse()
{
    Q_D(QWSServer);
    qDeleteAll(d->mousehandlers);
    d->mousehandlers.clear();
}

/*!
    Opens the mouse device(s).
*/
void QWSServer::openMouse()
{
    Q_D(QWSServer);
    QByteArray mice = qgetenv("QWS_MOUSE_PROTO");
    if (mice.isEmpty()) {
#if defined(QT_QWS_CASSIOPEIA)
        mice = "TPanel:/dev/tpanel";
#endif
        if (mice.isEmpty())
            mice = defaultMouse;
    }
    closeMouse();
    bool needviscurs = true;
    if (mice != "None") {
        QList<QByteArray> mouse = mice.split(' ');
        for (QList<QByteArray>::Iterator m=mouse.begin(); m!=mouse.end(); ++m) {
            QString ms = *m;
            QWSMouseHandler* h = d->newMouseHandler(ms);
            (void)h;
            /* XXX handle mouse cursor visibility sensibly
               if (!h->inherits("QCalibratedMouseHandler"))
               needviscurs = true;
            */
        }
    }
#ifndef QT_NO_QWS_CURSOR
    setCursorVisible(needviscurs);
#else
    Q_UNUSED(needviscurs)
#endif
}

/*!
  Suspends mouse handling by suspending each registered mouse handler.

  \sa resumeMouse()
*/
void QWSServer::suspendMouse()
{
    Q_D(QWSServer);
    for (int i=0; i < d->mousehandlers.size(); ++i)
        d->mousehandlers.at(i)->suspend();
}

/*!
  Resumes mouse handling by reactivating each registered mouse handler.

  \sa suspendMouse()
*/
void QWSServer::resumeMouse()
{
    Q_D(QWSServer);
    for (int i=0; i < d->mousehandlers.size(); ++i)
        d->mousehandlers.at(i)->resume();
}



QWSMouseHandler* QWSServerPrivate::newMouseHandler(const QString& spec)
{
    static int init=0;
    if (!init && qt_screen) {
        init = 1;
    }

    int c = spec.indexOf(':');
    QString mouseProto;
    QString mouseDev;
    if (c >= 0) {
        mouseProto = spec.left(c);
        mouseDev = spec.mid(c+1);
    } else {
        mouseProto = spec;
    }

    QWSMouseHandler *handler = 0;
    handler = QMouseDriverFactory::create(mouseProto, mouseDev);
    return handler;
}

#ifndef QT_NO_QWS_KEYBOARD

/*!
    Closes keyboard device(s).
*/
void QWSServer::closeKeyboard()
{
    Q_D(QWSServer);
    qDeleteAll(d->keyboardhandlers);
    d->keyboardhandlers.clear();
}

/*!
    Returns the primary keyboard handler.
*/
QWSKeyboardHandler* QWSServer::keyboardHandler()
{
    return qwsServerPrivate->keyboardhandlers.first();
}

/*!
    Sets the primary keyboard handler to \a kh.
*/
void QWSServer::setKeyboardHandler(QWSKeyboardHandler* kh)
{
    qwsServerPrivate->keyboardhandlers.prepend(kh);
}

/*!
    Opens the keyboard device(s).
*/
void QWSServer::openKeyboard()
{
    Q_D(QWSServer);
    QString keyboards = qgetenv("QWS_KEYBOARD");
    if (keyboards.isEmpty()) {
#if defined(QT_QWS_CASSIOPEIA)
        keyboards = "Buttons";
#endif
        if (keyboards.isEmpty()) {
            keyboards = defaultKeyboard;        // last resort
        }
    }
    closeKeyboard();
    if (keyboards == "None")
        return;
    QString device;
    QString type;
    QStringList keyboard = keyboards.split(" ");
    for (QStringList::Iterator k=keyboard.begin(); k!=keyboard.end(); ++k) {
        QString spec = *k;
        int colon=spec.indexOf(':');
        if (colon>=0) {
            type = spec.left(colon);
            device = spec.mid(colon+1);
        } else {
            type = spec;
        }
        QWSKeyboardHandler* kh = QKbdDriverFactory::create(type, device);
        d->keyboardhandlers.append(kh);
    }
}

#endif //QT_NO_QWS_KEYBOARD

QPoint QWSServer::mousePosition;
QBrush *QWSServerPrivate::bgBrush = 0;

void QWSServerPrivate::move_region(const QWSRegionMoveCommand *cmd)
{
    QWSClient *serverClient = clientMap.value(-1);
    invokeRegionMove(cmd, serverClient);
}

void QWSServerPrivate::set_altitude(const QWSChangeAltitudeCommand *cmd)
{
    QWSClient *serverClient = clientMap.value(-1);
    invokeSetAltitude(cmd, serverClient);
}

void QWSServerPrivate::set_opacity(const QWSSetOpacityCommand *cmd)
{
    QWSClient *serverClient = clientMap.value(-1);
    invokeSetOpacity(cmd, serverClient);
}


void QWSServerPrivate::request_focus(const QWSRequestFocusCommand *cmd)
{
    invokeSetFocus(cmd, clientMap.value(-1));
}

void QWSServerPrivate::set_identity(const QWSIdentifyCommand *cmd)
{
    invokeIdentify(cmd, clientMap.value(-1));
}

void QWSServerPrivate::repaint_region(int wid, bool opaque, QRegion region)
{
    QWSWindow* changingw = findWindow(wid, 0);
    if (!changingw) {
        return;
    }
    changingw->opaque = opaque;
    int level = windows.indexOf(changingw);
    exposeRegion(region, level);
}

void QWSServerPrivate::request_region(int wid, QWSMemId mid,
                                      int windowtype, QRegion region, QImage::Format imageFormat,
                                      QWSWindow *changingw)
{
    Q_Q(QWSServer);
    if (!changingw)
        changingw = findWindow(wid, 0);
    if (!changingw) {
        return;
    }
    bool isShow = !changingw->isVisible() && !region.isEmpty();

    if (windowtype == QWSBackingStore::OnScreen) {
        int i = 0;

        int oldPos = windows.indexOf(changingw);
        int newPos = oldPos < nReserved ? nReserved-1 : nReserved;
        while (i < nReserved) {
            QWSWindow *rw = windows.at(i);
            if (i != oldPos) {
                //for Reserved regions, requested_region is really the allocated region
                region -= rw->requested_region;
            }
            ++i;
        }
        windows.move(oldPos, newPos);
        nReserved = newPos + 1;
        if (isShow) {
            delete changingw->_backingStore;
            changingw->_backingStore = 0;
            changingw->requested_region = region;
        } else {
            //handle change
            QRegion oldRegion = changingw->requested_region;
            changingw->requested_region = region;
            exposeRegion(oldRegion - region, newPos);
        }
        changingw->client()->sendRegionEvent(wid, region, 0);
    } else {
        QWSBackingStore *backingStore = changingw->backingStore();
        QWSClient *client = changingw->client();

        if (client->clientId() == 0) {
            backingStore->setMemory(mid, region.boundingRect().size(), imageFormat, windowtype);
#ifndef QT_NO_QWS_MULTIPROCESS
        } else {
            backingStore->attach(mid, region.boundingRect().size(), imageFormat, windowtype);
            backingStore->setLock(changingw->client()->d_func()->clientLock);
#endif
        }

        changingw->opaque = windowtype != QWSBackingStore::Transparent && windowtype != QWSBackingStore::YellowThing;

        setWindowRegion(changingw, region);
    }
    if (isShow)
        emit q->windowEvent(changingw, QWSServer::Show);
    if (!region.isEmpty())
        emit q->windowEvent(changingw, QWSServer::Geometry);
    else
        emit q->windowEvent(changingw, QWSServer::Hide);
    if (region.isEmpty())
        handleWindowClose(changingw);



    //### if the window under our mouse changes, send update.
    //     if (containsMouse != changingw->allocatedRegion().contains(mousePosition))
    //         updateClientCursorPos();
}

void QWSServerPrivate::destroy_region(const QWSRegionDestroyCommand *cmd)
{
    invokeRegionDestroy(cmd, clientMap.value(-1));
}

void QWSServerPrivate::name_region(const QWSRegionNameCommand *cmd)
{
    invokeRegionName(cmd, clientMap.value(-1));
}

#ifndef QT_NO_QWS_INPUTMETHODS
void QWSServerPrivate::im_response(const QWSIMResponseCommand *cmd)
 {
     invokeIMResponse(cmd, clientMap.value(-1));
}

void QWSServerPrivate::im_update(const QWSIMUpdateCommand *cmd)
{
    invokeIMUpdate(cmd, clientMap.value(-1));
}

void QWSServerPrivate::send_im_mouse(const QWSIMMouseCommand *cmd)
{
    if (current_IM)
        current_IM->mouseHandler(cmd->simpleData.index, cmd->simpleData.state);
}
#endif

void QWSServerPrivate::openDisplay()
{
    qt_init_display();

//    rgnMan = qt_fbdpy->regionManager();
    swidth = qt_screen->deviceWidth();
    sheight = qt_screen->deviceHeight();
}

void QWSServerPrivate::closeDisplay()
{
    qt_screen->shutdownDevice();
}

const QBrush &QWSServer::backgroundBrush() const
{
    return *QWSServerPrivate::bgBrush;
}

/*!
    Sets the brush \a brush to be used as the background in the absence of
    obscuring windows.
*/
void QWSServer::setBackground(const QBrush &brush)
{
    if (!qwsServer)
        return;
    *QWSServerPrivate::bgBrush = brush;
    qt_screen->exposeRegion(QRect(0,0,qt_screen->width(), qt_screen->height()), 0);
}


#ifdef QT3_SUPPORT
/*!
    Sets the image \a img to be used as the background in the absence
    of obscuring windows.
*/
void QWSServer::setDesktopBackground(const QImage &img)
{
    if (img.isNull())
        setBackground(Qt::NoBrush);
    else
        setBackground(QBrush(QPixmap::fromImage(img)));
}

/*!
    \overload

    Sets the color \a c to be used as the background in the absence of
    obscuring windows.
*/
void QWSServer::setDesktopBackground(const QColor &c)
{
    setDesktopBackground(QBrush(c));
}
#endif //QT3_SUPPORT

/*!
  \internal
 */
void QWSServer::startup(int flags)
{
    if (qwsServer)
        return;
    unlink(qws_qtePipeFilename().toLatin1().constData());
    (void)new QWSServer(flags);
}

/*!
  \internal
*/

void QWSServer::closedown()
{
    unlink(qws_qtePipeFilename().toLatin1().constData());
    delete qwsServer;
    qwsServer = 0;
}

void QWSServerPrivate::emergency_cleanup()
{
#ifndef QT_NO_QWS_KEYBOARD
    if (qwsServer)
        qwsServer->closeKeyboard();
#endif
}

#ifndef QT_NO_QWS_KEYBOARD
static QList<QWSServer::KeyboardFilter*> *keyFilters = 0;

/*!
    Processes the key event characterized by the given parameters:

    \table
    \header \o Parameter \o Description
    \row
        \o \a unicode
        \o The unicode value of the key to send
    \row
        \o \a keycode
        \o The Qt keycode value as defined by the Qt::Key enum.
    \row
        \o \a modifiers
        \o An OR combination of Qt::KeyboardModifier values, indicating whether
            Shift/Alt/Ctrl keys are pressed.
    \row
        \o \a isPress
        \o True if this is a key down event; otherwise false.
    \row
        \o \a autoRepeat
        \o True if this event is caused by auto repeat (i.e. the
            user has held the key down and this is the second or subsequent
            key event being sent); otherwise false.
    \endtable

    This function is typically called internally by keyboard drivers.

    \sa sendKeyEvent(), {Character Input}
*/
void QWSServer::processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                bool isPress, bool autoRepeat)
{
    if (keyFilters) {
        for (int i = 0; i < keyFilters->size(); ++i) {
            QWSServer::KeyboardFilter *keyFilter = keyFilters->at(i);
            if (keyFilter->filter(unicode, keycode, modifiers, isPress, autoRepeat))
                return;
        }
    }
    sendKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

/*!
    Adds a filter \a f to be invoked for all key events from physical
    keyboard drivers (events sent via processKeyEvent()).

    The filter is not invoked for keys generated by virtual keyboard
    drivers (events sent via sendKeyEvent()).

    Keyboard filters are removed by removeKeyboardFilter().
*/
void QWSServer::addKeyboardFilter(KeyboardFilter *f)
{
     if (!keyFilters)
        keyFilters = new QList<QWSServer::KeyboardFilter*>;
     if (f) {
        keyFilters->prepend(f);
     }
}

/*
//#######
 We should probably obsolete the whole keyboard filter thing since
 it's not useful for input methods anyway

 We could do removeKeyboardFilter(KeyboardFilter *f), but
 the "remove and delete the filter" concept does not match "user
 remembers the pointer".
*/

/*!
    Removes and deletes the most-recently added filter.
    The caller is responsible for matching each addition with a
    corresponding removal.
*/
void QWSServer::removeKeyboardFilter()
{
     if (!keyFilters || keyFilters->isEmpty())
         return;
     delete keyFilters->takeAt(0);
}
#endif // QT_NO_QWS_KEYBOARD

/*!
    Sets an array of timeouts for the screensaver to a list of \a ms
    milliseconds. A setting of zero turns off the screensaver. The
    array must be 0-terminated.
*/
void QWSServer::setScreenSaverIntervals(int* ms)
{
    if (!qwsServerPrivate)
        return;
    delete [] qwsServerPrivate->screensaverintervals;
    if (ms) {
        int* t=ms;
        int n=0;
        while (*t++) n++;
        if (n) {
            n++; // the 0
            qwsServerPrivate->screensaverintervals = new int[n];
            memcpy(qwsServerPrivate->screensaverintervals, ms, n*sizeof(int));
        } else {
            qwsServerPrivate->screensaverintervals = 0;
        }
    } else {
        qwsServerPrivate->screensaverintervals = 0;
    }
    qwsServerPrivate->screensaverinterval = 0;

    qwsServerPrivate->screensavertimer->stop();
    qt_screen->blank(false);
    qwsServerPrivate->screenSaverWake();
}

/*!
    Sets the timeout for the screensaver to \a ms milliseconds. A
    setting of zero turns off the screensaver.
*/
void QWSServer::setScreenSaverInterval(int ms)
{
    int v[2];
    v[0] = ms;
    v[1] = 0;
    setScreenSaverIntervals(v);
}

extern bool qt_disable_lowpriority_timers; //in qeventloop_unix.cpp

void QWSServerPrivate::screenSaverWake()
{
    if (screensaverintervals) {
        if (screensaverinterval != screensaverintervals) {
            if (saver) saver->restore();
            screensaverinterval = screensaverintervals;
        } else {
            if (!screensavertimer->isActive()) {
                qt_screen->blank(false);
                if (saver) saver->restore();
            }
        }
        screensavertimer->start(*screensaverinterval);
        screensavertime.start();
    }
    qt_disable_lowpriority_timers=false;
}

void QWSServerPrivate::screenSaverSleep()
{
    qt_screen->blank(true);
#if !defined(QT_QWS_IPAQ) && !defined(QT_QWS_EBX)
    screensavertimer->stop();
#else
    if (screensaverinterval) {
        screensavertimer->start(*screensaverinterval);
        screensavertime.start();
    } else {
        screensavertimer->stop();
    }
#endif
    qt_disable_lowpriority_timers=true;
}

/*!
    \internal

    Deletes the current screen saver and sets the screen saver to be
    \a ss.
*/
void QWSServer::setScreenSaver(QWSScreenSaver* ss)
{
    QWSServerPrivate *qd = qwsServer->d_func();
    delete qd->saver;
    qd->saver = ss;
}

void QWSServerPrivate::screenSave(int level)
{
    if (saver) {
        if (saver->save(level)) {
            if (screensaverinterval && screensaverinterval[1]) {
                screensavertimer->start(*++screensaverinterval);
                screensavertime.start();
            } else {
                screensaverinterval = 0;
            }
        } else {
            // for some reason, the saver don't want us to change to the
            // next level, so we'll stay at this level for another interval
            if (screensaverinterval && *screensaverinterval) {
                screensavertimer->start(*screensaverinterval);
                screensavertime.start();
            }
        }
    } else {
        screensaverinterval = 0;//screensaverintervals;
        screenSaverSleep();
    }
}

void QWSServerPrivate::screenSaverTimeout()
{
    if (screensaverinterval) {
        if (screensavertime.elapsed() > *screensaverinterval*2) {
            // bogus (eg. unsuspend, system time changed)
            screenSaverWake(); // try again
            return;
        }
        screenSave(screensaverinterval - screensaverintervals);
    }
}

/*!
    Returns true if the screensaver is active (i.e. the screen is
    blanked); otherwise returns false.
*/
bool QWSServer::screenSaverActive()
{
    return qwsServerPrivate->screensaverinterval
        && !qwsServerPrivate->screensavertimer->isActive();
}

/*!
    If \a activate is true the screensaver is activated immediately;
    if \a activate is false the screensaver is deactivated.
*/
void QWSServer::screenSaverActivate(bool activate)
{
    if (activate)
        qwsServerPrivate->screenSaverSleep();
    else
        qwsServerPrivate->screenSaverWake();
}

void QWSServerPrivate::disconnectClient(QWSClient *c)
{
    QTimer::singleShot(0, c, SLOT(closeHandler()));
}

void QWSServerPrivate::updateClientCursorPos()
{
    Q_Q(QWSServer);
    QWSWindow *win = qwsServerPrivate->mouseGrabber ? qwsServerPrivate->mouseGrabber : qwsServer->windowAt(QWSServer::mousePosition);
    QWSClient *winClient = win ? win->client() : 0;
    if (winClient && winClient != cursorClient)
        q->sendMouseEvent(QWSServer::mousePosition, mouseState);
}

#ifndef QT_NO_QWS_INPUTMETHODS

/*!
    \class QWSInputMethod
    \brief The QWSInputMethod class provides international input methods
    for Qtopia Core.

    \ingroup qws

    Subclass this class to implement your own input method.

    An input methods consists of a keyboard filter and optionally a
    graphical interface. The keyboard filter intercepts key events
    from physical or virtual keyboards by implementing the filter()
    function.

    Use sendIMEvent() to send composition events.

    Use QWSServer::setCurrentInputMethod() to install an input method.

    This class is still subject to change.
*/

/*!
  Constructs a new input method
*/

QWSInputMethod::QWSInputMethod()
{

}

/*!
  Destructs the input method uninstalling it if it is currently installed.
*/
QWSInputMethod::~QWSInputMethod()
{
    if (current_IM == this)
        current_IM = 0;
}

/*!
    This function must be implemented in subclasses to handle key
    input from physical or virtual keyboards. Returning true will
    block the event from further processing.

    The Unicode value is given in \a unicode and the key code in \a
    keycode. Keyboard modifiers are OR-ed together in \a modifiers.
    If \a isPress is true this is a key press; otherwise it is a key
    release. If \a autoRepeat is true this is an auto-repeated key
    press.

    The default implementation returns false.
*/
bool QWSInputMethod::filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    Q_UNUSED(unicode);
    Q_UNUSED(keycode);
    Q_UNUSED(modifiers);
    Q_UNUSED(isPress);
    Q_UNUSED(autoRepeat);
    return false;
}

/*!
    This function must be implemented in subclasses to handle mouse
    input from physical or virtual pointer devices. Returning true will
    block the event from further processing.

    The position of the mouse event is given in \a position,
    with state and wheel values \a state and \a wheel.

    The default implementation returns false.
*/
bool QWSInputMethod::filter(const QPoint &position, int state, int wheel)
{
    Q_UNUSED(position);
    Q_UNUSED(state);
    Q_UNUSED(wheel);
    return false;
}

/*!
    Implemented in subclasses to reset the state of the input method.

    The default implementation calls sendIMEvent() with empty preedit
    and commit strings, if the input method is in compose mode.
*/
void QWSInputMethod::reset()
{
    if (current_IM_composing_win) {
        QInputMethodEvent ime;
        sendEvent(&ime);
    }
}

/*!
  \enum QWSInputMethod::UpdateType

  This enum describes different types of update events received by
  updateHandler.

  \value Update    The input widget is updated in some way; use sendQuery() with Qt::ImMicroFocus as an argument for more information.
  \value FocusIn   A new input widget receives focus.
  \value FocusOut  The input widget loses focus.
  \value Reset       The input method should be reset.
  \value Destroyed The input widget is destroyed.

*/

/*!
  Handles update events, including resets and focus changes.

  Reimplementations must call the base implementation for all cases that it
  does not handle itself.

  \a type is a value defined in \l UpdateType.
*/
void QWSInputMethod::updateHandler(int type)
{
    switch (type) {
    case FocusOut:
    case Reset:
        reset();
        break;

    default:
        break;
    }
}


/*!
  This event handler is implemented in subclasses to receive replies to an
  input method query.

  The specified \a property and \a result contain the property
  queried and the result returned in the reply.

  \sa QWSServer::sendIMQuery()
*/
void QWSInputMethod::queryResponse(int property, const QVariant &result)
{
    Q_UNUSED(property);
    Q_UNUSED(result);
}



/*!
  \fn void QWSInputMethod::mouseHandler(int x, int state)

  Implemented in subclasses to handle mouse presses/releases within
  the preedit text. The parameter \a x is the offset within
  the string that was sent with the InputMethodCompose event.
  \a state is either QWSServer::MousePress or QWSServer::MouseRelease

  if \a state < 0 then the mouse event is inside the widget, but outside the preedit text

  QWSServer::MouseOutside is sent when clicking in a different widget.

  The default implementation resets the input method on all mouse presses.

*/
void QWSInputMethod::mouseHandler(int, int state)
{
    if (state == QWSServer::MousePress || state == QWSServer::MouseOutside)
        reset();
}


/*!
  Sends an input method event with preedit string \a preeditString
  and cursor position \a cursorPosition. \a selectionLength is the
  number of characters to be marked as selected (starting at
  cursorPosition). If \a selectionLength is negative, the text before
  \a cursorPosition is marked.

  This is a convenience function for sendEvent().

  The preedit string is marked with \c QInputContext::PreeditFormat,
  and the selected part is marked with
  \c QInputContext::SelectionFormat.

  Sending an input method event with a non-empty preedit string will
  cause the input method to enter compose mode.  Sending an input
  method event with an empty preedit string will cause the input
  method to leave compose mode.

  \sa sendEvent(), sendCommitString(), QInputMethodEvent
*/

void QWSInputMethod::sendPreeditString(const QString &preeditString, int cursorPosition, int selectionLength)
{
    QList<QInputMethodEvent::Attribute> attributes;

    int selPos = cursorPosition;
    if (selectionLength == 0) {
        selPos = 0;
    } else if (selectionLength < 0) {
        selPos += selectionLength;
        selectionLength = -selectionLength;
    }
    if (selPos > 0)
        attributes += QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, selPos,
                                                   QVariant(int(QInputContext::PreeditFormat)));

    if (selectionLength)
        attributes += QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, selPos, selectionLength,
                                                   QVariant(int(QInputContext::SelectionFormat)));

    if (selPos + selectionLength < preeditString.length())
        attributes += QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
                                                   selPos + selectionLength,
                                                   preeditString.length() - selPos - selectionLength,
                                                   QVariant(int(QInputContext::PreeditFormat)));

    attributes += QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, cursorPosition,  0, QVariant());

    QInputMethodEvent ime(preeditString, attributes);
    qwsServer->sendIMEvent(&ime);
}

/*!
  Sends an input method event with commit string \a
  commitString. This is a convenience function for sendEvent().

  If \a replaceLength is greater than 0, the commit string will
  replace \a replaceLength characters of the receiving widget's
  previous text, starting at \a replaceFrom relative to the start of
  the preedit string.

  This will cause the input method to leave compose mode.

  \sa sendEvent(), sendPreeditString(), QInputMethodEvent
*/
void QWSInputMethod::sendCommitString(const QString &commitString, int replaceFrom, int replaceLength)
{
    QInputMethodEvent ime;
    ime.setCommitString(commitString, replaceFrom, replaceLength);
    qwsServer->sendIMEvent(&ime);
}

/*!
    \fn QWSInputMethod::sendIMEvent(QWSServer::IMState state, const QString &txt, int cpos, int selLen)

    Causes a QIMEvent to be sent to the focus widget.

    \a txt is the preedit string if \a state is QWSServer::IMCompose,
    or the commit string if \a state is QWSServer::IMEnd.

    If \a state is QWSServer::IMCompose, \a cpos is the cursor
    position within the preedit string, and \a selLen is the number of
    characters (starting at \a cpos) that should be marked as selected
    by the input widget receiving the event.

    Use sendEvent(), sendPreeditString() or sendCommitString() instead.
*/

/*!
  \fn void QWSInputMethod::sendQuery(int property)

  Sends an input method query for the specified \a property.

  Reimplement the virtual function queryResponse() to receive
  responses to input method queries.

  \sa queryResponse()
*/

/*!
  If \a isHigh is true and the device has a pointer device resolution twice or
  more of the screen resolution, then positions passed to filterMouse will be presented
  at the higher resolution.  Otherwise resoltion of positions passed to filterMouse
  will be equal to that of the screen resolution.

  Returns the resulting number of bits to shift down to go from pointer resolution to
  screen resolution in filterMouse.

  \sa filterMouse()
  */
uint QWSInputMethod::setInputResolution(bool isHigh)
{
    mIResolution = isHigh;
    return inputResolutionShift();
}

/*!
  Returns the number of bits to shift down to go from pointer resolution to
  screen resolution in filterMouse.
*/
uint QWSInputMethod::inputResolutionShift() const
{
    return 0; // default for devices with single resolution.
}

/*!
  Sends a mouse event at the position \a pos, with state and wheel values \a state and \wheel.
  The event will be not be tested by by the active input method.  This function will
  also transform \a pos if the screen coordinates do not match device pointer coordinates

  \note calling QWSServer::sendMouseEvent() will result in the event being filtered by the
  current inputmethod.

  \sa QWSServer::sendMouseEvent()
*/
void QWSInputMethod::sendMouseEvent( const QPoint &pos, int state, int wheel )
{
    QPoint tpos;
    if (qt_screen->isTransformed()) {
	QSize s = QSize(qt_screen->width(), qt_screen->height());
	tpos = qt_screen->mapToDevice(pos, s);
    } else {
	tpos = pos;
    }
    // because something else will transform it back later.
    QWSServerPrivate::sendMouseEventUnfiltered( tpos, state, wheel );
}
#endif // QT_NO_QWS_INPUTMETHODS

/*!
    \fn  QWSWindow::QWSWindow(int i, QWSClient * client)

    Constructs a new top-level window, associated with the client \a
    client and giving it the id \a i.
*/

/*!
    \fn QWSServer::windowEvent(QWSWindow * w, QWSServer::WindowEvent e)

    This signal is emitted whenever something happens to a top-level
    window (e.g. it's created or destroyed). \a w is the window to
    which the event of type \a e has occurred.
*/

/*!
    \fn QWSServer::keyMap()

    Returns the keyboard mapping table used to convert keyboard
    scancodes to Qt keycodes and Unicode values. It's used by the
    keyboard driver in \c qkeyboard_qws.cpp.
*/

/*!
    \enum QWSServer::ServerFlags

    This enum is used to pass various options to the window system
    server.

    \value DisableKeyboard Ignore all keyboard input.
    \value DisableMouse Ignore all mouse input.
*/

/*
    \class QWSServer::KeyMap
    \brief The QWSServer::KeyMap class is used for mapping scancodes.

    \ingroup qws

    The KeyMap structure records an individual KeyMap entry in the
    array used to map keyboard scancodes to Qt key codes and Unicode
    values.
*/

/*!
    \class QWSServer::KeyboardFilter
    \brief The KeyboardFilter class provides a global keyboard event filter.

    \ingroup qws

    The KeyboardFilter class is used to implement a global, low-level
    filter on key events in the Qtopia Core server application; this
    can be used to implement things like APM (advanced power
    management) suspend from a button without having to filter for it
    in all applications.
*/

/*!
    \fn QWSServer::KeyboardFilter::~KeyboardFilter()

    Destroys the keyboard filter.
*/

/*!
    \fn bool QWSServer::KeyboardFilter::filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)

    Returns true if the specified key should be filtered; otherwise returns
    false. A true return value stops the key from being processed any further.

    The Unicode value is given in \a unicode and the key code in \a
    keycode. Keyboard modifiers are OR-ed together in \a modifiers.
    If \a isPress is true this is a key press; otherwise it is a key
    release. If \a autoRepeat is true this is an auto-repeated key
    press.

    All normal key events should be blocked while in compose mode.
*/

/*!
    \enum QWSServer::WindowEvent

    This specifies what sort of event has occurred to a top-level window:

    \value Create A new window has been created (QWidget constructor).
    \value Destroy The window has been closed and deleted (QWidget destructor).
    \value Hide The window has been hidden with QWidget::hide().
    \value Show The window has been shown with QWidget::show() or similar.
    \value Raise The window has been raised to the top of the desktop.
    \value Lower The window has been lowered.
    \value Geometry The window has changed size or position.
    \value Active The window has become the active window (has keyboard focus).
    \value Name The window has been named.
*/

#include "moc_qwindowsystem_qws.cpp"
