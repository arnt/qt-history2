#include "qtextdocument.h"
#include "qtextpiecetable_p.h"
#include <qtextformat.h>
#include "qtextdocumentlayout_p.h"

#include "qtextdocument_p.h"
#define d d_func()
#define q q_func()

static void init(QTextDocumentPrivate *priv, QAbstractTextDocumentLayout *layout = 0)
{
    priv->pieceTable = new QTextPieceTable(layout);
    QObject::connect(static_cast<QTextPieceTable*>(priv->pieceTable), SIGNAL(contentsChanged()), priv->q, SIGNAL(contentsChanged()));
}


/*!
    \class QTextDocument qtextdocument.h
    \brief A document of text that can be displayed using a QTextEditor

    \ingroup text

    A QTextDocument is a rich text document that can be viewed and
    edited using a QTextEditor. Programmatic editing of the document
    is possible using a \a QTextCursor on the document.

    Custom layouting of the document can be achieved by creating your
    own \a QAbstractTextDocumentLayout class and setting this as the
    document layout for the document at construction time.

*/


/*!
  Constructs an empty QTextDocument.
*/
QTextDocument::QTextDocument(QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    init(d);
}

/*!
  Constructs a QTextDocument containing the (unformatted) text \a text.
*/
QTextDocument::QTextDocument(const QString &text, QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    init(d);
    QTextCursor(this).insertText(text);
}

/*!
  Constructs a QTextDocument with a custom document layout \a documentLayout.
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
  Returns the plain text (without formatting information) contained in the document.
*/
QString QTextDocument::plainText() const
{
    QString txt = d->pieceTable->plainText();
    txt.replace(QChar::ParagraphSeparator, '\n');
    return txt;
}

/*!
  Returns true if the document is empty (ie. it doesn't contain any text).
*/
bool QTextDocument::isEmpty() const
{
    /* because if we're empty we still have one single paragraph as
     * one single fragment */
    return d->pieceTable->length() <= 1;
}

/*!
  \fn void QTextDocument::undo();

  Undoes the last editing operation on the document.
*/

/*!
  \fn void QTextDocument::redo();

  Redoes the last editing operation on the document.
*/

/*! \internal
 */
void QTextDocument::undoRedo(bool undo)
{
    d->pieceTable->undoRedo(undo);
}

/*!
    Appends a custom undo \a item to the undo stack.
*/
void QTextDocument::appendUndoItem(QAbstractUndoItem *item)
{
    d->pieceTable->appendUndoItem(item);
}

/*!
    \property QTextDocument::enableUndoRedo

    Enables the document's undo stack if \a enable is true; disables
    it if \a enable is false. Disabling the undo stack will also
    remove all undo items currently on the stack. The default is
    enabled.
*/
void QTextDocument::enableUndoRedo(bool enable)
{
    d->pieceTable->enableUndoRedo(enable);
}

bool QTextDocument::isUndoRedoEnabled() const
{
    return d->pieceTable->isUndoRedoEnabled();
}

/*!
  \fn QTextDocument::undoAvailable(bool b);

  This signal is emitted whenever undo operations become available or unavailable.
*/

/*!
  \fn QTextDocument::redoAvailable(bool b);

  This signal is emitted whenever redo operations become available or unavailable.
*/

/*!
  Returns true is undo is available.
*/
bool QTextDocument::isUndoAvailable() const
{
    return d->pieceTable->isUndoAvailable();
}

/*!
  Returns true is redo is available.
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
  Returns the title of this document.
*/
QString QTextDocument::documentTitle() const
{
    return d->pieceTable->config()->title;
}

void QTextDocument::setPageSize(const QSize &s)
{
    d->pieceTable->layout()->setPageSize(s);
}

QSize QTextDocument::pageSize() const
{
    return d->pieceTable->layout()->pageSize();
}

int QTextDocument::numPages() const
{
    return d->pieceTable->layout()->numPages();
}
