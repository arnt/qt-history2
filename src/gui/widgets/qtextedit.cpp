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

#include <qfont.h>
#include <qpainter.h>
#include <qevent.h>
#include <qdebug.h>
#include <qdragobject.h>
#include <qclipboard.h>
#include <qmenu.h>
#include <qstyle.h>
#include <qbasictimer.h>
#include <qtimer.h>
#include <qscrollbar.h>
#include <private/qviewport_p.h>

#include "private/qtextdocumentlayout_p.h"
#include "private/qtextdocument_p.h"
#include "qtextdocument.h"
#include "qtextcursor.h"
#include "qtextdocumentfragment.h"
#include "qtextlist.h"

#include <qdatetime.h>
#include <qapplication.h>
#include <limits.h>
#include <qtexttable.h>

#ifndef QT_NO_ACCEL
#include <qkeysequence.h>
#define ACCEL_KEY(k) "\t" + QString(QKeySequence( Qt::CTRL | Qt::Key_ ## k ))
#else
#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)
#endif

#define d d_func()
#define q q_func()

class QRichTextDrag : public QTextDrag
{
public:
    QRichTextDrag(const QTextDocumentFragment &_fragment, QWidget *dragSource);

    virtual const char *format(int i) const;
    virtual QByteArray encodedData(const char *mime) const;

    static bool decode(const QMimeSource *e, QTextDocumentFragment &fragment);
    static bool canDecode(const QMimeSource* e);

private:
    QTextDocumentFragment fragment;
    mutable bool plainTextSet;
};

QRichTextDrag::QRichTextDrag(const QTextDocumentFragment &_fragment, QWidget *dragSource)
    : QTextDrag(dragSource), fragment(_fragment), plainTextSet(false)
{
}

const char *QRichTextDrag::format(int i) const
{
    const char *fmt = QTextDrag::format(i);
    if (fmt)
        return fmt;
    if (QTextDrag::format(i - 1))
        return "application/x-qt-richtext";
    return 0;
}

QByteArray QRichTextDrag::encodedData(const char *mime) const
{
    if (qstrcmp(mime, "application/x-qt-richtext") == 0) {
        QByteArray binary;
        QDataStream stream(&binary, IO_WriteOnly);
        stream << fragment;
        return binary;
    }

    if (!plainTextSet) {
        const_cast<QRichTextDrag *>(this)->setText(fragment.toPlainText());
        plainTextSet = true;
    }

    return QTextDrag::encodedData(mime);
}

bool QRichTextDrag::decode(const QMimeSource *e, QTextDocumentFragment &fragment)
{
    if (e->provides("application/x-qt-richtext")) {
        QDataStream stream(e->encodedData("application/x-qt-richtext"), IO_ReadOnly);
        stream >> fragment;
        return true;
    } else if (e->provides("application/x-qrichtext")) {
        fragment = QTextDocumentFragment::fromHTML(e->encodedData("application/x-qrichtext"));
        return true;
    }

    QString plainText;
    if (!QTextDrag::decode( e, plainText ))
        return false;

    fragment = QTextDocumentFragment::fromPlainText(plainText);
    return true;
}

bool QRichTextDrag::canDecode(const QMimeSource* e)
{
    if (e->provides("application/x-qt-richtext")
        || e->provides("application/x-qrichtext"))
        return true;
    return QTextDrag::canDecode(e);
}

class QTextEditPrivate : public QViewportPrivate
{
    Q_DECLARE_PUBLIC(QTextEdit)
public:
    inline QTextEditPrivate()
        : doc(0), cursorOn(false), readOnly(false),
          autoFormatting(QTextEdit::AutoAll), tabChangesFocus(false),
          mousePressed(false), mightStartDrag(false), wordWrap(QTextEdit::WidgetWidth), wrapColumnOrWidth(0),
          lastSelectionState(false), ignoreAutomaticScrollbarAdjustement(false), textFormat(Qt::AutoText)
    {}

    bool cursorMoveKeyEvent(QKeyEvent *e);

    void updateCurrentCharFormat();

    void indent();
    void outdent();

    void createAutoBulletList();

    void init(const QTextDocumentFragment &fragment = QTextDocumentFragment(),
              QTextDocument *document = 0);

    void startDrag();

    void paste(const QMimeSource *source);

    void setCursorPosition(const QPoint &pos);
    void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void update(const QRect &contentsRect);

    inline QPoint translateCoordinates(const QPoint &point)
    { return QPoint(point.x() + hbar->value(), point.y() + vbar->value()); }

    void selectionChanged();

    // helper for compat functions
    QTextBlock blockAt(const QPoint &pos, int *documentPosition = 0) const;

    inline int contentsX() const { return hbar->value(); }
    inline int contentsY() const { return vbar->value(); }
    inline int contentsWidth() const { return hbar->maximum() + viewport->width(); }
    inline int contentsHeight() const { return vbar->maximum() + viewport->height(); }

