/****************************************************************************
** $Id$
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32
**
** Created : 940630
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// the miscrosoft platform SDK says that the Unicode versions of
// TextOut and GetTextExtentsPoint32 are supported on all platforms, so we use them
// exclusively to simplify code, save a lot of conversions into the local encoding
// and have generally better unicode support :)

#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qcomplextext_p.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"

#include "qwidget.h"
#include "qpainter.h"
#include "qdict.h"
#include "qcache.h"
#include <limits.h>
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"



extern HDC   shared_dc;		// common dc for all fonts
extern HFONT stock_sysfont  = 0;

static int max_font_count = 256;

static inline HFONT systemFont()
{
    if ( stock_sysfont == 0 )
	stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}

QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool /*scale*/)
{
    QString family = QT_WA_INLINE( QString::fromUcs2((ushort*)lf.lfFaceName),
				   QString::fromLocal8Bit((char*)lf.lfFaceName) );
    QFont qf(family);
    if (lf.lfItalic)
	qf.setItalic( TRUE );
    if (lf.lfWeight != FW_DONTCARE)
	qf.setWeight(lf.lfWeight*99/900);
    int lfh = QABS( lf.lfHeight );
    Q_ASSERT(shared_dc);
    qf.setPointSizeFloat( lfh * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
    return qf;
}

#if 0
// ### FIXME
int QFont::pixelSize() const
{
    return (d->request.pointSize*GetDeviceCaps(shared_dc,LOGPIXELSY) + 360) / 720;
}

void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat( pixelSize * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
}
#endif


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
    if ( QFontPrivate::fontCache )
	return;
    shared_dc = CreateCompatibleDC( qt_display_dc() );
#if defined(QT_CHECK_RANGE)
    if ( !shared_dc )
	qSystemWarning( "QFont::initialize() (qfont_win.cpp, 163): couldn't create device context" );
#endif
    QFontPrivate::fontCache = new QFontCache();
}

void QFont::cleanup()
{
    if ( QFontPrivate::fontCache )
	QFontPrivate::fontCache->clear();
    delete QFontPrivate::fontCache;
    QFontPrivate::fontCache = 0;
    DeleteDC( shared_dc );
    shared_dc = 0;
}

// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT ( !d->fin || d->request.dirty )


