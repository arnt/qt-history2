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

#include "qdbusmessage.h"

#include <qdebug.h>
#include <qstringlist.h>

#include <dbus/dbus.h>

#include "qdbusargument_p.h"
#include "qdbuserror.h"
#include "qdbusmessage_p.h"
#include "qdbusmetatype.h"
#include "qdbusconnection_p.h"

static inline const char *data(const QByteArray &arr)
{
    return arr.isEmpty() ? 0 : arr.constData();
}

QDBusMessagePrivate::QDBusMessagePrivate()
    : msg(0), reply(0), type(DBUS_MESSAGE_TYPE_INVALID),
      timeout(-1), localReply(0), ref(1), delayedReply(false), localMessage(false)
{
}

QDBusMessagePrivate::~QDBusMessagePrivate()
{
    if (msg)
        dbus_message_unref(msg);
    if (reply)
        dbus_message_unref(reply);
    delete localReply;
}

/*!
    \since 4.3
     Returns the human-readable message associated with the error that was received.
*/
QString QDBusMessage::errorMessage() const
{
    if (d_ptr->type == ErrorMessage) {
        if (!d_ptr->message.isEmpty())
           return d_ptr->message;
        if (!d_ptr->arguments.isEmpty())
            return d_ptr->arguments.at(0).toString();
    }
    return QString();
}

/*!
    \internal
    Constructs a DBusMessage object from this object. The returned value must be de-referenced
    with dbus_message_unref.
*/
DBusMessage *QDBusMessagePrivate::toDBusMessage(const QDBusMessage &message)
{
    DBusMessage *msg = 0;
    const QDBusMessagePrivate *d_ptr = message.d_ptr;

    switch (d_ptr->type) {
    case DBUS_MESSAGE_TYPE_INVALID:
        //qDebug() << "QDBusMessagePrivate::toDBusMessage" <<  "message is invalid";
        break;
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        msg = dbus_message_new_method_call(data(d_ptr->service.toUtf8()), data(d_ptr->path.toUtf8()),
                                           data(d_ptr->interface.toUtf8()), data(d_ptr->name.toUtf8()));
        break;
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        msg = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);
        if (!d_ptr->localMessage) {
            dbus_message_set_destination(msg, dbus_message_get_sender(d_ptr->reply));
            dbus_message_set_reply_serial(msg, dbus_message_get_serial(d_ptr->reply));
        }
        break;
    case DBUS_MESSAGE_TYPE_ERROR:
        msg = dbus_message_new(DBUS_MESSAGE_TYPE_ERROR);
        dbus_message_set_error_name(msg, data(d_ptr->name.toUtf8()));
        if (!d_ptr->localMessage) {
            dbus_message_set_destination(msg, dbus_message_get_sender(d_ptr->reply));
            dbus_message_set_reply_serial(msg, dbus_message_get_serial(d_ptr->reply));
        }
        break;
    case DBUS_MESSAGE_TYPE_SIGNAL:
        msg = dbus_message_new_signal(data(d_ptr->path.toUtf8()), data(d_ptr->interface.toUtf8()),
                                      data(d_ptr->name.toUtf8()));
        break;
    default:
        Q_ASSERT(false);
        break;
    }
#if 0
    DBusError err;
    dbus_error_init(&err);
    if (dbus_error_is_set(&err)) {
        QDBusError qe(&err);
        qDebug() << "QDBusMessagePrivate::toDBusMessage" << qe;
    }
#endif
    if (!msg)
        return 0;

    QDBusMarshaller marshaller;
    QVariantList::ConstIterator it =  d_ptr->arguments.constBegin();
    QVariantList::ConstIterator cend = d_ptr->arguments.constEnd();
    dbus_message_iter_init_append(msg, &marshaller.iterator);
    if (!d_ptr->message.isEmpty())
        // prepend the error message
        marshaller.append(d_ptr->message);
    for ( ; it != cend; ++it)
        marshaller.appendVariantInternal(*it);

    // check if everything is ok
    if (marshaller.ok)
        return msg;

    // not ok;
    dbus_message_unref(msg);
    Q_ASSERT(false);
    return 0;
}

