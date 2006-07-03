/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextcontrol_p.h"
#include "qtextcontrol_p_p.h"
#include "qlineedit.h"

#ifndef QT_NO_TEXTCONTROL

#include <qfont.h>
#include <qpainter.h>
#include <qevent.h>
#include <qdebug.h>
#include <qmime.h>
#include <qdrag.h>
#include <qclipboard.h>
#include <qmenu.h>
#include <qstyle.h>
#include <qtimer.h>
#include "private/qtextdocumentlayout_p.h"
#include "private/qtextedit_p.h"
#include "qtextdocument.h"
#include "private/qtextdocument_p.h"
#include "qtextlist.h"
#include "private/qtextcontrol_p.h"
#include "qgraphicssceneevent.h"

#include <qtextformat.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <limits.h>
#include <qtexttable.h>
#include <qvariant.h>
#include <qurl.h>
#include <qdesktopservices.h>
#include <qinputcontext.h>

#ifndef QT_NO_SHORTCUT
#include <qkeysequence.h>
#define ACCEL_KEY(k) QString::fromLatin1("\t") + QString(QKeySequence( Qt::CTRL | Qt::Key_ ## k ))
#else
#define ACCEL_KEY(k) QString("\tCtrl+" #k)
#endif

// could go into QTextCursor...
static QTextLine currentTextLine(const QTextCursor &cursor)
{
    const QTextBlock block = cursor.block();
    if (!block.isValid())
        return QTextLine();

    const QTextLayout *layout = block.layout();
    if (!layout)
        return QTextLine();

    const int relativePos = cursor.position() - block.position();
    return layout->lineForTextPosition(relativePos);
}

QTextControlPrivate::QTextControlPrivate()
    : doc(0), cursorOn(false), cursorIsFocusIndicator(false),
      interactionFlags(Qt::TextEditorInteraction),
#ifndef QT_NO_DRAGANDDROP
      mousePressed(false), mightStartDrag(false),
#endif
      lastSelectionState(false), ignoreAutomaticScrollbarAdjustement(false),
      overwriteMode(false),
      acceptRichText(true),
      preeditCursor(0), hideCursor(false),
      hasFocus(false),
      layoutDirection(QApplication::layoutDirection()),
      isEnabled(true),
      hadSelectionOnMousePress(false),
      openExternalLinks(false)
{}

bool QTextControlPrivate::cursorMoveKeyEvent(QKeyEvent *e)
{
    Q_Q(QTextControl);
    if (cursor.isNull())
        return false;

    const QTextCursor oldSelection = cursor;
    const int oldCursorPos = cursor.position();

    QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;
    QTextCursor::MoveOperation op = QTextCursor::NoMove;
    

    if (e == QKeySequence::MoveToNextChar) {
            op = QTextCursor::Right;
    }
    else if (e == QKeySequence::MoveToPreviousChar) {
            op = QTextCursor::Left;
    }
    else if (e == QKeySequence::SelectNextChar) {
           op = QTextCursor::Right;
           mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousChar) {
            op = QTextCursor::Left;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectNextWord) {
            op = QTextCursor::WordRight;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousWord) {
            op = QTextCursor::WordLeft;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfLine) {
            op = QTextCursor::StartOfLine;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfLine) {
            op = QTextCursor::EndOfLine;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfBlock) {
            op = QTextCursor::StartOfBlock;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfBlock) {
            op = QTextCursor::EndOfBlock;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectStartOfDocument) {
            op = QTextCursor::Start;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectEndOfDocument) {
            op = QTextCursor::End;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectPreviousLine) {
            op = QTextCursor::Up;
            mode = QTextCursor::KeepAnchor;
    }
    else if (e == QKeySequence::SelectNextLine) {
            op = QTextCursor::Down;
            mode = QTextCursor::KeepAnchor;
            {
                QTextBlock block = cursor.block();
                QTextLine line = currentTextLine(cursor);
                if (!block.next().isValid()
                    && line.isValid()
                    && line.lineNumber() == block.layout()->lineCount() - 1)
                    op = QTextCursor::End;
            }
    }
    else if (e == QKeySequence::SelectNextLine) {
            op = QTextCursor::Down;
            mode = QTextCursor::KeepAnchor;
            {
                QTextBlock block = cursor.block();
                QTextLine line = currentTextLine(cursor);
                if (!block.next().isValid()
                    && line.isValid()
                    && line.lineNumber() == block.layout()->lineCount() - 1)
                    op = QTextCursor::End;
            }
    }
    else if (e == QKeySequence::MoveToNextWord) {
            op = QTextCursor::WordRight;
    }
    else if (e == QKeySequence::MoveToPreviousWord) {
            op = QTextCursor::WordLeft;
    }
    else if (e == QKeySequence::MoveToEndOfBlock) {
            op = QTextCursor::EndOfBlock;
    }
    else if (e == QKeySequence::MoveToStartOfBlock) {
            op = QTextCursor::StartOfBlock;
    }
    else if (e == QKeySequence::MoveToNextLine) {
            op = QTextCursor::Down;
    }
    else if (e == QKeySequence::MoveToPreviousLine) {
            op = QTextCursor::Up;
    }
    else if (e == QKeySequence::MoveToPreviousLine) {
            op = QTextCursor::Up;
    }
    else if (e == QKeySequence::MoveToStartOfLine) {
            op = QTextCursor::StartOfLine;
    }
    else if (e == QKeySequence::MoveToEndOfLine) {
            op = QTextCursor::EndOfLine;
    }
    else if (e == QKeySequence::MoveToStartOfDocument) {
            op = QTextCursor::Start;
    }
    else if (e == QKeySequence::MoveToEndOfDocument) {
            op = QTextCursor::End;
    } 
    else {
        return false;
    }
// Except for pageup and pagedown, Mac OS X has very different behavior, we don't do it all, but
// here's the breakdown:
// Shift still works as an anchor, but only one of the other keys can be down Ctrl (Command),
// Alt (Option), or Meta (Control).
// Command/Control + Left/Right -- Move to left or right of the line
//                 + Up/Down -- Move to top bottom of the file. (Control doesn't move the cursor)
// Option + Left/Right -- Move one word Left/right.
//        + Up/Down  -- Begin/End of Paragraph.
// Home/End Top/Bottom of file. (usually don't move the cursor, but will select)
   
    const bool moved = cursor.movePosition(op, mode);
    q->ensureCursorVisible();

    if (moved) {
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
        emit q->microFocusChanged();
    }
#ifdef QT_KEYPAD_NAVIGATION
    else if (QApplication::keypadNavigationEnabled()
        && (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)) {
        return false;
    }
#endif

    selectionChanged();

    repaintOldAndNewSelection(oldSelection);

    return true;
}

void QTextControlPrivate::updateCurrentCharFormat()
{
    Q_Q(QTextControl);

    QTextCharFormat fmt = cursor.charFormat();
    if (fmt == lastCharFormat)
        return;
    lastCharFormat = fmt;

    emit q->currentCharFormatChanged(fmt);
    emit q->microFocusChanged();
}

void QTextControlPrivate::indent()
{
    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextList *list = cursor.currentList();
    if (!list) {
        QTextBlockFormat modifier;
        modifier.setIndent(blockFmt.indent() + 1);
        cursor.mergeBlockFormat(modifier);
    } else {
        QTextListFormat format = list->format();
        format.setIndent(format.indent() + 1);

        if (list->itemNumber(cursor.block()) == 1)
            list->setFormat(format);
        else
            cursor.createList(format);
    }
}

void QTextControlPrivate::outdent()
{
    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextList *list = cursor.currentList();

    if (!list) {
        QTextBlockFormat modifier;
        modifier.setIndent(blockFmt.indent() - 1);
        cursor.mergeBlockFormat(modifier);
    } else {
        QTextListFormat listFmt = list->format();
        listFmt.setIndent(listFmt.indent() - 1);
        list->setFormat(listFmt);
    }
}

void QTextControlPrivate::gotoNextTableCell()
{
    QTextTable *table = cursor.currentTable();
    QTextTableCell cell = table->cellAt(cursor);

    int newColumn = cell.column() + cell.columnSpan();
    int newRow = cell.row();

    if (newColumn >= table->columns()) {
        newColumn = 0;
        ++newRow;
        if (newRow >= table->rows())
            table->insertRows(table->rows(), 1);
    }

    cell = table->cellAt(newRow, newColumn);
    cursor = cell.firstCursorPosition();
}

void QTextControlPrivate::gotoPreviousTableCell()
{
    QTextTable *table = cursor.currentTable();
    QTextTableCell cell = table->cellAt(cursor);

    int newColumn = cell.column() - 1;
    int newRow = cell.row();

    if (newColumn < 0) {
        newColumn = table->columns() - 1;
        --newRow;
        if (newRow < 0)
            return;
    }

    cell = table->cellAt(newRow, newColumn);
    cursor = cell.firstCursorPosition();
}

void QTextControlPrivate::createAutoBulletList()
{
    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextListFormat listFmt;
    listFmt.setStyle(QTextListFormat::ListDisc);
    listFmt.setIndent(blockFmt.indent() + 1);

    blockFmt.setIndent(0);
    cursor.setBlockFormat(blockFmt);

    cursor.createList(listFmt);

    cursor.endEditBlock();
}

