/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qproperty.cpp#2 $
**
** Implementation of QProperty class
**
** Created : 930418
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qstring.h"
#include "qfont.h"
#include "qpixmap.h"
#include "qmovie.h"
#include "qbrush.h"
#include "qrect.h"
#include "qsize.h"
#include "qcolor.h"
#include "qpalette.h"
#include "qstringlist.h"

#include "qproperty.h"

/*!
  \class QProperty qproperty.h
  \brief The class needs documentation.

  Not documented.
*/

QProperty::QProperty()
{
  typ = Empty;
}

/*!
  Subclasses which re-implement clear() should reimplement the
  destructor to call clear() - so that the overridden clear() is
  called.
*/
QProperty::~QProperty()
{
    clear();
}

QProperty::QProperty( const QProperty& p )
{
  typ = Empty;
  *this = p;
}

QProperty& QProperty::operator= ( const QProperty& p )
{
  clear();
  
  switch( p.type() )
    {
    case Empty:
      break;
    case StringType:
      val.ptr = new QString( p.stringValue() );
      break;
    case StringListType:
      val.ptr = new QStringList( p.stringListValue() );
      break;
    case FontType:
      val.ptr = new QFont( p.fontValue() );
      break;
    case MovieType:
      val.ptr = new QMovie( p.movieValue() );
      break;
    case PixmapType:
      val.ptr = new QPixmap( p.pixmapValue() );
      break;
    case BrushType:
      val.ptr = new QBrush( p.brushValue() );
      break;
    case RectType:
      val.ptr = new QRect( p.rectValue() );
      break;
    case SizeType:
      val.ptr = new QSize( p.sizeValue() );
      break;
    case ColorType:
      val.ptr = new QColor( p.colorValue() );
      break;
    case PaletteType:
      val.ptr = new QPalette( p.paletteValue() );
      break;
    case ColorGroupType:
      val.ptr = new QColorGroup( p.colorgroupValue() );
      break;
    case IntType:
      val.i = p.intValue();
      break;
    case BoolType:
      val.b = p.boolValue();
      break;
    case DoubleType:
      val.d = p.doubleValue();
      break;
    default:
      ASSERT( 0 );
    }

  typ = p.type();
  
  return *this;
}

QString QProperty::typeName() const
{
  return typeToName( typ );
}

void QProperty::setValue( const QString& _value )
{
  clear();
  typ = StringType;
  val.ptr = new QString( _value );
}

void QProperty::setValue( const QStringList& _value )
{
  clear();
  typ = StringListType;
  val.ptr = new QStringList( _value );
}

void QProperty::setValue( const QFont& _value )
{
  clear();
  typ = FontType;
  val.ptr = new QFont( _value );
}

void QProperty::setValue( const QPixmap& _value )
{
  clear();
  typ = PixmapType;
  val.ptr = new QPixmap( _value );
}

void QProperty::setValue( const QMovie& _value )
{
  clear();
  typ = MovieType;
  val.ptr = new QMovie( _value );
}

void QProperty::setValue( const QBrush& _value )
{
  clear();
  typ = BrushType;
  val.ptr = new QBrush( _value );
}

void QProperty::setValue( const QRect& _value )
{
  clear();
  typ = RectType;
  val.ptr = new QRect( _value );
}

void QProperty::setValue( const QSize& _value )
{
  clear();
  typ = SizeType;
  val.ptr = new QSize( _value );
}

void QProperty::setValue( const QColor& _value )
{
  clear();
  typ = ColorType;
  val.ptr = new QColor( _value );
}

void QProperty::setValue( const QPalette& _value )
{
  clear();
  typ = PaletteType;
  val.ptr = new QPalette( _value );
}

void QProperty::setValue( const QColorGroup& _value )
{
  clear();
  typ = ColorGroupType;
  val.ptr = new QColorGroup( _value );
}

void QProperty::setValue( int _value )
{
  clear();
  typ = IntType;
  val.i = _value;
}

void QProperty::setValue( bool _value )
{
  clear();
  typ = BoolType;
  val.b = _value;
}

void QProperty::setValue( double _value )
{
  clear();
  typ = DoubleType;
  val.d = _value;
}

/*!
  De-allocate, based on the type, producing an Empty property.
*/
void QProperty::clear()
{
  switch( typ )
    {
    case Empty:
    case IntType:
    case BoolType:
    case DoubleType:
      break;
    case StringType:
      delete (QString*)val.ptr;
      break;
    case StringListType:
      delete (QStringList*)val.ptr;
      break;
    case FontType:
      delete (QFont*)val.ptr;
      break;
    case MovieType:
      delete (QMovie*)val.ptr;
      break;
    case PixmapType:
      delete (QPixmap*)val.ptr;
      break;
    case BrushType:
      delete (QBrush*)val.ptr;
      break;
    case RectType:
      delete (QRect*)val.ptr;
      break;
    case SizeType:
      delete (QSize*)val.ptr;
      break;
    case ColorType:
      delete (QColor*)val.ptr;
      break;
    case PaletteType:
      delete (QPalette*)val.ptr;
      break;
    case ColorGroupType:
      delete (QColorGroup*)val.ptr;
      break;
    default:
      ASSERT( 0 );
    }  
  typ = Empty;
}

QString QProperty::typeToName( QProperty::Type _typ )
{
  switch( _typ )
    {
    case Empty:
      return QString();
    case StringType:
      return "QString";
    case StringListType:
      return "QStringList";
    case FontType:
      return "QFont";
    case MovieType:
      return "QMovie";
    case PixmapType:
      return "QPixmap";
    case BrushType:
      return "QBrush";
    case RectType:
      return "QRect";
    case SizeType:
      return "QSize";
    case ColorType:
      return "QColor";
    case PaletteType:
      return "QPalette";
    case ColorGroupType:
      return "QColorGroup";
    case IntType:
      return "int";
    case BoolType:
      return "bool";
    case DoubleType:
      return "double";
    default:
      ASSERT( 0 );
    }

  return QString();
}

/**
 Returns QProperty::Empty if the given name is empty or unknown.
*/
QProperty::Type QProperty::nameToType( const QString& _name )
{
   if ( _name.isEmpty() )
     return Empty;
   
   if ( _name == "QString" ) return StringType;
   if ( _name == "QStringList" ) return StringListType;
   if ( _name == "QFont" ) return FontType;
   if ( _name == "QMovie" ) return MovieType;
   if ( _name == "QPixmap" ) return PixmapType;
   if ( _name == "QBrush" ) return BrushType;
   if ( _name == "QRect" ) return RectType;
   if ( _name == "QSize" ) return SizeType;
   if ( _name == "QColor" ) return ColorType;
   if ( _name == "QPalette" ) return PaletteType;
   if ( _name == "QColorGroup" ) return ColorGroupType;
   if ( _name == "int" ) return IntType;
   if ( _name == "bool" ) return BoolType;
   if ( _name == "double" ) return DoubleType;
 
   return Empty;
}

