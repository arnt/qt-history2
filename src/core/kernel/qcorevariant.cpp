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
#include "qpoint.h"
#include "qrect.h"
#include "qsize.h"
#include "qurl.h"
#include "qlocale.h"
#include "private/qcorevariant_p.h"

#include <float.h>

#ifndef DBL_DIG
#define DBL_DIG 10
#endif //DBL_DIG

static void construct(QCoreVariant::Private *x, const void *copy)
{
   x->is_shared = false;

   if (copy) {
        switch(x->type) {
        case QCoreVariant::String:
            v_construct<QString>(x, copy);
            break;
        case QCoreVariant::Char:
            v_construct<QChar>(x, copy);
            break;
        case QCoreVariant::StringList:
            v_construct<QStringList>(x, copy);
            break;
#ifndef QT_NO_TEMPLATE_VARIANT
        case QCoreVariant::Map:
            v_construct<QCoreVariantMap>(x, copy);
            break;
        case QCoreVariant::List:
            v_construct<QCoreVariantList>(x, copy);
            break;
#endif
        case QCoreVariant::Date:
            v_construct<QDate>(x, copy);
            break;
        case QCoreVariant::Time:
            v_construct<QTime>(x, copy);
            break;
        case QCoreVariant::DateTime:
            v_construct<QDateTime>(x, copy);
            break;
        case QCoreVariant::ByteArray:
            v_construct<QByteArray>(x, copy);
            break;
        case QCoreVariant::BitArray:
            v_construct<QBitArray>(x, copy);
            break;
        case QCoreVariant::Size:
            v_construct<QSize>(x, copy);
            break;
        case QCoreVariant::Url:
            v_construct<QUrl>(x, copy);
            break;
        case QCoreVariant::Locale:
            v_construct<QLocale>(x, copy);
            break;
        case QCoreVariant::Rect:
            v_construct<QRect>(x, copy);
            break;
        case QCoreVariant::Point:
            v_construct<QPoint>(x, copy);
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
            x->data.d = *static_cast<const double*>(copy);
            break;
        case QCoreVariant::LongLong:
            x->data.ll = *static_cast<const qlonglong *>(copy);
            break;
        case QCoreVariant::ULongLong:
            x->data.ull = *static_cast<const qulonglong *>(copy);
            break;
        case QCoreVariant::Invalid:
        case QCoreVariant::UserType:
            break;
        default:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared(QMetaType::construct(x->type, copy));
            Q_ASSERT_X(x->data.shared->ptr, "QCoreVariant::construct()", "Unknown datatype");
            break;
        }
        x->is_null = false;
    } else {
        switch (x->type) {
        case QCoreVariant::Invalid:
        case QCoreVariant::UserType:
            break;
        case QCoreVariant::String:
            v_construct<QString>(x);
            break;
        case QCoreVariant::Char:
            v_construct<QChar>(x);
            break;
        case QCoreVariant::StringList:
            v_construct<QStringList>(x);
            break;
#ifndef QT_NO_TEMPLATE_VARIANT
        case QCoreVariant::Map:
            v_construct<QCoreVariantMap>(x);
            break;
        case QCoreVariant::List:
            v_construct<QCoreVariantList>(x);
            break;
#endif
        case QCoreVariant::Date:
            v_construct<QDate>(x);
            break;
        case QCoreVariant::Time:
            v_construct<QTime>(x);
            break;
        case QCoreVariant::DateTime:
            v_construct<QDateTime>(x);
            break;
        case QCoreVariant::ByteArray:
            v_construct<QByteArray>(x);
            break;
        case QCoreVariant::BitArray:
            v_construct<QBitArray>(x);
            break;
        case QCoreVariant::Size:
            v_construct<QSize>(x);
            break;
        case QCoreVariant::Url:
            v_construct<QUrl>(x);
            break;
        case QCoreVariant::Locale:
            v_construct<QLocale>(x);
            break;
        case QCoreVariant::Point:
            v_construct<QPoint>(x);
            break;
        case QCoreVariant::Rect:
            v_construct<QRect>(x);
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
            x->data.d = 0.0;
            break;
        case QCoreVariant::LongLong:
            x->data.ll = Q_INT64_C(0);
            break;
        case QCoreVariant::ULongLong:
            x->data.ull = Q_UINT64_C(0);
            break;
        default:
            x->is_shared = true;
            x->data.shared = new QCoreVariant::PrivateShared(QMetaType::construct(x->type, copy));
            Q_ASSERT_X(x->data.shared->ptr, "QCoreVariant::construct()", "Unknown datatype");
            break;
        }
    }
}

