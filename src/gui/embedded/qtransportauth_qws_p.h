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

#endif // QT_NO_QWS_MULTIPROCESS
#endif // QTRANSPORTAUTH_QWS_P_H

