/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcorevariant.h"
#ifndef QT_NO_VARIANT
#include "qbitarray.h"
#include "qbytearray.h"
#include "qdatastream.h"
#include "qdebug.h"
#include "qmap.h"
#include "qdatetime.h"
#include "qlist.h"
#include "qstring.h"
#include "qstringlist.h"

#include <float.h>

#ifndef DBL_DIG
#define DBL_DIG 10
#endif //DBL_DIG

#if defined Q_CC_MSVC && _MSC_VER < 1300

template<> QBitArray QVariant_to_helper<QBitArray>(const QCoreVariant &v, const QBitArray*)
{ return v.toBitArray(); }
template<> QString QVariant_to_helper<QString>(const QCoreVariant &v, const QString*) { return v.toString(); }
template<> QStringList QVariant_to_helper<QStringList>(const QCoreVariant &v, const QStringList*)
{ return v.toStringList(); }
template<> QDate QVariant_to_helper<QDate>(const QCoreVariant &v, const QDate*) { return v.toDate(); }
template<> QTime QVariant_to_helper<QTime>(const QCoreVariant &v, const QTime*) { return v.toTime(); }
template<> QDateTime QVariant_to_helper<QDateTime>(const QCoreVariant &v, const QDateTime*)
{ return v.toDateTime(); }
#ifndef QT_NO_TEMPLATE_VARIANT
template<> QList<QCoreVariant>
QVariant_to_helper<QList<QCoreVariant> >(const QCoreVariant &v, const QList<QCoreVariant>*)
{ return v.toList(); }
template<> QMap<QString,QCoreVariant>
QVariant_to_helper<QMap<QString,QCoreVariant> >(const QCoreVariant &v, const QMap<QString,QCoreVariant>*)
{ return v.toMap(); }
#endif

#else

template<> QBitArray QVariant_to<QBitArray>(const QCoreVariant &v) { return v.toBitArray(); }
template<> QString QVariant_to<QString>(const QCoreVariant &v) { return v.toString(); }
template<> QStringList QVariant_to<QStringList>(const QCoreVariant &v)
{ return v.toStringList(); }
template<> QDate QVariant_to<QDate>(const QCoreVariant &v) { return v.toDate(); }
template<> QTime QVariant_to<QTime>(const QCoreVariant &v) { return v.toTime(); }
template<> QDateTime QVariant_to<QDateTime>(const QCoreVariant &v) { return v.toDateTime(); }
#ifndef QT_NO_TEMPLATE_VARIANT
template<> QList<QCoreVariant> QVariant_to<QList<QCoreVariant> >(const QCoreVariant &v)
{ return v.toList(); }
template<> QMap<QString,QCoreVariant> QVariant_to<QMap<QString,QCoreVariant> >(const QCoreVariant &v)
{ return v.toMap(); }
#endif

#endif

// takes a type, returns the internal void* pointer casted
// to a pointer of the input type
template <typename T>
inline static const T *v_cast(const QCoreVariant::Private *d)
{
    if (QTypeInfo<T>::isLarge)
        // this is really a static_cast, but gcc 2.95 complains about it.
        return reinterpret_cast<const T*>(d->data.shared->value.ptr);
    return reinterpret_cast<const T*>(&d->data.ptr);
}

#define QCONSTRUCT(vType) \
    if (QTypeInfo<vType >::isLarge) {\
        x->data.shared = new QCoreVariant::PrivateShared(new vType( \
                    *static_cast<const vType *>(copy))); \
        x->is_shared = true; \
    } else \
        new (&x->data.ptr) vType(*static_cast<const vType *>(copy))

#define QCONSTRUCT_EMPTY(vType) \
    if (QTypeInfo<vType >::isLarge) { \
        x->data.shared = new QCoreVariant::PrivateShared(new vType); \
        x->is_shared = true; \
    } else \
        new (&x->data.ptr) vType

static void construct(QCoreVariant::Private *x, const void *copy)
{
    x->is_shared = false;

    if (copy) {
        switch(x->type) {
        case QCoreVariant::String:
            QCONSTRUCT(QString);
            break;
        case QCoreVariant::StringList:
            QCONSTRUCT(QStringList);
            break;
#ifndef QT_NO_TEMPLATE_VARIANT
        case QCoreVariant::Map:
            QCONSTRUCT(QCoreVariantMap);
            break;
        case QCoreVariant::List:
            QCONSTRUCT(QCoreVariantList);
            break;
#endif
        case QCoreVariant::Date:
            QCONSTRUCT(QDate);
            break;
        case QCoreVariant::Time:
            QCONSTRUCT(QTime);
            break;
        case QCoreVariant::DateTime:
            QCONSTRUCT(QDateTime);
            break;
        case QCoreVariant::ByteArray:
            QCONSTRUCT(QByteArray);
            break;
        case QCoreVariant::BitArray:
            QCONSTRUCT(QBitArray);
            break;
        case QCoreVariant::Int:
            x->data.i = *static_cast<const int *>(copy);
            break;
        case QCoreVariant::UInt:
            x->data.u = *static_cast<const uint *>(copy);
            break;
        case QCoreVariant::Bool:
            x->data.b = *static_cast<const bool *>(copy);
            break;
        case QCoreVariant::Double:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared();
            x->data.shared->value.d = *static_cast<const double *>(copy);
            break;
        case QCoreVariant::LongLong:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared();
            x->data.shared->value.ll = *static_cast<const Q_LLONG *>(copy);
            break;
        case QCoreVariant::ULongLong:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared();
            x->data.shared->value.ull = *static_cast<const Q_ULLONG *>(copy);
            break;
        case QCoreVariant::Invalid:
        case QCoreVariant::UserType:
            break;
        default:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared(QMetaType::construct(x->type, copy));
            Q_ASSERT_X(x->data.shared->value.ptr, "QCoreVariant::construct()", "Unknown datatype");
            break;
        }
        x->is_null = false;
    } else {
        switch (x->type) {
        case QCoreVariant::Invalid:
        case QCoreVariant::UserType:
            break;
        case QCoreVariant::String:
            QCONSTRUCT_EMPTY(QString);
            break;
        case QCoreVariant::StringList:
            QCONSTRUCT_EMPTY(QStringList);
            break;
#ifndef QT_NO_TEMPLATE_VARIANT
        case QCoreVariant::Map:
            QCONSTRUCT_EMPTY(QCoreVariantMap);
            break;
        case QCoreVariant::List:
            QCONSTRUCT_EMPTY(QCoreVariantList);
            break;
#endif
        case QCoreVariant::Date:
            QCONSTRUCT_EMPTY(QDate);
            break;
        case QCoreVariant::Time:
            QCONSTRUCT_EMPTY(QTime);
            break;
        case QCoreVariant::DateTime:
            QCONSTRUCT_EMPTY(QDateTime);
            break;
        case QCoreVariant::ByteArray:
            QCONSTRUCT_EMPTY(QByteArray);
            break;
        case QCoreVariant::BitArray:
            QCONSTRUCT_EMPTY(QBitArray);
            break;
        case QCoreVariant::Int:
            x->data.i = 0;
            break;
        case QCoreVariant::UInt:
            x->data.u = 0;
            break;
        case QCoreVariant::Bool:
            x->data.b = 0;
            break;
        case QCoreVariant::Double:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared();
            x->data.shared->value.d = 0;
            break;
        case QCoreVariant::LongLong:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared();
            x->data.shared->value.ll = Q_LLONG(0);
            break;
        case QCoreVariant::ULongLong:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared();
            x->data.shared->value.ull = Q_ULLONG(0);
            break;
        default:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared(QMetaType::construct(x->type, copy));
            Q_ASSERT_X(x->data.shared->value.ptr, "QCoreVariant::construct()", "Unknown datatype");
            break;
        }
    }
}

