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

#include "qdbusinterface.h"

#include <dbus/dbus.h>
#include <QtCore/qpointer.h>
#include <QtCore/qstringlist.h>

#include "qdbusmetatype_p.h"
#include "qdbusinterface_p.h"
#include "qdbusconnection_p.h"

static void assign(void *ptr, int id, const QVariant &value)
{
    Q_ASSERT(value.userType() == QDBusMetaTypeId::argument || value.userType() == id);

    switch (id)
    {
    case QVariant::Bool:
        *reinterpret_cast<bool *>(ptr) = qvariant_cast<bool>(value);
        return;

    case QMetaType::UChar:
        *reinterpret_cast<uchar *>(ptr) = qvariant_cast<uchar>(value);
        return;

    case QMetaType::Short:
        *reinterpret_cast<short *>(ptr) = qvariant_cast<short>(value);
        return;

    case QMetaType::UShort:
        *reinterpret_cast<ushort *>(ptr) = qvariant_cast<ushort>(value);
        return;

    case QVariant::Int:
        *reinterpret_cast<int *>(ptr) = qvariant_cast<int>(value);
        return;

    case QVariant::UInt:
        *reinterpret_cast<uint *>(ptr) = qvariant_cast<uint>(value);
        return;

    case QVariant::LongLong:
        *reinterpret_cast<qlonglong *>(ptr) = qvariant_cast<qlonglong>(value);
        return;

    case QVariant::ULongLong:
        *reinterpret_cast<qulonglong *>(ptr) = qvariant_cast<qulonglong>(value);
        return;

    case QVariant::Double:
        *reinterpret_cast<double *>(ptr) = qvariant_cast<double>(value);
        return;

    case QVariant::String:
        *reinterpret_cast<QString *>(ptr) = qvariant_cast<QString>(value);
        return;

    case QVariant::ByteArray:
        *reinterpret_cast<QByteArray *>(ptr) = qvariant_cast<QByteArray>(value);
        return;

    case QVariant::StringList:
        *reinterpret_cast<QStringList *>(ptr) = qvariant_cast<QStringList>(value);

    default:
        if (id == QDBusMetaTypeId::variant)
            *reinterpret_cast<QDBusVariant *>(ptr) = qvariant_cast<QDBusVariant>(value);
        else if (id == QDBusMetaTypeId::objectpath)
            *reinterpret_cast<QDBusObjectPath *>(ptr) = qvariant_cast<QDBusObjectPath>(value);
        else if (id == QDBusMetaTypeId::signature)
            *reinterpret_cast<QDBusSignature *>(ptr) = qvariant_cast<QDBusSignature>(value);
        else if (id == QDBusMetaTypeId::argument)
            QDBusMetaType::demarshall(qvariant_cast<QDBusArgument>(value), id, ptr);
        else
            qFatal("QDBusInterface::property(): got unknown type %d (%s) when demarshalling "
                   "remote property", id, QVariant::typeToName(QVariant::Type(id)));
    }
}

/*!
    \class QDBusInterface
    \brief Proxy class for interfaces on remote objects.

    QDBusInterface is a generic accessor class that is used to place calls to remote objects,
    connect to signals exported by remote objects and get/set the value of remote properties. This
    class is useful for dynamic access to remote objects: that is, when you do not have a generated
    code that represents the remote interface.

    Calls are usually placed by using the call() function, which constructs the message, sends it
    over the bus, waits for the reply and decodes the reply. Signals are connected to by using the
    normal QObject::connect() function. Finally, properties are accessed using the
    QObject::property() and QObject::setProperty() functions. 
*/

QDBusInterface::QDBusInterface(QDBusInterfacePrivate *p)
    : QDBusAbstractInterface(p)
{
}

/*!
    Destroy the object interface and frees up any resource used.
*/
QDBusInterface::~QDBusInterface()
{
    // resources are freed in QDBusInterfacePrivate::~QDBusInterfacePrivate()
}

/*!
    \internal
    Overrides QObject::metaObject to return our own copy.
*/
const QMetaObject *QDBusInterface::metaObject() const
{
    return d_func()->isValid ? d_func()->metaObject : &QDBusAbstractInterface::staticMetaObject;
}

/*!
    \internal
    Override QObject::qt_metacast to catch the interface name too.
*/
void *QDBusInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, "QDBusInterface"))
        return static_cast<void*>(const_cast<QDBusInterface*>(this));
    if (d_func()->interface.toLatin1() == _clname)
        return static_cast<void*>(const_cast<QDBusInterface*>(this));
    return QDBusAbstractInterface::qt_metacast(_clname);
}

