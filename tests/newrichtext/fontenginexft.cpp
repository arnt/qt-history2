#include <qpaintdevice.h>

#include "fontenginexft.h"

#include <qstring.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include <qt_x11.h>
#include "qtextlayout.h"

#include <stdlib.h>

class HackPaintDevice : public QPaintDevice
{
public:
    HackPaintDevice() : QPaintDevice( 0 ) {}
    XftDraw *xftDrawHandle() const {
	return (XftDraw *)rendhd;
    }
};

// ditto
static inline bool getGlyphInfo(XGlyphInfo *xgi, XftFont *font, int glyph)
{
    if (XftGlyphExists(QPaintDevice::x11AppDisplay(), font, glyph)) {
	XftTextExtents32(QPaintDevice::x11AppDisplay(), font, (XftChar32 *) &glyph, 1, xgi);
	return TRUE;
    }
    return FALSE;
}




FontEngineXft::FontEngineXft( XftFont *font, XftPattern *pattern, int cmap )
    : _font( font ), _pattern( pattern ), _cmap( cmap )
{

}

FontEngineXft::~FontEngineXft()
{
    XftFontClose( QPaintDevice::x11AppDisplay(),_font );
    XftPatternDestroy( _pattern );
    _font = 0;
    _pattern = 0;
}

FontEngineIface::Error FontEngineXft::stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs, bool reverse ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( reverse ) {
	int pos = len - 1;
	for ( int i = 0; i < len; i++ ) {
	    glyphs[i] = str[pos].unicode();
	    pos--;
	}
    } else {
	for ( int i = 0; i < len; i++ ) {
	    glyphs[i] = str[i].unicode();
	}
    }
    *nglyphs = len;
    return NoError;
}

void FontEngineXft::draw( QPainter *p, int x, int y, const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs )
{
    Qt::BGMode bgmode = p->backgroundMode();
    XftDraw *draw = ((HackPaintDevice *)p->device())->xftDrawHandle();
    const QColor &bgcolor = p->backgroundColor();
    const QColor &pen = p->pen().color();

    if (bgmode != Qt::TransparentMode) {
	XftColor col;
	col.color.red = bgcolor.red()     | bgcolor.red() << 8;
	col.color.green = bgcolor.green() | bgcolor.green() << 8;
	col.color.blue = bgcolor.blue()   | bgcolor.blue() << 8;
	col.color.alpha = 0xffff;
	col.pixel = bgcolor.pixel();
	XftDrawRect(draw, &col, x,  y - _font->ascent,
		    width( glyphs, offsets, numGlyphs ),
		    _font->ascent + _font->descent);
    }

    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
    XftDrawString16 (draw, &col, _font, x, y, (XftChar16 *) glyphs, numGlyphs);
}

int FontEngineXft::width( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs )
{
    int width = 0;

    XGlyphInfo xgi;

    for (int i = 0; i < numGlyphs; i++) {
	if ( getGlyphInfo( &xgi, _font, glyphs[i] ) )
	    width += xgi.xOff;
	else
	    width += ascent();
	width += offsets[i].x;
    }

    return width;

}

QCharStruct FontEngineXft::boundingBox( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs )
{
    XGlyphInfo xgi;

    QCharStruct overall;
    int x = 0;
    int y = 0;

    for (int i = 0; i < numGlyphs; i++) {
	x += offsets[i].x;
	y += offsets[i].y;
	if ( getGlyphInfo( &xgi, _font, glyphs[i] ) ) {
	    overall.ascent = QMAX(overall.ascent, xgi.y);
	    overall.descent = QMAX(overall.descent, (xgi.height - xgi.y));
	    overall.lbearing = QMIN(overall.lbearing, -xgi.x);
	    overall.rbearing = QMAX(overall.rbearing, x + (xgi.width - xgi.x));
	    x += xgi.xOff;
	} else {
	    int size = ascent();
	    overall.ascent = QMAX(overall.ascent, size+y);
	    overall.descent = QMAX(overall.descent, y);
	    overall.lbearing = QMIN(overall.lbearing, x);
	    x += size;
	    overall.rbearing = QMAX(overall.rbearing, x);
	}
	overall.width = QMAX( overall.width, x );
    }
    return overall;
}

int FontEngineXft::ascent() const
{
    return _font->ascent;
}

int FontEngineXft::descent() const
{
    return _font->descent;
}

int FontEngineXft::leading() const
{
    int l = qRound( (ascent() + descent() ) * 0.15 );
    return (l > 0) ? l : 1;
}

int FontEngineXft::maxCharWidth() const
{
    return _font->max_advance_width;
}

int FontEngineXft::cmap() const
{
    return _cmap;
}

const char *FontEngineXft::name() const
{
    return "xft";
}

bool FontEngineXft::canRender( const QChar *string,  int len )
{
    GlyphIndex glyphs[256];
    int nglyphs = 255;
    GlyphIndex *g = glyphs;
    if ( stringToCMap( string, len, g, &nglyphs, FALSE ) == OutOfMemory ) {
	g = (GlyphIndex *)malloc( nglyphs*sizeof(GlyphIndex) );
	stringToCMap( string, len, g, &nglyphs, FALSE );
    }

    bool allExist = TRUE;
    XGlyphInfo xgi;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !getGlyphInfo( &xgi, _font, g[i] ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( nglyphs > 255 )
	free( g );
	return allExist;

}
