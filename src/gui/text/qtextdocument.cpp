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

#include "qtextdocument.h"
#include <qtextformat.h>
#include "qtextdocumentlayout_p.h"
#include "qtextdocumentfragment.h"
#include "qtexttable.h"
#include "qtextlist.h"
#include <qdebug.h>
#include <qregexp.h>
#include <qvarlengtharray.h>
#include "qtexthtmlparser_p.h"

#include "qtextdocument_p.h"
#define d d_func()
#define q q_func()

/*!
    Returns true if the string \a text is likely to be rich text;
    otherwise returns false.

    This function uses a fast and therefore simple heuristic. It
    mainly checks whether there is something that looks like a tag
    before the first line break. Although the result may be correct
    for common cases, there is no guarantee.
*/
bool QText::mightBeRichText(const QString& text)
{
    if (text.isEmpty())
        return false;
    int start = 0;

    while (start < int(text.length()) && text.at(start).isSpace())
        ++start;
    if (text.mid(start, 5).toLower() == QLatin1String("<!doc"))
        return true;
    int open = start;
    while (open < int(text.length()) && text.at(open) != '<'
            && text.at(open) != '\n') {
        if (text.at(open) == '&' &&  text.mid(open+1,3) == "lt;")
            return true; // support desperate attempt of user to see <...>
        ++open;
    }
    if (open < (int)text.length() && text.at(open) == '<') {
        int close = text.indexOf('>', open);
        if (close > -1) {
            QString tag;
            for (int i = open+1; i < close; ++i) {
                if (text[i].isDigit() || text[i].isLetter())
                    tag += text[i];
                else if (!tag.isEmpty() && text[i].isSpace())
                    break;
                else if (!text[i].isSpace() && (!tag.isEmpty() || text[i] != '!'))
                    return false; // that's not a tag
            }
            return QTextHtmlParser::lookupElement(tag.toLower()) != -1;
        }
    }
    return false;
}

/*!
  Auxiliary function. Converts the plain text string \a plain to a
  rich text formatted string with any HTML meta-characters escaped.
 */
QString QText::escape(const QString& plain)
{
    QString rich;
    rich.reserve(int(plain.length() * 1.1));
    for (int i = 0; i < plain.length(); ++i) {
        if (plain.at(i) == QLatin1Char('<'))
            rich += QLatin1String("&lt;");
        else if (plain.at(i) == QLatin1Char('>'))
            rich += QLatin1String("&gt;");
        else if (plain.at(i) == QLatin1Char('&'))
            rich += QLatin1String("&amp;");
        else
            rich += plain.at(i);
    }
    return rich;
}

/*!  Auxiliary function. Converts the plain text string \a plain to a
    rich text formatted paragraph while preserving most of its look.

    \a mode defines the whitespace mode. Possible values are \c
    QStyleSheetItem::WhiteSpacePre (no wrapping, all whitespaces
    preserved) and \c QStyleSheetItem::WhiteSpaceNormal (wrapping,
    simplified whitespaces).

    \sa escape()
*/
QString QText::convertFromPlainText(const QString &plain, QText::WhiteSpaceMode mode)
{
    int col = 0;
    QString rich;
    rich += "<p>";
    for (int i = 0; i < plain.length(); ++i) {
        if (plain[i] == '\n'){
            int c = 1;
            while (i+1 < plain.length() && plain[i+1] == '\n') {
                i++;
                c++;
            }
            if (c == 1)
                rich += "<br>\n";
            else {
                rich += "</p>\n";
                while (--c > 1)
                    rich += "<br>\n";
                rich += "<p>";
            }
            col = 0;
        } else {
            if (mode == QText::WhiteSpacePre && plain[i] == '\t'){
                rich += 0x00a0U;
                ++col;
                while (col % 8) {
                    rich += 0x00a0U;
                    ++col;
                }
            }
            else if (mode == QText::WhiteSpacePre && plain[i].isSpace())
                rich += 0x00a0U;
            else if (plain[i] == '<')
                rich +="&lt;";
            else if (plain[i] == '>')
                rich +="&gt;";
            else if (plain[i] == '&')
                rich +="&amp;";
            else
                rich += plain[i];
            ++col;
        }
    }
    if (col != 0)
        rich += "</p>";
    return rich;
}

