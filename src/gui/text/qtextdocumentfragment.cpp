
#include "qtextdocumentfragment.h"
#include "qtextdocumentfragment_p.h"
#include "qtextcursor_p.h"

#include <qdebug.h>
#include <qstylesheet.h>
#include <qtextcodec.h>

QTextFormatCollectionState::QTextFormatCollectionState(const QTextHtmlParser &parser, int formatsNode)
{
    for (int formatNode = parser.findChild(formatsNode, "format");
         formatNode != -1; formatNode = parser.findNextChild(formatsNode, formatNode)) {

        int formatIdx = parser.at(formatNode).formatIndex;
        if (formatIdx == -1)
            continue;

        int typeNode = parser.findChild(formatNode, "type");
        if (typeNode == -1)
            continue;

        bool ok = false;
        int formatType = parser.at(typeNode).text.toInt(&ok);
        if (!ok)
            continue;

        QTextFormat format(formatType);

        for (int propertyNode = parser.findChild(formatNode, "property");
             propertyNode != -1; propertyNode = parser.findNextChild(formatNode, propertyNode)) {

            const QTextHtmlParserNode &node = parser.at(propertyNode);
            int propId = node.propertyId;
            if (propId < 0)
                continue;

            QString text = node.text;

            switch (stringToPropertyType(node.propertyType)) {
                case QTextFormat::Undefined:
                    break;
                case QTextFormat::Bool:
                    format.setProperty(propId, (text.toLower() == "true" ? true : false));
                    break;
                case QTextFormat::Integer:
                    format.setProperty(propId, text.toInt());
                    break;
                case QTextFormat::Float:
                    format.setProperty(propId, text.toFloat());
                    break;
                case QTextFormat::String:
                    format.setProperty(propId, text);
                    break;
                case QTextFormat::FormatGroup:
                    Q_ASSERT(propId == QTextFormat::GroupIndex);
                    format.setGroupIndex(text.toInt());
                    break;
                default:
                    Q_ASSERT(false);
            }

        }

        formats[formatIdx] = format;
    }

    /* the formatreference in the xml we dump looks like this:
     * <FormatReference ... />
     * <FormatReference ... />
     * <FormatReference ... />
     *
     * The html parser however puts them into a tree instead of flattening them out, so
     * it looks like this when parsed:
     *
     * <FormatReference ... >
     *     <FormatReference ... >
     *         <FormatReference ... />
     *     </FormatReference>
     * </FormatReference>
     *
     * That is why we have to traverse using findChild on our current node instead of
     * doing a flat traversal like with the format or the fragment nodes!
     */
    for (int formatReferenceNode = parser.findChild(formatsNode, "formatreference");
         formatReferenceNode != -1; formatReferenceNode = parser.findChild(formatReferenceNode, "formatreference")) {

        const QTextHtmlParserNode &n = parser.at(formatReferenceNode);
        references[n.formatReference] = n.formatIndex;
    }
}

QTextFormatCollectionState::QTextFormatCollectionState(const QTextFormatCollection *collection, const QList<int> &formatIndices)
{
    Q_FOREACH(int formatIdx, formatIndices) {
        QTextFormat format = collection->format(formatIdx);

        QTextFormatGroup *group = format.group();
        if (group) {
            QTextFormat groupFormat = group->commonFormat();
            Q_ASSERT(collection->hasFormatCached(groupFormat));
            int idx = const_cast<QTextFormatCollection *>(collection)->indexForFormat(groupFormat);

            formats[idx] = groupFormat;
            references[format.groupIndex()] = idx;
        }

        formats[formatIdx] = format;
    }
}

QMap<int, int> QTextFormatCollectionState::insertIntoOtherCollection(QTextFormatCollection *collection) const
{
    QMap<int, int> formatIndexMap;

    // maps from reference value used in formats to real reference index
    ReferenceMap insertedReferences;

    for (ReferenceMap::ConstIterator it = references.begin(); it != references.end(); ++it) {
        QTextFormat format = formats[it.value()];

        insertedReferences[it.key()] = collection->indexForGroup(collection->createGroup(format));
    }

    for (FormatMap::ConstIterator it = formats.begin(); it != formats.end(); ++it) {
        QTextFormat format = it.value();

        int groupIndex = format.groupIndex();
        if (groupIndex != -1) {
            groupIndex = insertedReferences.value(groupIndex, -1);
            format.setGroupIndex(groupIndex);
        }

        formatIndexMap[it.key()] = collection->indexForFormat(format);
    }

    return formatIndexMap;
}

