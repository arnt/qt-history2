/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_os2.cpp#2 $
**
** Implementation of QFont and QFontMetrics classes for OS/2 PM
**
** Author  : Haavard Nord
** Created : 940712
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#include "qpainter.h"
#define	 INCL_WIN
#include <os2.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfnt_os2.cpp#2 $";
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



bool QFont::changeFont( const char *name )
{
    bool res = TRUE;
//    QPainter::changedFont( this, TRUE );	// tell painter about new font
    return res;
}


// --------------------------------------------------------------------------
// QFontMetrics member functions
//

QFontMetrics::QFontMetrics( const QFont &font )
{
}

int QFontMetrics::ascent() const
{
    return 0;
}

int QFontMetrics::descent() const
{
    return 0;
}

int QFontMetrics::height() const
{
    return 0;
}


int QFontMetrics::width( const char *str, int len ) const
{
    return 0;
}