static void clear(QCoreVariant::Private *d)
{
    switch (d->type) {
    case QCoreVariant::String:
        v_clear<QString>(d);
        break;
    case QCoreVariant::Char:
        v_clear<QChar>(d);
        break;
    case QCoreVariant::StringList:
        v_clear<QStringList>(d);
        break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::Map:
        v_clear<QCoreVariantMap>(d);
        break;
    case QCoreVariant::List:
        v_clear<QCoreVariantList>(d);
        break;
#endif
    case QCoreVariant::Date:
        v_clear<QDate>(d);
        break;
    case QCoreVariant::Time:
        v_clear<QTime>(d);
        break;
    case QCoreVariant::DateTime:
        v_clear<QDateTime>(d);
        break;
    case QCoreVariant::ByteArray:
        v_clear<QByteArray>(d);
        break;
    case QCoreVariant::BitArray:
        v_clear<QBitArray>(d);
        break;
    case QCoreVariant::Point:
        v_clear<QPoint>(d);
        break;
    case QCoreVariant::Size:
        v_clear<QSize>(d);
        break;
    case QCoreVariant::Url:
        v_clear<QUrl>(d);
        break;
    case QCoreVariant::Locale:
        v_clear<QLocale>(d);
        break;
    case QCoreVariant::Rect:
        v_clear<QRect>(d);
        break;
    case QCoreVariant::LongLong:
    case QCoreVariant::ULongLong:
    case QCoreVariant::Double:
        break;
    case QCoreVariant::Invalid:
    case QCoreVariant::UserType:
    case QCoreVariant::Int:
    case QCoreVariant::UInt:
    case QCoreVariant::Bool:
        break;
    default:
        QMetaType::destroy(d->type, d->data.shared->ptr);
        delete d->data.shared;
        break;
    }

    d->type = QCoreVariant::Invalid;
    d->is_null = true;
    d->is_shared = false;
}

