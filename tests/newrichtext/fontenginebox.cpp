#include <qpaintdevice.h>

#include "fontenginebox.h"

#include <qstring.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include <qt_x11.h>
#include "qtextengine.h"

#include <stdlib.h>

// #define FONTENGINE_DEBUG


QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{

}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngineIface::Error QFontEngineBox::stringToCMap( const QChar *,  int len, glyph_t *glyphs, int *nglyphs ) const
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

void QFontEngineBox::draw( QPainter *p, int x, int y, const glyph_t */*glyphs*/,
			  const offset_t */*advances*/, const offset_t */*offsets*/, int numGlyphs, bool )
{
//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    QGlyphMetrics ci = boundingBox( glyphs, offsets, numGlyphs );
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
	QGlyphMetrics ci = boundingBox( glyphs[i] );
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

QGlyphMetrics QFontEngineBox::boundingBox( const glyph_t *, const offset_t *, const offset_t *, int numGlyphs )
{
    QGlyphMetrics overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

QGlyphMetrics QFontEngineBox::boundingBox( glyph_t )
{
    return QGlyphMetrics( 0, _size, _size, _size, _size, 0 );
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

int QFontEngineBox::cmap() const
{
    return -1;
}

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender( const QChar *,  int )
{
    return TRUE;
}

QFontEngineIface::Type QFontEngineBox::type() const
{
    return Box;
}
