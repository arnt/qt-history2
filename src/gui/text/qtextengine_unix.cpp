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

#include <private/qharfbuzz_p.h>

#include <private/qfontengine_ft_p.h>

static HB_Bool hb_stringToGlyphs(HB_Font font, const HB_UChar16 *string, uint32_t length, HB_Glyph *glyphs, uint32_t *numGlyphs, HB_Bool rightToLeft)
{
    QFontEngine *fe = (QFontEngine *)font->userData;

    QVarLengthArray<QGlyphLayout> qglyphs(*numGlyphs);

    QTextEngine::ShaperFlags shaperFlags(QTextEngine::GlyphIndicesOnly);
    if (rightToLeft)
        shaperFlags |= QTextEngine::RightToLeft;

    int nGlyphs = *numGlyphs;
    bool result = fe->stringToCMap(reinterpret_cast<const QChar *>(string), length, qglyphs.data(), &nGlyphs, shaperFlags);
    *numGlyphs = nGlyphs;
    if (!result)
        return false;

    for (uint32_t i = 0; i < *numGlyphs; ++i)
        glyphs[i] = qglyphs[i].glyph;

    return true;
}

static void hb_getAdvances(HB_Font font, const HB_Glyph *glyphs, int numGlyphs, HB_Fixed *advances, int flags)
{
    QFontEngine *fe = (QFontEngine *)font->userData;

    QVarLengthArray<QGlyphLayout> qglyphs(numGlyphs);
    for (int i = 0; i < numGlyphs; ++i)
        qglyphs[i].glyph = glyphs[i];

    fe->recalcAdvances(numGlyphs, qglyphs.data(), flags & HB_ShaperFlag_UseDesignMetrics ? QTextEngine::DesignMetrics : QFlag(0));

    for (int i = 0; i < numGlyphs; ++i)
        advances[i] = qglyphs[i].advance.x.value();
}

static HB_Bool hb_canRender(HB_Font font, const HB_UChar16 *string, uint32_t length)
{
    QFontEngine *fe = (QFontEngine *)font->userData;
    return fe->canRender(reinterpret_cast<const QChar *>(string), length);
}

static void hb_getGlyphMetrics(HB_Font font, HB_Glyph glyph, HB_GlyphMetrics *metrics)
{
    QFontEngine *fe = (QFontEngine *)font->userData;
    glyph_metrics_t m = fe->boundingBox(glyph);
    metrics->x = m.x.value();
    metrics->y = m.y.value();
    metrics->width = m.width.value();
    metrics->height = m.height.value();
    metrics->xOffset = m.xoff.value();
    metrics->yOffset = m.yoff.value();
}

static HB_Fixed hb_getAscent(HB_Font font)
{
    QFontEngine *fe = (QFontEngine *)font->userData;
    return fe->ascent().value();
}

const HB_FontClass hb_fontClass = {
    hb_stringToGlyphs, hb_getAdvances, hb_canRender, /*getPointInOutline*/0,
    hb_getGlyphMetrics, hb_getAscent
};

