/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOPCHANNEL_QWS_H
#define QCOPCHANNEL_QWS_H

#ifndef QT_H
#include "qobject.h"
#include "qcstring.h"
#endif // QT_H

#ifndef QT_NO_COP

class QWSClient;
class QCopChannelPrivate;

class QCopChannel : public QObject
{
    Q_OBJECT
public:
    QCopChannel( const QCString& channel, QObject* parent=0, const char* name=0 );
    virtual ~QCopChannel();

    QCString channel() const;

    static bool isRegistered( const QCString& channel );
    static bool send( const QCString &channel, const QCString &msg );
    static bool send( const QCString &channel, const QCString &msg,
		      const QByteArray &data );

    static void sendLocally(  const QCString &ch, const QCString &msg,
			       const QByteArray &data );

    virtual void receive( const QCString &msg, const QByteArray &data );

signals:
    void received( const QCString &msg, const QByteArray &data );

private:
    // server side
    static void registerChannel( const QString &ch, QWSClient *cl );
    static void detach( QWSClient *cl );
    static void answer( QWSClient *cl, const QCString &ch,
			const QCString &msg, const QByteArray &data );
    // client side
    QCopChannelPrivate* d;

    friend class QWSServer;
    friend class QApplication;
};

#endif

#endif // QCOPCHANNEL_QWS_H
