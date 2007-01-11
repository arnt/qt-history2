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

#include "qdbusbinding.h"
#include <QDebug>
#include <QMetaMethod>

static QScriptValue setupDBusInterface(QScriptEngine *engine, QDBusAbstractInterface *iface);

static QScriptValue do_dbus_call(QScriptContext *context, QScriptEngine *engine)
{
    int firstArgument = 0;
    QString functionName = context->callee().property("functionName").toString();
    if (functionName.isEmpty()) {
        functionName = context->argument(0).toString();
        ++firstArgument;
    }

    QScriptValue thisObject = context->thisObject();
    QDBusAbstractInterface *iface = qobject_cast<QDBusAbstractInterface *>(thisObject.toQObject());
    if (!iface)
        return QScriptValue();

    QDBusMessage msg = QDBusMessage::createMethodCall(iface->service(),
                                                      iface->path(),
                                                      iface->interface(),
                                                      functionName);

    QList<QVariant> args;
    for (int i = firstArgument; i < context->argumentCount(); ++i) {
        args.append(context->argument(i).toVariant());
    }
    msg.setArguments(args);

    msg = iface->connection().call(msg);

    QScriptValue returnValue = engine->nullScriptValue();
    args = msg.arguments();
    if (args.count() != 1)
        return returnValue;

    QVariant variant = args.first();
    if (variant.type() == QVariant::UserType
        && variant.userType() == qMetaTypeId<QDBusObjectPath>()) {
        QDBusObjectPath path = qvariant_cast<QDBusObjectPath>(variant);

        QDBusInterface *returnedIface = new QDBusInterface(iface->service(),
                                                           path.path(),
                                                           /*interface*/QString(),
                                                           iface->connection(),
                                                           engine);
        returnValue = setupDBusInterface(engine, returnedIface);
    } else {
        returnValue = engine->scriptValueFromVariant(variant);
    }

    return returnValue;
}

static QScriptValue setupDBusInterface(QScriptEngine *engine, QDBusAbstractInterface *iface)
{
    QScriptValue v = engine->scriptValueFromQObject(iface);

    if (!qobject_cast<QDBusConnectionInterface *>(iface)) {
        const QMetaObject *mo = iface->metaObject();
        for (int i = 0; i < mo->methodCount(); ++i) {
            const QMetaMethod method = mo->method(i);
            const QByteArray signature = method.signature();
            //qDebug() << "signature" << signature;
            int parenIndex = signature.indexOf('(');
            if (parenIndex == -1)
                continue;
            const QByteArray name = signature.left(parenIndex);
            if (name.isEmpty())
                continue;

            // don't try to override properties
            if (mo->indexOfProperty(name) != -1)
                continue;

            QScriptValue callWrapper = engine->scriptValue(do_dbus_call);
            callWrapper.setProperty("functionName", engine->scriptValue(QString::fromAscii(name)));
            v.setProperty(QString(name), callWrapper);
        }
    }

    v.setProperty("service", engine->scriptValue(iface->service()), QScriptValue::ReadOnly);
    v.setProperty("path", engine->scriptValue(iface->path()), QScriptValue::ReadOnly);
    v.setProperty("interface", engine->scriptValue(iface->interface()), QScriptValue::ReadOnly);
    v.setProperty("isValid", engine->scriptValue(iface->isValid()), QScriptValue::ReadOnly);
    v.setProperty("connection", engine->scriptValueFromQObject(new QScriptDBusConnection(iface->connection(), engine)), QScriptValue::ReadOnly);

    return v;
}

QDBusConnectionPrototype::QDBusConnectionPrototype(QScriptEngine *engine)
    : QObject(engine)
{
    QScriptValue ctorValue = engine->scriptValueFromQObject(this);
    QScriptValue proto = engine->scriptValue(&QDBusConnection::staticMetaObject, ctorValue);

    engine->globalObject().setProperty("QDBusConnection", proto);
}

QScriptValue QDBusConnectionPrototype::sessionBus() const
{
    return engine()->scriptValueFromQObject(new QScriptDBusConnection(QDBusConnection::sessionBus(), engine()));
}

QScriptValue QDBusConnectionPrototype::systemBus() const
{
    return engine()->scriptValueFromQObject(new QScriptDBusConnection(QDBusConnection::systemBus(), engine()));
}

