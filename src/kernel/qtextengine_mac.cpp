
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

void QTextEngine::itemize( int mode )
{
    if ( !items.d ) {
	int size = 1;
	items.d = (QScriptItemArrayPrivate *)malloc( sizeof( QScriptItemArrayPrivate ) +
						    sizeof( QScriptItem ) * size );
	items.d->alloc = size;
    }
    items.d->size = 0;

    if ( !(mode & NoBidi) ) {
	if ( direction == QChar::DirON )
	    direction = basicDirection( string );
	bidiItemize( string, items, direction == QChar::DirR, mode );
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

