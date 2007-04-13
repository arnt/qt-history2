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

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qmetaobject.h>
#include <qobject.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qthread.h>

#include "qdbusargument.h"
#include "qdbusconnection_p.h"
#include "qdbusinterface_p.h"
#include "qdbusmessage.h"
#include "qdbusmetatype.h"
#include "qdbusmetatype_p.h"
#include "qdbusabstractadaptor.h"
#include "qdbusabstractadaptor_p.h"
#include "qdbusutil_p.h"
#include "qdbusmessage_p.h"
#include "qdbuscontext_p.h"
#include "qdbusintegrator_p.h"

#include "qdbusthreaddebug_p.h"

static bool isDebugging;
#define qDBusDebug              if (!::isDebugging); else qDebug

static inline QDebug operator<<(QDebug dbg, const QThread *th)
{
    dbg.nospace() << "QThread(ptr=" << (void*)th;
    if (th && !th->objectName().isEmpty())
        dbg.nospace() << ", name=" << th->objectName();
    dbg.nospace() << ")";
    return dbg.space();
}

#if QDBUS_THREAD_DEBUG
static inline QDebug operator<<(QDebug dbg, const QDBusConnectionPrivate *conn)
{
    dbg.nospace() << "QDBusConnection("
                  << "ptr=" << (void*)conn
                  << ", name=" << conn->name
                  << ", baseService=" << conn->baseService
                  << ", thread=";
    if (conn->thread() == QThread::currentThread())
        dbg.nospace() << "same thread";
    else
        dbg.nospace() << conn->thread();
    dbg.nospace() << ")";
    return dbg.space();
}

QDBUS_EXPORT void qdbusDefaultThreadDebug(int action, int condition, QDBusConnectionPrivate *conn)
{
    qDBusDebug() << QThread::currentThread()
                 << "QtDBus threading action" << action
                 << (condition == QDBusLockerBase::BeforeLock ? "before lock" :
                     condition == QDBusLockerBase::AfterLock ? "after lock" :
                     condition == QDBusLockerBase::BeforeUnlock ? "before unlock" :
                     condition == QDBusLockerBase::AfterUnlock ? "after unlock" :
                     condition == QDBusLockerBase::BeforePost ? "before event posting" :
                     condition == QDBusLockerBase::AfterPost ? "after event posting" :
                     condition == QDBusLockerBase::BeforeDeliver ? "before event delivery" :
                     condition == QDBusLockerBase::AfterDeliver ? "after event delivery" :
                     condition == QDBusLockerBase::BeforeAcquire ? "before acquire" :
                     condition == QDBusLockerBase::AfterAcquire ? "after acquire" :
                     condition == QDBusLockerBase::BeforeRelease ? "before release" :
                     condition == QDBusLockerBase::AfterRelease ? "after release" :
                     "condition unknown")
                 << "in connection" << conn;
}
QDBUS_EXPORT qdbusThreadDebugFunc qdbusThreadDebug = qdbusDefaultThreadDebug;
#endif

typedef void (*QDBusSpyHook)(const QDBusMessage&);
typedef QVarLengthArray<QDBusSpyHook, 4> QDBusSpyHookList;
Q_GLOBAL_STATIC(QDBusSpyHookList, qDBusSpyHookList)

static bool qDBusRealAddTimeout(QDBusConnectionPrivate *d, DBusTimeout *timeout, int ms);
static dbus_bool_t qDBusAddTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

  //  qDebug("addTimeout %d", dbus_timeout_get_interval(timeout));

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);

    if (!dbus_timeout_get_enabled(timeout))
        return true;

    if (QThread::currentThread() == d->thread()) {
        // correct thread
        return qDBusRealAddTimeout(d, timeout, dbus_timeout_get_interval(timeout));
    } else {
        // wrong thread: sync back
        QDBusConnectionCallbackEvent *ev = new QDBusConnectionCallbackEvent;
        ev->subtype = QDBusConnectionCallbackEvent::AddTimeout;
        ev->timeout = timeout;
        ev->extra = dbus_timeout_get_interval(timeout);
        d->postEventToThread(AddTimeoutAction, d, ev);
        return true;
    }
}

static bool qDBusRealAddTimeout(QDBusConnectionPrivate *d, DBusTimeout *timeout, int ms)
{
    int timerId = d->startTimer(ms);
    if (!timerId)
        return false;

    d->timeouts[timerId] = timeout;
    return true;
}

static void qDBusRealRemoveTimeout(QDBusConnectionPrivate *d, DBusTimeout *timeout);
static void qDBusRemoveTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

  //  qDebug("removeTimeout");

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);

    if (QThread::currentThread() == d->thread()) {
        qDBusRealRemoveTimeout(d, timeout);
    } else {
        QDBusConnectionCallbackEvent *ev = new QDBusConnectionCallbackEvent;
        ev->subtype = QDBusConnectionCallbackEvent::RemoveTimeout;
        ev->timeout = timeout;
        d->postEventToThread(RemoveTimeoutAction, d, ev);
    }
}

static void qDBusRealRemoveTimeout(QDBusConnectionPrivate *d, DBusTimeout *timeout)
{
    QDBusConnectionPrivate::TimeoutHash::iterator it = d->timeouts.begin();
    while (it != d->timeouts.end()) {
        if (it.value() == timeout) {
            d->killTimer(it.key());
            it = d->timeouts.erase(it);
        } else {
            ++it;
        }
    }
}

static void qDBusToggleTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

    //qDebug("ToggleTimeout");

    qDBusRemoveTimeout(timeout, data);
    qDBusAddTimeout(timeout, data);
}

static bool qDBusRealAddWatch(QDBusConnectionPrivate *d, DBusWatch *watch, int flags, int fd);
static dbus_bool_t qDBusAddWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);

    int flags = dbus_watch_get_flags(watch);
    int fd = dbus_watch_get_fd(watch);

    if (QThread::currentThread() == d->thread()) {
        return qDBusRealAddWatch(d, watch, flags, fd);
    } else {
        QDBusConnectionCallbackEvent *ev = new QDBusConnectionCallbackEvent;
        ev->subtype = QDBusConnectionCallbackEvent::AddWatch;
        ev->watch = watch;
        ev->fd = fd;
        ev->extra = flags;
        d->postEventToThread(AddWatchAction, d, ev);
        return true;
    }
}

static bool qDBusRealAddWatch(QDBusConnectionPrivate *d, DBusWatch *watch, int flags, int fd)
{
    QDBusConnectionPrivate::Watcher watcher;
    if (flags & DBUS_WATCH_READABLE) {
        //qDebug("addReadWatch %d", fd);
        watcher.watch = watch;
        if (QCoreApplication::instance()) {
            watcher.read = new QSocketNotifier(fd, QSocketNotifier::Read, d);
            watcher.read->setEnabled(dbus_watch_get_enabled(watch));
            d->connect(watcher.read, SIGNAL(activated(int)), SLOT(socketRead(int)));
        }
    }
    if (flags & DBUS_WATCH_WRITABLE) {
        //qDebug("addWriteWatch %d", fd);
        watcher.watch = watch;
        if (QCoreApplication::instance()) {
            watcher.write = new QSocketNotifier(fd, QSocketNotifier::Write, d);
            watcher.write->setEnabled(dbus_watch_get_enabled(watch));
            d->connect(watcher.write, SIGNAL(activated(int)), SLOT(socketWrite(int)));
        }
    }
    d->watchers.insertMulti(fd, watcher);

    return true;
}

static void qDBusRealRemoveWatch(QDBusConnectionPrivate *d, DBusWatch *watch, int fd);
static void qDBusRemoveWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    //qDebug("remove watch");

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);
    int fd = dbus_watch_get_fd(watch);

    if (QThread::currentThread() == d->thread()) {
        qDBusRealRemoveWatch(d, watch, fd);
    } else {
        QDBusConnectionCallbackEvent *ev = new QDBusConnectionCallbackEvent;
        ev->subtype = QDBusConnectionCallbackEvent::RemoveWatch;
        ev->watch = watch;
        ev->fd = fd;
        d->postEventToThread(RemoveWatchAction, d, ev);
    }
}

static void qDBusRealRemoveWatch(QDBusConnectionPrivate *d, DBusWatch *watch, int fd)
{
    QDBusConnectionPrivate::WatcherHash::iterator i = d->watchers.find(fd);
    while (i != d->watchers.end() && i.key() == fd) {
        if (i.value().watch == watch) {
            delete i.value().read;
            delete i.value().write;
            d->watchers.erase(i);
            return;
        }
        ++i;
    }
}

