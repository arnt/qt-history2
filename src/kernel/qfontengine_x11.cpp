#include "qfontengine_p.h"

// #define FONTENGINE_DEBUG

#include <qcstring.h>
#include <qtextcodec.h>
#include <qpaintdevice.h>
#include "qpainter.h"

#include <qt_x11.h>

#include "qtextengine_p.h"
#include "qfont.h"

// defined in qfontdatbase_x11.cpp
extern int qt_mibForXlfd( const char * encoding );

QFontEngine::~QFontEngine()
{
}

// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------


QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{
    cache_cost = 1;
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

void QFontEngineBox::draw( QPainter *p, int x, int y, const glyph_t */*glyphs*/,
			  const advance_t */*advances*/, const offset_t */*offsets*/, int numGlyphs, bool )
{
//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, offsets, numGlyphs );
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
	glyph_metrics_t ci = boundingBox( glyphs[i] );
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

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}




// ------------------------------------------------------------------
// Xlfd cont engine
// ------------------------------------------------------------------


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


QFontEngineXLFD::QFontEngineXLFD( XFontStruct *fs, const char *name, const char *encoding, int cmap )
    : _fs( fs ), _name( name ), _codec( 0 ), _scale( 1. ), _cmap( cmap )
{
    int mib = qt_mibForXlfd( encoding );
    if ( mib ) _codec = QTextCodec::codecForMib( mib );

    cache_cost = (((fs->max_byte1 - fs->min_byte1) *
		   (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)) +
		  fs->max_char_or_byte2 - fs->min_char_or_byte2);
    cache_cost = ((fs->max_bounds.ascent + fs->max_bounds.descent) *
		  (fs->max_bounds.width * cache_cost / 8));
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont( QPaintDevice::x11AppDisplay(), _fs );
    _fs = 0;
}

QFontEngine::Error QFontEngineXLFD::stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( _codec ) {
	_codec->fromUnicodeInternal( str, glyphs, len );
    } else {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = str[i].unicode();
    }
    *nglyphs = len;

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    XCharStruct *xcs = charStruct( _fs, glyphs[i] );
	    advances[i] = (int)((xcs ? xcs->width : _fs->ascent)*_scale);
	}
    }
    return NoError;
}

void QFontEngineXLFD::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
			   const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse )
{
    if ( !numGlyphs )
	return;
//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

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
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, numGlyphs );
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
	    advance_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
	    glyph_metrics_t gi = boundingBox( glyphs[i] );
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
	    advance_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
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
	glyph_metrics_t ci = boundingBox( glyphs[i] );
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