static bool isNull(const QCoreVariant::Private *d)
{
    switch(d->type) {
    case QCoreVariant::String:
        return v_cast<QString>(d)->isNull();
    case QCoreVariant::Char:
        return v_cast<QChar>(d)->isNull();
    case QCoreVariant::Date:
        return v_cast<QDate>(d)->isNull();
    case QCoreVariant::Time:
        return v_cast<QTime>(d)->isNull();
    case QCoreVariant::DateTime:
        return v_cast<QDateTime>(d)->isNull();
    case QCoreVariant::ByteArray:
        return v_cast<QByteArray>(d)->isNull();
    case QCoreVariant::BitArray:
        return v_cast<QBitArray>(d)->isNull();
    case QCoreVariant::Size:
        return v_cast<QSize>(d)->isNull();
    case QCoreVariant::Rect:
        return v_cast<QRect>(d)->isNull();
    case QCoreVariant::Point:
        return v_cast<QPoint>(d)->isNull();
    case QCoreVariant::Url:
    case QCoreVariant::Locale:
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
        s >> *v_cast<QCoreVariantMap>(d);
        break;
    case QCoreVariant::List:
        s >> *v_cast<QCoreVariantList>(d);
        break;
#endif
    case QCoreVariant::String:
        s >> *v_cast<QString>(d);
        break;
    case QCoreVariant::Char:
        s >> *v_cast<QChar>(d);
        break;
    case QCoreVariant::StringList:
        s >> *v_cast<QStringList>(d);
        break;
    case QCoreVariant::Size:
        s >> *v_cast<QSize>(d);
        break;
    case QCoreVariant::Url:
        s >> *v_cast<QUrl>(d);
        break;
    case QCoreVariant::Locale:
        s >> *v_cast<QLocale>(d);
        break;
    case QCoreVariant::Rect:
        s >> *v_cast<QRect>(d);
        break;
    case QCoreVariant::Point:
        s >> *v_cast<QPoint>(d);
        break;
    case QCoreVariant::Int:
        s >> d->data.i;
        break;
    case QCoreVariant::UInt:
        s >> d->data.u;
        break;
    case QCoreVariant::LongLong:
        s >> d->data.ll;
        break;
    case QCoreVariant::ULongLong:
        s >> d->data.ull;
        break;
    case QCoreVariant::Bool: {
        qint8 x;
        s >> x;
        d->data.b = x;
    }
        break;
    case QCoreVariant::Double:
        s >> d->data.d;
        break;
    case QCoreVariant::Date:
        s >> *v_cast<QDate>(d);
        break;
    case QCoreVariant::Time:
        s >> *v_cast<QTime>(d);
        break;
    case QCoreVariant::DateTime:
        s >> *v_cast<QDateTime>(d);
        break;
    case QCoreVariant::ByteArray:
        s >> *v_cast<QByteArray>(d);
        break;
    case QCoreVariant::BitArray:
        s >> *v_cast<QBitArray>(d);
        break;
    default:
        if (QMetaType::isRegistered(d->type)) {
            if (!QMetaType::load(s, d->type, d->data.shared->ptr))
                qFatal("QCoreVariant::load: no streaming operators registered for type %d.", d->type);
            break;
        } else {
            qFatal("QCoreVariant::load: type %d unknown to QCoreVariant.", d->type);
        }
    }
}

static void save(const QCoreVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::List:
        s << *v_cast<QCoreVariantList>(d);
        break;
    case QCoreVariant::Map:
        s << *v_cast<QCoreVariantMap>(d);
        break;
#endif
    case QCoreVariant::String:
        s << *v_cast<QString>(d);
        break;
    case QCoreVariant::Char:
        s << *v_cast<QChar>(d);
        break;
    case QCoreVariant::StringList:
        s << *v_cast<QStringList>(d);
        break;
    case QCoreVariant::Size:
        s << *v_cast<QSize>(d);
        break;
    case QCoreVariant::Url:
        s << *v_cast<QUrl>(d);
        break;
    case QCoreVariant::Locale:
        s << *v_cast<QLocale>(d);
        break;
    case QCoreVariant::Point:
        s << *v_cast<QPoint>(d);
        break;
    case QCoreVariant::Rect:
        s << *v_cast<QRect>(d);
        break;
    case QCoreVariant::Int:
        s << d->data.i;
        break;
    case QCoreVariant::UInt:
        s << d->data.u;
        break;
    case QCoreVariant::LongLong:
        s << d->data.ll;
        break;
    case QCoreVariant::ULongLong:
        s << d->data.ull;
        break;
    case QCoreVariant::Bool:
        s << (qint8)d->data.b;
        break;
    case QCoreVariant::Double:
        s << d->data.d;
        break;
    case QCoreVariant::Date:
        s << *v_cast<QDate>(d);
        break;
    case QCoreVariant::Time:
        s << *v_cast<QTime>(d);
        break;
    case QCoreVariant::DateTime:
        s << *v_cast<QDateTime>(d);
        break;
    case QCoreVariant::ByteArray:
        s << *v_cast<QByteArray>(d);
        break;
    case QCoreVariant::BitArray:
        s << *v_cast<QBitArray>(d);
        break;
    case QCoreVariant::Invalid:
        s << QString();
        break;
    default:
        if (QMetaType::isRegistered(d->type)) {
            if (!QMetaType::save(s, d->type, d->data.shared->ptr))
                qFatal("QCoreVariant::save: no streaming operators registered for type %d.", d->type);
            break;
        } else {
            qFatal("QCoreVariant::save: type %d unknown to QCoreVariant.", d->type);
        }
    }
}
#endif // QT_NO_DATASTREAM

