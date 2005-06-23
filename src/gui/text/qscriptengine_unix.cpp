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

// ------------------------------------------------------------------------------------------------------------------
//
// Continuation of middle eastern languages
//
// ------------------------------------------------------------------------------------------------------------------

#include <qdebug.h>

// #### stil missing: identify invalid character combinations
static bool hebrew_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Hebrew);

#ifdef QT_HAVE_FREETYPE
    QOpenType *openType = item->font->openType();

    if (openType && openType->supportsScript(item->script)) {
        int nglyphs = item->num_glyphs;
        if (!item->font->stringToCMap(item->string->unicode()+item->from, item->length, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
            return false;
        heuristicSetGlyphAttributes(item);
        openType->init(item);

        openType->applyGSUBFeature(FT_MAKE_TAG('c', 'c', 'm', 'p'));
        // Uniscribe also defines dlig for Hebrew, but we leave this out for now, as it's mostly
        // ligatures one does not want in modern Hebrew (as lam-alef ligatures).

        openType->applyGPOSFeatures();
        item->num_glyphs = nglyphs;
        return openType->appendTo(item);
    }
#endif
    return basic_shape(item);
}

// #### stil missing: identify invalid character combinations
static bool syriac_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Syriac);

#ifdef QT_HAVE_FREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && openType->supportsScript(QUnicodeTables::Syriac)) {
        return arabicSyriacOpenTypeShape(openType, item);
    }
#endif
    return basic_shape(item);
}


static bool thaana_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Thaana);

#ifdef QT_HAVE_FREETYPE
    QOpenType *openType = item->font->openType();

    if (openType && openType->supportsScript(item->script)) {
        int nglyphs = item->num_glyphs;
        if (!item->font->stringToCMap(item->string->unicode()+item->from, item->length, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
            return false;
        heuristicSetGlyphAttributes(item);
        openType->init(item);

        // thaana only uses positioning features
        openType->applyGPOSFeatures();
        item->num_glyphs = nglyphs;
        return openType->appendTo(item);
    }
#endif
    return basic_shape(item);
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Indic languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------

enum Form {
    Invalid = 0x0,
    Unknown = Invalid,
    Consonant,
    Nukta,
    Halant,
    Matra,
    VowelMark,
    StressMark,
    IndependentVowel,
    LengthMark,
    Control,
    Other
};

static const unsigned char indicForms[0xe00-0x900] = {
    // Devangari
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, StressMark, StressMark, StressMark,
    StressMark, Unknown, Unknown, Unknown,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    IndependentVowel, IndependentVowel, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Bengali
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, VowelMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    IndependentVowel, IndependentVowel, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gurmukhi
    Invalid, Invalid, VowelMark, Invalid,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Unknown, Unknown, Unknown,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Invalid,

    Other, Other, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    StressMark, StressMark, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gujarati
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    Invalid, IndependentVowel, Invalid, IndependentVowel,

    IndependentVowel, IndependentVowel, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Oriya
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, Invalid, Invalid, Invalid,
    Invalid, Unknown, LengthMark, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    //Tamil
    Invalid, Invalid, VowelMark, Other,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Invalid, Invalid,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Invalid, Consonant, Consonant,

    Invalid, Invalid, Invalid, Consonant,
    Consonant, Invalid, Invalid, Invalid,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Telugu
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, LengthMark, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Kannada
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, LengthMark, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Consonant, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Malayalam
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Sinhala
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Invalid, Invalid,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Halant, Invalid,
    Invalid, Invalid, Invalid, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Invalid,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Matra, Matra,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
};

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post,
    Split,
    Base,
    Reph,
    Vattu,
    Inherit
};

static const unsigned char indicPosition[0xe00-0x900] = {
    // Devanagari
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, Above, Above,
    Above, Post, Post, Post,
    Post, None, None, None,

    None, Above, Below, Above,
    Above, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Bengali
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, None, Post,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, Below,
    Below, None, None, Pre,
    Pre, None, None, Split,
    Split, Below, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gurmukhi
    None, None, Above, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Below, None, None, None,
    None, Below, None, None,
    None, Below, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, None,
    None, None, None, Above,
    Above, None, None, Above,
    Above, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Above, Above, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gujarati
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, None, Above,
    Above, Post, None, Post,
    Post, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Oriya
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    Below, None, None, None,
    Below, None, None, None,
    Below, Below, Below, Post,

    Below, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Above,

    Post, Below, Below, Below,
    None, None, None, Pre,
    Split, None, None, Split,
    Split, None, None, None,

    None, None, None, None,
    None, None, Above, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Tamil
    None, None, Above, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Above, Below, Below, None,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Telugu
    None, Post, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, None, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Above, Post, Post, Post,
    Post, None, Above, Above,
    Split, None, Post, Above,
    Above, Halant, None, None,

    None, None, None, None,
    None, Above, Below, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Kannada
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Split, Post, Post, Post,
    Post, None, Above, Split,
    Split, None, Split, Split,
    Above, Halant, None, None,

    None, None, None, None,
    None, Post, Post, None,
    None, None, None, None,
    None, None, Below, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Malayalam
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Post, None, Below, None,
    None, Post, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Post, Post, Post, Post,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Sinhala
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Post, Post, Above, Above,
    Below, None, Below, None,
    Post, Pre, Split, Pre,
    Split, Split, Split, Post,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None
};

static inline Form form(unsigned short uc) {
    if (uc < 0x900 || uc > 0xdff) {
        if (uc == 0x25cc)
            return Consonant;
        if (uc == 0x200c || uc == 0x200d)
            return Control;
        return Other;
    }
    return (Form)indicForms[uc-0x900];
}

static inline Position indic_position(unsigned short uc) {
    if (uc < 0x900 || uc > 0xdff)
        return None;
    return (Position) indicPosition[uc-0x900];
}


enum IndicScriptProperties {
    HasReph = 0x01,
    HasSplit = 0x02
};

const uchar scriptProperties[10] = {
    // Devanagari,
    HasReph,
    // Bengali,
    HasReph|HasSplit,
    // Gurmukhi,
    0,
    // Gujarati,
    HasReph,
    // Oriya,
    HasReph|HasSplit,
    // Tamil,
    HasSplit,
    // Telugu,
    HasSplit,
    // Kannada,
    HasSplit|HasReph,
    // Malayalam,
    HasSplit,
    // Sinhala,
    HasSplit
};

struct IndicOrdering {
    Form form;
    Position position;
};

static const IndicOrdering devanagari_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { VowelMark, Below },
    { StressMark, Below },
    { Matra, Above },
    { Matra, Post },
    { Consonant, Reph },
    { VowelMark, Above },
    { StressMark, Above },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering bengali_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Reph },
    { VowelMark, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering gurmukhi_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Above },
    { (Form)0, None }
};