/*!
    \class QTextDocument qtextdocument.h
    \brief The QTextDocument class holds formatted text that can be
    viewed and edited using a QTextEdit.

    \ingroup text
    \mainclass

    A text document can be thought of as a list of strings and their
    associated formats. A format can contain "references" to objects,
    such as a QTextList, a QTextFrame, or a QTextTable, and these are
    available using object().

    A QTextDocument can be edited programmatically using a
    \l{QTextCursor}.

    The layout of a document is determined by the documentLayout();
    you can create your own QAbstractTextDocumentLayout subclass and
    set it using setDocumentLayout() if you want to use your own
    layout logic. The document's title is available using
    documentTitle().

    You can retrieve the contents of the document using plainText() or
    html(). If you want the text with format information, or wish to
    edit the text, use a QTextCursor The text can be searched using
    the find() functions. If you want to iterate over the contents of
    the document you can use begin(), end(), or findBlock() to
    retrieve a QTextBlock that you can query and iterate from.

    Undo/redo can be controlled using setUndoRedoEnabled(). undo() and
    redo() slots are provided, along with contentsChanged(),
    undoAvailable() and redoAvailable() signals.
*/

/*!
    Constructs an empty QTextDocument with the given \a parent.
*/
QTextDocument::QTextDocument(QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    d->init();
}

/*!
    Constructs a QTextDocument containing the plain (unformatted) \a text
    specified, and with the given \a parent.
*/
QTextDocument::QTextDocument(const QString &text, QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    d->init();
    QTextCursor(this).insertText(text);
}

/*!
    Destroys the document.
*/
QTextDocument::~QTextDocument()
{
}

/*!
    Returns true if the document is empty; otherwise returns false.
*/
bool QTextDocument::isEmpty() const
{
    /* because if we're empty we still have one single paragraph as
     * one single fragment */
    return d->length() <= 1;
}

/*!
    Undoes the last editing operation on the document if
    \link QTextDocument::isUndoAvailable() undo is available\endlink.
*/
void QTextDocument::undo()
{
    d->undoRedo(true);
}

/*!
    Redoes the last editing operation on the document if \link
    QTextDocument::isRedoAvailable() redo is available\endlink.
*/
void QTextDocument::redo()
{
    d->undoRedo(false);
}

/*!
    \internal

    Appends a custom undo \a item to the undo stack.
*/
void QTextDocument::appendUndoItem(QAbstractUndoItem *item)
{
    d->appendUndoItem(item);
}

/*!
    \property QTextDocument::undoRedoEnabled
    \brief Whether undo/redo are enabled for this document.

    This defaults to true. If disabled the undo stack is cleared and
    no items will be added to it.
*/
void QTextDocument::setUndoRedoEnabled(bool enable)
{
    d->enableUndoRedo(enable);
}

bool QTextDocument::isUndoRedoEnabled() const
{
    return d->isUndoRedoEnabled();
}

/*!
    \fn void QTextDocument::contentsChanged()

    This signal is emitted whenever the documents content changes, for
    example, text is inserted or deleted, or formatting is applied.
*/


/*!
    \fn QTextDocument::undoAvailable(bool b);

    This signal is emitted whenever undo operations become available
    (\a b is true) or unavailable (\a b is false).
*/

/*!
    \fn QTextDocument::redoAvailable(bool b);

    This signal is emitted whenever redo operations become available
    (\a b is true) or unavailable (\a b is false).
*/

/*!
    \enum QTextDocument::FindDirection

    This enum is used to specify the search direction when searching
    for text with the find() function.

    \value FindForward
    \value FindBackward
*/

/*!
    Returns true is undo is available; otherwise returns false.
*/
bool QTextDocument::isUndoAvailable() const
{
    return d->isUndoAvailable();
}

/*!
    Returns true is redo is available; otherwise returns false.
*/
bool QTextDocument::isRedoAvailable() const
{
    return d->isRedoAvailable();
}

/*!
    Sets the document to use the given \a layout. The previous layout
    is deleted.
*/
void QTextDocument::setDocumentLayout(QAbstractTextDocumentLayout *layout)
{
    d->setLayout(layout);
}

/*!
    Returns the document layout for this document.
*/
QAbstractTextDocumentLayout *QTextDocument::documentLayout() const
{
    if (!d->lout) {
        QTextDocument *that = const_cast<QTextDocument *>(this);
        that->d->setLayout(new QTextDocumentLayout(that));
    }
    return d->lout;
}


/*!
    Returns the document's title.
*/
QString QTextDocument::documentTitle() const
{
    return d->config()->title;
}

