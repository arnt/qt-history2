#ifndef FONTENGINEIFACE_H
#define FONTENGINEIFACE_H

#include <qshared.h>
#include "qtextlayout_p.h"
#include <qrect.h>

class QChar;
class QPainter;
class OpenTypeIface;
class Offset;

class QFontEngineIface : public QShared
{
public:
    enum Error {
	NoError,
	OutOfMemory
    };

    enum Type {
	Box,
	Xlfd,
	Xft
    };

    virtual ~QFontEngineIface() = 0;

    /* returns 0 as glyph index for non existant glyphs */
    virtual Error stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs ) const = 0;

    virtual OpenTypeIface *openTypeIface() const { return 0; }
    virtual int cmap() const = 0;

    virtual void draw( QPainter *p, int x, int y, const GlyphIndex *glyphs,
		       const Offset *advances, const Offset *offsets, int numGlyphs, bool reverse ) = 0;

    virtual QGlyphMetrics boundingBox( const GlyphIndex *glyphs,
				    const Offset *advances, const Offset *offsets, int numGlyphs ) = 0;
    virtual QGlyphMetrics boundingBox( GlyphIndex glyph ) = 0;

    virtual int ascent() const = 0;
    virtual int descent() const = 0;
    virtual int leading() const = 0;
    virtual int maxCharWidth() const = 0;

    virtual const char *name() const = 0;

    virtual bool canRender( const QChar *string,  int len ) = 0;

    virtual void setScale( double ) {}

    virtual Type type() const = 0;
};


inline QFontEngineIface::~QFontEngineIface()
{
}


#endif