void QTextControlPrivate::setContent(Qt::TextFormat format, const QString &text, QTextDocument *document)
{
    Q_Q(QTextControl);

    // for use when called from setPlainText. we may want to re-use the currently
    // set char format then.
    const QTextCharFormat charFormatForInsertion = cursor.charFormat();

    bool clearDocument = true;
    if (!doc) {
        if (document) {
            doc = document;
            clearDocument = false;
        } else {
            doc = new QTextDocument(q);
        }

        QObject::connect(doc->documentLayout(), SIGNAL(update(QRectF)), q, SIGNAL(updateRequest(QRectF)));
        QObject::connect(doc->documentLayout(), SIGNAL(documentSizeChanged(QSizeF)), q, SIGNAL(documentSizeChanged(QSizeF)));
        cursor = QTextCursor(doc);

// ####        doc->documentLayout()->setPaintDevice(viewport);

        QObject::connect(doc, SIGNAL(contentsChanged()), q, SLOT(_q_updateCurrentCharFormatAndSelection()));
        QObject::connect(doc, SIGNAL(cursorPositionChanged(QTextCursor)), q, SLOT(_q_emitCursorPosChanged(QTextCursor)));

        // convenience signal forwards
        QObject::connect(doc, SIGNAL(contentsChanged()), q, SIGNAL(textChanged()));
        QObject::connect(doc, SIGNAL(undoAvailable(bool)), q, SIGNAL(undoAvailable(bool)));
        QObject::connect(doc, SIGNAL(redoAvailable(bool)), q, SIGNAL(redoAvailable(bool)));
    }

    doc->setUndoRedoEnabled(false);

    // avoid multiple textChanged() signals being emitted
    QObject::disconnect(doc, SIGNAL(contentsChanged()), q, SIGNAL(textChanged()));

    if (!text.isEmpty()) {
        // clear 'our' cursor for insertion to prevent
        // the emission of the cursorPositionChanged() signal.
        // instead we emit it only once at the end instead of
        // at the end of the document after loading and when
        // positioning the cursor again to the start of the
        // document.
        cursor = QTextCursor();
        if (format == Qt::PlainText) {
            QTextCursor formatCursor(doc);
            // put the setPlainText and the setCharFormat into one edit block,
            // so that the syntax highlight triggers only /once/ for the entire
            // document, not twice.
            formatCursor.beginEditBlock();
            doc->setPlainText(text);
            doc->setUndoRedoEnabled(false);
            formatCursor.select(QTextCursor::Document);
            formatCursor.setCharFormat(charFormatForInsertion);
            formatCursor.endEditBlock();
        } else {
            doc->setHtml(text);
            doc->setUndoRedoEnabled(false);
        }
        cursor = QTextCursor(doc);
    } else if (clearDocument) {
        doc->clear();
        cursor.movePosition(QTextCursor::Start);
        QTextBlockFormat blockFmt;
        blockFmt.setLayoutDirection(layoutDirection);
        cursor.mergeBlockFormat(blockFmt);
        cursor.setCharFormat(charFormatForInsertion);
    }

    QObject::connect(doc, SIGNAL(contentsChanged()), q, SIGNAL(textChanged()));
    emit q->textChanged();
    doc->setUndoRedoEnabled(interactionFlags & Qt::TextEditable);
    _q_updateCurrentCharFormatAndSelection();
    doc->setModified(false);
    emit q->cursorPositionChanged();
}

void QTextControlPrivate::startDrag()
{
#ifndef QT_NO_DRAGANDDROP
    Q_Q(QTextControl);
    mousePressed = false;
    if (!contextWidget)
        return;
    QMimeData *data = q->createMimeDataFromSelection();

    QDrag *drag = new QDrag(contextWidget);
    drag->setMimeData(data);

    Qt::DropActions actions = Qt::CopyAction;
    if (interactionFlags & Qt::TextEditable)
        actions |= Qt::MoveAction;
    Qt::DropAction action = drag->start(actions);

    if (action == Qt::MoveAction && drag->target() != contextWidget)
        cursor.removeSelectedText();
#endif
}

void QTextControlPrivate::setCursorPosition(const QPointF &pos)
{
    const int cursorPos = doc->documentLayout()->hitTest(pos, Qt::FuzzyHit);
    if (cursorPos == -1)
        return;
    cursor.setPosition(cursorPos);
}

void QTextControlPrivate::setCursorPosition(int pos, QTextCursor::MoveMode mode)
{
    cursor.setPosition(pos, mode);

    if (mode != QTextCursor::KeepAnchor) {
        selectedWordOnDoubleClick = QTextCursor();
        selectedLineOnDoubleClick = QTextCursor();
    }
}

void QTextControlPrivate::repaintCursor()
{
    Q_Q(QTextControl);
    emit q->updateRequest(q->cursorRect());
}

void QTextControlPrivate::repaintOldAndNewSelection(const QTextCursor &oldSelection)
{
    Q_Q(QTextControl);
    QRectF updateRect = selectionRect() | selectionRect(oldSelection);

    if (cursor.hasSelection()
        && oldSelection.hasSelection()
        && cursor.currentFrame() == oldSelection.currentFrame()
        && !cursor.hasComplexSelection()
        && !oldSelection.hasComplexSelection()
        && cursor.anchor() == oldSelection.anchor()) {
        QTextCursor differenceSelection(doc);
        differenceSelection.setPosition(oldSelection.position());
        differenceSelection.setPosition(cursor.position(), QTextCursor::KeepAnchor);
        emit q->updateRequest(selectionRect(differenceSelection));
    } else {
        emit q->updateRequest(selectionRect(oldSelection));
        emit q->updateRequest(selectionRect());
    }
}

void QTextControlPrivate::selectionChanged()
{
    Q_Q(QTextControl);
    bool current = cursor.hasSelection();
    if (current == lastSelectionState)
        return;

    lastSelectionState = current;
    emit q->copyAvailable(current);
    emit q->selectionChanged();
    emit q->microFocusChanged();
}

void QTextControlPrivate::_q_updateCurrentCharFormatAndSelection()
{
    updateCurrentCharFormat();
    selectionChanged();
}

#ifndef QT_NO_CLIPBOARD
void QTextControlPrivate::setClipboardSelection()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (!cursor.hasSelection() || !clipboard->supportsSelection())
        return;
    Q_Q(QTextControl);
    QMimeData *data = q->createMimeDataFromSelection();
    clipboard->setMimeData(data, QClipboard::Selection);
}
#endif

void QTextControlPrivate::_q_emitCursorPosChanged(const QTextCursor &someCursor)
{
    Q_Q(QTextControl);
    if (someCursor.isCopyOf(cursor)) {
        emit q->cursorPositionChanged();
        emit q->microFocusChanged();
    }
}

void QTextControlPrivate::setBlinkingCursorEnabled(bool enable)
{
    Q_Q(QTextControl);
    if (enable && QApplication::cursorFlashTime() > 0) {
        cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, q);
        cursorOn = true;
    } else {
        cursorBlinkTimer.stop();
        cursorOn = false;
    }
    repaintCursor();
}

void QTextControlPrivate::extendWordwiseSelection(int suggestedNewPosition, qreal mouseXPosition)
{
    Q_Q(QTextControl);

    // if inside the initial selected word keep that
    if (suggestedNewPosition >= selectedWordOnDoubleClick.selectionStart()
        && suggestedNewPosition <= selectedWordOnDoubleClick.selectionEnd()) {
        q->setTextCursor(selectedWordOnDoubleClick);
        return;
    }

    QTextCursor curs = selectedWordOnDoubleClick;
    curs.setPosition(suggestedNewPosition, QTextCursor::KeepAnchor);

    if (!curs.movePosition(QTextCursor::StartOfWord))
        return;
    const int wordStartPos = curs.position();

    const int blockPos = curs.block().position();
    const QPointF blockCoordinates = doc->documentLayout()->blockBoundingRect(curs.block()).topLeft();

    QTextLine line = currentTextLine(curs);
    if (!line.isValid())
        return;

    const qreal wordStartX = line.cursorToX(curs.position() - blockPos) + blockCoordinates.x();

    if (!curs.movePosition(QTextCursor::EndOfWord))
        return;
    const int wordEndPos = curs.position();

    const QTextLine otherLine = currentTextLine(curs);
    if (otherLine.textStart() != line.textStart()
        || wordEndPos == wordStartPos)
        return;

    const qreal wordEndX = line.cursorToX(curs.position() - blockPos) + blockCoordinates.x();

    if (mouseXPosition < wordStartX || mouseXPosition > wordEndX)
        return;

    // keep the already selected word even when moving to the left
    // (#39164)
    if (suggestedNewPosition < selectedWordOnDoubleClick.position())
        cursor.setPosition(selectedWordOnDoubleClick.selectionEnd());
    else
        cursor.setPosition(selectedWordOnDoubleClick.selectionStart());

    const qreal differenceToStart = mouseXPosition - wordStartX;
    const qreal differenceToEnd = wordEndX - mouseXPosition;

    if (differenceToStart < differenceToEnd)
        setCursorPosition(wordStartPos, QTextCursor::KeepAnchor);
    else
        setCursorPosition(wordEndPos, QTextCursor::KeepAnchor);
}

