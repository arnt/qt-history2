/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextengine_p.h"

// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars and glyphs
// and no reordering.
// also computes logClusters heuristically
static void heuristicSetGlyphAttributes(const QChar *uc, int length, QGlyphLayout *glyphs, unsigned short *logClusters, int num_glyphs)
{
    // ### zeroWidth and justification are missing here!!!!!

    Q_ASSERT(num_glyphs <= length);

//     qDebug("QScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", item->num_glyphs);

    const bool symbolFont = false; // ####
    glyphs[0].attributes.mark = false;
    glyphs[0].attributes.clusterStart = true;
    glyphs[0].attributes.dontPrint = (!symbolFont && uc[0].unicode() == 0x00ad) || qIsControlChar(uc[0].unicode());

    int pos = 0;
    int lastCat = QChar::category(uc[0].unicode());
    for (int i = 1; i < length; ++i) {
        if (logClusters[i] == pos)
            // same glyph
            continue;
        ++pos;
        while (pos < logClusters[i]) {
            ++pos;
        }
        // hide soft-hyphens by default
        if ((!symbolFont && uc[i].unicode() == 0x00ad) || qIsControlChar(uc[i].unicode()))
            glyphs[pos].attributes.dontPrint = true;
        const QUnicodeTables::Properties *prop = QUnicodeTables::properties(uc[i].unicode());
        int cat = prop->category;

        // one gets an inter character justification point if the current char is not a non spacing mark.
        // as then the current char belongs to the last one and one gets a space justification point
        // after the space char.
        if (lastCat == QChar::Separator_Space)
            glyphs[pos-1].attributes.justification = HB_Space;
        else if (cat != QChar::Mark_NonSpacing)
            glyphs[pos-1].attributes.justification = HB_Character;
        else
            glyphs[pos-1].attributes.justification = HB_NoJustification;

        lastCat = cat;
    }
    pos = logClusters[length-1];
    if (lastCat == QChar::Separator_Space)
        glyphs[pos].attributes.justification = HB_Space;
    else
        glyphs[pos].attributes.justification = HB_Character;
}

void QTextEngine::shapeTextWithAtsui(int item) const
{
    QScriptItem &si = layoutData->items[item];

    si.glyph_data_offset = layoutData->used;

    QFontEngine *font = fontEngine(si, &si.ascent, &si.descent);
    Q_ASSERT(font->type() == QFontEngine::Multi);
    QFontEngineMacMulti *fe = static_cast<QFontEngineMacMulti *>(font);

    QTextEngine::ShaperFlags flags;
    if (si.analysis.bidiLevel % 2)
        flags |= RightToLeft;
    if (option.useDesignMetrics())
	flags |= DesignMetrics;

    attributes(); // pre-initialize char attributes

    int num_glyphs = length(item);

    while (true) {
	ensureSpace(num_glyphs);
        num_glyphs = layoutData->num_glyphs - layoutData->used;

        const QChar *str = layoutData->string.unicode() + si.position;
        int len = length(item);
        QGlyphLayout *g = glyphs(&si);
        unsigned short *log_clusters = logClusters(&si);

        if (fe->stringToCMap(str,
                             len,
                             g,
                             &num_glyphs,
                             flags,
                             log_clusters,
                             attributes())) {

		heuristicSetGlyphAttributes(str, len, g, log_clusters, num_glyphs);
		break;
	}
    }

    si.num_glyphs = num_glyphs;

    layoutData->used += si.num_glyphs;

    QGlyphLayout *g = glyphs(&si);

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while (g < end)
        si.width += (g++)->advance.x;
}

#if 0
void QTextEngine::shapeText(int item) const
{
    Q_ASSERT(item < layoutData->items.size());
    QScriptItem &si = layoutData->items[item];

    if (si.num_glyphs)
        return;

    si.glyph_data_offset = layoutData->used;

    QFontEngine *font = fontEngine(si, &si.ascent, &si.descent);

    QShaperItem shaper_item;
    shaper_item.charAttributes = attributes();
    shaper_item.script = si.analysis.script;
    shaper_item.string = &layoutData->string;
    shaper_item.from = si.position;
    shaper_item.length = length(item);
    shaper_item.font = font;
    shaper_item.num_glyphs = qMax(layoutData->num_glyphs - layoutData->used, shaper_item.length);
    shaper_item.flags = si.analysis.bidiLevel % 2 ? RightToLeft : 0;
    if (option.useDesignMetrics())
        shaper_item.flags |= DesignMetrics;

    //     qDebug("shaping");
    while (1) {
        //      qDebug("    . num_glyphs=%d, layoutData->used=%d, item.num_glyphs=%d", num_glyphs, layoutData->used, shaper_item.num_glyphs);
        ensureSpace(shaper_item.num_glyphs);
        shaper_item.num_glyphs = layoutData->num_glyphs - layoutData->used;
        //      qDebug("    .. num_glyphs=%d, layoutData->used=%d, item.num_glyphs=%d", num_glyphs, layoutData->used, shaper_item.num_glyphs);
        shaper_item.glyphs = glyphs(&si);
        shaper_item.log_clusters = logClusters(&si);
        if (qt_scriptEngines[shaper_item.script].shape(&shaper_item))
            break;
    }

    //     qDebug("    -> item: script=%d num_glyphs=%d", shaper_item.script, shaper_item.num_glyphs);
    si.num_glyphs = shaper_item.num_glyphs;

    layoutData->used += si.num_glyphs;

    QGlyphLayout *g = shaper_item.glyphs;
    if (this->font(si).d->kerning)
        font->doKerning(si.num_glyphs, g, option.useDesignMetrics() ?
                        QFlag(QTextEngine::DesignMetrics) : QFlag(0));

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while (g < end)
        si.width += (g++)->advance.x;

    return;
}
#endif
