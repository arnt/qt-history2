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
#include <qcoreevent.h>
#include <qdebug.h>
#include <qmetaobject.h>
#include <qobject.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qtimer.h>

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

static bool isDebugging;
#define qDBusDebug              if (!::isDebugging); else qDebug

typedef void (*QDBusSpyHook)(const QDBusMessage&);
typedef QVarLengthArray<QDBusSpyHook, 4> QDBusSpyHookList;
Q_GLOBAL_STATIC(QDBusSpyHookList, qDBusSpyHookList)

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

class ActivateObjectEvent: public QEvent
{
public:
    ActivateObjectEvent(const QDBusConnectionPrivate::ObjectTreeNode &n, const QDBusMessage &m)
        : QEvent(QEvent::Type(QEvent::User + 1)), node(n), message(m) {}

    QDBusConnectionPrivate::ObjectTreeNode node;
    QDBusMessage message;
};

struct QDBusSlotCache
{
    struct Data
    {
        int slotIdx;
        QList<int> metaTypes;
    };
    typedef QHash<QString, Data> Hash;
    Hash hash;
};

Q_DECLARE_METATYPE(QDBusSlotCache)

class CallDeliveryEvent: public QEvent
{
public:
    CallDeliveryEvent()
        : QEvent(QEvent::User), object(0), flags(0), slotIdx(-1)
        { }

    const QDBusConnectionPrivate *conn;
    QPointer<QObject> object;
    QDBusMessage message;
    QList<int> metaTypes;

    int flags;
    int slotIdx;
};

static dbus_bool_t qDBusAddTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

  //  qDebug("addTimeout %d", dbus_timeout_get_interval(timeout));

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);

    if (!dbus_timeout_get_enabled(timeout))
        return true;

    if (!QCoreApplication::instance()) {
        d->pendingTimeouts.append(timeout);
        return true;
    }
    int timerId = d->startTimer(dbus_timeout_get_interval(timeout));
    if (!timerId)
        return false;

    d->timeouts[timerId] = timeout;
    return true;
}

static void qDBusRemoveTimeout(DBusTimeout *timeout, void *data)
{
    Q_ASSERT(timeout);
    Q_ASSERT(data);

  //  qDebug("removeTimeout");

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);
    d->pendingTimeouts.removeAll(timeout);

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

static dbus_bool_t qDBusAddWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);

    int flags = dbus_watch_get_flags(watch);
    int fd = dbus_watch_get_fd(watch);

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

static void qDBusRemoveWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    //qDebug("remove watch");

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);
    int fd = dbus_watch_get_fd(watch);

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

static void qDBusToggleWatch(DBusWatch *watch, void *data)
{
    Q_ASSERT(watch);
    Q_ASSERT(data);

    QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);
    int fd = dbus_watch_get_fd(watch);

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

