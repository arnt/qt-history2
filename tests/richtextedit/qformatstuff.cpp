/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qformatstuff.cpp#14 $
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
    : ref( 1 ), logicalFontSize( 3 ), custom( 0 )
{
}

QtTextCharFormat::QtTextCharFormat( const QtTextCharFormat &format )
    : font_( format.font_ ), color_( format.color_ ),
      key( format.key ), ref( 1 ),
      logicalFontSize( format.logicalFontSize ),
      anchor_href( format.anchor_href ),
      anchor_name( format.anchor_name ),
      parent(0), custom( format.custom )
{
}

QtTextCharFormat::QtTextCharFormat( const QFont &f, const QColor &c )
    : font_( f ), color_( c ), ref( 1 ), logicalFontSize( 3 ),parent(0), custom( 0 )
{
    createKey();
}


 
QtTextCharFormat::~QtTextCharFormat()
{
}



void QtTextCharFormat::createKey()
{
    //TODO speedup avoiding argv
    key = QString( "%1_%2_%3_%4_%5_%6_%7_%8_%9_%10_%11" ).
	  arg(anchor_href).arg(anchor_name).
	  arg( color_.red() ).arg( color_.green() ).arg( color_.blue() ).
	  arg( font_.family() ).arg( font_.pointSize() ).arg( font_.weight() ).
	  arg( (int)font_.underline() ).arg( (int)font_.italic()).arg((ulong)custom);
}

QtTextCharFormat &QtTextCharFormat::operator=( const QtTextCharFormat &fmt )
{
    font_ = fmt.font_;
    color_ = fmt.color_;
    key = fmt.key;
    ref = 1;
    logicalFontSize = fmt.logicalFontSize;
    anchor_href = fmt.anchor_href;
    anchor_name = fmt.anchor_name;
    custom = fmt.custom;
    return *this;
}

bool QtTextCharFormat::operator==( const QtTextCharFormat &format )
{
    return format.key == key;
}

int QtTextCharFormat::addRef()
{
    return ++ref;
}

int QtTextCharFormat::removeRef()
{
    return --ref;
}

QtTextCharFormat QtTextCharFormat::makeTextFormat( const QStyleSheetItem *style, 
						   const QMap<QString,QString>& attr,
						   QtTextCustomItem*  item )
{
    QtTextCharFormat format(*this);
    format.custom = item;
    if ( style->isAnchor() ) {
	format.anchor_href = attr["href"];
	format.anchor_name = attr["name"];
    }
    
    if ( style->fontWeight() != QStyleSheetItem::Undefined )
        format.font_.setWeight( style->fontWeight() );
    if ( style->fontSize() != QStyleSheetItem::Undefined )
        format.font_.setPointSize( style->fontSize() );
    else if ( style->logicalFontSize() != QStyleSheetItem::Undefined )
        style->styleSheet()->scaleFont( format.font_, style->logicalFontSize() );
    else if ( style->logicalFontSizeStep() != QStyleSheetItem::Undefined )
        style->styleSheet()->scaleFont( format.font_,
                                       logicalFontSize + style->logicalFontSizeStep() );
    if ( !style->fontFamily().isEmpty() )
        format.font_.setFamily( style->fontFamily() );
    if ( style->color().isValid() )
        format.color_ = style->color();
    if ( style->definesFontItalic() )
        format.font_.setItalic( style->fontItalic() );
    if ( style->definesFontUnderline() )
        format.font_.setUnderline( style->fontUnderline() );

    format.createKey();
    return format;
}

QtTextFormatCollection::QtTextFormatCollection()
    : lastRegisterFormat( 0 )
{
}

QtTextCharFormat* QtTextFormatCollection::registerFormat( const QtTextCharFormat &format )
{
    if ( format.customItem() )
	qDebug("register format with  customItem");
    if ( lastRegisterFormat ) {
        if ( format.key == lastRegisterFormat->key ) {
	    lastRegisterFormat->addRef();
	    return lastRegisterFormat;
        }
    }
    
    if ( format.parent == this ) {
	QtTextCharFormat* f = ( QtTextCharFormat*) &format;
	f->addRef();
	lastRegisterFormat = f;
	return f;
    }

    QtTextCharFormat *fc = cKey[ format.key ];
    if ( fc ) {
	fc->addRef();
	lastRegisterFormat = fc;
	return fc;
    } else {
	QtTextCharFormat *f = new QtTextCharFormat( format );
	f->parent = this;
	cKey[ f->key ] = f;
	lastRegisterFormat = f;
	return f;
    }
}

void QtTextFormatCollection::unregisterFormat( const QtTextCharFormat &format )
{
    QtTextCharFormat* f  = 0;
    
    if ( format.parent == this )
	f = ( QtTextCharFormat*)&format;
    else if ( cKey.contains( format.key ) ) 
	f = cKey[ format.key ];

    if ( f ) {
	int ref = f->removeRef();
	if ( ref <= 0 ) {
	    if ( f == lastRegisterFormat )
		lastRegisterFormat = 0;
	    cKey.remove( format.key );
	    delete f;
	}
    }
}

