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

#include "qlineedit.h"
#include "qlineedit_p.h"

#ifndef QT_NO_LINEEDIT
#include "qaction.h"
#include "qapplication.h"
#include "qclipboard.h"
#include "qdrag.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qfontmetrics.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qpointer.h"
#include "qstringlist.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtimer.h"
#include "qvalidator.h"
#include "qvariant.h"
#include "qvector.h"
#include "qwhatsthis.h"
#include "qdebug.h"
#include <private/qinternal_p.h>
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
#ifndef QT_NO_IM
#include "qinputcontext.h"
#include "qlist.h"
#endif
#endif

#ifndef QT_NO_SHORTCUT
#include "qkeysequence.h"
#define ACCEL_KEY(k) "\t" + QString(QKeySequence(Qt::CTRL | Qt::Key_ ## k))
#else
#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)
#endif

#ifdef Q_WS_MAC
extern void qt_mac_secure_keyboard(bool); //qapplication_mac.cpp
#endif

#include <limits.h>

#define innerMargin 1

QStyleOptionFrame QLineEditPrivate::getStyleOption() const
{
    Q_Q(const QLineEdit);
    QStyleOptionFrame opt;
    opt.rect = q->rect();
    opt.palette = q->palette();
    opt.lineWidth = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    opt.midLineWidth = 0;
    opt.state = QStyle::State_None | QStyle::State_Sunken;
#ifdef QT_KEYPAD_NAVIGATION
    if (q->hasEditFocus())
        opt.state |= QStyle::State_HasEditFocus;
#endif
    return opt;
}

/*!
    \class QLineEdit
    \brief The QLineEdit widget is a one-line text editor.

    \ingroup basic
    \mainclass

    A line edit allows the user to enter and edit a single line of
    plain text with a useful collection of editing functions,
    including undo and redo, cut and paste, and drag and drop.

    By changing the echoMode() of a line edit, it can also be used as
    a "write-only" field, for inputs such as passwords.

    The length of the text can be constrained to maxLength(). The text
    can be arbitrarily constrained using a validator() or an
    inputMask(), or both.

    A related class is QTextEdit which allows multi-line, rich text
    editing.

    You can change the text with setText() or insert(). The text is
    retrieved with text(); the displayed text (which may be different,
    see \l{EchoMode}) is retrieved with displayText(). Text can be
    selected with setSelection() or selectAll(), and the selection can
    be cut(), copy()ied and paste()d. The text can be aligned with
    setAlignment().

    When the text changes the textChanged() signal is emitted; when
    the text changes other than by calling setText() the textEdited()
    signal is emitted; when the cursor is moved the
    cursorPositionChanged() signal is emitted; and when the Return or
    Enter key is pressed the returnPressed() signal is emitted.

    When editing is finished, either because the line edit lost focus
    or Return/Enter is pressed the editingFinished() signal is
    emitted.

    Note that if there is a validator set on the line edit, the
    returnPressed()/editingFinished() signals will only be emitted if
    the validator returns QValidator::Acceptable.

    By default, QLineEdits have a frame as specified by the Windows
    and Motif style guides; you can turn it off by calling
    setFrame(false).

    The default key bindings are described below. The line edit also
    provides a context menu (usually invoked by a right mouse click)
    that presents some of these editing options.
    \target desc
    \table
    \header \i Keypress \i Action
    \row \i Left Arrow \i Moves the cursor one character to the left.
    \row \i Shift+Left Arrow \i Moves and selects text one character to the left.
    \row \i Right Arrow \i Moves the cursor one character to the right.
    \row \i Shift+Right Arrow \i Moves and selects text one character to the right.
    \row \i Home \i Moves the cursor to the beginning of the line.
    \row \i End \i Moves the cursor to the end of the line.
    \row \i Backspace \i Deletes the character to the left of the cursor.
    \row \i Ctrl+Backspace \i Deletes the word to the left of the cursor.
    \row \i Delete \i Deletes the character to the right of the cursor.
    \row \i Ctrl+Delete \i Deletes the word to the right of the cursor.
    \row \i Ctrl+A \i Moves the cursor to the beginning of the line.
    \row \i Ctrl+B \i Moves the cursor one character to the left.
    \row \i Ctrl+C \i Copies the selected text to the clipboard.
    \row \i Ctrl+Insert \i Copies the selected text to the clipboard.
    \row \i Ctrl+D \i Deletes the character to the right of the cursor.
    \row \i Ctrl+E \i Moves the cursor to the end of the line.
    \row \i Ctrl+F \i Moves the cursor one character to the right.
    \row \i Ctrl+H \i Deletes the character to the left of the cursor.
    \row \i Ctrl+K \i Deletes to the end of the line.
    \row \i Ctrl+V \i Pastes the clipboard text into line edit.
    \row \i Shift+Insert \i Pastes the clipboard text into line edit.
    \row \i Ctrl+X \i Deletes the selected text and copies it to the clipboard.
    \row \i Shift+Delete \i Deletes the selected text and copies it to the clipboard.
    \row \i Ctrl+Z \i Undoes the last operation.
    \row \i Ctrl+Y \i Redoes the last undone operation.
    \endtable

    Any other key sequence that represents a valid character, will
    cause the character to be inserted into the line edit.

    \inlineimage macintosh-lineedit.png Screenshot in Macintosh style
    \inlineimage windows-lineedit.png Screenshot in Windows style

    \sa QTextEdit, QLabel, QComboBox, {fowler}{GUI Design Handbook: Field, Entry}
*/


/*!
    \fn void QLineEdit::textChanged(const QString &text)

    This signal is emitted whenever the text changes. The \a text
    argument is the new text.

    Unlike textEdited(), this signal is also emitted when
    programmatically setting the text via setText().
*/

/*!
    \fn void QLineEdit::textEdited(const QString &text)

    This signal is emitted whenever the text is edited. The \a text
    argument is the next text.

    Unlike textChanged(), this signal is \e not emitted when
    programmatically setting the text via setText().
*/

/*!
    \fn void QLineEdit::cursorPositionChanged(int old, int new)

    This signal is emitted whenever the cursor moves. The previous
    position is given by \a old, and the new position by \a new.

    \sa setCursorPosition(), cursorPosition()
*/

/*!
    \fn void QLineEdit::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa hasSelectedText(), selectedText()
*/

/*!
    Constructs a line edit with no text.

    The maximum text length is set to 32767 characters.

    The \a parent argument is sent to the QWidget constructor.

    \sa setText(), setMaxLength()
*/
QLineEdit::QLineEdit(QWidget* parent)
    : QWidget(*new QLineEditPrivate, parent,0)
{
    Q_D(QLineEdit);
    d->init(QString());
}

/*!
    Constructs a line edit containing the text \a contents.

    The cursor position is set to the end of the line and the maximum
    text length to 32767 characters.

    The \a parent and argument is sent to the QWidget
    constructor.

    \sa text(), setMaxLength()
*/
QLineEdit::QLineEdit(const QString& contents, QWidget* parent)
    : QWidget(*new QLineEditPrivate, parent, 0)
{
    Q_D(QLineEdit);
    d->init(contents);
}


#ifdef QT3_SUPPORT
/*!
    Constructs a line edit with no text.

    The maximum text length is set to 32767 characters.

    The \a parent and \a name arguments are sent to the QWidget constructor.

    \sa setText(), setMaxLength()
*/
QLineEdit::QLineEdit(QWidget* parent, const char* name)
    : QWidget(*new QLineEditPrivate, parent,0)
{
    Q_D(QLineEdit);
    setObjectName(name);
    d->init(QString());
}

/*!
    Constructs a line edit containing the text \a contents.

    The cursor position is set to the end of the line and the maximum
    text length to 32767 characters.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.

    \sa text(), setMaxLength()
*/

QLineEdit::QLineEdit(const QString& contents, QWidget* parent, const char* name)
    : QWidget(*new QLineEditPrivate, parent, 0)
{
    Q_D(QLineEdit);
    setObjectName(name);
    d->init(contents);
}

/*!
    Constructs a line edit with an input \a inputMask and the text \a
    contents.

    The cursor position is set to the end of the line and the maximum
    text length is set to the length of the mask (the number of mask
    characters and separators).

    The \a parent and \a name arguments are sent to the QWidget
    constructor.

    \sa setMask() text()
*/
QLineEdit::QLineEdit(const QString& contents, const QString &inputMask, QWidget* parent, const char* name)
    : QWidget(*new QLineEditPrivate, parent, 0)
{
    Q_D(QLineEdit);
    setObjectName(name);
    d->parseInputMask(inputMask);
    if (d->maskData) {
        QString ms = d->maskString(0, contents);
        d->init(ms + d->clearString(ms.length(), d->maxLength - ms.length()));
        d->cursor = d->nextMaskBlank(ms.length());
    } else {
        d->init(contents);
    }
}
#endif

/*!
    Destroys the line edit.
*/

QLineEdit::~QLineEdit()
{
}


/*!
    \property QLineEdit::text
    \brief the line edit's text

    Setting this property clears the selection, clears the undo/redo
    history, moves the cursor to the end of the line and resets the
    \l modified property to false. The text is not validated when
    inserted with setText().

    The text is truncated to maxLength() length.

    \sa insert(), clear()
*/
QString QLineEdit::text() const
{
    Q_D(const QLineEdit);
    QString res = d->text;
    if (d->maskData)
        res = d->stripString(d->text);
    return (res.isNull() ? QString::fromLatin1("") : res);
}

void QLineEdit::setText(const QString& text)
{
    Q_D(QLineEdit);
    d->setText(text, -1, false);
#ifdef QT_KEYPAD_NAVIGATION
    d->origText = d->text;
#endif
}


/*!
    \property QLineEdit::displayText
    \brief the displayed text

    If \l echoMode is \l Normal this returns the same as text(); if
    \l EchoMode is \l Password it returns a string of asterisks
    text().length() characters long, e.g. "******"; if \l EchoMode is
    \l NoEcho returns an empty string, "".

    \sa setEchoMode() text() EchoMode
*/

QString QLineEdit::displayText() const
{
    Q_D(const QLineEdit);
    if (d->echoMode == NoEcho)
        return QString::fromLatin1("");
    QString res = d->text;
    if (d->echoMode == Password) {
        QStyleOptionFrame opt = d->getStyleOption();
        res.fill(style()->styleHint(QStyle::SH_LineEdit_PasswordCharacter, &opt, this));
    }
    return (res.isNull() ? QString::fromLatin1("") : res);
}


