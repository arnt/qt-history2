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


#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextcursor_p.h"
#include "qtexttable.h"

#include <qdebug.h>
#include <qtextcodec.h>
#include <qbytearray.h>
#include <qdatastream.h>

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate(const QTextCursor &cursor)
    : hasTitle(false)
{
    if (!cursor.hasSelection())
        return;

    QTextDocumentPrivate *priv = cursor.d->priv;
    formatCollection = *priv->formatCollection();

    if (cursor.hasComplexSelection()) {
        QTextTable *table = cursor.currentTable();
        int row_start, col_start, num_rows, num_cols;
        cursor.selectedTableCells(&row_start, &num_rows, &col_start,  &num_cols);

        QTextTableFormat tableFormat = table->format();
        tableFormat.setColumns(num_cols);
        int objectIndex = formatCollection.createObjectIndex(tableFormat);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                int rspan = cell.rowSpan();
                int cspan = cell.columnSpan();
                if (rspan != 1) {
                    int cr = cell.row();
                    if (cr != r)
                        continue;
                }
                if (cspan != 1) {
                    int cc = cell.column();
                    if (cc != c)
                        continue;
                }

                // add the QTextBeginningOfFrame
                int cellPos = cell.firstPosition();
                appendFragment(priv, cellPos-1, cellPos, objectIndex);
                // add the contents
                appendFragments(priv, cellPos, cell.lastPosition());
            }
        }

        // add end of table
        int end = table->lastPosition();
        appendFragment(priv, end, end+1, objectIndex);

    } else {
        appendFragments(priv, cursor.selectionStart(), cursor.selectionEnd());
    }
}

int QTextDocumentFragmentPrivate::appendFragment(QTextDocumentPrivate *priv, int pos, int endPos, int objectIndex)
{
    const QString originalText = priv->buffer();
    QTextDocumentPrivate::FragmentIterator fragIt = priv->find(pos);
    const QTextFragmentData * const frag = fragIt.value();

    int charFormatIndex = frag->format;
    if (objectIndex != -1) {
        QTextFormat fmt = formatCollection.format(frag->format);
        Q_ASSERT(frag->size == 1 && fmt.objectIndex() != -1);
        fmt.setObjectIndex(objectIndex);
        charFormatIndex = formatCollection.indexForFormat(fmt);
    }

    const int inFragmentOffset = qMax(0, pos - fragIt.position());
    int charsToCopy = qMin(int(frag->size - inFragmentOffset), endPos - pos);

    QTextBlock nextBlock = priv->blocksFind(pos + 1);

    int blockIdx = -2;
    if (nextBlock.position() == pos + 1)
        blockIdx = formatCollection.indexForFormat(nextBlock.blockFormat());

    appendText(QString(originalText.constData() + frag->stringPosition + inFragmentOffset, charsToCopy),
               charFormatIndex, blockIdx);
    return charsToCopy;
}

void QTextDocumentFragmentPrivate::appendFragments(QTextDocumentPrivate *priv, int pos, int endPos)
{
    Q_ASSERT(pos < endPos);

    while (pos < endPos)
        pos += appendFragment(priv, pos, endPos);
}

void QTextDocumentFragmentPrivate::insert(QTextCursor &cursor) const
{
    if (cursor.isNull())
        return;

    QTextFormatCollection *formats = cursor.d->priv->formatCollection();
    QMap<int, int> formatIndexMap = fillFormatCollection(formats);

    QTextDocumentPrivate *destPieceTable = cursor.d->priv;
    destPieceTable->beginEditBlock();

    int defaultBlockFormat = formats->indexForFormat(cursor.blockFormat());
    int defaultCharFormat = formats->indexForFormat(cursor.charFormat());

    const bool documentWasEmpty = (destPieceTable->length() <= 1);
    bool firstFragmentWasBlock = false;

    for (int i = 0; i < fragments.count(); ++i) {
        const TextFragment &f = fragments.at(i);
        int blockFormatIdx = -2;
        if (f.blockFormat >= 0)
            blockFormatIdx = formatIndexMap.value(f.blockFormat, -1);
        else if (f.blockFormat == -1)
            blockFormatIdx = defaultBlockFormat;
        int formatIdx;
        if (f.charFormat != -1)
            formatIdx = formatIndexMap.value(f.charFormat, -1);
        else
            formatIdx = defaultCharFormat;

        QString text(localBuffer.constData() + f.position, f.size);

        if (blockFormatIdx == -2) {
            destPieceTable->insert(cursor.position(), text, formatIdx);
        } else {
            destPieceTable->insertBlock(text.at(0), cursor.position(), blockFormatIdx, formatIdx);
            if (i == 0)
                firstFragmentWasBlock = true;
        }
    }

    // if before the insertion the document was empty then we consider the
    // insertion as a replacement and must now also remove the initial block
    // that existed before, in case our fragment started with a block
    if (documentWasEmpty && firstFragmentWasBlock) {
        QTextCursor c = cursor;
        c.clearSelection();
        c.movePosition(QTextCursor::Start);
        c.deleteChar();
    }

    // ### UNDO
    if (hasTitle)
        destPieceTable->config()->title = title;

    destPieceTable->endEditBlock();
}

