/****************************************************************************
** $Id: $
**
** Definition of QVariant class
**
** Created : 990414
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QVARIANT_H
#define QVARIANT_H

#ifndef QT_H
#include "qstring.h"
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
class QColorGroup;
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
template <class Type> class QValueList;
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
	ColorGroup,
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
    QVariant( const QFont& );
    QVariant( const QPixmap& );
    QVariant( const QImage& );
    QVariant( const QBrush& );
    QVariant( const QPoint& );
    QVariant( const QRect& );
    QVariant( const QSize& );
    QVariant( const QColor& );
    QVariant( const QPalette& );
    QVariant( const QColorGroup& );
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
    QVariant( const QValueList<QVariant>& );
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
    QColorGroup toColorGroup() const;
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
    QValueList<QVariant> toList() const;
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
    QColorGroup& asColorGroup();
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
    QValueList<QVariant>& asList();
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
	Private( Type );
	Private( Type, void * );
	~Private();

	void clear();

	Type type : 30;
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
    return d->type;
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
