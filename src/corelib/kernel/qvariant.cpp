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

#include "qvariant.h"
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
#include "qurl.h"
#include "qlocale.h"
#include "private/qvariant_p.h"

#ifndef QT_NO_GEOM_VARIANT
#include "qsize.h"
#include "qpoint.h"
#include "qrect.h"
#include "qline.h"
#endif

#include <float.h>

#ifndef DBL_DIG
#define DBL_DIG 10
#endif //DBL_DIG

static void construct(QVariant::Private *x, const void *copy)
{
    x->is_shared = false;

    switch (x->type) {
    case QVariant::String:
        v_construct<QString>(x, copy);
        break;
    case QVariant::Char:
        v_construct<QChar>(x, copy);
        break;
    case QVariant::StringList:
        v_construct<QStringList>(x, copy);
        break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
        v_construct<QVariantMap>(x, copy);
        break;
    case QVariant::List:
        v_construct<QVariantList>(x, copy);
        break;
#endif
    case QVariant::Date:
        v_construct<QDate>(x, copy);
        break;
    case QVariant::Time:
        v_construct<QTime>(x, copy);
        break;
    case QVariant::DateTime:
        v_construct<QDateTime>(x, copy);
        break;
    case QVariant::ByteArray:
        v_construct<QByteArray>(x, copy);
        break;
    case QVariant::BitArray:
        v_construct<QBitArray>(x, copy);
        break;
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::Size:
        v_construct<QSize>(x, copy);
        break;
    case QVariant::Rect:
        v_construct<QRect>(x, copy);
        break;
    case QVariant::LineF:
        v_construct<QLineF>(x, copy);
        break;
    case QVariant::Line:
        v_construct<QLine>(x, copy);
        break;
    case QVariant::RectF:
        v_construct<QRectF>(x, copy);
        break;
    case QVariant::Point:
        v_construct<QPoint>(x, copy);
        break;
    case QVariant::PointF:
        v_construct<QPointF>(x, copy);
        break;
#endif
    case QVariant::Url:
        v_construct<QUrl>(x, copy);
        break;
    case QVariant::Locale:
        v_construct<QLocale>(x, copy);
        break;
    case QVariant::Int:
        x->data.i = copy ? *static_cast<const int *>(copy) : 0;
        break;
    case QVariant::UInt:
        x->data.u = copy ? *static_cast<const uint *>(copy) : 0u;
        break;
    case QVariant::Bool:
        x->data.b = copy ? *static_cast<const bool *>(copy) : false;
        break;
    case QVariant::Double:
        x->data.d = copy ? *static_cast<const double*>(copy) : 0.0;
        break;
    case QVariant::LongLong:
        x->data.ll = copy ? *static_cast<const qlonglong *>(copy) : Q_INT64_C(0);
        break;
    case QVariant::ULongLong:
        x->data.ull = copy ? *static_cast<const qulonglong *>(copy) : Q_UINT64_C(0);
        break;
    case QVariant::Invalid:
    case QVariant::UserType:
        break;
    default:
        x->is_shared = true;
        x->data.shared = new QVariant::PrivateShared(QMetaType::construct(x->type, copy));
        Q_ASSERT_X(x->data.shared->ptr, "QVariant::construct()", "Unknown datatype");
        break;
    }
    x->is_null = !copy;
}

static void clear(QVariant::Private *d)
{
    switch (d->type) {
    case QVariant::String:
        v_clear<QString>(d);
        break;
    case QVariant::Char:
        v_clear<QChar>(d);
        break;
    case QVariant::StringList:
        v_clear<QStringList>(d);
        break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
        v_clear<QVariantMap>(d);
        break;
    case QVariant::List:
        v_clear<QVariantList>(d);
        break;
#endif
    case QVariant::Date:
        v_clear<QDate>(d);
        break;
    case QVariant::Time:
        v_clear<QTime>(d);
        break;
    case QVariant::DateTime:
        v_clear<QDateTime>(d);
        break;
    case QVariant::ByteArray:
        v_clear<QByteArray>(d);
        break;
    case QVariant::BitArray:
        v_clear<QBitArray>(d);
        break;
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::Point:
        v_clear<QPoint>(d);
        break;
    case QVariant::PointF:
        v_clear<QPointF>(d);
        break;
    case QVariant::Size:
        v_clear<QSize>(d);
        break;
    case QVariant::Rect:
        v_clear<QRect>(d);
        break;
    case QVariant::LineF:
        v_clear<QLineF>(d);
        break;
    case QVariant::Line:
        v_clear<QLine>(d);
        break;
    case QVariant::RectF:
        v_clear<QRectF>(d);
        break;
#endif
    case QVariant::Url:
        v_clear<QUrl>(d);
        break;
    case QVariant::Locale:
        v_clear<QLocale>(d);
        break;
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
        break;
    case QVariant::Invalid:
    case QVariant::UserType:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Bool:
        break;
    default:
        QMetaType::destroy(d->type, d->data.shared->ptr);
        delete d->data.shared;
        break;
    }

    d->type = QVariant::Invalid;
    d->is_null = true;
    d->is_shared = false;
}

static bool isNull(const QVariant::Private *d)
{
    switch(d->type) {
    case QVariant::String:
        return v_cast<QString>(d)->isNull();
    case QVariant::Char:
        return v_cast<QChar>(d)->isNull();
    case QVariant::Date:
        return v_cast<QDate>(d)->isNull();
    case QVariant::Time:
        return v_cast<QTime>(d)->isNull();
    case QVariant::DateTime:
        return v_cast<QDateTime>(d)->isNull();
    case QVariant::ByteArray:
        return v_cast<QByteArray>(d)->isNull();
    case QVariant::BitArray:
        return v_cast<QBitArray>(d)->isNull();
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::Size:
        return v_cast<QSize>(d)->isNull();
    case QVariant::Rect:
        return v_cast<QRect>(d)->isNull();
    case QVariant::Line:
        return v_cast<QLine>(d)->isNull();
    case QVariant::LineF:
        return v_cast<QLineF>(d)->isNull();
    case QVariant::RectF:
        return v_cast<QRectF>(d)->isNull();
    case QVariant::Point:
        return v_cast<QPoint>(d)->isNull();
    case QVariant::PointF:
        return v_cast<QPointF>(d)->isNull();
#endif
    case QVariant::Url:
    case QVariant::Locale:
    case QVariant::StringList:
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
    case QVariant::List:
#endif
    case QVariant::Invalid:
    case QVariant::UserType:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Bool:
    case QVariant::Double:
        break;
    }
    return d->is_null;
}

#ifndef QT_NO_DATASTREAM
static void load(QVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
    case QVariant::Invalid: {
        // Since we wrote something, we should read something
        QString x;
        s >> x;
        d->is_null = true;
        break;
    }
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
        s >> *v_cast<QVariantMap>(d);
        break;
    case QVariant::List:
        s >> *v_cast<QVariantList>(d);
        break;
#endif
    case QVariant::String:
        s >> *v_cast<QString>(d);
        break;
    case QVariant::Char:
        s >> *v_cast<QChar>(d);
        break;
    case QVariant::StringList:
        s >> *v_cast<QStringList>(d);
        break;
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::Size:
        s >> *v_cast<QSize>(d);
        break;
    case QVariant::Rect:
        s >> *v_cast<QRect>(d);
        break;
    case QVariant::Line:
        s >> *v_cast<QLine>(d);
        break;
    case QVariant::LineF:
        s >> *v_cast<QLineF>(d);
        break;
    case QVariant::RectF:
        s >> *v_cast<QRectF>(d);
        break;
    case QVariant::Point:
        s >> *v_cast<QPoint>(d);
        break;
    case QVariant::PointF:
        s >> *v_cast<QPointF>(d);
        break;
#endif
    case QVariant::Url:
        s >> *v_cast<QUrl>(d);
        break;
    case QVariant::Locale:
        s >> *v_cast<QLocale>(d);
        break;
    case QVariant::Int:
        s >> d->data.i;
        break;
    case QVariant::UInt:
        s >> d->data.u;
        break;
    case QVariant::LongLong:
        s >> d->data.ll;
        break;
    case QVariant::ULongLong:
        s >> d->data.ull;
        break;
    case QVariant::Bool: {
        qint8 x;
        s >> x;
        d->data.b = x;
    }
        break;
    case QVariant::Double:
        s >> d->data.d;
        break;
    case QVariant::Date:
        s >> *v_cast<QDate>(d);
        break;
    case QVariant::Time:
        s >> *v_cast<QTime>(d);
        break;
    case QVariant::DateTime:
        s >> *v_cast<QDateTime>(d);
        break;
    case QVariant::ByteArray:
        s >> *v_cast<QByteArray>(d);
        break;
    case QVariant::BitArray:
        s >> *v_cast<QBitArray>(d);
        break;
    default:
        if (QMetaType::isRegistered(d->type)) {
            if (!QMetaType::load(s, d->type, d->data.shared->ptr))
                qFatal("QVariant::load: no streaming operators registered for type %d.", d->type);
            break;
        } else {
            qFatal("QVariant::load: type %d unknown to QVariant.", d->type);
        }
    }
}