void QTextDocumentFragmentPrivate::appendText(const QString &text, int formatIdx, int blockIdx)
{
    TextFragment f;
    f.position = localBuffer.length();
    localBuffer.append(text);
    f.size = text.length();
    f.charFormat = formatIdx;
    f.blockFormat = blockIdx;
    fragments.append(f);
}

QMap<int, int> QTextDocumentFragmentPrivate::fillFormatCollection(QTextFormatCollection *collection) const
{
    QMap<int, int> formatIndexMap;

    // maps from object index used in formats to real object index
    QMap<int, int> insertedGroups;

    const QVector<int> &objFormats = formatCollection.objFormats;
    for (int i = 0; i < objFormats.size(); ++i) {
        int objFormat = objFormats.at(i);
        insertedGroups[i] = collection->createObjectIndex(formatCollection.format(objFormat));
    }

    const QVector<QTextFormat> &formats = formatCollection.formats;
    for (int i = 0; i < formats.size(); ++i) {
        QTextFormat format = formats.at(i);

        int objectIndex = format.objectIndex();
        if (objectIndex != -1) {
            objectIndex = insertedGroups.value(objectIndex, -1);
            format.setObjectIndex(objectIndex);
        }

        formatIndexMap[i] = collection->indexForFormat(format);
    }

    return formatIndexMap;
}

/*!
    \class QTextDocumentFragment qtextdocumentfragment.h
    \brief The QTextDocumentFragment class represents a piece of
    formatted text.

    \ingroup text

    A QTextDocumentFragment is a fragment of rich text, that can be inserted into
    a QTextDocument. A document fragment can be created from a
    QTextDocument, from a QTextCursor's selection, or from another
    document fragment. Document fragments can also be created by the
    static functions, fromPlainText() and fromHTML().

    A document fragment's text can be obtained by calling
    toPlainText().
*/


/*!
    Constructs an empty QTextDocumentFragment.

    \sa isEmpty()
*/
QTextDocumentFragment::QTextDocumentFragment()
    : d(0)
{
}

/*!
    Converts the given \a document into a QTextDocumentFragment.
*/
QTextDocumentFragment::QTextDocumentFragment(QTextDocument *document)
    : d(0)
{
    if (!document)
        return;

    QTextCursor cursor(document);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    d = new QTextDocumentFragmentPrivate(cursor);
}

/*!
    Creates a QTextDocumentFragment from the \a{cursor}'s selection. 
    If the cursor doesn't have a selection, the created fragment is empty.

    \sa isEmpty()
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextCursor &cursor)
    : d(0)
{
    if (!cursor.hasSelection())
        return;

    d = new QTextDocumentFragmentPrivate(cursor);
}

/*!
    Creates a copy of the fragment \a rhs.
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextDocumentFragment &rhs)
    : d(0)
{
    (*this) = rhs;
}

/*!
    Assigns \a rhs to this fragment.
*/
QTextDocumentFragment &QTextDocumentFragment::operator=(const QTextDocumentFragment &rhs)
{
    if (&rhs == this || (!d && !rhs.d))
        return *this;

    if (d && !rhs.d) {
        delete d;
        d = 0;
        return *this;
    }

    if (!d)
        d = new QTextDocumentFragmentPrivate;

    *d = *rhs.d;

    return *this;
}

/*!
    Destroys the fragment.
*/
QTextDocumentFragment::~QTextDocumentFragment()
{
    delete d;
}

