#ifndef FONTENGINEIFACE_H
#define FONTENGINEIFACE_H

#include <qshared.h>
#include "qtextlayout.h"

class QChar;
class QPainter;
class OpenTypeIface;
class Offset;

struct QCharStruct
{
    QCharStruct() {
	ascent = -1000000;
	descent = -1000000;
	lbearing = 1000000;
	rbearing = -1000000;
	width = 0;
    }
    int ascent;
    int descent;
    int lbearing;
    int rbearing;
    int width;
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
    virtual Error stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs, bool reverse ) const = 0;

    virtual const OpenTypeIface *openTypeIface() const { return 0; }
    virtual int cmap() const = 0;

    // #### pass in offsets array
    virtual void draw( QPainter *p, int x, int y, const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs ) = 0;

    virtual int width( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs ) = 0;
    virtual QCharStruct boundingBox( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs ) = 0;

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
