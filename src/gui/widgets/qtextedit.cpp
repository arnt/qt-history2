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

#include "qtextedit.h"
#include "qtextedit_p.h"

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
#include "private/qtextdocument_p.h"
#include "qtextdocument.h"
#include "qtextlist.h"

#include <qdatetime.h>
#include <qapplication.h>
#include <limits.h>
#include <qtexttable.h>
#include <qvariant.h>

#ifdef Q_WS_X11
#include <qinputcontext.h>
#endif

#ifndef QT_NO_ACCEL
#include <qkeysequence.h>
#define ACCEL_KEY(k) "\t" + QString(QKeySequence( Qt::CTRL | Qt::Key_ ## k ))
#else
#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)
#endif

static bool dataHasText(const QMimeData *data)
{
    return data->hasText()
        || data->hasHtml()
        || data->hasFormat("application/x-qrichtext")
        || data->hasFormat("application/x-qt-richtext");
}

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
    return layout->findLine(relativePos);
}

bool QTextEditPrivate::cursorMoveKeyEvent(QKeyEvent *e)
{
    Q_Q(QTextEdit);

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
#endif
    switch (e->key()) {
#ifndef Q_WS_MAC  // Use the default Windows bindings.
        case Qt::Key_Up:
            op = QTextCursor::Up;
            break;
        case Qt::Key_Down:
            op = QTextCursor::Down;
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
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                    op = QTextCursor::End;
                else if (e->modifiers() & Qt::AltModifier)
                    op = QTextCursor::EndOfBlock;
                else
                    op = QTextCursor::Down;
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
            return pageDown(mode);
        case Qt::Key_PageUp:
            return pageUp(mode);
    default:
        return false;
    }

    const bool moved = cursor.movePosition(op, mode);
    q->ensureCursorVisible();

    if (moved) {
        emit q->cursorPositionChanged();
        q->updateMicroFocus();
    }

    selectionChanged();

    viewport->update();

    return true;
}

void QTextEditPrivate::updateCurrentCharFormat()
{
    Q_Q(QTextEdit);

    QTextCharFormat fmt = cursor.charFormat();
    if (fmt == lastCharFormat)
        return;
    lastCharFormat = fmt;

    emit q->currentCharFormatChanged(fmt);
#ifdef QT_COMPAT
    // compat signals
    emit q->currentFontChanged(fmt.font());
    emit q->currentColorChanged(fmt.textColor());
#endif
    q->updateMicroFocus();
}

void QTextEditPrivate::indent()
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

void QTextEditPrivate::outdent()
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

void QTextEditPrivate::gotoNextTableCell()
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

void QTextEditPrivate::gotoPreviousTableCell()
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

void QTextEditPrivate::createAutoBulletList()
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

void QTextEditPrivate::init(const QTextDocumentFragment &fragment, QTextDocument *document)
{
    Q_Q(QTextEdit);

    bool clearDocument = true;
    if (!doc) {
        if (document) {
            doc = document;
            clearDocument = false;
        } else {
            doc = new QTextDocument(q);
        }

        QObject::connect(doc->documentLayout(), SIGNAL(update(QRect)), q, SLOT(update(QRect)));
        QObject::connect(doc->documentLayout(), SIGNAL(usedSizeChanged()), q, SLOT(adjustScrollbars()));
        cursor = QTextCursor(doc);

        doc->documentLayout()->setDefaultFont(q->font());

        hbar->setSingleStep(20);
        vbar->setSingleStep(20);

        QObject::connect(doc, SIGNAL(contentsChanged()), q, SLOT(updateCurrentCharFormatAndSelection()));
        QObject::connect(doc, SIGNAL(cursorPositionChanged(const QTextCursor &)), q, SLOT(emitCursorPosChanged(const QTextCursor &)));

        // compat signals
        QObject::connect(doc, SIGNAL(contentsChanged()), q, SIGNAL(textChanged()));
        QObject::connect(doc, SIGNAL(undoAvailable(bool)), q, SIGNAL(undoAvailable(bool)));
        QObject::connect(doc, SIGNAL(redoAvailable(bool)), q, SIGNAL(redoAvailable(bool)));

        cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, q);

        viewport->setBackgroundRole(QPalette::Base);
        viewport->setAcceptDrops(true);
        viewport->setFocusProxy(q);
        q->setFocusPolicy(Qt::WheelFocus);
    }

    doc->setUndoRedoEnabled(false);

    q->setAttribute(Qt::WA_InputMethodEnabled);

    if (clearDocument) {
        q->clear();

        QTextCharFormat fmt;
        fmt.setFont(q->font());
        fmt.setTextColor(q->palette().color(QPalette::Text));
        cursor.movePosition(QTextCursor::Start);
        cursor.setBlockCharFormat(fmt);
    }

    viewport->setCursor(readOnly ? Qt::ArrowCursor : Qt::IbeamCursor);

    QTextFrame *rootFrame = doc->rootFrame();
    QTextFrameFormat ffmt = rootFrame->format();
    ffmt.setMargin(4);
    rootFrame->setFormat(ffmt);

    if (!fragment.isEmpty()) {
        cursor.movePosition(QTextCursor::Start);
        cursor.insertFragment(fragment);
    }

    doc->setUndoRedoEnabled(!q->isReadOnly());
    cursor.movePosition(QTextCursor::Start);
    updateCurrentCharFormatAndSelection();

    doc->setModified(false);

    anchorToScrollToWhenVisible = QString::null;
}

void QTextEditPrivate::startDrag()
{
    Q_Q(QTextEdit);
    mousePressed = false;
    QMimeData *data = q->createMimeDataFromSelection();

    QDrag *drag = new QDrag(q);
    drag->setMimeData(data);

    QDrag::DropActions actions = readOnly ? QDrag::CopyAction : QDrag::MoveAction;
    QDrag::DropAction action = drag->start(actions);

    if (action == QDrag::MoveAction && drag->target() != q)
        cursor.removeSelectedText();
}

void QTextEditPrivate::setCursorPosition(const QPoint &pos)
{
    const int cursorPos = doc->documentLayout()->hitTest(translateCoordinates(pos), Qt::FuzzyHit);
    if (cursorPos == -1)
        return;
    cursor.setPosition(cursorPos);
}

void QTextEditPrivate::setCursorPosition(int pos, QTextCursor::MoveMode mode)
{
    Q_Q(QTextEdit);
    cursor.setPosition(pos, mode);
    q->ensureCursorVisible();
}

void QTextEditPrivate::update(const QRect &contentsRect)
{
    const int xOffset = hbar->value();
    const int yOffset = vbar->value();
    const QRect visibleRect(xOffset, yOffset, viewport->width(), viewport->height());

    QRect r = contentsRect.intersect(visibleRect);
    if (r.isEmpty())
        return;

    r.translate(-xOffset, -yOffset);
    viewport->update(r);
}

void QTextEditPrivate::selectionChanged()
{
    Q_Q(QTextEdit);
    bool current = cursor.hasSelection();
    if (current == lastSelectionState)
        return;

    lastSelectionState = current;
    emit q->copyAvailable(current);
    emit q->selectionChanged();
    q->updateMicroFocus();
}

bool QTextEditPrivate::pageUp(QTextCursor::MoveMode moveMode)
{
    Q_Q(QTextEdit);
    int targetY = vbar->value() - viewport->height();
    bool moved = false;
    do {
        q->ensureCursorVisible();
        moved = cursor.movePosition(QTextCursor::Up, moveMode);
    } while (moved && vbar->value() > targetY);

    if (moved) {
        emit q->cursorPositionChanged();
        q->updateMicroFocus();
    }
    return moved;
}

bool QTextEditPrivate::pageDown(QTextCursor::MoveMode moveMode)
{
    Q_Q(QTextEdit);
    int targetY = vbar->value() + viewport->height();
    bool moved = false;
    do {
        q->ensureCursorVisible();
        moved = cursor.movePosition(QTextCursor::Down, moveMode);
    } while (moved && vbar->value() < targetY);

    if (moved) {
        emit q->cursorPositionChanged();
        q->updateMicroFocus();
    }
    return moved;
}

void QTextEditPrivate::updateCurrentCharFormatAndSelection()
{
    updateCurrentCharFormat();
    selectionChanged();
}