#define QCLEAR(vType) \
    if (QTypeInfo<vType >::isLarge) {\
        delete static_cast<vType *>(d->data.shared->value.ptr); \
        delete d->data.shared; \
    } else { \
        reinterpret_cast<vType *>(&d->data.ptr)->~vType(); \
    }

static void clear(QCoreVariant::Private *d)
{
    switch (d->type) {
    case QCoreVariant::String:
        QCLEAR(QString);
        break;
    case QCoreVariant::StringList:
        QCLEAR(QStringList);
        break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::Map:
        QCLEAR(QCoreVariantMap);
        break;
    case QCoreVariant::List:
        QCLEAR(QCoreVariantList);
        break;
#endif
    case QCoreVariant::Date:
        QCLEAR(QDate);
        break;
    case QCoreVariant::Time:
        QCLEAR(QTime);
        break;
    case QCoreVariant::DateTime:
        QCLEAR(QDateTime);
        break;
    case QCoreVariant::ByteArray:
        QCLEAR(QByteArray);
        break;
    case QCoreVariant::BitArray:
        QCLEAR(QBitArray);
        break;
    case QCoreVariant::LongLong:
    case QCoreVariant::ULongLong:
    case QCoreVariant::Double:
        delete d->data.shared;
        break;
    case QCoreVariant::Invalid:
    case QCoreVariant::UserType:
    case QCoreVariant::Int:
    case QCoreVariant::UInt:
    case QCoreVariant::Bool:
        break;
    default:
        if (QMetaType::isRegistered(d->type))
            QMetaType::destroy(d->type, d->data.shared->value.ptr);
        else
            qFatal("QCoreVariant::clear: type %d unknown to QCoreVariant.", d->type);
        break;
    }

    d->type = QCoreVariant::Invalid;
    d->is_null = true;
    d->is_shared = false;
}

// used internally by construct() only
#define QISNULL(vType) \
    if (QTypeInfo<vType >::isLarge) \
        return static_cast<vType *>(d->data.shared->value.ptr)->isNull(); \
    else \
        return reinterpret_cast<const vType *>(&d->data.ptr)->isNull()

static bool isNull(const QCoreVariant::Private *d)
{
    switch(d->type) {
    case QCoreVariant::String:
        QISNULL(QString);
    case QCoreVariant::Date:
        QISNULL(QDate);
    case QCoreVariant::Time:
        QISNULL(QTime);
    case QCoreVariant::DateTime:
        QISNULL(QDateTime);
    case QCoreVariant::ByteArray:
        QISNULL(QByteArray);
    case QCoreVariant::BitArray:
        QISNULL(QBitArray);
    case QCoreVariant::StringList:
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::Map:
    case QCoreVariant::List:
#endif
    case QCoreVariant::Invalid:
    case QCoreVariant::UserType:
    case QCoreVariant::Int:
    case QCoreVariant::UInt:
    case QCoreVariant::LongLong:
    case QCoreVariant::ULongLong:
    case QCoreVariant::Bool:
    case QCoreVariant::Double:
        break;
    }
    return d->is_null;
}

#ifndef QT_NO_DATASTREAM

#define QLOAD(vType) \
    if (QTypeInfo<vType >::isLarge) \
        s >> *static_cast<vType *>(d->data.shared->value.ptr); \
    else \
        s >> *reinterpret_cast<vType *>(&d->data.ptr)

static void load(QCoreVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
    case QCoreVariant::Invalid: {
        // Since we wrote something, we should read something
        QString x;
        s >> x;
        d->is_null = true;
        break;
    }
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::Map:
        QLOAD(QCoreVariantMap);
        break;
    case QCoreVariant::List:
        QLOAD(QCoreVariantList);
        break;
#endif
    case QCoreVariant::String:
        QLOAD(QString);
        break;
    case QCoreVariant::StringList:
        QLOAD(QStringList);
        break;
    case QCoreVariant::Int:
        s >> d->data.i;
        break;
    case QCoreVariant::UInt:
        s >> d->data.u;
        break;
    case QCoreVariant::LongLong:
        s >> d->data.shared->value.ll;
        break;
    case QCoreVariant::ULongLong:
        s >> d->data.shared->value.ull;
        break;
    case QCoreVariant::Bool: {
        Q_INT8 x;
        s >> x;
        d->data.b = x;
    }
        break;
    case QCoreVariant::Double:
        s >> d->data.shared->value.d;
        break;
    case QCoreVariant::Date:
        QLOAD(QDate);
        break;
    case QCoreVariant::Time:
        QLOAD(QTime);
        break;
    case QCoreVariant::DateTime:
        QLOAD(QDateTime);
        break;
    case QCoreVariant::ByteArray:
        QLOAD(QByteArray);
        break;
    case QCoreVariant::BitArray:
        QLOAD(QBitArray);
        break;
    default:
        qFatal("QCoreVariant::load: type %d unknown to QCoreVariant.", d->type);
    }
}

#define QSAVE(vType) \
    if (QTypeInfo<vType >::isLarge) \
        s << *static_cast<vType *>(d->data.shared->value.ptr); \
    else \
        s << *reinterpret_cast<const vType *>(&d->data.ptr)

static void save(const QCoreVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::List:
        QSAVE(QCoreVariantList);
        break;
    case QCoreVariant::Map:
        QSAVE(QCoreVariantMap);
        break;
#endif
    case QCoreVariant::String:
        QSAVE(QString);
        break;
    case QCoreVariant::StringList:
        QSAVE(QStringList);
        break;
    case QCoreVariant::Int:
        s << d->data.i;
        break;
    case QCoreVariant::UInt:
        s << d->data.u;
        break;
    case QCoreVariant::LongLong:
        s << d->data.shared->value.ll;
        break;
    case QCoreVariant::ULongLong:
        s << d->data.shared->value.ull;
        break;
    case QCoreVariant::Bool:
        s << (Q_INT8)d->data.b;
        break;
    case QCoreVariant::Double:
        s << d->data.shared->value.d;
        break;
    case QCoreVariant::Date:
        QSAVE(QDate);
        break;
    case QCoreVariant::Time:
        QSAVE(QTime);
        break;
    case QCoreVariant::DateTime:
        QSAVE(QDateTime);
        break;
    case QCoreVariant::ByteArray:
        QSAVE(QByteArray);
        break;
    case QCoreVariant::BitArray:
        QSAVE(QBitArray);
        break;
    case QCoreVariant::Invalid:
        s << QString();
        break;
    default:
        qFatal("QCoreVariant::save: type %d unknown to QCoreVariant.", d->type);
    }
}
#endif // QT_NO_DATASTREAM

#define QCOMPARE(vType) \
    if (QTypeInfo<vType >::isLarge) \
        return *static_cast<vType *>(a->data.shared->value.ptr) == \
            *static_cast<vType *>(b->data.shared->value.ptr); \
    else \
        return *reinterpret_cast<const vType *>(&a->data.ptr) \
            == *reinterpret_cast<const vType *>(&b->data.ptr);

