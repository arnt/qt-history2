#include "qtextlayout_p.h"

#include "scriptengine.h"
#include "scriptenginebasic.h"
#include "scriptenginesyriac.h"
#include "scriptenginearabic.h"
#include "scriptenginedevanagari.h"
#include "scriptenginebengali.h"
#include "scriptenginetamil.h"

#include <stdlib.h>

#include "bidi.cpp"


QScriptItemArray::~QScriptItemArray()
{
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


CharAttributesArray::~CharAttributesArray()
{
    free( d );
}


QShapedItem::QShapedItem()
{
    d = new QShapedItemPrivate();
}

QShapedItem::QShapedItem( const QShapedItem &other )
{
    other.d->ref();
    d = other.d;
}

QShapedItem::~QShapedItem()
{
    if ( d->deref() )
	delete d;
}


QShapedItem &QShapedItem::operator =( const QShapedItem &other )
{
    other.d->ref();
    if ( d->deref() )
	delete d;
    d = other.d;
    return *this;
}

const GlyphIndex *QShapedItem::glyphs() const
{
    return d->glyphs;

}

int QShapedItem::count() const
{
    return d->num_glyphs;
}

const Offset *QShapedItem::offsets() const
{
    return d->offsets;
}


int QShapedItem::ascent() const
{
    return d->ascent;
}

int QShapedItem::descent() const
{
    return d->descent;
}



QScriptEngine **scriptEngines = 0;

class TextLayoutQt : public TextLayout
{
public:

    void itemize( QScriptItemArray &items, const QString & ) const;

    void attributes( CharAttributesArray &attributes, const QString &string,
		     const QScriptItemArray &items, int item ) const;

    void shape( QShapedItem &shaped, const QFont &font, const QString &string,
		const QScriptItemArray &items, int item ) const;

    int cursorToX( QShapedItem &shaped, int cpos, Edge edge ) const;
    int xToCursor( QShapedItem &shaped, int x ) const;

    int width( QShapedItem &shaped ) const;
    int width( QShapedItem &shaped, int charFrom, int numChars ) const;
    bool split( QScriptItemArray &items, int item, QShapedItem &shaped, CharAttributesArray &attrs,
		int width, QShapedItem *splitoff ) const;

private:
    // not in the interface
    void shape( QShapedItem &shaped ) const;
    void position( QShapedItem &shaped ) const;

};



static TextLayout *_instance = 0;

const TextLayout *TextLayout::instance()
{
    if ( !_instance ) {
	_instance = new TextLayoutQt();

        if ( !scriptEngines ) {
	    scriptEngines = (QScriptEngine **) malloc( QFont::NScripts * sizeof( QScriptEngine * ) );
	    scriptEngines[0] = new QScriptEngineBasic;
	    for ( int i = 1; i < QFont::NScripts; i++ )
		scriptEngines[i] = scriptEngines[0];
	    scriptEngines[QFont::Arabic] = new QScriptEngineArabic;
	    scriptEngines[QFont::Syriac] = new QScriptEngineSyriac;
	    scriptEngines[QFont::Devanagari] = new QScriptEngineDevanagari;
	    scriptEngines[QFont::Bengali] = new QScriptEngineBengali;
	    scriptEngines[QFont::Tamil] = new QScriptEngineTamil;
	}
    }
    return _instance;
}


void TextLayout::bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder ) const
{
    ::bidiReorder(numRuns, levels, visualOrder );
}


void TextLayoutQt::itemize( QScriptItemArray &items, const QString &string ) const
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


void TextLayoutQt::attributes( CharAttributesArray &attrs, const QString &string,
			       const QScriptItemArray &items, int item ) const
{
    const QScriptItem &si = items[item];
    int from = si.position;
    item++;
    int len = ( item < items.size() ? items[item].position : string.length() ) - from;


    attrs.d = (CharAttributesArrayPrivate *)realloc( attrs.d, sizeof( CharAttributesArrayPrivate ) +
						     sizeof(CharAttributes)*len );

    scriptEngines[si.analysis.script]->charAttributes( string, from, len, attrs.d->attributes );
}

void TextLayoutQt::shape( QShapedItem &shaped, const QFont &f, const QString &string,
			 const QScriptItemArray &items, int item ) const
{
    const QScriptItem &si = items[item];
    QFont::Script script = (QFont::Script)si.analysis.script;
    int from = si.position;
    item++;
    int len = ( item < items.size() ? items[item].position : string.length() ) - from;

    shaped.d->string = string;
    shaped.d->from = from;
    shaped.d->length = len;
    shaped.d->fontEngine = f.engineForScript( script );
    shaped.d->analysis = si.analysis;

    shaped.d->isShaped = FALSE;
    shaped.d->isPositioned = FALSE;
}

