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
    : string( str ), fnt( f ), charAttributes( 0 )
{
    if ( !scriptEngines ) initialize();
    if ( fnt ) fnt->ref();
    itemize();
}

QTextEngine::~QTextEngine()
{
    if ( fnt ) fnt->deref();
    delete charAttributes;
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

const QCharAttributes *QTextEngine::attributes()
{
    if ( charAttributes )
	return charAttributes;


    charAttributes = (QCharAttributes *)malloc( sizeof(QCharAttributes)*string.length() );
    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem &si = items[i];
	int from = si.position;
	int len = length( i );
	scriptEngines[si.analysis.script]->charAttributes( string, from, len, charAttributes+from );
    }
    return charAttributes;
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

int QTextEngine::width( int item ) const
{
    const QShapedItem *shaped = shape( item );
    int width = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ )
	width += shaped->advances[i].x;
    return width;
}

int QTextEngine::width( int from, int len ) const
{
    int w = 0;

    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem &item = items[i];
	int pos = item.position;
	int ilen = length( i );
	if ( pos > from + len )
	    break;
	if ( pos + len > from ) {
	    const QShapedItem *shaped = shape( i );
	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = shaped->logClusters[charFrom];
	    int charEnd = from + len - 1 - pos;
	    if ( charEnd >= ilen )
		charEnd = ilen-1;
	    if ( charFrom > 0 && shaped->logClusters[charFrom-1] == glyphStart ) {
		while ( charFrom < len && shaped->logClusters[charFrom] == glyphStart )
		    charFrom++;
		if ( charFrom == len )
		    glyphStart = shaped->num_glyphs;
		else
		    glyphStart = shaped->logClusters[charFrom];
	    }
	    int glyphEnd = shaped->logClusters[charEnd];
	    while ( charEnd < len && shaped->logClusters[charEnd] == glyphEnd )
		charEnd++;
	    glyphEnd = (charEnd == len) ? shaped->num_glyphs-1 : shaped->logClusters[charEnd]-1;

	    for ( int i = glyphStart; i <= glyphEnd; i++ )
		w += shaped->advances[i].x;
	}
    }
    return w;
}

QGlyphMetrics QTextEngine::boundingBox( int from,  int len ) const
{
    QGlyphMetrics gm;

    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem &item = items[i];
	int pos = item.position;
	int ilen = length( i );
	if ( pos > from + len )
	    break;
	if ( pos + len > from ) {
	    const QShapedItem *shaped = shape( i );
	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = shaped->logClusters[charFrom];
	    int charEnd = from + len - 1 - pos;
	    if ( charEnd >= ilen )
		charEnd = ilen-1;
	    if ( charFrom > 0 && shaped->logClusters[charFrom-1] == glyphStart ) {
		while ( charFrom < len && shaped->logClusters[charFrom] == glyphStart )
		    charFrom++;
		if ( charFrom == len )
		    glyphStart = shaped->num_glyphs;
		else
		    glyphStart = shaped->logClusters[charFrom];
	    }
	    int glyphEnd = shaped->logClusters[charEnd];
	    while ( charEnd < len && shaped->logClusters[charEnd] == glyphEnd )
		charEnd++;
	    glyphEnd = (charEnd == len) ? shaped->num_glyphs-1 : shaped->logClusters[charEnd]-1;

	    if ( glyphStart <= glyphEnd  ) {
		QFontEngine *fe = item.fontEngine;
		QGlyphMetrics m = fe->boundingBox( shaped->glyphs+glyphStart, shaped->advances+glyphStart,
						   shaped->offsets+glyphStart, glyphEnd-glyphStart );
		gm.x = QMIN( gm.x, m.x + gm.xoff );
		gm.y = QMIN( gm.y, m.x + gm.yoff );
		gm.width = QMAX( gm.width, m.width+gm.xoff );
		gm.height = QMAX( gm.height, m.height+gm.yoff );
		gm.xoff += m.xoff;
		gm.yoff += m.yoff;
	    }
	}
    }
    return gm;
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