void QTextEditPrivate::adjustScrollbars()
{
    if (ignoreAutomaticScrollbarAdjustement)
        return;

    QAbstractTextDocumentLayout *layout = doc->documentLayout();

    const QSize viewportSize = viewport->size();
    const QSize docSize = layout->sizeUsed();

    hbar->setRange(0, docSize.width() - viewportSize.width());
    hbar->setPageStep(viewportSize.width());

    vbar->setRange(0, docSize.height() - viewportSize.height());
    vbar->setPageStep(viewportSize.height());
}

void QTextEditPrivate::setClipboardSelection()
{
    if (!cursor.hasSelection())
        return;
    Q_Q(QTextEdit);
    QMimeData *data = q->createMimeDataFromSelection();
    QApplication::clipboard()->setMimeData(data, QClipboard::Selection);
}

void QTextEditPrivate::ensureVisible(int documentPosition)
{
    // don't check for the visibility of the vertical scrollbar here,
    // always scroll to the position. we might have a layoutChildren
    // in QViewport pending, which will make things visible later on
    // then, for example when initially showing the widget and right
    // after that calling scrollToAnchor, which calls us. the vbar
    // isn't visible then, but that's okay.

    QTextBlock block = doc->findBlock(documentPosition);
    QTextLayout *layout = block.layout();
    QPointF layoutPos = layout->position();
    const int relativePos = documentPosition - block.position();
    QTextLine line = layout->findLine(relativePos);
    if (!line.isValid())
        return;

    const int y = qRound(layoutPos.y() + line.y());
    vbar->setValue(y);
}

// QRect QTextEditPrivate::cursorRect() const
// {
//     QTextFrame *frame = d->cursor.currentFrame();
//     QRect r = d->cursor.block().layout()->rect();
//     r.translate(d->doc->documentLayout()->frameBoundingRect(frame).topLeft());
//     return r;
// }

void QTextEditPrivate::emitCursorPosChanged(const QTextCursor &someCursor)
{
    Q_Q(QTextEdit);
    if (someCursor.isCopyOf(cursor)) {
        emit q->cursorPositionChanged();
        q->updateMicroFocus();
    }
}

void QTextEditPrivate::setBlinkingCursorEnabled(bool enable)
{
    Q_Q(QTextEdit);
    if (enable)
        cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, q);
    else
        cursorBlinkTimer.stop();
    update(cursorRect());
}

/*!
    \class QTextEdit
    \brief The QTextEdit class provides a widget that is used to edit and display
    both plain and rich text.

    \ingroup text
    \mainclass

    \tableofcontents

    \section1 Introduction and Concepts

    QTextEdit is an advanced WYSIWYG viewer/editor supporting rich
    text formatting using HTML-style tags. It is optimized to handle
    large documents and to respond quickly to user input.

    QTextEdit works on paragraphs and characters. A paragraph is a
    formatted string which is word-wrapped to fit into the width of
    the widget. By default when reading plain text, one newline
    signifies a paragraph. A document consists of zero or more
    paragraphs. The words in the paragraph are aligned in accordance
    with the paragraph's alignment. Paragraphs are separated by hard
    line breaks. Each character within a paragraph has its own
    attributes, for example, font and color.

    QTextEdit can display images, lists and tables. If the text is
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

    \section1 Using QTextEdit as a Display Widget

    QTextEdit can display a large HTML subset, including tables and
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
    the text edit widget. The setWordWrap() function is used to
    specify the kind of word wrap you want, or \c NoWrap if you don't
    want any wrapping. Call setWordWrap() to set a fixed pixel width
    \c FixedPixelWidth, or character column (e.g. 80 column) \c
    FixedColumnWidth with the pixels or columns specified with
    setWrapColumnOrWidth(). If you use word wrap to the widget's width
    \c WidgetWidth, you can specify whether to break on whitespace or
    anywhere with setWrapPolicy().

    The find() function can be used to find and select a given string
    within the text.

    \section2 Read-only key bindings

    When QTextEdit is used read-only the key-bindings are limited to
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
    \row \i Shift+Wheel
         \i Scrolls the page horizontally (the Wheel is the mouse wheel).
    \row \i Ctrl+Wheel        \i Zooms the text.
    \endtable

    The text edit may be able to provide some meta-information. For
    example, the documentTitle() function will return the text from
    within HTML \c{<title>} tags.

    \section1 Using QTextEdit as an Editor

    All the information about using QTextEdit as a display widget also
    applies here.

    The current char format's attributes are set with setFontItalic(),
    setFontBold(), setFontUnderline(), setFontFamily(),
    setFontPointSize(), setTextColor() and setCurrentFont(). The current
    paragraph's alignment is set with setAlignment().

    Selection of text is handled by the QTextCursor class, which provides
    functionality for creating selections, retrieving the text contents or
    deleting selections. You can retrieve the object that corresponds with
    the user-visible cursor using the textCursor() method. If you want to set
    a selection in QTextEdit just create one on a QTextCursor object and
    then make that cursor the visible cursor using setCursor(). The selection
    can be copied to the clipboard with copy(), or cut to the clipboard with
    cut(). The entire text can be selected using selectAll().

    When the cursor is moved and the underlying formatting attributes change,
    the currentCharFormatChanged() signal is emitted to reflect the new attributes
    at the new cursor position.

    QTextEdit holds a QTextDocument object which can be retrieved using the
    document() method. You can also set your own document object using setDocument().
    QTextDocument emits a textChanged() signal if the text changes and it also
    provides a isModified() function which will return true if the text has been
    modified since it was either loaded or since the last call to setModified
    with false as argument. In addition it provides methods for undo and redo.

    \section2 Editing key bindings

    The list of key-bindings which are implemented for editing:
    \table
    \header \i Keypresses \i Action
    \row \i Backspace \i Deletes the character to the left of the cursor.
    \row \i Delete \i Deletes the character to the right of the cursor.
    \row \i Ctrl+A \i Moves the cursor to the beginning of the line.
    \row \i Ctrl+B \i Moves the cursor one character to the left.
    \row \i Ctrl+C \i Copy the selected text to the clipboard.
    \row \i Ctrl+Insert \i Copy the selected text to the clipboard.
    \row \i Ctrl+D \i Delete the character to the right of the cursor.
    \row \i Ctrl+E \i Moves the cursor to the end of the line.
    \row \i Ctrl+F \i Moves the cursor one character to the right.
    \row \i Ctrl+H \i Deletes the character to the left of the cursor.
    \row \i Ctrl+K \i Deletes to the end of the line.
    \row \i Ctrl+N \i Moves the cursor one line down.
    \row \i Ctrl+P \i Moves the cursor one line up.
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
    \row \i Shift+Wheel \i Scrolls the page horizontally (the Wheel is the mouse wheel).
    \row \i Ctrl+Wheel \i Zooms the text.
    \endtable

    To select (mark) text hold down the Shift key whilst pressing one
    of the movement keystrokes, for example, \e{Shift+Right Arrow}
    will select the character to the right, and \e{Shift+Ctrl+Right
    Arrow} will select the word to the right, etc.

    \sa QTextDocument QTextCursor document() textCursor() setDocument() setTextCursor()

*/

/*!
    \property QTextEdit::undoRedoEnabled
    \brief whether undo and redo are enabled

    Users are only able to undo or redo actions if this property is
    true, and if there is an action that can be undone (or redone).
*/

/*!
    \enum QTextEdit::WordWrap

    \value NoWrap
    \value WidgetWidth
    \value FixedPixelWidth
    \value FixedColumnWidth
*/

/*!
    \enum QTextEdit::AutoFormattingFlag

    \value AutoNone Don't do any automatic formatting.
    \value AutoBulletList Automatically create bullet lists (e.g. when
    the user enters an asterisk ('*') in the left most column, or
    presses Enter in an existing list item.
    \value AutoAll Apply all automatic formatting. Currently only
    automatic bullet lists are supported.
*/

/*!
    \enum QTextEdit::CursorAction

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

/*!
    Constructs an empty QTextEdit with parent \a
    parent.
*/
QTextEdit::QTextEdit(QWidget *parent)
    : QViewport(*new QTextEditPrivate, parent)
{
    Q_D(QTextEdit);
    d->init();
}

