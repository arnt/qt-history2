#include "qfontengine_p.h"
#include <qglobal.h>
#include "qt_windows.h"
#include "qapplication_p.h"

#include <qpaintdevice.h>
#include <limits.h>

// defined in qtextengine_win.cpp
typedef void *SCRIPT_CACHE;
typedef HRESULT (WINAPI *fScriptFreeCache)( SCRIPT_CACHE *);
extern fScriptFreeCache ScriptFreeCache;


static unsigned char *getCMap( HDC hdc );
static Q_UINT16 getGlyphIndex( unsigned char *table, unsigned short unicode );


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
    QT_WA( {
	if ( hdc ) {				// one DC per font (Win NT)
	    //SelectObject( hdc, systemFont() );
	    if ( !stockFont )
		DeleteObject( hfont );
	    if ( !paintDevice )
		ReleaseDC( 0, hdc );
	    hdc = 0;
	    hfont = 0;
	}
    } , {
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
    } );
    delete [] cmap;

    // for Uniscribe
    if ( ScriptFreeCache )
	ScriptFreeCache( &script_cache );
}

HDC QFontEngine::dc() const
{
    if ( hdc || (qt_winver & Qt::WV_NT_based) ) // either NT_based or Printer
	return hdc;
    Q_ASSERT( shared_dc != 0 && hfont != 0 );
    if ( shared_dc_font != hfont ) {
	SelectObject( shared_dc, hfont );
	shared_dc_font = hfont;
    }
    return shared_dc;
}

void QFontEngine::getCMap()
{
    QT_WA( {
	ttf = (bool)(tm.w.tmPitchAndFamily & TMPF_TRUETYPE);
    } , {
	ttf = (bool)(tm.a.tmPitchAndFamily & TMPF_TRUETYPE);
    } );
    HDC hdc = dc();
    SelectObject( hdc, hfont );
    cmap = ttf ? ::getCMap( hdc ) : 0;
    if ( !cmap )
	ttf = false;
    script_cache = 0;
}

void QFontEngine::getGlyphIndexes( const QChar *ch, int numChars, glyph_t *glyphs ) const
{
    if ( ttf ) {
	while( numChars-- ) {
	    *glyphs = getGlyphIndex(cmap, ch->unicode() );
	    glyphs++;
	    ch++;
	}
    } else {
	while( numChars-- ) {
	    *glyphs = ch->unicode();
	    glyphs++;
	    ch++;
	}
    }
}


// non Uniscribe engine

QFontEngineWin::QFontEngineWin( const char * name, HDC _hdc, HFONT _hfont, bool stockFont )
{
    _name = name;

    hdc = _hdc;

    hfont = _hfont;
    stockFont = stockFont;

    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    HGDIOBJ obj = SelectObject( dc(), hfont );
#ifndef QT_NO_DEBUG
    if ( !obj ) {
	qSystemWarning( "QFontPrivate: SelectObject failed" );
    }
#endif
    BOOL res;
    QT_WA( {
	res = GetTextMetricsW( dc(), &tm.w );
    } , {
	res = GetTextMetricsA( dc(), &tm.a );
    } );
#ifndef QT_NO_DEBUG
    if ( !res )
	qSystemWarning( "QFontPrivate: GetTextMetrics failed" );
#endif
    cache_cost = tm.w.tmHeight * tm.w.tmAveCharWidth * 2000;
    getCMap();

    useTextOutA = FALSE;
    // TextOutW doesn't work for symbol fonts on Windows 95!
    // since we're using glyph indices we don't care for ttfs about this!
    if ( qt_winver == Qt::WV_95 && !ttf &&
	 ( _name == "Marlett" || _name == "Symbol" || _name == "Webdings" || _name == "Wingdings" ) )
	    useTextOutA = TRUE;
}


QFontEngine::Error QFontEngineWin::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    getGlyphIndexes( str, len, glyphs );

    if ( advances ) {
	HDC hdc = dc();
	for( int i = 0; i < len; i++ ) {
	    SIZE  size;
	    GetTextExtentPoint32W( hdc, (wchar_t *)str, 1, &size );
	    *advances = size.cx;
	    advances++;
	    str++;
	}
    }

    *nglyphs = len;
    return NoError;
}

