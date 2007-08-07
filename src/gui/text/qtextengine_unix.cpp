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

static HB_Bool hb_stringToGlyphs(HB_Font font, const HB_UChar16 *string, hb_uint32 length, HB_Glyph *glyphs, hb_uint32 *numGlyphs, HB_Bool rightToLeft)
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

    for (hb_uint32 i = 0; i < *numGlyphs; ++i)
        glyphs[i] = qglyphs[i].glyph;

    return true;
}

static void hb_getAdvances(HB_Font font, const HB_Glyph *glyphs, hb_uint32 numGlyphs, HB_Fixed *advances, int flags)
{
    QFontEngine *fe = (QFontEngine *)font->userData;

    QVarLengthArray<QGlyphLayout> qglyphs(numGlyphs);
    for (hb_uint32 i = 0; i < numGlyphs; ++i)
        qglyphs[i].glyph = glyphs[i];

    fe->recalcAdvances(numGlyphs, qglyphs.data(), flags & HB_ShaperFlag_UseDesignMetrics ? QTextEngine::DesignMetrics : QFlag(0));

    for (hb_uint32 i = 0; i < numGlyphs; ++i)
        advances[i] = qglyphs[i].advance.x.value();
}

static HB_Bool hb_canRender(HB_Font font, const HB_UChar16 *string, hb_uint32 length)
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

    QTextEngine::ShaperFlags shaperFlags(QTextEngine::GlyphIndicesOnly);
    if (item->item.bidiLevel % 2)
        shaperFlags |= QTextEngine::RightToLeft;

    bool result = fontEngine->stringToCMap(reinterpret_cast<const QChar *>(item->string + item->item.pos),
                                           item->item.length,
                                           qglyphs.data(), &nGlyphs,
                                           shaperFlags);
    item->num_glyphs = nGlyphs;
    if (!result)
        return false;

    for (hb_uint32 i = 0; i < item->num_glyphs; ++i)
        itemGlyphs[i] = qglyphs[i].glyph;

    return true;
}

void QTextEngine::shapeText(int item) const
{
    shapeTextWithHarfbuzz(item);
}