/*!
    \property QLineEdit::maxLength
    \brief the maximum permitted length of the text

    If the text is too long, it is truncated at the limit.

    If truncation occurs any selected text will be unselected, the
    cursor position is set to 0 and the first part of the string is
    shown.

    If the line edit has an input mask, the mask defines the maximum
    string length.

    \sa inputMask
*/

int QLineEdit::maxLength() const
{
    Q_D(const QLineEdit);
    return d->maxLength;
}

void QLineEdit::setMaxLength(int maxLength)
{
    Q_D(QLineEdit);
    if (d->maskData)
        return;
    d->maxLength = maxLength;
    setText(d->text);
}



/*!
    \property QLineEdit::frame
    \brief whether the line edit draws itself with a frame

    If enabled (the default) the line edit draws itself inside a
    frame, otherwise the line edit draws itself without any frame.
*/
bool QLineEdit::hasFrame() const
{
    Q_D(const QLineEdit);
    return d->frame;
}


void QLineEdit::setFrame(bool enable)
{
    Q_D(QLineEdit);
    d->frame = enable;
    update();
    updateGeometry();
}


/*!
    \enum QLineEdit::EchoMode

    This enum type describes how a line edit should display its
    contents.

    \value Normal   Display characters as they are entered. This is the
                    default.
    \value NoEcho   Do not display anything. This may be appropriate
                    for passwords where even the length of the
                    password should be kept secret.
    \value Password  Display asterisks instead of the characters
                    actually entered.

    \sa setEchoMode() echoMode()
*/


/*!
    \property QLineEdit::echoMode
    \brief the line edit's echo mode

    The initial setting is \l Normal, but QLineEdit also supports \l
    NoEcho and \l Password modes.

    The widget's display and the ability to copy or drag the text is
    affected by this setting.

    \sa EchoMode displayText()
*/

QLineEdit::EchoMode QLineEdit::echoMode() const
{
    Q_D(const QLineEdit);
    return (EchoMode) d->echoMode;
}

void QLineEdit::setEchoMode(EchoMode mode)
{
    Q_D(QLineEdit);
    if(mode == (EchoMode)d->echoMode)
        return;
    d->echoMode = mode;
    d->updateTextLayout();
    update();
#ifdef Q_WS_MAC
    if (hasFocus())
        qt_mac_secure_keyboard(d->echoMode == Password || d->echoMode == NoEcho);
#endif
}


#ifndef QT_NO_VALIDATOR
/*!
    Returns a pointer to the current input validator, or 0 if no
    validator has been set.

    \sa setValidator()
*/

const QValidator * QLineEdit::validator() const
{
    Q_D(const QLineEdit);
    return d->validator;
}

/*!
    Sets this line edit to only accept input that the validator, \a v,
    will accept. This allows you to place any arbitrary constraints on
    the text which may be entered.

    If \a v == 0, setValidator() removes the current input validator.
    The initial setting is to have no input validator (i.e. any input
    is accepted up to maxLength()).

    \sa validator() QIntValidator QDoubleValidator QRegExpValidator
*/

void QLineEdit::setValidator(const QValidator *v)
{
    Q_D(QLineEdit);
    d->validator = const_cast<QValidator*>(v);
}
#endif // QT_NO_VALIDATOR

/*!
    Returns a recommended size for the widget.

    The width returned, in pixels, is usually enough for about 15 to
    20 characters.
*/

QSize QLineEdit::sizeHint() const
{
    Q_D(const QLineEdit);
    ensurePolished();
    QFontMetrics fm(font());
    int h = qMax(fm.lineSpacing(), 14) + 2*innerMargin;
    int w = fm.width('x') * 17; // "some"
    int m = d->frame ? style()->pixelMetric(QStyle::PM_DefaultFrameWidth) : 0;
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::State_None;
    return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w + (2 * m), h + (2 * m)).
                                      expandedTo(QApplication::globalStrut()), this));
}


/*!
    Returns a minimum size for the line edit.

    The width returned is enough for at least one character.
*/

QSize QLineEdit::minimumSizeHint() const
{
    Q_D(const QLineEdit);
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    int h = fm.height() + qMax(2*innerMargin, fm.leading());
    int w = fm.maxWidth();
    int m = d->frame ? style()->pixelMetric(QStyle::PM_DefaultFrameWidth) : 0;
    return QSize(w + (2 * m), h + (2 * m));
}


/*!
    \property QLineEdit::cursorPosition
    \brief the current cursor position for this line edit

    Setting the cursor position causes a repaint when appropriate.
*/

int QLineEdit::cursorPosition() const
{
    Q_D(const QLineEdit);
    return d->cursor;
}

void QLineEdit::setCursorPosition(int pos)
{
    Q_D(QLineEdit);
    if (pos < 0)
        pos = 0;

    if (pos <= d->text.length())
        d->moveCursor(pos);
}

/*!
    Returns the cursor position under the point \a pos.
*/
// ### What should this do if the point is outside of contentsRect? Currently returns 0.
int QLineEdit::cursorPositionAt(const QPoint &pos)
{
    Q_D(QLineEdit);
    return d->xToPos(pos.x());
}


#ifdef QT3_SUPPORT
/*! \obsolete

    Use setText(), setCursorPosition() and setSelection() instead.
*/
bool QLineEdit::validateAndSet(const QString &newText, int newPos,
                                 int newMarkAnchor, int newMarkDrag)
{
    Q_D(QLineEdit);
    int priorState = d->undoState;
    d->selstart = 0;
    d->selend = d->text.length();
    d->removeSelectedText();
    d->insert(newText);
    d->finishChange(priorState);
    if (d->undoState > priorState) {
        d->cursor = newPos;
        d->selstart = qMin(newMarkAnchor, newMarkDrag);
        d->selend = qMax(newMarkAnchor, newMarkDrag);
        update();
        d->emitCursorPositionChanged();
        return true;
    }
    return false;
}
#endif //QT3_SUPPORT

/*!
    \property QLineEdit::alignment
    \brief the alignment of the line edit

    Only horizontal alignments are allowed in here, Qt::AlignJustify
    will map to Qt::AlignLeft.

    \sa Qt::Alignment
*/

Qt::Alignment QLineEdit::alignment() const
{
    Q_D(const QLineEdit);
    return QFlag(d->alignment);
}

void QLineEdit::setAlignment(Qt::Alignment alignment)
{
    Q_D(QLineEdit);
    d->alignment = alignment & Qt::AlignHorizontal_Mask;
    update();
}


/*!
    Moves the cursor forward \a steps characters. If \a mark is true
    each character moved over is added to the selection; if \a mark is
    false the selection is cleared.

    \sa cursorBackward()
*/

void QLineEdit::cursorForward(bool mark, int steps)
{
    Q_D(QLineEdit);
    int cursor = d->cursor;
    if (steps > 0) {
        while(steps--)
            cursor = d->textLayout.nextCursorPosition(cursor);
    } else if (steps < 0) {
        while (steps++)
            cursor = d->textLayout.previousCursorPosition(cursor);
    }
    d->moveCursor(cursor, mark);
}


/*!
    Moves the cursor back \a steps characters. If \a mark is true each
    character moved over is added to the selection; if \a mark is
    false the selection is cleared.

    \sa cursorForward()
*/
void QLineEdit::cursorBackward(bool mark, int steps)
{
    cursorForward(mark, -steps);
}

/*!
    Moves the cursor one word forward. If \a mark is true, the word is
    also selected.

    \sa cursorWordBackward()
*/
void QLineEdit::cursorWordForward(bool mark)
{
    Q_D(QLineEdit);
    d->moveCursor(d->textLayout.nextCursorPosition(d->cursor, QTextLayout::SkipWords), mark);
}

/*!
    Moves the cursor one word backward. If \a mark is true, the word
    is also selected.

    \sa cursorWordForward()
*/

void QLineEdit::cursorWordBackward(bool mark)
{
    Q_D(QLineEdit);
    d->moveCursor(d->textLayout.previousCursorPosition(d->cursor, QTextLayout::SkipWords), mark);
}


/*!
    If no text is selected, deletes the character to the left of the
    text cursor and moves the cursor one position to the left. If any
    text is selected, the cursor is moved to the beginning of the
    selected text and the selected text is deleted.

    \sa del()
*/
void QLineEdit::backspace()
{
    Q_D(QLineEdit);
    int priorState = d->undoState;
    if (d->hasSelectedText()) {
        d->removeSelectedText();
    } else if (d->cursor) {
            --d->cursor;
            if (d->maskData)
                d->cursor = d->prevMaskBlank(d->cursor);
            QChar uc = d->text.at(d->cursor);
            if (d->cursor > 0 && uc.unicode() >= 0xdc00 && uc.unicode() < 0xe000) {
                // second half of a surrogate, check if we have the first half as well,
                // if yes delete both at once
                uc = d->text.at(d->cursor - 1);
                if (uc.unicode() >= 0xd800 && uc.unicode() < 0xdc00) {
                    d->del(true);
                    --d->cursor;
                }
            }
            d->del(true);
    }
    d->finishChange(priorState);
}

/*!
    If no text is selected, deletes the character to the right of the
    text cursor. If any text is selected, the cursor is moved to the
    beginning of the selected text and the selected text is deleted.

    \sa backspace()
*/

void QLineEdit::del()
{
    Q_D(QLineEdit);
    int priorState = d->undoState;
    if (d->hasSelectedText()) {
        d->removeSelectedText();
    } else {
        int n = d->textLayout.nextCursorPosition(d->cursor) - d->cursor;
        while (n--)
            d->del();
    }
    d->finishChange(priorState);
}

/*!
    Moves the text cursor to the beginning of the line unless it is
    already there. If \a mark is true, text is selected towards the
    first position; otherwise, any selected text is unselected if the
    cursor is moved.

    \sa end()
*/

void QLineEdit::home(bool mark)
{
    Q_D(QLineEdit);
    d->moveCursor(0, mark);
}

/*!
    Moves the text cursor to the end of the line unless it is already
    there. If \a mark is true, text is selected towards the last
    position; otherwise, any selected text is unselected if the cursor
    is moved.

    \sa home()
*/

void QLineEdit::end(bool mark)
{
    Q_D(QLineEdit);
    d->moveCursor(d->text.length(), mark);
}


/*!
    \property QLineEdit::modified
    \brief whether the line edit's contents has been modified by the user

    The modified flag is never read by QLineEdit; it has a default value
    of false and is changed to true whenever the user changes the line
    edit's contents.

    This is useful for things that need to provide a default value but
    do not start out knowing what the default should be (perhaps it
    depends on other fields on the form). Start the line edit without
    the best default, and when the default is known, if modified()
    returns false (the user hasn't entered any text), insert the
    default value.

    Calling setText() resets the modified flag to false.
*/

