#include <assert.h>

// -----------------------------------------------------------------------------------------------------
//
// Text engine classes
//
// -----------------------------------------------------------------------------------------------------


QScriptItemArray::~QScriptItemArray()
{
    clear();
    free( d );
}

void QScriptItemArray::clear()
{
    if ( d ) {
	for ( unsigned int i = 0; i < d->size; i++ ) {
	    QScriptItem &si = d->items[i];
	    if ( si.fontEngine )
		si.fontEngine->deref();
	}
	d->size = 0;
    }
}

void QScriptItemArray::resize( int s )
{
    int alloc = (s + 8) >> 3 << 3;
    d = (QScriptItemArrayPrivate *)realloc( d, sizeof( QScriptItemArrayPrivate ) +
		 sizeof( QScriptItem ) * alloc );
    d->alloc = alloc;
}

void QScriptItemArray::split( int item, int pos )
{
    if ( pos <= 0 )
	return;

    if ( d->size == d->alloc )
	resize( d->size + 1 );

    int numMove = d->size - item-1;
    if ( numMove > 0 )
	memmove( d->items + item+2, d->items +item+1, numMove*sizeof( QScriptItem ) );
    d->size++;
    QScriptItem &newItem = d->items[item+1];
    QScriptItem &oldItem = d->items[item];
    newItem = oldItem;
    d->items[item+1].position += pos;
    if ( newItem.fontEngine )
	newItem.fontEngine->ref();

//     qDebug("split at position %d itempos=%d", pos, item );
}


void QTextEngine::bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder )
{
    ::bidiReorder(numRuns, levels, visualOrder );
}

void QTextEngine::itemize( bool doBidi )
{
    if ( !items.d ) {
	int size = 8;
	items.d = (QScriptItemArrayPrivate *)malloc( sizeof( QScriptItemArrayPrivate ) +
						    sizeof( QScriptItem ) * size );
	items.d->alloc = size;
    }
    items.d->size = 0;

    if ( doBidi ) {
	if ( direction == QChar::DirON )
	    direction = basicDirection( string );
	bidiItemize( string, items, direction == QChar::DirR );
    } else {
	BidiControl control( false );
	int start = 0;
	int stop = string.length() - 1;
	appendItems(items, start, stop, control, QChar::DirL, string.unicode() );
    }
}

void QTextEngine::shape( int item ) const
{
    QScriptItem &si = items[item];

    if ( si.num_glyphs )
	return;

    QFont::Script script = (QFont::Script)si.analysis.script;
    int from = si.position;
    int len = length( item );

    si.glyph_data_offset = used;

    if ( !si.fontEngine )
	si.fontEngine = fnt->engineForScript( script );
    si.fontEngine->ref();

    si.ascent = si.fontEngine->ascent();
    si.descent = si.fontEngine->descent();

    if ( si.fontEngine && si.fontEngine != (QFontEngine*)-1 ) {
	assert( script < QFont::NScripts );
	scriptEngines[script].shape( script, string, from, len, (QTextEngine *)this, &si );
    }
    ((QTextEngine *)this)->used += si.num_glyphs;

    si.width = 0;
    advance_t *advances = this->advances( &si );
    for ( int i = 0; i < si.num_glyphs; i++ )
	si.width += advances[i];

    return;
}

int QTextEngine::width( int from, int len ) const
{
    int w = 0;

//     qDebug("QTextEngine::width( from = %d, len = %d ), numItems=%d, strleng=%d", from,  len, items.size(), string.length() );
    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem *si = &items[i];
	int pos = si->position;
	int ilen = length( i );
// 	qDebug("item %d: from %d len %d", i, pos, ilen );
	if ( pos >= from + len )
	    break;
	if ( pos + ilen > from ) {
	    if ( !si->num_glyphs )
		shape( i );

	    advance_t *advances = this->advances( si );
	    unsigned short *logClusters = this->logClusters( si );

// 	    fprintf( stderr, "  logclusters:" );
// 	    for ( int k = 0; k < ilen; k++ )
// 		fprintf( stderr, " %d", logClusters[k] );
// 	    fprintf( stderr, "\n" );
	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = logClusters[charFrom];
	    if ( charFrom > 0 && logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = logClusters[charEnd];
		while ( charEnd < ilen && logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];

// 		qDebug("char: start=%d end=%d / glyph: start = %d, end = %d", charFrom, charEnd, glyphStart, glyphEnd );
		for ( int i = glyphStart; i < glyphEnd; i++ )
		    w += advances[i];
	    }
	}
    }
//     qDebug("   --> w= %d ", w );
    return w;
}

glyph_metrics_t QTextEngine::boundingBox( int from,  int len ) const
{
    glyph_metrics_t gm;

    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem *si = &items[i];
	int pos = si->position;
	int ilen = length( i );
	if ( pos > from + len )
	    break;
	if ( pos + len > from ) {
	    if ( !si->num_glyphs )
		shape( i );
	    advance_t *advances = this->advances( si );
	    unsigned short *logClusters = this->logClusters( si );
	    glyph_t *glyphs = this->glyphs( si );
	    offset_t *offsets = this->offsets( si );

	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = logClusters[charFrom];
	    if ( charFrom > 0 && logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = logClusters[charEnd];
		while ( charEnd < ilen && logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];
		if ( glyphStart <= glyphEnd  ) {
		    QFontEngine *fe = si->fontEngine;
		    glyph_metrics_t m = fe->boundingBox( glyphs+glyphStart, advances+glyphStart,
						       offsets+glyphStart, glyphEnd-glyphStart );
		    gm.x = QMIN( gm.x, m.x + gm.xoff );
		    gm.y = QMIN( gm.y, m.y + gm.yoff );
		    gm.width = QMAX( gm.width, m.width+gm.xoff );
		    gm.height = QMAX( gm.height, m.height+gm.yoff );
		    gm.xoff += m.xoff;
		    gm.yoff += m.yoff;
		}
	    }
	}
    }
    return gm;
}