/*!
    Returns the plain text contained in the document. If you want
    formatting information use a QTextCursor instead.

    \sa html()
*/
QString QTextDocument::plainText() const
{
    QString txt = d->plainText();
    txt.replace(QChar::ParagraphSeparator, '\n');
    return txt;
}

/*!
    Replaces the entire contents of the document with the given plain
    \a text.

    \sa setHtml()
*/
void QTextDocument::setPlainText(const QString &text)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromPlainText(text);
    QTextCursor cursor(this);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    q->setUndoRedoEnabled(false);
    cursor.insertFragment(fragment);
    q->setUndoRedoEnabled(true);
}


/*!
    Returns the document in HTML format. The conversion may not be
    perfect, especially for complex documents, due to the limitations
    of HTML.
*/
QString QTextDocument::html() const
{
    // ###########
    qWarning("QTextDocument::html() not implemented, returning plain text");
    return plainText();
}

/*!
    Replaces the entire contents of the document with the given
    HTML-formatted text in the \a html string.

    The HTML formatting is respected as much as possible, i.e.
    "<b>bold</b> text" will have the text "bold text" with the first
    word having a character format with a bold font weight.

    \sa setPlainText()
*/
void QTextDocument::setHtml(const QString &html)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(html);
    QTextCursor cursor(this);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    q->setUndoRedoEnabled(false);
    cursor.insertFragment(fragment);
    q->setUndoRedoEnabled(true);
}

/*!
    \enum QTextDocument::FindFlag

    This enum describes the options available to QTextDocument's find function. The options
    can be OR-red together from the following list:

    \value FindCaseSensitively By default find works case insensitive. Specifying this option
    changes the behaviour to a case sensitive find operation.
    \value FindWholeWords Makes find match only complete words.
*/

static bool findInBlock(const QTextBlock &block, const QString &text, const QString &expression, int offset, 
                        QTextDocument::FindFlags options, QTextDocument::FindDirection direction, QTextCursor &cursor)
{
    const Qt::CaseSensitivity cs = (options & QTextDocument::FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive;

    const int idx = (direction == QTextDocument::FindForward) ?
                    text.indexOf(expression, offset, cs) : text.lastIndexOf(expression, offset, cs);
    if (idx == -1)
        return false;

    if (options & QTextDocument::FindWholeWords) {
        const int start = idx;
        const int end = start + expression.length();
        if ((start != 0 && text.at(start - 1).isLetterOrNumber())
                || (end != text.length() && text.at(end).isLetterOrNumber()))
            return false;
    }

    // ### FIXME
    cursor = QTextCursor(block.docHandle()->document());
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, block.position() + idx);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, expression.length());
    return true;
}

/*!
    \fn QTextCursor QTextDocument::find(const QString &expr, int position, FindFlags options, FindDirection direction) const

    \overload

    Finds the next occurrence of the string, \a expr, in the document.
    The search starts at the given \a position, and proceeds in the
    \a direction specified. The \a options control the type of search
    performed.

    Returns a cursor with the match selected if \a expr was found; otherwise
    returns a null cursor.

    If the \a position is 0 (the default) the search begins from the beginning
    of the document; otherwise it begins at the specified position.
*/
QTextCursor QTextDocument::find(const QString &expr, int from, FindFlags options, FindDirection direction) const
{
    if (expr.isEmpty())
        return QTextCursor();

    int pos = from;

    QTextCursor cursor;
    QTextBlock block = d->blocksFind(pos);

    if (direction == FindForward) {
        while (block.isValid()) {
            int blockOffset = qMax(0, pos - block.position());
            const QString blockText = block.text();

            const int blockLength = block.length();
            while (blockOffset < blockLength) {
                if (findInBlock(block, blockText, expr, blockOffset, options, direction, cursor))
                    return cursor;

                blockOffset += expr.length();
            }

            block = block.next();
        }
    } else {
        while (block.isValid()) {
            int blockOffset = pos - block.position();
            if (pos > block.position())
                blockOffset = block.length() - 1;

            const QString blockText = block.text();

            while (blockOffset >= 0) {
                if (findInBlock(block, blockText, expr, blockOffset, options, direction, cursor))
                    return cursor;

                blockOffset -= expr.length();
            }

            block = block.previous();
        }
    }

    return QTextCursor();
}

