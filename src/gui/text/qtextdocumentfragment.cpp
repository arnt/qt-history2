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
#include "qtextlist.h"

#include <qdebug.h>
#include <qtextcodec.h>
#include <qbytearray.h>
#include <qdatastream.h>

// ##################### TEMPORARY, UNTIL ALL OF DOCFRAG USES QTextDocument
static bool useUglyContainsCompleteDocumentHack = false;
static bool uglyContainsCompleteDocumentHack = false;

QTextImportHelper::QTextImportHelper(QTextDocumentFragmentPrivate *docFragment, QTextDocumentPrivate *priv)
    : formatCollection(docFragment->formatCollection), originalText(priv->buffer())
{
    this->docFragment = docFragment;
    this->priv = priv;
}

int QTextImportHelper::convertFormatIndex(const QTextFormat &oldFormat, int objectIndexToSet)
{
    QTextFormat fmt = oldFormat;
    if (objectIndexToSet != -1) {
        fmt.setObjectIndex(objectIndexToSet);
    } else if (fmt.objectIndex() != -1) {
        int newObjectIndex = objectIndexMap.value(fmt.objectIndex(), -1);
        if (newObjectIndex == -1) {
            QTextFormat objFormat = priv->formatCollection()->objectFormat(fmt.objectIndex());
            Q_ASSERT(objFormat.objectIndex() == -1);
            newObjectIndex = formatCollection.createObjectIndex(objFormat);
            objectIndexMap.insert(fmt.objectIndex(), newObjectIndex);
        }
        fmt.setObjectIndex(newObjectIndex);
    }
    return formatCollection.indexForFormat(fmt);
}

int QTextImportHelper::appendFragment(int pos, int endPos, int objectIndex)
{
    QTextDocumentPrivate::FragmentIterator fragIt = priv->find(pos);
    const QTextFragmentData * const frag = fragIt.value();

    Q_ASSERT(objectIndex == -1
             || (frag->size == 1 && priv->formatCollection()->format(frag->format).objectIndex() != -1));

    const int charFormatIndex = convertFormatIndex(frag->format, objectIndex);

    const int inFragmentOffset = qMax(0, pos - fragIt.position());
    int charsToCopy = qMin(int(frag->size - inFragmentOffset), endPos - pos);

    QTextBlock nextBlock = priv->blocksFind(pos + 1);

    int blockIdx = -2;
    if (nextBlock.position() == pos + 1) {
        blockIdx = convertFormatIndex(nextBlock.blockFormat());
    } else if (pos == 0 && docFragment->containsCompleteDocument) {
        docFragment->appendText(QString(QChar::ParagraphSeparator),
                                convertFormatIndex(priv->blocksBegin().charFormat()),
                                convertFormatIndex(priv->blocksBegin().blockFormat()));
    }

    docFragment->appendText(QString(originalText.constData() + frag->stringPosition + inFragmentOffset, charsToCopy),
                            charFormatIndex, blockIdx);
    return charsToCopy;
}

void QTextImportHelper::appendFragments(int pos, int endPos)
{
    Q_ASSERT(pos < endPos);

    while (pos < endPos)
        pos += appendFragment(pos, endPos);
}

// #######################################################################################

QTextImportHelper2::QTextImportHelper2(QTextDocument *_doc, QTextDocumentPrivate *_priv)
    : doc(_doc), formatCollection(*_doc->docHandle()->formatCollection()), originalText(_priv->buffer())
{
    dst = doc->docHandle();
    insertPos = 0;
    priv = _priv;
}

int QTextImportHelper2::convertFormatIndex(const QTextFormat &oldFormat, int objectIndexToSet)
{
    QTextFormat fmt = oldFormat;
    if (objectIndexToSet != -1) {
        fmt.setObjectIndex(objectIndexToSet);
    } else if (fmt.objectIndex() != -1) {
        int newObjectIndex = objectIndexMap.value(fmt.objectIndex(), -1);
        if (newObjectIndex == -1) {
            QTextFormat objFormat = priv->formatCollection()->objectFormat(fmt.objectIndex());
            Q_ASSERT(objFormat.objectIndex() == -1);
            newObjectIndex = formatCollection.createObjectIndex(objFormat);
            objectIndexMap.insert(fmt.objectIndex(), newObjectIndex);
        }
        fmt.setObjectIndex(newObjectIndex);
    }
    int idx = formatCollection.indexForFormat(fmt);
    Q_ASSERT(formatCollection.format(idx).type() == oldFormat.type());
    return idx;
}

