/****************************************************************************
**
** Implementation of QSocket class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsocket.h"
#ifndef QT_NO_NETWORK
#include "qlist.h"
#include "qtimer.h"
#include "qsocketnotifier.h"
#include "qsocketdevice.h"
#include "private/qsocketdevice_p.h"
#include "private/qinternal_p.h"
#include "qlist.h"

#include <string.h>
#ifndef NO_ERRNO_H
#include <errno.h>
#endif

//#define QSOCKET_DEBUG
//#define QSOCKET_EXTRA_DEBUG

#ifdef QSOCKET_EXTRA_DEBUG
#include <stdio.h>
static void hexDump(const char * data, int len)
{
    fprintf(stderr, "(%d bytes) ", len);

    for (int i = 0; i < len; i++) {
        uint c = (uchar)data[i];
        fprintf(stderr, "%02x %c ", c, (c >' ' && c < '~') ? c : ' ');
    }
    fprintf(stderr, "\n");
}
#endif

#define d d_func()
#define q q_func()

/*
  Perhaps this private functionality needs to be refactored.

  Comment from Robert D. Gatlin (Intel):

    It would be nice to have the functionality inherent in QSocket available
    as a separate class as a standard part of the Qt library, something along
    the line of:

      class QByteBuffer : public QIODevice { ... }

    The same class could/would be used within QSocket for the Read/Write
    buffers.

    The above class could be used in the following way(s):

        buffer.open(IO_WriteOnly | IO_Append);
        buffer.writeBlock(a); // a = QByteArray
        buffer.close();

        QByteArray b;
        b.resize(buffer.size());
        buffer.open(IO_ReadOnly);
        buffer.readBlock(b.data(), b.size());
        buffer.close();

    But would also be useable with QDataStream (via QIODevice) with:

        buffer.open(IO_WriteOnly | IO_Append);
        QDataStream is(&buffer);
        is << 100;
        buffer.close();

        buffer.open(IO_ReadOnly);
        QDataStream os(&buffer);
        Q_UINT32 x;
        os >> x;
        buffer.close();

    The real usefulness is with any situations where data (QByteArray) arrives
    incrementally (as in QSocket and filter case above).

    I tried using QBuffer, but QBuffer does not trim bytes from the front of
    the buffer in cases like:

        QBuffer buf;
        buf.open(IO_ReadOnly);
        QDataStream ds(&buf);
        Q_INT32 x;
        ds >> x;
        buf.close();

    In the above case, buf.size() will be identical before and after the
    operation with QDataStream. Based on the implementation of QBuffer, it
    does not appear well suited for this kind of operation.
*/

// Private class for QSocket

class QSocketPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QSocket);

public:
    QSocketPrivate();
    ~QSocketPrivate();

    void init();
    void terminate();
    void closeSocket();
    void close();
    void internalConnectionClosed();
    void internalSetSocketDevice(QSocketDevice *device);
    bool consumeWriteBuf(Q_ULONG nbytes);
    void tryConnection();
    void setSocket(int socket);

    void tryConnecting(const QDnsHostInfo &);
    void emitErrorConnectionRefused();

    QSocket::State state;                           // connection state
    QString host;                                   // host name
    Q_UINT16 port;                                  // host port
    QSocketDevice *socket;                          // connection socket
    QSocketNotifier *rsn, *wsn;                     // socket notifiers
    QMembuf rba;                                    // read buffer
    Q_ULONG readBufferSize;                         // limit for the read buffer size
    QList<QByteArray *> wba;                        // list of write bufs
    QHostAddress addr;                              // connection address
    QList<QHostAddress> addresses;                  // alternatives looked up
    QIODevice::Offset wsize;                        // write total buf size
    QIODevice::Offset windex;                       // write index
    bool sn_read_alreadyCalled;                     // needed to avoid recursion
};

QSocketPrivate::QSocketPrivate()
{
    init();
}

QSocketPrivate::~QSocketPrivate()
{
    terminate();
}

void QSocketPrivate::init()
{
    state = QSocket::Idle;
    port = 0;
    socket = 0;
    rsn = 0;
    wsn = 0;
    readBufferSize = 0;
    wsize = 0;
    windex = 0;
    sn_read_alreadyCalled = false;
}

void QSocketPrivate::terminate()
{
    close();
    delete socket;
    while (!wba.isEmpty())
        delete wba.takeFirst();
    host.clear();
    addr.clear();
    addresses.clear();
}

void QSocketPrivate::closeSocket()
{
    // Order is important here - the socket notifiers must go away
    // before the socket does, otherwise libc or the kernel will
    // become unhappy.
    delete rsn;
    rsn = 0;
    delete wsn;
    wsn = 0;
    if (socket)
        socket->close();
}

void QSocketPrivate::close()
{
    closeSocket();
    wsize = 0;
    rba.clear();
    while (!wba.isEmpty())
        delete wba.takeFirst();
    windex = 0;
}

