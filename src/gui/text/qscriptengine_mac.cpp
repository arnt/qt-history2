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

#if defined(Q_NEW_MAC_FONTENGINE)
static bool mac_shape(QShaperItem *item)
{
    QFontEngineMac *fe = static_cast<QFontEngineMac *>(item->font);
    if (!fe->stringToCMap(item->string->unicode()+item->from, item->length,
                          item->glyphs, &item->num_glyphs, QFlag(item->flags),
                          item))
        return false;

    heuristicSetGlyphAttributes(item);
    return true;
}

const q_scriptEngine qt_scriptEngines[] = {
    // Common
    { mac_shape, 0 },
    // Hebrew
    { mac_shape, 0 },
    // Arabic
    { mac_shape, 0 },
    // Syriac
    { mac_shape, 0 },
    // Thaana
    { mac_shape, 0 },
    // Devanagari
    { mac_shape, 0 },
    // Bengali
    { mac_shape, 0 },
    // Gurmukhi
    { mac_shape, 0 },
    // Gujarati
    { mac_shape, 0 },
    // Oriya
    { mac_shape, 0 },
    // Tamil
    { mac_shape, 0 },
    // Telugu
    { mac_shape, 0 },
    // Kannada
    { mac_shape, 0 },
    // Malayalam
    { mac_shape, 0 },
    // Sinhala
    { mac_shape, 0 },
    // Thai
    { mac_shape, 0 },
    // Lao
    { mac_shape, 0 },
    // Tibetan
    { mac_shape, 0 },
    // Myanmar
    { mac_shape, 0 },
    // Hangul
    { mac_shape, 0 },
    // Khmer
    { mac_shape, 0 }
};
#else

const q_scriptEngine qt_scriptEngines[] = {
    // Common
    { basic_shape, 0 },
    // Hebrew
    { hebrew_shape, 0 },
    // Arabic
    { arabic_shape, 0 },
    // Syriac
    { basic_shape, 0 },
    // Thaana
    { basic_shape, 0 },
    // Devanagari
    { basic_shape, 0 },
    // Bengali
    { basic_shape, 0 },
    // Gurmukhi
    { basic_shape, 0 },
    // Gujarati
    { basic_shape, 0 },
    // Oriya
    { basic_shape, 0 },
    // Tamil
    { basic_shape, 0 },
    // Telugu
    { basic_shape, 0 },
    // Kannada
    { basic_shape, 0 },
    // Malayalam
    { basic_shape, 0 },
    // Sinhala
    { basic_shape, 0 },
    // Thai
    { basic_shape, 0 },
    // Lao
    { basic_shape, 0 },
    // Tibetan
    { basic_shape, 0 },
    // Myanmar
    { basic_shape, 0 },
    // Hangul
    { basic_shape, 0 },
    // Khmer
    { basic_shape, 0 }
};

#endif

