#ifndef FONTENGINEBOX_H
#define FONTENGINEBOX_H_H

#include "fontengine.h"


class FontEngineBox : public FontEngineIface
{
public:
    FontEngineBox( int size );
    ~FontEngineBox();

    Error stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs, bool reverse ) const;

    void draw( QPainter *p, int x, int y, const GlyphIndex *glyphs, const Offset *offsets, int numGlyphs );

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
    int _size;
};

#endif