void QSocketPrivate::internalConnectionClosed()
{
    // We keep the open state in case there's unread incoming data
    state = QSocket::Idle;
    closeSocket();
    while (!wba.isEmpty())
        delete wba.takeFirst();
    windex = wsize = 0;
    emit q->connectionClosed();
}

void QSocketPrivate::internalSetSocketDevice(QSocketDevice *device)
{
    delete socket;
    delete rsn;
    delete wsn;

    if (device) {
        socket = device;
    } else {
        socket = new QSocketDevice(QSocketDevice::Stream,
                                   (addr.isIPv4Address() ?
                                    QSocketDevice::IPv4 : QSocketDevice::IPv6), 0);
        socket->setBlocking(false);
        socket->setAddressReusable(true);
    }

    rsn = new QSocketNotifier(socket->socket(), QSocketNotifier::Read, q, "read");
    wsn = new QSocketNotifier(socket->socket(), QSocketNotifier::Write, q, "write");

    QObject::connect(rsn, SIGNAL(activated(int)), q, SLOT(sn_read()));
    rsn->setEnabled(false);
    QObject::connect(wsn, SIGNAL(activated(int)), q, SLOT(sn_write()));
    wsn->setEnabled(false);
}

/*
    Consumes nbytes bytes of data from the write buffer.
*/
bool QSocketPrivate::consumeWriteBuf(Q_ULONG nbytes)
{
    if (nbytes <= 0 || nbytes > wsize)
        return false;
#if defined(QSOCKET_DEBUG)
    qDebug("QSocket (%s): consumeWriteBuf %d bytes", q->name(), (int)nbytes);
#endif
    wsize -= nbytes;
    for (;;) {
        QByteArray *a = wba.first();
        if ((int)(windex + nbytes) >= a->size()) {
            nbytes -= a->size() - windex;
            wba.removeFirst();
            delete a;
            windex = 0;
            if (nbytes == 0)
                break;
        } else {
            windex += nbytes;
            break;
        }
    }
    return true;
}

void QSocketPrivate::tryConnection()
{
    if (socket->connect(addr, port)) {
        state = QSocket::Connected;
#if defined(QSOCKET_DEBUG)
        qDebug("QSocket (%s): sn_write: Got connection to %s", q->name(), q->peerName().ascii());
#endif
        if (rsn)
            rsn->setEnabled(true);
        emit q->connected();
    } else {
        state = QSocket::Idle;
        QTimer::singleShot(0, q, SLOT(emitErrorConnectionRefused()));
    }
}

/*
    Sets the socket to \a socket. This is used by both setSocket()
    and connectToHost() and can also be used on unconnected sockets.
*/
void QSocketPrivate::setSocket(int socket)
{
    if (state != QSocket::Idle) {
        q->clearPendingData();
        q->close();
    }

    terminate();
    init();

    if (socket >= 0) {
        QSocketDevice *sd = new QSocketDevice(socket, QSocketDevice::Stream);
        sd->setBlocking(false);
        sd->setAddressReusable(true);
        internalSetSocketDevice(sd);
    }
    state = QSocket::Idle;

    // Initialize the IO device flags
    q->setFlags(IO_Direct);
    q->resetStatus();
    q->open(QSocket::ReadWrite);

    // this is not very nice
    host.clear();
    port = 0;
}

/*
    Continues the connection process where connectToHost() leaves off.
*/
void QSocketPrivate::tryConnecting(const QDnsHostInfo &hostInfo)
{
#if defined(QSOCKET_DEBUG)
    qDebug("QSocket (%s)::tryConnecting()", q->name());
#endif
    // ### this ifdef isn't correct - addresses() also does /etc/hosts and
    // numeric-address-as-string handling.
#ifndef QT_NO_DNS
    if (state == QSocket::HostLookup) {
        if (hostInfo.addresses.isEmpty()) {
            // no results: give up
            state = QSocket::Idle;
            emit q->error(QSocket::ErrHostNotFound);
            return;
        }

        // we've found something. press on with that. if we later find
        // more, fine.
        emit q->hostFound();
        state = QSocket::Connecting;
    }

    if (state == QSocket::Connecting) {
        // put IPv4 addresses upfront, IPv6 back. the relative order of
        // the addresses is not important beyond that
	for (int i = 0; i < hostInfo.addresses.size(); ++i) {
	    const QHostAddress &a = hostInfo.addresses.at(i);
	    if (a.isIPv4Address())
		addresses.prepend(a);
            else
		addresses.append(a);
	}

        // try one address at a time, falling back to the next one if
        // there is a connection failure. (should also support a timeout,
        // or do multiple TCP-level connects at a time, with staggered
        // starts to avoid bandwidth waste and cause fewer
        // "connect-and-abort" errors. but that later.)
        bool stuck = true;
        while (stuck) {
            stuck = false;
            if (socket && !socket->connect(addr, port)) {
		if (socket->error() == QSocketDevice::NoError) {
                    if (wsn)
                        wsn->setEnabled(true);
                    return; // not serious, try again later
                }

#if defined(QSOCKET_DEBUG)
                qDebug("QSocket (%s)::tryConnecting: Gave up on IP address %s",
                       q->name(), socket->peerAddress().toString().ascii());
#endif
                delete wsn;
                wsn = 0;
                delete rsn;
                rsn = 0;
                delete socket;
                socket = 0;
            }
            // if the host has more addresses, try another some.
            if (socket == 0 && !addresses.isEmpty()) {
                addr = addresses.takeFirst();
                internalSetSocketDevice(0);
                stuck = true;
#if defined(QSOCKET_DEBUG)
                qDebug("QSocket (%s)::tryConnecting: Trying IP address %s",
                        q->name(), addr.toString().ascii());
#endif
            }
        }

        // The socket write notifier will fire when the connection succeeds
        if (wsn)
            wsn->setEnabled(true);
    }
#endif
}

