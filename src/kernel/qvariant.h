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
    QVariant(const QString &string);
    QVariant(const char *str);
#ifndef QT_NO_STRINGLIST
    QVariant(const QStringList &stringlist);
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
    QVariant(const QDate &date);
    QVariant(const QTime &time);
    QVariant(const QDateTime &datetime);
    QVariant(const QByteArray &bytearray);
    QVariant(const QBitArray &bitarray);
#ifndef QT_NO_ACCEL
    QVariant(const QKeySequence &keysequence);
#endif
    QVariant(const QPen &pen);
#ifndef QT_NO_TEMPLATE_VARIANT
    QVariant(const QList<QVariant> &list);
    QVariant(const QMap<QString,QVariant> &map);
#endif
    QVariant(int i);
    QVariant(uint ui);
    QVariant(Q_LLONG ll);
    QVariant(Q_ULLONG ull);
    QVariant(bool b, int i);
    QVariant(double d);
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

    QString toString() const;
#ifndef QT_NO_STRINGLIST
    QStringList toStringList() const;
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
    QDate toDate() const;
    QTime toTime() const;
    QDateTime toDateTime() const;
    QByteArray toByteArray() const;
    QBitArray toBitArray() const;
#ifndef QT_NO_ACCEL
    QKeySequence toKeySequence() const;
#endif
    QPen toPen() const;
    int toInt(bool *ok = 0) const;
    uint toUInt(bool *ok = 0) const;
    Q_LLONG toLongLong(bool *ok = 0) const;
    Q_ULLONG toULongLong(bool *ok = 0) const;
    bool toBool() const;
    double toDouble(bool *ok = 0) const;
#ifndef QT_NO_TEMPLATE_VARIANT
    QList<QVariant> toList() const;
    QMap<QString,QVariant> toMap() const;
#endif
    QSizePolicy toSizePolicy() const;

    QString &asString();
#ifndef QT_NO_STRINGLIST
    QStringList &asStringList();
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
    QDate &asDate();
    QTime &asTime();
    QDateTime &asDateTime();
    QByteArray &asByteArray();
    QBitArray &asBitArray();
#ifndef QT_NO_ACCEL
    QKeySequence &asKeySequence();
#endif
    QPen &asPen();
    int &asInt();
    uint &asUInt();
    Q_LLONG &asLongLong();
    Q_ULLONG &asULongLong();
    bool &asBool();
    double &asDouble();
#ifndef QT_NO_TEMPLATE_VARIANT
    QList<QVariant> &asList();
    QMap<QString,QVariant> &asMap();
#endif
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
#endif
    void *rawAccess(void *ptr = 0, Type typ = Invalid, bool deepCopy = FALSE);
    void *data();
private:
    void detach() { if (d->ref != 1) detach_helper(); }
    void detach_helper();

    struct Private
    {
	void clear();

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
    Private *d;
    static Private shared_invalid;
    Private *constructPrivate(Type type);
    Private *constructPrivate(Type type, void *v);
    Private *startConstruction();
    inline void cleanUp(Private *p) { p->clear(); delete p; }
};

inline QVariant::Type QVariant::type() const
{
    return d->type;
}

inline bool QVariant::isValid() const
{
    return d->type != Invalid;
}

inline QVariant::Private *QVariant::startConstruction()
{
    Private *p = new Private;
    p->ref = 1;
    p->is_null = true;
    return p;
}

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );
#endif

#endif //QT_NO_VARIANT
#endif // QVARIANT_H
