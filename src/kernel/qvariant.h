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

    inline QVariant();
    inline ~QVariant();
    inline QVariant(Type type, void *v = 0);
    inline QVariant(const QVariant &other);

#ifndef QT_NO_DATASTREAM
    QVariant(QDataStream &s);
#endif

    inline QVariant(int i);
    inline QVariant(uint ui);
    inline QVariant(Q_LLONG ll);
    inline QVariant(Q_ULLONG ull);
    inline QVariant(bool b);
    inline QVariant(double d);

    inline QVariant(const char *str);
    inline QVariant(const QByteArray &bytearray);
    inline QVariant(const QBitArray &bitarray);
    inline QVariant(const QString &string);
#ifndef QT_NO_STRINGLIST
    inline QVariant(const QStringList &stringlist);
#endif

    inline QVariant(const QDate &date);
    inline QVariant(const QTime &time);
    inline QVariant(const QDateTime &datetime);
#ifndef QT_NO_TEMPLATE_VARIANT
    inline QVariant(const QList<QVariant> &list);
    inline QVariant(const QMap<QString,QVariant> &map);
#endif

#ifdef QT_GUI_LIB
    inline QVariant(const QFont &font);
    inline QVariant(const QPixmap &pixmap);
    inline QVariant(const QImage &image);
    inline QVariant(const QBrush &brush);
    inline QVariant(const QPoint &pt);
    inline QVariant(const QRect &rect);
    inline QVariant(const QSize &size);
    inline QVariant(const QColor &color);
#ifndef QT_NO_PALETTE
    inline QVariant(const QPalette &palette);
#ifndef QT_NO_COMPAT
    inline QVariant(const QColorGroup &cg);
#endif
#endif
#ifndef QT_NO_ICONSET
    inline QVariant(const QIconSet &iconset);
#endif
    inline QVariant(const QPointArray &pointarray);
    inline QVariant(const QRegion &region);
    inline QVariant(const QBitmap &bitmap);
    inline QVariant(const QCursor &cursor);
#ifndef QT_NO_ACCEL
    inline QVariant(const QKeySequence &keysequence);
#endif
    inline QVariant(const QPen &pen);
    inline QVariant(const QSizePolicy &sp);
#endif // QT_GUI_LIB

    QVariant& operator=(const QVariant &other);
    bool operator==(const QVariant &other) const;
    inline bool operator!=(const QVariant &other) const
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
    QList<QVariant> toList() const;
    QMap<QString,QVariant> toMap() const;
#endif

#ifdef QT_GUI_LIB
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
#endif // QT_GUI_LIB

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
    inline QList<QVariant> &asList();
    inline QMap<QString,QVariant> &asMap();
#endif

#ifdef QT_GUI_LIB
    inline QFont &asFont();
    inline QPixmap &asPixmap();
    inline QImage &asImage();
    inline QBrush &asBrush();
    inline QPoint &asPoint();
    inline QRect &asRect();
    inline QSize &asSize();
    inline QColor &asColor();
#ifndef QT_NO_PALETTE
    inline QPalette &asPalette();
#ifndef QT_NO_COMPAT
    inline QColorGroup &asColorGroup();
#endif
#endif
    inline QIconSet &asIconSet();
    inline QPointArray &asPointArray();
    inline QBitmap &asBitmap();
    inline QRegion &asRegion();
    inline QCursor &asCursor();
#ifndef QT_NO_ACCEL
    inline QKeySequence &asKeySequence();
#endif
    inline QPen &asPen();
    inline QSizePolicy &asSizePolicy();
#endif //QT_GUI_LIB

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

 private:
    Private *d;
    static Private shared_invalid;

    static const Handler *handler;
    friend class QApplicationPrivate;

    Private *create(Type t, const void *v);
    inline void cleanUp(Private *p)
    { handler->clear(p); delete p; }
    void *castOrDetach(Type t);
};