HFONT QFont::handle() const
{
    if ( DIRTY_FONT ) {
	d->load();
    }
    return d->fin->hfont;
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


QString QFontPrivate::defaultFamily() const
{
    switch( request.styleHint ) {
	case QFont::Times:
	    return QString::fromLatin1("Times New Roman");
	case QFont::Courier:
	    return QString::fromLatin1("Courier New");
	case QFont::Decorative:
	    return QString::fromLatin1("Bookman Old Style");
	case QFont::Helvetica:
	    return QString::fromLatin1("Arial");
	case QFont::System:
	default:
	    return QString::fromLatin1("MS Sans Serif");
    }
}

QString QFontPrivate::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFontPrivate::lastResortFont() const
{
    return QString::fromLatin1("arial");
}

void QFontPrivate::initFontInfo()
{
    lineWidth = 1;
    actual = request;				// most settings are equal
    QT_WA( {
	TCHAR n[64];
	GetTextFaceW( fin->dc(), 64, n );
	actual.family = QString::fromUcs2((ushort*)n);
	actual.fixedPitch = !(fin->tm.w.tmPitchAndFamily & TMPF_FIXED_PITCH);
    } , {
	char an[64];
	GetTextFaceA( fin->dc(), 64, an );
	actual.family = QString::fromLocal8Bit(an);
	actual.fixedPitch = !(fin->tm.a.tmPitchAndFamily & TMPF_FIXED_PITCH);
    } );
    if ( actual.pointSize == -1 ) {
	if ( paintdevice )
	    actual.pointSize = actual.pixelSize * 720 / QPaintDeviceMetrics( paintdevice ).logicalDpiY();
	else {
	    actual.pointSize = actual.pixelSize * 720 / GetDeviceCaps( fin->dc(), LOGPIXELSY );
	}
    } else if ( actual.pixelSize == -1 ) {
	if ( paintdevice )
	    actual.pixelSize = actual.pointSize * QPaintDeviceMetrics( paintdevice ).logicalDpiY() / 720;
	else
	    actual.pixelSize = actual.pointSize * GetDeviceCaps( fin->dc(), LOGPIXELSY ) / 720;
    }

    actual.dirty = FALSE;
    exactMatch = ( actual.family == request.family &&
		   ( request.pointSize == -1 || ( actual.pointSize == request.pointSize ) ) &&
		   ( request.pixelSize == -1 || ( actual.pixelSize == request.pixelSize ) ) &&
		   actual.fixedPitch == request.fixedPitch );
}

void QFontPrivate::load()
{
    if ( !fontCache )				// not initialized
	return;
    if ( !request.dirty && fin )
	return;

    QFontEngine *qfs = 0;
    QString k = key();
    if ( paintdevice )
	k += QString::number( (long)(Q_LONG)paintdevice->handle() );
    qfs = fontCache->find( k );

    if ( qfs )
	qfs->ref();
    if ( fin )
	fin->deref();
    fin = qfs;

    if ( fin ) {
	request.dirty = FALSE;
	return;
    }

    // font was never loaded
    fin = new QFontEngineWin( k );

    if ( paintdevice ) {
	fin->hdc = paintdevice->handle();
	fin->paintDevice = TRUE;
    } else if ( qt_winver & Qt::WV_NT_based ) {
	fin->hdc = GetDC( 0 );
    }
    bool stockFont = fin->stockFont;
    fin->hfont = create( &stockFont, fin->hdc );
    fin->stockFont = stockFont;
    HGDIOBJ obj = SelectObject( fin->dc(), fin->hfont );
#ifndef QT_NO_DEBUG
    if ( !obj ) {
	qSystemWarning( "QFontPrivate: SelectObject failed" );
    }
#endif
    BOOL res;
    QT_WA( {
	res = GetTextMetricsW( fin->dc(), &fin->tm.w );
    } , {
	res = GetTextMetricsA( fin->dc(), &fin->tm.a );
    } );
#ifndef QT_NO_DEBUG
    if ( !res )
	qSystemWarning( "QFontPrivate: GetTextMetrics failed" );
#endif
    int cost = request.pointSize > 0 ? request.pointSize*2000 : request.pixelSize*2000;
    if ( paintdevice )
	cost *= 10;
    fin->cache_cost = cost;
    fin->getCMap();

    fin->useTextOutA = FALSE;
    // TextOutW doesn't work for symbol fonts on Windows 95!
    // since we're using glyph indices we don't care for ttfs about this!
    if ( qt_winver == Qt::WV_95 && !fin->ttf &&
	 ( request.family == "Marlett" || request.family == "Symbol" ||
	 request.family == "Webdings" || request.family == "Wingdings" ) )
	    fin->useTextOutA = TRUE;

    fontCache->insert( k, fin, cost );

    initFontInfo();
    request.dirty = FALSE;
}


#if !defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif


// compatMode is used to get Qt-2 compatibility when printing
HFONT QFontPrivate::create( bool *stockFont, HDC hdc, bool compatMode )
{
    QString fam = QFont::substitute( request.family );
    if ( request.rawMode ) {			// will choose a stock font
	int f, deffnt;
	// ### why different?
	if ( (qt_winver & Qt::WV_NT_based) || qt_winver == Qt::WV_32s )
	    deffnt = SYSTEM_FONT;
	else
	    deffnt = DEFAULT_GUI_FONT;
	fam = fam.lower();
	if ( fam == "default" )
	    f = deffnt;
	else if ( fam == "system" )
	    f = SYSTEM_FONT;
#ifndef Q_OS_TEMP
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
#endif
	else if ( fam[0] == '#' )
	    f = fam.right(fam.length()-1).toInt();
	else
	    f = deffnt;
	if ( stockFont )
	    *stockFont = TRUE;
	HFONT hfont = (HFONT)GetStockObject( f );
	if ( !hfont ) {
#ifndef QT_NO_DEBUG
	    qSystemWarning( "GetStockObject failed" );
#endif
	    hfont = systemFont();
	}
	return hfont;
    }

    int hint = FF_DONTCARE;
    switch ( request.styleHint ) {
	case QFont::Helvetica:
	    hint = FF_SWISS;
	    break;
	case QFont::Times:
	    hint = FF_ROMAN;
	    break;
	case QFont::Courier:
	    hint = FF_MODERN;
	    break;
	case QFont::OldEnglish:
	    hint = FF_DECORATIVE;
	    break;
	case QFont::System:
	    hint = FF_MODERN;
	    break;
	default:
	    break;
    }

    LOGFONT lf;
    memset( &lf, 0, sizeof(LOGFONT) );

    // ### fix compatibility mode when printing!!!
    if ( hdc && !compatMode ) {
	SIZE size;
	float sx = 1.;
	if ( GetWindowExtEx( hdc, &size ) ) {
	    sx = size.cy;
	    GetViewportExtEx( hdc, &size );
	    sx /= size.cy;
	}
	if ( request.pointSize != -1 )
    	    lf.lfHeight = -int((float)request.pointSize* sx *
	    		   GetDeviceCaps(hdc,LOGPIXELSY)/(float)720+0.5);
	else
	    lf.lfHeight = - request.pixelSize;
    } else {
	if ( request.pointSize != -1 )
    	    lf.lfHeight = -int((float)request.pointSize*
			   GetDeviceCaps(shared_dc,LOGPIXELSY)/(float)720+0.5);
	else
	    lf.lfHeight = - request.pixelSize;
    }
#ifdef Q_OS_TEMP
    lf.lfHeight		+= 3;
#endif
    lf.lfWidth		= 0;
    lf.lfEscapement	= 0;
    lf.lfOrientation	= 0;
    if ( request.weight == 50 )
	lf.lfWeight = FW_DONTCARE;
    else
	lf.lfWeight = (request.weight*900)/99;
    lf.lfItalic		= request.italic;
    lf.lfUnderline	= request.underline;
    lf.lfStrikeOut	= request.strikeOut;
    lf.lfCharSet	= DEFAULT_CHARSET;

    int strat = OUT_DEFAULT_PRECIS;
    if (  request.styleStrategy & QFont::PreferBitmap ) {
	strat = OUT_RASTER_PRECIS;
#ifndef Q_OS_TEMP
    } else if ( request.styleStrategy & QFont::PreferDevice ) {
	strat = OUT_DEVICE_PRECIS;
    } else if ( request.styleStrategy & QFont::PreferOutline ) {
	QT_WA( {
	    strat = OUT_OUTLINE_PRECIS;
	} , {
	    strat = OUT_TT_PRECIS;
	} );
    } else if ( request.styleStrategy & QFont::ForceOutline ) {
	strat = OUT_TT_ONLY_PRECIS;
#endif
    }

    lf.lfOutPrecision   = strat;

    int qual = DEFAULT_QUALITY;

    if ( request.styleStrategy & QFont::PreferMatch )
	qual = DRAFT_QUALITY;
#ifndef Q_OS_TEMP
    else if ( request.styleStrategy & QFont::PreferQuality )
	qual = PROOF_QUALITY;
#endif

    if ( request.styleStrategy & QFont::PreferAntialias ) {
	if ( qt_winver >= Qt::WV_XP )
	    qual = 5; // == CLEARTYPE_QUALITY;
	else
	    qual = ANTIALIASED_QUALITY;
    } else if ( request.styleStrategy & QFont::NoAntialias ) {
	qual = NONANTIALIASED_QUALITY;
    }

    lf.lfQuality	= qual;

    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfPitchAndFamily = DEFAULT_PITCH | hint;
    HFONT hfont;

    if ( (fam == "MS Sans Serif") && (request.italic || (-lf.lfHeight > 18 && -lf.lfHeight != 24)) )
       fam = "Arial"; // MS Sans Serif has bearing problems in italic, and does not scale

    QT_WA( {
	memcpy(lf.lfFaceName, fam.ucs2(), sizeof(TCHAR)*QMIN(fam.length()+1,32));  // 32 = Windows hard-coded
	hfont = CreateFontIndirect( &lf );
    } , {
	// LOGFONTA and LOGFONTW are binary compatible
	QCString lname = fam.local8Bit();
	memcpy(lf.lfFaceName,lname.data(),
	    QMIN(lname.length()+1,32));  // 32 = Windows hard-coded
	hfont = CreateFontIndirectA( (LOGFONTA*)&lf );
    } );
#ifndef QT_NO_DEBUG
    if ( !hfont )
	qSystemWarning( "CreateFontIndirect failed" );
#endif

    if ( stockFont )
	*stockFont = hfont == 0;

    if ( hfont && request.stretch != 100 ) {
	HGDIOBJ oldObj = SelectObject( fin->dc(), hfont );
	BOOL res;
	int avWidth = 0;
	QT_WA( {
	    TEXTMETRICW tm;
	    res = GetTextMetricsW( fin->dc(), &tm );
	    avWidth = tm.tmAveCharWidth;
	} , {
	    TEXTMETRICA tm;
	    res = GetTextMetricsA( fin->dc(), &tm);
	    avWidth = tm.tmAveCharWidth;
	} );
#ifndef QT_NO_DEBUG
	if ( res )
	    qSystemWarning( "QFontPrivate: GetTextMetrics failed" );
#endif

	SelectObject( fin->dc(), oldObj );
	DeleteObject( hfont );

	lf.lfWidth = avWidth * request.stretch/100;
	QT_WA( {
	    hfont = CreateFontIndirect( &lf );
	} , {
	    hfont = CreateFontIndirectA( (LOGFONTA*)&lf );
	} );
#ifndef QT_NO_DEBUG
	if ( !hfont )
	    qSystemWarning( "CreateFontIndirect with stretch failed" );
#endif
    }    

#ifndef Q_OS_TEMP
    if ( hfont == 0 )
	hfont = (HFONT)GetStockObject( ANSI_VAR_FONT );
#endif

    return hfont;
}


void *QFont::textMetric() const
{
    if ( DIRTY_FONT ) {
	d->load();
#if defined(QT_DEBUG)
	Q_ASSERT( d->fin );
#endif
    }
    QT_WA( {
	return (void *)&d->fin->tm.w;
    } , {
	return (void *)&d->fin->tm.a;
    } );
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

void *QFontMetrics::textMetric() const
{
    if ( painter ) {
	return painter->textMetric();
    } else  {
	QT_WA( {
	    return (void *)&d->fin->tm.w;
	} , {
	    return (void *)&d->fin->tm.a;
	} );
    }
}

#undef  TM
#undef  TMX
#undef  TMA
#undef  TMW
#define TMA ((TEXTMETRICA*)(painter ? painter->textMetric() : textMetric()))
#ifdef UNICODE
#define TMW ((TEXTMETRICW*)(painter ? painter->textMetric() : textMetric()))
#else
#define TMW TMA
#endif
#define TM(F) ( ( qt_winver & Qt::WV_NT_based ) ? TMW->F : TMA->F )
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
    if ( d->fin->ttf ) {
	glyph_t glyph;
	d->fin->getGlyphIndexes( &ch, 1, &glyph );
	return (glyph != 0);
    } else {
	QT_WA( {
	    WCHAR ch16 = ch.unicode();
	    if( ch16 < d->fin->tm.w.tmFirstChar || ch16 > d->fin->tm.w.tmLastChar )
		return FALSE;
	    return TRUE;//!d->boundingRect( ch ).isEmpty();
	} , {
	    if ( ch.row() || ch.cell() < d->fin->tm.w.tmFirstChar
		|| ch.cell() > d->fin->tm.w.tmLastChar )
		return FALSE;
	    return TRUE;//!d->boundingRect( ch ).isEmpty();
	} );
    }
}


int QFontMetrics::leftBearing(QChar ch) const
{
#ifdef Q_OS_TEMP
	return 0;
#else
    if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	ABC abc;
	QT_WA( {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(hdc(),ch16,ch16,&abc);
	} , {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 125 ) {
		QCString w = QString(ch).local8Bit();
		if ( w.length() != 1 )
		    return 0;
		ch8 = w[0];
	    } else {
		ch8 = ch.cell();
	    }
	    GetCharABCWidthsA(hdc(),ch8,ch8,&abc);
	} );
	return abc.abcA;
    } else {
	QT_WA( {
	    uint ch16 = ch.unicode();
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(hdc(),ch16,ch16,&abc);
	    return int(abc.abcfA);
	} , {
	    return 0;
	} );
    }
    return 0;
