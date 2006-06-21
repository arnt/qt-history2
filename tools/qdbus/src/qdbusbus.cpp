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

#include "qdbusbus.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of interface class QDBusBusService
 */

/*!
    \class QDBusBusService
    \inmodule QtDBus
    \brief Provides access to the D-Bus bus daemon service.

    The D-BUS bus server daemon provides one special interface \c
    org.freedesktop.DBus that allows clients to access certain
    properties of the bus, such as the current list of clients
    connected. The QDBusBusService class provides access to that
    interface.

    The most common uses of this class are to register and unregister
    service names on the bus (see requestName() and releaseName()),
    query about existing names (see listNames(), nameHasOwner() and
    getNameOwner()), and to receive notification that a client has
    registered or de-registered (see the nameOwnerChanged() signal).
*/

/*!
    \enum QDBusBusService::RequestNameOption

    Flags for requesting a name on the bus.

    \value QueueName            Attempts to register the requested name, but do not try to replace
                                it if another application already has it registered. Instead, simply
                                put this application in queue. This is the default.
    \value AllowReplacingName   Allow another application requesting the same name to take the name
                                from this application.
    \value ReplaceExistingName  If another application already has the name and allows replacing,
                                take the name and assign it to us.
    \value DoNotQueueName       Without this flag, if an application requests a name that is already
                                owned and does not allow replacing, it will be queued until the
                                name is given up. If this flag is given, no queueing will be
                                performed and the requestName() call will simply fail.
*/

/*!
    \enum QDBusBusService::RequestNameReply

    The possible return values from requestName():

    \value PrimaryOwnerReply    The caller is now the primary owner of the name.
    \value InQueueReply         The caller is in queue for the name, but does not own it.
    \value NameExistsReply      The name exists and could not be replaced, or the caller did
                                specify DoNotQueueName.
    \value AlreadyOwnerReply    The caller tried to request a name that it already owns.
*/

/*!
    \enum QDBusBusService::ReleaseNameReply

    The possible return values from releaseName():

    \value NameReleasedReply    The caller released his claim on the name.
    \value NameNonExistentReply The caller tried to release a name that did not exist.
    \value NotOwnerReply        The caller tried to release a name that it did not own or was not in
                                queue for.
*/

/*!
    \enum QDBusBusService::StartServiceReply

    The possible return values from startServiceByName():

    \value Success              The service was successfully started.
    \value AlreadyRunning       The service was already running.
*/

/*!
    \internal
*/
const char *QDBusBusService::staticInterfaceName()
{ return "org.freedesktop.DBus"; }


/*!
    \internal
*/
QDBusBusService::QDBusBusService(QDBusAbstractInterfacePrivate *p)
    : QDBusAbstractInterface(p)
{
    connect(this, SIGNAL(NameAcquired(QString)), this, SIGNAL(nameAcquired(QString)));
    connect(this, SIGNAL(NameLost(QString)), this, SIGNAL(nameLost(QString)));
    connect(this, SIGNAL(NameOwnerChanged(QString,QString,QString)),
            this, SIGNAL(nameOwnerChanged(QString,QString,QString)));
}

/*!
    \internal
*/
QDBusBusService::~QDBusBusService()
{
}

/*!
    \fn QDBusBusService::hello()
    \internal
    Sends a "Hello" request to the bus service. You do not want to call this.
*/
QDBusReply<QString> QDBusBusService::Hello()
{
    return call(QLatin1String("Hello"));
}

/*!
    \fn QDBusBusService::nameOwner(const QString &name)
    Returns the unique connection name of the primary owner of the name \a name. If the requested
    name doesn't have an owner, returns a org.freedesktop.DBus.Error.NameHasNoOwner error.
*/
QDBusReply<QString> QDBusBusService::GetNameOwner(const QString &name)
{
    return call(QLatin1String("GetNameOwner.s"), name);
}

/*!
    \fn QDBusBusService::listNames()
    Lists all names currently existing on the bus.
*/
QDBusReply<QStringList> QDBusBusService::ListNames()
{
    return call(QLatin1String("ListNames"));
}

/*!
    \fn QDBusBusService::listQueuedOwners(const QString &serviceName)
    Returns a list of all unique connection names in queue for the service name \a serviceName.
*/
QDBusReply<QStringList> QDBusBusService::ListQueuedOwners(const QString &serviceName)
{
    return call(QLatin1String("ListQueuedOwners.s"), serviceName);
}