static bool compare(const QCoreVariant::Private *a, const QCoreVariant::Private *b)
{
    switch(a->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::List:
        QCOMPARE(QCoreVariantList);
    case QCoreVariant::Map: {
        const QCoreVariantMap *m1 = v_cast<QCoreVariantMap>(a);
        const QCoreVariantMap *m2 = v_cast<QCoreVariantMap>(b);
        if (m1->count() != m2->count())
            return false;
        QCoreVariantMap::ConstIterator it = m1->constBegin();
        QCoreVariantMap::ConstIterator it2 = m2->constBegin();
        while (it != m1->constEnd()) {
            if (*it != *it2)
                return false;
            ++it;
            ++it2;
        }
        return true;
    }
#endif
    case QCoreVariant::String:
        QCOMPARE(QString);
    case QCoreVariant::StringList:
        QCOMPARE(QStringList);
    case QCoreVariant::Int:
        return a->data.i == b->data.i;
    case QCoreVariant::UInt:
        return a->data.u == b->data.u;
    case QCoreVariant::LongLong:
        return a->data.shared->value.ll == b->data.shared->value.ll;
    case QCoreVariant::ULongLong:
        return a->data.shared->value.ull == b->data.shared->value.ull;
    case QCoreVariant::Bool:
        return a->data.b == b->data.b;
    case QCoreVariant::Double:
        return a->data.shared->value.d == b->data.shared->value.d;
    case QCoreVariant::Date:
        QCOMPARE(QDate);
    case QCoreVariant::Time:
        QCOMPARE(QTime);
    case QCoreVariant::DateTime:
        QCOMPARE(QDateTime);
    case QCoreVariant::ByteArray:
        QCOMPARE(QByteArray);
    case QCoreVariant::BitArray:
        QCOMPARE(QBitArray);
    case QCoreVariant::Invalid:
        return true;
    default:
        if (!QMetaType::isRegistered(a->type))
            qFatal("QCoreVariant::compare: type %d unknown to QCoreVariant.", a->type);
        return a->data.shared->value.ptr == b->data.shared->value.ptr;
    }
    return false;
}

static void cast(const QCoreVariant::Private *d, QCoreVariant::Type t, void *result, bool *ok)
{
    Q_ASSERT(d->type != uint(t));
    switch (t) {
    case QCoreVariant::String: {
        QString *str = static_cast<QString *>(result);
        switch (d->type) {
        case QCoreVariant::Int:
            *str = QString::number(d->data.i);
            break;
        case QCoreVariant::UInt:
            *str = QString::number(d->data.u);
            break;
        case QCoreVariant::LongLong:
            *str = QString::number(d->data.shared->value.ll);
            break;
        case QCoreVariant::ULongLong:
            *str = QString::number(d->data.shared->value.ull);
            break;
        case QCoreVariant::Double:
            *str = QString::number(d->data.shared->value.d, 'g', DBL_DIG);
            break;
#if !defined(QT_NO_SPRINTF) && !defined(QT_NO_DATESTRING)
        case QCoreVariant::Date:
            *str = v_cast<QDate>(d)->toString(Qt::ISODate);
            break;
        case QCoreVariant::Time:
            *str = v_cast<QTime>(d)->toString(Qt::ISODate);
            break;
        case QCoreVariant::DateTime:
            *str = v_cast<QDateTime>(d)->toString(Qt::ISODate);
            break;
#endif
        case QCoreVariant::Bool:
            *str = QLatin1String(d->data.b ? "true" : "false");
            break;
        case QCoreVariant::ByteArray:
            *str = QString::fromAscii(v_cast<QByteArray>(d)->constData());
            break;
        case QCoreVariant::StringList:
            if (v_cast<QStringList>(d)->count() == 1)
                *str = v_cast<QStringList>(d)->at(0);
            break;
        default:
            break;
        }
        break;
    }
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::StringList:
        if (d->type == QCoreVariant::List) {
            QStringList *slst = static_cast<QStringList *>(result);
            const QCoreVariantList *list = v_cast<QCoreVariantList >(d);
            for (int i = 0; i < list->size(); ++i)
                slst->append(list->at(i).toString());
        } else if (d->type == QCoreVariant::String) {
            QStringList *slst = static_cast<QStringList *>(result);
            *slst = *v_cast<QString>(d);
        }
#endif
        break;
    case QCoreVariant::Date: {
        QDate *dt = static_cast<QDate *>(result);
        if (d->type == QCoreVariant::DateTime)
            *dt = v_cast<QDateTime>(d)->date();
#ifndef QT_NO_DATESTRING
        else if (d->type == QCoreVariant::String)
            *dt = QDate::fromString(*v_cast<QString>(d), Qt::ISODate);
#endif
        break;
    }
    case QCoreVariant::Time: {
        QTime *t = static_cast<QTime *>(result);
        switch (d->type) {
        case QCoreVariant::DateTime:
            *t = v_cast<QDateTime>(d)->time();
            break;
#ifndef QT_NO_DATESTRING
        case QCoreVariant::String:
            *t = QTime::fromString(*v_cast<QString>(d), Qt::ISODate);
            break;
#endif
        default:
            break;
        }
        break;
    }
    case QCoreVariant::DateTime: {
        QDateTime *dt = static_cast<QDateTime *>(result);
        switch (d->type) {
#ifndef QT_NO_DATESTRING
        case QCoreVariant::String:
            *dt = QDateTime::fromString(*v_cast<QString>(d), Qt::ISODate);
            break;
#endif
        case QCoreVariant::Date:
            *dt = QDateTime(*v_cast<QDate>(d));
            break;
        default:
            break;
        }
        break;
    }
    case QCoreVariant::ByteArray: {
        QByteArray *ba = static_cast<QByteArray *>(result);
        if (d->type == QCoreVariant::String)
            *ba = v_cast<QString>(d)->toAscii();
    }
    break;
    case QCoreVariant::Int: {
        int *i = static_cast<int *>(result);
        switch (d->type) {
        case QCoreVariant::String:
            *i = v_cast<QString>(d)->toInt(ok);
            break;
        case QCoreVariant::ByteArray:
            *i = v_cast<QByteArray>(d)->toInt(ok);
            break;
        case QCoreVariant::Int:
            *i = d->data.i;
            break;
        case QCoreVariant::UInt:
            *i = (int)d->data.u;
            break;
        case QCoreVariant::LongLong:
            *i = (int)d->data.shared->value.ll;
            break;
        case QCoreVariant::ULongLong:
            *i = (int)d->data.shared->value.ull;
            break;
        case QCoreVariant::Double:
            *i = (int)d->data.shared->value.d;
            break;
        case QCoreVariant::Bool:
            *i = (int)d->data.b;
            break;
        default:
            *i = 0;
            break;
        }
        break;
    }
    case QCoreVariant::UInt: {
        uint *u = static_cast<uint *>(result);
        switch (d->type) {
        case QCoreVariant::String:
            *u = v_cast<QString>(d)->toUInt(ok);
            break;
        case QCoreVariant::ByteArray:
            *u = v_cast<QByteArray>(d)->toUInt(ok);
            break;
        case QCoreVariant::Int:
            *u = (uint)d->data.i;
            break;
        case QCoreVariant::UInt:
            *u = d->data.u;
            break;
        case QCoreVariant::LongLong:
            *u = (uint)d->data.shared->value.ll;
            break;
        case QCoreVariant::ULongLong:
            *u = (uint)d->data.shared->value.ull;
            break;
        case QCoreVariant::Double:
            *u = (uint)d->data.shared->value.d;
            break;
        case QCoreVariant::Bool:
            *u = (uint)d->data.b;
            break;
        default:
            *u = 0;
            break;
        }
        break;
    }
    case QCoreVariant::LongLong: {
        Q_LLONG *l = static_cast<Q_LLONG *>(result);
        switch (d->type) {
        case QCoreVariant::String:
            *l = v_cast<QString>(d)->toLongLong(ok);
            break;
        case QCoreVariant::ByteArray:
            *l = v_cast<QByteArray>(d)->toLongLong(ok);
            break;
        case QCoreVariant::Int:
            *l = (Q_LLONG)d->data.i;
            break;
        case QCoreVariant::UInt:
            *l = (Q_LLONG)d->data.u;
            break;
        case QCoreVariant::LongLong:
            *l = d->data.shared->value.ll;
            break;
        case QCoreVariant::ULongLong:
            *l = (Q_LLONG)d->data.shared->value.ull;
            break;
        case QCoreVariant::Double:
            *l = (Q_LLONG)d->data.shared->value.d;
            break;
        case QCoreVariant::Bool:
            *l = (Q_LLONG)d->data.b;
            break;
        default:
            *l = 0;
            break;
        }
        break;
    }
    case QCoreVariant::ULongLong: {
        Q_ULLONG *l = static_cast<Q_ULLONG *>(result);
        switch (d->type) {
        case QCoreVariant::Int:
            *l = (Q_ULLONG)d->data.i;
            break;
        case QCoreVariant::UInt:
            *l = (Q_ULLONG)d->data.u;
            break;
        case QCoreVariant::LongLong:
            *l = (Q_ULLONG)d->data.shared->value.ll;
            break;
        case QCoreVariant::ULongLong:
            *l = d->data.shared->value.ull;
            break;
        case QCoreVariant::Double:
            *l = (Q_ULLONG)d->data.shared->value.d;
            break;
        case QCoreVariant::Bool:
            *l = (Q_ULLONG)d->data.b;
            break;
        case QCoreVariant::String:
            *l = v_cast<QString>(d)->toULongLong(ok);
            break;
        case QCoreVariant::ByteArray:
            *l = v_cast<QByteArray>(d)->toULongLong(ok);
            break;
        default:
            *l = 0;
            break;
        }
        break;
    }
    case QCoreVariant::Bool: {
        bool *b = static_cast<bool *>(result);
        switch(d->type) {
        case QCoreVariant::Double:
            *b = d->data.shared->value.d != 0.0;
            break;
        case QCoreVariant::Int:
            *b = d->data.i != 0;
            break;
        case QCoreVariant::UInt:
            *b = d->data.u != 0;
            break;
        case QCoreVariant::LongLong:
            *b = d->data.shared->value.ll != 0;
            break;
        case QCoreVariant::ULongLong:
            *b = d->data.shared->value.ull != 0;
            break;
        case QCoreVariant::String:
        {
            QString str = v_cast<QString>(d)->toLower();
            *b = !(str == QLatin1String("0") || str == QLatin1String("false") || str.isEmpty());
            break;
        }
        default:
            *b = false;
            break;
        }
        break;
    }
    case QCoreVariant::Double: {
        double *f = static_cast<double *>(result);
        switch (d->type) {
        case QCoreVariant::String:
            *f = v_cast<QString>(d)->toDouble(ok);
            break;
        case QCoreVariant::ByteArray:
            *f = v_cast<QByteArray>(d)->toDouble(ok);
            break;
        case QCoreVariant::Double:
            *f = d->data.shared->value.d;
            break;
        case QCoreVariant::Int:
            *f = (double)d->data.i;
            break;
        case QCoreVariant::Bool:
            *f = (double)d->data.b;
            break;
        case QCoreVariant::UInt:
            *f = (double)d->data.u;
            break;
        case QCoreVariant::LongLong:
            *f = (double)d->data.shared->value.ll;
            break;
        case QCoreVariant::ULongLong:
#if defined(Q_CC_MSVC) && !defined(Q_CC_MSVC_NET)
            *f = (double)(Q_LLONG)d->data.shared->value.ull;
#else
            *f = (double)d->data.shared->value.ull;
#endif
            break;
        default:
            *f = 0.0;
            break;
        }
        break;
    }
    case QCoreVariant::List:
        if (d->type == QCoreVariant::StringList) {
            QCoreVariantList *lst = static_cast<QCoreVariantList *>(result);
            const QStringList *slist = v_cast<QStringList>(d);
            for (int i = 0; i < slist->size(); ++i)
                lst->append(QCoreVariant(slist->at(i)));
        }
        break;

    default:
        Q_ASSERT(0);
    }
}

