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

static QFontEngineQPF::TagType tagTypes[QFontEngineQPF::NumTags] = {
    QFontEngineQPF::StringType, // FontName
    QFontEngineQPF::StringType, // FileName
    QFontEngineQPF::UInt32Type, // FileIndex
    QFontEngineQPF::UInt32Type, // FontRevision
    QFontEngineQPF::StringType, // FreeText
    QFontEngineQPF::FixedType,  // Ascent
    QFontEngineQPF::FixedType,  // Descent
    QFontEngineQPF::FixedType,  // Leading
    QFontEngineQPF::FixedType,  // XHeight
    QFontEngineQPF::FixedType,  // AverageCharWidth
    QFontEngineQPF::FixedType,  // MaxCharWidth
    QFontEngineQPF::FixedType,  // LineThickness
    QFontEngineQPF::FixedType,  // MinLeftBearing
    QFontEngineQPF::FixedType,  // MinRightBearing
    QFontEngineQPF::FixedType,  // UnderlinePosition
    QFontEngineQPF::UInt8Type,  // GlyphFormat
    QFontEngineQPF::UInt8Type,  // PixelSize
    QFontEngineQPF::UInt8Type,  // Weight
    QFontEngineQPF::UInt8Type,  // Style
    QFontEngineQPF::StringType  // EndOfHeader
};


