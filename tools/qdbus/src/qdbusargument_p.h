/* qdbusargument_p.h
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

#ifndef QDBUSARGUMENT_P_H
#define QDBUSARGUMENT_P_H

#include "qdbusargument.h"
#include <dbus/dbus.h>

class QDBusMarshaller;
class QDBusDemarshaller;
class QDBusArgumentPrivate
{
public:
    inline QDBusArgumentPrivate()
        : ref(1)
    { }
    inline virtual ~QDBusArgumentPrivate()
    { }

    bool checkRead();
    bool checkWrite();

    QDBusMarshaller *marshaller();
    QDBusDemarshaller *demarshaller();

    static QByteArray createSignature(int id);
    static inline QDBusArgument create(QDBusArgumentPrivate *d)
    {
        QDBusArgument q;
        q.d = d;
        return q;
    }
    static inline QDBusDemarshaller *demarshaller(const QDBusArgument &q)
    { if (q.d->checkRead()) return q.d->demarshaller(); return 0; }

public:
    QAtomic ref;
    enum Direction {
        Marshalling,
        Demarshalling
    } direction;
};

#ifdef Q_CC_GCC
# pragma GCC visibility push(internal)
#endif

class QDBusMarshaller: public QDBusArgumentPrivate
{
public:
    QDBusMarshaller() : parent(0), ba(0), closeCode(0), ok(true)
    { direction = Marshalling; }
    ~QDBusMarshaller() { close(); }

    void append(uchar arg);
    void append(bool arg);
    void append(short arg);
    void append(ushort arg);
    void append(int arg);
    void append(uint arg);
    void append(qlonglong arg);
    void append(qulonglong arg);
    void append(double arg);
    void append(const QString &arg);
    void append(const QDBusObjectPath &arg);
    void append(const QDBusSignature &arg);
    void append(const QStringList &arg);
    void append(const QByteArray &arg);
    bool append(const QDBusVariant &arg); // this one can fail

    QDBusArgument recurseStructure();
    QDBusArgument recurseArray(int id);
    QDBusArgument recurseMap(int kid, int vid);
    QDBusArgument recurseMapEntry();
    QDBusArgument recurseCommon(int code, const char *signature);
    void open(QDBusMarshaller &sub, int code, const char *signature);
    void close();
    void error();

    bool appendVariantInternal(const QVariant &arg);
    bool appendRegisteredType(const QVariant &arg);
    bool appendCrossMarshalling(QDBusDemarshaller *arg);

public:
    DBusMessageIter iterator;
    QDBusMarshaller *parent;
    QByteArray *ba;
    char closeCode;
    bool ok;
};

class QDBusDemarshaller: public QDBusArgumentPrivate
{
public:
    inline QDBusDemarshaller() : message(0) { direction = Demarshalling; }
    ~QDBusDemarshaller();

    QString currentSignature();

    uchar toByte();
    bool toBool();
    ushort toUShort();
    short toShort();
    int toInt();
    uint toUInt();
    qlonglong toLongLong();
    qulonglong toULongLong();
    double toDouble();
    QString toString();
    QDBusObjectPath toObjectPath();
    QDBusSignature toSignature();
    QDBusVariant toVariant();
    QStringList toStringList();
    QByteArray toByteArray();

    QDBusArgument recurseStructure();
    QDBusArgument recurseArray();
    QDBusArgument recurseMap();
    QDBusArgument recurseMapEntry();
    QDBusArgument recurseCommon();
    QDBusArgument duplicate();
    void close() { }

    bool atEnd();

    QVariant toVariantInternal();

public:
    DBusMessageIter iterator;
    DBusMessage *message;
};

#ifdef Q_CC_GCC
# pragma GCC visibility pop
#endif

inline QDBusMarshaller *QDBusArgumentPrivate::marshaller()
{ return static_cast<QDBusMarshaller *>(this); }

inline QDBusDemarshaller *QDBusArgumentPrivate::demarshaller()
{ return static_cast<QDBusDemarshaller *>(this); }

#endif