QObject *QDBusConnectionPrototype::qscript_call(const QString &name)
{
    return new QScriptDBusConnection(QDBusConnection(name), this);
}

void QDBusConnectionPrototype::disconnectFromBus(const QString &name)
{
    QDBusConnection::disconnectFromBus(name);
}

QScriptDBusConnection::QScriptDBusConnection(const QDBusConnection &conn, QObject *parent)
    : QObject(parent), connection(conn)
{
}

QScriptValue QScriptDBusConnection::interface() const
{
    QDBusConnectionInterface *iface = connection.interface();
    return setupDBusInterface(engine(), iface);
}

QScriptDBusInterfacePrototype::QScriptDBusInterfacePrototype(QScriptEngine *engine)
{
    QScriptValue ctorValue = engine->scriptValueFromQObject(this);
    QScriptValue klass = engine->scriptValue(metaObject(), ctorValue);
    engine->globalObject().setProperty("QDBusInterface", klass);
}

QScriptValue QScriptDBusInterfacePrototype::qscript_call(const QString &service, const QString &path, const QString &interface,
                                                         const QScriptValue &conn)
{
    QDBusConnection connection = QDBusConnection::sessionBus();

    QScriptDBusConnection *connWrapper = qobject_cast<QScriptDBusConnection *>(conn.toQObject());
    if (connWrapper)
        connection = connWrapper->dbusConnection();

    return setupDBusInterface(engine(), new QDBusInterface(service, path, interface, connection, engine()));
}

QScriptDBusMessagePrototype::QScriptDBusMessagePrototype(QScriptEngine *engine)
    : QObject(engine)
{
    proto = engine->scriptValue(metaObject(), engine->scriptValueFromQObject(this));

    proto.setProperty("createReply", engine->scriptValue(createReply));
    proto.setProperty("createErrorReply", engine->scriptValue(createErrorReply));

    engine->globalObject().setProperty("QDBusMessage", proto);
    engine->setDefaultPrototype(qMetaTypeId<QDBusMessage>(), proto);
}

QDBusMessage QScriptDBusMessagePrototype::createSignal(const QString &path, const QString &interface, const QString &name)
{
    return QDBusMessage::createSignal(path, interface, name);
}

QDBusMessage QScriptDBusMessagePrototype::createMethodCall(const QString &destination, const QString &path, const QString &interface, const QString &method)
{
    return QDBusMessage::createMethodCall(destination, path, interface, method);
}

QDBusMessage QScriptDBusMessagePrototype::createError(const QString &name, const QString &msg)
{
    return QDBusMessage::createError(name, msg);
}

static QScriptValue messageToScriptValue(QScriptEngine *engine, const QDBusMessage &message)
{
    QScriptValue v = engine->scriptValueFromVariant(QVariant::fromValue(message));
    v.setProperty("service", engine->scriptValue(message.service()), QScriptValue::ReadOnly);
    v.setProperty("path", engine->scriptValue(message.path()), QScriptValue::ReadOnly);
    v.setProperty("interface", engine->scriptValue(message.interface()), QScriptValue::ReadOnly);
    v.setProperty("member", engine->scriptValue(message.member()), QScriptValue::ReadOnly);
    v.setProperty("type", engine->scriptValue(message.type()), QScriptValue::ReadOnly);
    v.setProperty("signature", engine->scriptValue(message.signature()), QScriptValue::ReadOnly);
    v.setProperty("isReplyRequired", engine->scriptValue(message.isReplyRequired()), QScriptValue::ReadOnly);

    v.setProperty("delayedReply", engine->scriptValue(message.isDelayedReply()));
    QScriptValue argValue = engine->newArray();
    const QList<QVariant> args = message.arguments();
    for (int i = 0; i < args.count(); ++i)
        argValue.setProperty(engine->scriptValue(i).toString(),
                             engine->scriptValueFromVariant(args.at(i)));

    v.setProperty("arguments", argValue);

    return v;
}

static void scriptValueToMessage(const QScriptValue &value, QDBusMessage &message)
{
    QScriptEngine *eng = value.engine();

    QVariant v = value.toVariant();
    message = qvariant_cast<QDBusMessage>(v);
    message.setDelayedReply(value.property("delayedReply").toBoolean());

    QList<QVariant> args;
    quint32 len = value.property("length").toUInt32();
    for (quint32 i = 0; i < len; ++i) {
        QScriptValue item = value.property(eng->scriptValue(i).toString());
        args.append(item.toVariant());
    }
    message.setArguments(args);
}