static const IndicOrdering tamil_order [] = {
    { Matra, Above },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering telugu_order [] = {
    { Matra, Above },
    { Matra, Below },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering kannada_order [] = {
    { Matra, Above },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { LengthMark, Post },
    { Consonant, Reph },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering malayalam_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Consonant, Reph },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering sinhala_order [] = {
    { Matra, Below },
    { Matra, Above },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering * const indic_order[] = {
    devanagari_order, // Devanagari
    bengali_order, // Bengali
    gurmukhi_order, // Gurmukhi
    devanagari_order, // Gujarati
    bengali_order, // Oriya
    tamil_order, // Tamil
    telugu_order, // Telugu
    kannada_order, // Kannada
    malayalam_order, // Malayalam
    sinhala_order // Sinhala
};



// vowel matras that have to be split into two parts.
static const unsigned short split_matras[]  = {
    //  matra, split1, split2

    // bengalis
    0x9cb, 0x9c7, 0x9be,
    0x9cc, 0x9c7, 0x9d7,
    // oriya
    0xb48, 0xb47, 0xb56,
    0xb4b, 0xb47, 0xb3e,
    0xb4c, 0xb47, 0xb57,
    // tamil
    0xbca, 0xbc6, 0xbbe,
    0xbcb, 0xbc7, 0xbbe,
    0xbcc, 0xbc6, 0xbd7,
    // telugu
    0xc48, 0xc46, 0xc56,
    // kannada
    0xcc0, 0xcbf, 0xcd5,
    0xcc7, 0xcc6, 0xcd5,
    0xcc8, 0xcc6, 0xcd6,
    0xcca, 0xcc6, 0xcc2,
    0xccb, 0xcca, 0xcd5,
    // malayalam
    0xd4a, 0xd46, 0xd3e,
    0xd4b, 0xd47, 0xd3e,
    0xd4c, 0xd46, 0xd57,
    // sinhala
    0xdda, 0xdd9, 0xdca,
    0xddc, 0xdd9, 0xdcf,
    0xddd, 0xddc, 0xdca,
    0xdde, 0xdd9, 0xddf,
    0xffff
};

static inline void splitMatra(unsigned short *reordered, int matra, int &len, int &base)
{
    unsigned short matra_uc = reordered[matra];
    //qDebug("matra=%d, reordered[matra]=%x", matra, reordered[matra]);

    const unsigned short *split = split_matras;
    while (split[0] < matra_uc)
        split += 3;

    assert(*split == matra_uc);
    ++split;

    if (indic_position(*split) == Pre) {
        reordered[matra] = split[1];
        memmove(reordered + 1, reordered, len*sizeof(unsigned short));
        reordered[0] = split[0];
        base++;
    } else {
        memmove(reordered + matra + 1, reordered + matra, (len-matra)*sizeof(unsigned short));
        reordered[matra] = split[0];
        reordered[matra+1] = split[1];
    }
    len++;
}

// #define INDIC_DEBUG
#ifdef INDIC_DEBUG
#define IDEBUG qDebug
#else
#define IDEBUG if(0) qDebug
#endif

static bool indic_shape_syllable(QOpenType *openType, QShaperItem *item, bool invalid)
{
    Q_UNUSED(openType)
    int script = item->script;
    Q_ASSERT(script >= QUnicodeTables::Devanagari && script <= QUnicodeTables::Sinhala);
    const unsigned short script_base = 0x0900 + 0x80*(script-QUnicodeTables::Devanagari);
    const unsigned short ra = script_base + 0x30;
    const unsigned short halant = script_base + 0x4d;
    const unsigned short nukta = script_base + 0x3c;

    int len = item->length;
    IDEBUG(">>>>> indic shape: from=%d, len=%d invalid=%d", item->from, item->length, invalid);

    if (item->num_glyphs < len+4) {
        item->num_glyphs = len+4;
        return false;
    }

    QVarLengthArray<unsigned short> reordered(len+4);
    QVarLengthArray<unsigned char> position(len+4);

    unsigned char properties = scriptProperties[script-QUnicodeTables::Devanagari];

    if (invalid) {
        *reordered.data() = 0x25cc;
        memcpy(reordered.data()+1, item->string->unicode() + item->from, len*sizeof(QChar));
        len++;
    } else {
        memcpy(reordered.data(), item->string->unicode() + item->from, len*sizeof(QChar));
    }
    if (reordered[len-1] == 0x200c) // zero width non joiner
        len--;

    int i;
    int base = 0;
    int reph = -1;

#ifdef INDIC_DEBUG
    IDEBUG("original:");
    for (i = 0; i < len; i++) {
        IDEBUG("    %d: %4x", i, reordered[i]);
    }
#endif

    if (len != 1) {
        unsigned short *uc = reordered.data();
        bool beginsWithRa = false;

        // Rule 1: find base consonant
        //
        // The shaping engine finds the base consonant of the
        // syllable, using the following algorithm: starting from the
        // end of the syllable, move backwards until a consonant is
        // found that does not have a below-base or post-base form
        // (post-base forms have to follow below-base forms), or
        // arrive at the first consonant. The consonant stopped at
        // will be the base.
        //
        //  * If the syllable starts with Ra + H (in a script that has
        //    'Reph'), Ra is excluded from candidates for base
        //    consonants.
        //
        // * In Kannada and Telugu, the base consonant cannot be
        //   farther than 3 consonants from the end of the syllable.
        // #### replace the HasReph property by testing if the feature exists in the font!
        if (form(*uc) == Consonant || (script == QUnicodeTables::Bengali && form(*uc) == IndependentVowel)) {
            beginsWithRa = (properties & HasReph) && ((len > 2) && *uc == ra && *(uc+1) == halant);

            if (beginsWithRa && form(*(uc+2)) == Control)
                beginsWithRa = false;

            base = (beginsWithRa ? 2 : 0);
            IDEBUG("    length = %d, beginsWithRa = %d, base=%d", len, beginsWithRa, base);

            int lastConsonant = 0;
            int matra = -1;
            int skipped = 0;
            Position pos = Post;
            // we remember:
            // * the last consonant since we need it for rule 2
            // * the matras position for rule 3 and 4

            // figure out possible base glyphs
            memset(position.data(), 0, len);
            if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati) {
                bool vattu = false;
                for (i = base; i < len; ++i) {
                    position[i] = form(uc[i]);
                    if (position[i] == Consonant) {
                        lastConsonant = i;
                        vattu = (!vattu && uc[i] == ra);
                        if (vattu) {
                            IDEBUG("excluding vattu glyph at %d from base candidates", i);
                            position[i] = Vattu;
                        }
                    } else if (position[i] == Matra) {
                        matra = i;
                    }
                }
            } else {
                for (i = base; i < len; ++i) {
                    position[i] = form(uc[i]);
                    if (position[i] == Consonant)
                        lastConsonant = i;
                    else if (matra < 0 && position[i] == Matra)
                        matra = i;
                }
            }
            for (i = len-1; i > base; i--) {
                if (position[i] != Consonant
                    && (position[i] != Control || script == QUnicodeTables::Kannada))
                    continue;

                Position charPosition = indic_position(uc[i]);
                if (pos == Post && charPosition == Post) {
                    pos = Below;
                } else if ((pos == Post || pos == Below) && charPosition == Below) {
                    if (script != QUnicodeTables::Kannada && script != QUnicodeTables::Telugu)
                        pos = None;
                    if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati)
                        base = i;
                } else {
                    base = i;
                    break;
                }
                if (skipped == 2 && (script == QUnicodeTables::Kannada || script == QUnicodeTables::Telugu)) {
                    base = i;
                    break;
                }
                ++skipped;
            }

            IDEBUG("    base consonant at %d skipped=%d, lastConsonant=%d", base, skipped, lastConsonant);

            // Rule 2:
            //
            // If the base consonant is not the last one, Uniscribe
            // moves the halant from the base consonant to the last
            // one.
            if (lastConsonant > base && uc[base+1] == halant) {
                IDEBUG("    moving halant from %d to %d!", base+1, lastConsonant);
                for (i = base+1; i < lastConsonant; i++)
                    uc[i] = uc[i+1];
                uc[lastConsonant] = halant;

            }

            // Rule 3:
            //
            // If the syllable starts with Ra + H, Uniscribe moves
            // this combination so that it follows either:

            // * the post-base 'matra' (if any) or the base consonant
            //   (in scripts that show similarity to Devanagari, i.e.,
            //   Devanagari, Gujarati, Bengali)
            // * the base consonant (other scripts)
            // * the end of the syllable (Kannada)

            Position matra_position = None;
            if (matra > 0)
                matra_position = indic_position(uc[matra]);
            IDEBUG("    matra at %d with form %d, base=%d", matra, matra_position, base);

            if (beginsWithRa && base != 0) {
                int toPos = base+1;
                if (toPos < len && uc[toPos] == nukta)
                    toPos++;
                if (toPos < len && uc[toPos] == halant)
                    toPos++;
                if (toPos < len && uc[toPos] == 0x200d)
                    toPos++;
                if (toPos < len-1 && uc[toPos] == ra && uc[toPos+1] == halant)
                    toPos += 2;
                if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati || script == QUnicodeTables::Bengali) {
                    if (matra_position == Post || matra_position == Split) {
                        toPos = matra+1;
                        matra -= 2;
                    }
                } else if (script == QUnicodeTables::Kannada) {
                    toPos = len;
                    matra -= 2;
                }

                IDEBUG("moving leading ra+halant to position %d", toPos);
                for (i = 2; i < toPos; i++)
                    uc[i-2] = uc[i];
                uc[toPos-2] = ra;
                uc[toPos-1] = halant;
                base -= 2;
                if (properties & HasReph)
                    reph = toPos-2;
            }

            // Rule 4:

            // Uniscribe splits two- or three-part matras into their
            // parts. This splitting is a character-to-character
            // operation).
            //
            //      Uniscribe describes some moving operations for these
            //      matras here. For shaping however all pre matras need
            //      to be at the begining of the syllable, so we just move
            //      them there now.
            if (matra_position == Split) {
                splitMatra(uc, matra, len, base);
                // Handle three-part matras (0xccb in Kannada)
                matra_position = indic_position(uc[matra]);
                if (matra_position == Split)
                    splitMatra(uc, matra, len, base);
            } else if (matra_position == Pre) {
                unsigned short m = uc[matra];
                while (matra--)
                    uc[matra+1] = uc[matra];
                uc[0] = m;
                base++;
            }
        }

        // Rule 5:
        //
        // Uniscribe classifies consonants and 'matra' parts as
        // pre-base, above-base (Reph), below-base or post-base. This
        // classification exists on the character code level and is
        // language-dependent, not font-dependent.
        for (i = 0; i < base; ++i)
            position[i] = Pre;
        position[base] = Base;
        for (i = base+1; i < len; ++i) {
            position[i] = indic_position(uc[i]);
            // #### replace by adjusting table
            if (uc[i] == nukta || uc[i] == halant)
                position[i] = Inherit;
        }
        if (reph > 0) {
            // recalculate reph, it might have changed.
            for (i = base+1; i < len; ++i)
                if (uc[i] == ra)
                    reph = i;
            position[reph] = Reph;
            position[reph+1] = Inherit;
        }

        // all reordering happens now to the chars after the base
        int fixed = base+1;
        if (fixed < len && uc[fixed] == nukta)
            fixed++;
        if (fixed < len && uc[fixed] == halant)
            fixed++;
        if (fixed < len && uc[fixed] == 0x200d)
            fixed++;

#ifdef INDIC_DEBUG
        for (i = fixed; i < len; ++i)
            IDEBUG("position[%d] = %d, form=%d", i, position[i], form(uc[i]));
#endif
        // we continuosly position the matras and vowel marks and increase the fixed
        // until we reached the end.
        const IndicOrdering *finalOrder = indic_order[script-QUnicodeTables::Devanagari];

        IDEBUG("    reordering pass:");
        //IDEBUG("        base=%d fixed=%d", base, fixed);
        int toMove = 0;
        while (finalOrder[toMove].form && fixed < len-1) {
            //IDEBUG("        fixed = %d, moving form %d with pos %d", fixed, finalOrder[toMove].form, finalOrder[toMove].position);
            for (i = fixed; i < len; i++) {
                if (form(uc[i]) == finalOrder[toMove].form &&
                     position[i] == finalOrder[toMove].position) {
                    // need to move this glyph
                    int to = fixed;
                    if (i < len-1 && position[i+1] == Inherit) {
                        IDEBUG("         moving two chars from %d to %d", i, to);
                        unsigned short ch = uc[i];
                        unsigned short ch2 = uc[i+1];
                        unsigned char pos = position[i];
                        for (int j = i+1; j > to+1; j--) {
                            uc[j] = uc[j-2];
                            position[j] = uc[j-2];
                        }
                        uc[to] = ch;
                        uc[to+1] = ch2;
                        position[to] = pos;
                        position[to+1] = pos;
                        fixed += 2;
                    } else {
                        IDEBUG("         moving one char from %d to %d", i, to);
                        unsigned short ch = uc[i];
                        unsigned char pos = position[i];
                        for (int j = i; j > to; j--) {
                            uc[j] = uc[j-1];
                            position[j] = position[j-1];
                        }
                        uc[to] = ch;
                        position[to] = pos;
                        fixed++;
                    }
                }
            }
            toMove++;
        }

    }

    int nglyphs = item->num_glyphs;
    if (!item->font->stringToCMap((const QChar *)reordered.data(), len, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;

    if (reph > 0) {
        // recalculate reph, it might have changed.
        for (i = base+1; i < len; ++i)
            if (reordered[i] == ra)
                reph = i;
    }

    IDEBUG("  base=%d, reph=%d", base, reph);
    IDEBUG("reordered:");
    for (i = 0; i < len; i++) {
        item->glyphs[i].attributes.mark = false;
        item->glyphs[i].attributes.clusterStart = false;
        item->glyphs[i].attributes.justification = 0;
        item->glyphs[i].attributes.zeroWidth = false;
        IDEBUG("    %d: %4x", i, reordered[i]);
    }
    item->glyphs[0].attributes.clusterStart = true;


    // now we have the syllable in the right order, and can start running it through open type.

    bool control = false;
    for (i = 0; i < len; ++i)
        control |= (form(reordered[i]) == Control);

#ifdef QT_HAVE_FREETYPE
    if (openType) {

        // we need to keep track of where the base glyph is for some
        // scripts and abuse the logcluster feature for this.  This
        // also means we have to correct the logCluster output from
        // the open type engine manually afterwards.  for indic this
        // is rather simple, as all chars just point to the first
        // glyph in the syllable.
        QVarLengthArray<unsigned short> logClusters(len);
        QVarLengthArray<bool> where(len);
        memset(where.data(), 0, len*sizeof(bool));
        for (i = 0; i < len; ++i)
            logClusters[i] = i;

        item->log_clusters = logClusters.data();
        openType->init(item);

        // substitutions

        openType->applyGSUBFeature(FT_MAKE_TAG('c', 'c', 'm', 'p'));

 	where[0] = (item->from == 0
                    || !(item->string->unicode()[item->from-1].isLetter() ||  item->string->unicode()[item->from-1].isMark()));
        openType->applyGSUBFeature(FT_MAKE_TAG('i', 'n', 'i', 't'), where.data());
        openType->applyGSUBFeature(FT_MAKE_TAG('n', 'u', 'k', 't'));

        for (i = 0; i <= base; ++i)
            where[i] = true;
        openType->applyGSUBFeature(FT_MAKE_TAG('a', 'k', 'h', 'n'), where.data());

        memset(where.data(), 0, len*sizeof(bool));
        if (reph >= 0) {
            where[reph] = where[reph+1] = true;
            openType->applyGSUBFeature(FT_MAKE_TAG('r', 'p', 'h', 'f'), where.data());
            where[reph] = where[reph+1] = false;
        }

        for (i = base+1; i < len; ++i)
            where[i] = true;
        if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati) {
            // vattu glyphs need this aswell
            bool vattu = false;
            for (i = base-2; i > 1; --i) {
                if (form(reordered[i]) == Consonant) {
                    vattu = (!vattu && reordered[i] == ra);
                    if (vattu) {
                        IDEBUG("forming vattu ligature at %d", i);
                        where[i] = where[i+1] = true;
                    }
                }
            }
        }
        openType->applyGSUBFeature(FT_MAKE_TAG('b', 'l', 'w', 'f'), where.data());
        memset(where.data(), 0, len*sizeof(bool));
        for (i = 0; i < base; ++i)
            where[i] = true;
        if (control) {
            for (i = 2; i < len; ++i) {
                if (reordered[i] == 0x200d /* ZWJ */) {
                    where[i-1] = true;
                    where[i-2] = true;
                } else if (reordered[i] == 0x200c /* ZWNJ */) {
                    where[i-1] = false;
                    where[i-2] = false;
                }
            }
        }
        openType->applyGSUBFeature(FT_MAKE_TAG('h', 'a', 'l', 'f'), where.data());
        memset(where.data(), 0, len*sizeof(bool));
        for (i = base+1; i < len; ++i)
            where[i] = true;
        openType->applyGSUBFeature(FT_MAKE_TAG('p', 's', 't', 'f'), where.data());
        openType->applyGSUBFeature(FT_MAKE_TAG('v', 'a', 't', 'u'));

        // Conjunkts and typographical forms
        openType->applyGSUBFeature(FT_MAKE_TAG('p', 'r', 'e', 's'));
        openType->applyGSUBFeature(FT_MAKE_TAG('b', 'l', 'w', 's'));
        openType->applyGSUBFeature(FT_MAKE_TAG('a', 'b', 'v', 's'));

        if (reordered[len-1] != halant || base != len-2) {
            where[base] = true;
            openType->applyGSUBFeature(FT_MAKE_TAG('p', 's', 't', 's'), where.data());
        }

        // halant forms
        if (base < len-1 && reordered[base+1] == halant || script == QUnicodeTables::Malayalam) {
            // The hlnt feature needs to get always applied for malayalam according to the MS docs.
//             memset(where, script == QUnicodeTables::Malayalam ? 1 : 0, len*sizeof(bool));
//             where[base] = where[base+1] = true;
            openType->applyGSUBFeature(FT_MAKE_TAG('h', 'a', 'l', 'n'));
        }

        int newLen;
        const int *char_map = openType->mapping(newLen);

        // move the left matra back to it's correct position in malayalam and tamil
        if ((script == QUnicodeTables::Malayalam || script == QUnicodeTables::Tamil) && (form(reordered[0]) == Matra)) {
            // need to find the base in the shaped string and move the matra there
            int basePos = 0;
            while (basePos < newLen && char_map[basePos] <= base)
                basePos++;
            --basePos;
            if (basePos < newLen && basePos > 1) {
                IDEBUG("moving prebase matra to position %d in syllable newlen=%d", basePos, newLen);
                unsigned short *g = openType->glyphs();
                unsigned short m = g[0];
                --basePos;
                for (i = 0; i < basePos; ++i)
                    g[i] = g[i+1];
                g[basePos] = m;
            }
        }

        openType->applyGPOSFeatures();

        item->num_glyphs = nglyphs;
        if (!openType->appendTo(item, false))
            return false;

        if (control) {
            IDEBUG("found a control char in the syllable");
            int i = 0, j = 0;
            while (i < item->num_glyphs) {
                if (form(reordered[char_map[i]]) == Control) {
                    ++i;
                    if (i >= item->num_glyphs)
                        break;
                }
                item->glyphs[j] = item->glyphs[i];
                ++i;
                ++j;
            }
            item->num_glyphs = j;
        }

    }
#endif

    IDEBUG("<<<<<<");
    return true;
}


/* syllables are of the form:

   (Consonant Nukta? Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Nukta? Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   We return syllable boundaries on invalid combinations aswell
*/
static int indic_nextSyllableBoundary(int script, const QString &s, int start, int end, bool *invalid)
{
    *invalid = false;
    IDEBUG("indic_nextSyllableBoundary: start=%d, end=%d", start, end);
    const QChar *uc = s.unicode()+start;

    int pos = 0;
    Form state = form(uc[pos].unicode());
    IDEBUG("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode());
    pos++;

    if (state != Consonant && state != IndependentVowel) {
        if (state != Other && state != Control)
            *invalid = true;
        goto finish;
    }

    while (pos < end - start) {
        Form newState = form(uc[pos].unicode());
        IDEBUG("state[%d]=%d (uc=%4x)", pos, newState, uc[pos].unicode());
        switch(newState) {
        case Control:
            newState = state;
 	    if (state == Halant && uc[pos].unicode() == 0x200d /* ZWJ */)
  		break;
            // the control character should be the last char in the item
            ++pos;
            goto finish;
        case Consonant:
	    if (state == Halant && (script != QUnicodeTables::Sinhala || uc[pos-1].unicode() == 0x200d /* ZWJ */))
                break;
            goto finish;
        case Halant:
            if (state == Nukta || state == Consonant)
                break;
            // Bengali has a special exception allowing the combination Vowel_A/E + Halant + Ya
            if (script == QUnicodeTables::Bengali && pos == 1 &&
                 (uc[0].unicode() == 0x0985 || uc[0].unicode() == 0x098f))
                break;
            goto finish;
        case Nukta:
            if (state == Consonant)
                break;
            goto finish;
        case StressMark:
            if (state == VowelMark)
                break;
            // fall through
        case VowelMark:
            if (state == Matra || state == IndependentVowel)
                break;
            // fall through
        case Matra:
            if (state == Consonant || state == Nukta)
                break;
            // ### not sure if this is correct. If it is, does it apply only to Bengali or should
            // it work for all Indic languages?
            // the combination Independent_A + Vowel Sign AA is allowed.
            if (script == QUnicodeTables::Bengali && uc[pos].unicode() == 0x9be && uc[pos-1].unicode() == 0x985)
                break;
            if (script == QUnicodeTables::Tamil && state == Matra) {
                if (uc[pos-1].unicode() == 0x0bc6 &&
                     (uc[pos].unicode() == 0xbbe || uc[pos].unicode() == 0xbd7))
                    break;
                if (uc[pos-1].unicode() == 0x0bc7 && uc[pos].unicode() == 0xbbe)
                    break;
            }
            goto finish;

        case LengthMark:
        case IndependentVowel:
        case Invalid:
        case Other:
            goto finish;
        }
        state = newState;
        pos++;
    }
 finish:
    return pos+start;
}

static bool indic_shape(QShaperItem *item)
{
    Q_ASSERT(item->script >= QUnicodeTables::Devanagari && item->script <= QUnicodeTables::Sinhala);

#ifdef QT_HAVE_FREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && !openType->supportsScript(item->script))
        openType = 0;
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    IDEBUG("indic_shape: from %d length %d", item->from, item->length);
    while (sstart < end) {
        bool invalid;
        int send = indic_nextSyllableBoundary(item->script, *item->string, sstart, end, &invalid);
        IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "true" : "false");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!indic_shape_syllable(openType, &syllable, invalid)) {
            IDEBUG("syllable shaping failed, syllable requests %d glyphs", syllable.num_glyphs);
            item->num_glyphs += syllable.num_glyphs;
            return false;
        }
        // fix logcluster array
        IDEBUG("syllable:");
        for (int i = first_glyph; i < first_glyph + syllable.num_glyphs; ++i)
            IDEBUG("        %d -> glyph %x", i, item->glyphs[i].glyph);
        IDEBUG("    logclusters:");
        for (int i = sstart; i < send; ++i) {
            IDEBUG("        %d -> glyph %d", i, first_glyph);
            logClusters[i-item->from] = first_glyph;
        }
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return true;
}


static void indic_attributes(int script, const QString &text, int from, int len, QCharAttributes *attributes)
{
    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        bool invalid;
        int boundary = indic_nextSyllableBoundary(script, text, from+i, end, &invalid) - from;

        attributes[i].whiteSpace = ::isSpace(*uc) && (uc->unicode() != 0xa0);
        attributes[i].softBreak = false;
        attributes[i].charStop = true;
        attributes[i].wordStop = false;
        attributes[i].invalid = invalid;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].whiteSpace = ::isSpace(*uc) && (uc->unicode() != 0xa0);
            attributes[i].softBreak = false;
            attributes[i].charStop = false;
            attributes[i].wordStop = false;
            attributes[i].invalid = invalid;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }


}


// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Thai and Lao
//
// --------------------------------------------------------------------------------------------------------------------------------------------

#include <qtextcodec.h>
#include <qlibrary.h>


static void thaiWordBreaks(const QChar *string, const int len, QCharAttributes *attributes)
{
    typedef int (*th_brk_def)(const char*, int[], int);
    static QTextCodec *thaiCodec = QTextCodec::codecForMib(2259);
    static th_brk_def th_brk = 0;

    /* load libthai dynamically */
    if (!th_brk && thaiCodec) {
        th_brk = (th_brk_def)QLibrary::resolve("thai", "th_brk");
        if (!th_brk)
            thaiCodec = 0;
    }

    if (!th_brk)
        return;

    QByteArray cstr = thaiCodec->fromUnicode(QString(string, len));

    int brp[128];
    int *break_positions = brp;
    int numbreaks = th_brk(cstr.constData(), break_positions, 128);
    if (numbreaks > 128) {
        break_positions = new int[numbreaks];
        numbreaks = th_brk(cstr.data(),break_positions, numbreaks);
    }

    attributes[0].softBreak = true;
    for (int i = 1; i < len; ++i)
        attributes[i].softBreak = false;

    for (int i = 0; i < numbreaks; ++i)
        attributes[break_positions[i]].softBreak = true;

    if (break_positions != brp)
        delete [] break_positions;
}