bool QLineEdit::isModified() const
{
    Q_D(const QLineEdit);
    return d->modifiedState != d->undoState;
}

void QLineEdit::setModified(bool modified)
{
    Q_D(QLineEdit);
    if (modified)
        d->modifiedState = -1;
    else
        d->modifiedState = d->undoState;
}


/*!\fn QLineEdit::clearModified()

Use setModified(false) instead.

    \sa isModified()
*/


/*!
    \property QLineEdit::hasSelectedText
    \brief whether there is any text selected

    hasSelectedText() returns true if some or all of the text has been
    selected by the user; otherwise returns false.

    \sa selectedText()
*/


bool QLineEdit::hasSelectedText() const
{
    Q_D(const QLineEdit);
    return d->hasSelectedText();
}

/*!
    \property QLineEdit::selectedText
    \brief the selected text

    If there is no selected text this property's value is
    an empty string.

    \sa hasSelectedText()
*/

QString QLineEdit::selectedText() const
{
    Q_D(const QLineEdit);
    if (d->hasSelectedText())
        return d->text.mid(d->selstart, d->selend - d->selstart);
    return QString();
}

/*!
    selectionStart() returns the index of the first selected character in the
    line edit or -1 if no text is selected.

    \sa selectedText()
*/

int QLineEdit::selectionStart() const
{
    Q_D(const QLineEdit);
    return d->hasSelectedText() ? d->selstart : -1;
}


#ifdef QT3_SUPPORT

/*!
    \fn void QLineEdit::lostFocus()

    This signal is emitted when the line edit has lost focus.

    Use editingFinished() instead
    \sa editingFinished(), returnPressed()
*/

/*!
    Use isModified() instead.
*/
bool QLineEdit::edited() const { return isModified(); }
/*!
    Use setModified()  or setText().
*/
void QLineEdit::setEdited(bool on) { setModified(on); }

/*!
    There exists no equivalent functionality in Qt 4.
*/
int QLineEdit::characterAt(int xpos, QChar *chr) const
{
    Q_D(const QLineEdit);
    int pos = d->xToPos(xpos + contentsRect().x() - d->hscroll + innerMargin);
    if (chr && pos < (int) d->text.length())
        *chr = d->text.at(pos);
    return pos;

}

/*!
    Use selectedText() and selectionStart() instead.
*/
bool QLineEdit::getSelection(int *start, int *end)
{
    Q_D(QLineEdit);
    if (d->hasSelectedText() && start && end) {
        *start = d->selstart;
        *end = d->selend;
        return true;
    }
    return false;
}
#endif


/*!
    Selects text from position \a start and for \a length characters.
    Negative lengths are allowed.

    \sa deselect() selectAll() selectedText()
*/

void QLineEdit::setSelection(int start, int length)
{
    Q_D(QLineEdit);
    if (start < 0 || start > (int)d->text.length()) {
        qWarning("QLineEdit: Invalid start position: %d.", start);
        return;
    } else {
        if (length > 0) {
            d->selstart = start;
            d->selend = qMin(start + length, (int)d->text.length());
            d->cursor = d->selend;
        } else {
            d->selstart = qMax(start + length, 0);
            d->selend = start;
            d->cursor = d->selstart;
        }
    }
    update();
    d->emitCursorPositionChanged();
}


/*!
    \property QLineEdit::undoAvailable
    \brief whether undo is available
*/

bool QLineEdit::isUndoAvailable() const
{
    Q_D(const QLineEdit);
    return d->isUndoAvailable();
}

/*!
    \property QLineEdit::redoAvailable
    \brief whether redo is available
*/

bool QLineEdit::isRedoAvailable() const
{
    Q_D(const QLineEdit);
    return d->isRedoAvailable();
}

/*!
    \property QLineEdit::dragEnabled
    \brief whether the lineedit starts a drag if the user presses and
    moves the mouse on some selected text

    Dragging is disabled by default.
*/

bool QLineEdit::dragEnabled() const
{
    Q_D(const QLineEdit);
    return d->dragEnabled;
}

void QLineEdit::setDragEnabled(bool b)
{
    Q_D(QLineEdit);
    d->dragEnabled = b;
}


/*!
    \property QLineEdit::acceptableInput
    \brief whether the input satisfies the inputMask and the
    validator.

    \sa setInputMask(), setValidator()
*/
bool QLineEdit::hasAcceptableInput() const
{
    Q_D(const QLineEdit);
    return d->hasAcceptableInput(d->text);
}

/*!
    \property QLineEdit::inputMask
    \brief The validation input mask

    If no mask is set, inputMask() returns an empty string.

    Sets the QLineEdit's validation mask. Validators can be used
    instead of, or in conjunction with masks; see setValidator().

    Unset the mask and return to normal QLineEdit operation by passing
    an empty string ("") or just calling setInputMask() with no
    arguments.

    The mask format understands these mask characters:
    \table
    \header \i Character \i Meaning
    \row \i \c A \i ASCII alphabetic character required. A-Z, a-z.
    \row \i \c a \i ASCII alphabetic character permitted but not required.
    \row \i \c N \i ASCII alphanumeric character required. A-Z, a-z, 0-9.
    \row \i \c n \i ASCII alphanumeric character permitted but not required.
    \row \i \c X \i Any character required.
    \row \i \c x \i Any character permitted but not required.
    \row \i \c 9 \i ASCII digit required. 0-9.
    \row \i \c 0 \i ASCII digit permitted but not required.
    \row \i \c D \i ASCII digit required. 1-9.
    \row \i \c d \i ASCII digit permitted but not required (1-9).
    \row \i \c # \i ASCII digit or plus/minus sign permitted but not required.
    \row \i \c > \i All following alphabetic characters are uppercased.
    \row \i \c < \i All following alphabetic characters are lowercased.
    \row \i \c ! \i Switch off case conversion.
    \row \i \tt{\\} \i Use \tt{\\} to escape the special
                           characters listed above to use them as
                           separators.
    \endtable

    The mask consists of a string of mask characters and separators,
    optionally followed by a semicolon and the character used for
    blanks: the blank characters are always removed from the text
    after editing. The default blank character is space.

    Examples:
    \table
    \header \i Mask \i Notes
    \row \i \c 000.000.000.000;_ \i IP address; blanks are \c{_}.
    \row \i \c 0000-00-00 \i ISO Date; blanks are \c space
    \row \i \c >AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;# \i License number;
    blanks are \c - and all (alphabetic) characters are converted to
    uppercase.
    \endtable

    To get range control (e.g. for an IP address) use masks together
    with \link setValidator() validators\endlink.

    \sa maxLength
*/
QString QLineEdit::inputMask() const
{
    Q_D(const QLineEdit);
    return (d->maskData ? d->inputMask + ';' + d->blank : QString());
}

void QLineEdit::setInputMask(const QString &inputMask)
{
    Q_D(QLineEdit);
    d->parseInputMask(inputMask);
    if (d->maskData)
        d->moveCursor(d->nextMaskBlank(0));
}

/*!
    Selects all the text (i.e. highlights it) and moves the cursor to
    the end. This is useful when a default value has been inserted
    because if the user types before clicking on the widget, the
    selected text will be deleted.

    \sa setSelection() deselect()
*/

void QLineEdit::selectAll()
{
    Q_D(QLineEdit);
    d->selstart = d->selend = d->cursor = 0;
    d->moveCursor(d->text.length(), true);
}

/*!
    Deselects any selected text.

    \sa setSelection() selectAll()
*/

void QLineEdit::deselect()
{
    Q_D(QLineEdit);
    d->deselect();
    d->finishChange();
}


/*!
    Deletes any selected text, inserts \a newText, and validates the
    result. If it is valid, it sets it as the new contents of the line
    edit.

    \sa setText(), clear()
*/
void QLineEdit::insert(const QString &newText)
{
//     q->resetInputContext(); //#### FIX ME IN QT
    Q_D(QLineEdit);
    int priorState = d->undoState;
    d->removeSelectedText();
    d->insert(newText);
    d->finishChange(priorState);
}

/*!
    Clears the contents of the line edit.

    \sa setText(), insert()
*/
void QLineEdit::clear()
{
    Q_D(QLineEdit);
    int priorState = d->undoState;
    resetInputContext();
    d->selstart = 0;
    d->selend = d->text.length();
    d->removeSelectedText();
    d->separate();
    d->finishChange(priorState);
}

/*!
    Undoes the last operation if undo is \link
    QLineEdit::undoAvailable available\endlink. Deselects any current
    selection, and updates the selection start to the current cursor
    position.
*/
void QLineEdit::undo()
{
    Q_D(QLineEdit);
    resetInputContext();
    d->undo();
    d->finishChange(-1, true);
}

/*!
    Redoes the last operation if redo is \link
    QLineEdit::redoAvailable available\endlink.
*/
void QLineEdit::redo()
{
    Q_D(QLineEdit);
    resetInputContext();
    d->redo();
    d->finishChange();
}


/*!
    \property QLineEdit::readOnly
    \brief whether the line edit is read only.

    In read-only mode, the user can still copy the text to the
    clipboard, or drag and drop the text (if echoMode() is \l Normal),
    but cannot edit it.

    QLineEdit does not show a cursor in read-only mode.

    \sa setEnabled()
*/

bool QLineEdit::isReadOnly() const
{
    Q_D(const QLineEdit);
    return d->readOnly;
}

void QLineEdit::setReadOnly(bool enable)
{
    Q_D(QLineEdit);
    d->readOnly = enable;
    update();
}


#ifndef QT_NO_CLIPBOARD
/*!
    Copies the selected text to the clipboard and deletes it, if there
    is any, and if echoMode() is \l Normal.

    If the current validator disallows deleting the selected text,
    cut() will copy without deleting.

    \sa copy() paste() setValidator()
*/

void QLineEdit::cut()
{
    if (hasSelectedText()) {
        copy();
        del();
    }
}


/*!
    Copies the selected text to the clipboard, if there is any, and if
    echoMode() is \l Normal.

    \sa cut() paste()
*/

void QLineEdit::copy() const
{
    Q_D(const QLineEdit);
    d->copy();
}

/*!
    Inserts the clipboard's text at the cursor position, deleting any
    selected text, providing the line edit is not \link
    QLineEdit::readOnly read-only\endlink.

    If the end result would not be acceptable to the current
    \link setValidator() validator\endlink, nothing happens.

    \sa copy() cut()
*/

