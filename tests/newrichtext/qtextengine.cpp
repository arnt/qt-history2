#include "qtextengine.h"

#include "scriptengine.h"

#include <stdlib.h>

#include "bidi.cpp"
#include <qfont.h>
#include "qfontdata_p.h"
#include <qfontengine_p.h>

QScriptItemArray::~QScriptItemArray()
{
    for ( unsigned int i = 0; i < d->size; i++ ) {
	QScriptItem &si = d->items[i];
	if ( si.fontEngine )
	    si.fontEngine->deref();
	if ( si.shaped )
	    delete si.shaped;
    }
    free( d );
}

void QScriptItemArray::resize( int s )
{
    int alloc = (s + 8) >> 3 << 3;
    d = (QScriptItemArrayPrivate *)realloc( d, sizeof( QScriptItemArrayPrivate ) +
		 sizeof( QScriptItem ) * alloc );
    d->alloc = alloc;
}

void QScriptItemArray::split( int pos )
{
    unsigned int itemToSplit;
    for ( itemToSplit = 0; itemToSplit < d->size && d->items[itemToSplit].position <= pos; itemToSplit++ )
	;
    itemToSplit--;
    if ( d->items[itemToSplit].position == pos )
	// already a split at the requested position
	return;

    if ( d->size == d->alloc )
	resize( d->size + 1 );

    int numMove = d->size - itemToSplit-1;
    if ( numMove > 0 )
	memmove( d->items + itemToSplit+2, d->items +itemToSplit+1, numMove*sizeof( QScriptItem ) );
    d->size++;
    d->items[itemToSplit+1] = d->items[itemToSplit];
    d->items[itemToSplit+1].position = pos;
//     qDebug("split at position %d itempos=%d", pos, itemToSplit );
}



QScriptEngine **QTextEngine::scriptEngines = 0;

void QTextEngine::initialize()
{
    if ( !scriptEngines ) {
	scriptEngines = (QScriptEngine **) malloc( QFont::NScripts * sizeof( QScriptEngine * ) );
	scriptEngines[0] = new QScriptEngine;
	for ( int i = 1; i < QFont::NScripts; i++ )
	    scriptEngines[i] = scriptEngines[0];
	scriptEngines[QFont::Arabic] = new QScriptEngineArabic;
	scriptEngines[QFont::Syriac] = new QScriptEngineSyriac;
	scriptEngines[QFont::Devanagari] = new QScriptEngineDevanagari;
	scriptEngines[QFont::Bengali] = new QScriptEngineBengali;
	scriptEngines[QFont::Tamil] = new QScriptEngineTamil;
    }
}


void QTextEngine::bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder )
{
    ::bidiReorder(numRuns, levels, visualOrder );
}


QTextEngine::QTextEngine( const QString &str, QFontPrivate *f )
    : string( str ), fnt( f )
{
    if ( !scriptEngines ) initialize();
    if ( fnt ) fnt->ref();
    itemize();
}

QTextEngine::~QTextEngine()
{
    if ( fnt ) fnt->deref();
}


void QTextEngine::itemize()
{
    if ( !items.d ) {
	int size = 1;
	items.d = (QScriptItemArrayPrivate *)malloc( sizeof( QScriptItemArrayPrivate ) +
						    sizeof( QScriptItem ) * size );
	items.d->alloc = size;
    }
    items.d->size = 0;

    bidiItemize( string, items, QChar::DirON );
}


void QTextEngine::setFont( int item, QFontPrivate *f )
{
    QScriptItem &si = items[item];
    if ( !f )
	f = fnt;
    QFontEngine *fe = f->engineForScript( (QFont::Script)si.analysis.script );
    fe->ref();
    if ( si.fontEngine )
	si.fontEngine->deref();
    si.fontEngine = fe;

    if ( si.shaped ) {
	delete si.shaped;
	si.shaped = 0;
    }
}

QFontEngine *QTextEngine::font( int item )
{
    return items[item].fontEngine;
}

const QCharAttributes *QTextEngine::attributes( int item )
{
    QScriptItem &si = items[item];
    int from = si.position;
    item++;
    int len = ( item < items.size() ? items[item].position : string.length() ) - from;

    si.charAttributes = (QCharAttributes *)realloc( si.charAttributes, sizeof(QCharAttributes)*len );
    scriptEngines[si.analysis.script]->charAttributes( string, from, len, si.charAttributes );
    return si.charAttributes;
}

