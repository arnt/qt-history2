/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_win.cpp#1 $
**
** Implementation of QFont and QFontMetrics classes for Windows + NT
**
** Author  : Haavard Nord
** Created : 940630
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qpainter.h"
#include "qstring.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfnt_win.cpp#1 $";
#endif


void QFont::initialize()			// called from startup routines
{
}

void QFont::cleanup()
{
}

QFont::QFont()
{
    data = new QFontData;
    CHECK_PTR( data );
    data->hfont = GetStockObject( SYSTEM_FONT );
}

QFont::QFont( const char *name )
{
    data = new QFontData;
    CHECK_PTR( data );
    data->hfont = 0;
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



bool QFont::changeFont( const char *name )
{
    data->hfont = GetStockObject( SYSTEM_FONT );
    QPainter::changedFont( this, TRUE );	// tell painter about new font
    return TRUE;
}


// --------------------------------------------------------------------------
// QFontMetrics member functions
//

QFontMetrics::QFontMetrics( const QFont &font )
{
}

int QFontMetrics::ascent() const
{
    return 8;
}

int QFontMetrics::descent() const
{
    return 8;
}

int QFontMetrics::height() const
{
    return 8;
}


int QFontMetrics::width( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    return len*8;
}
