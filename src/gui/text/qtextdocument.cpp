#include "qtextdocument.h"
#include "qtextpiecetable_p.h"
#include <qtextformat.h>
#include "qtextdocumentlayout_p.h"

QTextDocument::QTextDocument(QObject *parent)
    : QObject(parent), pieceTable(new QTextPieceTable)
{
    init();
}

QTextDocument::QTextDocument(const QString &text, QObject *parent)
    : QObject(parent), pieceTable(new QTextPieceTable)
{
    init();

    QTextCursor(this).insertText(text);
}

QTextDocument::~QTextDocument()
{
}

QString QTextDocument::plainText() const
{
    QString txt = pieceTable->plainText();
    txt.replace(QTextParagraphSeparator, "\n");
    // remove initial paragraph
    txt.remove(0, 1);
    return txt;
}

bool QTextDocument::isEmpty() const
{
    return !pieceTable->length();
}

void QTextDocument::undoRedo(bool undo)
{
    pieceTable->undoRedo(undo);
}

/*!
    Appends a custom undo \a item to the undo stack.
*/
void QTextDocument::appendUndoItem(QAbstractUndoItem *item)
{
    pieceTable->appendUndoItem(item);
}

/*!
    Enables the document's undo stack if \a enable is true; disables
    it if \a enable is false. Disabling the undo stack will also
    remove all undo items currently on the stack. The default is
    enabled.
*/
void QTextDocument::enableUndoRedo(bool enable)
{
    pieceTable->enableUndoRedo(enable);
}

bool QTextDocument::isUndoRedoEnabled() const
{
    return pieceTable->isUndoRedoEnabled();
}

bool QTextDocument::isUndoRedoAvailable() const
{
    return pieceTable->isUndoRedoAvailable();
}

QString QTextDocument::documentTitle() const
{
    return pieceTable->config()->title;
}

void QTextDocument::init()
{
    connect(pieceTable, SIGNAL(contentsChanged()), this, SIGNAL(contentsChanged()));
}

void QTextDocument::setPageSize(const QSize &s)
{
    pieceTable->layout()->setPageSize(s);
}

QSize QTextDocument::pageSize() const
{
    return pieceTable->layout()->pageSize();
}

int QTextDocument::numPages() const
{
    return pieceTable->layout()->numPages();
}

QRect QTextDocument::pageRect(int page) const
{
    return pieceTable->layout()->pageRect(page);
}
