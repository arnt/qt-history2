#ifndef FONTENGINEXFT_H
#define FONTENGINEXFT_H

#include "fontengine.h"

#include <qcstring.h>
class QTextCodec;

#include <qt_x11.h>

class FontEngineXft : public FontEngineIface
{
public:
    FontEngineXft( XftFont *font, XftPattern *pattern, int cmap );
    ~FontEngineXft();

    OpenTypeIface *openTypeIface() const;

    Error stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs, bool reverse );

    Offset advance( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs );
    QGlyphInfo boundingBox( const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs );
    QGlyphInfo boundingBox( GlyphIndex glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;

    int cmap() const;
    const char *name() const;

    bool canRender( const QChar *string,  int len );


private:
    XftFont *_font;
    XftPattern *_pattern;
    OpenTypeIface *_openType;
    int _cmap;
};

#endif
