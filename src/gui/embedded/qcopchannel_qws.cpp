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

#include "qcopchannel_qws.h"

#ifndef QT_NO_COP

#include "qwsdisplay_qws.h"
#include "qwscommand_qws.h"
#include "qwindowsystem_qws.h"
#include "qlist.h"
#include "qmap.h"
#include "qdatastream.h"

#include "qdebug.h"

typedef QMap<QString, QList<QWSClient*> > QCopServerMap;
static QCopServerMap *qcopServerMap = 0;

typedef QMap<QString, QList<QCopChannel*> > QCopClientMap;
static QCopClientMap *qcopClientMap = 0;

class QCopChannelPrivate
{
public:
    QString channel;
};

/*!
    \class QCopChannel

    \brief The QCopChannel class provides communication capabilities
    between several clients.

    \ingroup qws

    The Qt Cop (QCOP) is a COmmunication Protocol, allowing clients to
    communicate both within the same address space and between
    different processes.

    Currently, this facility is only available for Qtopia Core. On X11
    and Windows we are exploring the use of existing standards such as
    DCOP and COM.

    QCopChannel provides send() and isRegistered() which are static
    functions usable without an object.

    The channel() function returns the name of the channel.

    In order to \e listen to the traffic on a channel, you should
    either subclass QCopChannel and reimplement receive(), or
    connect() to the received() signal.
*/

/*!
    Constructs a QCop channel and registers it with the server using
    the name \a channel. The standard \a parent argument is passed on
    to the QObject constructor.
*/

QCopChannel::QCopChannel(const QString& channel, QObject *parent) :
    QObject(parent)
{
    init(channel);
}

#ifdef QT3_SUPPORT
/*!
    Use the two argument overload, and call setObjectName() to \a name
    the instance, instead.
*/
QCopChannel::QCopChannel(const QString& channel, QObject *parent, const char *name) :
    QObject(parent)
{
    setObjectName(name);
    init(channel);
}
#endif

void QCopChannel::init(const QString& channel)
{
    d = new QCopChannelPrivate;
    d->channel = channel;

    if (!qt_fbdpy) {
        qFatal("QCopChannel: Must construct a QApplication "
                "before QCopChannel");
        return;
    }

    if (!qcopClientMap)
        qcopClientMap = new QCopClientMap;

    // do we need a new channel list ?
    QCopClientMap::Iterator it = qcopClientMap->find(channel);
    if (it != qcopClientMap->end()) {
        it.value().append(this);
        return;
    }

    it = qcopClientMap->insert(channel, QList<QCopChannel*>());
    it.value().append(this);

    // inform server about this channel
    qt_fbdpy->registerChannel(channel);
}

/*!
    Destroys the client's end of the channel and notifies the server
    that the client has closed its connection. The server will keep
    the channel open until the last registered client detaches.
*/

QCopChannel::~QCopChannel()
{
    QCopClientMap::Iterator it = qcopClientMap->find(d->channel);
    Q_ASSERT(it != qcopClientMap->end());
    it.value().removeAll(this);
    // still any clients connected locally ?
    if (it.value().isEmpty()) {
        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);
        s << d->channel;
        if (qt_fbdpy)
            send("", "detach()", data);
        qcopClientMap->remove(d->channel);
    }

    delete d;
}

/*!
    Returns the name of the channel.
*/

QString QCopChannel::channel() const
{
    return d->channel;
}

/*!
    This virtual function allows subclasses of QCopChannel to process
    data received from their channel.

    The default implementation emits the received() signal.

    Note that the format of \a data has to be well defined in order to
    extract the information it contains.

    Example:

    \code
        void MyClass::receive(const QString &msg, const QByteArray &data)
        {
            QDataStream in(data);
            if (msg == "execute(QString,QString)") {
                QString cmd;
                QString arg;
                in >> cmd >> arg;
                ...
            } else if (msg == "delete(QString)") {
                QString fileName;
                in >> fileName;
                ...
            } else {
                ...
            }
        }
    \endcode

    This example assumes that the \a msg is a DCOP-style function
    signature and the \a data contains the function's arguments. (See
    send().)

    Using the DCOP convention is a recommendation, but not a
    requirement. Whatever convention you use the sender and receiver
    \e must agree on the argument types.

    \sa send()
 */
void QCopChannel::receive(const QString& msg, const QByteArray &data)
{
    emit received(msg, data);
}

/*!
    \fn void QCopChannel::received(const QString& msg, const QByteArray &data)

    This signal is emitted with the \a msg and \a data whenever the
    receive() function gets incoming data.
*/

/*!
    Queries the server for the existence of \a channel.

    Returns true if \a channel is registered; otherwise returns false.
*/

bool QCopChannel::isRegistered(const QString&  channel)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << channel;
    if (!send("", "isRegistered()", data))
        return false;

    QWSQCopMessageEvent *e = qt_fbdpy->waitForQCopResponse();
    bool known = e->message == "known";
    delete e;
    return known;
}

/*!
    \overload
    Send the message \a msg on channel \a channel. The message will be
    distributed to all clients subscribed to the \a channel.

    \sa receive()
*/

bool QCopChannel::send(const QString& channel, const QString& msg)
{
    QByteArray data;
    return send(channel, msg, data);
}

