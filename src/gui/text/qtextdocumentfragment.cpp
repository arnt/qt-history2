
#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextcursor_p.h"

#include <qdebug.h>
#include <qstylesheet.h>
#include <qtextcodec.h>
#include <qbytearray.h>
#include <qdatastream.h>

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate(const QTextCursor &cursor)
    : hasTitle(false)
{
    if (!cursor.hasSelection())
        return;

    QTextDocumentPrivate *pieceTable = cursor.d->pieceTable;

    const int startPos = cursor.selectionStart();
    const int endPos = cursor.selectionEnd();

    Q_ASSERT(startPos < cursor.selectionEnd());

    const QString originalText = pieceTable->buffer();
    QVarLengthArray<int> usedFormats;
    int pos = startPos;

    while (pos < endPos) {
        QTextDocumentPrivate::FragmentIterator fragIt = pieceTable->find(pos);
        const QTextFragmentData * const frag = fragIt.value();

        const int inFragmentOffset = qMax(0, pos - fragIt.position());
        int charsToCopy = qMin(int(frag->size - inFragmentOffset), endPos - pos);

        QTextBlock nextBlock = pieceTable->blocksFind(pos + 1);

        int blockIdx = -2;
        if (nextBlock.position() == pos + 1) {
            blockIdx = pieceTable->formatCollection()->indexForFormat(nextBlock.blockFormat());
            usedFormats.append(blockIdx);
        }

        appendText(QString::fromRawData(originalText.constData() + frag->stringPosition + inFragmentOffset, charsToCopy), frag->format, blockIdx);
        usedFormats.append(frag->format);
        pos += charsToCopy;
    }

    formatCollection = *pieceTable->formatCollection();
}

