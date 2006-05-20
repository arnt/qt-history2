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

#include <qtextformat.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <limits.h>
#include <qtexttable.h>
#include <qvariant.h>
#include <qurl.h>

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

bool QTextControlPrivate::cursorMoveKeyEvent(QKeyEvent *e)
{
    Q_Q(QTextControl);
    if (cursor.isNull())
        return false;

    const int oldCursorPos = cursor.position();
    QTextCursor::MoveMode mode = e->modifiers() & Qt::ShiftModifier
                                   ? QTextCursor::KeepAnchor
                                   : QTextCursor::MoveAnchor;

    QTextCursor::MoveOperation op = QTextCursor::NoMove;
#ifdef Q_WS_MAC
    // There can be only one modifier (+ shift), but we also need to make sure
    // that we have a "move key" pressed before we reject it.
    bool twoModifiers
        = ((e->modifiers() & (Qt::ControlModifier | Qt::AltModifier))
           == (Qt::ControlModifier | Qt::AltModifier))
        || ((e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
            == (Qt::ControlModifier | Qt::MetaModifier))
        || ((e->modifiers() & (Qt::AltModifier | Qt::MetaModifier))
            == (Qt::AltModifier | Qt::MetaModifier));
#else
    if (e->modifiers() & (Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier)) {
        e->ignore();
        return false;
    }
#endif
    switch (e->key()) {
#ifndef Q_WS_MAC  // Use the default Windows bindings.
        case Qt::Key_Up:
            op = QTextCursor::Up;
            break;
        case Qt::Key_Down:
            op = QTextCursor::Down;
            if (mode == QTextCursor::KeepAnchor) {
                QTextBlock block = cursor.block();
                QTextLine line = currentTextLine(cursor);
                if (!block.next().isValid()
                    && line.isValid()
                    && line.lineNumber() == block.layout()->lineCount() - 1)
                    op = QTextCursor::End;
            }
            break;
        case Qt::Key_Left:
            op = e->modifiers() & Qt::ControlModifier
                 ? QTextCursor::WordLeft
                 : QTextCursor::Left;
            break;
        case Qt::Key_Right:
            op = e->modifiers() & Qt::ControlModifier
                 ? QTextCursor::WordRight
                 : QTextCursor::Right;
            break;
        case Qt::Key_Home:
            op = e->modifiers() & Qt::ControlModifier
                 ? QTextCursor::Start
                 : QTextCursor::StartOfLine;
            break;
        case Qt::Key_End:
            op = e->modifiers() & Qt::ControlModifier
                 ? QTextCursor::End
                 : QTextCursor::EndOfLine;
            break;
#else
// Except for pageup and pagedown, Mac OS X has very different behavior, we don't do it all, but
// here's the breakdown:
// Shift still works as an anchor, but only one of the other keys can be down Ctrl (Command),
// Alt (Option), or Meta (Control).
// Command/Control + Left/Right -- Move to left or right of the line
//                 + Up/Down -- Move to top bottom of the file. (Control doesn't move the cursor)
// Option + Left/Right -- Move one word Left/right.
//        + Up/Down  -- Begin/End of Paragraph.
// Home/End Top/Bottom of file. (usually don't move the cursor, but will select)
        case Qt::Key_Up:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                    op = QTextCursor::Start;
                else if (e->modifiers() & Qt::AltModifier)
                    op = QTextCursor::StartOfBlock;
                else
                    op = QTextCursor::Up;
            }
            break;
        case Qt::Key_Down:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) {
                    op = QTextCursor::End;
                } else if (e->modifiers() & Qt::AltModifier) {
                    op = QTextCursor::EndOfBlock;
                } else {
                    op = QTextCursor::Down;
                    if (mode == QTextCursor::KeepAnchor) {
                        QTextBlock block = cursor.block();
                        QTextLine line = currentTextLine(cursor);
                        if (!block.next().isValid()
                            && line.isValid()
                            && line.lineNumber() == block.layout()->lineCount() - 1)
                            op = QTextCursor::End;
                    }
                }
            }
            break;
        case Qt::Key_Left:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                    op = QTextCursor::StartOfLine;
                else if (e->modifiers() & Qt::AltModifier)
                    op = QTextCursor::WordLeft;
                else
                    op = QTextCursor::Left;
            }
            break;
        case Qt::Key_Right:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                    op = QTextCursor::EndOfLine;
                else if (e->modifiers() & Qt::AltModifier)
                    op = QTextCursor::WordRight;
                else
                    op = QTextCursor::Right;
            }
            break;
        case Qt::Key_Home:
            if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier | Qt::AltModifier)) {
                QApplication::beep();
                return true;
            } else {
                op = QTextCursor::Start;
            }
            break;
        case Qt::Key_End:
            if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier | Qt::AltModifier)) {
                QApplication::beep();
                return true;
            } else {
                op = QTextCursor::End;
            }
            break;
#endif
        case Qt::Key_PageDown:
            pageDown(mode);
            break;
        case Qt::Key_PageUp:
            pageUp(mode);
            break;
    default:
        return false;
    }

    const bool moved = cursor.movePosition(op, mode);
    q->ensureCursorVisible();

    if (moved) {
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
// ####        q->updateMicroFocus();
    }
#ifdef QT_KEYPAD_NAVIGATION
    else if (QApplication::keypadNavigationEnabled()
        && (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)) {
        return false;
    }
#endif

    selectionChanged();

    repaintSelection();

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
// ####    q->updateMicroFocus();
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

void QTextControlPrivate::init(const QString &html)
{
    setContent(Qt::RichText, html);

/* ####
#ifndef QT_NO_CURSOR
    viewport->setCursor(Qt::IBeamCursor);
#endif
*/
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

// ###        doc->setDefaultFont(q->font());
// ####        doc->documentLayout()->setPaintDevice(viewport);

        QObject::connect(doc, SIGNAL(contentsChanged()), q, SLOT(updateCurrentCharFormatAndSelection()));
        QObject::connect(doc, SIGNAL(cursorPositionChanged(QTextCursor)), q, SLOT(emitCursorPosChanged(QTextCursor)));

        // convenience signal forwards
        QObject::connect(doc, SIGNAL(contentsChanged()), q, SIGNAL(textChanged()));
        QObject::connect(doc, SIGNAL(undoAvailable(bool)), q, SIGNAL(undoAvailable(bool)));
        QObject::connect(doc, SIGNAL(redoAvailable(bool)), q, SIGNAL(redoAvailable(bool)));
    }

    doc->setUndoRedoEnabled(false);

// ###    q->setAttribute(Qt::WA_InputMethodEnabled);

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
            doc->setPlainText(text);
            QTextCursor formatCursor(doc);
            formatCursor.select(QTextCursor::Document);
            formatCursor.setCharFormat(charFormatForInsertion);
        } else {
            doc->setHtml(text);
        }
        cursor = QTextCursor(doc);
    } else if (clearDocument) {
        doc->clear();
        cursor.movePosition(QTextCursor::Start);
        QTextBlockFormat blockFmt;
        /* ####
        blockFmt.setLayoutDirection(q->layoutDirection());
        cursor.mergeBlockFormat(blockFmt);
        */
        cursor.setCharFormat(charFormatForInsertion);
    }

    QObject::connect(doc, SIGNAL(contentsChanged()), q, SIGNAL(textChanged()));
    emit q->textChanged();
    doc->setUndoRedoEnabled(!q->isReadOnly());
    updateCurrentCharFormatAndSelection();
    doc->setModified(false);
    anchorToScrollToWhenVisible.clear();
    emit q->cursorPositionChanged();
}

/*
void QTextControlPrivate::startDrag()
{
    Q_Q(QTextControl);
    mousePressed = false;
    QMimeData *data = q->createMimeDataFromSelection();

    QDrag *drag = new QDrag(q);
    drag->setMimeData(data);

    Qt::DropActions actions = Qt::CopyAction;
    if (!readOnly)
        actions |= Qt::MoveAction;
    Qt::DropAction action = drag->start(actions);

    if (action == Qt::MoveAction && drag->target() != q)
        cursor.removeSelectedText();
}
*/

