/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_win.cpp#18 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32
**
** Author  : Haavard Nord
** Created : 940630
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfont.h"
#include "qfontdta.h"
#include "qfontmet.h"
#include "qfontinf.h"
#include "qwidget.h"
#include "qpainter.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

extern WindowsVersion qt_winver;		// defined in qapp_win.cpp

RCSTAG("$Id: //depot/qt/main/src/kernel/qfont_win.cpp#18 $")


QFont *QFont::defFont = 0;			// default font


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

QFontData::QFontData()
{
    stockFont = TRUE;
    hfont = hdc = 0;
    tm = 0;
}

QFontData::~QFontData()
{
    if ( tm )
	delete tm;
    if ( hfont && !stockFont ) {
#if defined(DEBUG)
	ASSERT( hdc );
#endif
	DeleteObject( SelectObject(hdc,GetStockObject(SYSTEM_FONT)) );
    }
    if ( hdc )
	DeleteDC( hdc );
}

QFontData &QFontData::operator=( const QFontData &d )
{
    req = d.req;
    lineW = d.lineW;
    req.dirty = TRUE;				// reload font later
    if ( tm )
	delete tm;
    if ( hfont && !stockFont ) {
#if defined(DEBUG)
	ASSERT( hdc );
#endif
	DeleteObject( SelectObject(hdc,GetStockObject(SYSTEM_FONT)) );
    }
    if ( hdc )
	DeleteDC( hdc );

    stockFont = TRUE;
    hdc = hfont = 0;
    tm = 0;
    return *this;
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
    if ( !defFont )
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


QFont::QFont( bool )
{
    init();
    d->req.family    = "MS Sans Serif";		// default font
    d->req.pointSize = 8*10;			// approximate point size, hack
    d->req.weight    = QFont::Normal;
}


#define DIRTY_FONT (d->req.dirty)


HANDLE QFont::handle( HANDLE output_hdc ) const
{
    if ( DIRTY_FONT )
	loadFont( output_hdc );
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
    return "default";
}


void QFont::updateFontInfo() const
{
    if ( !d->act.dirty )
	return;
    if ( DIRTY_FONT )
	loadFont();
    d->act = d->req;				// !!!the easy way out
}


void QFont::loadFont( HANDLE output_hdc ) const
{
    d->exactMatch = TRUE;
    d->lineW	  = 1;
    d->req.dirty  = FALSE;
    d->act.dirty  = TRUE;			// no longer valid

    if ( d->hdc ) {
	if ( d->hfont && !d->stockFont )
	    DeleteObject( SelectObject(d->hdc,GetStockObject(SYSTEM_FONT)) );
    }
    else
	d->hdc = CreateCompatibleDC( qt_display_dc() );
    if ( d->tm ) {
	delete d->tm;
	d->tm = 0;
    }

    if ( d->req.rawMode ) {
	QString n = QFont::substitute( d->req.family );
	int	f, deffnt;

	if ( qt_winver == WV_NT )
	    deffnt = SYSTEM_FONT;		// Windows NT 3.x
	else if ( qt_winver == WV_32s )
	    deffnt = SYSTEM_FONT;		// Win32s
	else {
#if !defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif
	    deffnt = DEFAULT_GUI_FONT;		// Windows 95
	}
	n.lower();
	if ( n == "default" )
	    f = deffnt;
	else if ( n == "system" )
	    f = SYSTEM_FONT;
	else if ( n == "system_fixed" )
	    f = SYSTEM_FIXED_FONT;
	else if ( n == "ansi_fixed" )
	    f = ANSI_FIXED_FONT;
	else if ( n == "ansi_var" )
	    f = ANSI_VAR_FONT;
	else if ( n == "device_default" )
	    f = DEVICE_DEFAULT_FONT;
	else if ( n == "oem_fixed" )
	    f = OEM_FIXED_FONT;
	else if ( n[0] == '#' ) {
	    n = n.right( n.length()-1 );
	    f = n.toInt();
	}
	else
	    f = deffnt;
	d->hfont = GetStockObject( f );
	SelectObject( d->hdc, d->hfont );
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
    int weight;
    if ( d->req.weight == 50 )
	weight = FW_DONTCARE;
    else
	weight = (d->req.weight*900)/99;
    int height;
    if ( output_hdc )
	height = d->req.pointSize*GetDeviceCaps(output_hdc,LOGPIXELSY)/72;
    else
	height = d->req.pointSize;

    d->hfont = CreateFont(
	-height/10,				// height
	0,					// width
	0,					// escapement
	0,					// orientation
	weight,					// weight
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
    else {
	d->hfont = GetStockObject( ANSI_VAR_FONT );
	d->stockFont = TRUE;
    }
    SelectObject( d->hdc, d->hfont );
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

class InternalPainter : public QPainter		// Access to QPainter::tm
{
public:
    void *textMetric() const	   { return tm; }
    void  setTextMetric( void *x ) { tm = x; }
};

static void *get_tm( QWidget *w, QPainter *p )
{
    if ( w == 0 && p == 0 ) {
#if defined(CHECK_NULL)
	warning( "QFontMetrics: Invalid font metrics" );
#endif
	return 0;
    }
    void *tm;
    if ( w ) {
	QFont *f = (QFont *)&w->font();
	f->handle();
	tm = f->d->tm;
	if ( !tm ) {
	    tm = new TEXTMETRIC;
	    GetTextMetrics( f->d->hdc, (TEXTMETRIC*)tm );
	    f->d->tm = tm;
	}
    } else if ( p ) {
	InternalPainter *ip = (InternalPainter*)p;
	tm = ip->textMetric();
	if ( !tm ) {
	    tm = new TEXTMETRIC;
	    GetTextMetrics( ip->handle(), (TEXTMETRIC*)tm );
	    ip->setTextMetric( tm );
	}
    } else {
	tm = 0;
    }
    return tm;
}

int QFontMetrics::ascent() const
{
    TEXTMETRIC *tm = (TEXTMETRIC *)get_tm( w, p );
    return tm ? tm->tmAscent : 0;
}

int QFontMetrics::descent() const
{
    TEXTMETRIC *tm = (TEXTMETRIC *)get_tm( w, p );
    return tm ? tm->tmDescent : 0;
}

int QFontMetrics::height() const
{
    TEXTMETRIC *tm = (TEXTMETRIC *)get_tm( w, p );
    return tm ? tm->tmHeight : 0;
}

int QFontMetrics::leading() const
{
    TEXTMETRIC *tm = (TEXTMETRIC *)get_tm( w, p );
    return tm ? tm->tmExternalLeading : 0;
}

int QFontMetrics::lineSpacing() const
{
    TEXTMETRIC *tm = (TEXTMETRIC *)get_tm( w, p );
    return tm ? tm->tmHeight + tm->tmExternalLeading : 0;
}

int QFontMetrics::width( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    HDC hdc;
    if ( p ) {
	hdc = p->handle();
    } else if ( w ) {
	QFont f = w->font();
	f.handle();
	hdc = f.d->hdc;
    } else {
#if defined(CHECK_NULL)
	warning( "QFontMetrics: Invalid font metrics" );
#endif
	return 0;
    }

    SIZE s;
    GetTextExtentPoint( hdc, str, len, &s );
    return s.cx;
}

QRect QFontMetrics::boundingRect( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    HDC hdc;
    TEXTMETRIC *tm = (TEXTMETRIC *)get_tm( w, p );
    if ( !tm )
	return QRect( 0, 0, 0, 0 );
    if ( p ) {
	hdc = p->handle();
    } else if ( w ) {
	QFont f = w->font();
	f.handle();
	hdc = f.d->hdc;
    } else {
#if defined(CHECK_NULL)
	warning( "QFontMetrics: Invalid font metrics" );
#endif
	return QRect( 0, 0, 0, 0 );
    }

    SIZE s;
    GetTextExtentPoint32( hdc, str, len, &s );
    return QRect( 0, -tm->tmAscent, s.cx, tm->tmAscent+tm->tmDescent );
}

int QFontMetrics::maxWidth() const
{
    TEXTMETRIC *tm = (TEXTMETRIC *)get_tm( w, p );
    return tm ? tm->tmMaxCharWidth : 0;
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
