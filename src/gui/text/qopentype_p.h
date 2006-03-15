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

#ifndef QT_NO_OPENTYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftxopen.h"

enum { PositioningProperties = 0x80000000 };

class QShaperItem;

class QOpenType
{
public:
    QOpenType(QFontEngine *fe, FT_Face face);
    ~QOpenType();

    struct Features {
        uint tag;
        uint property;
    };

    bool supportsScript(unsigned int script) {
        Q_ASSERT(script < QUnicodeTables::ScriptCount);
        return supported_scripts[script];
    }
    void selectScript(unsigned int script, const Features *features = 0);
    
    void shape(QShaperItem *item, const unsigned int *properties = 0);
    bool positionAndAdd(QShaperItem *item, int availableGlyphs, bool doLogClusters = true);

    OTL_GlyphItem glyphs() const { return otl_buffer->in_string; }
    int len() const { return otl_buffer->in_length; }
    void setProperty(int index, uint property) { otl_buffer->in_string[index].properties = property; }


private: 
    bool checkScript(unsigned int script);
    QFontEngine *fontEngine;
    FT_Face face;
    TTO_GDEF gdef;
    TTO_GSUB gsub;
    TTO_GPOS gpos;
    bool supported_scripts[QUnicodeTables::ScriptCount];
    FT_ULong current_script;
    bool positioned : 1;
    OTL_Buffer otl_buffer;
    QGlyphLayout::Attributes *tmpAttributes;
    unsigned int *tmpLogClusters;
    int length;
    int orig_nglyphs;
    int loadFlags;
};

#endif // QT_NO_OPENTYPE

#endif // QOPENTYPE_P_H