void QTextFormatCollectionState::save(QTextStream &stream) const
{
    stream << "<Formats>" << endl;

    for (FormatMap::ConstIterator it = formats.begin(); it != formats.end(); ++it) {
        stream << "<Format index=\"" << it.key() << "\">" << endl;

        const QTextFormat &format = *it;
        stream << "<Type>" << format.type() << "</Type>" << endl;

        Q_FOREACH(int propId, format.allPropertyIds())
            if (format.propertyType(propId) != QTextFormat::Undefined) {
                stream << "<Property id=\"" << propId << "\" type=\"";
                switch (format.propertyType(propId)) {
                    case QTextFormat::Undefined:
                        Q_ASSERT(false);
                        break;
                    case QTextFormat::Bool:
                        stream << "Boolean\">" << (format.boolProperty(propId) ? QString::fromLatin1("true") : QString::fromLatin1("false"));
                        break;
                    case QTextFormat::Integer:
                        stream << "Integer\">" << format.intProperty(propId);
                        break;
                    case QTextFormat::Float:
                        stream << "Float\">" << format.floatProperty(propId);
                        break;
                    case QTextFormat::String:
                        stream << "String\">" << QStyleSheet::escape(format.stringProperty(propId));
                        break;
                    case QTextFormat::FormatGroup:
                        Q_ASSERT(propId == QTextFormat::GroupIndex);
                        stream << "FormatReference\">" << format.groupIndex();
                        break;
                }

                stream << "</Property>" << endl;
            }

        stream << "</Format>" << endl;
    }

    for (ReferenceMap::ConstIterator it = references.begin(); it != references.end(); ++it)
        stream << "<FormatReference reference=\"" << it.key() << "\" index=\"" << it.value() << "\" />" << endl;

    stream << "</Formats>" << endl;
}

QTextFormat::PropertyType QTextFormatCollectionState::stringToPropertyType(const QString &typeString)
{
    QString type = typeString.toLower();
    if (type == QLatin1String("boolean"))
        return QTextFormat::Bool;
    else if (type == QLatin1String("integer"))
        return QTextFormat::Integer;
    else if (type == QLatin1String("float"))
        return QTextFormat::Float;
    else if (type == QLatin1String("string"))
        return QTextFormat::String;
    else if (type == QLatin1String("formatreference"))
        return QTextFormat::FormatGroup;

    return QTextFormat::Undefined;
}

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate(const QTextCursor &cursor)
{
    localFormatCollection = new QTextFormatCollection;
    localFormatCollection->ref = 1;

    if (!cursor.hasSelection())
        return;

    const QTextPieceTable *p = cursor.d->pieceTable;
    pieceTable = const_cast<QTextPieceTable *>(p);
    Q_ASSERT(pieceTable);

    const int startPos = cursor.selectionStart();
    const int endPos = cursor.selectionEnd();

    Q_ASSERT(startPos < cursor.selectionEnd());

    const QString originalText = pieceTable->buffer();

    // ## varlengtharray
    QList<int> usedFormats;

    int pos = startPos;
    QTextBlockIterator currentBlock = p->blocksFind(pos);
    int remainingBlockLen = qMin(currentBlock.length() - 1, endPos - pos);

    QTextPieceTable::FragmentIterator currentFragment = p->find(pos);

    while (pos < endPos) {
        int inFragmentOffset = qMax(0, pos - currentFragment.position());

        int position = currentFragment.value()->stringPosition + inFragmentOffset;

        int size = qMin(int(currentFragment.value()->size - inFragmentOffset), endPos - pos);

        while (size > remainingBlockLen) {
            const int formatIdx = currentFragment.value()->format;
            usedFormats << formatIdx;

            appendText(QConstString(originalText.constData() + position, remainingBlockLen), formatIdx);

            pos += remainingBlockLen;
            size -= remainingBlockLen;
            position += remainingBlockLen;

            ++currentBlock;

            Q_ASSERT(currentBlock.position() == pos + 1);
            ++pos;
            --size;
            ++position;

            const int blockFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.blockFormat());
            const int charFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.charFormat());

            usedFormats << blockFormat << charFormat;

            appendBlock(blockFormat, charFormat);

            remainingBlockLen = qMin(currentBlock.length() - 1, endPos - pos);
        }

        const int formatIdx = currentFragment.value()->format;
        usedFormats << formatIdx;

        appendText(QConstString(originalText.constData() + position, size), formatIdx);
        pos += size;

        if (size == remainingBlockLen && pos < endPos) {
            ++currentBlock;
            ++pos;

            const int blockFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.blockFormat());
            const int charFormat = pieceTable->formatCollection()->indexForFormat(currentBlock.charFormat());

            usedFormats << blockFormat << charFormat;

            appendBlock(blockFormat, charFormat);

            remainingBlockLen = qMin(currentBlock.length() - 1, endPos - pos);
        }

        ++currentFragment;
    }

    QTextFormatCollectionState collState(pieceTable->formatCollection(), usedFormats);
    QMap<int, int> formatIndexMap = collState.insertIntoOtherCollection(localFormatCollection);

    for (int i = 0; i < blocks.count(); ++i) {
        Block &b = blocks[i];
        b.blockFormat = formatIndexMap.value(b.blockFormat, -1);
        b.charFormat = formatIndexMap.value(b.charFormat, -1);

        for (int i = 0; i < b.fragments.count(); ++i) {
            TextFragment &f = b.fragments[i];
            f.format = formatIndexMap.value(f.format, -1);
        }
    }
}

