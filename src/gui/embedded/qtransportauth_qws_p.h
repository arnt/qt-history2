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

#if !defined(QT_NO_SXV) || defined(SXV_INSTALLER)

#include <qmutex.h>

#include "private/qobject_p.h"

// Uncomment to generate debug output
// #define QTRANSPORTAUTH_DEBUG 1

#ifdef QTRANSPORTAUTH_DEBUG
void hexstring( char *buf, const unsigned char* key, size_t sz );
#endif

// Keys expire after a day
#define QSXV_KEY_PERIOD 86400

class QUnixSocketMessage;

/*!
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
    void setClient( QWSClient * );
    QWSClient *client() const;
    bool authToMessage( QTransportAuth::Data &, char *, const char *, int );
    bool authFromMessage( QTransportAuth::Data &, const char *, int );
Q_SIGNALS:
    void authViolation( QTransportAuth::Data & );
    void policyCheck( QTransportAuth::Data &, const QString & );
protected:
    qint64 writeData(const char *, qint64 );
private Q_SLOTS:
    void recvReadyRead();
private:
    bool authorizeMessage();

    QTransportAuth::Data *d;
    AuthDirection way;
    QIODevice *m_target;
    QWSClient *m_client;
    QByteArray msgQueue;
};

#define QSXV_KEY_LEN 16
#define QSXV_MAGIC_BYTES 4
#define QSXV_MAX_PROG_ID 255

// Number of bytes of each message to authenticate.  Just need to ensure
// that the command at the beginning hasn't been tampered with.  This value
// does not matter for trusted transports.
#define AMOUNT_TO_AUTHENTICATE 200

/*!
  \internal
  \class AuthCookie
  Struct to carry process authentication key and id
*/
#define QSXV_HEADER_LEN 24

/*!
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

#define AUTH_ID(k) ((unsigned char)(k[QSXV_KEY_LEN]))
#define AUTH_KEY(k) ((unsigned char *)(k))

// must be a largish -ve number under any endianess when cast as an int
const unsigned char magic[QSXV_MAGIC_BYTES] = { 0xBA, 0xD4, 0xD4, 0xBA };
const int magicInt = 0xBAD4D4BA;

#define AUTH_DATA(x) (unsigned char *)((x) + QSXV_HEADER_LEN)
#define AUTH_SPACE(x) ((x) + QSXV_HEADER_LEN)
#define QSXV_LEN_IDX 4
#define QSXV_KEY_IDX 6
#define QSXV_PROG_IDX 22
#define QSXV_SEQ_IDX 23

#define QSXV_KEYFILE "keyfile"

/*
  Header in above format, less the magic bytes.
  Useful for reading off the socket
*/
struct AuthHeader
{
    unsigned char len;
    unsigned char pad;
    unsigned char digest[QSXV_KEY_LEN];
    unsigned char id;
    unsigned char seq;
};

/*
  Header in a form suitable for authentication routines
*/
struct AuthMessage
{
    AuthMessage()
    {
        ::memset( authData, 0, sizeof(authData) );
        ::memcpy( pad_magic, magic, QSXV_MAGIC_BYTES );
    }
    unsigned char pad_magic[QSXV_MAGIC_BYTES];
    union {
        AuthHeader hdr;
        char authData[sizeof(AuthHeader)];
    };
    char payLoad[AMOUNT_TO_AUTHENTICATE];
};

/**
  Auth data as stored in _key
*/
struct AuthCookie
{
    unsigned char key[QSXV_KEY_LEN];
    unsigned char pad;
    unsigned char progId;
};

/*
  Auth data as written to the key file
*/
struct AuthRecord
{
    union {
        AuthCookie auth;
        char data[sizeof(struct AuthCookie)];
    };
    time_t change_time;
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
    bool keyChanged;
    QString m_logFilePath;
    QString m_keyFilePath;
    AuthCookie authKey;
    QList<QTransportAuth::Data*> data;
    QHash<QTransportAuth::Data*,QAuthDevice*> buffers;
    QList< QPointer<QObject> > policyReceivers;
    QHash<QWSClient*,QIODevice*> buffersByClient;
    QMutex keyfileMutex;
};

#endif // QT_NO_SXV
#endif // QTRANSPORTAUTH_QWS_P_H

