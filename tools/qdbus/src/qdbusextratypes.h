/* qdbusextratypes.h
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

#ifndef QDBUSEXTRATYPES_H
#define QDBUSEXTRATYPES_H

// define some useful types for D-BUS

#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>

#include "qdbusutil.h"

struct QDBusObjectPath
{
    inline QDBusObjectPath() { }

    inline explicit QDBusObjectPath(const char *path)
        : value(QString::fromLatin1(path))
    { if (!QDBusUtil::isValidObjectPath(value)) value.clear(); }

    inline explicit QDBusObjectPath(const QLatin1String &path)
        : value(path)
    { if (!QDBusUtil::isValidObjectPath(value)) value.clear(); }

    inline explicit QDBusObjectPath(const QString &path)
        : value(path)
    { if (!QDBusUtil::isValidObjectPath(value)) value.clear(); }

    QString value;
};
Q_DECLARE_METATYPE(QDBusObjectPath)

struct QDBusSignature
{
    inline QDBusSignature() { }

    inline explicit QDBusSignature(const char *signature)
        : value(QString::fromLatin1(signature))
    { if (!QDBusUtil::isValidSignature(value)) value.clear(); }

    inline explicit QDBusSignature(const QLatin1String &signature)
        : value(signature)
    { if (!QDBusUtil::isValidSignature(value)) value.clear(); }

    inline explicit QDBusSignature(const QString &signature)
        : value(signature)
    { if (!QDBusUtil::isValidSignature(value)) value.clear(); }

    QString value;
};
Q_DECLARE_METATYPE(QDBusSignature)

struct QDBusVariant
{
    inline QDBusVariant() { }
    inline explicit QDBusVariant(const QVariant &variant) : value(variant) { }
    QVariant value;
};
Q_DECLARE_METATYPE(QDBusVariant)

#endif
