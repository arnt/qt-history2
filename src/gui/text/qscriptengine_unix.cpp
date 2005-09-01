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
static bool syriac_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Syriac);

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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
            if (lastConsonant > base && uc[base+1] == halant
                && (script != QUnicodeTables::Telugu || lastConsonant == len - 1 || uc[lastConsonant+1] != halant)) {
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

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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
#ifndef QT_NO_TEXTCODEC
    typedef int (*th_brk_def)(const char*, int[], int);
    static QTextCodec *thaiCodec = QTextCodec::codecForMib(2259);
    static th_brk_def th_brk = 0;

#ifndef QT_NO_LIBRARY
    /* load libthai dynamically */
    if (!th_brk && thaiCodec) {
        th_brk = (th_brk_def)QLibrary::resolve("thai", "th_brk");
        if (!th_brk)
            thaiCodec = 0;
    }
#endif
    
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
#endif
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

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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


//  Vocabulary
//      Base ->         A consonant or an independent vowel in its full (not subscript) form. It is the
//                      center of the syllable, it can be surrounded by coeng (subscript) consonants, vowels,
//                      split vowels, signs... but there is only one base in a syllable, it has to be coded as
//                      the first character of the syllable.
//      split vowel --> vowel that has two parts placed separately (e.g. Before and after the consonant).
//                      Khmer language has five of them. Khmer split vowels either have one part before the
//                      base and one after the base or they have a part before the base and a part above the base.
//                      The first part of all Khmer split vowels is the same character, identical to
//                      the glyph of Khmer dependent vowel SRA EI
//      coeng -->  modifier used in Khmer to construct coeng (subscript) consonants
//                 Differently than indian languages, the coeng modifies the consonant that follows it,
//                 not the one preceding it  Each consonant has two forms, the base form and the subscript form
//                 the base form is the normal one (using the consonants code-point), the subscript form is
//                 displayed when the combination coeng + consonant is encountered.
//      Consonant of type 1 -> A consonant which has subscript for that only occupies space under a base consonant
//      Consonant of type 2.-> Its subscript form occupies space under and before the base (only one, RO)
//      Consonant of Type 3 -> Its subscript form occupies space under and after the base (KHO, CHHO, THHO, BA, YO, SA)
//      Consonant shifter -> Khmer has to series of consonants. The same dependent vowel has different sounds
//                           if it is attached to a consonant of the first series or a consonant of the second series
//                           Most consonants have an equivalent in the other series, but some of theme exist only in
//                           one series (for example SA). If we want to use the consonant SA with a vowel sound that
//                           can only be done with a vowel sound that corresponds to a vowel accompanying a consonant
//                           of the other series, then we need to use a consonant shifter: TRIISAP or MUSIKATOAN
//                           x17C9 y x17CA. TRIISAP changes a first series consonant to second series sound and
//                           MUSIKATOAN a second series consonant to have a first series vowel sound.
//                           Consonant shifter are both normally supercript marks, but, when they are followed by a
//                           superscript, they change shape and take the form of subscript dependent vowel SRA U.
//                           If they are in the same syllable as a coeng consonant, Unicode 3.0 says that they
//                           should be typed before the coeng. Unicode 4.0 breaks the standard and says that it should
//                           be placed after the coeng consonant.
//      Dependent vowel ->   In khmer dependent vowels can be placed above, below, before or after the base
//                           Each vowel has its own position. Only one vowel per syllable is allowed.
//      Signs            ->  Khmer has above signs and post signs. Only one above sign and/or one post sign are
//                           Allowed in a syllable.
//
//
//   order is important here! This order must be the same that is found in each horizontal
//   line in the statetable for Khmer (see khmerStateTable) .
//
enum KhmerCharClassValues {
    CC_RESERVED             =  0,
    CC_CONSONANT            =  1, // Consonant of type 1 or independent vowel
    CC_CONSONANT2           =  2, // Consonant of type 2
    CC_CONSONANT3           =  3, // Consonant of type 3
    CC_ZERO_WIDTH_NJ_MARK   =  4, // Zero Width non joiner character (0x200C)
    CC_CONSONANT_SHIFTER    =  5,
    CC_ROBAT                =  6, // Khmer special diacritic accent -treated differently in state table
    CC_COENG                =  7, // Subscript consonant combining character
    CC_DEPENDENT_VOWEL      =  8,
    CC_SIGN_ABOVE           =  9,
    CC_SIGN_AFTER           = 10,
    CC_ZERO_WIDTH_J_MARK    = 11, // Zero width joiner character
    CC_COUNT                = 12  // This is the number of character classes
};


enum KhmerCharClassFlags {
    CF_CLASS_MASK    = 0x0000FFFF,

    CF_CONSONANT     = 0x01000000,  // flag to speed up comparing
    CF_SPLIT_VOWEL   = 0x02000000,  // flag for a split vowel -> the first part is added in front of the syllable
    CF_DOTTED_CIRCLE = 0x04000000,  // add a dotted circle if a character with this flag is the first in a syllable
    CF_COENG         = 0x08000000,  // flag to speed up comparing
    CF_SHIFTER       = 0x10000000,  // flag to speed up comparing
    CF_ABOVE_VOWEL   = 0x20000000,  // flag to speed up comparing

    // position flags
    CF_POS_BEFORE    = 0x00080000,
    CF_POS_BELOW     = 0x00040000,
    CF_POS_ABOVE     = 0x00020000,
    CF_POS_AFTER     = 0x00010000,
    CF_POS_MASK      = 0x000f0000
};


// Characters that get refered to by name
enum KhmerChar {
    C_SIGN_ZWNJ     = 0x200C,
    C_SIGN_ZWJ      = 0x200D,
    C_DOTTED_CIRCLE = 0x25CC,
    C_RO            = 0x179A,
    C_VOWEL_AA      = 0x17B6,
    C_SIGN_NIKAHIT  = 0x17C6,
    C_VOWEL_E       = 0x17C1,
    C_COENG         = 0x17D2
};


//  simple classes, they are used in the statetable (in this file) to control the length of a syllable
//  they are also used to know where a character should be placed (location in reference to the base character)
//  and also to know if a character, when independently displayed, should be displayed with a dotted-circle to
//  indicate error in syllable construction
//
enum {
    _xx = CC_RESERVED,
    _sa = CC_SIGN_ABOVE | CF_DOTTED_CIRCLE | CF_POS_ABOVE,
    _sp = CC_SIGN_AFTER | CF_DOTTED_CIRCLE| CF_POS_AFTER,
    _c1 = CC_CONSONANT | CF_CONSONANT,
    _c2 = CC_CONSONANT2 | CF_CONSONANT,
    _c3 = CC_CONSONANT3 | CF_CONSONANT,
    _rb = CC_ROBAT | CF_POS_ABOVE | CF_DOTTED_CIRCLE,
    _cs = CC_CONSONANT_SHIFTER | CF_DOTTED_CIRCLE | CF_SHIFTER,
    _dl = CC_DEPENDENT_VOWEL | CF_POS_BEFORE | CF_DOTTED_CIRCLE,
    _db = CC_DEPENDENT_VOWEL | CF_POS_BELOW | CF_DOTTED_CIRCLE,
    _da = CC_DEPENDENT_VOWEL | CF_POS_ABOVE | CF_DOTTED_CIRCLE | CF_ABOVE_VOWEL,
    _dr = CC_DEPENDENT_VOWEL | CF_POS_AFTER | CF_DOTTED_CIRCLE,
    _co = CC_COENG | CF_COENG | CF_DOTTED_CIRCLE,

    // split vowel
    _va = _da | CF_SPLIT_VOWEL,
    _vr = _dr | CF_SPLIT_VOWEL
};


//   Character class: a character class value
//   ORed with character class flags.
//
typedef unsigned long KhmerCharClass;


//  Character class tables
//  _xx character does not combine into syllable, such as numbers, puntuation marks, non-Khmer signs...
//  _sa Sign placed above the base
//  _sp Sign placed after the base
//  _c1 Consonant of type 1 or independent vowel (independent vowels behave as type 1 consonants)
//  _c2 Consonant of type 2 (only RO)
//  _c3 Consonant of type 3
//  _rb Khmer sign robat u17CC. combining mark for subscript consonants
//  _cd Consonant-shifter
//  _dl Dependent vowel placed before the base (left of the base)
//  _db Dependent vowel placed below the base
//  _da Dependent vowel placed above the base
//  _dr Dependent vowel placed behind the base (right of the base)
//  _co Khmer combining mark COENG u17D2, combines with the consonant or independent vowel following
//      it to create a subscript consonant or independent vowel
//  _va Khmer split vowel in wich the first part is before the base and the second one above the base
//  _vr Khmer split vowel in wich the first part is before the base and the second one behind (right of) the base
//
static const KhmerCharClass khmerCharClasses[] = {
    _c1, _c1, _c1, _c3, _c1, _c1, _c1, _c1, _c3, _c1, _c1, _c1, _c1, _c3, _c1, _c1, // 1780 - 178F
    _c1, _c1, _c1, _c1, _c3, _c1, _c1, _c1, _c1, _c3, _c2, _c1, _c1, _c1, _c3, _c3, // 1790 - 179F
    _c1, _c3, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, _c1, // 17A0 - 17AF
    _c1, _c1, _c1, _c1, _dr, _dr, _dr, _da, _da, _da, _da, _db, _db, _db, _va, _vr, // 17B0 - 17BF
    _vr, _dl, _dl, _dl, _vr, _vr, _sa, _sp, _sp, _cs, _cs, _sa, _rb, _sa, _sa, _sa, // 17C0 - 17CF
    _sa, _sa, _co, _sa, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _sa, _xx, _xx  // 17D0 - 17DF
};

// this enum must reflect the range of khmerCharClasses
enum KhmerCharClassesRange {
    KhmerFirstChar = 0x1780,
    KhmerLastChar  = 0x17df
};

//  Below we define how a character in the input string is either in the khmerCharClasses table
//  (in which case we get its type back), a ZWJ or ZWNJ (two characters that may appear
//  within the syllable, but are not in the table) we also get their type back, or an unknown object
//  in which case we get _xx (CC_RESERVED) back
//
static inline KhmerCharClass getKhmerCharClass(const QChar &uc)
{
    if (uc.unicode() == C_SIGN_ZWJ) {
        return CC_ZERO_WIDTH_J_MARK;
    }

    if (uc.unicode() == C_SIGN_ZWNJ) {
        return CC_ZERO_WIDTH_NJ_MARK;
    }

    if (uc.unicode() < KhmerFirstChar || uc.unicode() > KhmerLastChar) {
        return CC_RESERVED;
    }

    return khmerCharClasses[uc.unicode() - KhmerFirstChar];
}


//  The stateTable is used to calculate the end (the length) of a well
//  formed Khmer Syllable.
//
//  Each horizontal line is ordered exactly the same way as the values in KhmerClassTable
//  CharClassValues. This coincidence of values allows the follow up of the table.
//
//  Each line corresponds to a state, which does not necessarily need to be a type
//  of component... for example, state 2 is a base, with is always a first character
//  in the syllable, but the state could be produced a consonant of any type when
//  it is the first character that is analysed (in ground state).
//
//  Differentiating 3 types of consonants is necessary in order to
//  forbid the use of certain combinations, such as having a second
//  coeng after a coeng RO,
//  The inexistent possibility of having a type 3 after another type 3 is permitted,
//  eliminating it would very much complicate the table, and it does not create typing
//  problems, as the case above.
//
//  The table is quite complex, in order to limit the number of coeng consonants
//  to 2 (by means of the table).
//
//  There a peculiarity, as far as Unicode is concerned:
//  - The consonant-shifter is considered in two possible different
//    locations, the one considered in Unicode 3.0 and the one considered in
//    Unicode 4.0. (there is a backwards compatibility problem in this standard).
//
//
//  xx    independent character, such as a number, punctuation sign or non-khmer char
//
//  c1    Khmer consonant of type 1 or an independent vowel
//        that is, a letter in which the subscript for is only under the
//        base, not taking any space to the right or to the left
//
//  c2    Khmer consonant of type 2, the coeng form takes space under
//        and to the left of the base (only RO is of this type)
//
//  c3    Khmer consonant of type 3. Its subscript form takes space under
//        and to the right of the base.
//
//  cs    Khmer consonant shifter
//
//  rb    Khmer robat
//
//  co    coeng character (u17D2)
//
//  dv    dependent vowel (including split vowels, they are treated in the same way).
//        even if dv is not defined above, the component that is really tested for is
//        KhmerClassTable::CC_DEPENDENT_VOWEL, which is common to all dependent vowels
//
//  zwj   Zero Width joiner
//
//  zwnj  Zero width non joiner
//
//  sa    above sign
//
//  sp    post sign
//
//  there are lines with equal content but for an easier understanding
//  (and maybe change in the future) we did not join them
//
static const signed char khmerStateTable[][CC_COUNT] =
{
   // xx  c1  c2  c3 zwnj cs  rb  co  dv  sa  sp zwj
    { 1,  2,  2,  2,  1,  1,  1,  6,  1,  1,  1,  2}, //  0 - ground state
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, //  1 - exit state (or sign to the right of the syllable)
    {-1, -1, -1, -1,  3,  4,  5,  6, 16, 17,  1, -1}, //  2 - Base consonant
    {-1, -1, -1, -1, -1,  4, -1, -1, 16, -1, -1, -1}, //  3 - First ZWNJ before a register shifter It can only be followed by a shifter or a vowel
    {-1, -1, -1, -1, 15, -1, -1,  6, 16, 17,  1, 14}, //  4 - First register shifter
    {-1, -1, -1, -1, -1, -1, -1, -1, 20, -1,  1, -1}, //  5 - Robat
    {-1,  7,  8,  9, -1, -1, -1, -1, -1, -1, -1, -1}, //  6 - First Coeng
    {-1, -1, -1, -1, 12, 13, -1, 10, 16, 17,  1, 14}, //  7 - First consonant of type 1 after coeng
    {-1, -1, -1, -1, 12, 13, -1, -1, 16, 17,  1, 14}, //  8 - First consonant of type 2 after coeng
    {-1, -1, -1, -1, 12, 13, -1, 10, 16, 17,  1, 14}, //  9 - First consonant or type 3 after ceong
    {-1, 11, 11, 11, -1, -1, -1, -1, -1, -1, -1, -1}, // 10 - Second Coeng (no register shifter before)
    {-1, -1, -1, -1, 15, -1, -1, -1, 16, 17,  1, 14}, // 11 - Second coeng consonant (or ind. vowel) no register shifter before
    {-1, -1,  1, -1, -1, 13, -1, -1, 16, -1, -1, -1}, // 12 - Second ZWNJ before a register shifter
    {-1, -1, -1, -1, 15, -1, -1, -1, 16, 17,  1, 14}, // 13 - Second register shifter
    {-1, -1, -1, -1, -1, -1, -1, -1, 16, -1, -1, -1}, // 14 - ZWJ before vowel
    {-1, -1, -1, -1, -1, -1, -1, -1, 16, -1, -1, -1}, // 15 - ZWNJ before vowel
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, 17,  1, 18}, // 16 - dependent vowel
    {-1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, 18}, // 17 - sign above
    {-1, -1, -1, -1, -1, -1, -1, 19, -1, -1, -1, -1}, // 18 - ZWJ after vowel
    {-1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1}, // 19 - Third coeng
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1}, // 20 - dependent vowel after a Robat
};