void QSocketPrivate::emitErrorConnectionRefused()
{
    emit q->error(QSocket::ErrConnectionRefused);
}

/*!
    \class QSocket
    \brief The QSocket class provides a buffered TCP connection.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    It provides a totally non-blocking QIODevice, and modifies and
    extends the API of QIODevice with socket-specific code.

    The functions you're likely to call most are connectToHost(),
    bytesAvailable(), canReadLine() and the ones it inherits from
    QIODevice.

    connectToHost() is the most-used function. As its name implies,
    it opens a connection to a named host.

    Most network protocols are either packet-oriented or
    line-oriented. canReadLine() indicates whether a connection
    contains an entire unread line or not, and bytesAvailable()
    returns the number of bytes available for reading.

    The signals error(), connected(), readyRead() and
    connectionClosed() inform you of the progress of the connection.
    There are also some less commonly used signals. hostFound() is
    emitted when connectToHost() has finished its DNS lookup and is
    starting its TCP connection. delayedCloseFinished() is emitted
    when close() succeeds. bytesWritten() is emitted when QSocket
    moves data from its "to be written" queue into the TCP
    implementation.

    There are several access functions for the socket: state() returns
    whether the object is idle, is doing a DNS lookup, is connecting,
    has an operational connection, etc. address() and port() return
    the IP address and port used for the connection. The peerAddress()
    and peerPort() functions return the IP address and port used by
    the peer, and peerName() returns the name of the peer (normally
    the name that was passed to connectToHost()). socket() returns a
    pointer to the QSocketDevice used for this socket.

    QSocket inherits QIODevice, and reimplements some functions. In
    general, you can treat it as a QIODevice for writing, and mostly
    also for reading. The match isn't perfect, since the QIODevice
    API is designed for devices that are controlled by the same
    machine, and an asynchronous peer-to-peer network connection isn't
    quite like that. For example, there is nothing that matches
    QIODevice::size() exactly. The documentation for open(), close(),
    flush(), size(), at(), atEnd(), readBlock(), writeBlock(),
    getch(), putch(), ungetch() and readLine() describes the
    differences in detail.

    \warning QSocket is not suitable for use in threads. If you need
    to uses sockets in threads use the lower-level QSocketDevice class.

    \sa QSocketDevice, QHostAddress, QSocketNotifier
*/


/*!
    Creates a QSocket object in \c QSocket::Idle state.

    The \a parent and \a name arguments are passed on to the QObject
    constructor.
*/

QSocket::QSocket(QObject *parent, const char *name)
    : QObject(parent, name), QIODevice(*new QSocketPrivate())
{
    /*
        The d_ptr member variable is necessary because we have two
        base classes with a variable called d_ptr.
    */
    d_ptr = QIODevice::d_ptr;

    d->internalSetSocketDevice(0);
    setFlags(IO_Direct);
    resetStatus();
}


/*!
    Destroys the socket. Closes the connection if necessary.

    \sa close()
*/

QSocket::~QSocket()
{
#if defined(QSOCKET_DEBUG)
    qDebug("QSocket (%s): Destroy", name());
#endif
    if (d->state != Idle)
        close();
}


/*!
    Returns a pointer to the internal socket device.

    There is normally no need to manipulate the socket device directly
    since this class does the necessary setup for most applications.
*/

QSocketDevice *QSocket::socketDevice()
{
    return d->socket;
}

/*!
    Sets the internal socket device to \a device. Passing a \a device
    of 0 will cause the internal socket device to be used. Any
    existing connection will be disconnected before using the new \a
    device.

    The new device should not be connected before being associated
    with a QSocket; after setting the socket call connectToHost() to
    make the connection.

    This function is useful if you need to subclass QSocketDevice and
    want to use the QSocket API, for example, to implement Unix domain
    sockets.
*/

void QSocket::setSocketDevice(QSocketDevice *device)
{
    if (d->state != Idle)
        close();
    d->internalSetSocketDevice(device);
}

/*!
    \enum QSocket::State

    This enum defines the connection states:

    \value Idle if there is no connection
    \value HostLookup during a DNS lookup
    \value Connecting during TCP connection establishment
    \value Connected when there is an operational connection
    \value Closing if the socket is closing down, but is not yet closed.
*/

/*!
    Returns the current state of the socket connection.

    \sa QSocket::State
*/