QTextDocumentFragmentPrivate::QTextDocumentFragmentPrivate(const QTextDocumentFragmentPrivate &rhs)
{
    localFormatCollection = 0;
    operator=(rhs);
}

QTextDocumentFragmentPrivate &QTextDocumentFragmentPrivate::operator=(const QTextDocumentFragmentPrivate &rhs)
{
    blocks = rhs.blocks;
    localBuffer = rhs.localBuffer;
    QTextFormatCollection *x = new QTextFormatCollection(*rhs.localFormatCollection);
    x->ref = 1;
    x = qAtomicSetPtr(&localFormatCollection, x);
    if (x && !--x->ref)
        delete x;

    pieceTable = rhs.pieceTable;

    return *this;
}


QTextDocumentFragmentPrivate::~QTextDocumentFragmentPrivate()
{
    if (!--localFormatCollection->ref)
        delete localFormatCollection;
}

void QTextDocumentFragmentPrivate::insert(QTextCursor &cursor) const
{
    if (cursor.isNull())
        return;

    // ###### don't do this if the fragment is a table
    if (cursor.blockFormat().tableCellEndOfRow())
        cursor.moveTo(QTextCursor::NextBlock);

    QTextFormatCollection *formats = cursor.d->pieceTable->formatCollection();
    QMap<int, int> formatIndexMap = formatCollectionState().insertIntoOtherCollection(formats);

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
            QConstString text(localBuffer.constData() + f.position, f.size);

            int formatIdx;
            if (f.format != -1)
                formatIdx = formatIndexMap.value(f.format, -1);
            else
                formatIdx = formats->indexForFormat(cursor.charFormat());

            destPieceTable->insert(cursor.position(), text, formatIdx);
        }
    }
}

QTextFormatCollectionState QTextDocumentFragmentPrivate::formatCollectionState() const
{
    QList<int> usedFormats;

    Q_FOREACH(const Block &b, blocks) {

        if (b.blockFormat != -1)
            usedFormats << b.blockFormat;

        if (b.charFormat != -1)
            usedFormats << b.charFormat;

        Q_FOREACH(const TextFragment &f, b.fragments)
            if (f.format != -1)
                usedFormats << f.format;
    }

    return QTextFormatCollectionState(localFormatCollection, usedFormats);
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
  \returns true if the fragment is empty.
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
            result += QConstString(d->localBuffer.constData() + f.position, f.size);
    }

    return result;
}

/*!
  Converts the fragment to an XML format.

  \sa fromXML
*/
QString QTextDocumentFragment::toXML() const
{
    QString result;
    QTextOStream stream(&result);
    save(stream);
    return result;
}

/*!
  Saves the fragment to a QTextStream
*/
void QTextDocumentFragment::save(QTextStream &stream) const
{
    if (!d)
        return;

    stream << "<?xml version=\"1.0\"?>" << endl
           << "<!DOCTYPE QRichText>" << endl
           << "<QRichText>" << endl;
    // ### version
    // ### rethink xml format. use more attributes?
    // ### clean up loading code, centralize read-node-text-and-turn-into-int alike
    // duplicated code

    d->formatCollectionState().save(stream);

    stream << "<Blocks>" << endl;

    Q_FOREACH(const QTextDocumentFragmentPrivate::Block &block, d->blocks) {

        stream << "<Block>" << endl; // ### save block attributes

        if (block.blockFormat != -1)
            stream << "<BlockFormatIndex>" << block.blockFormat << "</BlockFormatIndex>" << endl;
        if (block.charFormat != -1)
            stream << "<CharFormatIndex>" << block.charFormat << "</CharFormatIndex>" << endl;

        Q_FOREACH(const QTextDocumentFragmentPrivate::TextFragment &fragment, block.fragments) {
            stream << "<Fragment format=\"" << fragment.format << "\">";
            stream << QStyleSheet::escape(QConstString(d->localBuffer.constData() + fragment.position, fragment.size));
            stream << "</Fragment>" << endl;
        }

        stream << "</Block>" << endl;
    }

    stream << "</Blocks>" << endl;

    stream << "</QRichText>";
}