static bool compare(const QCoreVariant::Private *a, const QCoreVariant::Private *b)
{
    switch(a->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QCoreVariant::List:
        return *v_cast<QCoreVariantList>(a) == *v_cast<QCoreVariantList>(b);
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
        return *v_cast<QString>(a) == *v_cast<QString>(b);
    case QCoreVariant::Char:
        return *v_cast<QChar>(a) == *v_cast<QChar>(b);
    case QCoreVariant::StringList:
        return *v_cast<QStringList>(a) == *v_cast<QStringList>(b);
    case QCoreVariant::Size:
        return *v_cast<QSize>(a) == *v_cast<QSize>(b);
    case QCoreVariant::Url:
        return *v_cast<QUrl>(a) == *v_cast<QUrl>(b);
    case QCoreVariant::Locale:
        return *v_cast<QLocale>(a) == *v_cast<QLocale>(b);
    case QCoreVariant::Rect:
        return *v_cast<QRect>(a) == *v_cast<QRect>(b);
    case QCoreVariant::Point:
        return *v_cast<QPoint>(a) == *v_cast<QPoint>(b);
    case QCoreVariant::Int:
        return a->data.i == b->data.i;
    case QCoreVariant::UInt:
        return a->data.u == b->data.u;
    case QCoreVariant::LongLong:
        return a->data.ll == b->data.ll;
    case QCoreVariant::ULongLong:
        return a->data.ull == b->data.ull;
    case QCoreVariant::Bool:
        return a->data.b == b->data.b;
    case QCoreVariant::Double:
        return a->data.d == b->data.d;
    case QCoreVariant::Date:
        return *v_cast<QDate>(a) == *v_cast<QDate>(b);
    case QCoreVariant::Time:
        return *v_cast<QTime>(a) == *v_cast<QTime>(b);
    case QCoreVariant::DateTime:
        return *v_cast<QDateTime>(a) == *v_cast<QDateTime>(b);
    case QCoreVariant::ByteArray:
        return *v_cast<QByteArray>(a) == *v_cast<QByteArray>(b);
    case QCoreVariant::BitArray:
        return *v_cast<QBitArray>(a) == *v_cast<QBitArray>(b);
    case QCoreVariant::Invalid:
        return true;
    default:
        break;
    }
    if (!QMetaType::isRegistered(a->type))
        qFatal("QCoreVariant::compare: type %d unknown to QCoreVariant.", a->type);
    return a->data.shared->ptr == b->data.shared->ptr;
}