static void qDBusRealToggleWatch(QDBusConnectionPrivate *d, DBusWatch *watch, int fd);
static void qDBusToggleWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);
    int fd = dbus_watch_get_fd(watch);

    if (QThread::currentThread() == d->thread()) {
        qDBusRealToggleWatch(d, watch, fd);
    } else {
        QDBusConnectionCallbackEvent *ev = new QDBusConnectionCallbackEvent;
        ev->subtype = QDBusConnectionCallbackEvent::ToggleWatch;
        ev->watch = watch;
        ev->fd = fd;
        d->postEventToThread(ToggleWatchAction, d, ev);
    }
}

static void qDBusRealToggleWatch(QDBusConnectionPrivate *d, DBusWatch *watch, int fd)
{
    QDBusConnectionPrivate::WatcherHash::iterator i = d->watchers.find(fd);
    while (i != d->watchers.end() && i.key() == fd) {
        if (i.value().watch == watch) {
            bool enabled = dbus_watch_get_enabled(watch);
            int flags = dbus_watch_get_flags(watch);

            //qDebug("toggle watch %d to %d (write: %d, read: %d)", dbus_watch_get_fd(watch), enabled, flags & DBUS_WATCH_WRITABLE, flags & DBUS_WATCH_READABLE);

            if (flags & DBUS_WATCH_READABLE && i.value().read)
                i.value().read->setEnabled(enabled);
            if (flags & DBUS_WATCH_WRITABLE && i.value().write)
                i.value().write->setEnabled(enabled);
            return;
        }
        ++i;
    }
}

static void qDBusNewConnection(DBusServer *server, DBusConnection *c, void *data)
{
    Q_ASSERT(data); Q_ASSERT(server); Q_ASSERT(c);
    Q_UNUSED(data); Q_UNUSED(server); Q_UNUSED(c);

    qDebug("SERVER: GOT A NEW CONNECTION"); // TODO
}

static QByteArray buildMatchRule(const QString &service, const QString & /*owner*/,
                                 const QString &objectPath, const QString &interface,
                                 const QString &member, const QString & /*signature*/)
{
    QString result = QLatin1String("type='signal',");
    QString keyValue = QLatin1String("%1='%2',");

    if (!service.isEmpty())
        result += keyValue.arg(QLatin1String("sender"), service);
    if (!objectPath.isEmpty())
        result += keyValue.arg(QLatin1String("path"), objectPath);
    if (!interface.isEmpty())
        result += keyValue.arg(QLatin1String("interface"), interface);
    if (!member.isEmpty())
        result += keyValue.arg(QLatin1String("member"), member);

    result.chop(1);             // remove ending comma
    return result.toLatin1();
}

static bool findObject(const QDBusConnectionPrivate::ObjectTreeNode *root,
                       const QString &fullpath, int &usedLength,
                       QDBusConnectionPrivate::ObjectTreeNode &result)
{
    int start = 0;
    int length = fullpath.length();
    if (fullpath.at(0) == QLatin1Char('/'))
        start = 1;

    // walk the object tree
    const QDBusConnectionPrivate::ObjectTreeNode *node = root;
    while (start < length && node && !(node->flags & QDBusConnection::ExportChildObjects)) {
        int end = fullpath.indexOf(QLatin1Char('/'), start);
        end = (end == -1 ? length : end);
        QStringRef pathComponent(&fullpath, start, end - start);

        QDBusConnectionPrivate::ObjectTreeNode::DataList::ConstIterator it =
            qLowerBound(node->children.constBegin(), node->children.constEnd(), pathComponent);
        if (it != node->children.constEnd() && it->name == pathComponent)
            // match
            node = it;
        else
            node = 0;

        start = end + 1;
    }

    // found our object
    usedLength = (start > length ? length : start);
    if (node) {
        if (node->obj || !node->children.isEmpty())
            result = *node;
        else
            // there really is no object here
            // we're just looking at an unused space in the QVector
            node = 0;
    }
    return node;
}

static QObject *findChildObject(const QDBusConnectionPrivate::ObjectTreeNode *root,
                                const QString &fullpath, int start)
{
    int length = fullpath.length();

    // any object in the tree can tell us to switch to its own object tree:
    const QDBusConnectionPrivate::ObjectTreeNode *node = root;
    if (node && node->flags & QDBusConnection::ExportChildObjects) {
        QObject *obj = node->obj;

        while (obj) {
            if (start >= length)
                // we're at the correct level
                return obj;

            int pos = fullpath.indexOf(QLatin1Char('/'), start);
            pos = (pos == -1 ? length : pos);
            QStringRef pathComponent(&fullpath, start, pos - start);

            const QObjectList children = obj->children();

            // find a child with the proper name
            QObject *next = 0;
            QObjectList::ConstIterator it = children.constBegin();
            QObjectList::ConstIterator end = children.constEnd();
            for ( ; it != end; ++it)
                if ((*it)->objectName() == pathComponent) {
                    next = *it;
                    break;
                }

            if (!next)
                break;

            obj = next;
            start = pos + 1;
        }
    }

    // object not found
    return 0;
}

extern QDBUS_EXPORT void qDBusAddSpyHook(QDBusSpyHook);
void qDBusAddSpyHook(QDBusSpyHook hook)
{
    qDBusSpyHookList()->append(hook);
}

extern "C" {
static DBusHandlerResult
qDBusSignalFilter(DBusConnection *connection, DBusMessage *message, void *data)
{
    Q_ASSERT(data);
    Q_UNUSED(connection);
    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);
    if (d->mode == QDBusConnectionPrivate::InvalidMode)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    QDBusMessage amsg = QDBusMessagePrivate::fromDBusMessage(message);
    qDBusDebug() << QThread::currentThread() << "got message:" << amsg;

    return d->handleMessage(amsg) ?
        DBUS_HANDLER_RESULT_HANDLED :
        DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
}

bool QDBusConnectionPrivate::handleMessage(const QDBusMessage &amsg)
{
    const QDBusSpyHookList *list = qDBusSpyHookList();
    for (int i = 0; i < list->size(); ++i) {
        qDBusDebug() << "calling the message spy hook";
        (*(*list)[i])(amsg);
    }

    switch (amsg.type()) {
    case QDBusMessage::SignalMessage:
        handleSignal(amsg);
        return true;
        break;
    case QDBusMessage::MethodCallMessage:
        handleObjectCall(amsg);
        return true;
    case QDBusMessage::ReplyMessage:
    case QDBusMessage::ErrorMessage:
        qWarning("QDBusConnection received a message of type %d that it shouldn't have",
                 amsg.type());
        return false;           // we shouldn't have got them here
    case QDBusMessage::InvalidMessage:
        Q_ASSERT_X(false, "QDBusConnection", "Invalid message found when processing");
        break;
    }

    return false;
}

static void huntAndDestroy(QObject *needle, QDBusConnectionPrivate::ObjectTreeNode &haystack)
{
    QDBusConnectionPrivate::ObjectTreeNode::DataList::Iterator it = haystack.children.begin();
    QDBusConnectionPrivate::ObjectTreeNode::DataList::Iterator end = haystack.children.end();
    for ( ; it != end; ++it)
        huntAndDestroy(needle, *it);

    if (needle == haystack.obj) {
        haystack.obj = 0;
        haystack.flags = 0;
    }
}

static void huntAndEmit(DBusConnection *connection, DBusMessage *msg,
                        QObject *needle, const QDBusConnectionPrivate::ObjectTreeNode &haystack,
                        bool isScriptable, bool isAdaptor, const QString &path = QString())
{
    QDBusConnectionPrivate::ObjectTreeNode::DataList::ConstIterator it = haystack.children.constBegin();
    QDBusConnectionPrivate::ObjectTreeNode::DataList::ConstIterator end = haystack.children.constEnd();
    for ( ; it != end; ++it)
        huntAndEmit(connection, msg, needle, *it, isScriptable, isAdaptor, path + QLatin1String("/") + it->name);

    if (needle == haystack.obj) {
        // is this a signal we should relay?
        if (isAdaptor && (haystack.flags & QDBusConnection::ExportAdaptors) == 0)
            return;             // no: it comes from an adaptor and we're not exporting adaptors
        else if (!isAdaptor) {
            int mask = isScriptable
                       ? QDBusConnection::ExportScriptableSignals
                       : QDBusConnection::ExportNonScriptableSignals;
            if ((haystack.flags & mask) == 0)
                return;         // signal was not exported
        }

        QByteArray p = path.toLatin1();
        if (p.isEmpty())
            p = "/";
        qDBusDebug() << QThread::currentThread() << "emitting signal at" << p;
        DBusMessage *msg2 = dbus_message_copy(msg);
        dbus_message_set_path(msg2, p);
        dbus_connection_send(connection, msg2, 0);
        dbus_message_unref(msg2);
    }
}

