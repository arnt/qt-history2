/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextengine_p.h"

void QTextEngine::shapeText( int item ) const
{
    QScriptItem &si = items[item];

    if ( si.num_glyphs )
	return;

    QFont::Script script = (QFont::Script)si.analysis.script;
    int from = si.position;
    int len = length( item );

#if 0
    si.glyph_data_offset = used;
    if ( !si.font() )
	si.setFont(fnt->engineForScript( script ));
#endif

    QFontEngine *font = fontEngine(si);
    si.ascent = font->ascent();
    si.descent = font->descent();

    assert( script < QFont::NScripts );
    scriptEngines[script].shape( script, string, from, len, const_cast<QTextEngine*>(this), &si );

    const_cast<QTextEngine*>(this)->used += si.num_glyphs;

    si.width = 0;
    QGlyphLayout *glyphs = this->glyphs( &si );
    for ( int i = 0; i < si.num_glyphs; i++ )
	si.width += glyphs[i].advance.x;

    return;
}