static void thai_attributes( int script, const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    attributes += from;

    QCharAttributes *a = attributes;
    for ( int i = 0; i < len; i++ ) {
	QChar::Category cat = ::category( *uc );
	a->whiteSpace = (cat == QChar::Separator_Space) && (uc->unicode() != 0xa0);
	a->charStop = (cat != QChar::Mark_NonSpacing);
        // if we don't know any better, every charstop is a possible line break.
	a->softBreak = a->charStop;
	a->wordStop = false;
	a->invalid = false;
	++uc;
	++a;
    }

    if (script == QUnicodeTables::Thai)
        thaiWordBreaks(text.unicode() + from, len, attributes);
}



// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Tibetan
//
// --------------------------------------------------------------------------------------------------------------------------------------------

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


static inline TibetanForm tibetan_form(const QChar &c)
{
    return (TibetanForm)tibetanForm[c.unicode() - 0x0f40];
}

static bool tibetan_shape_syllable(QOpenType *openType, QShaperItem *item, bool invalid)
{
    Q_UNUSED(openType)
    int len = item->length;

    if (item->num_glyphs < item->length + 4) {
        item->num_glyphs = item->length + 4;
        return false;
    }

    int i;
    QVarLengthArray<unsigned short> reordered(len+4);

    const QChar *str = item->string->unicode() + item->from;
    if (invalid) {
        *reordered.data() = 0x25cc;
        memcpy(reordered.data()+1, str, len*sizeof(QChar));
        len++;
        str = (QChar *)reordered.data();
    }

    int nglyphs = item->num_glyphs;
    if (!item->font->stringToCMap(str, len, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;

    for (i = 0; i < item->length; i++) {
        item->glyphs[i].attributes.mark = false;
        item->glyphs[i].attributes.clusterStart = false;
        item->glyphs[i].attributes.justification = 0;
        item->glyphs[i].attributes.zeroWidth = false;
        IDEBUG("    %d: %4x", i, str[i].unicode());
    }
    item->glyphs[0].attributes.clusterStart = true;

    // now we have the syllable in the right order, and can start running it through open type.

#ifdef QT_HAVE_FREETYPE
    if (openType) {
        // we need to keep track of where the base glyph is for some scripts and abuse the logcluster feature for this.
        // This also means we have to correct the logCluster output from the open type engine manually afterwards.
        // for indic this is rather simple, as all chars just point to the first glyph in the syllable.
        QVarLengthArray<unsigned short> logClusters(len);
        for (i = 0; i < len; ++i)
            logClusters[i] = i;
        item->log_clusters = logClusters.data();

        openType->init(item);

        // substitutions
        openType->applyGSUBFeature(FT_MAKE_TAG('c', 'c', 'm', 'p'));
        openType->applyGSUBFeature(FT_MAKE_TAG('a', 'b', 'v', 's'));
        openType->applyGSUBFeature(FT_MAKE_TAG('b', 'l', 'w', 's'));
        openType->applyGPOSFeatures();

        item->num_glyphs = nglyphs;
        return openType->appendTo(item, false);
    }
#endif

    return true;
}


static int tibetan_nextSyllableBoundary(const QString &s, int start, int end, bool *invalid)
{
    const QChar *uc = s.unicode() + start;

    int pos = 0;
    TibetanForm state = tibetan_form(*uc);

//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode());
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

static bool tibetan_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Tibetan);

#ifdef QT_HAVE_FREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && !openType->supportsScript(item->script))
        openType = 0;
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    while (sstart < end) {
        bool invalid;
        int send = tibetan_nextSyllableBoundary(*(item->string), sstart, end, &invalid);
        IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "true" : "false");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!tibetan_shape_syllable(openType, &syllable, invalid)) {
            item->num_glyphs += syllable.num_glyphs;
            return false;
        }
        // fix logcluster array
        for (int i = sstart; i < send; ++i)
            logClusters[i-item->from] = first_glyph;
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return true;
}