inline QVariant::QVariant() :d(&shared_invalid)
{ ++d->ref; }
inline QVariant::~QVariant()
{ if (!--d->ref) cleanUp(d); }
inline QVariant::QVariant(Type type, void *v)
{ d = create(type, v); }
inline QVariant::QVariant(const QVariant &p) : d(p.d)
{ ++d->ref; }

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

inline QVariant::QVariant(const char *val)
{ QByteArray ba(val); d = create(ByteArray, &ba); }
inline QVariant::QVariant(const QByteArray &val)
{ d = create(ByteArray, &val); }
inline QVariant::QVariant(const QBitArray &val)
{ d = create(BitArray, &val); }
inline QVariant::QVariant(const QString &val)
{ d = create(String, &val); }
#ifndef QT_NO_STRINGLIST
inline QVariant::QVariant(const QStringList &val)
{ d = create(StringList, &val); }
#endif

inline QVariant::QVariant(const QDate &val)
{ d = create(Date, &val); }
inline QVariant::QVariant(const QTime &val)
{ d = create(Time, &val); }
inline QVariant::QVariant(const QDateTime &val)
{ d = create(DateTime, &val); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QVariant::QVariant(const QList<QVariant> &val)
{ d = create(List, &val); }
inline QVariant::QVariant(const QMap<QString,QVariant> &val)
{ d = create(Map, &val); }
#endif

inline QVariant::Type QVariant::type() const
{ return d->type; }
inline bool QVariant::isValid() const
{ return d->type != Invalid; }

inline int &QVariant::asInt()
{ return *static_cast<int *>(castOrDetach(Int)); }
inline uint &QVariant::asUInt()
{ return *static_cast<uint *>(castOrDetach(UInt)); }
inline Q_LLONG &QVariant::asLongLong()
{ return *static_cast<Q_LLONG *>(castOrDetach(LongLong)); }
inline Q_ULLONG &QVariant::asULongLong()
{ return *static_cast<Q_ULLONG *>(castOrDetach(ULongLong)); }
inline bool &QVariant::asBool()
{ return *static_cast<bool *>(castOrDetach(Bool)); }
inline double &QVariant::asDouble()
{ return *static_cast<double *>(castOrDetach(Double)); }
inline QByteArray& QVariant::asByteArray()
{ return *static_cast<QByteArray *>(castOrDetach(ByteArray)); }
inline QBitArray& QVariant::asBitArray()
{ return *static_cast<QBitArray *>(castOrDetach(BitArray)); }
inline QString& QVariant::asString()
{ return *static_cast<QString *>(castOrDetach(String)); }
#ifndef QT_NO_STRINGLIST
inline QStringList& QVariant::asStringList()
{ return *static_cast<QStringList *>(castOrDetach(StringList)); }
#endif
inline QDate& QVariant::asDate()
{ return *static_cast<QDate *>(castOrDetach(Date)); }
inline QTime& QVariant::asTime()
{ return *static_cast<QTime *>(castOrDetach(Time)); }
inline QDateTime& QVariant::asDateTime()
{ return *static_cast<QDateTime *>(castOrDetach(DateTime)); }
#ifndef QT_NO_TEMPLATE_VARIANT
inline QList<QVariant>& QVariant::asList()
{ return *static_cast<QList<QVariant> *>(castOrDetach(List)); }
inline QMap<QString, QVariant>& QVariant::asMap()
{ return *static_cast<QMap<QString, QVariant> *>(castOrDetach(Map)); }
#endif


#ifdef QT_GUI_LIB

inline QVariant::QVariant(const QFont &val)
{ d = create(Font, &val); }
inline QVariant::QVariant(const QPixmap &val)
{ d = create(Pixmap, &val); }
inline QVariant::QVariant(const QImage &val)
{ d = create(Image, &val); }
inline QVariant::QVariant(const QBrush &val)
{ d = create(Brush, &val); }
inline QVariant::QVariant(const QPoint &val)
{ d = create(Point, &val); }
inline QVariant::QVariant(const QRect &val)
{ d = create(Rect, &val); }
inline QVariant::QVariant(const QSize &val)
{ d = create(Size, &val); }
inline QVariant::QVariant(const QColor &val)
{ d = create(Color, &val); }
#ifndef QT_NO_PALETTE
inline QVariant::QVariant(const QPalette &val)
{ d = create(Palette, &val); }
#ifndef QT_NO_COMPAT
inline QVariant::QVariant(const QColorGroup &val)
{ d = create(ColorGroup, &val); }
#endif
#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
inline QVariant::QVariant(const QIconSet &val)
{ d = create(IconSet, &val); }
#endif //QT_NO_ICONSET
inline QVariant::QVariant(const QPointArray &val)
{ d = create(PointArray, &val); }
inline QVariant::QVariant(const QRegion &val)
{ d = create(Region, &val); }
inline QVariant::QVariant(const QBitmap& val)
{ d = create(Bitmap, &val); }
inline QVariant::QVariant(const QCursor &val)
{ d = create(Cursor, &val); }
#ifndef QT_NO_ACCEL
inline QVariant::QVariant(const QKeySequence &val)
{ d = create(KeySequence, &val); }
#endif
inline QVariant::QVariant(const QPen &val)
{ d = create(Pen, &val); }
inline QVariant::QVariant(const QSizePolicy &val)
{ d = create(SizePolicy, &val); }

inline QFont& QVariant::asFont()
{ return *static_cast<QFont *>(castOrDetach(Font)); }
inline QPixmap& QVariant::asPixmap()
{ return *static_cast<QPixmap *>(castOrDetach(Pixmap)); }
inline QImage& QVariant::asImage()
{ return *static_cast<QImage *>(castOrDetach(Image)); }
inline QBrush& QVariant::asBrush()
{ return *static_cast<QBrush *>(castOrDetach(Brush)); }
inline QPoint& QVariant::asPoint()
{ return *static_cast<QPoint *>(castOrDetach(Point)); }
inline QRect& QVariant::asRect()
{ return *static_cast<QRect *>(castOrDetach(Rect)); }
inline QSize& QVariant::asSize()
{ return *static_cast<QSize *>(castOrDetach(Size)); }
inline QColor& QVariant::asColor()
{ return *static_cast<QColor *>(castOrDetach(Color)); }
#ifndef QT_NO_PALETTE
inline QPalette& QVariant::asPalette()
{ return *static_cast<QPalette *>(castOrDetach(Palette)); }
#ifndef QT_NO_COMPAT
inline QColorGroup& QVariant::asColorGroup()
{ return *static_cast<QColorGroup *>(castOrDetach(ColorGroup)); }
#endif
#endif
#ifndef QT_NO_ICONSET
inline QIconSet& QVariant::asIconSet()
{ return *static_cast<QIconSet *>(castOrDetach(IconSet)); }
#endif
inline QPointArray& QVariant::asPointArray()
{ return *static_cast<QPointArray *>(castOrDetach(PointArray)); }
inline QBitmap& QVariant::asBitmap()
{ return *static_cast<QBitmap *>(castOrDetach(Bitmap)); }
inline QRegion& QVariant::asRegion()
{ return *static_cast<QRegion *>(castOrDetach(Region)); }
inline QCursor& QVariant::asCursor()
{ return *static_cast<QCursor *>(castOrDetach(Cursor)); }
#ifndef QT_NO_ACCEL
inline QKeySequence& QVariant::asKeySequence()
{ return *static_cast<QKeySequence *>(castOrDetach(KeySequence)); }
#endif
inline QPen& QVariant::asPen()
{ return *static_cast<QPen *>(castOrDetach(Pen)); }
inline QSizePolicy& QVariant::asSizePolicy()
{ return *static_cast<QSizePolicy *>(castOrDetach(SizePolicy)); }

#endif // QT_GUI_LIB


#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );
#endif

#endif //QT_NO_VARIANT
#endif // QVARIANT_H
