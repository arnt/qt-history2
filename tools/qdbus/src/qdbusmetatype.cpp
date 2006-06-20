/* qdbusmetatype.cpp
 *
 * Copyright (C) 2006 Trolltech AS. All rights reserved.
 *    Author: Thiago Macieira <thiago.macieira@trolltech.com>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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
    \fn QDBusMetaType::signatureToType(const QString &signature)
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