static bool canCast(const QCoreVariant::Private *d, QCoreVariant::Type t)
{
    if (d->type == (uint)t)
        return true;

    switch (t) {
    case QCoreVariant::Bool:
        return d->type == QCoreVariant::Double || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::UInt || d->type == QCoreVariant::LongLong
            || d->type == QCoreVariant::ULongLong || d->type == QCoreVariant::String;
    case QCoreVariant::Int:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::UInt
            || d->type == QCoreVariant::LongLong || d->type == QCoreVariant::ULongLong;
    case QCoreVariant::UInt:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::LongLong || d->type == QCoreVariant::ULongLong;
    case QCoreVariant::LongLong:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::UInt || d->type == QCoreVariant::ULongLong;
    case QCoreVariant::ULongLong:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::UInt || d->type == QCoreVariant::LongLong;
    case QCoreVariant::Double:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::UInt
            || d->type == QCoreVariant::LongLong || d->type == QCoreVariant::ULongLong;
    case QCoreVariant::String:
        if (d->type == QCoreVariant::StringList && v_cast<QStringList>(d)->count() == 1)
            return true;
        return d->type == QCoreVariant::ByteArray || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::UInt || d->type == QCoreVariant::Bool
            || d->type == QCoreVariant::Double || d->type == QCoreVariant::Date
            || d->type == QCoreVariant::Time || d->type == QCoreVariant::DateTime
            || d->type == QCoreVariant::LongLong || d->type == QCoreVariant::ULongLong;
    case QCoreVariant::ByteArray:
        return
#ifdef QT_COMPAT
            d->type == QCoreVariant::CString ||
#endif
            d->type == QCoreVariant::String;
    case QCoreVariant::Date:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::DateTime;
    case QCoreVariant::Time:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::DateTime;
    case QCoreVariant::DateTime:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Date;
    case QCoreVariant::List:
        return d->type == QCoreVariant::StringList;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::StringList:
        if (d->type == QCoreVariant::List) {
            const QCoreVariantList &varlist = *v_cast<QCoreVariantList >(d);
            for (int i = 0; i < varlist.size(); ++i) {
                if (!varlist.at(i).canCast(QCoreVariant::String))
                    return false;
            }
            return true;
        } else if (d->type == QCoreVariant::String) {
            return true;
        }
        return false;
#endif
    default:
        return false;
    }
}

const QCoreVariant::Handler qt_kernel_variant_handler = {
    construct,
    clear,
    isNull,
#ifndef QT_NO_DATASTREAM
    load,
    save,
#endif
    compare,
    cast,
    canCast
};

Q_CORE_EXPORT const QCoreVariant::Handler *qcoreVariantHandler()
{
    return &qt_kernel_variant_handler;
}


const QCoreVariant::Handler *QCoreVariant::handler = &qt_kernel_variant_handler;