QScriptValue QScriptDBusMessagePrototype::createReply(QScriptContext *context, QScriptEngine *engine)
{
    QDBusMessage msg;
    scriptValueToMessage(context->thisObject(), msg);

    QList<QVariant> args;
    for (int i = 0; i < context->argumentCount(); ++i) {
        QScriptValue value = context->argument(i);
        args.append(value.toVariant());
    }

    return messageToScriptValue(engine, msg.createReply(args));
}

QScriptValue QScriptDBusMessagePrototype::createErrorReply(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2)
        return engine->nullScriptValue();

    QDBusMessage msg;
    scriptValueToMessage(context->thisObject(), msg);

    QString name = context->argument(0).toString();
    QString errMsg = context->argument(1).toString();
    return messageToScriptValue(engine, msg.createErrorReply(name, errMsg));
}

template <typename T>
QScriptValue qDBusReplyToScriptValue(QScriptEngine *eng, const QDBusReply<T> &reply)
{
    return eng->scriptValue(reply.value());
}

template <>
QScriptValue qDBusReplyToScriptValue(QScriptEngine *eng, const QDBusReply<QStringList> &reply)
{
    QScriptValue v = eng->newArray();
    const QStringList &lst = reply.value();
    for (int i = 0; i < lst.count(); ++i)
        v.setProperty(eng->scriptValue(i).toString(), eng->scriptValue(lst.at(i)));
    return v;
}

template <typename T>
void qDBusReplyFromScriptValue(const QScriptValue &, QDBusReply<T> &)
{
    // never called
}

QScriptValue qDBusErrorToScriptValue(QScriptEngine *engine, const QDBusError &error)
{
    QScriptValue v = engine->newObject();
    v.setProperty("type", engine->scriptValue(error.type()), QScriptValue::ReadOnly);
    v.setProperty("name", engine->scriptValue(error.name()), QScriptValue::ReadOnly);
    v.setProperty("message", engine->scriptValue(error.message()), QScriptValue::ReadOnly);
    v.setProperty("isValid", engine->scriptValue(error.isValid()), QScriptValue::ReadOnly);
    return v;
}

void scriptValueToQDBusError(const QScriptValue &value, QDBusError &error)
{
    Q_UNUSED(value)
    Q_UNUSED(error)
    // never called
}

Q_DECLARE_METATYPE(QDBusReply<QString>)
Q_DECLARE_METATYPE(QDBusReply<QStringList>)
Q_DECLARE_METATYPE(QDBusReply<uint>)
Q_DECLARE_METATYPE(QDBusReply<bool>)
Q_DECLARE_METATYPE(QDBusReply<QDBusConnectionInterface::RegisterServiceReply>)
Q_DECLARE_METATYPE(QDBusError)

void registerDBusBindings(QScriptEngine *engine)
{
    qScriptRegisterMetaType<QDBusReply<QString> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<QStringList> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<uint> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<bool> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusReply<QDBusConnectionInterface::RegisterServiceReply> >(engine, qDBusReplyToScriptValue, qDBusReplyFromScriptValue);
    qScriptRegisterMetaType<QDBusMessage>(engine, messageToScriptValue, scriptValueToMessage);
    qScriptRegisterMetaType<QDBusError>(engine, qDBusErrorToScriptValue, scriptValueToQDBusError);

    QScriptValue connIfaceProto = engine->scriptValue(&QDBusConnectionInterface::staticMetaObject, engine->nullScriptValue());
    engine->globalObject().setProperty("QDBusConnectionInterface", connIfaceProto);

    QScriptValue qdbus = engine->newObject();
    qdbus.setProperty("NoBlock", engine->scriptValue(QDBus::NoBlock));
    qdbus.setProperty("Block", engine->scriptValue(QDBus::Block));
    qdbus.setProperty("BlockWithGui", engine->scriptValue(QDBus::BlockWithGui));
    qdbus.setProperty("AutoDetect", engine->scriptValue(QDBus::AutoDetect));
    engine->globalObject().setProperty("QDBus", qdbus);

    (void)new QDBusConnectionPrototype(engine);
    (void)new QScriptDBusInterfacePrototype(engine);
    (void)new QScriptDBusMessagePrototype(engine);
}

