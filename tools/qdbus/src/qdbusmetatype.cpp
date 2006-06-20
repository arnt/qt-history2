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

#include "qdbusmetatype.h"

#include <string.h>
#include <dbus/dbus.h>

#include <qbytearray.h>
#include <qglobal.h>
#include <qreadwritelock.h>
#include <qvector.h>

#include "qdbusmessage.h"
#include "qdbusutil.h"
#include "qdbusmetatype_p.h"
#include "qdbusargument_p.h"

static QByteArray constructListType(const QByteArray &signature)
{
    QByteArray result;
    int count = 0;
    while (count < signature.length() && signature.at(count) == DBUS_TYPE_ARRAY) {
        result += "QList<";
        ++count;
    }

    QByteArray partialSignature = signature;
    partialSignature.truncate(count);
    int type = QDBusMetaType::signatureToType(partialSignature);
    result += QVariant::typeToName( QVariant::Type(type) );

    while (count--) {
        if (result.endsWith('>'))
            result += ' ';
        result += '>';
    }

    return result;
}

class QDBusCustomTypeInfo
{
public:
    QDBusCustomTypeInfo() : signature(0, '\0'), marshall(0), demarshall(0)
    { }

    QByteArray signature;
    QDBusMetaType::MarshallFunction marshall;
    QDBusMetaType::DemarshallFunction demarshall;
};

template<typename T>
inline static void registerHelper(T * ptr = 0)
{
    void (*mf)(QDBusArgument &, const T *) = qDBusMarshallHelper<T>;
    void (*df)(const QDBusArgument &, T *) = qDBusDemarshallHelper<T>;
    QDBusMetaType::registerMarshallOperators(qt_variant_metatype_id(ptr),
        reinterpret_cast<QDBusMetaType::MarshallFunction>(mf),
        reinterpret_cast<QDBusMetaType::DemarshallFunction>(df));
}

int QDBusMetaTypeId::message;
int QDBusMetaTypeId::argument;
int QDBusMetaTypeId::variant;
int QDBusMetaTypeId::objectpath;
int QDBusMetaTypeId::signature;
void QDBusMetaTypeId::init()
{
    static volatile bool initialized = false;

    // reentrancy is not a problem since everything else is locked on their own
    // set the guard variable at the end
    if (!initialized) {
        // register our types with QtCore
        message = qRegisterMetaType<QDBusMessage>("QDBusMessage");
        argument = qRegisterMetaType<QDBusArgument>("QDBusArgument");
        variant = qRegisterMetaType<QDBusVariant>("QDBusVariant");
        objectpath = qRegisterMetaType<QDBusObjectPath>("QDBusObjectPath");
        signature = qRegisterMetaType<QDBusSignature>("QDBusSignature");

#ifndef QDBUS_NO_SPECIALTYPES
        // and register QtCore's with us
        registerHelper<QDate>();
        registerHelper<QTime>();
        registerHelper<QDateTime>();
        registerHelper<QRect>();
        registerHelper<QRectF>();
        registerHelper<QSize>();
        registerHelper<QSizeF>();
        registerHelper<QPoint>();
        registerHelper<QPointF>();
        registerHelper<QLine>();
        registerHelper<QLineF>();
        registerHelper<QVariantList>();
        registerHelper<QVariantMap>();

        qDBusRegisterMetaType<QList<bool> >("QList<bool>");
        qDBusRegisterMetaType<QList<short> >("QList<short>");
        qDBusRegisterMetaType<QList<ushort> >("QList<ushort>");
        qDBusRegisterMetaType<QList<int> >("QList<int>");
        qDBusRegisterMetaType<QList<uint> >("QList<uint>");
        qDBusRegisterMetaType<QList<qlonglong> >("QList<qlonglong>");
        qDBusRegisterMetaType<QList<qulonglong> >("QList<qulonglong>");
        qDBusRegisterMetaType<QList<double> >("QList<double>");
#endif

        initialized = true;
    }
}

Q_GLOBAL_STATIC(QVector<QDBusCustomTypeInfo>, customTypes)
Q_GLOBAL_STATIC(QReadWriteLock, customTypesLock)

/*!
    \class QDBusMetaType
    \brief Meta-type registration system for the QtDBus module.

    The QDBusMetaType class allows you to register class types for
    marshalling and demarshalling over D-BUS. D-BUS supports a very
    limited set of primitive types, but allows one to extend the type
    system by creating compound types, such as arrays (lists) and
    structs. In order to use them with QtDBus, those types must be
    registered.

    See \l {qdbustypesystem.html}{QtDBus type system} for more
    information on the type system and how to register additional
    types.

    \sa {qdbustypesystem.html}{QtDBus type system},
    qDBusRegisterMetaType(), QMetaType, QVariant, QDBusArgument
*/