    bool pageUp(QTextCursor::MoveMode moveMode);
    bool pageDown(QTextCursor::MoveMode moveMode);

    void updateCurrentCharFormatAndSelection();

    void adjustScrollbars();

    void setClipboardSelection();

    QTextDocument *doc;
    bool cursorOn;
    QTextCursor cursor;
    QTextCharFormat currentCharFormat;

    bool readOnly; /* ### move to document? */

    QTextEdit::AutoFormatting autoFormatting;
    bool tabChangesFocus;

    QBasicTimer cursorBlinkTimer;

    QBasicTimer trippleClickTimer;
    QPoint trippleClickPoint;

    bool mousePressed;

    bool mightStartDrag;
    QPoint dragStartPos;
    QBasicTimer dragStartTimer;

    QTextEdit::WordWrap wordWrap;
    int wrapColumnOrWidth;

    bool lastSelectionState;

    bool ignoreAutomaticScrollbarAdjustement;

    // Qt3 COMPAT only
    // ### non-compat'ed append needs it, too
    Qt::TextFormat textFormat;
};

bool QTextEditPrivate::cursorMoveKeyEvent(QKeyEvent *e)
{
    QTextCursor::MoveMode mode = e->state() & Qt::ShiftButton
                                   ? QTextCursor::KeepAnchor
                                   : QTextCursor::MoveAnchor;

    QTextCursor::MoveOperation op = QTextCursor::NoMove;
#ifdef Q_WS_MAC
    // There can be only one modifier (+ shift), but we also need to make sure
    // that we have a "move key" pressed before we reject it.
    bool twoModifiers
        = ((e->state() & (Qt::ControlButton | Qt::AltButton))
           == (Qt::ControlButton | Qt::AltButton))
        || ((e->state() & (Qt::ControlButton | Qt::MetaButton))
            == (Qt::ControlButton | Qt::MetaButton))
        || ((e->state() & (Qt::AltButton | Qt::MetaButton)) == (Qt::AltButton | Qt::MetaButton));
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
            op = e->state() & Qt::ControlButton
                 ? QTextCursor::WordLeft
                 : QTextCursor::Left;
            break;
        case Qt::Key_Right:
            op = e->state() & Qt::ControlButton
                 ? QTextCursor::WordRight
                 : QTextCursor::Right;
            break;
        case Qt::Key_Home:
            op = e->state() & Qt::ControlButton
                 ? QTextCursor::Start
                 : QTextCursor::StartOfLine;
            break;
        case Qt::Key_End:
            op = e->state() & Qt::ControlButton
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
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::Start;
                else if (e->state() & Qt::AltButton)
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
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::End;
                else if (e->state() & Qt::AltButton)
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
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::StartOfLine;
                else if (e->state() & Qt::AltButton)
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
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::EndOfLine;
                else if (e->state() & Qt::AltButton)
                    op = QTextCursor::WordRight;
                else
                    op = QTextCursor::Right;
            }
            break;
        case Qt::Key_Home:
            if (e->state() & (Qt::ControlButton | Qt::MetaButton | Qt::AltButton)) {
                QApplication::beep();
                return true;
            } else {
                op = QTextCursor::Start;
            }
            break;
        case Qt::Key_End:
            if (e->state() & (Qt::ControlButton | Qt::MetaButton | Qt::AltButton)) {
                QApplication::beep();
                return true;
            } else {
                op = QTextCursor::End;
            }
            break;
#endif
        case Qt::Key_Next:
            return pageDown(mode);
        case Qt::Key_Prior:
            return pageUp(mode);
    default:
        return false;
    }

    cursor.movePosition(op, mode);
    q->ensureCursorVisible();

    selectionChanged();

    viewport->update();

    return true;
}

