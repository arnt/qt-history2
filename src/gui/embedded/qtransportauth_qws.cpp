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

#include "qtransportauth_qws.h"
#include "qtransportauth_qws_p.h"

#ifndef QT_NO_SXE

#include "../../3rdparty/md5/md5.h"
#include "../../3rdparty/md5/md5.c"
#include "qwsutils_qws.h"
#include "qwscommand_qws_p.h"
#include "qwindowsystem_qws.h"
#include "qbuffer.h"
#include "qthread.h"
#include "qabstractsocket.h"
#include "qfile.h"
#include "qdebug.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

/*!
  \class QTransportAuth
  \internal

  \brief Authenticate a message transport.

  For performance reasons, message authentication is tied to an individual
  message transport instance.  For example in connection oriented transports
  the authentication cookie can be cached against the connection avoiding
  the overhead of authentication on every message.

  For each process there is one instance of the QTransportAuth object.
  For server processes it can determine the \link secure-exe-environ.html SXE
  Program Identity \endlink and provide access to policy data to determine if
  the message should be forwarded for action.  If not actioned, the message
  may be treated as being from a flawed or malicious process.

  Retrieve the instance with the getInstance() method.  The constructor is
  disabled and instances of QTransportAuth should never be constructed by
  calling classes.

  To make the Authentication easier to use a proxied QIODevice is provided
  which uses an internal QBuffer.

  In the server code first get a pointer to a QTransportAuth::Data object
  using the connectTransport() method:

  \code
  QTransportAuth::Data *conData;
  QTransportAuth *a = QTransportAuth::getInstance();

  conData = a->QTransportconnectTransport(
        QTransportAuth::Trusted | QTransportAuth::UnixStreamSock,
        socketDescriptor );
  \endcode

  Here it is asserted that the transport is trusted.  See the assumptions
  listed in the \link secure-exe-environ.html SXE documentation \endlink

  Then proxy in the authentication device:

  \code
  // mySocket can be any QIODevice subclass
  AuthDevice *ad = a->recvBuf( d, mySocket );

  // proxy in the auth device where the socket would have gone
  connect( ad, SIGNAL(readyRead()), this, SLOT(mySocketReadyRead()));
  \endcode

  In the client code it is similar.  Use the connectTransport() method
  just the same then proxy in the authentication device instead of the
  socket in write calls:

  \code
  AuthDevice *ad = a->authBuf( d, mySocket );

  ad->write( someData );
  \endcode
*/

static int hmac_md5(
        unsigned char*  text,         /* pointer to data stream */
        int             text_length,  /* length of data stream */
        const unsigned char*  key,    /* pointer to authentication key */
        int             key_length,   /* length of authentication key */
        unsigned char * digest        /* caller digest to be filled in */
        );



#define KEY_CACHE_SIZE 10

const char * const errorStrings[] = {
    "pending identity verification",
    "message too small to carry auth data",
    "cache miss on connection oriented transport",
    "no magic bytes on message",
    "key not found for prog id",
    "authorization key match failed",
    "key out of date"
};

const char *QTransportAuth::errorString( const Data &d )
{
    if (( d.status & ErrMask ) == Success )
        return "success";
    int e = d.status & ErrMask;
    if ( e > OutOfDate )
        return "unknown";
    return errorStrings[e];
}

QTransportAuthPrivate::QTransportAuthPrivate()
    : keyInitialised(false)
    , keyChanged(false)
{
}

QTransportAuthPrivate::~QTransportAuthPrivate()
{
    freeCache();
    while ( data.count() )
        delete data.takeLast();
}

/*!
  \internal
  Construct a new QTransportAuth
*/
QTransportAuth::QTransportAuth() : QObject(*new QTransportAuthPrivate)
{
    // qDebug( "creating transport auth" );
}

/*!
  \internal
  Destructor
*/
QTransportAuth::~QTransportAuth()
{
    // qDebug( "deleting transport auth" );
}

void QTransportAuth::setProcessKey( const char *authdata )
{
    qDebug( "setProcessKey" );
    Q_D(QTransportAuth);
    ::memcpy(&d->authKey, authdata, sizeof(struct AuthCookie));
    d->keyInitialised = true;
    d->keyChanged = true;
}

void QTransportAuth::registerPolicyReceiver( QObject *pr )
{
    Q_D(QTransportAuth);
    QPointer<QObject> guard = pr;
    d->policyReceivers.append(guard);
}

QTransportAuth::Data *QTransportAuth::connectTransport( unsigned char properties, int descriptor )
{
    Q_D(QTransportAuth);
    Data *data = new Data(properties, descriptor);
    data->status = Pending;
    d->data.append(data);
    return data;
}

