/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_win.cpp#98 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32
**
** Created : 940630
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qfont.h"
#include "qfontdata.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qfontdatabase.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qcache.h"
#include "qdict.h"
#include <limits.h>
#include "qt_windows.h"


extern Qt::WindowsVersion qt_winver;		// defined in qapplication_win.cpp

static HDC   shared_dc	    = 0;		// common dc for all fonts
static HFONT shared_dc_font = 0;		// used by Windows 95/98

static HFONT stock_sysfont  = 0;

static inline HFONT systemFont()
{
    if ( stock_sysfont == 0 )
	stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
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
    HDC		    dc()	 const;
    HFONT	    font()	 const;
    TEXTMETRICA	   *textMetricA() const;
    TEXTMETRICW	   *textMetricW() const;
    const QFontDef *spec()	 const;
    int		    lineWidth()  const;
    void	    reset();
private:
    QFontInternal( const QString & );
    QString	k;
    HDC		hdc;
    HFONT	hfont;
    bool	stockFont;
    union {
	TEXTMETRICW	w;
	TEXTMETRICA	a;
    } tm;
    QFontDef	s;
    int		lw;
    friend void QFont::load() const;
    friend void QFont::initFontInfo() const;
};

inline QFontInternal::QFontInternal( const QString &key )
    : k(key), hdc(0), hfont(0)
{
    s.dirty = TRUE;
}

inline bool QFontInternal::dirty() const
{
    return hfont == 0;
}

inline const char* QFontInternal::key() const
{
    return k;
}

inline HDC QFontInternal::dc() const
{
    if ( qt_winver == Qt::WV_NT )
	return hdc;
    ASSERT( shared_dc != 0 && hfont != 0 );
    if ( shared_dc_font != hfont ) {
	SelectObject( shared_dc, hfont );
	shared_dc_font = hfont;
    }
    return shared_dc;
}

inline HFONT QFontInternal::font() const
{
    return hfont;
}

inline TEXTMETRICA *QFontInternal::textMetricA() const
{
    QFontInternal *that = (QFontInternal *)this;
    return &that->tm.a;
}

inline TEXTMETRICW *QFontInternal::textMetricW() const
{
    QFontInternal *that = (QFontInternal *)this;
    return &that->tm.w;
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
    if ( qt_winver == Qt::WV_NT ) {
	if ( hdc ) {				// one DC per font (Win NT)
	    SelectObject( hdc, systemFont() );
	    if ( !stockFont )
		DeleteObject( hfont );
	    DeleteDC( hdc );
	    hdc = 0;
	    hfont = 0;
	}
    } else {
	if ( hfont ) {				// shared DC (Windows 95/98)
	    if ( shared_dc_font == hfont ) {	// this is the current font
		ASSERT( shared_dc != 0 );
		SelectObject( shared_dc, systemFont() );
		shared_dc_font = 0;
	    }
	    if ( !stockFont )
		DeleteObject( hfont );
	    hfont = 0;
	}
    }
}

inline QFontInternal::~QFontInternal()
{
    reset();
}


static const int fontCacheSize = 120;		// max number of loaded fonts


typedef QCacheIterator<QFontInternal> QFontCacheIt;
typedef QDict<QFontInternal>	      QFontDict;
typedef QDictIterator<QFontInternal>  QFontDictIt;


class QFontCache : public QCache<QFontInternal>
{
public:
    QFontCache( int maxCost, int size=17, bool cs=TRUE, bool ck=TRUE )
	: QCache<QFontInternal>(maxCost,size,cs,ck) {}
    void deleteItem( Item );
};

void QFontCache::deleteItem( Item d )
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
    shared_dc = CreateCompatibleDC( qt_display_dc() );
    shared_dc_font = 0;
    fontCache = new QFontCache( fontCacheSize, 29, TRUE, FALSE );
    CHECK_PTR( fontCache );
    fontDict  = new QFontDict( 29, TRUE, FALSE );
    CHECK_PTR( fontDict );
    if ( !defFont )
	defFont = new QFont( QFI0 );		// create the default font
}