void QTextEditPrivate::updateCurrentCharFormat()
{
    QTextCharFormat fmt = cursor.charFormat();
    if (fmt == currentCharFormat)
        return;
    currentCharFormat = fmt;

    emit q->currentCharFormatChanged(currentCharFormat);
    // compat signals
    emit q->currentFontChanged(currentCharFormat.font());
    emit q->currentColorChanged(currentCharFormat.textColor());
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
    if (!doc) {
        doc = document;
        if (!doc)
            doc = new QTextDocument(q);

        QObject::connect(doc->documentLayout(), SIGNAL(update(const QRect &)), q, SLOT(update(const QRect &)));
        QObject::connect(doc->documentLayout(), SIGNAL(usedSizeChanged()), q, SLOT(adjustScrollbars()));
        cursor = QTextCursor(doc);

        hbar->setSingleStep(20);
        vbar->setSingleStep(20);

        QObject::connect(doc, SIGNAL(contentsChanged()), q, SLOT(updateCurrentCharFormatAndSelection()));

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

    q->clear();

    QTextCharFormat fmt;
    fmt.setFont(q->font());
    fmt.setTextColor(q->palette().color(QPalette::Text));

    // ###############
//     d->cursor.movePosition(QTextCursor::Start);
//     d->cursor.setBlockFormat(fmt);

    viewport->setCursor(readOnly ? Qt::ArrowCursor : Qt::IbeamCursor);

    doc->setUndoRedoEnabled(false);

    QTextFrame *rootFrame = doc->rootFrame();
    QTextFrameFormat ffmt = rootFrame->format();
    ffmt.setMargin(4);
    rootFrame->setFormat(ffmt);

    if (!fragment.isEmpty()) {
        cursor.movePosition(QTextCursor::Start);
        cursor.insertFragment(fragment);
    }

    doc->setUndoRedoEnabled(true);
    cursor.movePosition(QTextCursor::Start);
    updateCurrentCharFormatAndSelection();

    doc->setModified(false);
}

void QTextEditPrivate::startDrag()
{
    mousePressed = false;
    QRichTextDrag *drag = new QRichTextDrag(cursor, viewport);
    if (readOnly) {
        drag->dragCopy();
    } else {
        if (drag->drag() && QDragObject::target() != q && QDragObject::target() != viewport)
            cursor.removeSelectedText();
    }
}

void QTextEditPrivate::paste(const QMimeSource *source)
{
    if (readOnly || !source || !QRichTextDrag::canDecode(source))
	return;

    QTextDocumentFragment fragment;
    if (!QRichTextDrag::decode(source, fragment))
	return;

    cursor.insertFragment(fragment);
    q->ensureCursorVisible();
}

void QTextEditPrivate::setCursorPosition(const QPoint &pos)
{
    const int cursorPos = doc->documentLayout()->hitTest(pos, QText::FuzzyHit);
    if (cursorPos == -1)
        return;
    cursor.setPosition(cursorPos);
}

void QTextEditPrivate::setCursorPosition(int pos, QTextCursor::MoveMode mode)
{
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

    r.moveBy(-xOffset, -yOffset);
    viewport->update(r);
}

void QTextEditPrivate::selectionChanged()
{
    bool current = cursor.hasSelection();
    if (current == lastSelectionState)
        return;

    lastSelectionState = current;
    emit q->copyAvailable(current);
    emit q->selectionChanged();
}

QTextBlock QTextEditPrivate::blockAt(const QPoint &pos, int *documentPosition) const
{
    const int docPos = doc->documentLayout()->hitTest(pos, QText::ExactHit);

    if (docPos == -1) {
        if (documentPosition)
            *documentPosition = -1;
        return QTextBlock();
    }

    if (documentPosition)
        *documentPosition = docPos;

    QTextDocumentPrivate *pt = doc->docHandle();
    return QTextBlock(pt, pt->blockMap().findNode(docPos));
}

bool QTextEditPrivate::pageUp(QTextCursor::MoveMode moveMode)
{
    int targetY = vbar->value() - viewport->height();
    bool moved = false;
    do {
        q->ensureCursorVisible();
        moved = cursor.movePosition(QTextCursor::Up, moveMode);
    } while (moved && vbar->value() > targetY);
    return moved;
}

bool QTextEditPrivate::pageDown(QTextCursor::MoveMode moveMode)
{
    int targetY = vbar->value() + viewport->height();
    bool moved = false;
    do {
        q->ensureCursorVisible();
        moved = cursor.movePosition(QTextCursor::Down, moveMode);
    } while (moved && vbar->value() < targetY);
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
    if (!d->cursor.hasSelection())
        return;
    QRichTextDrag *drag = new QRichTextDrag(d->cursor, 0);
    QApplication::clipboard()->setData(drag, QClipboard::Selection);
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
    \row \i Qt::UpArrow        \i Move one line up
    \row \i Qt::DownArrow        \i Move one line down
    \row \i Qt::LeftArrow        \i Move one character left
    \row \i Qt::RightArrow        \i Move one character right
    \row \i PageUp        \i Move one (viewport) page up
    \row \i PageDown        \i Move one (viewport) page down
    \row \i Home        \i Move to the beginning of the text
    \row \i End                \i Move to the end of the text
    \row \i Shift+Wheel
         \i Scroll the page horizontally (the Wheel is the mouse wheel)
    \row \i Ctrl+Wheel        \i Zoom the text
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
    the user-visible cursor using the cursor() method. If you want to set 
    a selection in QTextEdit just create one on a QTextCursor object and 
    then make that cursor the visible cursor using setCursor(). The selection 
    can be copied to the clipboard with copy(), or cut to the clipboard with 
    cut(). The entire text can be selected using selectAll().

    When the cursor is moved, the currentCharFormatChanged() signal is 
    emitted to reflect the new formatting attributes at the new cursor
    position.

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
    \row \i Backspace \i Delete the character to the left of the cursor
    \row \i Delete \i Delete the character to the right of the cursor
    \row \i Ctrl+A \i Move the cursor to the beginning of the line
    \row \i Ctrl+B \i Move the cursor one character left
    \row \i Ctrl+C \i Copy the marked text to the clipboard (also
                      Ctrl+Insert under Windows)
    \row \i Ctrl+D \i Delete the character to the right of the cursor
    \row \i Ctrl+E \i Move the cursor to the end of the line
    \row \i Ctrl+F \i Move the cursor one character right
    \row \i Ctrl+H \i Delete the character to the left of the cursor
    \row \i Ctrl+K \i Delete to end of line
    \row \i Ctrl+N \i Move the cursor one line down
    \row \i Ctrl+P \i Move the cursor one line up
    \row \i Ctrl+V \i Paste the clipboard text into line edit
                      (also Shift+Insert under Windows)
    \row \i Ctrl+X \i Cut the marked text, copy to clipboard
                      (also Shift+Delete under Windows)
    \row \i Ctrl+Z \i Undo the last operation
    \row \i Ctrl+Y \i Redo the last operation
    \row \i Qt::LeftArrow            \i Move the cursor one character left
    \row \i Ctrl+Qt::LeftArrow  \i Move the cursor one word left
    \row \i Qt::RightArrow            \i Move the cursor one character right
    \row \i Ctrl+Qt::RightArrow \i Move the cursor one word right
    \row \i Qt::UpArrow            \i Move the cursor one line up
    \row \i Ctrl+Qt::UpArrow    \i Move the cursor one word up
    \row \i Qt::DownArrow            \i Move the cursor one line down
    \row \i Ctrl+Down Arrow \i Move the cursor one word down
    \row \i PageUp            \i Move the cursor one page up
    \row \i PageDown            \i Move the cursor one page down
    \row \i Home            \i Move the cursor to the beginning of the line
    \row \i Ctrl+Home            \i Move the cursor to the beginning of the text
    \row \i End                    \i Move the cursor to the end of the line
    \row \i Ctrl+End            \i Move the cursor to the end of the text
    \row \i Shift+Wheel            \i Scroll the page horizontally
                            (the Wheel is the mouse wheel)
    \row \i Ctrl+Wheel            \i Zoom the text
    \endtable

    To select (mark) text hold down the Shift key whilst pressing one
    of the movement keystrokes, for example, \e{Shift+Right Arrow}
    will select the character to the right, and \e{Shift+Ctrl+Right
    Arrow} will select the word to the right, etc.

    \sa QTextDocument QTextCursor document() cursor() setDocument() setCursor()

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
*/

/*!
    Constructs an empty QTextEdit with parent \a
    parent.
*/
QTextEdit::QTextEdit(QWidget *parent)
    : QViewport(*new QTextEditPrivate, parent)
{
    d->init();
}

/*!
    Constructs a QTextEdit with parent \a parent. The text edit will display
    the text \a text. The text is interpreted as html.
*/
QTextEdit::QTextEdit(const QString &text, QWidget *parent)
    : QViewport(*new QTextEditPrivate, parent)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->init(fragment);
}

#ifdef QT_COMPAT
QTextEdit::QTextEdit(QWidget *parent, const char *name)
    : QViewport(*new QTextEditPrivate, parent)
{
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
    return d->currentCharFormat.fontPointSize();
}

/*!
    Returns the font family of the current format.

    \sa setFontFamily() setCurrentFont() setFontPointSize()
*/
QString QTextEdit::fontFamily() const
{
    return d->currentCharFormat.fontFamily();
}

/*!
    Returns the font weight of the current format.

    \sa setFontWeight() setCurrentFont() setFontPointSize()
*/
int QTextEdit::fontWeight() const
{
    return d->currentCharFormat.fontWeight();
}

/*!
    Returns true if the font of the current format is underlined; otherwise returns
    false.

    \sa setFontUnderline()
*/
bool QTextEdit::fontUnderline() const
{
    return d->currentCharFormat.fontUnderline();
}

/*!
    Returns true if the font of the current format is italic; otherwise returns
    false.

    \sa setFontItalic()
*/
bool QTextEdit::fontItalic() const
{
    return d->currentCharFormat.fontItalic();
}

/*!
    Returns the text color of the current format.

    \sa setTextColor()
*/
QColor QTextEdit::textColor() const
{
    return d->currentCharFormat.textColor();
}

/*!
    Returns the font of the current format.

    \sa setCurrentFont() setFontFamily() setFontPointSize()
*/
QFont QTextEdit::currentFont() const
{
    return d->currentCharFormat.font();
}

/*!
    Sets the alignment of the current paragraph to \a a. Valid
    alignments are \c Qt::AlignLeft, \c Qt::AlignRight,
    \c Qt::AlignJustify and \c Qt::AlignCenter (which centers
    horizontally).
*/
void QTextEdit::setAlignment(Qt::Alignment a)
{
    if (d->readOnly)
	return;
    QTextBlockFormat fmt;
    fmt.setAlignment(a);
    d->cursor.mergeBlockFormat(fmt);
}

/*!
    Returns the alignment of the current paragraph.

    \sa setAlignment()
*/
Qt::Alignment QTextEdit::alignment() const
{
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
    return d->doc;
}

/*!
    Sets the visible cursor to \a cursor.
*/
void QTextEdit::setCursor(const QTextCursor &cursor)
{
    d->cursor = cursor;
    d->updateCurrentCharFormatAndSelection();
    ensureCursorVisible();
}

/*!
    Returns a QTextCursor that represents the currently visible cursor.
 */
QTextCursor QTextEdit::cursor() const
{
    return d->cursor;
}

/*!
    Sets the font family of the current format to \a fontFamily.

    \sa fontFamily() setCurrentFont()
*/
void QTextEdit::setFontFamily(const QString &fontFamily)
{
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
    if (!d->cursor.hasSelection())
	return;
    QRichTextDrag *drag = new QRichTextDrag(d->cursor, 0);
    QApplication::clipboard()->setData(drag);
}

/*!
    Pastes the text from the clipboard into the text edit at the
    current cursor position.

    If there is no text in the clipboard nothing happens.

    \sa cut() copy()
*/

void QTextEdit::paste()
{
    d->paste(QApplication::clipboard()->data());
}

/*!
    Deletes all the text in the text edit.

    \sa cut() removeSelectedText() setPlainText() setHtml()
*/
void QTextEdit::clear()
{
    selectAll();
    d->cursor.removeSelectedText();
}


/*!
    Selects all text.

    \sa copy() cut() cursor()
 */
void QTextEdit::selectAll()
{
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
    if (ev->timerId() == d->cursorBlinkTimer.timerId()) {
        d->cursorOn = !d->cursorOn;

        if (d->cursor.hasSelection())
            d->cursorOn &= (style().styleHint(QStyle::SH_BlinkCursorWhenTextSelected) != 0);

        QRect r = d->cursor.block().layout()->rect();
        r.moveBy(-d->hbar->value(), -d->vbar->value());
        d->viewport->update(r);
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

    \sa plainText()
*/

void QTextEdit::setPlainText(const QString &text)
{
    if (text.isEmpty())
        return;

    QTextDocumentFragment fragment = QTextDocumentFragment::fromPlainText(text);
    d->init(fragment);
}

/*!
    Returns the text of the text edit as plain text.

    \sa setPlainText
 */
QString QTextEdit::plainText() const
{
    return d->doc->plainText();
}

/*!
    \overload

    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as rich text in html format.

    Note that the undo/redo history is cleared by this function.
*/

void QTextEdit::setHtml(const QString &text)
{
    if (text.isEmpty())
        return;

    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->init(fragment);
}
/*!
    \overload

    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as rich text in html format. Any encoding
    attribute specified in the header of the html is obeyed.

    Note that the undo/redo history is cleared by this function.
*/

void QTextEdit::setHtml(const QByteArray &text)
{
    if (text.isEmpty())
	return;

    // use QByteArray overload that obeys html content encoding
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->init(fragment);
}

/*! \reimp
*/
void QTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (d->readOnly) {
        QViewport::keyPressEvent(e);
        return;
    }

    bool updateCurrentFormat = true;

    if (d->cursorMoveKeyEvent(e))
        goto accept;

    if (e->state() & Qt::ControlButton) {
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
        d->cursor.deleteChar();
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
                if (d->cursor.atBlockStart()) {
                    d->indent();
                    break;
                }
            }

            if (e->key() == Qt::Key_Backtab) {
                if (d->tabChangesFocus) {
                    e->ignore();
                    return;
                }
                if (d->cursor.atBlockStart()) {
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

            if (!text.isEmpty()) {
                d->cursor.insertText(text, d->currentCharFormat);
                updateCurrentFormat = false;
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

    if (updateCurrentFormat)
        d->updateCurrentCharFormat();

}

/*!
    Returns how many pixels high the text edit needs to be to display
    all the text if the text edit is \a width pixels wide.
*/
int QTextEdit::heightForWidth(int width) const
{
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
    QPainter p(d->viewport);

    const int xOffset = d->hbar->value();
    const int yOffset = d->vbar->value();

    p.translate(-xOffset, -yOffset);

    QRect r = ev->rect();
    r.moveBy(xOffset, yOffset);
    p.setClipRect(r);

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.showCursor = (d->cursorOn && d->viewport->hasFocus());
    ctx.cursor = d->cursor;
    ctx.palette = palette();

    d->doc->documentLayout()->draw(&p, ctx);
}

/*! \reimp
*/
void QTextEdit::mousePressEvent(QMouseEvent *ev)
{
    if (!(ev->button() & Qt::LeftButton))
        return;

    const QPoint pos = d->translateCoordinates(ev->pos());

    d->mousePressed = true;
    d->mightStartDrag = false;

    if (d->trippleClickTimer.isActive()
        && ((ev->globalPos() - d->trippleClickPoint).manhattanLength() < QApplication::startDragDistance())) {

        d->cursor.movePosition(QTextCursor::StartOfLine);
        d->cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

        d->trippleClickTimer.stop();
    } else {
        int cursorPos = d->doc->documentLayout()->hitTest(pos, QText::FuzzyHit);
        if (cursorPos != -1) {

            if (d->cursor.hasSelection()
                && cursorPos >= d->cursor.selectionStart()
                && cursorPos <= d->cursor.selectionEnd()) {
                d->mightStartDrag = true;
                d->dragStartPos = ev->globalPos();
                d->dragStartTimer.start(QApplication::startDragTime(), this);
                return;
            }

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
    if (!(ev->state() & Qt::LeftButton)
        || !d->mousePressed)
        return;

    if (d->mightStartDrag) {
        d->dragStartTimer.stop();

        if ((ev->globalPos() - d->dragStartPos).manhattanLength() > QApplication::startDragDistance())
            d->startDrag();

        return;
    }

    int cursorPos = d->doc->documentLayout()->hitTest(d->translateCoordinates(ev->pos()), QText::FuzzyHit);
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
    if (d->mightStartDrag) {
        d->mousePressed = false;
        d->cursor.clearSelection();
        d->selectionChanged();
    }

    if (d->mousePressed) {
        d->mousePressed = false;
        d->setClipboardSelection();
    } else if (ev->button() == Qt::MidButton
               && !d->readOnly
               && QApplication::clipboard()->supportsSelection()) {
        d->setCursorPosition(d->translateCoordinates(ev->pos()));
        d->paste(QApplication::clipboard()->data(QClipboard::Selection));
    }

    d->viewport->update();

    if (d->dragStartTimer.isActive())
        d->dragStartTimer.stop();
}

/*! \reimp
*/
void QTextEdit::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton) {
        ev->ignore();
        return;
    }

    d->cursor.movePosition(QTextCursor::PreviousWord);
    d->cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    d->selectionChanged();
    d->viewport->update();

    d->trippleClickPoint = ev->globalPos();
    d->trippleClickTimer.start(qApp->doubleClickInterval(), this);
}

/*! \reimp
*/
bool QTextEdit::focusNextPrevChild(bool next)
{
    Q_UNUSED(next)
// ###
    return d->readOnly;
//    if (d->cursor.atBlockStart())
//        return false;
//    return QScrollView::focusNextPrevChild(next);
}

/*! \reimp
*/
void QTextEdit::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu *popup = createContextMenu(ev->pos());
    if (!popup)
	return;
    popup->exec(ev->globalPos());
    delete popup;
}

/*! \reimp
*/
void QTextEdit::dragEnterEvent(QDragEnterEvent *ev)
{
    if (d->readOnly || !QRichTextDrag::canDecode(ev)) {
        ev->ignore();
        return;
    }
    ev->acceptAction();
}

/*! \reimp
*/
void QTextEdit::dragMoveEvent(QDragMoveEvent *ev)
{
    if (d->readOnly || !QRichTextDrag::canDecode(ev)) {
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

    ev->acceptAction();
}

/*! \reimp
*/
void QTextEdit::dropEvent(QDropEvent *ev)
{
    if (d->readOnly || !QRichTextDrag::canDecode(ev))
        return;

    ev->acceptAction();

    if (ev->action() == QDropEvent::Move 
        && (ev->source() == this || ev->source() == d->viewport))
        d->cursor.removeSelectedText();

    d->setCursorPosition(d->translateCoordinates(ev->pos()));
    d->paste(ev);
}

/*! \reimp
*/
void QTextEdit::focusInEvent(QFocusEvent *ev)
{
    // if we have a selection then we need to repaint, because (on windows)
    // the palette for active and inactive windows can have different colors
    // for selections
    if (d->cursor.hasSelection())
        d->viewport->update();
    QViewport::focusInEvent(ev);
}

/*! \reimp
*/
void QTextEdit::focusOutEvent(QFocusEvent *ev)
{
    // if we have a selection then we need to repaint, because (on windows)
    // the palette for active and inactive windows can have different colors
    // for selections
    if (d->cursor.hasSelection())
        d->viewport->update();
    QViewport::focusOutEvent(ev);
}

/*!
    This function is called to create a right mouse button popup menu
    at the document position \a pos. If you want to create a custom
    popup menu, reimplement this function and return the created popup
    menu. Ownership of the popup menu is transferred to the caller.
*/
QMenu *QTextEdit::createContextMenu(const QPoint &pos)
{
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

        a = menu->addAction(tr("Clear"), this, SLOT(clear()));
        a->setEnabled(!d->doc->isEmpty());
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
    \property QTextEdit::readOnly
    \brief whether the text edit is read-only

    In a read-only text edit the user can only navigate through the
    text and select text; modifying the text is not possible.

    This property's default is false.
*/

bool QTextEdit::isReadOnly() const
{
    return d->readOnly;
}

void QTextEdit::setReadOnly(bool ro)
{
    if (d->readOnly == ro)
        return;

    d->readOnly = ro;
    d->viewport->setCursor(d->readOnly ? Qt::ArrowCursor : Qt::IbeamCursor);

    if (ro)
        d->cursorBlinkTimer.stop();
    else
        d->cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, this);
}

/*!
    If the editor has a selection then the properties of \a modifier are
    applied to the selection. In addition they are merged into the current
    char format.

    \sa QTextCursor::mergeCharFormat
 */
void QTextEdit::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
    if (d->readOnly)
        return;

    if (d->cursor.hasSelection())
	d->cursor.mergeCharFormat(modifier);

    d->currentCharFormat.merge(modifier);
}

/*!
    Sets the char format that is be used when inserting new text to
    \a format .
 */
void QTextEdit::setCurrentCharFormat(const QTextCharFormat &format)
{
    d->currentCharFormat = format;
}

/*!
    Returns the char format that is used when inserting new text.
 */
QTextCharFormat QTextEdit::currentCharFormat() const
{
    return d->currentCharFormat;
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
    return d->autoFormatting;
}

void QTextEdit::setAutoFormatting(AutoFormatting features)
{
    d->autoFormatting = features;
}

/*!
    Convenience slot that inserts \a text at the current
    cursor position.

    It is equivalent to

    \code
    edit->cursor().insertText(text);
    \endcode
 */
void QTextEdit::insertPlainText(const QString &text)
{
    d->cursor.insertText(text);
}

/*!
    Convenience slot that inserts \a text which is assumed to be of
    html formatting at the current cursor position.

    It is equivalent to:

    \code
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    edit->cursor().insertFragment(fragment);
    \endcode
 */
void QTextEdit::insertHtml(const QString &text)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->cursor.insertFragment(fragment);
}

/*! \property QTextEdit::tabChangesFocus
  \brief whether TAB changes focus or is accepted as input

  In some occasions text edits should not allow the user to input
  tabulators or change indentation using the TAB key, as this breaks
  the focus chain. The default is false.

*/

bool QTextEdit::tabChangesFocus() const
{
    return d->tabChangesFocus;
}

void QTextEdit::setTabChangesFocus(bool b)
{
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
    return d->wordWrap;
}

void QTextEdit::setWordWrap(WordWrap wrap)
{
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
    return d->wrapColumnOrWidth;
}

void QTextEdit::setWrapColumnOrWidth(int w)
{
    d->wrapColumnOrWidth = w;
    resizeEvent(0);
}

/*!
    Finds the next occurrence of the string, \a exp, using the given
    \a options. Returns true if \a exp was found and changes the cursor
    to select the match; otherwise returns false;
*/
bool QTextEdit::find(const QString &exp, QTextDocument::FindFlags options)
{
    QTextCursor search = d->doc->find(exp, d->cursor, options);
    if (search.isNull())
        return false;

    setCursor(search);
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


#ifdef QT_COMPAT
void QTextEdit::moveCursor(CursorAction action, QTextCursor::MoveMode mode)
{
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

void QTextEdit::doKeyboardAction(KeyboardAction action)
{
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

void QTextEdit::setText(const QString &text)
{
    if (d->textFormat == Qt::AutoText)
        d->textFormat = QText::mightBeRichText(text) ? Qt::RichText : Qt::PlainText;
    if (d->textFormat == Qt::RichText)
        setHtml(text);
    else
        setPlainText(text);
}

QString QTextEdit::text() const
{
    // ########## richtext case
    return document()->plainText();
}


void QTextEdit::setTextFormat(Qt::TextFormat f)
{
    d->textFormat = f;
}

Qt::TextFormat QTextEdit::textFormat() const
{
    return d->textFormat;
}

/*
static int blockNr(QTextBlock block)
{
    int nr = -1;

    for (; block.isValid(); block = block.previous())
        ++nr;

    return nr;
}

QTextBlock QTextEdit::blockAt(int blockNr) const
{
    QTextBlock block = d->doc->rootFrame()->begin().currentBlock();
    while (blockNr > 0 && block.isValid()) {
        block = block.next();
        --blockNr;
    }
    return block;
}

void QTextEdit::setCursorPosition(int parag, int index)
{
    QTextCursor c(blockAt(parag));
    c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, index);
    setCursor(c);
}

void QTextEdit::getCursorPosition(int *parag, int *index) const
{
    if (!parag || !index)
        return;

    QTextBlock block = d->cursor.block();

    Q_ASSERT(block.isValid());

    *parag = blockNr(block);
    *index = d->cursor.position() - block.position();
}

int QTextEdit::paragraphs() const
{
    return d->doc->docHandle()->numBlocks();
}

int QTextEdit::lines() const
{
    int l = 0;
    for (QTextBlock block = d->doc->rootFrame()->begin().currentBlock();
         block.isValid(); block = block.next())
        l += block.layout()->numLines();
    return l;
}

void QTextEdit::getSelection(int *paraFrom, int *indexFrom, int *paraTo, int *indexTo) const
{
    if (!paraFrom || !paraTo || !indexFrom || !indexTo)
        return;

    if (!d->cursor.hasSelection()) {
        *paraFrom = *indexFrom = *paraTo = *indexTo = -1;
        return;
    }

    QTextDocumentPrivate *pt = d->doc->docHandle();

    const int selStart = d->cursor.selectionStart();
    const int selEnd = d->cursor.selectionEnd();

    QTextBlock fromBlock(pt, pt->blockMap().findNode(selStart));
    *paraFrom = blockNr(fromBlock);
    *indexFrom = selStart - fromBlock.position();

    QTextBlock toBlock(pt, pt->blockMap().findNode(selEnd));
    *paraTo = blockNr(toBlock);
    *indexTo = selEnd - toBlock.position();
}
*/

/* really have this?
int QTextEdit::lineOfChar(int parag, int index) const
{
    QTextBlock block = d->blockAt(parag);
    if (!block.isValid())
        return -1;

    QTextLine line = block.layout()->findLine(index);
    if (!line.isValid())
        return -1;

    return line.line();
}
*/

/*
int QTextEdit::paragraphAt(const QPoint &pos) const
{
    return blockNr(d->blockAt(pos));
}

int QTextEdit::charAt(const QPoint &pos, int *parag) const
{
    int docPos = 0;
    QTextBlock block = d->blockAt(pos, &docPos);
    if (!block.isValid()) {
        if (parag)
            *parag = -1;
        return -1;
    }

    if (parag)
        *parag = blockNr(block);

    return docPos - block.position();
}

void QTextEdit::setParagraphBackgroundColor(int parag, const QColor &col)
{
    QTextBlock block = blockAt(parag);
    if (!block.isValid())
        return;

    QTextCursor c(block);
    QTextBlockFormat fmt;
    fmt.setBackgroundColor(col);
    c.mergeBlockFormat(fmt);
}
*/
#endif // QT_COMPAT


/*!
    Appends a new paragraph with \a text to the end of the text edit.
*/
void QTextEdit::append(const QString &text)
{
    Qt::TextFormat f = d->textFormat;
    if (f == Qt::AutoText) {
        if (QText::mightBeRichText(text))
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
        QTextDocumentFragment frag = QTextDocumentFragment::fromHTML(text);
        cursor.insertFragment(frag);
    }
    cursor.endEditBlock();

    if (atBottom && d->vbar->isVisible())
        d->vbar->setValue(d->vbar->maximum() - d->viewport->height());
}
/*!
    Ensures that the cursor is visible by scrolling the text edit if
    necessary.
*/
void QTextEdit::ensureCursorVisible()
{
    QTextBlock block = d->cursor.block();
    QTextLayout *layout = block.layout();
    QPoint layoutPos = layout->position();
    const int relativePos = d->cursor.position() - block.position();
    QTextLine line = layout->findLine(relativePos);
    if (!line.isValid())
        return;

    const int cursorX = layoutPos.x() + line.cursorToX(relativePos);
    const int cursorY = layoutPos.y() + line.y();
    const int cursorWidth = 1;
    const int cursorHeight = line.ascent() + line.descent();

    const int visibleWidth = d->viewport->width();
    const int visibleHeight = d->viewport->height();

    if (d->hbar->isVisible()) {
        if (cursorX < d->contentsX())
            d->hbar->setValue(cursorX - cursorWidth);
        else if (cursorX + cursorWidth > d->contentsX() + visibleWidth)
            d->hbar->setValue(cursorX + cursorWidth - visibleWidth);
    }

    if (d->vbar->isVisible()) {
        if (cursorY < d->contentsY())
            d->vbar->setValue(cursorY - cursorHeight);
        else if (cursorY + cursorHeight > d->contentsY() + visibleHeight)
            d->vbar->setValue(cursorY + cursorHeight - visibleHeight);
    }
}

#include "moc_qtextedit.cpp"
