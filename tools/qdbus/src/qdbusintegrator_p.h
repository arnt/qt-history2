/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSINTEGRATOR_P_H
#define QDBUSINTEGRATOR_P_H

#include <dbus/dbus.h>

#include "qcoreevent.h"
#include "qeventloop.h"
#include "qhash.h"
#include "qobject.h"
#include "private/qobject_p.h"
#include "qlist.h"
#include "qpointer.h"
#include "qsemaphore.h"

#include "qdbusconnection.h"
#include "qdbusmessage.h"
#include "qdbusconnection_p.h"

class QDBusConnectionPrivate;

// Really private structs used by qdbusintegrator.cpp
// Things that aren't used by any other file

class QDBusErrorHelper: public QObject
{
    Q_OBJECT
    friend class QDBusConnectionPrivate;
public:
    inline QDBusErrorHelper(QObject *target, const char *member)
    { connect(this, SIGNAL(pendingCallError(QDBusError,QDBusMessage)), target, member, Qt::QueuedConnection); }
signals:
    void pendingCallError(const QDBusError &, const QDBusMessage &);
};

class QDBusReplyWaiter: public QEventLoop
{
    Q_OBJECT
public:
    QDBusMessage replyMsg;

public slots:
    void reply(const QDBusMessage &msg);
    void error(const QDBusError &error);
};

struct QDBusPendingCall
{
    QPointer<QObject> receiver;
    QList<int> metaTypes;
    int methodIdx;
    DBusPendingCall *pending;
    const QDBusConnectionPrivate *connection;
    const char *errorMethod;
    QDBusMessage message;
};

struct QDBusSlotCache
{
    struct Data
    {
        int flags;
        int slotIdx;
        QList<int> metaTypes;
    };
    typedef QMultiHash<QString, Data> Hash;
    Hash hash;
};
Q_DECLARE_METATYPE(QDBusSlotCache)

class CallDeliveryEvent: public QMetaCallEvent
{
public:
    CallDeliveryEvent(const QDBusConnection &c, int id, QObject *sender,
                      const QDBusMessage &msg, const QList<int> &types, int f = 0)
        : QMetaCallEvent(id, sender, -1), connection(c), message(msg), metaTypes(types), flags(f)
        { }

    int placeMetaCall(QObject *object)
    {
        QDBusConnectionPrivate::d(connection)->deliverCall(object, flags, message, metaTypes, id());
        return -1;
    }

private:
    QDBusConnection connection; // just for refcounting
    QDBusMessage message;
    QList<int> metaTypes;
    int flags;
};

class ActivateObjectEvent: public QMetaCallEvent
{
public:
    ActivateObjectEvent(const QDBusConnection &c, QObject *sender,
                        const QDBusConnectionPrivate::ObjectTreeNode &n,
                        int p, const QDBusMessage &m, QSemaphore *s = 0)
        : QMetaCallEvent(-1, sender, -1, 0, 0, 0, s), connection(c), node(n),
          pathStartPos(p), message(m), handled(false)
        { }
    ~ActivateObjectEvent();

    int placeMetaCall(QObject *);

private:
    QDBusConnection connection; // just for refcounting
    QDBusConnectionPrivate::ObjectTreeNode node;
    int pathStartPos;
    QDBusMessage message;
    bool handled;
};

class QDBusConnectionCallbackEvent : public QEvent
{
public:
    QDBusConnectionCallbackEvent()
        : QEvent(User), subtype(Subtype(0))
    { }

    union {
        DBusTimeout *timeout;
        DBusWatch *watch;
    };
    int fd;
    int extra;

    enum Subtype {
        AddTimeout = 0,
        RemoveTimeout,
        AddWatch,
        RemoveWatch,
        ToggleWatch
    } subtype;
};

#endif
