/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qformatstuff.cpp#12 $
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

#include <qtextstream.h>
#include <qstylesheet.h>

QtTextCharFormat::QtTextCharFormat()
    : ref( 1 ), customItem_( 0 ), logicalFontSize( 3 )
{
}

QtTextCharFormat::QtTextCharFormat( const QtTextCharFormat &format )
    : font_( format.font_ ), color_( format.color_ ),
      key( format.key ), ref( 1 ), customItem_( 0 ),
      logicalFontSize( format.logicalFontSize )
{
}

QtTextCharFormat::QtTextCharFormat( const QFont &f, const QColor &c, QtTextCustomItem *ci )
    : font_( f ), color_( c ), ref( 1 ), customItem_( ci ), logicalFontSize( 3 )
{
    createKey();
}


void QtTextCharFormat::createKey()
{
    key = QString( "%1_%2_%3_%4_%5_%6_%7_%8" ).
          arg( color_.red() ).arg( color_.green() ).arg( color_.blue() ).
          arg( font_.family() ).arg( font_.pointSize() ).arg( font_.weight() ).
          arg( (int)font_.underline() ).arg( (int)font_.italic() );
}

QtTextCharFormat &QtTextCharFormat::operator=( const QtTextCharFormat &fmt )
{
    font_ = fmt.font_;
    color_ = fmt.color_;
    key = fmt.key;
    ref = 1;
    customItem_ = fmt.customItem_;
    logicalFontSize = fmt.logicalFontSize;

    return *this;
}

bool QtTextCharFormat::operator==( const QtTextCharFormat &format )
{
    return format.key == key;
}

QColor QtTextCharFormat::color() const
{
    return color_;
}

QFont QtTextCharFormat::font() const
{
    return font_;
}

QtTextCustomItem *QtTextCharFormat::customItem() const
{
    return customItem_;
}

int QtTextCharFormat::addRef()
{
    return ++ref;
}

int QtTextCharFormat::removeRef()
{
    return --ref;
}

QtTextCharFormat QtTextCharFormat::makeTextFormat( const QStyleSheetItem *item )
{
    QtTextCharFormat format = *this;
    if ( item->fontWeight() != QStyleSheetItem::Undefined )
        format.font_.setWeight( item->fontWeight() );
    if ( item->fontSize() != QStyleSheetItem::Undefined )
        format.font_.setPointSize( item->fontSize() );
    else if ( item->logicalFontSize() != QStyleSheetItem::Undefined )
        item->styleSheet()->scaleFont( format.font_, item->logicalFontSize() );
    else if ( item->logicalFontSizeStep() != QStyleSheetItem::Undefined )
        item->styleSheet()->scaleFont( format.font_,
                                       logicalFontSize + item->logicalFontSizeStep() );
    if ( !item->fontFamily().isEmpty() )
        format.font_.setFamily( item->fontFamily() );
    if ( item->color().isValid() )
        format.color_ = item->color();
    if ( item->definesFontItalic() )
        format.font_.setItalic( item->fontItalic() );
    if ( item->definesFontUnderline() )
        format.font_.setUnderline( item->fontUnderline() );

    format.createKey();
    return format;
}

QtTextFormatCollection::QtTextFormatCollection()
    : lastRegisterFormat( 0 ), lastRegisterIndex( 0 ),
      lastFormatIndex( 0 ), lastFormatFormat( 0 )
{
}

ushort QtTextFormatCollection::registerFormat( const QtTextCharFormat &format )
{
    if ( lastRegisterFormat ) {
        if ( format.key == lastRegisterFormat->key ) {
            lastRegisterFormat->addRef();
            return lastRegisterIndex;
        }
    }

    if ( cKey.contains( format.key ) ) {
        QtTextCharFormat *f = cKey[ format.key ];
        f->addRef();
        int i = cKeyIndex[ format.key ];
        lastRegisterFormat = f;
        lastRegisterIndex = i;
        return i;
    } else {
        QtTextCharFormat *f = new QtTextCharFormat( format );
        cKey[ f->key ] = f;
        int i = cIndex.count();
        cIndex[ i ] = f;
        cKeyIndex[ f->key ] = i;
        lastRegisterFormat = f;
        lastRegisterIndex = i;
        return i;
    }
}

void QtTextFormatCollection::unregisterFormat( ushort index )
{
    if ( cIndex.contains( index ) ) {
        QString key = cIndex[ index ]->key;
        QtTextCharFormat *f = cKey[ key ];
        int ref = f->removeRef();
        if ( ref <= 0 ) {
            delete f;
            cKey.remove( key );
            cIndex.remove( index );
            cKeyIndex.remove( key );
        }
    }

}

QtTextCharFormat QtTextFormatCollection::format( ushort index )
{
    if ( lastFormatIndex == index && lastFormatFormat )
        return *lastFormatFormat;

    if ( !cIndex[index] ) {
	QtTextCharFormat result;
	return result;
    }

    lastFormatFormat = cIndex[ index ];
    lastFormatIndex = index;

    return *lastFormatFormat;
}