/*!
    \fn int qDBusRegisterMetaType(const char *typeName)
    \relates QDBusMetaType
    \threadsafe

    Registers the \a typename to the type \c{T} with the
    \l {qdbustypesystem.html}{QtDBus type system} and the Qt \l
    {QMetaType}{meta type system}, if it's not already registered.

    In order to register a type, it must be declared as a meta-type
    with the Q_DECLARE_METATYPE() macro, and then registered as in the
    following example:

    \code
        qDBusRegisterMetaType<MyClass>("MyClass");
    \endcode

    If \c{T} isn't a type derived from one of \l
    {containers.html}{Qt's container classes}, the \c{operator<<} and
    \c{operator>>} streaming operators between \c{T} and QDBusArgument
    must be already declared. See the \l {qdbustypesystem.html}{QtDBus
    type system} page for more information on how to declare such
    types.

    This function returns the Qt meta type id for the type (the same
    value that is returned from qRegisterMetaType()).

    \sa {qdbustypesystem.html}{QtDBus type system},
        qRegisterMetaType(), QDBusMetaType
*/

/*!
    \internal
    Registers the marshalling and demarshalling functions for meta
    type \a id.
*/
void QDBusMetaType::registerMarshallOperators(int id, MarshallFunction mf,
                                              DemarshallFunction df)
{
    QByteArray var;
    QVector<QDBusCustomTypeInfo> *ct = customTypes();
    if (id < 0 || !mf || !df || !ct)
        return;                 // error!

    QWriteLocker locker(customTypesLock());
    if (id >= ct->size())
        ct->resize(id + 1);
    QDBusCustomTypeInfo &info = (*ct)[id];
    info.marshall = mf;
    info.demarshall = df;
}

/*!
    \internal
    Executes the marshalling of type \a id (whose data is contained in
    \a data) to the D-BUS marshalling argument \a arg. Returns true if
    the marshalling succeeded, or false if an error occurred.
*/
bool QDBusMetaType::marshall(QDBusArgument &arg, int id, const void *data)
{
    QDBusMetaTypeId::init();

    MarshallFunction mf;
    {
        QReadLocker locker(customTypesLock());
        QVector<QDBusCustomTypeInfo> *ct = customTypes();
        if (id >= ct->size())
            return false;       // non-existant

        const QDBusCustomTypeInfo &info = (*ct).at(id);
        if (!info.marshall) {
            mf = 0;             // make gcc happy
            return false;
        } else
            mf = info.marshall;
    }

    mf(arg, data);
    return true;
}

/*!
    \internal
    Executes the demarshalling of type \a id (whose data will be placed in
    \a data) from the D-BUS marshalling argument \a arg. Returns true if
    the demarshalling succeeded, or false if an error occurred.
*/
bool QDBusMetaType::demarshall(const QDBusArgument &arg, int id, void *data)
{
    QDBusMetaTypeId::init();

    DemarshallFunction df;
    {
        QReadLocker locker(customTypesLock());
        QVector<QDBusCustomTypeInfo> *ct = customTypes();
        if (id >= ct->size())
            return false;       // non-existant

        const QDBusCustomTypeInfo &info = (*ct).at(id);
        if (!info.demarshall) {
            df = 0;             // make gcc happy
            return false;
        } else
            df = info.demarshall;
    }

    df(arg, data);
    return true;
}

