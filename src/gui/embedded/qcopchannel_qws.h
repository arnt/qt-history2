/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
#endif // QT_H

#ifndef QT_NO_COP

class QWSClient;
class QCopChannelPrivate;

class QCopChannel : public QObject
{
    Q_OBJECT
public:
    QCopChannel(const QByteArray& channel, QObject* parent=0, const char* name=0);
    virtual ~QCopChannel();

    QByteArray channel() const;

    static bool isRegistered(const QByteArray& channel);
    static bool send(const QByteArray &channel, const QByteArray &msg);
    static bool send(const QByteArray &channel, const QByteArray &msg,
                      const QByteArray &data);

    static void sendLocally( const QByteArray &ch, const QByteArray &msg,
                               const QByteArray &data);

    virtual void receive(const QByteArray &msg, const QByteArray &data);

signals:
    void received(const QByteArray &msg, const QByteArray &data);

private:
    // server side
    static void registerChannel(const QString &ch, QWSClient *cl);
    static void detach(QWSClient *cl);
    static void answer(QWSClient *cl, const QByteArray &ch,
                        const QByteArray &msg, const QByteArray &data);
    // client side
    QCopChannelPrivate* d;

    friend class QWSServer;
    friend class QApplication;
};

#endif

#endif // QCOPCHANNEL_QWS_H