static int findSlot(const QMetaObject *mo, const QByteArray &name, int flags,
                    const QString &signature_, QList<int>& metaTypes)
{
    QByteArray msgSignature = signature_.toLatin1();

    for (int idx = mo->methodCount() - 1 ; idx >= QObject::staticMetaObject.methodCount(); --idx) {
        QMetaMethod mm = mo->method(idx);

        // check access:
        if (mm.access() != QMetaMethod::Public)
            continue;

        // check type:
        // unnecessary, since signals are never public:
        //if (mm.methodType() != QMetaMethod::Slot)
        //    continue;

        // check name:
        QByteArray slotname = mm.signature();
        int paren = slotname.indexOf('(');
        if (paren != name.length() || !slotname.startsWith(name))
            continue;

        int returnType = qDBusNameToTypeId(mm.typeName());
        bool isAsync = qDBusCheckAsyncTag(mm.tag());
        bool isScriptable = mm.attributes() & QMetaMethod::Scriptable;

        // consistency check:
        if (isAsync && returnType != QMetaType::Void)
            continue;

        int inputCount = qDBusParametersForMethod(mm, metaTypes);
        if (inputCount == -1)
            continue;           // problem parsing

        metaTypes[0] = returnType;
        bool hasMessage = false;
        if (inputCount > 0 &&
            metaTypes.at(inputCount) == QDBusMetaTypeId::message) {
            // "no input parameters" is allowed as long as the message meta type is there
            hasMessage = true;
            --inputCount;
        }

        // try to match the parameters
        int i;
        QByteArray reconstructedSignature;
        for (i = 1; i <= inputCount; ++i) {
            const char *typeSignature = QDBusMetaType::typeToSignature( metaTypes.at(i) );
            if (!typeSignature)
                break;          // invalid

            reconstructedSignature += typeSignature;
            if (!msgSignature.startsWith(reconstructedSignature))
                break;
        }

        if (reconstructedSignature != msgSignature)
            continue;           // we didn't match them all

        if (hasMessage)
            ++i;

        // make sure that the output parameters have signatures too
        if (returnType != 0 && QDBusMetaType::typeToSignature(returnType) == 0)
            continue;

        bool ok = true;
        for (int j = i; ok && j < metaTypes.count(); ++j)
            if (QDBusMetaType::typeToSignature(metaTypes.at(i)) == 0)
                ok = false;
        if (!ok)
            continue;

        // consistency check:
        if (isAsync && metaTypes.count() > i + 1)
            continue;

        if (isScriptable && (flags & QDBusConnection::ExportScriptableSlots) == 0)
            continue;           // not exported
        if (!isScriptable && (flags & QDBusConnection::ExportNonScriptableSlots) == 0)
            continue;           // not exported

        // if we got here, this slot matched
        return idx;
    }

    // no slot matched
    return -1;
}

CallDeliveryEvent* QDBusConnectionPrivate::prepareReply(QObject *object, int idx,
                                                        const QList<int> &metaTypes,
                                                        const QDBusMessage &msg)
{
    Q_ASSERT(object);
    Q_UNUSED(object);

    int n = metaTypes.count() - 1;
    if (metaTypes[n] == QDBusMetaTypeId::message)
        --n;

    // check that types match
    for (int i = 0; i < n; ++i)
        if (metaTypes.at(i + 1) != msg.arguments().at(i).userType() &&
            msg.arguments().at(i).userType() != qMetaTypeId<QDBusArgument>())
            return 0;           // no match

    // we can deliver
    // prepare for the call
    return new CallDeliveryEvent(QDBusConnection(this), idx, this, msg, metaTypes);
}

void QDBusConnectionPrivate::activateSignal(const QDBusConnectionPrivate::SignalHook& hook,
                                            const QDBusMessage &msg)
{
    // This is called by QDBusConnectionPrivate::handleSignal to deliver a signal
    // that was received from D-Bus
    //
    // Signals are delivered to slots if the parameters match
    // Slots can have less parameters than there are on the message
    // Slots can optionally have one final parameter that is a QDBusMessage
    // Slots receive read-only copies of the message (i.e., pass by value or by const-ref)
    CallDeliveryEvent *call = prepareReply(hook.obj, hook.midx, hook.params, msg);
    if (call)
        postEventToThread(ActivateSignalAction, hook.obj, call);
}

bool QDBusConnectionPrivate::activateCall(QObject* object, int flags, const QDBusMessage &msg)
{
    // This is called by QDBusConnectionPrivate::handleObjectCall to place a call
    // to a slot on the object.
    //
    // The call is delivered to the first slot that matches the following conditions:
    //  - has the same name as the message's target member
    //  - ALL of the message's types are found in slot's parameter list
    //  - optionally has one more parameter of type QDBusMessage
    // If none match, then the slot of the same name as the message target and with
    // the first type of QDBusMessage is delivered.
    //
    // The D-Bus specification requires that all MethodCall messages be replied to, unless the
    // caller specifically waived this requirement. This means that we inspect if the user slot
    // generated a reply and, if it didn't, we will. Obviously, if the user slot doesn't take a
    // QDBusMessage parameter, it cannot generate a reply.
    //
    // When a return message is generated, the slot's return type, if any, will be placed
    // in the message's first position. If there are non-const reference parameters to the
    // slot, they must appear at the end and will be placed in the subsequent message
    // positions.

    static const char cachePropertyName[] = "_qdbus_slotCache";

    if (!object)
        return false;

    Q_ASSERT_X(QThread::currentThread() == object->thread(),
               "QDBusConnection: internal threading error",
               "function called for an object that is in another thread!!");

    QDBusSlotCache slotCache =
        qvariant_cast<QDBusSlotCache>(object->property(cachePropertyName));
    QString cacheKey = msg.member(), signature = msg.signature();
    if (!signature.isEmpty()) {
        cacheKey.reserve(cacheKey.length() + 1 + signature.length());
        cacheKey += QLatin1Char('.');
        cacheKey += signature;
    }

    QDBusSlotCache::Hash::ConstIterator cacheIt = slotCache.hash.constFind(cacheKey);
    while (cacheIt != slotCache.hash.constEnd() && cacheIt->flags != flags &&
           cacheIt.key() == cacheKey)
        ++cacheIt;
    if (cacheIt == slotCache.hash.constEnd() || cacheIt.key() != cacheKey)
    {
        // not cached, analyse the meta object
        const QMetaObject *mo = object->metaObject();
        QByteArray memberName = msg.member().toUtf8();

        // find a slot that matches according to the rules above
        QDBusSlotCache::Data slotData;
        slotData.flags = flags;
        slotData.slotIdx = ::findSlot(mo, memberName, flags, msg.signature(), slotData.metaTypes);
        if (slotData.slotIdx == -1) {
            // ### this is where we want to add the connection as an arg too
            // try with no parameters, but with a QDBusMessage
            slotData.slotIdx = ::findSlot(mo, memberName, flags, QString(), slotData.metaTypes);
            if (slotData.metaTypes.count() != 2 ||
                slotData.metaTypes.at(1) != QDBusMetaTypeId::message) {
                // not found
                // save the negative lookup
                slotData.slotIdx = -1;
                slotData.metaTypes.clear();
                slotCache.hash.insert(cacheKey, slotData);
                object->setProperty(cachePropertyName, qVariantFromValue(slotCache));
                return false;
            }
        }

        // save to the cache
        slotCache.hash.insert(cacheKey, slotData);
        object->setProperty(cachePropertyName, qVariantFromValue(slotCache));

        // found the slot to be called
        deliverCall(object, flags, msg, slotData.metaTypes, slotData.slotIdx);
        return true;
    } else if (cacheIt->slotIdx == -1) {
        // negative cache
        return false;
    } else {
        // use the cache
        deliverCall(object, flags, msg, cacheIt->metaTypes, cacheIt->slotIdx);
        return true;
    }
}

