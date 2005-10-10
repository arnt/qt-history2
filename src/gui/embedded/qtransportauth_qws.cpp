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

#ifndef QT_NO_QWS_MULTIPROCESS

#include "md5.h"
#include "qwsutils_qws.h"
#include "qwscommand_qws.h"
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

static int hmac_md5(
        unsigned char*  text,         /* pointer to data stream */
        int             text_length,  /* length of data stream */
        const unsigned char*  key,    /* pointer to authentication key */
        int             key_length,   /* length of authentication key */
        unsigned char * digest        /* caller digest to be filled in */
        );



#define KEY_CACHE_SIZE 10

const char * const errorStrings[] = {
    QT_TRANSLATE_NOOP( "Transport Auth error",  "pending identity verification" ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "message too small to carry auth data" ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "cache miss on connection oriented transport"  ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "no magic bytes on message"  ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "key not found for prog id"  ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "authorization key match failed"  )
};

const char *QTransportAuth::errorString( const Data &d )
{
    if (( d.status & ErrMask ) == Success )
        return "success";
    return errorStrings[( d.status & ErrMask )];
}

/**
  \internal
  Construct a new QTransportAuth
*/
QTransportAuth::QTransportAuth()
    : keyInitialised( false )
{
    // qDebug( "creating transport auth" );
}

/**
  \internal
  Destructor
*/
QTransportAuth::~QTransportAuth()
{
    freeCache();
    while ( data.count() )
        delete data.takeLast();
    // qDebug( "deleting transport auth" );
}

void QTransportAuth::setProcessKey( const char *authdata )
{
    // qDebug( "set process key" );
    ::memcpy( &authKey, authdata, sizeof(authKey) );
    keyInitialised = true;
}

void QTransportAuth::registerPolicyReceiver( QObject *pr )
{
    QPointer<QObject> guard = pr;
    policyReceivers.append( guard );
}

QTransportAuth::Data *QTransportAuth::connectTransport( unsigned char properties, int descriptor )
{
    Data *d = new Data( properties, descriptor );
    d->status = Pending;
    data.append( d );
    return d;
}

/**
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

/**
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

/**
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

/**
  Assert that the transport is connnection oriented

  \sa connection()
*/
inline void QTransportAuth::Data::setConnection( bool t )
{
    properties = t ? properties | Connection : properties & ~Connection;
}

/**
  Return a pointer to the instance of this process's QTransportAuth object
*/
QTransportAuth *QTransportAuth::getInstance()
{
    static QTransportAuth theInstance;

    return &theInstance;
}

/**
  Set the full path to the key file

  Since this is normally relative to Qtopia::qpeDir() this needs to be
  set within the qtopia framework.

  The keyfile should be protected by file permissions or by MAC rules
  such that it can only be read/written by the "qpe" server process
*/
void QTransportAuth::setKeyFilePath( const QString &path )
{
    m_keyFilePath = path;
}

QString QTransportAuth::keyFilePath() const
{
    return m_keyFilePath;
}

void QTransportAuth::setLogFilePath( const QString &path )
{
    m_logFilePath = path;
}

QString QTransportAuth::logFilePath() const
{
    return m_logFilePath;
}

bool QTransportAuth::isDiscoveryMode() const
{
#if defined(SXV_DISCOVERY)
    static bool checked = false;
    static bool yesItIs = false;

    if ( checked ) return yesItIs;

    yesItIs = ( getenv( "SXV_DISCOVERY_MODE" ) != 0 );
    if ( yesItIs )
    {
        qWarning("SXV Discovery mode on, ALLOWING ALL requests and logging to %s",
                 qPrintable(logFilePath()));
        QFile::remove( logFilePath() );
    }
    checked = true;
    return yesItIs;
#else
    return false;
#endif
}

/**
  \internal
  Return the authorizer device mapped to this client.  Note that this
  could probably all be void* instead of QWSClient* for generality.
  Until the need for that rears its head its QWSClient* to save the casts.
*/
QIODevice *QTransportAuth::passThroughByClient( QWSClient *client ) const
{
    if ( client == 0 ) return 0;
    if ( buffersByClient.contains( client ))
    {
        return buffersByClient[client];
    }
    qWarning( "buffer not found for client %p", client );
    return 0;
}

