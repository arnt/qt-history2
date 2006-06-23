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

#ifndef QDBUSARGUMENT_P_H
#define QDBUSARGUMENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <qdbusargument.h>
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