/*
struct DBusMessage
{
    DBusAtomic refcount;
    DBusHeader header;
    DBusString body;
    char byte_order;
    unsigned int locked : 1;
DBUS_DISABLE_CHECKS
    unsigned int in_cache : 1;
#endif
    DBusList *size_counters;
    long size_counter_delta;
    dbus_uint32_t changed_stamp : CHANGED_STAMP_BITS;
    DBusDataSlotList slot_list;
#ifndef DBUS_DISABLE_CHECKS
    int generation;
#endif
};
*/

/*!
    \internal
    Constructs a QDBusMessage by parsing the given DBusMessage object.
*/
QDBusMessage QDBusMessagePrivate::fromDBusMessage(DBusMessage *dmsg)
{
    QDBusMessage message;
    if (!dmsg)
        return message;

    message.d_ptr->type = dbus_message_get_type(dmsg);
    message.d_ptr->path = QString::fromUtf8(dbus_message_get_path(dmsg));
    message.d_ptr->interface = QString::fromUtf8(dbus_message_get_interface(dmsg));
    message.d_ptr->name = message.d_ptr->type == DBUS_MESSAGE_TYPE_ERROR ?
                      QString::fromUtf8(dbus_message_get_error_name(dmsg)) :
                      QString::fromUtf8(dbus_message_get_member(dmsg));
    message.d_ptr->service = QString::fromUtf8(dbus_message_get_sender(dmsg));
    message.d_ptr->signature = QString::fromUtf8(dbus_message_get_signature(dmsg));
    message.d_ptr->msg = dbus_message_ref(dmsg);

    QDBusDemarshaller demarshaller;
    demarshaller.message = dbus_message_ref(dmsg);
    if (dbus_message_iter_init(demarshaller.message, &demarshaller.iterator))
        while (!demarshaller.atEnd())
            message << demarshaller.toVariantInternal();
    return message;
}

bool QDBusMessagePrivate::isLocal(const QDBusMessage &message)
{
    return message.d_ptr->localMessage;
}

QDBusMessage QDBusMessagePrivate::makeLocal(const QDBusConnectionPrivate &conn,
                                            const QDBusMessage &asSent)
{
    // simulate the message being sent to the bus and then received back
    // the only field that the bus sets when delivering the message
    // (as opposed to the message as we send it), is the sender
    // so we simply set the sender to our unique name

    // determine if we are carrying any complex types
    QString computedSignature;
    QVariantList::ConstIterator it = asSent.d_ptr->arguments.constBegin();
    QVariantList::ConstIterator end = asSent.d_ptr->arguments.constEnd();
    for ( ; it != end; ++it) {
        int id = it->userType();
        const char *signature = QDBusMetaType::typeToSignature(id);
        if ((id != QVariant::StringList && id != QVariant::ByteArray &&
             qstrlen(signature) != 1) || id == qMetaTypeId<QDBusVariant>()) {
            // yes, we are
            // we must marshall and demarshall again so as to create QDBusArgument
            // entries for the complex types
            DBusMessage *message = toDBusMessage(asSent);
            dbus_message_set_sender(message, conn.baseService.toUtf8());

            QDBusMessage retval = fromDBusMessage(message);
            retval.d_ptr->localMessage = true;
            dbus_message_unref(message);
            if (retval.d_ptr->service.isEmpty())
                retval.d_ptr->service = conn.baseService;
            return retval;
        } else {
            computedSignature += QLatin1String(signature);
        }
    }

    // no complex types seen
    // optimise by using the variant list itself
    QDBusMessage retval;
    QDBusMessagePrivate *d = retval.d_ptr;
    d->arguments = asSent.d_ptr->arguments;
    d->path = asSent.d_ptr->path;
    d->interface = asSent.d_ptr->interface;
    d->name = asSent.d_ptr->name;
    d->message = asSent.d_ptr->message;
    d->type = asSent.d_ptr->type;

    d->service = conn.baseService;
    d->signature = computedSignature;
    d->localMessage = true;
    return retval;
}

