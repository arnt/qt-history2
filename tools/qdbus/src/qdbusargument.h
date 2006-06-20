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

#ifndef QDBUSARGUMENT_H
#define QDBUSARGUMENT_H

#include <QtCore/qbytearray.h>
#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvariant.h>
#include <QtDBus/qdbusextratypes.h>
#include <QtDBus/qdbusmacros.h>

QT_BEGIN_HEADER


class QDBusArgumentPrivate;
class QDBusDemarshaller;
class QDBusMarshaller;
class QDBUS_EXPORT QDBusArgument
{
public:
    QDBusArgument();
    QDBusArgument(const QDBusArgument &other);
    QDBusArgument &operator=(const QDBusArgument &other);
    ~QDBusArgument();

    // used for marshalling (Qt -> D-BUS)
    QDBusArgument &operator<<(uchar arg);
    QDBusArgument &operator<<(bool arg);
    QDBusArgument &operator<<(short arg);
    QDBusArgument &operator<<(ushort arg);
    QDBusArgument &operator<<(int arg);
    QDBusArgument &operator<<(uint arg);
    QDBusArgument &operator<<(qlonglong arg);
    QDBusArgument &operator<<(qulonglong arg);
    QDBusArgument &operator<<(double arg);
    QDBusArgument &operator<<(const QString &arg);
    QDBusArgument &operator<<(const QDBusVariant &arg);
    QDBusArgument &operator<<(const QDBusObjectPath &arg);
    QDBusArgument &operator<<(const QDBusSignature &arg);
    QDBusArgument &operator<<(const QStringList &arg);
    QDBusArgument &operator<<(const QByteArray &arg);

    QDBusArgument newStructure();
    QDBusArgument newArray(int elementMetaTypeId);
    QDBusArgument newMap(int keyMetaTypeId, int valueMetaTypeId);
    QDBusArgument newMapEntry();

    // used for de-marshalling (D-BUS -> Qt)
    QString currentSignature() const;

    uchar toByte() const;
    bool toBool() const;
    ushort toUShort() const;
    short toShort() const;
    int toInt() const;
    uint toUInt() const;
    qlonglong toLongLong() const;
    qulonglong toULongLong() const;
    double toDouble() const;
    QString toString() const;
    QDBusObjectPath toObjectPath() const;
    QDBusSignature toSignature() const;
    QDBusVariant toVariant() const;
    QStringList toStringList() const;
    QByteArray toByteArray() const;

    QDBusArgument structure() const;
    QDBusArgument array() const;
    QDBusArgument map() const;
    QDBusArgument mapEntry() const;
    bool atEnd() const;

protected:
    friend class QDBusArgumentPrivate;
    QDBusArgumentPrivate *d;
};
Q_DECLARE_METATYPE(QDBusArgument)

template<typename T> inline T qdbus_cast(const QDBusArgument &arg, T * = 0)
{
    T item;
    arg >> item;
    return item;
}

template<typename T> inline T qdbus_cast(const QVariant &v, T * = 0)
{
    int id = v.userType();
    if (id == qMetaTypeId<QDBusArgument>())
        return qdbus_cast<T>(qvariant_cast<QDBusArgument>(v));
    else
        return qvariant_cast<T>(v);
}

QT_END_HEADER

#include <QtDBus/qdbusargumentoperators.h>

#endif