void QFont::cleanup()
{
    delete defFont;
    defFont = 0;
    delete fontCache;
    fontCache = 0;
    fontDict->setAutoDelete( TRUE );
    delete fontDict;
    ASSERT( shared_dc_font == 0 );
    DeleteDC( shared_dc );
    shared_dc = 0;
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


QFont::QFont( Internal )
{
    init();
    d->req.family    = "MS Sans Serif";		// default font
    d->req.pointSize = 8*10;
    d->req.weight    = QFont::Normal;
}


// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT (d->req.dirty || d->fin->dirty())


HFONT QFont::handle() const
{
    static HFONT last = 0;
    if ( DIRTY_FONT ) {
	load();
    } else {
	if ( d->fin->font() != last )
	    fontCache->find( d->fin->key() );
    }
    last = d->fin->font();
    return last;
}

QString QFont::rawName() const
{
    return family();
}


bool QFont::dirty() const
{
    return DIRTY_FONT;
}


QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Times:
	    return "Times New Roman";
	case Courier:
	    return "Courier New";
	case Decorative:
	    return "Bookman Old Style";
	case Helvetica:
	    return "Arial";
	case System:
	default:
	    return "MS Sans Serif";
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
    if ( qt_winver == Qt::WV_NT ) {
	TCHAR n[64];
	GetTextFace( f->dc(), 64, n );
	f->s.family = qt_winQString(n);
    } else {
	char an[64];
	GetTextFaceA( f->dc(), 64, an );
	f->s.family = an;
    }
    f->s.dirty = FALSE;
}


void QFont::load() const
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
	if ( qt_winver == Qt::WV_NT )
	    d->fin->hdc = GetDC(0);
	d->fin->hfont = create( &d->fin->stockFont, 0 );
	SelectObject( d->fin->dc(), d->fin->hfont );
#ifdef UNICODE
	if ( qt_winver == Qt::WV_NT ) {
	    GetTextMetricsW( d->fin->dc(), &d->fin->tm.w );
	} else
#endif
	{
	    GetTextMetricsA( d->fin->dc(), &d->fin->tm.a );
	}
	fontCache->insert( d->fin->key(), d->fin, 1 );
	initFontInfo();
    }
    d->exactMatch = TRUE;
    d->req.dirty = FALSE;
}


#if !defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif


