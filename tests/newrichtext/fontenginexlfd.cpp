#include "fontenginexlfd.h"

#include <qstring.h>
#include <qtextcodec.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include <qt_x11.h>
#include "qtextlayout.h"

#include <stdlib.h>

// #define FONTENGINE_DEBUG

// returns TRUE if the character doesn't exist (ie. zero bounding box)
static inline bool charNonExistent(const XCharStruct *xcs)
{
    return (!xcs || (xcs->width == 0 && xcs->ascent + xcs->descent == 0));
}


// return the XCharStruct for the specified cell in the single dimension font xfs
static inline XCharStruct *getCharStruct1d(XFontStruct *xfs, uint c)
{
    XCharStruct *xcs = 0;
    if (c >= xfs->min_char_or_byte2 &&
	c <= xfs->max_char_or_byte2) {
	if (xfs->per_char != 0) {
	    xcs = xfs->per_char + (c - xfs->min_char_or_byte2);
	    if (charNonExistent(xcs))
		xcs = 0;
	} else
	    xcs = &(xfs->min_bounds);
    }
    return xcs;
}


// return the XCharStruct for the specified row/cell in the 2 dimension font xfs
static inline XCharStruct *getCharStruct2d(XFontStruct *xfs, uint r, uint c)
{
    XCharStruct *xcs = 0;

    if (r >= xfs->min_byte1 &&
	r <= xfs->max_byte1 &&
	c >= xfs->min_char_or_byte2 &&
	c <= xfs->max_char_or_byte2) {
	if (xfs->per_char != 0) {
	    xcs = xfs->per_char + ((r - xfs->min_byte1) *
				   (xfs->max_char_or_byte2 -
				    xfs->min_char_or_byte2 + 1)) +
		  (c - xfs->min_char_or_byte2);
	    if (charNonExistent(xcs))
		xcs = 0;
	} else
	    xcs = &(xfs->min_bounds);
    }

    return xcs;
}

static inline XCharStruct *charStruct( XFontStruct *xfs, int ch )
{
    XCharStruct *xcs;
    if (! xfs->max_byte1)
	// single row font
	xcs = getCharStruct1d(xfs, ch);
    else
	xcs = getCharStruct2d(xfs, (ch>>8), ch&0xff);
    return xcs;
}


FontEngineXLFD::FontEngineXLFD( XFontStruct *fs, const char *name, QTextCodec *codec, int cmap )
    : _fs( fs ), _name( name ), _codec( codec ), _scale( 1. ), _cmap( cmap )
{

}

FontEngineXLFD::~FontEngineXLFD()
{
    XFreeFont( QPaintDevice::x11AppDisplay(), _fs );
    _fs = 0;
}

FontEngineIface::Error FontEngineXLFD::stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( _codec ) {
	// ### avoid deep copy
	QString string( str, len );
	QCString cstr = _codec->fromUnicode( string );

	// ### doesn't work with chinese!
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = (uchar)cstr[i];
    } else {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = str[i].unicode();
    }
    *nglyphs = len;
    return NoError;
}

void FontEngineXLFD::draw( QPainter *p, int x, int y, const GlyphIndex *glyphs,
			   const Offset *advances, const Offset *offsets, int numGlyphs, bool reverse )
{
    if ( !numGlyphs )
	return;
//     qDebug("FontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;
    Qt::BGMode bgmode = p->backgroundMode();

    Qt::HANDLE fid_last = 0;

    if (_fs->fid != fid_last) {
	XSetFont(dpy, gc, _fs->fid);
	fid_last = _fs->fid;
    }
#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    QGlyphMetrics ci = boundingBox( glyphs, advances, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
     qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    XChar2b ch[256];
    XChar2b *chars = ch;
    if ( numGlyphs > 255 )
	chars = (XChar2b *)malloc( numGlyphs*sizeof(XChar2b) );

    for (int i = 0; i < numGlyphs; i++) {
	chars[i].byte1 = glyphs[i] >> 8;
	chars[i].byte2 = glyphs[i] & 0xff;
    }

    if ( reverse ) {
	int i = numGlyphs;
	while( i-- ) {
	    Offset adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv.x;
	    y += adv.y;
	    QGlyphMetrics gi = boundingBox( glyphs[i] );
	    if (bgmode != Qt::TransparentMode)
		XDrawImageString16(dpy, hd, gc, x-offsets[i].x-gi.xoff, y+offsets[i].y-gi.yoff, chars+i, 1 );
	    else
		XDrawString16(dpy, hd, gc, x-offsets[i].x-gi.xoff, y+offsets[i].y-gi.yoff, chars+i, 1 );
	}
    } else {
	int i = 0;
	while( i < numGlyphs ) {
	    // ### might not work correctly with marks!
	    if (bgmode != Qt::TransparentMode)
		XDrawImageString16(dpy, hd, gc, x+offsets[i].x, y+offsets[i].y, chars+i, 1 );
	    else
		XDrawString16(dpy, hd, gc, x+offsets[i].x, y+offsets[i].y, chars+i, 1 );
	    Offset adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv.x;
	    y += adv.y;
	    i++;
	}
    }

    if ( numGlyphs > 255 )
	free( chars );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < numGlyphs; i++ ) {
	QGlyphMetrics ci = boundingBox( glyphs[i] );
	p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y,
	       advances[i].x, advances[i].y);
	x += advances[i].x;
	y += advances[i].y;
    }
    p->restore();