/*!
    \class QCoreVariant qvariant.h
    \brief The QCoreVariant class acts like a union for the most common Qt data types.

    \ingroup objectmodel
    \ingroup misc
    \mainclass

    Because C++ forbids unions from including types that have
    non-default constructors or destructors, most interesting Qt
    classes cannot be used in unions. Without QCoreVariant, this would be
    a problem for QObject::property() and for database work, etc.

    A QCoreVariant object holds a single value of a single type() at a
    time. (Some type()s are multi-valued, for example a string list.)
    You can find out what type, T, the variant holds, convert it to a
    different type using one of the asT() functions, e.g. asSize(),
    get its value using one of the toT() functions, e.g. toSize(), and
    check whether the type can be converted to a particular type using
    canCast().

    The methods named toT() (for any supported T, see the \c Type
    documentation for a list) are const. If you ask for the stored
    type, they return a copy of the stored object. If you ask for a
    type that can be generated from the stored type, toT() copies and
    converts and leaves the object itself unchanged. If you ask for a
    type that cannot be generated from the stored type, the result
    depends on the type (see the function documentation for details).

    Note that two data types supported by QCoreVariant are explicitly
    shared, namely QImage and QPointArray, and in these
    cases the toT() methods return a shallow copy. In almost all cases
    you must make a deep copy of the returned values before modifying
    them.

    The asT() functions are not const. They do conversion like the
    toT() methods, set the variant to hold the converted value, and
    return a reference to the new contents of the variant.

    Here is some example code to demonstrate the use of QCoreVariant:

    \code
    QDataStream out(...);
    QCoreVariant v(123);            // The variant now contains an int
    int x = v.toInt();              // x = 123
    out << v;                       // Writes a type tag and an int to out
    v = QCoreVariant("hello");      // The variant now contains a QByteArray
    v = QCoreVariant(tr("hello"));  // The variant now contains a QString
    int y = v.toInt();              // y = 0 since v cannot be converted to an int
    QString s = v.toString();       // s = tr("hello")  (see QObject::tr())
    out << v;                       // Writes a type tag and a QString to out
    ...
    QDataStream in(...);            // (opening the previously written stream)
    in >> v;                        // Reads an Int variant
    int z = v.toInt();              // z = 123
    qDebug("Type is %s",            // prints "Type is int"
            v.typeName());
    v.asInt() += 100;               // The variant now hold the value 223.
    v = QCoreVariant(QStringList());
    v.asStringList().append("Hello");
    \endcode

    You can even store QCoreVariantLists and
    QMap<QString,QCoreVariant>s in a variant, so you can easily construct
    arbitrarily complex data structures of arbitrary types. This is
    very powerful and versatile, but may prove less memory and speed
    efficient than storing specific types in standard data structures.

    QCoreVariant also supports the notion of NULL values, where you have a
    defined type with no value set.
    \code
    QCoreVariant x, y(QString()), z(QString(""));
    x.asInt();
    // x.isNull() == true,
    // y.isNull() == true, z.isNull() == false
    // y.isEmpty() == true, z.Empty() == true
    \endcode
*/

/*!
    \enum QCoreVariant::Type

    This enum type defines the types of variable that a QCoreVariant can
    contain.

    \value Invalid  no type
    \value BitArray  a QBitArray
    \value ByteArray  a QByteArray
    \value Bitmap  a QBitmap
    \value Bool  a bool
    \value Brush  a QBrush
    \value Color  a QColor
    \value Cursor  a QCursor
    \value Date  a QDate
    \value DateTime  a QDateTime
    \value Double  a double
    \value Font  a QFont
    \value Icon  a QIcon
    \value Image  a QImage
    \value Int  an int
    \value KeySequence  a QKeySequence
    \value List  a QCoreVariantList
    \value LongLong a long long
    \value ULongLong an unsigned long long
    \value Map  a QMap<QString,QCoreVariant>
    \value Palette  a QPalette
    \value Pen  a QPen
    \value Pixmap  a QPixmap
    \value Point  a QPoint
    \value PointArray  a QPointArray
    \value Rect  a QRect
    \value Region  a QRegion
    \value Size  a QSize
    \value SizePolicy  a QSizePolicy
    \value String  a QString
    \value StringList  a QStringList
    \value Time  a QTime
    \value UInt  an unsigned int

    \value UserType

    \omitvalue CString
    \omitvalue ColorGroup
    \omitvalue IconSet
    \omitvalue LastType

    Note that Qt's definition of bool depends on the compiler.
    \c qglobal.h has the system-dependent definition of bool.
*/

/*!
  \fn QCoreVariant::QCoreVariant()

    Constructs an invalid variant.
*/


/*!
    \fn QCoreVariant::QCoreVariant(int typeOrUserType, const void *copy)

    Constructs variant of type \a typeOrUserType, and initializes with
    \a copy if \a copy is not 0.
*/

/*!
    \fn QCoreVariant::QCoreVariant(Type type)

    Constructs a null variant of type \a type.
*/



/*!
    \fn QCoreVariant::create(int type, const void *copy)

    \internal

    Constructs a variant private of type \a type, and initializes with \a copy if
    \a copy is not 0.
*/

void QCoreVariant::create(int type, const void *copy)
{
    d.type = type;
    d.is_null = true;
    handler->construct(&d, copy);
}

/*!
  \fn QCoreVariant::~QCoreVariant()

    Destroys the QCoreVariant and the contained object.

    Note that subclasses that reimplement clear() should reimplement
    the destructor to call clear(). This destructor calls clear(), but
    because it is the destructor, QCoreVariant::clear() is called rather
    than a subclass's clear().
*/

QCoreVariant::~QCoreVariant()
{
    if (!d.is_shared || !--d.data.shared->ref)
        handler->clear(&d);
}

/*!
  \fn QCoreVariant::QCoreVariant(const QCoreVariant &p)

    Constructs a copy of the variant, \a p, passed as the argument to
    this constructor. Usually this is a deep copy, but a shallow copy
    is made if the stored data type is explicitly shared, as e.g.
    QImage is.
*/

QCoreVariant::QCoreVariant(const QCoreVariant &p)
{
    d.type = p.d.type;
    d.is_null = p.d.is_null;
    d.is_shared = p.d.is_shared;
    if (d.is_shared) {
        d.data.shared = p.d.data.shared;
        ++d.data.shared->ref;
    } else {
        construct(&d, p.constData());
    }
}

#ifndef QT_NO_DATASTREAM
/*!
    Reads the variant from the data stream, \a s.
*/
QCoreVariant::QCoreVariant(QDataStream &s)
{
    d.is_null = true;
    s >> *this;
}
#endif //QT_NO_DATASTREAM

/*!
  \fn QCoreVariant::QCoreVariant(const QString &val)

    Constructs a new variant with a string value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const QLatin1String &val)

    Constructs a new variant with a string value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const char *val)

    Constructs a new variant with a C-string value of \a val if \a val
    is non-null. The variant creates a deep copy of \a val.

    If \a val is null, the resulting variant has type Invalid.
*/


QCoreVariant::QCoreVariant(const char *val)
{
    QString s = QString::fromLatin1(val);
    create(String, &s);
}

/*!
  \fn QCoreVariant::QCoreVariant(const QStringList &val)

    Constructs a new variant with a string list value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const QMap<QString,QCoreVariant> &val)

    Constructs a new variant with a map of QCoreVariants, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const QDate &val)

    Constructs a new variant with a date value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const QTime &val)

    Constructs a new variant with a time value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const QDateTime &val)

    Constructs a new variant with a date/time value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const QByteArray &val)

    Constructs a new variant with a bytearray value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(const QBitArray &val)

    Constructs a new variant with a bitarray value, \a val.
*/


/*!
  \fn QCoreVariant::QCoreVariant(int val)

    Constructs a new variant with an integer value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(uint val)

    Constructs a new variant with an unsigned integer value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(Q_LLONG val)

    Constructs a new variant with a long long integer value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(Q_ULLONG val)

    Constructs a new variant with an unsigned long long integer value, \a val.
*/


/*!
  \fn QCoreVariant::QCoreVariant(bool val)

    Constructs a new variant with a boolean value, \a val. The integer
    argument is a dummy, necessary for compatibility with some
    compilers.
*/


/*!
  \fn QCoreVariant::QCoreVariant(double val)

    Constructs a new variant with a floating point value, \a val.
*/

