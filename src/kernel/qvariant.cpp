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

#include <float.h>

#include "qvariant.h"
#ifndef QT_NO_VARIANT
#include "qstring.h"
#include "qbytearray.h"
#include "qfont.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qbrush.h"
#include "qpoint.h"
#include "qrect.h"
#include "qsize.h"
#include "qcolor.h"
#include "qpalette.h"
#include "qiconset.h"
#include "qdatastream.h"
#include "qregion.h"
#include "qpointarray.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qsizepolicy.h"
#include "qshared.h"
#include "qbitarray.h"
#include "qkeysequence.h"
#include "qpen.h"
#include "qmap.h"
#include "qlist.h"

#ifndef DBL_DIG
#define DBL_DIG 10
#endif //DBL_DIG


QVariant::Private::Private()
{
    type = QVariant::Invalid;
    is_null = true;
}

QVariant::Private::Private( uint t )
{
    type = t;
    is_null = true;
    switch( type ) {
    case Invalid:
	break;
    case Bitmap:
	value.ptr = new QBitmap;
	break;
    case Region:
	value.ptr = new QRegion;
	break;
    case PointArray:
	value.ptr = new QPointArray;
	break;
    case String:
	value.ptr = new QString;
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	value.ptr = new QStringList;
	break;
#endif //QT_NO_STRINGLIST
    case Font:
	value.ptr = new QFont;
	break;
    case Pixmap:
	value.ptr = new QPixmap;
	break;
    case Image:
	// QImage is explicit shared
	value.ptr = new QImage;
	break;
    case Brush:
	value.ptr = new QBrush;
	// ## Force a detach
	// ((QBrush*)value.ptr)->setColor( ((QBrush*)value.ptr)->color() );
	break;
    case Point:
	value.ptr = new QPoint;
	break;
    case Rect:
	value.ptr = new QRect;
	break;
    case Size:
	value.ptr = new QSize;
	break;
    case Color:
	value.ptr = new QColor;
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	value.ptr = new QPalette;
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup:
	value.ptr = new QColorGroup;
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	value.ptr = new QIconSet;
	break;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
	value.ptr = new QMap<QString,QVariant>;
	break;
    case List:
	value.ptr = new QList<QVariant>;
	break;
#endif
    case Date:
	value.ptr = new QDate;
	break;
    case Time:
	value.ptr = new QTime;
	break;
    case DateTime:
	value.ptr = new QDateTime;
	break;
    case ByteArray:
	value.ptr = new QByteArray;
	break;
    case BitArray:
	value.ptr = new QBitArray;
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	value.ptr = new QKeySequence;
	break;
#endif
    case Pen:
	value.ptr = new QPen;
	break;
    case Int:
	value.i = 0;
	break;
    case UInt:
	value.u = 0;
	break;
    case Bool:
	value.b = 0;
	break;
    case Double:
	value.d = 0;
	break;
    case SizePolicy:
	value.ptr = new QSizePolicy;
	break;
    case Cursor:
	value.ptr = new QCursor;
	break;
    default:
	Q_ASSERT( 0 );
    }
}

QVariant::Private::Private( uint t, void *v )
{
    type = t;
    is_null = true;
    switch( type ) {
    case Invalid:
	break;
    case Bitmap:
	value.ptr = new QBitmap(*(QBitmap*)v);
	break;
    case Region:
	value.ptr = new QRegion(*(QRegion*)v);
	break;
    case PointArray:
	value.ptr = new QPointArray(*(QPointArray*)v);
	break;
    case String:
	value.ptr = new QString(*(QString*)v);
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	value.ptr = new QStringList(*(QStringList*)v);
	break;
#endif //QT_NO_STRINGLIST
    case Font:
	value.ptr = new QFont(*(QFont*)v);
	break;
    case Pixmap:
	value.ptr = new QPixmap(*(QPixmap*)v);
	break;
    case Image:
	value.ptr = new QImage(*(QImage*)v);
	break;
    case Brush:
	value.ptr = new QBrush(*(QBrush*)v);
	break;
    case Point:
	value.ptr = new QPoint(*(QPoint*)v);
	break;
    case Rect:
	value.ptr = new QRect(*(QRect*)v);
	break;
    case Size:
	value.ptr = new QSize(*(QSize*)v);
	break;
    case Color:
	value.ptr = new QColor(*(QColor*)v);
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	value.ptr = new QPalette(*(QPalette*)v);
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup:
	value.ptr = new QColorGroup(*(QColorGroup*)v);
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	value.ptr = new QIconSet(*(QIconSet*)v);
	break;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
	value.ptr = new QMap<QString,QVariant>(*(QMap<QString,QVariant>*)v);
	break;
    case List:
	value.ptr = new QList<QVariant>(*(QList<QVariant>*)v);
	break;
#endif
    case Date:
	value.ptr = new QDate(*(QDate*)v);
	break;
    case Time:
	value.ptr = new QTime(*(QTime*)v);
	break;
    case DateTime:
	value.ptr = new QDateTime(*(QDateTime*)v);
	break;
    case ByteArray:
	value.ptr = new QByteArray(*(QByteArray*)v);
	break;
    case BitArray:
	value.ptr = new QBitArray(*(QBitArray*)v);
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	value.ptr = new QKeySequence(*(QKeySequence*)v);
	break;
#endif
    case Pen:
	value.ptr = new QPen(*(QPen*)v);
	break;
    case Int:
	value.i = *(int*)v;
	break;
    case UInt:
	value.u = *(uint*)v;
	break;
    case Bool:
	value.b = *(bool*)v;
	break;
    case Double:
	value.d = *(double*)v;
	break;
    case SizePolicy:
	value.ptr = new QSizePolicy(*(QSizePolicy*)v);
	break;
    case Cursor:
	value.ptr = new QCursor(*(QCursor*)v);
	break;
    default:
	Q_ASSERT( 0 );
    }
}

QVariant::Private::~Private()
{
    clear();
}

void QVariant::Private::clear()
{
    switch(type) {
    case QVariant::Bitmap:
	delete (QBitmap*)value.ptr;
	break;
    case QVariant::Cursor:
	delete (QCursor*)value.ptr;
	break;
    case QVariant::Region:
	delete (QRegion*)value.ptr;
	break;
    case QVariant::PointArray:
	delete (QPointArray*)value.ptr;
	break;
    case QVariant::String:
	delete (QString*)value.ptr;
	break;
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	delete (QStringList*)value.ptr;
	break;
#endif //QT_NO_STRINGLIST
    case QVariant::Font:
	delete (QFont*)value.ptr;
	break;
    case QVariant::Pixmap:
	delete (QPixmap*)value.ptr;
	break;
    case QVariant::Image:
	delete (QImage*)value.ptr;
	break;
    case QVariant::Brush:
	delete (QBrush*)value.ptr;
	break;
    case QVariant::Point:
	delete (QPoint*)value.ptr;
	break;
    case QVariant::Rect:
	delete (QRect*)value.ptr;
	break;
    case QVariant::Size:
	delete (QSize*)value.ptr;
	break;
    case QVariant::Color:
	delete (QColor*)value.ptr;
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	delete (QPalette*)value.ptr;
	break;
#ifndef QT_NO_COMPAT
    case QVariant::ColorGroup:
	delete (QColorGroup*)value.ptr;
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	delete (QIconSet*)value.ptr;
	break;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
	delete (QMap<QString,QVariant>*)value.ptr;
	break;
    case QVariant::List:
	delete (QList<QVariant>*)value.ptr;
	break;
#endif
    case QVariant::SizePolicy:
	delete (QSizePolicy*)value.ptr;
	break;
    case QVariant::Date:
	delete (QDate*)value.ptr;
	break;
    case QVariant::Time:
	delete (QTime*)value.ptr;
	break;
    case QVariant::DateTime:
	delete (QDateTime*)value.ptr;
	break;
    case QVariant::ByteArray:
	delete (QByteArray*)value.ptr;
	break;
    case QVariant::BitArray:
	delete (QBitArray*)value.ptr;
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	delete (QKeySequence*)value.ptr;
	break;
#endif
    case QVariant::Pen:
	delete (QPen*)value.ptr;
	break;
    case QVariant::Invalid:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Bool:
    case QVariant::Double:
	break;
    }

    type = QVariant::Invalid;
    is_null = TRUE;
}

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
    // x.isNull() == TRUE, y.isNull() == TRUE, z.isNull() == FALSE
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
    Constructs an invalid variant.
