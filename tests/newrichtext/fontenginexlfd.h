#ifndef FONTENGINEXLFD_H
#define FONTENGINEXLFD_H

#include "fontengine.h"

#include <qcstring.h>
class QTextCodec;

#include <qt_x11.h>

class FontEngineXLFD : public FontEngineIface
{
public:
    FontEngineXLFD( XFontStruct *fs, const char *name, QTextCodec *codec, int cmap );
    ~FontEngineXLFD();

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
    XFontStruct *_fs;
    QCString _name;
    QTextCodec *_codec;
    float _scale; // needed for printing, to correctly scale font metrics for bitmap fonts
    int _cmap;
};

#endif