/*!
    \fn QCoreVariant::QCoreVariant(const QList<QCoreVariant> &val)

    Constructs a new variant with a list value, \a val.
*/


QCoreVariant::QCoreVariant(Type type)
{ create(type, 0); }
QCoreVariant::QCoreVariant(int typeOrUserType, const void *copy)
{ create(typeOrUserType, copy); d.is_null = false; }
QCoreVariant::QCoreVariant(int val)
{ create(Int, &val); }
QCoreVariant::QCoreVariant(uint val)
{ create(UInt, &val); }
QCoreVariant::QCoreVariant(Q_LLONG val)
{ create(LongLong, &val); }
QCoreVariant::QCoreVariant(Q_ULLONG val)
{ create(ULongLong, &val); }
QCoreVariant::QCoreVariant(bool val)
{ create(Bool, &val); }
QCoreVariant::QCoreVariant(double val)
{ create(Double, &val); }

QCoreVariant::QCoreVariant(const QByteArray &val)
{ create(ByteArray, &val); }
QCoreVariant::QCoreVariant(const QBitArray &val)
{ create(BitArray, &val); }
QCoreVariant::QCoreVariant(const QString &val)
{ create(String, &val); }
QCoreVariant::QCoreVariant(const QLatin1String &val)
{ QString str(val); create(String, &str); }
QCoreVariant::QCoreVariant(const QStringList &val)
{ create(StringList, &val); }

QCoreVariant::QCoreVariant(const QDate &val)
{ create(Date, &val); }
QCoreVariant::QCoreVariant(const QTime &val)
{ create(Time, &val); }
QCoreVariant::QCoreVariant(const QDateTime &val)
{ create(DateTime, &val); }
#ifndef QT_NO_TEMPLATE_VARIANT
QCoreVariant::QCoreVariant(const QList<QCoreVariant> &list)
{ create(List, &list); }
QCoreVariant::QCoreVariant(const QMap<QString,QCoreVariant> &map)
{ create(Map, &map); }
#endif

/*!
    Returns the storage type of the value stored in the variant.
    Usually it's best to test with canCast() whether the variant can
    deliver the data type you are interested in.
*/

QCoreVariant::Type QCoreVariant::type() const
{
    return d.type >= QMetaType::User ? UserType : static_cast<Type>(d.type);
}

/*!
    Returns the storage type of the value stored in the variant. For
    non-user types, this is the same as type().

    \sa type()
*/

int QCoreVariant::userType() const
{
    return d.type;
}

/*!
    Assigns the value of the variant \a variant to this variant.

    This is a deep copy of the variant, but note that if the variant
    holds an explicitly shared type such as QImage, a shallow copy is
    performed.
*/
QCoreVariant& QCoreVariant::operator=(const QCoreVariant &variant)
{
    if (this == &variant)
        return *this;

    clear();
    if (variant.d.is_shared) {
        ++variant.d.data.shared->ref;
        d = variant.d;
    } else {
        d.type = variant.d.type;
        handler->construct(&d, variant.constData());
        d.is_null = variant.d.is_null;
    }

    return *this;
}

/*!
    \fn void QCoreVariant::detach()

    \internal
*/

void QCoreVariant::detach()
{
    if (!d.is_shared || d.data.shared->ref == 1)
        return;

    Private dd;
    dd.type = d.type;
    handler->construct(&dd, constData());
    dd.data.shared = qAtomicSetPtr(&d.data.shared, dd.data.shared);
    if (!--dd.data.shared->ref)
        handler->clear(&dd);
}

/*!
    \fn bool QCoreVariant::isDetached() const

    \internal
*/

/*!
    Returns the name of the type stored in the variant. The returned
    strings describe the C++ datatype used to store the data: for
    example, "QFont", "QString", or "QCoreVariantList". An Invalid
    variant returns 0.
*/
const char *QCoreVariant::typeName() const
{
    return typeToName((Type)d.type);
}

/*!
    Convert this variant to type Invalid and free up any resources
    used.
*/
void QCoreVariant::clear()
{
    if (!d.is_shared || !--d.data.shared->ref)
        handler->clear(&d);
    d.type = Invalid;
    d.is_null = true;
    d.is_shared = false;
}

/* Attention!

   For dependency reasons, this table is duplicated in moc's
   generator.cpp. If you change one, change both.

   (Search for the word 'Attention' in generator.cpp)
*/
enum { ntypes = 36 };
static const char* const type_map[ntypes] =
{
    0,
    "QVariantMap",
    "QVariantList",
    "QString",
    "QStringList",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
#ifdef QT_COMPAT
    "QColorGroup",
#else
    "",
#endif
    "QIcon",
    "QPoint",
    "QImage",
    "int",
    "uint",
    "bool",
    "double",
    "",
    "QPointArray",
    "QRegion",
    "QBitmap",
    "QCursor",
    "QSizePolicy",
    "QDate",
    "QTime",
    "QDateTime",
    "QByteArray",
    "QBitArray",
    "QKeySequence",
    "QPen",
    "Q_LLONG",
    "Q_ULLONG",
    "UserType"
};


/*!
    Converts the enum representation of the storage type, \a typ, to
    its string representation.
*/
const char *QCoreVariant::typeToName(Type typ)
{
    if ((int)typ >= ntypes)
        return QMetaType::typeName(typ);
    return type_map[typ];
}


/*!
    Converts the string representation of the storage type given in \a
    name, to its enum representation.

    If the string representation cannot be converted to any enum
    representation, the variant is set to \c Invalid.
*/
QCoreVariant::Type QCoreVariant::nameToType(const char *name)
{
    if (name) {
        if (strcmp(name, "") == 0)
            return Invalid;
        if (strcmp(name, "QCString") == 0)
            return ByteArray;
        for (int i = 1; i < ntypes; ++i) {
            if (strcmp(type_map[i], name) == 0)
                return (Type)i;
        }
    }
    if (QMetaType::type(name))
        return QCoreVariant::UserType;
    return Invalid;
}

#ifndef QT_NO_DATASTREAM
/*!
    Internal function for loading a variant from stream \a s. Use the
    stream operators instead.

    \internal
*/
void QCoreVariant::load(QDataStream &s)
{
    clear();

    Q_UINT32 u;
    s >> u;
    create(static_cast<Type>(u), 0);
    d.is_null = false;
    handler->load(&d, s);
}

/*!
    Internal function for saving a variant to the stream \a s. Use the
    stream operators instead.

    \internal
*/
void QCoreVariant::save(QDataStream &s) const
{
    s << (Q_UINT32)type();
    handler->save(&d, s);
}

/*!
    Reads a variant \a p from the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator>>(QDataStream &s, QCoreVariant &p)
{
    p.load(s);
    return s;
}

/*!
    Writes a variant \a p to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator<<(QDataStream &s, const QCoreVariant &p)
{
    p.save(s);
    return s;
}

/*!
    Reads a variant type \a p in enum representation from the stream \a s.
*/
QDataStream& operator>>(QDataStream &s, QCoreVariant::Type &p)
{
    Q_UINT32 u;
    s >> u;
    p = (QCoreVariant::Type)u;

    return s;
}

/*!
    Writes a variant type \a p to the stream \a s.
*/
QDataStream& operator<<(QDataStream &s, const QCoreVariant::Type p)
{
    s << (Q_UINT32)p;

    return s;
}

#endif //QT_NO_DATASTREAM

/*!
    \fn bool QCoreVariant::isValid() const

    Returns true if the storage type of this variant is not
    QCoreVariant::Invalid; otherwise returns false.
*/

