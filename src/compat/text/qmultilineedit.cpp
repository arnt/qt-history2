/****************************************************************************
**
** Implementation of QMultiLineEdit widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmultilineedit.h"
#ifndef QT_NO_MULTILINEEDIT
#include "qpainter.h"
#include "qscrollbar.h"
#include "qcursor.h"
#include "qclipboard.h"
#include "qpixmap.h"
#include "qregexp.h"
#include "qapplication.h"
#include "qdragobject.h"
#include "qpopupmenu.h"
#include "qtimer.h"
#include <private/qrichtext_p.h>


/*!
  \class QMultiLineEdit qmultilineedit.h
  \obsolete

  \brief The QMultiLineEdit widget is a simple editor for inputting text.

  \ingroup advanced

  The QMultiLineEdit was a simple editor widget in former Qt versions.  Qt
  3.0 includes a new richtext engine which obsoletes QMultiLineEdit. It is
  still included for compatibility reasons. It is now a subclass of
  \l Q3TextEdit, and provides enough of the old QMultiLineEdit API to keep old
  applications working.

  If you implement something new with QMultiLineEdit, we suggest using
  \l Q3TextEdit instead and call Q3TextEdit::setTextFormat(Qt::PlainText).

  Although most of the old QMultiLineEdit API is still available, there is
  a few difference. The old QMultiLineEdit operated on lines, not on
  paragraphs.  As lines change all the time during wordwrap, the new
  richtext engine uses paragraphs as basic elements in the data structure.
  All functions (numLines(), textLine(), etc.) that operated on lines, now
  operate on paragraphs. Further, getString() has been removed completely.
  It revealed too much of the internal data structure.

  Applications which made normal and reasonable use of QMultiLineEdit
  should still work without problems. Some odd usage will require some
  porting. In these cases, it may be better to use \l Q3TextEdit now.

  \sa Q3TextEdit
*/

/*!
    \fn bool QMultiLineEdit::autoUpdate() const
    \obsolete
*/

/*!
    \fn virtual void QMultiLineEdit::setAutoUpdate(bool)
    \obsolete
*/

/*!
    \fn int QMultiLineEdit::totalWidth() const
    \obsolete
*/

/*!
    \fn int QMultiLineEdit::totalHeight() const
    \obsolete
*/

/*!
    \fn int QMultiLineEdit::maxLines() const
    \obsolete
*/

/*!
    \fn void QMultiLineEdit::setMaxLines(int)
    \obsolete
*/

/*!
    \fn void QMultiLineEdit::deselect()
    \obsolete
*/


class QMultiLineEditData
{
};


/*!
  Constructs a new, empty, QMultiLineEdit with parent \a parent called
  \a name.
*/

QMultiLineEdit::QMultiLineEdit(QWidget *parent , const char *name)
    : Q3TextEdit(parent, name)
{
    d = new QMultiLineEditData;
    setTextFormat(Qt::PlainText);
}

/*! \property QMultiLineEdit::numLines
  \brief the number of paragraphs in the editor

  The count includes any empty paragraph at top and bottom, so for an
  empty editor this method returns 1.
*/

int QMultiLineEdit::numLines() const
{
    return document()->lastParagraph()->paragId() + 1;
}

/*! \property QMultiLineEdit::atEnd
  \brief whether the cursor is placed at the end of the text

  \sa atBeginning
*/

bool QMultiLineEdit::atEnd() const
{
    return textCursor()->paragraph() == document()->lastParagraph() && textCursor()->atParagEnd();
}


/*! \property QMultiLineEdit::atBeginning
  \brief whether the cursor is placed at the beginning of the text

  \sa atEnd
*/

bool QMultiLineEdit::atBeginning() const
{
    return textCursor()->paragraph() == document()->firstParagraph() && textCursor()->atParagStart();
}

/*!  Returns the number of characters at paragraph number \a row. If
  \a row is out of range, -1 is returned.
*/

int QMultiLineEdit::lineLength(int row) const
{
    if (row < 0 || row > numLines())
        return -1;
    return document()->paragAt(row)->length() - 1;
}


/*! Destructor. */

QMultiLineEdit::~QMultiLineEdit()
{
    delete d;
}