/*!
  Is the transport trusted.  This is true iff data written into the
  transport medium cannot be intercepted or modified by another process.
  This is for example true for Unix Domain Sockets, but not for shared
  memory or UDP sockets.

  There is of course an underlying assumption that the kernel implementing
  the transport is sound, ie it cannot be compromised by writing to
  /dev/kmem or loading untrusted modules
*/
inline bool QTransportAuth::Data::trusted() const
{
    return (bool)(properties & Trusted);
}

/*!
  Assert that the transport is trusted.

  For example with respect to shared memory, if it is ensured that no untrusted
  root processes are running, and that unix permissions have been set such that
  any untrusted non-root processes do not have access rights, then a shared
  memory transport could be asserted to be trusted.

  \sa trusted()
*/
inline void QTransportAuth::Data::setTrusted( bool t )
{
    properties = t ? properties | Trusted : properties & ~Trusted;
}

/*!
  Is the transport connection oriented.  This is true iff once a connection
  has been accepted, and state established, then further messages over the
  transport are guaranteed to have come from the original connecting entity.
  This is for example true for Unix Domain Sockets, but not
  for shared memory or UDP sockets.

  By extension if the transport is not trusted() then it should not be
  assumed to be connection oriented, since spoofed connection information
  could be created.  For example if we assume the TCP/IP transport is
  trusted, it can be treated as connection oriented; but this is only the
  case if intervening routers are trusted.

  Connection oriented transports have authorization cached against the
  connection, and thus authorization is only done at connect time.
*/
inline bool QTransportAuth::Data::connection() const
{
    return (bool)(properties & Connection);
}

/*!
  Assert that the transport is connnection oriented

  \sa connection()
*/
inline void QTransportAuth::Data::setConnection( bool t )
{
    properties = t ? properties | Connection : properties & ~Connection;
}

/*!
  Return a pointer to the instance of this process's QTransportAuth object
*/
QTransportAuth *QTransportAuth::getInstance()
{
    static QTransportAuth theInstance;

    return &theInstance;
}

/*!
  Set the full path to the key file

  Since this is normally relative to Qtopia::qpeDir() this needs to be
  set within the qtopia framework.

  The keyfile should be protected by file permissions or by MAC rules
  such that it can only be read/written by the "qpe" server process
*/
void QTransportAuth::setKeyFilePath( const QString &path )
{
    Q_D(QTransportAuth);
    d->m_keyFilePath = path;
}

QString QTransportAuth::keyFilePath() const
{
    Q_D(const QTransportAuth);
    return d->m_keyFilePath;
}

void QTransportAuth::setLogFilePath( const QString &path )
{
    Q_D(QTransportAuth);
    d->m_logFilePath = path;
}

QString QTransportAuth::logFilePath() const
{
    Q_D(const QTransportAuth);
    return d->m_logFilePath;
}

bool QTransportAuth::isDiscoveryMode() const
{
#if defined(SXE_DISCOVERY)
    static bool checked = false;
    static bool yesItIs = false;

    if ( checked ) return yesItIs;

    yesItIs = ( getenv( "SXE_DISCOVERY_MODE" ) != 0 );
    if ( yesItIs )
    {
        qWarning("SXE Discovery mode on, ALLOWING ALL requests and logging to %s",
                 qPrintable(logFilePath()));
        QFile::remove( logFilePath() );
    }
    checked = true;
    return yesItIs;
#else
    return false;
#endif
}

/*!
  \internal
  Return the authorizer device mapped to this client.  Note that this
  could probably all be void* instead of QWSClient* for generality.
  Until the need for that rears its head its QWSClient* to save the casts.

  #### OK the need has arrived, but the public API is frozen.
*/
QIODevice *QTransportAuth::passThroughByClient( QWSClient *client ) const
{
    Q_D(const QTransportAuth);

    if ( client == 0 ) return 0;
    if ( d->buffersByClient.contains( reinterpret_cast<void*>( client )))
    {
        return d->buffersByClient[reinterpret_cast<void*>(client)];
    }
    // qWarning( "buffer not found for client %p", client );
    return 0;
}