void QTextControlPrivate::extendLinewiseSelection(int suggestedNewPosition)
{
    Q_Q(QTextControl);

    // if inside the initial selected line keep that
    if (suggestedNewPosition >= selectedLineOnDoubleClick.selectionStart()
        && suggestedNewPosition <= selectedLineOnDoubleClick.selectionEnd()) {
        q->setTextCursor(selectedLineOnDoubleClick);
        return;
    }

    if (suggestedNewPosition < selectedLineOnDoubleClick.position()) {
        cursor.setPosition(selectedLineOnDoubleClick.selectionEnd());
        cursor.setPosition(suggestedNewPosition, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(selectedLineOnDoubleClick.selectionStart());
        cursor.setPosition(suggestedNewPosition, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
}

void QTextControlPrivate::_q_deleteSelected()
{
    if (!(interactionFlags & Qt::TextEditable) || !cursor.hasSelection())
	return;
    cursor.removeSelectedText();
}

void QTextControl::undo()
{
    Q_D(QTextControl);
    QObject::connect(d->doc, SIGNAL(contentsChange(int, int, int)),
                     this, SLOT(_q_setCursorAfterUndoRedo(int, int, int)));
    d->doc->undo();
    QObject::disconnect(d->doc, SIGNAL(contentsChange(int, int, int)),
                        this, SLOT(_q_setCursorAfterUndoRedo(int, int, int)));
    ensureCursorVisible();
}

void QTextControl::redo()
{
    Q_D(QTextControl);
    QObject::connect(d->doc, SIGNAL(contentsChange(int, int, int)),
                     this, SLOT(_q_setCursorAfterUndoRedo(int, int, int)));
    d->doc->redo();
    QObject::disconnect(d->doc, SIGNAL(contentsChange(int, int, int)),
                        this, SLOT(_q_setCursorAfterUndoRedo(int, int, int)));
    ensureCursorVisible();
}

void QTextControlPrivate::_q_setCursorAfterUndoRedo(int undoPosition, int /*charsRemoved*/, int charsAdded)
{
    if (cursor.isNull())
        return;

    cursor.setPosition(undoPosition + charsAdded);
    // don't call ensureCursorVisible here but wait until the text is layouted, which will
    // be the case in QTextControl::undo() after calling undo() on the document.
}

QTextControl::QTextControl(QObject *parent)
    : QObject(*new QTextControlPrivate, parent)
{
    Q_D(QTextControl);
    d->setContent(); // init
}

QTextControl::QTextControl(const QString &text, QObject *parent)
    : QObject(*new QTextControlPrivate, parent)
{
    Q_D(QTextControl);
    d->setContent(Qt::RichText, text); // init
}

QTextControl::QTextControl(QTextDocument *doc, QObject *parent)
    : QObject(*new QTextControlPrivate, parent)
{
    Q_D(QTextControl);
    d->setContent(Qt::RichText, QString(), doc); // init
}

QTextControl::~QTextControl()
{
}

void QTextControl::setDocument(QTextDocument *document)
{
    Q_D(QTextControl);
    if (d->doc == document)
        return;

    d->doc->disconnect(this);
    d->doc->documentLayout()->disconnect(this);
    d->doc->documentLayout()->setPaintDevice(0);

    if (d->doc->parent() == this)
        delete d->doc;

    d->doc = 0;
    d->setContent(Qt::RichText, QString(), document);
}

QTextDocument *QTextControl::document() const
{
    Q_D(const QTextControl);
    return d->doc;
}

void QTextControl::setTextCursor(const QTextCursor &cursor)
{
    Q_D(QTextControl);
    const bool posChanged = cursor.position() != d->cursor.position();
    const QTextCursor oldSelection = d->cursor;
    d->cursor = cursor;
    d->_q_updateCurrentCharFormatAndSelection();
    ensureCursorVisible();
    d->repaintOldAndNewSelection(oldSelection);
    if (posChanged)
        emit cursorPositionChanged();
}

QTextCursor QTextControl::textCursor() const
{
    Q_D(const QTextControl);
    return d->cursor;
}

#ifndef QT_NO_CLIPBOARD

void QTextControl::cut()
{
    Q_D(QTextControl);
    if (!(d->interactionFlags & Qt::TextEditable) || !d->cursor.hasSelection())
	return;
    copy();
    d->cursor.removeSelectedText();
}

void QTextControl::copy()
{
    Q_D(QTextControl);
    if (!d->cursor.hasSelection())
	return;
    QMimeData *data = createMimeDataFromSelection();
    QApplication::clipboard()->setMimeData(data);
}

void QTextControl::paste()
{
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if (md)
        insertFromMimeData(md);
}
#endif

void QTextControl::clear()
{
    Q_D(QTextControl);
    // clears and sets empty content
    d->setContent();
}


void QTextControl::selectAll()
{
    Q_D(QTextControl);
    d->cursor.select(QTextCursor::Document);
    d->selectionChanged();
    emit updateRequest(QRectF(QPointF(), d->doc->documentLayout()->documentSize()));
}

void QTextControl::processEvent(QEvent *e, const QPointF &coordinateOffset, QWidget *contextWidget)
{
    QMatrix m;
    m.translate(coordinateOffset.x(), coordinateOffset.y());
    processEvent(e, m, contextWidget);
}

void QTextControl::processEvent(QEvent *e, const QMatrix &matrix, QWidget *contextWidget)
{

    Q_D(QTextControl);
    if (d->interactionFlags & Qt::NoTextInteraction)
        return;

    d->contextWidget = contextWidget;

    if (!d->contextWidget) {
        switch (e->type()) {
            case QEvent::GraphicsSceneMouseMove:
            case QEvent::GraphicsSceneMousePress:
            case QEvent::GraphicsSceneMouseRelease:
            case QEvent::GraphicsSceneMouseDoubleClick:
            case QEvent::GraphicsSceneContextMenu:
            case QEvent::GraphicsSceneHoverEnter:
            case QEvent::GraphicsSceneHoverMove:
            case QEvent::GraphicsSceneHoverLeave:
            case QEvent::GraphicsSceneHelp:
            case QEvent::GraphicsSceneDragEnter:
            case QEvent::GraphicsSceneDragMove:
            case QEvent::GraphicsSceneDragLeave:
            case QEvent::GraphicsSceneDrop: {
                QGraphicsSceneEvent *ev = static_cast<QGraphicsSceneEvent *>(e);
                d->contextWidget = ev->widget();
                break;
            }
            default: break;
        };
    }

    switch (e->type()) {
        case QEvent::KeyPress:
            d->keyPressEvent(static_cast<QKeyEvent *>(e));
            break;
        case QEvent::MouseButtonPress: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(e);
            d->mousePressEvent(ev->button(), matrix.map(ev->pos()), ev->modifiers(),
                               ev->buttons(), ev->globalPos());
            break; }
        case QEvent::MouseMove: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(e);
            d->mouseMoveEvent(ev->buttons(), matrix.map(ev->pos()));
            break; }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(e);
            d->mouseReleaseEvent(ev->button(), matrix.map(ev->pos()));
            break; }
        case QEvent::MouseButtonDblClick: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(e);
            d->mouseDoubleClickEvent(e, ev->button(), matrix.map(ev->pos()));
            break; }
        case QEvent::InputMethod:
            d->inputMethodEvent(static_cast<QInputMethodEvent *>(e));
            break;
        case QEvent::ContextMenu: {
            QContextMenuEvent *ev = static_cast<QContextMenuEvent *>(e);
            d->contextMenuEvent(ev->globalPos(), matrix.map(ev->pos()));
            break; }

        case QEvent::FocusIn:
        case QEvent::FocusOut:
            d->focusEvent(static_cast<QFocusEvent *>(e));
            break;

        case QEvent::EnabledChange:
            d->isEnabled = e->isAccepted();
            break;

#ifndef QT_NO_DRAGANDDROP
        case QEvent::DragEnter: {
            QDragEnterEvent *ev = static_cast<QDragEnterEvent *>(e);
            if (d->dragEnterEvent(e, ev->mimeData()))
                ev->acceptProposedAction();
            break;
        }
        case QEvent::DragLeave:
            d->dragLeaveEvent();
            break;
        case QEvent::DragMove: {
            QDragMoveEvent *ev = static_cast<QDragMoveEvent *>(e);
            if (d->dragMoveEvent(e, ev->mimeData(), matrix.map(ev->pos())))
                ev->acceptProposedAction();
            break;
        }
        case QEvent::Drop: {
            QDropEvent *ev = static_cast<QDropEvent *>(e);
            if (d->dropEvent(ev->mimeData(), matrix.map(ev->pos()), ev->proposedAction(), ev->source()))
                ev->acceptProposedAction();
            break;
        }
#endif

        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
            d->mousePressEvent(ev->button(), matrix.map(ev->pos()), ev->modifiers(), ev->buttons(),
                               ev->screenPos());
            break; }
        case QEvent::GraphicsSceneMouseMove: {
            QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
            d->mouseMoveEvent(ev->buttons(), matrix.map(ev->pos()));
            break; }
        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
            d->mouseReleaseEvent(ev->button(), matrix.map(ev->pos()));
            break; }
        case QEvent::GraphicsSceneMouseDoubleClick: {
            QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
            d->mouseDoubleClickEvent(e, ev->button(), matrix.map(ev->pos()));
            break; }
        case QEvent::GraphicsSceneContextMenu: {
            QGraphicsSceneContextMenuEvent *ev = static_cast<QGraphicsSceneContextMenuEvent *>(e);
            d->contextMenuEvent(ev->screenPos(), matrix.map(ev->pos()));
            break; }

        case QEvent::GraphicsSceneHoverMove: {
            QGraphicsSceneHoverEvent *ev = static_cast<QGraphicsSceneHoverEvent *>(e);
            d->mouseMoveEvent(Qt::NoButton, matrix.map(ev->pos()));
            break; }

        case QEvent::GraphicsSceneDragEnter: {
            QGraphicsSceneDragDropEvent *ev = static_cast<QGraphicsSceneDragDropEvent *>(e);
            if (d->dragEnterEvent(e, ev->mimeData()))
                ev->acceptProposedAction();
            break; }
        case QEvent::GraphicsSceneDragLeave:
            d->dragLeaveEvent();
            break;
        case QEvent::GraphicsSceneDragMove: {
            QGraphicsSceneDragDropEvent *ev = static_cast<QGraphicsSceneDragDropEvent *>(e);
            if (d->dragMoveEvent(e, ev->mimeData(), matrix.map(ev->pos())))
                ev->acceptProposedAction();
            break; }
        case QEvent::GraphicsSceneDrop: {
            QGraphicsSceneDragDropEvent *ev = static_cast<QGraphicsSceneDragDropEvent *>(e);
            if (d->dropEvent(ev->mimeData(), matrix.map(ev->pos()), ev->proposedAction(), ev->source()))
                ev->acceptProposedAction();
            break; }

        case QEvent::ShortcutOverride:
            if (d->interactionFlags & Qt::TextEditable) {
                QKeyEvent* ke = static_cast<QKeyEvent *>(e);
                if (ke->modifiers() == Qt::NoModifier
                    || ke->modifiers() == Qt::ShiftModifier
                    || ke->modifiers() == Qt::KeypadModifier) {
                    if (ke->key() < Qt::Key_Escape) {
                        ke->accept();
                    } else {
                        switch (ke->key()) {
                            case Qt::Key_Return:
                            case Qt::Key_Enter:
                            case Qt::Key_Delete:
                            case Qt::Key_Home:
                            case Qt::Key_End:
                            case Qt::Key_Backspace:
                            case Qt::Key_Left:
                            case Qt::Key_Right:
                            ke->accept();
                        default:
                            break;
                        }
                    }
                } else if (ke->modifiers() & Qt::ControlModifier) {
                    switch (ke->key()) {
                        case Qt::Key_C:
                        case Qt::Key_V:
                        case Qt::Key_X:
                        case Qt::Key_Y:
                        case Qt::Key_Z:
                        case Qt::Key_Left:
                        case Qt::Key_Right:
                        case Qt::Key_Up:
                        case Qt::Key_Down:
                        case Qt::Key_Home:
                        case Qt::Key_End:
#if !defined(Q_WS_MAC)
                        case Qt::Key_Insert:
                        case Qt::Key_Delete:
#endif
                        ke->accept();
                    default:
                        break;
                    }
                }
            }
            // FALL THROUGH
        default:
            break;
    }
}

