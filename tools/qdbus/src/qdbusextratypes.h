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
#include <QtDBus/qdbusmacros.h>

QT_BEGIN_HEADER

class QDBUS_EXPORT QDBusObjectPath : private QString
{
public:
    inline QDBusObjectPath() { }

    inline explicit QDBusObjectPath(const char *path)
        : QString(QString::fromLatin1(path))
    { check(); }

    inline explicit QDBusObjectPath(const QLatin1String &path)
        : QString(path)
    { check(); }

    inline explicit QDBusObjectPath(const QString &path)
        : QString(path)
    { check(); }

    inline void setPath(const QString &path)
    { QString::operator=(path); }

    inline QString path() const
    { return *this; }

private:
    void check();
};
Q_DECLARE_METATYPE(QDBusObjectPath)

class QDBUS_EXPORT QDBusSignature : private QString
{
public:
    inline QDBusSignature() { }

    inline explicit QDBusSignature(const char *signature)
        : QString(QString::fromAscii(signature))
    { check(); }

    inline explicit QDBusSignature(const QLatin1String &signature)
        : QString(signature)
    { check(); }

    inline explicit QDBusSignature(const QString &signature)
        : QString(signature)
    { check(); }

    inline void setSignature(const QString &signature)
    { QString::operator=(signature); }

    inline QString signature() const
    { return *this; }

private:
    void check();
};
Q_DECLARE_METATYPE(QDBusSignature)

class QDBusVariant : private QVariant
{
public:
    inline QDBusVariant() { }
    inline explicit QDBusVariant(const QVariant &variant)
        : QVariant(variant) { }

    inline void setVariant(const QVariant &variant)
    { QVariant::operator=(variant); }

    inline QVariant variant() const
    { return *this; }
};
Q_DECLARE_METATYPE(QDBusVariant)

QT_END_HEADER

#endif