QSocket::State QSocket::state() const
{
    return d->state;
}


#ifndef QT_NO_DNS

/*!
    Attempts to make a connection to \a host on the specified \a port
    and return immediately.

    Any connection or pending connection is closed immediately, and
    QSocket goes into the \c HostLookup state. When the lookup
    succeeds, it emits hostFound(), starts a TCP connection and goes
    into the \c Connecting state. Finally, when the connection
    succeeds, it emits connected() and goes into the \c Connected
    state. If there is an error at any point, it emits error().

    \a host may be an IP address in string form, or it may be a DNS
    name. QSocket will do a normal DNS lookup if required. Note that
    \a port is in native byte order, unlike some other libraries.

    \sa state()
*/

void QSocket::connectToHost(const QString &host, Q_UINT16 port)
{
#if defined(QSOCKET_DEBUG)
    qDebug("QSocket (%s)::connectToHost: host %s, port %d",
            name(), host.ascii(), port);
#endif
    d->setSocket(-1);
    d->state = HostLookup;
    d->host = host;
    d->port = port;

    QHostAddress hostAddr;
    if (hostAddr.setAddress(host)) {
	// try if the address is already available (for faster connecting)
	QDnsHostInfo h;
	h.addresses.append(hostAddr);
	d->tryConnecting(h);
    }

    if (d->state == HostLookup)
	QDns::getHostByName(host, this, SLOT(tryConnecting(QDnsHostInfo)));
}

#endif

/*!
    \enum QSocket::Error

    This enum specifies the possible errors:

    \value ErrConnectionRefused if the connection was refused
    \value ErrHostNotFound if the host was not found
    \value ErrSocketRead if a read from the socket failed
*/

/*!
    \fn void QSocket::error(int)

    This signal is emitted after an error occurred. The parameter is
    the \l Error value.
*/

/*!
    \fn void QSocket::hostFound()

    This signal is emitted after connectToHost() has been called and
    the host lookup has succeeded.

    \sa connected()
*/


/*!
    \fn void QSocket::connected()

    This signal is emitted after connectToHost() has been called and a
    connection has been successfully established.

    \sa connectToHost(), connectionClosed()
*/


/*!
    \fn void QSocket::connectionClosed()

    This signal is emitted when the other end has closed the
    connection. The read buffers may contain buffered input data which
    you can read after the connection was closed.

    \sa connectToHost(), close()
*/


/*!
    \fn void QSocket::delayedCloseFinished()

    This signal is emitted when a delayed close is finished.

    If you call close() and there is buffered output data to be
    written, QSocket goes into the \c QSocket::Closing state and
    returns immediately. It will then keep writing to the socket until
    all the data has been written. Then, the delayedCloseFinished()
    signal is emitted.

    \sa close()
*/


/*!
    \fn void QSocket::readyRead()

    This signal is emitted every time there is new incoming data.

    Bear in mind that new incoming data is only reported once; if you do not
    read all the data, this class buffers the data and you can read it later,
    but no signal is emitted unless new data arrives. A good practice is to
    read all data in the slot connected to this signal unless you are sure that
    you need to receive more data to be able to process it.

    \sa readBlock(), readLine(), bytesAvailable()
*/


/*!
    \fn void QSocket::bytesWritten(int nbytes)

    This signal is emitted when data has been written to the network.
    The \a nbytes parameter specifies how many bytes were written.

    The bytesToWrite() function is often used in the same context; it
    indicates how many buffered bytes there are left to write.

    \sa writeBlock(), bytesToWrite()
*/


/*!
    Opens the socket using the specified QIODevice file mode \a m.
    This function is called automatically when needed and you should
    not call it yourself.

    \sa close()
*/

bool QSocket::open(int m)
{
    if (isOpen()) {
        qWarning("QSocket::open: Already open");
        return false;
    }
    QIODevice::setMode(m & IO_ReadWrite);
    setState(IO_Open);
    return true;
}


/*!
    Closes the socket.

    The read buffer is cleared.

    If the output buffer is empty, the state is set to \c
    QSocket::Idle and the connection is terminated immediately. If the
    output buffer still contains data to be written, QSocket goes into
    the \c QSocket::Closing state and the rest of the data will be
    written. When all of the outgoing data have been written, the
    state is set to \c QSocket::Idle and the connection is terminated.
    At this point, the delayedCloseFinished() signal is emitted.

    If you don't want that the data of the output buffer is written, call
    clearPendingData() before you call close().

    \sa state(), bytesToWrite() clearPendingData()
*/

void QSocket::close()
{
    if (!isOpen() || d->state == Idle)        // already closed
        return;
    if (d->state == Closing)
        return;
    if (!d->rsn || !d->wsn)
        return;
#if defined(QSOCKET_DEBUG)
    qDebug("QSocket (%s): close socket", name());
#endif
    if (d->socket && d->wsize) {                // there's data to be written
        d->state = Closing;
        if (d->rsn)
            d->rsn->setEnabled(false);
        if (d->wsn)
            d->wsn->setEnabled(true);
        d->rba.clear();                                // clear incoming data
        return;
    }
    setFlags(IO_Sequential);
    resetStatus();
    setState(0);
    d->close();
    d->state = Idle;
}

