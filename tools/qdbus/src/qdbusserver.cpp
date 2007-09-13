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

#include "qdbusserver.h"
#include "qdbusconnection_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDBusServer
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusServer class provides peer-to-peer communication
    between processes on the same computer.
*/

/*!
    Constructs a QDBusServer with the given \a address, and the given
    \a parent.
*/
QDBusServer::QDBusServer(const QString &address, QObject *parent)
    : QObject(parent)
{
    d = new QDBusConnectionPrivate(this);

    if (address.isEmpty())
        return;

    QObject::connect(d, SIGNAL(newServerConnection(const QDBusConnection &)),
                     this, SIGNAL(newConnection(const QDBusConnection &)));

    // server = dbus_server_listen( "unix:tmpdir=/tmp", &error);
    QDBusErrorInternal error;
    d->setServer(dbus_server_listen(address.toUtf8().constData(), error), error);
}

/*!
    Returns true if this QDBusServer object is connected.

    If it isn't connected, you need to call the constructor again.
*/
bool QDBusServer::isConnected() const
{
    return d->server && dbus_server_get_is_connected(d->server);
}

/*!
    Returns the last error that happened in this server.

    This function is provided for low-level code.
*/
QDBusError QDBusServer::lastError() const
{
    return d->lastError;
}

/*!
    Returns the address this server is assosiated with.
*/
QString QDBusServer::address() const
{
    QString addr;
    if (d->server) {
        char *c = dbus_server_get_address(d->server);
        addr = QString::fromUtf8(c);
        dbus_free(c);
    }

    return addr;
}
/*!
  \fn void QDBusServer::newConnection(const QDBusConnection &connection)

  This signal is currently not used, but if and when it is

QT_END_NAMESPACE
  used, \a connection will be the new connection. 
 */