/*!
    Returns true if the fragment is empty; otherwise returns false.
*/
bool QTextDocumentFragment::isEmpty() const
{
    return !d || d->fragments.isEmpty();
}

/*!
    Returns the document fragment's text as plain text (i.e. with no
    formatting information).
*/
QString QTextDocumentFragment::toPlainText() const
{
    QString result = d->localBuffer;

    result.replace(QTextBeginningOfFrame, QChar::ParagraphSeparator);
    result.replace(QTextEndOfFrame, QChar::ParagraphSeparator);

    return result;
}

/*!
    \relates QTextDocumentFragment

    Writes the fragment \a frag to the stream \a s and returns a reference to the
    stream.
*/
QDataStream &operator<<(QDataStream &s, const QTextDocumentFragment &frag)
{
    if (!frag.d) {
        // null marker
        return s << Q_INT8(false);
    }

    return s << Q_INT8(true) << *frag.d;
}

/*!
    \relates QTextDocumentFragment

    Reads the  fragment \a frag from the stream \a s and returns a reference
    to the stream.
*/
QDataStream &operator>>(QDataStream &s, QTextDocumentFragment &frag)
{
    Q_INT8 marker;

    s >> marker;

    if (marker == false) {
        delete frag.d;
        frag.d = 0;
        return s;
    }

    if (!frag.d)
        frag.d = new QTextDocumentFragmentPrivate;

    return s >> *frag.d;
}

/*!
    Returns a document fragment that contains the given \a plainText.

    When inserting such a fragment into a QTextDocument the current char format of
    the QTextCursor used for insertion is used as format for the text.
*/
QTextDocumentFragment QTextDocumentFragment::fromPlainText(const QString &plainText)
{
    QTextDocumentFragment res;

    res.d = new QTextDocumentFragmentPrivate;

    QStringList blocks = plainText.split(QChar::ParagraphSeparator);
    for (int i = 0; i < blocks.count(); ++i) {
        if (i > 0)
            res.d->appendText(QString(QChar::ParagraphSeparator), -1, -1);
        // -1 as format idx means reuse current chat format when inserting/pasting
        res.d->appendText(blocks.at(i), -1);
    }

    return res;
}

QTextHTMLImporter::QTextHTMLImporter(QTextDocumentFragmentPrivate *_d, const QString &html)
    : d(_d), indent(0), setNamedAnchorInNextOutput(false)
{
    parse(html);
//    dumpHtml();
}

static QTextListFormat::Style nextListStyle(QTextListFormat::Style style)
{
    if (style == QTextListFormat::ListDisc)
        return QTextListFormat::ListCircle;
    else if (style == QTextListFormat::ListCircle)
        return QTextListFormat::ListSquare;
    return style;
}