glyph_metrics_t QFontEngineXLFD::boundingBox( const glyph_t *glyphs, const advance_t *advances, const offset_t *offsets, int numGlyphs )
{
    int i;

    glyph_metrics_t overall;
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
	    overall.xoff += advances[i];
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

glyph_metrics_t QFontEngineXLFD::boundingBox( glyph_t glyph )
{
    // ### scale missing!
    XCharStruct *xcs = charStruct( _fs, glyph );
    if (xcs) {
	return glyph_metrics_t( xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent, xcs->width, 0 );
    }
    int size = ascent();
    return glyph_metrics_t( 0, size, size, size, size, 0 );
}


int QFontEngineXLFD::ascent() const
{
    return (int)(_fs->ascent*_scale);
}

int QFontEngineXLFD::descent() const
{
    return (int)(_fs->descent*_scale);
}

int QFontEngineXLFD::leading() const
{
    int l = qRound((QMIN(_fs->ascent, _fs->max_bounds.ascent)
		    + QMIN(_fs->descent, _fs->max_bounds.descent)) * _scale * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineXLFD::maxCharWidth() const
{
    return (int)(_fs->max_bounds.width*_scale);
}

int QFontEngineXLFD::cmap() const
{
    return _cmap;
}

const char *QFontEngineXLFD::name() const
{
    return _name;
}

bool QFontEngineXLFD::canRender( const QChar *string,  int len )
{
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !g[i] || !charStruct( _fs, g[i] ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( nglyphs > 255 )
	free( g );

    return allExist;
}


void QFontEngineXLFD::setScale( double scale )
{
    _scale = scale;
}


QFontEngine::Type QFontEngineXLFD::type() const
{
    return Xlfd;
}


// ------------------------------------------------------------------
// Xft cont engine
// ------------------------------------------------------------------


#ifndef QT_NO_XFTFREETYPE
class Q_HackPaintDevice : public QPaintDevice
{
public:
    Q_HackPaintDevice() : QPaintDevice( 0 ) {}
    inline XftDraw *xftDrawHandle() const {
	return (XftDraw *)rendhd;
    }

};


#ifdef QT_XFT2
static inline void getGlyphInfo( XGlyphInfo *xgi, XftFont *font, int glyph )
{
    FT_UInt x = glyph;
    XftGlyphExtents( QPaintDevice::x11AppDisplay(), font, &x, 1, xgi );
}
#else
static inline XftFontStruct *getFontStruct( XftFont *font )
{
    if (font->core)
	return 0;
    return font->u.ft.font;
}

static inline void getGlyphInfo(XGlyphInfo *xgi, XftFont *font, int glyph)
{

    XftTextExtents32(QPaintDevice::x11AppDisplay(), font, (XftChar32 *) &glyph, 1, xgi);
}
#endif // QT_XFT2

static inline FT_Face lockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    return XftLockFace( font );
#else
    if (font->core) return 0;
    return font->u.ft.font->face;
#endif // QT_XFT2
}

static inline void unlockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    XftUnlockFace( font );
#else
    Q_UNUSED( font );
#endif // QT_XFT2
}



QFontEngineXft::QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap )
    : _font( font ), _pattern( pattern ), _openType( 0 ), _cmap( cmap )
{
#ifndef QT_XFT2
    XftFontStruct *xftfs = getFontStruct( _font );
    if ( xftfs ) {
	// dirty hack: we set the charmap in the Xftfreetype to -1, so
	// XftFreetype assumes no encoding and really draws glyph
	// indices. The FT_Face still has the Unicode encoding to we
	// can convert from Unicode to glyph index
	xftfs->charmap = -1;
    }
#endif // QT_XFT2

    _face = lockFTFace( _font );

    cache_cost = _font->height * _font->max_advance_width *
		 ( _face ? _face->num_glyphs : 1024 );

    // if the Xft font is not antialiased, it uses bitmaps instead of
    // 8-bit alpha maps... adjust the cache_cost to reflect this
    Bool antialiased = TRUE;
    if ( XftPatternGetBool( pattern, XFT_ANTIALIAS,
			    0, &antialiased ) == XftResultMatch &&
	 ! antialiased ) {
	cache_cost /= 8;
    }
}

QFontEngineXft::~QFontEngineXft()
{
    delete _openType;
    unlockFTFace( _font );

    XftFontClose( QPaintDevice::x11AppDisplay(),_font );
    XftPatternDestroy( _pattern );
    _font = 0;
    _pattern = 0;
}

QFontEngine::Error QFontEngineXft::stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

#ifdef QT_XFT2
    for ( int i = 0; i < len; ++i )
	glyphs[i] =
	    XftCharIndex( QPaintDevice::x11AppDisplay(), _font, str[i].unicode() );

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    XGlyphInfo gi;
	    FT_UInt glyph = *(glyphs + i);
	    XftGlyphExtents( QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi );
	    *(advances + i) = gi.xOff;
	}
    }
#else
    if ( !_face ) {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = str[i].unicode();
    } else {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = FT_Get_Char_Index (_face, str[i].unicode() );
    }

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    XGlyphInfo gi;
	    XftTextExtents16(QPaintDevice::x11AppDisplay(), _font,
			     (XftChar16 *) glyphs+i, 1, &gi);
	    *(advances++) = gi.xOff;
	}
    }
