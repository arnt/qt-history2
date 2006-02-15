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

// -----------------------------------------------------------------------------------------------
//
// The script engine jump table
//
// -----------------------------------------------------------------------------------------------

const q_scriptEngine qt_scriptEngines[] = {
    // Common
    { basic_shape, 0},
    // Hebrew
    { hebrew_shape, 0 },
    // Arabic
    { arabic_shape, 0},
    // Syriac
    { syriac_shape, 0},
    // Thaana
    { thaana_shape, 0 },
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
    { basic_shape, 0 },
    // Tibetan
    { tibetan_shape, tibetan_attributes },
    // Myanmar
    { basic_shape, 0 },
    // Hangul
    { hangul_shape, hangul_attributes },
    // Khmer
    { khmer_shape, khmer_attributes }
};
