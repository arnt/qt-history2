/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qvariant.cpp#6 $
**
** Implementation of QVariant class
**
** Created : 990414
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qvariant.h"

#include "qstring.h"
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
#include "qshared.h"

// Uncomment to test for memory leaks or to run qt/test/qvariant/main.cpp
// #define QVARIANT_DEBUG

#ifdef QVARIANT_DEBUG
int qv_count = 0;
int get_qv_count() { return qv_count; }
#endif

QVariantPrivate::QVariantPrivate()
{
#ifdef QVARIANT_DEBUG
    qv_count++;
#endif
    typ = QVariant::Invalid;
    custom_type = 0;
}

QVariantPrivate::QVariantPrivate( QVariantPrivate* d )
{
#ifdef QVARIANT_DEBUG
    qv_count++;
#endif
    custom_type = 0;
	
    switch( d->typ )
	{
	case QVariant::Invalid:
	    break;
	case QVariant::Custom:
	    custom_type = d->custom_type;
	    value.ptr = d->custom_type->copy( d->value.ptr );
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
	    // PointArray is explicit shared
	    value.ptr = new QPointArray( *((QPointArray*)d->value.ptr) );
	    break;
	case QVariant::String:
	    value.ptr = new QString( *((QString*)d->value.ptr) );
	    break;
	case QVariant::CString:
	    // QCString is explicit shared
	    value.ptr = new QCString( *((QCString*)d->value.ptr) );
	    break;
	case QVariant::StringList:
	    value.ptr = new QStringList( *((QStringList*)d->value.ptr) );
	    break;
	case QVariant::Map:
	    value.ptr = new QMap<QString,QVariant>( *((QMap<QString,QVariant>*)d->value.ptr) );
	    break;
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
	case QVariant::Palette:
	    // QPalette is explicit shared
	    value.ptr = new QPalette( *((QPalette*)d->value.ptr) );
	    break;
	case QVariant::ColorGroup:
	    value.ptr = new QColorGroup( *((QColorGroup*)d->value.ptr) );
	    break;
	case QVariant::IconSet:
	    value.ptr = new QIconSet( *((QIconSet*)d->value.ptr) );
	    break;
	case QVariant::List:
	    value.ptr = new QValueList<QVariant>( *((QValueList<QVariant>*)d->value.ptr) );
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
	default:
	    ASSERT( 0 );
	}

    typ = d->typ;
}

QVariantPrivate::~QVariantPrivate()
{
#ifdef QVARIANT_DEBUG
    qv_count--;
#endif
    clear();
}

void QVariantPrivate::clear()
{
    switch( typ )
	{
	case QVariant::Custom:
	    custom_type->destroy( value.ptr );
	    custom_type = 0;
	    break;
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
	case QVariant::Map:
	    delete (QMap<QString,QVariant>*)value.ptr;
	    break;
	case QVariant::StringList:
	    delete (QStringList*)value.ptr;
	    break;
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
	case QVariant::Palette:
	    delete (QPalette*)value.ptr;
	    break;
	case QVariant::ColorGroup:
	    delete (QColorGroup*)value.ptr;
	    break;
	case QVariant::IconSet:
	    delete (QIconSet*)value.ptr;
	    break;
	case QVariant::List:
	    delete (QValueList<QVariant>*)value.ptr;
	    break;
	case QVariant::Invalid:
	case QVariant::Int:
	case QVariant::UInt:
	case QVariant::Bool:
	case QVariant::Double:
	    break;
	default:
	    ASSERT(0);
	}

    typ = QVariant::Invalid;
}

// NOT REVISED
/*!
  \class QVariant qvariant.h
  \brief Acts like a union for the most common Qt data types.

  \ingroup objectmodel
  \ingroup misc

  C++ forbids unions from including classes that
  have constructors and destructors since the compiler and the runtime
  library cannot determine which constructor or destructor to call
  when an object goes in and out of scope.

  To overcome this, a QVariant can be used to store the most common Qt
  and C++ data types. Like a union it can hold only one value of one
  type at any one time.

  For each QVariant::Type that the variant can hold, there is a constructor
  to create a QVariant from a value of the type, and two access methods to
  retrieve the value. The access methods do automatic conversion. Which conversion
  is possible can be queried by calling canCast().
  
  The methods named toT() are const. They return a copy of the stored data type
  or do conversion if needed. The conversion is only done for the returned value
  but the variant itself does not change. Please notice that some Qt data types
  such as QImage, QBrush, QPointArray, QRegion, QCString and QPalette are explicit
  shared. The toT() methods only return a shallow copy. That means you should make 
  a deep copy of the returned values before modifying them.
  
  The methods named asT() are not const. They act like the toT() methods except for
  two very important points. asT() returns a reference to the stored value instead of
  returning a copy and the casting affects the variant itself. That means if you store
  for example an interger in your variant and call asDouble() then the variant is converted
  to contain a double now and a reference to this double is returned.
  
  Here is some example code to demonstrate the usage of QVariant:
  
  \code
    QDataStream out(...);
    QVariant v(123);          // The variant now contains an int
    int x = v.toInt();        // x = 123
    out << v;                 // Writes a type tag an int to out
    v = QVariant("hello");    // The variant now contains a QCString
    v = QVariant(tr("hello"));// The variant now contains a QString
    int y = v.toInt();        // x = 0, since v is not an int
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
    v.asStringList().append( "Hallo" );
  \endcode

  You can even have a QValueList<QVariant> stored in the variant - giving
  arbitrarily complex data values with lists of variants,
  some of which are strings while others
  are integers and other still are lists of lists of lists of variants.
  Of course, you'll need to be careful with your encapsulations or else
  the typelessness will make your code look like spam, spam, spam,
  baked beans and spam.

  You can find the type of a variant with type(). There is a special type,
  Invalid, which can be used for special cases. The isValid() function
  tests for this type.
*/

