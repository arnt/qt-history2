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

    Error stringToCMap( const QChar *str,  int len, int *glyphs, int *nglyphs, bool reverse ) const;

    void draw( QPainter *p, int x, int y, const int *glyphs, const Offset *offsets, int numGlyphs );

    int width( const int *glyphs, const Offset *offsets, int numGlyphs );
    QCharStruct boundingBox( const int *glyphs, const Offset *offsets, int numGlyphs );

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
    int _cmap;
};

#endif
