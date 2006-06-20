/* qdbusargument.cpp
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

#include "qdbusargument.h"

#include <qatomic.h>
#include <qbytearray.h>
#include <qlist.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>

#include "qdbusargument_p.h"
#include "qdbusmetatype_p.h"

QByteArray QDBusArgumentPrivate::createSignature(int id)
{
    QByteArray signature;
    QDBusMarshaller *marshaller = new QDBusMarshaller;
    marshaller->ba = &signature;

    // run it
    void *null = 0;
    QVariant v(id, null);
    QDBusArgument arg;
    arg.d = marshaller;
    QDBusMetaType::marshall(arg, v.userType(), v.constData());
    arg.d = 0;

    // delete it
    bool ok = marshaller->ok;
    delete marshaller;

    if (signature.isEmpty() || !ok || !dbus_signature_validate_single(signature, 0)) {
        qWarning("QDBusMarshaller: type `%s' produces invalid D-BUS signature `%s' "
                 "(Did you forget to call newStructure() ?)",
                 QVariant::typeToName( QVariant::Type(id) ),
                 signature.isEmpty() ? "<empty>" : signature.constData());
        return "";
    } else if (signature.at(0) != DBUS_TYPE_ARRAY && signature.at(0) != DBUS_STRUCT_BEGIN_CHAR ||
               (signature.at(0) == DBUS_TYPE_ARRAY && (signature.at(1) == DBUS_TYPE_BYTE ||
                                                       signature.at(1) == DBUS_TYPE_STRING))) {
        qWarning("QDBusMarshaller: type `%s' attempts to redefine basic D-BUS type '%s' (%s) "
                 "(Did you forget to call newStructure() ?)",
                 QVariant::typeToName( QVariant::Type(id) ),
                 signature.constData(),
                 QVariant::typeToName( QVariant::Type(QDBusMetaType::signatureToType(signature))) );
        return "";
    }
    return signature;
}

bool QDBusArgumentPrivate::checkWrite()
{
    if (direction == Marshalling)
        return marshaller()->ok;

#ifdef QT_DEBUG
    qFatal("QDBusArgument: write from a read-only object");
#else
    qWarning("QDBusArgument: write from a read-only object");
#endif
    return false;
}

bool QDBusArgumentPrivate::checkRead()
{
    if (direction == Demarshalling)
        return true;

#ifdef QT_DEBUG
    qFatal("QDBusArgument: read from a write-only object");
#else
    qWarning("QDBusArgument: read from a write-only object");
#endif

    return false;
}
        
QDBusArgument::QDBusArgument()
    : d(0)
{
}

QDBusArgument::QDBusArgument(const QDBusArgument &other)
    : d(other.d)
{
    if (d)
        d->ref.ref();
}

QDBusArgument &QDBusArgument::operator=(const QDBusArgument &other)
{
    if (other.d)
        other.d->ref.ref();
    QDBusArgumentPrivate *old = qAtomicSetPtr(&d, other.d);
    if (old && !old->ref.deref())
        delete old;
    return *this;
}

QDBusArgument::~QDBusArgument()
{
    if (d && !d->ref.deref())
        delete d;
}

QDBusArgument &QDBusArgument::operator<<(uchar arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(bool arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(short arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(ushort arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(int arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(uint arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(qlonglong arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(qulonglong arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(double arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(const QString &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(const QDBusObjectPath &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(const QDBusSignature &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(const QDBusVariant &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(const QStringList &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QDBusArgument &QDBusArgument::operator<<(const QByteArray &arg)
{
    if (d && d->checkWrite())
        d->marshaller()->append(arg);
    return *this;
}

QString QDBusArgument::currentSignature() const
{
    if (d && d->checkRead())
        return d->demarshaller()->currentSignature();

    return QString();
}

uchar QDBusArgument::toByte() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toByte();

    return 0;
}

bool QDBusArgument::toBool() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toBool();
    return false;
}

ushort QDBusArgument::toUShort() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toUShort();

    return 0;
}

short QDBusArgument::toShort() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toShort();

    return 0;
}

int QDBusArgument::toInt() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toInt();

    return 0;
}

uint QDBusArgument::toUInt() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toUInt();


    return 0;
}

qlonglong QDBusArgument::toLongLong() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toLongLong();


    return 0;
}

qulonglong QDBusArgument::toULongLong() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toULongLong();


    return 0;
}

double QDBusArgument::toDouble() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toDouble();


    return 0;
}

QString QDBusArgument::toString() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toString();


    return QString();
}

QDBusObjectPath QDBusArgument::toObjectPath() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toObjectPath();

    return QDBusObjectPath();
}

QDBusSignature QDBusArgument::toSignature() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toSignature();

    return QDBusSignature();
}

QDBusVariant QDBusArgument::toVariant() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toVariant();

    return QDBusVariant();
}

QStringList QDBusArgument::toStringList() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toStringList();

    return QStringList();
}

QByteArray QDBusArgument::toByteArray() const
{
    if (d && d->checkRead())
        return d->demarshaller()->toByteArray();

    return QByteArray();
}

QDBusArgument QDBusArgument::newStructure()
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseStructure();

    return QDBusArgument();
}

QDBusArgument QDBusArgument::newArray(int id)
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseArray(id);

    return QDBusArgument();
}

QDBusArgument QDBusArgument::newMap(int kid, int vid)
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseMap(kid, vid);

    return QDBusArgument();
}

QDBusArgument QDBusArgument::newMapEntry()
{
    if (d && d->checkWrite())
        return d->marshaller()->recurseMapEntry();

    return QDBusArgument();
}    

QDBusArgument QDBusArgument::structure() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseStructure();

    return QDBusArgument();
}

QDBusArgument QDBusArgument::array() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseArray();

    return QDBusArgument();
}

QDBusArgument QDBusArgument::map() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseMap();

    return QDBusArgument();
}

QDBusArgument QDBusArgument::mapEntry() const
{
    if (d && d->checkRead())
        return d->demarshaller()->recurseMapEntry();

    return QDBusArgument();
}

bool QDBusArgument::atEnd() const
{
    if (d && d->checkRead())
        return d->demarshaller()->atEnd();

    return true;                // at least, stop reading
}

// for optimization purposes, we include the marshallers here
#include "qdbusmarshaller.cpp"
#include "qdbusdemarshaller.cpp"