/*! \enum QVariant::Type

  This enum type defines the types of variable that a QVariant can
  contain.  The supported enum values and the associated types are: <ul>

  <li> \c Invalid - no type
  <li> \c List - a QValueList<QVariant>
  <li> \c Map - a QMap<QString,QVariant>
  <li> \c String - a QString
  <li> \c StringList - a QStringList
  <li> \c Font - a QFont
  <li> \c Pixmap - a QPixmap
  <li> \c Brush - a QBrush
  <li> \c Rect - a QRect
  <li> \c Size - a QSize
  <li> \c Color - a QColor
  <li> \c Palette - a QPalette
  <li> \c ColorGroup - a QColorGroup
  <li> \c IconSet - a QIconSet
  <li> \c Point - a QPoint
  <li> \c Image - a QImage
  <li> \c Int - an int
  <li> \c UInt - an unsigned int
  <li> \c Bool - a bool
  <li> \c Double - a doublea
  <li> \c CString - a QCString
  <li> \c PointArray - a QPointArray
  <li> \c Region - a QRegion
  <li> \c Bitmap - a QBitmap
  <li> \c Cursor - a QCursor
  
  </ul> Note that Qt's idea of bool depends on the compiler.
  qglobal.h has the system-dependent definition of bool.
*/

/*!
  Constructs an invalid variant.
*/
QVariant::QVariant()
{
    d = new QVariantPrivate;
}

/*!
  Destructs the QVariant and the contained object.

  Note that subclasses that re-implement clear() should reimplement
  the destructor to call clear().  This destructor calls clear(), but
  since it is the destructor, QVariant::clear() is called rather than
  any subclass.
*/
QVariant::~QVariant()
{
    if ( d->deref() ) delete d;
}

/*!
  Constructs a copy of the variant passed as argument to this
  constructor. Usually this is a deep copy. But if the stored data
  type is explicit shared then only a shallow copy is made.
  
  The variant just resembles the copy behaviour of the data type
  it contains.
*/
QVariant::QVariant( const QVariant& p )
{
    d = new QVariantPrivate;
    *this = p;
}

/*!
  Reads the variant from the data stream.
*/
QVariant::QVariant( QDataStream& s )
{
    d = new QVariantPrivate;
    s >> *this;
}

/*!
  Constructs a new variant with a string value.
*/
QVariant::QVariant( const QString& val )
{
    d = new QVariantPrivate;
    d->typ = String;
    d->value.ptr = new QString( val );
}

/*!
  Constructs a new variant with a c-string value. If you want to
  modify the QCString you passed to this constructor afterwards,
  then pass a deep copy by calling QCString::copy().
*/
QVariant::QVariant( const QCString& val )
{
    d = new QVariantPrivate;
    d->typ = CString;
    d->value.ptr = new QCString( val );
}

/*!
  Constructs a new variant with a c-string value. The variant
  creates a deep copy of the string. Passing a null string will
  result in an invalid variant.
*/
QVariant::QVariant( const char* val )
{
    d = new QVariantPrivate;
    if ( val == 0 )
	return;
    d->typ = CString;
    d->value.ptr = new QCString( val );
}

/*!
  Constructs a new variant with a string list value.
*/
QVariant::QVariant( const QStringList& val )
{
    d = new QVariantPrivate;
    d->typ = StringList;
    d->value.ptr = new QStringList( val );
}

/*!
  Constructs a new variant with a map of QVariants.
*/
QVariant::QVariant( const QMap<QString,QVariant>& val )
{
    d = new QVariantPrivate;
    d->typ = Map;
    d->value.ptr = new QMap<QString,QVariant>( val );
}

/*!
  Constructs a new variant with a font value.
*/
QVariant::QVariant( const QFont& val )
{
    d = new QVariantPrivate;
    d->typ = Font;
    d->value.ptr = new QFont( val );
}

/*!
  Constructs a new variant with a pixmap value.
*/
QVariant::QVariant( const QPixmap& val )
{
    d = new QVariantPrivate;
    d->typ = Pixmap;
    d->value.ptr = new QPixmap( val );
}


/*!
  Constructs a new variant with an image value. Since QImage is
  explicit shared you may consider to pass a deep copy to
  the variant by calling QImage::copy().
*/
QVariant::QVariant( const QImage& val )
{
    d = new QVariantPrivate;
    d->typ = Image;
    d->value.ptr = new QImage( val );
}

/*!
  Constructs a new variant with a brush value. The variant creates only
  a shallow copy of the brush since QBrush is explicit shared.
*/
QVariant::QVariant( const QBrush& val )
{
    d = new QVariantPrivate;
    d->typ = Brush;
    d->value.ptr = new QBrush( val );
    // ### Force a deep copy
    // ((QBrush*)d->value.ptr)->setColor( ((QBrush*)d->value.ptr)->color() );
}

/*!
  Constructs a new variant with a point value.
*/
QVariant::QVariant( const QPoint& val )
{
    d = new QVariantPrivate;
    d->typ = Point;
    d->value.ptr = new QPoint( val );
}

/*!
  Constructs a new variant with a rect value.
*/
QVariant::QVariant( const QRect& val )
{
    d = new QVariantPrivate;
    d->typ = Rect;
    d->value.ptr = new QRect( val );
}

/*!
  Constructs a new variant with a size value.
*/
QVariant::QVariant( const QSize& val )
{
    d = new QVariantPrivate;
    d->typ = Size;
    d->value.ptr = new QSize( val );
}

/*!
  Constructs a new variant with a color value.
*/
QVariant::QVariant( const QColor& val )
{
    d = new QVariantPrivate;
    d->typ = Color;
    d->value.ptr = new QColor( val );
}

/*!
  Constructs a new variant with a color palette value. Since QPalette
  is explicit shared you may consider to pass a deep copy to the
  variant by calling QPalette::copy().
*/
QVariant::QVariant( const QPalette& val )
{
    d = new QVariantPrivate;
    d->typ = Palette;
    d->value.ptr = new QPalette( val );
}

/*!
  Constructs a new variant with a color group value.
*/
QVariant::QVariant( const QColorGroup& val )
{
    d = new QVariantPrivate;
    d->typ = ColorGroup;
    d->value.ptr = new QColorGroup( val );
}

/*!
  Constructs a new variant with an iconset.
*/
QVariant::QVariant( const QIconSet& val )
{
    d = new QVariantPrivate;
    d->typ = IconSet;
    d->value.ptr = new QIconSet( val );
}