void QLineEdit::paste()
{
    insert(QApplication::clipboard()->text(QClipboard::Clipboard));
}

void QLineEditPrivate::copy(bool clipboard) const
{
    Q_Q(const QLineEdit);
    QString t = q->selectedText();
    if (!t.isEmpty() && echoMode == QLineEdit::Normal) {
        q->disconnect(QApplication::clipboard(), SIGNAL(selectionChanged()), q, 0);
        QApplication::clipboard()->setText(t, clipboard ? QClipboard::Clipboard : QClipboard::Selection);
        q->connect(QApplication::clipboard(), SIGNAL(selectionChanged()),
                   q, SLOT(clipboardChanged()));
    }
}

#endif // !QT_NO_CLIPBOARD

/*! \reimp
*/
bool QLineEdit::event(QEvent * e)
{
    Q_D(QLineEdit);
    if (e->type() == QEvent::ShortcutOverride && !d->readOnly) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if (ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier
             || ke->modifiers() == Qt::KeypadModifier) {
            if (ke->key() < Qt::Key_Escape) {
                ke->accept();
            } else {
                switch (ke->key()) {
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
// Those are too frequently used for application functionality
/*            case Qt::Key_A:
            case Qt::Key_B:
            case Qt::Key_D:
            case Qt::Key_E:
            case Qt::Key_F:
            case Qt::Key_H:
            case Qt::Key_K:
*/
            case Qt::Key_C:
            case Qt::Key_V:
            case Qt::Key_X:
            case Qt::Key_Y:
            case Qt::Key_Z:
            case Qt::Key_Left:
            case Qt::Key_Right:
#if !defined(Q_WS_MAC)
            case Qt::Key_Insert:
            case Qt::Key_Delete:
#endif
                ke->accept();
            default:
                break;
            }
        }
    } else if (e->type() == QEvent::Timer) {
        // should be timerEvent, is here for binary compatibility
        int timerId = ((QTimerEvent*)e)->timerId();
        if (timerId == d->cursorTimer) {
            QStyleOptionFrame opt = d->getStyleOption();
            if(!hasSelectedText()
               || style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, this))
                d->setCursorVisible(!d->cursorVisible);
#ifndef QT_NO_DRAGANDDROP
        } else if (timerId == d->dndTimer.timerId()) {
            d->drag();
#endif
        }
        else if (timerId == d->tripleClickTimer.timerId())
            d->tripleClickTimer.stop();
#ifdef QT_KEYPAD_NAVIGATION
        else if (timerId == d->deleteAllTimer.timerId()) {
            d->deleteAllTimer.stop();
            clear();
        }
#endif
    } else if (e->type() == QEvent::ContextMenu) {
#ifndef QT_NO_IM
        if (d->composeMode())
            return true;
#endif
        d->separate();
    }
#ifdef QT_KEYPAD_NAVIGATION
    else if (e->type() == QEvent::KeyRelease) {
        if (QApplication::keypadNavigationEnabled()) {
            QKeyEvent *ke = (QKeyEvent *)e;
            if ( !ke->isAutoRepeat() && !isReadOnly()
                    && ke->key() == Qt::Key_Back
                    && d->deleteAllTimer.isActive()) {
                d->deleteAllTimer.stop();
                backspace();
            }
        }
    }
#endif
    return QWidget::event(e);
}

/*! \reimp
*/
void QLineEdit::mousePressEvent(QMouseEvent* e)
{
    Q_D(QLineEdit);
    if (d->sendMouseEventToInputContext(e))
	return;
    if (e->button() == Qt::RightButton)
        return;
#ifdef QT_KEYPAD_NAVIGATION
    if (QApplication::keypadNavigationEnabled() && !hasEditFocus())
        setEditFocus(true);
#endif
    if (d->tripleClickTimer.isActive() && (e->pos() - d->tripleClick).manhattanLength() <
         QApplication::startDragDistance()) {
        selectAll();
        return;
    }
    bool mark = e->modifiers() & Qt::ShiftModifier;
    int cursor = d->xToPos(e->pos().x());
#ifndef QT_NO_DRAGANDDROP
    if (!mark && d->dragEnabled && d->echoMode == Normal &&
         e->button() == Qt::LeftButton && d->inSelection(e->pos().x())) {
        d->cursor = cursor;
        update();
        d->dndPos = e->pos();
        if (!d->dndTimer.isActive())
            d->dndTimer.start(QApplication::startDragTime(), this);
        d->emitCursorPositionChanged();
    } else
#endif
    {
        d->moveCursor(cursor, mark);
    }
}

/*! \reimp
*/
void QLineEdit::mouseMoveEvent(QMouseEvent * e)
{
    Q_D(QLineEdit);
    if (d->sendMouseEventToInputContext(e))
	return;

    if (e->buttons() & Qt::LeftButton) {
#ifndef QT_NO_DRAGANDDROP
        if (d->dndTimer.isActive()) {
            if ((d->dndPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
                d->drag();
        } else
#endif
        {
            d->moveCursor(d->xToPos(e->pos().x()), true);
        }
    }
}

/*! \reimp
*/
void QLineEdit::mouseReleaseEvent(QMouseEvent* e)
{
    Q_D(QLineEdit);
    if (d->sendMouseEventToInputContext(e))
	return;
#ifndef QT_NO_DRAGANDDROP
    if (e->button() == Qt::LeftButton) {
        if (d->dndTimer.isActive()) {
            d->dndTimer.stop();
            deselect();
            return;
        }
    }
#endif
#ifndef QT_NO_CLIPBOARD
    if (QApplication::clipboard()->supportsSelection()) {
        if (e->button() == Qt::LeftButton) {
            d->copy(false);
        } else if (!d->readOnly && e->button() == Qt::MidButton) {
            d->deselect();
            insert(QApplication::clipboard()->text(QClipboard::Selection));
        }
    }
#endif
}

/*! \reimp
*/
void QLineEdit::mouseDoubleClickEvent(QMouseEvent* e)
{
    Q_D(QLineEdit);
    if (d->sendMouseEventToInputContext(e))
	return;
    if (e->button() == Qt::LeftButton) {
        deselect();
        d->cursor = d->xToPos(e->pos().x());
        d->cursor = d->textLayout.previousCursorPosition(d->cursor, QTextLayout::SkipWords);
        // ## text layout should support end of words.
        int end = d->textLayout.nextCursorPosition(d->cursor, QTextLayout::SkipWords);
        while (end > d->cursor && d->text[end-1].isSpace())
            --end;
        d->moveCursor(end, true);
        d->tripleClickTimer.start(QApplication::doubleClickInterval(), this);
        d->tripleClick = e->pos();
    }
}

/*!
    \fn void  QLineEdit::returnPressed()

    This signal is emitted when the Return or Enter key is pressed.
    Note that if there is a validator() or inputMask() set on the line
    edit, the returnPressed() signal will only be emitted if the input
    follows the inputMask() and the validator() returns
    QValidator::Acceptable.
*/

/*!
    \fn void  QLineEdit::editingFinished()

    This signal is emitted when the Return or Enter key is pressed or
    the line edit looses focus. Note that if there is a validator() or
    inputMask() set on the line edit and enter/return is pressed, the
    editingFinished() signal will only be emitted if the input follows
    the inputMask() and the validator() returns QValidator::Acceptable.
*/

/*!
    Converts the given key press \a event into a line edit action.

    If Return or Enter is pressed and the current text is valid (or
    can be \link QValidator::fixup() made valid\endlink by the
    validator), the signal returnPressed() is emitted.

    The default key bindings are listed in the class's detailed
    description.
*/

void QLineEdit::keyPressEvent(QKeyEvent *event)
{
    Q_D(QLineEdit);
#ifdef QT_KEYPAD_NAVIGATION
    bool select = false;
    switch (event->key()) {
        case Qt::Key_Select:
            if (QApplication::keypadNavigationEnabled()) {
                if (hasEditFocus()) {
                    setEditFocus(false);
                    select = true;
                }
            }
            break;
        case Qt::Key_Back:
        case Qt::Key_No:
            if (!QApplication::keypadNavigationEnabled() || !hasEditFocus()) {
                event->ignore();
                return;
            }
            break;
        default:
            if (QApplication::keypadNavigationEnabled()) {
                if (!hasEditFocus() && !(event->modifiers() & Qt::ControlModifier)) {
                    if (event->text()[0].isPrint()) {
                        setEditFocus(true);
                        clear();
                    } else {
                        event->ignore();
                        return;
                    }
                }
            }
    }

    if (QApplication::keypadNavigationEnabled() && !select && !hasEditFocus()) {
        setEditFocus(true);
        if (event->key() == Qt::Key_Select)
            return; // Just start. No action.
    }
#endif

    d->setCursorVisible(true);
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (hasAcceptableInput()) {
            emit returnPressed();
            emit editingFinished();
        }
#ifndef QT_NO_VALIDATOR
        else {
            const QValidator * v = d->validator;
            QString textCopy = d->text;
            int cursorCopy = d->cursor;
            if (v && v->validate(textCopy, cursorCopy) != QValidator::Acceptable)
                v->fixup(textCopy);

            if (d->hasAcceptableInput(textCopy)) {
                if (v && (textCopy != d->text || cursorCopy != d->cursor))
                    d->setText(textCopy, cursorCopy);
                emit returnPressed();
                emit editingFinished();
            }
        }
#endif
        event->ignore();
        return;
    }
    bool unknown = false;

    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_A:
#if defined(Q_WS_X11)
            home(event->modifiers() & Qt::ShiftModifier);
#else
            selectAll();
#endif
            break;
        case Qt::Key_B:
            cursorForward(event->modifiers() & Qt::ShiftModifier, -1);
            break;
#ifndef QT_NO_CLIPBOARD
        case Qt::Key_C:
            copy();
            break;
#endif
        case Qt::Key_D:
            if (!d->readOnly)
                del();
            break;
        case Qt::Key_E:
            end(event->modifiers() & Qt::ShiftModifier);
            break;
        case Qt::Key_F:
            cursorForward(event->modifiers() & Qt::ShiftModifier, 1);
            break;
        case Qt::Key_H:
            if (!d->readOnly)
                backspace();
            break;
        case Qt::Key_K:
            if (!d->readOnly) {
                setSelection(d->cursor, d->text.size());
#ifndef QT_NO_CLIPBOARD
                copy();
#endif
                del();
            }
            break;
#if defined(Q_WS_X11)
        case Qt::Key_U:
            if (!d->readOnly) {
                setSelection(0, d->text.size());
#ifndef QT_NO_CLIPBOARD
                copy();
#endif
                del();
            }
            break;
#endif
#ifndef QT_NO_CLIPBOARD
        case Qt::Key_V:
            if (!d->readOnly)
                paste();
            break;
#endif
        case Qt::Key_X:
            if (!d->readOnly) {
#ifndef QT_NO_CLIPBOARD
                copy();
#endif
                del();
            }
            break;
#if !defined(Q_WS_MAC) && !defined(QT_NO_CLIPBOARD)
        case Qt::Key_Insert:
            copy();
            break;
#endif
        case Qt::Key_Delete:
            if (!d->readOnly) {
                cursorWordForward(true);
                del();
            }
            break;
        case Qt::Key_Backspace:
            if (!d->readOnly) {
                cursorWordBackward(true);
                del();
            }
            break;
        case Qt::Key_Right:
        case Qt::Key_Left:
            if ((layoutDirection() == Qt::RightToLeft) == (event->key() == Qt::Key_Right)) {
#ifndef Q_WS_MAC
                if (echoMode() == Normal)
                    cursorWordBackward(event->modifiers() & Qt::ShiftModifier);
                else
#endif
                    home(event->modifiers() & Qt::ShiftModifier);
            } else {
#ifndef Q_WS_MAC
                if (echoMode() == Normal)
                    cursorWordForward(event->modifiers() & Qt::ShiftModifier);
                else
#endif
                    end(event->modifiers() & Qt::ShiftModifier);
            }
            break;
        case Qt::Key_Z:
            if (!d->readOnly) {
                if(event->modifiers() & Qt::ShiftModifier)
                    redo();
                else
                    undo();
            }
            break;
        case Qt::Key_Y:
            if (!d->readOnly)
                redo();
            break;
        default:
            unknown = true;
        }
    } else { // ### check for *no* modifier
        switch (event->key()) {
        case Qt::Key_Shift:
            // ### TODO
            break;
        case Qt::Key_Left:
        case Qt::Key_Right: {
            int step =  ((layoutDirection() == Qt::RightToLeft) == (event->key() == Qt::Key_Right)) ? -1 : 1;
#ifdef Q_WS_MAC
            if (event->modifiers() & Qt::AltModifier) {
                if (step < 0)
                    cursorWordBackward(event->modifiers() & Qt::ShiftModifier);
                else
                    cursorWordForward(event->modifiers() & Qt::ShiftModifier);
            } else if (event->modifiers() & Qt::MetaModifier) {
                if (step < 0)
                    home(event->modifiers() & Qt::ShiftModifier);
                else
                    end(event->modifiers() & Qt::ShiftModifier);
            } else
#endif
            {
                cursorForward(event->modifiers() & Qt::ShiftModifier, step);
            }
        }
        break;
        case Qt::Key_Backspace:
            if (!d->readOnly)
                backspace();
            break;
        case Qt::Key_Home:
#ifdef Q_WS_MAC
            break; // Home and End do nothing on the mac (but Up and Down do).
        case Qt::Key_Up:
#endif
            home(event->modifiers() & Qt::ShiftModifier);
            break;
        case Qt::Key_End:
#ifdef Q_WS_MAC
            break;
        case Qt::Key_Down:
#endif
            end(event->modifiers() & Qt::ShiftModifier);
            break;
        case Qt::Key_Delete:
#if !defined(QT_NO_CLIPBOARD)
            if (!d->readOnly) {
#if !defined(Q_WS_MAC)
                if (event->modifiers() & Qt::ShiftModifier) {
                    cut();
                    break;
                }
#endif
                del();
            }
#endif
            break;
#if !defined(Q_WS_MAC) && !defined(QT_NO_CLIPBOARD)
        case Qt::Key_Insert:
            if (!d->readOnly && event->modifiers() & Qt::ShiftModifier)
                paste();
            else
                unknown = true;
            break;
#endif
        case Qt::Key_F14: // Undo key on Sun keyboards
            if (!d->readOnly)
                undo();
            break;
#ifndef QT_NO_CLIPBOARD
        case Qt::Key_F16: // Copy key on Sun keyboards
            copy();
            break;
        case Qt::Key_F18: // Paste key on Sun keyboards
            if (!d->readOnly)
                paste();
            break;
        case Qt::Key_F20: // Cut key on Sun keyboards
            if (!d->readOnly) {
                copy();
                del();
            }
            break;
#endif
#ifdef QT_KEYPAD_NAVIGATION
        case Qt::Key_Back:
            if (QApplication::keypadNavigationEnabled() && !event->isAutoRepeat()
                && !isReadOnly()) {
                if (text().length() == 0) {
                    setText(d->origText);
                    setEditFocus(false);
                } else if (!d->deleteAllTimer.isActive()) {
                    d->deleteAllTimer.start(750, this);
                }
            } else {
                unknown = true;
            }
            break;
#endif
        default:
            unknown = true;
        }
    }
    if (event->key() == Qt::Key_Direction_L || event->key() == Qt::Key_Direction_R) {
        setLayoutDirection((event->key() == Qt::Key_Direction_L) ? Qt::LeftToRight : Qt::RightToLeft);
        d->updateTextLayout();
        update();
        unknown = false;
    }

    if (unknown && !d->readOnly) {
        QString t = event->text();
        if (!t.isEmpty() && t.at(0).isPrint()) {
            insert(t);
            event->accept();
            return;
        }
    }

    if (unknown)
        event->ignore();
    else
        event->accept();
}