void QTextControlPrivate::setCursorPosition(const QPoint &pos)
{
    const int cursorPos = doc->documentLayout()->hitTest(mapToContents(pos), Qt::FuzzyHit);
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

void QTextControlPrivate::selectionChanged()
{
    Q_Q(QTextControl);
    bool current = cursor.hasSelection();
    if (current == lastSelectionState)
        return;

    lastSelectionState = current;
    emit q->copyAvailable(current);
    emit q->selectionChanged();
// ####    q->updateMicroFocus();
}

void QTextControlPrivate::pageUp(QTextCursor::MoveMode moveMode)
{
    /* ##################
    Q_Q(QTextControl);
    const int oldCursorPos = cursor.position();
    const QRectF vp = q->viewport();
    int targetY = int(vp.y() - vp.height());
    bool moved = false;
    qreal y;
    // move to the targetY using movePosition to keep the cursor's x
    do {
        const QRectF r = q->cursorRect();
        y = vp.y() + r.y() - r.height();
        moved = cursor.movePosition(QTextCursor::Up, moveMode);
    } while (moved && y > targetY);

    if (moved) {
        q->ensureCursorVisible();
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
// #####        q->updateMicroFocus();
    }
    */
}

void QTextControlPrivate::pageDown(QTextCursor::MoveMode moveMode)
{
    /* #################
    Q_Q(QTextControl);
    const int oldCursorPos = cursor.position();
    const QRectF vp = q->viewport();
    int targetY = int(vp.y() + 2 * vp.height());
    bool moved = false;
    qreal y;
    // move to the targetY using movePosition to keep the cursor's x
    do {
        y = vp.y() + q->cursorRect().bottom();
        moved = cursor.movePosition(QTextCursor::Down, moveMode);
    } while (moved && y < targetY);

    if (moved) {
        q->ensureCursorVisible();
        if (cursor.position() != oldCursorPos)
            emit q->cursorPositionChanged();
// ######        q->updateMicroFocus();
    }
    */
}

void QTextControlPrivate::updateCurrentCharFormatAndSelection()
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

void QTextControlPrivate::ensureVisible(int documentPosition)
{
    // don't check for the visibility of the vertical scrollbar here,
    // always scroll to the position. we might have a layoutChildren
    // in QAbstractScrollArea pending, which will make things visible later on
    // then, for example when initially showing the widget and right
    // after that calling scrollToAnchor, which calls us. the vbar
    // isn't visible then, but that's okay.

    QTextBlock block = doc->findBlock(documentPosition);
    QTextLayout *layout = block.layout();
    const qreal blockY = doc->documentLayout()->blockBoundingRect(block).top();

    const int relativePos = documentPosition - block.position();
    QTextLine line = layout->lineForTextPosition(relativePos);
    if (!line.isValid())
        return;

    Q_Q(QTextControl);
    emit q->visibilityRequest(QRectF(0, blockY + line.y(), 1, 1));
}

void QTextControlPrivate::emitCursorPosChanged(const QTextCursor &someCursor)
{
    Q_Q(QTextControl);
    if (someCursor.isCopyOf(cursor)) {
        emit q->cursorPositionChanged();
// #####        q->updateMicroFocus();
    }
}

void QTextControlPrivate::setBlinkingCursorEnabled(bool enable)
{
    Q_Q(QTextControl);
    if (enable && QApplication::cursorFlashTime() > 0)
        cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, q);
    else
        cursorBlinkTimer.stop();
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

void QTextControlPrivate::deleteSelected()
{
    if (readOnly || !cursor.hasSelection())
	return;
    cursor.removeSelectedText();
}

void QTextControlPrivate::undo()
{
    Q_Q(QTextControl);
    QObject::connect(doc, SIGNAL(contentsChange(int, int, int)),
                     q, SLOT(setCursorAfterUndoRedo(int, int, int)));
    doc->undo();
    QObject::disconnect(doc, SIGNAL(contentsChange(int, int, int)),
                        q, SLOT(setCursorAfterUndoRedo(int, int, int)));
    q->ensureCursorVisible();
}

void QTextControlPrivate::redo()
{
    Q_Q(QTextControl);
    QObject::connect(doc, SIGNAL(contentsChange(int, int, int)),
                     q, SLOT(setCursorAfterUndoRedo(int, int, int)));
    doc->redo();
    QObject::disconnect(doc, SIGNAL(contentsChange(int, int, int)),
                        q, SLOT(setCursorAfterUndoRedo(int, int, int)));
    q->ensureCursorVisible();
}

void QTextControlPrivate::setCursorAfterUndoRedo(int undoPosition, int /*charsRemoved*/, int charsAdded)
{
    if (cursor.isNull())
        return;

    cursor.setPosition(undoPosition + charsAdded);
    // don't call ensureCursorVisible here but wait until the text is layouted, which will
    // be the case in QTextControl::undo() after calling undo() on the document.
}

/*
    \class QTextControl
    \brief The QTextControl class provides a widget that is used to edit and display
    both plain and rich text.

    \ingroup text
    \mainclass

    \tableofcontents

    \section1 Introduction and Concepts

    QTextControl is an advanced WYSIWYG viewer/editor supporting rich
    text formatting using HTML-style tags. It is optimized to handle
    large documents and to respond quickly to user input.

    QTextControl works on paragraphs and characters. A paragraph is a
    formatted string which is word-wrapped to fit into the width of
    the widget. By default when reading plain text, one newline
    signifies a paragraph. A document consists of zero or more
    paragraphs. The words in the paragraph are aligned in accordance
    with the paragraph's alignment. Paragraphs are separated by hard
    line breaks. Each character within a paragraph has its own
    attributes, for example, font and color.

    QTextControl can display images, lists and tables. If the text is
    too large to view within the text edit's viewport, scrollbars will
    appear. The text edit can load both plain text and HTML files (a
    subset of HTML 3.2 and 4).

    If you just need to display a small piece of rich text use QLabel.

    Note that we do not intend to add a full-featured web browser
    widget to Qt (because that would easily double Qt's size and only
    a few applications would benefit from it). The rich
    text support in Qt is designed to provide a fast, portable and
    efficient way to add reasonable online help facilities to
    applications, and to provide a basis for rich text editors.

    \section1 Using QTextControl as a Display Widget

    QTextControl can display a large HTML subset, including tables and
    images.

    The text is set or replaced using setHtml() which deletes any
    existing text and replaces it with the text passed in the
    setHtml() call. If you call setHtml() with legacy HTML, and then
    call text(), the text that is returned may have different markup,
    but will render the same. The entire text can be deleted with clear().

    Text itself can be inserted using the QTextCursor class or using the
    convenience functions insertHtml(), insertPlainText(), append() or
    paste(). QTextCursor is also able to insert complex objects like tables
    or lists into the document, and it deals with creating selections
    and applying changes to selected text.

    By default the text edit wraps words at whitespace to fit within
    the text edit widget. The setLineWrapMode() function is used to
    specify the kind of line wrap you want, or \l NoWrap if you don't
    want any wrapping. Call setLineWrapMode() to set a fixed pixel width
    \l FixedPixelWidth, or character column (e.g. 80 column) \l
    FixedColumnWidth with the pixels or columns specified with
    setLineWrapColumnOrWidth(). If you use word wrap to the widget's width
    \l WidgetWidth, you can specify whether to break on whitespace or
    anywhere with setWordWrapMode().

    The find() function can be used to find and select a given string
    within the text.

    \section2 Read-only Key Bindings

    When QTextControl is used read-only the key-bindings are limited to
    navigation, and text may only be selected with the mouse:
    \table
    \header \i Keypresses \i Action
    \row \i Qt::UpArrow        \i Moves one line up.
    \row \i Qt::DownArrow        \i Moves one line down.
    \row \i Qt::LeftArrow        \i Moves one character to the left.
    \row \i Qt::RightArrow        \i Moves one character to the right.
    \row \i PageUp        \i Moves one (viewport) page up.
    \row \i PageDown        \i Moves one (viewport) page down.
    \row \i Home        \i Moves to the beginning of the text.
    \row \i End                \i Moves to the end of the text.
    \row \i Alt+Wheel
         \i Scrolls the page horizontally (the Wheel is the mouse wheel).
    \row \i Ctrl+Wheel        \i Zooms the text.
    \row \i Ctrl+A            \i Selects all text.
    \endtable

    The text edit may be able to provide some meta-information. For
    example, the documentTitle() function will return the text from
    within HTML \c{<title>} tags.

    \section1 Using QTextControl as an Editor

    All the information about using QTextControl as a display widget also
    applies here.

    The current char format's attributes are set with setFontItalic(),
    setFontWeight(), setFontUnderline(), setFontFamily(),
    setFontPointSize(), setTextColor() and setCurrentFont(). The current
    paragraph's alignment is set with setAlignment().

    Selection of text is handled by the QTextCursor class, which provides
    functionality for creating selections, retrieving the text contents or
    deleting selections. You can retrieve the object that corresponds with
    the user-visible cursor using the textCursor() method. If you want to set
    a selection in QTextControl just create one on a QTextCursor object and
    then make that cursor the visible cursor using setCursor(). The selection
    can be copied to the clipboard with copy(), or cut to the clipboard with
    cut(). The entire text can be selected using selectAll().

    When the cursor is moved and the underlying formatting attributes change,
    the currentCharFormatChanged() signal is emitted to reflect the new attributes
    at the new cursor position.

    QTextControl holds a QTextDocument object which can be retrieved using the
    document() method. You can also set your own document object using setDocument().
    QTextDocument emits a textChanged() signal if the text changes and it also
    provides a isModified() function which will return true if the text has been
    modified since it was either loaded or since the last call to setModified
    with false as argument. In addition it provides methods for undo and redo.

    \section2 Editing Key Bindings

    The list of key bindings which are implemented for editing:
    \table
    \header \i Keypresses \i Action
    \row \i Backspace \i Deletes the character to the left of the cursor.
    \row \i Delete \i Deletes the character to the right of the cursor.
    \row \i Ctrl+C \i Copy the selected text to the clipboard.
    \row \i Ctrl+Insert \i Copy the selected text to the clipboard.
    \row \i Ctrl+K \i Deletes to the end of the line.
    \row \i Ctrl+V \i Pastes the clipboard text into text edit.
    \row \i Shift+Insert \i Pastes the clipboard text into text edit.
    \row \i Ctrl+X \i Deletes the selected text and copies it to the clipboard.
    \row \i Shift+Delete \i Deletes the selected text and copies it to the clipboard.
    \row \i Ctrl+Z \i Undoes the last operation.
    \row \i Ctrl+Y \i Redoes the last operation.
    \row \i LeftArrow \i Moves the cursor one character to the left.
    \row \i Ctrl+LeftArrow \i Moves the cursor one word to the left.
    \row \i RightArrow \i Moves the cursor one character to the right.
    \row \i Ctrl+RightArrow \i Moves the cursor one word to the right.
    \row \i UpArrow \i Moves the cursor one line up.
    \row \i Ctrl+UpArrow \i Moves the cursor one word up.
    \row \i DownArrow \i Moves the cursor one line down.
    \row \i Ctrl+Down Arrow \i Moves the cursor one word down.
    \row \i PageUp \i Moves the cursor one page up.
    \row \i PageDown \i Moves the cursor one page down.
    \row \i Home \i Moves the cursor to the beginning of the line.
    \row \i Ctrl+Home \i Moves the cursor to the beginning of the text.
    \row \i End \i Moves the cursor to the end of the line.
    \row \i Ctrl+End \i Moves the cursor to the end of the text.
    \row \i Alt+Wheel \i Scrolls the page horizontally (the Wheel is the mouse wheel).
    \row \i Ctrl+Wheel \i Zooms the text.
    \endtable

    To select (mark) text hold down the Shift key whilst pressing one
    of the movement keystrokes, for example, \e{Shift+Right Arrow}
    will select the character to the right, and \e{Shift+Ctrl+Right
    Arrow} will select the word to the right, etc.

    \sa QTextDocument QTextCursor document() textCursor() setDocument() setTextCursor()

*/

/*
    \property QTextControl::undoRedoEnabled
    \brief whether undo and redo are enabled

    Users are only able to undo or redo actions if this property is
    true, and if there is an action that can be undone (or redone).
*/

/*
    \enum QTextControl::LineWrapMode

    \value NoWrap
    \value WidgetWidth
    \value FixedPixelWidth
    \value FixedColumnWidth
*/

/*
    \enum QTextControl::CursorAction

    \value MoveBackward
    \value MoveForward
    \value MoveWordBackward
    \value MoveWordForward
    \value MoveUp
    \value MoveDown
    \value MoveLineStart
    \value MoveLineEnd
    \value MoveHome
    \value MoveEnd
    \value MovePageUp
    \value MovePageDown

    \omitvalue MovePgUp
    \omitvalue MovePgDown
*/

/*
    Constructs an empty QTextControl with parent \a
    parent.
*/
QTextControl::QTextControl(QObject *parent)
    : QObject(*new QTextControlPrivate, parent)
{
    Q_D(QTextControl);
    d->init();
}

/*
    Constructs a QTextControl with parent \a parent. The text edit will display
    the text \a text. The text is interpreted as html.
*/
QTextControl::QTextControl(const QString &text, QObject *parent)
    : QObject(*new QTextControlPrivate, parent)
{
    Q_D(QTextControl);
    d->init(text);
}

/*
    Destructor.
*/
QTextControl::~QTextControl()
{
}

/*
    Makes \a document the new document of the text editor.

    The parent QObject of the provided document remains the owner
    of the object. If the current document is a child of the text
    editor, then it is deleted.

    \sa document()
*/
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
// ########    d->relayoutDocument();
}

