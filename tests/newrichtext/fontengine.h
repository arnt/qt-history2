#ifndef FONTENGINEIFACE_H
#define FONTENGINEIFACE_H

#include <qshared.h>
#include "qtextlayout.h"
#include <qrect.h>

class QChar;
class QPainter;
class OpenTypeIface;
class Offset;

// this uses the same coordinate system as Qt, but a different one to freetype and Xft.
// * y is usually negative, and is equal to the ascent.
// * negative yoff means the following stuff is drawn higher up.
// the characters bounding rect is given by QRect( x,y,width,height), it's advance by
// xoo and yoff
struct QGlyphInfo
{
    QGlyphInfo() {
	x = 100000;
	y = 100000;
	width = 0;
	height = 0;
	xoff = 0;
	yoff = 0;
    }
    QGlyphInfo( int _x, int _y, int _width, int _height, int _xoff, int _yoff ) {
	x = _x;
	y = _y;
	width = _width;
	height = _height;
	xoff = _xoff;
	yoff = _yoff;
    }
    int x;
    int y;
    int width;
    int height;
    int xoff;
    int yoff;
};

class FontEngineIface : public QShared
{
public:
    enum Error {
	NoError,
	OutOfMemory
    };
    virtual ~FontEngineIface() = 0;

    /* returns 0 as glyph index for non existant glyphs */
    virtual Error stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs ) const = 0;

    virtual OpenTypeIface *openTypeIface() const { return 0; }
    virtual int cmap() const = 0;

    virtual void draw( QPainter *p, int x, int y, const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs, bool reverse ) = 0;

    virtual Offset advance( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs ) = 0;
    virtual QGlyphInfo boundingBox( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs ) = 0;
    virtual QGlyphInfo boundingBox( GlyphIndex glyph ) = 0;

    virtual int ascent() const = 0;
    virtual int descent() const = 0;
    virtual int leading() const = 0;
    virtual int maxCharWidth() const = 0;

    virtual const char *name() const = 0;

    virtual bool canRender( const QChar *string,  int len ) = 0;
};


inline FontEngineIface::~FontEngineIface()
{
}


#endif