static void tibetan_attributes(int script, const QString &text, int from, int len, QCharAttributes *attributes)
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        bool invalid;
        int boundary = tibetan_nextSyllableBoundary(text, from+i, end, &invalid) - from;

        attributes[i].whiteSpace = ::isSpace(*uc);
        attributes[i].softBreak = false;
        attributes[i].charStop = true;
        attributes[i].wordStop = false;
        attributes[i].invalid = invalid;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].whiteSpace = ::isSpace(*uc);
            attributes[i].softBreak = false;
            attributes[i].charStop = false;
            attributes[i].wordStop = false;
            attributes[i].invalid = invalid;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Khmer
//
// --------------------------------------------------------------------------------------------------------------------------------------------

enum KhmerForm {
    Khmer_Cons, // Consonant
    Khmer_IndV, // Independent Vowel
    Khmer_Coeng, // COENG
    Khmer_PreV, // Pre dependent Vowel
    Khmer_BlwV, // Below dependent Vowel
    Khmer_Shift, // Regshift
    Khmer_AbvV, // Above dependent Vowel
    Khmer_AbvS, // Above Sign
    Khmer_PstV, // Post dependent Vowel
    Khmer_PstS, // Post sign
    Khmer_Other
};


static const unsigned char khmerForm[0x54] = {
    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,
    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,
    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,
    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,

    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,
    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,
    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,
    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_Cons,

    Khmer_Cons, Khmer_Cons, Khmer_Cons, Khmer_IndV,
    Khmer_IndV, Khmer_IndV, Khmer_IndV, Khmer_IndV,
    Khmer_IndV, Khmer_IndV, Khmer_IndV, Khmer_IndV,
    Khmer_IndV, Khmer_IndV, Khmer_IndV, Khmer_IndV,

    Khmer_IndV, Khmer_IndV, Khmer_IndV, Khmer_IndV,
    // #### the following two might not be independent vowels.
    Khmer_IndV, Khmer_IndV, Khmer_PstV, Khmer_AbvV,
    Khmer_AbvV, Khmer_AbvV, Khmer_AbvV, Khmer_BlwV,
    Khmer_BlwV, Khmer_BlwV, Khmer_AbvV, Khmer_PstV,

    Khmer_PstV, Khmer_PreV, Khmer_PreV, Khmer_PreV,
    Khmer_PstV, Khmer_PstV, Khmer_AbvS, Khmer_PstS,
    Khmer_PstS, Khmer_Shift, Khmer_Shift, Khmer_AbvS,
    Khmer_AbvS, Khmer_AbvS, Khmer_AbvS, Khmer_AbvS,

    Khmer_AbvS, Khmer_AbvS, Khmer_Coeng, Khmer_AbvS,
};

