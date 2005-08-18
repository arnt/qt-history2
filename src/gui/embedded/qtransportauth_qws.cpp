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

#define QTRANSPORTAUTHLIB 1

#include "qtransportauth_qws.h"
#include "md5.h"

#include "qwsutils_qws.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>

#define KEY_CACHE_SIZE 10

// Number of bytes of each message to authenticate.  Just need to ensure
// that the command at the beginning hasn't been tampered with.  This value
// does not matter for trusted transports.
#define AMOUNT_TO_AUTHENTICATE 200


// Uncomment this value to have the authentication mechanism always check
// for authentication bytes and remove them if detected.  Otherwise the
// client will be assumed to only ever put on the header on the first
// connection, and never at any other time
// #define AUTH_SOCK_SAFE 1

uint qHash( QTransportAuth::Data d )
{
    uint h = d.properties;
    return ( h << 8 ) ^ d.descriptor;
}

const char * QTransportAuth::errorStrings[] = {
    QT_TRANSLATE_NOOP( "Transport Auth error",  "pending identity verification" ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "message too small to carry auth data" ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "cache miss on connection oriented transport"  ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "no magic bytes on message"  ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "key not found for prog id"  ),
    QT_TRANSLATE_NOOP( "Transport Auth error",  "authorization key match failed"  )
};

/**
  \internal
  Construct a new QTransportAuth
*/
QTransportAuth::QTransportAuth()
    : keyInitialised( false )
{
}

/**
  \internal
  Destructor
*/
QTransportAuth::~QTransportAuth()
{
    freeCache();
}

void QTransportAuth::setProcessKey( const char *authdata )
{
    if ( keyInitialised ) return;
    ::memcpy( &authKey, authdata, sizeof(authKey) );
    keyInitialised = true;
}