/*!
    Send the message \a msg on channel \a channel with data \a data.
    The message will be distributed to all clients subscribed to the
    channel.

    Note that QDataStream provides a convenient way to fill the byte
    array with auxiliary data.

    Example:

    \code
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << QString("cat") << QString("file.txt");
        QCopChannel::send("System/Shell", "execute(QString,QString)",
                          data);
    \endcode

    Here the channel is "System/Shell". The \a msg is an arbitrary
    string, but in the example we've used the DCOP convention of
    passing a function signature. Such a signature is formatted as
    "functionname(types)" where types is a list of zero or more
    comma-separated type names, with no whitespace, no consts and no
    pointer or reference marks, i.e. no "*" or "&".

    Using the DCOP convention is a recommendation, but not a
    requirement. Whatever convention you use the sender and receiver
    \e must agree on the argument types.

    \sa receive()
*/

bool QCopChannel::send(const QString& channel, const QString& msg,
                       const QByteArray &data)
{
    if (!qt_fbdpy) {
        qFatal("QCopChannel::send: Must construct a QApplication "
                "before using QCopChannel");
        return false;
    }

    qt_fbdpy->sendMessage(channel, msg, data);

    return true;
}

class QWSServerSignalBridge : public QObject {
  Q_OBJECT

public:
  void emitNewChannel(const QString& channel);
  void emitRemovedChannel(const QString& channel);

  signals:
  void newChannel(const QString& channel);
  void removedChannel(const QString& channel);
};

void QWSServerSignalBridge::emitNewChannel(const QString& channel){
  emit newChannel(channel);
}

void QWSServerSignalBridge::emitRemovedChannel(const QString& channel) {
  emit removedChannel(channel);
}

/*!
    \internal
    Server side: subscribe client \a cl on channel \a ch.
*/

void QCopChannel::registerChannel(const QString& ch, QWSClient *cl)
{
    if (!qcopServerMap)
        qcopServerMap = new QCopServerMap;

    // do we need a new channel list ?
    QCopServerMap::Iterator it = qcopServerMap->find(ch);
    if (it == qcopServerMap->end())
      it = qcopServerMap->insert(ch, QList<QWSClient*>());

    // If this is the first client in the channel, announce the channel as being created.
    if (it.value().count() == 0) {
      QWSServerSignalBridge* qwsBridge = new QWSServerSignalBridge();
      connect(qwsBridge, SIGNAL(newChannel(QString)), qwsServer, SIGNAL(newChannel(QString)));
      qwsBridge->emitNewChannel(ch);
      delete qwsBridge;
    }

    it.value().append(cl);
}

/*!
    \internal
    Server side: unsubscribe \a cl from all channels.
*/

void QCopChannel::detach(QWSClient *cl)
{
    if (!qcopServerMap)
        return;

    QCopServerMap::Iterator it = qcopServerMap->begin();
    for (; it != qcopServerMap->end(); it++) {
      if (it.value().contains(cl)) {
        it.value().removeAll(cl);
        // If this was the last client in the channel, announce the channel as dead.
        if (it.value().count() == 0) {
          QWSServerSignalBridge* qwsBridge = new QWSServerSignalBridge();
          connect(qwsBridge, SIGNAL(removedChannel(QString)), qwsServer, SIGNAL(removedChannel(QString)));
          qwsBridge->emitRemovedChannel(it.key());
          delete qwsBridge;
        }
      }
    }
}

/*!
    \internal
    Server side: transmit the message to all clients registered to the
    specified channel.
*/

void QCopChannel::answer(QWSClient *cl, const QString& ch,
                          const QString& msg, const QByteArray &data)
{
    // internal commands
    if (ch.isEmpty()) {
        if (msg == "isRegistered()") {
            QByteArray c;
            QDataStream s(data);
            s >> c;
            bool known = qcopServerMap && qcopServerMap->contains(c)
                        && !((*qcopServerMap)[c]).isEmpty();
            QByteArray ans = known ? "known" : "unkown";
            QWSServer::sendQCopEvent(cl, "", ans, data, true);
            return;
        } else if (msg == "detach()") {
            QByteArray c;
            QDataStream s(data);
            s >> c;
            Q_ASSERT(qcopServerMap);
            QCopServerMap::Iterator it = qcopServerMap->find(c);
            if (it != qcopServerMap->end()) {
                Q_ASSERT(it.value().contains(cl));
                it.value().removeAll(cl);
                if (it.value().isEmpty()) {
                  // If this was the last client in the channel, announce the channel as dead
                  QWSServerSignalBridge* qwsBridge = new QWSServerSignalBridge();
                  connect(qwsBridge, SIGNAL(removedChannel(QString)), qwsServer, SIGNAL(removedChannel(QString)));
                  qwsBridge->emitRemovedChannel(it.key());
                  delete qwsBridge;
                  qcopServerMap->erase(it);
                }
            }
            return;
        }
        qWarning() << "QCopChannel: unknown internal command %s" << msg;
        QWSServer::sendQCopEvent(cl, "", "bad", data);
        return;
    }

    QList<QWSClient*> clist = (*qcopServerMap)[ch];
    if (clist.isEmpty()) {
        qWarning() << "QCopChannel: no client registered for channel %s" << ch;
        return;
    }

    for (int i=0; i < clist.size(); ++i) {
        QWSClient *c = clist.at(i);
        QWSServer::sendQCopEvent(c, ch, msg, data);
    }
}

/*!
    \internal
    Client side: distribute received event to the QCop instance managing the
    channel.
*/
void QCopChannel::sendLocally(const QString& ch, const QString& msg,
                                const QByteArray &data)
{
    Q_ASSERT(qcopClientMap);

    // filter out internal events
    if (ch.isEmpty())
        return;

    // feed local clients with received data
    QList<QCopChannel*> clients = (*qcopClientMap)[ch];
    for (int i = 0; i < clients.size(); ++i)
        clients.at(i)->receive(msg, data);
}
#include "qcopchannel_qws.moc"

#endif
