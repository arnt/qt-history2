/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_win.cpp#25 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32
**
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
#include "qcache.h"
#include "qdict.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

extern WindowsVersion qt_winver;		// defined in qapp_win.cpp

RCSTAG("$Id: //depot/qt/main/src/kernel/qfont_win.cpp#25 $");


static HANDLE stock_sysfont = 0;

inline static HANDLE systemFont()
{
    if ( stock_sysfont == 0 )
	stock_sysfont = GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}


/*****************************************************************************
  QFontInternal definition and implementation
 *****************************************************************************/

class QFontInternal
{
public:
   ~QFontInternal();
    bool        dirty()      const;
    const char *key()	     const;
    HANDLE	dc()	     const;
    HANDLE	font()	     const;
    TEXTMETRIC *textMetric() const;
    void	reset();
private:
    QFontInternal( const QString & );
    QString	k;
    HANDLE	hdc;
    HANDLE	hfont;
    bool	stockFont;
    TEXTMETRIC	tm;
    friend void QFont::load(HANDLE) const;
};

inline QFontInternal::QFontInternal( const QString &key )
    : k(key)
{
    hdc = hfont = 0;
}

inline bool QFontInternal::dirty() const
{
    return hdc == 0;
}

inline const char *QFontInternal::key() const
{
    return k;
}

inline HANDLE QFontInternal::dc() const
{
    return hdc;
}

inline HANDLE QFontInternal::font() const
{
    return hfont;
}

inline TEXTMETRIC *QFontInternal::textMetric() const
{
    QFontInternal *that = (QFontInternal *)this;
    return &that->tm;
}

void QFontInternal::reset()
{
#if defined(DEBUG)
    ASSERT( (hdc && hfont) || !(hdc || hfont) );
#endif
    if ( hdc ) {
	SelectObject( hdc, systemFont() );
	if ( !stockFont )
	    DeleteObject( hfont );
	DeleteDC( hdc );
	hdc = hfont = 0;
    }
}

inline QFontInternal::~QFontInternal()
{
    reset();
}


static const int fontCacheSize = 120;		// max number of loaded fonts


Q_DECLARE(QCacheM,QFontInternal);		// inherited by QFontCache
typedef Q_DECLARE(QCacheIteratorM,QFontInternal) QFontCacheIt;
typedef Q_DECLARE(QDictM,QFontInternal)		 QFontDict;
typedef Q_DECLARE(QDictIteratorM,QFontInternal)  QFontDictIt;


class QFontCache : public QCacheM(QFontInternal)
{
public:
    QFontCache( int maxCost, int size=17, bool cs=TRUE, bool ck=TRUE )
	: QCacheM(QFontInternal)(maxCost,size,cs,ck) {}
    void deleteItem( GCI );
};

void QFontCache::deleteItem( GCI d )
{
    QFontInternal *fin = (QFontInternal *)d;
    fin->reset();
}


static QFontCache    *fontCache	     = 0;	// cache of loaded fonts
static QFontDict     *fontDict	     = 0;	// dict of all loaded fonts
QFont		     *QFont::defFont = 0;	// default font


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

QFontData::QFontData()
{
    fin = 0;
}

QFontData::~QFontData()
{
  // Font data is cleaned up by font cache and font dict
}

QFontData::QFontData( const QFontData &d )
{
    req = d.req;
    act = d.act;
    exactMatch = d.exactMatch;
    lineW = d.lineW;
    fin = d.fin;				// safe to copy
}

QFontData &QFontData::operator=( const QFontData &d )
{
    req = d.req;
    act = d.act;
    exactMatch = d.exactMatch;
    lineW = d.lineW;
    fin = d.fin;				// safe to copy
    return *this;
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
    if ( fontCache )
	return;
    fontCache = new QFontCache( fontCacheSize, 29, TRUE, FALSE );
    CHECK_PTR( fontCache );
    fontDict  = new QFontDict( 29, TRUE, FALSE );
    CHECK_PTR( fontDict );
    if ( !defFont )
	defFont = new QFont( TRUE );		// create the default font
}

void QFont::cleanup()
{
    delete defFont;
    defFont = 0;
    delete fontCache;
    fontCache = 0;
    fontDict->setAutoDelete( TRUE );
    delete fontDict;
}

void QFont::cacheStatistics()
{
#if defined(DEBUG)
    fontCache->statistics();
    QFontCacheIt it(*fontCache);
    QFontInternal *fin;
    debug( "{" );
    while ( (fin = it.current()) ) {
	++it;
	debug( "   [%s]", fin->key() );
    }
    debug( "}" );
#endif
}


