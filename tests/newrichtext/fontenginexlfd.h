#ifndef FONTENGINEXLFD_H
#define FONTENGINEXLFD_H

#include "fontengine.h"

#include <qcstring.h>
class QTextCodec;

#include <qt_x11.h>

class QFontEngineXLFD : public QFontEngineIface
{
public:
    QFontEngineXLFD( XFontStruct *fs, const char *name, const char *encoding, int cmap );
    ~QFontEngineXLFD();

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

    void setScale( double scale );
    Type type() const;

private:
    friend class QFontPrivate;
    XFontStruct *_fs;
    QCString _name;
    QTextCodec *_codec;
    float _scale; // needed for printing, to correctly scale font metrics for bitmap fonts
    int _cmap;
};

#endif
