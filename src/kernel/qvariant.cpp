/****************************************************************************
**
** Implementation of QVariant class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qvariant.h"
#ifndef QT_NO_VARIANT
#include "qbitarray.h"
#include "qbitmap.h"
#include "qbytearray.h"
#include "qdatastream.h"
#include "qdatetime.h"
#include "qlist.h"
#include "qmap.h"
#include "qstring.h"

#include <float.h>

#ifndef DBL_DIG
#define DBL_DIG 10
#endif //DBL_DIG

QVariant::Private QVariant::shared_invalid = { Q_ATOMIC_INIT(1), Invalid, true, {0} };


static void construct(QVariant::Private *x, const void *v)
{
    if (v) {
	switch( x->type ) {
	case QVariant::String:
	    x->value.ptr = new QString(*static_cast<const QString *>(v));
	    break;
#ifndef QT_NO_STRINGLIST
	case QVariant::StringList:
	    x->value.ptr = new QStringList(*static_cast<const QStringList *>(v));
	    break;
#endif //QT_NO_STRINGLIST
#ifndef QT_NO_TEMPLATE_VARIANT
	case QVariant::Map:
	    x->value.ptr = new QMap<QString,QVariant>(*static_cast<const QMap<QString, QVariant> *>(v));
	    break;
	case QVariant::List:
	    x->value.ptr = new QList<QVariant>(*static_cast<const QList<QVariant> *>(v));
	    break;
#endif
	case QVariant::Date:
	    x->value.ptr = new QDate(*static_cast<const QDate *>(v));
	    break;
	case QVariant::Time:
	    x->value.ptr = new QTime(*static_cast<const QTime *>(v));
	    break;
	case QVariant::DateTime:
	    x->value.ptr = new QDateTime(*static_cast<const QDateTime *>(v));
	    break;
	case QVariant::ByteArray:
	    x->value.ptr = new QByteArray(*static_cast<const QByteArray *>(v));
	    break;
	case QVariant::BitArray:
	    x->value.ptr = new QBitArray(*static_cast<const QBitArray *>(v));
	    break;
	case QVariant::Int:
	    x->value.i = *static_cast<const int *>(v);
	    break;
	case QVariant::UInt:
	    x->value.u = *static_cast<const uint *>(v);
	    break;
	case QVariant::Bool:
	    x->value.b = *static_cast<const bool *>(v);
	    break;
	case QVariant::Double:
	    x->value.d = *static_cast<const double *>(v);
	    break;
	case QVariant::LongLong:
	    x->value.ll = *static_cast<const Q_LLONG *>(v);
	    break;
	case QVariant::ULongLong:
	    x->value.ull = *static_cast<const Q_ULLONG *>(v);
	    break;
	case QVariant::Invalid:
	    break;
	default:
	    Q_ASSERT( 0 );
	}
	x->is_null = false;
    } else {
	switch (x->type) {
	case QVariant::Invalid:
	    break;
	case QVariant::String:
	    x->value.ptr = new QString;
	    break;
#ifndef QT_NO_STRINGLIST
	case QVariant::StringList:
	    x->value.ptr = new QStringList;
	    break;
#endif //QT_NO_STRINGLIST
#ifndef QT_NO_TEMPLATE_VARIANT
	case QVariant::Map:
	    x->value.ptr = new QMap<QString,QVariant>;
	    break;
	case QVariant::List:
	    x->value.ptr = new QList<QVariant>;
	    break;
#endif
	case QVariant::Date:
	    x->value.ptr = new QDate;
	    break;
	case QVariant::Time:
	    x->value.ptr = new QTime;
	    break;
	case QVariant::DateTime:
	    x->value.ptr = new QDateTime;
	    break;
	case QVariant::ByteArray:
	    x->value.ptr = new QByteArray;
	    break;
	case QVariant::BitArray:
	    x->value.ptr = new QBitArray;
	    break;
	case QVariant::Int:
	    x->value.i = 0;
	    break;
	case QVariant::UInt:
	    x->value.u = 0;
	    break;
	case QVariant::Bool:
	    x->value.b = 0;
	    break;
	case QVariant::Double:
	    x->value.d = 0;
	    break;
	case QVariant::LongLong:
	    x->value.ll = Q_LLONG(0);
	    break;
	case QVariant::ULongLong:
	    x->value.ull = Q_ULLONG(0);
	    break;
	default:
	    Q_ASSERT( 0 );
	}

    }
}

static void clear(QVariant::Private *p)
{
    switch (p->type) {
    case QVariant::String:
	delete static_cast<QString *>(p->value.ptr);
	break;
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	delete static_cast<QStringList *>(p->value.ptr);
	break;
#endif //QT_NO_STRINGLIST
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
	delete static_cast<QMap<QString, QVariant> *>(p->value.ptr);
	break;
    case QVariant::List:
	delete static_cast<QList<QVariant> *>(p->value.ptr);
	break;
#endif
    case QVariant::Date:
	delete static_cast<QDate *>(p->value.ptr);
	break;
    case QVariant::Time:
	delete static_cast<QTime *>(p->value.ptr);
	break;
    case QVariant::DateTime:
	delete static_cast<QDateTime *>(p->value.ptr);
	break;
    case QVariant::ByteArray:
	delete static_cast<QByteArray *>(p->value.ptr);
	break;
    case QVariant::BitArray:
	delete static_cast<QBitArray *>(p->value.ptr);
	break;
    case QVariant::Invalid:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Bool:
    case QVariant::Double:
	break;
    default:
	qFatal("cannot handle GUI types of QVariant without a Gui application");
    }

    p->type = QVariant::Invalid;
    p->is_null = true;
}

static bool isNull(const QVariant::Private *d)
{
    switch( d->type ) {
    case QVariant::String:
	return static_cast<QString *>(d->value.ptr)->isNull();
    case QVariant::Date:
	return static_cast<QDate *>(d->value.ptr)->isNull();
    case QVariant::Time:
	return static_cast<QTime *>(d->value.ptr)->isNull();
    case QVariant::DateTime:
	return static_cast<QDateTime *>(d->value.ptr)->isNull();
    case QVariant::ByteArray:
	return static_cast<QByteArray *>(d->value.ptr)->isNull();
    case QVariant::BitArray:
	return static_cast<QBitArray *>(d->value.ptr)->isNull();
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
#endif //QT_NO_STRINGLIST
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
    case QVariant::List:
#endif
    case QVariant::Invalid:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Bool:
    case QVariant::Double:
	break;
    default:
	qFatal("cannot handle GUI types of QVariant without a Gui application");
    }
    return d->is_null;
}


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
	s >> *static_cast<QMap<QString, QVariant> *>(d->value.ptr);
	break;
    case QVariant::List:
	s >> *static_cast<QList<QVariant> *>(d->value.ptr);
	break;
#endif
    case QVariant::String:
	s >> *static_cast<QString *>(d->value.ptr);
	break;
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	s >> *static_cast<QStringList *>(d->value.ptr);
	break;
#endif // QT_NO_STRINGLIST
    case QVariant::Int:
	s >> d->value.i;
	break;
    case QVariant::UInt:
	s >> d->value.u;
	break;
    case QVariant::LongLong:
	s >> d->value.ll;
	break;
    case QVariant::ULongLong:
	s >> d->value.ull;
	break;
    case QVariant::Bool: {
	Q_INT8 x;
	s >> x;
	d->value.b = x;
    }
	break;
    case QVariant::Double:
	s >> d->value.d;
	break;
    case QVariant::Date:
	s >> *static_cast<QDate *>(d->value.ptr);
	break;
    case QVariant::Time:
	s >> *static_cast<QTime *>(d->value.ptr);
	break;
    case QVariant::DateTime:
	s >> *static_cast<QDateTime *>(d->value.ptr);
	break;
    case QVariant::ByteArray:
	s >> *static_cast<QByteArray *>(d->value.ptr);
	break;
    case QVariant::BitArray:
	s >> *static_cast<QBitArray *>(d->value.ptr);
	break;
    default:
	qFatal("cannot handle GUI types of QVariant without a Gui application");
    }
}


static void save(const QVariant::Private *d, QDataStream &s)
{
    switch (d->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::List:
	s << *static_cast<QList<QVariant> *>(d->value.ptr);
	break;
    case QVariant::Map:
	s << *static_cast<QMap<QString,QVariant> *>(d->value.ptr);
	break;
#endif
    case QVariant::String:
	s << *static_cast<QString *>(d->value.ptr);
	break;
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	s << *static_cast<QStringList *>(d->value.ptr);
	break;
#endif
    case QVariant::Int:
	s << d->value.i;
	break;
    case QVariant::UInt:
	s << d->value.u;
	break;
    case QVariant::LongLong:
	s << d->value.ll;
	break;
    case QVariant::ULongLong:
	s << d->value.ull;
	break;
    case QVariant::Bool:
	s << (Q_INT8)d->value.b;
	break;
    case QVariant::Double:
	s << d->value.d;
	break;
    case QVariant::Date:
	s << *static_cast<QDate *>(d->value.ptr);
	break;
    case QVariant::Time:
	s << *static_cast<QTime *>(d->value.ptr);
	break;
    case QVariant::DateTime:
	s << *static_cast<QDateTime *>(d->value.ptr);
	break;
    case QVariant::ByteArray:
	s << *static_cast<QByteArray *>(d->value.ptr);
	break;
    case QVariant::BitArray:
	s << *static_cast<QBitArray *>(d->value.ptr);
	break;
    case QVariant::Invalid:
	s << QString();
	break;
    default:
	qFatal("cannot handle GUI types of QVariant without a Gui application");
    }
}

static bool compare(const QVariant::Private *a, const QVariant::Private *b)
{
    switch(a->type) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::List:
	return *static_cast<QList<QVariant> *>(a->value.ptr)
	    == *static_cast<QList<QVariant> *>(b->value.ptr);
    case QVariant::Map: {
	QMap<QString,QVariant> *m1 = static_cast<QMap<QString,QVariant> *>(a->value.ptr);
	QMap<QString,QVariant> *m2 = static_cast<QMap<QString,QVariant> *>(b->value.ptr);
	if (m1->count() != m2->count())
	    return false;
	QMap<QString, QVariant>::ConstIterator it = m1->constBegin();
	QMap<QString, QVariant>::ConstIterator it2 = m2->constBegin();
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
	return *static_cast<QString *>(a->value.ptr)
	    == *static_cast<QString *>(b->value.ptr);
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	return *static_cast<QStringList *>(a->value.ptr)
	    == *static_cast<QStringList *>(b->value.ptr);
#endif
    case QVariant::Int:
	return a->value.i == b->value.i;
    case QVariant::UInt:
	return a->value.u == b->value.u;
    case QVariant::LongLong:
	return a->value.ll == b->value.ll;
    case QVariant::ULongLong:
	return a->value.ull == b->value.ull;
    case QVariant::Bool:
	return a->value.b == b->value.b;
    case QVariant::Double:
	return a->value.d == b->value.d;
    case QVariant::Date:
	return *static_cast<QDate *>(a->value.ptr)
	    == *static_cast<QDate *>(b->value.ptr);
    case QVariant::Time:
	return *static_cast<QTime *>(a->value.ptr)
	    == *static_cast<QTime *>(b->value.ptr);
    case QVariant::DateTime:
	return *static_cast<QDateTime *>(a->value.ptr)
	    == *static_cast<QDateTime *>(b->value.ptr);
    case QVariant::ByteArray:
	return *static_cast<QByteArray *>(a->value.ptr)
	    == *static_cast<QByteArray *>(b->value.ptr);
    case QVariant::BitArray:
	return *static_cast<QBitArray *>(a->value.ptr)
	    == *static_cast<QBitArray *>(b->value.ptr);
    case QVariant::Invalid:
	break;
    default:
	qFatal("cannot handle GUI types of QVariant without a Gui application");
    }
    return false;
}


static void cast(QVariant::Private *d, QVariant::Type t, void *result, bool *ok)
{
    Q_ASSERT(d->type !=t);
    switch (t) {
    case QVariant::String: {
	QString *str = static_cast<QString *>(result);
	switch (d->type) {
	case QVariant::Int:
	    *str = QString::number(d->value.i);
	    break;
	case QVariant::UInt:
	    *str = QString::number(d->value.u);
	    break;
	case QVariant::LongLong:
	    *str = QString::number(d->value.ll);
	    break;
	case QVariant::ULongLong:
	    *str = QString::number(d->value.ull);
	    break;
	case QVariant::Double:
	    *str = QString::number(d->value.d, 'g', DBL_DIG);
	    break;
#if !defined(QT_NO_SPRINTF) && !defined(QT_NO_DATESTRING)
	case QVariant::Date:
	    *str = static_cast<QDate *>(d->value.ptr)->toString(Qt::ISODate);
	    break;
	case QVariant::Time:
	    *str = static_cast<QTime *>(d->value.ptr)->toString(Qt::ISODate);
	    break;
	case QVariant::DateTime:
	    *str = static_cast<QDateTime *>(d->value.ptr)->toString(Qt::ISODate);
	    break;
#endif
	case QVariant::Bool:
	    *str = d->value.b ? "true" : "false";
	    break;
	case QVariant::ByteArray:
	    *str = QString(static_cast<QByteArray *>(d->value.ptr)->constData());
	    break;
	default:
	    break;
	}
	break;
    }
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::StringList:
	if (d->type == QVariant::List) {
	    QStringList *slst = static_cast<QStringList *>(result);
	    QList<QVariant> *list = static_cast<QList<QVariant> *>(d->value.ptr);
	    for (int i = 0; i < list->size(); ++i)
		slst->append(list->at(i).toString());
	}
#endif
	break;
    case QVariant::Date: {
	QDate *dt = static_cast<QDate *>(result);
	if (d->type == QVariant::DateTime)
	    *dt = static_cast<QDateTime *>(d->value.ptr)->date();
#ifndef QT_NO_DATESTRING
	else if (d->type == QVariant::String)
	    *dt = QDate::fromString(*static_cast<QString *>(d->value.ptr), Qt::ISODate);
#endif
	break;
    }
    case QVariant::Time: {
	QTime *t = static_cast<QTime *>(result);
	switch ( d->type ) {
	case QVariant::DateTime:
	    *t = static_cast<QDateTime*>(d->value.ptr)->time();
	    break;
#ifndef QT_NO_DATESTRING
	case QVariant::String:
	    *t = QTime::fromString( *static_cast<QString *>(d->value.ptr), Qt::ISODate );
	    break;
#endif
	default:
	    break;
	}
	break;
    }
    case QVariant::DateTime: {
	QDateTime *dt = static_cast<QDateTime *>(result);
	switch ( d->type ) {
#ifndef QT_NO_DATESTRING
	case QVariant::String:
	    *dt = QDateTime::fromString(*static_cast<QString *>(d->value.ptr), Qt::ISODate);
	    break;
#endif
	case QVariant::Date:
	    *dt = QDateTime(*static_cast<QDate*>(d->value.ptr));
	    break;
	default:
	    break;
	}
	break;
    }
    case QVariant::ByteArray: {
	QByteArray *ba = static_cast<QByteArray *>(result);
	if (d->type == QVariant::String)
	    *ba = static_cast<QString *>(d->value.ptr)->toAscii();
    }
    case QVariant::Int: {
	int *i = static_cast<int *>(result);
	switch (d->type) {
	case QVariant::String:
	    *i = static_cast<QString *>(d->value.ptr)->toInt(ok);
	    break;
	case QVariant::ByteArray:
	    *i = QString(*static_cast<QByteArray *>(d->value.ptr)).toInt(ok);
	    break;
	case QVariant::Int:
	    *i = d->value.i;
	    break;
	case QVariant::UInt:
	    *i = (int)d->value.u;
	    break;
	case QVariant::LongLong:
	    *i = (int)d->value.ll;
	    break;
	case QVariant::ULongLong:
	    *i = (int)d->value.ull;
	    break;
	case QVariant::Double:
	    *i = (int)d->value.d;
	    break;
	case QVariant::Bool:
	    *i = (int)d->value.b;
	    break;
	default:
	    *i = 0;
	    break;
	}
	break;
    }
    case QVariant::UInt: {
	uint *u = static_cast<uint *>(result);
	switch (d->type) {
	case QVariant::String:
	    *u = static_cast<QString *>(d->value.ptr)->toUInt(ok);
	    break;
	case QVariant::ByteArray:
	    *u = QString(*static_cast<QByteArray *>(d->value.ptr)).toUInt(ok);
	    break;
	case QVariant::Int:
	    *u = (uint)d->value.i;
	    break;
	case QVariant::UInt:
	    *u = d->value.u;
	    break;
	case QVariant::LongLong:
	    *u = (uint)d->value.ll;
	    break;
	case QVariant::ULongLong:
	    *u = (uint)d->value.ull;
	    break;
	case QVariant::Double:
	    *u = (uint)d->value.d;
	    break;
	case QVariant::Bool:
	    *u = (uint)d->value.b;
	    break;
	default:
	    *u = 0;
	    break;
	}
	break;
    }
    case QVariant::LongLong: {
	Q_LLONG *l = static_cast<Q_LLONG *>(result);
	switch (d->type) {
	case QVariant::String:
	    *l = static_cast<QString *>(d->value.ptr)->toLongLong(ok);
	    break;
	case QVariant::ByteArray:
	    *l = QString(*static_cast<QByteArray *>(d->value.ptr)).toLongLong(ok);
	    break;
	case QVariant::Int:
	    *l = (Q_LLONG)d->value.i;
	    break;
	case QVariant::UInt:
	    *l = (Q_LLONG)d->value.u;
	    break;
	case QVariant::LongLong:
	    *l = d->value.ll;
	    break;
	case QVariant::ULongLong:
	    *l = (Q_LLONG)d->value.ull;
	    break;
	case QVariant::Double:
	    *l = (Q_LLONG)d->value.d;
	    break;
	case QVariant::Bool:
	    *l = (Q_LLONG)d->value.b;
	    break;
	default:
	    *l = 0;
	    break;
	}
	break;
    }
    case QVariant::ULongLong: {
	Q_ULLONG *l = static_cast<Q_ULLONG *>(result);
	switch (d->type) {
	case QVariant::Int:
	    *l = (Q_ULLONG)d->value.i;
	    break;
	case QVariant::UInt:
	    *l = (Q_ULLONG)d->value.u;
	    break;
	case QVariant::LongLong:
	    *l = (Q_ULLONG)d->value.ll;
	    break;
	case QVariant::ULongLong:
	    *l = d->value.ull;
	    break;
	case QVariant::Double:
	    *l = (Q_ULLONG)d->value.d;
	    break;
	case QVariant::Bool:
	    *l = (Q_ULLONG)d->value.b;
	    break;
	case QVariant::String:
	    *l = static_cast<QString *>(d->value.ptr)->toULongLong(ok);
	    break;
	case QVariant::ByteArray:
	    *l = QString(*static_cast<QByteArray *>(d->value.ptr)).toULongLong(ok);
	    break;
	default:
	    *l = 0;
	    break;
	}
	break;
    }
    case QVariant::Bool: {
	bool *b = static_cast<bool *>(result);
	switch(d->type) {
	case QVariant::Double:
	    *b = d->value.d != 0.0;
	    break;
	case QVariant::Int:
	    *b = d->value.i != 0;
	    break;
	case QVariant::UInt:
	    *b = d->value.u != 0;
	    break;
	case QVariant::LongLong:
	    *b = d->value.ll != 0;
	    break;
	case QVariant::ULongLong:
	    *b = d->value.ull != 0;
	    break;
	case QVariant::String:
	{
	    QString str = static_cast<QString *>(d->value.ptr)->lower();
	    *b = !(str == "0" || str == "false" || str.isEmpty());
	    break;
	}
	default:
	    *b = false;
	    break;
	}
	break;
    }
    case QVariant::Double: {
	double *f = static_cast<double *>(result);
	switch (d->type) {
	case QVariant::String:
	    *f = static_cast<QString *>(d->value.ptr)->toDouble(ok);
	    break;
	case QVariant::ByteArray:
	    *f = QString(*static_cast<QByteArray *>(d->value.ptr)).toDouble(ok);
	    break;
	case QVariant::Double:
	    *f = d->value.d;
	    break;
	case QVariant::Int:
	    *f = (double)d->value.i;
	    break;
	case QVariant::Bool:
	    *f = (double)d->value.b;
	    break;
	case QVariant::UInt:
	    *f = (double)d->value.u;
	    break;
	case QVariant::LongLong:
	    *f = (double)d->value.ll;
	    break;
	case QVariant::ULongLong:
#if defined(Q_CC_MSVC) && !defined(Q_CC_MSVC_NET)
	    *f = (double)(Q_LLONG)d->value.ull;
#else
	    *f = (double)d->value.ull;
#endif
	    break;
	default:
	    *f = 0.0;
	    break;
	}
	break;
    }
#ifndef QT_NO_STRINGLIST
    case QVariant::List:
	if (d->type == QVariant::StringList) {
	    QList<QVariant> *lst = static_cast<QList<QVariant> *>(result);
	    QStringList *slist = static_cast<QStringList *>(d->value.ptr);
	    for (int i = 0; i < slist->size(); ++i)
		lst->append(QVariant(slist->at(i)));
	}
#endif //QT_NO_STRINGLIST
	break;

    default:
	Q_ASSERT(0);
    }
}

static bool canCast(QVariant::Private *d, QVariant::Type t)
{
    if (d->type == t)
	return true;

    switch ( t ) {
    case QVariant::Bool:
	return d->type == QVariant::Double || d->type == QVariant::Int
	    || d->type == QVariant::UInt || d->type == QVariant::LongLong
	    || d->type == QVariant::ULongLong || d->type == QVariant::String;
    case QVariant::Int:
	return d->type == QVariant::String || d->type == QVariant::Double
	    || d->type == QVariant::Bool || d->type == QVariant::UInt
	    || d->type == QVariant::LongLong || d->type == QVariant::ULongLong;
    case QVariant::UInt:
	return d->type == QVariant::String || d->type == QVariant::Double
	    || d->type == QVariant::Bool || d->type == QVariant::Int
	    || d->type == QVariant::LongLong || d->type == QVariant::ULongLong;
    case QVariant::LongLong:
	return d->type == QVariant::String || d->type == QVariant::Double
	    || d->type == QVariant::Bool || d->type == QVariant::Int
	    || d->type == QVariant::UInt || d->type == QVariant::ULongLong;
    case QVariant::ULongLong:
	return d->type == QVariant::String || d->type == QVariant::Double
	    || d->type == QVariant::Bool || d->type == QVariant::Int
	    || d->type == QVariant::UInt || d->type == QVariant::LongLong;
    case QVariant::Double:
	return d->type == QVariant::String || d->type == QVariant::Int
	    || d->type == QVariant::Bool || d->type == QVariant::UInt
	    || d->type == QVariant::LongLong || d->type == QVariant::ULongLong;
    case QVariant::String:
	return d->type == QVariant::ByteArray || d->type == QVariant::Int
	    || d->type == QVariant::UInt || d->type == QVariant::Bool
	    || d->type == QVariant::Double || d->type == QVariant::Date
	    || d->type == QVariant::Time || d->type == QVariant::DateTime
	    || d->type == QVariant::LongLong || d->type == QVariant::ULongLong;
    case QVariant::ByteArray:
	return d->type == QVariant::CString || d->type == QVariant::String;
    case QVariant::Date:
	return d->type == QVariant::String || d->type == QVariant::DateTime;
    case QVariant::Time:
	return d->type == QVariant::String || d->type == QVariant::DateTime;
    case QVariant::DateTime:
	return d->type == QVariant::String || d->type == QVariant::Date;
#ifndef QT_NO_STRINGLIST
    case QVariant::List:
	return d->type == QVariant::StringList;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::StringList:
	if (d->type == QVariant::List) {
	    const QList<QVariant> &varlist = *static_cast<QList<QVariant> *>(d->value.ptr);
	    for (int i = 0; i < varlist.size(); ++i) {
		if (!varlist.at(i).canCast(QVariant::String))
		    return false;
	    }
	    return true;
	}
	return false;
#endif
    default:
	return false;
    }
}

extern const QVariant::Handler qt_kernel_variant_handler;
const QVariant::Handler qt_kernel_variant_handler = {
    construct,
    clear,
    isNull,
    load,
    save,
    compare,
    cast,
    canCast
};

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
    canCast().

    The methods named toT() (for any supported T, see the \c Type
    documentation for a list) are const. If you ask for the stored
    type, they return a copy of the stored object. If you ask for a
    type that can be generated from the stored type, toT() copies and
    converts and leaves the object itself unchanged. If you ask for a
    type that cannot be generated from the stored type, the result
    depends on the type (see the function documentation for details).

    Note that two data types supported by QVariant are explicitly
    shared, namely QImage and QPointArray, and in these
    cases the toT() methods return a shallow copy. In almost all cases
    you must make a deep copy of the returned values before modifying
    them.

    The asT() functions are not const. They do conversion like the
    toT() methods, set the variant to hold the converted value, and
    return a reference to the new contents of the variant.

    Here is some example code to demonstrate the use of QVariant:

    \code
    QDataStream out(...);
    QVariant v(123);          // The variant now contains an int
    int x = v.toInt();        // x = 123
    out << v;                 // Writes a type tag and an int to out
    v = QVariant("hello");    // The variant now contains a QByteArray
    v = QVariant(tr("hello"));// The variant now contains a QString
    int y = v.toInt();        // y = 0 since v cannot be converted to an int
    QString s = v.toString(); // s = tr("hello")  (see QObject::tr())
    out << v;                 // Writes a type tag and a QString to out
    ...
    QDataStream in(...);      // (opening the previously written stream)
    in >> v;                  // Reads an Int variant
    int z = v.toInt();        // z = 123
    qDebug("Type is %s",      // prints "Type is int"
	    v.typeName());
    v.asInt() += 100;	      // The variant now hold the value 223.
    v = QVariant( QStringList() );
    v.asStringList().append( "Hello" );
    \endcode

    You can even store QList<QVariant>s and
    QMap<QString,QVariant>s in a variant, so you can easily construct
    arbitrarily complex data structures of arbitrary types. This is
    very powerful and versatile, but may prove less memory and speed
    efficient than storing specific types in standard data structures.

    QVariant also supports the notion of NULL values, where you have a
    defined type with no value set.
    \code
    QVariant x, y( QString() ), z( QString("") );
    x.asInt();
    // x.isNull() == true, y.isNull() == true, z.isNull() == false
    \endcode

    See the \link collection.html Collection Classes\endlink.
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
    \value IconSet  a QIconSet
    \value Image  a QImage
    \value Int  an int
    \value KeySequence  a QKeySequence
    \value List  a QList<QVariant>
    \value LongLong a long long
    \value ULongLong an unsigned long long
    \value Map  a QMap<QString,QVariant>
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

    Note that Qt's definition of bool depends on the compiler.
    \c qglobal.h has the system-dependent definition of bool.
*/

