/* qdbusdemarshaller.cpp
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

#include "qdbusargument_p.h"

template <typename T>
static inline T qIterGet(DBusMessageIter *it)
{
    T t;
    dbus_message_iter_get_basic(it, &t);
    dbus_message_iter_next(it);
    return t;
}

inline QDBusDemarshaller::~QDBusDemarshaller()
{
    if (message)
        dbus_message_unref(message);
}

inline QString QDBusDemarshaller::currentSignature()
{
    char *sig = dbus_message_iter_get_signature(&iterator);
    QString retval = QString::fromUtf8(sig);
    free(sig);

    return retval;
}

inline uchar QDBusDemarshaller::toByte()
{
    return qIterGet<uchar>(&iterator);
}

inline bool QDBusDemarshaller::toBool()
{
    return bool(qIterGet<dbus_bool_t>(&iterator));
}

inline ushort QDBusDemarshaller::toUShort()
{
    return qIterGet<dbus_uint16_t>(&iterator);
}

inline short QDBusDemarshaller::toShort()
{
    return qIterGet<dbus_int16_t>(&iterator);
}

inline int QDBusDemarshaller::toInt()
{
    return qIterGet<dbus_int32_t>(&iterator);
}

inline uint QDBusDemarshaller::toUInt()
{
    return qIterGet<dbus_uint32_t>(&iterator);
}

inline qlonglong QDBusDemarshaller::toLongLong()
{
    return qlonglong(qIterGet<dbus_int64_t>(&iterator));
}

inline qulonglong QDBusDemarshaller::toULongLong()
{
    return qulonglong(qIterGet<dbus_uint64_t>(&iterator));
}

inline double QDBusDemarshaller::toDouble()
{
    return qIterGet<double>(&iterator);
}

inline QString QDBusDemarshaller::toString()
{
    return QString::fromUtf8(qIterGet<char *>(&iterator));
}

inline QDBusObjectPath QDBusDemarshaller::toObjectPath()
{
    return QDBusObjectPath(QString::fromUtf8(qIterGet<char *>(&iterator)));
}

inline QDBusSignature QDBusDemarshaller::toSignature()
{
    return QDBusSignature(QString::fromUtf8(qIterGet<char *>(&iterator)));
}

inline QDBusVariant QDBusDemarshaller::toVariant()
{
    QDBusDemarshaller sub;
    sub.message = dbus_message_ref(message);
    dbus_message_iter_recurse(&iterator, &sub.iterator);
    dbus_message_iter_next(&iterator);

    return QDBusVariant( sub.toVariantInternal() );
}

inline QVariant QDBusDemarshaller::toVariantInternal()
{
    switch (dbus_message_iter_get_arg_type(&iterator)) {
    case DBUS_TYPE_BYTE:
        return qVariantFromValue(toByte());
    case DBUS_TYPE_INT16:
	return qVariantFromValue(toShort());
    case DBUS_TYPE_UINT16:
	return qVariantFromValue(toUShort());
    case DBUS_TYPE_INT32:
        return toInt();
    case DBUS_TYPE_UINT32:
        return toUInt();
    case DBUS_TYPE_DOUBLE:
        return toDouble();
    case DBUS_TYPE_BOOLEAN:
        return toBool();
    case DBUS_TYPE_INT64:
        return toLongLong();
    case DBUS_TYPE_UINT64:
        return toULongLong();
    case DBUS_TYPE_STRING:
        return toString();
    case DBUS_TYPE_OBJECT_PATH:
        return qVariantFromValue(toObjectPath());
    case DBUS_TYPE_SIGNATURE:
        return qVariantFromValue(toSignature());
    case DBUS_TYPE_VARIANT:
        return qVariantFromValue(toVariant());

    case DBUS_TYPE_ARRAY:
        switch (dbus_message_iter_get_element_type(&iterator)) {
        case DBUS_TYPE_BYTE:
            // QByteArray
            return toByteArray();
        case DBUS_TYPE_STRING:
            return toStringList();
        case DBUS_TYPE_DICT_ENTRY:
            return qVariantFromValue(duplicate());

        default:
            return qVariantFromValue(duplicate());
        }

    case DBUS_TYPE_STRUCT:
        return qVariantFromValue(duplicate());

    default:
        qWarning("QDDBusDemarshaller: Found unknown D-DBUS type %d '%c'",
                 dbus_message_iter_get_arg_type(&iterator),
                 dbus_message_iter_get_arg_type(&iterator));
        return QVariant();
        break;
    };
}

QStringList QDBusDemarshaller::toStringList()
{
    QStringList list;

    QDBusDemarshaller sub;
    dbus_message_iter_recurse(&iterator, &sub.iterator);
    dbus_message_iter_next(&iterator);
    while (!sub.atEnd())
        list.append(sub.toString());

    return list;
}

QByteArray QDBusDemarshaller::toByteArray()
{
    DBusMessageIter sub;
    dbus_message_iter_recurse(&iterator, &sub);
    dbus_message_iter_next(&iterator);
    int len = dbus_message_iter_get_array_len(&sub);
    char* data;
    dbus_message_iter_get_fixed_array(&sub,&data,&len);
    return QByteArray(data,len);
}

inline bool QDBusDemarshaller::atEnd()
{
    // dbus_message_iter_has_next is broken if the list has one single element
    return dbus_message_iter_get_arg_type(&iterator) == DBUS_TYPE_INVALID;
}

inline QDBusArgument QDBusDemarshaller::recurseStructure()
{
    return recurseCommon();
}

inline QDBusArgument QDBusDemarshaller::recurseArray()
{
    return recurseCommon();
}

inline QDBusArgument QDBusDemarshaller::recurseMap()
{
    return recurseCommon();
}

inline QDBusArgument QDBusDemarshaller::recurseMapEntry()
{
    return recurseCommon();
}

QDBusArgument QDBusDemarshaller::recurseCommon()
{
    QDBusDemarshaller *d = new QDBusDemarshaller;
    d->message = dbus_message_ref(message);

    // recurse
    dbus_message_iter_recurse(&iterator, &d->iterator);
    dbus_message_iter_next(&iterator);
    return QDBusArgumentPrivate::create(d);
}

QDBusArgument QDBusDemarshaller::duplicate()
{
    QDBusDemarshaller *d = new QDBusDemarshaller;
    d->iterator = iterator;
    d->message = dbus_message_ref(message);

    dbus_message_iter_next(&iterator);
    return QDBusArgumentPrivate::create(d);
}