/*!
  If there is selected text, sets \a line1, \a col1, \a line2 and \a col2
  to the start and end of the selected region and returns true. Returns
  false if there is no selected text.
 */
bool QMultiLineEdit::getMarkedRegion(int *line1, int *col1,
                                      int *line2, int *col2) const
{
    int p1,c1, p2, c2;
    getSelection(&p1, &c1, &p2, &c2);
    if (p1 == -1 && c1 == -1 && p2 == -1 && c2 == -1)
        return false;
    if (line1)
        *line1 = p1;
    if (col1)
        *col1 = c1;
    if (line2)
        *line2 = p2;
    if (col2)
        *col2 = c2;
    return true;
}


/*!
  Returns true if there is selected text.
*/

bool QMultiLineEdit::hasMarkedText() const
{
    return hasSelectedText();
}


/*!
  Returns a copy of the selected text.
*/

QString QMultiLineEdit::markedText() const
{
    return selectedText();
}

/*!
  Moves the cursor one page down.  If \a mark is true, the text
  is selected.
*/

void QMultiLineEdit::pageDown(bool mark)
{
    moveCursor(MoveDown, mark);
}


/*!
  Moves the cursor one page up.  If \a mark is true, the text
  is selected.
*/

void QMultiLineEdit::pageUp(bool mark)
{
    moveCursor(MovePgUp, mark);
}


/*!  Inserts \a txt at paragraph number \a line. If \a line is less
  than zero, or larger than the number of paragraphs, the new text is
  put at the end.  If \a txt contains newline characters, several
  paragraphs are inserted.

  The cursor position is not changed.
*/

void QMultiLineEdit::insertLine(const QString &txt, int line)
{
    insertParagraph(txt, line);
}

/*!  Deletes the paragraph at paragraph number \a paragraph. If \a
  paragraph is less than zero or larger than the number of paragraphs,
  nothing is deleted.
*/

void QMultiLineEdit::removeLine(int paragraph)
{
    removeParagraph(paragraph);
}

/*!  Inserts \a str at the current cursor position and selects the
  text if \a mark is true.
*/

void QMultiLineEdit::insertAndMark(const QString& str, bool mark)
{
    insert(str);
    if (mark)
        document()->setSelectionEnd(Q3TextDocument::Standard, *textCursor());
}

/*!  Splits the paragraph at the current cursor position.
*/

void QMultiLineEdit::newLine()
{
    insert("\n");
}


/*!  Deletes the character on the left side of the text cursor and
  moves the cursor one position to the left. If a text has been selected
  by the user (e.g. by clicking and dragging) the cursor is put at the
  beginning of the selected text and the selected text is removed.  \sa
  del()
*/

void QMultiLineEdit::backspace()
{
    if (document()->hasSelection(Q3TextDocument::Standard)) {
        removeSelectedText();
        return;
    }

    if (!textCursor()->paragraph()->prev() &&
         textCursor()->atParagStart())
        return;

    doKeyboardAction(ActionBackspace);
}


/*!  Moves the text cursor to the left end of the line. If \a mark is
  true, text is selected toward the first position. If it is false and the
  cursor is moved, all selected text is unselected.

  \sa end()
*/

void QMultiLineEdit::home(bool mark)
{
    moveCursor(MoveLineStart, mark);
}

/*!  Moves the text cursor to the right end of the line. If \a mark is
  true, text is selected toward the last position.  If it is false and the
  cursor is moved, all selected text is unselected.

  \sa home()
*/

void QMultiLineEdit::end(bool mark)
{
    moveCursor(MoveLineEnd, mark);
}


/*!
  \fn void QMultiLineEdit::setCursorPosition(int line, int col)
  \reimp
*/

/*!  Sets the cursor position to character number \a col in paragraph
  number \a line.  The parameters are adjusted to lie within the legal
  range.

  If \a mark is false, the selection is cleared. otherwise it is extended.

*/

void QMultiLineEdit::setCursorPosition(int line, int col, bool mark)
{
    if (!mark)
        selectAll(false);
    Q3TextEdit::setCursorPosition(line, col);
    if (mark)
        document()->setSelectionEnd(Q3TextDocument::Standard, *textCursor());
}

