/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qformatstuff.cpp#2 $
**
** Definition of the QtTextView class
**
** Created : 990101
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

#include "qformatstuff.h"

#include <qdatastream.h>
#include <qcstring.h>
#include <qstylesheet.h>

QtTextCharFormat::QtTextCharFormat()
    : ref( 1 )
{
}

QtTextCharFormat::QtTextCharFormat( const QtTextCharFormat &format )
    : _font( format._font ), _color( format._color ),
      key( format.key ), ref( 1 )
{
}

QtTextCharFormat::QtTextCharFormat( const QFont &f, const QColor &c )
    : _font( f ), _color( c ), ref( 1 )
{
    QDataStream s( key, IO_WriteOnly );
    s << _font << _color;

    qDebug( "key: %s", key.data() );
}

QtTextCharFormat::~QtTextCharFormat()
{
}

QColor QtTextCharFormat::color() const
{
    return _color;
}

QFont QtTextCharFormat::font() const
{
    return _font;
}

int QtTextCharFormat::addRef()
{
    return ++ref;
}

int QtTextCharFormat::removeRef()
{
    return --ref;
}

QtTextCharFormat QtTextCharFormat::makeTextFormat( const QStyleSheetItem &item )
{
    QtTextCharFormat format = *this;
    if ( item.fontWeight() != QStyleSheetItem::Undefined )
        format._font.setWeight( item.fontWeight() );
    if ( item.fontSize() != QStyleSheetItem::Undefined )
        format._font.setPointSize( item.fontSize() );
    if ( !item.fontFamily().isEmpty() )
        format._font.setFamily( item.fontFamily() );
    if ( item.color().isValid() )
        format._color = item.color();
    if ( item.definesFontItalic() )
        format._font.setItalic( item.fontItalic() );
    if ( item.definesFontUnderline() )
        format._font.setUnderline( item.fontUnderline() );

    return format;
}

QtTextCustomItem::~QtTextCustomItem()
{
}

QtTextFormatCollection::QtTextFormatCollection()
{
}

ushort QtTextFormatCollection::registerFormat( const QtTextCharFormat &format )
{
    if ( cKey.contains( format.key ) ) {
        cKey[ format.key ]->addRef();
        return cKeyIndex[ format.key ];
    } else {
        QtTextCharFormat *f = new QtTextCharFormat( format );
        cKey[ f->key ] = f;
        int i = cIndex.count();
        cIndex[ i ] = f;
        cKeyIndex[ f->key ] = i;
        return i;
    }
}

void QtTextFormatCollection::unregisterFormat( ushort index )
{
    if ( cIndex.contains( index ) ) {
        QCString key = cIndex[ index ]->key;
        QtTextCharFormat *f = cKey[ key ];
        int ref = f->removeRef();
        cKey.remove( key );
        cIndex.remove( index );
        cKeyIndex.remove( key );
        if ( ref <= 0 )
            delete f;
    }

}

QtTextCharFormat *QtTextFormatCollection::format( ushort index )
{
    return cIndex[ index ];
}

