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


void QTextEngine::shape( int item ) const
{
    QScriptItem &si = items[item];

    if ( si.num_glyphs )
	return;

    QFont::Script script = (QFont::Script)si.analysis.script;
    int from = si.position;
    int len = length( item );

    si.glyph_data_offset = used;


    if ( !si.font() )
	si.setFont(fnt->engineForScript( script ));

    si.ascent = si.font()->ascent();
    si.descent = si.font()->descent();

    if ( si.font() && si.font() != (QFontEngine*)-1 ) {
	assert( script < QFont::NScripts );
	scriptEngines[script].shape( script, string, from, len, (QTextEngine *)this, &si );
    }
    ((QTextEngine *)this)->used += si.num_glyphs;

    si.width = 0;
    QGlyphLayout *glyphs = this->glyphs( &si );
    for ( int i = 0; i < si.num_glyphs; i++ )
	si.width += glyphs[i].advance;

    return;
}

