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
    QObject::connect(priv->pieceTable, SIGNAL(contentsChanged()), priv->q, SIGNAL(contentsChanged()));
}

QTextDocument::QTextDocument(QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    init(d);
}

QTextDocument::QTextDocument(const QString &text, QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    init(d);
    QTextCursor(this).insertText(text);
}

QTextDocument::QTextDocument(QAbstractTextDocumentLayout *documentLayout, QObject *parent)
    : QObject(*new QTextDocumentPrivate, parent)
{
    init(d, documentLayout);
}

QTextDocument::~QTextDocument()
{
}

QString QTextDocument::plainText() const
{
    QString txt = d->pieceTable->plainText();
    txt.replace(QTextParagraphSeparator, "\n");
    // remove initial paragraph
    txt.remove(0, 1);
    return txt;
}

bool QTextDocument::isEmpty() const
{
    /* because if we're empty we still have one single paragraph as
     * one single fragment */
    return d->pieceTable->length() <= 1;
}

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

bool QTextDocument::isUndoRedoAvailable() const
{
    return d->pieceTable->isUndoRedoAvailable();
}

QAbstractTextDocumentLayout *QTextDocument::documentLayout() const
{
    return d->pieceTable->layout();
}


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