/*
    Returns a pointer to the underlying document.

    \sa setDocument()
*/
QTextDocument *QTextControl::document() const
{
    Q_D(const QTextControl);
    return d->doc;
}

/*
    Sets the visible \a cursor.
*/
void QTextControl::setTextCursor(const QTextCursor &cursor)
{
    Q_D(QTextControl);
    const bool posChanged = cursor.position() != d->cursor.position();
    d->cursor = cursor;
    d->updateCurrentCharFormatAndSelection();
    ensureCursorVisible();
    // ### be smarter, repaint only cursor/selector
    emit updateRequest();
    if (posChanged)
        emit cursorPositionChanged();
}

/*
    Returns a copy of the QTextCursor that represents the currently visible cursor.
    Note that changes on the returned cursor do not affect QTextControl's cursor; use
    setTextCursor() to update the visible cursor.
 */
QTextCursor QTextControl::textCursor() const
{
    Q_D(const QTextControl);
    return d->cursor;
}

/*
    \fn void QTextControl::undo() const

    Undoes the last operation.

    If there is no operation to undo, i.e. there is no undo step in
    the undo/redo history, nothing happens.

    \sa redo()
*/

/*
    \fn void QTextControl::redo() const

    Redoes the last operation.

    If there is no operation to redo, i.e. there is no redo step in
    the undo/redo history, nothing happens.

    \sa undo()
*/

#ifndef QT_NO_CLIPBOARD
/*
    Copies the selected text to the clipboard and deletes it from
    the text edit.

    If there is no selected text nothing happens.

    \sa copy() paste()
*/

void QTextControl::cut()
{
    Q_D(QTextControl);
    if (d->readOnly || !d->cursor.hasSelection())
	return;
    copy();
    d->cursor.removeSelectedText();
}

/*
    Copies any selected text to the clipboard.

    \sa copyAvailable()
*/

void QTextControl::copy()
{
    Q_D(QTextControl);
    if (!d->cursor.hasSelection())
	return;
    QMimeData *data = createMimeDataFromSelection();
    QApplication::clipboard()->setMimeData(data);
}

/*
    Pastes the text from the clipboard into the text edit at the
    current cursor position.

    If there is no text in the clipboard nothing happens.

    To change the behavior of this function, i.e. to modify what
    QTextControl can paste and how it is being pasted, reimplement the
    virtual canInsertFromMimeData() and insertFromMimeData()
    functions.

    \sa cut() copy()
*/

void QTextControl::paste()
{
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if (md)
        insertFromMimeData(md);
}
#endif

/*
    Deletes all the text in the text edit.

    Note that the undo/redo history is cleared by this function.

    \sa cut() setPlainText() setHtml()
*/
void QTextControl::clear()
{
    Q_D(QTextControl);
    // clears and sets empty content
    d->setContent();
}


/*
    Selects all text.

    \sa copy() cut() textCursor()
 */
void QTextControl::selectAll()
{
    Q_D(QTextControl);
    d->cursor.select(QTextCursor::Document);
    d->selectionChanged();
// ########    d->updateViewport();
}

/* #######
bool QTextControl::event(QEvent *e)
{
    Q_D(QTextControl);
    if (e->type() == QEvent::ContextMenu
        && static_cast<QContextMenuEvent *>(e)->reason() == QContextMenuEvent::Keyboard) {
        Q_D(QTextControl);
        ensureCursorVisible();
        const QPoint cursorPos = cursorRect().center();
        QContextMenuEvent ce(QContextMenuEvent::Keyboard, cursorPos, d->viewport->mapToGlobal(cursorPos));
        ce.setAccepted(e->isAccepted());
        const bool result = QAbstractScrollArea::event(&ce);
        e->setAccepted(ce.isAccepted());
        return result;
    } else if (e->type() == QEvent::ShortcutOverride && !d->readOnly) {
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
    return QAbstractScrollArea::event(e);
}
*/

/* \internal
*/