#endif
}


int QFontMetrics::rightBearing(QChar ch) const
{
#ifdef Q_OS_TEMP
	return 0;
#else
    if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	ABC abc;
	QT_WA( {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(hdc(),ch16,ch16,&abc);
	    return abc.abcC;
	} , {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 125 ) {
		QCString w = QString(ch).local8Bit();
		if ( w.length() != 1 )
		    return 0;
		ch8 = w[0];
	    } else {
		ch8 = ch.cell();
	    }
	    GetCharABCWidthsA(hdc(),ch8,ch8,&abc);
	} );
	return abc.abcC;
    } else {
	QT_WA( {
	    uint ch16 = ch.unicode();
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(hdc(),ch16,ch16,&abc);
	    return int(abc.abcfC);
	} , {
	    return -TMX->tmOverhang;
	} );
    }

    return 0;
#endif
}

static ushort char_table[] = {
	40,
	67,
	70,
	75,
	86,
	88,
	89,
	91,
	102,
	114,
	124,
	127,
	205,
	645,
	884,
	922,
	1070,
	3636,
	3660,
	12386,
	0
};


static int get_min_left_bearing( const QFontMetrics *f )
{
    int i = 0;
    int b = 0;
    while ( char_table[ i ] != 0 ) {
	b = QMIN( b, f->leftBearing( QChar( char_table[ i ] ) ) );
	++i;
    }
    return b - 1;
}