static void qDBusNewConnection(DBusServer *server, DBusConnection *connection, void *data)
{
    Q_ASSERT(data); Q_UNUSED(data);
    //QDBusConnectionPrivate *d = static_cast<QDBusConnectionPrivate *>(data);

    Q_ASSERT(server); Q_UNUSED(server);
    //const char *address = dbus_server_get_address(server);
    //Q_ASSERT(address); // ### for now; handle this case gracefully
    //delete address;
    
    // ### This is an initial untested implementation
    // ### TODO: test and verify that it works
    // ### We may want to separate the server from the QDBusConnectionPrivate

    Q_ASSERT(connection);

    dbus_connection_ref(connection); // keep the connection alive

    QDBusConnectionPrivate *d = new QDBusConnectionPrivate;

    // setConnection does the error handling for us
    d->setConnection(connection);

    const char *name = dbus_bus_get_unique_name(connection);
    Q_ASSERT(name); // ### for now; handle this case gracefully
    QString _name = QLatin1String(name);
    delete name;

    // register the name with the connection manager
    QDBusConnectionPrivate::setConnection(_name, d);

    QDBusConnection retval(_name);
    //d->emitNewServerConnection(retval);
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
    qDBusDebug() << "got message:" << amsg;

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
    case QDBusMessage::MethodCallMessage:
        handleObjectCall(amsg);
        return true;
    case QDBusMessage::ReplyMessage:
    case QDBusMessage::ErrorMessage:
        return false; // we should never get here
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
        qDBusDebug() << "Emitting signal at " << p;
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

static CallDeliveryEvent* prepareReply(QObject *object, int idx, const QList<int> &metaTypes,
                                       const QDBusMessage &msg)
{
    Q_ASSERT(object);

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
    CallDeliveryEvent *data = new CallDeliveryEvent;
    data->object = object;
    data->flags = 0;
    data->message = msg;
    data->metaTypes = metaTypes;
    data->slotIdx = idx;

    return data;
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
        QCoreApplication::postEvent(this, call);
    
}

CallDeliveryEvent *QDBusConnectionPrivate::activateCall(QObject* object, int flags,
                                                        const QDBusMessage &msg)
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
        return 0;

    QDBusSlotCache::Data slotData;

    QDBusSlotCache slotCache =
        qvariant_cast<QDBusSlotCache>(object->property(cachePropertyName));
    QString cacheKey = msg.path();
    cacheKey += msg.member();
    cacheKey += QLatin1Char('.');
    cacheKey += msg.signature();

    QDBusSlotCache::Hash::ConstIterator cacheIt = slotCache.hash.constFind(cacheKey);
    if (cacheIt == slotCache.hash.constEnd())
    {
        // not cached, analyse the meta object
        const QMetaObject *mo = object->metaObject();
        QByteArray memberName = msg.member().toUtf8();

        // find a slot that matches according to the rules above
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
                return 0;
	  }
        }

        // save to the cache
        slotCache.hash.insert(cacheKey, slotData);
        object->setProperty(cachePropertyName, qVariantFromValue(slotCache));
    } else if (cacheIt->slotIdx == -1) {
        // negative cache
        return 0;
    } else {
        slotData = cacheIt.value();
    }

    // found the slot to be called
    // prepare for the call:
    CallDeliveryEvent *call = new CallDeliveryEvent;

    // parameters:
    call->object = object;
    call->flags = flags;
    call->message = msg;

    // save our state:
    call->metaTypes = slotData.metaTypes;
    call->slotIdx = slotData.slotIdx;

    // ready
    return call;
}

