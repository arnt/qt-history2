/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextengine_p.h"

void QTextEngine::shapeText(int item) const
{    
    assert(item < items.size());
    QScriptItem &si = items[item];

    if (si.num_glyphs)
        return;

    si.glyph_data_offset = used;

    QFontEngine *font = fontEngine(si);

    if (!widthOnly) {
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
    shaper_item.flags = si.analysis.bidiLevel % 2 ? RightToLeft : 0;
    if (option.usesDesignMetrics())
        shaper_item.flags |= DesignMetrics;

    //     qDebug("shaping");
    while (1) {
        //      qDebug("    . num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
        ensureSpace(shaper_item.num_glyphs);
        shaper_item.num_glyphs = num_glyphs - used;
        //      qDebug("    .. num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
        shaper_item.glyphs = glyphs(&si);
        shaper_item.log_clusters = logClusters(&si);
        if (qt_scriptEngines[shaper_item.script].shape(&shaper_item))
            break;
    }

    //     qDebug("    -> item: script=%d num_glyphs=%d", shaper_item.script, shaper_item.num_glyphs);
    si.num_glyphs = shaper_item.num_glyphs;

    used += si.num_glyphs;

    QGlyphLayout *g = shaper_item.glyphs;

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while (g < end)
        si.width += (g++)->advance.x();

    return;
}
