/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_win.cpp#123 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32
**
** Created : 940630
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
#include "qfontdata_p.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"

#include "qwidget.h"
#include "qpainter.h"
#include "qdict.h"
#include "qcache.h"
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

QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool scale)
{
    QString family =
	qt_winver == Qt::WV_NT
	    ? qt_winQString(lf.lfFaceName)
	    : QString::fromLatin1((char*)lf.lfFaceName);
    QFont qf(family);
    if (lf.lfItalic)
	qf.setItalic( TRUE );
    if (lf.lfWeight != FW_DONTCARE)
	qf.setWeight(lf.lfWeight/10);
    int lfh = QABS( lf.lfHeight );
    if ( scale ) {
	ASSERT(shared_dc);
	qf.setPointSizeFloat( lfh * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
    } else {
	qf.setPointSize( lfh );
    }
    return qf;
}

int QFont::pixelSize() const
{
    return (d->req.pointSize*GetDeviceCaps(shared_dc,LOGPIXELSY) + 360) / 720;
}

void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat( pixelSize * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
}

/*****************************************************************************
  QFontInternal definition and implementation
 *****************************************************************************/

class QFontInternal
{
public:
   ~QFontInternal();
    bool	    dirty()      const;
    QString         key()	 const;
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

inline QString QFontInternal::key() const
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


class QFontCache : public QCache<QFontInternal>
{
public:
    QFontCache( int maxCost, int size=17, bool cs=TRUE )
	: QCache<QFontInternal>(maxCost,size,cs) {}

    // This pass-through function is needed to avoid MSVC++ 5.0 internal
    // compiler error. (occurs in QFont::load()).
    QFontInternal *find(const QString& k)
	{ return QCache<QFontInternal>::find(k,TRUE); }

    void deleteItem( Item );
};

void QFontCache::deleteItem( Item d )
{
    QFontInternal *fin = (QFontInternal *)d;
    fin->reset();
}


static QFontCache    *fontCache	     = 0;	// cache of loaded fonts
static QFontDict     *fontDict	     = 0;	// dict of all loaded fonts
						// default character set:
// ### Not used until we have support for for codecs on Windows 95/98
QFont::CharSet     QFont::defaultCharSet = QFont::AnyCharSet;

/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::locale_init()
{
    // ### Does nothing until we have support for codecs on Windows 95/98
}

void QFont::initialize()
{
    if ( fontCache )
	return;
    shared_dc = CreateCompatibleDC( qt_display_dc() );
    shared_dc_font = 0;
    fontCache = new QFontCache( fontCacheSize, 29 );
    CHECK_PTR( fontCache );
    fontDict = new QFontDict( 29 );
    CHECK_PTR( fontDict );
}

void QFont::cleanup()
{
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
    qDebug( "{" );
    while ( (fin = it.current()) ) {
	++it;
	qDebug( "   [%s]", fin->key().data() );
    }
    qDebug( "}" );
#endif
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

void QFont::setRawName( const QString &name )
{
    setFamily( name );
}


bool QFont::dirty() const
{
    return DIRTY_FONT;
}


QString QFont::defaultFamily() const
{
    switch( d->req.styleHint ) {
	case Times:
	    return QString::fromLatin1("Times New Roman");
	case Courier:
	    return QString::fromLatin1("Courier New");
	case Decorative:
	    return QString::fromLatin1("Bookman Old Style");
	case Helvetica:
	    return QString::fromLatin1("Arial");
	case System:
	default:
	    return QString::fromLatin1("MS Sans Serif");
    }
}


QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1("arial");
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
	f->s.family = QString::fromLocal8Bit(an);
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
	d->fin = fontDict->find(k);
	if ( !d->fin ) {			// font was never loaded
	    d->fin = new QFontInternal( k );
	    CHECK_PTR( d->fin );
	    fontDict->insert( d->fin->key(), d->fin );
	}
    }

    if ( !d->fin->font() ) {			// font not loaded
	if ( qt_winver == Qt::WV_NT )
	    d->fin->hdc = GetDC(0);
	d->fin->hfont = create( &d->fin->stockFont, d->fin->hdc );
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
	// ### why different?
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
    if ( hdc && !VxF ) {
	lf.lfHeight = -int((float)d->req.pointSize*
			   GetDeviceCaps(hdc,LOGPIXELSY)/(float)720+0.5);
    } else {
	lf.lfHeight = -int((float)d->req.pointSize*
			   GetDeviceCaps(shared_dc,LOGPIXELSY)/(float)720+0.5);
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
	    cs = DEFAULT_CHARSET; // Charset follows locale
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
	QCString lname = fam.local8Bit();
	memcpy(lf.lfFaceName,lname.data(),
	    QMIN(lname.length()+1,32));  // 32 = Windows hard-coded
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
	WCHAR ch16 = ch.unicode();
	return ch16 >= f->tmFirstChar
	    && ch16 <= f->tmLastChar;
    } else
#endif
    {
	TEXTMETRICA *f = TMA;
	if ( ch.row() )
	    return FALSE;
	return (WCHAR)ch.cell() >= f->tmFirstChar
	    && (WCHAR)ch.cell() <= f->tmLastChar;
    }
}


int QFontMetrics::leftBearing(QChar ch) const
{
    if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	ABC abc;
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(hdc(),ch16,ch16,&abc);
	} else {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 125 ) {
		QCString w = qt_winQString2MB(QString(ch));
		if ( w.length() != 1 )
		    return 0;
		ch8 = w[0];
	    } else {
		ch8 = ch.cell();
	    }
	    GetCharABCWidthsA(hdc(),ch8,ch8,&abc);
	}
	return abc.abcA;
    } else {
#ifdef UNICODE
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.unicode();
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
	ABC abc;
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(hdc(),ch16,ch16,&abc);
	    return abc.abcC;
	} else {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 125 ) {
		QCString w = qt_winQString2MB(QString(ch));
		if ( w.length() != 1 )
		    return 0;
		ch8 = w[0];
	    } else {
		ch8 = ch.cell();
	    }
	    GetCharABCWidthsA(hdc(),ch8,ch8,&abc);
	}
	return abc.abcC;
    } else {
	if ( qt_winver == Qt::WV_NT ) {
	    uint ch16 = ch.unicode();
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(hdc(),ch16,ch16,&abc);
	    return int(abc.abcfA);
	} else {
	    return -TMX->tmOverhang;
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
		ml = 0;
		mr = -TMX->tmOverhang;
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
    QString s(ch);
    return width(s,1);
}


int QFontMetrics::width( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();

    // Japanese win95 fails without this
    if ( len == 0 )
	return 0;

    SIZE s;
    const TCHAR* tc = (const TCHAR*)qt_winTchar(str,FALSE);
    GetTextExtentPoint32( hdc(), tc, len, &s );
    if ( qt_winver != Qt::WV_NT )
	s.cx -= TMX->tmOverhang;
    return s.cx;
}

QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();

    int cx = width(str,len);

    int l = len ? leftBearing(str[0]) : 0;
    int r = len ? -rightBearing(str[len-1]) : 0;
    // To be safer, check bearings of next-to-end characters too.
    if (len > 1 ) {
	int newl = width(str[0])+leftBearing(str[1]);
	int newr = -width(str[len-1])-rightBearing(str[len-2]);
	if ( newl < l ) l = newl;
	if ( newr > r ) r = newr;
    }
    TEXTMETRICA *tm = TMX;
    return QRect(l, -tm->tmAscent, cx+r-l, tm->tmAscent+tm->tmDescent);
}


HDC QFontMetrics::hdc() const
{
    if ( painter ) {
	painter->textMetric(); // ensure font is up-to-date
	return painter->handle();
    } else {
	return fin->dc();
    }
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