// see Uniscribe specs for details. A lot of the needed information can be found
// in the 3.2 annex, see http://www.unicode.org/reports/tr28/
//
// syllable analysis needs to respect these: type one has to come before type 2
// which has to come before type 3. type 2 can appear only once.
//
// We use 0xff to encode that this is a split vowel.
static const unsigned char khmerSubscriptType[0x54] = {
    1, 1, 1, 3,
    1, 1, 1, 1,
    3, 1, 1, 1,
    1, 3, 1, 1,

    1, 1, 1, 1,
    3, 1, 1, 1,
    1, 3, 2, 1,
    1, 1, 3, 3,

    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,

    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 0xff, 0xff,

    0xff, 1, 1, 1,
    0xff, 0xff, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,

    1, 1, 1, 1
};

static inline KhmerForm khmer_form(const QChar &uc) {
    if (uc.unicode() < 0x1780 || uc.unicode() > 0x17d3)
        return Khmer_Other;
    return (KhmerForm) khmerForm[uc.unicode()-0x1780];
}

static inline unsigned char khmer_subscript_type(const QChar &uc) {
    return khmerSubscriptType[uc.unicode()-0x1780];
}

// #define KHMER_DEBUG
#ifdef KHMER_DEBUG
#define KHDEBUG qDebug
#else
#define KHDEBUG if(0) qDebug
#endif

// Khmer syllables are of the form:
//     Cons +  {COENG + (Cons | IndV)} + [PreV | BlwV] + [Shift] + [AbvV] + {AbvS} + [PstV] + [PstS]
//     IndV
//    Number
//
// {...} == 0-2 occurrences

// According to the Unicode 3.0 standard, the syllable is as follows:
//     Cons (Coeng (Cons|IndV))* Shift? DepV?

// The above definitions disagree to a certain degree. Most probably the form mentioned by Uniscribe is the shaped form.

