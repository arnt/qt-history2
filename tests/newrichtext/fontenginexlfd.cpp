#include "fontenginexlfd.h"

#include <qstring.h>
#include <qtextcodec.h>
#include <qpaintdevice.h>
#include "qpainter.h"
#include <qt_x11.h>
#include "qtextlayout.h"

#include <stdlib.h>

// returns TRUE if the character doesn't exist (ie. zero bounding box)
static inline bool charNonExistent(XCharStruct *xcs)
{
    return (xcs == (XCharStruct *) -1 || !xcs ||
	    (xcs->width == 0 && xcs->ascent + xcs->descent == 0));
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

}

FontEngineIface::Error FontEngineXLFD::stringToCMap( const QChar *str,  int len, int *glyphs, int *nglyphs ) const
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
	for ( int i = 0; i < len; i++ ) {
	    glyphs[i] = cstr[i];
	}
    } else {
	for ( int i = 0; i < len; i++ ) {
	    glyphs[i] = str[i];
	}
    }
    *nglyphs = len;
    return NoError;
}

void FontEngineXLFD::draw( QPainter *p, int x, int y, const int *glyphs, const Offset *offsets, int numGlyphs )
{
    qDebug("FontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, numGlyphs );

    // ### add offset handling!!!

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;
    Qt::BGMode bgmode = p->backgroundMode();

    Qt::HANDLE fid_last = 0;

    if (_fs->fid != fid_last) {
	XSetFont(dpy, gc, _fs->fid);
	fid_last = _fs->fid;
    }

    if ( _fs->max_byte1 ) {
	XChar2b ch[256];
	XChar2b *chars = ch;
	if ( numGlyphs > 255 )
	    chars = (XChar2b *)malloc( numGlyphs*sizeof(XChar2b) );

	for (int i = 0; i < numGlyphs; i++) {
	    chars[i].byte1 = glyphs[i] >> 8;
	    chars[i].byte2 = glyphs[i] & 0xff;
	}

	if (bgmode != Qt::TransparentMode)
	    XDrawImageString16(dpy, hd, gc, x, y, chars, numGlyphs );
	else
	    XDrawString16(dpy, hd, gc, x, y, chars, numGlyphs );

	if ( numGlyphs > 255 )
	    free( chars );
    } else {
	char ch[256];
	char *chars = ch;
	if ( numGlyphs > 255 )
	    chars = (char *)malloc( numGlyphs*sizeof(char) );

	for (int i = 0; i < numGlyphs; i++)
	    chars[i] = glyphs[i] & 0xff;

	if (bgmode != Qt::TransparentMode)
	    XDrawImageString(dpy, hd, gc, x, y, chars, numGlyphs );
	else
	    XDrawString(dpy, hd, gc, x, y, chars, numGlyphs );

	if ( numGlyphs > 255 )
	    free( chars );
    }
}

int FontEngineXLFD::width( const int *glyphs, const Offset *offsets, int numGlyphs )
{
    int width = 0;

    for (int i = 0; i < numGlyphs; i++) {
	XCharStruct *xcs = charStruct( _fs, glyphs[i] );
	if (xcs) {
	    width += xcs->width;
	} else {
	    // ### might need something better
	    width += ascent();
	}
	width += offsets[i].x;
    }
    return (int)(width*_scale);
}

QCharStruct FontEngineXLFD::boundingBox( const int *glyphs, const Offset *offsets, int numGlyphs )
{
    int i;

    QCharStruct overall;
    int x = 0;
    int y = 0;
    for (i = 0; i < numGlyphs; i++) {
	XCharStruct *xcs = charStruct( _fs, glyphs[i] );
	x += offsets[i].x;
	y += offsets[i].y;
	if (xcs) {
	    overall.ascent = QMAX(overall.ascent, xcs->ascent+y);
	    overall.descent = QMAX(overall.descent, xcs->descent+y);
	    overall.lbearing = QMIN(overall.lbearing, x + xcs->lbearing);
	    x += xcs->width;
	    overall.rbearing = QMAX(overall.rbearing, x + xcs->rbearing);
	} else {
	    int size = ascent();
	    overall.ascent = QMAX(overall.ascent, size+y);
	    overall.descent = QMAX(overall.descent, y);
	    overall.lbearing = QMIN(overall.lbearing, x);
	    x += size;
	    overall.rbearing = QMAX(overall.rbearing, x);
	}
	overall.width = QMAX(overall.width, x);
    }
    overall.ascent = (int)(overall.ascent * _scale);
    overall.descent = (int)(overall.descent * _scale);
    overall.lbearing = (int)(overall.lbearing * _scale);
    overall.width = (int)(overall.width * _scale);
    overall.rbearing = (int)(overall.rbearing * _scale);
    return overall;
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

