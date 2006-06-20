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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSINTERFACEPRIVATE_H
#define QDBUSINTERFACEPRIVATE_H

#include "qdbusabstractinterface_p.h"
#include "qdbusmetaobject_p.h"
#include "qdbusinterface.h"

class QDBusInterfacePrivate: public QDBusAbstractInterfacePrivate
{
public:
    Q_DECLARE_PUBLIC(QDBusInterface)

    QDBusMetaObject *metaObject;

    inline QDBusInterfacePrivate(const QDBusConnection &con, QDBusConnectionPrivate *conp,
                                 const QString &serv, const QString &p, const QString &iface,
                                 QDBusMetaObject *mo = 0)
        : QDBusAbstractInterfacePrivate(con, conp, serv, p, iface), metaObject(mo)
    {
    }
    ~QDBusInterfacePrivate()
    {
        if (metaObject && !metaObject->cached)
            delete metaObject;
    }

    int metacall(QMetaObject::Call c, int id, void **argv);
};

#endif