void QDBusConnectionPrivate::deliverCall(const CallDeliveryEvent& data) const
{
    // resume state:
    const QList<int>& metaTypes = data.metaTypes;
    const QDBusMessage& msg = data.message;

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
    if (data.object.isNull()) {
        fail = true;
    } else {
        // FIXME: save the old sender!
        QDBusConnection conn(name);
        QDBusContextPrivate context(conn, msg);
	QDBusContextPrivate *old = QDBusContextPrivate::set(data.object, &context);
        QDBusConnectionPrivate::setSender(this);
        fail = data.object->qt_metacall(QMetaObject::InvokeMetaMethod,
                                        data.slotIdx, params.data()) >= 0;
        QDBusConnectionPrivate::setSender(0);
	// the object might be deleted in the slot
	if (!data.object.isNull())
	    QDBusContextPrivate::set(data.object, old);
    }

    // do we create a reply? Only if the caller is waiting for a reply and one hasn't been sent
    // yet.
    if (!msg.isReplyRequired() && !msg.isDelayedReply()) {
        if (!fail) {
            // normal reply
            qDBusDebug() << "Automatically sending reply:" << outputArgs;
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

void QDBusConnectionPrivate::customEvent(QEvent *e)
{
    switch (int(e->type())) {
    case QEvent::User: {
        CallDeliveryEvent* call = static_cast<CallDeliveryEvent *>(e);
        deliverCall(*call);
        break; }
    case QEvent::User + 1: {
        ActivateObjectEvent *activate = static_cast<ActivateObjectEvent *>(e);
        activateObject(activate->node, activate->message);
        break; }
    }
}

QDBusConnectionPrivate::QDBusConnectionPrivate(QObject *p)
    : QObject(p), ref(1), mode(InvalidMode), connection(0), server(0), busService(0),
      rootNode(QString(QLatin1Char('/')))
{
    extern bool qDBusInitThreads();
    static const bool threads = qDBusInitThreads();
    static const bool debugging = !qgetenv("QDBUS_DEBUG").isEmpty();

    Q_UNUSED(threads);
    ::isDebugging = debugging;

    QDBusMetaTypeId::init();
    dbus_error_init(&error);

    rootNode.flags = 0;

    connect(this, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(_q_serviceOwnerChanged(QString,QString,QString)));
}

QDBusConnectionPrivate::~QDBusConnectionPrivate()
{
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    closeConnection();
    rootNode.children.clear();  // free resources
    qDeleteAll(cachedMetaObjects);
}

void QDBusConnectionPrivate::closeConnection()
{
    QWriteLocker locker(&lock);
    ConnectionMode oldMode = mode;
    mode = InvalidMode; // prevent reentrancy
    baseService.clear();
    if (oldMode == ServerMode) {
        if (server) {
            dbus_server_disconnect(server);
            dbus_server_unref(server);
            server = 0;
        }
    } else if (oldMode == ClientMode) {
        if (connection) {
            dbus_connection_close(connection);
            // send the "close" message
            while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS)
                ;
            dbus_connection_unref(connection);
            connection = 0;
        }
    }
}

bool QDBusConnectionPrivate::handleError()
{
    lastError = QDBusError(&error);
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);
    return lastError.isValid();
}

void QDBusConnectionPrivate::bindToApplication()
{
    // Yay, now that we have an application we are in business
    Q_ASSERT_X(QCoreApplication::instance(), "QDBusConnection",
               "qDBusBindToApplication called without an application");
    moveToThread(QCoreApplication::instance()->thread());

    // Re-add all watchers
    WatcherHash oldWatchers = watchers;
    watchers.clear();
    QHashIterator<int, QDBusConnectionPrivate::Watcher> it(oldWatchers);
    while (it.hasNext()) {
        it.next();
        if (!it.value().read && !it.value().write) {
            qDBusAddWatch(it.value().watch, this);
        } else {
            watchers.insertMulti(it.key(), it.value());
        }
    }

    // Re-add all timeouts
    while (!pendingTimeouts.isEmpty())
       qDBusAddTimeout(pendingTimeouts.takeFirst(), this);
}

void QDBusConnectionPrivate::timerEvent(QTimerEvent *e)
{
    DBusTimeout *timeout = timeouts.value(e->timerId(), 0);
    dbus_timeout_handle(timeout);
}

void QDBusConnectionPrivate::doDispatch()
{
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
    QWriteLocker locker(&lock);
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

    QReadLocker locker(&lock);
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

void QDBusConnectionPrivate::sendNoSuchMethodError(const QDBusMessage &msg)
{
    QString interfaceMsg;
    if (msg.interface().isEmpty())
        interfaceMsg = QLatin1String("any interface");
    else
        interfaceMsg = QString::fromLatin1("interface '%1'").arg(msg.interface());

    send(msg.createErrorReply(QDBusError::UnknownMethod,
                              QString::fromLatin1("No such method '%1' in %2 at object path '%3' "
                                                  "(signature '%4')")
                              .arg(msg.member(), interfaceMsg, msg.path(), msg.signature())));
}

bool QDBusConnectionPrivate::activateInternalFilters(const ObjectTreeNode &node, const QDBusMessage &msg)
{
    // object may be null

    if (msg.interface().isEmpty() || msg.interface() == QLatin1String(DBUS_INTERFACE_INTROSPECTABLE)) {
        if (msg.member() == QLatin1String("Introspect") && msg.signature().isEmpty()) {
            //qDebug() << "QDBusConnectionPrivate::activateInternalFilters introspect" << msg.d_ptr->msg;
            QDBusMessage reply = msg.createReply(qDBusIntrospectObject(node));
            send(reply);
            return true;
        }
    }

    if (node.obj && (msg.interface().isEmpty() ||
                     msg.interface() == QLatin1String(DBUS_INTERFACE_PROPERTIES))) {
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
    }

    if (msg.interface() == QLatin1String(DBUS_INTERFACE_INTROSPECTABLE) ||
        msg.interface() == QLatin1String(DBUS_INTERFACE_PROPERTIES)) {
        // if we didn't handle above, it's because the method doesn't exist
        sendNoSuchMethodError(msg);
        return true;
    }

    return false;
}

void QDBusConnectionPrivate::activateObject(const ObjectTreeNode &node, const QDBusMessage &msg)
{
    // This is called by QDBusConnectionPrivate::handleObjectCall to place a call to a slot
    // on the object.
    //
    // The call is routed through the adaptor sub-objects if we have any

    // object may be null
    CallDeliveryEvent *result;

    QDBusAdaptorConnector *connector;
    if (node.flags & QDBusConnection::ExportAdaptors &&
        (connector = qDBusFindAdaptorConnector(node.obj))) {
        int newflags = node.flags | QDBusConnection::ExportNonScriptableSlots;

        if (msg.interface().isEmpty()) {
            // place the call in all interfaces
            // let the first one that handles it to work
            QDBusAdaptorConnector::AdaptorMap::ConstIterator it =
                connector->adaptors.constBegin();
            QDBusAdaptorConnector::AdaptorMap::ConstIterator end =
                connector->adaptors.constEnd();

            for ( ; it != end; ++it) {
                if ((result = activateCall(it->adaptor, newflags, msg))) {
                    deliverCall(*result);
                    delete result;
                    return;
                }
            }
        } else {
            // check if we have an interface matching the name that was asked:
            QDBusAdaptorConnector::AdaptorMap::ConstIterator it;
            it = qLowerBound(connector->adaptors.constBegin(), connector->adaptors.constEnd(),
                             msg.interface());
            if (it != connector->adaptors.constEnd() && msg.interface() == QLatin1String(it->interface)) {
                result = activateCall(it->adaptor, newflags, msg);
                if (result) {
                    deliverCall(*result);
                    delete result;
                } else {
                    sendNoSuchMethodError(msg);
                }
                return;
            }
        }
    }

    // no adaptors matched or were exported
    // try our standard filters
    if (activateInternalFilters(node, msg))
        return;

    // try the object itself:
    if (node.flags & (QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportNonScriptableSlots)) {
        bool interfaceFound = true;
        if (!msg.interface().isEmpty())
            interfaceFound = msg.interface() == qDBusInterfaceFromMetaObject(node.obj->metaObject());

        if (interfaceFound) {
            result = activateCall(node.obj, node.flags, msg);
            if (result) {
                deliverCall(*result);
                delete result;
                return;
            } else {
                sendNoSuchMethodError(msg);
            }
        }
     }

    // nothing matched, send an error code
    if (!msg.interface().isEmpty())
        send(msg.createErrorReply(QDBusError::UnknownMethod,
                                  QString::fromLatin1("No such interface '%1' at object '%2'")
                                  .arg(msg.interface(), msg.path())));
    else
        sendNoSuchMethodError(msg);
}

static bool findObject(const QDBusConnectionPrivate::ObjectTreeNode *root,
                       const QString &fullpath,
                       QDBusConnectionPrivate::ObjectTreeNode &result)
{
    // walk the object tree
    QStringList path = fullpath.split(QLatin1Char('/'));
    if (path.last().isEmpty())
        path.removeLast();      // happens if path is "/"
    int i = 1;
    const QDBusConnectionPrivate::ObjectTreeNode *node = root;

    // try our own tree first
    while (node && !(node->flags & QDBusConnection::ExportChildObjects) ) {
        if (i == path.count()) {
            // found our object
            result = *node;
            return true;
        }

        QDBusConnectionPrivate::ObjectTreeNode::DataList::ConstIterator it =
            qLowerBound(node->children.constBegin(), node->children.constEnd(), path.at(i));
        if (it != node->children.constEnd() && it->name == path.at(i))
            // match
            node = it;
        else
            node = 0;

        ++i;
    }

    // any object in the tree can tell us to switch to its own object tree:
    if (node && node->flags & QDBusConnection::ExportChildObjects) {
        QObject *obj = node->obj;

        while (obj) {
            if (i == path.count()) {
                // we're at the correct level
                QDBusConnectionPrivate::ObjectTreeNode fakenode(*node);
                result = *node;
                result.obj = obj;
                return true;
            }

            const QObjectList children = obj->children();

            // find a child with the proper name
            QObject *next = 0;
            QObjectList::ConstIterator it = children.constBegin();
            QObjectList::ConstIterator end = children.constEnd();
            for ( ; it != end; ++it)
                if ((*it)->objectName() == path.at(i)) {
                    next = *it;
                    break;
                }

            if (!next)
                break;

            ++i;
            obj = next;
        }
    }

    // object not found
    return false;
}

void QDBusConnectionPrivate::handleObjectCall(const QDBusMessage &msg)
{
    QReadLocker locker(&lock);

    ObjectTreeNode result;
    if (findObject(&rootNode, msg.path(), result)) {
        if (QDBusMessagePrivate::isLocal(msg)) {
            //qDebug() << "QDBusConnectionPrivate::activateCall" << msg.d_ptr->msg;
            activateObject(result, msg);
        } else {
            QCoreApplication::postEvent(this, new ActivateObjectEvent(result, msg));
        } 
        return;
    }
    // qDebug("Call failed: no object found at %s", qPrintable(msg.path()));
    send(msg.createErrorReply(QDBusError::UnknownMethod,
                              QString::fromLatin1("No such object '%1'").arg(msg.path())));
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

    QReadLocker locker(&lock);
    handleSignal(key, msg);                  // one try

    key.truncate(msg.member().length() + 1); // keep the ':'
    handleSignal(key, msg);                  // second try

    key = QLatin1Char(':');
    key += msg.interface();
    handleSignal(key, msg);                  // third try
}

static dbus_int32_t server_slot = -1;

void QDBusConnectionPrivate::setServer(DBusServer *s)
{
    if (!server) {
        handleError();
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

void QDBusConnectionPrivate::setConnection(DBusConnection *dbc)
{
    if (!dbc) {
        handleError();
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

        dbus_bus_add_match(connection, filter.constData(), &error);
        if (handleError()) {
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
        qDBusDebug() << "got message: " << msg;
        if (msg.type() == QDBusMessage::ErrorMessage) {
            // got an error, emit it
            emit QDBusErrorHelper(call->receiver, call->errorMethod).pendingCallError(QDBusError(msg), call->message);
            emit connection->callWithCallbackFailed(QDBusError(msg), call->message);
        } else {
            CallDeliveryEvent *e = prepareReply(call->receiver, call->methodIdx, call->metaTypes, msg);
            if (e)
                QCoreApplication::postEvent(connection, e);
            else
                qDBusDebug() << "Deliver failed!";
        }
    }
    dbus_pending_call_unref(pending);
    delete call;
}

int QDBusConnectionPrivate::send(const QDBusMessage& message) const
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

    qDBusDebug() << "sending message:" << message;
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

        qDBusDebug() << "sending message:" << message;
        DBusMessage *reply = dbus_connection_send_with_reply_and_block(connection, msg, timeout, &error);

        handleError();
        dbus_message_unref(msg);

        if (lastError.isValid())
            return QDBusMessage::createError(lastError);

        QDBusMessage amsg = QDBusMessagePrivate::fromDBusMessage(reply);
        dbus_message_unref(reply);
        qDBusDebug() << "got message:" << amsg;

        if (dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS)
            QMetaObject::invokeMethod(this, "doDispatch", Qt::QueuedConnection);

        return amsg;
    } else { // use the event loop
        QDBusReplyWaiter waiter;
        if (sendWithReplyAsync(message, &waiter, SLOT(reply(QDBusMessage)),
                               SLOT(error()), timeout) > 0) {
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
    qDBusDebug() << "sending message via local-loop:" << message;

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
    qDBusDebug() << "got message via local-loop:" << localReplyMsg;
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

    qDBusDebug() << "sending message:" << message;
    DBusPendingCall *pending = 0;
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
    connect(hook.obj, SIGNAL(destroyed(QObject*)), SLOT(objectDestroyed(QObject*)));

    if (connection) {
        qDBusDebug("Adding rule: %s", hook.matchRule.constData());
        dbus_bus_add_match(connection, hook.matchRule, &error);
        if (handleError())
            qWarning("QDBusConnectionPrivate::connectSignal: received error from D-Bus server "
                     "while connecting signal to %s::%s: %s (%s)",
                     hook.obj->metaObject()->className(),
                     hook.obj->metaObject()->method(hook.midx).signature(),
                     qPrintable(lastError.name()), qPrintable(lastError.message()));
    }
}

void QDBusConnectionPrivate::disconnectSignal(SignalHookHash::Iterator &it)
{
    const SignalHook &hook = it.value();
    hook.obj->disconnect(SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));

    // we don't care about errors here
    if (connection) {
        qDBusDebug("Removing rule: %s", hook.matchRule.constData());
        dbus_bus_remove_match(connection, hook.matchRule, NULL);
    }

    signalHooks.erase(it);
}

void QDBusConnectionPrivate::registerObject(const ObjectTreeNode *node)
{
    connect(node->obj, SIGNAL(destroyed(QObject*)), SLOT(objectDestroyed(QObject*)));

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
                this, SLOT(relaySignal(QObject*,const QMetaObject*,int,QVariantList)));
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
    QWriteLocker locker(&lock);
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
    QWriteLocker locker(&lock);
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
    if (!lastError.isValid() && reply.type() == QDBusMessage::ReplyMessage)
        return reply.arguments().at(0).toString();
    return QString();
}

QDBusMetaObject *
QDBusConnectionPrivate::findMetaObject(const QString &service, const QString &path,
                                       const QString &interface)
{
    // introspect the target object
    QDBusMessage msg = QDBusMessage::createMethodCall(service, path,
                                                QLatin1String(DBUS_INTERFACE_INTROSPECTABLE),
                                                QLatin1String("Introspect"));

    QDBusMessage reply = sendWithReply(msg, QDBus::Block);

    // it doesn't exist yet, we have to create it
    QDBusMetaObject *mo = 0;
    if (!interface.isEmpty()) {
        QReadLocker locker(&lock);
        mo = cachedMetaObjects.value(interface, 0);
    if (mo)
        // maybe it got created when we switched from read to write lock
        return mo;
    }

    QWriteLocker locker(&lock);
    QString xml;
    if (reply.type() == QDBusMessage::ReplyMessage)
        // fetch the XML description
        xml = reply.arguments().at(0).toString();
    else {
        lastError = reply;
        if (reply.type() != QDBusMessage::ErrorMessage || lastError.type() != QDBusError::UnknownMethod)
            return 0; // error
    }

    // release the lock and return
    return QDBusMetaObject::createMetaObject(interface, xml, cachedMetaObjects, lastError);
}

void QDBusConnectionPrivate::registerService(const QString &serviceName)
{
    QWriteLocker locker(&lock);
    serviceNames.append(serviceName);
}

void QDBusConnectionPrivate::unregisterService(const QString &serviceName)
{
    QWriteLocker locker(&lock);
    serviceNames.removeAll(serviceName);
}

bool QDBusConnectionPrivate::isServiceRegisteredByThread(const QString &serviceName) const
{
    if (serviceName == baseService)
        return true;
    QStringList copy = serviceNames;
    return copy.contains(serviceName);
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