static void save(const QVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::List:
        s << *v_cast<QVariantList>(d);
        break;
    case QVariant::Map:
        s << *v_cast<QVariantMap>(d);
        break;
#endif
    case QVariant::String:
        s << *v_cast<QString>(d);
        break;
    case QVariant::Char:
        s << *v_cast<QChar>(d);
        break;
    case QVariant::StringList:
        s << *v_cast<QStringList>(d);
        break;
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::Size:
        s << *v_cast<QSize>(d);
        break;
    case QVariant::Point:
        s << *v_cast<QPoint>(d);
        break;
    case QVariant::PointF:
        s << *v_cast<QPointF>(d);
        break;
    case QVariant::Rect:
        s << *v_cast<QRect>(d);
        break;
    case QVariant::Line:
        s << *v_cast<QLine>(d);
        break;
    case QVariant::LineF:
        s << *v_cast<QLineF>(d);
        break;
    case QVariant::RectF:
        s << *v_cast<QRectF>(d);
        break;
#endif
    case QVariant::Url:
        s << *v_cast<QUrl>(d);
        break;
    case QVariant::Locale:
        s << *v_cast<QLocale>(d);
        break;
    case QVariant::Int:
        s << d->data.i;
        break;
    case QVariant::UInt:
        s << d->data.u;
        break;
    case QVariant::LongLong:
        s << d->data.ll;
        break;
    case QVariant::ULongLong:
        s << d->data.ull;
        break;
    case QVariant::Bool:
        s << (qint8)d->data.b;
        break;
    case QVariant::Double:
        s << d->data.d;
        break;
    case QVariant::Date:
        s << *v_cast<QDate>(d);
        break;
    case QVariant::Time:
        s << *v_cast<QTime>(d);
        break;
    case QVariant::DateTime:
        s << *v_cast<QDateTime>(d);
        break;
    case QVariant::ByteArray:
        s << *v_cast<QByteArray>(d);
        break;
    case QVariant::BitArray:
        s << *v_cast<QBitArray>(d);
        break;
    case QVariant::Invalid:
        s << QString();
        break;
    default:
        if (QMetaType::isRegistered(d->type)) {
            if (!QMetaType::save(s, d->type, d->data.shared->ptr))
                qFatal("QVariant::save: no streaming operators registered for type %d.", d->type);
            break;
        } else {
            qFatal("QVariant::save: type %d unknown to QVariant.", d->type);
        }
    }
}
#endif // QT_NO_DATASTREAM

static bool compare(const QVariant::Private *a, const QVariant::Private *b)
{
    switch(a->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::List:
        return *v_cast<QVariantList>(a) == *v_cast<QVariantList>(b);
    case QVariant::Map: {
        const QVariantMap *m1 = v_cast<QVariantMap>(a);
        const QVariantMap *m2 = v_cast<QVariantMap>(b);
        if (m1->count() != m2->count())
            return false;
        QVariantMap::ConstIterator it = m1->constBegin();
        QVariantMap::ConstIterator it2 = m2->constBegin();
        while (it != m1->constEnd()) {
            if (*it != *it2)
                return false;
            ++it;
            ++it2;
        }
        return true;
    }
#endif
    case QVariant::String:
        return *v_cast<QString>(a) == *v_cast<QString>(b);
    case QVariant::Char:
        return *v_cast<QChar>(a) == *v_cast<QChar>(b);
    case QVariant::StringList:
        return *v_cast<QStringList>(a) == *v_cast<QStringList>(b);
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::Size:
        return *v_cast<QSize>(a) == *v_cast<QSize>(b);
    case QVariant::Rect:
        return *v_cast<QRect>(a) == *v_cast<QRect>(b);
    case QVariant::Line:
        return *v_cast<QLine>(a) == *v_cast<QLine>(b);
    case QVariant::LineF:
        return *v_cast<QLineF>(a) == *v_cast<QLineF>(b);
    case QVariant::RectF:
        return *v_cast<QRectF>(a) == *v_cast<QRectF>(b);
    case QVariant::Point:
        return *v_cast<QPoint>(a) == *v_cast<QPoint>(b);
    case QVariant::PointF:
        return *v_cast<QPointF>(a) == *v_cast<QPointF>(b);
#endif
    case QVariant::Url:
        return *v_cast<QUrl>(a) == *v_cast<QUrl>(b);
    case QVariant::Locale:
        return *v_cast<QLocale>(a) == *v_cast<QLocale>(b);
    case QVariant::Int:
        return a->data.i == b->data.i;
    case QVariant::UInt:
        return a->data.u == b->data.u;
    case QVariant::LongLong:
        return a->data.ll == b->data.ll;
    case QVariant::ULongLong:
        return a->data.ull == b->data.ull;
    case QVariant::Bool:
        return a->data.b == b->data.b;
    case QVariant::Double:
        return a->data.d == b->data.d;
    case QVariant::Date:
        return *v_cast<QDate>(a) == *v_cast<QDate>(b);
    case QVariant::Time:
        return *v_cast<QTime>(a) == *v_cast<QTime>(b);
    case QVariant::DateTime:
        return *v_cast<QDateTime>(a) == *v_cast<QDateTime>(b);
    case QVariant::ByteArray:
        return *v_cast<QByteArray>(a) == *v_cast<QByteArray>(b);
    case QVariant::BitArray:
        return *v_cast<QBitArray>(a) == *v_cast<QBitArray>(b);
    case QVariant::Invalid:
        return true;
    default:
        break;
    }
    if (!QMetaType::isRegistered(a->type))
        qFatal("QVariant::compare: type %d unknown to QVariant.", a->type);
    return a->data.shared->ptr == b->data.shared->ptr;
}

