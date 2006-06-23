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

#include "qdbusabstractinterface.h"

#include "qdbusabstractinterface_p.h"
#include "qdbusmetaobject_p.h"
#include "qdbusconnection_p.h"

QVariant QDBusAbstractInterfacePrivate::property(const QMetaProperty &mp) const
{
    // try to read this property
    QDBusMessage msg = QDBusMessage::methodCall(service, path,
                                                QLatin1String(DBUS_INTERFACE_PROPERTIES),
                                                QLatin1String("Get"));
    msg << interface << QString::fromUtf8(mp.name());
    QDBusMessage reply = connp->sendWithReply(msg, QDBusConnection::NoUseEventLoop);

    if (reply.type() == QDBusMessage::ReplyMessage && reply.count() == 1 &&
        reply.signature() == QLatin1String("v")) {
        QVariant value = qvariant_cast<QDBusVariant>(reply.at(0)).value;

        // make sure the type is right
        if (qstrcmp(mp.typeName(), value.typeName()) == 0) {
            if (mp.type() == QVariant::LastType)
                // QVariant is special in this context
                return qvariant_cast<QDBusVariant>(reply.at(0)).value;

            return value;
        }
    }

    // there was an error...
    if (reply.type() == QDBusMessage::ErrorMessage)
        lastError = reply;
    else if (reply.signature() != QLatin1String("v")) {
        QString errmsg = QLatin1String("Invalid signature `%1' in return from call to "
                                       DBUS_INTERFACE_PROPERTIES);
        lastError = QDBusError(QDBusError::InvalidSignature, errmsg.arg(reply.signature()));
    } else {
        QString errmsg = QLatin1String("Unexpected type `%1' when retrieving property "
                                       "`%2 %3.%4'");
        lastError = QDBusError(QDBusError::InvalidSignature,
                               errmsg.arg(QLatin1String(reply.at(0).typeName()),
                                          QLatin1String(mp.typeName()),
                                          interface, QString::fromUtf8(mp.name())));
    }

    return QVariant();
}

void QDBusAbstractInterfacePrivate::setProperty(const QMetaProperty &mp, const QVariant &value)
{
    // send the value
    QDBusMessage msg = QDBusMessage::methodCall(service, path,
                                                QLatin1String(DBUS_INTERFACE_PROPERTIES),
                                                QLatin1String("Set"));
    msg << interface << QString::fromUtf8(mp.name()) << qVariantFromValue(QDBusVariant(value));
    QDBusMessage reply = connp->sendWithReply(msg, QDBusConnection::NoUseEventLoop);

    if (reply.type() != QDBusMessage::ReplyMessage)
        lastError = reply;
}    

/*!
    \class QDBusAbstractInterface
    \inmodule QtDBus
    \brief Base class for all D-Bus interfaces in the QtDBus binding, allowing access to remote interfaces.

    Generated-code classes also derive from QDBusAbstractInterface, all methods described here are also
    valid for generated-code classes. In addition to those described here, generated-code classes
    provide member functions for the remote methods, which allow for compile-time checking of the
    correct parameters and return values, as well as property type-matching and signal
    parameter-matching.

    \sa {dbusxml2cpp.html}{The dbusxml2cpp compiler}, QDBusInterface
*/

/*!
    \enum QDBusAbstractInterface::CallMode

    Specifies how a call should be placed. The valid options are:
    \value AutoDetect           automatically detect if the called function has a reply to be returned
    \value NoWaitForReply       place the call but don't wait for the reply (the reply's contents
                                will be discarded)
    \value NoUseEventLoop       don't use an event loop to wait for a reply, but instead block on
                                network operations while waiting. This option means the
                                user-interface may not be updated for the duration of the call.
    \value UseEventLoop         use the Qt event loop to wait for a reply. This option means the
                                user-interface will update, but it also means other events may
                                happen, like signal delivery and other D-Bus method calls.

    When using UseEventLoop, applications must be prepared for reentrancy in any function.
*/

/*!
    \internal
*/
QDBusAbstractInterface::QDBusAbstractInterface(QDBusAbstractInterfacePrivate* d)
    : QObject(*d, 0)
{
}