/*!  Returns the top center point where the cursor is drawn.
*/

QPoint QMultiLineEdit::cursorPoint() const
{
    return QPoint(textCursor()->x(), textCursor()->y() + textCursor()->paragraph()->rect().y());
}

/*!  \property QMultiLineEdit::alignment
  \brief The editor's paragraph alignment

  Sets the alignment to flag, which must be \c Qt::AlignLeft, \c
  Qt::AlignHCenter or \c Qt::AlignRight.

  If flag is an illegal flag, nothing happens.
*/
void QMultiLineEdit::setAlignment(Qt::Alignment flag)
{
    if (flag == Qt::AlignCenter)
        flag = Qt::AlignHCenter;
    if (flag != Qt::AlignLeft && flag != Qt::AlignRight && flag != Qt::AlignHCenter)
        return;
    Q3TextParagraph *p = document()->firstParagraph();
    while (p) {
        p->setAlignment(flag);
        p = p->next();
    }
}

Qt::Alignment QMultiLineEdit::alignment() const
{
    return QFlag(document()->firstParagraph()->alignment());
}


void QMultiLineEdit::setEdited(bool e)
{
    setModified(e);
}

/*!  \property QMultiLineEdit::edited
  \brief whether the document has been edited by the user

  This is the same as Q3TextEdit's "modifed" property.
*/
bool QMultiLineEdit::edited() const
{
    return isModified();
}

/*!  Moves the cursor one word to the right.  If \a mark is true, the text
  is selected.

  \sa cursorWordBackward()
*/
void QMultiLineEdit::cursorWordForward(bool mark)
{
    moveCursor(MoveWordForward, mark);
}

/*!  Moves the cursor one word to the left.  If \a mark is true, the
  text is selected.

  \sa cursorWordForward()
*/
void QMultiLineEdit::cursorWordBackward(bool mark)
{
    moveCursor(MoveWordBackward, mark);
}

/*!
  \fn QMultiLineEdit::insertAt(const QString &s, int line, int col)
  \reimp
*/

/*!  Inserts string \a s at paragraph number \a line, after character
  number \a col in the paragraph.  If \a s contains newline
  characters, new lines are inserted.
  If \a mark is true the inserted string will be selected.

  The cursor position is adjusted.
 */

void QMultiLineEdit::insertAt(const QString &s, int line, int col, bool mark)
{
    Q3TextEdit::insertAt(s, line, col);
    if (mark)
        setSelection(line, col, line, col + s.length());
}

// ### reggie - is this documentation correct?

/*!  Deletes text from the current cursor position to the end of the
  line. (Note that this function still operates on lines, not paragraphs.)
*/

void QMultiLineEdit::killLine()
{
    doKeyboardAction(ActionKill);
}

/*!  Moves the cursor one character to the left. If \a mark is true,
  the text is selected.
  The \a wrap parameter is currently ignored.

  \sa cursorRight() cursorUp() cursorDown()
*/

void QMultiLineEdit::cursorLeft(bool mark, bool)
{
    moveCursor(MoveBackward, mark);
}

/*!  Moves the cursor one character to the right.  If \a mark is true,
  the text is selected.
  The \a wrap parameter is currently ignored.

  \sa cursorLeft() cursorUp() cursorDown()
*/

void QMultiLineEdit::cursorRight(bool mark, bool)
{
    moveCursor(MoveForward, mark);
}

/*!  Moves the cursor up one line.  If \a mark is true, the text is
  selected.

  \sa cursorDown() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorUp(bool mark)
{
    moveCursor(MoveUp, mark);
}

/*!
  Moves the cursor one line down.  If \a mark is true, the text
  is selected.
  \sa cursorUp() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorDown(bool mark)
{
    moveCursor(MoveDown, mark);
}


/*!  Returns the text at line number \a line (possibly the empty
  string), or a null if \a line is invalid.
*/

QString QMultiLineEdit::textLine(int line) const
{
    if (line < 0 || line >= numLines())
        return QString::null;
    QString str = document()->paragAt(line)->string()->toString();
    str.truncate(str.length() - 1);
    return str;
}

#endif