int QTextImportHelper2::appendFragment(int pos, int endPos, int objectIndex)
{
    QTextDocumentPrivate::FragmentIterator fragIt = priv->find(pos);
    const QTextFragmentData * const frag = fragIt.value();

    Q_ASSERT(objectIndex == -1
             || (frag->size == 1 && priv->formatCollection()->format(frag->format).objectIndex() != -1));

    const int charFormatIndex = convertFormatIndex(frag->format, objectIndex);

    const int inFragmentOffset = qMax(0, pos - fragIt.position());
    int charsToCopy = qMin(int(frag->size - inFragmentOffset), endPos - pos);

    QTextBlock nextBlock = priv->blocksFind(pos + 1);

    int blockIdx = -2;
    if (nextBlock.position() == pos + 1) {
        blockIdx = convertFormatIndex(nextBlock.blockFormat());
    } else if (pos == 0 && containsCompleteDocument) {
        dst->insertBlock(insertPos,
                         convertFormatIndex(priv->blocksBegin().blockFormat()),
                         convertFormatIndex(priv->blocksBegin().charFormat()));
        ++insertPos;
    }

    QString txtToInsert(originalText.constData() + frag->stringPosition + inFragmentOffset, charsToCopy);
    if (txtToInsert.length() == 1
        && (txtToInsert.at(0) == QChar::ParagraphSeparator
            || txtToInsert.at(0) == QTextBeginningOfFrame
            || txtToInsert.at(0) == QTextEndOfFrame
           )
       ) {
        dst->insertBlock(txtToInsert.at(0), insertPos, blockIdx, charFormatIndex);
        ++insertPos;
    } else {
        dst->insert(insertPos, txtToInsert, charFormatIndex);
        insertPos += txtToInsert.length();
    }

    return charsToCopy;
}

void QTextImportHelper2::appendFragments(int pos, int endPos)
{
    Q_ASSERT(pos < endPos);

    while (pos < endPos)
        pos += appendFragment(pos, endPos);
}

