/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_win.cpp#5 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Windows
**
** Author  : Haavard Nord
** Created : 940630
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfont.h"
#include "qfontdta.h"
#include "qfontmet.h"
#include "qfontinf.h"
#include "qpaintd.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qfnt_win.cpp#5 $";
#endif


QFont *QFont::defFont = 0;			// default font


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

QFontData::QFontData()
{
    stockFont = FALSE;
    hfont = 0;
}

QFontData::~QFontData()
{
    if ( !stockFont && hfont )
	DeleteObject( hfont );
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
    defFont = new QFont( TRUE );
}

void QFont::cleanup()
{
    delete defFont;
}

void QFont::cacheStatistics()
{
  // Not implemented for Windows
}


QFont::QFont( bool )				// create default font
{
    init();
    d->req.family    = "helvetica";
    d->req.pointSize = 10*14;
}


#define DIRTY_FONT	( d->req.dirty	 )
#define DIRTY_METRICS	( f.d->req.dirty )


HANDLE QFont::handle() const
{
    if ( DIRTY_FONT )
	loadFont();
    return d->hfont;
}

bool QFont::dirty() const
{
    return DIRTY_FONT;
}


QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Helvetica:
	    return "helvetica";
	case Times:
	    return "times";
	case Courier:
	    return "courier";
	case Decorative:
	    return "old english";
	case System:
	    return "helvetica";
	case AnyStyle:
	default:
	    return "helvetica";
    }
}

QString QFont::lastResortFamily() const
{
    return "helvetica";
}

QString QFont::lastResortFont() const
{
    return "system";
}


void QFont::updateFontInfo() const
{
    if ( DIRTY_FONT )
	loadFont();
    d->act = d->req;				// !!!the easy way out
}


void QFont::loadFont() const
{
    if ( !d->stockFont && d->hfont )
	DeleteObject( d->hfont );

    d->exactMatch = TRUE;
    d->lineW	  = 1;
    d->req.dirty  = FALSE;
    d->act.dirty  = TRUE;			// no longer valid

    if ( d->req.rawMode ) {
	QString n = d->req.family;
	int	f;
	n.lower();
	if ( n == (const char *)"system" )
	    f = SYSTEM_FONT;
	else if ( n == (const char *)"ansi_fixed" )
	    f = ANSI_FIXED_FONT;
	else if ( n == (const char *)"ansi_var" )
	    f = ANSI_VAR_FONT;
	else if ( n == (const char *)"device_default" )
	    f = DEVICE_DEFAULT_FONT;
	else if ( n == (const char *)"oem_fixed" )
	    f = OEM_FIXED_FONT;
	else if ( n == (const char *)"system_fixed" )
	    f = SYSTEM_FIXED_FONT;
	else
	    f = SYSTEM_FONT;
	d->hfont = GetStockObject( f );
	d->stockFont = TRUE;
	return;
    }

    int hint = FF_DONTCARE;
    switch ( styleHint() ) {
	case Helvetica:
	    hint = FF_SWISS;
	    break;
	case Times:
	    hint = FF_ROMAN;
	    break;
	case Courier:
	    hint = FF_MODERN;
	    break;
	case OldEnglish:
	    hint = FF_DECORATIVE;
	    break;
	case System:
	    hint = FF_MODERN;
	    break;
	case AnyStyle:
	    break;
    }

    const char *familyName = QFont::substitute( d->req.family );
    d->hfont = CreateFont(
	d->req.pointSize/10,			// height
	0,					// width
	0,					// escapement
	0,					// orientation
	d->req.weight * 10,			// weight
	d->req.italic,				// italic
	d->req.underline,			// underline
	d->req.strikeOut,			// strikeout
	ANSI_CHARSET,				// character set
	OUT_DEFAULT_PRECIS,			// output precision
	CLIP_DEFAULT_PRECIS,			// clip precision
	DEFAULT_QUALITY,			// output quality
	DEFAULT_PITCH | hint,			// pitch and family
	familyName );				// typeface

    if ( d->hfont )
	d->stockFont = FALSE;
    if ( !d->hfont ) {
	d->hfont = GetStockObject( ANSI_VAR_FONT );
	d->stockFont = TRUE;
    }
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

int QFontMetrics::ascent() const
{
//    if ( pdev && pdev->hdc ) {
	TEXTMETRIC tm;
	GetTextMetrics( qt_display_dc(), &tm );
	return tm.tmAscent;
//    }
//    return 0;
}

int QFontMetrics::descent() const
{
//    if ( pdev && pdev->hdc ) {
	TEXTMETRIC tm;
	GetTextMetrics( qt_display_dc(), &tm );
	return tm.tmDescent;
//    }
//    return 0;
}

int QFontMetrics::height() const
{
//    if ( pdev && pdev->hdc ) {
	TEXTMETRIC tm;
	GetTextMetrics( qt_display_dc(), &tm );
	return tm.tmHeight;
//    }
//    return 0;
}

int QFontMetrics::leading() const
{
//    if ( pdev && pdev->hdc ) {
	TEXTMETRIC tm;
	GetTextMetrics( qt_display_dc(), &tm );
	return tm.tmExternalLeading;		// !!! er dette riktig?
//    }
//    return 0;
}

int QFontMetrics::lineSpacing() const
{
//    if ( pdev && pdev->hdc ) {
	TEXTMETRIC tm;
	GetTextMetrics( qt_display_dc(), &tm );
	return tm.tmHeight + tm.tmExternalLeading;
//    }
//    return 0;
}

int QFontMetrics::width( const char *str, int len ) const
{
//    if ( pdev && pdev->hdc ) {
	if ( len < 0 )
	    len = strlen( str );
#if defined(_WS_WIN32_)
	SIZE s;
	GetTextExtentPoint( qt_display_dc(), str, len, &s );
	return s.cx;
#else
	not implemented for win 16
#endif
//    }
//    return 0;
}

QRect QFontMetrics::boundingRect( const char *str, int len ) const
{
 //   if ( pdev && pdev->hdc ) {
	if ( len < 0 )
	    len = strlen( str );
#if defined(_WS_WIN32_)
	TEXTMETRIC tm;
	GetTextMetrics( qt_display_dc(), &tm );
	SIZE s;
	GetTextExtentPoint( qt_display_dc(), str, len, &s );
	return QRect( 0, -tm.tmAscent, s.cy, tm.tmAscent+tm.tmDescent );
#else
	not implemented for win 16
#endif
 //   }
 //   return QRect( 0, 0, 0, 0 );
}

int QFontMetrics::maxWidth() const
{
//    if ( pdev && pdev->hdc ) {
	TEXTMETRIC tm;
	GetTextMetrics( qt_display_dc(), &tm );
	return tm.tmMaxCharWidth;
//    }
//    return 0;
}

int QFontMetrics::underlinePos() const
{
    return 2;
}

int QFontMetrics::strikeOutPos() const
{
    return -2;
}

int QFontMetrics::lineWidth() const
{
    return 1;
}
