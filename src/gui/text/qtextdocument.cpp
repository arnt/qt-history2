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
#include "qtextdocumentfragment_p.h"
#include "qtexttable.h"
#include "qtextlist.h"
#include <qdebug.h>
#include <qregexp.h>
#include <qvarlengtharray.h>
#include <qtextcodec.h>
#include "qtexthtmlparser_p.h"
#include "qpainter.h"
#include "qprinter.h"
#include "qtextedit.h"

#include "qtextdocument_p.h"

#include <limits.h>

/*!
    Returns true if the string \a text is likely to be rich text;
    otherwise returns false.

    This function uses a fast and therefore simple heuristic. It
    mainly checks whether there is something that looks like a tag
    before the first line break. Although the result may be correct
    for common cases, there is no guarantee.
*/
bool Qt::mightBeRichText(const QString& text)
{
    if (text.isEmpty())
        return false;
    int start = 0;

    while (start < text.length() && text.at(start).isSpace())
        ++start;

    // skip a leading <?xml ... ?> as for example with xhtml
    if (text.mid(start, 5) == QLatin1String("<?xml")) {
        while (start < text.length()) {
            if (text.at(start) == QLatin1Char('?')
                && start + 2 < text.length()
                && text.at(start + 1) == QLatin1Char('>')) {
                start += 2;
                break;
            }
            ++start;
        }

        while (start < text.length() && text.at(start).isSpace())
            ++start;
    }

    if (text.mid(start, 5).toLower() == QLatin1String("<!doc"))
        return true;
    int open = start;
    while (open < text.length() && text.at(open) != '<'
            && text.at(open) != '\n') {
        if (text.at(open) == '&' &&  text.mid(open+1,3) == "lt;")
            return true; // support desperate attempt of user to see <...>
        ++open;
    }
    if (open < text.length() && text.at(open) == '<') {
        const int close = text.indexOf('>', open);
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
QString Qt::escape(const QString& plain)
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

/*!
    \fn QString Qt::convertFromPlainText(const QString &plain, WhiteSpaceMode mode)

    Auxiliary function. Converts the plain text string \a plain to a
    rich text formatted paragraph while preserving most of its look.

    \a mode defines the whitespace mode. Possible values are \c
    QStyleSheetItem::WhiteSpacePre (no wrapping, all whitespaces
    preserved) and \c QStyleSheetItem::WhiteSpaceNormal (wrapping,
    simplified whitespaces).

    \sa escape()
*/
QString Qt::convertFromPlainText(const QString &plain, Qt::WhiteSpaceMode mode)
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
            if (mode == Qt::WhiteSpacePre && plain[i] == '\t'){
                rich += 0x00a0U;
                ++col;
                while (col % 8) {
                    rich += 0x00a0U;
                    ++col;
                }
            }
            else if (mode == Qt::WhiteSpacePre && plain[i].isSpace())
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
  \internal
*/
QTextCodec *Qt::codecForHtml(const QByteArray &ba)
{
    // determine charset
    int mib = 4; // Latin1
    int pos;
    QTextCodec *c = 0;

    if (ba.size() > 1 && (((uchar)ba[0] == 0xfe && (uchar)ba[1] == 0xff)
                          || ((uchar)ba[0] == 0xff && (uchar)ba[1] == 0xfe))) {
        mib = 1000; // utf16
    } else if (ba.size() > 2
             && (uchar)ba[0] == 0xef
             && (uchar)ba[1] == 0xbb
             && (uchar)ba[2] == 0xbf) {
        mib = 106; // utf-8
    } else if ((pos = ba.indexOf("http-equiv=")) != -1) {
        pos = ba.indexOf("charset=", pos) + strlen("charset=");
        if (pos != -1) {
            int pos2 = ba.indexOf('\"', pos+1);
            QByteArray cs = ba.mid(pos, pos2-pos);
//            qDebug("found charset: %s", cs.data());
            c = QTextCodec::codecForName(cs);
        }
    }
    if (!c)
        c = QTextCodec::codecForMib(mib);

    return c;
}

/*!
    \class QTextDocument qtextdocument.h
    \brief The QTextDocument class holds formatted text that can be
    viewed and edited using a QTextEdit.

    \ingroup text
    \mainclass

    QTextDocument is a container for structured rich text documents, providing
    support for styled text and various types of document elements, such as
    lists, tables, frames, and images.
    They can be created for use in a QTextEdit, or used independently.

    Each document element is described by an associated format object. Each
    format object is treated as a unique object by QTextDocuments, and can be
    passed to objectForFormat() to obtain the document element that it is
    applied to.

    A QTextDocument can be edited programmatically using a QTextCursor, and
    its contents can be examined by traversing the document structure. The
    entire document structure is stored as a hierarchy of document elements
    beneath the root frame, found with the rootFrame() function. Alternatively,
    if you just want to iterate over the textual contents of the document you
    can use begin(), end(), and findBlock() to retrieve text blocks that you
    can examine and iterate over.

    The layout of a document is determined by the documentLayout();
    you can create your own QAbstractTextDocumentLayout subclass and
    set it using setDocumentLayout() if you want to use your own
    layout logic. The document's title can be obtained by calling the
    documentTitle() function.

    The toPlainText() and toHtml() convenience functions allow you to retrieve the
    contents of the document as plain text and HTML.
    The document's text can be searched using the find() functions.

    Undo/redo of operations performed on the document can be controlled using
    the setUndoRedoEnabled() function. The undo/redo system can be controlled
    by an editor widget through the undo() and redo() slots; the document also
    provides contentsChanged(), undoAvailable(), and redoAvailable() signals
    that inform connected editor widgets about the state of the undo/redo
    system.

    \sa QTextCursor QTextEdit \link richtext.html Rich Text Processing\endlink
*/

/*!
    Constructs an empty QTextDocument with the given \a parent.
*/
QTextDocument::QTextDocument(QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    Q_D(QTextDocument);
    d->init();
}

/*!
    Constructs a QTextDocument containing the plain (unformatted) \a text
    specified, and with the given \a parent.
*/
QTextDocument::QTextDocument(const QString &text, QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    Q_D(QTextDocument);
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
  Creates a new QTextDocument that is a copy of this text document. \a
  parent is the parent of the returned text document.
*/
QTextDocument *QTextDocument::clone(QObject *parent) const
{
    QTextDocument *doc = new QTextDocument(parent);
    QTextCursor(doc).insertFragment(QTextDocumentFragment(this));
    return doc;
}

/*!
    Returns true if the document is empty; otherwise returns false.
*/
bool QTextDocument::isEmpty() const
{
    Q_D(const QTextDocument);
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
    Q_D(QTextDocument);
    d->undoRedo(true);
}

/*!
    Redoes the last editing operation on the document if \link
    QTextDocument::isRedoAvailable() redo is available\endlink.
*/
void QTextDocument::redo()
{
    Q_D(QTextDocument);
    d->undoRedo(false);
}

/*!
    \internal

    Appends a custom undo \a item to the undo stack.
*/
void QTextDocument::appendUndoItem(QAbstractUndoItem *item)
{
    Q_D(QTextDocument);
    d->appendUndoItem(item);
}

/*!
    \property QTextDocument::undoRedoEnabled
    \brief whether undo/redo are enabled for this document

    This defaults to true. If disabled, the undo stack is cleared and
    no items will be added to it.
*/
void QTextDocument::setUndoRedoEnabled(bool enable)
{
    Q_D(QTextDocument);
    d->enableUndoRedo(enable);
}

bool QTextDocument::isUndoRedoEnabled() const
{
    Q_D(const QTextDocument);
    return d->isUndoRedoEnabled();
}

/*!
    \fn void QTextDocument::contentsChanged()

    This signal is emitted whenever the documents content changes, for
    example, text is inserted or deleted, or formatting is applied.
*/


/*!
    \fn QTextDocument::undoAvailable(bool available);

    This signal is emitted whenever undo operations become available
    (\a available is true) or unavailable (\a available is false).
*/

/*!
    \fn QTextDocument::redoAvailable(bool available);

    This signal is emitted whenever redo operations become available
    (\a available is true) or unavailable (\a available is false).
*/

/*!
    \fn QTextDocument::cursorPositionChanged(const QTextCursor &cursor);

    This signal is emitted whenever the position of a cursor changed
    due to an editing operation. The cursor that changed is passed in
    \a cursor.
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
    Q_D(const QTextDocument);
    return d->isUndoAvailable();
}

/*!
    Returns true is redo is available; otherwise returns false.
*/
bool QTextDocument::isRedoAvailable() const
{
    Q_D(const QTextDocument);
    return d->isRedoAvailable();
}

/*!
    Sets the document to use the given \a layout. The previous layout
    is deleted.
*/
void QTextDocument::setDocumentLayout(QAbstractTextDocumentLayout *layout)
{
    Q_D(QTextDocument);
    d->setLayout(layout);
}

/*!
    Returns the document layout for this document.
*/
QAbstractTextDocumentLayout *QTextDocument::documentLayout() const
{
    Q_D(const QTextDocument);
    if (!d->lout) {
        QTextDocument *that = const_cast<QTextDocument *>(this);
        that->d_func()->setLayout(new QTextDocumentLayout(that));
    }
    return d->lout;
}


/*!
    Returns meta information about the document.
*/
QString QTextDocument::metaInformation(MetaInformation info) const
{
    if (info != DocumentTitle)
        return QString();
    Q_D(const QTextDocument);
    return d->config()->title;
}

void QTextDocument::setMetaInformation(MetaInformation info, const QString &string)
{
    if (info != DocumentTitle)
        return;
    Q_D(QTextDocument);
    d->config()->title = string;
}

/*!
    Returns the plain text contained in the document. If you want
    formatting information use a QTextCursor instead.

    \sa html()
*/
QString QTextDocument::toPlainText() const
{
    Q_D(const QTextDocument);
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
    setUndoRedoEnabled(false);
    cursor.insertFragment(fragment);
    setUndoRedoEnabled(true);
}

/*!
    Replaces the entire contents of the document with the given
    HTML-formatted text in the \a html string.

    The HTML formatting is respected as much as possible; for example,
    "<b>bold</b> text" will produce text where the first word has a font
    weight that gives it a bold appearance: "\bold{bold} text".

    \sa setPlainText()
*/
void QTextDocument::setHtml(const QString &html)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(html);
    QTextCursor cursor(this);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    setUndoRedoEnabled(false);
    cursor.insertFragment(fragment);
    setUndoRedoEnabled(true);
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
                        QTextDocument::FindFlags options, QTextCursor &cursor)
{
    const Qt::CaseSensitivity cs = (options & QTextDocument::FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive;

    const int idx = (options & QTextDocument::FindBackward) ?
                    text.lastIndexOf(expression, offset, cs) : text.indexOf(expression, offset, cs);
    if (idx == -1)
        return false;

    if (options & QTextDocument::FindWholeWords) {
        const int start = idx;
        const int end = start + expression.length();
        if ((start != 0 && text.at(start - 1).isLetterOrNumber())
                || (end != text.length() && text.at(end).isLetterOrNumber()))
            return false;
    }

    cursor = QTextCursor(block.docHandle(), block.position() + idx);
    cursor.setPosition(cursor.position() + expression.length(), QTextCursor::KeepAnchor);
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
QTextCursor QTextDocument::find(const QString &expr, int from, FindFlags options) const
{
    Q_D(const QTextDocument);

    if (expr.isEmpty())
        return QTextCursor();

    int pos = from;

    QTextCursor cursor;
    QTextBlock block = d->blocksFind(pos);

    if (!(options & FindBackward)) {
        while (block.isValid()) {
            int blockOffset = qMax(0, pos - block.position());
            const QString blockText = block.text();

            const int blockLength = block.length();
            while (blockOffset < blockLength) {
                if (findInBlock(block, blockText, expr, blockOffset, options, cursor))
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
                if (findInBlock(block, blockText, expr, blockOffset, options, cursor))
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
QTextCursor QTextDocument::find(const QString &expr, const QTextCursor &from, FindFlags options) const
{
    const int pos = (from.isNull() ? 0 : from.selectionEnd());
    return find(expr, pos, options);
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
    Q_D(const QTextDocument);
    return d->frameAt(pos);
}

/*!
    Returns the document's root frame.
*/
QTextFrame *QTextDocument::rootFrame() const
{
    Q_D(const QTextDocument);
    return d->rootFrame();
}

/*!
    Returns the text object associated with the given \a objectIndex.
*/
QTextObject *QTextDocument::object(int objectIndex) const
{
    Q_D(const QTextDocument);
    return d->objectForIndex(objectIndex);
}

/*!
    Returns the text object associated with the format \a f.
*/
QTextObject *QTextDocument::objectForFormat(const QTextFormat &f) const
{
    Q_D(const QTextDocument);
    return d->objectForFormat(f);
}


/*!
    Returns the text block that contains the \a{pos}-th character.
*/
QTextBlock QTextDocument::findBlock(int pos) const
{
    Q_D(const QTextDocument);
    return QTextBlock(docHandle(), d->blockMap().findNode(pos));
}

/*!
    Returns the document's first text block.
*/
QTextBlock QTextDocument::begin() const
{
    Q_D(const QTextDocument);
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
    \property QTextDocument::pageSize
    \brief the page size that should be used for layouting the document

    \sa modificationChanged()
*/

void QTextDocument::setPageSize(const QSizeF &size)
{
    Q_D(QTextDocument);
    d->pageSize = size;
    documentLayout()->documentChange(0, 0, d->length());
}

QSizeF QTextDocument::pageSize() const
{
    Q_D(const QTextDocument);
    return d->pageSize;
}

/*!
    Sets the default \a font to use in the document layout.
*/
void QTextDocument::setDefaultFont(const QFont &font)
{
    Q_D(QTextDocument);
    d->defaultFont = font;
    documentLayout()->documentChange(0, 0, d->length());
}

/*!
    Returns the default font to be used in the document layout.
*/
QFont QTextDocument::defaultFont() const
{
    Q_D(const QTextDocument);
    return d->defaultFont;
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

/*!
    Prints the document to the given \a printer. The QPrinter must be
    set up before being used with this function.
*/
void QTextDocument::print(QPrinter *printer) const
{
    QPainter p(printer);

    // Check that there is a valid device to print to.
    if (!p.device()) return;

    const int dpiy = p.device()->logicalDpiY();
    const int margin = (int) ((2/2.54)*dpiy); // 2 cm margins
    QRect body(margin, margin, p.device()->width() - 2*margin, p.device()->height() - 2*margin);

    QTextDocument *doc = clone();
    QAbstractTextDocumentLayout *layout = doc->documentLayout();
    QFont font(doc->defaultFont());
    font.setPointSize(10); // we define 10pt to be a nice base size for printing
    doc->setDefaultFont(font);
    layout->setPaintDevice(printer);
    doc->setPageSize(QSize(body.width(), INT_MAX));

    QRect view(0, 0, body.width(), body.height());
    p.translate(body.left(), body.top());

    int page = 1;
    do {
        QAbstractTextDocumentLayout::PaintContext ctx;
// ######        ctx.palette = palette();
        p.setClipRect(view);
        ctx.rect = view;
        layout->draw(&p, ctx);

        p.setClipping(false);
        p.setFont(font);
        p.drawText(view.right() - p.fontMetrics().width(QString::number(page)),
                view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page));

        view.translate(0, body.height());
        p.translate(0 , -body.height());

        if (view.top() >= layout->documentSize().height())
            break;

        printer->newPage();
        page++;
    } while (true);

    delete doc;
}

/*!
    \enum QTextDocument::ResourceType

    This enum describes the types of resources that can be loaded by
    QTextDocument's loadResource() function.

    \value HtmlResource  The resource contains HTML.
    \value ImageResource The resource contains image data.

    \sa loadResource()
*/

/*!
    This function is called by the rich text engine to request data that isn't
    directly stored by QTextDocument, but still associated with it. For example,
    images are referenced indirectly by the name attribute of a QTextImageFormat
    object.

    When called by Qt, \a type is one of the values of
    QTextDocument::ResourceType.

    If the QTextDocument is a child object of a QTextEdit, QTextBrowser,
    or a QTextDocument itself then the default implementation tries
    to retrieve the data from the parent.
*/
QVariant QTextDocument::loadResource(int type, const QUrl &name)
{
    if (QTextDocument *doc = qobject_cast<QTextDocument *>(parent()))
        return doc->loadResource(type, name);
    else if (QTextEdit *edit = qobject_cast<QTextEdit *>(parent()))
        return edit->loadResource(type, name);
    return QVariant();
}

static QTextFormat formatDifference(const QTextFormat &from, const QTextFormat &to)
{
    QTextFormat diff = to;

    const QMap<int, QVariant> props = to.properties();
    for (QMap<int, QVariant>::ConstIterator it = props.begin(), end = props.end();
         it != end; ++it)
        if (it.value() == from.property(it.key()))
            diff.clearProperty(it.key());

    return diff;
}

class QTextHtmlExporter
{
public:
    QTextHtmlExporter(const QTextDocument *_doc);

    QString toHtml(const QByteArray &encoding);

private:
    void emitFrame(QTextFrame::Iterator frameIt);
    void emitBlock(const QTextBlock &block);
    void emitTable(const QTextTable *table);
    void emitFragment(const QTextFragment &fragment);

    void emitBlockAttributes(const QTextBlock &block);
    bool emitCharFormatStyle(const QTextCharFormat &format);
    void emitTextLength(const char *attribute, const QTextLength &length);
    void emitAlignment(Qt::Alignment alignment);
    void emitFloatStyle(QTextFrameFormat::Position pos);
    void emitAttribute(const char *attribute, const QString &value);

    QString html;
    QTextCharFormat defaultCharFormat;
    const QTextDocument *doc;
};

QTextHtmlExporter::QTextHtmlExporter(const QTextDocument *_doc)
    : doc(_doc)
{
    const QFont defaultFont = doc->defaultFont();
    defaultCharFormat.setFont(defaultFont);
}

/*!
    Returns the document in HTML format. The conversion may not be
    perfect, especially for complex documents, due to the limitations
    of HTML.
*/
QString QTextHtmlExporter::toHtml(const QByteArray &encoding)
{
    // ### title

    html = QLatin1String("<html><head><meta name=\"qrichtext\" content=\"1\" />");

    if (!encoding.isEmpty())
        html += QString("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\" />").arg(QString::fromAscii(encoding));

    html += QString("</head><body style=\" white-space: pre-wrap; font-family:%1; font-weight:%2; font-style:%3; text-decoration:none;\">")
            .arg(defaultCharFormat.fontFamily())
            .arg(defaultCharFormat.fontWeight() * 8)
            .arg(defaultCharFormat.fontItalic() ? "italic" : "normal");

    emitFrame(doc->rootFrame()->begin());
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

    {
        const QString family = format.fontFamily();
        if (!family.isEmpty() && family != defaultCharFormat.fontFamily()) {
            html += QLatin1String(" font-family:");
            html += family;
            html += QLatin1Char(';');
            attributesEmitted = true;
        }
    }

    if (format.fontPointSize() != defaultCharFormat.fontPointSize()) {
        html += QLatin1String(" font-size:");
        html += QString::number(format.fontPointSize());
        html += QLatin1String("pt;");
        attributesEmitted = true;
    }

    if (format.fontWeight() != defaultCharFormat.fontWeight()) {
        html += QLatin1String(" font-weight:");
        html += QString::number(format.fontWeight() * 8);
        html += QLatin1Char(';');
        attributesEmitted = true;
    }

    if (format.fontItalic() != defaultCharFormat.fontItalic()) {
        html += QLatin1String(" font-style:");
        html += (format.fontItalic() ? QLatin1String("italic") : QLatin1String("normal"));
        html += QLatin1Char(';');
        attributesEmitted = true;
    }

    QLatin1String decorationTag(" text-decoration:");
    html += decorationTag;
    bool hasDecoration = false;
    bool atLeastOneDecorationSet = false;

    if (format.fontUnderline() != defaultCharFormat.fontUnderline()) {
        hasDecoration = true;
        if (format.fontUnderline()) {
            html += QLatin1String(" underline");
            atLeastOneDecorationSet = true;
        }
    }

    if (format.fontOverline() != defaultCharFormat.fontOverline()) {
        hasDecoration = true;
        if (format.fontOverline()) {
            html += QLatin1String(" overline");
            atLeastOneDecorationSet = true;
        }
    }

    if (format.fontStrikeOut() != defaultCharFormat.fontStrikeOut()) {
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
        html.chop(qstrlen(decorationTag.latin1()));
    }

    if (format.textColor() != defaultCharFormat.textColor()) {
        html += QLatin1String(" color:");
        html += format.textColor().name();
        html += QLatin1Char(';');
        attributesEmitted = true;
    }

    if (format.verticalAlignment() != defaultCharFormat.verticalAlignment()) {
        html += QLatin1String(" vertical-align:");

        QTextCharFormat::VerticalAlignment valign = format.verticalAlignment();
        if (valign == QTextCharFormat::AlignSubScript)
            html += QLatin1String("sub");
        else if (valign == QTextCharFormat::AlignSuperScript)
            html += QLatin1String("super");

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

void QTextHtmlExporter::emitAlignment(Qt::Alignment alignment)
{
    switch (alignment & Qt::AlignHorizontal_Mask) {
        case Qt::AlignLeft: break;
        case Qt::AlignRight: html += QLatin1String(" align='right'"); break;
        case Qt::AlignHCenter: html += QLatin1String(" align='center'"); break;
        case Qt::AlignJustify: html += QLatin1String(" align='justify'"); break;
    }
}

void QTextHtmlExporter::emitFloatStyle(QTextFrameFormat::Position pos)
{
    if (pos == QTextFrameFormat::InFlow)
        return;

    html += QLatin1String(" style=\"float:");
    if (pos == QTextFrameFormat::FloatLeft)
        html += QLatin1String(" left;\"");
    else if (pos == QTextFrameFormat::FloatRight)
        html += QLatin1String(" right;\"");
    else
        Q_ASSERT_X(0, "QTextHtmlExporter::emitFloatStyle()", "pos should be a valid enum type");
}

void QTextHtmlExporter::emitFragment(const QTextFragment &fragment)
{
    const QTextCharFormat format = fragment.charFormat();

    if (format.hasProperty(QTextFormat::DocumentFragmentMark)
        && (format.intProperty(QTextFormat::DocumentFragmentMark) & QTextDocumentFragmentPrivate::FragmentStart))
        html += QLatin1String("<!--StartFragment-->");

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
        html.chop(qstrlen(styleTag.latin1()));

    QString txt = fragment.text();
    if (txt.count() == 1 && txt.at(0) == QChar::ObjectReplacementCharacter) {
        if (format.isImageFormat()) {
            QTextImageFormat imgFmt = format.toImageFormat();

            html += QLatin1String("<img");

            if (imgFmt.hasProperty(QTextFormat::ImageName))
                emitAttribute("src", imgFmt.name());

            if (imgFmt.hasProperty(QTextFormat::ImageWidth))
                emitAttribute("width", QString::number(imgFmt.width()));

            if (imgFmt.hasProperty(QTextFormat::ImageHeight))
                emitAttribute("height", QString::number(imgFmt.height()));

            if (QTextFrame *imageFrame = qobject_cast<QTextFrame *>(doc->objectForFormat(imgFmt)))
                emitFloatStyle(imageFrame->frameFormat().position());

            html += QLatin1String(" />");
        }
    } else {
        Q_ASSERT(!txt.contains(QChar::ObjectReplacementCharacter));

        txt = Qt::escape(txt);

        // split for [\n{LineSeparator}]
        QString forcedLineBreakRegExp = QString::fromLatin1("[\\na]");
        forcedLineBreakRegExp[3] = QChar::LineSeparator;

        const QStringList lines = txt.split(QRegExp(forcedLineBreakRegExp));
        for (int i = 0; i < lines.count(); ++i) {
            if (i > 0)
                html += QLatin1String("<br />"); // space on purpose for compatibility with Netscape, Lynx & Co.
            html += lines.at(i);
        }
    }

    if (attributesEmitted)
        html += QLatin1String("</span>");

    if (closeAnchor)
        html += QLatin1String("</a>");

    if (format.hasProperty(QTextFormat::DocumentFragmentMark)
        && (format.intProperty(QTextFormat::DocumentFragmentMark) & QTextDocumentFragmentPrivate::FragmentEnd))
        html += QLatin1String("<!--EndFragment-->");
}

static bool isOrderedList(int style)
{
    return style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha
           || style == QTextListFormat::ListUpperAlpha;
}

void QTextHtmlExporter::emitBlockAttributes(const QTextBlock &block)
{
    QTextBlockFormat format = block.blockFormat();
    emitAlignment(format.alignment());

    QTextBlockFormat::Direction dir = format.direction();
    if (dir == QTextBlockFormat::LeftToRight)
        html += QLatin1String(" dir='ltr'");
    else if (dir == QTextBlockFormat::RightToLeft)
        html += QLatin1String(" dir='rtl'");

    bool hasStyle = false;
    QLatin1String style(" style=\"");
    html += style;

    if (format.hasProperty(QTextFormat::BlockTopMargin)) {
        html += QLatin1String(" margin-top:");
        html += QString::number(format.topMargin());
        html += QLatin1String("px;");
        hasStyle = true;
    }

    if (format.hasProperty(QTextFormat::BlockBottomMargin)) {
        html += QLatin1String(" margin-bottom:");
        html += QString::number(format.bottomMargin());
        html += QLatin1String("px;");
        hasStyle = true;
    }

    if (format.hasProperty(QTextFormat::BlockLeftMargin)) {
        html += QLatin1String(" margin-left:");
        html += QString::number(format.leftMargin());
        html += QLatin1String("px;");
        hasStyle = true;
    }

    if (format.hasProperty(QTextFormat::BlockRightMargin)) {
        html += QLatin1String(" margin-right:");
        html += QString::number(format.rightMargin());
        html += QLatin1String("px;");
        hasStyle = true;
    }

    if (format.hasProperty(QTextFormat::BlockIndent)) {
        html += QLatin1String(" -qt-block-indent:");
        html += QString::number(format.indent());
        html += QLatin1Char(';');
        hasStyle = true;
    }

    // ### 'if' needed as long as the block char format of a block at pos == 0
    // is equivalent to the char format at that position.
    // later on in the piecetable that's not the case, that's when the block char
    // fmt is at block.position() - 1
    // When changing this also change the 'if' in emitBlock
    if (block.position() > 0) {
        QTextCharFormat diff = formatDifference(defaultCharFormat, block.charFormat()).toCharFormat();
        if (!diff.properties().isEmpty())
            hasStyle |= emitCharFormatStyle(diff);
    }

    if (hasStyle)
        html += QLatin1Char('"');
    else
        html.chop(qstrlen(style.latin1()));

    if (format.hasProperty(QTextFormat::BlockBackgroundColor))
        emitAttribute("bgcolor", format.backgroundColor().name());
}

void QTextHtmlExporter::emitBlock(const QTextBlock &block)
{
    if (block.begin().atEnd()) {
        // ### HACK, remove once QTextFrame::Iterator is fixed
        int p = block.position();
        if (p > 0)
            --p;
        QTextDocumentPrivate::FragmentIterator frag = doc->docHandle()->find(p);
        QChar ch = doc->docHandle()->buffer().at(frag->stringPosition);
        if (ch == QTextBeginningOfFrame
            || ch == QTextEndOfFrame)
            return;

        html += "<p style=\"-qt-paragraph-type:empty;\">&nbsp;</p>";
        return;
    }

    // save and later restore, in case we 'change' the default format by
    // emitting block char format information
    QTextCharFormat oldDefaultCharFormat = defaultCharFormat;

    QTextList *list = block.textList();
    if (list) {
        if (list->itemNumber(block) == 0) { // first item? emit <ul> or appropriate
            const QTextListFormat format = list->format();
            const int style = format.style();
            switch (style) {
                case QTextListFormat::ListDecimal: html += QLatin1String("<ol"); break;
                case QTextListFormat::ListDisc: html += QLatin1String("<ul"); break;
                case QTextListFormat::ListCircle: html += QLatin1String("<ul type=circle"); break;
                case QTextListFormat::ListSquare: html += QLatin1String("<ul type=square"); break;
                case QTextListFormat::ListLowerAlpha: html += QLatin1String("<ol type=a"); break;
                case QTextListFormat::ListUpperAlpha: html += QLatin1String("<ol type=A"); break;
                default: html += QLatin1String("<ul"); // ### should not happen
            }

            if (format.hasProperty(QTextFormat::ListIndent)) {
                html += QLatin1String(" style=\"-qt-list-indent: ");
                html += QString::number(format.indent());
                html += QLatin1String(";\"");
            }

            html += QLatin1Char('>');
        }

        html += QLatin1String("<li");

        const QTextCharFormat blockFmt = formatDifference(defaultCharFormat, block.charFormat()).toCharFormat();
        if (!blockFmt.properties().isEmpty()) {
            html += QLatin1String(" style=\"");
            emitCharFormatStyle(blockFmt);
            html += QLatin1Char('\"');

            defaultCharFormat.merge(block.charFormat());
        }
    }

    const bool pre = block.blockFormat().nonBreakableLines();
    if (pre) {
        if (list)
            html += QLatin1Char('>');
        html += QLatin1String("<pre");
    } else if (!list) {
        html += QLatin1String("<p");
    }

    emitBlockAttributes(block);

    html += QLatin1Char('>');

    // ### 'if' needed as long as the block char format of a block at pos == 0
    // is equivalent to the char format at that position.
    // later on in the piecetable that's not the case, that's when the block char
    // fmt is at block.position() - 1
    // When changing this also change the 'if' in emitBlockAttributes
    if (block.position() > 0) {
        defaultCharFormat.merge(block.charFormat());
    }

    for (QTextBlock::Iterator it = block.begin();
         !it.atEnd(); ++it)
        emitFragment(it.fragment());

    if (pre)
        html += QLatin1String("</pre>");
    else if (!list)
        html += QLatin1String("</p>");

    if (list) {
        if (list->itemNumber(block) == list->count() - 1) { // last item? close list
            if (isOrderedList(list->format().style()))
                html += QLatin1String("</ol>");
            else
                html += QLatin1String("</ul>");
        }
    }

    defaultCharFormat = oldDefaultCharFormat;
}

void QTextHtmlExporter::emitTable(const QTextTable *table)
{
    QTextTableFormat format = table->format();

    html += QLatin1String("<table");

    if (format.hasProperty(QTextFormat::FrameBorder))
        emitAttribute("border", QString::number(format.border()));

    emitFloatStyle(format.position());
    emitAlignment(format.alignment());
    emitTextLength("width", format.width());

    if (format.hasProperty(QTextFormat::TableCellSpacing))
        emitAttribute("cellspacing", QString::number(format.cellSpacing()));
    if (format.hasProperty(QTextFormat::TableCellPadding))
        emitAttribute("cellpadding", QString::number(format.cellPadding()));

    if (format.hasProperty(QTextFormat::FrameBackgroundColor))
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
        html += QLatin1String("<tr>");

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

            emitFrame(cell.begin());

            html += QLatin1String("</td>");
        }

        html += QLatin1String("</tr>");
    }

    html += QLatin1String("</table>");
}

void QTextHtmlExporter::emitFrame(QTextFrame::Iterator frameIt)
{
    if (!frameIt.atEnd()) {
        QTextFrame::Iterator next = frameIt;
        ++next;
        if (next.atEnd()
            && frameIt.currentFrame() == 0
            && frameIt.parentFrame() != doc->rootFrame()
            && frameIt.currentBlock().begin().atEnd())
            return;
    }

    for (QTextFrame::Iterator it = frameIt;
         !it.atEnd(); ++it) {
        if (QTextTable *table = qobject_cast<QTextTable *>(it.currentFrame()))
            emitTable(table);
        else if (it.currentBlock().isValid())
            emitBlock(it.currentBlock());
    }
}

/*!
    Returns a string containing an HTML representation of the document.

    The \a encoding parameter specifies the value for the charset attribute
    in the html header. For example if 'utf-8' is specified then the
    beginning of the generated html will look like this:
    \code
    <html><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8"></head><body>...
    \endcode

    If no encoding is specified then no such meta information is generated.

    If you later on convert the returned html string into a byte array for
    transmission over a network or when saving to disk you should specify
    the encoding you're going to use for the conversion to a byte array here.
*/
QString QTextDocument::toHtml(const QByteArray &encoding) const
{
    return QTextHtmlExporter(this).toHtml(encoding);
}

QVector<QTextFormat> QTextDocument::allFormats() const
{
    Q_D(const QTextDocument);
    return d->formatCollection()->formats;
}


/*!
  \internal

  So that not all classes have to be friends of each other...
*/
QTextDocumentPrivate *QTextDocument::docHandle() const
{
    Q_D(const QTextDocument);
    return const_cast<QTextDocumentPrivate *>(d);
}