/*!
  \internal
  Return a QIODevice pointer (to an internal QBuffer) which can be used
  to receive data after authorisation on transport \a d.

  The return QIODevice will act as a pass-through.

  The data will be consumed from \a iod and forwarded on to the returned
  QIODevice which can be connected to readyRead() signal handlers in
  place of the original QIODevice \a iod.

  This will be called in the server process to handle incoming
  authenticated requests.

  \sa setTargetDevice()
*/
QAuthDevice *QTransportAuth::recvBuf( QTransportAuth::Data *data, QIODevice *iod )
{
    Q_D(QTransportAuth);

    if (d->buffers.contains(data))
        return d->buffers[data];
    QAuthDevice *authBuf = new QAuthDevice( iod, data, QAuthDevice::Receive );
    for ( int i = 0; i < d->policyReceivers.count(); ++i )
    {
        connect( authBuf, SIGNAL(policyCheck(QTransportAuth::Data &, const QString &)),
                d->policyReceivers[i], SLOT(policyCheck(QTransportAuth::Data &, const QString &)));
    }
    // qDebug( "created new authbuf %p", authBuf );
    d->buffers[data] = authBuf;
    return authBuf;
}

/*!
  Return a QIODevice pointer (to an internal QBuffer) which can be used
  to write data onto, for authorisation on transport \a d.

  The return QIODevice will act as a pass-through.

  The data written to the return QIODevice will be forwarded on to the
  returned QIODevice.  In the case of a QTcpSocket, this will cause it
  to send out the data with the authentication information on it.

  This will be called in the client process to generate outgoing
  authenticated requests.

  \sa setTargetDevice()
*/
QAuthDevice *QTransportAuth::authBuf( QTransportAuth::Data *data, QIODevice *iod )
{
    Q_D(QTransportAuth);
    if (d->buffers.contains(data))
        return d->buffers[data];
    QAuthDevice *authBuf = new QAuthDevice( iod, data, QAuthDevice::Send );
    d->buffers[data] = authBuf;
    return authBuf;
}

const unsigned char *QTransportAuth::getClientKey( unsigned char progId )
{
    Q_D(QTransportAuth);
    return d->getClientKey( progId );
}

void QTransportAuth::invalidateClientKeyCache()
{
    Q_D(QTransportAuth);
    d->invalidateClientKeyCache();
}

QMutex *QTransportAuth::getKeyFileMutex()
{
    Q_D(QTransportAuth);
    return &d->keyfileMutex;
}

static struct AuthRecord *keyCache[ KEY_CACHE_SIZE ] = { 0 };

/*!
  \internal
  Free the key cache on destruction of this object

TODO: reimplement this using Qt structures
*/
void QTransportAuthPrivate::freeCache()
{
    int i;
    for ( i = 0; i < KEY_CACHE_SIZE; ++i )
    {
        if ( keyCache[i] == NULL )
            break;
        ::free( keyCache[i] );
        keyCache[i] = NULL;
    }
}

/*!
  \internal
  Find the client key for the \a progId.  If it is cached should be very
  fast, otherwise requires a read of the secret key file

  In the success case a pointer to the key is returned.  The pointer is
  to storage owned by this class, and should be used immediately.

  NULL is returned in the following cases:
  \list
    \o the keyfile could not be accessed - error condition
    \o there was no key for the supplied program id - key auth failed
  \endlist

  Note that for the Keyfile, there is multi-thread concurrency issues:
  the Keyfile can be read by the qpe process when QTransportAuth is
  verifying a request, and it can be read or written by the Monitor
  thread within the qpe process when monitor rekeying is being done.

  To protect against this, the keyfileMutex is used.

Invariant:
  qpe is the only process which can access the Keyfile, there are no
  multi-process concurrency issues (file locking is not required).
*/
const unsigned char *QTransportAuthPrivate::getClientKey(unsigned char progId)
{
    int fd, i;
    struct AuthRecord kr;
    for ( i = 0; i < KEY_CACHE_SIZE; ++i )
    {
        if ( keyCache[i] == NULL )
            break;
        if ( keyCache[i]->auth.progId == progId )
        {
            return (unsigned char *)(keyCache[i]);
        }
    }
    if ( i == KEY_CACHE_SIZE ) // cache buffer has wrapped
        i = 0;
    ::memset( &kr, 0, sizeof( kr ));
    fd = ::open( m_keyFilePath.toLocal8Bit().constData(), O_RDONLY );
    if ( fd == -1 )
    {
        perror( "couldnt open keyfile" );
        qWarning( "check keyfile path %s", qPrintable(m_keyFilePath) );
        return NULL;
    }
    QMutexLocker keyfileLocker( &keyfileMutex );
    while ( ::read( fd, &kr, sizeof( struct AuthRecord )) != 0 )
    {
        if ( kr.auth.progId == progId )
        {
            if ( keyCache[i] == NULL )
                keyCache[i] = (AuthRecord*)(malloc( sizeof( kr )));
#ifdef QTRANSPORTAUTH_DEBUG
            qDebug( "Found client key for prog %u", progId );
#endif
            memcpy( (char*)(keyCache[i]), &kr, sizeof( kr ));
            ::close( fd );
            return (unsigned char *)(keyCache[i]);
        }
    }
    ::close( fd );
#ifdef QTRANSPORTAUTH_DEBUG
    qWarning( "No valid key found for prog %u", progId );
#endif
    return NULL;
}