/*!
    \fn QDBusBusService::nameHasOwner(const QString &serviceName)
    Returns true if the service name \a serviceName has an owner.
*/
QDBusReply<bool> QDBusBusService::NameHasOwner(const QString &serviceName)
{
    return call(QLatin1String("NameHasOwner.s"), serviceName);
}

/*!
    \fn QDBusBusService::addMatch(const QString &rule)
    Adds the rule \a rule for requesting messages from the bus.

    \sa removeMatch()
*/
QDBusReply<void> QDBusBusService::AddMatch(const QString &rule)
{
    return call(QLatin1String("AddMatch.s"), rule);
}

/*!
    \fn QDBusBusService::removeMatch(const QString &rule)
    Removes the rule \a rule, that had previously been added with addMatch().
*/
QDBusReply<void> QDBusBusService::RemoveMatch(const QString &rule)
{
    return call(QLatin1String("RemoveMatch.s"), rule);
}

/*!
    \fn QDBusBusService::connectionSELinuxSecurityContext(const QString &serviceName)
    Returns the SELinux security context of the process currently holding the bus service \a
    serviceName.
*/
QDBusReply<QByteArray> QDBusBusService::GetConnectionSELinuxSecurityContext(const QString &serviceName)
{
    return call(QLatin1String("GetConnectionSELinuxSecurityContext.s"), serviceName);
}

/*!
    \fn QDBusBusService::connectionUnixProcessID(const QString &serviceName)
    Returns the Unix Process ID (PID) for the process currently holding the bus service \a serviceName.
*/
QDBusReply<uint> QDBusBusService::GetConnectionUnixProcessID(const QString &serviceName)
{
    return call(QLatin1String("GetConnectionUnixProcessID.s"), serviceName);
}

/*!
    \fn QDBusBusService::connectionUnixUser(const QString &serviceName)
    Returns the Unix User ID (UID) for the process currently holding the bus service \a serviceName.
*/
QDBusReply<uint> QDBusBusService::GetConnectionUnixUser(const QString &serviceName)
{
    return call(QLatin1String("GetConnectionUnixUser.s"), serviceName);
}

/*!
    \fn QDBusBusService::reloadConfig()
    Asks the D-Bus server daemon to reload its configuration.
*/
QDBusReply<void> QDBusBusService::ReloadConfig()
{
    return call(QLatin1String("ReloadConfig"));
}

/*!
    \fn QDBusBusService::startServiceByName(const QString &name, uint flags)
    Requests that the bus start the service given by the name \a name.

    The \a flags parameter is currently not used.
*/
QDBusReply<QDBusBusService::StartServiceReply>
QDBusBusService::StartServiceByName(const QString &name, uint flags)
{
    return call(QLatin1String("StartServiceByName.su"), name, flags);
}

/*!
    \fn QDBusBusService::requestName(const QString &serviceName, RequestNameOptions flags)
    Requests the bus service name \a serviceName from the bus. The \a flags parameter specifies how the
    bus server daemon should act when the same name is requested by two different applications.

    \sa releaseName()
*/
QDBusReply<QDBusBusService::RequestNameReply>
QDBusBusService::RequestName(const QString &serviceName, RequestNameOptions flags)
{
    return call(QLatin1String("RequestName.su"), serviceName, uint(int(flags)));
}

/*!
    \fn QDBusBusService::releaseName(const QString &serviceName)
    Releases the claim on the bus service name \a serviceName, that had been previously requested with
    requestName(). If this application had ownership of the name, it will be released for other
    applications to claim. If it only had the name queued, it gives up its position in the queue.
*/
QDBusReply<QDBusBusService::ReleaseNameReply>
QDBusBusService::ReleaseName(const QString &serviceName)
{
    return call(QLatin1String("ReleaseName.s"), serviceName);
}

// signals
/*!
    \fn QDBusBusService::nameAcquired(const QString &serviceName)

    This signal is emitted by the D-Bus bus server when the bus service name (unique connection name
    or well-known service name) given by \a serviceName is acquired by this application.

    Name acquisition happens after the application requested a name using requestName().
*/

/*!
    \fn QDBusBusService::nameLost(const QString &serviceName)

    This signal is emitted by the D-Bus bus server when the application loses ownership of the bus
    service name given by \a serviceName.
*/

/*!
    \fn QDBusBusService::nameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)

    This signal is emitted by the D-Bus bus server whenever a name ownership change happens in the
    bus, including apparition and disparition of names.

    This signal means the application \a oldOwner lost ownership of bus name \a name to application
    \a newOwner. If \a oldOwner is an empty string, it means the name \a name has just been created;
    if \a newOwner is empty, the name \a name has no current owner.
*/

