

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
	    if ( si.shaped )
		delete si.shaped;
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


QShapedItem::~QShapedItem()
{
    if ( ownGlyphs ) {
	free( glyphs );
	free( advances );
	free( offsets );
	free( logClusters );
	free( glyphAttributes );
    }
}

void QTextEngine::bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder )
{
    ::bidiReorder(numRuns, levels, visualOrder );
}


QTextEngine::QTextEngine( const QString &str, QFontPrivate *f )
    : string( str ), fnt( f ), direction( QChar::DirON ), charAttributes( 0 )
{
    if ( fnt ) fnt->ref();
}

QTextEngine::~QTextEngine()
{
    if ( fnt ) fnt->deref();
    free( charAttributes );
}


void QTextEngine::itemize( bool doBidi )
{
    if ( !items.d ) {
	int size = 1;
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
	int script = si.analysis.script;
	assert( script < QFont::NScripts );
	scriptEngines[si.analysis.script].charAttributes( script, string, from, len, charAttributes );
    }
    return charAttributes;
}

QShapedItem *QTextEngine::shape( int item ) const
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
    si.fontEngine->ref();

    if ( si.fontEngine && si.fontEngine != (QFontEngine*)-1 ) {
	assert( script < QFont::NScripts );
	scriptEngines[script].shape( script, string, from, len, &si );
    }
    return si.shaped;
}

int QTextEngine::width( int from, int len ) const
{
    int w = 0;

//     qDebug("QTextEngine::width( from = %d, len = %d ), numItems=%d, strleng=%d", from,  len, items.size(), string.length() );
    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem &item = items[i];
	int pos = item.position;
	int ilen = length( i );
// 	qDebug("item %d: from %d len %d", i, pos, ilen );
	if ( pos >= from + len )
	    break;
	if ( pos + ilen > from ) {
	    const QShapedItem *shaped = shape( i );
// 	    fprintf( stderr, "  logclusters:" );
// 	    for ( int k = 0; k < ilen; k++ )
// 		fprintf( stderr, " %d", shaped->logClusters[k] );
// 	    fprintf( stderr, "\n" );
	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = shaped->logClusters[charFrom];
	    if ( charFrom > 0 && shaped->logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && shaped->logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = shaped->logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = shaped->logClusters[charEnd];
		while ( charEnd < ilen && shaped->logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? shaped->num_glyphs : shaped->logClusters[charEnd];

// 		qDebug("char: start=%d end=%d / glyph: start = %d, end = %d", charFrom, charEnd, glyphStart, glyphEnd );
		for ( int i = glyphStart; i < glyphEnd; i++ )
		    w += shaped->advances[i];
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
	    if ( charFrom > 0 && shaped->logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && shaped->logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = shaped->logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = shaped->logClusters[charEnd];
		while ( charEnd < ilen && shaped->logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? shaped->num_glyphs : shaped->logClusters[charEnd];
		if ( glyphStart <= glyphEnd  ) {
		    QFontEngine *fe = item.fontEngine;
		    glyph_metrics_t m = fe->boundingBox( shaped->glyphs+glyphStart, shaped->advances+glyphStart,
						       shaped->offsets+glyphStart, glyphEnd-glyphStart );
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