#endif // QT_XFT2

    *nglyphs = len;
    return NoError;
}

void QFontEngineXft::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
			   const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse )
{
    if ( !numGlyphs )
	return;

    Qt::BGMode bgmode = p->backgroundMode();
    XftDraw *draw = ((Q_HackPaintDevice *)p->device())->xftDrawHandle();
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
	glyph_metrics_t bb = boundingBox( glyphs, advances, offsets, numGlyphs );
	XftDrawRect(draw, &col, x+bb.x,  y + bb.y, bb.width, bb.height );
    }

    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
//     qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif
    if ( reverse ) {
	int i = numGlyphs;
	while( i-- ) {
	    advance_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
	    glyph_metrics_t gi = boundingBox( glyphs[i] );
#ifdef QT_XFT2
	    FT_UInt glyph = *(glyphs + i);
	    XftDrawGlyphs( draw, &col, _font, x-offsets[i].x-gi.xoff,
			   y+offsets[i].y-gi.yoff, &glyph, 1 );
#else
	    XftDrawString16 (draw, &col, _font, x-offsets[i].x-gi.xoff,
			     y+offsets[i].y-gi.yoff, (XftChar16 *) (glyphs+i), 1);
#endif // QT_XFT2
#ifdef FONTENGINE_DEBUG
	    p->drawRect( x - offsets[i].x - gi.xoff + gi.x, y + 100 + offsets[i].y - gi.yoff + gi.y, gi.width, gi.height );
	    p->drawLine( x - offsets[i].x - gi.xoff, y + 150 + 5*i , x - offsets[i].x, y + 150 + 5*i );

#endif
	}
    } else {
	int i = 0;
	while ( i < numGlyphs ) {
#ifdef QT_XFT2
	    FT_UInt glyph = *(glyphs + i);
	    XftDrawGlyphs( draw, &col, _font, x+offsets[i].x,
			   y+offsets[i].y, &glyph, 1 );
#else
	    XftDrawString16( draw, &col, _font, x+offsets[i].x,
			     y+offsets[i].y, (XftChar16 *) (glyphs+i), 1 );
#endif // QT_XFT2
	    advance_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
	    i++;
	}
    }
#ifdef FONTENGINE_DEBUG
    if ( !reverse ) {
	x = xp;
	y = yp;
	p->save();
	p->setPen( Qt::red );
	for ( int i = 0; i < numGlyphs; i++ ) {
	    glyph_metrics_t ci = boundingBox( glyphs[i] );
	    p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	    qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
		   ci.xoff, ci.yoff, offsets[i].x, offsets[i].y,
		   advances[i].x, advances[i].y);
	    x += advances[i].x;
	    y += advances[i].y;
	}
	p->restore();
    }
#endif
}

glyph_metrics_t QFontEngineXft::boundingBox( const glyph_t *glyphs, const advance_t *advances, const offset_t *offsets, int numGlyphs )
{
    XGlyphInfo xgi;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
	getGlyphInfo( &xgi, _font, glyphs[i] );
	int x = overall.xoff + offsets[i].x - xgi.x;
	int y = overall.yoff + offsets[i].y - xgi.y;
	overall.x = QMIN( overall.x, x );
	overall.y = QMIN( overall.y, y );
	xmax = QMAX( xmax, x + xgi.width );
	ymax = QMAX( ymax, y + xgi.height );
	overall.xoff += advances[i];
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    return overall;
}

glyph_metrics_t QFontEngineXft::boundingBox( glyph_t glyph )
{
    XGlyphInfo xgi;
    getGlyphInfo( &xgi, _font, glyph );
    return glyph_metrics_t( -xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff );
}



int QFontEngineXft::ascent() const
{
    return _font->ascent;
}

int QFontEngineXft::descent() const
{
    return _font->descent;
}