/*!
    \internal
*/
QTextEdit::QTextEdit(QTextEditPrivate &dd, QWidget *parent)
    : QViewport(dd, parent)
{
    Q_D(QTextEdit);
    d->init();
}

/*!
    Constructs a QTextEdit with parent \a parent. The text edit will display
    the text \a text. The text is interpreted as html.
*/
QTextEdit::QTextEdit(const QString &text, QWidget *parent)
    : QViewport(*new QTextEditPrivate, parent)
{
    Q_D(QTextEdit);
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(text);
    d->init(fragment);
}

#ifdef QT_COMPAT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QTextEdit::QTextEdit(QWidget *parent, const char *name)
    : QViewport(*new QTextEditPrivate, parent)
{
    Q_D(QTextEdit);
    d->init();
    setObjectName(name);
}
#endif


/*!
    Destructor.
*/
QTextEdit::~QTextEdit()
{
}

/*!
    Returns the point size of the font of the current format.

    \sa setFontFamily() setCurrentFont() setFontPointSize()
*/
float QTextEdit::fontPointSize() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat().fontPointSize();
}

/*!
    Returns the font family of the current format.

    \sa setFontFamily() setCurrentFont() setFontPointSize()
*/
QString QTextEdit::fontFamily() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat().fontFamily();
}

/*!
    Returns the font weight of the current format.

    \sa setFontWeight() setCurrentFont() setFontPointSize()
*/
int QTextEdit::fontWeight() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat().fontWeight();
}

/*!
    Returns true if the font of the current format is underlined; otherwise returns
    false.

    \sa setFontUnderline()
*/
bool QTextEdit::fontUnderline() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat().fontUnderline();
}

/*!
    Returns true if the font of the current format is italic; otherwise returns
    false.

    \sa setFontItalic()
*/
bool QTextEdit::fontItalic() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat().fontItalic();
}

/*!
    Returns the text color of the current format.

    \sa setTextColor()
*/
QColor QTextEdit::textColor() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat().textColor();
}

/*!
    Returns the font of the current format.

    \sa setCurrentFont() setFontFamily() setFontPointSize()
*/
QFont QTextEdit::currentFont() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat().font();
}

/*!
    Sets the alignment of the current paragraph to \a a. Valid
    alignments are \c Qt::AlignLeft, \c Qt::AlignRight,
    \c Qt::AlignJustify and \c Qt::AlignCenter (which centers
    horizontally).
*/
void QTextEdit::setAlignment(Qt::Alignment a)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextBlockFormat fmt;
    fmt.setAlignment(a);
    d->cursor.mergeBlockFormat(fmt);
    updateMicroFocus();
}

/*!
    Returns the alignment of the current paragraph.

    \sa setAlignment()
*/
Qt::Alignment QTextEdit::alignment() const
{
    Q_D(const QTextEdit);
    return d->cursor.blockFormat().alignment();
}

/*!
    Makes \a document the new document of the text editor.

    The parent QObject of the provided document remains the owner
    of the object. If the current document is a child of the text
    editor, then it is deleted.

    \sa document()
*/
void QTextEdit::setDocument(QTextDocument *document)
{
    Q_D(QTextEdit);
    d->doc->disconnect(this);
    d->doc->documentLayout()->disconnect(this);

    if (d->doc->parent() == this)
        delete d->doc;

    d->doc = 0;
    d->init(QTextDocumentFragment(), document);
}

/*!
    Returns a pointer to the underlying document.

    \sa setDocument()
*/
QTextDocument *QTextEdit::document() const
{
    Q_D(const QTextEdit);
    return d->doc;
}

/*!
    Sets the visible \a cursor.
*/
void QTextEdit::setTextCursor(const QTextCursor &cursor)
{
    Q_D(QTextEdit);
    d->cursor = cursor;
    d->updateCurrentCharFormatAndSelection();
    ensureCursorVisible();
    d->viewport->update();
}

/*!
    Returns a QTextCursor that represents the currently visible cursor.
 */
QTextCursor QTextEdit::textCursor() const
{
    Q_D(const QTextEdit);
    return d->cursor;
}

/*!
    Sets the font family of the current format to \a fontFamily.

    \sa fontFamily() setCurrentFont()
*/
void QTextEdit::setFontFamily(const QString &fontFamily)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontFamily(fontFamily);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the point size of the current format to \a s.

    Note that if \a s is zero or negative, the behavior of this
    function is not defined.

    \sa fontPointSize() setCurrentFont() setFontFamily()
*/
void QTextEdit::setFontPointSize(float s)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontPointSize(s);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the font weight of the current format to \a w.

    \sa fontWeight() setCurrentFont() setFontFamily()
*/
void QTextEdit::setFontWeight(int w)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontWeight(w);
    mergeCurrentCharFormat(fmt);
}

/*!
    If \a b is true sets the current format to underline; otherwise
    sets the current format to non-underline.

    \sa fontUnderline()
*/
void QTextEdit::setFontUnderline(bool b)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontUnderline(b);
    mergeCurrentCharFormat(fmt);
}

/*!
    If \a b is true sets the current format to italic; otherwise sets
    the current format to non-italic.

    \sa fontItalic()
*/
void QTextEdit::setFontItalic(bool b)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontItalic(b);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the text color of the current format to \a c.

    \sa textColor()
*/
void QTextEdit::setTextColor(const QColor &c)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setTextColor(c);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the font of the current format to \a f.

    \sa currentFont() setFontPointSize() setFontFamily()
*/
void QTextEdit::setCurrentFont(const QFont &f)
{
    Q_D(QTextEdit);
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFont(f);
    mergeCurrentCharFormat(fmt);
}

/*!
    \fn void QTextEdit::undo() const

    Undoes the last operation.

    If there is no operation to undo, i.e. there is no undo step in
    the undo/redo history, nothing happens.

    \sa redo()
*/

/*!
    \fn void QTextEdit::redo() const

    Redoes the last operation.

    If there is no operation to redo, i.e. there is no redo step in
    the undo/redo history, nothing happens.

    \sa undo()
*/

/*!
    Copies the selected text to the clipboard and deletes it from
    the text edit.

    If there is no selected text nothing happens.

    \sa copy() paste()
*/

void QTextEdit::cut()
{
    Q_D(QTextEdit);
    if (d->readOnly || !d->cursor.hasSelection())
	return;
    copy();
    d->cursor.removeSelectedText();
}

/*!
    Copies any selected text to the clipboard.

    \sa copyAvailable()
*/

void QTextEdit::copy()
{
    Q_D(QTextEdit);
    if (!d->cursor.hasSelection())
	return;
    QMimeData *data = createMimeDataFromSelection();
    QApplication::clipboard()->setMimeData(data);
}

/*!
    Pastes the text from the clipboard into the text edit at the
    current cursor position.

    If there is no text in the clipboard nothing happens.

    \sa cut() copy()
*/

void QTextEdit::paste()
{
    insertFromMimeData(QApplication::clipboard()->mimeData());
}

/*!
    Deletes all the text in the text edit.

    \sa cut() removeSelectedText() setPlainText() setHtml()
*/
void QTextEdit::clear()
{
    Q_D(QTextEdit);
    selectAll();
    d->cursor.removeSelectedText();
}


/*!
    Selects all text.

    \sa copy() cut() textCursor()
 */
void QTextEdit::selectAll()
{
    Q_D(QTextEdit);
    d->cursor.movePosition(QTextCursor::Start);
    d->cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    d->selectionChanged();
    d->viewport->update();
    d->setClipboardSelection();
}

/*! \internal
*/

void QTextEdit::timerEvent(QTimerEvent *ev)
{
    Q_D(QTextEdit);
    if (ev->timerId() == d->cursorBlinkTimer.timerId()) {
        d->cursorOn = !d->cursorOn;

        if (d->cursor.hasSelection())
            d->cursorOn &= (style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, 0, this)
                            != 0);

        d->update(d->cursorRect());
    } else if (ev->timerId() == d->dragStartTimer.timerId()) {
        d->dragStartTimer.stop();
        d->startDrag();
    } else if (ev->timerId() == d->trippleClickTimer.timerId()) {
        d->trippleClickTimer.stop();
    }
}

