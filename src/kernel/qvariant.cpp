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
  to create a QVariant from a value of the type, a setValue(T) function to
  change a variant to a value of the type, and a toT() function to retrieve
  the value. For example:

  \code
    QDataStream out(...);
    QVariant v(123);          // The variant now contains an int
    int x = v.toInt();        // x = 123
    out << v;                 // Writes a type tag an int to out
    v.setValue("hello");      // The variant now contains a QCString
    v.setValue(tr("hello"));  // The variant now contains a QString
    int y = v.toInt();        // x = 0, since v is not an int
    QString s = v.toString(); // s = tr("hello")  (see QObject::tr())
    out << v;                 // Writes a type tag and a QString to out
    ...
    QDataStream in(...);      // (opening the previously written stream)
    in >> v;                  // Reads an Int variant
    int z = v.toInt();        // z = 123
    qDebug("Type is %s",      // prints "Type is int"
      v.typeName());
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

  An exception to this pattern is setBoolValue(), which cannot be called
  setValue(bool), since some compilers cannot distinguish between
  bool and int.
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

  </ul> Note that Qt's idea of bool depends on the compiler.
  qglobal.h has the system-dependent definition of bool.
*/

/*!
  Constructs an invalid variant.
*/
QVariant::QVariant()
{
    typ = Invalid;
    custom_type = 0;
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
    clear();
}

/*!  Constructs a deep copy of the variant passed as argument to this
  constructor.
*/
QVariant::QVariant( const QVariant& p ) : QShared()
{
    typ = Invalid;
    *this = p;
}

/*!
  Reads the variant from the data stream.
*/
QVariant::QVariant( QDataStream& s )
{
    s >> *this;
}

