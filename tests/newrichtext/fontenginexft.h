#ifndef FONTENGINEXFT_H
#define FONTENGINEXFT_H

#include "fontengine.h"

#include <qcstring.h>
class QTextCodec;

#include <qt_x11.h>

class QFontEngineXft : public QFontEngine
{
public:
    QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap );
    ~QFontEngineXft();

    QOpenType *openTypeIface() const;

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const offset_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    virtual QGlyphMetrics boundingBox( const glyph_t *glyphs,
				    const offset_t *advances, const offset_t *offsets, int numGlyphs );
    QGlyphMetrics boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;

    int cmap() const;
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;

private:
    friend class QFontPrivate;
    XftFont *_font;
    XftPattern *_pattern;
    QOpenType *_openType;
    int _cmap;
};

#endif