/*!
  Converts a fragment in XML format back to a QTextDocumentFragment.
*/
QTextDocumentFragment QTextDocumentFragment::fromXML(const QString &xml)
{
    QTextDocumentFragment res;

    QTextHtmlParser parser;
    parser.parse(xml);

    int root = 0;
    for (; root < parser.count(); ++root)
        if (parser.at(root).tag == QLatin1String("qrichtext"))
            break;

    if (root >= parser.count())
        return res;

    int formatsNode = parser.findChild(root, "formats");
    int blocksNode = parser.findChild(root, "blocks");
    if (formatsNode == -1 || blocksNode == -1)
        return res;

    QTextDocumentFragmentPrivate *d = new QTextDocumentFragmentPrivate;

    QTextFormatCollectionState collState(parser, formatsNode);
    QMap<int, int> formatIndexMap = collState.insertIntoOtherCollection(d->localFormatCollection);

    for (int blockNode = parser.findChild(blocksNode, "block");
         blockNode != -1; blockNode = parser.findNextChild(blocksNode, blockNode)) {

        QTextDocumentFragmentPrivate::Block b;

        bool ok = false;

        int blockFormatNode = parser.findChild(blockNode, "blockformatindex");
        if (blockFormatNode != -1) {
            int idx = parser.at(blockFormatNode).text.toInt(&ok);
            if (ok)
                b.blockFormat = formatIndexMap.value(idx, -1);
        }

        int charFormatNode = parser.findChild(blockNode, "charformatindex");
        if (charFormatNode != -1) {
            int idx = parser.at(charFormatNode).text.toInt(&ok);
            if (ok)
                b.charFormat = formatIndexMap.value(idx, -1);
        }

        if (b.blockFormat != -1)
            d->blocks.append(b);

        for (int fragmentNode = parser.findChild(blockNode, "fragment");
                fragmentNode != -1; fragmentNode = parser.findNextChild(blockNode, fragmentNode)) {

            const QTextHtmlParserNode &n = parser.at(fragmentNode);
            d->appendText(n.text, formatIndexMap.value(n.formatIndex, -1));
        }
    }

    res.d = d;
    return res;
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

            const int idx = listReferences.size();
            listReferences.resize(idx + 1);
            listReferences[idx] = d->localFormatCollection->indexForGroup(d->localFormatCollection->createGroup(listFmt));
        } else if (node->tag == QLatin1String("table")) {
            const int idx = tableIndices.size();
            tableIndices.resize(tableIndices.size() + 1);
            tableIndices[idx] = d->localFormatCollection->indexForGroup(d->localFormatCollection->createGroup(QTextTableFormat()));
        } else if (node->isTableCell) {
            Q_ASSERT(!tableIndices.isEmpty());

            QTextBlockFormat fmt;
            fmt.setNonDeletable(true);
            fmt.setGroupIndex(tableIndices[tableIndices.size() - 1]);
            if (node->bgColor.isValid())
                fmt.setBackgroundColor(node->bgColor);
            d->appendBlock(fmt);
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

                if (node->wsm == QStyleSheetItem::WhiteSpacePre)
                    block.setNonBreakableLines(true);

                if (node->bgColor.isValid())
                    block.setBackgroundColor(node->bgColor);

                d->appendBlock(block);
            }
        } else if (node->isImage) {
            QTextImageFormat fmt;
            fmt.setName(node->imageName);

            if (node->imageWidth >= 0)
                fmt.setWidth(node->imageWidth);
            if (node->imageHeight >= 0)
                fmt.setHeight(node->imageHeight);

            d->appendImage(fmt);
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

        d->appendText(node->text, format);
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
            Q_ASSERT(!tableIndices.isEmpty());
            QTextBlockFormat fmt;
            fmt.setNonDeletable(true);
            fmt.setGroupIndex(tableIndices[tableIndices.size() - 1]);
            fmt.setTableCellEndOfRow(true);
            d->appendBlock(fmt);
        } else if (closedNode->tag == QLatin1String("table")) {
            Q_ASSERT(!tableIndices.isEmpty());
            QTextBlockFormat fmt;
            fmt.setNonDeletable(true);
            d->appendBlock(fmt);
            tableIndices.resize(tableIndices.size() - 1);
        } else if (closedNode->isListStart) {

            Q_ASSERT(!listReferences.isEmpty());

            listReferences.resize(listReferences.size() - 1);
            --indent;
        }

        closedNode = &at(closedNode->parent);
    }
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