/*!
    \fn QDBusMetaType::signatureToType(const char *signature)
    \internal

    Returns the Qt meta type id for the given D-Bus signature for exactly one full type, given
    by \a signature.

    Note: this function only handles the basic D-BUS types.

    \sa QDBusUtil::isValidSingleSignature(), typeToSignature(),
        QVariant::type(), QVariant::userType()
*/
int QDBusMetaType::signatureToType(const char *signature)
{
    QDBusMetaTypeId::init();
    if (!signature || !*signature ||
        !QDBusUtil::isValidSingleSignature(QString::fromLatin1(signature)))
        return QVariant::Invalid;

    switch (signature[0])
    {
    case DBUS_TYPE_BOOLEAN:
        return QVariant::Bool;

    case DBUS_TYPE_BYTE:
        return QMetaType::UChar;

    case DBUS_TYPE_INT16:
        return QMetaType::Short;

    case DBUS_TYPE_UINT16:
        return QMetaType::UShort;
        
    case DBUS_TYPE_INT32:
        return QVariant::Int;
        
    case DBUS_TYPE_UINT32:
        return QVariant::UInt;

    case DBUS_TYPE_INT64:
        return QVariant::LongLong;

    case DBUS_TYPE_UINT64:
        return QVariant::ULongLong;

    case DBUS_TYPE_DOUBLE:
        return QVariant::Double;

    case DBUS_TYPE_STRING:
        return QVariant::String;

    case DBUS_TYPE_OBJECT_PATH:
        return QDBusMetaTypeId::objectpath;

    case DBUS_TYPE_SIGNATURE:
        return QDBusMetaTypeId::signature;

    case DBUS_TYPE_VARIANT:
        return QDBusMetaTypeId::variant;

    case DBUS_TYPE_ARRAY:       // special case
        switch (signature[1]) {
        case '\0':              // invalid
            return QVariant::Invalid;

        case DBUS_TYPE_BYTE:
            return QVariant::ByteArray;

        case DBUS_TYPE_STRING:
            return QVariant::StringList;

        case DBUS_TYPE_VARIANT:
            return QVariant::List;

        case DBUS_DICT_ENTRY_BEGIN_CHAR: {
            if (strcmp(signature, "a{sv}") == 0)
                return QVariant::Map;

            // Try to construct a name
            if (strlen(signature) < 5)
                return QVariant::Invalid; // can't be a valid map signature
            if (!dbus_type_is_basic(signature[2]))
                return QVariant::Invalid;
            
            char key[2] = { signature[2], 0 };
            int kid = signatureToType(key);
            if (kid == QVariant::Invalid)
                return QVariant::Invalid;

            QByteArray value = signature + 3;
            value.truncate(value.length() - 1);
            int vid = signatureToType(value);
            if (vid == QVariant::Invalid)
                return QVariant::Invalid;

            QByteArray name = "QMap<";
            name += QVariant::typeToName( QVariant::Type(kid) );
            name += ',';
            name += QVariant::typeToName( QVariant::Type(vid) );
            if (name.endsWith('>'))
                name += ' ';
            name += '>';

            QVariant::Type result = QVariant::nameToType(name);
            if (result != QVariant::Invalid)
                return result;
        }

        default: {
            return QVariant::nameToType( constructListType(signature) );
        }
        }
    default:
        return QVariant::Invalid;
    }
}

/*!
    \fn QDBusMetaType::typeToSignature(int type)
    \internal 

    Returns the D-Bus signature equivalent to the supplied meta type id \a type.

    More types can be registered with the qDBusRegisterMetaType() function.

    \sa QDBusUtil::isValidSingleSignature(), signatureToType(),
        QVariant::type(), QVariant::userType()
*/
const char *QDBusMetaType::typeToSignature(int type)
{
    // check if it's a static type
    switch (type)
    {
    case QMetaType::UChar:
        return DBUS_TYPE_BYTE_AS_STRING;

    case QVariant::Bool:
        return DBUS_TYPE_BOOLEAN_AS_STRING;

    case QMetaType::Short:
        return DBUS_TYPE_INT16_AS_STRING;

    case QMetaType::UShort:
        return DBUS_TYPE_UINT16_AS_STRING;

    case QVariant::Int:
        return DBUS_TYPE_INT32_AS_STRING;

    case QVariant::UInt:
        return DBUS_TYPE_UINT32_AS_STRING;

    case QVariant::LongLong:
        return DBUS_TYPE_INT64_AS_STRING;

    case QVariant::ULongLong:
        return DBUS_TYPE_UINT64_AS_STRING;

    case QVariant::Double:
        return DBUS_TYPE_DOUBLE_AS_STRING;

    case QVariant::String:
        return DBUS_TYPE_STRING_AS_STRING;

    case QVariant::StringList:
        return DBUS_TYPE_ARRAY_AS_STRING
            DBUS_TYPE_STRING_AS_STRING; // as

    case QVariant::ByteArray:
        return DBUS_TYPE_ARRAY_AS_STRING
            DBUS_TYPE_BYTE_AS_STRING; // ay
    }

    QDBusMetaTypeId::init();
    if (type == QDBusMetaTypeId::variant)
        return DBUS_TYPE_VARIANT_AS_STRING;
    else if (type == QDBusMetaTypeId::objectpath)
        return DBUS_TYPE_OBJECT_PATH_AS_STRING;
    else if (type == QDBusMetaTypeId::signature)
        return DBUS_TYPE_SIGNATURE_AS_STRING;

    // try the database
    QVector<QDBusCustomTypeInfo> *ct = customTypes();
    {
        QReadLocker locker(customTypesLock());
        if (type >= ct->size())
            return 0;           // type not registered with us

        const QDBusCustomTypeInfo &info = (*ct).at(type);

        if (!info.signature.isNull())
            return info.signature;

        if (!info.marshall)
            return 0;           // type not registered with us
    }

    // call to user code to construct the signature type
    QDBusCustomTypeInfo *info;
    {
        QByteArray signature = QDBusArgumentPrivate::createSignature(type);

        // re-acquire lock
        QWriteLocker locker(customTypesLock());
        info = &(*ct)[type];
        info->signature = signature;
    }
    return info->signature;
}

