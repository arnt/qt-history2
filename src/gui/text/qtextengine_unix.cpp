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

#include <qdebug.h>

#if (FREETYPE_MAJOR == 2) && (FREETYPE_MINOR == 1) && (FREETYPE_PATCH < 3)
#  define FT_KERNING_DEFAULT ft_kerning_default
#  define FT_KERNING_UNFITTED ft_kerning_unfitted
#endif

#include <harfbuzz-shaper.h>

HB_LineBreakClass HB_GetLineBreakClass(HB_UChar32 ch)
{
    return (HB_LineBreakClass)QUnicodeTables::lineBreakClass(ch);
}

void HB_GetUnicodeCharProperties(HB_UChar32 ch, HB_CharCategory *category, int *combiningClass)
{
    *category = (HB_CharCategory)QChar::category(ch);
    *combiningClass = QChar::combiningClass(ch);
}

HB_CharCategory HB_GetUnicodeCharCategory(HB_UChar32 ch)
{
    return (HB_CharCategory)QChar::category(ch);
}

int HB_GetUnicodeCharCombiningClass(HB_UChar32 ch)
{
    return QChar::combiningClass(ch);
}

HB_UChar16 HB_GetMirroredChar(HB_UChar16 ch)
{
    return QChar::mirroredChar(ch);
}

void QTextEngine::shapeText(int item) const
{
    Q_ASSERT(item < layoutData->items.size());
    QScriptItem &si = layoutData->items[item];

    if (si.num_glyphs)
        return;

    si.glyph_data_offset = layoutData->used;

    QFontEngine *font = fontEngine(si, &si.ascent, &si.descent);

    QShaperItem shaper_item;
    shaper_item.kerning_enabled = this->font(si).d->kerning;
    shaper_item.kerning_applied = false;
    shaper_item.script = si.analysis.script;
    shaper_item.string = &layoutData->string;
    shaper_item.from = si.position;
    shaper_item.length = length(item);
    shaper_item.font = font;
    shaper_item.num_glyphs = qMax(int(layoutData->num_glyphs - layoutData->used), shaper_item.length);
    shaper_item.flags = si.analysis.bidiLevel % 2 ? RightToLeft : 0;
    if (option.useDesignMetrics())
        shaper_item.flags |= DesignMetrics;

//     qDebug("shaping");
    while (1) {
//         qDebug("    . num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
        ensureSpace(shaper_item.num_glyphs);
        shaper_item.num_glyphs = layoutData->num_glyphs - layoutData->used;
//          qDebug("    .. num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
        shaper_item.glyphs = glyphs(&si);
        shaper_item.log_clusters = logClusters(&si);
        if (qt_scriptEngines[shaper_item.script].shape(&shaper_item))
            break;
    }

//     qDebug("    -> item: script=%d num_glyphs=%d", shaper_item.script, shaper_item.num_glyphs);
    si.num_glyphs = shaper_item.num_glyphs;

    layoutData->used += si.num_glyphs;

    QGlyphLayout *g = shaper_item.glyphs;
    if (shaper_item.kerning_enabled && !shaper_item.kerning_applied) {
        font->doKerning(si.num_glyphs, g, option.useDesignMetrics() ? QFlag(QTextEngine::DesignMetrics) : QFlag(0));
    }

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while (g < end)
        si.width += (g++)->advance.x;

    return;
}

