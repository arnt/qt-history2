/****************************************************************************
**
** Definition of QVariant class.
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

#ifndef QVARIANT_H
#define QVARIANT_H

#ifndef QT_H
#include "qatomic.h"
#include "qbytearray.h"
#endif // QT_H

#ifndef QT_NO_VARIANT
class QString;
class QFont;
class QPixmap;
class QBrush;
class QRect;
class QPoint;
class QImage;
class QSize;
class QColor;
class QPalette;
#ifndef QT_NO_COMPAT
class QColorGroup;
#endif
class QIconSet;
class QDataStream;
class QPointArray;
class QRegion;
class QBitmap;
class QCursor;
class QStringList;
class QSizePolicy;
class QDate;
class QTime;
class QDateTime;
class QBitArray;
class QKeySequence;
class QPen;
class QVariant;
class QStringList;
template <class Type> class QList;
template <class Key, class Type> class QMap;
class QApplicationPrivate;

class Q_EXPORT QVariant
{
public:
    enum Type {
	Invalid,
	Map,
	List,
	String,
	StringList,
	Font,
	Pixmap,
	Brush,
	Rect,
	Size,
	Color,
	Palette,
#ifndef QT_NO_COMPAT
	ColorGroup,
#endif
	IconSet,
	Point,
	Image,
	Int,
	UInt,
	Bool,
	Double,
	PointArray,
	Region,
	Bitmap,
	Cursor,
	SizePolicy,
	Date,
	Time,
	DateTime,
	ByteArray,
#ifndef Q_QDOC
	CString = ByteArray,
#endif
	BitArray,
	KeySequence,
	Pen,
	LongLong,
	ULongLong
    };

    QVariant();
    ~QVariant();
    QVariant(Type type, void *v = 0);
    QVariant(const QVariant &other);

#ifndef QT_NO_DATASTREAM
    QVariant(QDataStream &s);
#endif

    QVariant(int i);
    QVariant(uint ui);
    QVariant(Q_LLONG ll);
    QVariant(Q_ULLONG ull);
    QVariant(bool b);
    QVariant(double d);
    QVariant(const char *str);
    QVariant(const QByteArray &bytearray);
    QVariant(const QBitArray &bitarray);
    QVariant(const QString &string);
#ifndef QT_NO_STRINGLIST
    QVariant(const QStringList &stringlist);
#endif
    QVariant(const QDate &date);
    QVariant(const QTime &time);
    QVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    QVariant(const QList<QVariant> &list);
    QVariant(const QMap<QString,QVariant> &map);
#endif

#ifndef QT_NO_COMPAT
    QVariant(const QColorGroup &cg);
#endif
    QVariant(const QFont &font);
    QVariant(const QPixmap &pixmap);
    QVariant(const QImage &image);
    QVariant(const QBrush &brush);
    QVariant(const QPoint &pt);
    QVariant(const QRect &rect);
    QVariant(const QSize &size);
    QVariant(const QColor &color);
    QVariant(const QPalette &palette);
    QVariant(const QIconSet &iconset);
    QVariant(const QPointArray &pointarray);
    QVariant(const QRegion &region);
    QVariant(const QBitmap &bitmap);
    QVariant(const QCursor &cursor);
#ifndef QT_NO_ACCEL
    QVariant(const QKeySequence &keysequence);
#endif
    QVariant(const QPen &pen);
    QVariant(QSizePolicy sp);

    QVariant& operator=(const QVariant &other);
    bool operator==(const QVariant &other) const;
    inline bool operator!=(const QVariant &other) const { return !(other == *this); }

    Type type() const;
    const char *typeName() const;

    bool canCast(Type t) const;
    bool cast(Type t);

    bool isValid() const;
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
    QList<QVariant> toList() const;
    QMap<QString,QVariant> toMap() const;
#endif

    QFont toFont() const;
    QPixmap toPixmap() const;
    const QImage toImage() const;
    QBrush toBrush() const;
    QPoint toPoint() const;
    QRect toRect() const;
    QSize toSize() const;
    QColor toColor() const;
    QPalette toPalette() const;
#ifndef QT_NO_COMPAT
    QColorGroup toColorGroup() const;
#endif
    QIconSet toIconSet() const;
    const QPointArray toPointArray() const;
    QBitmap toBitmap() const;
    QRegion toRegion() const;
    QCursor toCursor() const;
#ifndef QT_NO_ACCEL
    QKeySequence toKeySequence() const;
#endif
    QPen toPen() const;
    QSizePolicy toSizePolicy() const;

    int &asInt();
    uint &asUInt();
    Q_LLONG &asLongLong();
    Q_ULLONG &asULongLong();
    bool &asBool();
    double &asDouble();
    QByteArray &asByteArray();
    QBitArray &asBitArray();
    QString &asString();
#ifndef QT_NO_STRINGLIST
    QStringList &asStringList();
#endif
    QDate &asDate();
    QTime &asTime();
    QDateTime &asDateTime();
#ifndef QT_NO_TEMPLATE_VARIANT
    QList<QVariant> &asList();
    QMap<QString,QVariant> &asMap();
#endif

    QFont &asFont();
    QPixmap &asPixmap();
    QImage &asImage();
    QBrush &asBrush();
    QPoint &asPoint();
    QRect &asRect();
    QSize &asSize();
    QColor &asColor();
    QPalette &asPalette();
#ifndef QT_NO_COMPAT
    QColorGroup &asColorGroup();
#endif
    QIconSet &asIconSet();
    QPointArray &asPointArray();
    QBitmap &asBitmap();
    QRegion &asRegion();
    QCursor &asCursor();
#ifndef QT_NO_ACCEL
    QKeySequence &asKeySequence();
#endif
    QPen &asPen();
    QSizePolicy &asSizePolicy();

#ifndef QT_NO_DATASTREAM
    void load(QDataStream &ds);
    void save(QDataStream &ds) const;
#endif
    static const char *typeToName(Type type);
    static Type nameToType(const char *name);

#ifndef QT_NO_COMPAT
    inline const QByteArray toCString() const { return toByteArray(); }
    inline QByteArray &asCString() { return asByteArray(); }
    inline QVariant(bool b, int) { d = create(Bool, &b); }
#endif

    void *rawAccess(void *ptr = 0, Type typ = Invalid, bool deepCopy = FALSE);
    void *data();
private:
    void detach() { if (d->ref != 1) detach_helper(); }
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
private:
    Private *d;
    static Private shared_invalid;

    typedef void (*f_construct)(Private *, const void *);
    typedef void (*f_clear)(Private *);
    typedef bool (*f_null)(const Private *);
    typedef void (*f_load)(Private *, QDataStream &);
    typedef void (*f_save)(const Private *, QDataStream &);
    typedef bool (*f_compare)(const Private *, const Private *);
    typedef void (*f_cast)(QVariant::Private *d, Type t, void *, bool *);
    typedef bool (*f_canCast)(QVariant::Private *d, Type t);

    struct Handler {
	f_construct construct;
	f_clear clear;
	f_null isNull;
	f_load load;
	f_save save;
	f_compare compare;
	f_cast cast;
	f_canCast canCast;
    };
    static const Handler *handler;
    friend class QApplicationPrivate;


    Private *create(Type t, const void *v);
    inline void cleanUp(Private *p) { handler->clear(p); delete p; }
};


inline QVariant::QVariant() :d(&shared_invalid)
{ ++d->ref; }
inline QVariant::QVariant(const QVariant &p) : d(p.d)
{ ++d->ref; }
inline QVariant::QVariant(Type type, void *v)
{ d = create(type, v); }
inline QVariant::~QVariant()
{ if (!--d->ref) cleanUp(d); }

inline QVariant::QVariant(const QString &val)
{ d = create(String, &val); }
inline QVariant::QVariant(const char *val)
{ QByteArray ba(val); d = create(ByteArray, &ba); }

#ifndef QT_NO_STRINGLIST
inline QVariant::QVariant(const QStringList &val)
{ d = create(StringList, &val); }
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
inline QVariant::QVariant(const QMap<QString,QVariant> &val)
{ d = create(Map, &val); }
inline QVariant::QVariant(const QList<QVariant> &val)
{ d = create(List, &val); }
#endif
inline QVariant::QVariant(const QDate &val)
{ d = create(Date, &val); }
inline QVariant::QVariant(const QTime &val)
{ d = create(Time, &val); }
inline QVariant::QVariant(const QDateTime &val)
{ d = create(DateTime, &val); }
inline QVariant::QVariant(const QByteArray &val)
{ d = create(ByteArray, &val); }
inline QVariant::QVariant(const QBitArray &val)
{ d = create(BitArray, &val); }
inline QVariant::QVariant(int val)
{ d = create(Int, &val); }
inline QVariant::QVariant(uint val)
{ d = create(UInt, &val); }
inline QVariant::QVariant(Q_LLONG val)
{ d = create(LongLong, &val); }
inline QVariant::QVariant(Q_ULLONG val)
{ d = create(ULongLong, &val); }
inline QVariant::QVariant(bool val)
{ d = create(Bool, &val); }
inline QVariant::QVariant(double val)
{ d = create(Double, &val); }

inline QVariant::Type QVariant::type() const
{ return d->type; }

inline bool QVariant::isValid() const
{ return d->type != Invalid; }


#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );
#endif

#endif //QT_NO_VARIANT
#endif // QVARIANT_H