*/
QVariant::QVariant()
{
    d = new Private;
}

/*!
    \internal

    Constructs a variant of type \a type, and initializes with \a v if
    \a not 0.
*/
QVariant::QVariant( Type type, void *v )
{
    d = v ? new Private(type, v) : new Private(type);
}

/*!
    Destroys the QVariant and the contained object.

    Note that subclasses that reimplement clear() should reimplement
    the destructor to call clear(). This destructor calls clear(), but
    because it is the destructor, QVariant::clear() is called rather
    than a subclass's clear().
*/
QVariant::~QVariant()
{
    if ( d->deref() )
	delete d;
}

/*!
    Constructs a copy of the variant, \a p, passed as the argument to
    this constructor. Usually this is a deep copy, but a shallow copy
    is made if the stored data type is explicitly shared, as e.g.
    QImage is.
*/
QVariant::QVariant( const QVariant& p )
{
    p.d->ref();
    d = p.d;
}

#ifndef QT_NO_DATASTREAM
/*!
    Reads the variant from the data stream, \a s.
*/
QVariant::QVariant( QDataStream& s )
{
    d = new Private;
    s >> *this;
}
#endif //QT_NO_DATASTREAM

/*!
    Constructs a new variant with a string value, \a val.
*/
QVariant::QVariant( const QString& val )
{
    d = new Private;
    d->type = String;
    d->value.ptr = new QString( val );
}

/*!
    Constructs a new variant with a C-string value of \a val if \a val
    is non-null. The variant creates a deep copy of \a val.

    If \a val is null, the resulting variant has type Invalid.
*/
QVariant::QVariant( const char* val )
{
    d = new Private;
    if ( val == 0 )
	return;
    d->type = ByteArray;
    d->value.ptr = new QByteArray( val );
}

