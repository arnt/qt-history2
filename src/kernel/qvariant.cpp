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

// REVISED: warwick
/*!
  \class QVariant qvariant.h
  \brief Acts like a union for the most common Qt data types.

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
  Constructs a new variant with an empty iconset
*/
QVariant::QVariant( const QIconSet& val )
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

/*!
  Assigns the value of some \a other variant to this variant.
  This is a deep copy.
*/
QVariant& QVariant::operator= ( const QVariant& other )
{
    clear();

    switch( other.type() )
	{
	case Invalid:
	    break;
	case String:
	    value.ptr = new QString( other.toString() );
	    break;
	case CString:
	    value.ptr = new QCString( other.toCString() );
	    break;
	case StringList:
	    value.ptr = new QStringList( other.toStringList() );
	    break;
	case Map:
	    value.ptr = new QMap<QString,QVariant>( other.toMap() );
	    break;
	case Font:
	    value.ptr = new QFont( other.toFont() );
	    break;
	case Pixmap:
	    value.ptr = new QPixmap( other.toPixmap() );
	    break;
	case Image:
	    value.ptr = new QImage( other.toImage() );
	    break;
	case Brush:
	    value.ptr = new QBrush( other.toBrush() );
	    break;
	case Point:
	    value.ptr = new QPoint( other.toPoint() );
	    break;
	case Rect:
	    value.ptr = new QRect( other.toRect() );
	    break;
	case Size:
	    value.ptr = new QSize( other.toSize() );
	    break;
	case Color:
	    value.ptr = new QColor( other.toColor() );
	    break;
	case Palette:
	    value.ptr = new QPalette( other.toPalette() );
	    break;
	case ColorGroup:
	    value.ptr = new QColorGroup( other.toColorGroup() );
	    break;
	case IconSet:
	    value.ptr = new QIconSet( other.toIconSet() );
	    break;
	case List:
	    value.ptr = new QValueList<QVariant>( other.toList() );
	    break;
	case Int:
	    value.i = other.toInt();
	    break;
	case UInt:
	    value.u = other.toUInt();
	    break;
	case Bool:
	    value.b = other.toBool();
	    break;
	case Double:
	    value.d = other.toDouble();
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
  data, for example "QFont", "QString" or "QValueList<int>".
  An Invalid variant returns 0.
*/
const char* QVariant::typeName() const
{
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

/*!
  De-allocate any used memory,
  based on the type, producing an Invalid variant.
*/
void QVariant::clear()
{
    switch( typ )
	{
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
static const int ntypes = 21;
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
    "QCString"
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
	case Map:
	    { QMap<QString,QVariant> x; s >> x; setValue( x ); }
	    break;
	case List:
	    { QValueList<QVariant> x; s >> x; setValue( x ); }
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

    switch( typ )
	{
	case List:
	    s << toList();
	    break;
	case Map:
	    s << toMap();
	    break;
	case String:
	    s << toString();
	    break;
	case CString:
	    s << toCString();
	    break;
	case StringList:
	    s << toStringList();
	    break;
	case Font:
	    s << toFont();
	    break;
	case Pixmap:
	    s << toPixmap();
	    break;
	case Image:
	    s << toImage();
	    break;
	case Brush:
	    s << toBrush();
	    break;
	case Point:
	    s << toPoint();
	    break;
	case Rect:
	    s << toRect();
	    break;
	case Size:
	    s << toSize();
	    break;
	case Color:
	    s << toColor();
	    break;
	case Palette:
	    s << toPalette();
	    break;
	case ColorGroup:
	    s << toColorGroup();
	    break;
	case IconSet:
	    s << toIconSet().pixmap(); //### add stream operator to iconset #ME
	    break;
	case Int:
	    s << toInt();
	    break;
	case UInt:
	    s << toUInt();
	    break;
	case Bool:
	    s << (Q_INT8)toBool();
	    break;
	case Double:
	    s << toDouble();
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
QString QVariant::toString() const
{
    if ( typ == CString )
	return QString::fromLatin1( toCString() );
    if ( typ != String )
	return QString::null;
    return *((QString*)value.ptr);
}

/*!
  Returns the variant as a QCString if the variant has type()
  CString, or a null QCString otherwise.
*/
QCString QVariant::toCString() const
{
    if ( typ == CString )
	return *((QCString*)value.ptr);
    if ( typ == String )
	return ((QString*)value.ptr)->latin1();
    return 0;
}

/*!
  Returns the variant as a QStringList if the variant has type()
  StringList or List, or an empty list otherwise.
*/
QStringList QVariant::toStringList() const
{
    if ( typ == StringList )
	return *((QStringList*)value.ptr);
    if ( typ == List )
    {
	QStringList lst;
	QValueList<QVariant>::ConstIterator it = toList().begin();
	QValueList<QVariant>::ConstIterator end = toList().end();
	for( ; it != end; ++it )
	    lst.append( (*it).toString() );
	return lst;
    }
    return QStringList();
}

/*!
  \fn QMap<QString, QVariant> QVariant::toMap () const

  Returns the variant as a QMap<QString,QVariant> if the variant has type()
  Map, or an empty map otherwise.
*/
QMap<QString,QVariant> QVariant::toMap() const
{
    if ( typ != Map )
	return QMap<QString,QVariant>();
    return *((QMap<QString,QVariant>*)value.ptr);
}

/*!
  Returns the variant as a QFont if the variant has type()
  Font, or the default font otherwise.
*/
QFont QVariant::toFont() const
{
    if ( typ != Font )
	return QFont();
    return *((QFont*)value.ptr);
}

/*!
  Returns the variant as a QPixmap if the variant has type()
  Pixmap, or a null pixmap otherwise.
*/
QPixmap QVariant::toPixmap() const
{
    if ( typ != Pixmap )
	return QPixmap();
    return *((QPixmap*)value.ptr);
}

/*!
  Returns the variant as a QImage if the variant has type()
  Image, or a null image otherwise.
*/
QImage QVariant::toImage() const
{
    if ( typ != Image )
	return QImage();
    return *((QImage*)value.ptr);
}

/*!
  Returns the variant as a QBrush if the variant has type()
  Brush, or a default brush otherwise.
*/
QBrush QVariant::toBrush() const
{
    if( typ != Brush )
	return QBrush();
    return *((QBrush*)value.ptr);
}

/*!
  Returns the variant as a QPoint if the variant has type()
  Point, or a the point (0,0) otherwise.
*/
QPoint QVariant::toPoint() const
{
    if ( typ != Point )
	return QPoint();
    return *((QPoint*)value.ptr);
}

/*!
  Returns the variant as a QRect if the variant has type()
  Rect, or an empty rectangle otherwise.
*/
QRect QVariant::toRect() const
{
    if ( typ != Rect )
	return QRect();
    return *((QRect*)value.ptr);
}

/*!
  Returns the variant as a QSize if the variant has type()
  Size, or an invalid size otherwise.
*/
QSize QVariant::toSize() const
{
    if ( typ != Size )
	return QSize();
    return *((QSize*)value.ptr);
}

/*!
  Returns the variant as a QColor if the variant has type()
  Color, or an invalid color otherwise.
*/
QColor QVariant::toColor() const
{
    if ( typ != Color )
	return QColor();
    return *((QColor*)value.ptr);
}

/*!
  Returns the variant as a QPalette if the variant has type()
  Palette, or a completely black palette otherwise.
*/
QPalette QVariant::toPalette() const
{
    if ( typ != Palette )
	return QPalette();
    return *((QPalette*)value.ptr);
}

/*!
  Returns the variant as a QColorGroup if the variant has type()
  ColorGroup, or a completely black color group otherwise.
*/
QColorGroup QVariant::toColorGroup() const
{
    if ( typ != ColorGroup )
	return QColorGroup();
    return *((QColorGroup*)value.ptr);
}

/*!
  Returns the variant as a QIconSet if the variant has type()
  IconSet, or an icon set of null pixmaps otherwise.
*/
QIconSet QVariant::toIconSet() const
{
    if ( typ != IconSet )
	return QIconSet();
    return *((QIconSet*)value.ptr);
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
    return 0.0;
}

/*!
  Returns the variant as a QValueList<QVariant> if the variant has type()
  List or StringList, or an empty list otherwise.
*/
QValueList<QVariant> QVariant::toList() const
{
    if ( typ == List )
	return *((QValueList<QVariant>*)value.ptr);
    if ( typ == StringList )
    {
	QValueList<QVariant> lst;
	QStringList::ConstIterator it = toStringList().begin();
	QStringList::ConstIterator end = toStringList().end();
	for( ; it != end; ++it )
	    lst.append( QVariant( *it ) );
	return lst;
    }
    return QValueList<QVariant>();
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
  <li> List -> StringList
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
    return FALSE;
}
