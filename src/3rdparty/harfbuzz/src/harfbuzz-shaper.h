/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_SHAPER_H
#define HARFBUZZ_SHAPER_H

#include <harfbuzz.h>

HB_BEGIN_HEADER

typedef enum {
        HB_Script_Common,
        HB_Script_Greek,
        HB_Script_Cyrillic,
        HB_Script_Armenian,
        HB_Script_Hebrew,
        HB_Script_Arabic,
        HB_Script_Syriac,
        HB_Script_Thaana,
        HB_Script_Devanagari,
        HB_Script_Bengali,
        HB_Script_Gurmukhi,
        HB_Script_Gujarati,
        HB_Script_Oriya,
        HB_Script_Tamil,
        HB_Script_Telugu,
        HB_Script_Kannada,
        HB_Script_Malayalam,
        HB_Script_Sinhala,
        HB_Script_Thai,
        HB_Script_Lao,
        HB_Script_Tibetan,
        HB_Script_Myanmar,
        HB_Script_Georgian,
        HB_Script_Hangul,
        HB_Script_Ogham,
        HB_Script_Runic,
        HB_Script_Khmer,
        HB_Script_Inherited,
        HB_ScriptCount = HB_Script_Inherited
        /*
        HB_Script_Latin = Common,
        HB_Script_Ethiopic = Common,
        HB_Script_Cherokee = Common,
        HB_Script_CanadianAboriginal = Common,
        HB_Script_Mongolian = Common,
        HB_Script_Hiragana = Common,
        HB_Script_Katakana = Common,
        HB_Script_Bopomofo = Common,
        HB_Script_Han = Common,
        HB_Script_Yi = Common,
        HB_Script_OldItalic = Common,
        HB_Script_Gothic = Common,
        HB_Script_Deseret = Common,
        HB_Script_Tagalog = Common,
        HB_Script_Hanunoo = Common,
        HB_Script_Buhid = Common,
        HB_Script_Tagbanwa = Common,
        HB_Script_Limbu = Common,
        HB_Script_TaiLe = Common,
        HB_Script_LinearB = Common,
        HB_Script_Ugaritic = Common,
        HB_Script_Shavian = Common,
        HB_Script_Osmanya = Common,
        HB_Script_Cypriot = Common,
        HB_Script_Braille = Common,
        HB_Script_Buginese = Common,
        HB_Script_Coptic = Common,
        HB_Script_NewTaiLue = Common,
        HB_Script_Glagolitic = Common,
        HB_Script_Tifinagh = Common,
        HB_Script_SylotiNagri = Common,
        HB_Script_OldPersian = Common,
        HB_Script_Kharoshthi = Common,
        HB_Script_Balinese = Common,
        HB_Script_Cuneiform = Common,
        HB_Script_Phoenician = Common,
        HB_Script_PhagsPa = Common,
        HB_Script_Nko = Common
        */
} HB_Script;

typedef struct
{
    uint32_t pos;
    uint32_t length;
    HB_Script script;
    uint8_t bidiLevel;
} HB_ScriptItem;

typedef enum {
    HB_NoBreak,
    HB_SoftHyphen,
    HB_Break,
    HB_ForcedBreak
} HB_LineBreakType;


typedef struct {
    HB_LineBreakType lineBreakType  :2;
    HB_Bool whiteSpace              :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    HB_Bool charStop                :1;     // Valid cursor position (for left/right arrow)
    uint8_t unused                  :4;
} HB_CharAttributes;

void HB_GetCharAttributes(const HB_UChar16 *string, uint32_t stringLength,
                          const HB_ScriptItem *items, uint32_t numItems,
                          HB_CharAttributes *attributes);


typedef enum {
    HB_LeftToRight = 0,
    HB_RightToLeft = 1
} HB_StringToGlyphsFlags;

typedef enum {
    HB_ShaperFlag_Default = 0,
    HB_ShaperFlag_NoKerning = 1,
    HB_ShaperFlag_UseDesignMetrics = 2
} HB_ShaperFlag;