void QFontEngineWin::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	   const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse )
{
    Q_UNUSED( p );
    HDC hdc = dc();
    SelectObject( hdc, hfont );
    unsigned int options = 
#ifdef Q_OS_TEMP
	0;
#else
	ETO_NUMERICSLATIN;
    if ( ttf )
	options |= ETO_GLYPH_INDEX;
#endif

    if ( !reverse ) {
	// hack to get symbol fonts working on Win95. See also QFontPrivate::load()
	if ( useTextOutA ) {
	    // can only happen if !ttf
	    for( int i = 0; i < numGlyphs; i++ ) {
    		QChar chr = *glyphs;
		QConstString str( &chr, 1 );
		QCString cstr = str.string().local8Bit();
#ifndef Q_OS_TEMP
		TextOutA( hdc, x + offsets->x, y + offsets->y, cstr.data(), cstr.length() );
#else
		TextOut( hdc, x + offsets->x, y + offsets->y, QString(cstr).ucs2(), cstr.length() );
#endif
		x += *advances;
		glyphs++;
		offsets++;
		advances++;
	    }
	} else {
    		ExtTextOutW( hdc, x + offsets->x, y + offsets->y, options, 0, (wchar_t *)glyphs, numGlyphs, advances );
#if 0
	    for( int i = 0; i < numGlyphs; i++ ) {
    		wchar_t chr = *glyphs;
		TextOutW( hdc, x + offsets->x, y + offsets->y, &chr, 1 );
//    		ExtTextOutW( hdc, x + offsets->x, y + offsets->y, options, 0, &chr, 1, 0 );
		x += *advances;
		glyphs++;
		offsets++;
		advances++;
	    }
#endif
	}
    } else {
	offsets += numGlyphs;
	advances += numGlyphs;
	glyphs += numGlyphs;
	for( int i = 0; i < numGlyphs; i++ ) {
	    glyphs--;
	    offsets--;
	    advances--;
    	    wchar_t chr = *glyphs;
    	    ExtTextOutW( hdc, x + offsets->x, y + offsets->y, options, 0, &chr, 1, 0 );
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
#ifndef Q_OS_TEMP
    GLYPHMETRICS gm;

    if( !ttf ) {
	SIZE s;
	WCHAR ch = glyph;
	BOOL res = GetTextExtentPoint32W( dc(), &ch, 1, &s );
	Q_UNUSED( res );
	return glyph_metrics_t( 0, -tm.a.tmAscent, s.cx, tm.a.tmHeight, s.cx, 0 );
    } else {
	DWORD res = 0;
	MAT2 mat;
	mat.eM11.value = mat.eM22.value = 1;
	mat.eM11.fract = mat.eM22.fract = 0;
	mat.eM21.value = mat.eM12.value = 0;
	mat.eM21.fract = mat.eM12.fract = 0;
	QT_WA( {
	    res = GetGlyphOutlineW( dc(), glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat );
	} , {
	    res = GetGlyphOutlineA( dc(), glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat );
	} );
	if ( res != GDI_ERROR )
	    return glyph_metrics_t( gm.gmptGlyphOrigin.x, -gm.gmptGlyphOrigin.y-gm.gmBlackBoxY, 
				  gm.gmBlackBoxX, gm.gmBlackBoxY, gm.gmCellIncX, gm.gmCellIncY );
    }
#endif
    return glyph_metrics_t();
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

enum { max_font_count = 256 };
static const ushort char_table[] = {
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

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


int QFontEngineWin::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN )
	minRightBearing(); // calculates both

    return lbearing;
}

int QFontEngineWin::minRightBearing() const
{
#ifdef Q_OS_TEMP
	return 0;
#else
    if ( rbearing == SHRT_MIN ) {
	int ml = 0;
	int mr = 0;
	if ( ttf ) {
	    HDC hdc = dc();
	    HGDIOBJ oldobj = SelectObject( hdc, hfont );
	    ABC *abc = 0;
	    int n = QT_WA_INLINE( tm.w.tmLastChar - tm.w.tmFirstChar, tm.a.tmLastChar - tm.a.tmFirstChar );
	    if ( n <= max_font_count ) {
		abc = new ABC[n];
		QT_WA( {
		    GetCharABCWidths(hdc, tm.w.tmFirstChar, tm.w.tmLastChar, abc);
		}, {
		    GetCharABCWidthsA(hdc,tm.a.tmFirstChar,tm.a.tmLastChar,abc);
		} );
	    } else {
		abc = new ABC[char_table_entries];
		QT_WA( {
		    for( int i = 0; i < char_table_entries; i++ )
			GetCharABCWidths(hdc, char_table[i], char_table[i], abc+i);
		}, {
		    for( int i = 0; i < char_table_entries; i++ ) {
			QCString w = QString(QChar(char_table[i])).local8Bit();
			if ( w.length() == 1 ) {
			    uint ch8 = w[0];
			    GetCharABCWidthsA(hdc, ch8, ch8, abc+i );
			}
		    }
		} );
		n = char_table_entries;
	    }    
	    ml = abc[0].abcA;
	    mr = abc[0].abcC;
    	    for ( int i = 1; i < n; i++ ) {
		ml = QMIN(ml,abc[i].abcA);
		mr = QMIN(mr,abc[i].abcC);
	    }
	    delete [] abc;
	} else {
	    QT_WA( {
		ABCFLOAT *abc = 0;
		int n = tm.w.tmLastChar - tm.w.tmFirstChar+1;
		if ( n <= max_font_count ) {
		    abc = new ABCFLOAT[n];
		    GetCharABCWidthsFloat(hdc, tm.w.tmFirstChar, tm.w.tmLastChar, abc);
		} else {
		    abc = new ABCFLOAT[char_table_entries];
		    for( int i = 0; i < char_table_entries; i++ )
			GetCharABCWidthsFloat(hdc, char_table[i], char_table[i], abc+i);
		    n = char_table_entries;
		}    
		float fml = abc[0].abcfA;
		float fmr = abc[0].abcfC;
		for (int i=1; i<n; i++) {
		    fml = QMIN(fml,abc[i].abcfA);
		    fmr = QMIN(fmr,abc[i].abcfC);
		}
		ml = int(fml-0.9999);
		mr = int(fmr-0.9999);
		delete [] abc;
	    } , {
		ml = 0;
		mr = -tm.a.tmOverhang;
	    } );
	}
	((QFontEngine *)this)->lbearing = ml;
	((QFontEngine *)this)->rbearing = mr;
    }

    return rbearing;
#endif
}


const char *QFontEngineWin::name() const
{
    return 0;
}

bool QFontEngineWin::canRender( const QChar *string,  int len )
{
    while( len-- ) {
	if ( getGlyphIndex( cmap, string->unicode() ) == 0 ) 
	    return FALSE;
	string++;
    }
    return TRUE;
}

QFontEngine::Type QFontEngineWin::type() const
{
    return QFontEngine::Win;
}


// Uniscribe engine
#if 0
typedef HRESULT (WINAPI *fScriptTextOut)( const HDC, SCRIPT_CACHE *, int, int, UINT, const RECT *, const QScriptAnalysis *, 
					 const WCHAR *, int, const WORD *, int, const int *, const int *, const GOFFSET *);
extern fScriptTextOut ScriptTextOut;

void QFontEngineUniscribe::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	   const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse )
{
    ScriptTextOut( 
    HDC hdc = dc();
    ScriptTextOut( hdc, script_cache, x, y, 0, 0, analysis, 0, 0, glyphs, numGlyphs, advances, 0, offsets );
}

bool QFontEngineUniscribe::canRender( const QChar *string,  int len )
{
    // ### get default glyph for script and then check.
    return TRUE;
}

QFontEngine::Type QFontEngineUniscribe::type() const
{
    return QFontEngine::Uniscribe;
}

#endif



// box font engine



QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{
    cache_cost = 1;
    hdc = (qt_winver & Qt::WV_NT_based) ? GetDC( 0 ) : shared_dc;
#ifndef Q_OS_TEMP
    hfont = (HFONT)GetStockObject( ANSI_VAR_FONT );
#endif
    stockFont = TRUE;
    paintDevice = FALSE;
    ttf = FALSE;

    cmap = 0;
    script_cache = 0;
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    for ( int i = 0; i < len; i++ )
	*(glyphs++) = 0;
    *nglyphs = len;

    if ( advances ) {
	for ( int i = 0; i < len; i++ )
	    *(advances++) = _size;
    }
    return NoError;
}

void QFontEngineBox::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
			  const advance_t *advances, const offset_t *offsets, int numGlyphs, bool )
{
    Q_UNUSED( p );
    Q_UNUSED( x );
    Q_UNUSED( y );
    Q_UNUSED( glyphs );
    Q_UNUSED( advances );
    Q_UNUSED( offsets );
    Q_UNUSED( numGlyphs );
//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

    // ########
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





// ----------------------------------------------------------------------------
// True type support methods
// ----------------------------------------------------------------------------




#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((DWORD)(ch4)) << 24) | \
    (((DWORD)(ch3)) << 16) | \
    (((DWORD)(ch2)) << 8) | \
    ((DWORD)(ch1)) \
    )

static inline Q_UINT32 getUInt(unsigned char *p)
{
    Q_UINT32 val;
    val = *p++ << 24;
    val |= *p++ << 16;
    val |= *p++ << 8;
    val |= *p;

    return val;
}

static inline Q_UINT16 getUShort(unsigned char *p)
{
    Q_UINT16 val;
    val = *p++ << 8;
    val |= *p;

    return val;
}

static inline void tag_to_string( char *string, Q_UINT32 tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

static Q_UINT16 getGlyphIndex( unsigned char *table, unsigned short unicode )
{
    unsigned short format = getUShort( table );
    if ( format == 0 ) {
	if ( unicode < 256 )
	    return (int) *(table+6+unicode);
    } else if ( format == 2 ) {
	qWarning("format 2 encoding table for Unicode, not implemented!");
    } else if ( format == 4 ) {
	Q_UINT16 segCountX2 = getUShort( table + 6 );
	unsigned char *ends = table + 14;
	Q_UINT16 endIndex = 0;
	int i = 0;
	for ( ; i < segCountX2/2 && (endIndex = getUShort( ends + 2*i )) < unicode; i++ );

	unsigned char *idx = ends + segCountX2 + 2 + 2*i;
	Q_UINT16 startIndex = getUShort( idx );

	if ( startIndex > unicode )
	    return 0;

	idx += segCountX2;
	Q_INT16 idDelta = (Q_INT16)getUShort( idx );
	idx += segCountX2;
	Q_UINT16 idRangeoffset_t = (Q_UINT16)getUShort( idx );

	Q_UINT16 glyphIndex;
	if ( idRangeoffset_t ) {
	    Q_UINT16 id = getUShort( idRangeoffset_t + 2*(unicode - startIndex) + idx);
	    if ( id )
		glyphIndex = ( idDelta + id ) % 0x10000;
	    else
		glyphIndex = 0;
	} else {
	    glyphIndex = (idDelta + unicode) % 0x10000;
	}
	return glyphIndex;
    }

    return 0;
}


static unsigned char *getCMap( HDC hdc )
{
    const DWORD CMAP = MAKE_TAG( 'c', 'm', 'a', 'p' );

    unsigned char header[4];

    // get the CMAP header and the number of encoding tables
    DWORD bytes = 
#ifndef Q_OS_TEMP
	GetFontData( hdc, CMAP, 0, &header, 4 );
#else
	0;
#endif
    if ( bytes == GDI_ERROR )
	return 0;
    unsigned short version = getUShort( header );
    if ( version != 0 )
	return 0;

    unsigned short numTables = getUShort( header+2);
    unsigned char *maps = new unsigned char[8*numTables];

    // get the encoding table and look for Unicode
#ifndef Q_OS_TEMP
    bytes = GetFontData( hdc, CMAP, 4, maps, 8*numTables );
#endif
    if ( bytes == GDI_ERROR )
	return 0;

    unsigned int unicode_table = 0;
    for ( int n = 0; n < numTables; n++ ) {
	Q_UINT32 version = getUInt( maps + 8*n );
	// accept both symbol and Unicode encodings. prefer unicode.
	if ( version == 0x00030001 || version == 0x00030000 ) {
	    unicode_table = getUInt( maps + 8*n + 4 );
	    if ( version == 0x00030001 )
		break;
	}
    }

    if ( !unicode_table ) {
	// qDebug("no unicode table found" );
	return 0;
    }

    delete [] maps;

    // get the header of the unicode table
#ifndef Q_OS_TEMP
    bytes = GetFontData( hdc, CMAP, unicode_table, &header, 4 );
#endif
    if ( bytes == GDI_ERROR )
	return 0;

    unsigned short length = getUShort( header+2 );
    unsigned char *unicode_data = new unsigned char[length];

    // get the cmap table itself
#ifndef Q_OS_TEMP
    bytes = GetFontData( hdc, CMAP, unicode_table, unicode_data, length );
#endif
    if ( bytes == GDI_ERROR ) {
	delete [] unicode_data;
	return 0;
    }
    return unicode_data;
}