QDBusMessage QDBusMessagePrivate::makeLocalReply(const QDBusConnectionPrivate &conn,
                                                 const QDBusMessage &callMsg)
{
    // simulate the reply (return or error) message being sent to the bus and
    // then received back.
    if (callMsg.d_ptr->localReply)
        return makeLocal(conn, *callMsg.d_ptr->localReply);
    return QDBusMessage();      // failed
}

/*!
    \class QDBusMessage
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusMessage class represents one message sent or
    received over the D-Bus bus.

    This object can represent any of the four different types of
    messages (MessageType) that can occur on the bus:

    \list
      \o Method calls
      \o Method return values
      \o Signal emissions
      \o Error codes
    \endlist

    Objects of this type are created with the static createError(),
    createMethodCall() and createSignal() functions. Use the
    QDBusConnection::send() function to send the messages.
*/

/*!
    \enum QDBusMessage::MessageType
    The possible message types:

    \value MethodCallMessage    a message representing an outgoing or incoming method call
    \value SignalMessage        a message representing an outgoing or incoming signal emission
    \value ReplyMessage         a message representing the return values of a method call
    \value ErrorMessage         a message representing an error condition in response to a method call
    \value InvalidMessage       an invalid message: this is never set on messages received from D-Bus
*/

/*!
    Constructs a new DBus message with the given \a path, \a interface
    and \a name, representing a signal emission.

    A DBus signal is emitted from one application and is received by
    all applications that are listening for that signal from that
    interface.

    The QDBusMessage object that is returned can be sent using the
    QDBusConnection::send() function.
*/
QDBusMessage QDBusMessage::createSignal(const QString &path, const QString &interface,
                                        const QString &name)
{
    QDBusMessage message;
    message.d_ptr->type = DBUS_MESSAGE_TYPE_SIGNAL;
    message.d_ptr->path = path;
    message.d_ptr->interface = interface;
    message.d_ptr->name = name;

    return message;
}

/*!
    Constructs a new DBus message representing a method call.
    A method call always informs its destination address
    (\a service, \a path, \a interface and \a method).

    The DBus bus allows calling a method on a given remote object without specifying the
    destination interface, if the method name is unique. However, if two interfaces on the
    remote object export the same method name, the result is undefined (one of the two may be
    called or an error may be returned).

    When using DBus in a peer-to-peer context (i.e., not on a bus), the \a service parameter is
    optional.

    The QDBusObject and QDBusInterface classes provide a simpler abstraction to synchronous
    method calling.

    This function returns a QDBusMessage object that can be sent with
    QDBusConnection::call().
*/
QDBusMessage QDBusMessage::createMethodCall(const QString &service, const QString &path,
                                            const QString &interface, const QString &method)
{
    QDBusMessage message;
    message.d_ptr->type = DBUS_MESSAGE_TYPE_METHOD_CALL;
    message.d_ptr->service = service;
    message.d_ptr->path = path;
    message.d_ptr->interface = interface;
    message.d_ptr->name = method;

    return message;
}

/*!
    Constructs a new DBus message representing an error,
    with the given \a name and \a msg.
*/
QDBusMessage QDBusMessage::createError(const QString &name, const QString &msg)
{
    QDBusMessage error;
    error.d_ptr->type = DBUS_MESSAGE_TYPE_ERROR;
    error.d_ptr->name = name;
    error.d_ptr->message = msg;

    return error;
}

/*!
    \fn QDBusMessage QDBusMessage::createError(const QDBusError &error)

    Constructs a new DBus message representing the given \a error.
*/