const QShapedItem *QTextEngine::shape( int item ) const
{
    QScriptItem &si = items[item];

    if ( si.shaped )
	return si.shaped;

    QFont::Script script = (QFont::Script)si.analysis.script;
    int from = si.position;
    int len = length( item );

    if ( !si.shaped )
	si.shaped = new QShapedItem();
    if ( !si.fontEngine )
	si.fontEngine = fnt->engineForScript( script );

    if ( si.fontEngine && si.fontEngine != (QFontEngine*)-1 ) {
	scriptEngines[script]->shape( string, from, len, &si );
    }
    return si.shaped;
}

int QTextEngine::cursorToX( int item, int cpos, Edge edge ) const
{
    QShapedItem *shaped = items[item].shaped;
    if ( !shaped ) {
	shape( item );
	shaped = items[item].shaped;
    }

    int l = length( item );
    if ( cpos > l )
	cpos = l;
    if ( cpos < 0 )
	cpos = 0;

    int glyph_pos = cpos == l ? shaped->num_glyphs : shaped->logClusters[cpos];
    if ( edge == Trailing ) {
	// trailing edge is leading edge of next cluster
	while ( glyph_pos < shaped->num_glyphs && !shaped->glyphAttributes[glyph_pos].clusterStart )
	    glyph_pos++;
    }

    int x = 0;
    bool reverse = items[item].analysis.bidiLevel % 2;

    if ( reverse ) {
	for ( int i = shaped->num_glyphs-1; i >= glyph_pos; i-- )
	    x += shaped->advances[i].x;
    } else {
	for ( int i = 0; i < glyph_pos; i++ )
	    x += shaped->advances[i].x;
    }
//     qDebug("cursorToX: cpos=%d, gpos=%d x=%d", cpos, glyph_pos, x );
    return x;
}

int QTextEngine::xToCursor( int item, int x ) const
{
    QShapedItem *shaped = items[item].shaped;
    if ( !shaped ) {
	shape( item );
	shaped = items[item].shaped;
    }

    int l = length( item );
    bool reverse = items[item].analysis.bidiLevel % 2;
    if ( x < 0 )
	return reverse ? l : 0;


    if ( reverse ) {
	int width = 0;
	for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	    width += shaped->advances[i].x;
	}
	x = -x + width;
    }
    int cp_before = 0;
    int cp_after = 0;
    int x_before = 0;
    int x_after = 0;

    int lastCluster = 0;
    for ( int i = 1; i <= l; i++ ) {
	int newCluster = i < l ? shaped->logClusters[i] : shaped->num_glyphs;
	if ( newCluster != lastCluster ) {
	    // calculate cluster width
	    cp_before = cp_after;
	    x_before = x_after;
	    cp_after = i;
	    for ( int j = lastCluster; j < newCluster; j++ )
		x_after += shaped->advances[j].x;
	    // 		qDebug("cluster boundary: lastCluster=%d, newCluster=%d, x_before=%d, x_after=%d",
	    // 		       lastCluster, newCluster, x_before, x_after );
	    if ( x_after > x )
		break;
	    lastCluster = newCluster;
	}
    }

    bool before = (x - x_before) < (x_after - x);
//     qDebug("got cursor position for %d: %d/%d, x_ba=%d/%d using %d",
// 	   x, cp_before,cp_after,  x_before, x_after,  before ? cp_before : cp_after );

    return before ? cp_before : cp_after;
}

int QTextEngine::width( int item ) const
{
#if 0
    const QShapedItem *shaped = shape( item );
    int width = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ )
	width += shaped->advances[i].x;
    return width;
#endif
    return 0;
}