static QTextDocument *cloneSelection(QTextCursor cursor, QTextDocumentPrivate *priv, bool wholeDoc)
{
    QTextDocument *doc = new QTextDocument;

    doc->docHandle()->beginEditBlock();

    QTextImportHelper2 importHelper(doc, priv);
    importHelper.containsCompleteDocument = wholeDoc;

    if (cursor.hasComplexSelection()) {
        QTextTable *table = cursor.currentTable();
        int row_start, col_start, num_rows, num_cols;
        cursor.selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

        QTextTableFormat tableFormat = table->format();
        tableFormat.setColumns(num_cols);
        tableFormat.clearColumnWidthConstraints();
        const int objectIndex = doc->docHandle()->formatCollection()->createObjectIndex(tableFormat);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                const int rspan = cell.rowSpan();
                const int cspan = cell.columnSpan();
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
                QTextCharFormat cellFormat = cell.format();
                if (r + rspan >= row_start + num_rows) {
                    cellFormat.setTableCellRowSpan(row_start + num_rows - r);
                }
                if (c + cspan >= col_start + num_cols) {
                    cellFormat.setTableCellColumnSpan(col_start + num_cols - c);
                }
                const int charFormatIndex = importHelper.convertFormatIndex(cellFormat, objectIndex);

                int blockIdx = -2;
                const int cellPos = cell.firstPosition();
                QTextBlock block = priv->blocksFind(cellPos);
                if (block.position() == cellPos) {
                    blockIdx = importHelper.convertFormatIndex(block.blockFormat());
                }

                //appendText(QString(QTextBeginningOfFrame), charFormatIndex, blockIdx);
                doc->docHandle()->insertBlock(QTextBeginningOfFrame, importHelper.insertPos, blockIdx, charFormatIndex);
                ++importHelper.insertPos;

                // nothing to add for empty cells
                if (cell.lastPosition() > cellPos) {
                    // add the contents
                    importHelper.appendFragments(cellPos, cell.lastPosition());
                }
            }
        }

        // add end of table
        int end = table->lastPosition();
        importHelper.appendFragment(end, end+1, objectIndex);
    } else {
        importHelper.appendFragments(cursor.selectionStart(), cursor.selectionEnd());
    }

    doc->docHandle()->endEditBlock();

    return doc;
}

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate(const QTextCursor &_cursor)
    : hasTitle(false), containsCompleteDocument(false), setMarkerForHtmlExport(false)
{
    if (!_cursor.hasSelection())
        return;

    if (_cursor.selectionStart() == 0 && _cursor.selectionEnd() == _cursor.d->priv->length() - 1) {
        containsCompleteDocument = true;
        rootFrameFormat = _cursor.d->priv->rootFrame()->frameFormat();
    }

    if (useUglyContainsCompleteDocumentHack)
        containsCompleteDocument = uglyContainsCompleteDocumentHack;

    // ####
    QTextDocument *tmpDoc = cloneSelection(_cursor, _cursor.d->priv, containsCompleteDocument);
    QTextCursor cursor(tmpDoc);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QTextDocumentPrivate *priv = cursor.d->priv;
    QTextImportHelper importHelper(this, priv);

    if (cursor.hasComplexSelection()) {
        QTextTable *table = cursor.currentTable();
        int row_start, col_start, num_rows, num_cols;
        cursor.selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

        QTextTableFormat tableFormat = table->format();
        tableFormat.setColumns(num_cols);
        tableFormat.clearColumnWidthConstraints();
        const int objectIndex = formatCollection.createObjectIndex(tableFormat);

        Q_ASSERT(row_start != -1);
        for (int r = row_start; r < row_start + num_rows; ++r) {
            for (int c = col_start; c < col_start + num_cols; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                const int rspan = cell.rowSpan();
                const int cspan = cell.columnSpan();
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
                QTextCharFormat cellFormat = cell.format();
                if (r + rspan >= row_start + num_rows) {
                    cellFormat.setTableCellRowSpan(row_start + num_rows - r);
                }
                if (c + cspan >= col_start + num_cols) {
                    cellFormat.setTableCellColumnSpan(col_start + num_cols - c);
                }
                const int charFormatIndex = importHelper.convertFormatIndex(cellFormat, objectIndex);

                int blockIdx = -2;
                const int cellPos = cell.firstPosition();
                QTextBlock block = priv->blocksFind(cellPos);
                if (block.position() == cellPos) {
                    blockIdx = importHelper.convertFormatIndex(block.blockFormat());
                }

                appendText(QString(QTextBeginningOfFrame), charFormatIndex, blockIdx);

                // nothing to add for empty cells
                if (cell.lastPosition() > cellPos) {
                    // add the contents
                    importHelper.appendFragments(cellPos, cell.lastPosition());
                }
            }
        }

        // add end of table
        int end = table->lastPosition();
        importHelper.appendFragment(end, end+1, objectIndex);
    } else {
        importHelper.appendFragments(cursor.selectionStart(), cursor.selectionEnd());
    }

    delete tmpDoc;
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

        if (setMarkerForHtmlExport
            && (i == 0 || i == fragments.count() - 1)) {

            QTextCharFormat fmt = formats->charFormat(formatIdx);

            int flag = 0;
            if (i == 0)
                flag |= FragmentStart;
            if (i == fragments.count() - 1)
                flag |= FragmentEnd;

            fmt.setProperty(QTextFormat::DocumentFragmentMark, flag);
            formatIdx = formats->indexForFormat(fmt);
        }

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
        QTextCharFormat charFmt = formats->charFormat(destPieceTable->find(0)->format);
        if (charFmt.objectIndex() == -1) {
            c.setBlockCharFormat(charFmt);
            c.deleteChar();
        }
    }

    if (containsCompleteDocument)
        destPieceTable->rootFrame()->setFrameFormat(rootFrameFormat);

    // ### UNDO
    if (hasTitle)
        destPieceTable->document()->setMetaInformation(QTextDocument::DocumentTitle, title);

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
    \brief The QTextDocumentFragment class represents a piece of formatted text
    from a QTextDocument.

    \ingroup text

    A QTextDocumentFragment is a fragment of rich text, that can be inserted into
    a QTextDocument. A document fragment can be created from a
    QTextDocument, from a QTextCursor's selection, or from another
    document fragment. Document fragments can also be created by the
    static functions, fromPlainText() and fromHTML().

    The contents of a document fragment can be obtained as plain text
    by using the toPlainText() function, or it can be obtained as HTML
    with toHtml().
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
QTextDocumentFragment::QTextDocumentFragment(const QTextDocument *document)
    : d(0)
{
    if (!document)
        return;

    QTextCursor cursor(const_cast<QTextDocument *>(document));
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    d = new QTextDocumentFragmentPrivate(cursor);
}

