#include <qpaintdevice.h>

#include "fontenginebox.h"

#include <qstring.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include <qt_x11.h>
#include "qtextlayout.h"

#include <stdlib.h>

// #define FONTENGINE_DEBUG


FontEngineBox::FontEngineBox( int size )
    : _size( size )
{

}

FontEngineBox::~FontEngineBox()
{
}

FontEngineIface::Error FontEngineBox::stringToCMap( const QChar *,  int len, GlyphIndex *glyphs, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    for ( int i = 0; i < len; i++ )
	glyphs[i] = 0;
    *nglyphs = len;
    return NoError;
}

void FontEngineBox::draw( QPainter *p, int x, int y, const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs )
{
//     qDebug("FontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    QGlyphInfo ci = boundingBox( glyphs, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
     qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    XRectangle *rects = new XRectangle[numGlyphs];

    for (int k = 0; k < numGlyphs; k++) {
	rects[k].x = x + (k * _size);
	rects[k].y = y - _size + 2;
	rects[k].width = rects[k].height = _size - 3;
    }

    XDrawRectangles(dpy, hd, gc, rects, numGlyphs);
    delete [] rects;

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < numGlyphs; i++ ) {
	QGlyphInfo ci = boundingBox( glyphs[i] );
	x += offsets[i].x;
	y += offsets[i].y;
	p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offset=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y );
	x += ci.xoff;
	y += ci.yoff;
    }
    p->restore();
#endif
}

Offset FontEngineBox::advance( const GlyphIndex *, const Offset *, int numGlyphs )
{
    Offset advance;
    advance.x = _size*numGlyphs;
    advance.y = 0;
    return advance;

}

QGlyphInfo FontEngineBox::boundingBox( const GlyphIndex *, const Offset *, int numGlyphs )
{
    QGlyphInfo overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

QGlyphInfo FontEngineBox::boundingBox( GlyphIndex )
{
    return QGlyphInfo( 0, _size, _size, _size, _size, 0 );
}



int FontEngineBox::ascent() const
{
    return _size;
}

int FontEngineBox::descent() const
{
    return 0;
}

int FontEngineBox::leading() const
{
    int l = qRound( _size * 0.15 );
    return (l > 0) ? l : 1;
}

int FontEngineBox::maxCharWidth() const
{
    return _size;
}

int FontEngineBox::cmap() const
{
    return -1;
}

const char *FontEngineBox::name() const
{
    return "null";
}

bool FontEngineBox::canRender( const QChar *,  int )
{
    return TRUE;
}