/*!
  \fn QVariant::QVariant()

    Constructs an invalid variant.
*/

/*!
  \fn QVariant::QVariant(Type type, void *v)

    \internal

    Constructs a variant of type \a type, and initializes with \a v if
    \a not 0.
*/


QVariant::Private *QVariant::create(Type t, const void *v)
{
    Private *x = new Private;
    x->ref = 1;
    x->type = t;
    x->is_null = true;
    handler->construct(x, v);
    return x;
}

/*!
  \fn QVariant::~QVariant()

    Destroys the QVariant and the contained object.

    Note that subclasses that reimplement clear() should reimplement
    the destructor to call clear(). This destructor calls clear(), but
    because it is the destructor, QVariant::clear() is called rather
    than a subclass's clear().
*/

/*!
  \fn QVariant::QVariant(const QVariant &p)

    Constructs a copy of the variant, \a p, passed as the argument to
    this constructor. Usually this is a deep copy, but a shallow copy
    is made if the stored data type is explicitly shared, as e.g.
    QImage is.
*/

#ifndef QT_NO_DATASTREAM
/*!
    Reads the variant from the data stream, \a s.
*/
QVariant::QVariant(QDataStream &s)
{
    d = new Private;
    d->ref = 1;
    d->is_null = true;
    s >> *this;
}
#endif //QT_NO_DATASTREAM

