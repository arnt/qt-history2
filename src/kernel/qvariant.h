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
#include "qbytearray.h"
#include "qshared.h"
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
    QVariant( Type type, void *v = 0 );
    QVariant( const QVariant& );
#ifndef QT_NO_DATASTREAM
    QVariant( QDataStream& s );
#endif
    QVariant( const QString& );
    QVariant( const char* );
#ifndef QT_NO_STRINGLIST
    QVariant( const QStringList& );
#endif
#ifndef QT_NO_COMPAT
    QVariant( const QColorGroup& );
#endif
    QVariant( const QFont& );
    QVariant( const QPixmap& );
    QVariant( const QImage& );
    QVariant( const QBrush& );
    QVariant( const QPoint& );
    QVariant( const QRect& );
    QVariant( const QSize& );
    QVariant( const QColor& );
    QVariant( const QPalette& );
    QVariant( const QIconSet& );
    QVariant( const QPointArray& );
    QVariant( const QRegion& );
    QVariant( const QBitmap& );
    QVariant( const QCursor& );
    QVariant( const QDate& );
    QVariant( const QTime& );
    QVariant( const QDateTime& );
    QVariant( const QByteArray& );
    QVariant( const QBitArray& );
#ifndef QT_NO_ACCEL
    QVariant( const QKeySequence& );
#endif
    QVariant( const QPen& );
#ifndef QT_NO_TEMPLATE_VARIANT
    QVariant( const QList<QVariant>& );
    QVariant( const QMap<QString,QVariant>& );
#endif
    QVariant( int );
    QVariant( uint );
    QVariant( Q_LLONG );
    QVariant( Q_ULLONG );
    // ### Problems on some compilers ?
    QVariant( bool, int );
    QVariant( double );
    QVariant( QSizePolicy );

    QVariant& operator= ( const QVariant& );
    bool operator==( const QVariant& ) const;
    bool operator!=( const QVariant& ) const;

    Type type() const;
    const char* typeName() const;

    bool canCast( Type ) const;
    bool cast( Type );

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
    int toInt( bool * ok=0 ) const;
    uint toUInt( bool * ok=0 ) const;
    Q_LLONG toLongLong( bool * ok=0 ) const;
    Q_ULLONG toULongLong( bool * ok=0 ) const;
    bool toBool() const;
    double toDouble( bool * ok=0 ) const;
#ifndef QT_NO_TEMPLATE_VARIANT
    QList<QVariant> toList() const;
    QMap<QString,QVariant> toMap() const;
#endif
    QSizePolicy toSizePolicy() const;

    QString& asString();
#ifndef QT_NO_STRINGLIST
    QStringList& asStringList();
#endif
    QFont& asFont();
    QPixmap& asPixmap();
    QImage& asImage();
    QBrush& asBrush();
    QPoint& asPoint();
    QRect& asRect();
    QSize& asSize();
    QColor& asColor();
    QPalette& asPalette();
#ifndef QT_NO_COMPAT
    QColorGroup& asColorGroup();
#endif
    QIconSet& asIconSet();
    QPointArray& asPointArray();
    QBitmap& asBitmap();
    QRegion& asRegion();
    QCursor& asCursor();
    QDate& asDate();
    QTime& asTime();
    QDateTime& asDateTime();
    QByteArray& asByteArray();
    QBitArray& asBitArray();
#ifndef QT_NO_ACCEL
    QKeySequence& asKeySequence();
#endif
    QPen& asPen();
    int& asInt();
    uint& asUInt();
    Q_LLONG& asLongLong();
    Q_ULLONG& asULongLong();
    bool& asBool();
    double& asDouble();
#ifndef QT_NO_TEMPLATE_VARIANT
    QList<QVariant>& asList();
    QMap<QString,QVariant>& asMap();
#endif
    QSizePolicy& asSizePolicy();

#ifndef QT_NO_DATASTREAM
    void load( QDataStream& );
    void save( QDataStream& ) const;
#endif
    static const char* typeToName( Type type );
    static Type nameToType( const char* name );

#ifndef QT_NO_COMPAT
    const QByteArray toCString() const { return toByteArray(); }
    QByteArray& asCString() { return asByteArray(); }
#endif
private:
    void detach();

    class Private : public QShared
    {
    public:
	Private();
	Private( uint );
	Private( uint, void * );
	~Private();

	void clear();

	uint type : 30;
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
    };

    Private* d;

public:
    void* rawAccess( void* ptr = 0, Type typ = Invalid, bool deepCopy = FALSE );
    void* data();
};

inline QVariant::Type QVariant::type() const
{
    return (Type)d->type;
}

inline bool QVariant::isValid() const
{
    return (d->type != Invalid);
}

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );
#endif

#endif //QT_NO_VARIANT
#endif // QVARIANT_H