void QDBusConnectionPrivate::deliverCall(QObject *object, int /*flags*/, const QDBusMessage &msg,
                                         const QList<int> &metaTypes, int slotIdx)
{
    Q_ASSERT_X(!object || QThread::currentThread() == object->thread(),
               "QDBusConnection: internal threading error",
               "function called for an object that is in another thread!!");

    QVarLengthArray<void *, 10> params;
    params.reserve(metaTypes.count());

    QVariantList auxParameters;
    // let's create the parameter list

    // first one is the return type -- add it below
    params.append(0);

    // add the input parameters
    int i;
    int pCount = qMin(msg.arguments().count(), metaTypes.count() - 1);
    for (i = 1; i <= pCount; ++i) {
        int id = metaTypes[i];
        if (id == QDBusMetaTypeId::message)
            break;

        const QVariant &arg = msg.arguments().at(i - 1);
        if (arg.userType() == id)
            // no conversion needed
            params.append(const_cast<void *>(arg.constData()));
        else if (arg.userType() == qMetaTypeId<QDBusArgument>()) {
            // convert to what the function expects
            void *null = 0;
            auxParameters.append(QVariant(id, null));

            const QDBusArgument &in =
                *reinterpret_cast<const QDBusArgument *>(arg.constData());
            QVariant &out = auxParameters[auxParameters.count() - 1];

            if (!QDBusMetaType::demarshall(in, out.userType(), out.data()))
                qFatal("Internal error: demarshalling function for type '%s' (%d) failed!",
                       out.typeName(), out.userType());

            params.append(const_cast<void *>(out.constData()));
        } else {
            qFatal("Internal error: got invalid meta type %d (%s) "
                   "when trying to convert to meta type %d (%s)",
                   arg.userType(), QMetaType::typeName(arg.userType()),
                   id, QMetaType::typeName(id));
        }
    }

    bool takesMessage = false;
    if (metaTypes.count() > i && metaTypes[i] == QDBusMetaTypeId::message) {
        params.append(const_cast<void*>(static_cast<const void*>(&msg)));
        takesMessage = true;
        ++i;
    }

    // output arguments
    QVariantList outputArgs;
    void *null = 0;
    if (metaTypes[0] != QMetaType::Void) {
        QVariant arg(metaTypes[0], null);
        outputArgs.append( arg );
        params[0] = const_cast<void*>(outputArgs.at( outputArgs.count() - 1 ).constData());
    }
    for ( ; i < metaTypes.count(); ++i) {
        QVariant arg(metaTypes[i], null);
        outputArgs.append( arg );
        params.append(const_cast<void*>(outputArgs.at( outputArgs.count() - 1 ).constData()));
    }

    // make call:
    bool fail;
    if (!object) {
        fail = true;
    } else {
        // FIXME: save the old sender!
        QDBusContextPrivate context(QDBusConnection(this), msg);
        QDBusContextPrivate *old = QDBusContextPrivate::set(object, &context);
        QDBusConnectionPrivate::setSender(this);

        QPointer<QObject> ptr = object;
        fail = object->qt_metacall(QMetaObject::InvokeMetaMethod,
                                   slotIdx, params.data()) >= 0;
        QDBusConnectionPrivate::setSender(0);
	// the object might be deleted in the slot
	if (!ptr.isNull())
	    QDBusContextPrivate::set(object, old);
    }

    // do we create a reply? Only if the caller is waiting for a reply and one hasn't been sent
    // yet.
    if (!msg.isReplyRequired() && !msg.isDelayedReply()) {
        if (!fail) {
            // normal reply
            qDBusDebug() << QThread::currentThread() << "Automatically sending reply:" << outputArgs;
            send(msg.createReply(outputArgs));
        } else {
            // generate internal error
            qWarning("Internal error: Failed to deliver message");
            send(msg.createErrorReply(QDBusError::InternalError,
                                      QLatin1String("Failed to deliver message")));
        }
    }

    return;
}

QDBusConnectionPrivate::QDBusConnectionPrivate(QObject *p)
    : QObject(p), ref(1), mode(InvalidMode), connection(0), server(0), busService(0),
      rootNode(QString(QLatin1Char('/')))
{
    extern bool qDBusInitThreads();
    static const bool threads = qDBusInitThreads();
    static const bool debugging = !qgetenv("QDBUS_DEBUG").isEmpty();
    Q_UNUSED(threads)

    ::isDebugging = debugging;

    QDBusMetaTypeId::init();

    rootNode.flags = 0;

    connect(this, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(_q_serviceOwnerChanged(QString,QString,QString)));
}

QDBusConnectionPrivate::~QDBusConnectionPrivate()
{
    if (thread() && thread() != QThread::currentThread())
        qWarning("QDBusConnection(name=\"%s\")'s last reference in not in its creation thread! "
                 "Timer and socket errors will follow and the program will probably crash",
                 qPrintable(name));

    closeConnection();
    rootNode.children.clear();  // free resources
    qDeleteAll(cachedMetaObjects);

    if (server)
        dbus_server_unref(server);
    if (connection)
        dbus_connection_unref(connection);

    connection = 0;
    server = 0;
}

void QDBusConnectionPrivate::closeConnection()
{
    QDBusWriteLocker locker(CloseConnectionAction, this);
    ConnectionMode oldMode = mode;
    mode = InvalidMode; // prevent reentrancy
    baseService.clear();

    if (oldMode == ServerMode) {
        if (server) {
            dbus_server_disconnect(server);
        }
    } else if (oldMode == ClientMode) {
        if (connection) {
            dbus_connection_close(connection);
            // send the "close" message
            while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS)
                ;
        }
    }
}

void QDBusConnectionPrivate::checkThread()
{
    if (!thread()) {
        if (QCoreApplication::instance())
            moveToThread(QCoreApplication::instance()->thread());
        else
            qWarning("The thread that had QDBusConnection('%s') has died and there is no main thread",
                     qPrintable(name));
    }
}

bool QDBusConnectionPrivate::handleError(const QDBusErrorInternal &error)
{
    if (!error)
        return false;           // no error

    //lock.lockForWrite();
    lastError = error;
    //lock.unlock();
    return true;
}

void QDBusConnectionPrivate::timerEvent(QTimerEvent *e)
{
    DBusTimeout *timeout = timeouts.value(e->timerId(), 0);
    dbus_timeout_handle(timeout);
}

void QDBusConnectionPrivate::customEvent(QEvent *e)
{
    Q_ASSERT(e->type() == QEvent::User);

    QDBusConnectionCallbackEvent *ev = static_cast<QDBusConnectionCallbackEvent *>(e);
    QDBusLockerBase::reportThreadAction(int(AddTimeoutAction) + int(ev->subtype),
                                        QDBusLockerBase::BeforeDeliver, this);
    switch (ev->subtype)
    {
    case QDBusConnectionCallbackEvent::AddTimeout:
        qDBusRealAddTimeout(this, ev->timeout, ev->extra);
        break;

    case QDBusConnectionCallbackEvent::RemoveTimeout:
        qDBusRealRemoveTimeout(this, ev->timeout);
        break;

    case QDBusConnectionCallbackEvent::AddWatch:
        qDBusRealAddWatch(this, ev->watch, ev->extra, ev->fd);
        break;

    case QDBusConnectionCallbackEvent::RemoveWatch:
        qDBusRealRemoveWatch(this, ev->watch, ev->fd);
        break;

    case QDBusConnectionCallbackEvent::ToggleWatch:
        qDBusRealToggleWatch(this, ev->watch, ev->fd);
        break;
    }
    QDBusLockerBase::reportThreadAction(int(AddTimeoutAction) + int(ev->subtype),
                                        QDBusLockerBase::AfterDeliver, this);
}

void QDBusConnectionPrivate::doDispatch()
{
    QDBusDispatchLocker locker(DoDispatchAction, this);
    if (mode == ClientMode)
        while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS);
}

void QDBusConnectionPrivate::socketRead(int fd)
{
    QHashIterator<int, QDBusConnectionPrivate::Watcher> it(watchers);
    while (it.hasNext()) {
        it.next();
        if (it.key() == fd && it.value().read && it.value().read->isEnabled()) {
            if (!dbus_watch_handle(it.value().watch, DBUS_WATCH_READABLE))
                qDebug("OUT OF MEM");
        }
    }

    doDispatch();
}

void QDBusConnectionPrivate::socketWrite(int fd)
{
    QHashIterator<int, QDBusConnectionPrivate::Watcher> it(watchers);
    while (it.hasNext()) {
        it.next();
        if (it.key() == fd && it.value().write && it.value().write->isEnabled()) {
            if (!dbus_watch_handle(it.value().watch, DBUS_WATCH_WRITABLE))
                qDebug("OUT OF MEM");
        }
    }
}

void QDBusConnectionPrivate::objectDestroyed(QObject *obj)
{
    QDBusWriteLocker locker(ObjectDestroyedAction, this);
    huntAndDestroy(obj, rootNode);

    SignalHookHash::iterator sit = signalHooks.begin();
    while (sit != signalHooks.end()) {
        if (static_cast<QObject *>(sit.value().obj) == obj)
            sit = signalHooks.erase(sit);
        else
            ++sit;
    }

    obj->disconnect(this);
}

