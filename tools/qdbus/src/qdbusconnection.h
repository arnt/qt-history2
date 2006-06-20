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

#ifndef QDBUSCONNECTION_H
#define QDBUSCONNECTION_H

#include "qdbusmacros.h"
#include <QtCore/qstring.h>

class QDBusAbstractInterfacePrivate;
class QDBusInterface;
class QDBusError;
class QDBusMessage;
class QDBusBusService;
class QObject;

class QDBusConnectionPrivate;
class QDBUS_EXPORT QDBusConnection
{
public:
    enum BusType { SessionBus, SystemBus, ActivationBus };
    enum WaitMode { UseEventLoop, NoUseEventLoop };
    enum RegisterOption {
        ExportAdaptors = 0x01,

        ExportSlots = 0x10,
        ExportSignals = 0x20,
        ExportProperties = 0x40,
        ExportContents = 0xf0,

        ExportAllSlots = 0x110,
        ExportAllSignals = 0x220,
        ExportAllProperties = 0x440,
        ExportAllContents = 0xff0,

        ExportChildObjects = 0x1000
    };
    enum UnregisterMode {
        UnregisterNode,
        UnregisterTree
    };

    Q_DECLARE_FLAGS(RegisterOptions, RegisterOption)

    QDBusConnection(const QString &name);
    QDBusConnection(const QDBusConnection &other);
    ~QDBusConnection();

    QDBusConnection &operator=(const QDBusConnection &other);

    bool isConnected() const;
    QString baseService() const;
    QDBusError lastError() const;

    bool send(const QDBusMessage &message) const;
    QDBusMessage sendWithReply(const QDBusMessage &message, WaitMode mode = NoUseEventLoop) const;
    int sendWithReplyAsync(const QDBusMessage &message, QObject *receiver,
                           const char *slot) const;

    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, QObject *receiver, const char *slot);
    bool connect(const QString &service, const QString &path, const QString &interface,
                 const QString &name, const QString& signature,
                 QObject *receiver, const char *slot);

    bool registerObject(const QString &path, QObject *object,
                        RegisterOptions options = ExportAdaptors);
    void unregisterObject(const QString &path, UnregisterMode mode = UnregisterNode);

    template<class Interface>
    inline Interface *findInterface(const QString &service, const QString &path);
    QDBusInterface *findInterface(const QString& service, const QString& path,
                                  const QString& interface = QString());

    QDBusBusService *busService() const;

    static QDBusConnection addConnection(BusType type, const QString &name);
    static QDBusConnection addConnection(const QString &address, const QString &name);
    static void closeConnection(const QString &name);

private:
    QDBusAbstractInterfacePrivate *findInterface_helper(const QString &, const QString &,
                                                        const char*);
    QDBusConnectionPrivate *d;
};

template<class Interface>
inline Interface *QDBusConnection::findInterface(const QString &service, const QString &path)
{
    register QDBusAbstractInterfacePrivate *d_ptr;
    d_ptr = findInterface_helper(service, path, Interface::staticInterfaceName());
    if (d_ptr)
        return new Interface(d_ptr);
    return 0;
}

namespace QDBus {
    QDBUS_EXPORT QDBusConnection &sessionBus();
    QDBUS_EXPORT QDBusConnection &systemBus();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QDBusConnection::RegisterOptions)
#endif