/*!
  This function is not intended as polymorphic usage. Just a shared code
  fragment that calls QInputContext::mouseHandler for this
  class.
*/
bool QLineEditPrivate::sendMouseEventToInputContext( QMouseEvent *e )
{
    // ##### currently X11 only
#if (defined(Q_WS_X11) || defined(Q_WS_QWS)) && !defined QT_NO_IM
    Q_Q(QLineEdit);
    if ( composeMode() ) {
	int tmp_cursor = xToPosInternal( e->pos().x(), QTextLine::CursorOnCharacter );
	int mousePos = tmp_cursor - cursor;
	if ( mousePos < 0 || mousePos >= textLayout.preeditAreaText().length() ) {
            mousePos = -1;
	    // don't send move events outside the preedit area
            if ( e->type() == QEvent::MouseMove )
                return true;
        }

        QInputContext *qic = q->inputContext();
        if ( qic )
            // may be causing reset() in some input methods
            qic->mouseHandler(mousePos, e);
        if (!textLayout.preeditAreaText().isEmpty())
            return true;
    }
#else
    Q_UNUSED(e);
#endif

    return false;
}

/*! \reimp
 */
void QLineEdit::inputMethodEvent(QInputMethodEvent *e)
{
    Q_D(QLineEdit);
    if (d->readOnly) {
        e->ignore();
        return;
    }

    int priorState = d->undoState;
    d->removeSelectedText();

    int c = d->cursor; // cursor position after insertion of commit string
    if (e->replacementStart() <= 0)
        c += e->commitString().length() + qMin(-e->replacementStart(), e->replacementLength());

    d->cursor += e->replacementStart();

    // insert commit string
    if (e->replacementLength()) {
        d->selstart = d->cursor;
        d->selend = d->selstart + e->replacementLength();
        d->removeSelectedText();
    }
    if (!e->commitString().isEmpty())
        d->insert(e->commitString());

    d->cursor = c;

    d->textLayout.setPreeditArea(d->cursor, e->preeditString());
    QList<QTextLayout::FormatRange> formats;
    for (int i = 0; i < e->attributes().size(); ++i) {
        const QInputMethodEvent::Attribute &a = e->attributes().at(i);
        if (a.type != QInputMethodEvent::TextFormat)
            continue;
        QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
        if (f.isValid()) {
            QTextLayout::FormatRange o;
            o.start = a.start + d->cursor;
            o.length = a.length;
            o.format = f;
            formats.append(o);
        }
    }
    d->textLayout.setAdditionalFormats(formats);
    d->updateTextLayout();
    update();
    if (!e->commitString().isEmpty())
        d->emitCursorPositionChanged();
    d->finishChange(priorState);
}

/*!\reimp
*/
QVariant QLineEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
    Q_D(const QLineEdit);
    switch(property) {
    case Qt::ImMicroFocus:
        return d->cursorRect();
    case Qt::ImFont:
        return font();
    case Qt::ImCursorPosition:
        return QVariant(d->cursor);
    case Qt::ImSurroundingText:
        return QVariant(d->text);
    case Qt::ImCurrentSelection:
        return QVariant(selectedText());
    default:
        return QVariant();
    }
}

/*!\reimp
*/

void QLineEdit::focusInEvent(QFocusEvent *e)
{
    Q_D(QLineEdit);
    if (e->reason() == Qt::TabFocusReason ||
         e->reason() == Qt::BacktabFocusReason  ||
         e->reason() == Qt::ShortcutFocusReason)
        if (d->maskData)
            d->moveCursor(d->nextMaskBlank(0));
        else if (!d->hasSelectedText())
            selectAll();
    if (!d->cursorTimer) {
        int cft = QApplication::cursorFlashTime();
        d->cursorTimer = cft ? startTimer(cft/2) : -1;
    }
    QStyleOptionFrame opt = d->getStyleOption();
    if((!hasSelectedText() && d->textLayout.preeditAreaText().isEmpty())
       || style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, this))
        d->setCursorVisible(true);
#ifdef Q_WS_MAC
    if (d->echoMode == Password || d->echoMode == NoEcho)
        qt_mac_secure_keyboard(true);
#endif
#ifdef QT_KEYPAD_NAVIGATION
    d->origText = d->text;
#endif
    update();
}

/*!\reimp
*/

void QLineEdit::focusOutEvent(QFocusEvent *e)
{
    Q_D(QLineEdit);
    if (e->reason() != Qt::ActiveWindowFocusReason &&
         e->reason() != Qt::PopupFocusReason
         && !(e->reason() == Qt::MouseFocusReason
            && QApplication::activePopupWidget()
            && QApplication::activePopupWidget()->parentWidget() == this))
        deselect();
    d->setCursorVisible(false);
    if (d->cursorTimer > 0)
        killTimer(d->cursorTimer);
    d->cursorTimer = 0;
    if (e->reason() != Qt::PopupFocusReason
        && !(QApplication::activePopupWidget() && QApplication::activePopupWidget()->parentWidget() == this)) {
        emit editingFinished();
#ifdef QT3_SUPPORT
        emit lostFocus();
#endif
    }
#ifdef Q_WS_MAC
    if (d->echoMode == Password || d->echoMode == NoEcho)
        qt_mac_secure_keyboard(false);
#endif
#ifdef QT_KEYPAD_NAVIGATION
    d->origText = QString();
#endif
    update();
}

