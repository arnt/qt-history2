
#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextcursor_p.h"

#include <qdebug.h>
#include <qstylesheet.h>
#include <qtextcodec.h>
#include <qbytearray.h>
#include <qdatastream.h>

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate(const QTextCursor &cursor)
{
    if (!cursor.hasSelection())
        return;

    QTextPieceTablePointer pieceTable = cursor.d->pieceTable;

    const int startPos = cursor.selectionStart();
    const int endPos = cursor.selectionEnd();

    Q_ASSERT(startPos < cursor.selectionEnd());

    const QString originalText = pieceTable->buffer();

    QVarLengthArray<int> usedFormats;

    int pos = startPos;
    QTextBlockIterator currentBlock = pieceTable->blocksFind(pos);
    int charsLeftInCurrentBlock = qMin(currentBlock.length() - 1 - (pos - currentBlock.position()), endPos - pos);

    QTextPieceTable::FragmentIterator fragIt = pieceTable->find(pos);

    while (pos < endPos) {
        const QTextFragment * const frag = fragIt.value();

        const QChar * const fragText = originalText.constData() + frag->stringPosition;

        int inFragmentOffset = qMax(0, pos - fragIt.position());
        int charsToCopy = qMin(int(frag->size - inFragmentOffset), endPos - pos);

        // if the current fragment spans over the current block boundary we
        // need to create a new block. it's a loop actually as the fragment
        // may be larger than only one block
        while (charsToCopy > charsLeftInCurrentBlock) {
            appendText(QString::fromRawData(fragText + inFragmentOffset, charsLeftInCurrentBlock), frag->format);

            pos += charsLeftInCurrentBlock;
            charsToCopy -= charsLeftInCurrentBlock;
            inFragmentOffset += charsLeftInCurrentBlock;

            ++currentBlock;

            Q_ASSERT(currentBlock.position() == pos + 1);
            ++pos;
            --charsToCopy;
            ++inFragmentOffset;

            const int blockFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.blockFormat());
            const int charFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.charFormat());

            int idx = usedFormats.size();
            usedFormats.resize(idx + 2);
            usedFormats[idx++] = blockFormat;
            usedFormats[idx] = charFormat;

            appendBlock(blockFormat, charFormat);

            charsLeftInCurrentBlock = qMin(currentBlock.length() - 1, endPos - pos);
        }

        usedFormats.append(frag->format);

        appendText(QString::fromRawData(fragText + inFragmentOffset, charsToCopy), frag->format);
        pos += charsToCopy;

        if (charsToCopy == charsLeftInCurrentBlock && pos < endPos) {
            ++currentBlock;
            ++pos;

            const int blockFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.blockFormat());
            const int charFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.charFormat());

            int idx =  usedFormats.size();
            usedFormats.resize(idx + 2);
            usedFormats[idx++] = blockFormat;
            usedFormats[idx] = charFormat;

            appendBlock(blockFormat, charFormat);

            charsLeftInCurrentBlock = qMin(currentBlock.length() - 1, endPos - pos);
        }

        ++fragIt;
    }

    readFormatCollection(pieceTable->formatCollection(), usedFormats);
}

void QTextDocumentFragmentPrivate::insert(QTextCursor &cursor) const
{
    if (cursor.isNull())
        return;

    // ###### don't do this if the fragment is a table
    if (cursor.blockFormat().tableCellEndOfRow())
        cursor.moveTo(QTextCursor::NextBlock);

    QTextFormatCollection *formats = cursor.d->pieceTable->formatCollection();
    QMap<int, int> formatIndexMap = fillFormatCollection(formats);

    QTextPieceTablePointer destPieceTable = cursor.d->pieceTable;

    Q_FOREACH(const Block &b, blocks) {
        if (b.createBlockUponInsertion) {
            int blockFormatIdx;
            if (b.blockFormat != -1)
                blockFormatIdx = formatIndexMap.value(b.blockFormat, -1);
            else
                blockFormatIdx = formats->indexForFormat(cursor.blockFormat());

            destPieceTable->insertBlock(cursor.position(), blockFormatIdx, formats->indexForFormat(QTextCharFormat()));
        }

        Q_FOREACH(const TextFragment &f, b.fragments) {
            QString text = QString::fromRawData(localBuffer.constData() + f.position, f.size);

            int formatIdx;
            if (f.format != -1)
                formatIdx = formatIndexMap.value(f.format, -1);
            else
                formatIdx = formats->indexForFormat(cursor.charFormat());

            destPieceTable->insert(cursor.position(), text, formatIdx);
        }
    }
}