void QTextControl::timerEvent(QTimerEvent *e)
{
    Q_D(QTextControl);
    if (e->timerId() == d->cursorBlinkTimer.timerId()) {
        d->cursorOn = !d->cursorOn;

        /* ########
        if (d->cursor.hasSelection())
            d->cursorOn &= (style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, 0, this)
                            != 0);
        */

        d->repaintCursor();
/*
    } else if (e->timerId() == d->dragStartTimer.timerId()) {
        d->dragStartTimer.stop();
        d->startDrag();
*/
    } else if (e->timerId() == d->trippleClickTimer.timerId()) {
        d->trippleClickTimer.stop();
    }
/* #####
    else if (e->timerId() == d->autoScrollTimer.timerId()) {
        const QPoint globalPos = QCursor::pos();
        const QPoint pos = d->viewport->mapFromGlobal(globalPos);
        QMouseEvent ev(QEvent::MouseMove, pos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mouseMoveEvent(&ev);
    }
*/
#ifdef QT_KEYPAD_NAVIGATION
    else if (e->timerId() == d->deleteAllTimer.timerId()) {
        d->deleteAllTimer.stop();
        clear();
    }
#endif
}

/*
    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as plain text.

    Note that the undo/redo history is cleared by this function.

    \sa toPlainText()
*/

void QTextControl::setPlainText(const QString &text)
{
    Q_D(QTextControl);
    d->setContent(Qt::PlainText, text);
    d->preferRichText = false;
}

/*
    \fn QString QTextControl::toPlainText() const

    Returns the text of the text edit as plain text.

    \sa QTextControl::setPlainText()
 */


/*
    \property QTextControl::html

    This property provides an HTML interface to the text of the text edit.

    toHtml() returns the text of the text edit as html.

    setHtml() changes the text of the text edit.  Any previous text is
    removed. The input text is interpreted as rich text in html format.

    Note that the undo/redo history is cleared by calling setHtml().

    \sa {Supported HTML Subset}
*/

void QTextControl::setHtml(const QString &text)
{
    Q_D(QTextControl);
    d->setContent(Qt::RichText, text);
    d->preferRichText = true;
}

/* \reimp
*/
void QTextControl::keyPressEvent(QKeyEvent *e)
{
    Q_D(QTextControl);

    if (e->modifiers() & Qt::ControlModifier) {
        switch( e->key() ) {
#ifndef QT_NO_CLIPBOARD
        case Qt::Key_C:
        case Qt::Key_Insert:
        case Qt::Key_F16: // Copy key on Sun keyboards
            e->accept();
            copy();
            return;
#endif // QT_NO_CLIPBOARD
        case Qt::Key_A:
            e->accept();
            selectAll();
            return;
        default:
            break;
        }
    }

    if (d->readOnly) {
        /* #######
        switch (e->key()) {
            case Qt::Key_Home:
                e->accept();
                d->vbar->triggerAction(QAbstractSlider::SliderToMinimum);
                break;
            case Qt::Key_End:
                e->accept();
                d->vbar->triggerAction(QAbstractSlider::SliderToMaximum);
                break;
            case Qt::Key_Space:
                e->accept();
                if (e->modifiers() & Qt::ShiftModifier)
                    d->vbar->triggerAction(QAbstractSlider::SliderPageStepSub);
                else
                    d->vbar->triggerAction(QAbstractSlider::SliderPageStepAdd);
            default:
                QAbstractScrollArea::keyPressEvent(e);
                break;
        }
        */
        return;
    }

#ifdef QT_KEYPAD_NAVIGATION
    switch (e->key()) {
        case Qt::Key_Select:
            if (QApplication::keypadNavigationEnabled())
                setEditFocus(!hasEditFocus());
            break;
        case Qt::Key_Back:
        case Qt::Key_No:
            if (!QApplication::keypadNavigationEnabled()
                    || (QApplication::keypadNavigationEnabled() && !hasEditFocus())) {
                e->ignore();
                return;
            }
            break;
        default:
            if (QApplication::keypadNavigationEnabled()) {
                if (!hasEditFocus() && !(e->modifiers() & Qt::ControlModifier)) {
                    if (e->text()[0].isPrint()) {
                        setEditFocus(true);
                        clear();
                    } else {
                        e->ignore();
                        return;
                    }
                }
            }
    }
#endif

    // schedule a repaint of the region of the cursor, as when we move it we
    // want to make sure the old cursor disappears (not noticable when moving
    // only a few pixels but noticable when jumping between cells in tables for
    // example)
    d->repaintSelection();

    if (e->key() == Qt::Key_Direction_L || e->key() == Qt::Key_Direction_R) {
        QTextBlockFormat fmt;
        fmt.setLayoutDirection((e->key() == Qt::Key_Direction_L) ? Qt::LeftToRight : Qt::RightToLeft);
        d->cursor.mergeBlockFormat(fmt);
        goto accept;
    }

    if (d->cursorMoveKeyEvent(e))
        goto accept;

    if (e->modifiers() & Qt::ControlModifier) {
        switch( e->key() ) {
        case Qt::Key_Z:
            if (e->modifiers() & Qt::ShiftModifier)
                d->redo();
            else
                d->undo();
            break;
        case Qt::Key_Y:
            d->doc->redo();
            break;
#ifndef QT_NO_CLIPBOARD
        case Qt::Key_X:
        case Qt::Key_F20:  // Cut key on Sun keyboards
            cut();
            break;
        case Qt::Key_V:
        case Qt::Key_F18:  // Paste key on Sun keyboards
            paste();
            break;
#endif // QT_NO_CLIPBOARD
        case Qt::Key_Backspace:
            d->cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            goto process;
        case Qt::Key_Delete:
            d->cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
            goto process;
        case Qt::Key_K: {
            QTextBlock block = d->cursor.block();
            if (d->cursor.position() == block.position() + block.length() - 2)
                d->cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            else
                d->cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            d->cursor.deleteChar();
            break;
        }
        default:
            goto process;
        }
        goto accept;
    }

process:
    switch( e->key() ) {
    case Qt::Key_Backspace: {
#if defined(Q_WS_WIN)
        if (e->modifiers() & Qt::AltModifier) {
            if (e->modifiers() & Qt::ShiftModifier)
                d->redo();
            else
                d->undo();
        } else
#endif
        {
            QTextBlockFormat blockFmt = d->cursor.blockFormat();
            QTextList *list = d->cursor.currentList();
            if (list && d->cursor.atBlockStart()) {
                list->remove(d->cursor.block());
            } else if (d->cursor.atBlockStart() && blockFmt.indent() > 0) {
                blockFmt.setIndent(blockFmt.indent() - 1);
                d->cursor.setBlockFormat(blockFmt);
            } else {
                d->cursor.deletePreviousChar();
            }
        }
        break;
    }
    case Qt::Key_Delete:
#ifndef QT_NO_CLIPBOARD
        if (e->modifiers() & Qt::ShiftModifier)
            cut();
	else
#endif
            d->cursor.deleteChar();
        break;
#ifndef QT_NO_CLIPBOARD
    case Qt::Key_Insert:
        if (e->modifiers() & Qt::ShiftModifier)
            paste();
        break;
#endif
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (e->modifiers() & Qt::ControlModifier)
            d->cursor.insertText(QString(QChar::LineSeparator));
        else
            d->cursor.insertBlock();
        break;
#ifdef QT_KEYPAD_NAVIGATION
    case Qt::Key_Up:
    case Qt::Key_Down:
        if (QApplication::keypadNavigationEnabled()) {
            // Cursor position didn't change, so we want to leave
            // these keys to change focus.
            e->ignore();
            return;
        }
        break;
    case Qt::Key_Back:
        if (!e->isAutoRepeat()) {
            if (QApplication::keypadNavigationEnabled()) {
                if (document()->isEmpty()) {
                    setEditFocus(false);
                } else if (!d->deleteAllTimer.isActive()) {
                    d->deleteAllTimer.start(750, this);
                }
            } else {
                e->ignore();
                return;
            }
        }
        break;
#endif
    default:
        {
            QString text = e->text();

            if (!text.isEmpty() && (text.at(0).isPrint() || text.at(0) == QLatin1Char('\t'))) {
                if (d->overwriteMode
                    // no need to call deleteChar() if we have a selection, insertText
                    // does it already
                    && !d->cursor.hasSelection()
                    && !d->cursor.atBlockEnd())
                    d->cursor.deleteChar();

                d->cursor.insertText(text);
                d->selectionChanged();
            } else {
// ##########                QAbstractScrollArea::keyPressEvent(e);
                return;
            }
            break;
        }
    }

 accept:
    e->accept();
    d->cursorOn = true;

    ensureCursorVisible();

    d->updateCurrentCharFormat();
}

