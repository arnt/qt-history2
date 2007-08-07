/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/

#include "harfbuzz-shaper.h"
#include "harfbuzz-shaper-private.h"

#include <assert.h>

// tibetan syllables are of the form:
//    head position consonant
//    first sub-joined consonant
//    ....intermediate sub-joined consonants (if any)
//    last sub-joined consonant
//    sub-joined vowel (a-chung U+0F71)
//    standard or compound vowel sign (or 'virama' for devanagari transliteration)

enum TibetanForm {
    TibetanOther,
    TibetanHeadConsonant,
    TibetanSubjoinedConsonant,
    TibetanSubjoinedVowel,
    TibetanVowel
};

// this table starts at U+0f40
static const unsigned char tibetanForm[0x80] = {
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,

    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,

    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,

    TibetanOther, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,

    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanOther, TibetanOther, TibetanOther
};


static inline TibetanForm tibetan_form(HB_UChar16 c)
{
    return (TibetanForm)tibetanForm[c - 0x0f40];
}

static const HB_OpenTypeFeature tibetan_features[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('a', 'b', 'v', 's'), AboveSubstProperty },
    { FT_MAKE_TAG('b', 'l', 'w', 's'), BelowSubstProperty },
    {0, 0}
};

static bool tibetan_shape_syllable(HB_Bool openType, HB_ShaperItem *item, bool invalid)
{
    int len = item->item.length;

    if (item->num_glyphs < item->item.length + 4) {
        item->num_glyphs = item->item.length + 4;
        return false;
    }

    uint32_t i;
    HB_STACKARRAY(HB_UChar16, reordered, len + 4);

    const HB_UChar16 *str = item->string + item->item.pos;
    if (invalid) {
        *reordered = 0x25cc;
        memcpy(reordered+1, str, len*sizeof(HB_UChar16));
        len++;
        str = reordered;
    }

#ifndef NO_OPENTYPE
    const int availableGlyphs = item->num_glyphs;
#endif
    HB_Bool haveGlyphs = item->font->klass->stringToGlyphs(item->font,
                                                           str, len,
                                                           item->glyphs, &item->num_glyphs,
                                                           item->item.bidiLevel % 2);

    HB_FREE_STACKARRAY(reordered);

    if (!haveGlyphs)
        return false;

    for (i = 0; i < item->item.length; i++) {
        item->attributes[i].mark = false;
        item->attributes[i].clusterStart = false;
        item->attributes[i].justification = 0;
        item->attributes[i].zeroWidth = false;
//        IDEBUG("    %d: %4x", i, str[i]);
    }

    // now we have the syllable in the right order, and can start running it through open type.

#ifndef NO_OPENTYPE
    if (openType) {
        HB_OpenTypeShape(item, /*properties*/0);
        if (!HB_OpenTypePosition(item, availableGlyphs, /*doLogClusters*/false))
            return false;
    }
#endif

    item->attributes[0].clusterStart = true;
    return true;
}


static int tibetan_nextSyllableBoundary(const HB_UChar16 *s, int start, int end, bool *invalid)
{
    const HB_UChar16 *uc = s + start;

    int pos = 0;
    TibetanForm state = tibetan_form(*uc);

//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos]);
    pos++;

    if (state != TibetanHeadConsonant) {
        if (state != TibetanOther)
            *invalid = true;
        goto finish;
    }

    while (pos < end - start) {
        TibetanForm newState = tibetan_form(uc[pos]);
        switch(newState) {
        case TibetanSubjoinedConsonant:
        case TibetanSubjoinedVowel:
            if (state != TibetanHeadConsonant &&
                 state != TibetanSubjoinedConsonant)
                goto finish;
            state = newState;
            break;
        case TibetanVowel:
            if (state != TibetanHeadConsonant &&
                 state != TibetanSubjoinedConsonant &&
                 state != TibetanSubjoinedVowel)
                goto finish;
            break;
        case TibetanOther:
        case TibetanHeadConsonant:
            goto finish;
        }
        pos++;
    }

finish:
    *invalid = false;
    return start+pos;
}

HB_Bool HB_TibetanShape(HB_ShaperItem *item)
{
    assert(item->item.script == HB_Script_Tibetan);

    HB_Bool openType = false;
#ifndef QT_NO_OPENTYPE
    openType = HB_SelectScript(item, tibetan_features);
#endif
    unsigned short *logClusters = item->log_clusters;

    HB_ShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->item.pos;
    int end = sstart + item->item.length;
    while (sstart < end) {
        bool invalid;
        int send = tibetan_nextSyllableBoundary(item->string, sstart, end, &invalid);
//        IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
//               invalid ? "true" : "false");
        syllable.item.pos = sstart;
        syllable.item.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.attributes = item->attributes + first_glyph;
        syllable.offsets = item->offsets + first_glyph;
        syllable.advances = item->advances + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!tibetan_shape_syllable(openType, &syllable, invalid)) {
            item->num_glyphs += syllable.num_glyphs;
            return false;
        }
        // fix logcluster array
        for (int i = sstart; i < send; ++i)
            logClusters[i-item->item.pos] = first_glyph;
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return true;
}

extern "C" void HB_TibetanAttributes(HB_Script /*script*/, const HB_UChar16 *text, uint32_t from, uint32_t len, HB_CharAttributes *attributes)
{
    int end = from + len;
    const HB_UChar16 *uc = text + from;
    attributes += from;
    uint32_t i = 0;
    while (i < len) {
        bool invalid;
        uint32_t boundary = tibetan_nextSyllableBoundary(text, from+i, end, &invalid) - from;

        attributes[i].charStop = true;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].charStop = false;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }
}