/*!
  \fn QVariant::QVariant(const QString &val)

    Constructs a new variant with a string value, \a val.
*/

/*!
  \fn QVariant::QVariant(const char *val)

    Constructs a new variant with a C-string value of \a val if \a val
    is non-null. The variant creates a deep copy of \a val.

    If \a val is null, the resulting variant has type Invalid.
*/

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
  \fn QVariant::QVariant(int val)

    Constructs a new variant with an integer value, \a val.
*/

/*!
  \fn QVariant::QVariant(uint val)

    Constructs a new variant with an unsigned integer value, \a val.
*/

/*!
  \fn QVariant::QVariant(Q_LLONG val)

    Constructs a new variant with a long long integer value, \a val.
*/

/*!
  \fn QVariant::QVariant(Q_ULLONG val)

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
    Assigns the value of the variant \a variant to this variant.

    This is a deep copy of the variant, but note that if the variant
    holds an explicitly shared type such as QImage, a shallow copy is
    performed.
*/
QVariant& QVariant::operator=(const QVariant &variant)
{
    Private *x = variant.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	cleanUp(x);
    return *this;
}

/*!
    \internal
*/
void QVariant::detach_helper()
{
    Private *x = new Private;
    x->ref = 1;
    x->type = d->type;
    x->is_null = true;
    handler->construct(x, data());
    x->is_null = d->is_null;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	cleanUp(x);
}