bool QTextControl::event(QEvent *e)
{
    return QObject::event(e);
}

void QTextControl::timerEvent(QTimerEvent *e)
{
    Q_D(QTextControl);
    if (e->timerId() == d->cursorBlinkTimer.timerId()) {
        d->cursorOn = !d->cursorOn;

        if (d->cursor.hasSelection())
            d->cursorOn &= (QApplication::style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected)
                            != 0);

        d->repaintCursor();
    } else if (e->timerId() == d->dragStartTimer.timerId()) {
        d->dragStartTimer.stop();
        d->startDrag();
    } else if (e->timerId() == d->trippleClickTimer.timerId()) {
        d->trippleClickTimer.stop();
    }
}

void QTextControl::setPlainText(const QString &text)
{
    Q_D(QTextControl);
    d->setContent(Qt::PlainText, text);
}

void QTextControl::setHtml(const QString &text)
{
    Q_D(QTextControl);
    d->setContent(Qt::RichText, text);
}

void QTextControlPrivate::keyPressEvent(QKeyEvent *e)
{
    Q_Q(QTextControl);
    if (e == QKeySequence::SelectAll) {
            e->accept();
            q->selectAll();
            return;
    }
    else if (e == QKeySequence::Copy) {
            e->accept();
            q->copy();
            return;
    } 

    if (interactionFlags & Qt::TextSelectableByKeyboard
        && cursorMoveKeyEvent(e))
        goto accept;

    if (interactionFlags & Qt::LinksAccessibleByKeyboard) {
        if ((e->key() == Qt::Key_Return
             || e->key() == Qt::Key_Enter
#ifdef QT_KEYPAD_NAVIGATION
             || e->key() == Qt::Key_Select
#endif
             )
            && cursor.hasSelection()) {

            e->accept();

            QTextCursor tmp = cursor;
            if (tmp.selectionStart() != tmp.position())
                tmp.setPosition(tmp.selectionStart());
            tmp.movePosition(QTextCursor::NextCharacter);
            const QString href = tmp.charFormat().anchorHref();
            if (openExternalLinks)
                QDesktopServices::openUrl(href);
            else
                emit q->linkActivated(href);
            return;
        }
    }

    if (!(interactionFlags & Qt::TextEditable)) {
        e->ignore();
        return;
    }

    if (e->key() == Qt::Key_Direction_L || e->key() == Qt::Key_Direction_R) {
        QTextBlockFormat fmt;
        fmt.setLayoutDirection((e->key() == Qt::Key_Direction_L) ? Qt::LeftToRight : Qt::RightToLeft);
        cursor.mergeBlockFormat(fmt);
        goto accept;
    }

    // schedule a repaint of the region of the cursor, as when we move it we
    // want to make sure the old cursor disappears (not noticable when moving
    // only a few pixels but noticable when jumping between cells in tables for
    // example)
    repaintSelection();

    if (!e->modifiers()) { 
        switch (e->key()) {
        case Qt::Key_Backspace:
        {
            QTextBlockFormat blockFmt = cursor.blockFormat();
            QTextList *list = cursor.currentList();
            if (list && cursor.atBlockStart()) {
                list->remove(cursor.block());
            } else if (cursor.atBlockStart() && blockFmt.indent() > 0) {
                blockFmt.setIndent(blockFmt.indent() - 1);
                cursor.setBlockFormat(blockFmt);
            } else {
                cursor.deletePreviousChar();
            }
            goto accept;
        }
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (e->modifiers() & Qt::ControlModifier)
                cursor.insertText(QString(QChar::LineSeparator));
            else
                cursor.insertBlock();
            e->accept();
            goto accept;
        default:
            break;
        }
    }
    
    if (e == QKeySequence::Undo) {
            q->undo();
    }
    else if (e == QKeySequence::Redo) {
           q->redo();
    }
#ifndef QT_NO_CLIPBOARD
    else if (e == QKeySequence::Cut) {
           q->cut();
    }
    else if (e == QKeySequence::Paste) {
           q->paste();
    }    
#endif
    else if (e == QKeySequence::Delete) {
        cursor.deleteChar();
    }
    else if (e == QKeySequence::DeleteEndOfWord) {
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        cursor.deleteChar();
    }
    else if (e == QKeySequence::DeleteStartOfWord) {
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        cursor.deleteChar();
    }
    else if (e == QKeySequence::DeleteEndOfLine) {
        QTextBlock block = cursor.block();
        if (cursor.position() == block.position() + block.length() - 2)
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        else
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.deleteChar();
    }
    else {
        goto process;
    }
    goto accept;
    
process:
    {
        QString text = e->text();
        if (!text.isEmpty() && (text.at(0).isPrint() || text.at(0) == QLatin1Char('\t'))) {
            if (overwriteMode
                // no need to call deleteChar() if we have a selection, insertText
                // does it already
                && !cursor.hasSelection()
                && !cursor.atBlockEnd())
                cursor.deleteChar();

            cursor.insertText(text);
            selectionChanged();
        } else {
            e->ignore();
            return;
        }
    }

 accept:
 
    e->accept();
    cursorOn = true;

    q->ensureCursorVisible();

    updateCurrentCharFormat();
}

QVariant QTextControl::loadResource(int type, const QUrl &name)
{
    if (QTextEdit *textEdit = qobject_cast<QTextEdit *>(parent()))
        return textEdit->loadResource(type, name);
    return QVariant();
}

QRectF QTextControlPrivate::rectForPosition(int position) const
{
    const QTextBlock block = doc->findBlock(position);
    if (!block.isValid())
        return QRectF();
    const QAbstractTextDocumentLayout *docLayout = doc->documentLayout();
    const QTextLayout *layout = block.layout();
    const QPointF layoutPos = docLayout->blockBoundingRect(block).topLeft();
    int relativePos = position - block.position();
    if (preeditCursor != 0) {
        int preeditPos = layout->preeditAreaPosition();
        if (relativePos == preeditPos)
            relativePos += preeditCursor;
        else if (relativePos > preeditPos)
            relativePos += layout->preeditAreaText().length();
    }
    QTextLine line = layout->lineForTextPosition(relativePos);

    int cursorWidth;
    {
        bool ok = false;
#ifndef QT_NO_PROPERTIES
        cursorWidth = docLayout->property("cursorWidth").toInt(&ok);
#endif
        if (!ok)
            cursorWidth = 1;
    }

    QRectF r;

    if (line.isValid())
        r = QRectF(layoutPos.x() + line.cursorToX(relativePos) - 5 - cursorWidth, layoutPos.y() + line.y(),
                  2 * cursorWidth + 10, line.ascent() + line.descent()+1.);
    else
        r = QRectF(layoutPos.x() - 5 - cursorWidth, layoutPos.y(), 2 * cursorWidth + 10, 10); // #### correct height

    return r;
}