static bool stringToGlyphs(HB_ShaperItem *item, HB_Glyph *itemGlyphs, QFontEngine *fontEngine)
{
    int nGlyphs = item->num_glyphs;
    QVarLengthArray<QGlyphLayout> qglyphs(nGlyphs);

    bool result = fontEngine->stringToCMap(reinterpret_cast<const QChar *>(item->string + item->item.pos),
                                           item->item.length,
                                           qglyphs.data(), &nGlyphs,
                                           (item->item.bidiLevel % 2) ? QTextEngine::RightToLeft : QFlag(0));
    item->num_glyphs = nGlyphs;
    if (!result)
        return false;

    for (uint32_t i = 0; i < item->num_glyphs; ++i)
        itemGlyphs[i] = qglyphs[i].glyph;

    return true;
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

    HB_ShaperItem entire_shaper_item;
    entire_shaper_item.kerning_applied = false;
    entire_shaper_item.string = reinterpret_cast<const HB_UChar16 *>(layoutData->string.constData());
    entire_shaper_item.stringLength = layoutData->string.length();
    entire_shaper_item.item.script = (HB_Script)si.analysis.script;
    entire_shaper_item.item.pos = si.position;
    entire_shaper_item.item.length = length(item);
    entire_shaper_item.item.bidiLevel = si.analysis.bidiLevel;

    entire_shaper_item.shaperFlags = 0;
    if (!kerningEnabled)
        entire_shaper_item.shaperFlags |= HB_ShaperFlag_NoKerning;
    if (option.useDesignMetrics())
        entire_shaper_item.shaperFlags |= HB_ShaperFlag_UseDesignMetrics;

    entire_shaper_item.num_glyphs = qMax(layoutData->num_glyphs - layoutData->used, int(entire_shaper_item.item.length));

    QVarLengthArray<HB_Glyph> hb_initial_glyphs(entire_shaper_item.num_glyphs);

    if (!stringToGlyphs(&entire_shaper_item, hb_initial_glyphs.data(), font)) {
        hb_initial_glyphs.resize(entire_shaper_item.num_glyphs);
        if (!stringToGlyphs(&entire_shaper_item, hb_initial_glyphs.data(), font)) {
            // ############ if this happens there's a bug in the fontengine
            return;
        }
    }

    QVarLengthArray<int> itemBoundaries(2);
    // k * 2 entries, array[k] == index in string, array[k + 1] == index in glyphs
    itemBoundaries[0] = entire_shaper_item.item.pos;
    itemBoundaries[1] = 0;

    if (font->type() == QFontEngine::Multi) {
        uint lastEngine = 0;
        int charIdx = entire_shaper_item.item.pos;
        const int stringEnd = charIdx + entire_shaper_item.item.length;
        for (quint32 i = 0; i < entire_shaper_item.num_glyphs; ++i, ++charIdx) {
            uint engineIdx = hb_initial_glyphs[i] >> 24;
            if (engineIdx != lastEngine && i > 0) {
                itemBoundaries.append(charIdx);
                itemBoundaries.append(i);
            }
            lastEngine = engineIdx;
            if (HB_IsHighSurrogate(entire_shaper_item.string[charIdx])
                && charIdx < stringEnd - 1
                && HB_IsLowSurrogate(entire_shaper_item.string[charIdx + 1]))
                ++charIdx;
        }
    }

    int glyph_pos = 0;
    for (int k = 0; k < itemBoundaries.size(); k += 2) {

        HB_ShaperItem shaper_item = entire_shaper_item;

        shaper_item.item.pos = itemBoundaries[k];
        if (k < itemBoundaries.size() - 3) {
            shaper_item.item.length = itemBoundaries[k + 2] - shaper_item.item.pos;
            shaper_item.num_glyphs = itemBoundaries[k + 3] - itemBoundaries[k + 1];
        } else {
            shaper_item.item.length -= shaper_item.item.pos - entire_shaper_item.item.pos;
            shaper_item.num_glyphs -= itemBoundaries[k + 1];
        }

        QVarLengthArray<HB_Glyph> hb_glyphs(shaper_item.num_glyphs);
        QVarLengthArray<HB_GlyphAttributes> hb_attributes(shaper_item.num_glyphs);
        QVarLengthArray<HB_Fixed> hb_advances(shaper_item.num_glyphs);
        QVarLengthArray<HB_FixedPoint> hb_offsets(shaper_item.num_glyphs);

        QFontEngine *actualFontEngine = font;
        QFontEngineFT *ftEngine = 0;
        uint engineIdx = 0;
        if (font->type() == QFontEngine::Multi) {
            engineIdx = uint(hb_initial_glyphs[itemBoundaries[k + 1]] >> 24);

            actualFontEngine = static_cast<QFontEngineMulti *>(font)->engine(engineIdx);
        }

        if (actualFontEngine->type() == QFontEngine::Freetype) {
            ftEngine = static_cast<QFontEngineFT *>(actualFontEngine);
        }

        HB_FontClass fontClass = hb_fontClass;
        HB_FontRec hbFont;
        memset(&hbFont, 0, sizeof(hbFont));
        hbFont.klass = &fontClass;
        hbFont.userData = actualFontEngine;

        shaper_item.font = &hbFont;

        if (ftEngine) {
            Q_ASSERT(ftEngine);
            ftEngine->lockFace();

            ftEngine->setupHarfbuzzFont(&hbFont, &fontClass);

            shaper_item.face = ftEngine->harfbuzzFace();
        } else {
            shaper_item.face = qHBNewFace(0, 0);
        }

        while (1) {
            ensureSpace(glyph_pos + shaper_item.num_glyphs);
            shaper_item.num_glyphs = layoutData->num_glyphs - layoutData->used - glyph_pos;

            hb_glyphs.resize(shaper_item.num_glyphs);
            hb_attributes.resize(shaper_item.num_glyphs);
            hb_advances.resize(shaper_item.num_glyphs);
            hb_offsets.resize(shaper_item.num_glyphs);

            memset(hb_glyphs.data(), 0, hb_glyphs.size() * sizeof(HB_Glyph));
            memset(hb_attributes.data(), 0, hb_attributes.size() * sizeof(HB_GlyphAttributes));
            memset(hb_advances.data(), 0, hb_advances.size() * sizeof(HB_Fixed));
            memset(hb_offsets.data(), 0, hb_offsets.size() * sizeof(HB_FixedPoint));

            shaper_item.glyphs = hb_glyphs.data();
            shaper_item.attributes = hb_attributes.data();
            shaper_item.advances = hb_advances.data();
            shaper_item.offsets = hb_offsets.data();
            shaper_item.log_clusters = logClusters(&si) + shaper_item.item.pos - entire_shaper_item.item.pos;

//          qDebug("    .. num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);

            if (!qShapeItem(&shaper_item))
                continue;

            QGlyphLayout *g = glyphs(&si) + glyph_pos;

            for (uint32_t i = 0; i < shaper_item.num_glyphs; ++i) {
                g[i].glyph = shaper_item.glyphs[i] | (engineIdx << 24);
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

            for (uint32_t i = 0; i < shaper_item.item.length; ++i)
                shaper_item.log_clusters[i] += glyph_pos;

            if (kerningEnabled && !shaper_item.kerning_applied)
                font->doKerning(shaper_item.num_glyphs, g, option.useDesignMetrics() ? QFlag(QTextEngine::DesignMetrics) : QFlag(0));

            glyph_pos += shaper_item.num_glyphs;
            if (ftEngine)
                ftEngine->unlockFace();
            else
                qHBFreeFace(shaper_item.face);
            break;
        }
    }

//     qDebug("    -> item: script=%d num_glyphs=%d", shaper_item.script, shaper_item.num_glyphs);
    si.num_glyphs = glyph_pos;

    layoutData->used += si.num_glyphs;

    QGlyphLayout *g = glyphs(&si);

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while (g < end)
        si.width += (g++)->advance.x;

    return;
}

