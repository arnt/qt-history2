/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_x11.cpp#4 $
**
** Implementation of QFont and QFontMetrics classes for X11
**
** Author  : Haavard Nord
** Created : 940515
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#define QXFontStruct XFontStruct
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfont_x11.cpp#4 $";
#endif


// --------------------------------------------------------------------------
// QFont dictionary to make font loading faster than using XLoadQueryFont
//

#include "qdict.h"

typedef declare(QDictM,XFontStruct) QFontDict;

static QFontDict *fontDict = 0;			// dict of loaded fonts


// --------------------------------------------------------------------------
// QFont member functions
//

void QFont::initialize()			// called from startup routines
{
    fontDict = new QFontDict( 29 );
    CHECK_PTR( fontDict );
}

void QFont::cleanup()
{
    delete fontDict;
}

#define DEFAULT_FONT "6x13"

QFont::QFont()
{
    data = new QFontData;
    CHECK_PTR( data );
    if ( fontDict ) {
	XFontStruct *f = fontDict->find(DEFAULT_FONT);
	if ( !f ) {
	    f = XLoadQueryFont( qXDisplay(), DEFAULT_FONT );
#if defined(CHECK_NULL)
	    if ( !f )
		fatal( "QFont: Cannot find default font" );
#endif
	    fontDict->insert( DEFAULT_FONT, f );
	}
	data->name = DEFAULT_FONT;
	data->f = f;
    }
    else {
#if defined(CHECK_STATE)
	warning( "QFont: Not ready to set font" );
#endif
	data->f = 0;
    }
}

QFont::QFont( const char *name )
{
    data = new QFontData;
    CHECK_PTR( data );
    changeFont( name );
}

QFont::QFont( const QFont &font )
{
    data = font.data;
    data->ref();
}

QFont::~QFont()
{
    if ( data->deref() )
	delete data;
}

QFont &QFont::operator=( const QFont &font )
{
    font.data->ref();
    if ( data->deref() )
	delete data;
    data = font.data;
    return *this;
}


QFont QFont::copy() const
{
    QFont f( data->name );
    return f;
}


char *QFont::name() const			// get font name
{
    return data->name;
}

bool QFont::changeFont( const char *name )
{
    if ( !fontDict ) {
#if defined(CHECK_STATE)
	warning( "QFont::changeFont: Not ready to set font %s", name );
#endif
	return FALSE;
    }
    bool res = TRUE;
    XFontStruct *f = fontDict->find(name);
    if ( !f ) {					// font is not cached
	f = XLoadQueryFont( qXDisplay(), name );
	if ( f )				// save for later
	    fontDict->insert( name, f );
	else {
	    QFont dummy;			// creates default font
	    dummy = dummy;			// avoid optimization/warning
	    res = FALSE;
	    f = fontDict->find( DEFAULT_FONT );
	}
    }
    data->name = name;
    data->f = f;
    QPainter::changedFont( this, TRUE );	// tell painter about new font
    return res;
}


bool QFont::operator==( const QFont &f ) const
{
    return (f.data == data) || (f.data->name == data->name);
}


Font QFont::fontId() const
{
    return data->f->fid;
}


// --------------------------------------------------------------------------
// QFont stream functions
//

#include "qdstream.h"

QDataStream &operator<<( QDataStream &s, const QFont &f )
{
    return s << f.name();
}

QDataStream &operator>>( QDataStream &s, QFont &f )
{
    char *name;
    s >> name;
    f = QFont( name );
    delete name;
    return s;
}


// --------------------------------------------------------------------------
// QFontMetrics member functions
//

QFontMetrics::QFontMetrics( const QFont &font )
{
    f = font.data->f;
}

int QFontMetrics::ascent() const
{
    return f->ascent;
}

int QFontMetrics::descent() const
{
    return f->descent;
}

int QFontMetrics::height() const
{
    return f->ascent + f->descent;
}


int QFontMetrics::width( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    return XTextWidth( f, str, len );
}