/*!
  \fn QDBusMessage QDBusMessage::createError(QDBusError::ErrorType type, const QString &msg)

  Constructs a new DBus message for the error type \a type using
  the message \a msg. Returns the DBus message.
*/

/*!
    \fn QDBusMessage QDBusMessage::createReply(const QList<QVariant> &arguments) const

    Constructs a new DBus message representing a reply, with the given
    \a arguments.
*/
QDBusMessage QDBusMessage::createReply(const QVariantList &arguments) const
{
    QDBusMessage reply;
    reply.setArguments(arguments);
    reply.d_ptr->type = DBUS_MESSAGE_TYPE_METHOD_RETURN;
    if (d_ptr->msg)
        reply.d_ptr->reply = dbus_message_ref(d_ptr->msg);
    if (d_ptr->localMessage) {
        reply.d_ptr->localMessage = true;
        d_ptr->localReply = new QDBusMessage(reply); // keep an internal copy
    }

    // the reply must have a msg or be a local-loop optimisation
    Q_ASSERT(reply.d_ptr->reply || reply.d_ptr->localMessage);
    return reply;
}

/*!
    Constructs a new DBus message representing an error reply message,
    with the given \a name and \a msg.
*/
QDBusMessage QDBusMessage::createErrorReply(const QString name, const QString &msg) const
{
    QDBusMessage reply = QDBusMessage::createError(name, msg);
    if (d_ptr->msg)
        reply.d_ptr->reply = dbus_message_ref(d_ptr->msg);
    if (d_ptr->localMessage) {
        reply.d_ptr->localMessage = true;
        d_ptr->localReply = new QDBusMessage(reply); // keep an internal copy
    }

    // the reply must have a msg or be a local-loop optimisation
    Q_ASSERT(reply.d_ptr->reply || reply.d_ptr->localMessage);
    return reply;
}

/*!
   \fn QDBusMessage QDBusMessage::createReply(const QVariant &argument) const

    Constructs a new DBus message representing a reply, with the
    given \a argument.
*/

/*!
    \fn QDBusMessage QDBusMessage::createErrorReply(const QDBusError &error) const

    Constructs a new DBus message representing an error reply message,
    from the given \a error object.
*/

/*!
  \fn QDBusMessage QDBusMessage::createErrorReply(QDBusError::ErrorType type, const QString &msg) const

  Constructs a new DBus reply message for the error type \a type using
  the message \a msg. Returns the DBus message.
*/

/*!
    Constructs an empty, invalid QDBusMessage object.

    \sa createError(), createMethodCall(), createSignal()
*/
QDBusMessage::QDBusMessage()
{
    d_ptr = new QDBusMessagePrivate;
}

/*!
    Constructs a copy of the object given by \a other.

    Note: QDBusMessage objects are shared. Modifications made to the
    copy will affect the original one as well. See setDelayedReply()
    for more information.
*/
QDBusMessage::QDBusMessage(const QDBusMessage &other)
{
    d_ptr = other.d_ptr;
    d_ptr->ref.ref();
}

/*!
    Disposes of the object and frees any resources that were being held.
*/
QDBusMessage::~QDBusMessage()
{
    if (!d_ptr->ref.deref())
        delete d_ptr;
}

/*!
    Copies the contents of the object given by \a other.

    Note: QDBusMessage objects are shared. Modifications made to the
    copy will affect the original one as well. See setDelayedReply()
    for more information.
*/
QDBusMessage &QDBusMessage::operator=(const QDBusMessage &other)
{
    qAtomicAssign(d_ptr, other.d_ptr);
    return *this;
}

/*!
    Returns the name of the service or the bus address of the remote method call.
*/
QString QDBusMessage::service() const
{
    return d_ptr->service;
}

/*!
    Returns the path of the object that this message is being sent to (in the case of a
    method call) or being received from (for a signal).
*/
QString QDBusMessage::path() const
{
    return d_ptr->path;
}