QRectF QTextControlPrivate::selectionRect(const QTextCursor &cursor) const
{
    QRectF r = rectForPosition(cursor.selectionStart());

    if (cursor.hasComplexSelection() && cursor.currentTable()) {
        QTextTable *table = cursor.currentTable();

        r = doc->documentLayout()->frameBoundingRect(table);
        /*
        int firstRow, numRows, firstColumn, numColumns;
        cursor.selectedTableCells(&firstRow, &numRows, &firstColumn, &numColumns);

        const QTextTableCell firstCell = table->cellAt(firstRow, firstColumn);
        const QTextTableCell lastCell = table->cellAt(firstRow + numRows - 1, firstColumn + numColumns - 1);

        const QAbstractTextDocumentLayout * const layout = doc->documentLayout();

        QRectF tableSelRect = layout->blockBoundingRect(firstCell.firstCursorPosition().block());

        for (int col = firstColumn; col < firstColumn + numColumns; ++col) {
            const QTextTableCell cell = table->cellAt(firstRow, col);
            const qreal y = layout->blockBoundingRect(cell.firstCursorPosition().block()).top();

            tableSelRect.setTop(qMin(tableSelRect.top(), y));
        }

        for (int row = firstRow; row < firstRow + numRows; ++row) {
            const QTextTableCell cell = table->cellAt(row, firstColumn);
            const qreal x = layout->blockBoundingRect(cell.firstCursorPosition().block()).left();

            tableSelRect.setLeft(qMin(tableSelRect.left(), x));
        }

        for (int col = firstColumn; col < firstColumn + numColumns; ++col) {
            const QTextTableCell cell = table->cellAt(firstRow + numRows - 1, col);
            const qreal y = layout->blockBoundingRect(cell.lastCursorPosition().block()).bottom();

            tableSelRect.setBottom(qMax(tableSelRect.bottom(), y));
        }

        for (int row = firstRow; row < firstRow + numRows; ++row) {
            const QTextTableCell cell = table->cellAt(row, firstColumn + numColumns - 1);
            const qreal x = layout->blockBoundingRect(cell.lastCursorPosition().block()).right();

            tableSelRect.setRight(qMax(tableSelRect.right(), x));
        }

        r = tableSelRect.toRect();
        */
    } else if (cursor.hasSelection()) {
        const int position = cursor.selectionStart();
        const int anchor = cursor.selectionEnd();
        const QTextBlock posBlock = doc->findBlock(position);
        const QTextBlock anchorBlock = doc->findBlock(anchor);
        if (posBlock == anchorBlock && posBlock.layout()->lineCount()) {
            const QTextLine posLine = posBlock.layout()->lineForTextPosition(position - posBlock.position());
            const QTextLine anchorLine = anchorBlock.layout()->lineForTextPosition(anchor - anchorBlock.position());

            const int firstLine = qMin(posLine.lineNumber(), anchorLine.lineNumber());
            const int lastLine = qMax(posLine.lineNumber(), anchorLine.lineNumber());
            r = QRectF();
            for (int i = firstLine; i <= lastLine; ++i) {
                r |= posBlock.layout()->lineAt(i).rect().toRect();
            }
            r.translate(doc->documentLayout()->blockBoundingRect(posBlock).topLeft());
        } else {
            QRectF anchorRect = rectForPosition(cursor.selectionEnd());
            r |= anchorRect;
            QRectF frameRect(doc->documentLayout()->frameBoundingRect(cursor.currentFrame()));
            r.setLeft(frameRect.left());
            r.setRight(frameRect.right());
        }
        if (r.isValid())
            r.adjust(-1, -1, 1, 1);
    }

    return r;
}

void QTextControlPrivate::mousePressEvent(Qt::MouseButton button, const QPointF &pos, Qt::KeyboardModifiers modifiers,
                                          Qt::MouseButtons buttons, const QPoint &globalPos)
{
    Q_Q(QTextControl);
    cursorIsFocusIndicator = false;

    if (interactionFlags & Qt::LinksAccessibleByMouse) {
        anchorOnMousePress = q->anchorAt(pos);
    }
    if (!(button & Qt::LeftButton))
        return;

    if (!((interactionFlags & Qt::TextSelectableByMouse) || (interactionFlags & Qt::TextEditable)))
        return;

    const QTextCursor oldSelection = cursor;
    const int oldCursorPos = cursor.position();

    mousePressed = true;
#ifndef QT_NO_DRAGANDDROP
    mightStartDrag = false;
#endif

    if (trippleClickTimer.isActive()
        && ((pos - trippleClickPoint).toPoint().manhattanLength() < QApplication::startDragDistance())) {

#if defined(Q_WS_MAC)
        cursor.select(QTextCursor::LineUnderCursor);
        selectedLineOnDoubleClick = cursor;
        selectedWordOnDoubleClick = QTextCursor();
#else
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
#endif

        trippleClickTimer.stop();
    } else {
        int cursorPos = doc->documentLayout()->hitTest(pos, Qt::FuzzyHit);
        if (cursorPos == -1)
            return;

#if !defined(QT_NO_IM)
        QTextLayout *layout = cursor.block().layout();
        if (contextWidget && layout && !layout->preeditAreaText().isEmpty()) {
            QInputContext *ctx = contextWidget->inputContext();
            if (!ctx && contextWidget->parentWidget())
                ctx = contextWidget->parentWidget()->inputContext();
            if (ctx) {
                QMouseEvent ev(QEvent::MouseButtonPress, contextWidget->mapFromGlobal(globalPos), globalPos,
                               button, buttons, modifiers);
                ctx->mouseHandler(cursorPos - cursor.position(), &ev);
            }
            if (!layout->preeditAreaText().isEmpty())
                return;
        }
#endif
        if (modifiers == Qt::ShiftModifier) {
            if (selectedWordOnDoubleClick.hasSelection())
                extendWordwiseSelection(cursorPos, pos.x());
            else if (selectedLineOnDoubleClick.hasSelection())
                extendLinewiseSelection(cursorPos);
            else
                setCursorPosition(cursorPos, QTextCursor::KeepAnchor);
        } else {

            if (cursor.hasSelection()
                && cursorPos >= cursor.selectionStart()
                && cursorPos <= cursor.selectionEnd()
                && doc->documentLayout()->hitTest(pos, Qt::ExactHit) != -1) {
#ifndef QT_NO_DRAGANDDROP
                mightStartDrag = true;
                dragStartPos = pos.toPoint();
                dragStartTimer.start(QApplication::startDragTime(), q);
#endif
                return;
            }

            setCursorPosition(cursorPos);
        }
    }

    if (interactionFlags & Qt::TextEditable) {
        q->ensureCursorVisible();
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
        _q_updateCurrentCharFormatAndSelection();
    } else {
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
        selectionChanged();
    }
    repaintOldAndNewSelection(oldSelection);
    hadSelectionOnMousePress = cursor.hasSelection();
}

void QTextControlPrivate::mouseMoveEvent(Qt::MouseButtons buttons, const QPointF &mousePos)
{
    Q_Q(QTextControl);

    if (interactionFlags & Qt::LinksAccessibleByMouse) {
        QString anchor = q->anchorAt(mousePos);
        if (anchor != highlightedAnchor) {
            highlightedAnchor = anchor;
            emit q->linkHovered(anchor);
        }
    }

    if (!(buttons & Qt::LeftButton))
        return;

    if (!((interactionFlags & Qt::TextSelectableByMouse) || (interactionFlags & Qt::TextEditable)))
        return;

    if (!(mousePressed
          || selectedWordOnDoubleClick.hasSelection()
          || selectedLineOnDoubleClick.hasSelection()))
        return;

    const QTextCursor oldSelection = cursor;
    const int oldCursorPos = cursor.position();

    if (mightStartDrag) {
        dragStartTimer.stop();

        if ((mousePos.toPoint() - dragStartPos).manhattanLength() > QApplication::startDragDistance())
            startDrag();

        return;
    }
    const qreal mouseX = qreal(mousePos.x());

#if !defined(QT_NO_IM)
    QTextLayout *layout = cursor.block().layout();
    if (layout && !layout->preeditAreaText().isEmpty())
        return;
#endif

    int newCursorPos = doc->documentLayout()->hitTest(mousePos, Qt::FuzzyHit);
    if (newCursorPos == -1)
        return;

    if (selectedWordOnDoubleClick.hasSelection())
        extendWordwiseSelection(newCursorPos, mouseX);
    else if (selectedLineOnDoubleClick.hasSelection())
        extendLinewiseSelection(newCursorPos);
    else
        setCursorPosition(newCursorPos, QTextCursor::KeepAnchor);

    if (interactionFlags & Qt::TextEditable) {
        q->ensureCursorVisible();
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
        _q_updateCurrentCharFormatAndSelection();
    } else {
        emit q->visibilityRequest(QRectF(mousePos, QSizeF(1, 1)));
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
        selectionChanged();
    }
    repaintOldAndNewSelection(oldSelection);
}

