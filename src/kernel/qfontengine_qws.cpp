#include "qfontengine_p.h"
#include "qmemorymanager_qws.h"
#include <private/qunicodetables_p.h>
#include <qpainter.h>
#include <qgfxraster_qws.h>

/*QMemoryManager::FontID*/ void *QFontEngine::handle() const
{
    return id;
}

QFontEngine::QFontEngine( const QFontDef& d )
{
    QFontDef fontDef = d;
    id = memorymanager->refFont(fontDef);
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
		advances[i] = memorymanager->lockGlyphMetrics(handle(), str[i].unicode() )->advance;
    }
    return NoError;
}

void QFontEngine::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( p->txop == QPainter::TxTranslate )
	p->map( x, y, &x, &y );

    if ( textFlags ) {
	int lw = lineThickness();
	p->gfx->setBrush( p->cpen.color() );
	if ( textFlags & Qt::Underline )
	    p->gfx->fillRect( x, y+underlinePosition(), si->width, lw );
	if ( textFlags & Qt::StrikeOut )
	    p->gfx->fillRect( x, y-ascent()/3, si->width, lw );
	if ( textFlags & Qt::Overline )
	    p->gfx->fillRect( x, y-ascent()-1, si->width, lw );
	p->gfx->setBrush( p->cbrush );
    }

    if ( si->isSpace )
	return;

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    offset_t *offsets = engine->offsets( si );

    struct Pos {
	int x;
	int y;
    };
    Pos _positions[64];
    Pos *positions = _positions;
    if ( si->num_glyphs > 64 )
	positions = new Pos[si->num_glyphs];

    if ( si->analysis.bidiLevel % 2 ) {
	int i = si->num_glyphs;
	while( i-- ) {
	    x += advances[i];
	    glyph_metrics_t gi = boundingBox( glyphs[i] );
	    positions[i].x = x-offsets[i].x-gi.xoff;
	    positions[i].y = y+offsets[i].y-gi.yoff;
	}
    } else {
	int i = 0;
	while( i < si->num_glyphs ) {
	    positions[i].x = x+offsets[i].x;
	    positions[i].y = y+offsets[i].y;
	    x += advances[i];
	    i++;
	}
    }
    QConstString cstr( (QChar *)glyphs, si->num_glyphs );
    p->internalGfx()->drawGlyphs(handle(), glyphs, (QPoint *)positions, si->num_glyphs);

    if ( positions != _positions )
	delete [] positions;
}

glyph_metrics_t QFontEngine::boundingBox( const glyph_t *glyphs,
					  const advance_t *advances, const offset_t *offsets, int numGlyphs )
{
    Q_UNUSED( glyphs );
    Q_UNUSED( offsets );
    Q_UNUSED( advances );

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
    QGlyphMetrics *metrics = memorymanager->lockGlyphMetrics( handle(), glyph);
    return glyph_metrics_t( metrics->bearingx, metrics->bearingy, metrics->width, metrics->height, metrics->advance, 0 );
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

int QFontEngine::underlinePosition() const
{
    return memorymanager->fontUnderlinePos(handle());
}

int QFontEngine::lineThickness() const
{
    return memorymanager->fontLineWidth(handle());
}

