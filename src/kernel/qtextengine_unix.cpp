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

    if (!si.font())
	si.setFont(fnt->engineForScript( script ));

    if ( !widthOnly ) {
	si.ascent = si.font()->ascent();
	si.descent = si.font()->descent();
    }

    if ( si.font() && si.font() != (QFontEngine*)-1 ) {
	assert( script < QFont::NScripts );
	scriptEngines[script].shape( script, string, from, len, (QTextEngine *)this, &si );
    }
    ((QTextEngine *)this)->used += si.num_glyphs;

    QGlyphLayout *g = glyphs(&si);
#ifndef QT_NO_XFTFREETYPE
    if (kern && si.font()->type() == QFontEngine::Xft) {
	FT_Face face = static_cast<QFontEngineXft *>(si.font())->freetypeFace();
	if (FT_HAS_KERNING(face)) {
	    FT_Vector kerning;
	    bool kern = false;
	    for (int i = 0; i < si.num_glyphs-1; ++i) {
		FT_Get_Kerning(face, g[i].glyph, g[i+1].glyph, FT_KERNING_DEFAULT, &kerning);
		kerning.x >>= 6;
		g[i].advance += kerning.x;
		kern |= (kerning.x != 0);
	    }
	    si.hasPositioning |= kern;
	}
    }
#endif

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while ( g < end )
	si.width += (g++)->advance;

    return;
}