void QTextControlPrivate::mouseReleaseEvent(Qt::MouseButton button, const QPointF &pos)
{
    Q_Q(QTextControl);

    const QTextCursor oldSelection = cursor;

#ifndef QT_NO_DRAGANDDROP
    if (mightStartDrag) {
        mousePressed = false;
        setCursorPosition(pos);
        cursor.clearSelection();
        selectionChanged();
    }
#endif
    if (mousePressed) {
        mousePressed = false;
#ifndef QT_NO_CLIPBOARD
        if (interactionFlags & Qt::TextSelectableByMouse)
            setClipboardSelection();
    } else if (button == Qt::MidButton
               && (interactionFlags & Qt::TextEditable)
               && QApplication::clipboard()->supportsSelection()) {
        setCursorPosition(pos);
        const QMimeData *md = QApplication::clipboard()->mimeData(QClipboard::Selection);
        if (md)
            q->insertFromMimeData(md);
#endif
    }

    repaintOldAndNewSelection(oldSelection);
#ifndef QT_NO_DRAGANDDROP
    if (dragStartTimer.isActive())
        dragStartTimer.stop();
#endif

    if (interactionFlags & Qt::LinksAccessibleByMouse) {
        if (!(button & Qt::LeftButton))
            return;

        const QString anchor = q->anchorAt(pos);

        if (anchor.isEmpty())
            return;

        if (!cursor.hasSelection()
            || (anchor == anchorOnMousePress && hadSelectionOnMousePress))
            if (openExternalLinks)
                QDesktopServices::openUrl(anchor);
            else
                emit q->linkActivated(anchor);
    }
}

void QTextControlPrivate::mouseDoubleClickEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos)
{
    Q_Q(QTextControl);
    if (button != Qt::LeftButton
        || !(interactionFlags & Qt::TextSelectableByMouse)) {
        e->ignore();
        return;
    }
#if !defined(QT_NO_IM)
    QTextLayout *layout = cursor.block().layout();
    if (layout && !layout->preeditAreaText().isEmpty())
        return;
#endif

#ifndef QT_NO_DRAGANDDROP
    mightStartDrag = false;
#endif
    const QTextCursor oldSelection = cursor;
    setCursorPosition(pos);
    QTextLine line = currentTextLine(cursor);
    if (line.isValid() && line.textLength()) {
        cursor.select(QTextCursor::WordUnderCursor);
        selectionChanged();
#ifndef QT_NO_CLIPBOARD
        setClipboardSelection();
#endif
    }
    repaintOldAndNewSelection(oldSelection);

    selectedWordOnDoubleClick = cursor;

    trippleClickPoint = pos;
    trippleClickTimer.start(qApp->doubleClickInterval(), q);
}

void QTextControlPrivate::contextMenuEvent(const QPoint &screenPos, const QPointF &docPos)
{
#ifdef QT_NO_CONTEXTMENU
    Q_UNUSED(screenPos);
    Q_UNUSED(docPos);
#else
    Q_Q(QTextControl);
    if (!hasFocus)
        return;
    QMenu *menu = q->createStandardContextMenu(docPos);
    if (!menu)
        return;
    menu->exec(screenPos);
    delete menu;
#endif
}

bool QTextControlPrivate::dragEnterEvent(QEvent *e, const QMimeData *mimeData)
{
    Q_Q(QTextControl);
    if (!(interactionFlags & Qt::TextEditable) || !q->canInsertFromMimeData(mimeData)) {
        e->ignore();
        return false;
    }

    dndFeedbackCursor = QTextCursor();

    return true; // accept proposed action
}

void QTextControlPrivate::dragLeaveEvent()
{
    Q_Q(QTextControl);

    const QRectF crect = q->cursorRect(dndFeedbackCursor);
    dndFeedbackCursor = QTextCursor();

    if (crect.isValid())
        emit q->updateRequest(crect);
}

bool QTextControlPrivate::dragMoveEvent(QEvent *e, const QMimeData *mimeData, const QPointF &pos)
{
    Q_Q(QTextControl);
    if (!(interactionFlags & Qt::TextEditable) || !q->canInsertFromMimeData(mimeData)) {
        e->ignore();
        return false;
    }

    const int cursorPos = doc->documentLayout()->hitTest(pos, Qt::FuzzyHit);
    if (cursorPos != -1) {
        QRectF crect = q->cursorRect(dndFeedbackCursor);
        if (crect.isValid())
            emit q->updateRequest(crect);

        dndFeedbackCursor = cursor;
        dndFeedbackCursor.setPosition(cursorPos);

        crect = q->cursorRect(dndFeedbackCursor);
        emit q->updateRequest(crect);
    }

    return true; // accept proposed action
}

bool QTextControlPrivate::dropEvent(const QMimeData *mimeData, const QPointF &pos, Qt::DropAction dropAction, QWidget *source)
{
    Q_Q(QTextControl);
    dndFeedbackCursor = QTextCursor();

    if (!(interactionFlags & Qt::TextEditable) || !q->canInsertFromMimeData(mimeData))
        return false;

    repaintSelection();

    QTextCursor insertionCursor = q->cursorForPosition(pos);
    insertionCursor.beginEditBlock();

    if (dropAction == Qt::MoveAction && source == contextWidget)
        cursor.removeSelectedText();

    cursor = insertionCursor;
    q->insertFromMimeData(mimeData);
    insertionCursor.endEditBlock();
    q->ensureCursorVisible();
    return true; // accept proposed action
}

void QTextControlPrivate::inputMethodEvent(QInputMethodEvent *e)
{
    if (!(interactionFlags & Qt::TextEditable) || cursor.isNull()) {
        e->ignore();
        return;
    }
    cursor.beginEditBlock();

    cursor.removeSelectedText();

    // insert commit string
    if (!e->commitString().isEmpty() || e->replacementLength()) {
        QTextCursor c = cursor;
        c.setPosition(c.position() + e->replacementStart());
        c.setPosition(c.position() + e->replacementLength(), QTextCursor::KeepAnchor);
        c.insertText(e->commitString());
    }

    QTextBlock block = cursor.block();
    QTextLayout *layout = block.layout();
    layout->setPreeditArea(cursor.position() - block.position(), e->preeditString());
    QList<QTextLayout::FormatRange> overrides;
    preeditCursor = e->preeditString().length();
    hideCursor = false;
    for (int i = 0; i < e->attributes().size(); ++i) {
        const QInputMethodEvent::Attribute &a = e->attributes().at(i);
        if (a.type == QInputMethodEvent::Cursor) {
            preeditCursor = a.start;
            hideCursor = !a.length;
        } else if (a.type == QInputMethodEvent::TextFormat) {
            QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
            if (f.isValid()) {
                QTextLayout::FormatRange o;
                o.start = a.start + cursor.position() - block.position();
                o.length = a.length;
                o.format = f;
                overrides.append(o);
            }
        }
    }
    layout->setAdditionalFormats(overrides);
    cursor.endEditBlock();
}

QVariant QTextControl::inputMethodQuery(Qt::InputMethodQuery property) const
{
    Q_D(const QTextControl);
    QTextBlock block = d->cursor.block();
    switch(property) {
    case Qt::ImMicroFocus:
        return cursorRect();
    case Qt::ImFont:
        return QVariant(d->cursor.charFormat().font());
    case Qt::ImCursorPosition:
        return QVariant(d->cursor.position() - block.position());
    case Qt::ImSurroundingText:
        return QVariant(block.text());
    case Qt::ImCurrentSelection:
        return QVariant(d->cursor.selectedText());
    default:
        return QVariant();
    }
}

void QTextControl::setFocus(bool focus, Qt::FocusReason reason)
{
    QFocusEvent ev(focus ? QEvent::FocusIn : QEvent::FocusOut,
                   reason);
    processEvent(&ev);
}

void QTextControlPrivate::focusEvent(QFocusEvent *e)
{
    if (e->gotFocus()) {
        cursorOn = (interactionFlags & Qt::TextSelectableByKeyboard);
        if (interactionFlags & Qt::TextEditable) {
            setBlinkingCursorEnabled(true);
#ifdef QT_KEYPAD_NAVIGATION
            if (QApplication::keypadNavigationEnabled()) {
                if (e->reason() == Qt::TabFocusReason) {
                    cursor.movePosition(QTextCursor::Start);
                } else if (e->reason() == Qt::BacktabFocusReason) {
                    cursor.movePosition(QTextCursor::End);
                    cursor.movePosition(QTextCursor::StartOfLine);
                }
            }
#endif
        }
    } else {
        setBlinkingCursorEnabled(false);

        if (interactionFlags & Qt::LinksAccessibleByKeyboard
            && e->reason() != Qt::ActiveWindowFocusReason
            && e->reason() != Qt::PopupFocusReason
            && cursor.hasSelection()) {
            emit q_func()->updateRequest(selectionRect());
            cursor.clearSelection();
        }
    }
    hasFocus = e->gotFocus();
}