HFONT QFont::create( bool *stockFont, HDC hdc, bool VxF ) const
{
    QString fam = QFont::substitute( d->req.family );
    if ( d->req.rawMode ) {			// will choose a stock font
	int f, deffnt;
	if ( qt_winver == Qt::WV_NT || qt_winver == Qt::WV_32s )
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
	HFONT hfont = (HFONT)GetStockObject( f );
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
    memset( &lf, 0, sizeof(LOGFONT) );
    if ( hdc ) {
	if ( !VxF )
	    lf.lfHeight = -int((float)d->req.pointSize*
			       GetDeviceCaps(hdc,LOGPIXELSY)/(float)720+0.5);
	else
	    lf.lfHeight = -(d->req.pointSize/10);
    } else {
	lf.lfHeight = -(d->req.pointSize/10);
    }
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

    /*
	We use DEFAULT_CHARSET as much as possible, so that
	we get good Unicode-capable fonts on international
	platforms (eg. Japanese Windows NT).
    */
    int cs;
    switch ( charSet() ) {
	case AnyCharSet:
	case ISO_8859_1:
	    cs = DEFAULT_CHARSET;
	    break;
	case ISO_8859_2:
	    cs = EASTEUROPE_CHARSET;
	    break;
	case ISO_8859_9:
	    cs = TURKISH_CHARSET;
	    break;
	case ISO_8859_7:
	    cs = GREEK_CHARSET;
	    break;
	default:
	    cs = DEFAULT_CHARSET;
	    break;
    }

    lf.lfCharSet	= cs;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality	= DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | hint;
    HFONT hfont;

    if ( qt_winver == Qt::WV_NT ) {
	memcpy(lf.lfFaceName,qt_winTchar( fam, TRUE ),
	    sizeof(TCHAR)*QMIN(fam.length()+1,32));  // 32 = Windows hard-coded
	hfont = CreateFontIndirect( &lf );
    } else {
	// LOGFONTA and LOGFONTW are binary compatible
	memcpy(lf.lfFaceName,fam.ascii(),
	    sizeof(TCHAR)*QMIN(fam.length()+1,32));  // 32 = Windows hard-coded
	hfont = CreateFontIndirectA( (LOGFONTA*)&lf );
    }

    if ( stockFont )
	*stockFont = hfont == 0;
    if ( hfont == 0 )
	hfont = (HFONT)GetStockObject( ANSI_VAR_FONT );
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
#ifdef UNICODE
    if ( qt_winver == Qt::WV_NT )
	return d->fin->textMetricW();
    else
#endif
	return d->fin->textMetricA();
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

const QFontDef *QFontMetrics::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}

void *QFontMetrics::textMetric() const
{
    if ( painter ) {
	return painter->textMetric();
#ifdef UNICODE
    } else if ( qt_winver == Qt::WV_NT ) {
	return fin->textMetricW();
#endif
    } else {
	return fin->textMetricA();
    }
}

#undef  TM
#undef  TMX
#undef  TMA
#undef  TMW
#define TMA (painter ? (TEXTMETRICA*)painter->textMetric() : fin->textMetricA())
#ifdef UNICODE
#define TMW (painter ? (TEXTMETRICW*)painter->textMetric() : fin->textMetricW())
#else
#define TMW TMA
#endif
#define TM(F) ( qt_winver == Qt::WV_NT ? TMW->F : TMA->F )
#define TMX TMA // Initial metrix align


int QFontMetrics::ascent() const
{
    return TMX->tmAscent;
}

int QFontMetrics::descent() const
{
    return TMX->tmDescent;
}


bool QFontMetrics::inFont(QChar ch) const
{ 
#ifdef UNICODE
    if ( qt_winver == Qt::WV_NT ) {
	TEXTMETRICW *f = TMW;
	uint ch16 = ch.cell+256*ch.row;
	return ch16 >= f->tmFirstChar
	    && ch16 <= f->tmLastChar;
    } else
#endif
    {
	TEXTMETRICA *f = TMA;
	if ( ch.row )
	    return FALSE;
	return ch.cell >= (uint)f->tmFirstChar
	    && ch.cell <= (uint)f->tmLastChar;
    }
}


int QFontMetrics::leftBearing(QChar ch) const
{
    if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.cell+256*ch.row;
	    ABC abc;
	    GetCharABCWidths(hdc(),ch16,ch16,&abc);
	    return abc.abcA;
	} else {
	    if ( ch.row )
		return 0;
	    ABC abc;
	    GetCharABCWidthsA(hdc(),ch.cell,ch.cell,&abc);
	    return abc.abcA;
	}
    } else {
#ifdef UNICODE
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.cell+256*ch.row;
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(hdc(),ch16,ch16,&abc);
	    return int(abc.abcfA);
	} else
#endif
	{
	    return 0;
	}
    }
}