void QTransportAuthPrivate::invalidateClientKeyCache()
{
    QMutexLocker keyfileLocker( &keyfileMutex );
    for ( int i = 0; i < KEY_CACHE_SIZE; i++ )
    {
        if ( keyCache[i] == NULL )
            break;
        free( keyCache[i] );
        keyCache[i] = 0;
    }
}

////////////////////////////////////////////////////////////////////////
////
////  AuthDevice definition
////

QAuthDevice::QAuthDevice( QIODevice *parent, QTransportAuth::Data *data, AuthDirection dir )
    : QIODevice( parent )
    , d( data )
    , way( dir )
    , m_target( parent )
    , m_client( 0 )
{
    // qDebug( "constructing new auth device; parent %p", parent );
    if ( dir == Receive ) // server side
    {
        connect( m_target, SIGNAL(readyRead()),
                this, SLOT(recvReadyRead()));
    }
    connect( m_target, SIGNAL(bytesWritten(qint64)),
            this, SIGNAL(bytesWritten(qint64)) );
    open( QIODevice::WriteOnly | QIODevice::Unbuffered );
}

QAuthDevice::~QAuthDevice()
{
    // qDebug( "destroying authdevice" );
}

/*!
  \internal
  Store a pointer to the related device or instance which this
  authorizer is proxying for
*/
void QAuthDevice::setClient( void *cli )
{
    m_client = cli;
    QTransportAuth::getInstance()->d_func()->buffersByClient[cli] = this;
}

void *QAuthDevice::client() const
{
    return m_client;
}

/*
  \fn void QAuthDevice::authViolation(QTransportAuth::Data &)

  This signal is emitted if an authorization failure is generated, as
  described in checkAuth();

  \sa checkAuth()
*/


/*
  \fn void QAuthDevice::policyCheck(QTransportAuth::Data &transport, const QString &request )

  This signal is emitted when a transport successfully delivers a request
  and gives the opportunity to either deny or accept the request.

  This signal must be connected in the same thread, ie it cannot be queued.

  As soon as all handlers connected to this signal are processed the Allow or
  Deny state on the \a transport is checked, and the request is allowed or denied
  accordingly.

  \sa checkAuth()
*/

/*!
  \internal
  Reimplement QIODevice writeData method.

  For client end, when the device is written to the incoming data is
  processed and an authentication header calculated.  This is pushed
  into the target device, followed by the actual incoming data (the
  payload).

  For server end, the writeData implementation in QBuffer is called.
*/
qint64 QAuthDevice::writeData(const char *data, qint64 len)
{
    if ( way == Receive )  // server
    {
        Q_ASSERT( "Dont call writeData in server" );
        return 0;
    }
    // client
#ifdef QTRANSPORTAUTH_DEBUG
    char displaybuf[1024];
    qDebug( "write data %lli bytes, pid %li", len, ::getpid() );
#endif
    char header[QSXE_HEADER_LEN];
    qint64 bytes = 0;
    if ( authToMessage( *d, header, data, len ))
    {
        m_target->write( header, QSXE_HEADER_LEN );
#ifdef QTRANSPORTAUTH_DEBUG
        hexstring( displaybuf, (const unsigned char *)header, QSXE_HEADER_LEN );
        qDebug( "QAuthDevice::writeData - CLIENT: Header written: %s", displaybuf );
#endif
        bytes += QSXE_HEADER_LEN;
    }
    m_target->write( data, len );
    bytes += len;
#ifdef QTRANSPORTAUTH_DEBUG
    int bytesToDisplay = bytes;
    const unsigned char *dataptr = (const unsigned char *)data;
    qDebug( "QAuthDevice::writeData - CLIENT: Data written:" );
    while ( bytesToDisplay > 0 )
    {
        int amt = bytes < 500 ? bytes : 500;
        hexstring( displaybuf, dataptr, amt );
        qDebug( "\t\t%s", bytes > 0 ? displaybuf : "(null)" );
        dataptr += 500;
        bytesToDisplay -= 500;
    }
#endif
    if ( m_target->inherits( "QAbstractSocket" ))
        static_cast<QAbstractSocket*>(m_target)->flush();
    return bytes;
}

