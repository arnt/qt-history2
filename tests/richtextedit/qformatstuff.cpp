/****************************************************************************
**
** Definition of the QtTextView class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qformatstuff.h"

#include <qtextstream.h>
#include <qstylesheet.h>

QtTextCharFormat::QtTextCharFormat()
    : ref( 1 ), logicalFontSize( 3 ), stdPointSize( 12 ),
      custom( 0 )
{
}

QtTextCharFormat::QtTextCharFormat( const QtTextCharFormat &format )
    : font_( format.font_ ), color_( format.color_ ),
      key( format.key ), ref( 1 ),
      logicalFontSize( format.logicalFontSize ),
      stdPointSize( format.stdPointSize ),
      anchor_href( format.anchor_href ),
      anchor_name( format.anchor_name ),
      parent(0), custom( format.custom )
{
}

QtTextCharFormat::QtTextCharFormat( const QFont &f, const QColor &c )
    : font_( f ), color_( c ), ref( 1 ), logicalFontSize( 3 ), stdPointSize( f.pointSize() ),
      parent(0), custom( 0 )
{
    createKey();
}



QtTextCharFormat::~QtTextCharFormat()
{
}



void QtTextCharFormat::createKey()
{
    QTextOStream ts( &key );
    ts
	<< font_.pointSize() << "_"
	<< font_.weight() << "_"
	<< (int)font_.underline()
	<< (int) font_.italic()
	<< anchor_href << "_"
	<< anchor_name << "_"
	<< color_.pixel()
	<< font_.family() << "_"
	<<(ulong) custom;
}

QtTextCharFormat &QtTextCharFormat::operator=( const QtTextCharFormat &fmt )
{
    font_ = fmt.font_;
    color_ = fmt.color_;
    key = fmt.key;
    ref = 1;
    logicalFontSize = fmt.logicalFontSize;
    stdPointSize = fmt.stdPointSize;
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
						   QtTextCustomItem*  item ) const
{
    QtTextCharFormat format(*this);
    format.custom = item;
    bool changed = FALSE;
    if ( style ) {
	if ( style->name() == "font") {

	    if ( attr.contains("color") )
		format.color_.setNamedColor( attr["color"] );
	    if ( attr.contains("size") ) {
		QString a = attr["size"];
		int n = a.toInt();
		if ( a[0] == '+' || a[0] == '-' )
		    n += format.logicalFontSize;
		format.logicalFontSize = n;
		format.font_.setPointSize( format.stdPointSize );
		style->styleSheet()->scaleFont( format.font_, format.logicalFontSize );
	    }
	    if ( attr.contains("face") ) {
		QString a = attr["face"];
		if ( a.contains(',') )
		    a = a.left( a.find(',') );
		format.font_.setFamily( a );
	    }
	} else {

	    if ( style->isAnchor() ) {
		format.anchor_href = attr["href"];
		format.anchor_name = attr["name"];
		changed = TRUE;
	    }

	    if ( style->fontWeight() != QStyleSheetItem::Undefined )
		format.font_.setWeight( style->fontWeight() );
	    if ( style->fontSize() != QStyleSheetItem::Undefined )
		format.font_.setPointSize( style->fontSize() );
	    else if ( style->logicalFontSize() != QStyleSheetItem::Undefined ) {
		format.logicalFontSize = style->logicalFontSize();
		format.font_.setPointSize( format.stdPointSize );
		style->styleSheet()->scaleFont( format.font_, format.logicalFontSize );
	    }
	    else if ( style->logicalFontSizeStep() != QStyleSheetItem::Undefined ) {
		format.logicalFontSize += style->logicalFontSizeStep();
		format.font_.setPointSize( format.stdPointSize );
		style->styleSheet()->scaleFont( format.font_, format.logicalFontSize );
	    }
	    if ( !style->fontFamily().isEmpty() )
		format.font_.setFamily( style->fontFamily() );
	    if ( style->color().isValid() )
		format.color_ = style->color();
	    if ( style->definesFontItalic() )
		format.font_.setItalic( style->fontItalic() );
	    if ( style->definesFontUnderline() )
		format.font_.setUnderline( style->fontUnderline() );
	}
    }

    if ( item || font_ != format.font_ || changed || color_ != format.color_) // slight performance improvement
	format.createKey();
    return format;
}


QtTextCharFormat QtTextCharFormat::formatWithoutCustom()
{
    QtTextCharFormat fm( *this );
    fm.custom = 0;
    fm.createKey();
    return fm;
}

QtTextFormatCollection::QtTextFormatCollection()
    : cKey(199),lastRegisterFormat( 0 )
{
}


static int anchorcount = 0;
QtTextCharFormat* QtTextFormatCollection::registerFormat( const QtTextCharFormat &format )
{
    if ( format.parent == this ) {
	QtTextCharFormat* f = ( QtTextCharFormat*) &format;
	f->addRef();
	lastRegisterFormat = f;
	return f;
    }

    if ( lastRegisterFormat ) {
        if ( format.key == lastRegisterFormat->key ) {
	    lastRegisterFormat->addRef();
	    return lastRegisterFormat;
        }
    }

    if ( format.isAnchor() ) {
	// fancy speed optimization: do _not_ share any anchors to keep the map smaller
	// see unregisterFormat()
	++anchorcount;
	lastRegisterFormat =  new QtTextCharFormat( format );
	return lastRegisterFormat;
    }

    QtTextCharFormat *fc = cKey[ format.key ];
    if ( fc ) {
	fc->addRef();
	lastRegisterFormat = fc;
	return fc;
    } else {
	QtTextCharFormat *f = new QtTextCharFormat( format );
	f->parent = this;
	cKey.insert( f->key, f );
	lastRegisterFormat = f;
	return f;
    }
}

void QtTextFormatCollection::unregisterFormat( const QtTextCharFormat &format )
{
    QtTextCharFormat* f  = 0;

    if ( format.isAnchor() ) {
	// fancy speed optimization: do _not_ share any anchors to keep the map smaller
	// see registerFormat()
	f = (QtTextCharFormat*)&format;
	int ref = f->removeRef();
	if ( ref <= 0 ) {
	    if ( f == lastRegisterFormat )
		lastRegisterFormat = 0;
	    --anchorcount;
	    delete f;
	}
	return;
    }

    if ( format.parent == this )
	f = ( QtTextCharFormat*)&format;
    else
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