/*!
    Returns the name of the type stored in the variant. The returned
    strings describe the C++ datatype used to store the data: for
    example, "QFont", "QString", or "QList<QVariant>". An Invalid
    variant returns 0.
*/
const char *QVariant::typeName() const
{
    return typeToName(d->type);
}

/*!
    Convert this variant to type Invalid and free up any resources
    used.
*/
void QVariant::clear()
{
    if (d->ref != 1) {
	if (!--d->ref)
	    cleanUp(d);
	d = &shared_invalid;
	return;
    }
    handler->clear(d);
}

/* Attention!

   For dependency reasons, this table is duplicated in moc.y. If you
   change one, change both.

   (Search for the word 'Attention' in moc.y.)
*/
static const int ntypes = 35;
static const char* const type_map[ntypes] =
{
    0,
    "QMap<QString,QVariant>",
    "QList<QVariant>",
    "QString",
    "QStringList",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
#ifndef QT_NO_COMPAT
    "QColorGroup",
#endif
    "QIconSet",
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
    "Q_ULLONG"
};


/*!
    Converts the enum representation of the storage type, \a typ, to
    its string representation.
*/
const char *QVariant::typeToName(Type typ)
{
    if (typ >= ntypes)
	return 0;
    return type_map[typ];
}


/*!
    Converts the string representation of the storage type gven in \a
    name, to its enum representation.

    If the string representation cannot be converted to any enum
    representation, the variant is set to \c Invalid.
*/
QVariant::Type QVariant::nameToType(const char *name)
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
    Q_UINT32 u;
    s >> u;
    QVariant::Private *x = create((QVariant::Type)u, 0);
    x->is_null = false;
    handler->load(x, s);
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
	cleanUp(x);
}

