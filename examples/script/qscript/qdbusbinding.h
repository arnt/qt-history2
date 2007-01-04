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

#ifndef QDBUSBINDING_H
#define QDBUSBINDING_H

#include <QtDBus>
#include <QtScript/qscriptable.h>
#include <QtScript/qscriptengine.h>

class QDBusConnectionPrototype : public QObject,
                                 public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QScriptValue sessionBus READ sessionBus)
    Q_PROPERTY(QScriptValue systemBus READ systemBus)

public:
    QDBusConnectionPrototype(QScriptEngine *engine);

    QScriptValue sessionBus() const;
    QScriptValue systemBus() const;

public Q_SLOTS:
    QObject *qscript_call(const QString &name);

    void disconnectFromBus(const QString &name);
};

class QScriptDBusConnection : public QObject,
                              public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QString baseService READ baseService)
    Q_PROPERTY(bool isConnected READ isConnected)
    Q_PROPERTY(QScriptValue interface READ interface)
public:
    QScriptDBusConnection(const QDBusConnection &conn, QObject *parent);

    inline QString baseService() const { return connection.baseService(); }
    inline bool isConnected() const { return connection.isConnected(); }
    QScriptValue interface() const;

    inline QDBusConnection dbusConnection() const { return connection; }

public Q_SLOTS:
    inline bool send(const QDBusMessage &message) const
    { return connection.send(message); }
    inline QDBusMessage call(const QDBusMessage &message, int callMode = QDBus::Block, int timeout = -1) const
    { return connection.call(message, QDBus::CallMode(callMode), timeout); }

    inline bool registerService(const QString &serviceName)
    { return connection.registerService(serviceName); }
    inline bool unregisterService(const QString &serviceName)
    { return connection.unregisterService(serviceName); }

    inline QDBusError lastError() const
    { return connection.lastError(); }

    inline void unregisterObject(const QString &path, QDBusConnection::UnregisterMode mode = QDBusConnection::UnregisterNode)
    { return connection.unregisterObject(path, mode); }
    inline QObject *objectRegisteredAt(const QString &path) const
    { return connection.objectRegisteredAt(path); }

#if 0
    bool callWithCallback(const QDBusMessage &message, QObject *receiver,
                          const char *slot, int timeout = -1) const;

    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, QObject *receiver, const char *slot);
    bool disconnect(const QString &service, const QString &path, const QString &interface,
                    const QString &name, QObject *receiver, const char *slot);

    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, const QString& signature,
                 QObject *receiver, const char *slot);
    bool disconnect(const QString &service, const QString &path, const QString &interface,
                    const QString &name, const QString& signature,
                    QObject *receiver, const char *slot);

    bool registerObject(const QString &path, QObject *object,
                        RegisterOptions options = ExportAdaptors);

#endif

private:
    QDBusConnection connection;
};

Q_DECLARE_METATYPE(QScriptDBusConnection*)

class QScriptDBusInterfacePrototype : public QObject,
                                      public QScriptable
{
    Q_OBJECT
public:
    QScriptDBusInterfacePrototype(QScriptEngine *engine);

public Q_SLOTS:
    QScriptValue qscript_call(const QString &service, const QString &path, const QString &interface = QString(),
                              const QScriptValue &conn = QScriptValue());
};

Q_DECLARE_METATYPE(QDBusMessage)

class QScriptDBusMessagePrototype : public QObject, public QScriptable
{
    Q_OBJECT
    Q_ENUMS(MessageType)
public:
    enum MessageType {
        InvalidMessage = QDBusMessage::InvalidMessage,
        MethodCallMessage = QDBusMessage::MethodCallMessage,
        ReplyMessage = QDBusMessage::ReplyMessage,
        ErrorMessage = QDBusMessage::ErrorMessage,
        SignalMessage = QDBusMessage::SignalMessage
    };

    QScriptDBusMessagePrototype(QScriptEngine *engine);

    QScriptValue protoType() const { return proto; }

public Q_SLOTS:
    QDBusMessage createSignal(const QString &path, const QString &interface, const QString &name);
    QDBusMessage createMethodCall(const QString &destination, const QString &path, const QString &interface, const QString &method);
    QDBusMessage createError(const QString &name, const QString &msg);

public:
    static QScriptValue createReply(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue createErrorReply(QScriptContext *context, QScriptEngine *engine);

private:
    QScriptValue proto;
};

void registerDBusBindings(QScriptEngine *engine);

#endif // QDBUSBINDING_H