int QTextEngine::width( int charFrom, int numChars ) const
{
#if 0
    if ( !shaped.d->isPositioned )
	position( shaped );

    if ( charFrom + numChars > shaped.d->length )
	numChars = shaped.d->length - charFrom;
    if ( numChars <= 0 )
	return 0;

    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
    int glyphStart = shaped.d->logClusters[charFrom];
    int charEnd = charFrom + numChars - 1;
    if ( charFrom > 0 && shaped.d->logClusters[charFrom-1] == glyphStart ) {
	while ( charFrom < shaped.d->length && shaped.d->logClusters[charFrom] == glyphStart )
	    charFrom++;
	if ( charFrom == shaped.d->length )
	    return 0;
	glyphStart = shaped.d->logClusters[charFrom];
    }
    int glyphEnd = shaped.d->logClusters[charEnd];
    while ( charEnd < shaped.d->length && shaped.d->logClusters[charEnd] == glyphEnd )
	charEnd++;
    glyphEnd = (charEnd == shaped.d->length) ? shaped.d->num_glyphs-1 : shaped.d->logClusters[charEnd]-1;

    int width = 0;
    for ( int i = glyphStart; i <= glyphEnd; i++ )
	width += shaped.d->advances[i].x;
    return width;
#endif
    return 0;
}

#if 0
bool QTextEngine::split( int item, QShapedItem &shaped, QCharAttributesArray &attrs,
			 int width, QShapedItem *splitoff )
{
    if ( !shaped.d->isPositioned )
	position( shaped );

//     qDebug("QTextEngine::split: item=%d, width=%d", item, width );
    // line breaks are always done in logical order
    QShapedItemPrivate *d = shaped.d;

    int lastBreak = 0;
    int splitGlyph = 0;

    int lastWidth = 0;
    int w = 0;
    int lastCluster = 0;
    int clusterStart = 0;
    for ( int i = 1; i <= d->length; i++ ) {
	int newCluster = i < d->length ? d->logClusters[i] : d->num_glyphs;
	if ( newCluster != lastCluster ) {
	    // calculate cluster width
	    int x = 0;
	    for ( int j = lastCluster; j < newCluster; j++ )
		x += d->advances[j].x;
	    lastWidth += x;
	    if ( w + lastWidth > width )
		break;
	    lastCluster = newCluster;
	    clusterStart = i;
	    if ( attrs[i].softBreak ) {
		lastBreak = i;
		splitGlyph = newCluster;
		w += lastWidth;
		lastWidth = 0;
	    }
	}
    }

    if ( lastBreak == 0 )
	return FALSE;

    // we have the break position
    items.split( lastBreak + items[item].position );

    if ( splitoff ) {
	// split the shapedItem
	QShapedItemPrivate *sd = splitoff->d;
	sd->num_glyphs = d->num_glyphs - splitGlyph - 1;
	sd->glyphs = (glyph_t *) realloc( sd->glyphs, sd->num_glyphs*sizeof(glyph_t) );
	memcpy( sd->glyphs, d->glyphs + splitGlyph, sd->num_glyphs*sizeof(glyph_t) );
	sd->offsets = (offset_t *) realloc( sd->offsets, sd->num_glyphs*sizeof(offset_t) );
	sd->advances = (offset_t *) realloc( sd->advances, sd->num_glyphs*sizeof(offset_t) );
	memcpy( sd->offsets, d->offsets + splitGlyph, sd->num_glyphs*sizeof(offset_t) );
	memcpy( sd->advances, d->advances + splitGlyph, sd->num_glyphs*sizeof(offset_t) );
	sd->glyphAttributes = (GlyphAttributes *) realloc( sd->glyphAttributes, sd->num_glyphs*sizeof(GlyphAttributes) );
	memcpy( sd->glyphAttributes, d->glyphAttributes + splitGlyph, sd->num_glyphs*sizeof(GlyphAttributes) );
	sd->from = d->from + lastBreak;
	sd->length = d->length - lastBreak - 1;
	sd->logClusters = (unsigned short *) realloc( sd->logClusters, sd->length*sizeof( unsigned short ) );
	for ( int i = 0; i < sd->length; i++ )
	    sd->logClusters[i] = d->logClusters[i+lastBreak]-splitGlyph;
	sd->fontEngine = d->fontEngine;
	sd->string = d->string;
	sd->analysis = d->analysis;

	// ### these are somewhat incorrect
	sd->ascent = d->ascent;
	sd->descent = d->descent;

	sd->isShaped = TRUE;
	sd->isPositioned = TRUE;
    }

    d->num_glyphs = splitGlyph;
    d->length = lastBreak;

    return TRUE;
}
#endif
