/****************************************************************************
**
** Definition of QKernelVariant class.
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

#ifndef QKERNELVARIANT_H
#define QKERNELVARIANT_H

#ifndef QT_H
#include "qatomic.h"
#include "qbytearray.h"
#endif // QT_H

#ifndef QT_NO_VARIANT
class QBitArray;
class QDataStream;
class QDate;
class QDateTime;
class QKernelVariant;
class QString;
class QStringList;
class QStringList;
class QTime;

template <class Type> class QList;
template <class Key, class Type> class QMap;

class Q_KERNEL_EXPORT QKernelVariant
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
#ifndef QT_NO_COMPAT
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

    inline QKernelVariant();
    inline ~QKernelVariant();
    inline QKernelVariant(Type type, void *v = 0);
    inline QKernelVariant(const QKernelVariant &other);

#ifndef QT_NO_DATASTREAM
    inline QKernelVariant(QDataStream &s);
#endif

    inline QKernelVariant(int i);
    inline QKernelVariant(uint ui);
    inline QKernelVariant(Q_LLONG ll);
    inline QKernelVariant(Q_ULLONG ull);
    inline QKernelVariant(bool b);
    inline QKernelVariant(double d);

    inline QKernelVariant(const char *str);
    inline QKernelVariant(const QByteArray &bytearray);
    inline QKernelVariant(const QBitArray &bitarray);
    inline QKernelVariant(const QString &string);
#ifndef QT_NO_STRINGLIST
    inline QKernelVariant(const QStringList &stringlist);
#endif

    inline QKernelVariant(const QDate &date);
    inline QKernelVariant(const QTime &time);
    inline QKernelVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QKernelVariant(const QList<QKernelVariant> &list);
    inline QKernelVariant(const QMap<QString,QKernelVariant> &map);
#endif

    QKernelVariant& operator=(const QKernelVariant &other);
    bool operator==(const QKernelVariant &other) const;
    inline bool operator!=(const QKernelVariant &other) const
    { return !(other == *this); }

    inline Type type() const;
    const char *typeName() const;

    bool canCast(Type t) const;
    bool cast(Type t);

    inline bool isValid() const;
    bool isNull() const;

    void clear();

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
    QList<QKernelVariant> toList() const;
    QMap<QString,QKernelVariant> toMap() const;
#endif

    inline int &asInt();
    inline uint &asUInt();
    inline Q_LLONG &asLongLong();
    inline Q_ULLONG &asULongLong();
    inline bool &asBool();
    inline double &asDouble();
    inline QByteArray &asByteArray();
    inline QBitArray &asBitArray();
    inline QString &asString();
#ifndef QT_NO_STRINGLIST
    inline QStringList &asStringList();
#endif
    inline QDate &asDate();
    inline QTime &asTime();
    inline QDateTime &asDateTime();
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QList<QKernelVariant> &asList();
    inline QMap<QString,QKernelVariant> &asMap();
#endif

#ifndef QT_NO_DATASTREAM
    void load(QDataStream &ds);
    void save(QDataStream &ds) const;
#endif
    static const char *typeToName(Type type);
    static Type nameToType(const char *name);

#ifndef QT_NO_COMPAT
    inline const QByteArray toCString() const { return toByteArray(); }
    inline QByteArray &asCString() { return asByteArray(); }
    inline QKernelVariant(bool b, int) { d = create(Bool, &b); }
#endif

    void *rawAccess(void *ptr = 0, Type typ = Invalid, bool deepCopy = FALSE);
    void *data();
 private:
    inline void detach() { if (d->ref != 1) detach_helper(); }
    void detach_helper();

 public:
    struct Private
    {
	QAtomic ref;
	Type type;
	bool is_null;
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
    typedef void (*f_cast)(QKernelVariant::Private *d, Type t, void *, bool *);
    typedef bool (*f_canCast)(QKernelVariant::Private *d, Type t);
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


inline QKernelVariant::QKernelVariant() :d(&shared_invalid)
{ ++d->ref; }
inline QKernelVariant::~QKernelVariant()
{ if (!--d->ref) cleanUp(d); }
inline QKernelVariant::QKernelVariant(Type type, void *v)
{ d = create(type, v); }
inline QKernelVariant::QKernelVariant(const QKernelVariant &p) : d(p.d)
{ ++d->ref; }

inline QKernelVariant::QKernelVariant(int val)
{ d = create(Int, &val); }
inline QKernelVariant::QKernelVariant(uint val)
{ d = create(UInt, &val); }
inline QKernelVariant::QKernelVariant(Q_LLONG val)
{ d = create(LongLong, &val); }
inline QKernelVariant::QKernelVariant(Q_ULLONG val)
{ d = create(ULongLong, &val); }
inline QKernelVariant::QKernelVariant(bool val)
{ d = create(Bool, &val); }
inline QKernelVariant::QKernelVariant(double val)
{ d = create(Double, &val); }

inline QKernelVariant::QKernelVariant(const char *val)
{ QByteArray ba(val); d = create(ByteArray, &ba); }
inline QKernelVariant::QKernelVariant(const QByteArray &val)
{ d = create(ByteArray, &val); }
inline QKernelVariant::QKernelVariant(const QBitArray &val)
{ d = create(BitArray, &val); }
inline QKernelVariant::QKernelVariant(const QString &val)
{ d = create(String, &val); }
#ifndef QT_NO_STRINGLIST
inline QKernelVariant::QKernelVariant(const QStringList &val)
{ d = create(StringList, &val); }
#endif

inline QKernelVariant::QKernelVariant(const QDate &val)
{ d = create(Date, &val); }
inline QKernelVariant::QKernelVariant(const QTime &val)
{ d = create(Time, &val); }
inline QKernelVariant::QKernelVariant(const QDateTime &val)
{ d = create(DateTime, &val); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QKernelVariant::QKernelVariant(const QList<QKernelVariant> &val)
{ d = create(List, &val); }
inline QKernelVariant::QKernelVariant(const QMap<QString,QKernelVariant> &val)
{ d = create(Map, &val); }
#endif

inline QKernelVariant::Type QKernelVariant::type() const
{ return d->type; }
inline bool QKernelVariant::isValid() const
{ return d->type != Invalid; }

inline int &QKernelVariant::asInt()
{ return *static_cast<int *>(castOrDetach(Int)); }
inline uint &QKernelVariant::asUInt()
{ return *static_cast<uint *>(castOrDetach(UInt)); }
inline Q_LLONG &QKernelVariant::asLongLong()
{ return *static_cast<Q_LLONG *>(castOrDetach(LongLong)); }
inline Q_ULLONG &QKernelVariant::asULongLong()
{ return *static_cast<Q_ULLONG *>(castOrDetach(ULongLong)); }
inline bool &QKernelVariant::asBool()
{ return *static_cast<bool *>(castOrDetach(Bool)); }
inline double &QKernelVariant::asDouble()
{ return *static_cast<double *>(castOrDetach(Double)); }
inline QByteArray& QKernelVariant::asByteArray()
{ return *static_cast<QByteArray *>(castOrDetach(ByteArray)); }
inline QBitArray& QKernelVariant::asBitArray()
{ return *static_cast<QBitArray *>(castOrDetach(BitArray)); }
inline QString& QKernelVariant::asString()
{ return *static_cast<QString *>(castOrDetach(String)); }
#ifndef QT_NO_STRINGLIST
inline QStringList& QKernelVariant::asStringList()
{ return *static_cast<QStringList *>(castOrDetach(StringList)); }
#endif
inline QDate& QKernelVariant::asDate()
{ return *static_cast<QDate *>(castOrDetach(Date)); }
inline QTime& QKernelVariant::asTime()
{ return *static_cast<QTime *>(castOrDetach(Time)); }
inline QDateTime& QKernelVariant::asDateTime()
{ return *static_cast<QDateTime *>(castOrDetach(DateTime)); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QList<QKernelVariant>& QKernelVariant::asList()
{ return *static_cast<QList<QKernelVariant> *>(castOrDetach(List)); }
inline QMap<QString, QKernelVariant>& QKernelVariant::asMap()
{ return *static_cast<QMap<QString, QKernelVariant> *>(castOrDetach(Map)); }
#endif

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QKernelVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QKernelVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QKernelVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QKernelVariant::Type p );
#endif

#endif //QT_NO_VARIANT
#endif // QKERNELVARIANT_H