/*!
    Implementation of the abstract virtual QIODevice::flush() function.
*/

void QSocket::flush()
{
    bool osBufferFull = false;
    int consumed = 0;
    while (!osBufferFull && d->state >= Connecting && d->wsize > 0) {
#if defined(QSOCKET_DEBUG)
        qDebug("QSocket (%s): flush: Write data to the socket", name());
#endif
        QByteArray *a = d->wba.first();
        int nwritten;
        int i = 0;
        if ((int)a->size() - d->windex < 1460) {
            // Concatenate many smaller blocks.  the first may be
            // partial, but each subsequent block is copied entirely
            // or not at all.  the sizes here are picked so that we
            // generally won't trigger nagle's algorithm in the tcp
            // implementation: we concatenate if we'd otherwise send
            // less than PMTU bytes (we assume PMTU is 1460 bytes),
            // and concatenate up to the largest payload TCP/IP can
            // carry.  with these precautions, nagle's algorithm
            // should apply only when really appropriate.
            QByteArray out;
            out.resize(65536);
            int j = d->windex;
            int s = a->size() - j;
            int n = 0;
            while (n < d->wba.count() && i+s < (int)out.size()) {
#ifdef QSOCKET_EXTRA_DEBUG
                fprintf(stderr, "small block #%d: ", n);
                hexDump(a->constData()+j, s);
#endif
                memcpy(out.data()+i, a->constData()+j, s);
                j = 0;
                i += s;
                ++n;
                a = n >= d->wba.count() ? 0 : d->wba.at(n);
                s = a ? a->size() : 0;
            }
            nwritten = d->socket->writeBlock(out, i);
#ifdef QSOCKET_EXTRA_DEBUG
            fprintf(stderr, "writing concatenated block ");
            hexDump(out.data(), nwritten);
#endif
            if (d->wsn)
                d->wsn->setEnabled(false); // the QSocketNotifier documentation says so
        } else {
            // Big block, write it immediately
            i = a->size() - d->windex;
            nwritten = d->socket->writeBlock(a->constData() + d->windex, i);
#ifdef QSOCKET_EXTRA_DEBUG
            fprintf(stderr, "writing big block ");
            hexDump(a->constData() + d->windex, nwritten);
#endif
            if (d->wsn)
                d->wsn->setEnabled(false); // the QSocketNotifier documentation says so
        }
        if (nwritten > 0 && d->consumeWriteBuf(nwritten))
            consumed += nwritten;
        if (nwritten < i)
            osBufferFull = true;
    }
    if (consumed > 0) {
#if defined(QSOCKET_DEBUG)
        qDebug("QSocket (%s): flush: wrote %d bytes, %d left",
                name(), consumed, (int)d->wsize);
#endif
        emit bytesWritten(consumed);
    }
    if (d->state == Closing && d->wsize == 0) {
#if defined(QSOCKET_DEBUG)
        qDebug("QSocket (%s): flush: Delayed close done. Terminating.",
                name());
#endif
        setFlags(IO_Sequential);
        resetStatus();
        setState(0);
        d->close();
        d->state = Idle;
        emit delayedCloseFinished();
        return;
    }
    if (!d->socket->isOpen()) {
        d->internalConnectionClosed();
        return;
    }
    if (d->wsn)
        d->wsn->setEnabled(d->wsize > 0); // write if there's data
}


/*!
    Returns the number of incoming bytes that can be read right now
    (like bytesAvailable()).
*/

QIODevice::Offset QSocket::size() const
{
    return (Offset)bytesAvailable();
}


/*!
    Returns the current read index. Since QSocket is a sequential
    device, the current read index is always zero.
*/

QIODevice::Offset QSocket::at() const
{
    return 0;
}


/*!
    \overload

    Moves the read index forward to \a index and returns true if the
    operation was successful; otherwise returns false. Moving the
    index forward means skipping incoming data.
*/

bool QSocket::at(Offset index)
{
    if (index > d->rba.size())
        return false;
    d->rba.consumeBytes((Q_ULONG)index, 0);                        // throw away data 0..index-1
    // After we read data from our internal buffer, if we use the
    // setReadBufferSize() to limit our buffer, we might now be able to
    // read more data in our buffer. So enable the read socket notifier,
    // but do this only if we are not in a slot connected to the
    // readyRead() signal since this might cause a bad recursive behavior.
    // We can test for this condition by looking at the
    // sn_read_alreadyCalled flag.
    if (d->rsn && !d->sn_read_alreadyCalled)
        d->rsn->setEnabled(true);
    return true;
}


/*!
    Returns true if there is no more data to read; otherwise returns false.
*/

bool QSocket::atEnd() const
{
    if (d->socket == 0)
        return true;
    QSocket * that = (QSocket *)this;
    if (that->d->socket->bytesAvailable())        // a little slow, perhaps...
        that->sn_read();
    return that->d->rba.size() == 0;
}