/*!
    \fn QTextCursor QTextDocument::find(const QString &expr, const QTextCursor &cursor, FindFlags options, FindDirection direction) const

    Finds the next occurrence of the string, \a expr, in the document.
    The search starts at the position of the given \a cursor, and proceeds
    in the \a direction specified. The \a options control the type
    of search performed.

    Returns a cursor with the match selected if \a expr was found; otherwise
    returns a null cursor.

    If the given \a cursor has a selection, the search begins after the
    selection; otherwise it begins at the cursor's position.

    By default the search is case-sensitive, and can match text anywhere in the
    document.
*/
QTextCursor QTextDocument::find(const QString &expr, const QTextCursor &from, FindFlags options, FindDirection direction) const
{
    const int pos = (from.isNull() ? 0 : from.selectionEnd());
    return find(expr, pos, options, direction);
}



/*!
    \fn QTextObject *QTextDocument::createObject(const QTextFormat &format)

    Creates and returns a new document object (a QTextObject), based
    on the given \a format.

    QTextObjects will always get created through this method, so you
    must reimplement it if you use custom text objects inside your document.
*/
QTextObject *QTextDocument::createObject(const QTextFormat &f)
{
    QTextObject *obj = 0;
    if (f.isListFormat())
        obj = new QTextList(this);
    else if (f.isTableFormat())
        obj = new QTextTable(this);
    else if (f.isFrameFormat())
        obj = new QTextFrame(this);

    return obj;
}

/*!
    \internal

    Returns the frame that contains the text cursor position \a pos.
*/
QTextFrame *QTextDocument::frameAt(int pos) const
{
    return d->frameAt(pos);
}

/*!
    Returns the document's root frame.
*/
QTextFrame *QTextDocument::rootFrame() const
{
    return d->rootFrame();
}

/*!
    Returns the text object associated with the given \a objectIndex.
*/
QTextObject *QTextDocument::object(int objectIndex) const
{
    return d->objectForIndex(objectIndex);
}

/*!
    Returns the text object associated with the format \a f.
*/
QTextObject *QTextDocument::objectForFormat(const QTextFormat &f) const
{
    return d->objectForFormat(f);
}


/*!
    Returns the text block that contains the \a{pos}-th character.
*/
QTextBlock QTextDocument::findBlock(int pos) const
{
    return QTextBlock(docHandle(), d->blockMap().findNode(pos));
}

/*!
    Returns the document's first text block.
*/
QTextBlock QTextDocument::begin() const
{
    return QTextBlock(docHandle(), d->blockMap().begin().n);
}

/*!
    Returns the document's last text block.
*/
QTextBlock QTextDocument::end() const
{
    return QTextBlock(docHandle(), 0);
}

/*!
    \fn QTextDocument::modificationChanged(bool changed)

    This signal is emitted whenever the content of the document
    changes in a way that affects the modification state. If \a
    changed is true if the document has been modified; otherwise it is
    false.

    For example calling setModified(false) on a document and then
    inserting text causes the signal to get emitted. If you undo that
    operation, causing the document to return to its original
    unmodified state, the signal will get emitted again.
*/

/*!
    \property QTextDocument::modified
    \brief whether the document has been modified by the user

    \sa modificationChanged()
*/

bool QTextDocument::isModified() const
{
    return docHandle()->isModified();
}

void QTextDocument::setModified(bool m)
{
    docHandle()->setModified(m);
}

class QTextHtmlExporter
{
public:
    QTextHtmlExporter(const QTextDocument *_doc);

    QString toHtml();

private:
    void exportFrame(QTextFrame::Iterator frameIt);
    void exportBlock(const QTextBlock &block);
    void exportTable(const QTextTable *table);
    void exportFragment(const QTextFragment &fragment);

    void emitBlockFormatAttributes(const QTextBlockFormat &format);
    bool emitCharFormatStyle(const QTextCharFormat &format);
    void emitTextLength(const char *attribute, const QTextLength &length);

    void emitAttribute(const char *attribute, const QString &value);

    QString html;
    QFont defaultFont;
    const QTextDocument *doc;
};

QTextHtmlExporter::QTextHtmlExporter(const QTextDocument *_doc)
    : doc(_doc)
{
    defaultFont = doc->documentLayout()->defaultFont();
}

QString QTextHtmlExporter::toHtml()
{
    html = QLatin1String("<html><body>"); // ####
    exportFrame(doc->rootFrame()->begin());
    html += QLatin1String("</body></html>");
    return html;
}