void QDBusConnectionPrivate::relaySignal(QObject *obj, const QMetaObject *mo, int signalId,
                                         const QVariantList &args)
{
    int mciid = mo->indexOfClassInfo(QCLASSINFO_DBUS_INTERFACE);
    Q_ASSERT(mciid != -1);

    QMetaClassInfo mci = mo->classInfo(mciid);
    Q_ASSERT(mci.value());
    const char *interface = mci.value();

    QMetaMethod mm = mo->method(signalId);
    QByteArray memberName = mm.signature();
    memberName.truncate(memberName.indexOf('('));

    // check if it's scriptable
    bool isScriptable = mm.attributes() & QMetaMethod::Scriptable;
    bool isAdaptor = false;
    for ( ; mo; mo = mo->superClass())
        if (mo == &QDBusAbstractAdaptor::staticMetaObject) {
            isAdaptor = true;
            break;
        }

    QDBusReadLocker locker(RelaySignalAction, this);
    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/"), QLatin1String(interface),
                                                      QLatin1String(memberName));
    message.setArguments(args);
    DBusMessage *msg = QDBusMessagePrivate::toDBusMessage(message);
    if (!msg) {
        qWarning("QDBusConnection: Could not emit signal %s.%s", interface, memberName.constData());
        return;
    }

    //qDBusDebug() << "Emitting signal" << message;
    //qDBusDebug() << "for paths:";
    dbus_message_set_no_reply(msg, true); // the reply would not be delivered to anything
    huntAndEmit(connection, msg, obj, rootNode, isScriptable, isAdaptor);
    dbus_message_unref(msg);
}

void QDBusConnectionPrivate::_q_serviceOwnerChanged(const QString &name,
                                                    const QString &oldOwner, const QString &newOwner)
{
    if (oldOwner == baseService)
        unregisterService(name);
    if (newOwner == baseService)
        registerService(name);

    QDBusWriteLocker locker(UpdateSignalHookOwnerAction, this);
    QMutableHashIterator<QString, SignalHook> it(signalHooks);
    it.toFront();
    while (it.hasNext())
        if (it.next().value().service == name)
            it.value().owner = newOwner;
}

int QDBusConnectionPrivate::findSlot(QObject* obj, const QByteArray &normalizedName,
                                     QList<int> &params)
{
    int midx = obj->metaObject()->indexOfMethod(normalizedName);
    if (midx == -1)
        return -1;

    int inputCount = qDBusParametersForMethod(obj->metaObject()->method(midx), params);
    if ( inputCount == -1 || inputCount + 1 != params.count() )
        return -1;              // failed to parse or invalid arguments or output arguments

    return midx;
}

bool QDBusConnectionPrivate::prepareHook(QDBusConnectionPrivate::SignalHook &hook, QString &key,
                                         const QString &service, const QString &owner,
                                         const QString &path, const QString &interface, const QString &name,
                                         QObject *receiver, const char *signal, int minMIdx,
                                         bool buildSignature)
{
    QByteArray normalizedName = signal + 1;
    hook.midx = findSlot(receiver, signal + 1, hook.params);
    if (hook.midx == -1) {
        normalizedName = QMetaObject::normalizedSignature(signal + 1);
        hook.midx = findSlot(receiver, normalizedName, hook.params);
    }
    if (hook.midx < minMIdx) {
        if (hook.midx == -1)
            ;//qWarning("No such slot '%s' while connecting D-Bus", normalizedName.constData());
        return false;
    }

    hook.service = service;
    hook.owner = owner; // we don't care if the service has an owner yet
    hook.path = path;
    hook.obj = receiver;

    // build the D-Bus signal name and signature
    // This should not happen for QDBusConnection::connect, use buildSignature here, since
    // QDBusConnection::connect passes false and everything else uses true
    QString mname = name;
    if (buildSignature && mname.isNull()) {
        normalizedName.truncate(normalizedName.indexOf('('));
        mname = QString::fromUtf8(normalizedName);
    }
    key = mname;
    key.reserve(interface.length() + 1 + mname.length());
    key += QLatin1Char(':');
    key += interface;

    if (buildSignature) {
        hook.signature.clear();
        for (int i = 1; i < hook.params.count(); ++i)
            if (hook.params.at(i) != QDBusMetaTypeId::message)
                hook.signature += QLatin1String( QDBusMetaType::typeToSignature( hook.params.at(i) ) );
    }

    hook.matchRule = buildMatchRule(service, owner, path, interface, mname, hook.signature);
    return true;                // connect to this signal
}

void QDBusConnectionPrivate::sendError(const QDBusMessage &msg, QDBusError::ErrorType code)
{
    if (code == QDBusError::UnknownMethod) {
        QString interfaceMsg;
        if (msg.interface().isEmpty())
            interfaceMsg = QLatin1String("any interface");
        else
            interfaceMsg = QString::fromLatin1("interface '%1'").arg(msg.interface());

        send(msg.createErrorReply(code,
                                  QString::fromLatin1("No such method '%1' in %2 at object path '%3' "
                                                      "(signature '%4')")
                                  .arg(msg.member(), interfaceMsg, msg.path(), msg.signature())));
    } else if (code == QDBusError::UnknownInterface) {
        send(msg.createErrorReply(QDBusError::UnknownInterface,
                                  QString::fromLatin1("No such interface '%1' at object path '%2'")
                                  .arg(msg.interface(), msg.path())));
    } else if (code == QDBusError::UnknownObject) {
        send(msg.createErrorReply(QDBusError::UnknownObject,
                                  QString::fromLatin1("No such object path '%1'").arg(msg.path())));
    }
}

bool QDBusConnectionPrivate::activateInternalFilters(const ObjectTreeNode &node,
                                                     const QDBusMessage &msg)
{
    // object may be null
    const QString interface = msg.interface();

    if (interface.isEmpty() || interface == QLatin1String(DBUS_INTERFACE_INTROSPECTABLE)) {
        if (msg.member() == QLatin1String("Introspect") && msg.signature().isEmpty()) {
            //qDebug() << "QDBusConnectionPrivate::activateInternalFilters introspect" << msg.d_ptr->msg;
            QDBusMessage reply = msg.createReply(qDBusIntrospectObject(node));
            send(reply);
            return true;
        }

        if (!interface.isEmpty()) {
            sendError(msg, QDBusError::UnknownMethod);
            return true;
        }
    }

    if (node.obj && (interface.isEmpty() ||
                     interface == QLatin1String(DBUS_INTERFACE_PROPERTIES))) {
        //qDebug() << "QDBusConnectionPrivate::activateInternalFilters properties" << msg.d_ptr->msg;
        if (msg.member() == QLatin1String("Get") && msg.signature() == QLatin1String("ss")) {
            QDBusMessage reply = qDBusPropertyGet(node, msg);
            send(reply);
            return true;
        } else if (msg.member() == QLatin1String("Set") && msg.signature() == QLatin1String("ssv")) {
            QDBusMessage reply = qDBusPropertySet(node, msg);
            send(reply);
            return true;
        }

        if (!interface.isEmpty()) {
            sendError(msg, QDBusError::UnknownMethod);
            return true;
        }
    }

    return false;
}

void QDBusConnectionPrivate::activateObject(ObjectTreeNode &node, const QDBusMessage &msg,
                                            int pathStartPos)
{
    // This is called by QDBusConnectionPrivate::handleObjectCall to place a call to a slot
    // on the object.
    //
    // The call is routed through the adaptor sub-objects if we have any

    // object may be null

    if (pathStartPos != msg.path().length()) {
        node.flags &= ~QDBusConnection::ExportAllSignals;
        node.obj = findChildObject(&node, msg.path(), pathStartPos);
        if (!node.obj) {
            sendError(msg, QDBusError::UnknownObject);
            return;
        }
    }

    QDBusAdaptorConnector *connector;
    if (node.flags & QDBusConnection::ExportAdaptors &&
        (connector = qDBusFindAdaptorConnector(node.obj))) {
        int newflags = node.flags | QDBusConnection::ExportAllSlots;

        if (msg.interface().isEmpty()) {
            // place the call in all interfaces
            // let the first one that handles it to work
            QDBusAdaptorConnector::AdaptorMap::ConstIterator it =
                connector->adaptors.constBegin();
            QDBusAdaptorConnector::AdaptorMap::ConstIterator end =
                connector->adaptors.constEnd();

            for ( ; it != end; ++it)
                if (activateCall(it->adaptor, newflags, msg))
                    return;
        } else {
            // check if we have an interface matching the name that was asked:
            QDBusAdaptorConnector::AdaptorMap::ConstIterator it;
            it = qLowerBound(connector->adaptors.constBegin(), connector->adaptors.constEnd(),
                             msg.interface());
            if (it != connector->adaptors.constEnd() && msg.interface() == QLatin1String(it->interface)) {
                if (!activateCall(it->adaptor, newflags, msg))
                    sendError(msg, QDBusError::UnknownMethod);
                return;
            }
        }
    }

    // no adaptors matched or were exported
    // try our standard filters
    if (activateInternalFilters(node, msg))
        return;                 // internal filters have already run or an error has been sent

    // try the object itself:
    if (node.flags & (QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportNonScriptableSlots)) {
        bool interfaceFound = true;
        if (!msg.interface().isEmpty())
            interfaceFound = msg.interface() == qDBusInterfaceFromMetaObject(node.obj->metaObject());

        if (interfaceFound) {
            if (!activateCall(node.obj, node.flags, msg))
                sendError(msg, QDBusError::UnknownMethod);
            return;
        }
    }

    // nothing matched, send an error code
    if (msg.interface().isEmpty())
        sendError(msg, QDBusError::UnknownMethod);
    else
        sendError(msg, QDBusError::UnknownInterface);
}