/* \reimp
*/
void QTextControl::keyReleaseEvent(QKeyEvent * /* e */)
{
#ifdef QT_KEYPAD_NAVIGATION
    Q_D(QTextControl);
    if (QApplication::keypadNavigationEnabled()) {
        if (!e->isAutoRepeat() && e->key() == Qt::Key_Back
            && d->deleteAllTimer.isActive()) {
            d->deleteAllTimer.stop();
            QTextBlockFormat blockFmt = d->cursor.blockFormat();

            QTextList *list = d->cursor.currentList();
            if (list && d->cursor.atBlockStart()) {
                list->remove(d->cursor.block());
            } else if (d->cursor.atBlockStart() && blockFmt.indent() > 0) {
                blockFmt.setIndent(blockFmt.indent() - 1);
                d->cursor.setBlockFormat(blockFmt);
            } else {
                d->cursor.deletePreviousChar();
            }
        }
    }
#endif
}

/*
    Loads the resource specified by the given \a type and \a name.

    This function is an extension of QTextDocument::loadResource().

    \sa QTextDocument::loadResource()
*/
QVariant QTextControl::loadResource(int type, const QUrl &name)
{
    Q_UNUSED(type);
    Q_UNUSED(name);
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

    QRectF r;

    if (line.isValid())
        r = QRectF(layoutPos.x() + line.cursorToX(relativePos) - 5, layoutPos.y() + line.y(),
                  10, line.ascent() + line.descent() + 1.);
    else
        r = QRectF(layoutPos.x() - 5, layoutPos.y(), 10, 10); // #### correct height

    return r;
}

QRectF QTextControlPrivate::selectionRect() const
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
            if (posLine.lineNumber() == anchorLine.lineNumber()) {
                r = posLine.rect();
            } else {
                r = posBlock.layout()->boundingRect();
            }
            r.translate(doc->documentLayout()->blockBoundingRect(posBlock).topLeft());
        } else {
            QRectF anchorRect = rectForPosition(cursor.selectionEnd());
            r |= anchorRect;
            QRectF frameRect(doc->documentLayout()->frameBoundingRect(cursor.currentFrame()));
            r.setLeft(frameRect.left());
            r.setRight(frameRect.right());
        }
    }

    return r;
}

/* \reimp
*/
void QTextControl::mousePressEvent(QMouseEvent *e)
{
    Q_D(QTextControl);

    if (!(e->button() & Qt::LeftButton))
        return;

    const int oldCursorPos = d->cursor.position();

    const QPointF pos = d->mapToContents(e->pos());

    d->mousePressed = true;
#ifndef QT_NO_DRAGANDDROP
    d->mightStartDrag = false;
#endif
    d->repaintSelection();

    if (d->trippleClickTimer.isActive()
        && ((e->globalPos() - d->trippleClickPoint).manhattanLength() < QApplication::startDragDistance())) {

#if defined(Q_WS_MAC)
        d->cursor.select(QTextCursor::LineUnderCursor);
        d->selectedLineOnDoubleClick = d->cursor;
        d->selectedWordOnDoubleClick = QTextCursor();
#else
        d->cursor.movePosition(QTextCursor::StartOfBlock);
        d->cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
#endif

        d->trippleClickTimer.stop();
    } else {
        int cursorPos = d->doc->documentLayout()->hitTest(pos, Qt::FuzzyHit);
        if (cursorPos == -1)
            return;

#if !defined(QT_NO_IM)
        /* #########
        QTextLayout *layout = d->cursor.block().layout();
        if (layout && !layout->preeditAreaText().isEmpty()) {
            QInputContext *ctx = inputContext();
            if (ctx)
                ctx->mouseHandler(cursorPos - d->cursor.position(), e);
            if (!layout->preeditAreaText().isEmpty())
                return;
        }
        */
#endif
        if (e->modifiers() == Qt::ShiftModifier) {
            if (d->selectedWordOnDoubleClick.hasSelection())
                d->extendWordwiseSelection(cursorPos, pos.x());
            else if (d->selectedLineOnDoubleClick.hasSelection())
                d->extendLinewiseSelection(cursorPos);
            else
                d->setCursorPosition(cursorPos, QTextCursor::KeepAnchor);
        } else {

            if (d->cursor.hasSelection()
                && cursorPos >= d->cursor.selectionStart()
                && cursorPos <= d->cursor.selectionEnd()
                && d->doc->documentLayout()->hitTest(pos, Qt::ExactHit) != -1) {
#ifndef QT_NO_DRAGANDDROP
                d->mightStartDrag = true;
                d->dragStartPos = e->globalPos();
                d->dragStartTimer.start(QApplication::startDragTime(), this);
#endif
                return;
            }

            d->setCursorPosition(cursorPos);
        }
    }

    if (d->readOnly) {
        if (d->cursor.position() != oldCursorPos)
            emit cursorPositionChanged();
        d->selectionChanged();
    } else {
        ensureCursorVisible();
        if (d->cursor.position() != oldCursorPos)
            emit cursorPositionChanged();
        d->updateCurrentCharFormatAndSelection();
    }
    d->repaintSelection();
}

/* \reimp
*/
void QTextControl::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QTextControl);
    if (!(e->buttons() & Qt::LeftButton))
        return;

    if (!(d->mousePressed
          || d->selectedWordOnDoubleClick.hasSelection()
          || d->selectedLineOnDoubleClick.hasSelection()))
        return;

    const int oldCursorPos = d->cursor.position();

/* ####
    if (d->mightStartDrag) {
        d->dragStartTimer.stop();

        if ((e->globalPos() - d->dragStartPos).manhattanLength() > QApplication::startDragDistance())
            d->startDrag();

        return;
    }
*/
    d->repaintSelection();
    const QPointF mousePos = d->mapToContents(e->pos());
    const qreal mouseX = qreal(mousePos.x());

    /* ###### portme
    if (d->autoScrollTimer.isActive()) {
        if (d->viewport->rect().contains(e->pos()))
            d->autoScrollTimer.stop();
    } else {
        if (!d->viewport->rect().contains(e->pos()))
            d->autoScrollTimer.start(100, this);
    }
    */

#if !defined(QT_NO_IM)
    QTextLayout *layout = d->cursor.block().layout();
    if (layout && !layout->preeditAreaText().isEmpty())
        return;
#endif

    int newCursorPos = d->doc->documentLayout()->hitTest(mousePos, Qt::FuzzyHit);
    if (newCursorPos == -1)
        return;

    if (d->selectedWordOnDoubleClick.hasSelection())
        d->extendWordwiseSelection(newCursorPos, mouseX);
    else if (d->selectedLineOnDoubleClick.hasSelection())
        d->extendLinewiseSelection(newCursorPos);
    else
        d->setCursorPosition(newCursorPos, QTextCursor::KeepAnchor);

    if (d->readOnly) {
        const QPointF pos = d->mapToContents(e->pos());
        emit visibilityRequest(QRectF(pos, QSizeF(1, 1)));
        if (d->cursor.position() != oldCursorPos)
            emit cursorPositionChanged();
        d->selectionChanged();
    } else {
        ensureCursorVisible();
        if (d->cursor.position() != oldCursorPos)
            emit cursorPositionChanged();
        d->updateCurrentCharFormatAndSelection();
    }
    d->repaintSelection();
}

/* \reimp
*/
void QTextControl::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QTextControl);
#if defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD)
    Q_UNUSED(e);
#endif

    d->autoScrollTimer.stop();
    d->repaintSelection();

#ifndef QT_NO_DRAGANDDROP
    if (d->mightStartDrag) {
        d->mousePressed = false;
        d->setCursorPosition(e->pos());
        d->cursor.clearSelection();
        d->selectionChanged();
    }
#endif
    if (d->mousePressed) {
        d->mousePressed = false;
#ifndef QT_NO_CLIPBOARD
        d->setClipboardSelection();
    } else if (e->button() == Qt::MidButton
               && !d->readOnly
               && QApplication::clipboard()->supportsSelection()) {
        d->setCursorPosition(e->pos());
        const QMimeData *md = QApplication::clipboard()->mimeData(QClipboard::Selection);
        if (md)
            insertFromMimeData(md);
#endif
    }

    d->repaintSelection();
#ifndef QT_NO_DRAGANDDROP
    if (d->dragStartTimer.isActive())
        d->dragStartTimer.stop();
#endif
}

/* \reimp
*/
void QTextControl::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_D(QTextControl);
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
#if !defined(QT_NO_IM)
    QTextLayout *layout = d->cursor.block().layout();
    if (layout && !layout->preeditAreaText().isEmpty())
        return;
#endif

#ifndef QT_NO_DRAGANDDROP
    d->mightStartDrag = false;
#endif
    d->repaintSelection();
    d->setCursorPosition(e->pos());
    QTextLine line = currentTextLine(d->cursor);
    if (line.isValid() && line.textLength()) {
        d->cursor.select(QTextCursor::WordUnderCursor);
        d->selectionChanged();
#ifndef QT_NO_CLIPBOARD
        d->setClipboardSelection();
#endif
        d->repaintSelection();
    }

    d->selectedWordOnDoubleClick = d->cursor;

    d->trippleClickPoint = e->globalPos();
    d->trippleClickTimer.start(qApp->doubleClickInterval(), this);
}