static bool cast(const QCoreVariant::Private *d, QCoreVariant::Type t, void *result, bool *ok)
{
    Q_ASSERT(d->type != uint(t));
    switch (t) {
    case QCoreVariant::String: {
        QString *str = static_cast<QString *>(result);
        switch (d->type) {
        case QCoreVariant::Char:
            *str = QString(*v_cast<QChar>(d));
            break;
        case QCoreVariant::Int:
            *str = QString::number(d->data.i);
            break;
        case QCoreVariant::UInt:
            *str = QString::number(d->data.u);
            break;
        case QCoreVariant::LongLong:
            *str = QString::number(d->data.ll);
            break;
        case QCoreVariant::ULongLong:
            *str = QString::number(d->data.ull);
            break;
        case QCoreVariant::Double:
            *str = QString::number(d->data.d, 'g', DBL_DIG);
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
            return false;
        }
        break;
    }
    case QCoreVariant::Char: {
        QChar *c = static_cast<QChar *>(result);
        switch (d->type) {
        case QCoreVariant::Int:
            *c = QChar(d->data.i);
            break;
        case QCoreVariant::UInt:
            *c = QChar(d->data.u);
            break;
        default:
            return false;
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
            *slst = QStringList(*v_cast<QString>(d));
        } else {
            return false;
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
        else
            return false;
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
            return false;
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
            return false;
        }
        break;
    }
    case QCoreVariant::ByteArray: {
        QByteArray *ba = static_cast<QByteArray *>(result);
        if (d->type == QCoreVariant::String)
            *ba = v_cast<QString>(d)->toAscii();
        else
            return false;
    }
    break;
    case QCoreVariant::Int: {
        int *i = static_cast<int *>(result);
        switch (d->type) {
        case QCoreVariant::String:
            *i = v_cast<QString>(d)->toInt(ok);
            break;
        case QCoreVariant::Char:
            *i = v_cast<QChar>(d)->unicode();
            break;
        case QCoreVariant::ByteArray:
            *i = v_cast<QByteArray>(d)->toInt(ok);
            break;
        case QCoreVariant::Int:
            *i = d->data.i;
            break;
        case QCoreVariant::UInt:
            *i = int(d->data.u);
            break;
        case QCoreVariant::LongLong:
            *i = int(d->data.ll);
            break;
        case QCoreVariant::ULongLong:
            *i = int(d->data.ull);
            break;
        case QCoreVariant::Double:
            *i = qRound(d->data.d);
            break;
        case QCoreVariant::Bool:
            *i = (int)d->data.b;
            break;
        default:
            *i = 0;
            return false;
        }
        break;
    }
    case QCoreVariant::UInt: {
        uint *u = static_cast<uint *>(result);
        switch (d->type) {
        case QCoreVariant::String:
            *u = v_cast<QString>(d)->toUInt(ok);
            break;
        case QCoreVariant::Char:
            *u = v_cast<QChar>(d)->unicode();
            break;
        case QCoreVariant::ByteArray:
            *u = v_cast<QByteArray>(d)->toUInt(ok);
            break;
        case QCoreVariant::Int:
            *u = uint(d->data.i);
            break;
        case QCoreVariant::UInt:
            *u = d->data.u;
            break;
        case QCoreVariant::LongLong:
            *u = uint(d->data.ll);
            break;
        case QCoreVariant::ULongLong:
            *u = uint(d->data.ull);
            break;
        case QCoreVariant::Double:
            *u = qRound(d->data.d);
            break;
        case QCoreVariant::Bool:
            *u = uint(d->data.b);
            break;
        default:
            *u = 0u;
            return false;
        }
        break;
    }
    case QCoreVariant::LongLong: {
        qlonglong *l = static_cast<qlonglong *>(result);
        switch (d->type) {
        case QCoreVariant::String:
            *l = v_cast<QString>(d)->toLongLong(ok);
            break;
        case QCoreVariant::Char:
            *l = v_cast<QChar>(d)->unicode();
            break;
        case QCoreVariant::ByteArray:
            *l = v_cast<QByteArray>(d)->toLongLong(ok);
            break;
        case QCoreVariant::Int:
            *l = qlonglong(d->data.i);
            break;
        case QCoreVariant::UInt:
            *l = qlonglong(d->data.u);
            break;
        case QCoreVariant::LongLong:
            *l = d->data.ll;
            break;
        case QCoreVariant::ULongLong:
            *l = qlonglong(d->data.ull);
            break;
        case QCoreVariant::Double:
            *l = qRound64(d->data.d);
            break;
        case QCoreVariant::Bool:
            *l = qlonglong(d->data.b);
            break;
        default:
            *l = Q_INT64_C(0);
            return false;
        }
        break;
    }
    case QCoreVariant::ULongLong: {
        qulonglong *l = static_cast<qulonglong *>(result);
        switch (d->type) {
        case QCoreVariant::Int:
            *l = qulonglong(d->data.i);
            break;
        case QCoreVariant::UInt:
            *l = qulonglong(d->data.u);
            break;
        case QCoreVariant::LongLong:
            *l = qulonglong(d->data.ll);
            break;
        case QCoreVariant::ULongLong:
            *l = d->data.ull;
            break;
        case QCoreVariant::Double:
            *l = qRound64(d->data.d);
            break;
        case QCoreVariant::Bool:
            *l = qulonglong(d->data.b);
            break;
        case QCoreVariant::String:
            *l = v_cast<QString>(d)->toULongLong(ok);
            break;
        case QCoreVariant::Char:
            *l = v_cast<QChar>(d)->unicode();
            break;
        case QCoreVariant::ByteArray:
            *l = v_cast<QByteArray>(d)->toULongLong(ok);
            break;
        default:
            *l = Q_UINT64_C(0);
            return false;
        }
        break;
    }
    case QCoreVariant::Bool: {
        bool *b = static_cast<bool *>(result);
        switch(d->type) {
        case QCoreVariant::Double:
            *b = d->data.d != 0.0;
            break;
        case QCoreVariant::Int:
            *b = d->data.i != 0;
            break;
        case QCoreVariant::UInt:
            *b = d->data.u != 0;
            break;
        case QCoreVariant::LongLong:
            *b = d->data.ll != Q_INT64_C(0);
            break;
        case QCoreVariant::ULongLong:
            *b = d->data.ull != Q_UINT64_C(0);
            break;
        case QCoreVariant::String:
        {
            QString str = v_cast<QString>(d)->toLower();
            *b = !(str == QLatin1String("0") || str == QLatin1String("false") || str.isEmpty());
            break;
        }
        case QCoreVariant::Char:
            *b = !v_cast<QChar>(d)->isNull();
            break;
        default:
            *b = false;
            return false;
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
            *f = d->data.d;
            break;
        case QCoreVariant::Int:
            *f = double(d->data.i);
            break;
        case QCoreVariant::Bool:
            *f = double(d->data.b);
            break;
        case QCoreVariant::UInt:
            *f = double(d->data.u);
            break;
        case QCoreVariant::LongLong:
            *f = double(d->data.ll);
            break;
        case QCoreVariant::ULongLong:
#if defined(Q_CC_MSVC) && !defined(Q_CC_MSVC_NET)
            *f = (double)(qlonglong)d->data.ull;
#else
            *f = double(d->data.ull);
#endif
            break;
        default:
            *f = 0.0;
            return false;
        }
        break;
    }
    case QCoreVariant::List:
        if (d->type == QCoreVariant::StringList) {
            QCoreVariantList *lst = static_cast<QCoreVariantList *>(result);
            const QStringList *slist = v_cast<QStringList>(d);
            for (int i = 0; i < slist->size(); ++i)
                lst->append(QCoreVariant(slist->at(i)));
        } else {
            return false;
        }
        break;

    default:
        return false;
    }
    return true;
}