/*!
    Returns the number of incoming bytes that can be read, i.e. the
    size of the input buffer. Equivalent to size().

    \sa bytesToWrite()
*/

Q_ULONG QSocket::bytesAvailable() const
{
    if (d->socket == 0)
        return 0;
    QSocket * that = (QSocket *)this;
    if (that->d->socket->bytesAvailable()) // a little slow, perhaps...
        (void)that->sn_read();
    return that->d->rba.size();
}


/*!
    Wait up to \a msecs milliseconds for more data to be available.

    If \a msecs is -1 the call will block indefinitely.

    Returns the number of bytes available.

    If \a timeout is non-null and no error occurred (i.e. it does not
    return -1): this function sets \a *timeout to true, if the reason
    for returning was that the timeout was reached; otherwise it sets
    \a *timeout to false. This is useful to find out if the peer
    closed the connection.

    \warning This is a blocking call and should be avoided in event
    driven applications.

    \sa bytesAvailable()
*/

Q_ULONG QSocket::waitForMore(int msecs, bool *timeout) const
{
    if (d->socket == 0)
        return 0;
    QSocket * that = (QSocket *)this;
    if (that->d->socket->waitForMore(msecs, timeout) > 0)
        (void)that->sn_read(true);
    return that->d->rba.size();
}

/*!
    Returns the number of bytes that are waiting to be written, i.e.
    the size of the output buffer.

    \sa bytesAvailable() clearPendingData()
*/

Q_ULONG QSocket::bytesToWrite() const
{
    return d->wsize;
}

/*!
    Deletes the data that is waiting to be written. This is useful if you want
    to close the socket without waiting for all the data to be written.

    \sa bytesToWrite() close() delayedCloseFinished()
*/

void QSocket::clearPendingData()
{
    while (!d->wba.isEmpty())
        delete d->wba.takeFirst();
    d->windex = d->wsize = 0;
}

/*!
    Reads \a maxlen bytes from the socket into \a data and returns the
    number of bytes read. Returns -1 if an error occurred.
*/

Q_LONG QSocket::readBlock(char *data, Q_ULONG maxlen)
{
    if (data == 0 && maxlen != 0) {
        qWarning("QSocket::readBlock: Null pointer error");
        return -1;
    }
    if (!isOpen()) {
        qWarning("QSocket::readBlock: Socket is not open");
        return -1;
    }
    if (maxlen >= d->rba.size())
        maxlen = d->rba.size();
#if defined(QSOCKET_DEBUG)
    qDebug("QSocket (%s): readBlock %d bytes", name(), (int)maxlen);
#endif
    d->rba.consumeBytes(maxlen, data);
#if defined(QSOCKET_EXTRA_DEBUG)
    fprintf(stderr, "     ");
    hexDump(data, maxlen);
#endif
    // After we read data from our internal buffer, if we use the
    // setReadBufferSize() to limit our buffer, we might now be able to
    // read more data in our buffer. So enable the read socket notifier,
    // but do this only if we are not in a slot connected to the
    // readyRead() signal since this might cause a bad recursive behavior.
    // We can test for this condition by looking at the
    // sn_read_alreadyCalled flag.
    if (d->rsn && !d->sn_read_alreadyCalled)
        d->rsn->setEnabled(true);
    return maxlen;
}


/*!
    Writes \a len bytes to the socket from \a data and returns the
    number of bytes written. Returns -1 if an error occurred.
*/

Q_LONG QSocket::writeBlock(const char *data, Q_ULONG len)
{
    if (data == 0 && len != 0) {
        qWarning("QSocket::writeBlock: Null pointer error");
    }
    if (!isOpen()) {
        qWarning("QSocket::writeBlock: Socket is not open");
        return -1;
    }
    if (d->state == Closing) {
        qWarning("QSocket::writeBlock: Cannot write, socket is closing");
    }
    if (len == 0 || d->state == Closing || d->state == Idle)
        return 0;
    QByteArray *a = d->wba.isEmpty() ? 0 : d->wba.last();

    // next bit is sensitive.  if we're writing really small chunks,
    // try to buffer up since system calls are expensive, and nagle's
    // algorithm is even more expensive.  but if anything even
    // remotely large is being written, try to issue a write at once.

    bool writeNow = (d->wsize + len >= 1400 || len > 512);

    if (a && a->size() + len < 128) {
        // small buffer, resize
        int i = a->size();
        a->resize(i+len);
        memcpy(a->data()+i, data, len);
    } else {
        // append new buffer
        a = new QByteArray;
        a->resize(len);
        memcpy(a->data(), data, len);
        d->wba.append(a);
    }
    d->wsize += len;
    if (writeNow)
        flush();
    else if (d->wsn)
        d->wsn->setEnabled(true);
#if defined(QSOCKET_DEBUG)
    qDebug("QSocket (%s): writeBlock %d bytes", name(), (int)len);
#endif
    return len;
}


/*!
    Reads a single byte/character from the internal read buffer.
    Returns the byte/character read, or -1 if there is nothing to be
    read.

    \sa bytesAvailable(), putch()
*/

