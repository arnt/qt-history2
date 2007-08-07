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

#include <private/qfontengine_ft_p.h>

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

    bool kerningEnabled = this->font(si).d->kerning;

#if 0
    QShaperItem shaper_item;
    shaper_item.kerning_enabled = kerningEnabled;
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

    bool ke
#else
    HB_ShaperItem shaper_item;
    shaper_item.kerning_applied = false;
    shaper_item.string = reinterpret_cast<const HB_UChar16 *>(layoutData->string.constData());
    shaper_item.stringLength = layoutData->string.length();
    shaper_item.item.script = (HB_Script)si.analysis.script;
    shaper_item.item.pos = si.position;
    shaper_item.item.length = length(item);
    shaper_item.item.bidiLevel = si.analysis.bidiLevel;
    // ##### multi font engines...
    Q_ASSERT(font->type() != QFontEngine::Multi);
    Q_ASSERT(font->type() == QFontEngine::Freetype);
    shaper_item.font = static_cast<QFontEngineFT *>(font)->harfbuzzFont();
    shaper_item.num_glyphs = qMax(uint32_t(layoutData->num_glyphs - layoutData->used), shaper_item.item.length);

    //shaper_item.flags = si.analysis.bidiLevel % 2 ? RightToLeft : 0;

    shaper_item.shaperFlags = 0;
    if (!kerningEnabled)
        shaper_item.shaperFlags |= HB_ShaperFlag_NoKerning;
    if (option.useDesignMetrics())
        shaper_item.shaperFlags |= HB_ShaperFlag_UseDesignMetrics;

    while (1) {
//         qDebug("    . num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
        ensureSpace(shaper_item.num_glyphs);
        shaper_item.num_glyphs = layoutData->num_glyphs - layoutData->used;

        // ################
        QVarLengthArray<HB_Glyph> hb_glyphs(shaper_item.num_glyphs);
        QVarLengthArray<HB_GlyphAttributes> hb_attributes(shaper_item.num_glyphs);
        QVarLengthArray<HB_Fixed> hb_advances(shaper_item.num_glyphs);
        QVarLengthArray<HB_FixedPoint> hb_offsets(shaper_item.num_glyphs);
        memset(hb_glyphs.data(), 0, hb_glyphs.size() * sizeof(HB_Glyph));
        memset(hb_attributes.data(), 0, hb_attributes.size() * sizeof(HB_GlyphAttributes));
        memset(hb_advances.data(), 0, hb_advances.size() * sizeof(HB_Fixed));
        memset(hb_offsets.data(), 0, hb_offsets.size() * sizeof(HB_FixedPoint));

        shaper_item.glyphs = hb_glyphs.data();
        shaper_item.attributes = hb_attributes.data();
        shaper_item.advances = hb_advances.data();
        shaper_item.offsets = hb_offsets.data();
        shaper_item.log_clusters = logClusters(&si);

//          qDebug("    .. num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);
//        shaper_item.glyphs = glyphs(&si);
//        shaper_item.log_clusters = logClusters(&si);

        if (HB_ShapeItem(&shaper_item)) {
            QGlyphLayout *g = glyphs(&si);

            for (uint32_t i = 0; i < shaper_item.num_glyphs; ++i) {
                g[i].glyph = shaper_item.glyphs[i];
                g[i].advance.x = QFixed::fromFixed(shaper_item.advances[i]);
                g[i].advance.y = QFixed();
                g[i].offset.x = QFixed::fromFixed(shaper_item.offsets[i].x);
                g[i].offset.y = QFixed::fromFixed(shaper_item.offsets[i].y);

                g[i].attributes.justification = shaper_item.attributes[i].justification;
                g[i].attributes.clusterStart = shaper_item.attributes[i].clusterStart;
                g[i].attributes.mark = shaper_item.attributes[i].mark;
                g[i].attributes.zeroWidth = shaper_item.attributes[i].zeroWidth;
                g[i].attributes.dontPrint = shaper_item.attributes[i].dontPrint;
                g[i].attributes.combiningClass = shaper_item.attributes[i].combiningClass;
            }

            break;
        }
    }

//     qDebug("    -> item: script=%d num_glyphs=%d", shaper_item.script, shaper_item.num_glyphs);
    si.num_glyphs = shaper_item.num_glyphs;
#endif

    layoutData->used += si.num_glyphs;

    QGlyphLayout *g = glyphs(&si);
    if (kerningEnabled && !shaper_item.kerning_applied) {
        font->doKerning(si.num_glyphs, g, option.useDesignMetrics() ? QFlag(QTextEngine::DesignMetrics) : QFlag(0));
    }

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while (g < end)
        si.width += (g++)->advance.x;

    return;
}

