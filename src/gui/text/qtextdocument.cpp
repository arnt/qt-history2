#include "qtextdocument.h"
#include "qtextpiecetable_p.h"
#include <qtextformat.h>
#include "qtextdocumentlayout_p.h"
#include "qtextdocumentfragment.h"
#include <qdebug.h>

#include "qtextdocument_p.h"
#define d d_func()
#define q q_func()

static void init(QTextDocumentPrivate *priv, QAbstractTextDocumentLayout *layout = 0)
{
    priv->pieceTable = new QTextPieceTable(layout);
    QObject::connect(static_cast<QTextPieceTable*>(priv->pieceTable), SIGNAL(contentsChanged()), priv->q, SIGNAL(contentsChanged()));
    QObject::connect(static_cast<QTextPieceTable*>(priv->pieceTable), SIGNAL(undoAvailable(bool)), priv->q, SIGNAL(undoAvailable(bool)));
    QObject::connect(static_cast<QTextPieceTable*>(priv->pieceTable), SIGNAL(redoAvailable(bool)), priv->q, SIGNAL(redoAvailable(bool)));
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
    pass an instance of your subclass to the QTextDocument constructor
    if you want to use your own layout logic.

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
    init(d);
}

/*!
    Constructs a QTextDocument containing the plain (unformatted) text
    \a text, and with parent \a parent.
*/
QTextDocument::QTextDocument(const QString &text, QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    init(d);
    QTextCursor(this).insertText(text);
}

/*!
    Constructs a QTextDocument with a custom document layout \a
    documentLayout, and with parent \a parent.
*/
QTextDocument::QTextDocument(QAbstractTextDocumentLayout *documentLayout, QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    init(d, documentLayout);
}

/*!
    Destroys the document.
*/
QTextDocument::~QTextDocument()
{
}

/*!
  Returns the plain text contained in the document. If you want
  formatting information use a QTextCursor instead.
*/
QString QTextDocument::plainText() const
{
    QString txt = d->pieceTable->plainText();
    txt.replace(QChar::ParagraphSeparator, '\n');
    return txt;
}

/*!
    Returns true if the document is empty; otherwise returns false.
*/
bool QTextDocument::isEmpty() const
{
    /* because if we're empty we still have one single paragraph as
     * one single fragment */
    return d->pieceTable->length() <= 1;
}

/*!
    \fn void QTextDocument::undo();

    Undoes the last editing operation on the document if
    \link QTextDocument::isUndoAvailable() undo is available\endlink.
*/

/*!
    \fn void QTextDocument::redo();

    Redoes the last editing operation on the document if \link
    QTextDocument::isRedoAvailable() redo is available\endlink.
*/

/*! \internal
 */
void QTextDocument::undoRedo(bool undo)
{
    d->pieceTable->undoRedo(undo);
}

/*!
    \internal

    Appends a custom undo \a item to the undo stack.
*/
void QTextDocument::appendUndoItem(QAbstractUndoItem *item)
{
    d->pieceTable->appendUndoItem(item);
}

/*!
    \property QTextDocument::undoRedoEnabled
    \brief Whether undo/redo are enabled for this document.

    This defaults to true. If disabled the undo stack is cleared and
    no items will be added to it.
*/
void QTextDocument::setUndoRedoEnabled(bool enable)
{
    d->pieceTable->enableUndoRedo(enable);
}

bool QTextDocument::isUndoRedoEnabled() const
{
    return d->pieceTable->isUndoRedoEnabled();
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
    return d->pieceTable->isUndoAvailable();
}

/*!
    Returns true is redo is available; otherwise returns false.
*/
bool QTextDocument::isRedoAvailable() const
{
    return d->pieceTable->isRedoAvailable();
}


/*!
    Returns the document layout for this document.
*/
QAbstractTextDocumentLayout *QTextDocument::documentLayout() const
{
    return d->pieceTable->layout();
}


/*!
    Returns the document's title.
*/
QString QTextDocument::documentTitle() const
{
    return d->pieceTable->config()->title;
}

/*!
    Sets the page size for the current documentLayout() to \a s.

    \sa pageSize()
*/
void QTextDocument::setPageSize(const QSize &s)
{
    d->pieceTable->layout()->setPageSize(s);
}

/*!
    Returns the page size for the current documentLayout().

    \sa setPageSize() numPages()
*/
QSize QTextDocument::pageSize() const
{
    return d->pieceTable->layout()->pageSize();
}

/*!
    Returns the number of pages that the document occupies using the
    current documentLayout() and pageSize().
*/
int QTextDocument::numPages() const
{
    return d->pieceTable->layout()->numPages();
}


/*!
    Insert the arbitrary piece of HTML formatted text in \a html into
    the document at the current cursor position.
*/
void QTextDocument::setHtml(const QString &html)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(html);
    QTextCursor cursor(this);
    cursor.moveTo(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.insertFragment(fragment);
}


/*!
    Returns the name of the anchor at point \a pos, or an empty string
    if there's no anchor at that point.
*/
QString QTextDocument::anchorAt(const QPoint& pos) const
{
    int cursorPos = d->pieceTable->layout()->hitTest(pos, QText::ExactHit);
    if (cursorPos == -1)
        return QString();

    QTextPieceTable::FragmentIterator it = d->pieceTable->find(cursorPos);
    QTextCharFormat fmt = d->pieceTable->formatCollection()->charFormat(it->format);
    return fmt.anchorName();
}

/*!
    \overload

    Finds the next occurrence of the string, \a expr, starting at
    position \a from. Returns a cursor with the match selected if \a
    expr was found; otherwise returns a null cursor.

    If \a from is 0 (the default) the search begins from the beginning
    of the document; otherwise from the specified position.

    If \a cs is QString::CaseSensitive (the default), the search is
    case sensitive; otherwise the search is case insensitive. If \a
    mode is \c FindAnything the default) the search looks for any
    matching text; otherwise it only searches for whole word matches.
*/
QTextCursor QTextDocument::find(const QString &expr, int from, QString::CaseSensitivity cs, FindMode mode) const
{
    if (expr.isEmpty())
        return QTextCursor();

    int pos = from;

    QTextBlockIterator block = d->pieceTable->blocksFind(pos);
    while (!block.atEnd()) {
        const int blockOffset = qMax(0, pos - block.position());
        const int idx = block.blockText().indexOf(expr, blockOffset, cs);
        if (idx >= 0) {
            QTextCursor cursor(d->pieceTable, block.position() + blockOffset + idx);
            // ### testme
            if (mode == FindWords) {
                const int findPos = cursor.position();
                cursor.moveTo(QTextCursor::NextWord);
                cursor.moveTo(QTextCursor::PreviousWord);
                // ### test end of word, too - needs something like EndOfWord in API
                if (cursor.position() != findPos) {
                    pos = findPos + 1;
                    continue;
                }

                // ### EndOfWord
                cursor.moveTo(QTextCursor::NextWord, QTextCursor::KeepAnchor);
            } else {
                cursor.moveTo(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, expr.length());
            }
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

    If \a cs is QString::CaseSensitive (the default), the search is
    case sensitive; otherwise the search is case insensitive. If \a
    mode is \c FindAnything the default) the search looks for any
    matching text; otherwise it only searches for whole word matches.
*/
QTextCursor QTextDocument::find(const QString &expr, const QTextCursor &from, QString::CaseSensitivity cs, FindMode mode) const
{
    const int pos = (from.isNull() ? 0 : from.selectionEnd());
    return find(expr, pos, cs, mode);
}