/*!
  Constructs a new variant with a region. The variant creates only
  a shallow copy of the brush since QBrush is explicit shared.
*/
QVariant::QVariant( const QRegion& val )
{
    d = new QVariantPrivate;
    d->typ = Region;
    // ## Force a detach
    d->value.ptr = new QRegion( val );
    ((QRegion*)d->value.ptr)->translate( 0, 0 );
}

/*!
  Constructs a new variant with a bitmap.
*/
QVariant::QVariant( const QBitmap& val )
{
    d = new QVariantPrivate;
    d->typ = Bitmap;
    d->value.ptr = new QBitmap( val );
}

/*!
  Constructs a new variant with a cursor..
*/
QVariant::QVariant( const QCursor& val )
{
    d = new QVariantPrivate;
    d->typ = Cursor;
    d->value.ptr = new QCursor( val );
}

/*!
  Constructs a new variant with an array of points. Since QPointArray
  is explicit shared you may consider to pass a deep copy to the variant
  by calling QPointArray::copy().
*/
QVariant::QVariant( const QPointArray& val )
{
    d = new QVariantPrivate;
    d->typ = PointArray;
    d->value.ptr = new QPointArray( val );
}

/*!
  Constructs a new variant with an integer value.
*/
QVariant::QVariant( int val )
{
    d = new QVariantPrivate;
    d->typ = Int;
    d->value.i = val;
}

/*!
  Constructs a new variant with an unsigned integer value.
*/
QVariant::QVariant( uint val )
{
    d = new QVariantPrivate;
    d->typ = UInt;
    d->value.u = val;
}

/*!
  Constructs a new variant with a boolean value.
*/
QVariant::QVariant( bool val )
{
    d = new QVariantPrivate;
    d->typ = Bool;
    d->value.b = val;
}


/*!
  Constructs a new variant with a floating point value.
*/
QVariant::QVariant( double val )
{
    d = new QVariantPrivate;
    d->typ = Double;
    d->value.d = val;
}

/*!
  Constructs a new variant with a list value.
*/
QVariant::QVariant( const QValueList<QVariant>& val )
{
    d = new QVariantPrivate;
    d->typ = List;
    d->value.ptr = new QValueList<QVariant>( val );
}	

/*!
  Constructs a new variant with a custom value. The variant creates
  a copy of the custom value with the help of the passes \a type.
  The QVariantTypeBase object must be alive as long as the variant
  holds this custom value since the variant stores a pointer to
  the \a type.
 */
QVariant::QVariant( void* custom, const QVariantTypeBase* type )
{
    d = new QVariantPrivate;
    if ( !custom || !type )
	return;

    d->custom_type = type;
    d->typ = Custom;
    d->value.ptr = d->custom_type->copy( custom );
}

/*!
  Constructs a new variant with a custom value. This is a convenience
  method that uses the QVariantValue template. If you have to deal with
  variants and custom data types a lot then you might consider to use this
  template since it gives you type checking and makes the source code nicer.
*/
QVariant::QVariant( const QVariantValueBase& v )
{
    d = new QVariantPrivate;
    d->typ = Custom;
    d->custom_type = v.type();
    d->value.ptr = d->custom_type->copy( v.value() );
}

/*!
  Assigns the value of some \a other variant to this variant.
  This is a deep copy.
*/
QVariant& QVariant::operator= ( const QVariant& variant )
{
    if ( d->deref() ) delete d;

    QVariant& other = (QVariant&)variant;
    d = other.d;
    d->ref();

    return *this;
}

/*!
  Internal
*/
void QVariant::detach()
{
    if ( d->count == 1 )
	return;

    d->deref();
    d = new QVariantPrivate( d );
}

/*!
  Returns the name of the type stored in the variant.
  The returned strings describe the C++ datatype used to store the
  data, for example "QFont", "QString" or "QValueList<QVariant>".
  An Invalid variant returns 0.
*/
const char* QVariant::typeName() const
{
    if ( d->typ == Custom )
	return d->custom_type->typeName();
    return typeToName( d->typ );
}

/*!
  De-allocate any used memory,
  based on the type, producing an Invalid variant.
*/
void QVariant::clear()
{
    if ( d->count > 1 )
    {
	d->deref();
	d = new QVariantPrivate;
	return;
    }

    d->clear();
}

/*
  Attention!
  For dependency reasons, this table is duplicated in moc.y. If you
  change one, change both.
*/
static const int ntypes = 26;
static const char* type_map[ntypes] =
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
    "PointArray",
    "Region",
    "Bitmap",
    "Cursor",
    "Custom"
};

/*!
  Converts the enum representation of the storage type to its
  string representation.
*/
const char* QVariant::typeToName( Type typ )
{
    if ( typ >= ntypes )
	return 0;
    return type_map[typ];
}

/*!
  Converts the string representation of the storage type to
  its enum representation.
*/
QVariant::Type QVariant::nameToType( const char* name )
{
    for ( int i = 0; i < ntypes; i++ ) {
	if ( !qstrcmp( type_map[i], name ) )
	    return (Type) i;
    }
    if ( QVariantTypeBase::type( name ) )
	return Custom;
    return Invalid;
}