/*!
    Releases this object's resources.
*/
QDBusAbstractInterface::~QDBusAbstractInterface()
{
}

/*!
    Returns true if this is a valid reference to a remote object. It returns false if
    there was an error during the creation of this interface (for instance, if the remote
    application does not exist).

    Note: when dealing with remote objects, it is not always possible to determine if it
    exists when creating a QDBusInterface or QDBusInterfacePtr object.
*/
bool QDBusAbstractInterface::isValid() const
{
    return d_func()->isValid;
}

/*!
    Returns the connection this interface is assocated with.
*/
QDBusConnection QDBusAbstractInterface::connection() const
{
    return d_func()->conn;
}

/*!
    Returns the name of the service this interface is associated with.
*/
QString QDBusAbstractInterface::service() const
{
    return d_func()->service;
}

/*!
    Returns the object path that this interface is associated with.
*/
QString QDBusAbstractInterface::path() const
{
    return d_func()->path;
}

/*!
    Returns the name of this interface.
*/
QString QDBusAbstractInterface::interface() const
{
    return d_func()->interface;
}

/*!
    Returns the error the last operation produced, or an invalid error if the last operation did not
    produce an error.
*/
QDBusError QDBusAbstractInterface::lastError() const
{
    return d_func()->lastError;
}

/*!
    Places a call to the remote method specified by \a method on this interface, using \a args as
    arguments. This function returns the message that was received as a reply, which can be a normal
    QDBusMessage::ReplyMessage (indicating success) or QDBusMessage::ErrorMessage (if the call
    failed). The \a mode parameter specifies how this call should be placed.

    If the call succeeds, lastError() will be cleared; otherwise, it will contain the error this
    call produced.

    Normally, you should place calls using call().

    \warning If you use \c UseEventLoop, your code must be prepared to deal with any reentrancy:
             other method calls and signals may be delivered before this function returns, as well
             as other Qt queued signals and events.

    \threadsafe
*/
QDBusMessage QDBusAbstractInterface::callWithArgs(const QString& method, const QList<QVariant>& args,
                                          CallMode mode)
{
    Q_D(QDBusAbstractInterface);

    QString m = method;
    // split out the signature from the method
    int pos = method.indexOf(QLatin1Char('.'));
    if (pos != -1)
        m.truncate(pos);

    if (mode == AutoDetect) {
        // determine if this a sync or async call
        mode = NoUseEventLoop;
        const QMetaObject *mo = metaObject();
        QByteArray match = m.toLatin1() + '(';

        for (int i = staticMetaObject.methodCount(); i < mo->methodCount(); ++i) {
            QMetaMethod mm = mo->method(i);
            if (QByteArray(mm.signature()).startsWith(match)) {
                // found a method with the same name as what we're looking for
                // hopefully, nobody is overloading asynchronous and synchronous methods with
                // the same name

                QList<QByteArray> tags = QByteArray(mm.tag()).split(' ');
                if (tags.contains("Q_NOREPLY"))
                    mode = NoWaitForReply;

                break;
            }
        }
    }

    QDBusMessage msg = QDBusMessage::methodCall(service(), path(), interface(), m);
    msg.QList<QVariant>::operator=(args);

    QDBusMessage reply;
    if (mode != NoWaitForReply)
        reply = d->conn.sendWithReply(msg, mode == UseEventLoop ?
                                      QDBusConnection::UseEventLoop : QDBusConnection::NoUseEventLoop);
    else
        d->conn.send(msg);

    d->lastError = reply;       // will clear if reply isn't an error

    // ensure that there is at least one element
    if (reply.isEmpty())
        reply << QVariant();

    return reply;
}

/*!
    \overload
    Places a call to the remote method specified by \a method on this interface, using \a args as
    arguments. This function will return immediately after queueing the call. The reply from the
    remote function or any errors emitted by it will be delivered to the \a slot slot on object \a
    receiver.

    This function returns true if the queueing succeeded: it does not indicate that the call
    succeeded. If it failed, the slot will be called with an error message. lastError() will not be
    set under those circumstances.

    \sa QDBusError, QDBusMessage
*/
bool QDBusAbstractInterface::callWithArgs(const QString &method, QObject *receiver, const char *slot,
                                          const QList<QVariant> &args)
{
    Q_D(QDBusAbstractInterface);
    
    QString m = method;
    // split out the signature from the method
    int pos = method.indexOf(QLatin1Char('.'));
    if (pos != -1)
        m.truncate(pos);

    QDBusMessage msg = QDBusMessage::methodCall(service(), path(), interface(), m);
    msg.QList<QVariant>::operator=(args);

    d->lastError = 0;           // clear
    return d->conn.sendWithReplyAsync(msg, receiver, slot);
}