/*!
    Returns the interface of the method being called (in the case of a method call) or of
    the signal being received from.
*/
QString QDBusMessage::interface() const
{
    return d_ptr->interface;
}

/*!
    Returns the name of the signal that was emitted or the name of the method that was called.
*/
QString QDBusMessage::member() const
{
    if (d_ptr->type != ErrorMessage)
        return d_ptr->name;
    return QString();
}

/*!
    Returns the name of the error that was received.
*/
QString QDBusMessage::errorName() const
{
    if (d_ptr->type == ErrorMessage)
        return d_ptr->name;
    return QString();
}

/*!
    Returns the signature of the signal that was received or for the output arguments
    of a method call.
*/
QString QDBusMessage::signature() const
{
    return d_ptr->signature;
}

/*!
    Returns the flag that indicates if this message should see a reply
    or not. This is only meaningful for \l {MethodCallMessage}{method
    call messages}: any other kind of message cannot have replies and
    this function will always return false for them.
*/
bool QDBusMessage::isReplyRequired() const
{
    if (!d_ptr->msg)
        return false;
    return !dbus_message_get_no_reply(d_ptr->msg);
}

/*!
    Sets whether the message will be replied later (if \a enable is
    true) or if an automatic reply should be generated by QtDBus
    (if \a enable is false).

    In D-Bus, all method calls must generate a reply to the caller, unless the
    caller explicitly indicates otherwise (see isReplyRequired()). QtDBus
    automatically generates such replies for any slots being called, but it
    also allows slots to indicate whether they will take responsibility
    of sending the reply at a later time, after the function has finished
    processing.

    \sa {Delayed Replies}
*/
void QDBusMessage::setDelayedReply(bool enable) const
{
    d_ptr->delayedReply = enable;
}

/*!
    Returns the delayed reply flag, as set by setDelayedReply(). By default, this
    flag is false, which means QtDBus will generate automatic replies
    when necessary.
*/
bool QDBusMessage::isDelayedReply() const
{
    return d_ptr->delayedReply;
}

/*!
    Sets the arguments that are going to be sent over D-Bus to \a arguments. Those
    will be the arguments to a method call or the parameters in the signal.

    \sa arguments()
*/
void QDBusMessage::setArguments(const QList<QVariant> &arguments)
{
    // FIXME: should we detach?
    d_ptr->arguments = arguments;
}

/*!
    Returns the list of arguments that are going to be sent or were received from
    D-Bus.
*/
QList<QVariant> QDBusMessage::arguments() const
{
    return d_ptr->arguments;
}

/*!
    Appends the argument \a arg to the list of arguments to be sent over D-Bus in
    a method call or signal emission.
*/

QDBusMessage &QDBusMessage::operator<<(const QVariant &arg)
{
    // FIXME: should we detach?
    d_ptr->arguments.append(arg);
    return *this;
}

/*!
    Returns the message type.
*/
QDBusMessage::MessageType QDBusMessage::type() const
{
    switch (d_ptr->type) {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        return MethodCallMessage;
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        return ReplyMessage;
    case DBUS_MESSAGE_TYPE_ERROR:
        return ErrorMessage;
    case DBUS_MESSAGE_TYPE_SIGNAL:
        return SignalMessage;
    default:
        break;
    }
    return InvalidMessage;
}

/*!
    Sends the message without waiting for a reply. This is suitable
    for errors, signals, and return values as well as calls whose
    return values are not necessary.

    Returns true if the message was queued successfully;
    otherwise returns false.

    \sa QDBusConnection::send()
*/
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, QDBusMessage::MessageType t)
{
    switch (t)
    {
    case QDBusMessage::MethodCallMessage:
        return dbg << "MethodCall";
    case QDBusMessage::ReplyMessage:
        return dbg << "MethodReturn";
    case QDBusMessage::SignalMessage:
        return dbg << "Signal";
    case QDBusMessage::ErrorMessage:
        return dbg << "Error";
    default:
        return dbg << "Invalid";
    }
}