void QDBusConnectionPrivate::handleObjectCall(const QDBusMessage &msg)
{
    // if the msg is external, we were called from inside doDispatch
    // that means the dispatchLock mutex is locked
    // must not call out to user code in that case
    //
    // however, if the message is internal, handleMessage was called
    // directly and no lock is in place. We can therefore call out to
    // user code, if necessary
    ObjectTreeNode result;
    int usedLength;
    QThread *objThread = 0;
    QSemaphore sem;
    bool semWait;

    {
        QDBusReadLocker locker(HandleObjectCallAction, this);
        if (!findObject(&rootNode, msg.path(), usedLength, result)) {
            // qDebug("Call failed: no object found at %s", qPrintable(msg.path()));
            sendError(msg, QDBusError::UnknownObject);
            return;
        }

        if (!result.obj) {
            // no object -> no threading issues
            // it's either going to be an error, or an internal filter
            activateObject(result, msg, usedLength);
            return;
        }

        objThread = result.obj->thread();
        if (!objThread) {
            send(msg.createErrorReply(QDBusError::InternalError,
                                      QString::fromLatin1("Object '%1' (at path '%2')"
                                                          " has no thread. Cannot deliver message.")
                                      .arg(result.obj->objectName(), msg.path())));
            return;
        }

        if (!QDBusMessagePrivate::isLocal(msg)) {
            // external incoming message
            // post it and forget
            postEventToThread(HandleObjectCallPostEventAction, result.obj,
                              new ActivateObjectEvent(QDBusConnection(this), this, result, usedLength, msg));
            return;
        } else if (objThread != QThread::currentThread()) {
            // synchronize with other thread
            postEventToThread(HandleObjectCallPostEventAction, result.obj,
                              new ActivateObjectEvent(QDBusConnection(this), this, result,
                                                      usedLength, msg, &sem));
            semWait = true;
        } else {
            semWait = false;
        }
    } // release the lock

    if (semWait)
        SEM_ACQUIRE(HandleObjectCallSemaphoreAction, sem);
    else
        activateObject(result, msg, usedLength);
}

ActivateObjectEvent::~ActivateObjectEvent()
{
    if (!handled) {
        // we're being destroyed without delivering
        // it means the object was deleted between posting and delivering
        QDBusConnectionPrivate *that = QDBusConnectionPrivate::d(connection);
        that->sendError(message, QDBusError::UnknownObject);
    }

    // semaphore releasing happens in ~QMetaCallEvent
}

int ActivateObjectEvent::placeMetaCall(QObject *)
{
    QDBusConnectionPrivate *that = QDBusConnectionPrivate::d(connection);

    QDBusLockerBase::reportThreadAction(HandleObjectCallPostEventAction,
                                        QDBusLockerBase::BeforeDeliver, that);
    that->activateObject(node, message, pathStartPos);
    QDBusLockerBase::reportThreadAction(HandleObjectCallPostEventAction,
                                        QDBusLockerBase::AfterDeliver, that);

    handled = true;
    return -1;
}

void QDBusConnectionPrivate::handleSignal(const QString &key, const QDBusMessage& msg)
{
    SignalHookHash::const_iterator it = signalHooks.find(key);
    SignalHookHash::const_iterator end = signalHooks.constEnd();
    //qDebug("looking for: %s", path.toLocal8Bit().constData());
    //qDBusDebug() << signalHooks.keys();
    for ( ; it != end && it.key() == key; ++it) {
        const SignalHook &hook = it.value();
        if (!hook.owner.isEmpty() && hook.owner != msg.service())
            continue;
        if (!hook.path.isEmpty() && hook.path != msg.path())
            continue;
        if (!hook.signature.isEmpty() && hook.signature != msg.signature())
            continue;
        if (hook.signature.isEmpty() && !hook.signature.isNull() && !msg.signature().isEmpty())
            continue;

        activateSignal(hook, msg);
    }
}

void QDBusConnectionPrivate::handleSignal(const QDBusMessage& msg)
{
    // We call handlesignal(QString, QDBusMessage) three times:
    //  one with member:interface
    //  one with member:
    //  one with :interface
    // This allows us to match signals with wildcards on member or interface
    // (but not both)

    QString key = msg.member();
    key.reserve(key.length() + 1 + msg.interface().length());
    key += QLatin1Char(':');
    key += msg.interface();

    QDBusReadLocker locker(HandleSignalAction, this);
    handleSignal(key, msg);                  // one try

    key.truncate(msg.member().length() + 1); // keep the ':'
    handleSignal(key, msg);                  // second try

    key = QLatin1Char(':');
    key += msg.interface();
    handleSignal(key, msg);                  // third try
}

static dbus_int32_t server_slot = -1;

void QDBusConnectionPrivate::setServer(DBusServer *s, const QDBusErrorInternal &error)
{
    if (!server) {
        handleError(error);
        return;
    }

    server = s;
    mode = ServerMode;

    dbus_server_allocate_data_slot(&server_slot);
    if (server_slot < 0)
        return;

    dbus_server_set_watch_functions(server, qDBusAddWatch, qDBusRemoveWatch,
                                    qDBusToggleWatch, this, 0); // ### check return type?
    dbus_server_set_timeout_functions(server, qDBusAddTimeout, qDBusRemoveTimeout,
                                      qDBusToggleTimeout, this, 0);
    dbus_server_set_new_connection_function(server, qDBusNewConnection, this, 0);

    dbus_server_set_data(server, server_slot, this, 0);
}

void QDBusConnectionPrivate::setConnection(DBusConnection *dbc, const QDBusErrorInternal &error)
{
    if (!dbc) {
        handleError(error);
        return;
    }

    connection = dbc;
    mode = ClientMode;

    dbus_connection_set_exit_on_disconnect(connection, false);
    dbus_connection_set_watch_functions(connection, qDBusAddWatch, qDBusRemoveWatch,
                                        qDBusToggleWatch, this, 0);
    dbus_connection_set_timeout_functions(connection, qDBusAddTimeout, qDBusRemoveTimeout,
                                          qDBusToggleTimeout, this, 0);

    // Initialize the match rules
    // We want all messages that have us as destination
    // signals don't have destinations, but connectSignal() takes care of them
    const char *service = dbus_bus_get_unique_name(connection);
    if (service) {
        QVarLengthArray<char, 56> filter;
        filter.append("destination='", 13);
        filter.append(service, qstrlen(service));
        filter.append("\'\0", 2);

        QDBusErrorInternal error;
        dbus_bus_add_match(connection, filter.constData(), error);
        if (handleError(error)) {
            closeConnection();
            return;
        }

        baseService = QString::fromUtf8(service);
    } else {
        qWarning("QDBusConnectionPrivate::SetConnection: Unable to get base service");
    }

    dbus_connection_add_filter(connection, qDBusSignalFilter, this, 0);

    //qDebug("base service: %s", service);

    // schedule a dispatch:
    QMetaObject::invokeMethod(this, "doDispatch", Qt::QueuedConnection);
}

extern "C"{
static void qDBusResultReceived(DBusPendingCall *pending, void *user_data)
{
    QDBusConnectionPrivate::messageResultReceived(pending, user_data);
}
}

