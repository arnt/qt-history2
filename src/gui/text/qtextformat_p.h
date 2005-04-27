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

#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

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

#include "qtextformat.h"
#include <qvector.h>
#include <qset.h>

class QTextFormatCollection
{
public:
    QTextFormatCollection() {}
    ~QTextFormatCollection();

    QTextFormatCollection(const QTextFormatCollection &rhs);
    QTextFormatCollection &operator=(const QTextFormatCollection &rhs);

    QTextFormat objectFormat(int objectIndex) const;
    void setObjectFormat(int objectIndex, const QTextFormat &format);

    int objectFormatIndex(int objectIndex) const;
    void setObjectFormatIndex(int objectIndex, int formatIndex);

    int createObjectIndex(const QTextFormat &f);

    int indexForFormat(const QTextFormat &f);
    bool hasFormatCached(const QTextFormat &format) const;

    QTextFormat format(int idx) const;
    inline QTextBlockFormat blockFormat(int index) const
    { return format(index).toBlockFormat(); }
    inline QTextCharFormat charFormat(int index) const
    { return format(index).toCharFormat(); }
    inline QTextListFormat listFormat(int index) const
    { return format(index).toListFormat(); }
    inline QTextTableFormat tableFormat(int index) const
    { return format(index).toTableFormat(); }
    inline QTextImageFormat imageFormat(int index) const
    { return format(index).toImageFormat(); }

    inline int numFormats() const { return formats.count(); }

    typedef QVector<QTextFormat> FormatVector;

    FormatVector formats;
    QVector<qint32> objFormats;
    QSet<uint> hashes;
};

#endif // QTEXTFORMAT_P_H