/*
  Shows the standard context menu created with createStandardContextMenu().

  If you do not want the text edit to have a context menu, you can set
  its \l contextMenuPolicy to Qt::NoContextMenu. If you want to
  customize the context menu, reimplement this function. If you want
  to extend the standard context menu, reimplement this function, call
  createStandardContextMenu() and extend the menu returned.

  Information about the event is passed in \a e.

    \code
    void TextEdit::contextMenuEvent(QContextMenuEvent * e) {
            QMenu *menu = createStandardContextMenu();
            menu->addAction(My Menu Item");
            //...
            menu->exec(e->globalPos());
            delete menu;
    }
    \endcode
*/
void QTextControl::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QT_NO_CONTEXTMENU
    Q_UNUSED(e);
#else
    QMenu *menu = createStandardContextMenu();
    menu->exec(e->globalPos());
    delete menu;
#endif
}

/*
void QTextControl::dragEnterEvent(QDragEnterEvent *e)
{
    Q_D(QTextControl);
    if (d->readOnly || !canInsertFromMimeData(e->mimeData())) {
        e->ignore();
        return;
    }

    d->dndFeedbackCursor = QTextCursor();

    e->acceptProposedAction();
}

void QTextControl::dragLeaveEvent(QDragLeaveEvent *)
{
    Q_D(QTextControl);

    const QRect crect = cursorRect(d->dndFeedbackCursor);
    d->dndFeedbackCursor = QTextCursor();

    if (crect.isValid())
        d->viewport->update(crect);
}

void QTextControl::dragMoveEvent(QDragMoveEvent *e)
{
    Q_D(QTextControl);
    if (d->readOnly || !canInsertFromMimeData(e->mimeData())) {
        e->ignore();
        return;
    }

    const int cursorPos = d->doc->documentLayout()->hitTest(d->mapToContents(e->pos()), Qt::FuzzyHit);
    if (cursorPos != -1) {
        QRect crect = cursorRect(d->dndFeedbackCursor);
        if (crect.isValid())
            d->viewport->update(crect);

        d->dndFeedbackCursor = d->cursor;
        d->dndFeedbackCursor.setPosition(cursorPos);

        crect = cursorRect(d->dndFeedbackCursor);
        d->viewport->update(crect);
    }

    e->acceptProposedAction();
}

void QTextControl::dropEvent(QDropEvent *e)
{
    Q_D(QTextControl);
    d->dndFeedbackCursor = QTextCursor();

    if (d->readOnly || !canInsertFromMimeData(e->mimeData()))
        return;

    e->acceptProposedAction();

    d->repaintSelection();

    QTextCursor insertionCursor = cursorForPosition(e->pos());
    insertionCursor.beginEditBlock();

    if (e->dropAction() == Qt::MoveAction
        && (e->source() == this || e->source() == d->viewport))
        d->cursor.removeSelectedText();

    d->cursor = insertionCursor;
    insertFromMimeData(e->mimeData());
    insertionCursor.endEditBlock();
}

*/

/* \reimp
 */
void QTextControl::inputMethodEvent(QInputMethodEvent *e)
{
    Q_D(QTextControl);
    if (d->readOnly || d->cursor.isNull()) {
        e->ignore();
        return;
    }
#ifdef QT_KEYPAD_NAVIGATION
    if (QApplication::keypadNavigationEnabled() && !hasEditFocus())
        setEditFocus(true);
#endif
    d->cursor.beginEditBlock();

    d->cursor.removeSelectedText();

    // insert commit string
    if (!e->commitString().isEmpty() || e->replacementLength()) {
        QTextCursor c = d->cursor;
        c.setPosition(c.position() + e->replacementStart());
        c.setPosition(c.position() + e->replacementLength(), QTextCursor::KeepAnchor);
        c.insertText(e->commitString());
    }

    QTextBlock block = d->cursor.block();
    QTextLayout *layout = block.layout();
    layout->setPreeditArea(d->cursor.position() - block.position(), e->preeditString());
    QList<QTextLayout::FormatRange> overrides;
    d->preeditCursor = e->preeditString().length();
    d->hideCursor = false;
    for (int i = 0; i < e->attributes().size(); ++i) {
        const QInputMethodEvent::Attribute &a = e->attributes().at(i);
        if (a.type == QInputMethodEvent::Cursor) {
            d->preeditCursor = a.start;
            d->hideCursor = !a.length;
        } else if (a.type == QInputMethodEvent::TextFormat) {
            QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
            if (f.isValid()) {
                QTextLayout::FormatRange o;
                o.start = a.start + d->cursor.position() - block.position();
                o.length = a.length;
                o.format = f;
                overrides.append(o);
            }
        }
    }
    layout->setAdditionalFormats(overrides);
    d->cursor.endEditBlock();
}

/*\reimp
*/
QVariant QTextControl::inputMethodQuery(Qt::InputMethodQuery property) const
{
    Q_D(const QTextControl);
    QTextBlock block = d->cursor.block();
    switch(property) {
    /* ###### viewport.pos() is obviously wrong for the control
    case Qt::ImMicroFocus:
        return cursorRect().translated(d->viewport->pos());
    */
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
    Q_D(QTextControl);
    if (focus) {
        if (!d->readOnly) {
            d->cursorOn = true;
            d->setBlinkingCursorEnabled(true);
#ifdef QT_KEYPAD_NAVIGATION
            if (QApplication::keypadNavigationEnabled()) {
                if (e->reason() == Qt::TabFocusReason) {
                    d->cursor.movePosition(QTextCursor::Start);
                } else if (e->reason() == Qt::BacktabFocusReason) {
                    d->cursor.movePosition(QTextCursor::End);
                    d->cursor.movePosition(QTextCursor::StartOfLine);
                }
            }
#endif
        }
    } else {
        d->setBlinkingCursorEnabled(false);
        if (reason != Qt::PopupFocusReason)
            d->cursorOn = false;
    }
}

/* \reimp
*/
void QTextControl::showEvent(QShowEvent *)
{
    Q_D(QTextControl);
    if (!d->anchorToScrollToWhenVisible.isEmpty()) {
        scrollToAnchor(d->anchorToScrollToWhenVisible);
        d->anchorToScrollToWhenVisible.clear();
    }
}

#ifndef QT_NO_CONTEXTMENU
/*  This function creates the standard context menu which is shown
  when the user clicks on the line edit with the right mouse
  button. It is called from the default contextMenuEvent() handler.
  The popup menu's ownership is transferred to the caller.
*/

QMenu *QTextControl::createStandardContextMenu()
{
    Q_D(QTextControl);

    QMenu *menu = new QMenu;
    QAction *a;

    if (!d->readOnly) {
        a = menu->addAction(tr("&Undo") + ACCEL_KEY(Z), this, SLOT(undo()));
        a->setEnabled(d->doc->isUndoAvailable());
        a = menu->addAction(tr("&Redo") + ACCEL_KEY(Y), this, SLOT(redo()));
        a->setEnabled(d->doc->isRedoAvailable());
        menu->addSeparator();

        a = menu->addAction(tr("Cu&t") + ACCEL_KEY(X), this, SLOT(cut()));
        a->setEnabled(d->cursor.hasSelection());
    }

    a = menu->addAction(tr("&Copy") + ACCEL_KEY(C), this, SLOT(copy()));
    a->setEnabled(d->cursor.hasSelection());


    if (!d->readOnly) {
#if !defined(QT_NO_CLIPBOARD)
        a = menu->addAction(tr("&Paste") + ACCEL_KEY(V), this, SLOT(paste()));
        const QMimeData *md = QApplication::clipboard()->mimeData();
        a->setEnabled(md && canInsertFromMimeData(md));
#endif
        a = menu->addAction(tr("Delete"), this, SLOT(deleteSelected()));
        a->setEnabled(d->cursor.hasSelection());
    }


    menu->addSeparator();
    a = menu->addAction(tr("Select All") + ACCEL_KEY(A), this, SLOT(selectAll()));

    a->setEnabled(!d->doc->isEmpty());

    if (!d->readOnly) {
        menu->addSeparator();
        QUnicodeControlCharacterMenu *ctrlCharacterMenu = new QUnicodeControlCharacterMenu(this, menu);
        menu->addMenu(ctrlCharacterMenu);
    }

    return menu;
}
#endif // QT_NO_CONTEXTMENU

/*
  returns a QTextCursor at position \a pos (in viewport coordinates).
*/
QTextCursor QTextControl::cursorForPosition(const QPointF &pos) const
{
    Q_D(const QTextControl);
    int cursorPos = d->doc->documentLayout()->hitTest(d->mapToContents(pos), Qt::FuzzyHit);
    if (cursorPos == -1)
        cursorPos = 0;
    QTextCursor c(d->doc);
    c.setPosition(cursorPos);
    return c;
}

/*
  returns a rectangle (in viewport coordinates) that includes the
  \a cursor.
 */
QRectF QTextControl::cursorRect(const QTextCursor &cursor) const
{
    Q_D(const QTextControl);
    if (cursor.isNull())
        return QRectF();

   return d->rectForPosition(cursor.position());
}

/*
  returns a rectangle (in viewport coordinates) that includes the
  cursor of the text edit.
 */
QRectF QTextControl::cursorRect() const
{
    Q_D(const QTextControl);
    return cursorRect(d->cursor);
}


/*
    Returns the reference of the anchor at position \a pos, or an
    empty string if no anchor exists at that point.
*/
QString QTextControl::anchorAt(const QPointF &pos) const
{
    Q_D(const QTextControl);
    return d->doc->documentLayout()->anchorAt(d->mapToContents(pos));
}

/*
   \property QTextControl::overwriteMode
   \since 4.1
*/

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

/*
    \property QTextControl::tabStopWidth
    \brief the tab stop width in pixels
    \since 4.1
*/

int QTextControl::tabStopWidth() const
{
    Q_D(const QTextControl);
    if (QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(d->doc->documentLayout()))
        return qRound(layout->tabStopWidth());
    return 0;
}

void QTextControl::setTabStopWidth(int width)
{
    Q_D(QTextControl);
    if (QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(d->doc->documentLayout()))
        layout->setTabStopWidth(qreal(width));
}

/*
    \property QTextControl::acceptRichText
    \brief whether the text edit accepts rich text insertions by the user
    \since 4.1

    When this propery is set to false text edit will accept only
    plain text input from the user. For example through clipboard or drag and drop.

    This property's default is true.
*/

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

/*
    \class QTextControl::ExtraSelection
    \since 4.2
    \brief The QTextControl::ExtraSelection structure provides a way of specifying a
           character format for a given selection in a document
*/

/*
    \variable QTextControl::ExtraSelection::cursor
    A cursor that contains a selection in a QTextDocument
*/

/*
    \variable QTextControl::ExtraSelection::format
    A format that is used to specify a foreground or background brush/color
    for the selection.
*/

/*
    \since 4.2
    This function allows temporarily marking certain regions in the document
    with a given color, specified as \a selections. This can be useful for
    example in a programming editor to mark a whole line of text with a given
    background color to indicate the existance of a breakpoint.

    \sa QTextControl::ExtraSelection, extraSelections()
*/
void QTextControl::setExtraSelections(const QList<ExtraSelection> &selections)
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

    d->extraSelections.resize(selections.count());
    for (int i = 0; i < selections.count(); ++i) {
        d->extraSelections[i].cursor = selections.at(i).cursor;
        d->extraSelections[i].format = selections.at(i).format;
    }
    // ### smarter update
    emit updateRequest();
}

