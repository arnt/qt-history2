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

#ifndef QTRANSPORTAUTH_QWS_P_H
#define QTRANSPORTAUTH_QWS_P_H

#include "qtransportauth_qws.h"
#include "qbuffer.h"

#ifndef QT_NO_QWS_MULTIPROCESS

#include "private/qobject_p.h"

/**
  \internal
  \class QAuthDevice

  \brief Pass-through QIODevice sub-class for authentication.

   Use this class to forward on or receive forwarded data over a real
   device for authentication.
*/
class QAuthDevice : public QBuffer
{
    Q_OBJECT
public:
    enum AuthDirection {
        Receive,
        Send
    };
    QAuthDevice( QIODevice *, QTransportAuth::Data *, AuthDirection );
    ~QAuthDevice();
    void setTarget( QIODevice *t ) { m_target = t; }
    QIODevice *target() const { return m_target; }
    void setClient( QWSClient *c );
    QWSClient *client() const;
    bool authToMessage( QTransportAuth::Data &d, char *hdr, const char *msg, int msgLen );
    bool authFromMessage( QTransportAuth::Data &d, const char *msg, int msgLen );
Q_SIGNALS:
    void authViolation( QTransportAuth::Data & );
    void policyCheck( QTransportAuth::Data &, const QString & );
protected:
    qint64 writeData(const char *, qint64 );
private Q_SLOTS:
    void recvReadyRead();
private:
    void authorizeMessage();

    QTransportAuth::Data *d;
    AuthDirection way;
    QIODevice *m_target;
    QWSClient *m_client;
};


#define KEY_TEMPLATE "XOXOXOauthOXOXOX99"
#define APP_KEY const char *_key = KEY_TEMPLATE;
#define QL_APP_KEY const char *_ql_key = KEY_TEMPLATE;

#define KEY_LEN 16
#define MAGIC_BYTES 4
#define MAX_PROG_ID 255

// Number of bytes of each message to authenticate.  Just need to ensure
// that the command at the beginning hasn't been tampered with.  This value
// does not matter for trusted transports.
#define AMOUNT_TO_AUTHENTICATE 200

/**
  \internal
  \class AuthCookie
  Struct to carry process authentication key and id
*/
#define HEADER_LEN 24

/**
  \macro AUTH_ID
  Macro to manage authentication header.  Format of header is:
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

  However this may change.  Instead of coding these numbers use the AUTH_ID,
  AUTH_KEY, AUTH_DATA and AUTH_SPACE macros.
*/

#define AUTH_ID(k) ((unsigned char)(k[KEY_LEN]))
#define AUTH_KEY(k) ((unsigned char *)(k))

// must be a largish -ve number under any endianess when cast as an int
const unsigned char magic[MAGIC_BYTES] = { 0xBA, 0xD4, 0xD4, 0xBA };
const int magicInt = 0xBAD4D4BA;

#define AUTH_DATA(x) (unsigned char *)((x) + HEADER_LEN)
#define AUTH_SPACE(x) ((x) + HEADER_LEN)
#define LEN_IDX 4
#define KEY_IDX 6
#define PROG_IDX 22
#define SEQ_IDX 23

#define KEYFILE "keyfile"

/**
  Header in above format, less the magic bytes.
  Useful for reading off the socket
*/
struct AuthHeader
{
    unsigned char len;
    unsigned char pad;
    unsigned char digest[KEY_LEN];
    unsigned char id;
    unsigned char seq;
};

/**
  Header in a form suitable for authentication routines
*/
struct AuthMessage
{
    AuthMessage()
    {
        ::memset( authData, 0, sizeof(authData) );
        ::memcpy( pad_magic, magic, MAGIC_BYTES );
    }
    unsigned char pad_magic[MAGIC_BYTES];
    union {
        AuthHeader hdr;
        char authData[sizeof(AuthHeader)];
    };
    char payLoad[AMOUNT_TO_AUTHENTICATE];
};

/**
  Auth data as written to the key file
*/
struct AuthCookie
{
    unsigned char key[KEY_LEN];
    unsigned char pad;
    unsigned char progId;
};

class QTransportAuthPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTransportAuth)
public:
    QTransportAuthPrivate();
    ~QTransportAuthPrivate();

    void freeCache();
    const unsigned char *getClientKey( unsigned char progId );

    bool keyInitialised;
    QString m_logFilePath;
    QString m_keyFilePath;
    AuthCookie authKey;
    QList<QTransportAuth::Data*> data;
    QHash<QTransportAuth::Data*,QAuthDevice*> buffers;
    QList< QPointer<QObject> > policyReceivers;
    QHash<QWSClient*,QIODevice*> buffersByClient;
};

#endif // QT_NO_QWS_MULTIPROCESS
#endif // QTRANSPORTAUTH_QWS_P_H

