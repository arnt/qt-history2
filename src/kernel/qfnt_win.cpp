/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfnt_win.cpp#46 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32
**
** Created : 940630
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
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
#include <limits.h>

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

extern WindowsVersion qt_winver;		// defined in qapp_win.cpp

RCSTAG("$Id: //depot/qt/main/src/kernel/qfnt_win.cpp#46 $");


static HANDLE stock_sysfont = 0;

static inline HANDLE systemFont()
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
    bool	    dirty()      const;
    const char	   *key()	 const;
    HANDLE	    dc()	 const;
    HANDLE	    font()	 const;
    TEXTMETRIC	   *textMetric() const;
    const QFontDef *spec()	 const;
    int		    lineWidth()  const;
    void	    reset();
private:
    QFontInternal( const QString & );
    QString	k;
    HANDLE	hdc;
    HANDLE	hfont;
    bool	stockFont;
    TEXTMETRIC	tm;
    QFontDef	s;
    int		lw;
    friend void QFont::load(HANDLE) const;
    friend void QFont::initFontInfo() const;
};

inline QFontInternal::QFontInternal( const QString &key )
    : k(key), hdc(0), hfont(0)
{
    s.dirty = TRUE;
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

inline const QFontDef *QFontInternal::spec() const
{
    return &s;
}

inline int QFontInternal::lineWidth() const
{
    return lw;
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
    d->req.pointSize = 8*10;
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


void QFont::initFontInfo() const
{
    QFontInternal *f = d->fin;
    if ( !f->s.dirty )				// already initialized
	return;
    f->lw = 1;
    f->s = d->req;				// most settings are equal
    char n[64];
    GetTextFace( f->dc(), 64, n );
    if ( stricmp(f->s.family,n) != 0 )
	f->s.family = n;
    f->s.dirty = FALSE;
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
	    fontDict->insert( d->fin->key(), d->fin );
	}
    }
    if ( !d->fin->font() ) {			// font not loaded
	d->fin->hdc   = CreateCompatibleDC( qt_display_dc() );
	d->fin->hfont = create( &d->fin->stockFont, 0 );
	SelectObject( d->fin->hdc, d->fin->hfont );
	GetTextMetrics( d->fin->hdc, &d->fin->tm );
	fontCache->insert( d->fin->key(), d->fin, 1 );
	initFontInfo();
    }
    d->exactMatch = TRUE;
    d->req.dirty = FALSE;
}


#if !defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif


HANDLE QFont::create( bool *stockFont, HANDLE hdc ) const
{
    QString fam = QFont::substitute( d->req.family );
    if ( d->req.rawMode ) {			// will choose a stock font
	int f, deffnt;
	if ( qt_winver == WV_NT || qt_winver == WV_32s )
	    deffnt = SYSTEM_FONT;		// Windows NT or Win32s
	else
	    deffnt = DEFAULT_GUI_FONT;		// Windows 95
	fam = fam.lower();
	if ( fam == "default" )
	    f = deffnt;
	else if ( fam == "system" )
	    f = SYSTEM_FONT;
	else if ( fam == "system_fixed" )
	    f = SYSTEM_FIXED_FONT;
	else if ( fam == "ansi_fixed" )
	    f = ANSI_FIXED_FONT;
	else if ( fam == "ansi_var" )
	    f = ANSI_VAR_FONT;
	else if ( fam == "device_default" )
	    f = DEVICE_DEFAULT_FONT;
	else if ( fam == "oem_fixed" )
	    f = OEM_FIXED_FONT;
	else if ( fam[0] == '#' )
	    f = fam.right(fam.length()-1).toInt();
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
    int cs;
    switch ( charSet() ) {
	case AnyCharSet:
	case Latin1:
	    cs = ANSI_CHARSET;
	    break;
	case Latin2:
	    cs = EASTEUROPE_CHARSET;
	    break;
	case Latin5:
	    cs = TURKISH_CHARSET;
	    break;
	case Latin7:
	    cs = GREEK_CHARSET;
	    break;
	default:
	    cs = ANSI_CHARSET;
	    break;
    }
    lf.lfCharSet	= cs;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS; 
    lf.lfQuality	= DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | hint;
    strcpy( lf.lfFaceName, fam );
    
    HANDLE hfont = CreateFontIndirect( &lf );
    if ( stockFont )
	*stockFont = hfont == 0;
    if ( hfont == 0 )
	hfont = GetStockObject( ANSI_VAR_FONT );
    return hfont;
}


void *QFont::textMetric() const
{
    if ( DIRTY_FONT ) {
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


const QFontDef *QFontMetrics::spec() const
{
    const QFontDef *s;
    if ( type() == FontInternal ) {
	s = u.f->spec();
    } else if ( type() == Widget && u.w ) {
	QFont *f = (QFont *)&u.w->font();
	f->handle();
	s = f->d->fin->spec();
    } else if ( type() == Painter && u.p ) {
	QFont *f = (QFont *)&u.p->font();
	f->handle();
	s = f->d->fin->spec();
    } else {
	s = 0;
    }
#if defined(CHECK_NULL)
    if ( !s )
	warning( "QFontMetrics: Invalid font metrics" );
#endif
    return s;
}

void *QFontMetrics::textMetric() const
{
    if ( type() == FontInternal ) {
	return u.f->textMetric();
    } else if ( type() == Widget && u.w ) {
	QFont *f = (QFont *)&u.w->font();
	f->handle();
	return f->d->fin->textMetric();
    } else if ( type() == Painter && u.p ) {
	return ((QPainter_Protected*)u.p)->tm();
    } else {
#if defined(CHECK_NULL)
	warning( "QFontMetrics: Invalid font metrics" );
#endif
	return 0;
    }
}

#undef  TM
#define TM (type() == FontInternal ? u.f->textMetric() : (TEXTMETRIC*)textMetric())


int QFontMetrics::ascent() const
{
    return TM->tmAscent;
}

int QFontMetrics::descent() const
{
    return TM->tmDescent;
}


bool QFontMetrics::inFont(char ch) const
{
    TEXTMETRIC *f = TM;
    return (uint)ch >= f->tmFirstChar
        && (uint)ch <= f->tmLastChar;
}


int QFontMetrics::leftBearing(char ch) const
{
    if (TM->tmPitchAndFamily & TMPF_TRUETYPE ) {
	ABC abc;
	GetCharABCWidths(hdc(),ch,ch,&abc);
	return abc.abcA;
    } else {
	ABCFLOAT abc;
	GetCharABCWidthsFloat(hdc(),ch,ch,&abc);
	return abc.abcA;
    }
}


int QFontMetrics::rightBearing(char ch) const
{
    if (TM->tmPitchAndFamily & TMPF_TRUETYPE ) {
	ABC abc;
	GetCharABCWidths(hdc(),ch,ch,&abc);
	return abc.abcC;
    } else {
	ABCFLOAT abc;
	GetCharABCWidthsFloat(hdc(),ch,ch,&abc);
	return abc.abcC;
    }
}


int QFontMetrics::minLeftBearing() const
{
    // Safely cast away const, as we cache rbearing there.
    QFontDef* def = (QFontDef*)spec();

    if ( def->lbearing == SHRT_MIN ) {
	minRightBearing(); // calculates both
    }

    return def->lbearing;
}


int QFontMetrics::minRightBearing() const
{
    // Safely cast away const, as we cache rbearing there.
    QFontDef* def = (QFontDef*)spec();

    if ( def->rbearing == SHRT_MIN ) {
	TEXTMETRIC *tm = TM;
	int n = tm->tmLastChar - tm->tmFirstChar+1;
	int ml;
	int mr;
	if (tm->tmPitchAndFamily & TMPF_TRUETYPE ) {
	    ABCFLOAT *abc = new ABCFLOAT[n];
	    GetCharABCWidthsFloat(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
	    float fml = abc[0].abcA;
	    float fmr = abc[0].abcC;
	    for (int i=1; i<n; i++) {
		fml = QMIN(ml,abc[i].abcA);
		fmr = QMIN(mr,abc[i].abcC);
	    }
	    ml = int(fml-0.9999);
	    mr = int(fmr-0.9999);
	} else {
	    ABC *abc = new ABC[n];
	    GetCharABCWidths(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
	    ml = abc[0].abcA;
	    mr = abc[0].abcC;
	    for (int i=1; i<n; i++) {
		ml = QMIN(ml,abc[i].abcA);
		mr = QMIN(mr,abc[i].abcC);
	    }
	}
	def->lbearing = ml;
	def->rbearing = mr;
	delete [] abc;
    }

    return def->rbearing;
}


int QFontMetrics::height() const
{
    return TM->tmHeight;
}

int QFontMetrics::leading() const
{
    return TM->tmExternalLeading;
}

int QFontMetrics::lineSpacing() const
{
    TEXTMETRIC *tm = TM;
    return tm->tmHeight + tm->tmExternalLeading;
}

int QFontMetrics::width( char ch ) const
{
    char tmp[2];
    tmp[1] = '\0';
    tmp[0] = ch;
    return width( tmp, 1 );
}


int QFontMetrics::width( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    SIZE s;
    if ( type() == FontInternal ) {
	GetTextExtentPoint( u.f->dc(), str, len, &s );
	return s.cx;
    }
    GetTextExtentPoint( hdc(), str, len, &s );
    return s.cx;
}

QRect QFontMetrics::boundingRect( const char *str, int len ) const
{
    if ( len < 0 )
	len = strlen( str );
    SIZE s;
    TEXTMETRIC *tm = TM;
    GetTextExtentPoint32( hdc(), str, len, &s );

    int l = len ? leftBearing(*str) : 0;
    int r = len ? rightBearing(str[len-1]) : 0;
    // To be safer, check bearings of next-to-end characters too.
    if (len > 1 ) {
	int newl = width(*str)+leftBearing(str[1]);
	int newr = rightBearing(str[len-2])-width(str[len-1]);
	if ( l < newl ) l = newl;
	if ( r > newr ) r = newr;
    }

    return QRect(l, -tm->tmAscent, s.cx+r-l, tm->tmAscent+tm->tmDescent);
}


HDC QFontMetrics::hdc() const
{
    if ( type() == FontInternal ) {
	return u.f->dc();
    } else {
	if ( type() == Widget && u.w ) {
	    QFont *f = (QFont *)&u.w->font();
	    f->handle();	    
	    return f->d->fin->dc();
	} else if ( type() == Painter && u.p ) {
	    return u.p->handle();
	} else {
#if defined(CHECK_NULL)
	    warning( "QFontMetrics: Invalid font metrics" );
#endif
	    return 0;
	}
    }
}

int QFontMetrics::maxWidth() const
{
    return TM->tmMaxCharWidth;
}

int QFontMetrics::underlinePos() const
{
    int pos = (lineWidth()*2 + 3)/6;
    return pos ? pos : 1;
}

int QFontMetrics::strikeOutPos() const
{
    int pos = TM->tmAscent/3;
    return pos ? pos : 1;
}

int QFontMetrics::lineWidth() const
{
    if ( type() == FontInternal ) {
	return u.f->lineWidth();
    } else {
	QFont f = font();
	f.handle();
	return f.d->fin->lineWidth();
    }
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

const QFontDef *QFontInfo::spec() const
{
    const QFontDef *s;
    if ( type() == FontInternal ) {
	s = u.f->spec();
    } else if ( type() == Widget && u.w ) {
	QFont *f = (QFont *)&u.w->font();
	f->handle();
	s = f->d->fin->spec();
    } else if ( type() == Painter && u.p ) {
	QFont *f = (QFont *)&u.p->font();
	f->handle();
	s = f->d->fin->spec();
    } else {
	s = 0;
    }
#if defined(CHECK_NULL)
    if ( !s )
	warning( "QFontInfo: Invalid font info" );
#endif
    return s;
}