void QDBusConnectionPrivate::messageResultReceived(DBusPendingCall *pending, void *user_data)
{
    QDBusPendingCall *call = reinterpret_cast<QDBusPendingCall *>(user_data);
    QDBusConnectionPrivate *connection = const_cast<QDBusConnectionPrivate *>(call->connection);
    Q_ASSERT(call->pending == pending);

    if (!call->receiver.isNull() && call->methodIdx != -1) {
        DBusMessage *reply = dbus_pending_call_steal_reply(pending);

        // Deliver the return values of a remote function call.
        //
        // There is only one connection and it is specified by idx
        // The slot must have the same parameter types that the message does
        // The slot may have less parameters than the message
        // The slot may optionally have one final parameter that is QDBusMessage
        // The slot receives read-only copies of the message (i.e., pass by value or by const-ref)

        QDBusMessage msg = QDBusMessagePrivate::fromDBusMessage(reply);
        qDBusDebug() << QThread::currentThread() << "got message reply (async):" << msg;
        if (msg.type() == QDBusMessage::ErrorMessage) {
            // got an error, emit it
            emit QDBusErrorHelper(call->receiver, call->errorMethod)
                .pendingCallError(QDBusError(msg), call->message);
            emit connection->callWithCallbackFailed(QDBusError(msg), call->message);
        } else {
            CallDeliveryEvent *e = connection->prepareReply(call->receiver, call->methodIdx,
                                                            call->metaTypes, msg);
            if (e)
                connection->postEventToThread(MessageResultReceivedAction, call->receiver, e);
            else
                qDBusDebug() << "Deliver failed!";
        }
    }
    dbus_pending_call_unref(pending);
    delete call;
}

int QDBusConnectionPrivate::send(const QDBusMessage& message)
{
    if (QDBusMessagePrivate::isLocal(message))
        return -1;              // don't send; the reply will be retrieved by the caller
                                // through the d_ptr->localReply link

    DBusMessage *msg = QDBusMessagePrivate::toDBusMessage(message);
    if (!msg) {
        if (message.type() == QDBusMessage::MethodCallMessage)
            qWarning("QDBusConnection: error: could not send message to service \"%s\" path \"%s\" interface \"%s\" member \"%s\"",
                     qPrintable(message.service()), qPrintable(message.path()),
                     qPrintable(message.interface()), qPrintable(message.member()));
        else if (message.type() == QDBusMessage::SignalMessage)
            qWarning("QDBusConnection: error: could not send signal path \"%s\" interface \"%s\" member \"%s\"",
                     qPrintable(message.path()), qPrintable(message.interface()),
                     qPrintable(message.member()));
        else
            qWarning("QDBusConnection: error: could not send %s message to service \"%s\"",
                     message.type() == QDBusMessage::ReplyMessage ? "reply" :
                     message.type() == QDBusMessage::ErrorMessage ? "error" :
                     "invalid", qPrintable(message.service()));
        return 0;
    }

    dbus_message_set_no_reply(msg, true); // the reply would not be delivered to anything

    qDBusDebug() << QThread::currentThread() << "sending message (no reply):" << message;
    checkThread();
    bool isOk = dbus_connection_send(connection, msg, 0);
    int serial = 0;
    if (isOk)
        serial = dbus_message_get_serial(msg);

    dbus_message_unref(msg);
    return serial;
}

QDBusMessage QDBusConnectionPrivate::sendWithReply(const QDBusMessage &message,
                                                   int sendMode, int timeout)
{
    checkThread();
    if ((sendMode == QDBus::BlockWithGui || sendMode == QDBus::Block)
         && isServiceRegisteredByThread(message.service()))
        // special case for synchronous local calls
        return sendWithReplyLocal(message);

    if (!QCoreApplication::instance() || sendMode == QDBus::Block) {
        DBusMessage *msg = QDBusMessagePrivate::toDBusMessage(message);
        if (!msg) {
            qWarning("QDBusConnection: error: could not send message to service \"%s\" path \"%s\" interface \"%s\" member \"%s\"",
                     qPrintable(message.service()), qPrintable(message.path()),
                     qPrintable(message.interface()), qPrintable(message.member()));
            return QDBusMessage();
        }

        qDBusDebug() << QThread::currentThread() << "sending message (blocking):" << message;
        QDBusErrorInternal error;
        DBusMessage *reply = dbus_connection_send_with_reply_and_block(connection, msg, timeout, error);

        dbus_message_unref(msg);

        if (!!error) {
            QDBusError qe = error;
            lastError = qe;
            return QDBusMessage::createError(qe);
        }

        QDBusMessage amsg = QDBusMessagePrivate::fromDBusMessage(reply);
        dbus_message_unref(reply);
        qDBusDebug() << QThread::currentThread() << "got message reply (blocking):" << amsg;

        if (dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS)
            QMetaObject::invokeMethod(this, "doDispatch", Qt::QueuedConnection);

        return amsg;
    } else { // use the event loop
        QDBusReplyWaiter waiter;
        if (sendWithReplyAsync(message, &waiter, SLOT(reply(QDBusMessage)),
                               SLOT(error(QDBusMessage)), timeout) > 0) {
            // enter the event loop and wait for a reply
            waiter.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents);

            lastError = waiter.replyMsg; // set or clear error
            return waiter.replyMsg;
        }

        return QDBusMessage();
    }
}

QDBusMessage QDBusConnectionPrivate::sendWithReplyLocal(const QDBusMessage &message)
{
    qDBusDebug() << QThread::currentThread() << "sending message via local-loop:" << message;

    QDBusMessage localCallMsg = QDBusMessagePrivate::makeLocal(*this, message);
    bool handled = handleMessage(localCallMsg);

    if (!handled) {
        QString interface = message.interface();
        if (interface.isEmpty())
            interface = QLatin1String("<no-interface>");
        return QDBusMessage::createError(QDBusError::InternalError,
                                         QString::fromLatin1("Internal error trying to call %1.%2 at %3 (signature '%4'")
                                         .arg(interface, message.member(),
                                              message.path(), message.signature()));
    }

    // if the message was handled, there might be a reply
    QDBusMessage localReplyMsg = QDBusMessagePrivate::makeLocalReply(*this, localCallMsg);
    if (localReplyMsg.type() == QDBusMessage::InvalidMessage) {
        qWarning("QDBusConnection: cannot call local method '%s' at object %s (with signature '%s') "
                 "on blocking mode", qPrintable(message.member()), qPrintable(message.path()),
                 qPrintable(message.signature()));
        return QDBusMessage::createError(
            QDBusError(QDBusError::InternalError,
                       QLatin1String("local-loop message cannot have delayed replies")));
    }

    // there is a reply
    qDBusDebug() << QThread::currentThread() << "got message via local-loop:" << localReplyMsg;
    return localReplyMsg;
}

int QDBusConnectionPrivate::sendWithReplyAsync(const QDBusMessage &message, QObject *receiver,
                                               const char *returnMethod, const char *errorMethod,
                                               int timeout)
{
    if (!receiver || !returnMethod || !*returnMethod) {
        // would not be able to deliver a reply
        qWarning("QDBusConnection::sendWithReplyAsync: error: cannot deliver a reply to %s::%s (%s)",
                 receiver ? receiver->metaObject()->className() : "(null)",
                 returnMethod ? returnMethod + 1 : "(null)",
                 receiver ? qPrintable(receiver->objectName()) : "no name");
        return send(message);
    }

    int slotIdx = -1;
    QList<int> metaTypes;
    slotIdx = findSlot(receiver, returnMethod + 1, metaTypes);
    if (slotIdx == -1) {
        QByteArray normalizedName = QMetaObject::normalizedSignature(returnMethod + 1);
        slotIdx = findSlot(receiver, normalizedName, metaTypes);
    }
    if (slotIdx == -1) {
        // would not be able to deliver a reply
        qWarning("QDBusConnection::sendWithReplyAsync: error: cannot deliver a reply to %s::%s (%s)",
                 receiver->metaObject()->className(),
                 returnMethod + 1, qPrintable(receiver->objectName()));
        return send(message);
    }

    DBusMessage *msg = QDBusMessagePrivate::toDBusMessage(message);
    if (!msg) {
        qWarning("QDBusConnection: error: could not send message to service \"%s\" path \"%s\" interface \"%s\" member \"%s\"",
                 qPrintable(message.service()), qPrintable(message.path()),
                 qPrintable(message.interface()), qPrintable(message.member()));
        return 0;
    }

    checkThread();
    qDBusDebug() << QThread::currentThread() << "sending message (async):" << message;
    DBusPendingCall *pending = 0;
    QDBusDispatchLocker locker(SendWithReplyAsyncAction, this);
    if (dbus_connection_send_with_reply(connection, msg, &pending, timeout)) {
        int serial = dbus_message_get_serial(msg);
        dbus_message_unref(msg);

        QDBusPendingCall *pcall = new QDBusPendingCall;
        pcall->receiver = receiver;
        pcall->metaTypes = metaTypes;
        pcall->methodIdx = slotIdx;
        pcall->connection = this;
        pcall->pending = pending;
        pcall->errorMethod = errorMethod;
        pcall->message = message;
        dbus_pending_call_set_notify(pending, qDBusResultReceived, pcall, 0);

        return serial;
    }

    dbus_message_unref(msg);
    return 0;
}