/*!
    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as plain text.

    Note that the undo/redo history is cleared by this function.

    \sa toPlainText()
*/

void QTextEdit::setPlainText(const QString &text)
{
    Q_D(QTextEdit);
    QTextDocumentFragment fragment = QTextDocumentFragment::fromPlainText(text);
    d->init(fragment);
    d->preferRichText = false;
}

/*!
    \fn QString QTextEdit::toPlainText() const

    Returns the text of the text edit as plain text.

    \sa setPlainText
 */
/*!
    \fn QString QTextEdit::toHtml() const

    Returns the text of the text edit as html.

    \sa setHtml
 */

/*!
    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as rich text in html format.

    Note that the undo/redo history is cleared by this function.
*/

void QTextEdit::setHtml(const QString &text)
{
    Q_D(QTextEdit);
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(text);
    d->init(fragment);
    d->preferRichText = true;
}

/*! \reimp
*/
void QTextEdit::keyPressEvent(QKeyEvent *e)
{
    Q_D(QTextEdit);
    if (d->readOnly) {
        switch (e->key()) {
            case Qt::Key_Home:
                d->vbar->triggerAction(QAbstractSlider::SliderToMinimum);
                break;
            case Qt::Key_End:
                d->vbar->triggerAction(QAbstractSlider::SliderToMaximum);
                break;
            case Qt::Key_Space:
                if (e->modifiers() & Qt::ShiftModifier)
                    d->vbar->triggerAction(QAbstractSlider::SliderPageStepSub);
                else
                    d->vbar->triggerAction(QAbstractSlider::SliderPageStepAdd);
            default:
                QViewport::keyPressEvent(e);
                break;
        }
        return;
    }

    // schedule a repaint of the region of the cursor, as when we move it we
    // want to make sure the old cursor disappears (not noticable when moving
    // only a few pixels but noticable when jumping between cells in tables for
    // example)
    d->update(d->cursorRect());

    if (d->cursorMoveKeyEvent(e))
        goto accept;

    if (e->modifiers() & Qt::ControlModifier) {
        switch( e->key() ) {
        case Qt::Key_Z:
            d->doc->undo();
            break;
        case Qt::Key_Y:
            d->doc->redo();
            break;
        case Qt::Key_X:
        case Qt::Key_F20:  // Cut key on Sun keyboards
            cut();
            break;
        case Qt::Key_C:
        case Qt::Key_Insert:
        case Qt::Key_F16: // Copy key on Sun keyboards
            copy();
            break;
        case Qt::Key_V:
        case Qt::Key_F18:  // Paste key on Sun keyboards
            paste();
            break;
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
            e->ignore();
            return;
        }
        goto accept;
    }

process:
    switch( e->key() ) {
    case Qt::Key_Backspace: {
        QTextList *list = d->cursor.currentList();
        if (list && d->cursor.atBlockStart())
            list->remove(d->cursor.block());
        else
            d->cursor.deletePreviousChar();
        break;
    }
    case Qt::Key_Delete:
        if (e->modifiers() & Qt::ShiftModifier)
            cut();
	else
            d->cursor.deleteChar();
        break;
    case Qt::Key_Insert:
        if (e->modifiers() & Qt::ShiftModifier)
            paste();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        d->cursor.insertBlock();
        break;
    default:
        {
            QString text = e->text();

            if (e->key() == Qt::Key_Tab) {
                if (d->tabChangesFocus) {
                    e->ignore();
                    return;
                }
                if (d->cursor.currentTable()) {
                    d->gotoNextTableCell();
                    break;
                } else if (d->cursor.atBlockStart()) {
                    d->indent();
                    break;
                }
            }

            if (e->key() == Qt::Key_Backtab) {
                if (d->tabChangesFocus) {
                    e->ignore();
                    return;
                }
                if (d->cursor.currentTable()) {
                    d->gotoPreviousTableCell();
                    break;
                } else if (d->cursor.atBlockStart()) {
                    d->outdent();
                    break;
                }
            }

            if (d->cursor.atBlockStart()
                && (d->autoFormatting & AutoBulletList)
                && (!text.isEmpty())
                && (text[0] == '-' || text[0] == '*')
                && (!d->cursor.currentList())) {

                text.remove(0, 1);
                d->createAutoBulletList();
            }

            if (!text.isEmpty() && text.at(0).isPrint()) {
                d->cursor.insertText(text);
                d->selectionChanged();
            } else {
                QViewport::keyPressEvent(e);
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

/*!
    Returns how many pixels high the text edit needs to be to display
    all the text if the text edit is \a width pixels wide.
*/
int QTextEdit::heightForWidth(int width) const
{
    Q_D(const QTextEdit);
    QAbstractTextDocumentLayout *layout = d->doc->documentLayout();
    const int oldWidth = layout->sizeUsed().width();
    layout->setPageSize(QSize(width, INT_MAX));
    const int height = layout->sizeUsed().height();
    layout->setPageSize(QSize(oldWidth, INT_MAX));
    return height;
}

/*! \reimp
*/
void QTextEdit::resizeEvent(QResizeEvent *)
{
    Q_D(QTextEdit);
    QAbstractTextDocumentLayout *layout = d->doc->documentLayout();

    if (QTextDocumentLayout *tlayout = qt_cast<QTextDocumentLayout *>(layout)) {
        if (d->wordWrap == NoWrap)
            tlayout->setBlockTextFlags(tlayout->blockTextFlags() | Qt::TextSingleLine);
        else
            tlayout->setBlockTextFlags(tlayout->blockTextFlags() & ~Qt::TextSingleLine);

        if (d->wordWrap == FixedColumnWidth)
            tlayout->setFixedColumnWidth(d->wrapColumnOrWidth);
        else
            tlayout->setFixedColumnWidth(-1);
    }

    int width = d->viewport->width();
    if (d->wordWrap == FixedPixelWidth)
        width = d->wrapColumnOrWidth;

    const QSize lastUsedSize = layout->sizeUsed();

    // ignore calls to adjustScrollbars caused by an emission of the
    // usedSizeChanged() signal in the layout, as we're calling it
    // later on our own anyway (or deliberately not) .
    d->ignoreAutomaticScrollbarAdjustement = true;

    layout->setPageSize(QSize(width, INT_MAX));

    d->ignoreAutomaticScrollbarAdjustement = false;

    QSize usedSize = layout->sizeUsed();

    // this is an obscure situation in the layout that can happen:
    // if a character at the end of a line is the tallest one and therefore
    // influencing the total height of the line and the line right below it
    // is always taller though, then it can happen that if due to line breaking
    // that tall character wraps into the lower line the document not only shrinks
    // horizontally (causing the character to wrap in the first place) but also
    // vertically, because the original line is now smaller and the one below kept
    // its size. So a layout with less width _can_ take up less vertical space, too.
    // If the wider case causes a vertical scrollbar to appear and the narrower one
    // (narrower because the vertical scrollbar takes up horizontal space)) to disappear
    // again then we have an endless loop, as adjustScrollBars sets new ranges on the
    // scrollbars, the QViewport will find out about it and try to show/hide the scrollbars
    // again. That's why we try to detect this case here and break out.
    //
    // (if you change this please also check the layoutingLoop() testcase in
    // QTextEdit's autotests)
    if (lastUsedSize.isValid()
        && d->vbar->isShown()
        && d->viewport->width() < lastUsedSize.width()
        && usedSize.height() < lastUsedSize.height()
        && usedSize.height() <= d->viewport->height())
        return;

    d->adjustScrollbars();
}

/*! \reimp
*/
void QTextEdit::paintEvent(QPaintEvent *ev)
{
    Q_D(QTextEdit);
    QPainter p(d->viewport);

    const int xOffset = d->hbar->value();
    const int yOffset = d->vbar->value();

    QRect r = ev->rect();
    p.translate(-xOffset, -yOffset);
    r.translate(xOffset, yOffset);
    p.setClipRect(r);

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.showCursor = (d->cursorOn && d->viewport->hasFocus() && isEnabled());
    ctx.cursor = d->cursor;
    ctx.palette = palette();
    ctx.rect = r;
    ctx.imStart = d->imstart;
    ctx.imEnd = d->imend;
    ctx.imSelectionStart = d->imselstart;
    ctx.imSelectionEnd = d->imselend;
    ctx.focusIndicator = d->focusIndicator;

    d->doc->documentLayout()->draw(&p, ctx);
}

/*! \reimp
*/
void QTextEdit::mousePressEvent(QMouseEvent *ev)
{
    Q_D(QTextEdit);
    if (!(ev->button() & Qt::LeftButton))
        return;

    const QPoint pos = d->translateCoordinates(ev->pos());

    d->mousePressed = true;
    d->mightStartDrag = false;

    if (d->trippleClickTimer.isActive()
        && ((ev->globalPos() - d->trippleClickPoint).manhattanLength() < QApplication::startDragDistance())) {

        d->cursor.movePosition(QTextCursor::StartOfBlock);
        d->cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        d->trippleClickTimer.stop();
    } else {
        int cursorPos = d->doc->documentLayout()->hitTest(pos, Qt::FuzzyHit);
        if (cursorPos != -1) {

            if (d->cursor.hasSelection()
                && cursorPos >= d->cursor.selectionStart()
                && cursorPos <= d->cursor.selectionEnd()) {
                d->mightStartDrag = true;
                d->dragStartPos = ev->globalPos();
                d->dragStartTimer.start(QApplication::startDragTime(), this);
                return;
            }

#ifdef Q_WS_X11
            if (d->imstart != d->imend) {
                inputContext()->mouseHandler(cursorPos - d->imstart, ev);
                if (d->imstart != d->imend)
                    return;
            }
#endif
            d->setCursorPosition(cursorPos);
        }

        d->cursor.clearSelection();
    }

    d->updateCurrentCharFormatAndSelection();
    d->viewport->update();
}

/*! \reimp
*/
void QTextEdit::mouseMoveEvent(QMouseEvent *ev)
{
    Q_D(QTextEdit);
    if (!(ev->buttons() & Qt::LeftButton)
        || !d->mousePressed)
        return;

    if (d->mightStartDrag) {
        d->dragStartTimer.stop();

        if ((ev->globalPos() - d->dragStartPos).manhattanLength() > QApplication::startDragDistance())
            d->startDrag();

        return;
    }

    int cursorPos = d->doc->documentLayout()->hitTest(d->translateCoordinates(ev->pos()), Qt::FuzzyHit);
    if (cursorPos == -1)
        return;

    d->setCursorPosition(cursorPos, QTextCursor::KeepAnchor);
    d->updateCurrentCharFormatAndSelection();
    d->viewport->update();
}

/*! \reimp
*/
void QTextEdit::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_D(QTextEdit);
    if (d->mightStartDrag) {
        d->mousePressed = false;
        d->setCursorPosition(ev->pos());
        d->cursor.clearSelection();
        d->selectionChanged();
    }

    if (d->mousePressed) {
        d->mousePressed = false;
        d->setClipboardSelection();
    } else if (ev->button() == Qt::MidButton
               && !d->readOnly
               && QApplication::clipboard()->supportsSelection()) {
        d->setCursorPosition(ev->pos());
        insertFromMimeData(QApplication::clipboard()->mimeData(QClipboard::Selection));
    }

    d->viewport->update();

    if (d->dragStartTimer.isActive())
        d->dragStartTimer.stop();
}

/*! \reimp
*/
void QTextEdit::mouseDoubleClickEvent(QMouseEvent *ev)
{
    Q_D(QTextEdit);
    if (ev->button() != Qt::LeftButton) {
        ev->ignore();
        return;
    }

    d->mightStartDrag = false;
    d->setCursorPosition(ev->pos());
    QTextLine line = currentTextLine(d->cursor);
    if (line.isValid() && line.length()) {
        d->cursor.select(QTextCursor::WordUnderCursor);
        d->selectionChanged();
        d->setClipboardSelection();
        d->viewport->update();
    }

    d->trippleClickPoint = ev->globalPos();
    d->trippleClickTimer.start(qApp->doubleClickInterval(), this);
}

/*! \reimp
*/
bool QTextEdit::focusNextPrevChild(bool next)
{
    Q_D(QTextEdit);

    qDebug() << "QTextEdit::focusNextPrevChild(" << next << ")";

    // ##### hmm, this could go into QTextBrowser

    if (!d->readOnly)
        return false;

    if (!d->focusIndicator.hasSelection()) {
        d->focusIndicator = QTextCursor(d->doc);
        if (next)
            d->focusIndicator.movePosition(QTextCursor::Start);
        else
            d->focusIndicator.movePosition(QTextCursor::End);
    }

    Q_ASSERT(!d->focusIndicator.isNull());

    int anchorStart = -1;
    int anchorEnd = -1;

    if (next) {
        int startPos = d->focusIndicator.selectionEnd();
        QTextBlock block = d->doc->findBlock(startPos);

        while (block.isValid()) {
            anchorStart = -1;

            QTextBlock::Iterator it = block.begin();

            while (!it.atEnd() && it.fragment().position() < startPos)
                ++it;

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

                break;
            }

            block = block.next();
            startPos = block.position();
        }
    } else {
        int startPos = d->focusIndicator.selectionStart();
        if (startPos > 0)
            --startPos;

        QTextBlock block = d->doc->findBlock(startPos);

        while (block.isValid()) {
            anchorStart = -1;

            const QTextBlock::Iterator blockStart = block.begin();
            QTextBlock::Iterator it = blockStart;

            while (!it.atEnd() && it.fragment().position() < startPos)
                ++it;

            if (!it.atEnd()) {
                do {
                    const QTextFragment fragment = it.fragment();
                    const QTextCharFormat fmt = fragment.charFormat();

                    if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
                        anchorStart = fragment.position();
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
                        anchorEnd = fragment.position();
                        break;
                    }

                    if (it == blockStart)
                        it = QTextBlock::Iterator();
                    else
                        --it;
                } while (!it.atEnd());

                if (anchorEnd == -1)
                    anchorEnd = qMax(0, block.position() - 1);

                break;
            }

            block = block.previous();
            startPos = block.position() + block.length() - 1;
        }

    }

    if (anchorStart != -1 && anchorEnd != -1) {
        qDebug() << "anchorStart" << anchorStart << "anchorEnd" << anchorEnd;
        d->focusIndicator.setPosition(anchorStart);
        d->focusIndicator.setPosition(anchorEnd, QTextCursor::KeepAnchor);
    } else {
        d->focusIndicator.clearSelection();
    }

    if (d->focusIndicator.hasSelection()) {
        qSwap(d->focusIndicator, d->cursor);
        ensureCursorVisible();
        qSwap(d->focusIndicator, d->cursor);
        d->viewport->update();
        return true;
    } else {
        d->viewport->update();
        return false;
    }
}

/*! \reimp
*/
void QTextEdit::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu *popup = createPopupMenu(ev->pos());
    if (!popup) {
        ev->ignore();
        return;
    }
    popup->exec(ev->globalPos());
    delete popup;
    ev->accept();
}

