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
    const QString originalText = priv->buffer();

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

    appendText(QString::fromRawData(originalText.constData() + frag->stringPosition + inFragmentOffset, charsToCopy),
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

        QString text = QString::fromRawData(localBuffer.constData() + f.position, f.size);

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
    if (documentWasEmpty && firstFragmentWasBlock)
        destPieceTable->remove(0, 1);

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
    \brief The QTextDocumentFragment class represents a piece of rich
    text formatted text.

    \ingroup text

    A QTextDocumentFragment is a fragment of rich text, that can be inserted into
    a QTextDocument. A document fragment can be created from a
    QTextDocument, from a QTextCursor's selection, or from another
    document fragment. Document fragments can also be created by the
    static functions, fromPlainText() and fromHTML().

    A document fragment is represented by a rich text format that can
    be converted to and from XML by writing to or reading from a
    QDataStream using operator<<() and operator>>().

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
    Creates a QTextDocumentFragment from the cursor \a{range}'s
    selection. If the cursor doesn't contain a selection, the created
    fragment is empty.

    \sa isEmpty()
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextCursor &range)
    : d(0)
{
    if (!range.hasSelection())
        return;

    d = new QTextDocumentFragmentPrivate(range);
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

    Saves the given \a fragment as rich text to the \a stream in an
    XML format.
*/
QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragment &fragment)
{
    if (!fragment.d) {
        // null marker
        return stream << Q_INT8(false);
    }

    return stream << Q_INT8(true) << *fragment.d;
}

/*!
    \relates QTextDocumentFragment

    Reads a \a stream containing a document fragment in XML format,
    and populates \a fragment with the rich text that has been read.
*/
QDataStream &operator>>(QDataStream &stream, QTextDocumentFragment &fragment)
{
    Q_INT8 marker;

    stream >> marker;

    if (marker == false) {
        delete fragment.d;
        fragment.d = 0;
        return stream;
    }

    if (!fragment.d)
        fragment.d = new QTextDocumentFragmentPrivate;

    return stream >> *fragment.d;
}

/*!
    Returns a document fragment that contains the given \a plainText.
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
    : d(_d), indent(0)
{
    parse(html);
//    dumpHtml();
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
        if (i > 0 && (node->parent != i - 1) && (i < count() - 1))
            closeTag(i);

        if (node->isListStart) {
            QTextListFormat listFmt;
            listFmt.setStyle(node->listStyle);

            ++indent;
            listFmt.setIndent(indent);

            listReferences.append(d->formatCollection.createObjectIndex(listFmt));
        } else if (node->tag == QLatin1String("table")) {
            QTextTableFormat fmt;
            fmt.setBorder(1);
            Table t;
            t.tableIndex = d->formatCollection.createObjectIndex(fmt);
            tables.append(t);
            hasBlock = false;
        } else if (node->isTableCell) {
            Q_ASSERT(!tables.isEmpty());

            QTextCharFormat charFmt;
            charFmt.setObjectIndex(tables[tables.size() - 1].tableIndex);
            tables[tables.size() -1].currentColumnCount++;

            QTextBlockFormat fmt;
            if (node->bgColor.isValid())
                fmt.setBackgroundColor(node->bgColor);
            appendBlock(fmt, charFmt, QTextBeginningOfFrame);
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

            if (node->wsm == QStyleSheetItem::WhiteSpacePre)
                block.setNonBreakableLines(true);

            if (node->bgColor.isValid())
                block.setBackgroundColor(node->bgColor);

            if (hasBlock) {
                d->fragments.last().blockFormat = d->formatCollection.indexForFormat(block);
                d->fragments.last().charFormat = d->formatCollection.indexForFormat(node->charFormat());
            } else {
                appendBlock(block, node->charFormat());
            }
            hasBlock = true;
        } else if (node->isImage) {
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
        if (node->text.size() == 0)
            continue;
        hasBlock = false;

        appendText(node->text, node->charFormat());
    }

    if (listReferences.size() || tables.size())
        closeTag(count() - 1);

}

void QTextHTMLImporter::closeTag(int i)
{
    const bool atLastNode = (i == count() - 1);
    const QTextHtmlParserNode *closedNode = &at(i - 1);
    const int endDepth = atLastNode ? - 1 : depth(i) - 1;
    int depth = this->depth(i - 1);

    while (depth > endDepth) {
        if (closedNode->tag == QLatin1String("tr")) {
            Q_ASSERT(!tables.isEmpty());

            Table &t = tables[tables.size() -1];
            QTextTableFormat fmt = d->formatCollection.objectFormat(t.tableIndex).toTableFormat();
            fmt.setColumns(qMax(fmt.columns(), t.currentColumnCount));
            d->formatCollection.setObjectFormat(t.tableIndex, fmt);
            t.currentColumnCount = 0;

            // ################### Fix table columns!
#if 0
            Q_ASSERT(!tableIndices.isEmpty());
            QTextCharFormat charFmt;
            charFmt.setObjectIndex(tables[tables.size() - 1].tableIndex);
            QTextBlockFormat fmt;
            appendBlock(fmt, charFmt);
#endif
            ;
        } else if (closedNode->tag == QLatin1String("table")) {
            Q_ASSERT(!tables.isEmpty());
            QTextCharFormat charFmt;
            charFmt.setObjectIndex(tables[tables.size() - 1].tableIndex);
            QTextBlockFormat fmt;
            appendBlock(fmt, charFmt, QTextEndOfFrame);
            tables.resize(tables.size() - 1);
        } else if (closedNode->isListStart) {

            Q_ASSERT(!listReferences.isEmpty());

            listReferences.resize(listReferences.size() - 1);
            --indent;
        }

        closedNode = &at(closedNode->parent);
        --depth;
    }
}

void QTextHTMLImporter::appendBlock(const QTextBlockFormat &format, const QTextCharFormat &charFmt, const QChar &separator)
{
    d->appendText(QString(separator), d->formatCollection.indexForFormat(charFmt), d->formatCollection.indexForFormat(format));
}

void QTextHTMLImporter::appendText(const QString &text, const QTextFormat &format)
{
    d->appendText(text, d->formatCollection.indexForFormat(format));
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
*/
QTextDocumentFragment QTextDocumentFragment::fromHTML(const QByteArray &html)
{
    QTextCodec *codec = QTextHtmlParser::codecForStream(html);
    QString unicode = codec->toUnicode(html);
    return fromHTML(unicode);
}