/*!
    Creates a QTextDocumentFragment from the \a{cursor}'s selection.
    If the cursor doesn't have a selection, the created fragment is empty.

    \sa isEmpty() QTextCursor::selection()
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextCursor &cursor)
    : d(0)
{
    if (!cursor.hasSelection())
        return;

    d = new QTextDocumentFragmentPrivate(cursor);
}

/*!
    \fn QTextDocumentFragment::QTextDocumentFragment(const QTextDocumentFragment &other)

    Copy constructor. Creates a copy of the \a other fragment.
*/
QTextDocumentFragment::QTextDocumentFragment(const QTextDocumentFragment &rhs)
    : d(0)
{
    (*this) = rhs;
}

/*!
    \fn QTextDocumentFragment &QTextDocumentFragment::operator=(const QTextDocumentFragment &other)

    Assigns the \a other fragment to this fragment.
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
    Destroys the document fragment.
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

// pull in from qtextdocument.cpp
void qt_replace_special_text_characters(QString *text);

/*!
    Returns the document fragment's text as plain text (i.e. with no
    formatting information).

    \sa toHtml()
*/
QString QTextDocumentFragment::toPlainText() const
{
    if (!d)
        return QString();

    QString result = d->localBuffer;

    // if we have a complete document that contains the initial paragraph
    // at the beginning then we don't want to see that one in the plaintext
    // output, as otherwise all plaintext output would always start with a
    // newline
    if (d->containsCompleteDocument
        && !result.isEmpty()
        && result.at(0) == QChar::ParagraphSeparator)
        result.remove(0, 1);

    qt_replace_special_text_characters(&result);
    return result;
}

/*!
    Returns the contents of the document fragment as HTML.

    \sa toPlainText()
*/
QString QTextDocumentFragment::toHtml() const
{
    if (!d)
        return QString();

    QTextDocument doc;
    QTextCursor cursor(&doc);

    d->setMarkerForHtmlExport = (d->containsCompleteDocument == false);
    cursor.insertFragment(*this);
    d->setMarkerForHtmlExport = false;
    return doc.toHtml();
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

    bool seenCRLF = false;

    int textStart = 0;
    for (int i = 0; i < plainText.length(); ++i) {
        QChar ch = plainText.at(i);
        if (ch == QLatin1Char('\n')
            || ch == QChar::ParagraphSeparator) {

            const int textEnd = (seenCRLF ? i - 1 : i);

            if (textEnd > textStart)
                res.d->appendText(QString::fromRawData(plainText.unicode() + textStart, textEnd - textStart), -1);

            textStart = i + 1;
            res.d->appendText(QString(QChar::ParagraphSeparator), -1, -1);

            seenCRLF = false;
        } else if (ch == QLatin1Char('\r')
                   && (i + 1) < plainText.length()
                   && plainText.at(i + 1) == QLatin1Char('\n')) {
            seenCRLF = true;
        }
    }
    if (textStart < plainText.length())
        res.d->appendText(QString::fromRawData(plainText.unicode() + textStart, plainText.length() - textStart), -1);

    return res;
}

