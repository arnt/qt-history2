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

#ifndef QDBUSABSTRACTADAPTORPRIVATE_H
#define QDBUSABSTRACTADAPTORPRIVATE_H

#include <qdbusabstractadaptor.h>

#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <QtCore/qhash.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include "private/qobject_p.h"

#define QCLASSINFO_DBUS_INTERFACE       "D-Bus Interface"
#define QCLASSINFO_DBUS_INTROSPECTION   "D-Bus Introspection"

class QDBusAbstractAdaptor;
class QDBusAdaptorConnector;
class QDBusAdaptorManager;
class QDBusConnectionPrivate;

class QDBusAbstractAdaptorPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QDBusAbstractAdaptor)
public:
    QDBusAbstractAdaptorPrivate() : autoRelaySignals(false) {}
    QString xml;
    bool autoRelaySignals;

    static QString retrieveIntrospectionXml(QDBusAbstractAdaptor *adaptor);
    static void saveIntrospectionXml(QDBusAbstractAdaptor *adaptor, const QString &xml);
};

class QDBusAdaptorConnector: public QObject
{
public: // Q_OBJECT
    static const QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **);

public: // typedefs
    struct AdaptorData
    {
        const char *interface;
        QDBusAbstractAdaptor *adaptor;

        inline bool operator<(const AdaptorData &other) const
        { return QByteArray(interface) < other.interface; }
        inline bool operator<(const QString &other) const
        { return QLatin1String(interface) < other; }
        inline bool operator<(const QByteArray &other) const
        { return interface < other; }
    };
    typedef QVector<AdaptorData> AdaptorMap;

public: // methods
    explicit QDBusAdaptorConnector(QObject *parent);
    ~QDBusAdaptorConnector();

    void addAdaptor(QDBusAbstractAdaptor *adaptor);
    void connectAllSignals(QObject *object);
    void disconnectAllSignals(QObject *object);
    void relay(QObject *sender, int id, void **);

//public slots:
    void relaySlot(void **);
    void polish();

protected:
//signals:
    void relaySignal(QObject *obj, const QMetaObject *metaObject, int sid, const QVariantList &args);

public: // member variables
    AdaptorMap adaptors;
    bool waitingForPolish : 1;
};

extern QDBusAdaptorConnector *qDBusFindAdaptorConnector(QObject *object);
extern QDBusAdaptorConnector *qDBusCreateAdaptorConnector(QObject *object);

#endif // QDBUSABSTRACTADAPTORPRIVATE_H