void QTextDocumentFragmentPrivate::appendBlock(int blockFormatIndex, int charFormatIndex)
{
    Block b;
    b.blockFormat = blockFormatIndex;
    b.charFormat = charFormatIndex;
    blocks.append(b);
}

void QTextDocumentFragmentPrivate::appendText(const QString &text, int formatIdx)
{
    if (blocks.isEmpty()) {
        Block b;
        b.createBlockUponInsertion = false;
        blocks.append(b);
    }

    TextFragment f;
    f.position = localBuffer.length();
    localBuffer.append(text);
    f.size = text.length();
    f.format = formatIdx;
    blocks.last().fragments.append(f);
}

// need this to make sure our copied format doesn't hold a reference
// to the original format collection anymore
static QTextFormat cloneFormat(const QTextFormat &fmt)
{
    QTextFormat result(fmt.type());
    result.merge(fmt);
    return result;
}

void QTextDocumentFragmentPrivate::readFormatCollection(const QTextFormatCollection *collection, const QVarLengthArray<int> &formatIndices)
{
    formats.clear();
    formatGroups.clear();

    for (int i = 0; i < formatIndices.size(); ++i) {
        const int formatIdx = formatIndices[i];
        if (formatIdx == -1)
            continue;

        QTextFormat format = collection->format(formatIdx);

        QTextFormatGroup *group = format.group();
        if (group) {
            QTextFormat groupFormat = group->commonFormat();
            Q_ASSERT(collection->hasFormatCached(groupFormat));
            const int idx = const_cast<QTextFormatCollection *>(collection)->indexForFormat(groupFormat);

            formats[idx] = cloneFormat(groupFormat);
            formatGroups[format.groupIndex()] = idx;
        }

        formats[formatIdx] = cloneFormat(format);
    }
}