int QFontEngineXft::leading() const
{
    int l = qRound( (ascent() + descent() ) * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineXft::maxCharWidth() const
{
    return _font->max_advance_width;
}

int QFontEngineXft::cmap() const
{
    return _cmap;
}

const char *QFontEngineXft::name() const
{
    return "xft";
}

bool QFontEngineXft::canRender( const QChar *string,  int len )
{
    bool allExist = TRUE;

#ifdef QT_XFT2
    for ( int i = 0; i < len; i++ ) {
	if ( ! XftCharExists( QPaintDevice::x11AppDisplay(), _font,
			      string[i].unicode() ) ) {
	    allExist = FALSE;
	    break;
	}
    }
#else
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs );
    }

    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !XftGlyphExists(QPaintDevice::x11AppDisplay(), _font, g[i]) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( nglyphs > 255 )
	free( g );
#endif // QT_XFT2

    return allExist;
}

QOpenType *QFontEngineXft::openType() const
{
//     qDebug("openTypeIface requested!");
    if ( _openType )
	return _openType;

    QFontEngineXft *that = (QFontEngineXft *)this;
    that->_openType = new QOpenType( that->_face );
    return _openType;
}


QFontEngine::Type QFontEngineXft::type() const
{
    return Xft;
}
#endif


//  --------------------------------------------------------------------------------------------------------------------
// Open type support
//  --------------------------------------------------------------------------------------------------------------------

#ifndef QT_NO_XFTFREETYPE

static inline void tag_to_string( char *string, FT_ULong tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG( 'D', 'F', 'L', 'T' )

struct Features {
    FT_ULong tag;
    unsigned short bit;
};

// GPOS features are always applied. We only have to note the GSUB features that should not get
// applied in all cases here.

const Features standardFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x8000 },
    { FT_MAKE_TAG( 'c', 'l', 'i', 'g' ), 0x8000 },
    { 0,  0 }
};

// always keep in sync with Shape enum in scriptenginearabic.cpp
const Features arabicFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), 0x01 },
    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), 0x02 },
    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), 0x04 },
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), 0x08 },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), 0x4000 },
    { FT_MAKE_TAG( 'c', 'a', 'l', 't' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x4000 },
    { FT_MAKE_TAG( 'd', 'l', 'i', 'g' ), 0x8000 },
    // mset is used in old Win95 fonts that don't have a 'mark' positioning table.
    { FT_MAKE_TAG( 'm', 's', 'e', 't' ), 0x8000 },
    { 0,  0 }
};

const Features syriacFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), 0x01 },
    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), 0x02 },
    { FT_MAKE_TAG( 'f', 'i', 'n', '2' ), 0x02 },
    { FT_MAKE_TAG( 'f', 'i', 'n', '3' ), 0x02 },
    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), 0x04 },
    { FT_MAKE_TAG( 'm', 'e', 'd', '2' ), 0x04 },
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), 0x08 },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), 0x4000 },
    { FT_MAKE_TAG( 'c', 'a', 'l', 't' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x8000 },
    { FT_MAKE_TAG( 'd', 'l', 'i', 'g' ), 0x8000 },
    { 0,  0 }
};

const Features indicFeatures[] = {
    // Language based forms
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), InitFeature },
    { FT_MAKE_TAG( 'n', 'u', 'k', 't' ), NuktaFeature },
    { FT_MAKE_TAG( 'a', 'k', 'h', 'n' ), AkhantFeature },
    { FT_MAKE_TAG( 'r', 'p', 'h', 'f' ), RephFeature },
    { FT_MAKE_TAG( 'b', 'l', 'w', 'f' ), BelowFormFeature },
    { FT_MAKE_TAG( 'h', 'a', 'l', 'f' ), HalfFormFeature },
    { FT_MAKE_TAG( 'p', 's', 'b', 'f' ), PostFormFeature },
    { FT_MAKE_TAG( 'v', 'a', 't', 'u' ), VattuFeature },
    // Conjunkts and typographical forms
    { FT_MAKE_TAG( 'p', 'r', 'e', 's' ), PreSubstFeature },
    { FT_MAKE_TAG( 'b', 'l', 'w', 's' ), BelowSubstFeature },
    { FT_MAKE_TAG( 'a', 'b', 'v', 's' ), AboveSubstFeature },
    { FT_MAKE_TAG( 'p', 's', 't', 's' ), PostSubstFeature },
    // halant forms
    { FT_MAKE_TAG( 'h', 'a', 'l', 'n' ), HalantFeature },
    { 0,  0 }
};

