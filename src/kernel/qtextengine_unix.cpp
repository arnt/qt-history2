/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <assert.h>


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

#ifndef FT_KERNING_DEFAULT
#  define FT_KERNING_DEFAULT ft_kerning_default
#endif
void QTextEngine::shape( int item ) const
{
    assert( item < items.size() );
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

    if ( !widthOnly ) {
	si.ascent = si.fontEngine->ascent();
	si.descent = si.fontEngine->descent();
    }

    if ( si.fontEngine && si.fontEngine != (QFontEngine*)-1 ) {
	assert( script < QFont::NScripts );
	scriptEngines[script].shape( script, string, from, len, (QTextEngine *)this, &si );
    }
    ((QTextEngine *)this)->used += si.num_glyphs;

    advance_t *advances = this->advances( &si );

    if (kern && si.fontEngine->type() == QFontEngine::Xft) {
	FT_Face face = static_cast<QFontEngineXft *>(si.fontEngine)->freetypeFace();
	if (FT_HAS_KERNING(face)) {
	    FT_Vector kerning;
	    glyph_t *g = glyphs(&si);
	    bool kern = false;
	    for (int i = 0; i < si.num_glyphs-1; ++i) {
		FT_Get_Kerning(face, *g, *(g+1), FT_KERNING_DEFAULT, &kerning);
		++g;
		kerning.x >>= 6;
		advances[i] += kerning.x;
		kern |= (kerning.x != 0);
	    }
	    si.hasPositioning |= kern;
	}
    }

    si.width = 0;
    advance_t *end = advances + si.num_glyphs;
    while ( advances < end )
	si.width += *(advances++);

    return;
}