int QSocket::getch()
{
    if (isOpen() && d->rba.size() > 0) {
        uchar c;
        d->rba.consumeBytes(1, (char*)&c);
        // After we read data from our internal buffer, if we use the
        // setReadBufferSize() to limit our buffer, we might now be able to
        // read more data in our buffer. So enable the read socket notifier,
        // but do this only if we are not in a slot connected to the
        // readyRead() signal since this might cause a bad recursive behavior.
        // We can test for this condition by looking at the
        // sn_read_alreadyCalled flag.
        if (d->rsn && !d->sn_read_alreadyCalled)
            d->rsn->setEnabled(true);
        return c;
    }
    return -1;
}


/*!
    Writes the character \a ch to the output buffer.

    Returns \a ch, or -1 if an error occurred.

    \sa getch()
*/

int QSocket::putch(int ch)
{
    char buf[2];
    buf[0] = ch;
    return writeBlock(buf, 1) == 1 ? ch : -1;
}


/*!
    This implementation of the virtual function QIODevice::ungetch()
    prepends the character \a ch to the read buffer so that the next
    read returns this character as the first character of the output.
*/

int QSocket::ungetch(int ch)
{
    if (!isOpen()) {
        qWarning("QSocket::ungetch: Socket not open");
        return -1;
    }
    return d->rba.ungetch(ch);
}


/*!
    Returns true if it's possible to read an entire line of text from
    this socket at this time; otherwise returns false.

    Note that if the peer closes the connection unexpectedly, this
    function returns false. This means that loops such as this won't
    work:

    \code
        while(!socket->canReadLine()) // WRONG
            ;
    \endcode

    \sa readLine()
*/

bool QSocket::canReadLine() const
{
    if (((QSocket*)this)->d->rba.scanNewline(0))
        return true;
    return (bytesAvailable() > 0 &&
             ((QSocket*)this)->d->rba.scanNewline(0));
}

/*!
  \reimp
  \internal
    So that it's not hidden by our other readLine().
*/
Q_LONG QSocket::readLine(char *data, Q_ULONG maxlen)
{
    return QIODevice::readLine(data,maxlen);
}

/*!
    Returns a line of text including a terminating newline character
    (\n). Returns "" if canReadLine() returns false.

    \sa canReadLine()
*/

QString QSocket::readLine()
{
    QByteArray a;
    a.resize(256);
    bool nl = d->rba.scanNewline(&a);
    QString s;
    if (nl) {
        at(a.size());                                // skips the data read
        s = QString(a);
    }
    return s;
}

/*!
  \internal
    Internal slot for handling socket read notifications.

    This function has can usually only be entered once (i.e. no
    recursive calls). If the argument \a force is true, the function
    is executed, but no readyRead() signals are emitted. This
    behaviour is useful for the waitForMore() function, so that it is
    possible to call waitForMore() in a slot connected to the
    readyRead() signal.
*/

void QSocket::sn_read(bool force)
{
    Q_LONG maxToRead = 0;
    if (d->readBufferSize > 0) {
        maxToRead = d->readBufferSize - d->rba.size();
        if (maxToRead <= 0) {
            if (d->rsn)
                d->rsn->setEnabled(false);
            return;
        }
    }

    // Use sn_read_alreadyCalled to avoid recursive calls of
    // sn_read() (and as a result avoid emitting the readyRead() signal in a
    // slot for readyRead(), if you use bytesAvailable()).
    if (!force && d->sn_read_alreadyCalled)
        return;
    d->sn_read_alreadyCalled = true;

    char buf[4096];
    Q_LONG nbytes = d->socket->bytesAvailable();
    Q_LONG nread;
    QByteArray *a = 0;

    if (d->state == Connecting) {
        if (nbytes > 0) {
            d->tryConnection();
        } else {
            // nothing to do, nothing to care about
            goto end;
        }
    }
    if (d->state == Idle)
        goto end;

    if (nbytes <= 0) {                        // connection closed?
        // On Windows this may happen when the connection is still open.
        // This happens when the system is heavily loaded and we have
        // read all the data on the socket before a new WSAAsyncSelect
        // event is processed. A new read operation would then block.
        // This code is also useful when QSocket is used without an
        // event loop.
        nread = d->socket->readBlock(buf, maxToRead ? qMin((Q_LONG)sizeof(buf),maxToRead) : sizeof(buf));
        if (nread == 0) {                        // really closed
#if defined(QSOCKET_DEBUG)
            qDebug("QSocket (%s): sn_read: Connection closed", name());
#endif
            // ### we should rather ask the socket device if it is closed
            d->internalConnectionClosed();
            goto end;
        } else {
            if (nread < 0) {
                if (d->socket->error() == QSocketDevice::NoError) {
                    // all is fine
                    goto end;
                }
#if defined(QSOCKET_DEBUG)
                qWarning("QSocket::sn_read (%s): Close error", name());
#endif
                if (d->rsn)
                    d->rsn->setEnabled(false);
                emit error(ErrSocketRead);
                goto end;
            }
            a = new QByteArray;
            a->resize(nread);
            memcpy(a->data(), buf, nread);
        }
    } else {                                        // data to be read
#if defined(QSOCKET_DEBUG)
        qDebug("QSocket (%s): sn_read: %ld incoming bytes", name(), nbytes);
#endif
        if (nbytes > (int)sizeof(buf)) {
            // big
            a = new QByteArray;
            a->resize(nbytes);
            nread = d->socket->readBlock(a->data(), maxToRead ? qMin(nbytes,maxToRead) : nbytes);
        } else {
            a = 0;
            nread = d->socket->readBlock(buf, maxToRead ? qMin((Q_LONG)sizeof(buf),maxToRead) : sizeof(buf));
            if (nread > 0) {
                // ##### could setRawData
                a = new QByteArray;
                a->resize(nread);
                memcpy(a->data(), buf, nread);
            }
        }
        if (nread == 0) {
#if defined(QSOCKET_DEBUG)
            qDebug("QSocket (%s): sn_read: Connection closed", name());
#endif
            // ### we should rather ask the socket device if it is closed
            d->internalConnectionClosed();
            goto end;
        } else if (nread < 0) {
            if (d->socket->error() == QSocketDevice::NoError) {
                // all is fine
                goto end;
            }
            qWarning("QSocket::sn_read: Read error");
            delete a;
            if (d->rsn)
                d->rsn->setEnabled(false);
            emit error(ErrSocketRead);
            goto end;
        }
        if (nread != (int)a->size()) {                // unexpected
#if defined(CHECK_RANGE) && !defined(Q_OS_WIN32)
            qWarning("QSocket::sn_read: Unexpected short read");
#endif
            a->resize(nread);
        }
    }
    d->rba.append(a);
    if (!force) {
        if (d->rsn)
            d->rsn->setEnabled(false);
        emit readyRead();
        if (d->rsn)
            d->rsn->setEnabled(true);
    }

end:
    d->sn_read_alreadyCalled = false;
}