static bool convert(const QVariant::Private *d, QVariant::Type t, void *result, bool *ok)
{
    Q_ASSERT(d->type != uint(t));
    switch (t) {
    case QVariant::String: {
        QString *str = static_cast<QString *>(result);
        switch (d->type) {
        case QVariant::Char:
            *str = QString(*v_cast<QChar>(d));
            break;
        case QVariant::Int:
            *str = QString::number(d->data.i);
            break;
        case QVariant::UInt:
            *str = QString::number(d->data.u);
            break;
        case QVariant::LongLong:
            *str = QString::number(d->data.ll);
            break;
        case QVariant::ULongLong:
            *str = QString::number(d->data.ull);
            break;
        case QVariant::Double:
            *str = QString::number(d->data.d, 'g', DBL_DIG);
            break;
#if !defined(QT_NO_SPRINTF) && !defined(QT_NO_DATESTRING)
        case QVariant::Date:
            *str = v_cast<QDate>(d)->toString(Qt::ISODate);
            break;
        case QVariant::Time:
            *str = v_cast<QTime>(d)->toString(Qt::ISODate);
            break;
        case QVariant::DateTime:
            *str = v_cast<QDateTime>(d)->toString(Qt::ISODate);
            break;
#endif
        case QVariant::Bool:
            *str = QLatin1String(d->data.b ? "true" : "false");
            break;
        case QVariant::ByteArray:
            *str = QString::fromAscii(v_cast<QByteArray>(d)->constData());
            break;
        case QVariant::StringList:
            if (v_cast<QStringList>(d)->count() == 1)
                *str = v_cast<QStringList>(d)->at(0);
            break;
        default:
            return false;
        }
        break;
    }
    case QVariant::Char: {
        QChar *c = static_cast<QChar *>(result);
        switch (d->type) {
        case QVariant::Int:
            *c = QChar(d->data.i);
            break;
        case QVariant::UInt:
            *c = QChar(d->data.u);
            break;
        default:
            return false;
        }
        break;
    }

#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::StringList:
        if (d->type == QVariant::List) {
            QStringList *slst = static_cast<QStringList *>(result);
            const QVariantList *list = v_cast<QVariantList >(d);
            for (int i = 0; i < list->size(); ++i)
                slst->append(list->at(i).toString());
        } else if (d->type == QVariant::String) {
            QStringList *slst = static_cast<QStringList *>(result);
            *slst = QStringList(*v_cast<QString>(d));
        } else {
            return false;
        }
#endif
        break;
    case QVariant::Date: {
        QDate *dt = static_cast<QDate *>(result);
        if (d->type == QVariant::DateTime)
            *dt = v_cast<QDateTime>(d)->date();
#ifndef QT_NO_DATESTRING
        else if (d->type == QVariant::String)
            *dt = QDate::fromString(*v_cast<QString>(d), Qt::ISODate);
#endif
        else
            return false;
        break;
    }
    case QVariant::Time: {
        QTime *t = static_cast<QTime *>(result);
        switch (d->type) {
        case QVariant::DateTime:
            *t = v_cast<QDateTime>(d)->time();
            break;
#ifndef QT_NO_DATESTRING
        case QVariant::String:
            *t = QTime::fromString(*v_cast<QString>(d), Qt::ISODate);
            break;
#endif
        default:
            return false;
        }
        break;
    }
    case QVariant::DateTime: {
        QDateTime *dt = static_cast<QDateTime *>(result);
        switch (d->type) {
#ifndef QT_NO_DATESTRING
        case QVariant::String:
            *dt = QDateTime::fromString(*v_cast<QString>(d), Qt::ISODate);
            break;
#endif
        case QVariant::Date:
            *dt = QDateTime(*v_cast<QDate>(d));
            break;
        default:
            return false;
        }
        break;
    }
    case QVariant::ByteArray: {
        QByteArray *ba = static_cast<QByteArray *>(result);
        if (d->type == QVariant::String)
            *ba = v_cast<QString>(d)->toAscii();
        else
            return false;
    }
    break;
    case QVariant::Int: {
        int *i = static_cast<int *>(result);
        switch (d->type) {
        case QVariant::String:
            *i = v_cast<QString>(d)->toInt(ok);
            break;
        case QVariant::Char:
            *i = v_cast<QChar>(d)->unicode();
            break;
        case QVariant::ByteArray:
            *i = v_cast<QByteArray>(d)->toInt(ok);
            break;
        case QVariant::Int:
            *i = d->data.i;
            break;
        case QVariant::UInt:
            *i = int(d->data.u);
            break;
        case QVariant::LongLong:
            *i = int(d->data.ll);
            break;
        case QVariant::ULongLong:
            *i = int(d->data.ull);
            break;
        case QVariant::Double:
            *i = qRound(d->data.d);
            break;
        case QVariant::Bool:
            *i = (int)d->data.b;
            break;
        default:
            *i = 0;
            return false;
        }
        break;
    }
    case QVariant::UInt: {
        uint *u = static_cast<uint *>(result);
        switch (d->type) {
        case QVariant::String:
            *u = v_cast<QString>(d)->toUInt(ok);
            break;
        case QVariant::Char:
            *u = v_cast<QChar>(d)->unicode();
            break;
        case QVariant::ByteArray:
            *u = v_cast<QByteArray>(d)->toUInt(ok);
            break;
        case QVariant::Int:
            *u = uint(d->data.i);
            break;
        case QVariant::UInt:
            *u = d->data.u;
            break;
        case QVariant::LongLong:
            *u = uint(d->data.ll);
            break;
        case QVariant::ULongLong:
            *u = uint(d->data.ull);
            break;
        case QVariant::Double:
            *u = qRound(d->data.d);
            break;
        case QVariant::Bool:
            *u = uint(d->data.b);
            break;
        default:
            *u = 0u;
            return false;
        }
        break;
    }
    case QVariant::LongLong: {
        qlonglong *l = static_cast<qlonglong *>(result);
        switch (d->type) {
        case QVariant::String:
            *l = v_cast<QString>(d)->toLongLong(ok);
            break;
        case QVariant::Char:
            *l = v_cast<QChar>(d)->unicode();
            break;
        case QVariant::ByteArray:
            *l = v_cast<QByteArray>(d)->toLongLong(ok);
            break;
        case QVariant::Int:
            *l = qlonglong(d->data.i);
            break;
        case QVariant::UInt:
            *l = qlonglong(d->data.u);
            break;
        case QVariant::LongLong:
            *l = d->data.ll;
            break;
        case QVariant::ULongLong:
            *l = qlonglong(d->data.ull);
            break;
        case QVariant::Double:
            *l = qRound64(d->data.d);
            break;
        case QVariant::Bool:
            *l = qlonglong(d->data.b);
            break;
        default:
            *l = Q_INT64_C(0);
            return false;
        }
        break;
    }
    case QVariant::ULongLong: {
        qulonglong *l = static_cast<qulonglong *>(result);
        switch (d->type) {
        case QVariant::Int:
            *l = qulonglong(d->data.i);
            break;
        case QVariant::UInt:
            *l = qulonglong(d->data.u);
            break;
        case QVariant::LongLong:
            *l = qulonglong(d->data.ll);
            break;
        case QVariant::ULongLong:
            *l = d->data.ull;
            break;
        case QVariant::Double:
            *l = qRound64(d->data.d);
            break;
        case QVariant::Bool:
            *l = qulonglong(d->data.b);
            break;
        case QVariant::String:
            *l = v_cast<QString>(d)->toULongLong(ok);
            break;
        case QVariant::Char:
            *l = v_cast<QChar>(d)->unicode();
            break;
        case QVariant::ByteArray:
            *l = v_cast<QByteArray>(d)->toULongLong(ok);
            break;
        default:
            *l = Q_UINT64_C(0);
            return false;
        }
        break;
    }
    case QVariant::Bool: {
        bool *b = static_cast<bool *>(result);
        switch(d->type) {
        case QVariant::Double:
            *b = d->data.d != 0.0;
            break;
        case QVariant::Int:
            *b = d->data.i != 0;
            break;
        case QVariant::UInt:
            *b = d->data.u != 0;
            break;
        case QVariant::LongLong:
            *b = d->data.ll != Q_INT64_C(0);
            break;
        case QVariant::ULongLong:
            *b = d->data.ull != Q_UINT64_C(0);
            break;
        case QVariant::String:
        {
            QString str = v_cast<QString>(d)->toLower();
            *b = !(str == QLatin1String("0") || str == QLatin1String("false") || str.isEmpty());
            break;
        }
        case QVariant::Char:
            *b = !v_cast<QChar>(d)->isNull();
            break;
        default:
            *b = false;
            return false;
        }
        break;
    }
    case QVariant::Double: {
        double *f = static_cast<double *>(result);
        switch (d->type) {
        case QVariant::String:
            *f = v_cast<QString>(d)->toDouble(ok);
            break;
        case QVariant::ByteArray:
            *f = v_cast<QByteArray>(d)->toDouble(ok);
            break;
        case QVariant::Double:
            *f = d->data.d;
            break;
        case QVariant::Int:
            *f = double(d->data.i);
            break;
        case QVariant::Bool:
            *f = double(d->data.b);
            break;
        case QVariant::UInt:
            *f = double(d->data.u);
            break;
        case QVariant::LongLong:
            *f = double(d->data.ll);
            break;
        case QVariant::ULongLong:
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
    case QVariant::List:
        if (d->type == QVariant::StringList) {
            QVariantList *lst = static_cast<QVariantList *>(result);
            const QStringList *slist = v_cast<QStringList>(d);
            for (int i = 0; i < slist->size(); ++i)
                lst->append(QVariant(slist->at(i)));
        } else {
            return false;
        }
        break;
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::RectF:
        if (d->type == QVariant::Rect)
            *static_cast<QRectF *>(result) = *v_cast<QRect>(d);
        else
            return false;
    case QVariant::PointF:
        if (d->type == QVariant::Point)
            *static_cast<QPointF *>(result) = *v_cast<QPoint>(d);
        else
            return false;
#endif
    default:
        return false;
    }
    return true;
}

static bool canConvert(const QVariant::Private *d, QVariant::Type t)
{
    if (d->type == (uint)t)
        return true;

    switch (t) {
    case QVariant::Bool:
        return d->type == QVariant::Double || d->type == QVariant::Int
            || d->type == QVariant::UInt || d->type == QVariant::LongLong
            || d->type == QVariant::ULongLong || d->type == QVariant::String
            || d->type == QVariant::Char;
    case QVariant::Int:
        return d->type == QVariant::String || d->type == QVariant::Double
            || d->type == QVariant::Bool || d->type == QVariant::UInt
            || d->type == QVariant::LongLong || d->type == QVariant::ULongLong
            || d->type == QVariant::Char;
    case QVariant::UInt:
        return d->type == QVariant::String || d->type == QVariant::Double
            || d->type == QVariant::Bool || d->type == QVariant::Int
            || d->type == QVariant::LongLong || d->type == QVariant::ULongLong
            || d->type == QVariant::Char;
    case QVariant::LongLong:
        return d->type == QVariant::String || d->type == QVariant::Double
            || d->type == QVariant::Bool || d->type == QVariant::Int
            || d->type == QVariant::UInt || d->type == QVariant::ULongLong
            || d->type == QVariant::Char;
    case QVariant::ULongLong:
        return d->type == QVariant::String || d->type == QVariant::Double
            || d->type == QVariant::Bool || d->type == QVariant::Int
            || d->type == QVariant::UInt || d->type == QVariant::LongLong
            || d->type == QVariant::Char;
    case QVariant::Double:
        return d->type == QVariant::String || d->type == QVariant::Int
            || d->type == QVariant::Bool || d->type == QVariant::UInt
            || d->type == QVariant::LongLong || d->type == QVariant::ULongLong;
    case QVariant::String:
        if (d->type == QVariant::StringList && v_cast<QStringList>(d)->count() == 1)
            return true;
        return d->type == QVariant::ByteArray || d->type == QVariant::Int
            || d->type == QVariant::UInt || d->type == QVariant::Bool
            || d->type == QVariant::Double || d->type == QVariant::Date
            || d->type == QVariant::Time || d->type == QVariant::DateTime
            || d->type == QVariant::LongLong || d->type == QVariant::ULongLong
            || d->type == QVariant::Char;
    case QVariant::Char:
        return d->type == QVariant::Int || d->type == QVariant::UInt;
    case QVariant::ByteArray:
        return
#ifdef QT3_SUPPORT
            d->type == QVariant::CString ||
#endif
            d->type == QVariant::String;
    case QVariant::Date:
        return d->type == QVariant::String || d->type == QVariant::DateTime;
    case QVariant::Time:
        return d->type == QVariant::String || d->type == QVariant::DateTime;
    case QVariant::DateTime:
        return d->type == QVariant::String || d->type == QVariant::Date;
    case QVariant::List:
        return d->type == QVariant::StringList;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::StringList:
        if (d->type == QVariant::List) {
            const QVariantList &varlist = *v_cast<QVariantList >(d);
            for (int i = 0; i < varlist.size(); ++i) {
                if (!varlist.at(i).canConvert(QVariant::String))
                    return false;
            }
            return true;
        } else if (d->type == QVariant::String) {
            return true;
        }
        return false;
#endif
    case QVariant::RectF:
        return d->type == QVariant::Rect;
    case QVariant::PointF:
        return d->type == QVariant::Point;
    default:
        return false;
    }
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
void streamDebug(QDebug dbg, const QVariant &v)
{
    switch(v.type()) {
    case QVariant::Int:
        dbg.nospace() << v.toInt();
        break;
    case QVariant::UInt:
        dbg.nospace() << v.toUInt();
        break;
    case QVariant::LongLong:
        dbg.nospace() << v.toLongLong();
        break;
    case QVariant::ULongLong:
        dbg.nospace() << v.toULongLong();
        break;
    case QVariant::Double:
        dbg.nospace() << v.toDouble();
        break;
    case QVariant::Bool:
        dbg.nospace() << v.toBool();
        break;
    case QVariant::String:
        dbg.nospace() << v.toString();
        break;
    case QVariant::Char:
        dbg.nospace() << v.toChar();
        break;
    case QVariant::StringList:
        dbg.nospace() << v.toStringList();
        break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
        dbg.nospace() << v.toMap();
        break;
    case QVariant::List:
        dbg.nospace() << v.toList();
        break;
#endif
    case QVariant::Date:
        dbg.nospace() << v.toDate();
        break;
    case QVariant::Time:
        dbg.nospace() << v.toTime();
        break;
    case QVariant::DateTime:
        dbg.nospace() << v.toDateTime();
        break;
    case QVariant::ByteArray:
        dbg.nospace() << v.toByteArray();
        break;
    case QVariant::Url:
        dbg.nospace() << v.toUrl();
        break;
#ifndef QT_NO_GEOM_VARIANT
    case QVariant::Point:
        dbg.nospace() << v.toPoint();
        break;
    case QVariant::PointF:
        dbg.nospace() << v.toPointF();
        break;
    case QVariant::Rect:
        dbg.nospace() << v.toRect();
        break;
    case QVariant::Size:
        dbg.nospace() << v.toSize();
        break;
    case QVariant::Line:
        dbg.nospace() << v.toLine();
        break;
    case QVariant::LineF:
        dbg.nospace() << v.toLineF();
        break;
    case QVariant::RectF:
        dbg.nospace() << v.toRectF();
        break;
#endif
    case QVariant::BitArray:
        //dbg.nospace() << v.toBitArray();
        break;
    default:
        break;
    }
}
#endif

const QVariant::Handler qt_kernel_variant_handler = {
    construct,
    clear,
    isNull,
#ifndef QT_NO_DATASTREAM
    load,
    save,
#endif
    compare,
    convert,
    canConvert,
#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
    streamDebug
#else
    0
#endif
};

Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler()
{
    return &qt_kernel_variant_handler;
}


const QVariant::Handler *QVariant::handler = &qt_kernel_variant_handler;

/*!
    \class QVariant qvariant.h
    \brief The QVariant class acts like a union for the most common Qt data types.

    \ingroup objectmodel
    \ingroup misc
    \mainclass

    Because C++ forbids unions from including types that have
    non-default constructors or destructors, most interesting Qt
    classes cannot be used in unions. Without QVariant, this would be
    a problem for QObject::property() and for database work, etc.

    A QVariant object holds a single value of a single type() at a
    time. (Some type()s are multi-valued, for example a string list.)
    You can find out what type, T, the variant holds, convert it to a
    different type using one of the asT() functions, e.g. asSize(),
    get its value using one of the toT() functions, e.g. toSize(), and
    check whether the type can be converted to a particular type using
    canConvert().

    The methods named toT() (for any supported T, see the \c Type
    documentation for a list) are const. If you ask for the stored
    type, they return a copy of the stored object. If you ask for a
    type that can be generated from the stored type, toT() copies and
    converts and leaves the object itself unchanged. If you ask for a
    type that cannot be generated from the stored type, the result
    depends on the type (see the function documentation for details).

    Note that two data types supported by QVariant are explicitly
    shared, namely QImage and QPolygon, and in these
    cases the toT() methods return a shallow copy. In almost all cases
    you must make a deep copy of the returned values before modifying
    them.

    Here is some example code to demonstrate the use of QVariant:

    \code
    QDataStream out(...);
    QVariant v(123);            // The variant now contains an int
    int x = v.toInt();              // x = 123
    out << v;                       // Writes a type tag and an int to out
    v = QVariant("hello");      // The variant now contains a QByteArray
    v = QVariant(tr("hello"));  // The variant now contains a QString
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
    v = QVariant(QStringList());
    \endcode

    You can even store QVariantLists and
    QMap<QString,QVariant>s in a variant, so you can easily construct
    arbitrarily complex data structures of arbitrary types. This is
    very powerful and versatile, but may prove less memory and speed
    efficient than storing specific types in standard data structures.

    QVariant also supports the notion of NULL values, where you have a
    defined type with no value set.
    \code
    QVariant x, y(QString()), z(QString(""));
    x.convert(QVariant::Int);
    // x.isNull() == true,
    // y.isNull() == true, z.isNull() == false
    // y.isEmpty() == true, z.Empty() == true
    \endcode
*/

/*!
    \enum QVariant::Type

    This enum type defines the types of variable that a QVariant can
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
    \value LineF  a QLineF
    \value Line  a QLine
    \value List  a QVariantList
    \value LongLong a long long
    \value ULongLong an unsigned long long
    \value Map  a QMap<QString,QVariant>
    \value Palette  a QPalette
    \value Pen  a QPen
    \value Pixmap  a QPixmap
    \value Point  a QPoint
    \value PointF  a QPointF
    \value Polygon a QPolygon
    \value Rect  a QRect
    \value RectF  a QRectF
    \value Region  a QRegion
    \value Size  a QSize
    \value SizePolicy  a QSizePolicy
    \value String  a QString
    \value StringList  a QStringList
    \value Time  a QTime
    \value UInt  an unsigned int
    \value TextLength  a QTextLength
    \value TextFormat  a QTextFormat
    \value Char  a QChar
    \value Locale  a QLocale
    \value Url  a QUrl
    \value PointArray  a QPointArray

    \value UserType

    \omitvalue CString
    \omitvalue ColorGroup
    \omitvalue IconSet
    \omitvalue LastType

    Note that Qt's definition of bool depends on the compiler.
    \c qglobal.h has the system-dependent definition of bool.
*/

/*!
  \fn QVariant::QVariant()

    Constructs an invalid variant.
*/


/*!
    \fn QVariant::QVariant(int typeOrUserType, const void *copy)

    Constructs variant of type \a typeOrUserType, and initializes with
    \a copy if \a copy is not 0.
*/

/*!
    \fn QVariant::QVariant(Type type)

    Constructs a null variant of type \a type.
*/



/*!
    \fn QVariant::create(int type, const void *copy)

    \internal

    Constructs a variant private of type \a type, and initializes with \a copy if
    \a copy is not 0.
*/

void QVariant::create(int type, const void *copy)
{
    d.type = type;
    d.is_null = true;
    handler->construct(&d, copy);
}

/*!
  \fn QVariant::~QVariant()

    Destroys the QVariant and the contained object.

    Note that subclasses that reimplement clear() should reimplement
    the destructor to call clear(). This destructor calls clear(), but
    because it is the destructor, QVariant::clear() is called rather
    than a subclass's clear().
*/

QVariant::~QVariant()
{
    if (!d.is_shared || !d.data.shared->ref.deref())
        handler->clear(&d);
}

/*!
  \fn QVariant::QVariant(const QVariant &p)

    Constructs a copy of the variant, \a p, passed as the argument to
    this constructor. Usually this is a deep copy, but a shallow copy
    is made if the stored data type is explicitly shared, as e.g.
    QImage is.
*/

QVariant::QVariant(const QVariant &p)
{
    d.type = p.d.type;
    d.is_shared = p.d.is_shared;
    if (d.is_shared) {
        d.data.shared = p.d.data.shared;
        d.data.shared->ref.ref();
    } else {
        handler->construct(&d, p.constData());
    }
    d.is_null = p.d.is_null;
}

#ifndef QT_NO_DATASTREAM
/*!
    Reads the variant from the data stream, \a s.
*/
QVariant::QVariant(QDataStream &s)
{
    d.is_null = true;
    s >> *this;
}
#endif //QT_NO_DATASTREAM

/*!
  \fn QVariant::QVariant(const QString &val)

    Constructs a new variant with a string value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QLatin1String &val)

    Constructs a new variant with a string value, \a val.
*/

/*!
  \fn QVariant::QVariant(const char *val)

    Constructs a new variant with a C-string value of \a val if \a val
    is non-null. The variant creates a deep copy of \a val.

    If \a val is null, the resulting variant has type Invalid.
*/


QVariant::QVariant(const char *val)
{
    QString s = QString::fromLatin1(val);
    create(String, &s);
}

/*!
  \fn QVariant::QVariant(const QStringList &val)

    Constructs a new variant with a string list value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QMap<QString,QVariant> &val)

    Constructs a new variant with a map of QVariants, \a val.
*/

/*!
  \fn QVariant::QVariant(const QDate &val)

    Constructs a new variant with a date value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QTime &val)

    Constructs a new variant with a time value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QDateTime &val)

    Constructs a new variant with a date/time value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QByteArray &val)

    Constructs a new variant with a bytearray value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QBitArray &val)

    Constructs a new variant with a bitarray value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QPoint &val)

  Constructs a new variant with a point value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QPointF &val)

  Constructs a new variant with a point value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QRectF &val)

  Constructs a new variant with a rect value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QLineF &val)

  Constructs a new variant with a line value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QLine &val)

  Constructs a new variant with a line value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QRect &val)

  Constructs a new variant with a rect value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QSize &val)

  Constructs a new variant with a size value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QSizeF &val)

  Constructs a new variant with a size value of \a val.
 */

/*!
  \fn QVariant::QVariant(const QUrl &val)

  Constructs a new variant with a url value of \a val.
 */

/*!
  \fn QVariant::QVariant(int val)

    Constructs a new variant with an integer value, \a val.
*/

/*!
  \fn QVariant::QVariant(uint val)

    Constructs a new variant with an unsigned integer value, \a val.
*/

/*!
  \fn QVariant::QVariant(qlonglong val)

    Constructs a new variant with a long long integer value, \a val.
*/

/*!
  \fn QVariant::QVariant(qulonglong val)

    Constructs a new variant with an unsigned long long integer value, \a val.
*/


/*!
  \fn QVariant::QVariant(bool val)

    Constructs a new variant with a boolean value, \a val. The integer
    argument is a dummy, necessary for compatibility with some
    compilers.
*/


/*!
  \fn QVariant::QVariant(double val)

    Constructs a new variant with a floating point value, \a val.
*/

/*!
    \fn QVariant::QVariant(const QList<QVariant> &val)

    Constructs a new variant with a list value, \a val.
*/

/*!
  \fn QVariant::QVariant(const QChar &c)

  Constructs a new variant with a char value, \a c.
*/

/*!
  \fn QVariant::QVariant(const QLocale &l)

  Constructs a new variant with a locale value, \a l.
*/

QVariant::QVariant(Type type)
{ create(type, 0); }
QVariant::QVariant(int typeOrUserType, const void *copy)
{ create(typeOrUserType, copy); d.is_null = false; }
QVariant::QVariant(int val)
{ create(Int, &val); }
QVariant::QVariant(uint val)
{ create(UInt, &val); }
QVariant::QVariant(qlonglong val)
{ create(LongLong, &val); }
QVariant::QVariant(qulonglong val)
{ create(ULongLong, &val); }
QVariant::QVariant(bool val)
{ create(Bool, &val); }
QVariant::QVariant(double val)
{ create(Double, &val); }

QVariant::QVariant(const QByteArray &val)
{ create(ByteArray, &val); }
QVariant::QVariant(const QBitArray &val)
{ create(BitArray, &val); }
QVariant::QVariant(const QString &val)
{ create(String, &val); }
QVariant::QVariant(const QChar &val)
{ create (Char, &val); }
QVariant::QVariant(const QLatin1String &val)
{ QString str(val); create(String, &str); }
QVariant::QVariant(const QStringList &val)
{ create(StringList, &val); }

QVariant::QVariant(const QDate &val)
{ create(Date, &val); }
QVariant::QVariant(const QTime &val)
{ create(Time, &val); }
QVariant::QVariant(const QDateTime &val)
{ create(DateTime, &val); }
#ifndef QT_NO_TEMPLATE_VARIANT
QVariant::QVariant(const QList<QVariant> &list)
{ create(List, &list); }
QVariant::QVariant(const QMap<QString,QVariant> &map)
{ create(Map, &map); }
#endif
#ifndef QT_NO_GEOM_VARIANT
QVariant::QVariant(const QPoint &pt) { create(Point, &pt); }
QVariant::QVariant(const QPointF &pt) { create (PointF, &pt); }
QVariant::QVariant(const QRectF &r) { create (RectF, &r); }
QVariant::QVariant(const QLineF &l) { create (LineF, &l); }
QVariant::QVariant(const QLine &l) { create (Line, &l); }
QVariant::QVariant(const QRect &r) { create(Rect, &r); }
QVariant::QVariant(const QSize &s) { create(Size, &s); }
#endif
QVariant::QVariant(const QUrl &u) { create(Url, &u); }
QVariant::QVariant(const QLocale &l) { create(Locale, &l); }

/*!
    Returns the storage type of the value stored in the variant.
    Usually it's best to test with canConvert() whether the variant can
    deliver the data type you are interested in.
*/

QVariant::Type QVariant::type() const
{
    return d.type >= QMetaType::User ? UserType : static_cast<Type>(d.type);
}

/*!
    Returns the storage type of the value stored in the variant. For
    non-user types, this is the same as type().

    \sa type()
*/

int QVariant::userType() const
{
    return d.type;
}

/*!
    Assigns the value of the variant \a variant to this variant.

    This is a deep copy of the variant, but note that if the variant
    holds an explicitly shared type such as QImage, a shallow copy is
    performed.
*/
QVariant& QVariant::operator=(const QVariant &variant)
{
    if (this == &variant)
        return *this;

    clear();
    if (variant.d.is_shared) {
        variant.d.data.shared->ref.ref();
        d = variant.d;
    } else {
        d.type = variant.d.type;
        handler->construct(&d, variant.constData());
        d.is_null = variant.d.is_null;
    }

    return *this;
}

/*!
    \fn void QVariant::detach()

    \internal
*/

void QVariant::detach()
{
    if (!d.is_shared || d.data.shared->ref == 1)
        return;

    Private dd;
    dd.type = d.type;
    handler->construct(&dd, constData());
    dd.data.shared = qAtomicSetPtr(&d.data.shared, dd.data.shared);
    if (!dd.data.shared->ref.deref())
        handler->clear(&dd);
}

/*!
    \fn bool QVariant::isDetached() const

    \internal
*/

/*!
    Returns the name of the type stored in the variant. The returned
    strings describe the C++ datatype used to store the data: for
    example, "QFont", "QString", or "QVariantList". An Invalid
    variant returns 0.
*/
const char *QVariant::typeName() const
{
    return typeToName((Type)d.type);
}

/*!
    Convert this variant to type Invalid and free up any resources
    used.
*/
void QVariant::clear()
{
    if (!d.is_shared || !d.data.shared->ref.deref())
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
enum { ntypes = 44 };
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
    "QLocale",
    "QLineF",
    "QRectF",
    "QPointF",
    "QLine"
};


/*!
    Converts the enum representation of the storage type, \a typ, to
    its string representation.
*/
const char *QVariant::typeToName(Type typ)
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
QVariant::Type QVariant::nameToType(const char *name)
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
void QVariant::load(QDataStream &s)
{
    clear();

    quint32 u;
    s >> u;
    if (u >= QVariant::UserType) {
        QByteArray name;
        s >> name;
        u = QMetaType::type(name);
        if (!u)
            qFatal("QVariant::load(QDataStream &s): type %s unknown to QVariant.", name.data());
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
void QVariant::save(QDataStream &s) const
{
    s << (quint32)type();
    if (type() == QVariant::UserType) {
        s << QMetaType::typeName(userType());
    }
    handler->save(&d, s);
}

/*!
    Reads a variant \a p from the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator>>(QDataStream &s, QVariant &p)
{
    p.load(s);
    return s;
}

/*!
    Writes a variant \a p to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator<<(QDataStream &s, const QVariant &p)
{
    p.save(s);
    return s;
}

/*!
    Reads a variant type \a p in enum representation from the stream \a s.
*/
QDataStream& operator>>(QDataStream &s, QVariant::Type &p)
{
    quint32 u;
    s >> u;
    p = (QVariant::Type)u;

    return s;
}

/*!
    Writes a variant type \a p to the stream \a s.
*/
QDataStream& operator<<(QDataStream &s, const QVariant::Type p)
{
    s << (quint32)p;

    return s;
}

#endif //QT_NO_DATASTREAM

/*!
    \fn bool QVariant::isValid() const

    Returns true if the storage type of this variant is not
    QVariant::Invalid; otherwise returns false.
*/

#define Q_VARIANT_TO(f) \
Q##f QVariant::to##f() const { \
    if (d.type == f) \
        return *v_cast<Q##f >(&d); \
    Q##f ret; \
    handler->convert(&d, f, &ret, 0); \
    return ret; \
}

Q_VARIANT_TO(StringList)
Q_VARIANT_TO(Date)
Q_VARIANT_TO(Time)
Q_VARIANT_TO(DateTime)
Q_VARIANT_TO(ByteArray)
Q_VARIANT_TO(Char)
Q_VARIANT_TO(Url)
Q_VARIANT_TO(Locale)
#ifndef QT_NO_GEOM_VARIANT
Q_VARIANT_TO(Size)
Q_VARIANT_TO(Rect)
Q_VARIANT_TO(Point)
Q_VARIANT_TO(PointF)
Q_VARIANT_TO(LineF)
Q_VARIANT_TO(Line)
Q_VARIANT_TO(RectF)
#endif

/*!
  \fn QString QVariant::toString() const

    Returns the variant as a QString if the variant has type() String,
    ByteArray, Int, Uint, Bool, Double, Date, Time, DateTime,
    KeySequence, Font or Color; otherwise returns an empty string.
*/


/*!
  \fn QStringList QVariant::toStringList() const

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



QString QVariant::toString() const
{
    if (d.type == String)
        return *reinterpret_cast<const QString *>(&d.data.ptr);

    QString ret;
    handler->convert(&d, String, &ret, 0);
    return ret;
}
#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant as a QMap<QString,QVariant> if the variant has
    type() Map; otherwise returns an empty map.

    Note that if you want to iterate over the map, you should iterate
    over a copy, e.g.
    \code
    QVariantMap map = myVariant.toMap();
    QVariantMap::Iterator it = map.begin();
    while(it != map.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode
*/
QVariantMap QVariant::toMap() const
{
    if (d.type != Map)
        return QMap<QString,QVariant>();

    return *v_cast<QVariantMap>(&d);
}
#endif

/*!
  \fn QDate QVariant::toDate() const

    Returns the variant as a QDate if the variant has type() Date,
    DateTime or String; otherwise returns an invalid date.

    Note that if the type() is String an invalid date will be returned
    if the string cannot be parsed as a Qt::ISODate format date.
*/


/*!
  \fn QTime QVariant::toTime() const

    Returns the variant as a QTime if the variant has type() Time,
    DateTime or String; otherwise returns an invalid time.

    Note that if the type() is String an invalid time will be returned
    if the string cannot be parsed as a Qt::ISODate format time.
*/

/*!
  \fn QDateTime QVariant::toDateTime() const

    Returns the variant as a QDateTime if the variant has type()
    DateTime, Date or String; otherwise returns an invalid date/time.

    Note that if the type() is String an invalid date/time will be
    returned if the string cannot be parsed as a Qt::ISODate format
    date/time.
*/


/*!
  \fn QByteArray QVariant::toByteArray() const

    Returns the variant as a QByteArray if the variant has type()
    ByteArray; otherwise returns an empty bytearray.
*/

/*!
  \fn QPoint QVariant::toPoint() const

  Returns the variant as a QPoint if the variant has type()
  Point; otherwise returns a null QPoint.
 */

/*!
  \fn QRect QVariant::toRect() const

  Returns the variant as a QRect if the variant has type()
  Rect; otherwise returns an invalid QRect.
 */

/*!
  \fn QSize QVariant::toSize() const

  Returns the variant as a QSize if the variant has type()
  Size or SizeF; otherwise returns an invalid QSize.
 */

/*!
  \fn QSizeF QVariant::toSizeF() const

  Returns the variant as a QSizeF if the variant has type()
  SizeF or Size; otherwise returns an invalid QSizeF.
 */

/*!
  \fn QUrl QVariant::toUrl() const

  Returns the variant as a QUrl if the variant has type()
  Url; otherwise returns an invalid QUrl.
 */

/*!
  \fn QLocale QVariant::toLocale() const

  Returns the variant as a QLocale if the variant has type()
  Locale; otherwise returns an invalid QLocale.
 */

/*!
  \fn QRectF QVariant::toRectF() const

  Returns the variant as a QRectF if the variant has type()
  Rect or RectF; otherwise returns an invalid QRectF.
 */

/*!
  \fn QLineF QVariant::toLineF() const

  Returns the variant as a QLineF if the variant has type()
  LineF; otherwise returns an invalid QLineF.
 */

/*!
  \fn QLine QVariant::toLine() const

  Returns the variant as a QLine if the variant has type()
  Line; otherwise returns an invalid QLine.
 */

/*!
  \fn QPointF QVariant::toPointF() const

  Returns the variant as a QPointF if the variant has type()
  Point or PointF; otherwise returns an invalid QPointF.
 */

/*!
  \fn QChar QVariant::toChar() const

  Returns the variant as a QChar if the variant has type()
  Char, String or contains a numeric value; otherwise returns
  an invalid QChar.
 */

/*!
    Returns the variant as a QBitArray if the variant has type()
    BitArray; otherwise returns an empty bitarray.
*/
QBitArray QVariant::toBitArray() const
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

    \sa canConvert()
*/
int QVariant::toInt(bool *ok) const
{
    if (d.type == Int) {
        if (ok)
            *ok = true;
        return d.data.i;
    }

    bool c = canConvert(Int);
    if (ok)
        *ok = c;
    int res = 0;
    if (c)
        handler->convert(&d, Int, &res, ok);

    return res;
}

/*!
    Returns the variant as an unsigned int if the variant has type()
    String, ByteArray, UInt, Int, Double, or Bool; otherwise returns 0.

    If \a ok is non-null: \c{*}\a{ok} is set to true if the value could be
    converted to an unsigned int; otherwise \c{*}\a{ok} is set to false.
*/
uint QVariant::toUInt(bool *ok) const
{
    if (d.type == UInt) {
        if (ok)
            *ok = true;
        return d.data.u;
    }

    bool c = canConvert(UInt);
    if (ok)
        *ok = c;
    uint res = 0u;
    if (c)
        handler->convert(&d, UInt, &res, ok);

    return res;
}

/*!
    Returns the variant as a long long int if the variant has type()
    LongLong, ULongLong, any type allowing a toInt() conversion;
    otherwise returns 0.

    If \a ok is non-null: \c{*}\c{ok} is set to true if the value could be
    converted to an int; otherwise \c{*}\c{ok} is set to false.

    \sa canConvert()
*/
qlonglong QVariant::toLongLong(bool *ok) const
{
    if (d.type == LongLong) {
        if (ok)
            *ok = true;
        return d.data.ll;
    }

    bool c = canConvert(LongLong);
    if (ok)
        *ok = c;
    qlonglong res = Q_INT64_C(0);
    if (c)
        handler->convert(&d, LongLong, &res, ok);

    return res;
}

/*!
    Returns the variant as as an unsigned long long int if the variant
    has type() LongLong, ULongLong, any type allowing a toUInt()
    conversion; otherwise returns 0.

    If \a ok is non-null: \c{*}\a{ok} is set to true if the value could be
    converted to an int; otherwise \c{*}\a{ok} is set to false.

    \sa canConvert()
*/
qulonglong QVariant::toULongLong(bool *ok) const
{
    if (d.type == ULongLong) {
        if (ok)
            *ok = true;
        return d.data.ull;
    }

    bool c = canConvert(ULongLong);
    if (ok)
        *ok = c;
    qulonglong res = Q_UINT64_C(0);
    if (c)
        handler->convert(&d, ULongLong, &res, ok);

    return res;
}

/*!
    Returns the variant as a bool if the variant has type() Bool.

    Returns true if the variant has type Int, UInt or Double and its
    value is non-zero, or if the variant has type String and its lower-case
    content is not empty, "0" or "false"; otherwise returns false.
*/
bool QVariant::toBool() const
{
    if (d.type == Bool)
        return d.data.b;

    bool res = false;
    handler->convert(&d, Bool, &res, 0);

    return res;
}

/*!
    Returns the variant as a double if the variant has type() String,
    ByteArray, Double, Int, UInt, LongLong, ULongLong or Bool; otherwise
    returns 0.0.

    If \a ok is non-null: \c{*}\a{ok} is set to true if the value could be
    converted to a double; otherwise \c{*}\a{ok} is set to false.
*/
double QVariant::toDouble(bool *ok) const
{
    if (d.type == Double) {
        if (ok)
        *ok = true;
        return d.data.d;
    }

    bool c = canConvert(Double);
    if (ok)
        *ok = c;
    double res = 0.0;
    if (c)
        handler->convert(&d, Double, &res, ok);

    return res;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
  \fn QVariantList QVariant::toList() const

    Returns the variant as a QVariantList if the variant has
    type() List or StringList; otherwise returns an empty list.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QVariantList list = myVariant.toList();
    QVariantList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode
*/
QVariantList QVariant::toList() const
{
    if (d.type == List)
        return *v_cast<QVariantList>(&d);
    QVariantList res;
    handler->convert(&d, List, &res, 0);
    return res;
}
#endif

/*! \fn QVariant::canCast(Type t) const
    Use canConvert() instead.
*/

/*! \fn QVariant::cast(Type t)
    Use convert() instead.
*/

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
bool QVariant::canConvert(Type t) const
{
    return handler->canConvert(&d, t);
}

/*!
    Casts the variant to the requested type. If the cast cannot be
    done, the variant is set to the default value of the requested
    type (e.g. an empty string if the requested type \a t is
    QVariant::String, an empty point array if the requested type \a t
    is QVariant::Polygon, etc). Returns true if the current type of
    the variant was successfully cast; otherwise returns false.

    \sa canConvert()
*/

bool QVariant::convert(Type t)
{
    if (d.type == uint(t))
        return true;

    QVariant oldValue = *this;

    clear();
    if (!handler->canConvert(&oldValue.d, t))
        return false;

    create(t, 0);
    bool isOk = true;
    handler->convert(&oldValue.d, t, data(), &isOk);
    return isOk;
}

/*!
    \fn bool operator==(const QVariant &v1, const QVariant &v2)

    \relates QVariant

    Returns true if \a v1 and \a v2 are equal; otherwise returns false.
*/
/*!
    \fn bool operator!=(const QVariant &v1, const QVariant &v2)

    \relates QVariant

    Returns false if \a v1 and \a v2 are equal; otherwise returns true.
*/

/*! \fn bool QVariant::operator==(const QVariant &v) const
    Compares this QVariant with \a v and returns true if they are
    equal; otherwise returns false.
*/

/*!
    \fn bool QVariant::operator!=(const QVariant &v) const
    Compares this QVariant with \a v and returns true if they are not
    equal; otherwise returns false.
*/


/*! \internal
 */
bool QVariant::cmp(const QVariant &v) const
{
    QVariant v2 = v;
    if (d.type != v2.d.type) {
        if (!v2.canConvert(Type(d.type)))
            return false;
        v2.convert(Type(d.type));
    }
    return handler->compare(&d, &v2.d);
}

/*! \internal
 */

const void *QVariant::constData() const
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
    default:
        return d.is_shared ? d.data.shared->ptr : reinterpret_cast<const void *>(&d.data.ptr);
    }
}

/*!
    \fn const void* QVariant::data() const

    \internal
*/

/*! \internal */
void* QVariant::data()
{
    detach();
    return const_cast<void *>(constData());
}


#ifdef QT3_SUPPORT
/*! \internal
 */
void *QVariant::castOrDetach(Type t)
{
    if (d.type != uint(t))
        convert(t);
    else
        detach();
    return data();
}
#endif

/*!
  Returns true if this is a NULL variant, false otherwise.
*/
bool QVariant::isNull() const
{
    return handler->isNull(&d);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QVariant &v)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QVariant(" << v.typeName() << ", ";
    QVariant::handler->debugStream(dbg, v);
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QVariant to QDebug");
    return dbg;
    Q_UNUSED(v);
#endif
}

QDebug operator<<(QDebug dbg, const QVariant::Type p)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QVariant::" << QVariant::typeToName(p);
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QVariant::Type to QDebug");
    return dbg;
    Q_UNUSED(p);
#endif
}
#endif

/*!
    \fn int &QVariant::asInt()

    Use toInt() instead.
*/

/*!
    \fn uint &QVariant::asUInt()

    Use toUInt() instead.
*/

/*!
    \fn qlonglong &QVariant::asLongLong()

    Use toLongLong() instead.
*/

/*!
    \fn qulonglong &QVariant::asULongLong()

    Use toULongLong() instead.
*/

/*!
    \fn bool &QVariant::asBool()

    Use toBool() instead.
*/

/*!
    \fn double &QVariant::asDouble()

    Use toDouble() instead.
*/

/*!
    \fn QByteArray &QVariant::asByteArray()

    Use toByteArray() instead.
*/

/*!
    \fn QBitArray &QVariant::asBitArray()

    Use toBitArray() instead.
*/

/*!
    \fn QString &QVariant::asString()

    Use toString() instead.
*/

/*!
    \fn QStringList &QVariant::asStringList()

    Use toStringList() instead.
*/

/*!
    \fn QDate &QVariant::asDate()

    Use toDate() instead.
*/

/*!
    \fn QTime &QVariant::asTime()

    Use toTime() instead.
*/

/*!
    \fn QDateTime &QVariant::asDateTime()

    Use toDateTime() instead.
*/

/*!
    \fn QList<QVariant> &QVariant::asList()

    Use toList() instead.
*/

/*!
    \fn QMap<QString,QVariant> &QVariant::asMap()

    Use toMap() instead.
*/

/*!
    \fn QVariant::QVariant(bool b, int dummy)

    Use the QVariant(bool) constructor instead.

*/

/*!
    \fn const QByteArray QVariant::toCString() const

    Use toByteArray() instead.
*/

/*!
    \fn QByteArray &QVariant::asCString()

    Use toByteArray() instead.
*/

/*!
    \fn QPoint &QVariant::asPoint()

    Use toPoint() instead.
 */

/*!
    \fn QRect &QVariant::asRect()

    Use toRect() instead.
 */

/*!
    \fn QSize &QVariant::asSize()

    Use toSize() instead.
 */

/*! \fn void QVariant::setValue(const T &value)

    Stores a copy of \a value. If \c{T} is a type that QVariant
    doesn't support, QMetaType is used to store the value. A compile
    error will occur if QMetaType doesn't handle the type.

    Example:

    \code
    QVariant v;

    v.setValue(5);
    int i = v.toInt(); // i is now 5
    QString s = v.toString() // s is now "5"

    MyCustomStruct c;
    v.setValue(c);

    ...

    MyCustomStruct c2 = v.value<MyCustomStruct>();
    \endcode

    \sa value(), fromValue(), canConvert()
 */

/*! \fn T QVariant::value() const

    Returns the stored value converted to the template type \c{T}.
    Call canConvert() to find out whether a type can be converted.
    If the value cannot be converted, \l{default-constructed value}
    will be returned.

    If the type \c{T} is supported by QVariant, this function behaves
    exactly as toString(), toInt() etc.

    Example:

    \code
    QVariant v;

    MyCustomStruct c;
    if (v.canConvert<MyCustomStruct>())
        c = v.value<MyCustomStruct>(v);

    v = 7;
    int i = v.value<int>(); // same as v.toInt()
    QString s = v.value<QString>(); // same as v.toString(), s is now "7"
    MyCustomStruct c2 = v.value<MyCustomStruct>(); // conversion failed, c2 is empty
    \endcode

    \sa setValue(), fromValue(), canConvert()
*/

/*! \fn bool QVariant::canConvert() const

    Returns true if the variant can be converted to the template type \c{T},
    otherwise false.

    Example:
    \code
    QVariant v = 42;

    v.canConvert<int>(); // returns true
    v.canConvert<QString>(); // returns true

    MyCustomStruct s;
    v.setValue(s);

    v.canConvert<int>(); // returns false
    v.canConvert<MyCustomStruct>(); // returns true
    \endcode

    \sa convert()
*/

/*! \fn static QVariant QVariant::fromValue(const T &value)

    Returns a QVariant containing a copy of \a value. Behaves
    exactly like setValue() otherwise.

    Example:
    \code
    MyCustomStruct s;
    return QVariant::fromValue(s);
    \endcode

    \sa setValue(), value()
*/

/*! \fn QVariant qVariantFromValue(const T &value)

    \relates QVariant

    Replacement function for QVariant::fromValue() for compilers
    that do not support template member methods.

    \sa QVariant::fromValue()
*/

/*! \fn void qVariantSetValue(QVariant &v, const T &value)

    \relates QVariant

    Replacement function for QVariant::setValue() for compilers
    that do not support template member methods.

    \sa QVariant::setValue()
*/

/*! \fn T qVariantValue(const QVariant &v)

    \relates QVariant

    Replacement function for QVariant::value() for compilers
    that do not support template member methods.

    \sa QVariant::value()
*/

/*! \fn bool qVariantCanConvert(const QVariant &v)

    \relates QVariant

    Replacement function for QVariant::canConvert() for compilers
    that do not support template member methods.

    \sa QVariant::canConvert()
*/

#endif //QT_NO_VARIANT
