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

#ifndef QCOPCHANNEL_QWS_H
#define QCOPCHANNEL_QWS_H

#include "qobject.h"

#ifndef QT_NO_COP

class QWSClient;
class QCopChannelPrivate;

class QCopChannel : public QObject
{
    Q_OBJECT
public:
    QCopChannel(const char *channel, QObject *parent=0);
#ifdef QT_COMPAT
    QCopChannel(const char *channel, QObject *parent, const char *name);
#endif
    virtual ~QCopChannel();

    const char* channel() const;

    static bool isRegistered(const char * channel);
    static bool send(const char *channel, const char *msg);
    static bool send(const char *channel, const char *msg,
                      const QByteArray &data);

    static void sendLocally( const char *ch, const char *msg,
                               const QByteArray &data);

    virtual void receive(const char *msg, const QByteArray &data);

signals:
    void received(const char *msg, const QByteArray &data);

private:
    void init(const char *channel);

    // server side
    static void registerChannel(const char *ch, QWSClient *cl);
    static void detach(QWSClient *cl);
    static void answer(QWSClient *cl, const char *ch,
                        const char *msg, const QByteArray &data);
    // client side
    QCopChannelPrivate* d;

    friend class QWSServer;
    friend class QApplication;
};

#endif

#endif // QCOPCHANNEL_QWS_H