#ifndef QT_NO_CONTEXTMENU
QMenu *QTextControl::createStandardContextMenu(const QPointF &pos)
{
    Q_D(QTextControl);

    const bool showTextSelectionActions = d->interactionFlags & (Qt::TextEditable | Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);

    d->linkToCopy = QString();
    if (!pos.isNull())
        d->linkToCopy = anchorAt(pos);

    if (d->linkToCopy.isEmpty() && !showTextSelectionActions)
        return 0;

    QMenu *menu = new QMenu;
    QAction *a;

    if (d->interactionFlags & Qt::TextEditable) {
        a = menu->addAction(tr("&Undo") + ACCEL_KEY(Z), this, SLOT(undo()));
        a->setEnabled(d->doc->isUndoAvailable());
        a = menu->addAction(tr("&Redo") + ACCEL_KEY(Y), this, SLOT(redo()));
        a->setEnabled(d->doc->isRedoAvailable());
        menu->addSeparator();

        a = menu->addAction(tr("Cu&t") + ACCEL_KEY(X), this, SLOT(cut()));
        a->setEnabled(d->cursor.hasSelection());
    }

    if (showTextSelectionActions) {
        a = menu->addAction(tr("&Copy") + ACCEL_KEY(C), this, SLOT(copy()));
        a->setEnabled(d->cursor.hasSelection());
    }

    if ((d->interactionFlags & Qt::LinksAccessibleByKeyboard)
            || (d->interactionFlags & Qt::LinksAccessibleByMouse)) {

        a = menu->addAction(tr("Copy &Link Location"), this, SLOT(_q_copyLink()));
        a->setEnabled(!d->linkToCopy.isEmpty());
    }

    if (d->interactionFlags & Qt::TextEditable) {
#if !defined(QT_NO_CLIPBOARD)
        a = menu->addAction(tr("&Paste") + ACCEL_KEY(V), this, SLOT(paste()));
        const QMimeData *md = QApplication::clipboard()->mimeData();
        a->setEnabled(md && canInsertFromMimeData(md));
#endif
        a = menu->addAction(tr("Delete"), this, SLOT(_q_deleteSelected()));
        a->setEnabled(d->cursor.hasSelection());
    }


    if (showTextSelectionActions) {
        menu->addSeparator();
        a = menu->addAction(tr("Select All") + ACCEL_KEY(A), this, SLOT(selectAll()));
        a->setEnabled(!d->doc->isEmpty());
    }

    if (d->interactionFlags & Qt::TextEditable) {
        menu->addSeparator();
        QUnicodeControlCharacterMenu *ctrlCharacterMenu = new QUnicodeControlCharacterMenu(this, menu);
        menu->addMenu(ctrlCharacterMenu);
    }

    return menu;
}
#endif // QT_NO_CONTEXTMENU

QTextCursor QTextControl::cursorForPosition(const QPointF &pos) const
{
    Q_D(const QTextControl);
    int cursorPos = d->doc->documentLayout()->hitTest(pos, Qt::FuzzyHit);
    if (cursorPos == -1)
        cursorPos = 0;
    QTextCursor c(d->doc);
    c.setPosition(cursorPos);
    return c;
}

QRectF QTextControl::cursorRect(const QTextCursor &cursor) const
{
    Q_D(const QTextControl);
    if (cursor.isNull())
        return QRectF();

   return d->rectForPosition(cursor.position());
}

QRectF QTextControl::cursorRect() const
{
    Q_D(const QTextControl);
    return cursorRect(d->cursor);
}

QString QTextControl::anchorAt(const QPointF &pos) const
{
    Q_D(const QTextControl);
    return d->doc->documentLayout()->anchorAt(pos);
}

bool QTextControl::overwriteMode() const
{
    Q_D(const QTextControl);
    return d->overwriteMode;
}

void QTextControl::setOverwriteMode(bool overwrite)
{
    Q_D(QTextControl);
    d->overwriteMode = overwrite;
}

int QTextControl::tabStopWidth() const
{
    Q_D(const QTextControl);
    return qRound(d->doc->documentLayout()->property("tabStopWidth").toDouble());
}

void QTextControl::setTabStopWidth(int width)
{
    Q_D(QTextControl);
    d->doc->documentLayout()->setProperty("tabStopWidth", QVariant(double(width)));
}

int QTextControl::cursorWidth() const
{
    Q_D(const QTextControl);
    return d->doc->documentLayout()->property("cursorWidth").toInt();
}

void QTextControl::setCursorWidth(int width)
{
    Q_D(QTextControl);
    d->doc->documentLayout()->setProperty("cursorWidth", width);
    d->repaintCursor();
}

bool QTextControl::acceptRichText() const
{
    Q_D(const QTextControl);
    return d->acceptRichText;
}

void QTextControl::setAcceptRichText(bool accept)
{
    Q_D(QTextControl);
    d->acceptRichText = accept;
}

void QTextControl::setExtraSelections(const QList<QTextEdit::ExtraSelection> &selections)
{
    Q_D(QTextControl);
    if (selections.count() == d->extraSelections.count()) {
        bool needUpdate = false;
        for (int i = 0; i < selections.count(); ++i)
            if (selections.at(i).cursor != d->extraSelections.at(i).cursor
                || selections.at(i).format != d->extraSelections.at(i).format) {
                needUpdate = true;
                break;
            }
        if (!needUpdate)
            return;
    }

    for (int i = 0; i < d->extraSelections.count(); ++i) {
        QRectF r = d->selectionRect(d->extraSelections.at(i).cursor);
        if (d->extraSelections.at(i).format.boolProperty(QTextFormat::FullWidthSelection)) {
            r.setLeft(0);
            r.setWidth(INT_MAX);
        }
        emit updateRequest(r);
    }

    d->extraSelections.resize(selections.count());
    for (int i = 0; i < selections.count(); ++i) {
        d->extraSelections[i].cursor = selections.at(i).cursor;
        d->extraSelections[i].format = selections.at(i).format;
    }

    for (int i = 0; i < d->extraSelections.count(); ++i) {
        QRectF r = d->selectionRect(d->extraSelections.at(i).cursor);
        if (d->extraSelections.at(i).format.boolProperty(QTextFormat::FullWidthSelection)) {
            r.setLeft(0);
            r.setWidth(INT_MAX);
        }
        emit updateRequest(r);
    }
}

QList<QTextEdit::ExtraSelection> QTextControl::extraSelections() const
{
    Q_D(const QTextControl);
    QList<QTextEdit::ExtraSelection> selections;
    for (int i = 0; i < d->extraSelections.count(); ++i) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = d->extraSelections.at(i).cursor;
        sel.format = d->extraSelections.at(i).format;
        selections.append(sel);
    }
    return selections;
}

void QTextControl::setTextWidth(qreal width)
{
    Q_D(QTextControl);
    d->doc->setTextWidth(width);
}

qreal QTextControl::textWidth() const
{
    Q_D(const QTextControl);
    return d->doc->textWidth();
}

QSizeF QTextControl::size() const
{
    Q_D(const QTextControl);
    return d->doc->size();
}

void QTextControl::setOpenExternalLinks(bool open)
{
    Q_D(QTextControl);
    d->openExternalLinks = open;
}

bool QTextControl::openExternalLinks() const
{
    Q_D(const QTextControl);
    return d->openExternalLinks;
}

void QTextControl::moveCursor(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    Q_D(QTextControl);
    const QTextCursor oldSelection = d->cursor;
    const bool moved = d->cursor.movePosition(op, mode);
    d->_q_updateCurrentCharFormatAndSelection();
    ensureCursorVisible();
    d->repaintOldAndNewSelection(oldSelection);
    if (moved)
        emit cursorPositionChanged();
}

QMimeData *QTextControl::createMimeDataFromSelection() const
{
    Q_D(const QTextControl);
    const QTextDocumentFragment fragment(d->cursor);
    return new QTextEditMimeData(fragment);
}

bool QTextControl::canInsertFromMimeData(const QMimeData *source) const
{
    Q_D(const QTextControl);
    if (d->acceptRichText)
        return source->hasText()
            || source->hasHtml()
            || source->hasFormat(QLatin1String("application/x-qrichtext"))
            || source->hasFormat(QLatin1String("application/x-qt-richtext"));
    else
        return source->hasText();
}

void QTextControl::insertFromMimeData(const QMimeData *source)
{
    Q_D(QTextControl);
    if (!(d->interactionFlags & Qt::TextEditable) || !source)
	return;

    bool hasData = false;
    QTextDocumentFragment fragment;
    if (source->hasFormat(QLatin1String("application/x-qrichtext")) && d->acceptRichText) {
        // x-qrichtext is always UTF-8 (taken from Qt3 since we don't use it anymore).
        fragment = QTextDocumentFragment::fromHtml(QString::fromUtf8(source->data(QLatin1String("application/x-qrichtext"))));
        hasData = true;
    } else if (source->hasHtml() && d->acceptRichText) {
        fragment = QTextDocumentFragment::fromHtml(source->html());
        hasData = true;
    } else {
        QString text = source->text();
        if (!text.isNull()) {
            fragment = QTextDocumentFragment::fromPlainText(text);
            hasData = true;
        }
    }

    if (hasData)
        d->cursor.insertFragment(fragment);
    ensureCursorVisible();
}

