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

#ifndef QDBUSINTERFACE_H
#define QDBUSINTERFACE_H

#include "qdbusabstractinterface.h"

class QDBusInterfacePrivate;
class QDBUS_EXPORT QDBusInterface: public QDBusAbstractInterface
{
    friend class QDBusConnection;
private:
    QDBusInterface(QDBusInterfacePrivate *p);
    
public:
    ~QDBusInterface();
    
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **);

private:
    Q_DECLARE_PRIVATE(QDBusInterface)
    Q_DISABLE_COPY(QDBusInterface)
};

struct QDBUS_EXPORT QDBusInterfacePtr
{
    QDBusInterfacePtr(QDBusInterface *iface) : d(iface) { }
    QDBusInterfacePtr(QDBusConnection &conn, const QString &service, const QString &path,
             const QString &interface = QString());
    QDBusInterfacePtr(const QString &service, const QString &path, const QString &interface = QString());
    ~QDBusInterfacePtr() { delete d; }

    QDBusInterface *interface() { return d; }
    QDBusInterface *operator->() { return d; }
private:
    QDBusInterface *const d;
    Q_DISABLE_COPY(QDBusInterfacePtr)
};

#endif