void QTextHTMLImporter::import()
{
    bool hasBlock = false;
    for (int i = 0; i < count(); ++i) {
        const QTextHtmlParserNode *node = &at(i);

        /* emit 'closing' table blocks or adjust current indent level
         * if we
         *  1) are beyond the first node
         *  2) the current node not being a child of the previous node
         *      means there was a tag closing in the input html
         *  3) this isn't the last node, as we deal with that at the end
         */
        if (i > 0 && (node->parent != i - 1) && (i < count() - 1)) {
            const bool blockTagClosed = closeTag(i);
            if (hasBlock && blockTagClosed)
                hasBlock = false;
        }

        if (node->tag == QLatin1String("style")) {
            // ignore the body of style tags
            continue;
        } else if (node->isListStart) {

            QTextListFormat::Style style = node->listStyle;

            if (node->id == Html_ul && !node->hasOwnListStyle && node->parent) {
                const QTextHtmlParserNode *n = &at(node->parent);
                while (n) {
                    if (n->tag == QLatin1String("ul")) {
                        style = nextListStyle(node->listStyle);
                    }
                    if (n->parent)
                        n = &at(n->parent);
                    else
                        n = 0;
                }
            }

            QTextListFormat listFmt;
            listFmt.setStyle(style);

            ++indent;
            listFmt.setIndent(indent);

            listReferences.append(d->formatCollection.createObjectIndex(listFmt));
        } else if (node->id == Html_table) {
            QTextTableFormat fmt;
            fmt.setBorder(node->tableBorder);
            Table t;
            t.tableIndex = d->formatCollection.createObjectIndex(fmt);
            tables.append(t);
            scanTable(i, &tables[tables.size() - 1]);
            hasBlock = false;
            continue;
        } else if (node->id == Html_tr) {
            continue;
        } else if (node->isTableCell) {
            Q_ASSERT(!tables.isEmpty());

            QTextCharFormat charFmt;
            charFmt.setObjectIndex(tables[tables.size() - 1].tableIndex);

            if (node->bgColor.isValid())
                charFmt.setTableCellBackgroundColor(node->bgColor);

            charFmt.setTableCellColumnSpan(node->tableCellColSpan);
            charFmt.setTableCellRowSpan(node->tableCellRowSpan);

            QTextBlockFormat fmt;
            appendBlock(fmt, charFmt, QTextBeginningOfFrame);

            tables[tables.size() - 1].currentColumnCount++;

            hasBlock = false;
        }

        if (node->isBlock) {
            QTextBlockFormat block;

            if (hasBlock) {
                Q_ASSERT(d->fragments.last().blockFormat >= 0);
                block = d->formatCollection.blockFormat(d->fragments.last().blockFormat);
            }

            block.setTopMargin(topMargin(i));
            block.setBottomMargin(bottomMargin(i));
            block.setLeftMargin(leftMargin(i));
            block.setRightMargin(rightMargin(i));
            block.setFirstLineMargin(firstLineMargin(i));

            if (node->isListItem) {
                Q_ASSERT(!listReferences.isEmpty());
                block.setObjectIndex(listReferences.last());
            } else if (indent && block.objectIndex() == listReferences.last()) {
                block.setIndent(indent);
            }
            block.setAlignment(node->alignment);

            // ####################
//                block.setFloatPosition(node->cssFloat);

            if (node->wsm == QTextHtmlParserNode::WhiteSpacePre)
                block.setNonBreakableLines(true);

            if (node->bgColor.isValid())
                block.setBackgroundColor(node->bgColor);

            QTextCharFormat charFmt = node->charFormat();

            if (hasBlock) {
                d->fragments.last().blockFormat = d->formatCollection.indexForFormat(block);
                d->fragments.last().charFormat = d->formatCollection.indexForFormat(charFmt);
            } else {
                appendBlock(block, charFmt);
            }
            hasBlock = true;
        } else if (node->id == Html_img) {
            QTextImageFormat fmt;
            fmt.setName(node->imageName);

            if (node->imageWidth >= 0)
                fmt.setWidth(node->imageWidth);
            if (node->imageHeight >= 0)
                fmt.setHeight(node->imageHeight);
            QTextFrameFormat::Position f = node->cssFloat;
            // HTML4 compat
            if (f == QTextFrameFormat::InFlow) {
                if (node->alignment == Qt::AlignLeft)
                    f = QTextFrameFormat::FloatLeft;
                else if (node->alignment == Qt::AlignRight)
                    f = QTextFrameFormat::FloatRight;
            }
            QTextFrameFormat ffmt;
            ffmt.setPosition(f);
            int objIndex = d->formatCollection.createObjectIndex(ffmt);
            fmt.setObjectIndex(objIndex);

            appendImage(fmt);
            hasBlock = false;
            continue;
        } else if (node->tag == QLatin1String("title")) {
            d->hasTitle = true;
            d->title = node->text;
            continue;
        }
        if (node->isAnchor && !node->anchorName.isEmpty()) {
            setNamedAnchorInNextOutput = true;
            namedAnchor = node->anchorName;
        }
        if (node->text.size() == 0)
            continue;
        hasBlock = false;

        appendText(node->text, node->charFormat());
    }

    if (listReferences.size() || tables.size())
        closeTag(count() - 1);

}