/*!\reimp
*/
void QLineEdit::paintEvent(QPaintEvent *)
{
    Q_D(QLineEdit);
    QPainter p(this);

    QRect r = rect();
    const QPalette &pal = palette();

    if (d->frame) {
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        QStyleOptionFrame opt;
        opt.init(this);
        opt.lineWidth = frameWidth;
        opt.midLineWidth = 0;
        opt.state |= QStyle::State_Sunken;
        style()->drawPrimitive(QStyle::PE_FrameLineEdit, &opt, &p, this);

        r.adjust(frameWidth, frameWidth, -frameWidth, -frameWidth);
    }
    p.setClipRect(r);

    QStyleOptionFrame panel = d->getStyleOption();
    panel.state |= (isEnabled() ? QStyle::State_Enabled : QStyle::State_None);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

    QFontMetrics fm = fontMetrics();
    QRect lineRect(r.x() + innerMargin, r.y() + (r.height() - fm.height() + 1) / 2,
                    r.width() - 2*innerMargin, fm.height());
    QTextLine line = d->textLayout.lineAt(0);

    // locate cursor position
    int cix = qRound(line.cursorToX(d->cursor));

    // horizontal scrolling
    int minLB = qMax(0, -fm.minLeftBearing());
    int minRB = qMax(0, -fm.minRightBearing());


    int widthUsed = qRound(line.naturalTextWidth()) + 1 + minRB;
    if ((minLB + widthUsed) <=  lineRect.width()) {
        Qt::Alignment va = QStyle::visualAlignment(layoutDirection(), QFlag(d->alignment));
        va &= ~(Qt::AlignAbsolute|Qt::AlignVertical_Mask);
        switch (va) {
        case Qt::AlignRight:
            d->hscroll = widthUsed - lineRect.width() + 1;
            break;
        case Qt::AlignHCenter:
            d->hscroll = (widthUsed - lineRect.width()) / 2;
            break;
        default:
            // Left
            d->hscroll = 0;
            break;
        }
        d->hscroll -= minLB;
    } else if (cix - d->hscroll >= lineRect.width()) {
        d->hscroll = cix - lineRect.width() + 1;
    } else if (cix - d->hscroll < 0) {
        d->hscroll = cix;
    } else if (widthUsed - d->hscroll < lineRect.width()) {
        d->hscroll = widthUsed - lineRect.width() + 1;
    }
    // the y offset is there to keep the baseline constant in case we have script changes in the text.
    QPoint topLeft = lineRect.topLeft() - QPoint(d->hscroll, d->ascent-fm.ascent());

    // draw text, selections and cursors
    p.setPen(pal.text().color());
    bool supressCursor = d->readOnly;

    QVector<QTextLayout::FormatRange> selections;
    if (d->selstart < d->selend || (d->cursorVisible && d->maskData)) {
        QTextLayout::FormatRange o;
        const QPalette &pal = palette();
        if (d->selstart < d->selend) {
            o.start = d->selstart;
            o.length = d->selend - d->selstart;
            o.format.setBackground(pal.brush(QPalette::Highlight));
            o.format.setForeground(pal.brush(QPalette::HighlightedText));
        } else {
            // mask selection
            o.start = d->cursor;
            o.length = 1;
            o.format.setBackground(pal.brush(QPalette::Text));
            o.format.setForeground(pal.brush(QPalette::Background));
        }
        selections.append(o);
    }

    // Asian users see an IM selection text as cursor on candidate
    // selection phase of input method, so the ordinary cursor should be
    // invisible if we have a preedit string.
    bool showCursor = (d->cursorVisible && !supressCursor && !d->textLayout.preeditAreaText().length());
    d->textLayout.draw(&p, topLeft, selections, r);
    if (showCursor && d->textLayout.preeditAreaText().isEmpty())
        d->textLayout.drawCursor(&p, topLeft, d->cursor);

}


#ifndef QT_NO_DRAGANDDROP
/*!\reimp
*/
void QLineEdit::dragMoveEvent(QDragMoveEvent *e)
{
    Q_D(QLineEdit);
    if (!d->readOnly && e->mimeData()->hasFormat("text/plain")) {
        e->acceptProposedAction();
        d->cursor = d->xToPos(e->pos().x());
        d->cursorVisible = true;
        update();
        d->emitCursorPositionChanged();
    }
}

/*!\reimp */
void QLineEdit::dragEnterEvent(QDragEnterEvent * e)
{
    QLineEdit::dragMoveEvent(e);
}

/*!\reimp */
void QLineEdit::dragLeaveEvent(QDragLeaveEvent *)
{
    Q_D(QLineEdit);
    if (d->cursorVisible) {
        d->cursorVisible = false;
        update();
    }
}

/*!\reimp */
void QLineEdit::dropEvent(QDropEvent* e)
{
    Q_D(QLineEdit);
    QString str = e->mimeData()->text();

    if (!str.isNull() && !d->readOnly) {
        if (e->source() == this && e->dropAction() == Qt::CopyAction)
            deselect();
        d->cursor =d->xToPos(e->pos().x());
        int selStart = d->cursor;
        int oldSelStart = d->selstart;
        int oldSelEnd = d->selend;
        d->cursorVisible = false;
        e->acceptProposedAction();
        insert(str);
        if (e->source() == this) {
            if (e->dropAction() == Qt::MoveAction) {
                if (selStart > oldSelStart && selStart <= oldSelEnd)
                    setSelection(oldSelStart, str.length());
                else if (selStart > oldSelEnd)
                    setSelection(selStart - str.length(), str.length());
                else
                    setSelection(selStart, str.length());
            } else {
                setSelection(selStart, str.length());
            }
        }
    } else {
        e->ignore();
        update();
    }
}

void QLineEditPrivate::drag()
{
    Q_Q(QLineEdit);
    dndTimer.stop();
    QMimeData *data = new QMimeData;
    data->setText(q->selectedText());
    QDrag *drag = new QDrag(q);
    drag->setMimeData(data);
    Qt::DropAction action = drag->start();
    if (action == Qt::MoveAction && !readOnly && drag->target() != q) {
        int priorState = undoState;
        removeSelectedText();
        finishChange(priorState);
    }
}

#endif // QT_NO_DRAGANDDROP

#ifndef QT_NO_MENU
/*!
    Shows the standard context menu created with
    createStandardContextMenu().

    If you do not want the line edit to have a context menu, you can set
    its \l contextMenuPolicy to Qt::NoContextMenu. If you want to
    customize the context menu, reimplement this function. If you want
    to extend the standard context menu, reimplement this function, call
    createStandardContextMenu() and extend the menu returned.

    \code
        void LineEdit::contextMenuEvent(QContextMenuEvent *event)
        {
            QMenu *menu = createStandardContextMenu();
            menu->addAction(tr("My Menu Item"));
            //...
            menu->exec(event->globalPos());
            delete menu;
        }
    \endcode

    The \a event parameter is used to obtain the position where
    the mouse cursor was when the event was generated.

    \sa setContextMenuPolicy()
*/
void QLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->exec(event->globalPos());
    delete menu;
}

/*!  This function creates the standard context menu which is shown
  when the user clicks on the line edit with the right mouse
  button. It is called from the default contextMenuEvent() handler.
  The popup menu's ownership is transferred to the caller.
*/

QMenu *QLineEdit::createStandardContextMenu()
{
    Q_D(QLineEdit);
    d->actions[QLineEditPrivate::UndoAct]->setEnabled(d->isUndoAvailable());
    d->actions[QLineEditPrivate::RedoAct]->setEnabled(d->isRedoAvailable());
#ifndef QT_NO_CLIPBOARD
    d->actions[QLineEditPrivate::CutAct]->setEnabled(!d->readOnly && d->hasSelectedText());
    d->actions[QLineEditPrivate::CopyAct]->setEnabled(d->hasSelectedText());
    d->actions[QLineEditPrivate::PasteAct]->setEnabled(!d->readOnly && !QApplication::clipboard()->text().isEmpty());
#else
    d->actions[QLineEditPrivate::CutAct]->setEnabled(false);
    d->actions[QLineEditPrivate::CopyAct]->setEnabled(false);
    d->actions[QLineEditPrivate::PasteAct]->setEnabled(false);
#endif
    d->actions[QLineEditPrivate::ClearAct]->setEnabled(!d->readOnly && !d->text.isEmpty());
    d->actions[QLineEditPrivate::SelectAllAct]->setEnabled(!d->text.isEmpty() && !d->allSelected());

    QMenu *popup = new QMenu(this);
    popup->setObjectName("qt_edit_menu");
    popup->addAction(d->actions[QLineEditPrivate::UndoAct]);
    popup->addAction(d->actions[QLineEditPrivate::RedoAct]);
    popup->addSeparator();
    popup->addAction(d->actions[QLineEditPrivate::CutAct]);
    popup->addAction(d->actions[QLineEditPrivate::CopyAct]);
    popup->addAction(d->actions[QLineEditPrivate::PasteAct]);
    popup->addAction(d->actions[QLineEditPrivate::ClearAct]);
    popup->addSeparator();
    popup->addAction(d->actions[QLineEditPrivate::SelectAllAct]);
#if (defined(Q_WS_X11) || defined(Q_WS_QWS)) && !defined(QT_NO_IM)
    QInputContext *qic = inputContext();
    if (qic) {
        QList<QAction *> imActions = qic->actions();
        for (int i = 0; i < imActions.size(); ++i)
            popup->addAction(imActions.at(i));
    }
#endif
    return popup;
}
#endif // QT_NO_MENU

/*! \reimp */
void QLineEdit::changeEvent(QEvent *ev)
{
    Q_D(QLineEdit);
    if(ev->type() == QEvent::ActivationChange) {
        if (!palette().isEqual(QPalette::Active, QPalette::Inactive))
            update();
    } else if (ev->type() == QEvent::FontChange) {
        d->updateTextLayout();
    }
    QWidget::changeEvent(ev);
}

void QLineEditPrivate::clipboardChanged()
{
}