/*! \reimp
*/
void QTextEdit::dragEnterEvent(QDragEnterEvent *ev)
{
    Q_D(QTextEdit);
    if (d->readOnly || !dataHasText(ev->mimeData())) {
        ev->ignore();
        return;
    }

    ev->acceptProposedAction();
}

/*! \reimp
*/
void QTextEdit::dragMoveEvent(QDragMoveEvent *ev)
{
    Q_D(QTextEdit);
    if (d->readOnly || !dataHasText(ev->mimeData())) {
        ev->ignore();
        return;
    }

    // don't change the cursor position here, as that would
    // destroy/change our visible selection and it would look ugly
    // and inconsistent. In Qt3's textedit the selection is independent
    // from the cursor, but now it's one thing. In Qt3 when dnd'ing
    // the cursor gets placed at where the text can be dropped. We can't
    // do this however, unless we introduce either a temporary selection
    // or a temporary second cursor. (Simon)

    ev->acceptProposedAction();
}

/*! \reimp
*/
void QTextEdit::dropEvent(QDropEvent *ev)
{
    Q_D(QTextEdit);
    if (d->readOnly || !dataHasText(ev->mimeData()))
        return;

    ev->acceptProposedAction();

    if (ev->dropAction() == QDrag::MoveAction
        && (ev->source() == this || ev->source() == d->viewport))
        d->cursor.removeSelectedText();

    d->setCursorPosition(ev->pos());
    insertFromMimeData(ev->mimeData());
}