/*!
  \internal
    Internal slot for handling socket write notifications.
*/

void QSocket::sn_write()
{
    if (d->state == Connecting)                // connection established?
        d->tryConnection();
    flush();
}

/*!
    Returns the socket number, or -1 if there is no socket at the moment.
*/

int QSocket::socket() const
{
    if (d->socket == 0)
        return -1;
    return d->socket->socket();
}

/*!
    Sets the socket to use \a socket and the state() to \c Connected.
    The socket must already be connected.

    This allows us to use the QSocket class as a wrapper for other
    socket types (e.g. Unix Domain Sockets).
*/

void QSocket::setSocket(int socket)
{
    d->setSocket(socket);
    d->state = Connection;
    d->rsn->setEnabled(true);
}

/*!
    Returns the host port number of this socket, in native byte order.
*/

Q_UINT16 QSocket::port() const
{
    if (d->socket == 0)
        return 0;
    return d->socket->port();
}


/*!
    Returns the peer's host port number, normally as specified to the
    connectToHost() function. If none has been set, this function
    returns 0.

    Note that Qt always uses native byte order, i.e. 67 is 67 in Qt;
    there is no need to call htons().
*/

Q_UINT16 QSocket::peerPort() const
{
    if (d->socket == 0)
        return 0;
    return d->socket->peerPort();
}


/*!
    Returns the host address of this socket. (This is normally the
    main IP address of the host, but can be e.g. 127.0.0.1 for
    connections to localhost.)
*/

QHostAddress QSocket::address() const
{
    if (d->socket == 0)
        return QHostAddress();
    return d->socket->address();
}


/*!
    Returns the host address as resolved from the name specified to
    the connectToHost() function.
*/

QHostAddress QSocket::peerAddress() const
{
    if (d->socket == 0)
        return QHostAddress();
    return d->socket->peerAddress();
}


/*!
    Returns the host name as specified to the connectToHost()
    function. An empty string is returned if none has been set.
*/

QString QSocket::peerName() const
{
    return d->host;
}

/*!
    Sets the size of the QSocket's internal read buffer to \a bufSize.

    Usually QSocket reads all data that is available from the operating
    system's socket. If the buffer size is limited to a certain size, this
    means that the QSocket class doesn't buffer more than this size of data.

    If the size of the read buffer is 0, the read buffer is unlimited and all
    incoming data is buffered. This is the default.

    If you read the data in the readyRead() signal, you shouldn't use this
    option since it might slow down your program unnecessary. This option is
    useful if you only need to read the data at certain points in time, like in
    a realtime streaming application.

    \sa readBufferSize()
*/

void QSocket::setReadBufferSize(Q_ULONG bufSize)
{
    d->readBufferSize = bufSize;
}

/*!
    Returns the size of the read buffer.

    \sa setReadBufferSize()
*/

Q_ULONG QSocket::readBufferSize() const
{
    return d->readBufferSize;
}

#include "moc_qsocket.cpp"

#endif //QT_NO_NETWORK