void QLineEditPrivate::init(const QString& txt)
{
    Q_Q(QLineEdit);
#ifndef QT_NO_CURSOR
    q->setCursor(Qt::IBeamCursor);
#endif
    q->setFocusPolicy(Qt::StrongFocus);
    q->setAttribute(Qt::WA_InputMethodEnabled);
    //   Specifies that this widget can use more, but is able to survive on
    //   less, horizontal space; and is fixed vertically.
    q->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    q->setBackgroundRole(QPalette::Base);
    q->setAttribute(Qt::WA_KeyCompression);
    q->setMouseTracking(true);
    q->setAcceptDrops(true);
    text = txt;
    updateTextLayout();
    cursor = text.length();

#ifndef QT_NO_MENU
    actions[UndoAct] = new QAction(q->tr("&Undo") + ACCEL_KEY(Z), q);
    QObject::connect(actions[UndoAct], SIGNAL(triggered()), q, SLOT(undo()));
    actions[RedoAct] = new QAction(q->tr("&Redo") + ACCEL_KEY(Y), q);
    QObject::connect(actions[RedoAct], SIGNAL(triggered()), q, SLOT(redo()));
    //popup->insertSeparator();
    actions[CutAct] = new QAction(q->tr("Cu&t") + ACCEL_KEY(X), q);
    QObject::connect(actions[CutAct], SIGNAL(triggered()), q, SLOT(cut()));
    actions[CopyAct] = new QAction(q->tr("&Copy") + ACCEL_KEY(C), q);
    QObject::connect(actions[CopyAct], SIGNAL(triggered()), q, SLOT(copy()));
    actions[PasteAct] = new QAction(q->tr("&Paste") + ACCEL_KEY(V), q);
    QObject::connect(actions[PasteAct], SIGNAL(triggered()), q, SLOT(paste()));
    actions[ClearAct] = new QAction(q->tr("Clear"), q);
    QObject::connect(actions[ClearAct], SIGNAL(triggered()), q, SLOT(clear()));
    //popup->insertSeparator();
    actions[SelectAllAct] = new QAction(q->tr("Select All")
#ifndef Q_WS_X11
                                        + ACCEL_KEY(A)
#endif
                                        , q);
    QObject::connect(actions[SelectAllAct], SIGNAL(triggered()), q, SLOT(selectAll()));
#endif // QT_NO_MENU
}

void QLineEditPrivate::updateTextLayout()
{
    // replace all non-printable characters with spaces (to avoid
    // drawing boxes when using fonts that don't have glyphs for such
    // characters)
    Q_Q(QLineEdit);
    QString str = q->displayText();
    QChar* uc = str.data();
    for (int i = 0; i < (int)str.length(); ++i) {
        if (! uc[i].isPrint())
            uc[i] = QChar(0x0020);
    }
    textLayout.setFont(q->font());
    textLayout.setText(str);
    QTextOption option;
    option.setTextDirection(q->layoutDirection());
    option.setFlags(QTextOption::IncludeTrailingSpaces);
    textLayout.setTextOption(option);

    textLayout.beginLayout();
    QTextLine l = textLayout.createLine();
    textLayout.endLayout();
    ascent = qRound(l.ascent());
}

int QLineEditPrivate::xToPosInternal(int x, QTextLine::CursorPosition betweenOrOn) const
{
    Q_Q(const QLineEdit);
    x-= q->contentsRect().x() - hscroll + innerMargin;
    QTextLine l = textLayout.lineAt(0);
    if (x >= 0 && x < l.naturalTextWidth())
        return l.xToCursor(x, betweenOrOn);
    return x < 0 ? -1 : text.length();
}


int QLineEditPrivate::xToPos(int x, QTextLine::CursorPosition betweenOrOn) const
{
    int pos = xToPosInternal(x, betweenOrOn);
    return (pos < 0) ? 0 : pos;
}


QRect QLineEditPrivate::cursorRect() const
{
    Q_Q(const QLineEdit);
    QRect cr = q->contentsRect();
    int frameWidth = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int cix = cr.x() + frameWidth - hscroll + innerMargin;
    QTextLine l = textLayout.lineAt(0);
    cix += qRound(l.cursorToX(cursor));
    int ch = qMin(cr.height(), q->fontMetrics().height() + 1);
    return QRect(cix-5, cr.y() + (cr.height() -  ch) / 2, 10, ch);
}

void QLineEditPrivate::moveCursor(int pos, bool mark)
{
    Q_Q(QLineEdit);
    if (pos != cursor) {
        separate();
        if (maskData)
            pos = pos > cursor ? nextMaskBlank(pos) : prevMaskBlank(pos);
    }
    bool fullUpdate = mark || hasSelectedText();
    if (mark) {
        int anchor;
        if (selend > selstart && cursor == selstart)
            anchor = selend;
        else if (selend > selstart && cursor == selend)
            anchor = selstart;
        else
            anchor = cursor;
        selstart = qMin(anchor, pos);
        selend = qMax(anchor, pos);
        updateTextLayout();
    } else {
        deselect();
    }
    if (fullUpdate) {
        cursor = pos;
        q->update();
    } else {
        setCursorVisible(false);
        cursor = pos;
        setCursorVisible(true);
    }
    QStyleOptionFrame opt = getStyleOption();
    if (mark && !q->style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, q))
        setCursorVisible(false);
    if (mark || selDirty) {
        selDirty = false;
        emit q->selectionChanged();
    }
    emitCursorPositionChanged();
}

void QLineEditPrivate::finishChange(int validateFromState, bool update, bool edited)
{
    Q_Q(QLineEdit);
    bool lineDirty = selDirty;
    if (textDirty) {
        // do validation
        bool wasValidInput = validInput;
        validInput = true;
#ifndef QT_NO_VALIDATOR
        if (validator && validateFromState >= 0) {
            QString textCopy = text;
            int cursorCopy = cursor;
            validInput = (validator->validate(textCopy, cursorCopy) != QValidator::Invalid);
            if (validInput) {
                if (text != textCopy) {
                    setText(textCopy, cursorCopy);
                    return;
                }
                cursor = cursorCopy;
            }
        }
#endif
        if (validateFromState >= 0 && wasValidInput && !validInput) {
            undo(validateFromState);
            history.resize(undoState);
            if (modifiedState > undoState)
                modifiedState = -1;
            validInput = true;
            textDirty = false;
        }
        updateTextLayout();
        lineDirty |= textDirty;
        if (textDirty) {
            textDirty = false;
            QString actualText = maskData ? stripString(text) : text;
            if (edited)
                emit q->textEdited(actualText);
            q->updateMicroFocus();
            emit q->textChanged(actualText);
        }
#ifndef QT_NO_ACCESSIBILITY
        QAccessible::updateAccessibility(q, 0, QAccessible::ValueChanged);
#endif
    }
    if (selDirty) {
        selDirty = false;
        emit q->selectionChanged();
    }
    if (lineDirty || update)
        q->update();
    emitCursorPositionChanged();
}

void QLineEditPrivate::emitCursorPositionChanged()
{
    Q_Q(QLineEdit);
    if (cursor != lastCursorPos) {
        const int oldLast = lastCursorPos;
        lastCursorPos = cursor;
        emit q->cursorPositionChanged(oldLast, cursor);
    }
}

void QLineEditPrivate::setText(const QString& txt, int pos, bool edited)
{
    Q_Q(QLineEdit);
    q->resetInputContext();
    deselect();
    QString oldText = text;
    if (maskData) {
        text = maskString(0, txt, true);
        text += clearString(text.length(), maxLength - text.length());
    } else {
        text = txt.isEmpty() ? txt : txt.left(maxLength);
    }
    history.clear();
    modifiedState =  undoState = 0;
    cursor = (pos < 0 || pos > text.length()) ? text.length() : pos;
    textDirty = (oldText != text);
    finishChange(-1, true, edited);
}


void QLineEditPrivate::setCursorVisible(bool visible)
{
    Q_Q(QLineEdit);
    if ((bool)cursorVisible == visible)
        return;
    if (cursorTimer)
        cursorVisible = visible;
    QRect r = cursorRect();
    if (maskData || !q->contentsRect().contains(r))
        q->update();
    else
        q->update(r);
}

void QLineEditPrivate::addCommand(const Command& cmd)
{
    if (separator && undoState && history[undoState-1].type != Separator) {
        history.resize(undoState + 2);
        history[undoState++] = Command(Separator, 0, 0);
    } else {
        history.resize(undoState + 1);
    }
    separator = false;
    history[undoState++] = cmd;
}

void QLineEditPrivate::insert(const QString& s)
{
    if (maskData) {
        QString ms = maskString(cursor, s);
        for (int i = 0; i < (int) ms.length(); ++i) {
            addCommand (Command(DeleteSelection, cursor+i, text.at(cursor+i)));
            addCommand(Command(Insert, cursor+i, ms.at(i)));
        }
        text.replace(cursor, ms.length(), ms);
        cursor += ms.length();
        cursor = nextMaskBlank(cursor);
    } else {
        int remaining = maxLength - text.length();
        text.insert(cursor, s.left(remaining));
        for (int i = 0; i < (int) s.left(remaining).length(); ++i)
            addCommand(Command(Insert, cursor++, s.at(i)));
    }
    textDirty = true;
}

void QLineEditPrivate::del(bool wasBackspace)
{
    if (cursor < (int) text.length()) {
        addCommand (Command((CommandType)((maskData?2:0)+(wasBackspace?Remove:Delete)), cursor, text.at(cursor)));
        if (maskData) {
            text.replace(cursor, 1, clearString(cursor, 1));
            addCommand(Command(Insert, cursor, text.at(cursor)));
        } else {
            text.remove(cursor, 1);
        }
        textDirty = true;
    }
}

void QLineEditPrivate::removeSelectedText()
{
    if (selstart < selend && selend <= (int) text.length()) {
        separate();
        int i ;
        if (selstart <= cursor && cursor < selend) {
            // cursor is within the selection. Split up the commands
            // to be able to restore the correct cursor position
            for (i = cursor; i >= selstart; --i)
                addCommand (Command(DeleteSelection, i, text.at(i)));
            for (i = selend - 1; i > cursor; --i)
                addCommand (Command(DeleteSelection, i - cursor + selstart - 1, text.at(i)));
        } else {
            for (i = selend-1; i >= selstart; --i)
                addCommand (Command(RemoveSelection, i, text.at(i)));
        }
        if (maskData) {
            text.replace(selstart, selend - selstart,  clearString(selstart, selend - selstart));
            for (int i = 0; i < selend - selstart; ++i)
                addCommand(Command(Insert, selstart + i, text.at(selstart + i)));
        } else {
            text.remove(selstart, selend - selstart);
        }
        if (cursor > selstart)
            cursor -= qMin(cursor, selend) - selstart;
        deselect();
        textDirty = true;
    }
}