QTextHTMLImporter::QTextHTMLImporter(QTextDocument *_doc, const QString &html)
    : hasTitle(false), indent(0), setNamedAnchorInNextOutput(false), doc(_doc)
{
    cursor = QTextCursor(doc);
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
    bool hasBlock = true;
    bool forceBlockMerging = false;
    for (int i = 0; i < count(); ++i) {
        const QTextHtmlParserNode *node = &at(i);

        /*
         * process each node in three stages:
         * 1) check if the hierarchy changed and we therefore passed the
         *    equivalent of a closing tag -> we may need to finish off
         *    some structures like tables
         *
         * 2) check if the current node is a special node like a
         *    <table>, <ul> or <img> tag that requires special processing
         *
         * 3) if the node should result in a QTextBlock create one and
         *    finally insert text that may be attached to the node
         */

        /* emit 'closing' table blocks or adjust current indent level
         * if we
         *  1) are beyond the first node
         *  2) the current node not being a child of the previous node
         *      means there was a tag closing in the input html
         */
        if (i > 0 && (node->parent != i - 1)) {
            const bool blockTagClosed = closeTag(i);
            if (hasBlock && blockTagClosed)
                hasBlock = false;

            // make sure there's a block for 'Blah' after <ul><li>foo</ul>Blah
            if (blockTagClosed
                && !hasBlock
                && !node->isBlock
                && !node->text.isEmpty()
                && node->displayMode != QTextHtmlElement::DisplayNone) {

                QTextBlockFormat block = node->blockFormat();
                block.setIndent(indent);

                appendBlock(block, node->charFormat());

                hasBlock = true;
            }
        }

        if (node->displayMode == QTextHtmlElement::DisplayNone) {
            if (node->id == Html_title) {
                hasTitle = true;
                title = node->text;
            }
            // ignore explicitly 'invisible' elements
            continue;
        } else if (node->id == Html_body) {
            containsCompleteDocument = true;
            if (node->bgColor.isValid()) {
                QTextFrameFormat fmt;
                fmt.setBackground(node->bgColor);
                doc->rootFrame()->setFrameFormat(fmt);
                const_cast<QTextHtmlParserNode *>(node)->bgColor = QColor();
            }
        } else if (node->isListStart) {

            QTextListFormat::Style style = node->listStyle;

            if (node->id == Html_ul && !node->hasOwnListStyle && node->parent) {
                const QTextHtmlParserNode *n = &at(node->parent);
                while (n) {
                    if (n->id == Html_ul) {
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
            if (node->hasCssListIndent)
                listFmt.setIndent(node->cssListIndent);
            else
                listFmt.setIndent(indent);

            List l;
            l.format = listFmt;
            lists.append(l);

            if (node->text.isEmpty())
                continue;
        } else if (node->id == Html_table) {
            Table t;
            if (scanTable(i, &t)) {
                tables.append(t);
                hasBlock = false;
            }
            continue;
        } else if (node->id == Html_tr && !tables.isEmpty()) {
            continue;
        } else if (node->id == Html_img) {
            QTextImageFormat fmt;
            fmt.setName(node->imageName);

            QTextCharFormat nodeFmt = node->charFormat();
            if (nodeFmt.hasProperty(QTextFormat::IsAnchor))
                fmt.setAnchor(nodeFmt.isAnchor());
            if (nodeFmt.hasProperty(QTextFormat::AnchorHref))
                fmt.setAnchorHref(nodeFmt.anchorHref());
            if (nodeFmt.hasProperty(QTextFormat::AnchorName))
                fmt.setAnchorName(nodeFmt.anchorName());

            if (node->imageWidth >= 0)
                fmt.setWidth(node->imageWidth);
            if (node->imageHeight >= 0)
                fmt.setHeight(node->imageHeight);

            QTextFrameFormat::Position f = QTextFrameFormat::Position(node->cssFloat);
            QTextFrameFormat ffmt;
            ffmt.setPosition(f);
            QTextObject *obj = doc->docHandle()->createObject(ffmt);
            fmt.setObjectIndex(obj->objectIndex());

            cursor.insertImage(fmt);
            hasBlock = false;
            continue;
        } else if (node->id == Html_hr) {
            QTextBlockFormat blockFormat;
            blockFormat.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth, node->width);
            appendBlock(blockFormat);
            hasBlock = false;
            continue;
        }

        if (node->isBlock) {
            QTextBlockFormat block;
            QTextCharFormat charFmt;

            if (node->isTableCell && !tables.isEmpty()) {
                Table &t = tables.last();
                if (t.table) {
                    cursor = t.currentPosition.cell().firstCursorPosition();
                }
                hasBlock = true;

                if (node->bgColor.isValid())
                    charFmt.setBackground(QBrush(node->bgColor));
            }

            if (hasBlock) {
                block = cursor.blockFormat();
                charFmt = cursor.blockCharFormat();
            }

            // collapse
            block.setTopMargin(qMax(block.topMargin(), (qreal)topMargin(i)));

            int bottomMargin = this->bottomMargin(i);

            // for list items we may want to collapse with the bottom margin of the
            // list.
            if (node->isListItem) {
                if (node->parent && at(node->parent).isListStart) {
                    const int listId = node->parent;
                    const QTextHtmlParserNode *list = &at(listId);
                    if (list->children.last() == i /* == index of node */)
                        bottomMargin = qMax(bottomMargin, this->bottomMargin(listId));
                }
            }

            block.setBottomMargin(bottomMargin);

            block.setLeftMargin(leftMargin(i));
            block.setRightMargin(rightMargin(i));

            if (!node->isListItem
                && indent != 0
                && (lists.isEmpty()
                    || !hasBlock
                    || !lists.last().list
                    || lists.last().list->itemNumber(cursor.block()) == -1
                   )
               )
               block.setIndent(indent);

            block.merge(node->blockFormat());
            charFmt.merge(node->charFormat());

            // ####################
//                block.setFloatPosition(node->cssFloat);

            if (node->wsm == QTextHtmlParserNode::WhiteSpacePre)
                block.setNonBreakableLines(true);

            if (node->bgColor.isValid())
                block.setBackground(QBrush(node->bgColor));

            if (hasBlock && (!node->isEmptyParagraph || forceBlockMerging)) {
                if (cursor.position() == 0)
                    containsCompleteDocument = true;
                cursor.setBlockFormat(block);
                cursor.setBlockCharFormat(charFmt);
            } else {
                appendBlock(block, charFmt);
            }

            if (node->isListItem && !lists.isEmpty()) {
                List &l = lists.last();
                if (l.list) {
                    l.list->add(cursor.block());
                } else {
                    l.list = cursor.createList(l.format);
                }
            }

            forceBlockMerging = false;
            if (node->id == Html_body || node->id == Html_html)
                forceBlockMerging = true;

            if (node->isEmptyParagraph) {
                hasBlock = false;
                continue;
            }

            hasBlock = true;
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

    if (lists.size() || tables.size())
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
        if (closedNode->id == Html_tr && !tables.isEmpty()) {
            blockTagClosed = true;
        } else if (closedNode->id == Html_table && !tables.isEmpty()) {
            Table &t = tables.last();
            if (QTextTable *parentTable = qobject_cast<QTextTable *>(t.lastFrame)) {
                cursor = parentTable->cellAt(t.lastRow, t.lastColumn).lastCursorPosition();
            } else {
                cursor = t.lastFrame->lastCursorPosition();
            }

            tables.resize(tables.size() - 1);
            // we don't need an extra block after tables, so we don't
            // claim to have closed one for the creation of a new one
            // in import()
            blockTagClosed = false;
        } else if (closedNode->isTableCell && !tables.isEmpty()) {
            Table &t = tables.last();
            if (t.table)
                ++tables.last().currentPosition;
            blockTagClosed = true;
        } else if (closedNode->isListStart) {

            Q_ASSERT(!lists.isEmpty());

            lists.resize(lists.size() - 1);
            --indent;
            blockTagClosed = true;
        } else if (closedNode->id == Html_hr || closedNode->id == Html_center) {
            blockTagClosed = true;
        }

        closedNode = &at(closedNode->parent);
        --depth;
    }

    return blockTagClosed;
}

bool QTextHTMLImporter::scanTable(int tableNodeIdx, Table *table)
{
    table->columns = 0;

    QVector<QTextLength> columnWidths;
    QVector<int> rowSpanCellsPerRow;

    int effectiveRow = 0;
    foreach (int row, at(tableNodeIdx).children) {
        if (at(row).id == Html_tr) {
            int colsInRow = 0;

            foreach (int cell, at(row).children)
                if (at(cell).isTableCell) {

                    const QTextHtmlParserNode &c = at(cell);
                    colsInRow += c.tableCellColSpan;

                    if (c.tableCellRowSpan > 1) {
                        rowSpanCellsPerRow.resize(effectiveRow + c.tableCellRowSpan + 1);

                        for (int r = effectiveRow + 1; r < effectiveRow + c.tableCellRowSpan; ++r)
                            rowSpanCellsPerRow[r]++;
                    }

                    while (columnWidths.count() < colsInRow)
                        columnWidths << c.width;
                }

            table->columns = qMax(table->columns, colsInRow + rowSpanCellsPerRow.value(effectiveRow, 0));

            ++effectiveRow;
            rowSpanCellsPerRow.append(0);
        }
    }
    table->rows = effectiveRow;

    if (table->rows == 0 || table->columns == 0)
        return false;

    QTextFrameFormat fmt;
    const QTextHtmlParserNode &node = at(tableNodeIdx);
    if (node.isTableFrame) {
        // for plain text frames we set the frame margin
        // for all of top/bottom/left/right, so in the import 
        // here it doesn't matter which one we pick
        fmt.setMargin(node.uncollapsedMargin(QTextHtmlParser::MarginTop));
    } else {
        QTextTableFormat tableFmt;
        tableFmt.setCellSpacing(node.tableCellSpacing);
        tableFmt.setCellPadding(node.tableCellPadding);
        if (node.alignment)
            tableFmt.setAlignment(node.alignment);
        tableFmt.setColumns(table->columns);
        tableFmt.setColumnWidthConstraints(columnWidths);
        fmt = tableFmt;
    }

    fmt.setBorder(node.tableBorder);
    fmt.setWidth(node.width);
    fmt.setHeight(node.height);

    if (node.direction < 2)
        fmt.setLayoutDirection(Qt::LayoutDirection(node.direction));
    if (node.bgColor.isValid())
        fmt.setBackground(QBrush(node.bgColor));
    else
        fmt.clearBackground();
    fmt.setPosition(QTextFrameFormat::Position(node.cssFloat));

    table->lastFrame = cursor.currentFrame();
    if (QTextTable *parentTable = qobject_cast<QTextTable *>(table->lastFrame)) {
        QTextTableCell cell = parentTable->cellAt(cursor);
        table->lastRow = cell.row();
        table->lastColumn = cell.column();
    }

    if (node.isTableFrame) {
        cursor.insertFrame(fmt);
    } else {
        table->table = cursor.insertTable(table->rows, table->columns, fmt.toTableFormat());

        TableIterator it(table->table);
        foreach (int row, at(tableNodeIdx).children)
            if (at(row).id == Html_tr)
                foreach (int cell, at(row).children)
                    if (at(cell).isTableCell) {
                        const QTextHtmlParserNode &c = at(cell);

                        if (c.tableCellColSpan > 1 || c.tableCellRowSpan > 1)
                            table->table->mergeCells(it.row, it.column, c.tableCellRowSpan, c.tableCellColSpan);

                        ++it;
                    }

        table->currentPosition = TableIterator(table->table);
    }
    return true;
}

void QTextHTMLImporter::appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt)
{
    if (setNamedAnchorInNextOutput) {
        charFmt.setAnchor(true);
        charFmt.setAnchorName(namedAnchor);
        setNamedAnchorInNextOutput = false;
    }

    cursor.insertBlock(format, charFmt);
}

void QTextHTMLImporter::appendText(QString text, QTextCharFormat format)
{
    if (setNamedAnchorInNextOutput && !text.isEmpty()) {
        QTextCharFormat fmt = format;
        fmt.setAnchor(true);
        fmt.setAnchorName(namedAnchor);
        cursor.insertText(QString(text.at(0)), fmt);

        text.remove(0, 1);
        format.setAnchor(false);
        format.setAnchorName(QString());

        setNamedAnchorInNextOutput = false;
    }

    if (!text.isEmpty())
        cursor.insertText(text, format);
}

/*!
    \fn QTextDocumentFragment QTextDocumentFragment::fromHtml(const QString &text)

    Returns a QTextDocumentFragment based on the arbitrary piece of
    HTML in the given \a text. The formatting is preserved as much as
    possible; for example, "<b>bold</b>" will become a document
    fragment with the text "bold" with a bold character format.
*/
QTextDocumentFragment QTextDocumentFragment::fromHtml(const QString &_html)
{
    QTextDocument doc;

    QString html = _html;

    const int startFragmentPos = html.indexOf(QLatin1String("<!--StartFragment-->"));
    if (startFragmentPos != -1) {
        const int endFragmentPos = html.indexOf(QLatin1String("<!--EndFragment-->"));
        if (startFragmentPos < endFragmentPos)
            html = html.mid(startFragmentPos, endFragmentPos - startFragmentPos);
        else
            html = html.mid(startFragmentPos);

        html.prepend(QLatin1String("<meta name=\"qrichtext\" content=\"1\" />"));
    }

    QTextHTMLImporter importer(&doc, html);
    importer.import();
    useUglyContainsCompleteDocumentHack = true;
    uglyContainsCompleteDocumentHack = importer.containsCompleteDocument;
    QTextDocumentFragment res = QTextDocumentFragment(&doc);
    useUglyContainsCompleteDocumentHack = false;
    res.d->hasTitle = importer.hasTitle;
    res.d->title = importer.title;
    return res;
}