/*! \reimp
 */
void QTextEdit::inputMethodEvent(QInputMethodEvent *e)
{
    Q_D(QTextEdit);
    if (d->readOnly) {
        e->ignore();
        return;
    }
    switch(e->type()) {
    case QEvent::InputMethodStart:
        d->cursor.removeSelectedText();
        d->imstart = d->imend = d->imselstart = d->imselend = d->cursor.position();
        break;
    case QEvent::InputMethodCompose:
        d->cursor.setPosition(d->imstart);
        d->cursor.setPosition(d->imend, QTextCursor::KeepAnchor);
        d->cursor.insertText(e->text());
        d->imend = d->imstart + e->text().length();
        d->imselstart = d->imstart + e->cursorPos();
        d->imselend = d->imselstart + e->selectionLength();
        d->cursor.setPosition(d->imselstart);
        d->viewport->update();
        break;
    case QEvent::InputMethodEnd:
        d->cursor.setPosition(d->imstart);
        d->cursor.setPosition(d->imend, QTextCursor::KeepAnchor);
        d->cursor.insertText(e->text());
        d->imselstart = d->imselend = d->imend = d->imstart = 0;
        d->viewport->update();
        break;
    default:
        Q_ASSERT(false);
    }
}

/*!\reimp
*/
QVariant QTextEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
    Q_D(const QTextEdit);
    QTextBlock block = d->cursor.block();
    switch(property) {
    case Qt::ImMicroFocus:
        return d->cursorRect();
    case Qt::ImFont:
        return currentFont();
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

/*! \reimp
*/
void QTextEdit::focusInEvent(QFocusEvent *ev)
{
    Q_D(QTextEdit);
    if (QFocusEvent::reason() == QFocusEvent::ActiveWindow) {
        // if we have a selection then we need to repaint, because (on windows)
        // the palette for active and inactive windows can have different colors
        // for selections
        if (d->cursor.hasSelection())
            d->viewport->update();

        if (!d->readOnly)
            d->setBlinkingCursorEnabled(true);
    }

    QViewport::focusInEvent(ev);
}

/*! \reimp
*/
void QTextEdit::focusOutEvent(QFocusEvent *ev)
{
    Q_D(QTextEdit);
    if (QFocusEvent::reason() == QFocusEvent::ActiveWindow) {
        // if we have a selection then we need to repaint, because (on windows)
        // the palette for active and inactive windows can have different colors
        // for selections
        if (d->cursor.hasSelection())
            d->viewport->update();

        if (d->cursorBlinkTimer.isActive())
            d->setBlinkingCursorEnabled(false);
    } else {
        d->focusIndicator.clearSelection();
    }
    QViewport::focusOutEvent(ev);
}

/*! \reimp
*/
void QTextEdit::showEvent(QShowEvent *)
{
    Q_D(QTextEdit);
    if (!d->anchorToScrollToWhenVisible.isEmpty()) {
        scrollToAnchor(d->anchorToScrollToWhenVisible);
        d->anchorToScrollToWhenVisible = QString::null;
    }
}

/*! \reimp
*/
void QTextEdit::changeEvent(QEvent *ev)
{
    Q_D(QTextEdit);
    QViewport::changeEvent(ev);
    if (ev->type() == QEvent::ApplicationFontChange
        || ev->type() == QEvent::FontChange) {
        d->doc->documentLayout()->setDefaultFont(font());
        // ####
        for (QFragmentMap<QTextBlockData>::ConstIterator it = d->doc->docHandle()->blockMap().begin();
             !it.atEnd(); ++it)
            it.value()->invalidate();
        resizeEvent(0);
    }
}

/*! \reimp
*/
void QTextEdit::wheelEvent(QWheelEvent *ev)
{
    Q_D(QTextEdit);
    if (d->readOnly) {
        if (ev->modifiers() & Qt::ControlModifier) {
            const int delta = ev->delta();
            if (delta > 0)
                zoomOut();
            else if (delta < 0)
                zoomIn();
            return;
        }
    }
    QViewport::wheelEvent(ev);
    updateMicroFocus();
}

/*!
    This function is called to create a right mouse button popup menu
    at the document position \a pos. If you want to create a custom
    popup menu, reimplement this function and return the created popup
    menu. Ownership of the popup menu is transferred to the caller.
*/
QMenu *QTextEdit::createPopupMenu(const QPoint &pos)
{
    Q_D(QTextEdit);
    Q_UNUSED(pos);

    QMenu *menu = new QMenu(this);
    QAction *a;

    if (!d->readOnly) {
        a = menu->addAction(tr("&Undo") + ACCEL_KEY(Z), d->doc, SLOT(undo()));
        a->setEnabled(d->doc->isUndoAvailable());
        a = menu->addAction(tr("&Redo") + ACCEL_KEY(Y), d->doc, SLOT(redo()));
        a->setEnabled(d->doc->isRedoAvailable());
        menu->addSeparator();

        a = menu->addAction(tr("Cu&t") + ACCEL_KEY(X), this, SLOT(cut()));
        a->setEnabled(d->cursor.hasSelection());
    }

    a = menu->addAction(tr("&Copy") + ACCEL_KEY(C), this, SLOT(copy()));
    a->setEnabled(d->cursor.hasSelection());

    if (!d->readOnly) {
        a = menu->addAction(tr("&Paste") + ACCEL_KEY(V), this, SLOT(paste()));
        a->setEnabled(!QApplication::clipboard()->text().isEmpty());
    }

    menu->addSeparator();
    a = menu->addAction(tr("Select All")
#if !defined(Q_WS_X11)
                        + ACCEL_KEY(A)
#endif
                        , this, SLOT(selectAll()));

    a->setEnabled(!d->doc->isEmpty());

    return menu;
}

/*!
    This function is called whenever the textedit's selection needs
    to be encapsulated into a new QMimeData object, for drag and
    drop or for copying to the clipboard for example.

    If you re-implement this function note that the ownership of
    the returned QMimeData object is passed to the caller. The
    selection can be retrieved using the textCursor() function.
*/
QMimeData *QTextEdit::createMimeDataFromSelection() const
{
    Q_D(const QTextEdit);
    const QTextDocumentFragment fragment(d->cursor);
    QMimeData *data = new QMimeData;

    QByteArray binary;
    QDataStream stream(&binary, QIODevice::WriteOnly);
    stream << fragment;
    data->setData("application/x-qt-richtext", binary);

    data->setHtml(fragment.toHtml());

    QString txt = fragment.toPlainText();
    txt.replace(QChar::Nbsp, ' ');
    data->setText(txt);
    return data;
};

/*!
    This function is called whenever text is to be inserted as the
    result of pasting from the clipboard or from accepting a drop.
*/
void QTextEdit::insertFromMimeData(const QMimeData *source)
{
    Q_D(QTextEdit);
    if (d->readOnly || !source)
	return;

    bool hasData = false;
    QTextDocumentFragment fragment;
    if (source->hasFormat("application/x-qt-richtext")) {
        QDataStream stream(source->data("application/x-qt-richtext"));
        stream >> fragment;
        hasData = true;
    } else if (source->hasFormat("application/x-qrichtext")) {
        fragment = QTextDocumentFragment::fromHtml(source->data("application/x-qrichtext"));
        hasData = true;
    } else if (source->hasHtml()) {
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

/*!
    \property QTextEdit::readOnly
    \brief whether the text edit is read-only

    In a read-only text edit the user can only navigate through the
    text and select text; modifying the text is not possible.

    This property's default is false.
*/

bool QTextEdit::isReadOnly() const
{
    Q_D(const QTextEdit);
    return d->readOnly;
}

void QTextEdit::setReadOnly(bool ro)
{
    Q_D(QTextEdit);
    if (d->readOnly == ro)
        return;

    d->readOnly = ro;
    d->viewport->setCursor(d->readOnly ? Qt::ArrowCursor : Qt::IbeamCursor);

    d->setBlinkingCursorEnabled(!ro);
}

/*!
    If the editor has a selection then the properties of \a modifier are
    applied to the selection. Without a selection the properties are applied
    to the word under the cursor. In addition they are always merged into
    the current char format.

    \sa QTextCursor::mergeCharFormat
 */
void QTextEdit::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
    Q_D(QTextEdit);
    if (d->readOnly)
        return;

    if (!d->cursor.hasSelection()) {
        QTextCursor word = d->cursor;
        word.select(QTextCursor::WordUnderCursor);
        word.mergeCharFormat(modifier);
    }
    d->cursor.mergeCharFormat(modifier);
    d->lastCharFormat = d->cursor.charFormat();
}

/*!
    Sets the char format that is be used when inserting new text to
    \a format .
 */
void QTextEdit::setCurrentCharFormat(const QTextCharFormat &format)
{
    Q_D(QTextEdit);
    d->cursor.setCharFormat(format);
    d->lastCharFormat = format;
}

/*!
    Returns the char format that is used when inserting new text.
 */
QTextCharFormat QTextEdit::currentCharFormat() const
{
    Q_D(const QTextEdit);
    return d->cursor.charFormat();
}

/*!
    \property QTextEdit::autoFormatting
    \brief the enabled set of auto formatting features

    The value can be any combination of the values in the \c
    AutoFormattingFlag enum.  The default is \c AutoAll. Choose \c AutoNone
    to disable all automatic formatting.

    Currently, the only automatic formatting feature provided is \c
    AutoBulletList; future versions of Qt may offer more.
*/

QTextEdit::AutoFormatting QTextEdit::autoFormatting() const
{
    Q_D(const QTextEdit);
    return d->autoFormatting;
}

void QTextEdit::setAutoFormatting(AutoFormatting features)
{
    Q_D(QTextEdit);
    d->autoFormatting = features;
}

/*!
    Convenience slot that inserts \a text at the current
    cursor position.

    It is equivalent to

    \code
    edit->textCursor().insertText(text);
    \endcode
 */
void QTextEdit::insertPlainText(const QString &text)
{
    Q_D(QTextEdit);
    d->cursor.insertText(text);
}

/*!
    Convenience slot that inserts \a text which is assumed to be of
    html formatting at the current cursor position.

    It is equivalent to:

    \code
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(text);
    edit->textCursor().insertFragment(fragment);
    \endcode
 */
void QTextEdit::insertHtml(const QString &text)
{
    Q_D(QTextEdit);
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(text);
    d->cursor.insertFragment(fragment);
}

/*!
    Scrolls the text edit so that the anchor with the given \a name is
    visible; does nothing if the \a name is empty, or is already
    visible, or isn't found.
*/
void QTextEdit::scrollToAnchor(const QString &name)
{
    Q_D(QTextEdit);
    if (name.isEmpty())
        return;

    if (!isVisible()) {
        d->anchorToScrollToWhenVisible = name;
        return;
    }

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

/*!
    \fn QTextEdit::zoomIn(int range)

    Zooms in on the text by by making the base font size \a range
    points larger and recalculating all font sizes to be the new size.
    This does not change the size of any images.

    \sa zoomOut()
*/
void QTextEdit::zoomIn(int range)
{
    QFont f = font();
    f.setPointSize(f.pointSize() + range);
    setFont(f);
}

/*!
    \fn QTextEdit::zoomOut(int range)

    \overload

    Zooms out on the text by making the base font size \a range points
    smaller and recalculating all font sizes to be the new size. This
    does not change the size of any images.

    \sa zoomIn()
*/
void QTextEdit::zoomOut(int range)
{
    QFont f = font();
    f.setPointSize(qMax(1, f.pointSize() - range));
    setFont(f);
}

/*! \property QTextEdit::tabChangesFocus
  \brief whether TAB changes focus or is accepted as input

  In some occasions text edits should not allow the user to input
  tabulators or change indentation using the TAB key, as this breaks
  the focus chain. The default is false.

*/

bool QTextEdit::tabChangesFocus() const
{
    Q_D(const QTextEdit);
    return d->tabChangesFocus;
}

void QTextEdit::setTabChangesFocus(bool b)
{
    Q_D(QTextEdit);
    d->tabChangesFocus = b;
}

/*!
    \property QTextEdit::documentTitle
    \brief the title of the document parsed from the text.
*/

/*!
    \property QTextEdit::wordWrap
    \brief the word wrap mode

    The default mode is \c WidgetWidth which causes words to be
    wrapped at the right edge of the text edit. Wrapping occurs at
    whitespace, keeping whole words intact. If you want wrapping to
    occur within words use setWrapPolicy(). If you set a wrap mode of
    \c FixedPixelWidth or \c FixedColumnWidth you should also call
    setWrapColumnOrWidth() with the width you want.

    \sa WordWrap, wrapColumnOrWidth, wrapPolicy,
*/

QTextEdit::WordWrap QTextEdit::wordWrap() const
{
    Q_D(const QTextEdit);
    return d->wordWrap;
}

void QTextEdit::setWordWrap(WordWrap wrap)
{
    Q_D(QTextEdit);
    if (d->wordWrap == wrap)
        return;
    d->wordWrap = wrap;
    resizeEvent(0);
}

/*!
    \property QTextEdit::wrapColumnOrWidth
    \brief the position (in pixels or columns depending on the wrap mode) where text will be wrapped

    If the wrap mode is \c FixedPixelWidth, the value is the number of
    pixels from the left edge of the text edit at which text should be
    wrapped. If the wrap mode is \c FixedColumnWidth, the value is the
    column number (in character columns) from the left edge of the
    text edit at which text should be wrapped.

    \sa wordWrap
*/

int QTextEdit::wrapColumnOrWidth() const
{
    Q_D(const QTextEdit);
    return d->wrapColumnOrWidth;
}

void QTextEdit::setWrapColumnOrWidth(int w)
{
    Q_D(QTextEdit);
    d->wrapColumnOrWidth = w;
    resizeEvent(0);
}

/*!
    Finds the next occurrence of the string, \a exp, using the given
    \a options searching in direction \a direction. Returns true if \a
    exp was found and changes the cursor to select the match;
    otherwise returns false;
*/
bool QTextEdit::find(const QString &exp, QTextDocument::FindFlags options, QTextDocument::FindDirection direction)
{
    Q_D(QTextEdit);
    QTextCursor search = d->doc->find(exp, d->cursor, options, direction);
    if (search.isNull())
        return false;

    setTextCursor(search);
    return true;
}

/*!
    \fn void QTextEdit::copyAvailable(bool yes)

    This signal is emitted when text is selected or de-selected in the
    text edit.

    When text is selected this signal will be emitted with \a yes set
    to true. If no text has been selected or if the selected text is
    de-selected this signal is emitted with \a yes set to false.

    If \a yes is true then copy() can be used to copy the selection to
    the clipboard. If \a yes is false then copy() does nothing.

    \sa selectionChanged()
*/

/*!
    \fn void QTextEdit::currentCharFormatChanged(const QTextCharFormat &f)

    This signal is emitted if the current character format has changed, for
    example caused by a change of the cursor position.

    The new format is \a f.

    \sa setCurrentCharFormat()
*/

/*!
    \fn void QTextEdit::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa copyAvailable()
*/

/*!
    \fn void QTextEdit::cursorPositionChanged()

    This signal is emitted whenever the position of the
    cursor changed.
*/

#ifdef QT_COMPAT
/*!
    Use the QTextCursor() class instead.
*/
void QTextEdit::moveCursor(CursorAction action, QTextCursor::MoveMode mode)
{
    Q_D(QTextEdit);
    if (action == MovePageUp) {
        d->pageUp(mode);
        return;
    } else if (action == MovePageDown) {
        d->pageDown(mode);
        return;
    }

    QTextCursor::MoveOperation op = QTextCursor::NoMove;
    switch (action) {
        case MoveBackward: op = QTextCursor::Left; break;
        case MoveForward: op = QTextCursor::Right; break;
        case MoveWordBackward: op = QTextCursor::WordLeft; break;
        case MoveWordForward: op = QTextCursor::WordRight; break;
        case MoveUp: op = QTextCursor::Up; break;
        case MoveDown: op = QTextCursor::Down; break;
        case MoveLineStart: op = QTextCursor::StartOfLine; break;
        case MoveLineEnd: op = QTextCursor::EndOfLine; break;
        case MoveHome: op = QTextCursor::Start; break;
        case MoveEnd: op = QTextCursor::End; break;
        default: return;
    }
    d->cursor.movePosition(op, mode);
    ensureCursorVisible();
    d->updateCurrentCharFormatAndSelection();
}

/*!
    Executes keyboard action \a action.

    Use the QTextCursor API instead.

    \sa textCursor()
*/
void QTextEdit::doKeyboardAction(KeyboardAction action)
{
    Q_D(QTextEdit);
    switch (action) {
        case ActionBackspace: d->cursor.deletePreviousChar(); break;
        case ActionDelete: d->cursor.deleteChar(); break;
        case ActionReturn: d->cursor.insertBlock(); break;
        case ActionKill: {
                QTextBlock block = d->cursor.block();
                if (d->cursor.position() == block.position() + block.length() - 2)
                    d->cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                else
                    d->cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                d->cursor.deleteChar();
                break;
            }
        case ActionWordBackspace:
            d->cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            d->cursor.deletePreviousChar();
            break;
        case ActionWordDelete:
            d->cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
            d->cursor.deleteChar();
            break;
    }
}

/*!
    Sets the text edit's \a text. The text can be plain text or HTML
    and the text edit will try to guess the right format.

    Use setHtml() or setPlainText() directly to avoid text edit's guessing.
*/
void QTextEdit::setText(const QString &text)
{
    Q_D(QTextEdit);
    if (d->textFormat == Qt::AutoText)
        d->textFormat = Qt::mightBeRichText(text) ? Qt::RichText : Qt::PlainText;
    if (d->textFormat == Qt::RichText)
        setHtml(text);
    else
        setPlainText(text);
}

/*!
    Returns all the text in the text edit as plain text.
*/
QString QTextEdit::text() const
{
    Q_D(const QTextEdit);
    if (d->textFormat == Qt::RichText || (d->textFormat == Qt::AutoText && d->preferRichText))
        return d->doc->toHtml();
    else
        return d->doc->toPlainText();
}


/*!
    Sets the text format to format \a f.

    \sa textFormat()
*/
void QTextEdit::setTextFormat(Qt::TextFormat f)
{
    Q_D(QTextEdit);
    d->textFormat = f;
}

/*!
    Returns the text format.

    \sa setTextFormat()
*/
Qt::TextFormat QTextEdit::textFormat() const
{
    Q_D(const QTextEdit);
    return d->textFormat;
}

#endif // QT_COMPAT


/*!
    Appends a new paragraph with \a text to the end of the text edit.
*/
void QTextEdit::append(const QString &text)
{
    Q_D(QTextEdit);
    Qt::TextFormat f = d->textFormat;
    if (f == Qt::AutoText) {
        if (Qt::mightBeRichText(text))
            f = Qt::RichText;
        else
            f = Qt::PlainText;
    }

    const bool atBottom = d->contentsY() >= d->contentsHeight() - d->viewport->height();

    QTextCursor cursor(d->doc);
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    if (f == Qt::PlainText) {
        QString txt = text;
        txt.replace('\n', QChar::ParagraphSeparator);
        cursor.insertText(txt);
    } else {
        QTextDocumentFragment frag = QTextDocumentFragment::fromHtml(text);
        cursor.insertFragment(frag);
    }
    cursor.endEditBlock();

    if (atBottom && d->vbar->isVisible())
        d->vbar->setValue(d->vbar->maximum() - d->viewport->height());
}

/*!
  Returns the current cursor rectangle.
*/
QRect QTextEditPrivate::cursorRect() const
{
    QTextFrame *frame = cursor.currentFrame();
    const QAbstractTextDocumentLayout *docLayout = doc->documentLayout();
    QTextBlock block = cursor.block();
    QTextLayout *layout = block.layout();
    QPointF layoutPos = layout->position() + docLayout->frameBoundingRect(frame).topLeft();
    const int relativePos = cursor.position() - block.position();
    QTextLine line = layout->findLine(relativePos);
    if (!line.isValid())
        return QRect(qRound(layoutPos.x()), qRound(layoutPos.y()), 1, 10); // ###

    return QRectF(layoutPos.x() + line.cursorToX(relativePos), layoutPos.y() + line.y(),
                  1, line.ascent() + line.descent()+1.).toRect();
}

/*!
    Ensures that the cursor is visible by scrolling the text edit if
    necessary.
*/
void QTextEdit::ensureCursorVisible()
{
    Q_D(QTextEdit);
    QRect crect = d->cursorRect();

    const int visibleWidth = d->viewport->width();
    const int visibleHeight = d->viewport->height();

    if (d->hbar->isVisible()) {
        if (crect.x() < d->contentsX())
            d->hbar->setValue(crect.x() - crect.width());
        else if (crect.x() + crect.width() > d->contentsX() + visibleWidth)
            d->hbar->setValue(crect.x() + crect.width() - visibleWidth);
    }

    if (d->vbar->isVisible()) {
        if (crect.y() < d->contentsY())
            d->vbar->setValue(crect.y() - crect.height());
        else if (crect.y() + crect.height() > d->contentsY() + visibleHeight)
            d->vbar->setValue(crect.y() + crect.height() - visibleHeight);
    }
    updateMicroFocus();
}


/*!
    \enum QTextEdit::KeyboardAction

    \compat

    \value ActionBackspace
    \value ActionDelete
    \value ActionReturn
    \value ActionKill
    \value ActionWordBackspace
    \value ActionWordDelete
*/

/*!
    \fn bool QTextEdit::find(const QString &exp, bool cs, bool wo)

    Use the find() overload that takes a QTextDocument::FindFlags
    argument.
*/

/*!
    \fn void QTextEdit::sync()

    Does nothing.
*/

/*!
    \fn void QTextEdit::setBold(bool b)

    Use setFontWeight() instead.
*/

/*!
    \fn void QTextEdit::setUnderline(bool b)

    Use setFontUnderline() instead.
*/

/*!
    \fn void QTextEdit::setItalic(bool i)

    Use setFontItalic() instead.
*/

/*!
    \fn void QTextEdit::setFamily(const QString &family)

    Use setFontFamily() instead.
*/

/*!
    \fn void QTextEdit::setPointSize(int size)

    Use setFontPointSize() instead.
*/

/*!
    \fn bool QTextEdit::italic() const

    Use fontItalic() instead.
*/

/*!
    \fn bool QTextEdit::bold() const

    Use fontWeight() >= QFont::Bold instead.
*/

/*!
    \fn bool QTextEdit::underline() const

    Use fontUnderline() instead.
*/

/*!
    \fn QString QTextEdit::family() const

    Use fontFamily() instead.
*/

/*!
    \fn int QTextEdit::pointSize() const

    Use int(fontPointSize()+0.5) instead.
*/

/*!
    \fn bool QTextEdit::hasSelectedText() const

    Use textCursor().hasSelection() instead.
*/

/*!
    \fn QString QTextEdit::selectedText() const

    Use textCursor().selectedText() instead.
*/

/*!
    \fn bool QTextEdit::isUndoAvailable() const

    Use document()->isUndoAvailable() instead.
*/

/*!
    \fn bool QTextEdit::isRedoAvailable() const

    Use document()->isRedoAvailable() instead.
*/

/*!
    \fn void QTextEdit::insert(const QString &text)

    Use insertPlainText() instead.
*/

/*!
    \fn bool QTextEdit::isModified() const

    Use document()->isModified() instead.
*/

/*!
    \fn QColor QTextEdit::color() const

    Use textColor() instead.
*/

/*!
    \fn void QTextEdit::textChanged();

###
*/

/*!
    \fn void QTextEdit::undoAvailable(bool b);

###
*/

/*!
    \fn void QTextEdit::redoAvailable(bool b);

###
*/

/*!
    \fn void QTextEdit::currentFontChanged(const QFont &font);

    Use currentCharFormatChanged() instead.
*/

/*!
    \fn void QTextEdit::currentColorChanged(const QColor &color);

    Use currentCharFormatChanged() instead.
*/

/*!
    \fn void QTextEdit::setModified(bool m)

    Use document->setModified() instead.
*/

/*!
    \fn void QTextEdit::setColor(const QColor &color)

    Use setTextColor() instead.
*/


#define d d_func()
#include "moc_qtextedit.cpp"