void QLineEditPrivate::parseInputMask(const QString &maskFields)
{
    int delimiter = maskFields.indexOf(';');
    if (maskFields.isEmpty() || delimiter == 0) {
        if (maskData) {
            delete [] maskData;
            maskData = 0;
            maxLength = 32767;
            setText(QString());
        }
        return;
    }

    if (delimiter == -1) {
        blank = ' ';
        inputMask = maskFields;
    } else {
        inputMask = maskFields.left(delimiter);
        blank = (delimiter + 1 < maskFields.length()) ? maskFields[delimiter + 1] : QChar(' ');
    }

    // calculate maxLength / maskData length
    maxLength = 0;
    QChar c = 0;
    for (int i=0; i<inputMask.length(); i++) {
        c = inputMask.at(i);
        if (i > 0 && inputMask.at(i-1) == '\\') {
            maxLength++;
            continue;
        }
        if (c != '\\' && c != '!' &&
             c != '<' && c != '>' &&
             c != '{' && c != '}' &&
             c != '[' && c != ']')
            maxLength++;
    }

    delete [] maskData;
    maskData = new MaskInputData[maxLength];

    MaskInputData::Casemode m = MaskInputData::NoCaseMode;
    c = 0;
    bool s;
    bool escape = false;
    int index = 0;
    for (int i = 0; i < inputMask.length(); i++) {
        c = inputMask.at(i);
        if (escape) {
            s = true;
            maskData[index].maskChar = c;
            maskData[index].separator = s;
            maskData[index].caseMode = m;
            index++;
            escape = false;
        } else if (c == '<') {
                m = MaskInputData::Lower;
        } else if (c == '>') {
            m = MaskInputData::Upper;
        } else if (c == '!') {
            m = MaskInputData::NoCaseMode;
        } else if (c != '{' && c != '}' && c != '[' && c != ']') {
            switch (c.unicode()) {
            case 'A':
            case 'a':
            case 'N':
            case 'n':
            case 'X':
            case 'x':
            case '9':
            case '0':
            case 'D':
            case 'd':
            case '#':
                s = false;
                break;
            case '\\':
                escape = true;
            default:
                s = true;
                break;
            }

            if (!escape) {
                maskData[index].maskChar = c;
                maskData[index].separator = s;
                maskData[index].caseMode = m;
                index++;
            }
        }
    }
    setText(text);
}


/* checks if the key is valid compared to the inputMask */
bool QLineEditPrivate::isValidInput(QChar key, QChar mask) const
{
    switch (mask.unicode()) {
    case 'A':
        if (key.isLetter())
            return true;
        break;
    case 'a':
        if (key.isLetter() || key == blank)
            return true;
        break;
    case 'N':
        if (key.isLetterOrNumber())
            return true;
        break;
    case 'n':
        if (key.isLetterOrNumber() || key == blank)
            return true;
        break;
    case 'X':
        if (key.isPrint())
            return true;
        break;
    case 'x':
        if (key.isPrint() || key == blank)
            return true;
        break;
    case '9':
        if (key.isNumber())
            return true;
        break;
    case '0':
        if (key.isNumber() || key == blank)
            return true;
        break;
    case 'D':
        if (key.isNumber() && key.digitValue() > 0)
            return true;
        break;
    case 'd':
        if ((key.isNumber() && key.digitValue() > 0) || key == blank)
            return true;
        break;
    case '#':
        if (key.isNumber() || key == '+' || key == '-' || key == blank)
            return true;
        break;
    default:
        break;
    }
    return false;
}

bool QLineEditPrivate::hasAcceptableInput(const QString &str) const
{
#ifndef QT_NO_VALIDATOR
    QString textCopy = str;
    int cursorCopy = cursor;
    if (validator && validator->validate(textCopy, cursorCopy)
        != QValidator::Acceptable)
        return false;
#endif

    if (!maskData)
        return true;

    if (str.length() != maxLength)
        return false;

    for (int i=0; i < maxLength; ++i) {
        if (maskData[i].separator) {
            if (str.at(i) != maskData[i].maskChar)
                return false;
        } else {
            if (!isValidInput(str.at(i), maskData[i].maskChar))
                return false;
        }
    }
    return true;
}

/*
  Applies the inputMask on \a str starting from position \a pos in the mask. \a clear
  specifies from where characters should be gotten when a separator is met in \a str - true means
  that blanks will be used, false that previous input is used.
  Calling this when no inputMask is set is undefined.
*/
QString QLineEditPrivate::maskString(uint pos, const QString &str, bool clear) const
{
    if (pos >= (uint)maxLength)
        return QString::fromLatin1("");

    QString fill;
    fill = clear ? clearString(0, maxLength) : text;

    int strIndex = 0;
    QString s = QString::fromLatin1("");
    int i = pos;
    while (i < maxLength) {
        if (strIndex < str.length()) {
            if (maskData[i].separator) {
                s += maskData[i].maskChar;
                if (str[(int)strIndex] == maskData[i].maskChar)
                    strIndex++;
                ++i;
            } else {
                if (isValidInput(str[(int)strIndex], maskData[i].maskChar)) {
                    switch (maskData[i].caseMode) {
                    case MaskInputData::Upper:
                        s += str[(int)strIndex].toUpper();
                        break;
                    case MaskInputData::Lower:
                        s += str[(int)strIndex].toLower();
                        break;
                    default:
                        s += str[(int)strIndex];
                    }
                    ++i;
                } else {
                    // search for separator first
                    int n = findInMask(i, true, true, str[(int)strIndex]);
                    if (n != -1) {
                        if (str.length() != 1 || i == 0 || (i > 0 && (!maskData[i-1].separator || maskData[i-1].maskChar != str[(int)strIndex]))) {
                            s += fill.mid(i, n-i+1);
                            i = n + 1; // update i to find + 1
                        }
                    } else {
                        // search for valid blank if not
                        n = findInMask(i, true, false, str[(int)strIndex]);
                        if (n != -1) {
                            s += fill.mid(i, n-i);
                            switch (maskData[n].caseMode) {
                            case MaskInputData::Upper:
                                s += str[(int)strIndex].toUpper();
                                break;
                            case MaskInputData::Lower:
                                s += str[(int)strIndex].toLower();
                                break;
                            default:
                                s += str[(int)strIndex];
                            }
                            i = n + 1; // updates i to find + 1
                        }
                    }
                }
                strIndex++;
            }
        } else
            break;
    }

    return s;
}



/*
  Returns a "cleared" string with only separators and blank chars.
  Calling this when no inputMask is set is undefined.
*/
QString QLineEditPrivate::clearString(uint pos, uint len) const
{
    if (pos >= (uint)maxLength)
        return QString();

    QString s;
    int end = qMin((uint)maxLength, pos + len);
    for (int i=pos; i<end; i++)
        if (maskData[i].separator)
            s += maskData[i].maskChar;
        else
            s += blank;

    return s;
}

/*
  Strips blank parts of the input in a QLineEdit when an inputMask is set,
  separators are still included. Typically "127.0__.0__.1__" becomes "127.0.0.1".
*/
QString QLineEditPrivate::stripString(const QString &str) const
{
    if (!maskData)
        return str;

    QString s;
    int end = qMin(maxLength, (int)str.length());
    for (int i=0; i < end; i++)
        if (maskData[i].separator)
            s += maskData[i].maskChar;
        else
            if (str[i] != blank)
                s += str[i];

    return s;
}

/* searches forward/backward in maskData for either a separator or a blank */
int QLineEditPrivate::findInMask(int pos, bool forward, bool findSeparator, QChar searchChar) const
{
    if (pos >= maxLength || pos < 0)
        return -1;

    int end = forward ? maxLength : -1;
    int step = forward ? 1 : -1;
    int i = pos;

    while (i != end) {
        if (findSeparator) {
            if (maskData[i].separator && maskData[i].maskChar == searchChar)
                return i;
        } else {
            if (!maskData[i].separator) {
                if (searchChar.isNull())
                    return i;
                else if (isValidInput(searchChar, maskData[i].maskChar))
                    return i;
            }
        }
        i += step;
    }
    return -1;
}

void QLineEditPrivate::undo(int until)
{
    if (!isUndoAvailable())
        return;
    deselect();
    while (undoState && undoState > until) {
        Command& cmd = history[--undoState];
        switch (cmd.type) {
        case Insert:
            text.remove(cmd.pos, 1);
            cursor = cmd.pos;
            break;
        case Remove:
        case RemoveSelection:
            text.insert(cmd.pos, cmd.uc);
            cursor = cmd.pos + 1;
            break;
        case Delete:
        case DeleteSelection:
            text.insert(cmd.pos, cmd.uc);
            cursor = cmd.pos;
            break;
        case Separator:
            continue;
        }
        if (until < 0 && undoState) {
            Command& next = history[undoState-1];
            if (next.type != cmd.type && next.type < RemoveSelection
                 && !(cmd.type >= RemoveSelection && next.type != Separator))
                break;
        }
    }
    textDirty = true;
    emitCursorPositionChanged();
}

void QLineEditPrivate::redo() {
    if (!isRedoAvailable())
        return;
    deselect();
    while (undoState < (int)history.size()) {
        Command& cmd = history[undoState++];
        switch (cmd.type) {
        case Insert:
            text.insert(cmd.pos, cmd.uc);
            cursor = cmd.pos + 1;
            break;
        case Remove:
        case Delete:
        case RemoveSelection:
        case DeleteSelection:
            text.remove(cmd.pos, 1);
            cursor = cmd.pos;
            break;
        case Separator:
            continue;
        }
        if (undoState < (int)history.size()) {
            Command& next = history[undoState];
            if (next.type != cmd.type && cmd.type < RemoveSelection
                 && !(next.type >= RemoveSelection && cmd.type != Separator))
                break;
        }
    }
    textDirty = true;
    emitCursorPositionChanged();
}

/*!
    \fn void QLineEdit::repaintArea(int a, int b)

    Use update() instead.
*/

/*!
    \fn void QLineEdit::cursorLeft(bool mark, int steps)

    Use cursorForward() with a negative number of steps instead. For
    example, cursorForward(mark, -steps).
*/

/*!
    \fn void QLineEdit::cursorRight(bool mark, int steps)

    Use cursorForward() instead.
*/

/*!
    \fn bool QLineEdit::frame() const

    Use hasFrame() instead.
*/

/*!
    \fn void QLineEdit::clearValidator()

    Use setValidator(0) instead.
*/

/*!
    \fn bool QLineEdit::hasMarkedText() const

    Use hasSelectedText() instead.
*/

/*!
    \fn QString QLineEdit::markedText() const

    Use selectedText() instead.
*/


#include "moc_qlineedit.cpp"

#endif // QT_NO_LINEEDIT
