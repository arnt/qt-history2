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

inline const QDBusArgument &operator>>(const QDBusArgument &a, QVariant &v)
{
    QDBusVariant dbv;
    a >> dbv;
    v = dbv.value;
    return a;
}

// QVariant types
#ifndef QDBUS_NO_SPECIALTYPES
inline const QDBusArgument &operator>>(const QDBusArgument &a, QDate &date)
{
    int y, m, d;
    a.beginStructure();
    a >> y >> m >> d;
    a.endStructure();

    if (y != 0 && m != 0 && d != 0)
        date.setYMD(y, m, d);
    else
        date = QDate();
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QDate &date)
{
    a.beginStructure();
    if (date.isValid())
        a << date.year() << date.month() << date.day();
    else
        a << 0 << 0 << 0;
    a.endStructure();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QTime &time)
{
    int h, m, s, ms;
    a.beginStructure();
    a >> h >> m >> s >> ms;
    a.endStructure();

    if (h < 0)
        time = QTime();
    else
        time.setHMS(h, m, s, ms);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QTime &time)
{
    a.beginStructure();
    if (time.isValid())
        a << time.hour() << time.minute() << time.second() << time.msec();
    else
        a << -1 << -1 << -1 << -1;
    a.endStructure();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QDateTime &dt)
{
    QDate date;
    QTime time;
    int timespec;

    a.beginStructure();
    a >> date >> time >> timespec;
    a.endStructure();

    dt = QDateTime(date, time, Qt::TimeSpec(timespec));
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QDateTime &dt)
{
    a.beginStructure();
    a << dt.date() << dt.time() << int(dt.timeSpec());
    a.endStructure();
    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QRect &rect)
{
    int x, y, width, height;
    a.beginStructure();
    a >> x >> y >> width >> height;
    a.endStructure();

    rect.setRect(x, y, width, height);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QRect &rect)
{
    a.beginStructure();
    a << rect.x() << rect.y() << rect.width() << rect.height();
    a.endStructure();

    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QRectF &rect)
{
    qreal x, y, width, height;
    a.beginStructure();
    a >> x >> y >> width >> height;
    a.endStructure();

    rect.setRect(x, y, width, height);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QRectF &rect)
{
    a.beginStructure();
    a << rect.x() << rect.y() << rect.width() << rect.height();
    a.endStructure();

    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QSize &size)
{
    a.beginStructure();
    a >> size.rwidth() >> size.rheight();
    a.endStructure();

    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QSize &size)
{
    a.beginStructure();
    a << size.width() << size.height();
    a.endStructure();

    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QSizeF &size)
{
    a.beginStructure();
    a >> size.rwidth() >> size.rheight();
    a.endStructure();

    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QSizeF &size)
{
    a.beginStructure();
    a << size.width() << size.height();
    a.endStructure();

    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QPoint &pt)
{
    a.beginStructure();
    a >> pt.rx() >> pt.ry();
    a.endStructure();

    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QPoint &pt)
{
    a.beginStructure();
    a << pt.x() << pt.y();
    a.endStructure();

    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QPointF &pt)
{
    a.beginStructure();
    a >> pt.rx() >> pt.ry();
    a.endStructure();

    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QPointF &pt)
{
    a.beginStructure();
    a << pt.x() << pt.y();
    a.endStructure();

    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QLine &line)
{
    QPoint p1, p2;
    a.beginStructure();
    a >> p1 >> p2;
    a.endStructure();

    line = QLine(p1, p2);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QLine &line)
{
    a.beginStructure();
    a << line.p1() << line.p2();
    a.endStructure();

    return a;
}

inline const QDBusArgument &operator>>(const QDBusArgument &a, QLineF &line)
{
    QPointF p1, p2;
    a.beginStructure();
    a >> p1 >> p2;
    a.endStructure();

    line = QLineF(p1, p2);
    return a;
}
inline QDBusArgument &operator<<(QDBusArgument &a, const QLineF &line)
{
    a.beginStructure();
    a << line.p1() << line.p2();
    a.endStructure();

    return a;
}
#endif

template<template <typename> class Container, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const Container<T> &list)
{
    int id = qt_variant_metatype_id(static_cast<T *>(0));
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
    int id = qt_variant_metatype_id(static_cast<T *>(0));
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
    int kid = qt_variant_metatype_id(static_cast<Key *>(0));
    int vid = qt_variant_metatype_id(static_cast<T *>(0));
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
