/****************************************************************************
**
** Definition of QCoreVariant class.
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

#ifndef QCOREVARIANT_H
#define QCOREVARIANT_H

#ifndef QT_H
#include "qatomic.h"
#include "qbytearray.h"
#endif // QT_H

#ifndef QT_NO_VARIANT
class QBitArray;
class QDataStream;
class QDate;
class QDateTime;
class QCoreVariant;
class QString;
class QStringList;
class QStringList;
class QTime;

template <class Type> class QList;
template <class Key, class Type> class QMap;

class Q_CORE_EXPORT QCoreVariant
{
 public:
    enum Type {
	Invalid = 0,
	Map = 1,
	List = 2,
	String = 3,
	StringList =4,
	Font = 5,
	Pixmap = 6,
	Brush = 7,
	Rect = 8,
	Size = 9,
	Color = 10,
	Palette = 11,
#ifdef QT_COMPAT
	ColorGroup = 12,
#endif
	IconSet = 13,
	Point = 14,
	Image = 15,
	Int = 16,
	UInt = 17,
	Bool = 18,
	Double = 19,
	PointArray = 21,
	Region = 22,
	Bitmap = 23,
	Cursor = 24,
	SizePolicy = 25,
	Date = 26,
	Time = 27,
	DateTime = 28,
	ByteArray = 29,
#ifndef Q_QDOC
	CString = ByteArray,
#endif
	BitArray = 30,
	KeySequence = 31,
	Pen = 32,
	LongLong = 33,
	ULongLong = 34
    };

    inline QCoreVariant();
    inline ~QCoreVariant();
    inline QCoreVariant(Type type, void *v = 0);
    inline QCoreVariant(const QCoreVariant &other);

#ifndef QT_NO_DATASTREAM
    inline QCoreVariant(QDataStream &s);
#endif

    inline QCoreVariant(int i);
    inline QCoreVariant(uint ui);
    inline QCoreVariant(Q_LLONG ll);
    inline QCoreVariant(Q_ULLONG ull);
    inline QCoreVariant(bool b);
    inline QCoreVariant(double d);

    inline QCoreVariant(const char *str);
    inline QCoreVariant(const QByteArray &bytearray);
    inline QCoreVariant(const QBitArray &bitarray);
    inline QCoreVariant(const QString &string);
#ifndef QT_NO_STRINGLIST
    inline QCoreVariant(const QStringList &stringlist);
#endif

    inline QCoreVariant(const QDate &date);
    inline QCoreVariant(const QTime &time);
    inline QCoreVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QCoreVariant(const QList<QCoreVariant> &list);
    inline QCoreVariant(const QMap<QString,QCoreVariant> &map);
#endif

    QCoreVariant& operator=(const QCoreVariant &other);

    bool operator==(const QCoreVariant &other) const;
    inline bool operator!=(const QCoreVariant &other) const
    { return !(other == *this); }

    inline Type type() const;
    const char *typeName() const;

    bool canCast(Type t) const;
    bool cast(Type t);

    inline bool isValid() const;
    bool isNull() const;

    void clear();


    inline void detach() { if (d->ref != 1) detach_helper(); }
    inline bool isDetached() const;

    int toInt(bool *ok = 0) const;
    uint toUInt(bool *ok = 0) const;
    Q_LLONG toLongLong(bool *ok = 0) const;
    Q_ULLONG toULongLong(bool *ok = 0) const;
    bool toBool() const;
    double toDouble(bool *ok = 0) const;
    QByteArray toByteArray() const;
    QBitArray toBitArray() const;
    QString toString() const;
#ifndef QT_NO_STRINGLIST
    QStringList toStringList() const;
#endif
    QDate toDate() const;
    QTime toTime() const;
    QDateTime toDateTime() const;
#ifndef QT_NO_TEMPLATE_VARIANT
    QList<QCoreVariant> toList() const;
    QMap<QString,QCoreVariant> toMap() const;
#endif

#ifdef QT_COMPAT
    inline QT_COMPAT int &asInt();
    inline QT_COMPAT uint &asUInt();
    inline QT_COMPAT Q_LLONG &asLongLong();
    inline QT_COMPAT Q_ULLONG &asULongLong();
    inline QT_COMPAT bool &asBool();
    inline QT_COMPAT double &asDouble();
    inline QT_COMPAT QByteArray &asByteArray();
    inline QT_COMPAT QBitArray &asBitArray();
    inline QT_COMPAT QString &asString();
#ifndef QT_NO_STRINGLIST
    inline QT_COMPAT QStringList &asStringList();
#endif
    inline QT_COMPAT QDate &asDate();
    inline QT_COMPAT QTime &asTime();
    inline QT_COMPAT QDateTime &asDateTime();
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QT_COMPAT QList<QCoreVariant> &asList();
    inline QT_COMPAT QMap<QString,QCoreVariant> &asMap();
#endif
#endif //QT_COMPAT

#ifndef QT_NO_DATASTREAM
    void load(QDataStream &ds);
    void save(QDataStream &ds) const;
#endif
    static const char *typeToName(Type type);
    static Type nameToType(const char *name);

#ifdef QT_COMPAT
    QCoreVariant(bool, int);
    inline QT_COMPAT const QByteArray toCString() const { return toByteArray(); }
    inline QT_COMPAT QByteArray &asCString() { return *static_cast<QByteArray *>(castOrDetach(ByteArray)); }
#endif

    void *rawAccess(void *ptr = 0, Type typ = Invalid, bool deepCopy = FALSE);
    void *data();
 private:
    void detach_helper();

 public:
    struct Private
    {
	QAtomic ref;
	uint type : 31;
	uint is_null : 1;
	union
	{
	    uint u;
	    int i;
	    Q_LLONG ll;
	    Q_ULLONG ull;
	    bool b;
	    double d;
	    void *ptr;
	} value;
	mutable void *str_cache;
    };
 public:
    typedef void (*f_construct)(Private *, const void *);
    typedef void (*f_clear)(Private *);
    typedef bool (*f_null)(const Private *);
#ifndef QT_NO_DATASTREAM
    typedef void (*f_load)(Private *, QDataStream &);
    typedef void (*f_save)(const Private *, QDataStream &);
#endif
    typedef bool (*f_compare)(const Private *, const Private *);
    typedef void (*f_cast)(QCoreVariant::Private *d, Type t, void *, bool *);
    typedef bool (*f_canCast)(QCoreVariant::Private *d, Type t);
    struct Handler {
	f_construct construct;
	f_clear clear;
	f_null isNull;
#ifndef QT_NO_DATASTREAM
	f_load load;
	f_save save;
#endif
	f_compare compare;
	f_cast cast;
	f_canCast canCast;
    };

protected:
    Private *d;
    static Private shared_invalid;

    static const Handler *handler;

    Private *create(Type t, const void *v);
    inline void cleanUp(Private *p)
    { handler->clear(p); delete p; }
    void *castOrDetach(Type t);
};


inline QCoreVariant::QCoreVariant() :d(&shared_invalid)
{ ++d->ref; }
inline QCoreVariant::~QCoreVariant()
{ if (!--d->ref) cleanUp(d); }
inline QCoreVariant::QCoreVariant(Type type, void *v)
{ d = create(type, v); }
inline QCoreVariant::QCoreVariant(const QCoreVariant &p) : d(p.d)
{ ++d->ref; }

inline QCoreVariant::QCoreVariant(int val)
{ d = create(Int, &val); }
inline QCoreVariant::QCoreVariant(uint val)
{ d = create(UInt, &val); }
inline QCoreVariant::QCoreVariant(Q_LLONG val)
{ d = create(LongLong, &val); }
inline QCoreVariant::QCoreVariant(Q_ULLONG val)
{ d = create(ULongLong, &val); }
inline QCoreVariant::QCoreVariant(bool val)
{ d = create(Bool, &val); }
inline QCoreVariant::QCoreVariant(double val)
{ d = create(Double, &val); }

inline QCoreVariant::QCoreVariant(const char *val)
{ QByteArray ba(val); d = create(ByteArray, &ba); }
inline QCoreVariant::QCoreVariant(const QByteArray &val)
{ d = create(ByteArray, &val); }
inline QCoreVariant::QCoreVariant(const QBitArray &val)
{ d = create(BitArray, &val); }
inline QCoreVariant::QCoreVariant(const QString &val)
{ d = create(String, &val); }
#ifndef QT_NO_STRINGLIST
inline QCoreVariant::QCoreVariant(const QStringList &val)
{ d = create(StringList, &val); }
#endif

inline QCoreVariant::QCoreVariant(const QDate &val)
{ d = create(Date, &val); }
inline QCoreVariant::QCoreVariant(const QTime &val)
{ d = create(Time, &val); }
inline QCoreVariant::QCoreVariant(const QDateTime &val)
{ d = create(DateTime, &val); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QCoreVariant::QCoreVariant(const QList<QCoreVariant> &val)
{ d = create(List, &val); }
inline QCoreVariant::QCoreVariant(const QMap<QString,QCoreVariant> &val)
{ d = create(Map, &val); }
#endif

inline QCoreVariant::Type QCoreVariant::type() const
{ return (Type)d->type; }
inline bool QCoreVariant::isValid() const
{ return d->type != Invalid; }

#ifdef QT_COMPAT
inline int &QCoreVariant::asInt()
{ return *static_cast<int *>(castOrDetach(Int)); }
inline uint &QCoreVariant::asUInt()
{ return *static_cast<uint *>(castOrDetach(UInt)); }
inline Q_LLONG &QCoreVariant::asLongLong()
{ return *static_cast<Q_LLONG *>(castOrDetach(LongLong)); }
inline Q_ULLONG &QCoreVariant::asULongLong()
{ return *static_cast<Q_ULLONG *>(castOrDetach(ULongLong)); }
inline bool &QCoreVariant::asBool()
{ return *static_cast<bool *>(castOrDetach(Bool)); }
inline double &QCoreVariant::asDouble()
{ return *static_cast<double *>(castOrDetach(Double)); }
inline QByteArray& QCoreVariant::asByteArray()
{ return *static_cast<QByteArray *>(castOrDetach(ByteArray)); }
inline QBitArray& QCoreVariant::asBitArray()
{ return *static_cast<QBitArray *>(castOrDetach(BitArray)); }
inline QString& QCoreVariant::asString()
{ return *static_cast<QString *>(castOrDetach(String)); }
#ifndef QT_NO_STRINGLIST
inline QStringList& QCoreVariant::asStringList()
{ return *static_cast<QStringList *>(castOrDetach(StringList)); }
#endif
inline QDate& QCoreVariant::asDate()
{ return *static_cast<QDate *>(castOrDetach(Date)); }
inline QTime& QCoreVariant::asTime()
{ return *static_cast<QTime *>(castOrDetach(Time)); }
inline QDateTime& QCoreVariant::asDateTime()
{ return *static_cast<QDateTime *>(castOrDetach(DateTime)); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QList<QCoreVariant>& QCoreVariant::asList()
{ return *static_cast<QList<QCoreVariant> *>(castOrDetach(List)); }
inline QMap<QString, QCoreVariant>& QCoreVariant::asMap()
{ return *static_cast<QMap<QString, QCoreVariant> *>(castOrDetach(Map)); }
#endif
#endif //QT_COMPAT

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream& operator>> ( QDataStream& s, QCoreVariant& p );
Q_CORE_EXPORT QDataStream& operator<< ( QDataStream& s, const QCoreVariant& p );
Q_CORE_EXPORT QDataStream& operator>> ( QDataStream& s, QCoreVariant::Type& p );
Q_CORE_EXPORT QDataStream& operator<< ( QDataStream& s, const QCoreVariant::Type p );
#endif

inline bool QCoreVariant::isDetached() const
{ return d->ref == 1; }

template<typename T> T qt_cast(const QCoreVariant &v);
template<> inline int qt_cast<int>(const QCoreVariant &v) { return v.toInt(); }
template<> inline uint qt_cast<uint>(const QCoreVariant &v) { return v.toUInt(); }
template<> inline Q_LLONG qt_cast<Q_LLONG>(const QCoreVariant &v) { return v.toLongLong(); }
template<> inline Q_ULLONG qt_cast<Q_ULLONG>(const QCoreVariant &v) { return v.toULongLong(); }
template<> inline bool qt_cast<bool>(const QCoreVariant &v) { return v.toBool(); }
template<> inline double qt_cast<double>(const QCoreVariant &v) { return v.toDouble(); }
template<> inline QByteArray qt_cast<QByteArray>(const QCoreVariant &v) { return v.toByteArray(); }

template<> QString qt_cast<QString>(const QCoreVariant &v);
template<> QBitArray qt_cast<QBitArray>(const QCoreVariant &v);
#ifndef QT_NO_STRINGLIST
template<> QStringList qt_cast<QStringList>(const QCoreVariant &v);
#endif
template<> QDate qt_cast<QDate>(const QCoreVariant &v);
template<> QTime qt_cast<QTime>(const QCoreVariant &v);
template<> QDateTime qt_cast<QDateTime>(const QCoreVariant &v);
#ifndef QT_NO_TEMPLATE_VARIANT
template<> QList<QCoreVariant> qt_cast<QList<QCoreVariant> >(const QCoreVariant &v);
template<> QMap<QString,QCoreVariant> qt_cast<QMap<QString,QCoreVariant> >(const QCoreVariant &v);
#endif

Q_DECLARE_TYPEINFO(QCoreVariant, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QCoreVariant);

#endif //QT_NO_VARIANT
#endif // QCOREVARIANT_H
