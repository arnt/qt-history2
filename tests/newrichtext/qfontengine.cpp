#include "qfontengine_p.h"

// #define FONTENGINE_DEBUG

#include <qcstring.h>
#include <qtextcodec.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include "opentype.h"

#include <qt_x11.h>

// defined in qfontdatbase_x11.cpp
extern int qt_mibForXlfd( const char * encoding );


// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------


QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{

}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *,  int len, glyph_t *glyphs, int *nglyphs ) const
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
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont( QPaintDevice::x11AppDisplay(), _fs );
    _fs = 0;
}

QFontEngine::Error QFontEngineXLFD::stringToCMap( const QChar *str,  int len, glyph_t *glyphs, int *nglyphs ) const
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
    return NoError;
}

void QFontEngineXLFD::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
			   const offset_t *advances, const offset_t *offsets, int numGlyphs, bool reverse )
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
	    offset_t adv = advances[i];
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
	    offset_t adv = advances[i];
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

QGlyphMetrics QFontEngineXLFD::boundingBox( const glyph_t *glyphs, const offset_t *advances, const offset_t *offsets, int numGlyphs )
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

QGlyphMetrics QFontEngineXLFD::boundingBox( glyph_t glyph )
{
    // ### scale missing!
    XCharStruct *xcs = charStruct( _fs, glyph );
    if (xcs) {
	return QGlyphMetrics( xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent, xcs->width, 0 );
    }
    int size = ascent();
    return QGlyphMetrics( 0, size, size, size, size, 0 );
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
    if ( stringToCMap( string, len, g, &nglyphs ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, &nglyphs );
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

class Q_HackPaintDevice : public QPaintDevice
{
public:
    Q_HackPaintDevice() : QPaintDevice( 0 ) {}
    inline XftDraw *xftDrawHandle() const {
	return (XftDraw *)rendhd;
    }
};

// ### all this won't work with Xft2!!!!
// we need to encapsulate these in some methods to get it working!

inline XftFontStruct *
getFontStruct( XftFont *font )
{
    if (font->core)
	return 0;
    return font->u.ft.font;
}

// ditto
static inline void getGlyphInfo(XGlyphInfo *xgi, XftFont *font, int glyph)
{
    XftTextExtents32(QPaintDevice::x11AppDisplay(), font, (XftChar32 *) &glyph, 1, xgi);
}




QFontEngineXft::QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap )
    : _font( font ), _pattern( pattern ), _openType( 0 ), _cmap( cmap )
{
    XftFontStruct *xftfs = getFontStruct( _font );
    if ( xftfs ) {
	// dirty hack: we set the charmap in the Xftfreetype to -1, so XftFreetype assumes no encoding and
	// really draws glyph indices. The FT_Face still has the Unicode encoding to we can convert from
	// Unicode to glyph index
	xftfs->charmap = -1;
    }
}

QFontEngineXft::~QFontEngineXft()
{
    XftFontClose( QPaintDevice::x11AppDisplay(),_font );
    XftPatternDestroy( _pattern );
    _font = 0;
    _pattern = 0;
    delete _openType;
}

QFontEngine::Error QFontEngineXft::stringToCMap( const QChar *str,  int len, glyph_t *glyphs, int *nglyphs ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    XftFontStruct *fs = getFontStruct( _font );
    if ( !fs ) {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = str[i].unicode();
    } else {
	for ( int i = 0; i < len; i++ )
	    glyphs[i] = FT_Get_Char_Index (fs->face, str[i].unicode() );
    }
    *nglyphs = len;
    return NoError;
}

void QFontEngineXft::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
			   const offset_t *advances, const offset_t *offsets, int numGlyphs, bool reverse )
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
	QGlyphMetrics bb = boundingBox( glyphs, advances, offsets, numGlyphs );
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
    QGlyphMetrics ci = boundingBox( glyphs, advances, offsets, numGlyphs );
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
	    offset_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv.x;
	    y += adv.y;
	    QGlyphMetrics gi = boundingBox( glyphs[i] );
	    XftDrawString16 (draw, &col, _font, x-offsets[i].x-gi.xoff, y+offsets[i].y-gi.yoff,
			     (XftChar16 *) (glyphs+i), 1);
#ifdef FONTENGINE_DEBUG
	    p->drawRect( x - offsets[i].x - gi.xoff + gi.x, y + 100 + offsets[i].y - gi.yoff + gi.y, gi.width, gi.height );
	    p->drawLine( x - offsets[i].x - gi.xoff, y + 150 + 5*i , x - offsets[i].x, y + 150 + 5*i );

#endif
	}
    } else {
	int i = 0;
	while ( i < numGlyphs ) {
	    XftDrawString16 (draw, &col, _font, x+offsets[i].x, y+offsets[i].y,
			     (XftChar16 *) (glyphs+i), 1);
	    offset_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv.x;
	    y += adv.y;
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
	    QGlyphMetrics ci = boundingBox( glyphs[i] );
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

QGlyphMetrics QFontEngineXft::boundingBox( const glyph_t *glyphs, const offset_t *advances, const offset_t *offsets, int numGlyphs )
{
    XGlyphInfo xgi;

    QGlyphMetrics overall;
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
	overall.xoff += advances[i].x;
	overall.yoff -= advances[i].y;
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    return overall;
}

QGlyphMetrics QFontEngineXft::boundingBox( glyph_t glyph )
{
    XGlyphInfo xgi;
    getGlyphInfo( &xgi, _font, glyph );
    return QGlyphMetrics( -xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff );
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
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, &nglyphs ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, &nglyphs );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !XftGlyphExists(QPaintDevice::x11AppDisplay(), _font, g[i]) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( nglyphs > 255 )
	free( g );
	return allExist;

}

QOpenType *QFontEngineXft::openType() const
{
//     qDebug("openTypeIface requested!");
    if ( _openType )
	return _openType;
    XftFontStruct *xftfs = getFontStruct( _font );
    if ( !xftfs ) {
	qDebug("font is core font!");
	return 0;
    }

    QFontEngineXft *that = (QFontEngineXft *)this;

    that->_openType = new QOpenType( xftfs->face );
    return _openType;
}


QFontEngine::Type QFontEngineXft::type() const
{
    return Xft;
}