/*!
    Internal function for saving a variant to the stream \a s. Use the
    stream operators instead.

    \internal
*/
void QVariant::save(QDataStream &s) const
{
    s << (Q_UINT32)type();
    handler->save(d, s);
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
    Q_UINT32 u;
    s >> u;
    p = (QVariant::Type)u;

    return s;
}

/*!
    Writes a variant type \a p to the stream \a s.
*/
QDataStream& operator<<(QDataStream &s, const QVariant::Type p)
{
    s << (Q_UINT32)p;

    return s;
}

#endif //QT_NO_DATASTREAM

/*!
    \fn Type QVariant::type() const

    Returns the storage type of the value stored in the variant.
    Usually it's best to test with canCast() whether the variant can
    deliver the data type you are interested in.
*/

/*!
    \fn bool QVariant::isValid() const

    Returns true if the storage type of this variant is not
    QVariant::Invalid; otherwise returns false.
*/

/*! \fn QByteArray QVariant::toCString() const
  \obsolete
    Returns the variant as a QCString if the variant has type()
    CString or String; otherwise returns 0.

    \sa asCString()
*/

#define Q_VARIANT_TO(f) \
Q##f QVariant::to##f() const { \
    if ( d->type == f ) \
        return *static_cast<Q##f *>(d->value.ptr); \
    Q##f ret; \
    handler->cast(d, f, &ret, 0); \
    return ret; \
}