/*!
  Internal function for loading a variant. Use the stream operators
  instead.
*/
void QVariant::load( QDataStream& s )
{
    Q_UINT32 u;
    s >> u;
    Type t = (Type)u;

    switch( t )
	{
	case Invalid:
	    d->typ = t;
	    break;
	case Custom:
	    {
		QCString t;
		s >> t;
		d->custom_type = QVariantTypeBase::type( t );
		if ( !d->custom_type )
	        {
		    d->typ = Invalid;
		    return;
		}
		d->value.ptr = d->custom_type->create();
		d->custom_type->load( d->value.ptr, s );
	    }
	    break;
	case Map:
	    { QMap<QString,QVariant>* x = new QMap<QString,QVariant>; s >> *x; d->value.ptr = x; }
	    break;
	case List:
	    { QValueList<QVariant>* x = new QValueList<QVariant>; s >> *x; d->value.ptr = x; }
	    break;
	case Cursor:
	    { QCursor* x = new QCursor; s >> *x; d->value.ptr = x; }
	    break;
	case Bitmap:
	    { QBitmap* x = new QBitmap; s >> *x; d->value.ptr = x; }
	    break;
	case Region:
	    { QRegion* x = new QRegion; s >> *x; d->value.ptr = x; }
	    break;
	case PointArray:
	    { QPointArray* x = new QPointArray; s >> *x; d->value.ptr = x; }
	    break;
	case String:
	    { QString* x = new QString; s >> *x; d->value.ptr = x; }
	    break;
	case CString:
	    { QCString* x = new QCString; s >> *x; d->value.ptr = x; }
	    break;
	case StringList:
	    { QStringList* x = new QStringList; s >> *x; d->value.ptr = x; }
	    break;
	case Font:
	    { QFont* x = new QFont; s >> *x; d->value.ptr = x; }
	    break;
	case Pixmap:
	    { QPixmap* x = new QPixmap; s >> *x; d->value.ptr = x; }
	    break;
	case Image:
	    { QImage* x = new QImage; s >> *x; d->value.ptr = x; }
	    break;
	case Brush:
	    { QBrush* x = new QBrush; s >> *x; d->value.ptr = x; }
	    break;
	case Rect:
	    { QRect* x = new QRect; s >> *x; d->value.ptr = x; }
	    break;
	case Point:
	    { QPoint* x = new QPoint; s >> *x; d->value.ptr = x; }
	    break;
	case Size:
	    { QSize* x = new QSize; s >> *x; d->value.ptr = x; }
	    break;
	case Color:
	    { QColor* x = new QColor; s >> *x; d->value.ptr = x; }
	    break;
	case Palette:
	    { QPalette* x = new QPalette; s >> *x; d->value.ptr = x; }
	    break;
	case ColorGroup:
	    { QColorGroup* x = new QColorGroup; s >> *x; d->value.ptr = x; }
	    break;
	case IconSet:
	    { QPixmap* x = new QPixmap; s >> *x; d->value.ptr = x; }
	    break;
	case Int:
	    { int x; s >> x; d->value.i = x; }
	    break;
	case UInt:
	    { uint x; s >> x; d->value.u = x; }
	    break;
	case Bool:
	    { Q_INT8 x; s >> x; d->value.b = x; }
	    break;
	case Double:
	    { double x; s >> x; d->value.d = x; }
	    break;
	default:
	    ASSERT(0);
	}

    d->typ = t;
}

/*!
  Internal function for saving a variant. Use the stream operators
  instead.
*/
void QVariant::save( QDataStream& s ) const
{
    s << (Q_UINT32)type();

    switch( d->typ )
	{
	case Custom:
	    s << d->custom_type->typeName();
	    d->custom_type->save( d->value.ptr, s );
	    break;
	case Cursor:
	    s << *((QCursor*)d->value.ptr);
	    break;
	case Bitmap:
	    s << *((QBitmap*)d->value.ptr);
	    break;
	case PointArray:
	    s << *((QPointArray*)d->value.ptr);
	    break;
	case Region:
	    s << *((QRegion*)d->value.ptr);
	    break;
	case List:
	    s << *((QValueList<QVariant>*)d->value.ptr);
	    break;
	case Map:
	    s << *((QMap<QString,QVariant>*)d->value.ptr);
	    break;
	case String:
	    s << *((QString*)d->value.ptr);
	    break;
	case CString:
	    s << *((QCString*)d->value.ptr);
	    break;
	case StringList:
	    s << *((QStringList*)d->value.ptr);
	    break;
	case Font:
	    s << *((QFont*)d->value.ptr);
	    break;
	case Pixmap:
	    s << *((QPixmap*)d->value.ptr);
	    break;
	case Image:
	    s << *((QImage*)d->value.ptr);
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
	case Palette:
	    s << *((QPalette*)d->value.ptr);
	    break;
	case ColorGroup:
	    s << *((QColorGroup*)d->value.ptr);
	    break;
	case IconSet:
	    s << ((QIconSet*)d->value.ptr)->pixmap(); //### add stream operator to iconset #ME
	    break;
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
	case Invalid: // fall through
	default:
	    s << QString();
	    break;
	}
}

/*!
  Reads a variant \a p from the stream \a s.
*/
QDataStream& operator>> ( QDataStream& s, QVariant& p )
{
    p.load( s );
    return s;
}

/*!
  Writes a variant \a p to the stream \a s.
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

/*! \fn Type QVariant::type() const
  Returns the stoarge type of the value stored in the
  variant currently. Usually you may want to test with
  canCast() wether the variant can deliver the data type you
  are interested in.
*/

/*! \fn bool QVariant::isValid() const
  Returns TRUE if the storage type of this variant is not QVariant::Invalid.
*/

/*! \fn QValueListConstIterator<QString> QVariant::stringListBegin() const
  If the type of the variant is StringList, then an iterator to the first string
  in the list is returned. Otherwise a null iterator is returned.
 */

/*! \fn QValueListConstIterator<QString> QVariant::stringListEnd() const
  If the type of the variant is StringList, then the end iterator
  of the list is returned. Otherwise a null iterator is returned.
 */

/*! \fn QValueListConstIterator<QVariant> QVariant::listBegin() const
  If the type of the variant is List, then an iterator to the first element
  in the list is returned. Otherwise a null iterator is returned.
 */

/*! \fn QValueListConstIterator<QVariant> QVariant::listEnd() const
  If the type of the variant is List, then the end iterator
  of the list is returned. Otherwise a null iterator is returned.
 */

/*! \fn QMapConstIterator<QString,QVariant> QVariant::mapBegin() const
  If the type of the variant is Map, then an iterator to the first entry
  in the map is returned. Otherwise a null iterator is returned.
 */

/*! \fn QMapConstIterator<QString,QVariant> QVariant::mapEnd() const
  If the type of the variant is Map, then the end iterator
  of the map is returned. Otherwise a null iterator is returned.
 */