#endif
}

QGlyphMetrics FontEngineXLFD::boundingBox( const GlyphIndex *glyphs, const Offset *advances, const Offset *offsets, int numGlyphs )
{
    int i;

    QGlyphMetrics overall;
    int ymax = 0;
    int xmax = 0;
    for (i = 0; i < numGlyphs; i++) {
	XCharStruct *xcs = charStruct( _fs, glyphs[i] );
	if (xcs) {
	    int x = overall.xoff + offsets[i].x - xcs->lbearing;
	    int y = overall.yoff + offsets[i].y - xcs->ascent;
	    overall.x = QMIN( overall.x, x );
	    overall.y = QMIN( overall.y, y );
	    xmax = QMAX( xmax, overall.xoff + offsets[i].x + xcs->rbearing );
	    ymax = QMAX( ymax, y + xcs->ascent + xcs->descent );
	    overall.xoff += advances[i].x;
	    overall.yoff += advances[i].y;
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

    overall.x = (int)(overall.x * _scale);
    overall.y = (int)(overall.y * _scale);
    overall.height = (int)(overall.height * _scale);
    overall.width = (int)(overall.width * _scale);
    overall.xoff = (int)(overall.xoff * _scale);
    overall.yoff = (int)(overall.yoff * _scale);
    return overall;
}

QGlyphMetrics FontEngineXLFD::boundingBox( GlyphIndex glyph )
{
    // ### scale missing!
    XCharStruct *xcs = charStruct( _fs, glyph );
    if (xcs) {
	return QGlyphMetrics( xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent, xcs->width, 0 );
    }
    int size = ascent();
    return QGlyphMetrics( 0, size, size, size, size, 0 );
}


int FontEngineXLFD::ascent() const
{
    return (int)(_fs->ascent*_scale);
}

int FontEngineXLFD::descent() const
{
    return (int)(_fs->descent*_scale);
}

int FontEngineXLFD::leading() const
{
    int l = qRound((QMIN(_fs->ascent, _fs->max_bounds.ascent)
		    + QMIN(_fs->descent, _fs->max_bounds.descent)) * _scale * 0.15 );
    return (l > 0) ? l : 1;
}

int FontEngineXLFD::maxCharWidth() const
{
    return (int)(_fs->max_bounds.width*_scale);
}

int FontEngineXLFD::cmap() const
{
    return _cmap;
}

const char *FontEngineXLFD::name() const
{
    return _name;
}

bool FontEngineXLFD::canRender( const QChar *string,  int len )
{
    GlyphIndex glyphs[256];
    int nglyphs = 255;
    GlyphIndex *g = glyphs;
    if ( stringToCMap( string, len, g, &nglyphs ) == OutOfMemory ) {
	g = (GlyphIndex *)malloc( nglyphs*sizeof(GlyphIndex) );
	stringToCMap( string, len, g, &nglyphs );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !charStruct( _fs, g[i] ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( nglyphs > 255 )
	free( g );
	return allExist;
}


void FontEngineXLFD::setScale( double scale )
{
    _scale = scale;
}


FontEngineIface::Type FontEngineXLFD::type() const
{
    return Xlfd;
}