void QTextHtmlExporter::emitAttribute(const char *attribute, const QString &value)
{
    html += QLatin1Char(' ');
    html += attribute;
    html += QLatin1String("=\"");
    html += value;
    html += QLatin1Char('"');
}

bool QTextHtmlExporter::emitCharFormatStyle(const QTextCharFormat &format)
{
    bool attributesEmitted = false;

    if (format.hasProperty(QTextFormat::FontFamily)) {
        html += QLatin1String(" font-family:");
        html += format.fontFamily();
        html += QLatin1Char(';');
        attributesEmitted = true;
    }

    if (format.hasProperty(QTextFormat::FontPointSize)) {
        html += QLatin1String(" font-size:");
        html += QString::number(format.fontPointSize());
        html += QLatin1String("pt;");
        attributesEmitted = true;
    }

    if (format.hasProperty(QTextFormat::FontWeight)) {
        html += QLatin1String(" font-weight:");
        html += QString::number(format.fontWeight() * 8);
        html += QLatin1Char(';');
        attributesEmitted = true;

    }

    if (format.hasProperty(QTextFormat::FontItalic)) {
        html += QLatin1String(" font-style:");
        html += (format.fontItalic() ? QLatin1String("italic") : QLatin1String("normal"));
        html += QLatin1Char(';');
        attributesEmitted = true;
    }

    QLatin1String decorationTag(" text-decoration:");
    html += decorationTag;
    bool hasDecoration = false;
    bool atLeastOneDecorationSet = false;

    if (format.hasProperty(QTextFormat::FontUnderline)) {
        hasDecoration = true;
        if (format.fontUnderline()) {
            html += QLatin1String(" underline");
            atLeastOneDecorationSet = true;
        }
    }

    if (format.hasProperty(QTextFormat::FontOverline)) {
        hasDecoration = true;
        if (format.fontOverline()) {
            html += QLatin1String(" overline");
            atLeastOneDecorationSet = true;
        }
    }

    if (format.hasProperty(QTextFormat::FontStrikeOut)) {
        hasDecoration = true;
        if (format.fontStrikeOut()) {
            html += QLatin1String(" line-through");
            atLeastOneDecorationSet = true;
        }
    }

    if (hasDecoration) {
        if (!atLeastOneDecorationSet)
            html += QLatin1String("none");
        html += QLatin1Char(';');
        attributesEmitted = true;
    } else {
        html.truncate(html.size() - qstrlen(decorationTag.latin1()));
    }

    if (format.hasProperty(QTextFormat::TextColor)) {
        html += QLatin1String(" color:");
        html += format.textColor().name();
        html += QLatin1Char(';');
        attributesEmitted = true;
    }

    return attributesEmitted;
}

void QTextHtmlExporter::emitTextLength(const char *attribute, const QTextLength &length)
{
    if (length.type() == QTextLength::VariableLength) // default
        return;

    html += QLatin1Char(' ');
    html += attribute;
    html += QLatin1String("=\"");
    html += QString::number(length.rawValue());

    if (length.type() == QTextLength::PercentageLength)
        html += QLatin1String("%\"");
    else
        html += QLatin1String("\"");
}

void QTextHtmlExporter::exportFragment(const QTextFragment &fragment)
{
    const QTextCharFormat format = fragment.charFormat();

    bool closeAnchor = false;

    if (format.isAnchor()) {
        const QString name = format.anchorName();
        if (!name.isEmpty()) {
            html += QLatin1String("<a name=\"");
            html += name;
            html += QLatin1String("\"></a>");
        }
        const QString href = format.anchorHref();
        if (!href.isEmpty()) {
            html += QLatin1String("<a href=\"");
            html += href;
            html += QLatin1String("\">");
            closeAnchor = true;
        }
    }

    QLatin1String styleTag("<span style=\"");
    html += styleTag;

    const bool attributesEmitted = emitCharFormatStyle(format);
    if (attributesEmitted)
        html += QLatin1String("\">");
    else
        html.truncate(html.size() - qstrlen(styleTag.latin1()));

    html += QText::escape(fragment.text());

    if (attributesEmitted)
        html += QLatin1String("</span>");

    if (closeAnchor)
        html += QLatin1String("</a>");
}