/*! \fn QMapConstIterator<QString,QVariant> QVariant::mapFind( const QString& key ) const
  If the type of the variant is Map, then an iterator to the entry with \a key is returned.
  Otherwise, or if no such entry exists, a null iterator is returned.
 */

/*!
  Returns the variant as a QString if the variant has type()
  String or CString, or QString::null otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asString()
*/
const QString QVariant::toString() const
{
    if ( d->typ == CString )
	return QString::fromLatin1( toCString() );
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, String ).toString();
    if ( d->typ != String )
	return QString::null;

    return *((QString*)d->value.ptr);
}

/*!
  Returns the variant as a QCString if the variant has type()
  CString, or a 0 otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asCString()
*/
const QCString QVariant::toCString() const
{
    if ( d->typ == CString )
	return *((QCString*)d->value.ptr);
    if ( d->typ == String )
	return ((QString*)d->value.ptr)->latin1();
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, CString ).toCString();

    return 0;
}

/*!
  Returns the variant as a QStringList if the variant has type()
  StringList or List, or an empty list otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asStringList()
*/
const QStringList QVariant::toStringList() const
{
    if ( d->typ == StringList )
	return *((QStringList*)d->value.ptr);
    if ( d->typ == List )
    {
	QStringList lst;
	QValueList<QVariant>::ConstIterator it = listBegin();
	QValueList<QVariant>::ConstIterator end = listEnd();
	for( ; it != end; ++it )
	    lst.append( (*it).toString() );
	return lst;
    }
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, StringList ).toStringList();

    return QStringList();
}

/*!
  \fn QMap<QString, QVariant> QVariant::toMap () const

  Returns the variant as a QMap<QString,QVariant> if the variant has type()
  Map, or an empty map otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asMap()
*/
const QMap<QString,QVariant> QVariant::toMap() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Map ).toMap();
    if ( d->typ != Map )
	return QMap<QString,QVariant>();

    return *((QMap<QString,QVariant>*)d->value.ptr);
}

/*!
  Returns the variant as a QFont if the variant has type()
  Font, or the default font otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asFont()
*/
const QFont QVariant::toFont() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Font ).toFont();
    if ( d->typ != Font )
	return QFont();

    return *((QFont*)d->value.ptr);
}

/*!
  Returns the variant as a QPixmap if the variant has type()
  Pixmap, or a null pixmap otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asPixmap()
*/
const QPixmap QVariant::toPixmap() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Pixmap ).toPixmap();
    if ( d->typ != Pixmap )
	return QPixmap();

    return *((QPixmap*)d->value.ptr);
}

/*!
  Returns the variant as a QImage if the variant has type()
  Image, or a null image otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asImage()
*/
const QImage QVariant::toImage() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Image ).toImage();
    if ( d->typ != Image )
	return QImage();

    return *((QImage*)d->value.ptr);
}

/*!
  Returns the variant as a QBrush if the variant has type()
  Brush, or a default brush with black colors otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asBrush()
*/
const QBrush QVariant::toBrush() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Brush ).toBrush();
    if( d->typ != Brush )
	return QBrush();

    return *((QBrush*)d->value.ptr);
}

/*!
  Returns the variant as a QPoint if the variant has type()
  Point, or a the point (0,0) otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asPoint()
*/
const QPoint QVariant::toPoint() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Point ).toPoint();
    if ( d->typ != Point )
	return QPoint();

    return *((QPoint*)d->value.ptr);
}

/*!
  Returns the variant as a QRect if the variant has type()
  Rect, or an empty rectangle otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asRect()
*/
const QRect QVariant::toRect() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Rect ).toRect();
    if ( d->typ != Rect )
	return QRect();

    return *((QRect*)d->value.ptr);
}

/*!
  Returns the variant as a QSize if the variant has type()
  Size, or an invalid size otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asSize()
*/
const QSize QVariant::toSize() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Size ).toSize();
    if ( d->typ != Size )
	return QSize();

    return *((QSize*)d->value.ptr);
}

/*!
  Returns the variant as a QColor if the variant has type()
  Color, or an invalid color otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asColor()
*/
const QColor QVariant::toColor() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Color ).toColor();
    if ( d->typ != Color )
	return QColor();

    return *((QColor*)d->value.ptr);
}

/*!
  Returns the variant as a QPalette if the variant has type()
  Palette, or a completely black palette otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asPalette()
*/
const QPalette QVariant::toPalette() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Palette ).toPalette();
    if ( d->typ != Palette )
	return QPalette();

    return *((QPalette*)d->value.ptr);
}

/*!
  Returns the variant as a QColorGroup if the variant has type()
  ColorGroup, or a completely black color group otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asColorGroup()
*/
const QColorGroup QVariant::toColorGroup() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, ColorGroup ).toColorGroup();
    if ( d->typ != ColorGroup )
	return QColorGroup();

    return *((QColorGroup*)d->value.ptr);
}

/*!
  Returns the variant as a QIconSet if the variant has type()
  IconSet, or an icon set of null pixmaps otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asIconSet()
*/
const QIconSet QVariant::toIconSet() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, IconSet ).toIconSet();
    if ( d->typ != IconSet )
	return QIconSet();

    return *((QIconSet*)d->value.ptr);
}

/*!
  Returns the variant as a QPointArray if the variant has type()
  PointArray, or an empty QPointArray otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asPointArray()
*/
const QPointArray QVariant::toPointArray() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, PointArray ).toPointArray();
    if ( d->typ != PointArray )
	return QPointArray();

    return *((QPointArray*)d->value.ptr);
}

/*!
  Returns the variant as a QBitmap if the variant has type()
  Bitmap, or a null QBitmap otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asBitmap()
*/
const QBitmap QVariant::toBitmap() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Bitmap ).toBitmap();
    if ( d->typ != Bitmap )
	return QBitmap();

    return *((QBitmap*)d->value.ptr);
}

/*!
  Returns the variant as a QRegion if the variant has type()
  Region, or an empty QRegion otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asRegion()
*/
const QRegion QVariant::toRegion() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Region ).toRegion();
    if ( d->typ != Region )
	return QRegion();

    return *((QRegion*)d->value.ptr);
}