void QTextDocumentFragmentPrivate::insert(QTextCursor &cursor) const
{
    if (cursor.isNull())
        return;

    QTextFormatCollection *formats = cursor.d->pieceTable->formatCollection();
    QMap<int, int> formatIndexMap = fillFormatCollection(formats);

    QTextDocumentPrivate *destPieceTable = cursor.d->pieceTable;
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
    \brief A fragment of text

    \ingroup text

    A QTextDocumentFragment is a fragment of rich text, that can be inserted into
    a QTextDocument, converted from and to XML.

    A QTextDocumentFragment can also be created from HTML.
*/


/*!
  Constructs an empty QTextDocumentFragment.
*/
QTextDocumentFragment::QTextDocumentFragment()
    : d(0)
{
}

/*!
  Converts a QTextDocument into a QTextDocumentFragment.
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
  Creates a QTextDocumentFragment from a selection in the cursor \a
  range. If the cursor doesn't contain a selection, the created
  fragment is empty.
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
  Returns true if the fragment is empty.
*/
bool QTextDocumentFragment::isEmpty() const
{
    return !d || d->fragments.isEmpty();
}

/*!
  Converts the fragment to plain text.
*/
QString QTextDocumentFragment::toPlainText() const
{
    QString result = d->localBuffer;

    result.replace(QTextBeginningOfFrame, QChar::ParagraphSeparator);
    result.replace(QTextEndOfFrame, QChar::ParagraphSeparator);

    return result;
}

/*!
  Saves the fragment to a QDataStream.
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
  Converts a fragment in XML format back to a QTextDocumentFragment.
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
  Converts \a plainText to a QTextDocumentFragment.
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
}

void QTextHTMLImporter::import()
{
    bool hasBlock = true;
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
            tableIndices.append(d->formatCollection.createObjectIndex(QTextTableFormat()));
        } else if (node->isTableCell) {
            Q_ASSERT(!tableIndices.isEmpty());

            QTextCharFormat charFmt;
            charFmt.setNonDeletable(true);
            QTextBlockFormat fmt;
            fmt.setObjectIndex(tableIndices[tableIndices.size() - 1]);
            if (node->bgColor.isValid())
                fmt.setBackgroundColor(node->bgColor);
            appendBlock(fmt, charFmt, QTextBeginningOfFrame);
        }

        if (node->isBlock) {
            if (hasBlock) {
                hasBlock = false;
            } else {
                QTextBlockFormat block;

                block.setTopMargin(topMargin(i));
                block.setBottomMargin(bottomMargin(i));
                block.setLeftMargin(leftMargin(i));
                block.setRightMargin(rightMargin(i));
                block.setFirstLineMargin(firstLineMargin(i));

                if (node->isListItem) {
                    Q_ASSERT(!listReferences.isEmpty());
                    block.setObjectIndex(listReferences[listReferences.size() - 1]);
                } else if (indent)
                    block.setIndent(indent);

                block.setAlignment(node->alignment);

                // ####################
//                block.setFloatPosition(node->cssFloat);

                if (node->wsm == QStyleSheetItem::WhiteSpacePre)
                    block.setNonBreakableLines(true);

                if (node->bgColor.isValid())
                    block.setBackgroundColor(node->bgColor);

                appendBlock(block);
            }
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
            continue;
        } else if (node->tag == QLatin1String("title")) {
              d->hasTitle = true;
              d->title = node->text;
            continue;
        }
        if (node->text.size() == 0)
            continue;
        hasBlock = false;

        QTextCharFormat format;

        format.setFontItalic(node->fontItalic);
        format.setFontUnderline(node->fontUnderline);
        format.setFontStrikeOut(node->fontStrikeOut);
        format.setFontFixedPitch(node->fontFixedPitch);
        if (node->fontFamily.size())
            format.setFontFamily(node->fontFamily);
        format.setFontPointSize(node->fontPointSize);
        format.setFontWeight(node->fontWeight);
        if (node->color.isValid())
            format.setColor(node->color);
        if (node->isAnchor) {
            format.setAnchor(true);
            format.setAnchorHref(node->anchorHref);
            format.setAnchorName(node->anchorName);
            format.setFontUnderline(true);
            format.setColor(Qt::blue); // ### use css
        }

        appendText(node->text, format);
    }

    if (listReferences.size() || tableIndices.size())
        closeTag(count() - 1);

}

void QTextHTMLImporter::closeTag(int i)
{
    const bool atLastNode = (i == count() - 1);
    const QTextHtmlParserNode *node = &at(i);
    const int grandParent = atLastNode ? 0 : at(node->parent).parent;
    const QTextHtmlParserNode *closedNode = &at(i - 1);

    while (closedNode->parent != grandParent || (atLastNode && closedNode != &at(0))) {
        if (closedNode->tag == QLatin1String("tr")) {
            // ################### Fix table columns!
#if 0
            Q_ASSERT(!tableIndices.isEmpty());
            QTextCharFormat charFmt;
            charFmt.setNonDeletable(true);
            QTextBlockFormat fmt;
            fmt.setObjectIndex(tableIndices[tableIndices.size() - 1]);
//             fmt.setTableCellEndOfRow(true);
            appendBlock(fmt, charFmt);
#endif
            ;
        } else if (closedNode->tag == QLatin1String("table")) {
            Q_ASSERT(!tableIndices.isEmpty());
            QTextCharFormat charFmt;
            charFmt.setNonDeletable(true);
            QTextBlockFormat fmt;
            appendBlock(fmt, charFmt, QTextEndOfFrame);
            tableIndices.resize(tableIndices.size() - 1);
        } else if (closedNode->isListStart) {

            Q_ASSERT(!listReferences.isEmpty());

            listReferences.resize(listReferences.size() - 1);
            --indent;
        }

        closedNode = &at(closedNode->parent);
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
  Converts a piece of HTML into a QTextDocumentFragment.
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

  Converts a piece of HTML into a QTextDocumentFragment.
*/
QTextDocumentFragment QTextDocumentFragment::fromHTML(const QByteArray &html)
{
    QTextCodec *codec = QTextHtmlParser::codecForStream(html);
    QString unicode = codec->toUnicode(html);
    return fromHTML(unicode);
}

