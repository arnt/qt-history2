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

#ifndef QFONTSUBSET_P_H
#define QFONTSUBSET_P_H

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

#include "private/qfontengine_p.h"

#ifndef QT_NO_PRINTER

class QFontSubset
{
public:
    QFontSubset(QFontEngine *fe, int obj_id = 0)
        : object_id(obj_id), fontEngine(fe)
        { fontEngine->ref.ref(); addGlyph(0); }
    ~QFontSubset() {
        if (!fontEngine->ref.deref())
            delete fontEngine;
    }

    QByteArray toTruetype() const;
    QByteArray toType1() const;
    QByteArray widthArray() const;
    QByteArray createToUnicodeMap() const;
    QVector<int> getReverseMap() const;
    QByteArray glyphName(unsigned int glyph, const QVector<int> reverseMap) const;

    static QByteArray glyphName(unsigned short unicode, bool symbol);

    int addGlyph(int index);
    const int object_id;
    QFontEngine *fontEngine;
    QList<int> glyph_indices;
    int nGlyphs() const { return glyph_indices.size(); }
    mutable QFixed emSquare;
    mutable QVector<QFixed> widths;
};

#endif
#endif