/**
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
QAuthDevice *QTransportAuth::recvBuf( QTransportAuth::Data *d, QIODevice *iod )
{
    if ( buffers.contains( d ))
        return buffers[d];
    QAuthDevice *authBuf = new QAuthDevice( iod, d, QAuthDevice::Receive );
    for ( int i = 0; i < policyReceivers.count(); ++i )
    {
        connect( authBuf, SIGNAL(policyCheck(QTransportAuth::Data &, const QString &)),
                policyReceivers[i], SLOT(policyCheck(QTransportAuth::Data &, const QString &)));
    }
    // qDebug( "created new authbuf %p", authBuf );
    buffers[d] = authBuf;
    return authBuf;
}

/**
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
QAuthDevice *QTransportAuth::authBuf( QTransportAuth::Data *d, QIODevice *iod )
{
    if ( buffers.contains( d ))
        return buffers[d];
    QAuthDevice *authBuf = new QAuthDevice( iod, d, QAuthDevice::Send );
    buffers[d] = authBuf;
    return authBuf;
}

static struct AuthCookie *keyCache[ KEY_CACHE_SIZE ] = { 0 };

/**
  \internal
  Free the key cache on destruction of this object

TODO: reimplement this using Qt structures
*/
void QTransportAuth::freeCache()
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

/**
  \internal
  Find the client key for the \a progId.  If it is cached should be very
  fast, otherwise requires a read of the secret key file
*/
const unsigned char *QTransportAuth::getClientKey( unsigned char progId )
{
    int fd, i;
    struct AuthCookie kr;
    for ( i = 0; i < KEY_CACHE_SIZE; ++i )
    {
        if ( keyCache[i] == NULL )
            break;
        if ( keyCache[i]->progId == progId )
            return keyCache[i]->key;
    }
    if ( i == KEY_CACHE_SIZE ) // cache buffer has wrapped
        i = 0;
    memset( &kr, 0, sizeof( kr ));
    fd = ::open( m_keyFilePath.toLocal8Bit().constData(), O_RDONLY );
    if ( fd == -1 )
    {
        perror( "couldnt open keyfile" );
        qWarning( "check keyfile path %s", m_keyFilePath.toLocal8Bit().constData() );
    }
    // block until file lock obtained
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    fcntl( fd, F_SETLKW, &lock );

    while ( ::read( fd, &kr, sizeof( struct AuthCookie )) != 0 )
    {
        if ( kr.progId == progId )
        {
            if ( keyCache[i] == NULL )
                keyCache[i] = (AuthCookie *)(malloc( sizeof( kr )));
            memcpy( keyCache[i]->key, kr.key, KEY_LEN );
#ifdef QTRANSPORTAUTH_DEBUG
            qDebug( "Found client key for prog %u", progId );
#endif
            ::close( fd );  // release lock
            return keyCache[i]->key;
        }
    }
    ::close( fd );  // release lock
    qWarning( "Not found client key for prog %u", progId );
    return NULL;
}


////////////////////////////////////////////////////////////////////////
////
////  AuthDevice definition
////

QAuthDevice::QAuthDevice( QIODevice *parent, QTransportAuth::Data *data, AuthDirection dir )
    : QBuffer( parent )
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
    open( QIODevice::WriteOnly );
}

QAuthDevice::~QAuthDevice()
{
    // qDebug( "destroying authdevice" );
}

void QAuthDevice::setClient( QWSClient *cli )
{
    m_client = cli;
    QTransportAuth::getInstance()->buffersByClient[cli] = this;
}

QWSClient *QAuthDevice::client() const
{
    return m_client;
}

/**
  \function authViolation(QTransportAuth::Data&)
  This signal is emitted if an authorization failure is generated, as
  described in checkAuth();

  \sa checkAuth()
*/


/**
  \function policyCheck(QTransportAuth::Data& transport, const QString &request )
  This signal is emitted when a transport successfully delivers a request
  and gives the opportunity to either deny or accept the request.

  This signal must be connected in the same thread, ie it cannot be queued.

  As soon as all handlers connected to this signal are processed the Allow or
  Deny state on the \a transport is checked, and the request is allowed or denied
  accordingly.

  \sa checkAuth()
*/