/*!
  Reimplement from QIODevice

  read data out of the internal message queue, reduce the queue by the amount
  read.
*/
qint64 QAuthDevice::readData( char *data, qint64 maxSize )
{
    if ( msgQueue.size() == 0 )
        return 0;
#ifdef QTRANSPORTAUTH_DEBUG
    char displaybuf[1024];
    hexstring( displaybuf, reinterpret_cast<const unsigned char *>(msgQueue.constData()),
            msgQueue.size() > 500 ? 500 : msgQueue.size() );
    qDebug( "QAuthDevice::readData() buffer %li - %s", msgQueue.size(), displaybuf );
#endif
    if ( maxSize > msgQueue.size() )
    {
        ::memcpy( data, msgQueue.constData(), msgQueue.size() );
        msgQueue.clear();
        msgQueue.resize(0);
        return msgQueue.size();
    }
    else
    {
        ::memcpy( data, msgQueue.constData(), maxSize );
        msgQueue.remove( 0, maxSize );
        return maxSize;
    }
}

/*!
  \internal
  Receive readyRead signal from the target recv device.  In response
  authorize the data, and write results out to the recvBuf() device
  for processing by the application.  Trigger the readyRead signal.

  Authorizing involves first checking the transport is valid, ie the
  handshake has either already been done and is cached on a trusted
  transport, or was valid with this message; then second passing the
  string representation of the service request up to any policyReceivers

  If either of these fail, the message is denied.  In discovery mode
  denied messages are allowed, but the message is logged.
*/
void QAuthDevice::recvReadyRead()
{
    qint64 bytes = m_target->bytesAvailable();
    if ( bytes <= 0 ) return;
    QUnixSocket *usock = static_cast<QUnixSocket*>(m_target);
    QUnixSocketMessage msg = usock->read();
    msgQueue.append( msg.bytes() );
    d->processId = msg.processId();
    // if "fragmented" packet 1/2 way through start of a command, ie
    // in the QWS msg type, cant do anything, come back later when
    // there's more of the packet
    if ( msgQueue.size() < (int)sizeof(int) )
    {
        qDebug() << "returning: msg size too small" << msgQueue.size();
        return;
    }

#ifdef QTRANSPORTAUTH_DEBUG
    char displaybuf[1024];
    hexstring( displaybuf, reinterpret_cast<const unsigned char *>(msgQueue.constData()),
            msgQueue.size() > 500 ? 500 : msgQueue.size() );
    qDebug( "recv ready read %lli bytes - msg %s", bytes, displaybuf );
#endif

    bool bufHasMessages = msgQueue.size() > (int)sizeof(int);
    while ( bufHasMessages )
    {
        unsigned char saveStatus = d->status;
        if ( !authFromMessage( *d, msgQueue, msgQueue.size() ))
        {
            // not all arrived yet?  come back later
            if (( d->status & QTransportAuth::ErrMask ) == QTransportAuth::TooSmall )
            {
                // qDebug() << "returning: auth header not all received";
                d->status = saveStatus;
                return;
            }
        }

        if (( d->status & QTransportAuth::ErrMask ) == QTransportAuth::NoMagic )
        {
            // no msg auth header, don't change the success status for connections
            if ( d->connection() )
                d->status = saveStatus;
        }
        else
        {
            // msg auth header detected and auth determined, remove hdr
            msgQueue = msgQueue.mid( QSXE_HEADER_LEN );
        }

        bufHasMessages = authorizeMessage();
    }
}