void QTextHtmlExporter::emitBlockFormatAttributes(const QTextBlockFormat &format)
{
    Qt::Alignment align = format.alignment();
    if (align == Qt::AlignRight)
        html += QLatin1String(" align='right'");
    else if (align == Qt::AlignHCenter)
        html += QLatin1String(" align='center'");
    else if (align == Qt::AlignJustify)
        html += QLatin1String(" align='justify'");

    QTextBlockFormat::Direction dir = format.direction();
    if (dir == QTextBlockFormat::LeftToRight)
        html += QLatin1String(" dir='ltr'");
    else if (dir == QTextBlockFormat::RightToLeft)
        html += QLatin1String(" dir='rtl'");

    // ### margins

    if (format.hasProperty(QTextFormat::BlockBackgroundColor))
        emitAttribute("bgcolor", format.backgroundColor().name());
}

void QTextHtmlExporter::exportBlock(const QTextBlock &block)
{
    if (block.begin().atEnd())
        return;

    const bool pre = block.blockFormat().nonBreakableLines();
    if (pre)
        html += QLatin1String("<pre");
    else
        html += QLatin1String("<p");

    emitBlockFormatAttributes(block.blockFormat());
    html += QLatin1Char('>');

    for (QTextBlock::Iterator it = block.begin();
         !it.atEnd(); ++it)
        exportFragment(it.fragment());

    if (pre)
        html += QLatin1String("</pre>");
    else
        html += QLatin1String("</p>");
}

void QTextHtmlExporter::exportTable(const QTextTable *table)
{
    QTextTableFormat format = table->format();

    html += QLatin1String("<table");

    if (format.hasProperty(QTextFormat::FrameBorder))
        emitAttribute("border", QString::number(format.border()));

    // ### style="float: ..."
    // ### align

    emitTextLength("width", format.width());

    if (format.hasProperty(QTextFormat::TableCellSpacing))
        emitAttribute("cellspacing", QString::number(format.cellSpacing()));
    if (format.hasProperty(QTextFormat::TableCellPadding))
        emitAttribute("cellpadding", QString::number(format.cellPadding()));

    if (format.hasProperty(QTextFormat::TableBackgroundColor))
        emitAttribute("bgcolor", format.backgroundColor().name());

    html += QLatin1Char('>');

    const int rows = table->rows();
    const int columns = table->columns();

    QVector<QTextLength> columnWidths = format.columnWidthConstraints();
    if (columnWidths.isEmpty()) {
        columnWidths.resize(columns);
        columnWidths.fill(QTextLength());
    }
    Q_ASSERT(columnWidths.count() == columns);

    QVarLengthArray<bool> widthEmittedForColumn(columns);
    for (int i = 0; i < columns; ++i)
        widthEmittedForColumn[i] = false;

    for (int row = 0; row < rows; ++row) {
        html += QLatin1String("<tr>"); // ### attr

        for (int col = 0; col < columns; ++col) {
            const QTextTableCell cell = table->cellAt(row, col);

            // for col/rowspans
            if (cell.row() != row)
                break;

            if (cell.column() != col)
                break;

            html += QLatin1String("<td");

            if (!widthEmittedForColumn[col]) {
                emitTextLength("width", columnWidths.at(col));
                widthEmittedForColumn[col] = true;
            }

            if (cell.columnSpan() > 1)
                emitAttribute("colspan", QString::number(cell.columnSpan()));

            if (cell.rowSpan() > 1)
                emitAttribute("rowspan", QString::number(cell.rowSpan()));

            const QTextCharFormat cellFormat = cell.format();
            if (cellFormat.hasProperty(QTextFormat::TableCellBackgroundColor))
                emitAttribute("bgcolor", cellFormat.tableCellBackgroundColor().name());

            html += QLatin1Char('>');

            exportFrame(cell.begin());

            html += QLatin1String("</td>");
        }

        html += QLatin1String("</tr>"); // ### attr
    }

    html += QLatin1String("</table>");
}

void QTextHtmlExporter::exportFrame(QTextFrame::Iterator frameIt)
{
    for (QTextFrame::Iterator it = frameIt;
         !it.atEnd(); ++it) {
        if (QTextTable *table = qt_cast<QTextTable *>(it.currentFrame()))
            exportTable(table);
        else if (it.currentBlock().isValid())
            exportBlock(it.currentBlock());
    }
}

QString QTextDocument::toHtml() const
{
    return QTextHtmlExporter(this).toHtml();
}

/*!
  \internal

  So that not all classes have to be friends of each other...
*/
QTextDocumentPrivate *QTextDocument::docHandle() const
{
    return const_cast<QTextDocumentPrivate *>(d);
}