const Features tibetanFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'b', 'l', 'w', 's' ), BelowSubstFeature },
    { FT_MAKE_TAG( 'a', 'b', 'v', 's' ), AboveSubstFeature }
};


struct SupportedScript {
    FT_ULong tag;
    const Features *features;
    unsigned short required_bits;
    unsigned short always_apply;
};


const SupportedScript supported_scripts [] = {
// 	// European Alphabetic Scripts
// 	Latin,
    { FT_MAKE_TAG( 'l', 'a', 't', 'n' ), standardFeatures, 0x0000, 0x8000 },
// 	Greek,
    { FT_MAKE_TAG( 'g', 'r', 'e', 'k' ), standardFeatures, 0x0000, 0x8000 },
// 	Cyrillic,
    { FT_MAKE_TAG( 'c', 'y', 'r', 'l' ), standardFeatures, 0x0000, 0x8000 },
// 	Armenian,
        { FT_MAKE_TAG( 'a', 'r', 'm', 'n' ), standardFeatures, 0x0000, 0x8000 },
// 	Georgian,
    { FT_MAKE_TAG( 'g', 'e', 'o', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Runic,
    { FT_MAKE_TAG( 'r', 'u', 'n', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Ogham,
    { FT_MAKE_TAG( 'o', 'g', 'a', 'm' ), standardFeatures, 0x0000, 0x8000 },
// 	SpacingModifiers,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	CombiningMarks,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },

// 	// Middle Eastern Scripts
// 	Hebrew,
    { FT_MAKE_TAG( 'h', 'e', 'b', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Arabic,
    { FT_MAKE_TAG( 'a', 'r', 'a', 'b' ), arabicFeatures, 0x400e, 0xc000 },
// 	Syriac,
    { FT_MAKE_TAG( 's', 'y', 'r', 'c' ), syriacFeatures, 0x400e, 0xc000 },
// 	Thaana,
    { FT_MAKE_TAG( 't', 'h', 'a', 'a' ), standardFeatures, 0x0000, 0x8000 },

// 	// South and Southeast Asian Scripts
// 	Devanagari,
    { FT_MAKE_TAG( 'd', 'e', 'v', 'a' ), indicFeatures, 0x1fbe, 0x8f00 },
// 	Bengali,
    { FT_MAKE_TAG( 'b', 'e', 'n', 'g' ), indicFeatures, 0x1fff, 0x8f00 },
// 	Gurmukhi,
    { FT_MAKE_TAG( 'g', 'u', 'r', 'u' ), indicFeatures, 0x1fd2, 0x8f00 },
// 	Gujarati,
    { FT_MAKE_TAG( 'g', 'u', 'j', 'r' ), indicFeatures, 0x1fbe, 0x8f00 },
// 	Oriya,
    { FT_MAKE_TAG( 'o', 'r', 'y', 'a' ), indicFeatures, 0x1ffe, 0x8f00 },
// 	Tamil,
    { FT_MAKE_TAG( 't', 'a', 'm', 'l' ), indicFeatures, 0x1f24, 0x8f00 },
// 	Telugu,
    { FT_MAKE_TAG( 't', 'e', 'l', 'u' ), indicFeatures, 0x1e14, 0x8f00 },
// 	Kannada,
    { FT_MAKE_TAG( 'k', 'n', 'd', 'a' ), indicFeatures, 0x1e1c, 0x8f00 },
// 	Malayalam,
    { FT_MAKE_TAG( 'm', 'l', 'y', 'm' ), indicFeatures, 0x1f7c, 0x8f00 },
// 	Sinhala,
    // ### could not find any OT specs on this
    { FT_MAKE_TAG( 's', 'i', 'n', 'h' ), standardFeatures, 0x0000, 0x8000 },
// 	Thai,
    { FT_MAKE_TAG( 't', 'h', 'a', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Lao,
    { FT_MAKE_TAG( 'l', 'a', 'o', ' ' ), standardFeatures, 0x0000, 0x8000 },
// 	Tibetan,
    { FT_MAKE_TAG( 't', 'i', 'b', 't' ), tibetanFeatures, AboveSubstFeature|BelowSubstFeature, 0x8000 },
// 	Myanmar,
    { FT_MAKE_TAG( 'm', 'y', 'm', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Khmer,
    { FT_MAKE_TAG( 'k', 'h', 'm', 'r' ), standardFeatures, 0x0000, 0x8000 },

// 	// East Asian Scripts
// 	Han,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Hiragana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Katakana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Hangul,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'g' ), standardFeatures, 0x0000, 0x8000 },
// 	Bopomofo,
    { FT_MAKE_TAG( 'b', 'o', 'p', 'o' ), standardFeatures, 0x0000, 0x8000 },
// 	Yi,
    { FT_MAKE_TAG( 'y', 'i', ' ', ' ' ), standardFeatures, 0x0000, 0x8000 },

// 	// Additional Scripts
// 	Ethiopic,
    { FT_MAKE_TAG( 'e', 't', 'h', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Cherokee,
    { FT_MAKE_TAG( 'c', 'h', 'e', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	CanadianAboriginal,
    { FT_MAKE_TAG( 'c', 'a', 'n', 's' ), standardFeatures, 0x0000, 0x8000 },
// 	Mongolian,
    { FT_MAKE_TAG( 'm', 'o', 'n', 'g' ), standardFeatures, 0x0000, 0x8000 },

// 	// Symbols
// 	CurrencySymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	LetterlikeSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	NumberForms,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	MathematicalOperators,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	TechnicalSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	GeometricSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	MiscellaneousSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	EnclosedAndSquare,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	Braille,
    { FT_MAKE_TAG( 'b', 'r', 'a', 'i' ), standardFeatures, 0x0000, 0x8000 },

//                Unicode, should be used
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 }
    // ### where are these?
// 	FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
//     FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
    // ### Hangul Jamo
//     FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
};


bool QOpenType::loadTables( FT_ULong script)
{
    always_apply = 0;

    assert( script < QFont::Unicode );
    // find script in our list of supported scripts.
    const SupportedScript *s = supported_scripts + script;

    FT_Error error = TT_GSUB_Select_Script( gsub, s->tag, &script_index );
    if ( error ) {
// 	qDebug("could not select script %d: %d", (int)script, error );
	if ( s->tag == DefaultScript ) {
	    // try to load default language system
	    error = TT_GSUB_Select_Script( gsub, DefaultScript, &script_index );
	    if ( error )
		return FALSE;
	} else {
	    return FALSE;
	}
    }
    script = s->tag;

//     qDebug("arabic is script %d", script_index );

    TTO_FeatureList featurelist = gsub->FeatureList;

    int numfeatures = featurelist.FeatureCount;

//     qDebug("table has %d features", numfeatures );


    found_bits = 0;
    for( int i = 0; i < numfeatures; i++ ) {
	TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	FT_ULong feature = r->FeatureTag;
	const Features *f = s->features;
	while ( f->tag ) {
	    if ( f->tag == feature ) {
		found_bits |= f->bit;
		break;
	    }
	    f++;
	}
	FT_UShort feature_index;
	TT_GSUB_Select_Feature( gsub, f->tag, script_index, DefaultLangSys,
				&feature_index );
	TT_GSUB_Add_Feature( gsub, feature_index, f->bit );

	char featureString[5];
	tag_to_string( featureString, r->FeatureTag );
// 	qDebug("found feature '%s' in GSUB table", featureString );
// 	qDebug("setting bit %x for feature, feature_index = %d", f->bit, feature_index );
    }
    if ( hasGPos ) {
	FT_UShort script_index;
	error = TT_GPOS_Select_Script( gpos, script, &script_index );
	if ( error ) {
// 	    qDebug("could not select arabic script in gpos table: %d", error );
	    return TRUE;
	}

	TTO_FeatureList featurelist = gpos->FeatureList;

	int numfeatures = featurelist.FeatureCount;

// 	qDebug("table has %d features", numfeatures );

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );
	    TT_GPOS_Add_Feature( gpos, feature_index, s->always_apply );

	    char featureString[5];
	    tag_to_string( featureString, r->FeatureTag );
// 	    qDebug("found feature '%s' in GPOS table", featureString );
	}


    }
    if ( found_bits & s->required_bits != s->required_bits ) {
// 	qDebug( "not all required features for script found! found_bits=%x", found_bits );
	TT_GSUB_Clear_Features( gsub );
	return FALSE;
    }
//     qDebug("found_bits = %x",  (uint)found_bits );

    always_apply = s->always_apply;
    current_script = script;

    return TRUE;
}


QOpenType::QOpenType( FT_Face _face )
    : face( _face ), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    hasGDef = hasGSub = hasGPos = TRUE;
}

QOpenType::~QOpenType()
{
    if ( hasGDef )
	TT_Done_GDEF_Table( gdef );
    if ( hasGSub )
	TT_Done_GSUB_Table( gsub );
    if ( hasGPos )
	TT_Done_GPOS_Table( gpos );
}

bool QOpenType::supportsScript( unsigned int script )
{
    if ( current_script == supported_scripts[script].tag )
	return TRUE;

    char featureString[5];
    tag_to_string( featureString, supported_scripts[script].tag );
//     qDebug("trying to load tables for script %d (%s))", script, featureString);

    FT_Error error;
    if ( !gdef ) {
	if ( (error = TT_Load_GDEF_Table( face, &gdef )) ) {
// 	    qDebug("error loading gdef table: %d", error );
	    hasGDef = FALSE;
// 	    return FALSE;
	}
    }

    if ( !gsub ) {
	if ( (error = TT_Load_GSUB_Table( face, &gsub, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
// 		qDebug("error loading gsub table: %d", error );
		return FALSE;
	    } else {
// 		qDebug("face doesn't have a gsub table" );
		hasGSub = FALSE;
	    }
	}
    }

    if ( !gpos ) {
	if ( (error = TT_Load_GPOS_Table( face, &gpos, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
// 		qDebug("error loading gpos table: %d", error );
		return FALSE;
	    } else {
// 		qDebug("face doesn't have a gpos table" );
		hasGPos = FALSE;
	    }
	}
    }

    if ( loadTables( script ) ) {
	return TRUE;
    }
    return FALSE;
}

extern void q_calculateAdvances( QScriptItem *item );

void QOpenType::apply( unsigned int script, unsigned short *featuresToApply, QScriptItem *item, int stringLength )
{
    if ( current_script != supported_scripts[script].tag ) {
	TT_GSUB_Clear_Features( gsub );

	if ( !loadTables( script ) )
	    return;
    }

    QShapedItem *shaped = item->shaped;

    // shaping code

    TTO_GSUB_String *in = 0;
    TTO_GSUB_String *out = 0;

    TT_GSUB_String_New( face->memory, &in );
    TT_GSUB_String_Set_Length( in, shaped->num_glyphs );
    TT_GSUB_String_New( face->memory, &out);
    TT_GSUB_String_Set_Length( out, shaped->num_glyphs );
    out->length = 0;

    for ( int i = 0; i < shaped->num_glyphs; i++) {
      in->string[i] = shaped->glyphs[i];
      in->logClusters[i] = i;
      in->properties[i] = ~((featuresToApply ? featuresToApply[i] : 0)|always_apply);
    }
    in->max_ligID = 0;

    TT_GSUB_Apply_String (gsub, in, out);

    if ( shaped->num_glyphs < (int)out->length ) {
	shaped->glyphs = ( glyph_t *) realloc( shaped->glyphs, out->length*sizeof(glyph_t) );
    }
    shaped->num_glyphs = out->length;

//     qDebug("out: num_glyphs = %d", shaped->num_glyphs );
    GlyphAttributes *oldAttrs = shaped->glyphAttributes;
    shaped->glyphAttributes = ( GlyphAttributes *) malloc( out->length*sizeof(GlyphAttributes) );

    int clusterStart = 0;
    int oldlc = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	shaped->glyphs[i] = out->string[i];
	int lc = out->logClusters[i];
	shaped->glyphAttributes[i] = oldAttrs[lc];
	if ( !shaped->glyphAttributes[i].mark && shaped->glyphAttributes[i].clusterStart && lc != oldlc ) {
	    for ( int j = oldlc; j < lc; j++ )
		shaped->logClusters[j] = clusterStart;
	    clusterStart = i;
	    oldlc = lc;
	}
//     	qDebug("    glyph[%d]=%4x logcluster=%d mark=%d", i, out->string[i], out->logClusters[i], shaped->glyphAttributes[i].mark );
	// ### need to fix logclusters aswell!!!!
    }
    for ( int j = oldlc; j < stringLength; j++ )
	shaped->logClusters[j] = clusterStart;
//     qDebug("log clusters after shaping:");
//     for ( int j = 0; j < stringLength; j++ )
// 	qDebug("    log[%d] = %d", j, shaped->logClusters[j] );
    free( oldAttrs );

    TT_GSUB_String_Done( in );


    // positioning code:

    q_calculateAdvances( item );

    if ( hasGPos ) {
	item->width = 0;
	TTO_GPOS_Data *positions = 0;

	bool reverse = (item->analysis.bidiLevel % 2);
	// ### is FT_LOAD_DEFAULT the right thing to do?
	TT_GPOS_Apply_String( face, gpos, FT_LOAD_DEFAULT, out, &positions, FALSE, reverse );

	advance_t *advances = shaped->advances;
	offset_t *offsets = shaped->offsets;

	//     qDebug("positioned glyphs:" );
	for ( int i = 0; i < shaped->num_glyphs; i++) {
// 	     	qDebug("    %d:\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
// 	     	       (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6 ),
// 	    	       (int)(positions[i].x_pos >> 6 ), (int)(positions[i].y_pos >> 6),
// 	     	       positions[i].back, positions[i].new_advance );
	    // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
	    if ( positions[i].new_advance ) {
		advances[i] = positions[i].x_advance >> 6;
		//advances[i].y = -positions[i].y_advance >> 6;
	    } else {
		advances[i] += positions[i].x_advance >> 6;
		//advances[i].y -= positions[i].y_advance >> 6;
	    }
	    offsets[i].x = positions[i].x_pos >> 6;
	    offsets[i].y = -(positions[i].y_pos >> 6);
	    int back = positions[i].back;
	    while ( back ) {
		offsets[i].x -= advances[i-back];
		//offsets[i].y -= advances[i-back].y;
		back--;
	    }
	    item->width += advances[i];
	    // 	qDebug("   ->\tadv=(%d/%d)\tpos=(%d/%d)",
	    // 	       advances[i].x, advances[i].y, offsets[i].x, offsets[i].y );
	}
	free( positions );
    }

    if ( out )
	TT_GSUB_String_Done( out );
}

#endif