QMap<int, int> QTextDocumentFragmentPrivate::fillFormatCollection(QTextFormatCollection *collection) const
{
    QMap<int, int> formatIndexMap;

    // maps from group index used in formats to real group index
    GroupMap insertedGroups;

    for (GroupMap::ConstIterator it = formatGroups.begin(); it != formatGroups.end(); ++it) {
        QTextFormat format = formats[it.value()];

        insertedGroups[it.key()] = collection->indexForGroup(collection->createGroup(format));
    }

    for (FormatMap::ConstIterator it = formats.begin(); it != formats.end(); ++it) {
        QTextFormat format = it.value();

        int groupIndex = format.groupIndex();
        if (groupIndex != -1) {
            groupIndex = insertedGroups.value(groupIndex, -1);
            format.setGroupIndex(groupIndex);
        }

        formatIndexMap[it.key()] = collection->indexForFormat(format);
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
    cursor.moveTo(QTextCursor::End, QTextCursor::KeepAnchor);
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
    return !d || d->blocks.isEmpty();
}

/*!
  Converts the fragment to plain text.
*/
QString QTextDocumentFragment::toPlainText() const
{
    QString result;

    if (!d)
        return result;

    for (int i = 0; i < d->blocks.count(); ++i) {
        if (i > 0)
            result += '\n';

        Q_FOREACH(const QTextDocumentFragmentPrivate::TextFragment &f, d->blocks[i].fragments)
            result += QString::fromRawData(d->localBuffer.constData() + f.position, f.size);
    }

    return result;
}

/*!
  Saves the fragment to a QDataStream.
*/
QDataStream &operator<<(QDataStream &stream, const QTextDocumentFragment &fragment)
{
    if (!fragment.d)
        return stream;

    return stream << *fragment.d;
}

/*!
  Converts a fragment in XML format back to a QTextDocumentFragment.
*/
QDataStream &operator>>(QDataStream &stream, QTextDocumentFragment &fragment)
{
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

    QStringList blocks = plainText.split(QTextParagraphSeparator);
    for (int i = 0; i < blocks.count(); ++i) {
        if (i > 0)
            res.d->appendBlock(-1, -1);
        // -1 as format idx means reuse current chat format when inserting/pasting
        res.d->appendText(blocks.at(i), -1);
    }

    return res;
}

QTextHTMLImporter::QTextHTMLImporter(QTextDocumentFragmentPrivate *_d, const QString &html)
    : d(_d), indent(0)
{
    ++formats.ref;

    parse(html);
    //dumpHtml();
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

            listReferences.append(formats.indexForGroup(formats.createGroup(listFmt)));
        } else if (node->tag == QLatin1String("table")) {
            tableIndices.append(formats.indexForGroup(formats.createGroup(QTextTableFormat())));
        } else if (node->isTableCell) {
            Q_ASSERT(!tableIndices.isEmpty());

            QTextCharFormat charFmt;
            charFmt.setNonDeletable(true);
            QTextBlockFormat fmt;
            fmt.setGroupIndex(tableIndices[tableIndices.size() - 1]);
            if (node->bgColor.isValid())
                fmt.setBackgroundColor(node->bgColor);
            appendBlock(fmt, charFmt);
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
                    block.setGroupIndex(listReferences[listReferences.size() - 1]);
                } else if (indent)
                    block.setIndent(indent);

                block.setAlignment(node->alignment);
                block.setFloatPosition(node->cssFloat);

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
            QTextFormat::FloatPosition f = node->cssFloat;
            // HTML4 compat
            if (f == QTextFormat::FloatNone) {
                if (node->alignment == Qt::AlignLeft)
                    f = QTextFormat::FloatLeft;
                else if (node->alignment == Qt::AlignRight)
                    f = QTextFormat::FloatRight;
            }
            fmt.setFloatPosition(f);

            appendImage(fmt);
            continue;
        } else if (node->tag == QLatin1String("title")) {
            // ### fixme
//            d->pieceTable->config()->title = node->text;
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

    QVarLengthArray<int> usedFormats;

    Q_FOREACH(const QTextDocumentFragmentPrivate::Block &b, d->blocks) {

        int idx = usedFormats.size();
        usedFormats.resize(usedFormats.size() + 2 + b.fragments.size());

        usedFormats[idx++] = b.blockFormat;
        usedFormats[idx++] = b.charFormat;

        Q_FOREACH(const QTextDocumentFragmentPrivate::TextFragment &f, b.fragments)
            usedFormats[idx++] = f.format;
    }

    d->readFormatCollection(&formats, usedFormats);
}

void QTextHTMLImporter::closeTag(int i)
{
    const bool atLastNode = (i == count() - 1);
    const QTextHtmlParserNode *node = &at(i);
    const int grandParent = atLastNode ? 0 : at(node->parent).parent;
    const QTextHtmlParserNode *closedNode = &at(i - 1);

    while (closedNode->parent != grandParent || (atLastNode && closedNode != &at(0))) {
        if (closedNode->tag == QLatin1String("tr")) {
            Q_ASSERT(!tableIndices.isEmpty());
            QTextCharFormat charFmt;
            charFmt.setNonDeletable(true);
            QTextBlockFormat fmt;
            fmt.setGroupIndex(tableIndices[tableIndices.size() - 1]);
            fmt.setTableCellEndOfRow(true);
            appendBlock(fmt, charFmt);
        } else if (closedNode->tag == QLatin1String("table")) {
            Q_ASSERT(!tableIndices.isEmpty());
            QTextCharFormat charFmt;
            charFmt.setNonDeletable(true);
            QTextBlockFormat fmt;
            appendBlock(fmt, charFmt);
            tableIndices.resize(tableIndices.size() - 1);
        } else if (closedNode->isListStart) {

            Q_ASSERT(!listReferences.isEmpty());

            listReferences.resize(listReferences.size() - 1);
            --indent;
        }

        closedNode = &at(closedNode->parent);
    }
}

void QTextHTMLImporter::appendBlock(const QTextBlockFormat &format, const QTextCharFormat &charFmt)
{
    d->appendBlock(formats.indexForFormat(format), formats.indexForFormat(charFmt));
}

void QTextHTMLImporter::appendText(const QString &text, const QTextFormat &format)
{
    d->appendText(text, formats.indexForFormat(format)); 
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

