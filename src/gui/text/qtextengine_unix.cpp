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
void QTextEngine::shapeText( int item ) const
{
    assert( item < items.size() );
    QScriptItem &si = items[item];

    if (si.num_glyphs)
	return;

    si.glyph_data_offset = used;

    QFontEngine *font = fontEngine(si);

    if ( !widthOnly ) {
	si.ascent = font->ascent();
	si.descent = font->descent();
    }

    QShaperItem shaper_item;
    shaper_item.script = si.analysis.script;
    shaper_item.string = &string;
    shaper_item.from = si.position;
    shaper_item.length = length(item);
    shaper_item.font = font;
    shaper_item.num_glyphs = qMax(num_glyphs - used, shaper_item.length);
    // ### DesignMetrics
    shaper_item.flags = si.analysis.bidiLevel % 2 ? RightToLeft : 0;

//     qDebug("shaping");
    while (1) {
// 	qDebug("    . num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
	ensureSpace(shaper_item.num_glyphs);
	shaper_item.num_glyphs = num_glyphs - used;
//  	qDebug("    .. num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
	shaper_item.glyphs = glyphs(&si);
	shaper_item.log_clusters = logClusters(&si);
	if (scriptEngines[shaper_item.script].shape(&shaper_item))
	    break;
    }

//     qDebug("    -> item: script=%d num_glyphs=%d", shaper_item.script, shaper_item.num_glyphs);
    si.num_glyphs = shaper_item.num_glyphs;
    si.hasPositioning = true; // ##### get rid of me

    used += si.num_glyphs;

    QGlyphLayout *g = shaper_item.glyphs;
    // ############ general solution needed
#if defined(Q_WS_X11) && !defined(QT_NO_XFTFREETYPE)
    if ( this->font(si).d->kerning && font->type() == QFontEngine::Xft) {
	FT_Face face = static_cast<QFontEngineXft *>(font)->freetypeFace();
	if (FT_HAS_KERNING(face)) {
	    FT_Vector kerning;
	    bool kern = false;
	    for (int i = 0; i < si.num_glyphs-1; ++i) {
		FT_Get_Kerning(face, g[i].glyph, g[i+1].glyph, FT_KERNING_DEFAULT, &kerning);
		g[i].advance.x += Q26Dot6(kerning.x, F26Dot6);
		g[i].advance.y += Q26Dot6(kerning.y, F26Dot6);
		kern |= (kerning.x != 0);
		kern |= (kerning.y != 0);
	    }
	    si.hasPositioning |= kern;
	}
    }
#endif

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while ( g < end )
	si.width += (g++)->advance.x;

    return;
}

