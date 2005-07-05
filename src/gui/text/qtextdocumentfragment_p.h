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
#include "qtexttable.h"

#include <qlist.h>
#include <qmap.h>
#include <qpointer.h>
#include <qvarlengtharray.h>
#include <qdatastream.h>

class QTextDocumentFragmentPrivate;

class QTextImportHelper
{
public:
    QTextImportHelper(QTextDocumentFragmentPrivate *docFragment, QTextDocumentPrivate *priv);

    void appendFragments(int pos, int endPos);
    int appendFragment(int pos, int endPos, int objectIndex = -1);
    int convertFormatIndex(const QTextFormat &oldFormat, int objectIndexToSet = -1);
    inline int convertFormatIndex(int oldFormatIndex, int objectIndexToSet = -1)
    { return convertFormatIndex(priv->formatCollection()->format(oldFormatIndex), objectIndexToSet); }

private:
    QTextDocumentFragmentPrivate *docFragment;
    QTextDocumentPrivate *priv;
    QTextFormatCollection &formatCollection;
    const QString originalText;
    QMap<int, int> objectIndexMap;
};

class QTextDocumentFragmentPrivate
{
public:
    enum MarkerValues { FragmentStart = 1, FragmentEnd = 2 };

    QTextDocumentFragmentPrivate() : hasTitle(false), containsCompleteDocument(false), setMarkerForHtmlExport(false) {}
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);

    void insert(QTextCursor &cursor) const;

    void appendText(const QString &text, int formatIdx, int blockIdx = -2);

    QMap<int, int> fillFormatCollection(QTextFormatCollection *collection) const;

    // ### TODO: merge back into one big vector.

    struct TextFragment
    {
        TextFragment()
            : position(0), size(0),
              charFormat(-1), blockFormat(-2) {}
        qint32 position;
        quint32 size;
        qint32 charFormat;
        qint32 blockFormat;
    };
    typedef QVector<TextFragment> FragmentVector;

    FragmentVector fragments;

    QString localBuffer;

    QTextFormatCollection formatCollection;

    qint8 hasTitle;
    QString title;

    QTextFrameFormat rootFrameFormat;

    uint containsCompleteDocument : 1;
    uint setMarkerForHtmlExport : 1;
};

class QTextHTMLImporter : public QTextHtmlParser
{
    struct Table;
public:
    QTextHTMLImporter(QTextDocument *_doc, const QString &html);

    void import();

    bool hasTitle;
    QString title;
    bool containsCompleteDocument;
    QTextFrameFormat rootFrameFormat;

private:
    bool closeTag(int i);

    bool scanTable(int tableNodeIdx, Table *table);

    void appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt = QTextCharFormat());
    void appendText(QString text, QTextCharFormat format);

    struct List
    {
        QTextListFormat format;
        QPointer<QTextList> list;
    };
    QVector<List> lists;
    int indent;

    // insert a named anchor the next time we emit a char format,
    // either in a block or in regular text
    bool setNamedAnchorInNextOutput;
    QString namedAnchor;

    struct TableIterator
    {
        inline TableIterator(QTextTable *t = 0) : table(t), row(0), column(0) {}

        inline TableIterator &operator++() {
            do {
                column += table->cellAt(row, column).columnSpan();
                if (column >= table->columns()) {
                    column = 0;
                    ++row;
                }
            } while (row < table->rows() && table->cellAt(row, column).row() != row);

            return *this;
        }

        QTextTableCell cell() const { return table->cellAt(row, column); }

        QTextTable *table;
        int row;
        int column;
    };

    struct Table
    {
        Table() : rows(0), columns(0), lastRow(-1), lastColumn(-1) {}
        QPointer<QTextTable> table;
        int rows;
        int columns;
        QPointer<QTextFrame> lastFrame;
        int lastRow, lastColumn;
        TableIterator currentPosition;
    };
    QVector<Table> tables;

    QTextDocument *doc;
    QTextCursor cursor;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
