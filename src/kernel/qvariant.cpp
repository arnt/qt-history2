/****************************************************************************
** $Id: $
**
** Implementation of QVariant class
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

#include "qvariant.h"
#ifndef QT_NO_VARIANT
#include "qstring.h"
#include "qcstring.h"
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

// Uncomment to test for memory leaks or to run qt/test/qvariant/main.cpp
#define QVARIANT_DEBUG

extern Q_EXPORT const int qt_variant_types[];

const int qt_variant_types[]  = {
	QVariant::Invalid,
	QVariant::Map,
	QVariant::List,
	QVariant::String,
	QVariant::StringList,
	QVariant::Font,
	QVariant::Pixmap,
	QVariant::Brush,
	QVariant::Rect,
	QVariant::Size,
	QVariant::Color,
	QVariant::Palette,
	QVariant::ColorGroup,
	QVariant::IconSet,
	QVariant::Point,
	QVariant::Image,
	QVariant::Int,
	QVariant::UInt,
	QVariant::Bool,
	QVariant::Double,
	QVariant::CString,
	QVariant::PointArray,
	QVariant::Region,
	QVariant::Bitmap,
	QVariant::Cursor,
	QVariant::SizePolicy,
	QVariant::Date,
	QVariant::Time,
	QVariant::DateTime,
	QVariant::ByteArray,
	QVariant::BitArray,
	QVariant::KeySequence,
	QVariant::Pen
};


#ifdef QVARIANT_DEBUG
int qv_count = 0;
int get_qv_count() { return qv_count; }
#endif

QVariant::Private::Private()
{
#ifdef QVARIANT_DEBUG
    qv_count++;
#endif
    typ = QVariant::Invalid;
    is_null = TRUE;
}

QVariant::Private::Private( Private* d )
{
#ifdef QVARIANT_DEBUG
    qv_count++;
#endif

    switch( d->typ )
	{
	case QVariant::Invalid:
	    break;
	case QVariant::Bitmap:
	    value.ptr = new QBitmap( *((QBitmap*)d->value.ptr) );
	    break;
	case QVariant::Region:
	    value.ptr = new QRegion( *((QRegion*)d->value.ptr) );
	    // ## Force a detach
	    // ((QRegion*)value.ptr)->translate( 0, 0 );
	    break;
	case QVariant::PointArray:
	    // QPointArray is explicit shared
	    value.ptr = new QPointArray( *((QPointArray*)d->value.ptr) );
	    break;
	case QVariant::String:
	    value.ptr = new QString( *((QString*)d->value.ptr) );
	    break;
	case QVariant::CString:
	    // QCString is explicit shared
	    value.ptr = new QCString( *((QCString*)d->value.ptr) );
	    break;
#ifndef QT_NO_STRINGLIST
	case QVariant::StringList:
	    value.ptr = new QStringList( *((QStringList*)d->value.ptr) );
	    break;
#endif //QT_NO_STRINGLIST
	case QVariant::Font:
	    value.ptr = new QFont( *((QFont*)d->value.ptr) );
	    break;
	case QVariant::Pixmap:
	    value.ptr = new QPixmap( *((QPixmap*)d->value.ptr) );
	    break;
	case QVariant::Image:
	    // QImage is explicit shared
	    value.ptr = new QImage( *((QImage*)d->value.ptr) );
	    break;
	case QVariant::Brush:
	    value.ptr = new QBrush( *((QBrush*)d->value.ptr) );
	    // ## Force a detach
	    // ((QBrush*)value.ptr)->setColor( ((QBrush*)value.ptr)->color() );
	    break;
	case QVariant::Point:
	    value.ptr = new QPoint( *((QPoint*)d->value.ptr) );
	    break;
	case QVariant::Rect:
	    value.ptr = new QRect( *((QRect*)d->value.ptr) );
	    break;
	case QVariant::Size:
	    value.ptr = new QSize( *((QSize*)d->value.ptr) );
	    break;
	case QVariant::Color:
	    value.ptr = new QColor( *((QColor*)d->value.ptr) );
	    break;
#ifndef QT_NO_PALETTE
	case QVariant::Palette:
	    value.ptr = new QPalette( *((QPalette*)d->value.ptr) );
	    break;
	case QVariant::ColorGroup:
	    value.ptr = new QColorGroup( *((QColorGroup*)d->value.ptr) );
	    break;
#endif
#ifndef QT_NO_ICONSET
	case QVariant::IconSet:
	    value.ptr = new QIconSet( *((QIconSet*)d->value.ptr) );
	    break;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
	case QVariant::Map:
	    value.ptr = new QMap<QString,QVariant>( *((QMap<QString,QVariant>*)d->value.ptr) );
	    break;
	case QVariant::List:
	    value.ptr = new QValueList<QVariant>( *((QValueList<QVariant>*)d->value.ptr) );
	    break;
#endif
	case QVariant::Date:
	    value.ptr = new QDate( *((QDate*)d->value.ptr) );
	    break;
	case QVariant::Time:
	    value.ptr = new QTime( *((QTime*)d->value.ptr) );
	    break;
	case QVariant::DateTime:
	    value.ptr = new QDateTime( *((QDateTime*)d->value.ptr) );
	    break;
	case QVariant::ByteArray:
	    value.ptr = new QByteArray( *((QByteArray*)d->value.ptr) );
	    break;
	case QVariant::BitArray:
	    value.ptr = new QBitArray( *((QBitArray*)d->value.ptr) );
	    break;
#ifndef QT_NO_ACCEL
	case QVariant::KeySequence:
	    value.ptr = new QKeySequence( *((QKeySequence*)d->value.ptr) );
	    break;
#endif
	case QVariant::Pen:
	    value.ptr = new QPen( *((QPen*)d->value.ptr) );
	    break;
	case QVariant::Int:
	    value.i = d->value.i;
	    break;
	case QVariant::UInt:
	    value.u = d->value.u;
	    break;
	case QVariant::Bool:
	    value.b = d->value.b;
	    break;
	case QVariant::Double:
	    value.d = d->value.d;
	    break;
	case QVariant::SizePolicy:
	    value.ptr = new QSizePolicy( *((QSizePolicy*)d->value.ptr) );
	    break;
	case QVariant::Cursor:
	    value.ptr = new QCursor( *((QCursor*)d->value.ptr) );
	    break;
	default:
	    Q_ASSERT( 0 );
	}

    typ = d->typ;
    is_null = d->is_null;
}

QVariant::Private::~Private()
{
#ifdef QVARIANT_DEBUG
    qv_count--;
#endif
    clear();
}

void QVariant::Private::clear()
{
    switch( typ )
	{
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
	case QVariant::CString:
	    delete (QCString*)value.ptr;
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
	case QVariant::ColorGroup:
	    delete (QColorGroup*)value.ptr;
	    break;
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
	    delete (QValueList<QVariant>*)value.ptr;
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
	case QVariant::Bool:
	case QVariant::Double:
	    break;
	}

    typ = QVariant::Invalid;
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

    Note that three data types supported by QVariant are explicitly
    shared, namely QImage, QPointArray, and QCString, and in these
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
    v = QVariant("hello");    // The variant now contains a QCString
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

    You can even store QValueList<QVariant>s and
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
    \value ColorGroup  a QColorGroup
    \value Cursor  a QCursor
    \value Date  a QDate
    \value DateTime  a QDateTime
    \value Double  a double
    \value Font  a QFont
    \value IconSet  a QIconSet
    \value Image  a QImage
    \value Int  an int
    \value KeySequence  a QKeySequence
    \value List  a QValueList<QVariant>
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
    \value CString  a QCString
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
    d = new Private;
    *this = p;
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
    d->typ = String;
    d->value.ptr = new QString( val );
}

/*!
    Constructs a new variant with a C-string value, \a val.

    If you want to modify the QCString after you've passed it to this
    constructor, we recommend passing a deep copy (see
    QCString::copy()).
*/
QVariant::QVariant( const QCString& val )
{
    d = new Private;
    d->typ = CString;
    d->value.ptr = new QCString( val );
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
    d->typ = CString;
    d->value.ptr = new QCString( val );
}