// highest value means highest priority for justification. Justification is done by first inserting kashidas
// starting with the highest priority positions, then stretching spaces, afterwards extending inter char
// spacing, and last spacing between arabic words.
// NoJustification is for example set for arabic where no Kashida can be inserted or for diacritics.
typedef enum {
    HB_NoJustification= 0,   // Justification can't be applied after this glyph
    HB_Arabic_Space   = 1,   // This glyph represents a space inside arabic text
    HB_Character      = 2,   // Inter-character justification point follows this glyph
    HB_Space          = 4,   // This glyph represents a blank outside an Arabic run
    HB_Arabic_Normal  = 7,   // Normal Middle-Of-Word glyph that connects to the right (begin)
    HB_Arabic_Waw     = 8,    // Next character is final form of Waw/Ain/Qaf/Fa
    HB_Arabic_BaRa    = 9,   // Next two chars are Ba + Ra/Ya/AlefMaksura
    HB_Arabic_Alef    = 10,  // Next character is final form of Alef/Tah/Lam/Kaf/Gaf
    HB_Arabic_HaaDal  = 11,  // Next character is final form of Haa/Dal/Taa Marbutah
    HB_Arabic_Seen    = 12,  // Initial or Medial form Of Seen/Sad
    HB_Arabic_Kashida = 13   // Kashida(U+640) in middle of word
} HB_JustificationClass;

typedef struct {
    unsigned short justification   :4;  // Justification class
    unsigned short clusterStart    :1;  // First glyph of representation of cluster
    unsigned short mark            :1;  // needs to be positioned around base char
    unsigned short zeroWidth       :1;  // ZWJ, ZWNJ etc, with no width
    unsigned short dontPrint       :1;
    unsigned short combiningClass  :8;
} HB_GlyphAttributes;

typedef struct {
    HB_Bool isSymbolFont;

    HB_GDEF gdef;
    HB_GSUB gsub;
    HB_GPOS gpos;
    HB_Bool supported_scripts[HB_ScriptCount];
    HB_Buffer buffer;
    HB_Script current_script;
    int current_flags; // HB_ShaperFlags
    HB_Bool has_opentype_kerning;
    HB_Bool glyphs_substituted;
    HB_GlyphAttributes *tmpAttributes;
    unsigned int *tmpLogClusters;
    int length;
    int orig_nglyphs;
} HB_FaceRec, *HB_Face;


HB_Face HB_NewFace(HB_Font font);
void HB_FreeFace(HB_Face face);

typedef struct {
    HB_Bool (*stringToGlyphs)(HB_Font font, const HB_UChar16 *string, uint32_t length, HB_Glyph *glyphs, uint32_t *numGlyphs, HB_Bool rightToLeft);
    void    (*getAdvances)(HB_Font font, const HB_Glyph *glyphs, int numGlyphs, HB_Fixed *advances, int flags /*HB_ShaperFlag*/);
    HB_Bool (*canRender)(HB_Font font, const HB_UChar16 *string, uint32_t length);
    HB_Stream (*getSFntTable)(HB_Font font, HB_Tag tag);
    /* implementation needs to make sure to load a scaled glyph, so /no/ FT_LOAD_NO_SCALE */
    HB_Error (*getPointInOutline)(HB_Font font, HB_Glyph glyph, int flags /*HB_ShaperFlag*/, uint32_t point, HB_Fixed *xpos, HB_Fixed *ypos, uint32_t *nPoints);
} HB_FontClass;

typedef struct HB_Font_ {
    const HB_FontClass *klass;

    /* Metrics */
    HB_UShort x_ppem, y_ppem;
    HB_16Dot16 x_scale, y_scale;

    void *faceData;
    void *userData;
} HB_FontRec;

typedef struct {
    const HB_UChar16 *string;
    uint32_t stringLength;
    HB_ScriptItem item;
    HB_Font font;
    HB_Face face;
    int shaperFlags; // HB_ShaperFlags

    uint32_t num_glyphs; // in: available glyphs out: glyphs used/needed
    HB_Glyph *glyphs; // out parameter
    HB_GlyphAttributes *attributes; // out
    HB_Fixed *advances; // out
    HB_FixedPoint *offsets; // out
    unsigned short *log_clusters; // out

    // internal
    HB_Bool kerning_applied; // out: kerning applied by shaper
} HB_ShaperItem;

HB_Bool HB_ShapeItem(HB_ShaperItem *item);

HB_END_HEADER

#endif // HARFBUZZ_SHAPER_H
