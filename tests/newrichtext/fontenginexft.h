#ifndef FONTENGINEXFT_H
#define FONTENGINEXFT_H

#include "fontengine.h"

#include <qcstring.h>
class QTextCodec;

#include <qt_x11.h>

class QFontEngineXft : public QFontEngineIface
{
public:
    QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap );
    ~QFontEngineXft();

    QOpenType *openTypeIface() const;

    Error stringToCMap( const QChar *str,  int len, GlyphIndex *glyphs, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const GlyphIndex *glyphs,
	       const Offset *advances, const Offset *offsets, int numGlyphs, bool reverse );

    virtual QGlyphMetrics boundingBox( const GlyphIndex *glyphs,
				    const Offset *advances, const Offset *offsets, int numGlyphs );
    QGlyphMetrics boundingBox( GlyphIndex glyph );

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