//  #define KHMER_DEBUG
#ifdef KHMER_DEBUG
#define KHDEBUG qDebug
#else
#define KHDEBUG if(0) qDebug
#endif

//  Given an input string of characters and a location in which to start looking
//  calculate, using the state table, which one is the last character of the syllable
//  that starts in the starting position.
//
static inline int khmer_nextSyllableBoundary(const QString &s, int start, int end, bool *invalid)
{
    *invalid = false;
    const QChar *uc = s.unicode() + start;
    int state = 0;
    int pos = start;

    while (pos < end) {
        KhmerCharClass charClass = getKhmerCharClass(*uc);
        if (pos == start) {
            *invalid = (charClass > 0) && ! (charClass & CF_CONSONANT);
        }
        state = khmerStateTable[state][charClass & CF_CLASS_MASK];

        KHDEBUG("state[%d]=%d class=%8lx (uc=%4x)", pos - start, state,
                charClass, uc->unicode() );

        if (state < 0) {
            break;
        }
        ++uc;
        ++pos;
    }
    return pos;
}



static bool khmer_shape_syllable(QOpenType *openType, QShaperItem *item)
{
    // according to the specs this is the max length one can get
    // ### the real value should be smaller
    assert(item->length < 13);

    KHDEBUG("syllable from %d len %d, str='%s'", item->from, item->length,
	    item->string->mid(item->from, item->length).toUtf8().data());

    int len = 0;
    int syllableEnd = item->from + item->length;
    unsigned short reordered[16];
    unsigned char properties[16];
    enum {
	AboveForm = 0x01,
	PreForm = 0x02,
	PostForm = 0x04,
	BelowForm = 0x08
    };
    memset(properties, 0, 16*sizeof(unsigned char));

#ifdef KHMER_DEBUG
    qDebug("original:");
    for (int i = from; i < syllableEnd; i++) {
        qDebug("    %d: %4x", i, string[i].unicode());
    }
#endif

    // write a pre vowel or the pre part of a split vowel first
    // and look out for coeng + ro. RO is the only vowel of type 2, and
    // therefore the only one that requires saving space before the base.
    //
    int coengRo = -1;  // There is no Coeng Ro, if found this value will change
    for (int i = item->from; i < syllableEnd; i += 1) {
        KhmerCharClass charClass = getKhmerCharClass(item->string->at(i));

        // if a split vowel, write the pre part. In Khmer the pre part
        // is the same for all split vowels, same glyph as pre vowel C_VOWEL_E
        if (charClass & CF_SPLIT_VOWEL) {
            reordered[len] = C_VOWEL_E;
            properties[len] = PreForm;
            ++len;
            break; // there can be only one vowel
        }
        // if a vowel with pos before write it out
        if (charClass & CF_POS_BEFORE) {
            reordered[len] = item->string->at(i).unicode();
            properties[len] = PreForm;
            ++len;
            break; // there can be only one vowel
        }
        // look for coeng + ro and remember position
        // works because coeng + ro is always in front of a vowel (if there is a vowel)
        // and because CC_CONSONANT2 is enough to identify it, as it is the only consonant
        // with this flag
        if ( (charClass & CF_COENG) && (i + 1 < syllableEnd) &&
              ( (getKhmerCharClass(item->string->at(i+1)) & CF_CLASS_MASK) == CC_CONSONANT2) ) {
            coengRo = i;
        }
    }

    // write coeng + ro if found
    if (coengRo > -1) {
        reordered[len] = C_COENG;
        properties[len] = PreForm;
        ++len;
        reordered[len] = C_RO;
        properties[len] = PreForm;
        ++len;
    }

    // shall we add a dotted circle?
    // If in the position in which the base should be (first char in the string) there is
    // a character that has the Dotted circle flag (a character that cannot be a base)
    // then write a dotted circle
    if (getKhmerCharClass(item->string->at(item->from)) & CF_DOTTED_CIRCLE) {
        reordered[len] = C_DOTTED_CIRCLE;
        ++len;
    }

    // copy what is left to the output, skipping before vowels and
    // coeng Ro if they are present
    for (int i = item->from; i < syllableEnd; i += 1) {
        QChar uc = item->string->at(i);
        KhmerCharClass charClass = getKhmerCharClass(uc);

        // skip a before vowel, it was already processed
        if (charClass & CF_POS_BEFORE) {
            continue;
        }

        // skip coeng + ro, it was already processed
        if (i == coengRo) {
            i += 1;
            continue;
        }

        switch (charClass & CF_POS_MASK)
        {
            case CF_POS_ABOVE :
                reordered[len] = uc.unicode();
                properties[len] = AboveForm;
                ++len;
                break;

            case CF_POS_AFTER :
                reordered[len] = uc.unicode();
                properties[len] = PostForm;
                ++len;
                break;

            case CF_POS_BELOW :
                reordered[len] = uc.unicode();
                properties[len] = BelowForm;
                ++len;
                break;

            default:
                // assign the correct flags to a coeng consonant
                // Consonants of type 3 are taged as Post forms and those type 1 as below forms
                if ( (charClass & CF_COENG) && i + 1 < syllableEnd ) {
                    unsigned char property = (getKhmerCharClass(item->string->at(i+1)) & CF_CLASS_MASK) == CC_CONSONANT3 ?
                                              PostForm : BelowForm;
                    reordered[len] = uc.unicode();
                    properties[len] = property;
                    ++len;
                    i += 1;
                    reordered[len] = item->string->at(i).unicode();
                    properties[len] = property;
                    ++len;
                    break;
                }

                // if a shifter is followed by an above vowel change the shifter to below form,
                // an above vowel can have two possible positions i + 1 or i + 3
                // (position i+1 corresponds to unicode 3, position i+3 to Unicode 4)
                // and there is an extra rule for C_VOWEL_AA + C_SIGN_NIKAHIT also for two
                // different positions, right after the shifter or after a vowel (Unicode 4)
                if ( (charClass & CF_SHIFTER) && (i + 1 < syllableEnd) ) {
                    if (getKhmerCharClass(item->string->at(i+1)) & CF_ABOVE_VOWEL ) {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                    if (i + 2 < syllableEnd &&
                        (item->string->at(i+1).unicode() == C_VOWEL_AA) &&
                        (item->string->at(i+2).unicode() == C_SIGN_NIKAHIT) )
                    {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                    if (i + 3 < syllableEnd && (getKhmerCharClass(item->string->at(i+3)) & CF_ABOVE_VOWEL) ) {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                    if (i + 4 < syllableEnd &&
                        (item->string->at(i+3).unicode() == C_VOWEL_AA) &&
                        (item->string->at(i+4).unicode() == C_SIGN_NIKAHIT) )
                    {
                        reordered[len] = uc.unicode();
                        properties[len] = BelowForm;
                        ++len;
                        break;
                    }
                }

                // default - any other characters
                reordered[len] = uc.unicode();
                ++len;
                break;
        } // switch
    } // for
    
    if (!item->font->stringToCMap((const QChar *)reordered, len, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;

    KHDEBUG("after shaping: len=%d", len);
    for (int i = 0; i < len; i++) {
	item->glyphs[i].attributes.mark = false;
	item->glyphs[i].attributes.clusterStart = false;
	item->glyphs[i].attributes.justification = 0;
	item->glyphs[i].attributes.zeroWidth = false;
	KHDEBUG("    %d: %4x property=%x", i, reordered[i], properties[i]);
    }
    item->glyphs[0].attributes.clusterStart = true;

    // now we have the syllable in the right order, and can start running it through open type.

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
    if (openType) {
	unsigned short logClusters[16];
	for (int i = 0; i < len; ++i)
	    logClusters[i] = i;


	openType->init(item);

 	bool where[16];

	// substitutions
	const struct {
	    int feature; int form;
	} features[] = {
	    { FT_MAKE_TAG( 'p', 'r', 'e', 'f' ), PreForm },
	    { FT_MAKE_TAG( 'b', 'l', 'w', 'f' ), BelowForm },
	    { FT_MAKE_TAG( 'a', 'b', 'v', 'f' ), AboveForm },
	    { FT_MAKE_TAG( 'p', 's', 't', 'f' ), PostForm }
	};
	for (int j = 0; j < 4; ++j) {
	    for (int i = 0; i < len; ++i)
		where[i] = (properties[i] & features[j].form);
	    openType->applyGSUBFeature(features[j].feature, where);
	}

	const int features2 [] = {
	    FT_MAKE_TAG( 'p', 'r', 'e', 's' ),
	    FT_MAKE_TAG( 'b', 'l', 'w', 's' ),
	    FT_MAKE_TAG( 'a', 'b', 'v', 's' ),
	    FT_MAKE_TAG( 'p', 's', 't', 's' ),
	    FT_MAKE_TAG( 'c', 'l', 'i', 'g' )
	};
	for (int i = 0; i < 5; ++i)
	    openType->applyGSUBFeature(features2[i]);

	openType->applyGPOSFeatures();

	openType->appendTo(item, false);
    } else
#endif
    {
	KHDEBUG("Not using openType");
	Q_UNUSED(openType);
    }

    return true;
}

static bool khmer_shape(QShaperItem *item)
{
    assert(item->script == QUnicodeTables::Khmer);

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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
    KHDEBUG("khmer_shape: from %d length %d", item->from, item->length);
    while (sstart < end) {
        bool invalid;
        int send = khmer_nextSyllableBoundary(*item->string, sstart, end, &invalid);
        KHDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "true" : "false");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!khmer_shape_syllable(openType, &syllable)) {
            KHDEBUG("syllable shaping failed, syllable requests %d glyphs", syllable.num_glyphs);
            item->num_glyphs += syllable.num_glyphs;
            return false;
        }
        // fix logcluster array
        KHDEBUG("syllable:");
        for (int i = first_glyph; i < first_glyph + syllable.num_glyphs; ++i)
            KHDEBUG("        %d -> glyph %x", i, item->glyphs[i].glyph);
        KHDEBUG("    logclusters:");
        for (int i = sstart; i < send; ++i) {
            KHDEBUG("        %d -> glyph %d", i, first_glyph);
            logClusters[i-item->from] = first_glyph;
        }
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return true;
}

static void khmer_attributes( int script, const QString &text, int from, int len, QCharAttributes *attributes )
{
    Q_UNUSED(script);

    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while ( i < len ) {
	bool invalid;
	int boundary = khmer_nextSyllableBoundary( text, from+i, end, &invalid ) - from;

	attributes[i].whiteSpace = ::isSpace(*uc);
	attributes[i].softBreak = false;
	attributes[i].charStop = true;
	attributes[i].wordStop = false;
	attributes[i].invalid = invalid;

	if ( boundary > len-1 ) boundary = len;
	i++;
	while ( i < boundary ) {
	    attributes[i].whiteSpace = ::isSpace(*uc);
	    attributes[i].softBreak = false;
	    attributes[i].charStop = false;
	    attributes[i].wordStop = false;
	    attributes[i].invalid = invalid;
	    ++uc;
	    ++i;
	}
	assert( i == boundary );
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

#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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
#if defined(QT_HAVE_FREETYPE) && !defined(QT_NO_FREETYPE)
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
