#ifndef FONTENGINEBOX_H
#define FONTENGINEBOX_H_H

#include "fontengine.h"


class FontEngineBox : public FontEngineIface
{
public:
    FontEngineBox( int size );
    ~FontEngineBox();

    Error stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const GlyphIndex *glyphs,
	       const Offset *advances, const Offset *offsets, int numGlyphs, bool reverse );

    virtual QGlyphInfo boundingBox( const GlyphIndex *glyphs,
				    const Offset *advances, const Offset *offsets, int numGlyphs );
    QGlyphInfo boundingBox( GlyphIndex glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;

    int cmap() const;
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;
    inline int size() const { return _size; }

private:
    friend class QFontPrivate;
    int _size;
};

#endif