static int khmer_nextSyllableBoundary(const QString &s, int start, int end, bool *invalid)
{
    const QChar *uc = s.unicode() + start;

    int pos = 0;
    unsigned int coengCount = 0;
    unsigned int abvSCount = 0;
    unsigned int subscriptType = 1;

    KhmerForm state = khmer_form(*uc);

    KHDEBUG("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode());
    pos++;

    if (state != Khmer_Cons && state != Khmer_IndV) {
         if (state != Khmer_Other)
            *invalid = true;
        goto finish;
    }

    while (pos < end - start) {
        KhmerForm newState = khmer_form(uc[pos]);
        switch(newState) {
        case Khmer_Coeng:
            if (coengCount > 1 || (state != Khmer_Cons && state != Khmer_IndV))
                goto finish;
            ++coengCount;
            break;
        case Khmer_Cons:
        case Khmer_IndV: {
            unsigned int t = khmer_subscript_type(uc[pos]);
            if (state != Khmer_Coeng || t < subscriptType)
                goto finish;
            subscriptType = t;
            // only one consonant of type 2 can be present
            if (t == 2)
                t = 3;
            break;
            }
        case Khmer_PstS:
            if (state == Khmer_PstV)
                break;
        case Khmer_PstV:
        case Khmer_AbvS:
            if (newState == Khmer_AbvS) {
                if (abvSCount > 1)
                    goto finish;
                ++abvSCount;
            }
            if (state == Khmer_AbvS || state == Khmer_AbvV)
                break;
            // fall through
        case Khmer_AbvV:
            if (state == Khmer_Shift)
                break;
            // fall through
        case Khmer_Shift:
            if (state == Khmer_PreV || state == Khmer_BlwV)
                break;
            // fall through
        case Khmer_PreV:
        case Khmer_BlwV:
            if (state != Khmer_Cons && state != Khmer_IndV)
                goto finish;
            break;
        case Khmer_Other:
            goto finish;
        }
        state = newState;
        pos++;
    }

finish:
    // makse sure we don't have an invalid Coeng at the end
    if (state == Khmer_Coeng && pos > 1)
        --pos;

    *invalid = false;
    return start+pos;
}


static bool khmer_shape_syllable(QOpenType *openType, QShaperItem *item, bool invalid)
{
    Q_UNUSED(openType)
    enum {
        Coeng = 0x17d2,
        VowelSignE = 0x17c1
    };

    // according to the specs this is the max length one can get
    // ### the real value should be smaller
    Q_ASSERT(item->length < 13);

    KHDEBUG("syllable from %d len %d, str='%s'", item->from, item->length,
            item->string->mid(item->from,item->length).toUtf8().constData());
    int len = item->length;

    int i;
    unsigned short reordered[16];
    unsigned char properties[16];
    enum {
        AboveForm = 0x01,
        PreForm = 0x02,
        PostForm = 0x04,
        BelowForm = 0x08
    };
    memset(properties, 0, 16*sizeof(unsigned char));

    if (invalid) {
        *reordered = 0x25cc;
        memcpy(reordered+1, item->string->unicode() + item->from, len*sizeof(unsigned short));
        len++;
    } else {
        memcpy(reordered, item->string->unicode() + item->from, len*sizeof(unsigned short));
    }

#ifdef KHMER_DEBUG
    qDebug("original:");
    for (i = 0; i < len; i++) {
        qDebug("    %d: %4x", i, reordered[i]);
    }
#endif

    if (len > 1) {
        // rule 2, move COENG+Ro to front
        for (i = 1; i < 4 && i < len-1; i += 2) {
            if (khmer_form(reordered[i]) != Khmer_Coeng)
                break;
            int t = khmer_subscript_type(reordered[i + 1]);
            if (t == 1) {
                properties[i] = properties[i+1] = BelowForm;
            } else if (t == 2) {
                // move COENG + RO to beginning of syllable
                unsigned short uc = reordered[i + 1];
                for (int j = i + 1; j > 1; --j) {
                    reordered[j] = reordered[j - 2];
                    properties[j] = properties[j - 2];
                }
                reordered[0] = Coeng;
                reordered[1] = uc;
                properties[0] = properties[1] = PreForm;
            } else if (t == 3) {
                properties[i] = properties[i+1] = PostForm;
            }
        }

        // Rule 3
        for (i = 1; i < len-1; ++i) {
            if (khmer_form(reordered[i]) == Khmer_Shift &&
                khmer_form(reordered[i+1]) == Khmer_AbvV) {
                properties[i] = BelowForm;
                break;
            }
        }

        // Rule 4 and 5
        // The Uniscribe docs state that the feature to apply should be Abvf even for post
        // vowels. This is clearly incorrect (comparing with how fonts are build up and how
        // Uniscribe behaves).
        for (i = 1; i < len; ++i) {
            if (khmer_subscript_type(reordered[i]) == 0xff) {
                KHDEBUG("split vowel at %d", i);
                properties[i] = (khmer_form(reordered[i]) == Khmer_AbvV) ? AboveForm : PostForm;
                memmove(reordered+1, reordered, len*sizeof(unsigned short));
                memmove(properties+1, properties, len*sizeof(unsigned char));
                reordered[0] = VowelSignE;
                properties[0] = PreForm;
                ++len;
                ++i;
            }
        }

        // rule not stated in the MS docs about Khmer, but it's logical to do this (in accordance to
        // all indic scripts) and Uniscribe seems to work the same way:
        // Move Pre Vowels to the beginning of the syllable
        for (i = len-1; i > 0; --i) {
            if (khmer_form(reordered[i]) == Khmer_PreV) {
                KHDEBUG("moving Pre Vowel at %d to start", i);
                unsigned short pre = reordered[i];
                memmove(reordered+1, reordered, i*sizeof(unsigned short));
                memmove(properties+1, properties, i*sizeof(unsigned char));
                reordered[0] = pre;
                properties[0] = PreForm;
                break;
            }
        }

    }

    KHDEBUG("after shaping: len=%d", len);
    int nglyphs = item->num_glyphs;
    if (!item->font->stringToCMap((QChar *)reordered, len, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;
    for (i = 0; i < len; i++) {
        item->glyphs[i].attributes.mark = false;
        item->glyphs[i].attributes.clusterStart = false;
        item->glyphs[i].attributes.justification = 0;
        item->glyphs[i].attributes.zeroWidth = false;
        KHDEBUG("    %d: %4x property=%x", i, reordered[i], properties[i]);
    }
    item->glyphs[0].attributes.clusterStart = true;

    // now we have the syllable in the right order, and can start running it through open type.

#ifdef QT_HAVE_FREETYPE
    int j;
    if (openType) {
        unsigned short logClusters[16];
        for (i = 0; i < len; ++i)
            logClusters[i] = i;
        item->log_clusters = logClusters;

        openType->init(item);

        bool where[16];

        // substitutions
        const struct {
            int feature; int form;
        } features[] = {
            { FT_MAKE_TAG('p', 'r', 'e', 'f'), PreForm },
            { FT_MAKE_TAG('b', 'l', 'w', 'f'), BelowForm },
            { FT_MAKE_TAG('a', 'b', 'v', 'f'), AboveForm },
            { FT_MAKE_TAG('p', 's', 't', 'f'), PostForm }
        };
        for (j = 0; j < 4; ++j) {
            for (i = 0; i < len; ++i)
                where[i] = (properties[i] & features[j].form);
            openType->applyGSUBFeature(features[j].feature, where);
        }

        const int features2 [] = {
            FT_MAKE_TAG('p', 'r', 'e', 's'),
            FT_MAKE_TAG('b', 'l', 'w', 's'),
            FT_MAKE_TAG('a', 'b', 'v', 's'),
            FT_MAKE_TAG('p', 's', 't', 's'),
            FT_MAKE_TAG('c', 'l', 'i', 'g')
        };
        for (i = 0; i < 5; ++i)
            openType->applyGSUBFeature(features2[i]);

        openType->applyGPOSFeatures();

        item->num_glyphs = nglyphs;
        return openType->appendTo(item, false);
    }
#endif

    return true;
}

static bool khmer_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Khmer);

#ifdef QT_HAVE_FREETYPE
    QOpenType *openType = item->font->openType();
    if (openType && !openType->supportsScript(item->script))
        openType = 0;
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    while (sstart < end) {
        bool invalid;
        int send = khmer_nextSyllableBoundary(*(item->string), sstart, end, &invalid);
        IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "true" : "false");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!khmer_shape_syllable(openType, &syllable, invalid)) {
            item->num_glyphs += syllable.num_glyphs;
            return false;
        }
        // fix logcluster array
        for (int i = sstart; i < send; ++i)
            logClusters[i-item->from] = first_glyph;
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return true;
}