Q_VARIANT_TO(String)
#ifndef QT_NO_STRINGLIST
Q_VARIANT_TO(StringList)
#endif
Q_VARIANT_TO(Date)
Q_VARIANT_TO(Time)
Q_VARIANT_TO(DateTime)
Q_VARIANT_TO(ByteArray)

/*!
  \fn QString QVariant::toString() const

    Returns the variant as a QString if the variant has type() String,
    ByteArray, Int, Uint, Bool, Double, Date, Time, DateTime,
    KeySequence, Font or Color; otherwise returns QString::null.

    \sa asString()
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
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa asStringList()
*/


#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant as a QMap<QString,QVariant> if the variant has
    type() Map; otherwise returns an empty map.

    Note that if you want to iterate over the map, you should iterate
    over a copy, e.g.
    \code
    QMap<QString, QVariant> map = myVariant.toMap();
    QMap<QString, QVariant>::Iterator it = map.begin();
    while( it != map.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa asMap()
*/
QMap<QString, QVariant> QVariant::toMap() const
{
    if (d->type != Map)
	return QMap<QString,QVariant>();

    return *static_cast<QMap<QString, QVariant> *>(d->value.ptr);
}
#endif

/*!
  \fn QDate QVariant::toDate() const

    Returns the variant as a QDate if the variant has type() Date,
    DateTime or String; otherwise returns an invalid date.

    Note that if the type() is String an invalid date will be returned
    if the string cannot be parsed as a Qt::ISODate format date.

    \sa asDate()
*/


/*!
  \fn QTime QVariant::toTime() const

    Returns the variant as a QTime if the variant has type() Time,
    DateTime or String; otherwise returns an invalid time.

    Note that if the type() is String an invalid time will be returned
    if the string cannot be parsed as a Qt::ISODate format time.

    \sa asTime()
*/

/*!
  \fn QDateTime QVariant::toDateTime() const

    Returns the variant as a QDateTime if the variant has type()
    DateTime or String; otherwise returns an invalid date/time.

    Note that if the type() is String an invalid date/time will be
    returned if the string cannot be parsed as a Qt::ISODate format
    date/time.

    \sa asDateTime()
*/

/*!
  \fn QByteArray QVariant::toByteArray() const

    Returns the variant as a QByteArray if the variant has type()
    ByteArray; otherwise returns an empty bytearray.

    \sa asByteArray()
*/

/*!
    Returns the variant as a QBitArray if the variant has type()
    BitArray; otherwise returns an empty bitarray.

    \sa asBitArray()
*/
QBitArray QVariant::toBitArray() const
{
    if (d->type == BitArray)
	return *static_cast<QBitArray *>(d->value.ptr);
    return QBitArray();
}

/*!
    Returns the variant as an int if the variant has type() String,
    Int, UInt, Double, Bool or KeySequence; otherwise returns
    0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an int; otherwise \a *ok is set to false.

    \sa asInt() canCast()
*/
int QVariant::toInt(bool *ok) const
{
    if (d->type == Int) {
	if (ok)
	    *ok = true;
	return d->value.i;
    }

    bool c = canCast(Int);
    if (ok)
	*ok = c;
    int res = 0;
    if (c)
	handler->cast(d, Int, &res, ok);

    return res;
}

/*!
    Returns the variant as an unsigned int if the variant has type()
    String, ByteArray, UInt, Int, Double, or Bool; otherwise returns 0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an unsigned int; otherwise \a *ok is set to false.

    \sa asUInt()
*/
uint QVariant::toUInt(bool *ok) const
{
    if (d->type == UInt) {
	if (ok)
	    *ok = true;
	return d->value.u;
    }

    bool c = canCast(UInt);
    if (ok)
	*ok = c;
    uint res = 0;
    if (c)
	handler->cast(d, UInt, &res, ok);

    return res;
}

/*!
    Returns the variant as a long long int if the variant has type()
    LongLong, ULongLong, any type allowing a toInt() conversion;
    otherwise returns 0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an int; otherwise \a *ok is set to false.

    \sa asLongLong() canCast()
*/
Q_LLONG QVariant::toLongLong(bool *ok) const
{
    if (d->type == LongLong) {
	if (ok)
	    *ok = true;
	return d->value.ll;
    }

    bool c = canCast(LongLong);
    if (ok)
	*ok = c;
    Q_LLONG res = 0;
    if (c)
	handler->cast(d, LongLong, &res, ok);

    return res;
}

/*!
    Returns the variant as as an unsigned long long int if the variant
    has type() LongLong, ULongLong, any type allowing a toUInt()
    conversion; otherwise returns 0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to an int; otherwise \a *ok is set to false.

    \sa asULongLong() canCast()
*/
Q_ULLONG QVariant::toULongLong(bool *ok) const
{
    if (d->type == ULongLong) {
	if (ok)
	    *ok = true;
	return d->value.ull;
    }

    bool c = canCast(ULongLong);
    if (ok)
	*ok = c;
    Q_ULLONG res = 0;
    if (c)
	handler->cast(d, ULongLong, &res, ok);

    return res;
}

/*!
    Returns the variant as a bool if the variant has type() Bool.

    Returns true if the variant has type Int, UInt or Double and its
    value is non-zero, or if the variant has type String and its lower-case
    content is not empty, "0" or "false"; otherwise returns false.

    \sa asBool()
*/
bool QVariant::toBool() const
{
    if (d->type == Bool)
	return d->value.b;

    bool res = false;
    handler->cast(d, Bool, &res, 0);

    return res;
}

/*!
    Returns the variant as a double if the variant has type() String,
    ByteArray, Double, Int, UInt, LongLong, ULongLong or Bool; otherwise
    returns 0.0.

    If \a ok is non-null: \a *ok is set to true if the value could be
    converted to a double; otherwise \a *ok is set to false.

    \sa asDouble()
*/
double QVariant::toDouble(bool *ok) const
{
    if (d->type == Double) {
	if (ok)
	*ok = true;
	return d->value.d;
    }

    bool c = canCast(Double);
    if (ok)
	*ok = c;
    double res = 0;
    if (c)
	handler->cast(d, Double, &res, ok);

    return res;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
  \fn QList<QVariant> QVariant::toList() const

    Returns the variant as a QList<QVariant> if the variant has
    type() List or StringList; otherwise returns an empty list.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<QVariant> list = myVariant.toList();
    QList<QVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa asList()
*/
QList<QVariant> QVariant::toList() const
{
    if (d->type == List)
	return *static_cast<QList<QVariant> *>(d->value.ptr);
    QList<QVariant> res;
    handler->cast(d, List, &res, 0);
    return res;
}
#endif


/*!
    \fn QString& QVariant::asString()

    Tries to convert the variant to hold a string value. If that is
    not possible the variant is set to an empty string.

    Returns a reference to the stored string.

    \sa toString()
*/

/*!
    \fn QCString& QVariant::asCString()

    \obsolete

    Tries to convert the variant to hold a string value. If that is
    not possible the variant is set to an empty string.

    Returns a reference to the stored string.

    \sa toCString()
*/

/*!
    \fn QStringList& QVariant::asStringList()

    Tries to convert the variant to hold a QStringList value. If that
    is not possible the variant is set to an empty string list.

    Returns a reference to the stored string list.

    Note that if you want to iterate over the list, you should
    iterate over a copy, e.g.
    \code
    QStringList list = myVariant.asStringList();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa toStringList()
*/

/*!
    \fn QFont& QVariant::asFont()

    Tries to convert the variant to hold a QFont. If that is not
    possible the variant is set to the application's default font.

    Returns a reference to the stored font.

    \sa toFont()
*/

/*!
    \fn QPixmap& QVariant::asPixmap()

    Tries to convert the variant to hold a pixmap value. If that is
    not possible the variant is set to a null pixmap.

    Returns a reference to the stored pixmap.

    \sa toPixmap()
*/

/*!
    \fn QImage& QVariant::asImage()

    Tries to convert the variant to hold an image value. If that is
    not possible the variant is set to a null image.

    Returns a reference to the stored image.

    \sa toImage()
*/

/*!
    \fn QBrush& QVariant::asBrush()

    Tries to convert the variant to hold a brush value. If that is not
    possible the variant is set to a default black brush.

    Returns a reference to the stored brush.

    \sa toBrush()
*/

/*!
    \fn QPoint& QVariant::asPoint()

    Tries to convert the variant to hold a point value. If that is not
    possible the variant is set to a (0, 0) point.

    Returns a reference to the stored point.

    \sa toPoint()
*/

/*!
    \fn QRect& QVariant::asRect()

    Tries to convert the variant to hold a rectangle value. If that is
    not possible the variant is set to an empty rectangle.

    Returns a reference to the stored rectangle.

    \sa toRect()
*/

/*!
    \fn QSize& QVariant::asSize()

    Tries to convert the variant to hold a QSize value. If that is not
    possible the variant is set to an invalid size.

    Returns a reference to the stored size.

    \sa toSize() QSize::isValid()
*/

/*!
    \fn QSizePolicy& QVariant::asSizePolicy()

    Tries to convert the variant to hold a QSizePolicy value. If that
    fails, the variant is set to an arbitrary (valid) size policy.
*/


/*!
    \fn QColor& QVariant::asColor()

    Tries to convert the variant to hold a QColor value. If that is
    not possible the variant is set to an invalid color.

    Returns a reference to the stored color.

    \sa toColor() QColor::isValid()
*/

/*!
    \fn QPalette& QVariant::asPalette()

    Tries to convert the variant to hold a QPalette value. If that is
    not possible the variant is set to a palette of black colors.

    Returns a reference to the stored palette.

    \sa toString()
*/

/*!
    \fn QIconSet& QVariant::asIconSet()

    Tries to convert the variant to hold a QIconSet value. If that is
    not possible the variant is set to an empty iconset.

    Returns a reference to the stored iconset.

    \sa toIconSet()
*/

/*!
    \fn QPointArray& QVariant::asPointArray()

    Tries to convert the variant to hold a QPointArray value. If that
    is not possible the variant is set to an empty point array.

    Returns a reference to the stored point array.

    \sa toPointArray()
*/

/*!
    \fn QBitmap& QVariant::asBitmap()

    Tries to convert the variant to hold a bitmap value. If that is
    not possible the variant is set to a null bitmap.

    Returns a reference to the stored bitmap.

    \sa toBitmap()
*/

/*!
    \fn QRegion& QVariant::asRegion()

    Tries to convert the variant to hold a QRegion value. If that is
    not possible the variant is set to a null region.

    Returns a reference to the stored region.

    \sa toRegion()
*/

/*!
    \fn QCursor& QVariant::asCursor()

    Tries to convert the variant to hold a QCursor value. If that is
    not possible the variant is set to a default arrow cursor.

    Returns a reference to the stored cursor.

    \sa toCursor()
*/

/*!
    \fn QDate& QVariant::asDate()

    Tries to convert the variant to hold a QDate value. If that is not
    possible then the variant is set to an invalid date.

    Returns a reference to the stored date.

    \sa toDate()
*/

/*!
    \fn QTime& QVariant::asTime()

    Tries to convert the variant to hold a QTime value. If that is not
    possible then the variant is set to an invalid time.

    Returns a reference to the stored time.

    \sa toTime()
*/

/*!
    \fn QDateTime& QVariant::asDateTime()

    Tries to convert the variant to hold a QDateTime value. If that is
    not possible then the variant is set to an invalid date/time.

    Returns a reference to the stored date/time.

    \sa toDateTime()
*/

/*!
    \fn QByteArray& QVariant::asByteArray()

    Tries to convert the variant to hold a QByteArray value. If that
    is not possible then the variant is set to an empty bytearray.

    Returns a reference to the stored bytearray.

    \sa toByteArray()
*/

/*!
    \fn QBitArray& QVariant::asBitArray()

    Tries to convert the variant to hold a QBitArray value. If that is
    not possible then the variant is set to an empty bitarray.

    Returns a reference to the stored bitarray.

    \sa toBitArray()
*/

/*!
    \fn QKeySequence& QVariant::asKeySequence()

    Tries to convert the variant to hold a QKeySequence value. If that
    is not possible then the variant is set to an empty key sequence.

    Returns a reference to the stored key sequence.

    \sa toKeySequence()
*/

/*! \fn QPen& QVariant::asPen()

  Tries to convert the variant to hold a QPen value. If that
  is not possible then the variant is set to an empty pen.

  Returns a reference to the stored pen.

  \sa toPen()
*/

/*!
  \fn int &QVariant::asInt()

    Returns the variant's value as int reference.
*/

/*!
  \fn uint &QVariant::asUInt()

    Returns the variant's value as unsigned int reference.
*/

/*!
  \fn Q_LLONG &QVariant::asLongLong()

    Returns the variant's value as long long reference.
*/

/*!
  \fn Q_ULLONG &QVariant::asULongLong()

    Returns the variant's value as unsigned long long reference.
*/

/*!
  \fn bool &QVariant::asBool()

    Returns the variant's value as bool reference.
*/

/*!
  \fn double &QVariant::asDouble()

    Returns the variant's value as double reference.
*/

/*!
  \fn QList<QVariant>& QVariant::asList()

    Returns the variant's value as variant list reference.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<QVariant> list = myVariant.asList();
    QList<QVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/

/*!
  \fn QMap<QString, QVariant>& QVariant::asMap()

    Returns the variant's value as variant map reference.

    Note that if you want to iterate over the map, you should iterate
    over a copy, e.g.
    \code
    QMap<QString, QVariant> map = myVariant.asMap();
    QMap<QString, QVariant>::Iterator it = map.begin();
    while( it != map.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/

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
bool QVariant::canCast(Type t) const
{
    return handler->canCast(d, t);
}

/*!
    Casts the variant to the requested type. If the cast cannot be
    done, the variant is set to the default value of the requested
    type (e.g. an empty string if the requested type \a t is
    QVariant::String, an empty point array if the requested type \a t
    is QVariant::PointArray, etc). Returns true if the current type of
    the variant was successfully cast; otherwise returns false.

    \sa canCast()
*/

bool QVariant::cast(Type t)
{
    if (d->type == t)
	return true;

    bool c = handler->canCast(d, t);

    Private *x = create(t, 0);
    x = qAtomicSetPtr(&d, x);
    if (c)
	handler->cast(x, t, data(), 0);
    if (!--x->ref)
	cleanUp(x);
    return c;
}

/*!
    Compares this QVariant with \a v and returns true if they are
    equal; otherwise returns false.
*/

bool QVariant::operator==(const QVariant &v) const
{
    QVariant v2 = v;
    if (d->type != v2.d->type) {
	if (!v2.canCast(d->type))
	    return false;
	v2.cast(d->type);
    }
    return handler->compare(d, v2.d);
}

/*!
    \fn bool QVariant::operator!=( const QVariant &v ) const
    Compares this QVariant with \a v and returns true if they are not
    equal; otherwise returns false.
*/

/*! \internal

  Reads or sets the variant type and ptr
 */
void *QVariant::rawAccess(void *ptr, Type typ, bool deepCopy)
{
    if (ptr) {
	clear();
	d->type = typ;
	d->value.ptr = ptr;
	d->is_null = false;
	if (deepCopy) {
	    Private *x = new Private;
	    x->ref = 1;
	    x->type = d->type;
	    handler->construct(x, data());
	    x->is_null = d->is_null;
	    x = qAtomicSetPtr(&d, x);
	    if (!--x->ref)
		cleanUp(x);
	}
    }
    if (!deepCopy)
	return d->value.ptr;
    Private *p = new Private;
    p->type = d->type;
    handler->construct(p, data());
    void *ret = (void*)p->value.ptr;
    p->type = Invalid;
    delete p;
    return ret;
}

/*! \internal
 */
void* QVariant::data()
{
    switch(d->type) {
    case Int:
    case UInt:
    case LongLong:
    case ULongLong:
    case Double:
    case Bool:
	return &d->value;
    default:
	return d->value.ptr;
    }
}


/*! \internal
 */
void *QVariant::castOrDetach(Type t)
{
    if ( d->type != t )
	cast(t);
    else
	detach();
    return data();
}

/*!
  Returns true if this is a NULL variant, false otherwise.
*/
bool QVariant::isNull() const
{
    return handler->isNull(d);
}


#endif //QT_NO_VARIANT