#ifndef QT_NO_STRINGLIST
/*!
    Constructs a new variant with a string list value, \a val.
*/
QVariant::QVariant( const QStringList& val )
{
    d = new Private;
    d->type = StringList;
    d->value.ptr = new QStringList( val );
    d->is_null = FALSE;
}
#endif // QT_NO_STRINGLIST

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Constructs a new variant with a map of QVariants, \a val.
*/
QVariant::QVariant( const QMap<QString,QVariant>& val )
{
    d = new Private;
    d->type = Map;
    d->value.ptr = new QMap<QString,QVariant>( val );
    d->is_null = FALSE;
}
#endif
/*!
    Constructs a new variant with a font value, \a val.
*/
QVariant::QVariant( const QFont& val )
{
    d = new Private;
    d->type = Font;
    d->value.ptr = new QFont( val );
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a pixmap value, \a val.
*/
QVariant::QVariant( const QPixmap& val )
{
    d = new Private;
    d->type = Pixmap;
    d->value.ptr = new QPixmap( val );
}


/*!
    Constructs a new variant with an image value, \a val.

    Because QImage is explicitly shared, you may need to pass a deep
    copy to the variant using QImage::copy(), e.g. if you intend
    changing the image you've passed later on.
*/
QVariant::QVariant( const QImage& val )
{
    d = new Private;
    d->type = Image;
    d->value.ptr = new QImage( val );
}

/*!
    Constructs a new variant with a brush value, \a val.
*/
QVariant::QVariant( const QBrush& val )
{
    d = new Private;
    d->type = Brush;
    d->value.ptr = new QBrush( val );
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a point value, \a val.
*/
QVariant::QVariant( const QPoint& val )
{
    d = new Private;
    d->type = Point;
    d->value.ptr = new QPoint( val );
}

/*!
    Constructs a new variant with a rect value, \a val.
*/
QVariant::QVariant( const QRect& val )
{
    d = new Private;
    d->type = Rect;
    d->value.ptr = new QRect( val );
}

/*!
    Constructs a new variant with a size value, \a val.
*/
QVariant::QVariant( const QSize& val )
{
    d = new Private;
    d->type = Size;
    d->value.ptr = new QSize( val );
}

/*!
    Constructs a new variant with a color value, \a val.
*/
QVariant::QVariant( const QColor& val )
{
    d = new Private;
    d->type = Color;
    d->value.ptr = new QColor( val );
    d->is_null = FALSE;
}

#ifndef QT_NO_PALETTE
/*!
    Constructs a new variant with a color palette value, \a val.
*/
QVariant::QVariant( const QPalette& val )
{
    d = new Private;
    d->type = Palette;
    d->value.ptr = new QPalette( val );
    d->is_null = FALSE;
}

#ifndef QT_NO_COMPAT
QVariant::QVariant( const QColorGroup& val )
{
    d = new Private;
    d->type = ColorGroup;
    d->value.ptr = new QColorGroup( val );
    d->is_null = FALSE;
}
#endif

#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
/*!
    Constructs a new variant with an icon set value, \a val.
*/
QVariant::QVariant( const QIconSet& val )
{
    d = new Private;
    d->type = IconSet;
    d->value.ptr = new QIconSet( val );
}
#endif //QT_NO_ICONSET
/*!
    Constructs a new variant with a region value, \a val.
*/
QVariant::QVariant( const QRegion& val )
{
    d = new Private;
    d->type = Region;
    // ## Force a detach
    d->value.ptr = new QRegion( val );
    ((QRegion*)d->value.ptr)->translate( 0, 0 );
}

/*!
    Constructs a new variant with a bitmap value, \a val.
*/
QVariant::QVariant( const QBitmap& val )
{
    d = new Private;
    d->type = Bitmap;
    d->value.ptr = new QBitmap( val );
}

/*!
    Constructs a new variant with a cursor value, \a val.
*/
QVariant::QVariant( const QCursor& val )
{
    d = new Private;
    d->type = Cursor;
    d->value.ptr = new QCursor( val );
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a point array value, \a val.

    Because QPointArray is explicitly shared, you may need to pass a
    deep copy to the variant using QPointArray::copy(), e.g. if you
    intend changing the point array you've passed later on.
*/
QVariant::QVariant( const QPointArray& val )
{
    d = new Private;
    d->type = PointArray;
    d->value.ptr = new QPointArray( val );
}

/*!
    Constructs a new variant with a date value, \a val.
*/
QVariant::QVariant( const QDate& val )
{
    d = new Private;
    d->type = Date;
    d->value.ptr = new QDate( val );
}

/*!
    Constructs a new variant with a time value, \a val.
*/
QVariant::QVariant( const QTime& val )
{
    d = new Private;
    d->type = Time;
    d->value.ptr = new QTime( val );
}

/*!
    Constructs a new variant with a date/time value, \a val.
*/
QVariant::QVariant( const QDateTime& val )
{
    d = new Private;
    d->type = DateTime;
    d->value.ptr = new QDateTime( val );
}

/*!
    Constructs a new variant with a bytearray value, \a val.
*/
QVariant::QVariant( const QByteArray& val )
{
    d = new Private;
    d->type = ByteArray;
    d->value.ptr = new QByteArray( val );
}

/*!
    Constructs a new variant with a bitarray value, \a val.
*/
QVariant::QVariant( const QBitArray& val )
{
    d = new Private;
    d->type = BitArray;
    d->value.ptr = new QBitArray( val );
}

#ifndef QT_NO_ACCEL

/*!
    Constructs a new variant with a key sequence value, \a val.
*/
QVariant::QVariant( const QKeySequence& val )
{
    d = new Private;
    d->type = KeySequence;
    d->value.ptr = new QKeySequence( val );
    d->is_null = FALSE;
}

#endif

/*!
    Constructs a new variant with a pen value, \a val.
*/
QVariant::QVariant( const QPen& val )
{
    d = new Private;
    d->type = Pen;
    d->value.ptr = new QPen( val );
}

/*!
    Constructs a new variant with an integer value, \a val.
*/
QVariant::QVariant( int val )
{
    d = new Private;
    d->type = Int;
    d->value.i = val;
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with an unsigned integer value, \a val.
*/
QVariant::QVariant( uint val )
{
    d = new Private;
    d->type = UInt;
    d->value.u = val;
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a long long integer value, \a val.
*/
QVariant::QVariant( Q_LLONG val )
{
    d = new Private;
    d->type = LongLong;
    d->value.ll = val;
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with an unsigned long long integer value, \a val.
*/

QVariant::QVariant( Q_ULLONG val )
{
    d = new Private;
    d->type = ULongLong;
    d->value.ull = val;
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a boolean value, \a val. The integer
    argument is a dummy, necessary for compatibility with some
    compilers.
*/
QVariant::QVariant( bool val, int )
{ // this is the comment that does NOT name said compiler.
    d = new Private;
    d->type = Bool;
    d->value.b = val;
    d->is_null = FALSE;
}


/*!
    Constructs a new variant with a floating point value, \a val.
*/
QVariant::QVariant( double val )
{
    d = new Private;
    d->type = Double;
    d->value.d = val;
    d->is_null = FALSE;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Constructs a new variant with a list value, \a val.
*/
QVariant::QVariant( const QList<QVariant>& val )
{
    d = new Private;
    d->type = List;
    d->value.ptr = new QList<QVariant>( val );
    d->is_null = FALSE;
}
#endif

/*!
    Constructs a new variant with a size policy value, \a val.
*/
QVariant::QVariant( QSizePolicy val )
{
    d = new Private;
    d->type = SizePolicy;
    d->value.ptr = new QSizePolicy( val );
    d->is_null = FALSE;
}

/*!
    Assigns the value of the variant \a variant to this variant.

    This is a deep copy of the variant, but note that if the variant
    holds an explicitly shared type such as QImage, a shallow copy is
    performed.
*/
QVariant& QVariant::operator= ( const QVariant& variant )
{
    QVariant& other = (QVariant&)variant;

    other.d->ref();
    if ( d->deref() )
	delete d;

    d = other.d;

    return *this;
}

/*!
    \internal
*/
void QVariant::detach()
{
    if ( d->count == 1 )
	return;

    d->deref();
    Private *x = new Private( d->type, data() );
    x->is_null = d->is_null;
    d = x;

}

/*!
    Returns the name of the type stored in the variant. The returned
    strings describe the C++ datatype used to store the data: for
    example, "QFont", "QString", or "QList<QVariant>". An Invalid
    variant returns 0.
*/
const char* QVariant::typeName() const
{
    return typeToName( (Type) d->type );
}

/*!
    Convert this variant to type Invalid and free up any resources
    used.
*/
void QVariant::clear()
{
    if ( d->count > 1 )
    {
	d->deref();
	d = new Private;
	return;
    }

    d->clear();
}

/* Attention!

   For dependency reasons, this table is duplicated in moc.y. If you
   change one, change both.

   (Search for the word 'Attention' in moc.y.)
*/
static const int ntypes = 34;
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
const char* QVariant::typeToName( Type typ )
{
    if ( typ >= ntypes )
	return 0;
    return type_map[typ];
}


/*!
    Converts the string representation of the storage type gven in \a
    name, to its enum representation.

    If the string representation cannot be converted to any enum
    representation, the variant is set to \c Invalid.
*/
QVariant::Type QVariant::nameToType( const char* name )
{
    if (name) {
	if ( strcmp(name, "QCString") == 0 )
	    return ByteArray;
	for ( int i = 1; i < ntypes; i++ ) {
	    if ( !strcmp( type_map[i], name ) )
		return (Type) i;
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
void QVariant::load( QDataStream& s )
{
    Q_UINT32 u;
    s >> u;
    Type t = (Type)u;

    switch( t ) {
    case Invalid: {
	// Since we wrote something, we should read something
	QString x;
	s >> x;
	d->type = t;
	d->is_null = TRUE;
    }
	break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map: {
	QMap<QString,QVariant>* x = new QMap<QString,QVariant>;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
    case List: {
	QList<QVariant>* x = new QList<QVariant>;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
#endif
    case Cursor: {
#ifndef QT_NO_CURSOR
	QCursor* x = new QCursor;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
#endif
    }
	break;
    case Bitmap: {
	QBitmap* x = new QBitmap;
#ifndef QT_NO_IMAGEIO
	s >> *x;
#endif
	d->value.ptr = x;
    }
	break;
    case Region: {
	QRegion* x = new QRegion;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case PointArray: {
	QPointArray* x = new QPointArray;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case String: {
	QString* x = new QString;
	s >> *x;
	d->value.ptr = x;
    }
	break;
#ifndef QT_NO_STRINGLIST
    case StringList: {
	QStringList* x = new QStringList;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
#endif // QT_NO_STRINGLIST
    case Font: {
	QFont* x = new QFont;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
    case Pixmap: {
	QPixmap* x = new QPixmap;
#ifndef QT_NO_IMAGEIO
	s >> *x;
#endif
	d->value.ptr = x;
    }
	break;
    case Image: {
	QImage* x = new QImage;
#ifndef QT_NO_IMAGEIO
	s >> *x;
#endif
	d->value.ptr = x;
    }
	break;
    case Brush: {
	QBrush* x = new QBrush;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
    case Rect: {
	QRect* x = new QRect;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Point: {
	QPoint* x = new QPoint;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Size: {
	QSize* x = new QSize;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Color: {
	QColor* x = new QColor;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
#ifndef QT_NO_PALETTE
    case Palette: {
	QPalette* x = new QPalette;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup: {
	QColorGroup* x = new QColorGroup;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet: {
	QPixmap x;
	s >> x;
	d->value.ptr = new QIconSet( x );
    }
	break;
#endif
    case Int: {
	int x;
	s >> x;
	d->value.i = x;
	d->is_null = FALSE;
    }
	break;
    case UInt: {
	uint x;
	s >> x;
	d->value.u = x;
	d->is_null = FALSE;
    }
	break;
    case LongLong: {
	Q_LLONG x;
	s >> x;
	d->value.ll = x;
    }
	break;
    case ULongLong: {
	Q_ULLONG x;
	s >> x;
	d->value.ull = x;
    }
	break;
    case Bool: {
	Q_INT8 x;
	s >> x;
	d->value.b = x;
	d->is_null = FALSE;
    }
	break;
    case Double: {
	double x;
	s >> x;
	d->value.d = x;
	d->is_null = FALSE;
    }
	break;
    case SizePolicy: {
	int h,v;
	Q_INT8 hfw;
	s >> h >> v >> hfw;
	d->value.ptr = new QSizePolicy( (QSizePolicy::SizeType)h,
					(QSizePolicy::SizeType)v,
					(bool) hfw);
	d->is_null = FALSE;
    }
	break;
    case Date: {
	QDate* x = new QDate;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case Time: {
	QTime* x = new QTime;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case DateTime: {
	QDateTime* x = new QDateTime;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case ByteArray: {
	QByteArray* x = new QByteArray;
	s >> *x;
	d->value.ptr = x;
    }
	break;
    case BitArray: {
	QBitArray* x = new QBitArray;
	s >> *x;
	d->value.ptr = x;
    }
	break;
#ifndef QT_NO_ACCEL
    case KeySequence: {
	QKeySequence* x = new QKeySequence;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
#endif // QT_NO_ACCEL
    case Pen: {
	QPen* x = new QPen;
	s >> *x;
	d->value.ptr = x;
	d->is_null = FALSE;
    }
	break;
    }
    d->type = t;
}

/*!
    Internal function for saving a variant to the stream \a s. Use the
    stream operators instead.

    \internal
*/
void QVariant::save( QDataStream& s ) const
{
    s << (Q_UINT32)type();

    switch( d->type ) {
    case Cursor:
	s << *((QCursor*)d->value.ptr);
	break;
    case Bitmap:
#ifndef QT_NO_IMAGEIO
	s << *((QBitmap*)d->value.ptr);
#endif
	break;
    case PointArray:
	s << *((QPointArray*)d->value.ptr);
	break;
    case Region:
	s << *((QRegion*)d->value.ptr);
	break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	s << *((QList<QVariant>*)d->value.ptr);
	break;
    case Map:
	s << *((QMap<QString,QVariant>*)d->value.ptr);
	break;
#endif
    case String:
	s << *((QString*)d->value.ptr);
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	s << *((QStringList*)d->value.ptr);
	break;
#endif
    case Font:
	s << *((QFont*)d->value.ptr);
	break;
    case Pixmap:
#ifndef QT_NO_IMAGEIO
	s << *((QPixmap*)d->value.ptr);
#endif
	break;
    case Image:
#ifndef QT_NO_IMAGEIO
	s << *((QImage*)d->value.ptr);
#endif
	break;
    case Brush:
	s << *((QBrush*)d->value.ptr);
	break;
    case Point:
	s << *((QPoint*)d->value.ptr);
	break;
    case Rect:
	s << *((QRect*)d->value.ptr);
	break;
    case Size:
	s << *((QSize*)d->value.ptr);
	break;
    case Color:
	s << *((QColor*)d->value.ptr);
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	s << *((QPalette*)d->value.ptr);
	break;
#ifndef QT_NO_COMPAT
    case ColorGroup:
	s << *((QColorGroup*)d->value.ptr);
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	//### add stream operator to iconset
	s << ((QIconSet*)d->value.ptr)->pixmap();
	break;
#endif
    case Int:
	s << d->value.i;
	break;
    case UInt:
	s << d->value.u;
	break;
    case LongLong:
	s << d->value.ll;
	break;
    case ULongLong:
	s << d->value.ull;
	break;
    case Bool:
	s << (Q_INT8)d->value.b;
	break;
    case Double:
	s << d->value.d;
	break;
    case SizePolicy:
	{
	    QSizePolicy p = toSizePolicy();
	    s << (int) p.horData() << (int) p.verData()
	      << (Q_INT8) p.hasHeightForWidth();
	}
	break;
    case Date:
	s << *((QDate*)d->value.ptr);
	break;
    case Time:
	s << *((QTime*)d->value.ptr);
	break;
    case DateTime:
	s << *((QDateTime*)d->value.ptr);
	break;
    case ByteArray:
	s << *((QByteArray*)d->value.ptr);
	break;
    case BitArray:
	s << *((QBitArray*)d->value.ptr);
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	s << *((QKeySequence*)d->value.ptr);
	break;
#endif
    case Pen:
	s << *((QPen*)d->value.ptr);
	break;
    case Invalid:
	s << QString(); // ### looks wrong.
	break;
    }
}

/*!
    Reads a variant \a p from the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator>> ( QDataStream& s, QVariant& p )
{
    p.load( s );
    return s;
}

/*!
    Writes a variant \a p to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream
    operators \endlink
*/
QDataStream& operator<< ( QDataStream& s, const QVariant& p )
{
    p.save( s );
    return s;
}

/*!
    Reads a variant type \a p in enum representation from the stream \a s.
*/
QDataStream& operator>> ( QDataStream& s, QVariant::Type& p )
{
    Q_UINT32 u;
    s >> u;
    p = (QVariant::Type) u;

    return s;
}

/*!
    Writes a variant type \a p to the stream \a s.
*/
QDataStream& operator<< ( QDataStream& s, const QVariant::Type p )
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

    Returns TRUE if the storage type of this variant is not
    QVariant::Invalid; otherwise returns FALSE.
*/

/*! \fn QByteArray QVariant::toCString() const
  \obsolete
    Returns the variant as a QCString if the variant has type()
    CString or String; otherwise returns 0.

    \sa asCString()
*/

/*!
    Returns the variant as a QString if the variant has type() String,
    ByteArray, Int, Uint, Bool, Double, Date, Time, DateTime,
    KeySequence, Font or Color; otherwise returns QString::null.

    \sa asString()
*/
QString QVariant::toString() const
{
    switch( d->type ) {
    case Int:
	return QString::number( toInt() );
    case UInt:
	return QString::number( toUInt() );
    case LongLong:
	return QString::number( toLongLong() );
    case ULongLong:
	return QString::number( toULongLong() );
    case Double:
	return QString::number( toDouble(), 'g', DBL_DIG );
#if !defined(QT_NO_SPRINTF) && !defined(QT_NO_DATESTRING)
    case Date:
	return toDate().toString( Qt::ISODate );
    case Time:
	return toTime().toString( Qt::ISODate );
    case DateTime:
	return toDateTime().toString( Qt::ISODate );
#endif
    case Bool:
	return toInt() ? "true" : "false";
#ifndef QT_NO_ACCEL
    case KeySequence:
	return (QString) *( (QKeySequence*)d->value.ptr );
#endif
    case ByteArray:
	return QString( *((QByteArray*)d->value.ptr) );
    case Font:
	return toFont().toString();
    case Color:
	return toColor().name();
    case String:
	return *((QString*)d->value.ptr);
    default:
	return QString::null;
    }
}

#ifndef QT_NO_STRINGLIST
/*!
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
QStringList QVariant::toStringList() const
{
    switch ( d->type ) {
    case StringList:
	return *((QStringList*)d->value.ptr);
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	{
	    QStringList slst;
	    QList<QVariant> list(toList());
	    QList<QVariant>::ConstIterator it = list.begin();
	    QList<QVariant>::ConstIterator end = list.end();
	    while( it != end ) {
		QString tmp = (*it).toString();
		++it;
		slst.append( tmp );
	    }
	    return slst;
	}
#endif
    default:
	return QStringList();
    }
}
#endif //QT_NO_STRINGLIST

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
    if ( d->type != Map )
	return QMap<QString,QVariant>();

    return *((QMap<QString,QVariant>*)d->value.ptr);
}
#endif
/*!
    Returns the variant as a QFont if the variant has type() Font;
    otherwise returns the application's default font.

  \sa asFont()
*/
QFont QVariant::toFont() const
{
    switch ( d->type ) {
    case String:
	{
	    QFont fnt;
	    fnt.fromString( toString() );
	    return fnt;
	}
    case Font:
	return *((QFont*)d->value.ptr);
    default:
	return QFont();
    }
}

/*!
    Returns the variant as a QPixmap if the variant has type() Pixmap;
    otherwise returns a null pixmap.

    \sa asPixmap()
*/
QPixmap QVariant::toPixmap() const
{
    if ( d->type != Pixmap )
	return QPixmap();

    return *((QPixmap*)d->value.ptr);
}

/*!
    Returns the variant as a QImage if the variant has type() Image;
    otherwise returns a null image.

    \sa asImage()
*/
const QImage QVariant::toImage() const
{
    if ( d->type != Image )
	return QImage();

    return *((QImage*)d->value.ptr);
}

/*!
    Returns the variant as a QBrush if the variant has type() Brush;
    otherwise returns a default brush (with all black colors).

    \sa asBrush()
*/
QBrush QVariant::toBrush() const
{
    if( d->type != Brush )
	return QBrush();

    return *((QBrush*)d->value.ptr);
}

/*!
    Returns the variant as a QPoint if the variant has type() Point;
    otherwise returns a point (0, 0).

    \sa asPoint()
*/
QPoint QVariant::toPoint() const
{
    if ( d->type != Point )
	return QPoint();

    return *((QPoint*)d->value.ptr);
}

/*!
    Returns the variant as a QRect if the variant has type() Rect;
    otherwise returns an empty rectangle.

    \sa asRect()
*/
QRect QVariant::toRect() const
{
    if ( d->type != Rect )
	return QRect();

    return *((QRect*)d->value.ptr);
}

/*!
    Returns the variant as a QSize if the variant has type() Size;
    otherwise returns an invalid size.

    \sa asSize()
*/
QSize QVariant::toSize() const
{
    if ( d->type != Size )
	return QSize();

    return *((QSize*)d->value.ptr);
}

/*!
    Returns the variant as a QColor if the variant has type() Color;
    otherwise returns an invalid color.

    \sa asColor()
*/
QColor QVariant::toColor() const
{
    switch ( d->type ) {
    case String:
	{
	    QColor col;
	    col.setNamedColor( toString() );
	    return col;
	}
    case Color:
	return *((QColor*)d->value.ptr);
    default:
	return QColor();
    }
}
#ifndef QT_NO_PALETTE
/*!
    Returns the variant as a QPalette if the variant has type()
    Palette; otherwise returns a completely black palette.

    \sa asPalette()
*/
QPalette QVariant::toPalette() const
{
    if ( d->type != Palette )
	return QPalette();

    return *((QPalette*)d->value.ptr);
}

#ifndef QT_NO_COMPAT
QColorGroup QVariant::toColorGroup() const
{
    if ( d->type != ColorGroup )
	return QColorGroup();
    return *((QColorGroup*)d->value.ptr);
}
#endif
#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
/*!
    Returns the variant as a QIconSet if the variant has type()
    IconSet; otherwise returns an icon set of null pixmaps.

    \sa asIconSet()
*/
QIconSet QVariant::toIconSet() const
{
    if ( d->type != IconSet )
	return QIconSet();

    return *((QIconSet*)d->value.ptr);
}
#endif //QT_NO_ICONSET
/*!
    Returns the variant as a QPointArray if the variant has type()
    PointArray; otherwise returns an empty QPointArray.

    \sa asPointArray()
*/
const QPointArray QVariant::toPointArray() const
{
    if ( d->type != PointArray )
	return QPointArray();

    return *((QPointArray*)d->value.ptr);
}

/*!
    Returns the variant as a QBitmap if the variant has type() Bitmap;
    otherwise returns a null QBitmap.

    \sa asBitmap()
*/
QBitmap QVariant::toBitmap() const
{
    if ( d->type != Bitmap )
	return QBitmap();

    return *((QBitmap*)d->value.ptr);
}

/*!
    Returns the variant as a QRegion if the variant has type() Region;
    otherwise returns an empty QRegion.

    \sa asRegion()
*/
QRegion QVariant::toRegion() const
{
    if ( d->type != Region )
	return QRegion();

    return *((QRegion*)d->value.ptr);
}

/*!
    Returns the variant as a QCursor if the variant has type() Cursor;
    otherwise returns the default arrow cursor.

    \sa asCursor()
*/
QCursor QVariant::toCursor() const
{
#ifndef QT_NO_CURSOR
    if ( d->type != Cursor )
	return QCursor();
#endif

    return *((QCursor*)d->value.ptr);
}

/*!
    Returns the variant as a QDate if the variant has type() Date,
    DateTime or String; otherwise returns an invalid date.

    Note that if the type() is String an invalid date will be returned
    if the string cannot be parsed as a Qt::ISODate format date.

    \sa asDate()
*/
QDate QVariant::toDate() const
{
    switch ( d->type ) {
    case Date:
	return *((QDate*)d->value.ptr);
    case DateTime:
	return ((QDateTime*)d->value.ptr)->date();
#ifndef QT_NO_DATESTRING
    case String:
	return QDate::fromString( *((QString*)d->value.ptr), Qt::ISODate );
#endif
    default:
	return QDate();
    }
}

/*!
    Returns the variant as a QTime if the variant has type() Time,
    DateTime or String; otherwise returns an invalid time.

    Note that if the type() is String an invalid time will be returned
    if the string cannot be parsed as a Qt::ISODate format time.

    \sa asTime()
*/
QTime QVariant::toTime() const
{
    switch ( d->type ) {
    case Time:
	return *((QTime*)d->value.ptr);
    case DateTime:
	return ((QDateTime*)d->value.ptr)->time();
#ifndef QT_NO_DATESTRING
    case String:
	return QTime::fromString( *((QString*)d->value.ptr), Qt::ISODate );
#endif
    default:
	return QTime();
    }
}

/*!
    Returns the variant as a QDateTime if the variant has type()
    DateTime or String; otherwise returns an invalid date/time.

    Note that if the type() is String an invalid date/time will be
    returned if the string cannot be parsed as a Qt::ISODate format
    date/time.

    \sa asDateTime()
*/
QDateTime QVariant::toDateTime() const
{
    switch ( d->type ) {
    case DateTime:
	return *((QDateTime*)d->value.ptr);
#ifndef QT_NO_DATESTRING
    case String:
	return QDateTime::fromString( *((QString*)d->value.ptr), Qt::ISODate );
#endif
    case Date:
	return QDateTime( *((QDate*)d->value.ptr) );
    default:
	return QDateTime();
    }
}

/*!
    Returns the variant as a QByteArray if the variant has type()
    ByteArray; otherwise returns an empty bytearray.

    \sa asByteArray()
*/
QByteArray QVariant::toByteArray() const
{
    if (d->type == ByteArray)
	return *((QByteArray*)d->value.ptr);
    else if (d->type == String)
	return ((QString*)d->value.ptr)->toAscii();
    return QByteArray();
}

/*!
    Returns the variant as a QBitArray if the variant has type()
    BitArray; otherwise returns an empty bitarray.

    \sa asBitArray()
*/
QBitArray QVariant::toBitArray() const
{
    if ( d->type == BitArray )
	return *((QBitArray*)d->value.ptr);
    return QBitArray();
}

#ifndef QT_NO_ACCEL

/*!
    Returns the variant as a QKeySequence if the variant has type()
    KeySequence, Int or String; otherwise returns an empty key
    sequence.

    Note that not all Ints and Strings are valid key sequences and in
    such cases an empty key sequence will be returned.

    \sa asKeySequence()
*/
QKeySequence QVariant::toKeySequence() const
{
    switch ( d->type ) {
    case KeySequence:
	return *((QKeySequence*)d->value.ptr);
    case String:
	return QKeySequence( toString() );
    case Int:
	return QKeySequence( toInt() );
    default:
	return QKeySequence();
    }
}

#endif // QT_NO_ACCEL

/*!
    Returns the variant as a QPen if the variant has type()
    Pen; otherwise returns an empty QPen.

    \sa asPen()
*/
QPen QVariant::toPen() const
{
    if ( d->type != Pen )
	return QPen();

    return *((QPen*)d->value.ptr);
}

/*!
    Returns the variant as an int if the variant has type() String,
    Int, UInt, Double, Bool or KeySequence; otherwise returns
    0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to an int; otherwise \a *ok is set to FALSE.

    \sa asInt() canCast()
*/
int QVariant::toInt( bool * ok ) const
{
    if ( ok )
	*ok = canCast( Int );

    switch ( d->type ) {
    case String:
	return ((QString*)d->value.ptr)->toInt( ok );
    case ByteArray:
	return QString(*((QByteArray*)d->value.ptr)).toInt( ok );
    case Int:
	return d->value.i;
    case UInt:
	return (int)d->value.u;
    case LongLong:
        return (int)d->value.ll;
    case ULongLong:
        return (int)d->value.ull;
    case Double:
	return (int)d->value.d;
    case Bool:
	return (int)d->value.b;
#ifndef QT_NO_ACCEL
    case KeySequence:
	return (int) *( (QKeySequence*)d->value.ptr );
#endif
    default:
	return 0;
    }
}

/*!
    Returns the variant as an unsigned int if the variant has type()
    String, ByteArray, UInt, Int, Double, or Bool; otherwise returns 0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to an unsigned int; otherwise \a *ok is set to FALSE.

    \sa asUInt()
*/
uint QVariant::toUInt( bool * ok ) const
{
    if ( ok )
	*ok = canCast( UInt );

    switch( d->type ) {
    case String:
	return ((QString*)d->value.ptr)->toUInt( ok );
    case ByteArray:
	return QString(*((QByteArray*)d->value.ptr)).toUInt( ok );
    case Int:
	return (uint)d->value.i;
    case UInt:
	return d->value.u;
    case LongLong:
        return (uint)d->value.ll;
    case ULongLong:
        return (uint)d->value.ull;
    case Double:
	return (uint)d->value.d;
    case Bool:
	return (uint)d->value.b;
    default:
	return 0;
    }
}

/*!
    Returns the variant as a long long int if the variant has type()
    LongLong, ULongLong, any type allowing a toInt() conversion;
    otherwise returns 0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to an int; otherwise \a *ok is set to FALSE.

    \sa asLongLong() canCast()
*/
Q_LLONG QVariant::toLongLong( bool * ok ) const
{
    if ( ok )
	*ok = canCast( LongLong );

    switch ( d->type ) {
    case String:
	return ((QString*)d->value.ptr)->toLongLong( ok );
    case ByteArray:
	return QString(*((QByteArray*)d->value.ptr)).toLongLong( ok );
    case Int:
	return (Q_LLONG)d->value.i;
    case UInt:
	return (Q_LLONG)d->value.u;
    case LongLong:
	return d->value.ll;
    case ULongLong:
	return (Q_LLONG)d->value.ull;
    case Double:
	return (Q_LLONG)d->value.d;
    case Bool:
	return (Q_LLONG)d->value.b;
    default:
	return 0;
    }
}

/*!
    Returns the variant as as an unsigned long long int if the variant
    has type() LongLong, ULongLong, any type allowing a toUInt()
    conversion; otherwise returns 0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to an int; otherwise \a *ok is set to FALSE.

    \sa asULongLong() canCast()
*/
Q_ULLONG QVariant::toULongLong( bool * ok ) const
{
    if ( ok )
	*ok = canCast( ULongLong );

    switch ( d->type ) {
    case Int:
	return (Q_ULLONG)d->value.i;
    case UInt:
	return (Q_ULLONG)d->value.u;
    case LongLong:
	return (Q_ULLONG)d->value.ll;
    case ULongLong:
	return d->value.ull;
    case Double:
	return (Q_ULLONG)d->value.d;
    case Bool:
	return (Q_ULLONG)d->value.b;
    case String:
	return ((QString*)d->value.ptr)->toULongLong( ok );
    case ByteArray:
	return QString(*((QByteArray*)d->value.ptr)).toULongLong( ok );
    default:
	return 0;
    }
}

/*!
    Returns the variant as a bool if the variant has type() Bool.

    Returns TRUE if the variant has type Int, UInt or Double and its
    value is non-zero, or if the variant has type String and its lower-case
    content is not empty, "0" or "false"; otherwise returns FALSE.

    \sa asBool()
*/
bool QVariant::toBool() const
{
    switch( d->type ) {
    case Bool:
	return d->value.b;
    case Double:
	return d->value.d != 0.0;
    case Int:
	return d->value.i != 0;
    case UInt:
	return d->value.u != 0;
    case LongLong:
	return d->value.ll != 0;
    case ULongLong:
	return d->value.ull != 0;
    case String:
	{
	    QString str = toString().lower();
	    return !(str == "0" || str == "false" || str.isEmpty() );
	}
    default:
	return FALSE;
    }
}

/*!
    Returns the variant as a double if the variant has type() String,
    ByteArray, Double, Int, UInt, LongLong, ULongLong or Bool; otherwise
    returns 0.0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to a double; otherwise \a *ok is set to FALSE.

    \sa asDouble()
*/
double QVariant::toDouble( bool * ok ) const
{
    if ( ok )
	*ok = canCast( Double );

    switch ( d->type ) {
    case String:
	return ((QString*)d->value.ptr)->toDouble( ok );
    case ByteArray:
	return QString(*((QByteArray*)d->value.ptr)).toDouble( ok );
    case Double:
	return d->value.d;
    case Int:
	return (double)d->value.i;
    case Bool:
	return (double)d->value.b;
    case UInt:
	return (double)d->value.u;
    case LongLong:
	return (double)d->value.ll;
    case ULongLong:
#if defined(Q_CC_MSVC) && !defined(Q_CC_MSVC_NET)
	return (double)(Q_LLONG)d->value.ull;
#else
	return (double)d->value.ull;
#endif
    default:
	return 0.0;
    }
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
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
    if ( d->type == List )
	return *((QList<QVariant>*)d->value.ptr);
#ifndef QT_NO_STRINGLIST
    if ( d->type == StringList ) {
	QList<QVariant> lst;
	QStringList slist(toStringList());
	QStringList::ConstIterator it = slist.begin();
	QStringList::ConstIterator end = slist.end();
	for( ; it != end; ++it )
	    lst.append( QVariant( *it ) );
	return lst;
    }
#endif //QT_NO_STRINGLIST
    return QList<QVariant>();
}
#endif

/*!
    Returns the variant as a QSizePolicy if the variant has type()
    SizePolicy; otherwise returns an undefined (but legal) size
    policy.
*/

QSizePolicy QVariant::toSizePolicy() const
{
    if ( d->type == SizePolicy )
	return *((QSizePolicy*)d->value.ptr);

    return QSizePolicy();
}


#define Q_VARIANT_AS( f ) Q##f& QVariant::as##f() { \
   if ( d->type != f ) *this = QVariant( to##f() ); else detach(); return *((Q##f*)d->value.ptr);}

Q_VARIANT_AS(String)
#ifndef QT_NO_STRINGLIST
Q_VARIANT_AS(StringList)
#endif
Q_VARIANT_AS(Font)
Q_VARIANT_AS(Pixmap)
Q_VARIANT_AS(Image)
Q_VARIANT_AS(Brush)
Q_VARIANT_AS(Point)
Q_VARIANT_AS(Rect)
Q_VARIANT_AS(Size)
Q_VARIANT_AS(Color)
#ifndef QT_NO_PALETTE
Q_VARIANT_AS(Palette)
#ifndef QT_NO_COMPAT
Q_VARIANT_AS(ColorGroup)
#endif
#endif
#ifndef QT_NO_ICONSET
Q_VARIANT_AS(IconSet)
#endif
Q_VARIANT_AS(PointArray)
Q_VARIANT_AS(Bitmap)
Q_VARIANT_AS(Region)
Q_VARIANT_AS(Cursor)
Q_VARIANT_AS(SizePolicy)
Q_VARIANT_AS(Date)
Q_VARIANT_AS(Time)
Q_VARIANT_AS(DateTime)
Q_VARIANT_AS(ByteArray)
Q_VARIANT_AS(BitArray)
#ifndef QT_NO_ACCEL
Q_VARIANT_AS(KeySequence)
#endif
Q_VARIANT_AS(Pen)

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
    Returns the variant's value as int reference.
*/
int& QVariant::asInt()
{
    if ( d->type != Int ) {
	detach();
	int i = toInt();
	d->clear();
 	d->value.i = i;
	d->type = Int;
    }
    return d->value.i;
}

/*!
    Returns the variant's value as unsigned int reference.
*/
uint& QVariant::asUInt()
{
    if ( d->type != UInt ) {
	detach();
	uint u = toUInt();
	d->clear();
	d->value.u = u;
	d->type = UInt;
    }
    return d->value.u;
}

/*!
    Returns the variant's value as long long reference.
*/
Q_LLONG& QVariant::asLongLong()
{
    if ( d->type != LongLong ) {
	detach();
	Q_LLONG ll = toLongLong();
	d->clear();
 	d->value.ll = ll;
	d->type = LongLong;
    }
    return d->value.ll;
}

/*!
    Returns the variant's value as unsigned long long reference.
*/
Q_ULLONG& QVariant::asULongLong()
{
    if ( d->type != ULongLong ) {
	detach();
	Q_ULLONG ull = toULongLong();
	d->clear();
 	d->value.ull = ull;
	d->type = ULongLong;
    }
    return d->value.ull;
}

/*!
    Returns the variant's value as bool reference.
*/
bool& QVariant::asBool()
{
    if ( d->type != Bool ) {
	detach();
	bool b = toBool();
	d->clear();
	d->value.b = b;
	d->type = Bool;
    }
    return d->value.b;
}

/*!
    Returns the variant's value as double reference.
*/
double& QVariant::asDouble()
{
    if ( d->type != Double ) {
	detach();
	double dbl = toDouble();
	d->clear();
	d->value.d = dbl;
	d->type = Double;
    }
    return d->value.d;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
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
QList<QVariant>& QVariant::asList()
{
    if ( d->type != List )
	*this = QVariant( toList() );
    return *((QList<QVariant>*)d->value.ptr);
}

/*!
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
QMap<QString, QVariant>& QVariant::asMap()
{
    if ( d->type != Map )
	*this = QVariant( toMap() );
    return *((QMap<QString,QVariant>*)d->value.ptr);
}
#endif

/*!
    Returns TRUE if the variant's type can be cast to the requested
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
bool QVariant::canCast( Type t ) const
{
    if ( Type( d->type ) == t )
	return TRUE;
    switch ( t ) {
    case Bool:
	return d->type == Double || d->type == Int || d->type == UInt
	     || d->type == LongLong || d->type == ULongLong || d->type == String;
    case Int:
	return d->type == String || d->type == Double || d->type == Bool
	    || d->type == UInt || d->type == LongLong || d->type == ULongLong
	    || d->type == KeySequence;
    case UInt:
	return d->type == String || d->type == Double || d->type == Bool ||
	       d->type == Int || d->type == LongLong || d->type == ULongLong;
    case LongLong:
	return d->type == String || d->type == Double || d->type == Bool ||
	       d->type == Int || d->type == UInt || d->type == ULongLong ||
	       d->type == KeySequence;
    case ULongLong:
	return d->type == String || d->type == Double || d->type == Bool
	    || d->type == Int || d->type == UInt || d->type == LongLong;
    case Double:
	return d->type == String || d->type == Int || d->type == Bool
	    || d->type == UInt;
    case String:
	return d->type == ByteArray || d->type == Int
	    || d->type == UInt || d->type == Bool || d->type == Double
	    || d->type == Date || d->type == Time || d->type == DateTime
	    || d->type == KeySequence || d->type == Font || d->type == Color
	    || d->type == LongLong || d->type == ULongLong;
    case ByteArray:
	return d->type == CString || d->type == String;
    case Date:
	return d->type == String || d->type == DateTime;
    case Time:
	return d->type == String || d->type == DateTime;
    case DateTime:
	return d->type == String || d->type == Date;
    case KeySequence:
	return d->type == String || d->type == Int;
    case Font:
	return d->type == String;
    case Color:
	return d->type == String;
#ifndef QT_NO_STRINGLIST
    case List:
	return d->type == StringList;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case StringList:
	if ( d->type == List ) {
	    QList<QVariant> varlist;
	    QList<QVariant>::ConstIterator it = varlist.begin();
	    QList<QVariant>::ConstIterator end = varlist.end();
	    for( ; it != end; ++it ) {
		if ( !(*it).canCast( String ) )
		    return FALSE;
	    }
	    return TRUE;
	}
	return FALSE;
#endif
    default:
	return FALSE;
    }
}

/*!
    Casts the variant to the requested type. If the cast cannot be
    done, the variant is set to the default value of the requested
    type (e.g. an empty string if the requested type \a t is
    QVariant::String, an empty point array if the requested type \a t
    is QVariant::PointArray, etc). Returns TRUE if the current type of
    the variant was successfully cast; otherwise returns FALSE.

    \sa canCast()
*/

bool QVariant::cast( Type t )
{
    switch ( t ) {
#ifndef QT_NO_TEMPLATE_VARIANT
    case QVariant::Map:
	asMap();
	break;
    case QVariant::List:
	asList();
	break;
#endif
    case QVariant::String:
	asString();
	break;
#ifndef QT_NO_STRINGLIST
    case QVariant::StringList:
	asStringList();
	break;
#endif
    case QVariant::Font:
	asFont();
	break;
    case QVariant::Pixmap:
	asPixmap();
	break;
    case QVariant::Brush:
	asBrush();
	break;
    case QVariant::Rect:
	asRect();
	break;
    case QVariant::Size:
	asSize();
	break;
    case QVariant::Color:
	asColor();
	break;
#ifndef QT_NO_PALETTE
    case QVariant::Palette:
	asPalette();
	break;
#ifndef QT_NO_COMPAT
    case QVariant::ColorGroup:
	asColorGroup();
	break;
#endif
#endif
#ifndef QT_NO_ICONSET
    case QVariant::IconSet:
	asIconSet();
	break;
#endif
    case QVariant::Point:
	asPoint();
	break;
    case QVariant::Image:
	asImage();
	break;
    case QVariant::Int:
	asInt();
	break;
    case QVariant::UInt:
	asUInt();
	break;
    case QVariant::Bool:
	asBool();
	break;
    case QVariant::Double:
	asDouble();
	break;
    case QVariant::PointArray:
	asPointArray();
	break;
    case QVariant::Region:
	asRegion();
	break;
    case QVariant::Bitmap:
	asBitmap();
	break;
    case QVariant::Cursor:
	asCursor();
	break;
    case QVariant::SizePolicy:
	asSizePolicy();
	break;
    case QVariant::Date:
	asDate();
	break;
    case QVariant::Time:
	asTime();
	break;
    case QVariant::DateTime:
	asDateTime();
	break;
    case QVariant::ByteArray:
	asByteArray();
	break;
    case QVariant::BitArray:
	asBitArray();
	break;
#ifndef QT_NO_ACCEL
    case QVariant::KeySequence:
	asKeySequence();
	break;
#endif
    case QVariant::Pen:
	asPen();
	break;
    case QVariant::LongLong:
	asLongLong();
	break;
    case QVariant::ULongLong:
	asULongLong();
	break;
    default:
    case QVariant::Invalid:
	(*this) = QVariant();
    }
    return canCast( t );
}

/*!
    Compares this QVariant with \a v and returns TRUE if they are
    equal; otherwise returns FALSE.
*/

bool QVariant::operator==( const QVariant &v ) const
{
    if ( !v.canCast( type() ) )
	return FALSE;
    switch( d->type ) {
    case Cursor:
#ifndef QT_NO_CURSOR
	return v.toCursor().shape() == toCursor().shape();
#endif
    case Bitmap:
	return v.toBitmap().serialNumber() == toBitmap().serialNumber();
    case PointArray:
	return v.toPointArray() == toPointArray();
    case Region:
	return v.toRegion() == toRegion();
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	return v.toList() == toList();
    case Map: {
	if ( v.toMap().count() != toMap().count() )
	    return FALSE;
	QMap<QString, QVariant>::ConstIterator it = v.toMap().begin();
	QMap<QString, QVariant>::ConstIterator it2 = toMap().begin();
	while ( it != v.toMap().end() ) {
	    if ( *it != *it2 )
		return FALSE;
	    ++it;
	    ++it2;
	}
	return TRUE;
    }
#endif
    case String:
	return v.toString() == toString();
#ifndef QT_NO_STRINGLIST
    case StringList:
	return v.toStringList() == toStringList();
#endif
    case Font:
	return v.toFont() == toFont();
    case Pixmap:
	return v.toPixmap().serialNumber() == toPixmap().serialNumber();
    case Image:
	return v.toImage() == toImage();
    case Brush:
	return v.toBrush() == toBrush();
    case Point:
	return v.toPoint() == toPoint();
    case Rect:
	return v.toRect() == toRect();
    case Size:
	return v.toSize() == toSize();
    case Color:
	return v.toColor() == toColor();
#ifndef QT_NO_PALETTE
    case Palette:
	return v.toPalette() == toPalette();
#ifndef QT_NO_COMPAT
    case ColorGroup:
	return v.toColorGroup() == toColorGroup();
#endif
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	return v.toIconSet().pixmap().serialNumber()
	    == toIconSet().pixmap().serialNumber();
#endif
    case Int:
	return v.toInt() == toInt();
    case UInt:
	return v.toUInt() == toUInt();
    case LongLong:
	return v.toLongLong() == toLongLong();
    case ULongLong:
	return v.toULongLong() == toULongLong();
    case Bool:
	return v.toBool() == toBool();
    case Double:
	return v.toDouble() == toDouble();
    case SizePolicy:
	return v.toSizePolicy() == toSizePolicy();
    case Date:
	return v.toDate() == toDate();
    case Time:
	return v.toTime() == toTime();
    case DateTime:
	return v.toDateTime() == toDateTime();
    case ByteArray:
	return v.toByteArray() == toByteArray();
    case BitArray:
	return v.toBitArray() == toBitArray();
#ifndef QT_NO_ACCEL
    case KeySequence:
	return v.toKeySequence() == toKeySequence();
#endif
    case Pen:
	return v.toPen() == toPen();
    case Invalid:
	break;
    }
    return FALSE;
}

/*!
    Compares this QVariant with \a v and returns TRUE if they are not
    equal; otherwise returns FALSE.
*/

bool QVariant::operator!=( const QVariant &v ) const
{
    return !( v == *this );
}


/*! \internal

  Reads or sets the variant type and ptr
 */
void* QVariant::rawAccess( void* ptr, Type typ, bool deepCopy )
{
    if ( ptr ) {
	clear();
	d->type = typ;
	d->value.ptr = ptr;
	d->is_null = FALSE;
	if ( deepCopy ) {
	    QVariant::Private* p = new Private( d->type, data() );
	    p->is_null = d->is_null;
	    d->type = Invalid;
	    delete d;
	    d = p;
	}
    }

    if ( !deepCopy )
	return d->value.ptr;
    QVariant::Private* p = new Private( d->type, data() );
    p->is_null = d->is_null;
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

/*!
  Returns TRUE if this is a NULL variant, FALSE otherwise.
*/
bool QVariant::isNull() const
{
    switch( d->type ) {
    case Bitmap:
	return ((QBitmap*) d->value.ptr)->isNull();
    case Region:
	return ((QRegion*) d->value.ptr)->isEmpty();
    case PointArray:
	return ((QPointArray*) d->value.ptr)->isEmpty();
    case String:
	return ((QString*) d->value.ptr)->isNull();
    case Pixmap:
	return ((QPixmap*) d->value.ptr)->isNull();
    case Image:
	return ((QImage*) d->value.ptr)->isNull();
    case Point:
	return ((QPoint*) d->value.ptr)->isNull();
    case Rect:
	return ((QRect*) d->value.ptr)->isNull();
    case Size:
	return ((QSize*) d->value.ptr)->isNull();
#ifndef QT_NO_ICONSET
    case IconSet:
	return ((QIconSet*) d->value.ptr)->isNull();
#endif
    case Date:
	return ((QDate*) d->value.ptr)->isNull();
    case Time:
	return ((QTime*) d->value.ptr)->isNull();
    case DateTime:
	return ((QDateTime*) d->value.ptr)->isNull();
    case ByteArray:
	return ((QByteArray*) d->value.ptr)->isNull();
    case BitArray:
	return ((QBitArray*) d->value.ptr)->isNull();
    case Cursor:
#ifndef QT_NO_STRINGLIST
    case StringList:
#endif //QT_NO_STRINGLIST
    case Font:
    case Brush:
    case Color:
#ifndef QT_NO_PALETTE
    case Palette:
#ifndef QT_NO_COMPAT
    case ColorGroup:
#endif
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
    case List:
#endif
    case SizePolicy:
#ifndef QT_NO_ACCEL
    case KeySequence:
#endif
    case Pen:
    case Invalid:
    case Int:
    case UInt:
    case LongLong:
    case ULongLong:
    case Bool:
    case Double:
	break;
    }
    return d->is_null;
}
#endif //QT_NO_VARIANT
