/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qproperty.h#1 $
**
** Definition of QProperty class
**
** Created : 930419
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

#ifndef QPROPERTY_H
#define QPROPERTY_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

class QString;
class QFont;
class QPixmap;
class QMovie;
class QBrush;
class QRect;
class QSize;
class QColor;
class QPalette;
class QColorGroup;

class Q_EXPORT QProperty
{
public:
    enum Type {
      Empty,
      StringType,
      StringListType,
      FontType,
      MovieType,
      PixmapType,
      BrushType,
      RectType,
      SizeType,
      ColorType,
      PaletteType,
      ColorGroupType,
      IntType,
      BoolType,
      DoubleType, 
      CustomType = 0x1000
    };
  
    QProperty();
    QProperty( const QProperty& );
  
    QProperty& operator= ( const QProperty& );
    
    void setValue( const QString& );
    void setValue( const QStringList& );
    void setValue( const QFont& );
    void setValue( const QPixmap& );
    void setValue( const QMovie& );
    void setValue( const QBrush& );
    void setValue( const QRect& );
    void setValue( const QSize& );
    void setValue( const QColor& );
    void setValue( const QPalette& );
    void setValue( const QColorGroup& );
    void setValue( int );
    void setValue( bool );
    void setValue( double );

    Type type() const { return typ; }
    QString typeName() const;
  
    bool isEmpty() const { return ( typ == Empty ); }
  
    const QString& stringValue() const { ASSERT( typ == StringType ); return *((QString*)val.ptr); }
    const QStringList& stringListValue() const { ASSERT( typ == StringType ); return *((QStringList*)val.ptr); }
    const QFont& fontValue() const { ASSERT( typ == FontType ); return *((QFont*)val.ptr); }
    const QPixmap& pixmapValue() const { ASSERT( typ == PixmapType ); return *((QPixmap*)val.ptr); }
    const QMovie& movieValue() const { ASSERT( typ == MovieType ); return *((QMovie*)val.ptr); }
    const QBrush& brushValue() const { ASSERT( typ == BrushType ); return *((QBrush*)val.ptr); }
    const QRect& rectValue() const { ASSERT( typ == RectType ); return *((QRect*)val.ptr); }
    const QSize& sizeValue() const { ASSERT( typ == SizeType ); return *((QSize*)val.ptr); }
    const QColor& colorValue() const { ASSERT( typ == ColorType ); return *((QColor*)val.ptr); }
    const QPalette& paletteValue() const { ASSERT( typ == PaletteType ); return *((QPalette*)val.ptr); }
    const QColorGroup& colorgroupValue() const { ASSERT( typ == ColorGroupType );
                                               return *((QColorGroup*)val.ptr); }
    int intValue() const { ASSERT( typ == IntType ); return val.i; }
    bool boolValue() const { ASSERT( typ == BoolType ); return val.b; }
    double doubleValue() const { ASSERT( typ == DoubleType ); return val.d; }

    static QString typeToName( Type _typ );
    static Type nameToType( const QString& _name );
  
protected:
    virtual void clear();
  
    Type typ;
    union
    {
      int i;
      bool b;
      double d;
      void *ptr;
    } val;
};

#endif
