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

#ifndef QDBUSEXTRATYPES_H
#define QDBUSEXTRATYPES_H

// define some useful types for D-BUS

#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>

#include <QtDBus/qdbusutil.h>

QT_BEGIN_HEADER


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

QT_END_HEADER

#endif