/*!
  Constructs a new variant with a string value.
*/
QVariant::QVariant( const QString& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a c-string value.
*/
QVariant::QVariant( const QCString& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a c-string value.
*/
QVariant::QVariant( const char* val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a string list value.
*/
QVariant::QVariant( const QStringList& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a map of QVariants.
*/
QVariant::QVariant( const QMap<QString,QVariant>& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a font value.
*/
QVariant::QVariant( const QFont& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a pixmap value.
*/
QVariant::QVariant( const QPixmap& val )
{
    typ = Invalid;
    setValue( val );
}


/*!
  Constructs a new variant with an image value.
*/
QVariant::QVariant( const QImage& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a brush value.
*/
QVariant::QVariant( const QBrush& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a point value.
*/
QVariant::QVariant( const QPoint& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a rect value.
*/
QVariant::QVariant( const QRect& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a size value.
*/
QVariant::QVariant( const QSize& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a color value.
*/
QVariant::QVariant( const QColor& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a color palette value.
*/
QVariant::QVariant( const QPalette& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a color group value.
*/
QVariant::QVariant( const QColorGroup& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with an iconset.
*/
QVariant::QVariant( const QIconSet& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a region.
*/
QVariant::QVariant( const QRegion& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a bitmap.
*/
QVariant::QVariant( const QBitmap& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a cursor..
*/
QVariant::QVariant( const QCursor& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with an array of points.
*/
QVariant::QVariant( const QPointArray& val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with an integer value.
*/
QVariant::QVariant( int val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with an unsigned integer value.
*/
QVariant::QVariant( uint val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a boolean value.
*/
QVariant::QVariant( bool val )
{
    typ = Invalid;
    setBoolValue( val );
}


/*!
  Constructs a new variant with a floating point value.
*/
QVariant::QVariant( double val )
{
    typ = Invalid;
    setValue( val );
}

/*!
  Constructs a new variant with a list value.
*/
QVariant::QVariant( const QValueList<QVariant>& val )
{
    typ = Invalid;
    setValue( val );
}

QVariant::QVariant( void* custom, const QVariantTypeBase* type )
{
    typ = Invalid;
    setValue( custom, type );
}

QVariant::QVariant( const QVariantValueBase& v )
{
    typ = Invalid;
    setValue( v );
}

QVariant& QVariant::operator= ( const QString& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QCString& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const char* v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QStringList& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QFont& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QPixmap& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QImage& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QBrush& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QPoint& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QRect& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QSize& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QColor& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QPalette& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QColorGroup& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QIconSet& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QValueList<QVariant>& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QMap<QString,QVariant>& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( int v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( uint v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( double v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QVariantValueBase& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QRegion& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QBitmap& v )
{
    setValue( v );
    return *this;
}

QVariant& QVariant::operator= ( const QPointArray& v )
{
    setValue( v );
    return *this;
}

/*!
  Assigns the value of some \a other variant to this variant.
  This is a deep copy.
*/
QVariant& QVariant::operator= ( const QVariant& variant )
{
    clear();

    QVariant& other = (QVariant&)variant;

    switch( other.type() )
	{
	case Invalid:
	    break;
	case Custom:
	    custom_type = other.custom_type;
	    value.ptr = custom_type->copy( value.ptr );
	    break;
	case Bitmap:
	    value.ptr = new QBitmap( other.asBitmap() );
	    break;
	case Region:
	    value.ptr = new QRegion( other.asRegion() );
	    break;
	case PointArray:
	    value.ptr = new QPointArray( other.asPointArray() );
	    break;
	case String:
	    value.ptr = new QString( other.asString() );
	    break;
	case CString:
	    value.ptr = new QCString( other.asCString() );
	    break;
	case StringList:
	    value.ptr = new QStringList( other.asStringList() );
	    break;
	case Map:
	    value.ptr = new QMap<QString,QVariant>( other.asMap() );
	    break;
	case Font:
	    value.ptr = new QFont( other.asFont() );
	    break;
	case Pixmap:
	    value.ptr = new QPixmap( other.asPixmap() );
	    break;
	case Image:
	    value.ptr = new QImage( other.asImage() );
	    break;
	case Brush:
	    value.ptr = new QBrush( other.asBrush() );
	    break;
	case Point:
	    value.ptr = new QPoint( other.asPoint() );
	    break;
	case Rect:
	    value.ptr = new QRect( other.asRect() );
	    break;
	case Size:
	    value.ptr = new QSize( other.asSize() );
	    break;
	case Color:
	    value.ptr = new QColor( other.asColor() );
	    break;
	case Palette:
	    value.ptr = new QPalette( other.asPalette() );
	    break;
	case ColorGroup:
	    value.ptr = new QColorGroup( other.asColorGroup() );
	    break;
	case IconSet:
	    value.ptr = new QIconSet( other.asIconSet() );
	    break;
	case List:
	    value.ptr = new QValueList<QVariant>( other.asList() );
	    break;
	case Int:
	    value.i = other.asInt();
	    break;
	case UInt:
	    value.u = other.asUInt();
	    break;
	case Bool:
	    value.b = other.asBool();
	    break;
	case Double:
	    value.d = other.asDouble();
	    break;
	default:
	    ASSERT( 0 );
	}

    typ = other.type();

    return *this;
}

/*!
  Returns the name of the type stored in the variant.
  The returned strings describe the C++ datatype used to store the
  data, for example "QFont", "QString" or "QValueList<QVariant>".
  An Invalid variant returns 0.
*/
const char* QVariant::typeName() const
{
    if ( typ == Custom )
	return custom_type->typeName();
    return typeToName( typ );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QString& val )
{
    clear();
    typ = String;
    value.ptr = new QString( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QCString& val )
{
    clear();
    typ = CString;
    value.ptr = new QCString( val );
}

/*!
  Changes the value of this variant to \a val.
  The Variant creates a copy of the passed string.
*/
void QVariant::setValue( const char* val )
{
    clear();
    typ = CString;
    value.ptr = new QCString( val );
}

/*!
  Changes the value of this variant to \a val.
  This function creates a copy of the list. This is very fast since
  QStringList is implicit shared.
*/
void QVariant::setValue( const QStringList& val )
{
    clear();
    typ = StringList;
    value.ptr = new QStringList( val );
}

/*!
  Changes the value of this variant to \a val.
  This function creates a copy of the map. This is very fast since
  QMap is implicit shared.
*/
void QVariant::setValue( const QMap<QString,QVariant>& val )
{
    clear();
    typ = Map;
    value.ptr = new QMap<QString,QVariant>( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QFont& val )
{
    clear();
    typ = Font;
    value.ptr = new QFont( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QPixmap& val )
{
    clear();
    typ = Pixmap;
    value.ptr = new QPixmap( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QImage& val )
{
    clear();
    typ = Image;
    value.ptr = new QImage( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QBrush& val )
{
    clear();
    typ = Brush;
    value.ptr = new QBrush( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QRect& val )
{
    clear();
    typ = Rect;
    value.ptr = new QRect( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QPoint& val )
{
    clear();
    typ = Point;
    value.ptr = new QPoint( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QSize& val )
{
    clear();
    typ = Size;
    value.ptr = new QSize( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QColor& val )
{
    clear();
    typ = Color;
    value.ptr = new QColor( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QPalette& val )
{
    clear();
    typ = Palette;
    value.ptr = new QPalette( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QColorGroup& val )
{
    clear();
    typ = ColorGroup;
    value.ptr = new QColorGroup( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QIconSet& val )
{
    clear();
    typ = IconSet;
    value.ptr = new QIconSet( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QBitmap& val )
{
    clear();
    typ = Bitmap;
    value.ptr = new QBitmap( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QRegion& val )
{
    clear();
    typ = Region;
    value.ptr = new QRegion( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QCursor& val )
{
    clear();
    typ = Cursor;
    value.ptr = new QCursor( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( const QPointArray& val )
{
    clear();
    typ = PointArray;
    value.ptr = new QPointArray( val );
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( int val )
{
    clear();
    typ = Int;
    value.i = val;
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( uint val )
{
    clear();
    typ = UInt;
    value.u = val;
}

/*!
  Changes the value of this variant to \a val.
  This is not called setValue(bool), since some compilers
  cannot distinguish between bool and int.
*/
void QVariant::setBoolValue( bool val )
{
    clear();
    typ = Bool;
    value.b = val;
}

/*!
  Changes the value of this variant to \a val.
*/
void QVariant::setValue( double val )
{
    clear();
    typ = Double;
    value.d = val;
}

/*!
  Changes the value of this variant to \a val.
  This function creates a copy of the list.
*/
void QVariant::setValue( const QValueList<QVariant>& val )
{
    clear();
    typ = List;
    value.ptr = new QValueList<QVariant>( val );
}

void QVariant::setValue( void* custom, const QVariantTypeBase* type )
{
    clear();
    if ( !custom || !type )
	return;

    custom_type = type;
    typ = Custom;
    value.ptr = custom_type->copy( custom );
}

void QVariant::setValue( const QVariantValueBase& v )
{
    clear();
    typ = Custom;
    custom_type = v.type();
    value.ptr = custom_type->copy( v.value() );
}

/*!
  De-allocate any used memory,
  based on the type, producing an Invalid variant.
*/
void QVariant::clear()
{
    switch( typ )
	{
	case Custom:
	    custom_type->destroy( value.ptr );
	    custom_type = 0;
	    break;
	case Bitmap:
	    delete (QBitmap*)value.ptr;
	    break;
	case Cursor:
	    delete (QCursor*)value.ptr;
	    break;
	case Region:
	    delete (QRegion*)value.ptr;
	    break;
	case PointArray:
	    delete (QPointArray*)value.ptr;
	    break;
	case String:
	    delete (QString*)value.ptr;
	    break;
	case CString:
	    delete (QCString*)value.ptr;
	    break;
	case Map:
	    delete (QMap<QString,QVariant>*)value.ptr;
	    break;
	case StringList:
	    delete (QStringList*)value.ptr;
	    break;
	case Font:
	    delete (QFont*)value.ptr;
	    break;
	case Pixmap:
	    delete (QPixmap*)value.ptr;
	    break;
	case Image:
	    delete (QImage*)value.ptr;
	    break;
	case Brush:
	    delete (QBrush*)value.ptr;
	    break;
	case Point:
	    delete (QPoint*)value.ptr;
	    break;
	case Rect:
	    delete (QRect*)value.ptr;
	    break;
	case Size:
	    delete (QSize*)value.ptr;
	    break;
	case Color:
	    delete (QColor*)value.ptr;
	    break;
	case Palette:
	    delete (QPalette*)value.ptr;
	    break;
	case ColorGroup:
	    delete (QColorGroup*)value.ptr;
	    break;
	case IconSet:
	    delete (QIconSet*)value.ptr;
	    break;
	case List:
	    delete (QValueList<QVariant>*)value.ptr;
	    break;
	case Invalid:
	case Int:
	case UInt:
	case Bool:
	case Double:
	    break;
	default:
	    ASSERT(0);
	}

    typ = Invalid;
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
	    typ = t;
	    break;
	case Custom:
	    {
		QCString t;
		s >> t;
		custom_type = QVariantTypeBase::type( t );
		if ( !custom_type )
	        {
		    typ = Invalid;
		    return;
		}
		value.ptr = custom_type->create();
		custom_type->load( value.ptr, s );
	    }
	    break;
	case Map:
	    { QMap<QString,QVariant> x; s >> x; setValue( x ); }
	    break;
	case List:
	    { QValueList<QVariant> x; s >> x; setValue( x ); }
	    break;
	case Cursor:
	    { QCursor x; s >> x; setValue( x ); }
	    break;
	case Bitmap:
	    { QBitmap x; s >> x; setValue( x ); }
	    break;
	case Region:
	    { QRegion x; s >> x; setValue( x ); }
	    break;
	case PointArray:
	    { QPointArray x; s >> x; setValue( x ); }
	    break;
	case String:
	    { QString x; s >> x; setValue( x ); }
	    break;
	case CString:
	    { QCString x; s >> x; setValue( x ); }
	    break;
	case StringList:
	    { QStringList x; s >> x; setValue( x ); }
	    break;
	case Font:
	    { QFont x; s >> x; setValue( x ); }
	    break;
	case Pixmap:
	    { QPixmap x; s >> x; setValue( x ); }
	    break;
	case Image:
	    { QImage x; s >> x; setValue( x ); }
	    break;
	case Brush:
	    { QBrush x; s >> x; setValue( x ); }
	    break;
	case Rect:
	    { QRect x; s >> x; setValue( x ); }
	    break;
	case Point:
	    { QPoint x; s >> x; setValue( x ); }
	    break;
	case Size:
	    { QSize x; s >> x; setValue( x ); }
	    break;
	case Color:
	    { QColor x; s >> x; setValue( x ); }
	    break;
	case Palette:
	    { QPalette x; s >> x; setValue( x ); }
	    break;
	case ColorGroup:
	    { QColorGroup x; s >> x; setValue( x ); }
	    break;
	case IconSet:
	    { QPixmap x; s >> x; setValue( QIconSet( x ) ); }
	    break;
	case Int:
	    { int x; s >> x; setValue( x ); };
	    break;
	case UInt:
	    { uint x; s >> x; setValue( x ); };
	    break;
	case Bool:
	    { Q_INT8 x; s >> x; setBoolValue( x ); };
	    break;
	case Double:
	    { double x; s >> x; setValue( x ); };
	    break;
	default:
	    ASSERT(0);
	}
}

/*!
  Internal function for saving a variant. Use the stream operators
  instead.
*/
void QVariant::save( QDataStream& s ) const
{
    s << (Q_UINT32)type();

    QVariant* that = (QVariant*)this;

    switch( typ )
	{
	case Custom:
	    s << custom_type->typeName();
	    custom_type->save( value.ptr, s );
	    break;
	case Cursor:
	    s << that->asCursor();
	    break;
	case Bitmap:
	    s << that->asBitmap();
	    break;
	case PointArray:
	    s << that->asPointArray();
	    break;
	case Region:
	    s << that->asRegion();
	    break;
	case List:
	    s << that->asList();
	    break;
	case Map:
	    s << that->asMap();
	    break;
	case String:
	    s << that->asString();
	    break;
	case CString:
	    s << that->asCString();
	    break;
	case StringList:
	    s << that->asStringList();
	    break;
	case Font:
	    s << that->asFont();
	    break;
	case Pixmap:
	    s << that->asPixmap();
	    break;
	case Image:
	    s << that->asImage();
	    break;
	case Brush:
	    s << that->asBrush();
	    break;
	case Point:
	    s << that->asPoint();
	    break;
	case Rect:
	    s << that->asRect();
	    break;
	case Size:
	    s << that->asSize();
	    break;
	case Color:
	    s << that->asColor();
	    break;
	case Palette:
	    s << that->asPalette();
	    break;
	case ColorGroup:
	    s << that->asColorGroup();
	    break;
	case IconSet:
	    s << that->asIconSet().pixmap(); //### add stream operator to iconset #ME
	    break;
	case Int:
	    s << that->asInt();
	    break;
	case UInt:
	    s << that->asUInt();
	    break;
	case Bool:
	    s << (Q_INT8)that->asBool();
	    break;
	case Double:
	    s << that->asDouble();
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


/*!
  Returns the variant as a QString if the variant has type()
  String or CString, or QString::null otherwise.
*/
const QString QVariant::toString() const
{
    if ( typ == CString )
	return QString::fromLatin1( toCString() );
    if ( typ != String )
	return QString::null;
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, String ).toString();

    return *((QString*)value.ptr);
}

/*!
  Returns the variant as a QCString if the variant has type()
  CString, or a null QCString otherwise.
*/
const QCString QVariant::toCString() const
{
    if ( typ == CString )
	return *((QCString*)value.ptr);
    if ( typ == String )
	return ((QString*)value.ptr)->latin1();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, CString ).toCString();

    return 0;
}

/*!
  Returns the variant as a QStringList if the variant has type()
  StringList or List, or an empty list otherwise.
*/
const QStringList QVariant::toStringList() const
{
    if ( typ == StringList )
	return *((QStringList*)value.ptr);
    if ( typ == List )
    {
	QStringList lst;
	QValueList<QVariant> vl = toList();
	QValueList<QVariant>::ConstIterator it = vl.begin();
	QValueList<QVariant>::ConstIterator end = vl.end();
	/*
	QValueList<QVariant>::ConstIterator it = toList().begin();
	QValueList<QVariant>::ConstIterator end = toList().end(); */
	for( ; it != end; ++it )
	    lst.append( (*it).toString() );
	return lst;
    }
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, StringList ).toStringList();

    return QStringList();
}

/*!
  \fn QMap<QString, QVariant> QVariant::toMap () const

  Returns the variant as a QMap<QString,QVariant> if the variant has type()
  Map, or an empty map otherwise.
*/
const QMap<QString,QVariant> QVariant::toMap() const
{
    if ( typ != Map )
	return QMap<QString,QVariant>();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Map ).toMap();

    return *((QMap<QString,QVariant>*)value.ptr);
}

/*!
  Returns the variant as a QFont if the variant has type()
  Font, or the default font otherwise.
*/
const QFont QVariant::toFont() const
{
    if ( typ != Font )
	return QFont();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Font ).toFont();

    return *((QFont*)value.ptr);
}

/*!
  Returns the variant as a QPixmap if the variant has type()
  Pixmap, or a null pixmap otherwise.
*/
const QPixmap QVariant::toPixmap() const
{
    if ( typ != Pixmap )
	return QPixmap();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Pixmap ).toPixmap();

    return *((QPixmap*)value.ptr);
}

/*!
  Returns the variant as a QImage if the variant has type()
  Image, or a null image otherwise.
*/
const QImage QVariant::toImage() const
{
    if ( typ != Image )
	return QImage();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Image ).toImage();

    return *((QImage*)value.ptr);
}

/*!
  Returns the variant as a QBrush if the variant has type()
  Brush, or a default brush otherwise.
*/
const QBrush QVariant::toBrush() const
{
    if( typ != Brush )
	return QBrush();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Brush ).toBrush();

    return *((QBrush*)value.ptr);
}

/*!
  Returns the variant as a QPoint if the variant has type()
  Point, or a the point (0,0) otherwise.
*/
const QPoint QVariant::toPoint() const
{
    if ( typ != Point )
	return QPoint();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Point ).toPoint();

    return *((QPoint*)value.ptr);
}

/*!
  Returns the variant as a QRect if the variant has type()
  Rect, or an empty rectangle otherwise.
*/
const QRect QVariant::toRect() const
{
    if ( typ != Rect )
	return QRect();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Rect ).toRect();

    return *((QRect*)value.ptr);
}

/*!
  Returns the variant as a QSize if the variant has type()
  Size, or an invalid size otherwise.
*/
const QSize QVariant::toSize() const
{
    if ( typ != Size )
	return QSize();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Size ).toSize();

    return *((QSize*)value.ptr);
}

/*!
  Returns the variant as a QColor if the variant has type()
  Color, or an invalid color otherwise.
*/
const QColor QVariant::toColor() const
{
    if ( typ != Color )
	return QColor();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Color ).toColor();

    return *((QColor*)value.ptr);
}

/*!
  Returns the variant as a QPalette if the variant has type()
  Palette, or a completely black palette otherwise.
*/
const QPalette QVariant::toPalette() const
{
    if ( typ != Palette )
	return QPalette();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Palette ).toPalette();

    return *((QPalette*)value.ptr);
}

/*!
  Returns the variant as a QColorGroup if the variant has type()
  ColorGroup, or a completely black color group otherwise.
*/
const QColorGroup QVariant::toColorGroup() const
{
    if ( typ != ColorGroup )
	return QColorGroup();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, ColorGroup ).toColorGroup();

    return *((QColorGroup*)value.ptr);
}

/*!
  Returns the variant as a QIconSet if the variant has type()
  IconSet, or an icon set of null pixmaps otherwise.
*/
const QIconSet QVariant::toIconSet() const
{
    if ( typ != IconSet )
	return QIconSet();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, IconSet ).toIconSet();

    return *((QIconSet*)value.ptr);
}

const QPointArray QVariant::toPointArray() const
{
    if ( typ != PointArray )
	return QPointArray();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, PointArray ).toPointArray();

    return *((QPointArray*)value.ptr);}

const QBitmap QVariant::toBitmap() const
{
    if ( typ != Bitmap )
	return QBitmap();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Bitmap ).toBitmap();

    return *((QBitmap*)value.ptr);
}

const QRegion QVariant::toRegion() const
{
    if ( typ != Region )
	return QRegion();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Region ).toRegion();

    return *((QRegion*)value.ptr);
}

const QCursor QVariant::toCursor() const
{
    if ( typ != Cursor )
	return QCursor();
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Cursor ).toCursor();

    return *((QCursor*)value.ptr);
}

/*!
  Returns the variant as an int if the variant has type()
  Int, UInt, Double or Bool, or 0 otherwise.
*/
int QVariant::toInt() const
{
    if( typ == Int )
	return value.i;
    if( typ == UInt )
	return (int)value.u;
    if ( typ == Double )
	return (int)value.d;
    if ( typ == Bool )
	return (int)value.b;
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Int ).toInt();

    /* if ( typ == String )
	return ((QString*)value.ptr)->toInt();
    if ( typ == CString )
    return ((QCString*)value.ptr)->toInt(); */
    return 0;
}

/*!
  Returns the variant as an unsigned int if the variant has type()
  UInt, Int, Double or Bool, or 0 otherwise.
*/
uint QVariant::toUInt() const
{
    if( typ == Int )
	return value.i;
    if( typ == UInt )
	return (int)value.u;
    if ( typ == Double )
	return (int)value.d;
    if ( typ == Bool )
	return (int)value.b;
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, UInt ).toUInt();

    /* if ( typ == String )
	return ((QString*)value.ptr)->toInt();
    if ( typ == CString )
    return ((QCString*)value.ptr)->toInt(); */
    return 0;
}

/*!
  Returns the variant as a bool if the variant has type()
  Bool, or FALSE otherwise. The only exceptions to this rule are
  the types Int, UInt, Double. In this case TRUE is returned if the numerical
  value is not zero or FALSE otherwise.
*/
bool QVariant::toBool() const
{
    if ( typ == Bool )
	return value.b;
    if ( typ == Double )
	return value.d != 0.0;
    if ( typ == Int )
	return value.i != 0;
    if ( typ == UInt )
	return value.u != 0;
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Bool ).toBool();

    /* if ( typ == String )
	return *((QString*)value.ptr) == "true";
    if ( typ == CString )
    return *((QCString*)value.ptr) == "true"; */
    return FALSE;
}

/*!
  Returns the variant as a double if the variant has type()
  Double, Int, UInt or Bool, or 0.0 otherwise.
*/
double QVariant::toDouble() const
{
    if ( typ == Double )
	return value.d;
    if ( typ == Int )
	return (double)value.i;
    if ( typ == Bool )
	return (double)value.b;
    if ( typ == UInt )
	return (double)value.u;
    /* if ( typ == String )
	return ((QString*)value.ptr)->toDouble();
    if ( typ == CString )
    return ((QCString*)value.ptr)->toDouble(); */
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, Double ).toDouble();
    return 0.0;
}

/*!
  Returns the variant as a QValueList<QVariant> if the variant has type()
  List or StringList, or an empty list otherwise.
*/
const QValueList<QVariant> QVariant::toList() const
{
    if ( typ == List )
	return *((QValueList<QVariant>*)value.ptr);
    if ( typ == StringList ) {
	QValueList<QVariant> lst;
	/*
	QStringList::ConstIterator it = toStringList().begin();
	QStringList::ConstIterator end = toStringList().end(); */
	QStringList sl = toStringList();
	QStringList::ConstIterator it = sl.begin();
	QStringList::ConstIterator end = sl.end();
	for( ; it != end; ++it )
	    lst.append( QVariant( *it ) );
	return lst;
    }
    if ( typ == Custom )
	return custom_type->castTo( value.ptr, List ).toList();

    return QValueList<QVariant>();
}

void* QVariant::toCustom( const QVariantTypeBase* type ) const
{
    if ( type == 0 )
	return 0;

    // Both the same custom type ?
    if ( typ == Custom && type == custom_type )
	return custom_type->copy( value.ptr );

    // Both custom types, but different ones ?
    if ( typ == Custom )
    {
	// Can the passes type do the conversion
	if ( type->canCastFrom( custom_type ) )
	    return type->castFrom( *this );
	// Can our type do the conversion ?
	if ( custom_type->canCastTo( type ) )
	    return custom_type->castTo( value.ptr, type );
	return 0;
    }

    // This is a basic type that is to be converted to
    // a custom type.
    return type->castFrom( *this );
}

/*!  
  Returns the variant's custom value as void pointer or 0, if the
  variant is no custom type.
 */
const void* QVariant::asCustom() const
{
    if ( typ != Custom )
	return 0;
    return value.ptr;
}

/*!  
  Returns the variant's custom value as void pointer or 0, if the
  variant is no custom type.
 */
void* QVariant::asCustom()
{
    if ( typ != Custom )
	return 0;
    return value.ptr;
}

/*!  
  Returns the variant's custom value converted to the custom
  variant type \a type or 0, if a cast was not possible.

  \sa canCast()
 */
void* QVariant::asCustom( const QVariantTypeBase* type )
{
    if ( type == 0 )
	return 0;

    // Both the same custom type ?
    if ( typ == Custom && type == custom_type )
	return value.ptr;

    // Both custom types, but different ones ?
    if ( typ == Custom ) {
	// Can the passes type do the conversion
	if ( type->canCastFrom( custom_type ) ) {
	    void* ptr = type->castFrom( *this );
	    if ( !ptr )
		return 0;
	    clear();
	    custom_type = type;
	    value.ptr = ptr;
	    return ptr;
	}
	// Can our type do the conversion ?
	if ( custom_type->canCastTo( type ) ) {
	    void* ptr = custom_type->castTo( value.ptr, type );
	    if ( !ptr )
		return 0;
	    clear();
	    custom_type = type;
	    value.ptr = ptr;
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
    custom_type = type;
    value.ptr = ptr;
    return ptr;
}

#define Q_VARIANT_AS( f ) Q##f& QVariant::as##f() { \
   if ( typ != ##f ) setValue( to##f() ); return *((Q##f*)value.ptr);}

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
  
  Returns the variant's value as string reference.
 */
/*! \fn QCString& QVariant::asCString() 
  
  Returns the variant's value as cstring reference.
 */
/*! \fn QStringList& QVariant::asStringList() 
  
  Returns the variant's value as stringlist  reference.
 */
/*! \fn QFont& QVariant::asFont() 
  
  Returns the variant's value as font  reference.
 */
/*! \fn QPixmap& QVariant::asPixmap() 
  
  Returns the variant's value as pixmap  reference.
 */
/*! \fn QImage& QVariant::asImage() 
  
  Returns the variant's value as image  reference.
 */
/*! \fn QBrush& QVariant::asBrush() 
  
  Returns the variant's value as brush reference.
 */
/*! \fn QPoint& QVariant::asPoint() 
  
  Returns the variant's value as point  reference.
 */
/*! \fn QRect& QVariant::asRect() 
  
  Returns the variant's value as rect  reference.
 */
/*! \fn QSize& QVariant::asSize() 
  
  Returns the variant's value as size  reference.
 */
/*! \fn QColor& QVariant::asColor() 
  
  Returns the variant's value as color  reference.
 */
/*! \fn QPalette& QVariant::asPalette() 
  
  Returns the variant's value as palette  reference.
 */
/*! \fn QColorGroup& QVariant::asColorGroup() 
  
  Returns the variant's value as colorgroup  reference.
 */
/*! \fn QIconSet& QVariant::asIconSet() 
  
  Returns the variant's value as iconset  reference.
 */
/*! \fn QPointArray& QVariant::asPointArray() 
  
  Returns the variant's value as  pointarray  reference.
 */
/*! \fn QBitmap& QVariant::asBitmap() 
  
  Returns the variant's value as bitmap  reference.
 */
/*! \fn QRegion& QVariant::asRegion() 
  
  Returns the variant's value as region  reference.
 */
/*! \fn QCursor& QVariant::asCursor() 
  
  Returns the variant's value as cursor  reference.
 */

/*!
  Returns the variant's value as int reference.
 */
int& QVariant::asInt()
{
    if ( typ != Int )
	setValue( toInt() );
    return value.i;
}

/*!
  Returns the variant's value as unsigned int reference.
 */
uint& QVariant::asUInt()
{
    if ( typ != UInt )
	setValue( toUInt() );
    return value.u;
}

/*!
  Returns the variant's value as bool reference.
 */
bool& QVariant::asBool()
{
    if ( typ != Bool )
	setBoolValue( toBool() );
    return value.b;
}

/*!
  Returns the variant's value as double reference.
 */
double& QVariant::asDouble()
{
    if ( typ != Double )
	setValue( toDouble() );
    return value.d;
}

/*!
  Returns the variant's value as variant list reference.
 */
QValueList<QVariant>& QVariant::asList()
{
    if ( typ != List )
	setValue( toList() );
    return *((QValueList<QVariant>*)value.ptr);
}

/*!
  Returns the variant's value as variant map reference.
 */
QMap<QString,QVariant>& QVariant::asMap()
{
    if ( typ != Map )
	setValue( toMap() );
    return *((QMap<QString,QVariant>*)value.ptr);
}

/*!
  Returns the variant's value as string list reference.
 */
const QStringList& QVariant::asStringList() const
{
    static QStringList* ptr = 0;

    if ( typ != StringList )  {
	qDebug("WARNING: in \"const QStringList& QVariant::asStringList() const\"" );
	qDebug("         The variant contains the data type %s instead of QStringList.", typeName() );
	if ( !ptr ) ptr = new QStringList;
	return *ptr;
    }

    return *((QStringList*)value.ptr);
}

/*!
  Returns the variant's value as variant list reference.
 */
const QValueList<QVariant>& QVariant::asList() const
{
    static QValueList<QVariant>* ptr = 0;

    if ( typ != List ) {
	qDebug("WARNING: in \"const QValueList<QVariant>& QVariant::asList() const\"" );
	qDebug("         The variant contains the data type %s instead of QValueList<QVariant>.", typeName() );
	if ( !ptr ) ptr = new QValueList<QVariant>;
	return *ptr;
    }

    return *((QValueList<QVariant>*)value.ptr);
}

/*!
  Returns the variant's value as variant map reference.
 */
const QMap<QString,QVariant>& QVariant::asMap() const
{
    static QMap<QString,QVariant>* ptr = 0;

    if ( typ != List ) {
	qDebug("WARNING: in \"const QMap<QString,QVariant>& QVariant::asMap() const\"" );
	qDebug("         The variant contains the data type %s instead of QMap<QString,QVariant>.", typeName() );
	if ( !ptr ) ptr = new QMap<QString,QVariant>;
	return *ptr;
    }

    return *((QMap<QString,QVariant>*)value.ptr);
}

/*!
  Returns TRUE if the current type of the variant can be casted to
  the requested type. The casting is done automatically when calling
  the toInt(), toBool(), ... methods.

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
*/
bool QVariant::canCast( Type t ) const
{
    if ( typ == t )
	return TRUE;
    if ( t == Bool && ( typ == Double || typ == Int || typ == UInt ) )
	 return TRUE;
    if ( t == Int && ( typ == Double || typ == Bool || typ == UInt ) )
	return TRUE;
    if ( t == UInt && ( typ == Double || typ == Bool || typ == Int ) )
	return TRUE;
    if ( t == Double && ( typ == Int || typ == Bool || typ == UInt ) )
	return TRUE;
    if ( t == CString && typ == String )
	return TRUE;
    if ( t == String && typ == CString )
	return TRUE;
    if ( t == List && typ == StringList )
	return TRUE;
    if ( t == StringList && typ == List )   {
	/* QValueList<QVariant>::ConstIterator it = toList().begin();
	   QValueList<QVariant>::ConstIterator end = toList().end(); */
	QValueList<QVariant> vl = toList();
	QValueList<QVariant>::ConstIterator it = vl.begin();
	QValueList<QVariant>::ConstIterator end = vl.end();
	for( ; it != end; ++it ) {
	    qDebug("Testing '%s'", (*it).typeName() );
	    if ( !(*it).canCast( String ) )
		return FALSE;
	}
	return TRUE;
    }
    if ( typ == Custom )
	return custom_type->canCastTo( t );

    return FALSE;
}

/*!
  Returns TRUE if the current type of the variant can be casted to
  the requested custom QVariantType type.
*/
bool QVariant::canCast( const QVariantTypeBase* type ) const
{
    if ( type == 0 )
	return FALSE;

    if ( typ == Custom ) {
	if ( type == custom_type )
	    return TRUE;
	if ( type->canCastFrom( custom_type ) )
	    return TRUE;
	if ( custom_type->canCastTo( type ) )
	    return TRUE;
	return FALSE;
    }

    return type->canCastFrom( typ );
}

/*!
  Returns the variant's custom type or 0, if the variant is no custom type.
  
  \sa type()
 */
const QVariantTypeBase* QVariant::customType() const
{
    if ( typ == Custom )
	return custom_type;
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