/**
  \internal
  Pre-process the message to determine what QWS command it is.  This
  information is used as the "request" for the purposes of authorization.

  The request and other data on the connnection (id, PID etc) are forwarded
  to all policy listeners by emitting a signal.

  The signal must be processed synchronously because on return the allow/deny
  status is used immediately to either drop or continue processing the message.
*/
bool QAuthDevice::authorizeMessage()
{
    QBuffer cmdBuf( &msgQueue );
    cmdBuf.open( QIODevice::ReadOnly | QIODevice::Unbuffered );
    QWSCommand::Type command_type = (QWSCommand::Type)(qws_read_uint( &cmdBuf ));
    QWSCommand *command = QWSCommand::factory(command_type);
    // if NULL, factory will have already printed warning for bogus
    // command_type just purge the bad stuff and attempt to recover
    if ( command == NULL )
    {
        msgQueue = msgQueue.mid( sizeof(int) );
        // qDebug() << "bad command - removing" << sizeof(int) << "bytes";
        return msgQueue.size() > (int)sizeof(int);
    }
    QString request( qws_getCommandTypeString( command_type ));
#ifndef QT_NO_COP
    // not all command arrived yet - come back later
    if ( !command->read( &cmdBuf ))
    {
        delete command;
        // qDebug() << msgQueue.size() << "buffer size: exhausted before command complete - returning";
        return false;
    }
    if ( command_type == QWSCommand::QCopSend )
    {
        QWSQCopSendCommand *sendCommand = static_cast<QWSQCopSendCommand*>(command);
        request += QString( "/QCop/%1/%2" ).arg( sendCommand->channel ).arg( sendCommand->message );
    }
    if ( command_type == QWSCommand::QCopRegisterChannel )
    {
        QWSQCopRegisterChannelCommand *registerCommand = static_cast<QWSQCopRegisterChannelCommand*>(command);
        request += QString( "/QCop/RegisterChannel/%1" ).arg( registerCommand->channel );
    }
#endif
    bool isAuthorized = true;
    QTransportAuth *auth = QTransportAuth::getInstance();
    if ( !request.isEmpty() && request != "Unknown" )
    {
        d->status &= QTransportAuth::ErrMask;  // clear the status
        // d is now carrying the PID from the readyRead
        emit policyCheck( *d, request );
        isAuthorized = (( d->status & QTransportAuth::StatusMask ) == QTransportAuth::Allow );
    }

#if defined(SXE_DISCOVERY)
    if (auth->isDiscoveryMode()) {
#ifndef QT_NO_TEXTSTREAM
        if (!auth->logFilePath().isEmpty()) {
            QFile log( auth->logFilePath() );
            if (!log.open(QIODevice::WriteOnly | QIODevice::Append)) {
                qWarning("Could not write to log in discovery mode: %s",
                         qPrintable(auth->logFilePath()));
            } else {
                QTextStream ts( &log );
                ts << d->progId << '\t' << ( isAuthorized ? "Allow" : "Deny" ) << '\t' << request << endl;
            }
        }
#endif
        isAuthorized = true;
    }
#endif
    // copy message into the authBuf...
    close();
    int commandSize = QWS_PROTOCOL_ITEM_SIZE( *command );
    if ( isAuthorized )
    {
#ifdef QTRANSPORTAUTH_DEBUG
        qDebug() << "authorized: releasing" << commandSize << "byte command" << request;
#endif
        open( QIODevice::ReadOnly | QIODevice::Unbuffered );
        emit QIODevice::readyRead();
        qDebug() << "return from emit";
    }
    else
    {
        qWarning( "%s - denied: for Program Id %u [PID %lu]"
#if defined(SXE_DISCOVERY)
                "(to turn on discovery mode, export SXE_DISCOVERY_MODE=1)"
#endif
                , qPrintable(request), d->progId, d->processId );
    }
    msgQueue = msgQueue.mid( commandSize );
    delete command;
    return msgQueue.size() > (int)sizeof(int);
}

/*!
  \internal
   Add authentication header to the beginning of a message

   Note that the per-process auth cookie is used.  This key should be rewritten in
   the binary image of the executable at install time to make it unique.

   For this to be secure some mechanism (eg MAC kernel or other
   permissions) must prevent other processes from reading the key.

   The buffer must have AUTH_SPACE(0) bytes spare at the beginning for the
   authentication header to be added.

   Returns true if header successfully added.  Will fail if the
   per-process key has not yet been set with setProcessKey()
*/
bool QAuthDevice::authToMessage( QTransportAuth::Data &d, char *hdr, const char *msg, int msgLen )
{
    // qDebug( "authToMessage(): prog id %u", d.progId );
    QTransportAuth *a = QTransportAuth::getInstance();
    // only authorize connection oriented transports once, unless key has changed
    if ( !a->d_func()->keyChanged && d.connection() &&
            (( d.status & QTransportAuth::ErrMask ) != QTransportAuth::Pending ))
        return false;
    a->d_func()->keyChanged = false;
    // If Unix socket credentials are being used the key wont be set
    if ( ! a->d_func()->keyInitialised )
        return false;
    unsigned char digest[QSXE_KEY_LEN];
    char *msgPtr = hdr;
    // magic always goes on the beginning
    for ( int m = 0; m < QSXE_MAGIC_BYTES; ++m )
        *msgPtr++ = magic[m];
    hdr[ QSXE_LEN_IDX ] = (unsigned char)msgLen;
    if ( !d.trusted())
    {
        // Use HMAC
        int rc = hmac_md5( (unsigned char *)msg, msgLen, a->d_func()->authKey.key, QSXE_KEY_LEN, digest );
        if ( rc == -1 )
            return false;
        memcpy( hdr + QSXE_KEY_IDX, digest, QSXE_KEY_LEN );
    }
    else
    {
        memcpy( hdr + QSXE_KEY_IDX, a->d_func()->authKey.key, QSXE_KEY_LEN );
    }

    hdr[ QSXE_PROG_IDX ] = a->d_func()->authKey.progId;

#ifdef QTRANSPORTAUTH_DEBUG
    char keydisplay[QSXE_KEY_LEN*2+1];
    hexstring( keydisplay, a->d_func()->authKey.key, QSXE_KEY_LEN );

    qDebug( "Auth to message %s against prog id %u and key %s\n",
            msg, a->d_func()->authKey.progId, keydisplay );
#endif

    // TODO implement sequence to prevent replay attack, not required
    // for trusted transports
    hdr[ QSXE_SEQ_IDX ] = 1;  // dummy sequence

    d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::Success;
    return true;
}