static int get_min_right_bearing( const QFontMetrics *f )
{
    int i = 0;
    int b = 0;
    while ( char_table[ i ] != 0 ) {
	b = QMIN( b, f->rightBearing( QChar( char_table[ i ] ) ) );
	++i;
    }
    return b - 1;
}

int QFontMetrics::minLeftBearing() const
{
    const QFontDef* def = &d->actual;

    if ( def->lbearing == SHRT_MIN ) {
		minRightBearing(); // calculates both
    }

    return def->lbearing;
}

int QFontMetrics::minRightBearing() const
{
#ifdef Q_OS_TEMP
	return 0;
#else
    // Safely cast away const, as we cache bearings there.
    QFontDef* def = (QFontDef*)&d->actual;

    if ( def->rbearing == SHRT_MIN ) {
	int ml = 0;
	int mr = 0;
	if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	    ABC *abc = 0;
	    int n;
	    QT_WA( {
		const TEXTMETRIC *tm = TMW;
		n = tm->tmLastChar - tm->tmFirstChar+1;
		if ( n <= max_font_count ) {
		    abc = new ABC[n];
		    GetCharABCWidths(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
		} else {
		    ml = get_min_left_bearing( this );
		    mr = get_min_right_bearing( this );
		}
	    } , {
		const TEXTMETRICA *tm = TMA;
		n = tm->tmLastChar - tm->tmFirstChar+1;
		if ( n <= max_font_count ) {
		    abc = new ABC[n];
		    GetCharABCWidthsA(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
		} else {
		    ml = get_min_left_bearing( this );
		    mr = get_min_right_bearing( this );
		}
	    } );
	    if ( n <= max_font_count ) {
		ml = abc[0].abcA;
		mr = abc[0].abcC;
    		for (int i=1; i<n; i++) {
		    ml = QMIN(ml,abc[i].abcA);
		    mr = QMIN(mr,abc[i].abcC);
		}
		delete [] abc;
	    }
	} else {
	    QT_WA( {
		const TEXTMETRIC *tm = TMW;
		int n = tm->tmLastChar - tm->tmFirstChar+1;
		if ( n <= max_font_count ) {
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
		    ml = get_min_left_bearing( this );
		    mr = get_min_right_bearing( this );
		}
	    } , {
		ml = 0;
		mr = -TMX->tmOverhang;
	    } );
	}
	def->lbearing = ml;
	def->rbearing = mr;
    }

    return def->rbearing;
#endif
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
    if ( ch.category() == QChar::Mark_NonSpacing )
	return 0;

    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( TRUE );

    SIZE s = {0,0};
    wchar_t tc = ch.unicode();
    BOOL res = GetTextExtentPoint32W( hdc(), &tc, 1, &s );

    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( FALSE );

#ifndef QT_NO_DEBUG
    if ( !res )
	qSystemWarning( "QFontMetrics::width: GetTextExtentPoint32 failed" );
#endif
    if ( (qt_winver & Qt::WV_NT_based) == 0 )
	s.cx -= TMX->tmOverhang;
    return s.cx;
}


int QFontMetrics::width( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();
    if ( len == 0 )
	return 0;

    QTextEngine layout( str, d );
    layout.itemize( FALSE );
    return layout.width( 0, len );
}

int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( TRUE );
    QTextEngine layout( str,  d );
    layout.itemize( FALSE );
    int w = layout.width( pos, 1 );
    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( FALSE );

    if ( (qt_winver & Qt::WV_NT_based) == 0 )
	w -= TMX->tmOverhang;
    return w;
}

QRect QFontMetrics::boundingRect( QChar ch ) const
{
    glyph_t glyphs[10];
    int nglyphs = 9;
    advance_t advances[10];
    d->fin->stringToCMap( &ch, 1, glyphs, advances, &nglyphs );
    glyph_metrics_t gi = d->fin->boundingBox( glyphs[0] );
    return QRect( gi.x, gi.y, gi.width, gi.height );
}


QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    if (len < 0)
	len = str.length();
    if (len == 0)
	return QRect();

    QTextEngine layout( str, d );
    layout.itemize( FALSE );
    glyph_metrics_t gm = layout.boundingBox( 0, len );
    return QRect( gm.x, gm.y, gm.width, gm.height );
}


HDC QFontMetrics::hdc() const
{
    if ( painter ) {
	painter->textMetric(); // ensure font is up-to-date
	return painter->handle();
    } else {
	return d->fin->dc();
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
	return painter->cfont.d->lineWidth;
    } else {
	return d->lineWidth;
    }
}