QFont::QFont( bool )
{
    init();
    d->req.family    = "MS Sans Serif";		// default font
    d->req.pointSize = 8*10;			// approximate point size, hack
    d->req.weight    = QFont::Normal;
}


// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT (d->req.dirty || d->fin->dirty())


HANDLE QFont::handle( HANDLE output_hdc ) const
{
    static HANDLE last = 0;
    if ( DIRTY_FONT ) {
	load( output_hdc );
    } else {
	if ( d->fin->font() != last )
	    fontCache->find( d->fin->key() );
    }
    last = d->fin->font();
    return last;
}

bool QFont::dirty() const
{
    return DIRTY_FONT;
}


QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Times:
	    return "times";
	case Courier:
	    return "courier";
	case Decorative:
	    return "old english";
	case Helvetica:
	case System:
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
    return "arial";
}


void QFont::updateFontInfo() const
{
    if ( !d->act.dirty )
	return;
    if ( DIRTY_FONT )
	load();
    d->act = d->req;				// !!!the easy way out
}


void QFont::load( HANDLE ) const
{
    if ( !fontCache )				// not initialized
	return;

    QString k = key();
    d->fin = fontCache->find( k );
    if ( !d->fin ) {				// font not loaded
	d->fin = fontDict->find( k );
	if ( !d->fin ) {			// font was never loaded
	    d->fin = new QFontInternal( k );
	    CHECK_PTR( d->fin );
	    fontDict->insert( k, d->fin );
	}
    }
    if ( !d->fin->font() ) {			// font not loaded
	d->fin->hdc   = CreateCompatibleDC( qt_display_dc() );
	d->fin->hfont = create( &d->fin->stockFont, 0 );
	SelectObject( d->fin->hdc, d->fin->hfont );
	GetTextMetrics( d->fin->hdc, &d->fin->tm );
	fontCache->insert( k, d->fin, 1 );
    }
    d->exactMatch = TRUE;
    d->req.dirty = FALSE;
    d->act.dirty = TRUE;	// actual font information no longer valid
    d->lineW = 1;
}


#if !defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif


HANDLE QFont::create( bool *stockFont, HANDLE hdc ) const
{
    if ( d->req.rawMode ) {			// will choose a stock font
	QString n = QFont::substitute( d->req.family );
	int	f, deffnt;
	if ( qt_winver == WV_NT )
	    deffnt = SYSTEM_FONT;		// Windows NT 3.x
	else if ( qt_winver == WV_32s )
	    deffnt = SYSTEM_FONT;		// Win32s
	else
	    deffnt = DEFAULT_GUI_FONT;		// Windows 95
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
	else if ( n[0] == '#' )
	    n = n.right(n.length()-1).toInt();
	else
	    f = deffnt;
	if ( stockFont )
	    *stockFont = TRUE;
	HANDLE hfont = GetStockObject( f );
	if ( !hfont )
	    hfont = systemFont();
	return hfont;
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
	default:
	    break;
    }

    LOGFONT lf;
    if ( hdc )
	lf.lfHeight = -(d->req.pointSize*GetDeviceCaps(hdc,LOGPIXELSY)/720);
    else
	lf.lfHeight = -(d->req.pointSize/10);
    lf.lfWidth		= 0;
    lf.lfEscapement	= 0;
    lf.lfOrientation	= 0;
    if ( d->req.weight == 50 )
	lf.lfWeight = FW_DONTCARE;
    else
	lf.lfWeight = (d->req.weight*900)/99;
    lf.lfItalic		= d->req.italic;
    lf.lfUnderline	= d->req.underline;
    lf.lfStrikeOut	= d->req.strikeOut;
    lf.lfCharSet	= ANSI_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS; 
    lf.lfQuality	= DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | hint;
    strcpy( lf.lfFaceName, QFont::substitute(d->req.family) );
    
    HANDLE hfont = CreateFontIndirect( &lf );
    if ( stockFont )
	*stockFont = hfont == 0;
    if ( hfont == 0 )
	hfont = GetStockObject( ANSI_VAR_FONT );
    return hfont;
}


void *QFont::textMetric() const
{
    if ( !d->fin ) {
	load();
#if defined(DEBUG)
	ASSERT( d->fin && d->fin->font() );
#endif
    }
    return d->fin->textMetric();
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

class QPainter_Protected : public QPainter	// access to textMetric()
{
public:
    void *tm() { return textMetric(); }
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
	tm = f->d->fin->textMetric();
#if defined(DEBUG)
	ASSERT( tm );
#endif
    } else if ( p ) {
	QPainter_Protected *pp = (QPainter_Protected*)p;
	tm = pp->tm();
#if defined(DEBUG)
	ASSERT( tm );
#endif
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
	hdc = f.d->fin->dc();
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
	hdc = f.d->fin->dc();
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