bool QTextControlPrivate::findNextPrevAnchor(bool next, int &start, int &end)
{
    if (!cursor.hasSelection()) {
        cursor = QTextCursor(doc);
        if (next)
            cursor .movePosition(QTextCursor::Start);
        else
            cursor.movePosition(QTextCursor::End);
    }

    Q_ASSERT(!cursor.isNull());

    int anchorStart = -1;
    int anchorEnd = -1;

    if (next) {
        const int startPos = cursor.selectionEnd();

        QTextBlock block = doc->findBlock(startPos);
        QTextBlock::Iterator it = block.begin();

        while (!it.atEnd() && it.fragment().position() < startPos)
            ++it;

        while (block.isValid()) {
            anchorStart = -1;

            // find next anchor
            for (; !it.atEnd(); ++it) {
                const QTextFragment fragment = it.fragment();
                const QTextCharFormat fmt = fragment.charFormat();

                if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
                    anchorStart = fragment.position();
                    break;
                }
            }

            if (anchorStart != -1) {
                anchorEnd = -1;

                // find next non-anchor fragment
                for (; !it.atEnd(); ++it) {
                    const QTextFragment fragment = it.fragment();
                    const QTextCharFormat fmt = fragment.charFormat();

                    if (!fmt.isAnchor()) {
                        anchorEnd = fragment.position();
                        break;
                    }
                }

                if (anchorEnd == -1)
                    anchorEnd = block.position() + block.length() - 1;

                // make found selection
                break;
            }

            block = block.next();
            it = block.begin();
        }
    } else {
        int startPos = cursor.selectionStart();
        if (startPos > 0)
            --startPos;

        QTextBlock block = doc->findBlock(startPos);
        QTextBlock::Iterator blockStart = block.begin();
        QTextBlock::Iterator it = block.end();

        if (startPos == block.position()) {
            it = block.begin();
        } else {
            do {
                if (it == blockStart) {
                    it = QTextBlock::Iterator();
                    block = QTextBlock();
                } else {
                    --it;
                }
            } while (!it.atEnd() && it.fragment().position() + it.fragment().length() - 1 > startPos);
        }

        while (block.isValid()) {
            anchorStart = -1;

            if (!it.atEnd()) {
                do {
                    const QTextFragment fragment = it.fragment();
                    const QTextCharFormat fmt = fragment.charFormat();

                    if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
                        anchorStart = fragment.position() + fragment.length();
                        break;
                    }

                    if (it == blockStart)
                        it = QTextBlock::Iterator();
                    else
                        --it;
                } while (!it.atEnd());
            }

            if (anchorStart != -1 && !it.atEnd()) {
                anchorEnd = -1;

                do {
                    const QTextFragment fragment = it.fragment();
                    const QTextCharFormat fmt = fragment.charFormat();

                    if (!fmt.isAnchor()) {
                        anchorEnd = fragment.position() + fragment.length();
                        break;
                    }

                    if (it == blockStart)
                        it = QTextBlock::Iterator();
                    else
                        --it;
                } while (!it.atEnd());

                if (anchorEnd == -1)
                    anchorEnd = qMax(0, block.position());

                break;
            }

            block = block.previous();
            it = block.end();
            if (it != block.begin())
                --it;
            blockStart = block.begin();
        }

    }

    if (anchorStart != -1 && anchorEnd != -1) {
        start = anchorStart;
        end = anchorEnd;
        return true;
    }

    return false;
}

bool QTextControl::setFocusToNextOrPreviousAnchor(bool next)
{
    Q_D(QTextControl);

    if (!(d->interactionFlags & Qt::LinksAccessibleByKeyboard))
        return false;

    QRectF crect = d->selectionRect(d->cursor);
    emit updateRequest(crect);

    int anchorStart, anchorEnd;
    if (d->findNextPrevAnchor(next, anchorStart, anchorEnd)) {
        d->cursor.setPosition(anchorStart);
        d->cursor.setPosition(anchorEnd, QTextCursor::KeepAnchor);
        d->cursorIsFocusIndicator = true;
    } else {
        d->cursor.clearSelection();
    }

    if (d->cursor.hasSelection()) {
        crect = d->selectionRect(d->cursor);
        emit updateRequest(crect);
        emit visibilityRequest(crect);
        return true;
    } else {
        return false;
    }
}

void QTextControl::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
    Q_D(QTextControl);
    if (flags == d->interactionFlags)
        return;
    d->interactionFlags = flags;

    if (d->hasFocus)
        d->setBlinkingCursorEnabled(flags & Qt::TextEditable);
}

Qt::TextInteractionFlags QTextControl::textInteractionFlags() const
{
    Q_D(const QTextControl);
    return d->interactionFlags;
}

void QTextControl::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
    Q_D(QTextControl);

    if (!d->cursor.hasSelection()) {
        if (d->cursor.atBlockStart() && d->cursor.atBlockEnd()) {
            d->cursor.mergeBlockCharFormat(modifier);
        } else {
            QTextCursor word = d->cursor;
            word.select(QTextCursor::WordUnderCursor);
            word.mergeCharFormat(modifier);
        }
    }
    d->cursor.mergeCharFormat(modifier);
    d->lastCharFormat = d->cursor.charFormat();
}

void QTextControl::setCurrentCharFormat(const QTextCharFormat &format)
{
    Q_D(QTextControl);
    d->cursor.setCharFormat(format);
    d->lastCharFormat = format;
}

QTextCharFormat QTextControl::currentCharFormat() const
{
    Q_D(const QTextControl);
    return d->cursor.charFormat();
}

void QTextControl::insertPlainText(const QString &text)
{
    Q_D(QTextControl);
    d->cursor.insertText(text);
}

void QTextControl::insertHtml(const QString &text)
{
    Q_D(QTextControl);
    d->cursor.insertHtml(text);
}

QPointF QTextControl::anchorPosition(const QString &name) const
{
    Q_D(const QTextControl);
    if (name.isEmpty())
        return QPointF();

    QRectF r;
    for (QTextBlock block = d->doc->begin(); block.isValid(); block = block.next()) {
        QTextCharFormat format = block.charFormat();
        if (format.isAnchor() && format.anchorName() == name) {
            r = d->rectForPosition(block.position());
            break;
        }

        for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            format = fragment.charFormat();
            if (format.isAnchor() && format.anchorName() == name) {
                r = d->rectForPosition(fragment.position());
                block = QTextBlock();
                break;
            }
        }
    }
    if (!r.isValid())
        return QPointF();
    return QPointF(0, r.top());
}

void QTextControl::adjustSize()
{
    Q_D(QTextControl);
    d->doc->adjustSize();
}

QTextOption::WrapMode QTextControl::wordWrapMode() const
{
    Q_D(const QTextControl);
    if (QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(d->doc->documentLayout()))
        return layout->wordWrapMode();
    return QTextOption::WordWrap;
}

void QTextControl::setWordWrapMode(QTextOption::WrapMode mode)
{
    Q_D(QTextControl);
    if (QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(d->doc->documentLayout()))
        layout->setWordWrapMode(mode);
}

bool QTextControl::find(const QString &exp, QTextDocument::FindFlags options)
{
    Q_D(QTextControl);
    QTextCursor search = d->doc->find(exp, d->cursor, options);
    if (search.isNull())
        return false;

    setTextCursor(search);
    return true;
}

void QTextControl::append(const QString &text)
{
    Q_D(QTextControl);
    QTextCursor cursor(d->doc);
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);

    if (!d->doc->isEmpty())
        cursor.insertBlock(d->cursor.blockFormat(), d->cursor.charFormat());

    // preserve the char format
    QTextCharFormat oldCharFormat = d->cursor.charFormat();
    if (Qt::mightBeRichText(text)) {
        cursor.insertHtml(text);
    } else {
        cursor.insertText(text);
    }
    if (!d->cursor.hasSelection())
        d->cursor.setCharFormat(oldCharFormat);

    cursor.endEditBlock();
}

void QTextControl::ensureCursorVisible()
{
    Q_D(QTextControl);
    QRectF crect = d->rectForPosition(d->cursor.position());
    emit visibilityRequest(crect);
    emit microFocusChanged();
}

QPalette QTextControl::palette() const
{
    Q_D(const QTextControl);
    return d->palette;
}

void QTextControl::setPalette(const QPalette &pal)
{
    Q_D(QTextControl);
    d->palette = pal;
}

void QTextControl::drawContents(QPainter *p, const QRectF &rect)
{
    Q_D(QTextControl);
    p->save();
    QAbstractTextDocumentLayout::PaintContext ctx;
    if (rect.isValid())
        p->setClipRect(rect);
    ctx.clip = rect;
    ctx.selections = d->extraSelections;

    for (int i = 0; i < ctx.selections.count(); ++i)
        if (ctx.selections.at(i).format.boolProperty(QTextFormat::FullWidthSelection)) {
            ctx.clip.setLeft(0);
            ctx.clip.setWidth(INT_MAX);
        }

    ctx.palette = d->palette;
    if (d->cursorOn && d->isEnabled) {
        if (d->hideCursor)
            ctx.cursorPosition = -1;
        else if (d->preeditCursor != 0)
            ctx.cursorPosition = - (d->preeditCursor + 2);
        else
            ctx.cursorPosition = d->cursor.position();
    }

    if (!d->dndFeedbackCursor.isNull())
        ctx.cursorPosition = d->dndFeedbackCursor.position();
    if (d->cursor.hasSelection()) {
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = d->cursor;
        if (d->cursorIsFocusIndicator) {
            QPen outline(ctx.palette.color(QPalette::Text), 1, Qt::DotLine);
            selection.format.setProperty(QTextFormat::OutlinePen, outline);
        } else {
            selection.format.setBackground(ctx.palette.brush(QPalette::Highlight));
            selection.format.setForeground(ctx.palette.brush(QPalette::HighlightedText));
        }
        ctx.selections.append(selection);
    }

    d->doc->documentLayout()->draw(p, ctx);
    p->restore();
}

void QTextControlPrivate::_q_copyLink()
{
    QMimeData *md = new QMimeData;
    md->setText(linkToCopy);
    QApplication::clipboard()->setMimeData(md);
}

#endif // QT_NO_TEXTCONTROL

#include "moc_qtextcontrol_p.cpp"