/*!
  Check authorization on the \a msg, which must be of size \a msgLen,
  for the transport \a d.

  If able to determine authorization, return the program identity of
  the message source in the reference \a progId, and return true.

  Otherwise return false.

  If data is being recieved on a socket, it may be that more data is yet
  needed before authentication can proceed.

  Also the message may not be an authenticated at all.

  In these cases the method returns false to indicate authorization could
  not be determined:
  \list
    \i The message is too small to carry the authentication data
       (status TooSmall is set on the \a d transport )
    \i The 4 magic bytes are missing from the message start
       (status NoMagic is set on the \a d transport )
    \i The message is too small to carry the auth + claimed payload
       (status TooSmall is set on the \a d transport )
  \endlist

  If however the authentication header (preceded by the magic bytes) and
  any authenticated payload is received the method will determine the
  authentication status, and return true.

  In the following cases as well as returning true it will also emit
  an authViolation():
  \list
    \i If the program id claimed by the message is not found in the key file
       (status NoSuchKey is set on the \a d transport )
    \i The authentication token failed against the claimed program id:
        \list
            \i in the case of trusted transports, the secret did not match
            \i in the case of untrusted transports the HMAC code did not match
        \endlist
       (status FailMatch is set on the \a d transport )
    \endlist

  In these cases the authViolation( QTransportAuth::Data d ) signal is emitted
  and the error string can be obtained from the status like this:
  \code
      QTransportAuth::Result r = d.status & QTransportAuth::ErrMask;
      qWarning( "error: %s", QTransportAuth::errorStrings[r] );
  \endcode
*/
bool QAuthDevice::authFromMessage( QTransportAuth::Data &d, const char *msg, int msgLen )
{
    if ( msgLen < QSXE_MAGIC_BYTES )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::TooSmall;
        return false;
    }
    // if no magic bytes, exit straight away
    int m;
    const unsigned char *mptr = reinterpret_cast<const unsigned char *>(msg);
    for ( m = 0; m < QSXE_MAGIC_BYTES; ++m )
    {
        if ( *mptr++ != magic[m] )
        {
            d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::NoMagic;
            return false;
        }
    }
    QTransportAuth *a = QTransportAuth::getInstance();
    if ( msgLen < AUTH_SPACE(1) )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::TooSmall;
        return false;
    }

#ifdef QTRANSPORTAUTH_DEBUG
    char authhdr[QSXE_HEADER_LEN*2+1];
    hexstring( authhdr, reinterpret_cast<const unsigned char *>(msg), QSXE_HEADER_LEN );
    qDebug( "authFromMessage(): message header is %s", authhdr );
#endif

    unsigned char authLen = (unsigned char)(msg[ QSXE_LEN_IDX ]);

    if ( msgLen < AUTH_SPACE(authLen) )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::TooSmall;
        return false;
    }
    unsigned char progbuf = (unsigned char)(msg[ QSXE_PROG_IDX ]);
    const unsigned char *clientKey = a->d_func()->getClientKey( progbuf );
    if ( clientKey == NULL )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::NoSuchKey;
        return false;
    }
    AuthRecord *ar = (AuthRecord *)clientKey;
    time_t now = time(0);
    if ( ar->change_time + QSXE_KEY_PERIOD < now )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::OutOfDate;
#ifdef QTRANSPORTAUTH_DEBUG
        qDebug( "authFromMessage() - key out of date" );
#endif
        return false;
    }

