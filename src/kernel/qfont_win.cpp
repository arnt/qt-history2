/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_win.cpp#123 $
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

#include "qfont.h"
#include "qfontdata_p.h"
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
#include "qcleanuphandler.h"


static HDC   shared_dc	    = 0;		// common dc for all fonts
static HFONT shared_dc_font = 0;		// used by Windows 95/98

static HFONT stock_sysfont  = 0;
static int max_font_count = 256;

static inline HFONT systemFont()
{
    if ( stock_sysfont == 0 )
	stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}

QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool scale)
{
    QString family =
	(qt_winver & Qt::WV_NT_based)
	    ? qt_winQString(lf.lfFaceName)
	    : QString::fromLatin1((char*)lf.lfFaceName);
    QFont qf(family);
    if (lf.lfItalic)
	qf.setItalic( TRUE );
    if (lf.lfWeight != FW_DONTCARE)
	qf.setWeight(lf.lfWeight/10);
    int lfh = QABS( lf.lfHeight );
    if ( scale ) {
	Q_ASSERT(shared_dc);
	qf.setPointSizeFloat( lfh * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
    } else {
	qf.setPointSize( lfh );
    }
    return qf;
}

int QFont::pixelSize() const
{
    return (d->request.pointSize*GetDeviceCaps(shared_dc,LOGPIXELSY) + 360) / 720;
}

void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat( pixelSize * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
}

/*****************************************************************************
  QFontStruct implementation
 *****************************************************************************/

QFontStruct::QFontStruct( const QString &key )
    : QShared(), k(key), hdc(0), hfont(0)
{
    s.dirty = TRUE;
	cache_cost = 1;
}

HDC QFontStruct::dc() const
{
    if ( qt_winver & Qt::WV_NT_based )
	return hdc;
    Q_ASSERT( shared_dc != 0 && hfont != 0 );
    if ( shared_dc_font != hfont ) {
	SelectObject( shared_dc, hfont );
	shared_dc_font = hfont;
    }
    return shared_dc;
}

TEXTMETRICA *QFontStruct::textMetricA() const
{
    QFontStruct *that = (QFontStruct *)this;
    return &that->tm.a;
}

TEXTMETRICW *QFontStruct::textMetricW() const
{
    QFontStruct *that = (QFontStruct *)this;
    return &that->tm.w;
}

void QFontStruct::reset()
{
    if ( qt_winver & Qt::WV_NT_based ) {
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
		Q_ASSERT( shared_dc != 0 );
		SelectObject( shared_dc, systemFont() );
		shared_dc_font = 0;
	    }
	    if ( !stockFont )
		DeleteObject( hfont );
	    hfont = 0;
	}
    }
}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

QCleanupHandler<QFontCache> cleanup_fontcache;

void QFont::initialize()
{
    if ( QFontPrivate::fontCache )
	return;
    shared_dc = CreateCompatibleDC( qt_display_dc() );
    shared_dc_font = 0;
    QFontPrivate::fontCache = new QFontCache();
    Q_CHECK_PTR( QFontPrivate::fontCache );
    cleanup_fontcache.add( QFontPrivate::fontCache );
}

void QFont::cleanup()
{
    QFontPrivate::fontCache = 0;
    Q_ASSERT( shared_dc_font == 0 );
    DeleteDC( shared_dc );
    shared_dc = 0;
}

// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT (d->request.dirty || d->fin->dirty())