int QFontMetrics::rightBearing(QChar ch) const
{
    if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.cell+256*ch.row;
	    ABC abc;
	    GetCharABCWidths(hdc(),ch16,ch16,&abc);
	    return abc.abcC;
	} else {
	    if ( ch.row )
		return 0;
	    ABC abc;
	    GetCharABCWidthsA(hdc(),ch.cell,ch.cell,&abc);
	    return abc.abcC;
	}
    } else {
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.cell+256*ch.row;
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(hdc(),ch16,ch16,&abc);
	    return int(abc.abcfA);
	} else {
	    return 0;
	}
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
	int ml;
	int mr;
	if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	    ABC *abc;
	    int n;
	    if ( qt_winver == Qt::WV_NT ) {
		TEXTMETRIC *tm = TMW;
		n = tm->tmLastChar - tm->tmFirstChar+1;
		abc = new ABC[n];
		GetCharABCWidths(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
	    } else {
		TEXTMETRICA *tm = TMA;
		n = tm->tmLastChar - tm->tmFirstChar+1;
		abc = new ABC[n];
		GetCharABCWidthsA(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
	    }
	    ml = abc[0].abcA;
	    mr = abc[0].abcC;
	    for (int i=1; i<n; i++) {
		ml = QMIN(ml,abc[i].abcA);
		mr = QMIN(mr,abc[i].abcC);
	    }
	    delete [] abc;
	} else {
	    if ( qt_winver == Qt::WV_NT ) {
		TEXTMETRIC *tm = TMW;
		int n = tm->tmLastChar - tm->tmFirstChar+1;
		ABCFLOAT *abc = new ABCFLOAT[n];
		GetCharABCWidthsFloat(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
		float fml = abc[0].abcfA;
		float fmr = abc[0].abcfC;
		for (int i=1; i<n; i++) {
		    fml = QMIN(fml,abc[i].abcfA);
		    fmr = QMIN(fmr,abc[i].abcfC);
		}
		ml = int(fml-0.9999);
		mr = int(fmr-0.9999);
		delete [] abc;
	    } else {
		ml = mr = 0;
	    }
	}
	def->lbearing = ml;
	def->rbearing = mr;
    }

    return def->rbearing;
}


int QFontMetrics::height() const
{
    return TMX->tmHeight;
}

int QFontMetrics::leading() const
{
    return TMX->tmExternalLeading;
}

int QFontMetrics::lineSpacing() const
{
    return TMX->tmHeight + TMX->tmExternalLeading;
}

int QFontMetrics::width( QChar ch ) const
{
    QString s;
    s += ch;
    if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
        return width(s,1);
    } else {
        return width(s,1)-TMX->tmOverhang;
    }
}


int QFontMetrics::width( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();
    SIZE s;
    const TCHAR* tc = (const TCHAR*)qt_winTchar(str,FALSE);
    GetTextExtentPoint32( hdc(), tc, len, &s );
    return s.cx;
}

QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    int cx = width(str,len);

    int l = len ? leftBearing(str[0]) : 0;
    int r = len ? -rightBearing(str[len-1]) : 0;
    // To be safer, check bearings of next-to-end characters too.
    /* ### eaa: trenger aa fikse noe her.
    if (len > 1 ) {
	int newl = width(str[0])+leftBearing(str[1]);
	int newr = -width(str[len-1])-rightBearing(str[len-2]);
	if ( newl < l ) l = newl;
	if ( newr > r ) r = newr;
    }
    */
    TEXTMETRICA *tm = TMX;
    return QRect(l, -tm->tmAscent, cx+r-l, tm->tmAscent+tm->tmDescent);
}


HDC QFontMetrics::hdc() const
{
    return painter ? painter->handle() : fin->dc();
}

int QFontMetrics::maxWidth() const
{
    return TMX->tmMaxCharWidth;
}

int QFontMetrics::underlinePos() const
{
    int pos = (lineWidth()*2 + 3)/6;
    return QMAX(pos,1);
}

int QFontMetrics::strikeOutPos() const
{
    int pos = TMX->tmAscent/3;
    return QMAX(pos,1);
}

int QFontMetrics::lineWidth() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->lineWidth();
    } else {
	return fin->lineWidth();
    }
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

const QFontDef *QFontInfo::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}


/*****************************************************************************
  QFontData member functions
 *****************************************************************************/

const QTextCodec* QFontData::mapper() const
{
    return 0;
}

void* QFontData::fontSet() const
{
    return 0;
}

/*--------------------------------------------------------------------------
  -------------------------- QFontDatabase ---------------------------------
  --------------------------------------------------------------------------*/

static QFontDatabasePrivate *db=0;

class QFontDatabasePrivate {
public:
    QFontDatabasePrivate();
    QStringList families;
};

