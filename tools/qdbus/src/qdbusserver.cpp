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

QDBusServer::QDBusServer(const QString &addr, QObject *p)
    : QObject(p)
{
    d = new QDBusConnectionPrivate(this);

    if (addr.isEmpty())
        return;

    // server = dbus_server_listen( "unix:tmpdir=/tmp",  &error );
    d->setServer(dbus_server_listen(addr.toUtf8().constData(), &d->error));
}

bool QDBusServer::isConnected() const
{
    return d->server && dbus_server_get_is_connected(d->server);
}

QDBusError QDBusServer::lastError() const
{
    return d->lastError;
}

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