/*!
    \internal
    Catch signal connections.
*/
void QDBusAbstractInterface::connectNotify(const char *signal)
{
    // someone connecting to one of our signals
    Q_D(QDBusAbstractInterface);

    d->connp->connectRelay(d->service, d->path, d->interface, this, signal);
}

/*!
    \internal
    Catch signal disconnections.
*/
void QDBusAbstractInterface::disconnectNotify(const char *signal)
{
    // someone disconnecting from one of our signals
    Q_D(QDBusAbstractInterface);

    d->connp->disconnectRelay(d->service, d->path, d->interface, this, signal);
}

/*!
    \internal
    Get the value of the property \a propname.
*/
QVariant QDBusAbstractInterface::internalPropGet(const char *propname) const
{
    // assume this property exists and is readable
    // we're only called from generated code anyways

    int idx = metaObject()->indexOfProperty(propname);
    if (idx != -1)
        return d_func()->property(metaObject()->property(idx));
    qWarning("QDBusAbstractInterface::internalPropGet called with unknown property '%s'", propname);
    return QVariant();          // error
}

/*!
    \internal
    Set the value of the property \a propname to \a value.
*/
void QDBusAbstractInterface::internalPropSet(const char *propname, const QVariant &value)
{
    Q_D(QDBusAbstractInterface);

    // assume this property exists and is writeable
    // we're only called from generated code anyways

    int idx = metaObject()->indexOfProperty(propname);
    if (idx != -1)
        d->setProperty(metaObject()->property(idx), value);
    else
        qWarning("QDBusAbstractInterface::internalPropGet called with unknown property '%s'", propname);
}

/*!
    \fn QDBusMessage QDBusAbstractInterface::call(const QString &method)

    Calls the method \a method on this interface and passes the parameters to this function to the
    method.

    The parameters to \c call are passed on to the remote function via D-Bus as input
    arguments. Output arguments are returned in the QDBusMessage reply. If the reply is an error
    reply, lastError() will also be set to the contents of the error message.

    This function is implemented by actually 9 different function overloads called \c call, so you
    can pass up to 8 parameters to your function call, which can be of any type accepted by QtDBus
    (see the \l {qdbustypesystem.html}{QtDBus type system} page for information on what types are
    accepted and how to extend it).

    It can be used the following way:

    \code
      QString value = retrieveValue();
      QDBusMessage reply;

      QDBusReply<int> api = interface->call(QLatin1String("GetAPIVersion"));
      if (api >= 14)
        reply = interface->call(QLatin1String("ProcessWorkUnicode"), value);
      else
        reply = interface->call(QLatin1String("ProcessWork"), QLatin1String("UTF-8"), value.toUtf8());
    \endcode

    This example illustrates function calling with 0, 1 and 2 parameters and illustrates different
    parameter types passed in each (the first call to \c "ProcessWorkUnicode" will contain one
    Unicode string, the second call to \c "ProcessWork" will contain one string and one byte array).
*/

/*!
    \fn QDBusMessage QDBusAbstractInterface::call(CallMode mode, const QString &method)
    \overload

    Calls the method \a method on this interface and passes the
    parameters to this function to the method. If \a mode is \c
    NoWaitForReply, then this function will return immediately after
    placing the call, without waiting for a reply from the remote
    method. Otherwise, \a mode indicates whether this function should
    activate the Qt Event Loop while waiting for the reply to arrive.
         
    If this function reenters the Qt event loop in order to wait for the
    reply, it will exclude user input. During the wait, it may deliver
    signals and other method calls to your application. Therefore, it
    must be prepared to handle a reentrancy whenever a call is placed
    with call().
*/