HFONT QFont::handle() const
{
    static HFONT last = 0;
    if ( DIRTY_FONT ) {
	d->load();
    } else {
	if ( d->fin->font() != last )
		QFontPrivate::fontCache->find( d->fin->key() );
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

static MAT2 *mat = 0;

QRect QFontPrivate::boundingRect( const QChar &ch )
{
	GLYPHMETRICS gm;
	if ( !mat ) {
		mat = new MAT2;
		mat->eM11.value = mat->eM22.value = 1;
		mat->eM11.fract = mat->eM22.fract = 0;
		mat->eM21.value = mat->eM12.value = 0;
		mat->eM21.fract = mat->eM12.fract = 0;
	}
#ifdef UNICODE
	uint chr = ch.unicode();
#else
	uint chr = ch.latin1();
#endif
	if ( GetGlyphOutline( currHDC, chr, GGO_METRICS, &gm, 0, 0, mat ) == GDI_ERROR )
		qDebug( "glyph metrics call failed" );
    return QRect(gm.gmptGlyphOrigin.x, -gm.gmptGlyphOrigin.y, gm.gmBlackBoxX, gm.gmBlackBoxY);
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
    if ( !fin->s.dirty )				// already initialized
	return;
    lineWidth = 1;
    fin->s = request;				// most settings are equal
    if ( qt_winver & Qt::WV_NT_based ) {
	TCHAR n[64];
	GetTextFace( fin->dc(), 64, n );
	fin->s.family = qt_winQString(n);
    } else {
	char an[64];
	GetTextFaceA( fin->dc(), 64, an );
	fin->s.family = QString::fromLocal8Bit(an);
    }
    fin->s.dirty = FALSE;
}

void QFontPrivate::load()
{
    if ( !fontCache )				// not initialized
		return;
	if ( !request.dirty )
		return;

    QString k = key();
	QFontStruct *qfs = fontCache->find( k );
	
	if ( !qfs ) {			// font was never loaded
	    qfs = new QFontStruct( k );
	    Q_CHECK_PTR( qfs );
	}
	qfs->ref();
	if ( fin ) fin->deref();
	fin = qfs;

    if ( !fin->font() ) {			// font not loaded
	if ( qt_winver & Qt::WV_NT_based )
	    fin->hdc = GetDC(0);
	fin->hfont = create( &fin->stockFont, fin->hdc );
	SelectObject( fin->dc(), fin->hfont );
#ifdef UNICODE
	if ( qt_winver & Qt::WV_NT_based ) {
	    GetTextMetricsW( fin->dc(), &fin->tm.w );
	} else
#endif
	{
	    GetTextMetricsA( fin->dc(), &fin->tm.a );
	}
	fontCache->insert( fin->key(), fin, 1 );
	initFontInfo();
    }
    exactMatch = TRUE;
    request.dirty = FALSE;
}


#if !defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif

HFONT QFont::create( bool *stockFont, HDC hdc, bool VxF )
{
	return d->create(stockFont, hdc, VxF);
}

HFONT QFontPrivate::create( bool *stockFont, HDC hdc, bool VxF )
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
    if ( hdc && !VxF ) {
	lf.lfHeight = -int((float)request.pointSize*
			   GetDeviceCaps(hdc,LOGPIXELSY)/(float)720+0.5);
    } else {
	lf.lfHeight = -int((float)request.pointSize*
			   GetDeviceCaps(shared_dc,LOGPIXELSY)/(float)720+0.5);
    }
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

#if 0
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
#else
	lf.lfCharSet	= DEFAULT_CHARSET;
#endif

    int strat = OUT_DEFAULT_PRECIS;
    if (  request.styleStrategy & QFont::PreferBitmap ) {
	strat = OUT_RASTER_PRECIS;
    } else if ( request.styleStrategy & QFont::PreferDevice ) {
	strat = OUT_DEVICE_PRECIS;
    } else if ( request.styleStrategy & QFont::PreferOutline ) {
	if ( qt_winver & Qt::WV_NT_based )
	    strat = OUT_OUTLINE_PRECIS;
	else
	    strat = OUT_TT_PRECIS;
    } else if ( request.styleStrategy & QFont::ForceOutline ) {
	strat = OUT_TT_ONLY_PRECIS;
    }

    lf.lfOutPrecision   = strat;

    int qual = DEFAULT_QUALITY;
    if ( request.styleStrategy & QFont::PreferMatch )
	qual = DRAFT_QUALITY;
    else if ( request.styleStrategy & QFont::PreferQuality )
	qual = PROOF_QUALITY;

    lf.lfQuality	= qual;
    
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfPitchAndFamily = DEFAULT_PITCH | hint;
    HFONT hfont;

    if ( qt_winver & Qt::WV_NT_based ) {
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

#ifdef UNICODE
#define TMX (fin->textMetricW())
#else
#define TMX (fin->textMetricA())
#endif

int QFontPrivate::textWidth( const QString &str, int pos, int len )
{
    // Japanese win95 fails without this
    if ( len == 0 )
	return 0;

    int width = 0;
    SIZE s;
    const TCHAR* tc = (const TCHAR*)qt_winTchar(str.mid(pos, len),FALSE);
    int i;
    int last = 0;
    const QChar *uc = str.unicode() + pos;
    for ( i = 0; i < len; i++ ) {
	if ( uc->combiningClass() != 0 && pos + i > 0 ) {
	    if ( i - last > 0 ) {
		GetTextExtentPoint32( fin->dc(), tc + last, i - last, &s );
		width += s.cx;
	    }
	    last = i + 1;
	}
	uc++;
    }
    GetTextExtentPoint32( fin->dc(), tc + last, i - last, &s );
    width += s.cx;

    if ( (qt_winver & Qt::WV_NT_based) == 0 )
	width -= TMX->tmOverhang;
    return width;
	
}

int QFontPrivate::textWidth( HDC hdc, const QString &str, int pos, int len, QFontPrivate::TextRun *cache )
{
    // Japanese win95 fails without this
    if ( len == 0 )
	return 0;
	if ( hdc )
		currHDC = hdc;
	else
		currHDC = fin->dc();

	int width = 0;
    SIZE s;
    const TCHAR* tc = (const TCHAR*)qt_winTchar(str.mid(pos, len),FALSE);
	QPointArray pa;
	int nmarks = 0;
	int i;
	int lasts = 0;
	const QChar *uc = str.unicode() + pos;
	for ( i = 0; i < len; i++ ) {
		if ( uc->combiningClass() != 0 && !nmarks && pos + i > 0 ) {
			cache->setParams( width, 0, str.unicode() + lasts, i - lasts );
			GetTextExtentPoint32( currHDC, tc + lasts, i - lasts, &s );
			width += s.cx;
			cache->next = new QFontPrivate::TextRun();
			cache = cache->next;
			lasts = i;
			pa = QComplexText::positionMarks( this, str, pos + i - 1 );
			nmarks = pa.size();
		} else if ( nmarks ) {
			QPoint p = pa[(int)(pa.size() - nmarks)];
			cache->setParams( width + p.x(), p.y(), str.unicode() + lasts, i - lasts );
			cache->next = new QFontPrivate::TextRun();
			cache = cache->next;
			nmarks--;
			lasts = i;
		}
		uc++;
	}
	
	if ( nmarks ) {
		QPoint p = pa[(int)(pa.size() - nmarks)];
		cache->setParams( width + p.x(), p.y(), str.unicode() + lasts, i - lasts );
	} else {
		cache->setParams( width, 0, str.unicode() + lasts, i - lasts );
		GetTextExtentPoint32( currHDC, tc + lasts, i - lasts, &s );
		width += s.cx;
	}
				
	if ( (qt_winver & Qt::WV_NT_based) == 0 )
		width -= TMX->tmOverhang;
    return width;
}

void QFontPrivate::drawText( HDC hdc, int x, int y, QFontPrivate::TextRun *cache )
{
    while ( cache ) {
//		qDebug( "drawing '%s' at (%d/%d)", 
//			QConstString( (QChar *)cache->string, cache->length).string().latin1(),
//			x + cache->xoff, y + cache->yoff);
		const TCHAR *tc = (const TCHAR*)qt_winTchar(QConstString( (QChar *)cache->string, cache->length).string(),FALSE); 
		TextOut( hdc, x + cache->xoff, y + cache->yoff, tc, cache->length );
		cache = cache->next;
    }

}

void *QFont::textMetric() const
{
    if ( DIRTY_FONT ) {
		d->load();
#if defined(QT_DEBUG)
		Q_ASSERT( d->fin && d->fin->font() );
#endif
    }
#ifdef UNICODE
    if ( qt_winver & Qt::WV_NT_based )
		return d->fin->textMetricW();
    else
#endif
		return d->fin->textMetricA();
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

void *QFontMetrics::textMetric() const
{
    if ( painter ) {
		return painter->textMetric();
#ifdef UNICODE
    } else if ( qt_winver & Qt::WV_NT_based ) {
		return d->fin->textMetricW();
#endif
    } else {
		return d->fin->textMetricA();
    }
}

#undef  TM
#undef  TMX
#undef  TMA
#undef  TMW
#define TMA (painter ? (TEXTMETRICA*)painter->textMetric() : d->fin->textMetricA())
#ifdef UNICODE
#define TMW (painter ? (TEXTMETRICW*)painter->textMetric() : d->fin->textMetricW())
#else
#define TMW TMA
#endif
#define TM(F) ( (qt_winver & Qt::WV_NT_based) ? TMW->F : TMA->F )
#define TMX TMA // Initial metrix align


int QFontMetrics::ascent() const
{
    return TMX->tmAscent;
}

int QFontMetrics::descent() const
{
    return TMX->tmDescent;
}

// #### FIXME. This does not work at all.
bool QFontMetrics::inFont(QChar ch) const
{
#ifdef UNICODE
    if ( qt_winver & Qt::WV_NT_based ) {
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
	if ( qt_winver & Qt::WV_NT_based ) {
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
	if ( qt_winver & Qt::WV_NT_based ) {
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
	if ( qt_winver & Qt::WV_NT_based ) {
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
	if ( qt_winver & Qt::WV_NT_based ) {
	    uint ch16 = ch.unicode();
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(hdc(),ch16,ch16,&abc);
	    return int(abc.abcfA);
	} else {
	    return -TMX->tmOverhang;
	}
    }
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
    // Safely cast away const, as we cache rbearing there.
    const QFontDef* def = d->fin->spec();

    if ( def->lbearing == SHRT_MIN ) {
		minRightBearing(); // calculates both
    }

    return def->lbearing;
}

int QFontMetrics::minRightBearing() const
{
    // Safely cast away const, as we cache rbearing there.
    QFontDef* def = (QFontDef*)d->fin->spec();

    if ( def->rbearing == SHRT_MIN ) {
	int ml = 0;
	int mr = 0;
	if (TM(tmPitchAndFamily) & TMPF_TRUETYPE ) {
	    ABC *abc = 0;
	    int n;
	    if ( qt_winver & Qt::WV_NT_based ) {
		TEXTMETRIC *tm = TMW;
		n = tm->tmLastChar - tm->tmFirstChar+1;
		if ( n <= max_font_count ) {
		    abc = new ABC[n];
		    GetCharABCWidths(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
		} else {
		    ml = get_min_left_bearing( this );
		    mr = get_min_right_bearing( this );
		}
	    } else {
		TEXTMETRICA *tm = TMA;
		n = tm->tmLastChar - tm->tmFirstChar+1;
		if ( n <= max_font_count ) {
		    abc = new ABC[n];
		    GetCharABCWidthsA(hdc(),tm->tmFirstChar,tm->tmLastChar,abc);
		} else {
		    ml = get_min_left_bearing( this );
		    mr = get_min_right_bearing( this );
		}
	    }
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
	    if ( qt_winver & Qt::WV_NT_based ) {
		TEXTMETRIC *tm = TMW;
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
    if ( ch.combiningClass() > 0 )
	return 0;
    //qObsolete( "QFontMetrics", "width" );
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

    return d->textWidth( str, 0, len );
}

int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    QChar ch = str[pos];
    if ( ch.combiningClass() > 0 )
	return 0;
    ch = QComplexText::shapedCharacter( str, pos );
    if ( !ch.unicode() )
		return 0;
    SIZE s;
#ifdef UNICODE
    TCHAR tc = ch.unicode();
#else
    TCHAR tc = ch.latin1();
#endif
    GetTextExtentPoint32( hdc(), &tc, 1, &s );
    if ( (qt_winver & Qt::WV_NT_based) == 0 )
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


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

#if 0
const QFontDef *QFontInfo::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}
#endif