void QDBusConnectionPrivate::connectSignal(const QString &key, const SignalHook &hook)
{
    signalHooks.insertMulti(key, hook);
    connect(hook.obj, SIGNAL(destroyed(QObject*)), SLOT(objectDestroyed(QObject*)),
            Qt::DirectConnection);

    if (connection) {
        qDBusDebug("Adding rule: %s", hook.matchRule.constData());
        QDBusErrorInternal error;
        dbus_bus_add_match(connection, hook.matchRule, error);
        if (!!error) {
            QDBusError qerror = error;
            qWarning("QDBusConnectionPrivate::connectSignal: received error from D-Bus server "
                     "while connecting signal to %s::%s: %s (%s)",
                     hook.obj->metaObject()->className(),
                     hook.obj->metaObject()->method(hook.midx).signature(),
                     qPrintable(qerror.name()), qPrintable(qerror.message()));
        }
    }
}

void QDBusConnectionPrivate::disconnectSignal(SignalHookHash::Iterator &it)
{
    const SignalHook &hook = it.value();

    // we don't care about errors here
    if (connection) {
        qDBusDebug("Removing rule: %s", hook.matchRule.constData());
        dbus_bus_remove_match(connection, hook.matchRule, NULL);
    }

    signalHooks.erase(it);
}

void QDBusConnectionPrivate::registerObject(const ObjectTreeNode *node)
{
    connect(node->obj, SIGNAL(destroyed(QObject*)), SLOT(objectDestroyed(QObject*)),
            Qt::DirectConnection);

    if (node->flags & (QDBusConnection::ExportAdaptors
                       | QDBusConnection::ExportScriptableSignals
                       | QDBusConnection::ExportNonScriptableSignals)) {
        QDBusAdaptorConnector *connector = qDBusCreateAdaptorConnector(node->obj);

        if (node->flags & (QDBusConnection::ExportScriptableSignals
                           | QDBusConnection::ExportNonScriptableSignals)) {
            connector->disconnectAllSignals(node->obj);
            connector->connectAllSignals(node->obj);
        }

        // disconnect and reconnect to avoid duplicates
        connector->disconnect(SIGNAL(relaySignal(QObject*,const QMetaObject*,int,QVariantList)),
                              this, SLOT(relaySignal(QObject*,const QMetaObject*,int,QVariantList)));
        connect(connector, SIGNAL(relaySignal(QObject*,const QMetaObject*,int,QVariantList)),
                this, SLOT(relaySignal(QObject*,const QMetaObject*,int,QVariantList)),
                Qt::DirectConnection);
    }
}

void QDBusConnectionPrivate::connectRelay(const QString &service, const QString &owner,
                                          const QString &path, const QString &interface,
                                          QDBusAbstractInterface *receiver,
                                          const char *signal)
{
    // this function is called by QDBusAbstractInterface when one of its signals is connected
    // we set up a relay from D-Bus into it
    SignalHook hook;
    QString key;

    if (!prepareHook(hook, key, service, owner, path, interface, QString(), receiver, signal,
                     QDBusAbstractInterface::staticMetaObject.methodCount(), true))
        return;                 // don't connect

    // add it to our list:
    QDBusWriteLocker locker(ConnectRelayAction, this);
    SignalHookHash::ConstIterator it = signalHooks.find(key);
    SignalHookHash::ConstIterator end = signalHooks.constEnd();
    for ( ; it != end && it.key() == key; ++it) {
        const SignalHook &entry = it.value();
        if (entry.service == hook.service &&
            entry.owner == hook.owner &&
            entry.path == hook.path &&
            entry.signature == hook.signature &&
            entry.obj == hook.obj &&
            entry.midx == hook.midx)
            return;             // already there, no need to re-add
    }

    connectSignal(key, hook);
}

void QDBusConnectionPrivate::disconnectRelay(const QString &service, const QString &owner,
                                             const QString &path, const QString &interface,
                                             QDBusAbstractInterface *receiver,
                                             const char *signal)
{
    // this function is called by QDBusAbstractInterface when one of its signals is disconnected
    // we remove relay from D-Bus into it
    SignalHook hook;
    QString key;

    if (!prepareHook(hook, key, service, owner, path, interface, QString(), receiver, signal,
                     QDBusAbstractInterface::staticMetaObject.methodCount(), true))
        return;                 // don't connect

    // remove it from our list:
    QDBusWriteLocker locker(DisconnectRelayAction, this);
    SignalHookHash::Iterator it = signalHooks.find(key);
    SignalHookHash::Iterator end = signalHooks.end();
    for ( ; it != end && it.key() == key; ++it) {
        const SignalHook &entry = it.value();
        if (entry.service == hook.service &&
            entry.owner == hook.owner &&
            entry.path == hook.path &&
            entry.signature == hook.signature &&
            entry.obj == hook.obj &&
            entry.midx == hook.midx) {
            // found it
            disconnectSignal(it);
            return;
        }
    }

    qWarning("QDBusConnectionPrivate::disconnectRelay called for a signal that was not found");
}

QString QDBusConnectionPrivate::getNameOwner(const QString& serviceName)
{
    if (QDBusUtil::isValidUniqueConnectionName(serviceName))
        return serviceName;
    if (!connection || !QDBusUtil::isValidBusName(serviceName))
        return QString();

    QDBusMessage msg = QDBusMessage::createMethodCall(QLatin1String(DBUS_SERVICE_DBUS),
            QLatin1String(DBUS_PATH_DBUS), QLatin1String(DBUS_INTERFACE_DBUS),
            QLatin1String("GetNameOwner"));
    msg << serviceName;
    QDBusMessage reply = sendWithReply(msg, QDBus::Block);
    if (reply.type() == QDBusMessage::ReplyMessage)
        return reply.arguments().at(0).toString();
    return QString();
}

QDBusMetaObject *
QDBusConnectionPrivate::findMetaObject(const QString &service, const QString &path,
                                       const QString &interface, QDBusError &error)
{
    // service must be a unique connection name
    if (!interface.isEmpty()) {
        QDBusReadLocker locker(FindMetaObject1Action, this);
        QDBusMetaObject *mo = cachedMetaObjects.value(interface, 0);
        if (mo)
            return mo;
    }

    // introspect the target object
    QDBusMessage msg = QDBusMessage::createMethodCall(service, path,
                                                QLatin1String(DBUS_INTERFACE_INTROSPECTABLE),
                                                QLatin1String("Introspect"));

    QDBusMessage reply = sendWithReply(msg, QDBus::Block);

    // it doesn't exist yet, we have to create it
    QDBusWriteLocker locker(FindMetaObject2Action, this);
    QDBusMetaObject *mo = 0;
    if (!interface.isEmpty())
        mo = cachedMetaObjects.value(interface, 0);
    if (mo)
        // maybe it got created when we switched from read to write lock
        return mo;

    QString xml;
    if (reply.type() == QDBusMessage::ReplyMessage)
        // fetch the XML description
        xml = reply.arguments().at(0).toString();
    else {
        error = reply;
        lastError = error;
        if (reply.type() != QDBusMessage::ErrorMessage || error.type() != QDBusError::UnknownMethod)
            return 0; // error
    }

    // release the lock and return
    QDBusMetaObject *result = QDBusMetaObject::createMetaObject(interface, xml,
                                                                cachedMetaObjects, error);
    lastError = error;
    return result;
}

void QDBusConnectionPrivate::registerService(const QString &serviceName)
{
    QDBusWriteLocker locker(RegisterServiceAction, this);
    serviceNames.append(serviceName);
}

void QDBusConnectionPrivate::unregisterService(const QString &serviceName)
{
    QDBusWriteLocker locker(UnregisterServiceAction, this);
    serviceNames.removeAll(serviceName);
}

bool QDBusConnectionPrivate::isServiceRegisteredByThread(const QString &serviceName) const
{
    if (serviceName == baseService)
        return true;
    QStringList copy = serviceNames;
    return copy.contains(serviceName);
}

void QDBusConnectionPrivate::postEventToThread(int action, QObject *object, QEvent *ev)
{
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::BeforePost, this);
    QCoreApplication::postEvent(object, ev);
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::AfterPost, this);
}

void QDBusReplyWaiter::reply(const QDBusMessage &msg)
{
    replyMsg = msg;
    quit();
}

void QDBusReplyWaiter::error(const QDBusError &err)
{
    replyMsg = QDBusMessage::createError(err);
    quit();
}