#define Q_VARIANT_TO(f) \
Q##f QCoreVariant::to##f() const { \
    if (d.type == f) \
        return *v_cast<Q##f >(&d); \
    Q##f ret; \
    handler->cast(&d, f, &ret, 0); \
    return ret; \
}

Q_VARIANT_TO(StringList)
Q_VARIANT_TO(Date)
Q_VARIANT_TO(Time)
Q_VARIANT_TO(DateTime)
Q_VARIANT_TO(ByteArray)

/*!
  \fn QString QCoreVariant::toString() const

    Returns the variant as a QString if the variant has type() String,
    ByteArray, Int, Uint, Bool, Double, Date, Time, DateTime,
    KeySequence, Font or Color; otherwise returns an empty string.
*/


/*!
  \fn QStringList QCoreVariant::toStringList() const

    Returns the variant as a QStringList if the variant has type()
    StringList or List of a type that can be converted to QString;
    otherwise returns an empty list.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myVariant.toStringList();
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode
*/



QString QCoreVariant::toString() const
{
    if (d.type == String)
        return *reinterpret_cast<const QString *>(&d.data.ptr);

    QString ret;
    handler->cast(&d, String, &ret, 0);
    return ret;
}
#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant as a QMap<QString,QCoreVariant> if the variant has
    type() Map; otherwise returns an empty map.

    Note that if you want to iterate over the map, you should iterate
    over a copy, e.g.
    \code
    QCoreVariantMap map = myVariant.toMap();
    QCoreVariantMap::Iterator it = map.begin();
    while(it != map.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode
*/
QCoreVariantMap QCoreVariant::toMap() const
{
    if (d.type != Map)
        return QMap<QString,QCoreVariant>();

    return *v_cast<QCoreVariantMap>(&d);
}
#endif

/*!
  \fn QDate QCoreVariant::toDate() const

    Returns the variant as a QDate if the variant has type() Date,
    DateTime or String; otherwise returns an invalid date.

    Note that if the type() is String an invalid date will be returned
    if the string cannot be parsed as a Qt::ISODate format date.
*/


/*!
  \fn QTime QCoreVariant::toTime() const

    Returns the variant as a QTime if the variant has type() Time,
    DateTime or String; otherwise returns an invalid time.

    Note that if the type() is String an invalid time will be returned
    if the string cannot be parsed as a Qt::ISODate format time.
*/

/*!
  \fn QDateTime QCoreVariant::toDateTime() const

    Returns the variant as a QDateTime if the variant has type()
    DateTime, Date or String; otherwise returns an invalid date/time.

    Note that if the type() is String an invalid date/time will be
    returned if the string cannot be parsed as a Qt::ISODate format
    date/time.
*/


/*!
  \fn QByteArray QCoreVariant::toByteArray() const

    Returns the variant as a QByteArray if the variant has type()
    ByteArray; otherwise returns an empty bytearray.
*/

/*!
    Returns the variant as a QBitArray if the variant has type()
    BitArray; otherwise returns an empty bitarray.
*/
QBitArray QCoreVariant::toBitArray() const
{
    if (d.type == BitArray)
        return *v_cast<QBitArray>(&d);
    return QBitArray();
}

/*!
    Returns the variant as an int if the variant has type() String,
    Int, UInt, Double, Bool or KeySequence; otherwise returns
    0.

    If \a ok is non-null: \c{*}\a{ok} is set to true if the value could be
    converted to an int; otherwise \c{*}\a{ok} is set to false.

    \sa canCast()
*/
int QCoreVariant::toInt(bool *ok) const
{
    if (d.type == Int) {
        if (ok)
            *ok = true;
        return d.data.i;
    }

    bool c = canCast(Int);
    if (ok)
        *ok = c;
    int res = 0;
    if (c)
        handler->cast(&d, Int, &res, ok);

    return res;
}

/*!
    Returns the variant as an unsigned int if the variant has type()
    String, ByteArray, UInt, Int, Double, or Bool; otherwise returns 0.

    If \a ok is non-null: \c{*}\a{ok} is set to true if the value could be
    converted to an unsigned int; otherwise \c{*}\a{ok} is set to false.
*/
uint QCoreVariant::toUInt(bool *ok) const
{
    if (d.type == UInt) {
        if (ok)
            *ok = true;
        return d.data.u;
    }

    bool c = canCast(UInt);
    if (ok)
        *ok = c;
    uint res = 0;
    if (c)
        handler->cast(&d, UInt, &res, ok);

    return res;
}

/*!
    Returns the variant as a long long int if the variant has type()
    LongLong, ULongLong, any type allowing a toInt() conversion;
    otherwise returns 0.

    If \a ok is non-null: \c{*}\c{ok} is set to true if the value could be
    converted to an int; otherwise \c{*}\c{ok} is set to false.

    \sa canCast()
*/
Q_LLONG QCoreVariant::toLongLong(bool *ok) const
{
    if (d.type == LongLong) {
        if (ok)
            *ok = true;
        return d.data.shared->value.ll;
    }

    bool c = canCast(LongLong);
    if (ok)
        *ok = c;
    Q_LLONG res = 0;
    if (c)
        handler->cast(&d, LongLong, &res, ok);

    return res;
}

/*!
    Returns the variant as as an unsigned long long int if the variant
    has type() LongLong, ULongLong, any type allowing a toUInt()
    conversion; otherwise returns 0.

    If \a ok is non-null: \c{*}\a{ok} is set to true if the value could be
    converted to an int; otherwise \c{*}\a{ok} is set to false.

    \sa canCast()
*/
Q_ULLONG QCoreVariant::toULongLong(bool *ok) const
{
    if (d.type == ULongLong) {
        if (ok)
            *ok = true;
        return d.data.shared->value.ull;
    }

    bool c = canCast(ULongLong);
    if (ok)
        *ok = c;
    Q_ULLONG res = 0;
    if (c)
        handler->cast(&d, ULongLong, &res, ok);

    return res;
}

/*!
    Returns the variant as a bool if the variant has type() Bool.

    Returns true if the variant has type Int, UInt or Double and its
    value is non-zero, or if the variant has type String and its lower-case
    content is not empty, "0" or "false"; otherwise returns false.
*/
bool QCoreVariant::toBool() const
{
    if (d.type == Bool)
        return d.data.b;

    bool res = false;
    handler->cast(&d, Bool, &res, 0);

    return res;
}

/*!
    Returns the variant as a double if the variant has type() String,
    ByteArray, Double, Int, UInt, LongLong, ULongLong or Bool; otherwise
    returns 0.0.

    If \a ok is non-null: \c{*}\a{ok} is set to true if the value could be
    converted to a double; otherwise \c{*}\a{ok} is set to false.
*/
double QCoreVariant::toDouble(bool *ok) const
{
    if (d.type == Double) {
        if (ok)
        *ok = true;
        return d.data.shared->value.d;
    }

    bool c = canCast(Double);
    if (ok)
        *ok = c;
    double res = 0;
    if (c)
        handler->cast(&d, Double, &res, ok);

    return res;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
  \fn QCoreVariantList QCoreVariant::toList() const

    Returns the variant as a QCoreVariantList if the variant has
    type() List or StringList; otherwise returns an empty list.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QCoreVariantList list = myVariant.toList();
    QCoreVariantList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode
*/
QCoreVariantList QCoreVariant::toList() const
{
    if (d.type == List)
        return *v_cast<QCoreVariantList>(&d);
    QCoreVariantList res;
    handler->cast(&d, List, &res, 0);
    return res;
}
#endif

/*!
    Returns true if the variant's type can be cast to the requested
    type, \a t. Such casting is done automatically when calling the
    toInt(), toBool(), ... or asInt(), asBool(), ... methods.

    The following casts are done automatically:
    \table
    \header \i Type \i Automatically Cast To
    \row \i Bool \i Double, Int, UInt, LongLong, ULongLong
    \row \i Color \i String
    \row \i Date \i String, DateTime
    \row \i DateTime \i String, Date, Time
    \row \i Double \i String, Int, Bool, UInt
    \row \i Font \i String
    \row \i Int \i String, Double, Bool, UInt
    \row \i List \i StringList (if the list contains strings or
    something that can be cast to a string)
    \row \i String \i CString, Int, Uint, Bool, Double, Date,
    Time, DateTime, KeySequence, Font, Color
    \row \i CString \i String
    \row \i StringList \i List
    \row \i Time \i String
    \row \i UInt \i String, Double, Bool, Int
    \row \i KeySequence \i String, Int
    \endtable
*/
bool QCoreVariant::canCast(Type t) const
{
    return handler->canCast(&d, t);
}

/*!
    Casts the variant to the requested type. If the cast cannot be
    done, the variant is set to the default value of the requested
    type (e.g. an empty string if the requested type \a t is
    QCoreVariant::String, an empty point array if the requested type \a t
    is QCoreVariant::PointArray, etc). Returns true if the current type of
    the variant was successfully cast; otherwise returns false.

    \sa canCast()
*/

bool QCoreVariant::cast(Type t)
{
    if (d.type == uint(t))
        return true;

    QCoreVariant oldValue = *this;

    clear();
    if (!handler->canCast(&oldValue.d, t))
        return false;

    create(t, 0);
    bool isOk = true;
    handler->cast(&oldValue.d, t, data(), &isOk);
    return isOk;
}

/*!
    Compares this QCoreVariant with \a v and returns true if they are
    equal; otherwise returns false.
*/

bool QCoreVariant::operator==(const QCoreVariant &v) const
{
    QCoreVariant v2 = v;
    if (d.type != v2.d.type) {
        if (!v2.canCast(Type(d.type)))
            return false;
        v2.cast(Type(d.type));
    }
    return handler->compare(&d, &v2.d);
}

/*!
    \fn bool QCoreVariant::operator!=(const QCoreVariant &v) const
    Compares this QCoreVariant with \a v and returns true if they are not
    equal; otherwise returns false.
*/

/*! \internal
 */
#define QDATA(vType) \
    if (QTypeInfo<vType >::isLarge) \
        return d.data.shared->value.ptr; \
    else \
        return &d.data.ptr

const void *QCoreVariant::constData() const
{
    switch(d.type) {
    case Int:
    case UInt:
    case Bool:
        return &d.data;
    case LongLong:
        return &d.data.shared->value.ll;
    case ULongLong:
        return &d.data.shared->value.ull;
    case Double:
        return &d.data.shared->value.d;
    case String:
        QDATA(QString);
    case StringList:
        QDATA(QStringList);
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
        QDATA(QCoreVariantMap);
    case List:
        QDATA(QCoreVariantList);
#endif
    case Date:
        QDATA(QDate);
    case Time:
        QDATA(QTime);
    case QCoreVariant::DateTime:
        QDATA(QDateTime);
    case QCoreVariant::ByteArray:
        QDATA(QByteArray);
    case QCoreVariant::BitArray:
        QDATA(QBitArray);
    default:
        return d.is_shared ? d.data.shared->value.ptr : d.data.ptr;
    }
}

/*!
    \fn const void* QCoreVariant::data() const

    \internal
*/

/*! \internal */
void* QCoreVariant::data()
{
    detach();
    return const_cast<void*>(constData());
}


/*! \internal
 */
void *QCoreVariant::castOrDetach(Type t)
{
    if (d.type != uint(t))
        cast(t);
    else
        detach();
    return data();
}

/*!
  Returns true if this is a NULL variant, false otherwise.
*/
bool QCoreVariant::isNull() const
{
    return handler->isNull(&d);
}

#if !defined(Q_OS_DARWIN) || (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QCoreVariant &v)
{
#ifndef Q_NO_STREAMING_DEBUG
    dbg.nospace() << "QCoreVariant(" << v.typeName() << ", ";
    switch(v.type()) {
    case QCoreVariant::Int:
        dbg.nospace() << v.toInt();
        break;
    case QCoreVariant::UInt:
        dbg.nospace() << v.toUInt();
        break;
    case QCoreVariant::LongLong:
        dbg.nospace() << v.toLongLong();
        break;
    case QCoreVariant::ULongLong:
        dbg.nospace() << v.toULongLong();
        break;
    case QCoreVariant::Double:
        dbg.nospace() << v.toDouble();
        break;
    case QCoreVariant::Bool:
        dbg.nospace() << v.toBool();
        break;
    case QCoreVariant::String:
        dbg.nospace() << v.toString();
        break;
    case QCoreVariant::StringList:
        dbg.nospace() << v.toStringList();
        break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::Map:
//         dbg.nospace() << v.toMap();
        break;
    case QCoreVariant::List:
        dbg.nospace() << v.toList();
        break;
#endif
    case QCoreVariant::Date:
        dbg.nospace() << v.toDate();
        break;
    case QCoreVariant::Time:
        dbg.nospace() << v.toTime();
        break;
    case QCoreVariant::DateTime:
        dbg.nospace() << v.toDateTime();
        break;
    case QCoreVariant::ByteArray:
        dbg.nospace() << v.toByteArray();
        break;
    case QCoreVariant::BitArray:
        //dbg.nospace() << v.toBitArray();
        break;
    default:
        break;
    }

    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler does not support streaming QDebug");
    return dbg;
    Q_UNUSED(v);
#endif
}
#endif
#endif

/*!
    \fn int &QCoreVariant::asInt()

    Use toInt() instead.
*/

/*!
    \fn uint &QCoreVariant::asUInt()

    Use toUInt() instead.
*/

/*!
    \fn Q_LLONG &QCoreVariant::asLongLong()

    Use toLongLong() instead.
*/

/*!
    \fn Q_ULLONG &QCoreVariant::asULongLong()

    Use toULongLong() instead.
*/

/*!
    \fn bool &QCoreVariant::asBool()

    Use toBool() instead.
*/

/*!
    \fn double &QCoreVariant::asDouble()

    Use toDouble() instead.
*/

/*!
    \fn QByteArray &QCoreVariant::asByteArray()

    Use toByteArray() instead.
*/

/*!
    \fn QBitArray &QCoreVariant::asBitArray()

    Use toBitArray() instead.
*/

/*!
    \fn QString &QCoreVariant::asString()

    Use toString() instead.
*/

/*!
    \fn QStringList &QCoreVariant::asStringList()

    Use toStringList() instead.
*/

/*!
    \fn QDate &QCoreVariant::asDate()

    Use toDate() instead.
*/

/*!
    \fn QTime &QCoreVariant::asTime()

    Use toTime() instead.
*/

/*!
    \fn QDateTime &QCoreVariant::asDateTime()

    Use toDateTime() instead.
*/

/*!
    \fn QList<QCoreVariant> &QCoreVariant::asList()

    Use toList() instead.
*/

/*!
    \fn QMap<QString,QCoreVariant> &QCoreVariant::asMap()

    Use toMap() instead.
*/

/*!
    \fn QCoreVariant::QCoreVariant(bool b, int dummy)

    Use the QCoreVariant(bool) constructor instead.

*/

/*!
    \fn const QByteArray QCoreVariant::toCString() const

    Use toByteArray() instead.
*/

/*!
    \fn QByteArray &QCoreVariant::asCString()

    Use toByteArray() instead.
*/


#endif //QT_NO_VARIANT
