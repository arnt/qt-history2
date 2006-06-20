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

#ifndef QDBUSARGUMENTOPERATORS_H
#define QDBUSARGUMENTOPERATORS_H

#include <QtCore/qvariant.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qrect.h>
#include <QtCore/qline.h>

QT_BEGIN_HEADER


inline const QDBusArgument &operator>>(const QDBusArgument &a, uchar &arg)
{ arg = a.toByte(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, bool &arg)
{ arg = a.toBool(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, ushort &arg)
{ arg = a.toUShort(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, short &arg)
{ arg = a.toShort(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, int &arg)
{ arg = a.toInt(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, uint &arg)
{ arg = a.toUInt(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, qlonglong &arg)
{ arg = a.toLongLong(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, qulonglong &arg)
{ arg = a.toULongLong(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, double &arg)
{ arg = a.toDouble(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, QString &arg)
{ arg = a.toString(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, QDBusObjectPath &arg)
{ arg = a.toObjectPath(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, QDBusSignature &arg)
{ arg = a.toSignature(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, QVariant &arg)
{ arg = a.toVariant().value; return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, QDBusVariant &arg)
{ arg = a.toVariant(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, QStringList &arg)
{ arg = a.toStringList(); return a; }

inline const QDBusArgument &operator>>(const QDBusArgument &a, QByteArray &arg)
{ arg = a.toByteArray(); return a; }

// QVariant types
#ifndef QDBUS_NO_SPECIALTYPES
inline const QDBusArgument &operator>>(const QDBusArgument &a, QDate &date)
{
    int y, m, d;
    a.structure() >> y >> m >> d;
    if (y != 0 && m != 0 && d != 0)
        date.setYMD(y, m, d);
    else
        date = QDate();
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QDate &date)
{
    if (date.isValid())
        a.newStructure() << date.year() << date.month() << date.day();
    else
        a.newStructure() << 0 << 0 << 0;
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QTime &time)
{
    int h, m, s, ms;
    a.structure() >> h >> m >> s >> ms;
    if (h < 0)
        time = QTime();
    else
        time.setHMS(h, m, s, ms);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QTime &time)
{
    if (time.isValid())
        a.newStructure() << time.hour() << time.minute() << time.second() << time.msec();
    else
        a.newStructure() << -1 << -1 << -1 << -1;
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QDateTime &dt)
{
    QDate date;
    QTime time;
    int timespec;
    a.structure() >> date >> time >> timespec;
    dt = QDateTime(date, time, Qt::TimeSpec(timespec));
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QDateTime &dt)
{
    QDBusArgument sub = a.newStructure();
    sub << dt.date() << dt.time() << int(dt.timeSpec());
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QRect &rect)
{
    int x, y, width, height;
    a.structure() >> x >> y >> width >> height;
    rect.setRect(x, y, width, height);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QRect &rect)
{
    a.newStructure() << rect.x() << rect.y() << rect.width() << rect.height();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QRectF &rect)
{
    qreal x, y, width, height;
    a.structure() >> x >> y >> width >> height;
    rect.setRect(x, y, width, height);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QRectF &rect)
{
    a.newStructure() << rect.x() << rect.y() << rect.width() << rect.height();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QSize &size)
{
    a.structure() >> size.rwidth() >> size.rheight();
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QSize &size)
{
    a.newStructure() << size.width() << size.height();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QSizeF &size)
{
    a.structure() >> size.rwidth() >> size.rheight();
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QSizeF &size)
{
    a.newStructure() << size.width() << size.height();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QPoint &pt)
{
    a.structure() >> pt.rx() >> pt.ry();
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QPoint &pt)
{
    a.newStructure() << pt.x() << pt.y();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QPointF &pt)
{
    a.structure() >> pt.rx() >> pt.ry();
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QPointF &pt)
{
    a.newStructure() << pt.x() << pt.y();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QLine &line)
{
    QPoint p1, p2;
    a.structure() >> p1 >> p2;
    line = QLine(p1, p2);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QLine &line)
{
    QDBusArgument sub = a.newStructure();
    sub << line.p1() << line.p2();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QLineF &line)
{
    QPointF p1, p2;
    a.structure() >> p1 >> p2;
    line = QLineF(p1, p2);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QLineF &line)
{
    QDBusArgument sub = a.newStructure();
    sub << line.p1() << line.p2();
    return a;
}
#endif

template<template <typename> class Container, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const Container<T> &list)
{
    int id = qt_variant_metatype_id((T*)0);
    QDBusArgument array = arg.newArray(id);
    typename Container<T>::const_iterator it = list.begin();
    typename Container<T>::const_iterator end = list.end();
    for ( ; it != end; ++it)
        array << *it;
    return arg;
}

template<template <typename> class Container, typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, Container<T> &list)
{
    QDBusArgument array = arg.array();
    list.clear();
    while (!array.atEnd()) {
        T item;
        array >> item;
        list.push_back(item);
    }

    return arg;
}

// QList specializations
template<typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const QList<T> &list)
{
    int id = qt_variant_metatype_id((T*)0);
    QDBusArgument array = arg.newArray(id);
    typename QList<T>::ConstIterator it = list.constBegin();
    typename QList<T>::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it)
        array << *it;
    return arg;
}

template<typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, QList<T> &list)
{
    QDBusArgument array = arg.array();
    list.clear();
    while (!array.atEnd()) {
        T item;
        array >> item;
        list.push_back(item);
    }

    return arg;
}

inline QDBusArgument &operator<<(QDBusArgument &arg, const QVariantList &list)
{
    int id = qMetaTypeId<QDBusVariant>();
    QDBusArgument array = arg.newArray(id);
    QVariantList::ConstIterator it = list.constBegin();
    QVariantList::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it)
        array << QDBusVariant(*it);
    return arg;
}

#if 0
template<template <typename, typename> class AssociativeContainer, typename Key, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const AssociativeContainer<Key, T> &map)
{
    int kid = qt_variant_metatype_id((Key*)0);
    int vid = qt_variant_metatype_id((T*)0);
    QDBusArgument amap = arg.newMap(kid, vid);
    typename AssociativeContainer<Key, T>::const_iterator it = map.begin();
    typename AssociativeContainer<Key, T>::const_iterator end = map.end();
    for ( ; it != end; ++it) {
        QDBusArgument entry = amap.newMapEntry();
        entry << it.key() << it.value();
    }
    return arg;
}

template<template <typename, typename> class AssociativeContainer, typename Key, typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, AssociativeContainer<Key, T> &map)
{
    QDBusArgument amap = arg.map();
    map.clear();
    while (!amap.atEnd()) {
        Key key;
        T value;
        QDBusArgument entry = amap.mapEntry();
        entry >> key >> value;
        map.insertMulti(key, value);
    }
    return arg;
}
#endif

// QMap specializations
template<typename Key, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const QMap<Key, T> &map)
{
    int kid = qt_variant_metatype_id((Key*)0);
    int vid = qt_variant_metatype_id((T*)0);
    QDBusArgument amap = arg.newMap(kid, vid);
    typename QMap<Key, T>::ConstIterator it = map.constBegin();
    typename QMap<Key, T>::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        QDBusArgument entry = amap.newMapEntry();
        entry << it.key() << it.value();
    }
    return arg;
}

template<typename Key, typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, QMap<Key, T> &map)
{
    QDBusArgument amap = arg.map();
    map.clear();
    while (!amap.atEnd()) {
        Key key;
        T value;
        QDBusArgument entry = amap.mapEntry();
        entry >> key >> value;
        map.insertMulti(key, value);
    }
    return arg;
}

inline QDBusArgument &operator<<(QDBusArgument &arg, const QVariantMap &map)
{
    QDBusArgument amap = arg.newMap(QVariant::String, qMetaTypeId<QDBusVariant>());
    QVariantMap::ConstIterator it = map.constBegin();
    QVariantMap::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        QDBusArgument entry = amap.newMapEntry();
        entry << it.key() << QDBusVariant(it.value());
    }
    return arg;
}

QT_END_HEADER

#endif
