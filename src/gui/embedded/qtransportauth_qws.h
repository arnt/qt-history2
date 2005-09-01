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

#ifndef QTRANSPORTAUTH_QWS_H
#define QTRANSPORTAUTH_QWS_H

#include <QtCore/qobject.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

QT_MODULE(Gui)

// Uncomment to generate debug output
#define QTRANSPORTAUTH_DEBUG 1

#define KEY_LEN 16
#define MAGIC_BYTES 4

/**
  \internal
  \class AuthCookie
  \table
  \header \i BYTES  \i  CONTENT
     \row \i 0-3    \i  magic numbers
     \row \i 4      \i  length of authenticated data (max 255 bytes)
     \row i\ 5      \i  reserved
     \row \i 6-21   \i  MAC digest, or shared secret in case of simple auth
     \row \i 22     \i  program id
     \row \i 23     \i  sequence number
  \endtable
  Total length of the header is 24 bytes
*/
#define HEADER_LEN 24
struct AuthCookie
{
    unsigned char key[KEY_LEN];
    unsigned char progId;
    unsigned char seq;
};

#define AUTH_ID(k) ((unsigned char)(k[KEY_LEN]))
#define AUTH_KEY(k) ((unsigned char *)(k))

const unsigned char magic[MAGIC_BYTES] = { 0xBA, 0xD4, 0xF7, 0x38 };

#define AUTH_DATA(x) (unsigned char *)((x) + HEADER_LEN)
#define AUTH_SPACE(x) ((x) + HEADER_LEN)
#define LEN_IDX 4
#define KEY_IDX 6
#define PROG_IDX 22
#define SEQ_IDX 23

/**
  \class QTransportAuth
  \brief Authenticate a message transport.
  For performance reasons, message authentication is tied to an individual
  message transport instance.  For example in connection oriented transports
  the authentication cookie can be cached against the connection avoiding
  the overhead of authentication on every message.

  For each process there is one instance of the QTransportAuth object.
  For server processes it can determine the \link secure-exe-environ.html SXV
  Program Identity \endlink and provide access to policy data to determine if
  the message should be forwarded for action.  If not actioned, the message
  may be treated as being from a flawed or malicious process.

  Retrieve the instance with the getInstance() method.  The constructor is
  disabled and instances of QTransportAuth should never be constructed by
  calling classes.

  Check the authorization using the checkAuth() method.

  For client processes authentication information can be added using the
  addAuth() method.  Note that if the transport is connection oriented
  it will be assumed that the server has cached the authentication infor-
  mation and this method will be a no-op.

  Generally these methods should never need to be called except by lower
  level message handling logic.
*/
class Q_GUI_EXPORT QTransportAuth : public QObject
{
    Q_OBJECT
public:
    static QTransportAuth *getInstance();

    static const char *errorStrings[6];

    enum Result {
        // Error codes
        Pending = 0x00,
        TooSmall = 0x01,
        CacheMiss = 0x02,
        NoMagic = 0x03,
        NoSuchKey = 0x04,
        FailMatch = 0x05,
        // reserved for expansion
        Success = 0x1e,
        ErrMask = 0x1f,

        // Verification codes
        Allow = 0x20,
        Deny = 0x40,
        Ask = 0x60,
        // reserved
        StatusMask = 0xe0
    };

    enum Properties {
        Trusted = 0x01,
        Connection = 0x02,
        UnixStreamSock = 0x04,
        SharedMemory = 0x08,
        MessageQueue = 0x10,
        UDP = 0x20,
        TCP = 0x40,
        UserDefined = 0x80,
        TransportType = 0xfc
    };

    struct Data
    {
        unsigned char properties;
        unsigned char progId;
        unsigned char status;
        unsigned int descriptor;   // socket fd or shmget key

        bool operator==( const Data &rhs ) const
        {
            return ( this->properties & TransportType == rhs.properties & TransportType &&
                    this->descriptor == rhs.descriptor );
        }
    };

    void connectTransport( unsigned char properties, int descriptor );
    bool addAuth( char *msg, int msgLen, Data d );
    bool checkAuth( char *msg, int msgLen, Data &d );
    void setKeyFilePath( const QString & );
    void setProcessKey( const char * );

    Result authFromSocket( unsigned char properties, int descriptor );
    void authToSocket( unsigned char properties, int descriptor, char *msg, int len );

    bool trusted( QTransportAuth::Data );
    void setTrusted( bool, QTransportAuth::Data & );
    bool connection( QTransportAuth::Data );
    void setConnection( bool, QTransportAuth::Data & );

signals:
    void authViolation( QTransportAuth::Data );
    void policyCheck( QTransportAuth::Data );
public slots:
    void closeTransport(  unsigned char properties, int descriptor );
private:
    // users should never construct their own
    QTransportAuth();
    ~QTransportAuth();
    void freeCache();
    const unsigned char *getClientKey( unsigned char progId );

    bool keyInitialised;
    QString keyFilePath;
    AuthCookie authKey;
    QHash<Data, Data> data;
};

uint qHash( QTransportAuth::Data );

int hmac_md5(
        unsigned char*  text,         /* pointer to data stream */
        int             text_length,  /* length of data stream */
        const unsigned char*  key,    /* pointer to authentication key */
        int             key_length,   /* length of authentication key */
        unsigned char * digest        /* caller digest to be filled in */
        );


#ifdef QTRANSPORTAUTH_DEBUG
void stringify_key( char *buf, const unsigned char* key, size_t sz );
#endif

#endif // QTRANSPORTAUTH_QWS_H