static bool canCast(const QCoreVariant::Private *d, QCoreVariant::Type t)
{
    if (d->type == (uint)t)
        return true;

    switch (t) {
    case QCoreVariant::Bool:
        return d->type == QCoreVariant::Double || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::UInt || d->type == QCoreVariant::LongLong
            || d->type == QCoreVariant::ULongLong || d->type == QCoreVariant::String
            || d->type == QCoreVariant::Char;
    case QCoreVariant::Int:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::UInt
            || d->type == QCoreVariant::LongLong || d->type == QCoreVariant::ULongLong
            || d->type == QCoreVariant::Char;
    case QCoreVariant::UInt:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::LongLong || d->type == QCoreVariant::ULongLong
            || d->type == QCoreVariant::Char;
    case QCoreVariant::LongLong:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::UInt || d->type == QCoreVariant::ULongLong
            || d->type == QCoreVariant::Char;
    case QCoreVariant::ULongLong:
        return d->type == QCoreVariant::String || d->type == QCoreVariant::Double
            || d->type == QCoreVariant::Bool || d->type == QCoreVariant::Int
            || d->type == QCoreVariant::UInt || d->type == QCoreVariant::LongLong
            || d->type == QCoreVariant::Char;
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
            || d->type == QCoreVariant::LongLong || d->type == QCoreVariant::ULongLong
            || d->type == QCoreVariant::Char;
    case QCoreVariant::Char:
        return d->type == QCoreVariant::Int || d->type == QCoreVariant::UInt;
    case QCoreVariant::ByteArray:
        return
#ifdef QT3_SUPPORT
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
    shared, namely QImage and QPolygon, and in these
    cases the toT() methods return a shallow copy. In almost all cases
    you must make a deep copy of the returned values before modifying
    them.

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
    v = v.toInt() + 100;            // The variant now hold the value 223.
    v = QCoreVariant(QStringList());
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
    x.cast(QCoreVariant::Int)cast(QCoreVariant::Int);
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
    \value Polygon a QPolygon
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
    d.is_shared = p.d.is_shared;
    if (d.is_shared) {
        d.data.shared = p.d.data.shared;
        ++d.data.shared->ref;
    } else {
        handler->construct(&d, p.constData());
    }
    d.is_null = p.d.is_null;
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
  \fn QCoreVariant::QCoreVariant(const QPoint &val)

  Constructs a new variant with a point value of \a val.
 */

/*!
  \fn QCoreVariant::QCoreVariant(const QRect &val)

  Constructs a new variant with a rect value of \a val.
 */

/*!
  \fn QCoreVariant::QCoreVariant(const QSize &val)

  Constructs a new variant with a size value of \a val.
 */

/*!
  \fn QCoreVariant::QCoreVariant(const QUrl &val)

  Constructs a new variant with a url value of \a val.
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
  \fn QCoreVariant::QCoreVariant(qlonglong val)

    Constructs a new variant with a long long integer value, \a val.
*/

/*!
  \fn QCoreVariant::QCoreVariant(qulonglong val)

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
QCoreVariant::QCoreVariant(qlonglong val)
{ create(LongLong, &val); }
QCoreVariant::QCoreVariant(qulonglong val)
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
QCoreVariant::QCoreVariant(const QChar &val)
{ create (Char, &val); }
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
QCoreVariant::QCoreVariant(const QPoint &pt) { create(Point, &pt); }
QCoreVariant::QCoreVariant(const QRect &r) { create(Rect, &r); }
QCoreVariant::QCoreVariant(const QSize &s) { create(Size, &s); }
QCoreVariant::QCoreVariant(const QUrl &u) { create(Url, &u); }
QCoreVariant::QCoreVariant(const QLocale &l) { create(Locale, &l); }

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
enum { ntypes = 40 };
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
#ifdef QT3_SUPPORT
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
    "QPolygon",
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
    "qlonglong",
    "qulonglong",
    "QChar",
    "QUrl",
    "QTextLength",
    "QTextFormat",
    "QLocale"
};


/*!
    Converts the enum representation of the storage type, \a typ, to
    its string representation.
*/
const char *QCoreVariant::typeToName(Type typ)
{
    if (typ == UserType)
        return "UserType";
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
    if (!name)
        return Invalid;
    if (strcmp(name, "") == 0)
        return Invalid;
    if (strcmp(name, "Q3CString") == 0)
        return ByteArray;
    if (strcmp(name, "Q_LLONG") == 0)
        return LongLong;
    if (strcmp(name, "Q_ULLONG") == 0)
        return ULongLong;
    if (strcmp(name, "QIconSet") == 0)
        return Icon;
    if (strcmp(name, "UserType") == 0)
        return UserType;
    for (int i = 1; i < ntypes; ++i) {
        if (strcmp(type_map[i], name) == 0)
            return (Type)i;
    }
    if (QMetaType::type(name))
        return UserType;
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

    quint32 u;
    s >> u;
    if (u >= QCoreVariant::UserType) {
        QByteArray name;
        s >> name;
        u = QMetaType::type(name);
        if (!u)
            qFatal("QCoreVariant::load(QDataStream &s): type %s unknown to QCoreVariant.", name.data());
    }
    create(static_cast<int>(u), 0);
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
    s << (quint32)type();
    if (type() == QCoreVariant::UserType) {
        s << QMetaType::typeName(userType());
    }
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
    quint32 u;
    s >> u;
    p = (QCoreVariant::Type)u;

    return s;
}

/*!
    Writes a variant type \a p to the stream \a s.
*/
QDataStream& operator<<(QDataStream &s, const QCoreVariant::Type p)
{
    s << (quint32)p;

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
Q_VARIANT_TO(Char)
Q_VARIANT_TO(Rect)
Q_VARIANT_TO(Size)
Q_VARIANT_TO(Point)
Q_VARIANT_TO(Url)
Q_VARIANT_TO(Locale)

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
  \fn QPoint QCoreVariant::toPoint() const

  Returns the variant as a QPoint if the variant has type()
  Point; otherwise returns a null QPoint.
 */

/*!
  \fn QRect QCoreVariant::toRect() const

  Returns the variant as a QRect if the variant has type()
  Rect; otherwise returns an invalid QRect.
 */

/*!
  \fn QSize QCoreVariant::toSize() const

  Returns the variant as a QSize if the variant has type()
  Size; otherwise returns an invalid QSize.
 */

/*!
  \fn QUrl QCoreVariant::toUrl() const

  Returns the variant as a QUrl if the variant has type()
  Url; otherwise returns an invalid QUrl.
 */

/*!
  \fn QLocale QCoreVariant::toLocale() const

  Returns the variant as a QLocale if the variant has type()
  Locale; otherwise returns an invalid QLocale.
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
    uint res = 0u;
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
qlonglong QCoreVariant::toLongLong(bool *ok) const
{
    if (d.type == LongLong) {
        if (ok)
            *ok = true;
        return d.data.ll;
    }

    bool c = canCast(LongLong);
    if (ok)
        *ok = c;
    qlonglong res = Q_INT64_C(0);
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
qulonglong QCoreVariant::toULongLong(bool *ok) const
{
    if (d.type == ULongLong) {
        if (ok)
            *ok = true;
        return d.data.ull;
    }

    bool c = canCast(ULongLong);
    if (ok)
        *ok = c;
    qulonglong res = Q_UINT64_C(0);
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
        return d.data.d;
    }

    bool c = canCast(Double);
    if (ok)
        *ok = c;
    double res = 0.0;
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
    toInt(), toBool(), ... methods.

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
    is QCoreVariant::Polygon, etc). Returns true if the current type of
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

const void *QCoreVariant::constData() const
{
    switch(d.type) {
    case Int:
        return &d.data.i;
    case UInt:
        return &d.data.u;
    case Bool:
        return &d.data.b;
    case LongLong:
        return &d.data.ll;
    case ULongLong:
        return &d.data.ull;
    case Double:
        return &d.data.d;
    case String:
        return v_cast<QString>(&d);
    case Char:
        return v_cast<QChar>(&d);
    case StringList:
        return v_cast<QStringList>(&d);
    case Rect:
        return v_cast<QRect>(&d);
    case Url:
        return v_cast<QUrl>(&d);
    case Locale:
        return v_cast<QLocale>(&d);
    case Point:
        return v_cast<QPoint>(&d);
    case Size:
        return v_cast<QSize>(&d);
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
        return v_cast<QCoreVariantMap>(&d);
    case List:
        return v_cast<QCoreVariantList>(&d);
#endif
    case Date:
        return v_cast<QDate>(&d);
    case Time:
        return v_cast<QTime>(&d);
    case QCoreVariant::DateTime:
        return v_cast<QDateTime>(&d);
    case QCoreVariant::ByteArray:
        return v_cast<QByteArray>(&d);
    case QCoreVariant::BitArray:
        return v_cast<QBitArray>(&d);
    default:
        return d.is_shared ? d.data.shared->ptr : reinterpret_cast<const void *>(&d.data.ptr);
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

#ifndef QT_NO_DEBUG_OUTPUT
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
    case QCoreVariant::Char:
        dbg.nospace() << v.toChar();
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

/*!
    \fn int &QCoreVariant::asInt()

    Use toInt() instead.
*/

/*!
    \fn uint &QCoreVariant::asUInt()

    Use toUInt() instead.
*/

/*!
    \fn qlonglong &QCoreVariant::asLongLong()

    Use toLongLong() instead.
*/

/*!
    \fn qulonglong &QCoreVariant::asULongLong()

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

/*!
    \fn QPoint &QCoreVariant::asPoint()

    Use toPoint() instead.
 */

/*!
    \fn QRect &QCoreVariant::asRect()

    Use toRect() instead.
 */

/*!
    \fn QSize &QCoreVariant::asSize()

    Use toSize() instead.
 */

#endif //QT_NO_VARIANT