#ifndef QT_NO_STRINGLIST
/*!
    Constructs a new variant with a string list value, \a val.
*/
QVariant::QVariant( const QStringList& val )
{
    d = new Private;
    d->typ = StringList;
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
    d->typ = Map;
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
    d->typ = Font;
    d->value.ptr = new QFont( val );
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a pixmap value, \a val.
*/
QVariant::QVariant( const QPixmap& val )
{
    d = new Private;
    d->typ = Pixmap;
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
    d->typ = Image;
    d->value.ptr = new QImage( val );
}

/*!
    Constructs a new variant with a brush value, \a val.
*/
QVariant::QVariant( const QBrush& val )
{
    d = new Private;
    d->typ = Brush;
    d->value.ptr = new QBrush( val );
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a point value, \a val.
*/
QVariant::QVariant( const QPoint& val )
{
    d = new Private;
    d->typ = Point;
    d->value.ptr = new QPoint( val );
}

/*!
    Constructs a new variant with a rect value, \a val.
*/
QVariant::QVariant( const QRect& val )
{
    d = new Private;
    d->typ = Rect;
    d->value.ptr = new QRect( val );
}

/*!
    Constructs a new variant with a size value, \a val.
*/
QVariant::QVariant( const QSize& val )
{
    d = new Private;
    d->typ = Size;
    d->value.ptr = new QSize( val );
}

/*!
    Constructs a new variant with a color value, \a val.
*/
QVariant::QVariant( const QColor& val )
{
    d = new Private;
    d->typ = Color;
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
    d->typ = Palette;
    d->value.ptr = new QPalette( val );
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with a color group value, \a val.
*/
QVariant::QVariant( const QColorGroup& val )
{
    d = new Private;
    d->typ = ColorGroup;
    d->value.ptr = new QColorGroup( val );
    d->is_null = FALSE;
}
#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
/*!
    Constructs a new variant with an icon set value, \a val.
*/
QVariant::QVariant( const QIconSet& val )
{
    d = new Private;
    d->typ = IconSet;
    d->value.ptr = new QIconSet( val );
}
#endif //QT_NO_ICONSET
/*!
    Constructs a new variant with a region value, \a val.
*/
QVariant::QVariant( const QRegion& val )
{
    d = new Private;
    d->typ = Region;
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
    d->typ = Bitmap;
    d->value.ptr = new QBitmap( val );
}

/*!
    Constructs a new variant with a cursor value, \a val.
*/
QVariant::QVariant( const QCursor& val )
{
    d = new Private;
    d->typ = Cursor;
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
    d->typ = PointArray;
    d->value.ptr = new QPointArray( val );
}

/*!
    Constructs a new variant with a date value, \a val.
*/
QVariant::QVariant( const QDate& val )
{
    d = new Private;
    d->typ = Date;
    d->value.ptr = new QDate( val );
}

/*!
    Constructs a new variant with a time value, \a val.
*/
QVariant::QVariant( const QTime& val )
{
    d = new Private;
    d->typ = Time;
    d->value.ptr = new QTime( val );
}

/*!
    Constructs a new variant with a date/time value, \a val.
*/
QVariant::QVariant( const QDateTime& val )
{
    d = new Private;
    d->typ = DateTime;
    d->value.ptr = new QDateTime( val );
}

/*!
    Constructs a new variant with a bytearray value, \a val.
*/
QVariant::QVariant( const QByteArray& val )
{
    d = new Private;
    d->typ = ByteArray;
    d->value.ptr = new QByteArray( val );
}

/*!
    Constructs a new variant with a bitarray value, \a val.
*/
QVariant::QVariant( const QBitArray& val )
{
    d = new Private;
    d->typ = BitArray;
    d->value.ptr = new QBitArray( val );
}

#ifndef QT_NO_ACCEL

/*!
    Constructs a new variant with a key sequence value, \a val.
*/
QVariant::QVariant( const QKeySequence& val )
{
    d = new Private;
    d->typ = KeySequence;
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
    d->typ = Pen;
    d->value.ptr = new QPen( val );
}

/*!
    Constructs a new variant with an integer value, \a val.
*/
QVariant::QVariant( int val )
{
    d = new Private;
    d->typ = Int;
    d->value.i = val;
    d->is_null = FALSE;
}

/*!
    Constructs a new variant with an unsigned integer value, \a val.
*/
QVariant::QVariant( uint val )
{
    d = new Private;
    d->typ = UInt;
    d->value.u = val;
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
    d->typ = Bool;
    d->value.b = val;
    d->is_null = FALSE;
}


/*!
    Constructs a new variant with a floating point value, \a val.
*/
QVariant::QVariant( double val )
{
    d = new Private;
    d->typ = Double;
    d->value.d = val;
    d->is_null = FALSE;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Constructs a new variant with a list value, \a val.
*/
QVariant::QVariant( const QValueList<QVariant>& val )
{
    d = new Private;
    d->typ = List;
    d->value.ptr = new QValueList<QVariant>( val );
    d->is_null = FALSE;
}
#endif

/*!
    Constructs a new variant with a size policy value, \a val.
*/
QVariant::QVariant( QSizePolicy val )
{
    d = new Private;
    d->typ = SizePolicy;
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
    d = new Private( d );
}

/*!
    Returns the name of the type stored in the variant. The returned
    strings describe the C++ datatype used to store the data: for
    example, "QFont", "QString", or "QValueList<QVariant>". An Invalid
    variant returns 0.
*/
const char* QVariant::typeName() const
{
    return typeToName( (Type) d->typ );
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
static const int ntypes = 33;
static const char* const type_map[ntypes] =
{
    0,
    "QMap<QString,QVariant>",
    "QValueList<QVariant>",
    "QString",
    "QStringList",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
    "QColorGroup",
    "QIconSet",
    "QPoint",
    "QImage",
    "int",
    "uint",
    "bool",
    "double",
    "QCString",
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
    "QPen"
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
    for ( int i = 0; i < ntypes; i++ ) {
	if ( !qstrcmp( type_map[i], name ) )
	    return (Type) i;
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
    case Invalid:
	d->typ = t;
	break;
#ifndef QT_NO_TEMPLATE_VARIANT
    case Map:
	{
	    QMap<QString,QVariant>* x = new QMap<QString,QVariant>;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case List:
	{
	    QValueList<QVariant>* x = new QValueList<QVariant>;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif
    case Cursor:
	{
#ifndef QT_NO_CURSOR
	    QCursor* x = new QCursor;
	    s >> *x;
	    d->value.ptr = x;
#endif
	}
	break;
    case Bitmap:
	{
	    QBitmap* x = new QBitmap;
#ifndef QT_NO_IMAGEIO
	    s >> *x;
#endif
	    d->value.ptr = x;
	}
	break;
    case Region:
	{
	    QRegion* x = new QRegion;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case PointArray:
	{
	    QPointArray* x = new QPointArray;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case String:
	{
	    QString* x = new QString;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case CString:
	{
	    QCString* x = new QCString;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#ifndef QT_NO_STRINGLIST
    case StringList:
	{
	    QStringList* x = new QStringList;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif // QT_NO_STRINGLIST
    case Font:
	{
	    QFont* x = new QFont;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Pixmap:
	{
	    QPixmap* x = new QPixmap;
#ifndef QT_NO_IMAGEIO
	    s >> *x;
#endif
	    d->value.ptr = x;
	}
	break;
    case Image:
	{
	    QImage* x = new QImage;
#ifndef QT_NO_IMAGEIO
	    s >> *x;
#endif
	    d->value.ptr = x;
	}
	break;
    case Brush:
	{
	    QBrush* x = new QBrush;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Rect:
	{
	    QRect* x = new QRect;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Point:
	{
	    QPoint* x = new QPoint;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Size:
	{
	    QSize* x = new QSize;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Color:
	{
	    QColor* x = new QColor;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#ifndef QT_NO_PALETTE
    case Palette:
	{
	    QPalette* x = new QPalette;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case ColorGroup:
	{
	    QColorGroup* x = new QColorGroup;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif
#ifndef QT_NO_ICONSET
    case IconSet:
	{
	    QPixmap* x = new QPixmap;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif
    case Int:
	{
	    int x;
	    s >> x;
	    d->value.i = x;
	}
	break;
    case UInt:
	{
	    uint x;
	    s >> x;
	    d->value.u = x;
	}
	break;
    case Bool:
	{
	    Q_INT8 x;
	    s >> x;
	    d->value.b = x;
	}
	break;
    case Double:
	{
	    double x;
	    s >> x;
	    d->value.d = x;
	}
	break;
    case SizePolicy:
	{
	    int h,v;
	    Q_INT8 hfw;
	    s >> h >> v >> hfw;
	    d->value.ptr = new QSizePolicy( (QSizePolicy::SizeType)h,
					    (QSizePolicy::SizeType)v,
					    (bool) hfw);
	}
	break;
    case Date:
	{
	    QDate* x = new QDate;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case Time:
	{
	    QTime* x = new QTime;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case DateTime:
	{
	    QDateTime* x = new QDateTime;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case ByteArray:
	{
	    QByteArray* x = new QByteArray;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    case BitArray:
	{
	    QBitArray* x = new QBitArray;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#ifndef QT_NO_ACCEL
    case KeySequence:
	{
	    QKeySequence* x = new QKeySequence;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
#endif // QT_NO_ACCEL
    case Pen:
	{
	    QPen* x = new QPen;
	    s >> *x;
	    d->value.ptr = x;
	}
	break;
    }
    d->typ = t;
}

/*!
    Internal function for saving a variant to the stream \a s. Use the
    stream operators instead.

    \internal
*/
void QVariant::save( QDataStream& s ) const
{
    s << (Q_UINT32)type();

    switch( d->typ ) {
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
	s << *((QValueList<QVariant>*)d->value.ptr);
	break;
    case Map:
	s << *((QMap<QString,QVariant>*)d->value.ptr);
	break;
#endif
    case String:
	s << *((QString*)d->value.ptr);
	break;
    case CString:
	s << *((QCString*)d->value.ptr);
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
    case ColorGroup:
	s << *((QColorGroup*)d->value.ptr);
	break;
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
    case KeySequence:
	s << *((QKeySequence*)d->value.ptr);
	break;
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

/*!
    \fn QValueListConstIterator<QString> QVariant::stringListBegin() const

    Returns an iterator to the first string in the list if the
    variant's type is StringList; otherwise returns a null iterator.
*/

/*!
    \fn QValueListConstIterator<QString> QVariant::stringListEnd() const

    Returns the end iterator for the list if the variant's type is
    StringList; otherwise returns a null iterator.
*/

/*!
    \fn QValueListConstIterator<QVariant> QVariant::listBegin() const

    Returns an iterator to the first item in the list if the variant's
    type is appropriate; otherwise returns a null iterator.
*/

/*!
    \fn QValueListConstIterator<QVariant> QVariant::listEnd() const

    Returns the end iterator for the list if the variant's type is
    appropriate; otherwise returns a null iterator.
*/

/*!
    \fn QMapConstIterator<QString, QVariant> QVariant::mapBegin() const

    Returns an iterator to the first item in the map, if the variant's
    type is appropriate; otherwise returns a null iterator.
*/

/*!
    \fn QMapConstIterator<QString, QVariant> QVariant::mapEnd() const

    Returns the end iterator for the map, if the variant's type is
    appropriate; otherwise returns a null iterator.
*/

/*!
    \fn QMapConstIterator<QString, QVariant> QVariant::mapFind( const QString& key ) const

    Returns an iterator to the item in the map with \a key as key, if
    the variant's type is appropriate and \a key is a valid key;
    otherwise returns a null iterator.
*/

/*!
    Returns the variant as a QString if the variant has type() String,
    CString, ByteArray, Int, Uint, Bool, Double, Date, Time, DateTime,
    KeySequence, Font or Color; otherwise returns QString::null.

    \sa asString()
*/
const QString QVariant::toString() const
{
    switch( d->typ ) {
    case CString:
	return QString::fromLatin1( toCString() );
    case Int:
	return QString::number( toInt() );
    case UInt:
	return QString::number( toUInt() );
    case Double:
	return QString::number( toDouble() );
#if !defined(QT_NO_SPRINTF) && !defined(QT_NO_DATESTRING)
    case Date:
	return toDate().toString( Qt::ISODate );
    case Time:
	return toTime().toString( Qt::ISODate );
    case DateTime:
	return toDateTime().toString( Qt::ISODate );
#endif
    case Bool:
	return QString::number( toInt() );
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
/*!
    Returns the variant as a QCString if the variant has type()
    CString or String; otherwise returns 0.

    \sa asCString()
*/
const QCString QVariant::toCString() const
{
    switch ( d->typ ) {
    case CString:
	return *((QCString*)d->value.ptr);
    case String:
	return ((QString*)d->value.ptr)->latin1();
    default:
	return 0;
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
const QStringList QVariant::toStringList() const
{
    switch ( d->typ ) {
    case StringList:
	return *((QStringList*)d->value.ptr);
#ifndef QT_NO_TEMPLATE_VARIANT
    case List:
	{
	    QStringList lst;
	    QValueList<QVariant>::ConstIterator it = listBegin();
	    QValueList<QVariant>::ConstIterator end = listEnd();
	    while( it != end ) {
		QString tmp = (*it).toString();
		++it;
		lst.append( tmp );
	    }
	    return lst;
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
const QMap<QString, QVariant> QVariant::toMap() const
{
    if ( d->typ != Map )
	return QMap<QString,QVariant>();

    return *((QMap<QString,QVariant>*)d->value.ptr);
}
#endif
/*!
    Returns the variant as a QFont if the variant has type() Font;
    otherwise returns the application's default font.

  \sa asFont()
*/
const QFont QVariant::toFont() const
{
    switch ( d->typ ) {
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
const QPixmap QVariant::toPixmap() const
{
    if ( d->typ != Pixmap )
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
    if ( d->typ != Image )
	return QImage();

    return *((QImage*)d->value.ptr);
}

/*!
    Returns the variant as a QBrush if the variant has type() Brush;
    otherwise returns a default brush (with all black colors).

    \sa asBrush()
*/
const QBrush QVariant::toBrush() const
{
    if( d->typ != Brush )
	return QBrush();

    return *((QBrush*)d->value.ptr);
}

/*!
    Returns the variant as a QPoint if the variant has type() Point;
    otherwise returns a point (0, 0).

    \sa asPoint()
*/
const QPoint QVariant::toPoint() const
{
    if ( d->typ != Point )
	return QPoint();

    return *((QPoint*)d->value.ptr);
}

/*!
    Returns the variant as a QRect if the variant has type() Rect;
    otherwise returns an empty rectangle.

    \sa asRect()
*/
const QRect QVariant::toRect() const
{
    if ( d->typ != Rect )
	return QRect();

    return *((QRect*)d->value.ptr);
}

/*!
    Returns the variant as a QSize if the variant has type() Size;
    otherwise returns an invalid size.

    \sa asSize()
*/
const QSize QVariant::toSize() const
{
    if ( d->typ != Size )
	return QSize();

    return *((QSize*)d->value.ptr);
}

/*!
    Returns the variant as a QColor if the variant has type() Color;
    otherwise returns an invalid color.

    \sa asColor()
*/
const QColor QVariant::toColor() const
{
    switch ( d->typ ) {
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
const QPalette QVariant::toPalette() const
{
    if ( d->typ != Palette )
	return QPalette();

    return *((QPalette*)d->value.ptr);
}

/*!
    Returns the variant as a QColorGroup if the variant has type()
    ColorGroup; otherwise returns a completely black color group.

    \sa asColorGroup()
*/
const QColorGroup QVariant::toColorGroup() const
{
    if ( d->typ != ColorGroup )
	return QColorGroup();

    return *((QColorGroup*)d->value.ptr);
}
#endif //QT_NO_PALETTE
#ifndef QT_NO_ICONSET
/*!
    Returns the variant as a QIconSet if the variant has type()
    IconSet; otherwise returns an icon set of null pixmaps.

    \sa asIconSet()
*/
const QIconSet QVariant::toIconSet() const
{
    if ( d->typ != IconSet )
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
    if ( d->typ != PointArray )
	return QPointArray();

    return *((QPointArray*)d->value.ptr);
}

/*!
    Returns the variant as a QBitmap if the variant has type() Bitmap;
    otherwise returns a null QBitmap.

    \sa asBitmap()
*/
const QBitmap QVariant::toBitmap() const
{
    if ( d->typ != Bitmap )
	return QBitmap();

    return *((QBitmap*)d->value.ptr);
}

/*!
    Returns the variant as a QRegion if the variant has type() Region;
    otherwise returns an empty QRegion.

    \sa asRegion()
*/
const QRegion QVariant::toRegion() const
{
    if ( d->typ != Region )
	return QRegion();

    return *((QRegion*)d->value.ptr);
}

/*!
    Returns the variant as a QCursor if the variant has type() Cursor;
    otherwise returns the default arrow cursor.

    \sa asCursor()
*/
const QCursor QVariant::toCursor() const
{
#ifndef QT_NO_CURSOR
    if ( d->typ != Cursor )
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
const QDate QVariant::toDate() const
{
    switch ( d->typ ) {
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
const QTime QVariant::toTime() const
{
    switch ( d->typ ) {
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
const QDateTime QVariant::toDateTime() const
{
    switch ( d->typ ) {
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
const QByteArray QVariant::toByteArray() const
{
    if ( d->typ == ByteArray )
	return *((QByteArray*)d->value.ptr);
    return QByteArray();
}

/*!
    Returns the variant as a QBitArray if the variant has type()
    BitArray; otherwise returns an empty bitarray.

    \sa asBitArray()
*/
const QBitArray QVariant::toBitArray() const
{
    if ( d->typ == BitArray )
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
const QKeySequence QVariant::toKeySequence() const
{
    switch ( d->typ ) {
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
const QPen QVariant::toPen() const
{
    if ( d->typ != Pen )
	return QPen();

    return *((QPen*)d->value.ptr);
}

/*!
    Returns the variant as an int if the variant has type() String,
    CString, Int, UInt, Double, Bool or KeySequence; otherwise returns
    0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to an int; otherwise \a *ok is set to FALSE.

    \sa asInt() canCast()
*/
int QVariant::toInt( bool * ok ) const
{
    if ( ok )
	*ok = canCast( UInt );

    switch ( d->typ ) {
    case String:
	return ((QString*)d->value.ptr)->toInt( ok );
    case CString:
	return ((QCString*)d->value.ptr)->toInt( ok );
    case Int:
	return d->value.i;
    case UInt:
	return (int)d->value.u;
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
    String, CString, UInt, Int, Double, or Bool; otherwise returns 0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to an unsigned int; otherwise \a *ok is set to FALSE.

    \sa asUInt()
*/
uint QVariant::toUInt( bool * ok ) const
{
    if ( ok )
	*ok = canCast( UInt );

    switch( d->typ ) {
    case String:
	return ((QString*)d->value.ptr)->toUInt( ok );
    case CString:
	return ((QCString*)d->value.ptr)->toUInt( ok );
    case Int:
	return d->value.i;
    case UInt:
	return (int)d->value.u;
    case Double:
	return (int)d->value.d;
    case Bool:
	return (int)d->value.b;
    default:
	return 0;
    }
}

/*!
    Returns the variant as a bool if the variant has type() Bool.

    Returns TRUE if the variant has type Int, UInt or Double and its
    value is non-zero; otherwise returns FALSE.

    \sa asBool()
*/
bool QVariant::toBool() const
{
    switch( d->typ ) {
    case Bool:
	return d->value.b;
    case Double:
	return d->value.d != 0.0;
    case Int:
	return d->value.i != 0;
    case UInt:
	return d->value.u != 0;
    default:
	return FALSE;
    }
}

/*!
    Returns the variant as a double if the variant has type() String,
    CString, Double, Int, UInt, or Bool; otherwise returns 0.0.

    If \a ok is non-null: \a *ok is set to TRUE if the value could be
    converted to a double; otherwise \a *ok is set to FALSE.

    \sa asDouble()
*/
double QVariant::toDouble( bool * ok ) const
{
    if ( ok )
	*ok = canCast( Double );

    switch ( d->typ ) {
    case String:
	return ((QString*)d->value.ptr)->toDouble( ok );
    case CString:
	return ((QCString*)d->value.ptr)->toDouble( ok );
    case Double:
	return d->value.d;
    case Int:
	return (double)d->value.i;
    case Bool:
	return (double)d->value.b;
    case UInt:
	return (double)d->value.u;
    default:
	return 0.0;
    }
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant as a QValueList<QVariant> if the variant has
    type() List or StringList; otherwise returns an empty list.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QValueList<QVariant> list = myVariant.toList();
    QValueList<QVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa asList()
*/
const QValueList<QVariant> QVariant::toList() const
{
    if ( d->typ == List )
	return *((QValueList<QVariant>*)d->value.ptr);
#ifndef QT_NO_STRINGLIST
    if ( d->typ == StringList ) {
	QValueList<QVariant> lst;
	QStringList::ConstIterator it = stringListBegin();
	QStringList::ConstIterator end = stringListEnd();
	for( ; it != end; ++it )
	    lst.append( QVariant( *it ) );
	return lst;
    }
#endif //QT_NO_STRINGLIST
    return QValueList<QVariant>();
}
#endif

/*!
    Returns the variant as a QSizePolicy if the variant has type()
    SizePolicy; otherwise returns an undefined (but legal) size
    policy.
*/

QSizePolicy QVariant::toSizePolicy() const
{
    if ( d->typ == SizePolicy )
	return *((QSizePolicy*)d->value.ptr);

    return QSizePolicy();
}


#define Q_VARIANT_AS( f ) Q##f& QVariant::as##f() { \
   if ( d->typ != f ) *this = QVariant( to##f() ); else detach(); return *((Q##f*)d->value.ptr);}

Q_VARIANT_AS(String)
Q_VARIANT_AS(CString)
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
Q_VARIANT_AS(ColorGroup)
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
    \fn QColorGroup& QVariant::asColorGroup()

    Tries to convert the variant to hold a QColorGroup value. If that
    is not possible the variant is set to a color group of all black
    colors.

    Returns a reference to the stored color group.

    \sa toColorGroup()
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
    detach();
    if ( d->typ != Int ) {
	int i = toInt();
	d->clear();
 	d->value.i = i;
	d->typ = Int;
    }
    return d->value.i;
}

/*!
    Returns the variant's value as unsigned int reference.
*/
uint& QVariant::asUInt()
{
    detach();
    if ( d->typ != UInt ) {
	uint u = toUInt();
	d->clear();
	d->value.u = u;
	d->typ = UInt;
    }
    return d->value.u;
}

/*!
    Returns the variant's value as bool reference.
*/
bool& QVariant::asBool()
{
    detach();
    if ( d->typ != Bool ) {
	bool b = toBool();
	d->clear();
	d->value.b = b;
	d->typ = Bool;
    }
    return d->value.b;
}

/*!
    Returns the variant's value as double reference.
*/
double& QVariant::asDouble()
{
    if ( d->typ != Double ) {
	double dbl = toDouble();
	d->clear();
	d->value.d = dbl;
	d->typ = Double;
    }
    return d->value.d;
}

#ifndef QT_NO_TEMPLATE_VARIANT
/*!
    Returns the variant's value as variant list reference.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QValueList<QVariant> list = myVariant.asList();
    QValueList<QVariant>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QValueList<QVariant>& QVariant::asList()
{
    if ( d->typ != List )
	*this = QVariant( toList() );
    return *((QValueList<QVariant>*)d->value.ptr);
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
    if ( d->typ != Map )
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
    \row \i Bool \i Double, Int, UInt
    \row \i Color \i String
    \row \i Date \i String, DateTime
    \row \i DateTime \i String, Date, Time
    \row \i Double \i String, Int, Bool, UInt
    \row \i Font \i String
    \row \i Int \i String, Double, Bool, UInt
    \row \i List \i StringList (if the list contains strings or
    something that can be cast to a string)
    \row \i String \i CString, Int, Uint, Double, Date, Time,
    DateTime, KeySequence, Font, Color
    \row \i CString \i String
    \row \i StringList \i List
    \row \i Time \i String
    \row \i UInt \i String, Double, Bool, Int
    \endtable
*/
bool QVariant::canCast( Type t ) const
{
    if ( d->typ == t )
	return TRUE;

    switch ( t ) {
    case Bool:
	return d->typ == Double || d->typ == Int || d->typ == UInt;
    case Int:
	return d->typ == String || d->typ == Double || d->typ == Bool || d->typ == UInt  || d->typ == KeySequence;
    case UInt:
	return d->typ == String || d->typ == Double || d->typ == Bool || d->typ == Int;
    case Double:
	return d->typ == String || d->typ == Int || d->typ == Bool || d->typ == UInt;
    case CString:
	return d->typ == String;
    case String:
	return d->typ == CString || d->typ == Int || d->typ == UInt || d->typ == Double || d->typ == Date || d->typ == Time || d->typ == DateTime || d->typ == KeySequence || d->typ == Font || d->typ == Color;
    case Date:
	return d->typ == String || d->typ == DateTime;
    case Time:
	return d->typ == String || d->typ == DateTime;
    case DateTime:
	return d->typ == String || d->typ == Date;
    case KeySequence:
	return d->typ == String || d->typ == Int;
    case Font:
	return d->typ == String;
    case Color:
	return d->typ == String;
#ifndef QT_NO_STRINGLIST
    case List:
	return d->typ == StringList;
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
    case StringList:
	if ( d->typ == List ) {
	    QValueList<QVariant>::ConstIterator it = listBegin();
	    QValueList<QVariant>::ConstIterator end = listEnd();
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
    case QVariant::ColorGroup:
	asColorGroup();
	break;
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
    case QVariant::CString:
	asCString();
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
    switch( d->typ ) {
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
	for ( ; it != v.toMap().end(); ++it ) {
	    if ( *it != *it2 )
		return FALSE;
	}
	return TRUE;
    }
#endif
    case String:
	return v.toString() == toString();
    case CString:
	return v.toCString() == toCString();
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
    case ColorGroup:
	return v.toColorGroup() == toColorGroup();
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
	d->typ = typ;
	d->value.ptr = ptr;
	d->is_null = FALSE;
	if ( deepCopy ) {
	    QVariant::Private* p = new Private( d );
	    d->typ = Invalid;
	    delete d;
	    d = p;
	}
    }

    if ( !deepCopy )
	return d->value.ptr;
    QVariant::Private* p = new Private( d );
    void *ret = (void*)p->value.ptr;
    p->typ = Invalid;
    delete p;
    return ret;
}

/*!
  Returns TRUE if this is a NULL variant, FALSE otherwise.
*/
bool QVariant::isNull() const
{
    switch( d->typ )
	{
	case QVariant::Bitmap:
	    return ((QBitmap*) d->value.ptr)->isNull();
    	    break;
	case QVariant::Region:
	    return ((QRegion*) d->value.ptr)->isNull();
	    break;
	case QVariant::PointArray:
	    return ((QPointArray*) d->value.ptr)->isNull();
	    break;
	case QVariant::String:
	    return ((QString*) d->value.ptr)->isNull();
	    break;
	case QVariant::CString:
	    return ((QCString*) d->value.ptr)->isNull();
	    break;
	case QVariant::Pixmap:
	    return ((QPixmap*) d->value.ptr)->isNull();
	    break;
	case QVariant::Image:
	    return ((QImage*) d->value.ptr)->isNull();
	    break;
	case QVariant::Point:
	    return ((QPoint*) d->value.ptr)->isNull();
	    break;
	case QVariant::Rect:
	    return ((QRect*) d->value.ptr)->isNull();
	    break;
	case QVariant::Size:
	    return ((QSize*) d->value.ptr)->isNull();
	    break;
#ifndef QT_NO_ICONSET
	case QVariant::IconSet:
	    return ((QIconSet*) d->value.ptr)->isNull();
	    break;
#endif
	case QVariant::Date:
	    return ((QDate*) d->value.ptr)->isNull();
	    break;
	case QVariant::Time:
	    return ((QTime*) d->value.ptr)->isNull();
	    break;
	case QVariant::DateTime:
	    return ((QDateTime*) d->value.ptr)->isNull();
	    break;
	case QVariant::ByteArray:
	    return ((QByteArray*) d->value.ptr)->isNull();
	    break;
	case QVariant::BitArray:
	    return ((QBitArray*) d->value.ptr)->isNull();
	    break;
	case QVariant::Cursor:
#ifndef QT_NO_STRINGLIST
	case QVariant::StringList:
#endif //QT_NO_STRINGLIST
	case QVariant::Font:
	case QVariant::Brush:
	case QVariant::Color:
#ifndef QT_NO_PALETTE
	case QVariant::Palette:
	case QVariant::ColorGroup:
#endif
#ifndef QT_NO_TEMPLATE_VARIANT
	case QVariant::Map:
	case QVariant::List:
#endif
	case QVariant::SizePolicy:
#ifndef QT_NO_ACCEL
	case QVariant::KeySequence:
#endif
	case QVariant::Pen:
	case QVariant::Invalid:
	case QVariant::Int:
	case QVariant::UInt:
	case QVariant::Bool:
	case QVariant::Double:
	    break;
	}
    return d->is_null;
}
#endif //QT_NO_VARIANT