/**
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
        return QBuffer::writeData( data, len );
    // qDebug( "write data %lli bytes", len );
    char header[AUTH_SPACE(0)];
    qint64 bytes = 0;
    if ( authToMessage( *d, header, data, len ))
        bytes = QBuffer::writeData( header, AUTH_SPACE(0) );
    bytes += QBuffer::writeData( data, len );
    close();
    char buf[128];
    qint64 ba;
    qint64 tot = 0;
    open( QIODevice::ReadOnly );
    qint64 amtRead;
    qint64 amtWrit;
#ifdef QTRANSPORTAUTH_DEBUG
    char displaybuf[1024];
    char *dbufptr = displaybuf;
#endif
    while (( ba = bytesAvailable() ))
    {
        amtRead = read( buf, ba < 128 ? ba : 128 );
        amtWrit = m_target->write( buf, amtRead );
#ifdef QTRANSPORTAUTH_DEBUG
        if (( displaybuf + 1023 - dbufptr ) >= ( amtRead * 2 ))
        {
            hexstring( dbufptr, (unsigned char *)buf, amtRead );
            dbufptr += ( amtRead * 2 );
        }
#endif
        tot += amtWrit;
    }
    if ( m_target->inherits( "QAbstractSocket" ))
        reinterpret_cast<QAbstractSocket*>(m_target)->flush();
    close();
    buffer().resize( 0 );
    open( QIODevice::WriteOnly );
#ifdef QTRANSPORTAUTH_DEBUG
    qDebug( "%lli bytes written from authdevice to target: %s", tot, displaybuf );
#endif
    return tot;
}

/**
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
    char *lookahead = (char*)(malloc( bytes ));
    Q_CHECK_PTR( lookahead );
    qint64 peeked = m_target->peek( lookahead, bytes );
    if ( peeked == -1 )
    {
        qWarning( "socket/device error in auth: %s",
                m_target->errorString().toLocal8Bit().constData() );
        free( lookahead );
        return;
    }
#ifdef QTRANSPORTAUTH_DEBUG
    char displaybuf[1024];
    hexstring( displaybuf, reinterpret_cast<const unsigned char *>(lookahead), bytes > 500 ? 500 : bytes );
    qDebug( "recv ready read %lli bytes - msg %s", bytes, displaybuf );
#endif
    if ( !authFromMessage( *d, lookahead, bytes ))
    {
        // not all arrived yet?  come back later
        if (( d->status & QTransportAuth::ErrMask ) == QTransportAuth::TooSmall )
        {
            free( lookahead );
            return;
        }
    }
    free( lookahead );
    authorizeMessage();

    // ...and re-open, and triggering its readyRead
    open( QIODevice::ReadOnly );
    emit QBuffer::readyRead();
}

void QAuthDevice::authorizeMessage()
{
    char buf[128];
    // msg auth header detected and auth determined, remove hdr
    if (( d->status & QTransportAuth::ErrMask ) != QTransportAuth::NoMagic )
        m_target->read( buf, HEADER_LEN );
    QBuffer cmdBuf;
    cmdBuf.open( QIODevice::WriteOnly );
    qint64 bytes;
    while (( bytes = m_target->bytesAvailable() ))
    {
        m_target->read( buf, bytes > 128 ? 128 : bytes );
        cmdBuf.write( buf, bytes > 128 ? 128 : bytes );
    }
    cmdBuf.close();
    cmdBuf.open( QIODevice::ReadOnly );
    QWSCommand::Type command_type = (QWSCommand::Type)(qws_read_uint( &cmdBuf ));
    QString request( getCommandTypeString( command_type ));
#ifndef QT_NO_COP
    if ( command_type == QWSCommand::QCopSend )
    {
        QWSQCopSendCommand *command = reinterpret_cast<QWSQCopSendCommand*>(QWSCommand::factory(command_type));
        // not all command arrived yet - come back later
        if ( !command->read( &cmdBuf ))
            return;
        request += QString( "/QCop/%1/%2" ).arg( command->channel ).arg( command->message );
    }
#endif

    if ( !request.isEmpty() )
        emit policyCheck( *d, request );

    QTransportAuth *auth = QTransportAuth::getInstance();
    bool isAuthorized = (( d->status & QTransportAuth::StatusMask ) == QTransportAuth::Allow );
#if defined(SXV_DISCOVERY)
    if ( auth->isDiscoveryMode() )
    {
#ifndef QT_NO_TEXTSTREAM
        QFile log( auth->logFilePath() );
        if ( !log.open( QIODevice::WriteOnly | QIODevice::Append ))
        {
            qWarning("Could not write to log in discovery mode: %s",
                     qPrintable(auth->logFilePath()));
        }
        else
        {
            QTextStream ts( &log );
            ts << d->progId << '\t' << ( isAuthorized ? "Allow" : "Deny" ) << '\t' << request << endl;
        }
#endif
        isAuthorized = true;
    }
#endif
    // copy message into the authBuf...
    close();
    buffer().resize(0);
    if ( isAuthorized )
        setData( cmdBuf.buffer() );
#if defined(SXV_DISCOVERY)
    else
        qWarning("%s - denied: (to turn on discovery mode, export SXV_DISCOVERY_MODE=1",
                 qPrintable(request));
#endif
}

/**
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
    QTransportAuth *a = QTransportAuth::getInstance();
    // only authorize connection oriented transports once
    if ( d.connection() &&
            (( d.status & QTransportAuth::ErrMask ) != QTransportAuth::Pending ))
        return false;
    if ( ! a->keyInitialised )
    {
        qWarning( "Cannot add transport authentication - key not initialised!" );
        return false;
    }
    unsigned char digest[KEY_LEN];
    char *msgPtr = hdr;
    // magic always goes on the beginning
    for ( int m = 0; m < MAGIC_BYTES; ++m )
        *msgPtr++ = magic[m];
    hdr[ LEN_IDX ] = (unsigned char)msgLen;
    if ( !d.trusted())
    {
        // Use HMAC
        int rc = hmac_md5( (unsigned char *)msg, msgLen, a->authKey.key, KEY_LEN, digest );
        if ( rc == -1 )
            return false;
        memcpy( hdr + KEY_IDX, digest, KEY_LEN );
    }
    else
    {
        memcpy( hdr + KEY_IDX, a->authKey.key, KEY_LEN );
    }

    hdr[ PROG_IDX ] = a->authKey.progId;

#ifdef QTRANSPORTAUTH_DEBUG
    char keydisplay[KEY_LEN*2+1];
    hexstring( keydisplay, a->authKey.key, KEY_LEN );

    qDebug( "Auth to message %s against prog id %u and key %s\n",
            msg, a->authKey.progId, keydisplay );
#endif

    // TODO implement sequence to prevent replay attack, not required
    // for trusted transports
    hdr[ SEQ_IDX ] = 1;  // dummy sequence

    d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::Success;
    return true;
}


/**
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
    if ( msgLen < MAGIC_BYTES )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::TooSmall;
        return false;
    }
    // if no magic bytes, exit straight away
    int m;
    const unsigned char *mptr = reinterpret_cast<const unsigned char *>(msg);
    for ( m = 0; m < MAGIC_BYTES; ++m )
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
    char authhdr[HEADER_LEN*2+1];
    hexstring( authhdr, reinterpret_cast<const unsigned char *>(msg), HEADER_LEN );
    qDebug( "authFromMessage(): message header is %s", authhdr );
#endif

    unsigned char authLen = (unsigned char)(msg[ LEN_IDX ]);

    if ( msgLen < AUTH_SPACE(authLen) )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::TooSmall;
        return false;
    }
    unsigned char progbuf = (unsigned char)(msg[ PROG_IDX ]);
    const unsigned char *clientKey = a->getClientKey( progbuf );
    if ( clientKey == NULL )
    {
        d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::NoSuchKey;
        emit authViolation( d );
        return false;
    }

#ifdef QTRANSPORTAUTH_DEBUG
    char keydisplay[KEY_LEN*2+1];
    hexstring( keydisplay, clientKey, KEY_LEN );
    qDebug( "authFromMessage(): message %s against prog id %u and key %s\n",
            AUTH_DATA(msg), ((unsigned int)(msg[ PROG_IDX ])), keydisplay );
#endif

    const unsigned char *auth_tok;
    unsigned char digest[KEY_LEN];
    if ( !d.trusted())
    {
        hmac_md5( AUTH_DATA(msg), authLen, clientKey, KEY_LEN, digest );
        auth_tok = digest;
    }
    else
    {
        auth_tok = clientKey;
    }
    mptr = reinterpret_cast<const unsigned char *>( msg + KEY_IDX );
    for ( m = 0; m < KEY_LEN; ++m )
    {
        if ( *mptr++ != *auth_tok++ )
        {
            d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::FailMatch;
            emit authViolation( d );
            return false;
        }
    }
    // TODO - provide sequence number check against replay attack
    d.progId = msg[PROG_IDX];
    d.status = ( d.status & QTransportAuth::StatusMask ) | QTransportAuth::Success;
    return true;
}




#ifdef QTRANSPORTAUTH_DEBUG
/**
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

/**
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

#endif // QT_NO_QWS_MULTIPROCESS
