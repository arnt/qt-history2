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

#ifndef QTEXTDOCUMENTFRAGMENT_P_H
#define QTEXTDOCUMENTFRAGMENT_P_H

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

#include "qtextdocument.h"
#include "qtexthtmlparser_p.h"
#include "qtextdocument_p.h"

#include <qlist.h>
#include <qmap.h>
#include <qpointer.h>
#include <qvarlengtharray.h>
#include <qdatastream.h>

class QTextDocumentFragmentPrivate
{
public:
    enum MarkerValues { FragmentStart = 1, FragmentEnd = 2 };

    QTextDocumentFragmentPrivate() : hasTitle(false), setMarkerForHtmlExport(false) {}
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);

    void insert(QTextCursor &cursor) const;

    void appendFragments(QTextDocumentPrivate *priv, int pos, int endPos);
    int appendFragment(QTextDocumentPrivate *priv, int pos, int endPos, int objectIndex = -1);
    void appendText(const QString &text, int formatIdx, int blockIdx = -2);

    QMap<int, int> fillFormatCollection(QTextFormatCollection *collection) const;

    // ### TODO: merge back into one big vector.

    struct TextFragment
    {
        TextFragment()
            : position(0), size(0),
              charFormat(-1), blockFormat(-2) {}
        Q_INT32 position;
        Q_UINT32 size;
        Q_INT32 charFormat;
        Q_INT32 blockFormat;
    };
    typedef QVector<TextFragment> FragmentVector;

    FragmentVector fragments;

    QString localBuffer;

    QTextFormatCollection formatCollection;

    Q_INT8 hasTitle;
    QString title;

    bool setMarkerForHtmlExport;
};

// ###### Versioning!

inline QDataStream &operator<<(QDataStream &stream,
                               const QTextDocumentFragmentPrivate::TextFragment &fragment)
{
    return stream << fragment.position
                  << fragment.size
                  << fragment.charFormat
                  << fragment.blockFormat;
}

inline QDataStream &operator>>(QDataStream &stream,
                               QTextDocumentFragmentPrivate::TextFragment &fragment)
{
    return stream >> fragment.position
                  >> fragment.size
                  >> fragment.charFormat
                  >> fragment.blockFormat;
}

inline QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragmentPrivate &priv)
{
    return stream << priv.formatCollection
                  << priv.fragments
                  << priv.localBuffer
                  << priv.hasTitle
                  << priv.title;
}

inline QDataStream &operator>>(QDataStream &stream, QTextDocumentFragmentPrivate &priv)
{
    return stream >> priv.formatCollection
                  >> priv.fragments
                  >> priv.localBuffer
                  >> priv.hasTitle
                  >> priv.title;
}

class QTextHTMLImporter : public QTextHtmlParser
{
    struct Table;
public:
    QTextHTMLImporter(QTextDocumentFragmentPrivate *d, QString html);

    void import();

private:
    bool closeTag(int i);

    bool scanTable(int tableNodeIdx, Table *table);

    void appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt = QTextCharFormat(), const QChar &separator = QChar::ParagraphSeparator);
    void appendText(QString text, QTextCharFormat format);
    inline void appendImage(const QTextImageFormat &format)
    { appendText(QString(QChar::ObjectReplacementCharacter), format); }

    QTextDocumentFragmentPrivate *d;
    QVector<int> listReferences;
    int indent;

    // insert a named anchor the next time we emit a char format,
    // either in a block or in regular text
    bool setNamedAnchorInNextOutput;
    QString namedAnchor;

    struct Table
    {
        Table() : tableIndex(-1), currentColumnCount(0), currentRow(-1) {}
        int tableIndex; // objectIndex
        int currentColumnCount;
        int columns;
        QVector<int> rowSpanCellsPerRow;
        int currentRow;
    };
    QVector<Table> tables;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