/*!
  Returns the variant as a QCursor if the variant has type()
  Cursor, or the default arrow cursor otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asCustom()
*/
const QCursor QVariant::toCursor() const
{
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Cursor ).toCursor();
    if ( d->typ != Cursor )
	return QCursor();

    return *((QCursor*)d->value.ptr);
}

/*!
  Returns the variant as an int if the variant has type()
  Int, UInt, Double or Bool, or 0 otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asInt()
*/
int QVariant::toInt() const
{
    if( d->typ == Int )
	return d->value.i;
    if( d->typ == UInt )
	return (int)d->value.u;
    if ( d->typ == Double )
	return (int)d->value.d;
    if ( d->typ == Bool )
	return (int)d->value.b;
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Int ).toInt();

    /* if ( d->typ == String )
	return ((QString*)d->value.ptr)->toInt();
    if ( d->typ == CString )
    return ((QCString*)d->value.ptr)->toInt(); */
    return 0;
}

/*!
  Returns the variant as an unsigned int if the variant has type()
  UInt, Int, Double or Bool, or 0 otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asUInt()
*/
uint QVariant::toUInt() const
{
    if( d->typ == Int )
	return d->value.i;
    if( d->typ == UInt )
	return (int)d->value.u;
    if ( d->typ == Double )
	return (int)d->value.d;
    if ( d->typ == Bool )
	return (int)d->value.b;
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, UInt ).toUInt();

    /* if ( d->typ == String )
	return ((QString*)d->value.ptr)->toInt();
    if ( d->typ == CString )
    return ((QCString*)d->value.ptr)->toInt(); */
    return 0;
}

/*!
  Returns the variant as a bool if the variant has type()
  Bool, or FALSE otherwise. The only exceptions to this rule are
  the types Int, UInt, Double. In this case TRUE is returned if the numerical
  value is not zero or FALSE otherwise.
  
  But if the type is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asBool()
*/
bool QVariant::toBool() const
{
    if ( d->typ == Bool )
	return d->value.b;
    if ( d->typ == Double )
	return d->value.d != 0.0;
    if ( d->typ == Int )
	return d->value.i != 0;
    if ( d->typ == UInt )
	return d->value.u != 0;
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Bool ).toBool();

    /* if ( d->typ == String )
	return *((QString*)d->value.ptr) == "true";
    if ( d->typ == CString )
    return *((QCString*)d->value.ptr) == "true"; */
    return FALSE;
}

/*!
  Returns the variant as a double if the variant has type()
  Double, Int, UInt or Bool, or 0.0 otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asDouble()
*/
double QVariant::toDouble() const
{
    if ( d->typ == Double )
	return d->value.d;
    if ( d->typ == Int )
	return (double)d->value.i;
    if ( d->typ == Bool )
	return (double)d->value.b;
    if ( d->typ == UInt )
	return (double)d->value.u;
    /* if ( d->typ == String )
	return ((QString*)d->value.ptr)->toDouble();
    if ( d->typ == CString )
    return ((QCString*)d->value.ptr)->toDouble(); */
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, Double ).toDouble();
    return 0.0;
}

/*!
  Returns the variant as a QValueList<QVariant> if the variant has type()
  List or StringList, or an empty list otherwise. But if the type
  is Custom then the associated QVariantTypeBase is asked to do the
  conversion if possible.
  
  \sa asList()
*/
const QValueList<QVariant> QVariant::toList() const
{
    if ( d->typ == List )
	return *((QValueList<QVariant>*)d->value.ptr);
    if ( d->typ == StringList ) {
	QValueList<QVariant> lst;
	QStringList::ConstIterator it = stringListBegin();
	QStringList::ConstIterator end = stringListEnd();
	for( ; it != end; ++it )
	    lst.append( QVariant( *it ) );
	return lst;
    }
    if ( d->typ == Custom )
	return d->custom_type->castTo( d->value.ptr, List ).toList();

    return QValueList<QVariant>();
}

/*!
  Returns a copy of a custom data type. If the variant currently
  stores a value of the requested \a type, then just a copy is returned.
  
  If a custom value of another type is stored, then \a type is first
  asked wether it can do the conversion. If that is not possible, then
  customType() is queried wether it can convert the stored value to \a type.
  
  If a basic value like String, List, Image etc. is stored in the variant
  then \a type is asked to do the conversion if possible.
  
  If no conversion was possible then 0 is returned.
  
  The caller takes over ownership of the returned copy.
  
  \sa asCustom() canCast()
*/
void* QVariant::toCustom( const QVariantTypeBase* type ) const
{
    if ( type == 0 )
	return 0;

    // Both the same custom type ?
    if ( d->typ == Custom && type == d->custom_type )
	return d->custom_type->copy( d->value.ptr );

    // Both custom types, but different ones ?
    if ( d->typ == Custom )
    {
	// Can the passes type do the conversion
	if ( type->canCastFrom( d->custom_type ) )
	    return type->castFrom( *this );
	// Can our type do the conversion ?
	if ( d->custom_type->canCastTo( type ) )
	    return d->custom_type->castTo( d->value.ptr, type );
	return 0;
    }

    // This is a basic type that is to be converted to
    // a custom type.
    return type->castFrom( *this );
}

/*!
  Returns the variant's custom value as void pointer or 0, if the
  variant is no custom type.
  
  \sa toCustom() canCast()
 */
const void* QVariant::asCustom() const
{
    if ( d->typ != Custom )
	return 0;
    return d->value.ptr;
}

/*!
  Returns the variant's custom value as void pointer or 0, if the
  variant is no custom type.
  
  \sa toCustom() canCast()
 */
void* QVariant::asCustom()
{
    if ( d->typ != Custom )
	return 0;
    return d->value.ptr;
}

/*!
  Tries to convert the Variant to \a type. If that succeeds then
  a pointer to the stored custom value is returned. The caller does
  NOT have ownership in contrast to toCustom().
  If the conversion is impossible then the variant does not change and
  0 is returned.
  
  The steps taken to make the conversion possible are the same as documented
  in toCustom().

  \sa toCustom() canCast()
 */
