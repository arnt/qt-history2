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

#ifndef QDBUSBUS_H
#define QDBUSBUS_H

#include <QtCore/qstringlist.h>

#include <QtDBus/qdbusabstractinterface.h>
#include <QtDBus/qdbusreply.h>

QT_BEGIN_HEADER


class QDBusConnection;
class QString;
class QByteArray;

/*
 * Proxy class for interface org.freedesktop.DBus
 */
class QDBUS_EXPORT QDBusBusService: public QDBusAbstractInterface
{
    Q_OBJECT
    friend class QDBusConnection;
    static inline const char *staticInterfaceName();

    explicit QDBusBusService(QDBusAbstractInterfacePrivate *p);

    ~QDBusBusService();

public:
    // taken out of http://dbus.freedesktop.org/doc/dbus-specification.html
    // update if the standard updates
    enum RequestNameOption {
        QueueName = 0x0,
        AllowReplacingName = 0x1,
        ReplaceExistingName = 0x2,
        DoNotQueueName = 0x4
    };
    Q_DECLARE_FLAGS(RequestNameOptions, RequestNameOption)

    enum RequestNameReply {
        PrimaryOwnerReply = 1,
        InQueueReply = 2,
        NameExistsReply = 3,
        AlreadyOwnerReply = 4
    };

    enum ReleaseNameReply {
        NameReleasedReply = 1,
        NameNonExistentReply = 2,
        NotOwnerReply = 3
    };

    enum StartServiceReply {
        Success = 1,
        AlreadyRunning = 2
    };    

#ifndef Q_QDOC
    // D-Bus names
public: // METHODS
    QDBusReply<QString> Hello();
    QDBusReply<void> ReloadConfig();

    QDBusReply<QStringList> ListNames();
        
    QDBusReply<bool> NameHasOwner(const QString &service);
    QDBusReply<QString> GetNameOwner(const QString &name);
    QDBusReply<ReleaseNameReply> ReleaseName(const QString &service);    
    QDBusReply<RequestNameReply> RequestName(const QString &service, RequestNameOptions flags);
    QDBusReply<QStringList> ListQueuedOwners(const QString &service);

    QDBusReply<void> AddMatch(const QString &rule);
    QDBusReply<void> RemoveMatch(const QString &rule);

    QDBusReply<QByteArray> GetConnectionSELinuxSecurityContext(const QString &service);
    QDBusReply<uint> GetConnectionUnixProcessID(const QString &service);
    QDBusReply<uint> GetConnectionUnixUser(const QString &service);

    QDBusReply<StartServiceReply> StartServiceByName(const QString &name, uint flags);

Q_SIGNALS: // SIGNALS
    void NameAcquired(const QString &service);
    void NameLost(const QString &service);
    void NameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
#endif

    // Qt-style naming    
public Q_SLOTS:
    QDBusReply<QString> hello()
    { return Hello(); }
    QDBusReply<void> reloadConfig()
    { return ReloadConfig(); }

    QDBusReply<QStringList> listNames()
    { return ListNames(); }
        
    QDBusReply<bool> nameHasOwner(const QString &serviceName)
    { return NameHasOwner(serviceName); }
    QDBusReply<QString> nameOwner(const QString &name)
    { return GetNameOwner(name); }
    QDBusReply<ReleaseNameReply> releaseName(const QString &serviceName)
    { return ReleaseName(serviceName); }
    QDBusReply<RequestNameReply> requestName(const QString &serviceName, RequestNameOptions flags = QueueName)
    { return RequestName(serviceName, flags); }
    QDBusReply<QStringList> listQueuedOwners(const QString &serviceName)
    { return ListQueuedOwners(serviceName); }

    QDBusReply<void> addMatch(const QString &rule)
    { return AddMatch(rule); }
    QDBusReply<void> removeMatch(const QString &rule)
    { return RemoveMatch(rule); }

    QDBusReply<QByteArray> connectionSELinuxSecurityContext(const QString &serviceName)
    { return GetConnectionSELinuxSecurityContext(serviceName); }
    QDBusReply<uint> connectionUnixProcessID(const QString &serviceName)
    { return GetConnectionUnixProcessID(serviceName); }
    QDBusReply<uint> connectionUnixUser(const QString &serviceName)
    { return GetConnectionUnixUser(serviceName); }

    QDBusReply<StartServiceReply> startServiceByName(const QString &name, uint flags)
    { return StartServiceByName(name, flags); }

Q_SIGNALS:
    void nameAcquired(const QString &service);
    void nameLost(const QString &service);
    void nameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDBusBusService::RequestNameOptions)
template<> inline int qt_variant_metatype_id(QDBusBusService::StartServiceReply *)
{ return QVariant::Int; }
template<> inline int qt_variant_metatype_id(QDBusBusService::ReleaseNameReply *)
{ return QVariant::UInt; }
template<> inline int qt_variant_metatype_id(QDBusBusService::RequestNameReply *)
{ return QVariant::UInt; }

QT_END_HEADER

#endif