/*!
    \internal
    Dispatch the call through the private.
*/
int QDBusInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractInterface::qt_metacall(_c, _id, _a);
    if (_id < 0 || !d_func()->isValid)
        return _id;
    return d_func()->metacall(_c, _id, _a);
}

int QDBusInterfacePrivate::metacall(QMetaObject::Call c, int id, void **argv)
{
    Q_Q(QDBusInterface);
    
    if (c == QMetaObject::InvokeMetaMethod) {
        int offset = metaObject->methodOffset();
        QMetaMethod mm = metaObject->method(id + offset);
        
        if (mm.methodType() == QMetaMethod::Signal) {
            // signal relay from D-Bus world to Qt world
            QMetaObject::activate(q, metaObject, id, argv);
            
        } else if (mm.methodType() == QMetaMethod::Slot) {
            // method call relay from Qt world to D-Bus world
            // get D-Bus equivalent signature
            QString methodName = QLatin1String(metaObject->dbusNameForMethod(id));
            const int *inputTypes = metaObject->inputTypesForMethod(id);
            int inputTypesCount = *inputTypes;

            // we will assume that the input arguments were passed correctly
            QVariantList args;
            for (int i = 1; i <= inputTypesCount; ++i)
                args << QVariant(inputTypes[i], argv[i]);

            // make the call
            QPointer<QDBusInterface> qq = q;
            QDBusMessage reply = q->callWithArgs(methodName, args);
            args.clear();

            // we ignore return values

            // access to "this" or to "q" below this point must check for "qq"
            // we may have been deleted!

            if (!qq.isNull())
                lastError = reply;

            // done
            return -1;
        }
    } else if (c == QMetaObject::ReadProperty) {
        // Qt doesn't support non-readable properties
        // we have to re-check
        QMetaProperty mp = metaObject->property(id + metaObject->propertyOffset());
        if (!mp.isReadable())
            return -1;          // don't read

        QVariant value = property(mp);
        if (value.type() == QVariant::Invalid)
            // an error occurred -- property() already set lastError
            return -1;
        else if (mp.type() == QVariant::LastType)
            // QVariant is special in this context
            *reinterpret_cast<QVariant *>(argv[0]) = value;
        else
            assign(argv[0], metaObject->propertyMetaType(id), value);

        return -1; // handled
    } else if (c == QMetaObject::WriteProperty) {
        // QMetaProperty::write has already checked that we're writable
        // it has also checked that the type is right
        QVariant value(metaObject->propertyMetaType(id), argv[0]);
        QMetaProperty mp = metaObject->property(id + metaObject->propertyOffset());

        setProperty(mp, value);
        return -1;
    }
    return id;
}

/*!
    \class QDBusInterfacePtr
    \brief The QDBusInterfacePtr is a container for a QDBusInterface object.

    QDBusInterface objects are always created on the heap and must,
    therefore, be disposed of when no longer necessary. However, for a
    simple call to a remote function, QDBusInterfacePtr provides a
    convenient way of creating an object on the stack, which will be
    automatically disposed of when freed.

    The following code snippet demonstrates how to perform a
    mathematical operation of \tt{"2 + 2"} in a remote application
    called \c com.example.Calculator, accessed via the session bus.

    \code
        QDBusInterfacePtr remoteApp( "com.example.Calculator", "/Calculator/Operations",
                                     "org.mathematics.RPNCalculator");
        remoteApp->call( "PushOperand", 2 );
        remoteApp->call( "PushOperand", 2 );
        remoteApp->call( "ExecuteOperation", "+" );
        QDBusReply<int> reply = remoteApp->call( "PopOperand" );

        if ( reply.isSuccess() )
            printf( "%d", reply.value() );          // prints 4
    \endcode

    Since QDBusInterfacePtr creates an object each time it is instantiated
    and destroys it when it goes out of scope, it is not suitable for
    use in tight loops. Instead, prefer to create QDBusInterface
    objects and cache them in your application.
*/

/*!
    Creates a QDBusInterfacePtr object that references the remote
    interface \a iface on the object \a path on application \a
    service. The connection \a conn is used to access the bus.
*/
QDBusInterfacePtr::QDBusInterfacePtr(QDBusConnection &conn, const QString &service, const QString &path,
                   const QString &iface)
    : d(conn.findInterface(service, path, iface))
{
}

/*!
    \overload

    Creates a QDBusInterfacePtr object that references the remote
    interface \a iface on the object \a path on application \a
    service. This constructor uses the connection object returned by
    QDBus::sessionBus() to access the bus.
*/
QDBusInterfacePtr::QDBusInterfacePtr(const QString &service, const QString &path, const QString &iface)
    : d(QDBus::sessionBus().findInterface(service, path, iface))
{
}

