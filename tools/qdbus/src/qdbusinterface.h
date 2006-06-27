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

#include <QtDBus/qdbusabstractinterface.h>

QT_BEGIN_HEADER

namespace QDBus { QDBUS_EXPORT QDBusConnection sessionBus(); }

class QDBusInterfacePrivate;
class QDBUS_EXPORT QDBusInterface: public QDBusAbstractInterface
{
    friend class QDBusConnection;
private:
    QDBusInterface(QDBusInterfacePrivate *p);
    
public:
    QDBusInterface(const QString &service, const QString &path, const QString &interface = QString(),
                   const QDBusConnection &connection = QDBus::sessionBus(),
                   QObject *parent = 0);
    ~QDBusInterface();
    
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **);

private:
    Q_DECLARE_PRIVATE(QDBusInterface)
};

QT_END_HEADER

#endif
