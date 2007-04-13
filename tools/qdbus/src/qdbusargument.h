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

    void beginStructure();
    void endStructure();
    void beginArray(int elementMetaTypeId);
    void endArray();
    void beginMap(int keyMetaTypeId, int valueMetaTypeId);
    void endMap();
    void beginMapEntry();
    void endMapEntry();

    // used for de-marshalling (D-BUS -> Qt)
    QString currentSignature() const;

    const QDBusArgument &operator>>(uchar &arg) const;
    const QDBusArgument &operator>>(bool &arg) const;
    const QDBusArgument &operator>>(short &arg) const;
    const QDBusArgument &operator>>(ushort &arg) const;
    const QDBusArgument &operator>>(int &arg) const;
    const QDBusArgument &operator>>(uint &arg) const;
    const QDBusArgument &operator>>(qlonglong &arg) const;
    const QDBusArgument &operator>>(qulonglong &arg) const;
    const QDBusArgument &operator>>(double &arg) const;
    const QDBusArgument &operator>>(QString &arg) const;
    const QDBusArgument &operator>>(QDBusVariant &arg) const;
    const QDBusArgument &operator>>(QDBusObjectPath &arg) const;
    const QDBusArgument &operator>>(QDBusSignature &arg) const;
    const QDBusArgument &operator>>(QStringList &arg) const;
    const QDBusArgument &operator>>(QByteArray &arg) const;

    void beginStructure() const;
    void endStructure() const;
    void beginArray() const;
    void endArray() const;
    void beginMap() const;
    void endMap() const;
    void beginMapEntry() const;
    void endMapEntry() const;
    bool atEnd() const;

protected:
    QDBusArgument(QDBusArgumentPrivate *d);
    friend class QDBusArgumentPrivate;
    mutable QDBusArgumentPrivate *d;
};
Q_DECLARE_METATYPE(QDBusArgument)

template<typename T> inline T qdbus_cast(const QDBusArgument &arg
#ifndef Q_QDOC
, T * = 0
#endif
    )
{
    T item;
    arg >> item;
    return item;
}

template<typename T> inline T qdbus_cast(const QVariant &v
#ifndef Q_QDOC
, T * = 0
#endif
    )
{
    int id = v.userType();
    if (id == qMetaTypeId<QDBusArgument>())
        return qdbus_cast<T>(qvariant_cast<QDBusArgument>(v));
    else
        return qvariant_cast<T>(v);
}

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QVariant &v);

// QVariant types
#ifndef QDBUS_NO_SPECIALTYPES

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QDate &date);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QDate &date);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QTime &time);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QTime &time);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QDateTime &dt);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QDateTime &dt);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QRect &rect);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QRect &rect);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QRectF &rect);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QRectF &rect);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QSize &size);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QSize &size);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QSizeF &size);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QSizeF &size);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QPoint &pt);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QPoint &pt);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QPointF &pt);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QPointF &pt);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QLine &line);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QLine &line);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QLineF &line);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QLineF &line);
#endif

template<template <typename> class Container, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const Container<T> &list)
{
    int id = qMetaTypeId<T>();
    arg.beginArray(id);
    typename Container<T>::const_iterator it = list.begin();
    typename Container<T>::const_iterator end = list.end();
    for ( ; it != end; ++it)
        arg << *it;
    arg.endArray();
    return arg;
}

template<template <typename> class Container, typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, Container<T> &list)
{
    arg.beginArray();
    list.clear();
    while (!arg.atEnd()) {
        T item;
        arg >> item;
        list.push_back(item);
    }

    arg.endArray();
    return arg;
}

// QList specializations
template<typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const QList<T> &list)
{
    int id = qMetaTypeId<T>();
    arg.beginArray(id);
    typename QList<T>::ConstIterator it = list.constBegin();
    typename QList<T>::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it)
        arg << *it;
    arg.endArray();
    return arg;
}

template<typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, QList<T> &list)
{
    arg.beginArray();
    list.clear();
    while (!arg.atEnd()) {
        T item;
        arg >> item;
        list.push_back(item);
    }
    arg.endArray();

    return arg;
}

inline QDBusArgument &operator<<(QDBusArgument &arg, const QVariantList &list)
{
    int id = qMetaTypeId<QDBusVariant>();
    arg.beginArray(id);
    QVariantList::ConstIterator it = list.constBegin();
    QVariantList::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it)
        arg << QDBusVariant(*it);
    arg.endArray();
    return arg;
}

// QMap specializations
template<typename Key, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const QMap<Key, T> &map)
{
    int kid = qMetaTypeId<Key>();
    int vid = qMetaTypeId<T>();
    arg.beginMap(kid, vid);
    typename QMap<Key, T>::ConstIterator it = map.constBegin();
    typename QMap<Key, T>::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        arg.beginMapEntry();
        arg << it.key() << it.value();
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}

template<typename Key, typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, QMap<Key, T> &map)
{
    arg.beginMap();
    map.clear();
    while (!arg.atEnd()) {
        Key key;
        T value;
        arg.beginMapEntry();
        arg >> key >> value;
        map.insertMulti(key, value);
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}

inline QDBusArgument &operator<<(QDBusArgument &arg, const QVariantMap &map)
{
    arg.beginMap(QVariant::String, qMetaTypeId<QDBusVariant>());
    QVariantMap::ConstIterator it = map.constBegin();
    QVariantMap::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        arg.beginMapEntry();
        arg << it.key() << QDBusVariant(it.value());
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}

QT_END_HEADER

#endif