/*
    \since 4.2
    Returns previously set extra selections.

    \sa setExtraSelections()
*/
QList<QTextControl::ExtraSelection> QTextControl::extraSelections() const
{
    Q_D(const QTextControl);
    QList<ExtraSelection> selections;
    for (int i = 0; i < d->extraSelections.count(); ++i) {
        ExtraSelection sel;
        sel.cursor = d->extraSelections.at(i).cursor;
        sel.format = d->extraSelections.at(i).format;
        selections.append(sel);
    }
    return selections;
}

/*
    This function returns a new MIME data object to represent the contents
    of the text edit's current selection. It is called when the selection needs
    to be encapsulated into a new QMimeData object; for example, when a drag
    and drop operation is started, or when data is copyied to the clipboard.

    If you reimplement this function, note that the ownership of the returned
    QMimeData object is passed to the caller. The selection can be retrieved
    by using the textCursor() function.
*/
QMimeData *QTextControl::createMimeDataFromSelection() const
{
    Q_D(const QTextControl);
    const QTextDocumentFragment fragment(d->cursor);
    return new QTextEditMimeData(fragment);
}

/*
    This function returns true if the contents of the MIME data object, specified
    by \a source, can be decoded and inserted into the document. It is called
    for example when during a drag operation the mouse enters this widget and it
    is necessary to determine whether it is possible to accept the drag.
 */
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

/*
    This function inserts the contents of the MIME data object, specified
    by \a source, into the text edit at the current cursor position. It is
    called whenever text is inserted as the result of a clipboard paste
    operation, or when the text edit accepts data from a drag and drop
    operation.
*/
void QTextControl::insertFromMimeData(const QMimeData *source)
{
    Q_D(QTextControl);
    if (d->readOnly || !source)
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

/*
    \property QTextControl::readOnly
    \brief whether the text edit is read-only

    In a read-only text edit the user can only navigate through the
    text and select text; modifying the text is not possible.

    This property's default is false.
*/

bool QTextControl::isReadOnly() const
{
    Q_D(const QTextControl);
    return d->readOnly;
}

void QTextControl::setReadOnly(bool ro)
{
    Q_D(QTextControl);
    if (d->readOnly == ro)
        return;

    d->readOnly = ro;
    d->cursorOn = !ro;

    /* ####
    if (hasFocus())
        d->setBlinkingCursorEnabled(!ro);
    */
}

/*
    If the editor has a selection then the properties of \a modifier are
    applied to the selection. Without a selection the properties are applied
    to the word under the cursor. In addition they are always merged into
    the current char format.

    \sa QTextCursor::mergeCharFormat()
 */
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

/*
    Sets the char format that is be used when inserting new text to
    \a format .
 */
void QTextControl::setCurrentCharFormat(const QTextCharFormat &format)
{
    Q_D(QTextControl);
    d->cursor.setCharFormat(format);
    d->lastCharFormat = format;
}

/*
    Returns the char format that is used when inserting new text.
 */
QTextCharFormat QTextControl::currentCharFormat() const
{
    Q_D(const QTextControl);
    return d->cursor.charFormat();
}

/*
    Convenience slot that inserts \a text at the current
    cursor position.

    It is equivalent to

    \code
    edit->textCursor().insertText(text);
    \endcode
 */
void QTextControl::insertPlainText(const QString &text)
{
    Q_D(QTextControl);
    d->cursor.insertText(text);
}

/*
    Convenience slot that inserts \a text which is assumed to be of
    html formatting at the current cursor position.

    It is equivalent to:

    \code
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(text);
    edit->textCursor().insertFragment(fragment);
    \endcode
 */
void QTextControl::insertHtml(const QString &text)
{
    Q_D(QTextControl);
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(text);
    d->cursor.insertFragment(fragment);
}

/*
    Scrolls the text edit so that the anchor with the given \a name is
    visible; does nothing if the \a name is empty, or is already
    visible, or isn't found.
*/
void QTextControl::scrollToAnchor(const QString &name)
{
    Q_D(QTextControl);
    if (name.isEmpty())
        return;

    /* ####
    if (!isVisible()) {
        d->anchorToScrollToWhenVisible = name;
        return;
    }
    */

    for (QTextBlock block = d->doc->begin(); block.isValid(); block = block.next()) {
        QTextCharFormat format = block.charFormat();
        if (format.isAnchor() && format.anchorName() == name) {
            d->ensureVisible(block.position());
            return;
        }

        for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            format = fragment.charFormat();
            if (format.isAnchor() && format.anchorName() == name) {
                d->ensureVisible(fragment.position());
                return;
            }
        }
    }
}

void QTextControl::adjustSize()
{
    // Pull this private function in from qglobal.cpp
    Q_CORE_EXPORT unsigned int qt_int_sqrt(unsigned int n);

    Q_D(QTextControl);
    QFont f = d->doc->defaultFont();
    QFontMetrics fm(f);
    int mw =  fm.width(QLatin1Char('x')) * 80;
    int w = mw;
    d->doc->setPageSize(QSizeF(w, INT_MAX));
    QSizeF size = d->doc->documentLayout()->documentSize();
    if (size.width() != 0) {
        w = qt_int_sqrt((uint)(5 * size.height() * size.width() / 3));
        d->doc->setPageSize(QSizeF(qMin(w, mw), INT_MAX));

        size = d->doc->documentLayout()->documentSize();
        if (w*3 < 5*size.height()) {
            w = qt_int_sqrt((uint)(2 * size.height() * size.width()));
            d->doc->setPageSize(QSizeF(qMin(w, mw), INT_MAX));
        }
    }
}

