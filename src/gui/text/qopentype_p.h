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

#ifndef QOPENTYPE_P_H
#define QOPENTYPE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtextengine_p.h"

#ifdef QT_HAVE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftxopen.h"

enum IndicFeatures {
    CcmpFeature,
    InitFeature,
    NuktaFeature,
    AkhantFeature,
    RephFeature,
    BelowFormFeature,
    HalfFormFeature,
    PostFormFeature,
    VattuFeature,
    PreSubstFeature,
    AboveSubstFeature,
    BelowSubstFeature,
    PostSubstFeature,
    HalantFeature
};

class QShaperItem;

class QOpenType
{
public:
    QOpenType(QFontEngine *fe, FT_Face face);
    ~QOpenType();

    bool supportsScript(unsigned int script);

    void applyGSUBFeature(unsigned int feature, bool *where = 0);
    void applyGPOSFeatures();


    void init(QShaperItem *item);
    bool appendTo(QShaperItem *item, bool doLogClusters = true);

    const int *mapping(int &len);
    inline void setLength(int len) { str->length = len; }
    unsigned short *glyphs() { return str->string; }
private:
    bool loadTables(FT_ULong script);

    QFontEngine *fontEngine;
    FT_Face face;
    TTO_GDEF gdef;
    TTO_GSUB gsub;
    TTO_GPOS gpos;
    FT_UShort script_index;
    FT_ULong current_script;
    bool hasGDef : 1;
    bool hasGSub : 1;
    bool hasGPos : 1;
    bool positioned : 1;
    TTO_GSUB_String *str;
    TTO_GSUB_String *tmp;
    TTO_GPOS_Data *positions;
    QGlyphLayout::Attributes *tmpAttributes;
    unsigned short *tmpLogClusters;
    int length;
    int orig_nglyphs;
    int loadFlags;
};

#endif // QT_HAVE_FREETYPE

#endif // QOPENTYPE_P_H