void QTransportAuth::connectTransport( unsigned char properties, int descriptor )
{
    Data d;
    d.properties = properties;
    d.descriptor = descriptor;
    d.status = Pending;
    data[d] = d;
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
inline bool QTransportAuth::trusted( QTransportAuth::Data d )
{
    return (bool)(d.properties & Trusted);
}

/**
  Assert that the transport is trusted.

  For example with respect to shared memory, if it is ensured that no untrusted
  root processes are running, and that unix permissions have been set such that
  any untrusted non-root processes do not have access rights, then a shared
  memory transport could be asserted to be trusted.

  \sa trusted()
*/
inline void QTransportAuth::setTrusted( bool t, QTransportAuth::Data &d )
{
    d.properties = t ? d.properties | Trusted : d.properties & ~Trusted;
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

  Connection oriented transports may have authorization cached against the
  connection.
*/
inline bool QTransportAuth::connection( QTransportAuth::Data d )
{
    return (bool)(d.properties & Connection);
}

/**
  Assert that the transport is connnection oriented

  \sa connection()
*/
inline void QTransportAuth::setConnection( bool t, QTransportAuth::Data &d )
{
    d.properties = t ? d.properties | Connection : d.properties & ~Connection;
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
  Only required if it is other than the default of $QPEDIR/Settings/keyfile
  The keyfile should be protected by file permissions or by MAC rules
  such that it can only be read by the "qpe" server process,
  and can be written by the "install_key" tool, and appended to by the
  "PackageManager".
*/
inline void QTransportAuth::setKeyFilePath( const QString &path )
{
    keyFilePath = path;
}

/**
  \function violation(AuthError)
  This signal is emitted if an authorization failure is generated, as
  described in checkAuth();

  \sa checkAuth()
*/

void QTransportAuth::closeTransport( unsigned char properties, int descriptor )
{
    Data d;
    d.properties = properties;
    d.descriptor = descriptor;
    data.remove( d );
}

/**
  \internal
   Add authentication header to the beginning of a message

   Note that the per-process auth cookie is used.  This key should be rewritten in
   the binary image of the executable at install time to make it unique.

   For this to be secure some mechanism (eg MAC kernel or other
   permissions) must prevent other processes from reading the key.

   The buffer must have bytes spare at the beginning for the
   authentication header to be added.

   Use code like:
    \code
       char msg_buf[ AUTH_SPACE(100) ];
       memcpy( AUTH_DATA(msg_buf), &msg_data, 100 );
       bool result = QTransportAuth::getInstance()->addAuth( msg_buf, AUTH_SPACE(100), d );
   \endcode

   Returns true if header successfully added
*/
bool QTransportAuth::addAuth( char *msg, int msgLen, QTransportAuth::Data d )
{
    if ( ! keyInitialised )
    {
        qWarning( "Cannot add transport authentication - key not initialised!" );
        return false;
    }
    unsigned char digest[KEY_LEN];
    char *msgPtr = msg;
    // magic always goes on the beginning
    for ( int m = 0; m < MAGIC_BYTES; ++m )
        *msgPtr++ = magic[m];
    if ( msgLen > AMOUNT_TO_AUTHENTICATE )
        msgLen = AMOUNT_TO_AUTHENTICATE;
    msg[ LEN_IDX ] = (unsigned char)msgLen;
    if ( !trusted( d ))
    {
        // Use HMAC
        int rc = hmac_md5( AUTH_DATA(msg), msgLen, authKey.key, KEY_LEN, digest );
        if ( rc == -1 )
            return false;
        memcpy( msg + KEY_IDX, digest, KEY_LEN );
    }
    else
    {
        memcpy( msg + KEY_IDX, authKey.key, KEY_LEN );
    }

    msg[ PROG_IDX ] = authKey.progId;

#ifdef QTRANSPORTAUTH_DEBUG
    char keydisplay[KEY_LEN*2+1];
    stringify_key( keydisplay, authKey.key, KEY_LEN );

    qDebug( "adding auth to message %s against prog id %u and key %s\n",
            AUTH_DATA(msg), authKey.progId, keydisplay );
#endif

    // TODO implement sequence to prevent replay attack, not required
    // for trusted transports
    msg[ SEQ_IDX ] = 1;  // dummy sequence

    return true;
}

/**
  Add authentication information to the socket \a descriptor, of type
  \a properties.  The authentication will use the \a msg of \a len
  as the authenticated payload.
*/
void QTransportAuth::authToSocket( unsigned char properties, int descriptor, char *msg, int len )
{
    qDebug( "auth to socket %d: %s", descriptor, msg );
    struct Data d;
    d.properties = properties;
    d.descriptor = descriptor;
    if ( data.contains( d ))
        d = data[d];
    if ( connection( d ))
        return;
    char buf[AUTH_SPACE(AMOUNT_TO_AUTHENTICATE)];
    if ( len > AMOUNT_TO_AUTHENTICATE )
        len = AMOUNT_TO_AUTHENTICATE;
    if ( msg != NULL )
        memcpy( AUTH_DATA(buf), msg, len );
    if ( ! addAuth( buf, len, d ))
        return;
    int rs = ::send( d.descriptor, buf, len, 0 );
    if ( rs == -1 )
        perror( "putting auth data on socket" );
}

static struct AuthCookie *keyCache[ KEY_CACHE_SIZE ] = { 0 };

/**
  \internal
  Free the key cache on destruction of this object
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
    fd = ::open( keyFilePath.toLocal8Bit().constData(), O_RDONLY );
    while ( ::read( fd, &kr, sizeof( struct AuthCookie )) != 0 )
    {
        if ( kr.progId == progId )
        {
            if ( keyCache[i] == NULL )
                keyCache[i] = (AuthCookie *)(malloc( sizeof( kr )));
            memcpy( keyCache[i]->key, kr.key, KEY_LEN );
            return keyCache[i]->key;
        }
    }
    return NULL;
}

/**
  Check authorization on the \a msg, which must be of size \a msgLen.

  If successful return the program identity of the message source in the
  reference \a progId, and return true.

  Otherwise return false.

  This method will return false in the following cases:
  \list
    \i The message is too small to carry the authentication data
    \i The 4 magic bytes are missing from the message start
    \i The program id claimed by the message is not found in the key file
    \i The authentication token failed against the claimed program id:
        \list
            \i in the case of trusted transports, the secret did not match
            \i in the case of untrusted transports the HMAC code did not match
        \endlist
    \endlist

  In these cases the authViolation( QTransportAuth::Data d ) signal is emitted
  and the error code can be obtained by eg:
  \code
      QTransportAuth::Result r = d.status & QTransportAuth::ErrMask;
      qWarning( "error: %s", QTransportAuth::errorStrings[r]; );
  \endcode
*/
bool QTransportAuth::checkAuth( char *msg, int msgLen, QTransportAuth::Data &d )
{
    if ( msgLen < AUTH_SPACE(1) )
    {
        d.status = ( d.status & StatusMask ) | TooSmall;
        emit authViolation( d );
        return false;
    }
    // if no magic bytes, exit straight away
    int m;
    unsigned char *mptr = (unsigned char *)msg;
    for ( m = 0; m < MAGIC_BYTES; ++m )
    {
        if ( *mptr++ != magic[m] )
        {
            d.status = ( d.status & StatusMask ) | NoMagic;
            emit authViolation( d );
            return false;
        }
    }
    unsigned char authLen = (unsigned char)(msg[ LEN_IDX ]);
    // its OK to be more, we may be peeking ahead into the buffer
    if ( msgLen < AUTH_SPACE(authLen) )
    {
        d.status = ( d.status & StatusMask ) | TooSmall;
        emit authViolation( d );
        return false;
    }
    unsigned char progbuf = (unsigned char)(msg[ PROG_IDX ]);
    const unsigned char *clientKey = getClientKey( progbuf );
    if ( clientKey == NULL )
    {
        d.status = ( d.status & StatusMask ) | NoSuchKey;
        emit authViolation( d );
        return false;
    }

#ifdef QTRANSPORTAUTH_DEBUG
    char keydisplay[KEY_LEN*2+1];
    stringify_key( keydisplay, clientKey, KEY_LEN );

    qDebug( "checking auth of message %s against prog id %u and key %s\n",
            AUTH_DATA(msg), ((unsigned int)(msg[ PROG_IDX ])), keydisplay );
#endif

    const unsigned char *auth_tok;
    if ( !trusted( d ))
    {
        unsigned char digest[KEY_LEN];
        hmac_md5( AUTH_DATA(msg), authLen, clientKey, KEY_LEN, digest );
        auth_tok = digest;
    }
    else
    {
        auth_tok = clientKey;
    }
    for ( m = 0; m < KEY_LEN; ++m )
    {
        if ( *msg++ != *auth_tok++ )
        {
            d.status = ( d.status & StatusMask ) | FailMatch;
            emit authViolation( d );
            return false;
        }
    }
    // TODO - provide sequence number check against replay attack
    d.progId = msg[PROG_IDX];
    d.status = ( d.status & StatusMask ) | Success;
    return true;
}

/**
  Apply the checkAuth() method to the data on the socket represented by
  \a properties and \a descriptor.

  If the checkAuth() passes a checkPolicy() signal will be emitted
  with a QTransportAuth::Data object containing the program identity.
*/
QTransportAuth::Result QTransportAuth::authFromSocket( unsigned char properties, int descriptor )
{
    struct Data d;
    d.properties = properties;
    d.descriptor = descriptor;
    if ( data.contains( d ))
        d = data[d];
    if ( connection( d ))
    {
        if (( d.status & ErrMask ) == Success )
        {
            emit policyCheck( d );
            return Success;
        }
        if (( d.status & ErrMask ) != Pending )
        {
            d.status = Deny | CacheMiss;
            emit authViolation( d );
            return CacheMiss;
        }
    }
    char buf[AMOUNT_TO_AUTHENTICATE];
    int rs = ::recv( d.descriptor, buf, AMOUNT_TO_AUTHENTICATE, MSG_PEEK );
    bool authGood = checkAuth( buf, AMOUNT_TO_AUTHENTICATE, d );
    // dispel magic from message
    if ( authGood || (( d.status & ErrMask ) != NoMagic
            && ( d.status & ErrMask ) != TooSmall ))
        rs = ::recv( d.descriptor, buf, AUTH_SPACE( 0 ), 0 );
    return Success;
}


#ifdef QTRANSPORTAUTH_DEBUG
/**
  In order to printf in hex, need to break the key up into unsigned ints
  so the %x format can be used.

  The target buf should be [ key_len * 2 + 1 ] in size
*/
void stringify_key( char *buf, const unsigned char* key, size_t key_len )
{
    unsigned int i;
    const unsigned int *iptr = (const unsigned int *)key;
    for ( i = 0; i < key_len; i += sizeof(i), buf += sizeof(i)*2, iptr++ )
        sprintf( buf, "%x", *iptr );
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

int hmac_md5(
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
