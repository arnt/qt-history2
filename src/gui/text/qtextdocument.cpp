#include "qtextdocument.h"
#include <qtextformat.h>
#include "qtextdocumentlayout_p.h"
#include "qtextdocumentfragment.h"
#include "qtexttable.h"
#include "qtextlist.h"
#include <qdebug.h>
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
            return QTextHtmlParser::lookupElement(tag) != -1;
        }
    }
    return false;
}


/*!
    \class QTextDocument qtextdocument.h
    \brief The QTextDocument class holds formatted text that can be
    viewed and edited using a QTextEdit.

    \ingroup text

    A QTextDocument can be edited programmatically using a
    \l{QTextCursor}.

    The layout of a document is determined by the documentLayout();
    you can create your own QAbstractTextDocumentLayout subclass and
    set it using setDocumentLayout() if you want to use your own
    layout logic.

    You can retrieve the plain text content of the document using
    plainText(); if you want the text and the format information use a
    QTextCursor. The text can be searched using the find() functions.

    The document's title is available using documentTitle(). The page
    size can be set with setPageSize(), and the number of pages the
    document occupies (with the current layout and page size) is given
    by numPages(). Arbitrary HTML formatted text can be inserted using
    setHtml(); for any other edits use a QTextCursor.

    Undo/redo can be controlled using setUndoRedoEnabled(). undo() and
    redo() slots are provided, along with contentsChanged(),
    undoAvailable() and redoAvailable() signals.
*/

/*!
    \enum QTextDocument::FindMode

    \value FindWords
    \value FindAnything
*/

/*!
    Constructs an empty QTextDocument with parent \a parent.
*/
QTextDocument::QTextDocument(QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    d->init();
}

/*!
    Constructs a QTextDocument containing the plain (unformatted) text
    \a text, and with parent \a parent.
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
    Sets a new layout on the document. The old layout will be deleted.
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
        qDebug("auto creating layout");
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
*/
QString QTextDocument::plainText() const
{
    QString txt = d->plainText();
    txt.replace(QChar::ParagraphSeparator, '\n');
    return txt;
}

void QTextDocument::setPlainText(const QString &text)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromPlainText(text);
    QTextCursor cursor(this);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    q->setUndoRedoEnabled(false);
    cursor.insertFragment(fragment);
    q->setUndoRedoEnabled(true);
}


QString QTextDocument::html() const
{
    // ###########
    qWarning("QTextDocument::html() not implemented, returning plain text");
    return plainText();
}

/*!
    Clears the text and replaces it with the arbitrary piece of HTML
    formatted text in the \a html string.

    The HTML formatting is respected as much as possible, i.e.
    "<b>bold</b> text" will have the text "bold text" with the first
    word having a character format with a bold font weight.
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
    \overload

    Finds the next occurrence of the string, \a expr, starting at
    position \a from. Returns a cursor with the match selected if \a
    expr was found; otherwise returns a null cursor.

    If \a from is 0 (the default) the search begins from the beginning
    of the document; otherwise from the specified position.

    The comparison is made in accordance with the
    \l{Qt::StringComparisonFlags}, \a flags. By default the search is
    case-sensitive, and can match anywhere.
*/
QTextCursor QTextDocument::find(const QString &expr, int from, StringComparison flags) const
{
    if (expr.isEmpty())
        return QTextCursor();

    int pos = from;

    QString::CaseSensitivity cs;
    if (flags & CaseSensitive)
        cs = QString::CaseSensitive;
    else
        cs = QString::CaseInsensitive;

    QTextBlockIterator block = d->blocksFind(pos);
    while (!block.atEnd()) {
        const int blockOffset = qMax(0, pos - block.position());
        QString text = block.blockText();
        int idx = -1;
        QTextLayout *layout = block.layout();

        if (flags & Contains) {
            idx = text.indexOf(expr, blockOffset, cs);
        } else {
            int i = blockOffset;
            while (i < text.length() && idx == -1) {
                int nextWordPos = layout->nextCursorPosition(i, QTextLayout::SkipWords);
                QString word = text.mid(i, nextWordPos - i).trimmed();
                if ((flags & BeginsWith) && word.startsWith(expr, cs)) {
                    idx = i;
                } else if ((flags & EndsWith) && word.endsWith(expr, cs)) {
                    idx = i + word.length() - expr.length();
                } else if ((flags & ExactMatch)
                           && (((cs == QString::CaseSensitive) && word == expr)
                               || (cs == QString::CaseInsensitive) && word.toLower() == expr.toLower())) {
                        idx = i;
                } else {
                    i = nextWordPos;
                }
            }
        }

        if (idx >= 0) {
            QTextCursor cursor(d, block.position() + blockOffset + idx);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, expr.length());
            return cursor;
        }
        ++block;
    }

    return QTextCursor();
}

/*!
    Finds the next occurrence of the string, \a expr, starting at
    position \a from. Returns a cursor with the match selected if \a
    expr was found; otherwise returns a null cursor.

    If the \a from cursor has a selection the search begins after the
    selection; otherwise from the position of the cursor.
*/
QTextCursor QTextDocument::find(const QString &expr, const QTextCursor &from, StringComparison flags) const
{
    const int pos = (from.isNull() ? 0 : from.selectionEnd());
    return find(expr, pos, flags);
}



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

QTextFrame *QTextDocument::frameAt(int pos) const
{
    return d->frameAt(pos);
}

QTextFrame *QTextDocument::rootFrame() const
{
    return d->rootFrame();
}

