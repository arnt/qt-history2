#include "qfontengine_p.h"
#include "qmemorymanager_qws.h"
#include <private/qunicodetables_p.h>
#include <qpainter.h>
#include <qgfx_qws.h>

/*QMemoryManager::FontID*/ void *QFontEngine::handle() const
{
    return id;
}

QFontEngine::QFontEngine( const QFontDef& d )
{
    QFontDef s = d;
    if ( s.pointSize == -1 )
	s.pointSize = s.pixelSize*10; // effectively sets the resolution of the display to 72dpi
    id = memorymanager->refFont(s);
}

QFontEngine::~QFontEngine()
{
    memorymanager->derefFont(id);
}


/* returns 0 as glyph index for non existant glyphs */
QFontEngine::Error QFontEngine::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const
{
    if(*nglyphs < len) {
	*nglyphs = len;
	return OutOfMemory;
    }
    for(int i = 0; i < len; i++ )
	glyphs[i] = str[i].unicode();
    *nglyphs = len;
    if(advances) {
	for(int i = 0; i < len; i++)
	    if ( ::category(str[i]) == QChar::Mark_NonSpacing )
		advances[i] = 0;
	    else
		advances[i] = memorymanager->lockGlyphMetrics(handle(), str[i] )->advance;
    }
    return NoError;
}

void QFontEngine::draw( QPainter *p, int x, int y, const glyph_t *glyphs,
			const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse, int textFlags )
{
    // ### Fix non spacing marks, use advances and offsets
    p->internalGfx()->drawText(x, y, QConstString((QChar *)glyphs, numGlyphs).string() );
}

glyph_metrics_t QFontEngine::boundingBox( const glyph_t *glyphs,
			   const advance_t *advances, const offset_t *offsets, int numGlyphs )
{
    if ( numGlyphs == 0 )
	return glyph_metrics_t();

    int w = 0;
    const advance_t *end = advances + numGlyphs;
    while( end > advances )
	w += *(--end);

    return glyph_metrics_t(0, -ascent(), w, ascent()+descent(), w, 0 );
}

glyph_metrics_t QFontEngine::boundingBox( glyph_t glyph )
{
    QChar ch = glyph;
    QGlyphMetrics *metrics = memorymanager->lockGlyphMetrics(handle(), ch);
    return glyph_metrics_t( metrics->bearingx, metrics->bearingy, metrics->width, metrics->height, metrics->width, 0 );
}

bool QFontEngine::canRender( const QChar *string,  int len )
{
    bool allExist = TRUE;
    while ( len-- )
	if ( !memorymanager->inFont( handle(), string[len] ) ) {
	    allExist = FALSE;
	    break;
	}

    return allExist;
}


int QFontEngine::ascent() const
{
    return memorymanager->fontAscent(handle());
}

int QFontEngine::descent() const
{
    return memorymanager->fontDescent(handle());
}

int QFontEngine::leading() const
{
    return memorymanager->fontLeading(handle());
}

int QFontEngine::maxCharWidth() const
{
    return memorymanager->fontMaxWidth(handle());
}

int QFontEngine::minLeftBearing() const
{
    return memorymanager->fontMinLeftBearing(handle());
}

int QFontEngine::minRightBearing() const
{
    return memorymanager->fontMinRightBearing(handle());
}

int QFontEngine::underlinePos() const
{
    return memorymanager->fontUnderlinePos(handle());
}

int QFontEngine::lineWidth() const
{
    return memorymanager->fontLineWidth(handle());
}