static int CALLBACK
storeFont( ENUMLOGFONTEX* f, TEXTMETRIC*, int type, LPARAM p )
{
    QFontDatabasePrivate* d = (QFontDatabasePrivate*)p;

    TCHAR* tc = f->elfLogFont.lfFaceName;
    QString name;
    if ( qt_winver == Qt::WV_NT ) {
	name = qt_winQString(tc);
    } else {
	name = QString((const char*)tc);
    }
    d->families.append(name);
    return 1; // Keep enumerating.
}

QFontDatabasePrivate::QFontDatabasePrivate()
{
    QWidget dummy;
    if ( qt_winver == Qt::WV_NT ) {
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0] = 0;
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesEx( dummy.handle(), &lf,
	    (FONTENUMPROC)storeFont, (LPARAM)this, 0 );
    } else {
	LOGFONTA lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0] = 0;
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesExA( dummy.handle(), &lf,
	    (FONTENUMPROCA)storeFont, (LPARAM)this, 0 );
    }
}

QFontDatabase::QFontDatabase()
{
    if ( !db ) db = new QFontDatabasePrivate;
    d = db;
}

const QList<QFontFamily>& QFontDatabase::families() const
{
    // ##### Not implemented
    static QList<QFontFamily> dummy;
    return dummy;
}

const QStringList& QFontDatabase::familyNames() const
{
    return d->families;
}

const QFontFamily& QFontDatabase::family( const QString &familyName ) const
{
    // ##### Not implemented
    static QFontFamily dummy;
    return dummy;
}

QFontStyle::QFontStyle( const QFont &/*f*/ )
{
}

bool QFontStyle::isNull() const
{
    return TRUE;
}

QString QFontStyle::name() const
{
    return "None";
}


bool QFontStyle::scalable() const
{
    return FALSE;
}

bool QFontStyle::smoothlyScalable() const
{
    return FALSE;
}


bool QFontStyle::italic() const
{
    return FALSE;
}

int  QFontStyle::weight() const
{
    return 100;
}


const QArray<int> &QFontStyle::pointSizes() const
{
    static QArray<int> dummy;
    return dummy;
}

int QFontStyle::nSizes() const
{
    return 0;
}

QFont QFontStyle::font( int pointSize ) const
{
    return QFont("Arial", pointSize);
}


QFontStyle::QFontStyle( QFontStylePrivate *data )
{
}

bool QFontCharSet::isNull() const
{
    return FALSE;
}

QString QFontCharSet::name() const
{
    return "No charset";
}

QFont::CharSet QFontCharSet::charSet() const
{
    return QFont::Latin1;
}


const QList<QFontStyle> &QFontCharSet::styles() const
{
    static QList<QFontStyle> dummy;
    return dummy;
}

const QStringList &QFontCharSet::styleNames() const
{
    static QStringList dummy;
    return dummy;
}

const QFontStyle &QFontCharSet::style( const QString &styleName ) const
{
    static QFontStyle dummy;
    return dummy;
}


QFontCharSet::QFontCharSet( QFontCharSetPrivate *data )
{
}


QFontFamily::QFontFamily( const QString &familyName )
{
}

bool QFontFamily::isNull() const
{
    return FALSE;
}

QString QFontFamily::name() const
{
    return "No family";
}

const QList<QFontCharSet> &QFontFamily::charSets() const
{
    static QList<QFontCharSet> dummy;
    return dummy;
}

const QStringList &QFontFamily::charSetNames() const
{
    static QStringList dummy;
    return dummy;
}

const QFontCharSet &QFontFamily::charSet( const QString &charSetName ) const
{
    static QFontCharSet dummy;
    return dummy;
}

const QFontCharSet &QFontFamily::charSet( QFont::CharSet ) const
{
    static QFontCharSet dummy;
    return dummy;
}


bool QFontFamily::supportsCharSet( QFont::CharSet /*cs*/ ) const
{
    return TRUE;
}


QFontFamily::QFontFamily( QFontFamilyPrivate *data )
{
}