static void khmer_attributes(int script, const QString &text, int from, int len, QCharAttributes *attributes)
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        bool invalid;
        int boundary = khmer_nextSyllableBoundary(text, from+i, end, &invalid) - from;

        attributes[i].whiteSpace = ::isSpace(*uc);
        attributes[i].softBreak = false;
        attributes[i].charStop = true;
        attributes[i].wordStop = false;
        attributes[i].invalid = invalid;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].whiteSpace = ::isSpace(*uc);
            attributes[i].softBreak = false;
            attributes[i].charStop = false;
            attributes[i].wordStop = false;
            attributes[i].invalid = invalid;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Hangul
//
// --------------------------------------------------------------------------------------------------------------------------------------------

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
// syllables fill characers L_f and V_f can be inserted.

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

static int hangul_nextSyllableBoundary(const QString &s, int start, int end)
{
    const QChar *uc = s.unicode() + start;

    HangulType state = hangul_type(uc->unicode());
    int pos = 1;

    while (pos < end - start) {
        HangulType newState = hangul_type(uc[pos].unicode());
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

static bool hangul_shape_syllable(QOpenType *openType, QShaperItem *item)
{
    Q_UNUSED(openType)
    const QChar *ch = item->string->unicode() + item->from;

    int i;
    unsigned short composed = 0;
    // see if we can compose the syllable into a modern hangul
    if (item->length == 2) {
        int LIndex = ch[0].unicode() - Hangul_LBase;
        int VIndex = ch[1].unicode() - Hangul_VBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + Hangul_SBase;
    } else if (item->length == 3) {
        int LIndex = ch[0].unicode() - Hangul_LBase;
        int VIndex = ch[1].unicode() - Hangul_VBase;
        int TIndex = ch[2].unicode() - Hangul_TBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount &&
            TIndex >= 0 && TIndex < Hangul_TCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + TIndex + Hangul_SBase;
    }


    int len = item->length;
    QChar c(composed);

    // ### icc says 'chars' is unused
    // const QChar *chars = ch;

    // if we have a modern hangul use the composed form
    if (composed) {
        // chars = &c;
        len = 1;
    }

    int nglyphs = item->num_glyphs;
    if (!item->font->stringToCMap(ch, len, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;
    for (i = 0; i < len; i++) {
        item->glyphs[i].attributes.mark = false;
        item->glyphs[i].attributes.clusterStart = false;
        item->glyphs[i].attributes.justification = 0;
        item->glyphs[i].attributes.zeroWidth = false;
        IDEBUG("    %d: %4x", i, ch[i].unicode());
    }
    item->glyphs[0].attributes.clusterStart = true;

#ifdef QT_HAVE_FREETYPE
    if (openType && !composed) {

        QVarLengthArray<unsigned short> logClusters(len);
        for (i = 0; i < len; ++i)
            logClusters[i] = i;
        item->log_clusters = logClusters.data();

        openType->init(item);

        const int features[] = {
            FT_MAKE_TAG('c', 'c', 'm', 'p'),
            FT_MAKE_TAG('l', 'j', 'm', 'o'),
            FT_MAKE_TAG('j', 'j', 'm', 'o'),
            FT_MAKE_TAG('t', 'j', 'm', 'o'),
            0
        };
        const int *f = features;
        while (*f)
            openType->applyGSUBFeature(*f++);
        openType->applyGPOSFeatures();

        item->num_glyphs = nglyphs;
        return openType->appendTo(item, false);

    }
#endif

    return true;
}

static bool hangul_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Hangul);

    const QChar *uc = item->string->unicode() + item->from;

    bool allPrecomposed = true;
    for (int i = 0; i < item->length; ++i) {
        if (!hangul_isPrecomposed(uc[i].unicode())) {
            allPrecomposed = false;
            break;
        }
    }

    if (!allPrecomposed) {
#ifdef QT_HAVE_FREETYPE
        QOpenType *openType = item->font->openType();
        if (openType && !openType->supportsScript(item->script))
            openType = 0;
#else
        QOpenType *openType = 0;
#endif

        unsigned short *logClusters = item->log_clusters;

        QShaperItem syllable = *item;
        int first_glyph = 0;

        int sstart = item->from;
        int end = sstart + item->length;
        while (sstart < end) {
            int send = hangul_nextSyllableBoundary(*(item->string), sstart, end);

            syllable.from = sstart;
            syllable.length = send-sstart;
            syllable.glyphs = item->glyphs + first_glyph;
            syllable.num_glyphs = item->num_glyphs - first_glyph;
            if (!hangul_shape_syllable(openType, &syllable)) {
                item->num_glyphs += syllable.num_glyphs;
                return false;
            }
            // fix logcluster array
            for (int i = sstart; i < send; ++i)
                logClusters[i-item->from] = first_glyph;
            sstart = send;
            first_glyph += syllable.num_glyphs;
        }
        item->num_glyphs = first_glyph;
        return true;
    }

    return basic_shape(item);
}

static void hangul_attributes(int script, const QString &text, int from, int len, QCharAttributes *attributes)
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        int boundary = hangul_nextSyllableBoundary(text, from+i, end) - from;

        attributes[i].whiteSpace = false;
        attributes[i].softBreak = true;
        attributes[i].charStop = true;
        attributes[i].wordStop = false;
        attributes[i].invalid = false;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].whiteSpace = false;
            attributes[i].softBreak = true;
            attributes[i].charStop = false;
            attributes[i].wordStop = false;
            attributes[i].invalid = false;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }
}

// -----------------------------------------------------------------------------------------------
//
// The script engine jump table
//
// -----------------------------------------------------------------------------------------------

const q_scriptEngine qt_scriptEngines[] = {
    // Common
    { basic_shape, basic_attributes },
    // Hebrew
    { hebrew_shape, basic_attributes },
    // Arabic
    { arabic_shape, arabic_attributes },
    // Syriac
    { syriac_shape, arabic_attributes },
    // Thaana
    { thaana_shape, basic_attributes },
    // Devanagari
    { indic_shape, indic_attributes },
    // Bengali
    { indic_shape, indic_attributes },
    // Gurmukhi
    { indic_shape, indic_attributes },
    // Gujarati
    { indic_shape, indic_attributes },
    // Oriya
    { indic_shape, indic_attributes },
    // Tamil
    { indic_shape, indic_attributes },
    // Telugu
    { indic_shape, indic_attributes },
    // Kannada
    { indic_shape, indic_attributes },
    // Malayalam
    { indic_shape, indic_attributes },
    // Sinhala
    { indic_shape, indic_attributes },
    // Thai
    { basic_shape, thai_attributes },
    // Lao
    { basic_shape, thai_attributes },
    // Tibetan
    { tibetan_shape, tibetan_attributes },
    // Myanmar
    { basic_shape, basic_attributes },
    // Hangul
    { hangul_shape, hangul_attributes },
    // Khmer
    { khmer_shape, khmer_attributes }

#if 0
    // ### What about this one?
    // Unicode
    { unicode_shape, basic_attributes }
#endif

};
