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
#include "qatomic.h"

#include <qlist.h>
#include <qmap.h>
#include <qpointer.h>
#include <qvarlengtharray.h>
#include <qdatastream.h>

class QTextDocumentFragmentPrivate;

class QTextCopyHelper
{
public:
    QTextCopyHelper(const QTextCursor &_source, const QTextCursor &_destination, bool forceCharFormat = false, const QTextCharFormat &fmt = QTextCharFormat());

    void copy();

private:
    void appendFragments(int pos, int endPos);
    int appendFragment(int pos, int endPos, int objectIndex = -1);
    int convertFormatIndex(const QTextFormat &oldFormat, int objectIndexToSet = -1);
    inline int convertFormatIndex(int oldFormatIndex, int objectIndexToSet = -1)
    { return convertFormatIndex(src->formatCollection()->format(oldFormatIndex), objectIndexToSet); }
    inline QTextFormat convertFormat(const QTextFormat &fmt)
    { return dst->formatCollection()->format(convertFormatIndex(fmt)); }

    int insertPos;

    bool forceCharFormat;
    int primaryCharFormatIndex;

    QTextCursor cursor;
    QTextDocumentPrivate *dst;
    QTextDocumentPrivate *src;
    QTextFormatCollection &formatCollection;
    const QString originalText;
    QMap<int, int> objectIndexMap;
};

class QTextDocumentFragmentPrivate
{
public:
    QTextDocumentFragmentPrivate() : ref(1), doc(0), containsCompleteDocument(false), importedFromPlainText(false) {}
    QTextDocumentFragmentPrivate(const QTextCursor &cursor);
    inline ~QTextDocumentFragmentPrivate() { delete doc; }

    void insert(QTextCursor &cursor) const;

    QAtomic ref;
    QTextDocument *doc;

    uint containsCompleteDocument : 1;
    uint importedFromPlainText : 1;
private:
    Q_DISABLE_COPY(QTextDocumentFragmentPrivate);
};

class QTextHtmlImporter : public QTextHtmlParser
{
    struct Table;
public:
    QTextHtmlImporter(QTextDocument *_doc, const QString &html);

    void import();

    bool containsCompleteDocument() const { return containsCompleteDoc; }

private:
    bool closeTag(int i);

    bool scanTable(int tableNodeIdx, Table *table);

    void appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt = QTextCharFormat());

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

        inline bool atEnd() const { return table == 0 || row >= table->rows(); }

        QTextTableCell cell() const { return table->cellAt(row, column); }

        QTextTable *table;
        int row;
        int column;
    };

    struct Table
    {
        Table() : rows(0), columns(0), lastRow(-1), lastColumn(-1), currentRow(0) {}
        QPointer<QTextTable> table;
        int rows;
        int columns;
        QPointer<QTextFrame> lastFrame;
        int lastRow, lastColumn;
        int currentRow;
        TableIterator currentPosition;
    };
    QVector<Table> tables;

    QTextDocument *doc;
    QTextCursor cursor;
    bool containsCompleteDoc;
};

#endif // QTEXTDOCUMENTFRAGMENT_P_H