#ifdef QTRANSPORTAUTH_DEBUG
    char keydisplay[QSXE_KEY_LEN*2+1];
    hexstring( keydisplay, clientKey, QSXE_KEY_LEN );
    qDebug( "authFromMessage(): message %s against prog id %u and key %s\n",
            AUTH_DATA(msg), ((unsigned int)(msg[ QSXE_PROG_IDX ])), keydisplay );
#endif

    const unsigned char *auth_tok;
    unsigned char digest[QSXE_KEY_LEN];
    if ( !d.trusted())
    {
        hmac_md5( AUTH_DATA(msg), authLen, clientKey, QSXE_KEY_LEN, digest );
        auth_tok = digest;
    }
    else
    {
        auth_tok = clientKey;
    }
    mptr = reinterpret_cast<const unsigned char *>( msg + QSXE_KEY_IDX );
    for ( m = 0; m < QSXE_KEY_LEN; ++m )
    {
        if ( *mptr++ != *auth_tok++ )
        {
            d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::FailMatch;
            emit authViolation( d );
            return false;
        }
    }
    // TODO - provide sequence number check against replay attack
    // Note that this is only reqd for promiscuous transports (not UDS)
    d.progId = msg[QSXE_PROG_IDX];
    d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::Success;
    return true;
}


#ifdef QTRANSPORTAUTH_DEBUG
/*!
  In order to printf in hex, need to break the key up into unsigned ints
  so the %x format can be used.

  The target buf should be [ key_len * 2 + 1 ] in size
*/
void hexstring( char *buf, const unsigned char* key, size_t key_len )
{
    unsigned int i, p;
    for ( i = 0, p = 0; i < key_len; i++, p+=2 )
    {
        unsigned char lo_nibble = key[i] & 0x0f;
        unsigned char hi_nibble = key[i] >> 4;
        buf[p] = (int)hi_nibble > 9 ? hi_nibble-10 + 'A' : hi_nibble + '0';
        buf[p+1] = (int)lo_nibble > 9 ? lo_nibble-10 + 'A' : lo_nibble + '0';
    }
    buf[p] = '\0';
}
#endif

/*
  HMAC MD5 as listed in RFC 2104

  This code is taken from:

      http://www.faqs.org/rfcs/rfc2104.html

  with the allowance for keys other than length 16 removed, but otherwise
  a straight cut-and-paste.

  The HMAC_MD5 transform looks like:

  \code
      MD5(K XOR opad, MD5(K XOR ipad, text))
  \endcode

  \list
    \i where K is an n byte key
    \i ipad is the byte 0x36 repeated 64 times
    \i opad is the byte 0x5c repeated 64 times
    \i and text is the data being protected
  \endlist

  Hardware is available with accelerated implementations of HMAC-MD5 and
  HMAC-SHA1.  Where this hardware is available, this routine should be
  replaced with a call into the accelerated version.
*/

static int hmac_md5(
        unsigned char*  text,         /* pointer to data stream */
        int             text_length,  /* length of data stream */
        const unsigned char*  key,    /* pointer to authentication key */
        int             key_length,   /* length of authentication key */
        unsigned char * digest        /* caller digest to be filled in */
        )
{
        MD5_CTX context;
        unsigned char k_ipad[65];    /* inner padding - * key XORd with ipad */
        unsigned char k_opad[65];    /* outer padding - * key XORd with opad */
        int i;

        /* in this implementation key_length == 16 */
        if ( key_length != 16 )
        {
            fprintf( stderr, "Key length was %d - must be 16 bytes", key_length );
            return 0;
        }

        /* start out by storing key in pads */
        memset( k_ipad, 0, sizeof k_ipad );
        memset( k_opad, 0, sizeof k_opad );
        memcpy( k_ipad, key, key_length );
        memcpy( k_opad, key, key_length );

        /* XOR key with ipad and opad values */
        for (i=0; i<64; i++) {
                k_ipad[i] ^= 0x36;
                k_opad[i] ^= 0x5c;
        }

        /* perform inner MD5 */
        MD5Init(&context);                   /* init context for 1st pass */
        MD5Update(&context, k_ipad, 64);     /* start with inner pad */
        MD5Update(&context, text, text_length); /* then text of datagram */
        MD5Final(digest, &context);          /* finish up 1st pass */

        /* perform outer MD5 */
        MD5Init(&context);                   /* init context for 2nd pass */
        MD5Update(&context, k_opad, 64);     /* start with outer pad */
        MD5Update(&context, digest, 16);     /* then results of 1st * hash */
        MD5Final(digest, &context);          /* finish up 2nd pass */
        return 1;
}

#include "moc_qtransportauth_qws_p.cpp"

#endif // QT_NO_SXE