void TextLayoutQt::shape( QShapedItem &shaped ) const
{
    if ( shaped.d->isShaped )
	return;
    QFont::Script script = (QFont::Script)shaped.d->analysis.script;
    if ( shaped.d->fontEngine && shaped.d->fontEngine != (QFontEngineIface*)-1 )
	scriptEngines[script]->shape( &shaped );
    shaped.d->isShaped = TRUE;
}

void TextLayoutQt::position( QShapedItem &shaped ) const
{
    if ( !shaped.d->isShaped )
	shape( shaped );
    if ( shaped.d->isPositioned )
	return;
    QFont::Script script = (QFont::Script)shaped.d->analysis.script;
    if ( shaped.d->fontEngine && shaped.d->fontEngine != (QFontEngineIface*)-1 )
	scriptEngines[script]->position( &shaped );
    shaped.d->isPositioned = TRUE;
}

int TextLayoutQt::cursorToX( QShapedItem &shaped, int cpos, Edge edge ) const
{
    if ( !shaped.d->isPositioned )
	position( shaped );

    QShapedItemPrivate *d = shaped.d;

    if ( cpos > d->length )
	cpos = d->length;
    if ( cpos < 0 )
	cpos = 0;

    int glyph_pos = cpos == d->length ? d->num_glyphs : d->logClusters[cpos];
    if ( edge == Trailing ) {
	// trailing edge is leading edge of next cluster
	while ( glyph_pos < d->num_glyphs && !d->glyphAttributes[glyph_pos].clusterStart )
	    glyph_pos++;
    }

    int x = 0;
    bool reverse = d->analysis.bidiLevel % 2;

    if ( reverse ) {
	for ( int i = d->num_glyphs-1; i >= glyph_pos; i-- )
	    x += d->advances[i].x;
    } else {
	for ( int i = 0; i < glyph_pos; i++ )
	    x += d->advances[i].x;
    }
//     qDebug("cursorToX: cpos=%d, gpos=%d x=%d", cpos, glyph_pos, x );
    return x;
}

int TextLayoutQt::xToCursor( QShapedItem &shaped, int x ) const
{
    if ( !shaped.d->isPositioned )
	position( shaped );

    QShapedItemPrivate *d = shaped.d;

    bool reverse = d->analysis.bidiLevel % 2;
    if ( x < 0 )
	return reverse ? d->length : 0;


    if ( reverse ) {
	int width = 0;
	for ( int i = 0; i < d->num_glyphs; i++ ) {
	    width += d->advances[i].x;
	}
	x = -x + width;
    }
    int cp_before = 0;
    int cp_after = 0;
    int x_before = 0;
    int x_after = 0;

    int lastCluster = 0;
    for ( int i = 1; i <= d->length; i++ ) {
	int newCluster = i < d->length ? d->logClusters[i] : d->num_glyphs;
	if ( newCluster != lastCluster ) {
	    // calculate cluster width
	    cp_before = cp_after;
	    x_before = x_after;
	    cp_after = i;
	    for ( int j = lastCluster; j < newCluster; j++ )
		x_after += d->advances[j].x;
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


int TextLayoutQt::width( QShapedItem &shaped ) const
{
    if ( !shaped.d->isPositioned )
	position( shaped );

    int width = 0;
    for ( int i = 0; i < shaped.d->num_glyphs; i++ )
	width += shaped.d->advances[i].x;
    return width;
}

int TextLayoutQt::width( QShapedItem &shaped, int charFrom, int numChars ) const
{
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
}

bool TextLayoutQt::split( QScriptItemArray &items, int item, QShapedItem &shaped, CharAttributesArray &attrs, int width, QShapedItem *splitoff ) const
{
    if ( !shaped.d->isPositioned )
	position( shaped );

//     qDebug("TextLayoutQt::split: item=%d, width=%d", item, width );
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
	sd->glyphs = (GlyphIndex *) realloc( sd->glyphs, sd->num_glyphs*sizeof(GlyphIndex) );
	memcpy( sd->glyphs, d->glyphs + splitGlyph, sd->num_glyphs*sizeof(GlyphIndex) );
	sd->offsets = (Offset *) realloc( sd->offsets, sd->num_glyphs*sizeof(Offset) );
	sd->advances = (Offset *) realloc( sd->advances, sd->num_glyphs*sizeof(Offset) );
	memcpy( sd->offsets, d->offsets + splitGlyph, sd->num_glyphs*sizeof(Offset) );
	memcpy( sd->advances, d->advances + splitGlyph, sd->num_glyphs*sizeof(Offset) );
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