// returns true if a block tag was closed
bool QTextHTMLImporter::closeTag(int i)
{
    const bool atLastNode = (i == count() - 1);
    const QTextHtmlParserNode *closedNode = &at(i - 1);
    const int endDepth = atLastNode ? - 1 : depth(i) - 1;
    int depth = this->depth(i - 1);
    bool blockTagClosed = false;

    while (depth > endDepth) {
        if (closedNode->tag == QLatin1String("tr")) {
            Q_ASSERT(!tables.isEmpty());

            Table &t = tables[tables.size() -1];

            QTextCharFormat charFmt;
            charFmt.setObjectIndex(t.tableIndex);

            while (t.currentColumnCount < t.columns) {
                appendBlock(QTextBlockFormat(), charFmt, QTextBeginningOfFrame);
                ++t.currentColumnCount;
            }

            t.currentColumnCount = 0;
            blockTagClosed = true;
        } else if (closedNode->tag == QLatin1String("table")) {
            Q_ASSERT(!tables.isEmpty());
            QTextCharFormat charFmt;
            charFmt.setObjectIndex(tables[tables.size() - 1].tableIndex);
            QTextBlockFormat fmt;
            appendBlock(fmt, charFmt, QTextEndOfFrame);
            tables.resize(tables.size() - 1);
            blockTagClosed = true;
        } else if (closedNode->isListStart) {

            Q_ASSERT(!listReferences.isEmpty());

            listReferences.resize(listReferences.size() - 1);
            --indent;
            blockTagClosed = true;
        }

        closedNode = &at(closedNode->parent);
        --depth;
    }

    return blockTagClosed;
}

void QTextHTMLImporter::scanTable(int tableNodeIdx, Table *table)
{
    // ### spans
    table->columns = 0;

    QList<int> constraintTypes;
    QList<int> constraintValues;

    bool inFirstRow = true;
    foreach (int row, at(tableNodeIdx).children) {
        if (at(row).tag == QLatin1String("tr")) {
            int colsInRow = 0;

            foreach (int cell, at(row).children)
                if (at(cell).isTableCell) {
                    const QTextHtmlParserNode &c = at(cell);
                    colsInRow += c.tableCellColSpan;

                    if (inFirstRow || colsInRow > constraintTypes.count()) {
                        Q_ASSERT(colsInRow == constraintTypes.count() + c.tableCellColSpan);

                        for (int i = 0; i < c.tableCellColSpan; ++i) {
                            constraintTypes << c.tableColConstraint;
                            constraintValues << c.tableColConstraintValue;
                        }
                    }
                }

            table->columns = qMax(table->columns, colsInRow);
            inFirstRow = false;
        }
    }

    QTextTableFormat fmt = d->formatCollection.objectFormat(table->tableIndex).toTableFormat();
    fmt.setColumns(table->columns);
    fmt.setTableColumConstraints(constraintTypes, constraintValues);
    d->formatCollection.setObjectFormat(table->tableIndex, fmt);
}

void QTextHTMLImporter::appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt, const QChar &separator)
{
    if (setNamedAnchorInNextOutput) {
        charFmt.setAnchor(true);
        charFmt.setAnchorName(namedAnchor);
        setNamedAnchorInNextOutput = false;
    }

    d->appendText(QString(separator), d->formatCollection.indexForFormat(charFmt), d->formatCollection.indexForFormat(format));
}

void QTextHTMLImporter::appendText(QString text, QTextCharFormat format)
{
    if (setNamedAnchorInNextOutput && !text.isEmpty()) {
        QTextCharFormat fmt = format;
        fmt.setAnchor(true);
        fmt.setAnchorName(namedAnchor);
        d->appendText(QString(text.at(0)), d->formatCollection.indexForFormat(fmt));

        text.remove(0, 1);
        format.setAnchor(false);
        format.setAnchorName(QString::null);

        setNamedAnchorInNextOutput = false;
    }

    if (!text.isEmpty()) {
        d->appendText(text, d->formatCollection.indexForFormat(format));
    }
}

/*!
    Returns a QTextDocumentFragment based on the arbitrary piece of
    HTML in the string \a html. The formatting is preserved as much as
    possible; for example, "<b>bold</b>" will become a document
    fragment with the text "bold" with a bold character format.
*/
QTextDocumentFragment QTextDocumentFragment::fromHTML(const QString &html)
{
    QTextDocumentFragment res;
    res.d = new QTextDocumentFragmentPrivate;

    QTextHTMLImporter(res.d, html).import();
    return res;
}

/*!
    \overload

    The text encoding of the string \a html is determined from encoding
    attributes specified in the header.
*/
QTextDocumentFragment QTextDocumentFragment::fromHTML(const QByteArray &html)
{
    QTextCodec *codec = QTextHtmlParser::codecForStream(html);
    QString unicode = codec->toUnicode(html);
    return fromHTML(unicode);
}


