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

// Hangul is a syllable based script. Unicode reserves a large range
// for precomposed hangul, where syllables are already precomposed to
// their final glyph shape. In addition, a so called jamo range is
// defined, that can be used to express old Hangul. Modern hangul
// syllables can also be expressed as jamo, and should be composed
// into syllables. The operation is rather simple and mathematical.

// Every hangul jamo is classified as being either a Leading consonant
// (L), and intermediat Vowel (V) or a trailing consonant (T). Modern
// hangul syllables (the ones in the precomposed area can be of type
// LV or LVT.
//
// Syllable breaks do _not_ occur between:
//
// L              L, V or precomposed
// V, LV          V, T
// LVT, T         T
//
// A standard syllable is of the form L+V+T*. The above rules allow
// nonstandard syllables L*V*T*. To transform them into standard
// syllables fill characters L_f and V_f can be inserted.

enum {
    Hangul_SBase = 0xac00,
    Hangul_LBase = 0x1100,
    Hangul_VBase = 0x1161,
    Hangul_TBase = 0x11a7,
    Hangul_SCount = 11172,
    Hangul_LCount = 19,
    Hangul_VCount = 21,
    Hangul_TCount = 28,
    Hangul_NCount = 21*28
};

static inline bool hangul_isPrecomposed(unsigned short uc) {
    return (uc >= Hangul_SBase && uc < Hangul_SBase + Hangul_SCount);
}

static inline bool hangul_isLV(unsigned short uc) {
    return ((uc - Hangul_SBase) % Hangul_TCount == 0);
}

enum HangulType {
    L,
    V,
    T,
    LV,
    LVT,
    X
};

static inline HangulType hangul_type(unsigned short uc) {
    if (uc > Hangul_SBase && uc < Hangul_SBase + Hangul_SCount)
        return hangul_isLV(uc) ? LV : LVT;
    if (uc < Hangul_LBase || uc > 0x11ff)
        return X;
    if (uc < Hangul_VBase)
        return L;
    if (uc < Hangul_TBase)
        return V;
    return T;
}

static int hangul_nextSyllableBoundary(const HB_UChar16 *s, int start, int end)
{
    const HB_UChar16 *uc = s + start;

    HangulType state = hangul_type(*uc);
    int pos = 1;

    while (pos < end - start) {
        HangulType newState = hangul_type(uc[pos]);
        switch(newState) {
        case X:
            goto finish;
        case L:
        case V:
        case T:
            if (state > newState)
                goto finish;
            state = newState;
            break;
        case LV:
            if (state > L)
                goto finish;
            state = V;
            break;
        case LVT:
            if (state > L)
                goto finish;
            state = T;
        }
        ++pos;
    }

 finish:
    return start+pos;
}

#ifndef NO_OPENTYPE
static const HB_OpenTypeFeature hangul_features [] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('l', 'j', 'm', 'o'), CcmpProperty },
    { FT_MAKE_TAG('j', 'j', 'm', 'o'), CcmpProperty },
    { FT_MAKE_TAG('t', 'j', 'm', 'o'), CcmpProperty },
    { 0, 0 }
};
#endif

static bool hangul_shape_syllable(HB_ShaperItem *item, HB_Bool openType)
{
    const HB_UChar16 *ch = item->string + item->item.pos;

    int i;
    HB_UChar16 composed = 0;
    // see if we can compose the syllable into a modern hangul
    if (item->item.length == 2) {
        int LIndex = ch[0] - Hangul_LBase;
        int VIndex = ch[1] - Hangul_VBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + Hangul_SBase;
    } else if (item->item.length == 3) {
        int LIndex = ch[0] - Hangul_LBase;
        int VIndex = ch[1] - Hangul_VBase;
        int TIndex = ch[2] - Hangul_TBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount &&
            TIndex >= 0 && TIndex < Hangul_TCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + TIndex + Hangul_SBase;
    }


    int len = item->item.length;

    // if we have a modern hangul use the composed form
    if (composed) {
        ch = &composed;
        len = 1;
    }

#ifndef NO_OPENTYPE
    const int availableGlyphs = item->num_glyphs;
#endif
    if (!item->font->klass->stringToGlyphs(item->font,
                                           ch, len,
                                           item->glyphs, &item->num_glyphs,
                                           item->item.bidiLevel % 2))
        return false;
    for (i = 0; i < len; i++) {
        item->attributes[i].mark = false;
        item->attributes[i].clusterStart = false;
        item->attributes[i].justification = 0;
        item->attributes[i].zeroWidth = false;
        //IDEBUG("    %d: %4x", i, ch[i].unicode());
    }

#ifndef NO_OPENTYPE
    if (!composed && openType) {

        HB_STACKARRAY(unsigned short, logClusters, len);
        for (i = 0; i < len; ++i)
            logClusters[i] = i;
        item->log_clusters = logClusters;

        HB_OpenTypeShape(item, /*properties*/0);

        HB_Bool positioned = HB_OpenTypePosition(item, availableGlyphs, /*doLogClusters*/false);

        HB_FREE_STACKARRAY(logClusters);

        if (!positioned)
            return false;
    }
#endif

    item->attributes[0].clusterStart = true;
    return true;
}

HB_Bool HB_HangulShape(HB_ShaperItem *item)
{
    assert(item->item.script == HB_Script_Hangul);

    const HB_UChar16 *uc = item->string + item->item.pos;;;;

    bool allPrecomposed = true;
    for (uint32_t i = 0; i < item->item.length; ++i) {
        if (!hangul_isPrecomposed(uc[i])) {
            allPrecomposed = false;
            break;
        }
    }

    if (!allPrecomposed) {
        HB_Bool openType = false;
#ifndef NO_OPENTYPE
        openType = HB_SelectScript(item, hangul_features);
#endif

        unsigned short *logClusters = item->log_clusters;

        HB_ShaperItem syllable = *item;
        int first_glyph = 0;

        int sstart = item->item.pos;
        int end = sstart + item->item.length;
        while (sstart < end) {
            int send = hangul_nextSyllableBoundary(item->string, sstart, end);

            syllable.item.pos = sstart;
            syllable.item.length = send-sstart;
            syllable.glyphs = item->glyphs + first_glyph;
            syllable.attributes = item->attributes + first_glyph;
            syllable.offsets = item->offsets + first_glyph;
            syllable.advances = item->advances + first_glyph;
            syllable.num_glyphs = item->num_glyphs - first_glyph;
            if (!hangul_shape_syllable(&syllable, openType)) {
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

    return HB_BasicShape(item);
}