static void debugVariantList(QDebug dbg, const QVariantList &list);
static void debugVariantMap(QDebug dbg, const QVariantMap &map);

static void debugVariant(QDebug dbg, const QVariant &v)
{
    if (v.userType() == qMetaTypeId<QDBusArgument>()) {
        dbg.nospace() << "argument of type "
                      << qvariant_cast<QDBusArgument>(v).currentSignature();
        return;
    }
    dbg.nospace() << v.typeName() << "(";
    switch (v.userType())
    {
    case QVariant::Bool:
        dbg.nospace() << v.toBool();
        break;
    case QMetaType::UChar:
        dbg.nospace() << qvariant_cast<uchar>(v);
        break;
    case QMetaType::Short:
        dbg.nospace() << qvariant_cast<short>(v);
        break;
    case QMetaType::UShort:
        dbg.nospace() << qvariant_cast<ushort>(v);
        break;
    case QVariant::Int:
        dbg.nospace() << v.toInt();
        break;
    case QVariant::UInt:
        dbg.nospace() << v.toUInt();
        break;
    case QVariant::LongLong:
        dbg.nospace() << v.toLongLong();
        break;
    case QVariant::ULongLong:
        dbg.nospace() << v.toULongLong();
        break;
    case QVariant::Double:
        dbg.nospace() << v.toDouble();
        break;
    case QVariant::String:
        dbg.nospace() << v.toString();
        break;
    case QVariant::ByteArray:
        dbg.nospace() << v.toByteArray();
        break;
    case QVariant::StringList:
        dbg.nospace() << v.toStringList();
        break;
    case QVariant::List:
        debugVariantList(dbg, v.toList());
        break;
    case QVariant::Map:
        debugVariantMap(dbg, v.toMap());
        break;

    default: {
        int id = v.userType();
        if (id == qMetaTypeId<QDBusVariant>())
            debugVariant(dbg, qvariant_cast<QDBusVariant>(v).variant());
        else if (id == qMetaTypeId<QDBusObjectPath>())
            dbg.nospace() << qvariant_cast<QDBusObjectPath>(v).path();
        else if (id == qMetaTypeId<QDBusSignature>())
            dbg.nospace() << qvariant_cast<QDBusSignature>(v).signature();
        else
            dbg.nospace() << "unknown";
    }
    }
    dbg.nospace() << ")";
}

static void debugVariantList(QDebug dbg, const QVariantList &list)
{
    bool first = true;
    QVariantList::ConstIterator it = list.constBegin();
    QVariantList::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it) {
        if (!first)
            dbg.nospace() << ", ";
        debugVariant(dbg, *it);
        first = false;
    }
}

static void debugVariantMap(QDebug dbg, const QVariantMap &map)
{
    QVariantMap::ConstIterator it = map.constBegin();
    QVariantMap::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        dbg << "(" << it.key() << ", ";
        debugVariant(dbg, it.value());
        dbg.nospace() << ") ";
    }
}

QDebug operator<<(QDebug dbg, const QDBusMessage &msg)
{
    dbg.nospace() << "QDBusMessage(type=" << msg.type()
                  << ", service=" << msg.service();
    if (msg.type() == QDBusMessage::MethodCallMessage ||
        msg.type() == QDBusMessage::SignalMessage)
        dbg.nospace() << ", path=" << msg.path()
                      << ", interface=" << msg.interface()
                      << ", member=" << msg.member();
    if (msg.type() == QDBusMessage::ErrorMessage)
        dbg.nospace() << ", error name=" << msg.errorName()
                      << ", error message=" << msg.errorMessage();
    dbg.nospace() << ", signature=" << msg.signature()
                  << ", contents=(";
    debugVariantList(dbg, msg.arguments());
    dbg.nospace() << ") )";
    return dbg.space();
}
#endif

