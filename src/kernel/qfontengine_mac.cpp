#include "qfontengine_p.h"
#include <qglobal.h>
#include "qapplication_p.h"

QFontEngine::~QFontEngine()
{

}


//SDM? This file is completely bogus..

QFontEngine::Error 
QFontEngineMac::stringToCMap(const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs) const
{
    return QFontEngine::OutOfMemory; 
}

void 
QFontEngineMac::draw(QPainter *p, int x, int y, const glyph_t *glyphs,
		      const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse)
{

}

QGlyphMetrics 
QFontEngineMac::boundingBox(const glyph_t *glyphs,
			    const advance_t *advances, const offset_t *offsets, int numGlyphs)
{
    QGlyphMetrics ret;
    return ret;
}

QGlyphMetrics 
QFontEngineMac::boundingBox(glyph_t glyph)
{
    QGlyphMetrics ret;
    return ret;
}

bool 
QFontEngineMac::canRender( const QChar *string,  int len)
{
    return TRUE;
}



