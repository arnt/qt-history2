#include <qpaintdevice.h>

#include "fontenginexft.h"

#include <qstring.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include <qt_x11.h>
#include "qtextlayout.h"

#include <stdlib.h>

// #define DEBUG

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
	// ### not quite correct, should rather use the bounding box here.
	XftDrawRect(draw, &col, x,  y - _font->ascent,
		    advance( glyphs, offsets, numGlyphs ).x,
		    _font->ascent + _font->descent);
    }

    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
#if DEBUG
    p->save();
    p->setBrush( Qt::white );
    QGlyphInfo ci = boundingBox( glyphs, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
#endif
    XftDrawString16 (draw, &col, _font, x, y, (XftChar16 *) glyphs, numGlyphs);
#if DEBUG
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < numGlyphs; i++ ) {
	QGlyphInfo ci = boundingBox( glyphs[i] );
	p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offset=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y );
	x += ci.xoff + offsets[i].x;
	y += ci.yoff + offsets[i].y;
    }
    p->restore();
#endif
}

Offset FontEngineXft::advance( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs )
{
    Offset advance = { 0,  0 };

    XGlyphInfo xgi;

    for (int i = 0; i < numGlyphs; i++) {
	if ( getGlyphInfo( &xgi, _font, glyphs[i] ) ) {
	    advance.x += xgi.xOff;
	    advance.y += xgi.yOff;
	} else {
	    advance.x += ascent();
	}
	advance.x += offsets[i].x;
	advance.y += offsets[i].y;
    }

    return advance;

}

QGlyphInfo FontEngineXft::boundingBox( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs )
{
    XGlyphInfo xgi;

    QGlyphInfo overall;
    int ymax = 0;
    int xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
	overall.xoff += offsets[i].x;
	overall.yoff += offsets[i].y;
	if ( getGlyphInfo( &xgi, _font, glyphs[i] ) ) {
	    overall.x = QMIN( overall.x, overall.xoff - xgi.x );
	    overall.y = QMIN( overall.y, overall.yoff - xgi.y );
	    xmax = QMAX( xmax, overall.xoff - xgi.x + xgi.width );
	    ymax = QMAX( ymax, overall.yoff - xgi.y + xgi.height );
	    overall.xoff += xgi.xOff;
	    overall.yoff -= xgi.yOff;
	} else {
	    int size = ascent();
	    overall.x = QMIN(overall.x, overall.xoff );
	    overall.y = QMIN(overall.y, overall.yoff - size );
	    ymax = QMAX( ymax, overall.yoff );
	    overall.xoff += size;
	    xmax = QMAX( xmax, overall.xoff );
	}
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    return overall;
}

QGlyphInfo FontEngineXft::boundingBox( GlyphIndex glyph )
{
    XGlyphInfo xgi;
    if ( getGlyphInfo( &xgi, _font, glyph ) ) {
	return QGlyphInfo( -xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff );
    }
    int size = ascent();
    return QGlyphInfo( 0, size, size, size, size, 0 );
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