void* QVariant::asCustom( const QVariantTypeBase* type )
{
    if ( type == 0 )
	return 0;

    // Both the same custom type ?
    if ( d->typ == Custom && type == d->custom_type )
	return d->value.ptr;

    // Both custom types, but different ones ?
    if ( d->typ == Custom ) {
	// Can the passes type do the conversion
	if ( type->canCastFrom( d->custom_type ) ) {
	    void* ptr = type->castFrom( *this );
	    if ( !ptr )
		return 0;
	    clear();
	    d->custom_type = type;
	    d->value.ptr = ptr;
	    return ptr;
	}
	// Can our type do the conversion ?
	if ( d->custom_type->canCastTo( type ) ) {
	    void* ptr = d->custom_type->castTo( d->value.ptr, type );
	    if ( !ptr )
		return 0;
	    clear();
	    d->custom_type = type;
	    d->value.ptr = ptr;
	    return ptr;
	}
	return 0;
    }

    // This is a basic type that is to be converted to
    // a custom type.
    void* ptr = type->castFrom( *this );
    if ( !ptr )
	return 0;
    clear();
    d->custom_type = type;
    d->value.ptr = ptr;
    return ptr;
}

#define Q_VARIANT_AS( f ) Q##f& QVariant::as##f() { \
   if ( d->typ != ##f ) *this = QVariant( to##f() ); else detach(); return *((Q##f*)d->value.ptr);}

Q_VARIANT_AS(String)
Q_VARIANT_AS(CString)
Q_VARIANT_AS(StringList)
Q_VARIANT_AS(Font)
Q_VARIANT_AS(Pixmap)
Q_VARIANT_AS(Image)
Q_VARIANT_AS(Brush)
Q_VARIANT_AS(Point)
Q_VARIANT_AS(Rect)
Q_VARIANT_AS(Size)
Q_VARIANT_AS(Color)
Q_VARIANT_AS(Palette)
Q_VARIANT_AS(ColorGroup)
Q_VARIANT_AS(IconSet)
Q_VARIANT_AS(PointArray)
Q_VARIANT_AS(Bitmap)
Q_VARIANT_AS(Region)
Q_VARIANT_AS(Cursor)

/*! \fn QString& QVariant::asString()

  Tries to convert the variant to hold a string value. If that
  is not possible then the variant holds an empty string.
  
  Returns a reference to the stored string.
  
  \sa toString()
 */
    
/*! \fn QCString& QVariant::asCString()

  Tries to convert the variant to hold a string value. If that
  is not possible then the variant holds an empty string.
  
  Returns a reference to the stored string.
  
  \sa toCString()
 */
    
/*! \fn QStringList& QVariant::asStringList()

  Tries to convert the variant to hold a QStringList value. If that
  is not possible then the variant holds an empty string list.
  
  Returns a reference to the stored string list.
  
  \sa toStringList()
 */
    
/*! \fn QFont& QVariant::asFont()

  Tries to convert the variant to hold a QFont. If that
  is not possible then the variant holds a default font.
  
  Returns a reference to the stored font.
  
  \sa toFont()
 */
    
/*! \fn QPixmap& QVariant::asPixmap()

  Tries to convert the variant to hold a pixmap value. If that
  is not possible then the variant holds a null pixmap.
  
  Returns a reference to the stored pixmap.
  
  \sa toPixmap()
 */
    
/*! \fn QImage& QVariant::asImage()

  Tries to convert the variant to hold an image value. If that
  is not possible then the variant holds a null image.
  
  Returns a reference to the stored image.
  
  \sa toImage()
 */
    
/*! \fn QBrush& QVariant::asBrush()
  
  Tries to convert the variant to hold a brush value. If that
  is not possible then the variant holds a default black brush.
  
  Returns a reference to the stored brush.
  
  \sa toBrush()
 */
    
/*! \fn QPoint& QVariant::asPoint()

  Tries to convert the variant to hold a point value. If that
  is not possible then the variant holds a null point.
  
  Returns a reference to the stored point.
  
  \sa toPoint()


 */
    
/*! \fn QRect& QVariant::asRect()

  Tries to convert the variant to hold a rectangle value. If that
  is not possible then the variant holds an empty rectangle.
  
  Returns a reference to the stored rectangle.
  
  \sa toRect()
 */
    
/*! \fn QSize& QVariant::asSize()

  Tries to convert the variant to hold a QSize value. If that
  is not possible then the variant holds an invalid size.
  
  Returns a reference to the stored size.
  
  \sa toSize() QSize::isValid()

 */
    
/*! \fn QColor& QVariant::asColor()

  Tries to convert the variant to hold a QColor value. If that
  is not possible then the variant holds an invalid color.
  
  Returns a reference to the stored color.
  
  \sa toColor() QColor::isValid()
 */
    
/*! \fn QPalette& QVariant::asPalette()

  Tries to convert the variant to hold a QPalette value. If that
  is not possible then the variant holds a palette with black colors only.
  
  Returns a reference to the stored palette.
  
  \sa toString()
  
 */
    
/*! \fn QColorGroup& QVariant::asColorGroup()

  Tries to convert the variant to hold a QColorGroup value. If that
  is not possible then the variant holds a color group with all colors
  set to black.
  
  Returns a reference to the stored color group.
  
  \sa toColorGroup()

 */
    
/*! \fn QIconSet& QVariant::asIconSet()

  Tries to convert the variant to hold a QIconSet value. If that
  is not possible then the variant holds an empty iconset.
  
  Returns a reference to the stored iconset.
  
  \sa toIconSet()

 */
    
/*! \fn QPointArray& QVariant::asPointArray()

  Tries to convert the variant to hold a QPointArray value. If that
  is not possible then the variant holds an empty point array.
  
  Returns a reference to the stored point array.
  
  \sa toPointArray()

 */
    
/*! \fn QBitmap& QVariant::asBitmap()

  Tries to convert the variant to hold a bitmap value. If that
  is not possible then the variant holds a null bitmap.
  
  Returns a reference to the stored bitmap.
  
  \sa toBitmap()
 */
    
/*! \fn QRegion& QVariant::asRegion()

  Tries to convert the variant to hold a QRegion value. If that
  is not possible then the variant holds a null region.
  
  Returns a reference to the stored region.
  
  \sa toRegion()
 */
    
/*! \fn QCursor& QVariant::asCursor()

  Tries to convert the variant to hold a QCursor value. If that
  is not possible then the variant holds a default arrow cursor.
  
  Returns a reference to the stored cursor.
  
  \sa toCursor()

 */

/*!
  Returns the variant's value as int reference.
 */
int& QVariant::asInt()
{
    detach();
    if ( d->typ != Int )
    {
	d->value.i = toInt();
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
    if ( d->typ != UInt )
    {
	d->value.u = toUInt();
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
    if ( d->typ != Bool )
    {
	d->value.b = toBool();
	d->typ = Bool;
    }
    return d->value.b;
}

/*!
  Returns the variant's value as double reference.
 */
double& QVariant::asDouble()
{
    if ( d->typ != Double )
    {
	d->value.d = toDouble();
	d->typ = Double;
    }
    return d->value.d;
}

/*!
  Returns the variant's value as variant list reference.
 */
QValueList<QVariant>& QVariant::asList()
{
    if ( d->typ != List )
	*this = QVariant( toList() );
    return *((QValueList<QVariant>*)d->value.ptr);
}

/*!
  Returns the variant's value as variant map reference.
 */
QMap<QString,QVariant>& QVariant::asMap()
{
    if ( d->typ != Map )
	*this = QVariant( toMap() );
    return *((QMap<QString,QVariant>*)d->value.ptr);
}

/*!
  Returns TRUE if the current type of the variant can be casted to
  the requested type. The casting is done automatically when calling
  the toInt(), toBool(), ... or asInt(), asBool(), ... methods.

  The following casts are done automatically:
  <ul>
  <li> Bool -> Double, Int, UInt
  <li> Double -> Int, Bool, UInt
  <li> Int -> Double, Bool, UInt
  <li> UInt -> Double, Bool, Int
  <li> String -> CString
  <li> CString -> String
  <li> List -> StringList ( but only of the list only contains strings or
                            something that can be casted to a string ).
  <li> StringList -> List
  </ul>
  
  If the stored value is a custom type then customType() is asked wether it
  can do the requested conversion.
*/
bool QVariant::canCast( Type t ) const
{
    if ( d->typ == t )
	return TRUE;
    if ( t == Bool && ( d->typ == Double || d->typ == Int || d->typ == UInt ) )
	 return TRUE;
    if ( t == Int && ( d->typ == Double || d->typ == Bool || d->typ == UInt ) )
	return TRUE;
    if ( t == UInt && ( d->typ == Double || d->typ == Bool || d->typ == Int ) )
	return TRUE;
    if ( t == Double && ( d->typ == Int || d->typ == Bool || d->typ == UInt ) )
	return TRUE;
    if ( t == CString && d->typ == String )
	return TRUE;
    if ( t == String && d->typ == CString )
	return TRUE;
    if ( t == List && d->typ == StringList )
	return TRUE;
    if ( t == StringList && d->typ == List )   {
	QValueList<QVariant> vl = toList();
	QValueList<QVariant>::ConstIterator it = listBegin();
	QValueList<QVariant>::ConstIterator end = listEnd();
	for( ; it != end; ++it ) {
	    if ( !(*it).canCast( String ) )
		return FALSE;
	}
	return TRUE;
    }
    if ( d->typ == Custom )
	return d->custom_type->canCastTo( t );

    return FALSE;
}

/*!
  Returns TRUE if the current type of the variant can be casted to
  the requested custom QVariantType \a type.
  
  Casting is possible if one of the following steps succeeds
  <ul>
  <li> The stored value is a custom value of type \a type.
  <li> \a type can cast the stored custom value to the requested type.
  <li> customType() can cast the stored custom value to the requested type.
  <li> \a type can cast the stored value - which is a basic data type - to the requested type.
  </ul>
  
  Otherwise FALSE is returned.
*/
bool QVariant::canCast( const QVariantTypeBase* type ) const
{
    if ( type == 0 )
	return FALSE;

    if ( d->typ == Custom ) {
	if ( type == d->custom_type )
	    return TRUE;
	if ( type->canCastFrom( d->custom_type ) )
	    return TRUE;
	if ( d->custom_type->canCastTo( type ) )
	    return TRUE;
	return FALSE;
    }

    return type->canCastFrom( d->typ );
}

/*!
  Returns the variant's custom type or 0, if the variant is no custom type.

  \sa type()
 */
const QVariantTypeBase* QVariant::customType() const
{
    if ( d->typ == Custom )
	return d->custom_type;
    return 0;
}

// ------------------------------------------------------------------

#include <qasciidict.h>

static QAsciiDict<QVariantTypeBase>* typeDict = 0;

QVariantTypeBase::QVariantTypeBase( const char* type )
    : m_type( type )
{
    if ( typeDict == 0 )
	typeDict = new QAsciiDict<QVariantTypeBase>;
    typeDict->insert( type, this );
}

QVariantTypeBase::~QVariantTypeBase()
{
    typeDict->remove( m_type );
}

void* QVariantTypeBase::castFrom( const QVariant& ) const
{
    return 0;
}

QVariant QVariantTypeBase::castTo( const void*, QVariant::Type ) const
{
    return QVariant();
}

void* QVariantTypeBase::castTo( const void*, const QVariantTypeBase* ) const
{
    return 0;
}

bool QVariantTypeBase::canCastTo( const QVariantTypeBase* ) const
{
    return FALSE;
}

bool QVariantTypeBase::canCastTo( QVariant::Type ) const
{
    return FALSE;
}

bool QVariantTypeBase::canCastFrom( const QVariantTypeBase* ) const
{
    return FALSE;
}

bool QVariantTypeBase::canCastFrom( QVariant::Type ) const
{
    return FALSE;
}

const char* QVariantTypeBase::typeName() const
{
    return (const char*)m_type;
}

const QVariantTypeBase* QVariantTypeBase::type( const char* t )
{
    if ( !typeDict )
	return 0;

    return (*typeDict)[ t ];
}
