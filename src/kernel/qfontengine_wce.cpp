#include "qfontengine_p.h"
#include <qglobal.h>
#include "qt_windows.h"
#include "qapplication_p.h"
#include <qpainter.h>

#include <qpaintdevice.h>
#include <private/qunicodetables_p.h>

#include <limits.h>

// ### Probably don't need this
HDC   shared_dc	    = 0;		// common dc for all fonts
static HFONT shared_dc_font = 0;		// used by Windows 95/98
static HFONT stock_sysfont  = 0;

static inline HFONT systemFont()
{
    if ( stock_sysfont == 0 )
	stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}

// general font engine

QFontEngine::~QFontEngine()
{
    if ( hdc ) {				// one DC per font (Win NT)
	//SelectObject( hdc, systemFont() );
	if ( !stockFont )
	    DeleteObject( hfont );
	if ( !paintDevice )
	    ReleaseDC( 0, hdc );
	hdc = 0;
	hfont = 0;
    }
}

// ##### get these from windows
int QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 ) lw = 2;
    if ( lw == 0 ) lw = 1;

    return lw;
}

// ##### get these from windows
int QFontEngine::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}


HDC QFontEngine::dc() const
{
    return hdc;
}

void QFontEngine::getCMap()
{
}


// non Uniscribe engine

QFontEngineWin::QFontEngineWin( const char * name, HDC _hdc, HFONT _hfont, bool stockFont, LOGFONT )
{
//    qDebug("regular windows font engine created!");

    _name = name;

    hdc = _hdc;
    hfont = _hfont;
    SelectObject( hdc, hfont );
    stockFont = stockFont;

    lbearing = rbearing = 0;

    BOOL res = GetTextMetricsW( hdc, &tm.w );
#ifndef QT_NO_DEBUG
    if ( !res )
	qSystemWarning( "QFontPrivate: GetTextMetrics failed" );
#endif
    cache_cost = tm.w.tmHeight * tm.w.tmAveCharWidth * 2000;
}

QFontEngine::Error QFontEngineWin::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances,
						 int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( mirrored ) {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = ::mirroredChar(str[i]).unicode();
    } else {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = str[i].unicode();
    }

    if ( advances ) {
	for( int i = 0; i < len; i++ ) {
	    SIZE  size;
	    GetTextExtentPoint32( hdc, (wchar_t *)str, 1, &size );
	    *advances = size.cx;
	    advances++;
	    str++;
	}
    }

    *nglyphs = len;
    return NoError;
}

void QFontEngineWin::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    const unsigned int options = 0;

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    offset_t *offsets = engine->offsets( si );

    // #### fix the other transformations
    if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    y -= ascent();

    if ( !(si->analysis.bidiLevel % 2) ) {
	// fast path
	ExtTextOutW( hdc, x, y, options, 0, (wchar_t *)glyphs, si->num_glyphs, advances );
    } else {
	advances += si->num_glyphs;
	glyphs += si->num_glyphs;
	for( int i = 0; i < si->num_glyphs; i++ ) {
	    glyphs--;
	    advances--;
    	    wchar_t chr = *glyphs;
    	    ExtTextOutW( hdc, x, y, options, 0, &chr, 1, 0 );
	    x += *advances;
	}
    }
}

glyph_metrics_t QFontEngineWin::boundingBox( const glyph_t *glyphs,
				const advance_t *advances, const offset_t *offsets, int numGlyphs )
{
    Q_UNUSED( glyphs );
    Q_UNUSED( offsets );

    if ( numGlyphs == 0 )
	return glyph_metrics_t();

    int w = 0;
    const advance_t *end = advances + numGlyphs;
    while( end > advances )
	w += *(--end);

    return glyph_metrics_t(0, -tm.w.tmAscent, w, tm.w.tmHeight, w, 0 );
}

glyph_metrics_t QFontEngineWin::boundingBox( glyph_t glyph )
{
    SIZE  size;
    wchar_t g = glyph;
    GetTextExtentPoint32( hdc, &g, 1, &size );

    return glyph_metrics_t(0, -tm.w.tmAscent, size.cx, tm.w.tmHeight, size.cx, 0 );
}

int QFontEngineWin::ascent() const
{
    return tm.w.tmAscent;
}

int QFontEngineWin::descent() const
{
    return tm.w.tmDescent;
}

int QFontEngineWin::leading() const
{
    return tm.w.tmExternalLeading;
}

int QFontEngineWin::maxCharWidth() const
{
    return tm.w.tmMaxCharWidth;
}

int QFontEngineWin::minLeftBearing() const
{
    return lbearing;
}

int QFontEngineWin::minRightBearing() const
{
    return rbearing;
}

const char *QFontEngineWin::name() const
{
    return 0;
}

bool QFontEngineWin::canRender( const QChar *string,  int len )
{
#if 0
    while( len-- ) {
	if ( getGlyphIndex( cmap, string->unicode() ) == 0 )
	    return FALSE;
	string++;
    }
#endif
    return TRUE;
}

QFontEngine::Type QFontEngineWin::type() const
{
    return QFontEngine::Win;
}

// box font engine
QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{
    cache_cost = 1;
    hdc = GetDC( 0 );
    hfont = (HFONT)GetStockObject( SYSTEM_FONT );
    stockFont = TRUE;
    paintDevice = FALSE;

//    qDebug("box font engine created!");
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    for ( int i = 0; i < len; i++ )
	*(glyphs++) = str[i].unicode();
    *nglyphs = len;

    if ( advances ) {
	for ( int i = 0; i < len; i++ )
	    *(advances++) = _size;
    }
    return NoError;
}

void QFontEngineBox::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    offset_t *offsets = engine->offsets( si );

    // #### fix the other transformations
    if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    y -= ascent();
    const unsigned int options = 0;

    if ( !(si->analysis.bidiLevel %2) ) {
	// fast path
	ExtTextOutW( hdc, x, y, options, 0, (wchar_t *)glyphs, si->num_glyphs, advances );
    } else {
	advances += si->num_glyphs;
	glyphs += si->num_glyphs;
	for( int i = 0; i < si->num_glyphs; i++ ) {
	    glyphs--;
	    advances--;
    	    wchar_t chr = *glyphs;
    	    ExtTextOutW( hdc, x, y, options, 0, &chr, 1, 0 );
	    x += *advances;
	}
    }
    Rectangle( hdc, x, y, 10, 10 );
}

glyph_metrics_t QFontEngineBox::boundingBox( const glyph_t *, const advance_t *, const offset_t *, int numGlyphs )
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

glyph_metrics_t QFontEngineBox::boundingBox( glyph_t )
{
    return glyph_metrics_t( 0, _size, _size, _size, _size, 0 );
}



int QFontEngineBox::ascent() const
{
    return _size;
}

int QFontEngineBox::descent() const
{
    return 0;
}

int QFontEngineBox::leading() const
{
    int l = qRound( _size * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineBox::maxCharWidth() const
{
    return _size;
}

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender( const QChar *,  int )
{
    return TRUE;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}