/*
    \property QTextControl::documentTitle
    \brief the title of the document parsed from the text.
*/

/*
    \property QTextControl::lineWrapMode
    \brief the line wrap mode

    The default mode is WidgetWidth which causes words to be
    wrapped at the right edge of the text edit. Wrapping occurs at
    whitespace, keeping whole words intact. If you want wrapping to
    occur within words use setWrapPolicy(). If you set a wrap mode of
    FixedPixelWidth or FixedColumnWidth you should also call
    setWrapColumnOrWidth() with the width you want.

    \sa lineWrapColumnOrWidth
*/

QTextControl::LineWrapMode QTextControl::lineWrapMode() const
{
    Q_D(const QTextControl);
    return d->lineWrap;
}

void QTextControl::setLineWrapMode(LineWrapMode wrap)
{
    Q_D(QTextControl);
    if (d->lineWrap == wrap)
        return;
    d->lineWrap = wrap;
// ####    d->relayoutDocument();
}

/*
    \property QTextControl::lineWrapColumnOrWidth
    \brief the position (in pixels or columns depending on the wrap mode) where text will be wrapped

    If the wrap mode is FixedPixelWidth, the value is the number of
    pixels from the left edge of the text edit at which text should be
    wrapped. If the wrap mode is FixedColumnWidth, the value is the
    column number (in character columns) from the left edge of the
    text edit at which text should be wrapped.

    \sa lineWrapMode
*/

int QTextControl::lineWrapColumnOrWidth() const
{
    Q_D(const QTextControl);
    return d->lineWrapColumnOrWidth;
}

void QTextControl::setLineWrapColumnOrWidth(int w)
{
    Q_D(QTextControl);
    d->lineWrapColumnOrWidth = w;
// #####    d->relayoutDocument();
}

/*
    \property QTextControl::wordWrapMode
    \brief the mode QTextControl will use when wrapping text by words

    \sa QTextOption::WrapMode
*/

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

/*
    Finds the next occurrence of the string, \a exp, using the given
    \a options. Returns true if \a exp was found and changes the
    cursor to select the match; otherwise returns false.
*/
bool QTextControl::find(const QString &exp, QTextDocument::FindFlags options)
{
    Q_D(QTextControl);
    QTextCursor search = d->doc->find(exp, d->cursor, options);
    if (search.isNull())
        return false;

    setTextCursor(search);
    return true;
}

/*
    \fn void QTextControl::copyAvailable(bool yes)

    This signal is emitted when text is selected or de-selected in the
    text edit.

    When text is selected this signal will be emitted with \a yes set
    to true. If no text has been selected or if the selected text is
    de-selected this signal is emitted with \a yes set to false.

    If \a yes is true then copy() can be used to copy the selection to
    the clipboard. If \a yes is false then copy() does nothing.

    \sa selectionChanged()
*/

/*
    \fn void QTextControl::currentCharFormatChanged(const QTextCharFormat &f)

    This signal is emitted if the current character format has changed, for
    example caused by a change of the cursor position.

    The new format is \a f.

    \sa setCurrentCharFormat()
*/

/*
    \fn void QTextControl::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa copyAvailable()
*/

/*
    \fn void QTextControl::cursorPositionChanged()

    This signal is emitted whenever the position of the
    cursor changed.
*/

/*
    Appends a new paragraph with \a text to the end of the text edit.
*/
void QTextControl::append(const QString &text)
{
    Q_D(QTextControl);
    Qt::TextFormat f = d->textFormat;
    if (f == Qt::AutoText) {
        if (Qt::mightBeRichText(text))
            f = Qt::RichText;
        else
            f = Qt::PlainText;
    }

    QTextCursor cursor(d->doc);
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);

    if (!d->doc->isEmpty())
        cursor.insertBlock(d->cursor.blockFormat(), d->cursor.charFormat());

    // preserve the char format
    QTextCharFormat oldCharFormat = d->cursor.charFormat();
    if (f == Qt::PlainText) {
        cursor.insertText(text);
    } else {
        cursor.insertFragment(QTextDocumentFragment::fromHtml(text));
    }
    if (!d->cursor.hasSelection())
        d->cursor.setCharFormat(oldCharFormat);

    cursor.endEditBlock();
}

/*
    Ensures that the cursor is visible by scrolling the text edit if
    necessary.
*/
void QTextControl::ensureCursorVisible()
{
    Q_D(QTextControl);
    QRectF crect = d->rectForPosition(d->cursor.position());
    emit visibilityRequest(crect);
// ####    updateMicroFocus();
}


/*
    \enum QTextControl::KeyboardAction

    \compat

    \value ActionBackspace
    \value ActionDelete
    \value ActionReturn
    \value ActionKill
    \value ActionWordBackspace
    \value ActionWordDelete
*/

/*
    \fn bool QTextControl::find(const QString &exp, bool cs, bool wo)

    Use the find() overload that takes a QTextDocument::FindFlags
    argument.
*/

/*
    \fn void QTextControl::sync()

    Does nothing.
*/

/*
    \fn void QTextControl::setBold(bool b)

    Use setFontWeight() instead.
*/

/*
    \fn void QTextControl::setUnderline(bool b)

    Use setFontUnderline() instead.
*/

/*
    \fn void QTextControl::setItalic(bool i)

    Use setFontItalic() instead.
*/

/*
    \fn void QTextControl::setFamily(const QString &family)

    Use setFontFamily() instead.
*/

/*
    \fn void QTextControl::setPointSize(int size)

    Use setFontPointSize() instead.
*/

/*
    \fn bool QTextControl::italic() const

    Use fontItalic() instead.
*/

/*
    \fn bool QTextControl::bold() const

    Use fontWeight() >= QFont::Bold instead.
*/

/*
    \fn bool QTextControl::underline() const

    Use fontUnderline() instead.
*/

/*
    \fn QString QTextControl::family() const

    Use fontFamily() instead.
*/

/*
    \fn int QTextControl::pointSize() const

    Use int(fontPointSize()+0.5) instead.
*/

/*
    \fn bool QTextControl::hasSelectedText() const

    Use textCursor().hasSelection() instead.
*/

/*
    \fn QString QTextControl::selectedText() const

    Use textCursor().selectedText() instead.
*/

/*
    \fn bool QTextControl::isUndoAvailable() const

    Use document()->isUndoAvailable() instead.
*/

/*
    \fn bool QTextControl::isRedoAvailable() const

    Use document()->isRedoAvailable() instead.
*/

/*
    \fn void QTextControl::insert(const QString &text)

    Use insertPlainText() instead.
*/

/*
    \fn bool QTextControl::isModified() const

    Use document()->isModified() instead.
*/

/*
    \fn QColor QTextControl::color() const

    Use textColor() instead.
*/

/*
    \fn void QTextControl::textChanged()

    This signal is emitted whenever the document's content changes; for
    example, when text is inserted or deleted, or when formatting is applied.
*/

/*
    \fn void QTextControl::undoAvailable(bool available)

    This signal is emitted whenever undo operations become available
    (\a available is true) or unavailable (\a available is false).
*/

/*
    \fn void QTextControl::redoAvailable(bool available)

    This signal is emitted whenever redo operations become available
    (\a available is true) or unavailable (\a available is false).
*/

/*
    \fn void QTextControl::currentFontChanged(const QFont &font)

    Use currentCharFormatChanged() instead.
*/

/*
    \fn void QTextControl::currentColorChanged(const QColor &color)

    Use currentCharFormatChanged() instead.
*/

/*
    \fn void QTextControl::setModified(bool m)

    Use document->setModified() instead.
*/

/*
    \fn void QTextControl::setColor(const QColor &color)

    Use setTextColor() instead.
*/

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
    ctx.palette = d->palette;
    if (d->cursorOn /* ####### && q->isEnabled()*/)
        ctx.cursorPosition = d->cursor.position();
    if (!d->dndFeedbackCursor.isNull())
        ctx.cursorPosition = d->dndFeedbackCursor.position();
    if (d->cursor.hasSelection()) {
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = d->cursor;
        selection.format.setBackground(ctx.palette.brush(QPalette::Highlight));
        selection.format.setForeground(ctx.palette.brush(QPalette::HighlightedText));
        ctx.selections.append(selection);
    }
    if (d->focusIndicator.hasSelection()) {
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = d->focusIndicator;
        QPen outline(ctx.palette.color(QPalette::Text), 1, Qt::DotLine);
        selection.format.setProperty(QTextFormat::OutlinePen, outline);
        ctx.selections.append(selection);
    }

    d->doc->documentLayout()->draw(p, ctx);
    p->restore();
}

#endif // QT_NO_TEXTCONTROL

#include "moc_qtextcontrol_p.cpp"
