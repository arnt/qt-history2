/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.h#6 $
**
** Definition of the QTextEdit class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtextedit.h"

#ifndef QT_NO_TEXTEDIT

#include "qcursor.h"

/*!
  \class QTextEdit qtextedit.h
  \brief The QTextEdit widget provides a sophisticated single-page rich text editor.

  \ingroup basic

    QTextEdit is an advanced WYSIWYG editor supporting rich text
    formatting. It is optimized to handle large documents and to respond
    quickly to user input.

    If you create a new QTextEdit, and want to allow the user to
    edit rich text, call setTextFormat(Qt::RichText) to ensure that the
    text is treated as rich text. (Rich text uses HTML tags to set text
    formatting attributes. See QStyleSheet for information on the HTML
    tags that are supported.)

    The text edit documentation uses the following concepts:
    <ul>
    <li><i>current format</i> --
    this is the format at the current cursor position, \e and it
    is the format of the selected text if any.
    <li><i>current paragraph</i> -- the paragraph which contains the
    cursor.
    </ul>

    QTextEdit extends QTextView with keyboard and mouse handling for
    user input. QTextEdit provides functions to change the text and its
    formatting.

    The text is set or replaced using setText() which deletes any
    existing text and replaces it with the text passed in the setText()
    call. Text can be inserted with insert(), paste() and
    pasteSubType(). Text can also be cut(). The entire text is deleted
    with clear() and the selected text is deleted with
    removeSelectedText(). Selected (marked) text can also be deleted
    with del() (which will delete the character to the right of the
    cursor if no text is selected).

    The current format's attributes are set with setItalic(), setBold(),
    setUnderline(), setFamily() (font family), setPointSize(),
    setColor() and setCurrentFont().  The current
    paragraph's style is set with setParagType() and its alignment is
    set with setAlignment().

    Internally QTextEdit works on paragraphs and characters. A paragraph
    is a formatted string which is word-wrapped to fit into the width of
    the widget. Paragraphs are separated by hard line breaks. Each
    character has its own attributes, for example, font and color.

    Use setSelection() to programmatically select text. The
    setSelectionAttributes() function is used to set how selected text
    should be displayed. The currently selected text's position is
    available using QTextView::getSelection() and the selected text
    itself is returned by QTextView::selectedText(). The selection can
    be copied to the clipboard with copy(), or cut to the clipboard with
    cut(). It can be deleted with removeSelectedText(). The entire text
    can be selected (or unselected) using selectAll().

    Set and get the position of the cursor with setCursorPosition() and
    getCursorPosition() respectively. When the cursor is moved, the
    signals currentFontChanged(), currentColorChanged() and
    currentAlignmentChanged() are emitted to reflect the font, color and
    alignment at the new cursor position.

    If the text changes, the textChanged() signal is emitted, and if the
    user inserts a new line by pressing Return or Enter, returnPressed()
    is emitted.

  QTextEdit provides command-based undo/redo. To set the depth of the
  command history use setUndoDepth() which defaults to 100 steps. To
  undo or redo the last operation call undo() or redo(). The signals
  undoAvailable() and redoAvailable() indicate whether the undo and redo
  operations can be executed.

    The indent() function is used to reindent a paragraph. It is useful
    for code editors, for example in <em>Qt Designer</em>'s code editor
    \e{Ctrl+I} invokes the indent() function.

  Loading and saving text is achieved using setText() and text(), for
  example:
  \code
    QFile file( fileName ); // Read the text from a file
    if ( file.open( IO_ReadOnly ) ) {
	QTextStream ts( &file );
	textEdit->setText( ts.read() );
    }
  \endcode
  \code
    QFile file( fileName ); // Write the text to a file
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream ts( &file );
	ts << textEdit->text();
	textEdit->setModified( FALSE );
    }
  \endcode

  The list of key-bindings which are implemented for editing:

  <ul>
  <li><i> Backspace </i> -- Delete the character to the left of the cursor
  <li><i> Delete </i> -- Delete the character to the right of the cursor
  <li><i> Ctrl+A </i> -- Move the cursor to the beginning of the line
  <li><i> Ctrl+B </i> -- Move the cursor one character left
  <li><i> Ctrl+C </i> -- Copy the marked text to the clipboard (also
  <i>Ctrl+Insert</i> under Windows)
  <li><i> Ctrl+D </i> -- Delete the character to the right of the cursor
  <li><i> Ctrl+E </i> -- Move the cursor to the end of the line
  <li><i> Ctrl+F </i> -- Move the cursor one character right
  <li><i> Ctrl+H </i> -- Delete the character to the left of the cursor
  <li><i> Ctrl+K </i> -- Delete to end of line
  <li><i> Ctrl+N </i> -- Move the cursor one line down
  <li><i> Ctrl+P </i> -- Move the cursor one line up
  <li><i> Ctrl+V </i> -- Paste the clipboard text into line edit (also
  <i>Shift+Insert</i> under Windows)
  <li><i> Ctrl+X </i> -- Cut the marked text, copy to clipboard (also
  <i>Shift+Delete</i> under Windows)
  <li><i> Ctrl+Z </i> -- Undo the last operation
  <li><i> Ctrl+Y </i> -- Redo the last operation
  <li><i> Left Arrow </i> -- Move the cursor one character left
  <li><i> Ctrl+Left Arrow </i> -- Move the cursor one word left
  <li><i> Right Arrow </i> -- Move the cursor one character right
  <li><i> Ctrl+Right Arrow </i> -- Move the cursor one word right
  <li><i> Up Arrow </i> -- Move the cursor one line up
  <li><i> Ctrl+Up Arrow </i> -- Move the cursor one word up
  <li><i> Down Arrow </i> -- Move the cursor one line down
  <li><i> Ctrl+Down Arrow </i> -- Move the cursor one word down
  <li><i> Page Up </i> -- Move the cursor one page up
  <li><i> Page Down </i> -- Move the cursor one page down
  <li><i> Home </i> -- Move the cursor to the beginning of the line
  <li><i> Ctrl+Home Arrow </i> -- Move the cursor to the beginning of the text
  <li><i> End </i> -- Move the cursor to the end of the line
  <li><i> Ctrl+End Arrow </i> -- Move the cursor to the end of the text
    <li><i> Shift+Wheel</i> -- Scroll the page horizontally (the Wheel is
    the mouse wheel)
    <li><i> Ctrl+Wheel</i> -- Zoom the text
  </ul>

    To select (mark) text hold down the Shift key whilst pressing one of
    the movement keystrokes, for example, <i>Shift+Right Arrow</i> will
    select the character to the right, and <i>Shift+Ctrl+Right Arrow</i>
    will select the word to the right, etc.

    By default the text edit widget operates in insert mode so all text
    that the user enters is inserted into the text edit and any text to
    the right of the cursor is moved out of the way. The mode can be
    changed to overwrite, where new text overwrites any text to the right
    of the cursor, using setOverwriteMode().
*/

/*! \enum QTextEdit::KeyboardAction

  This enum is used by doKeyboardAction() to specify which action
  should be executed:

  \value ActionBackspace  Delete the character to the left of the
  cursor.

  \value ActionDelete  Delete the character to the right of the cursor.

  \value ActionReturn  Split the paragraph at the cursor position.

  \value ActionKill If the cursor is not at the end of the paragraph,
  delete the text from the cursor position until the end of the
  paragraph. If the cursor is at the end of the paragraph, delete the
  hard line break at the end of the paragraph - this will cause this
  paragraph to be joined with the following paragraph.
*/

/*! \enum QTextEdit::MoveDirection

  This enum is used by moveCursor() to specify in which direction
  the cursor should be moved:

  \value MoveLeft  Moves the cursor one character left (or one word left
  if a Ctrl key is pressed)

  \value MoveRight  Moves the cursor one character right (or one word
  right if a Ctrl key is pressed)

  \value MoveUp  Moves the cursor up one line

  \value MoveDown  Moves the cursor down one line

  \value MoveHome  Moves the cursor to the beginning of the line

  \value MoveEnd Moves the cursor to the end of the line

  \value MovePgUp  Moves the cursor one page up

  \value MovePgDown  Moves the cursor one page down
*/


/*! \fn void QTextEdit::getCursorPosition( int &parag, int &index ) const

  This function sets the \a parag and \a index parameters to the
  current cursor position.

  \sa setCursorPosition()
 */

/*! \property bool QTextEdit::modified 
  \brief whether the document has been modified by the user
*/

/*! \fn bool QTextEdit::italic() const

    Returns TRUE if the current format is italic; otherwise returns FALSE.

    \sa setItalic()
*/

/*! \fn bool QTextEdit::bold() const

    Returns TRUE if the current format is bold; otherwise returns FALSE.

    \sa setBold()
*/

/*! \fn bool QTextEdit::underline() const

    Returns TRUE if the current format is underlined; otherwise returns FALSE.

    \sa setUnderline()
*/

/*! \fn QString QTextEdit::family() const

  Returns the font family of the current format.

  \sa setFamily() setCurrentFont() setPointSize()
*/

/*! \fn int QTextEdit::pointSize() const

  Returns the point size of the font of the current format.

  \sa setFamily() setCurrentFont() setPointSize()

*/

/*! \fn QColor QTextEdit::color() const

  Returns the color of the current format.

  \sa setColor()
*/

/*! \fn QFont QTextEdit::font() const

  Returns the font of the current format.

  \sa setCurrentFont()

*/

/*! \fn int QTextEdit::alignment() const

  Returns the alignment of the current paragraph.

  \sa setAlignment()
*/

/*!
  \property QTextEdit::overwriteMode
  \brief the text edit's overwrite mode

  If TRUE, the editor is in overwrite mode, i.e. characters entered by
  the user overwrite any characters to the right of the cursor position.
  If FALSE characters entered by the user are inserted with any
  characters to the right being moved out of the way.
*/


/*! \fn void QTextEdit::insert( const QString &text, bool indent, bool checkNewLine, bool removeSelected )

  Inserts \a text at the current cursor position. If \a indent is TRUE,
  the paragraph is re-indented. If \a checkNewLine is TRUE, newline
  characters in \a text result in hard line breaks (i.e. new
  paragraphs). If \a checkNewLine is FALSE the behaviour of the editor
  is undefined if the \a text contains newlines. If \a removeSelected is
  TRUE, any selected text is removed before the text is inserted.

  \sa paste() pasteSubType()
*/

/*! \fn void QTextEdit::undo()

  Undoes the last operation.

  If there is no operation to undo, e.g. there is no undo step in the
  undo/redo history, nothing happens.

  \sa undoAvaliable() redo() undoDepth()
*/

/*! \fn void QTextEdit::redo()

  Redoes the last operation.

  If there is no operation to redo, e.g. there is no redo step in the
  undo/redo history, nothing happens.

  \sa redoAvaliable() undo() undoDepth()
*/

/*! \fn void QTextEdit::cut()

    Copies the selected text to the clipboard and deletes it from the
    text edit.

    If there is no selected text nothing happens.

    \sa QTextView::copy() paste() pasteSubType()
*/

/*! \fn void QTextEdit::paste()

    Pastes the text from the clipboard into the text edit at the current
    cursor position. Only plain text is pasted.

    If there is no text in the clipboard nothing happens.

    \sa pasteSubType() cut() QTextView::copy()
*/

/*! \fn void QTextEdit::pasteSubType( const QCString &subtype )

    Pastes the text with format \a subtype from the clipboard into the
    text edit at the current cursor position. The \a subtype can be
    "plain" or "html".

    If there is no text with format \a subtype in the clipboard nothing
    happens.

    \sa paste() cut() QTextView::copy()
*/

/*! \fn void QTextEdit::clear()

  Deletes all the text in the text edit.

  \sa cut() removeSelectedText()

*/

/*! \fn void QTextEdit::del()

    If there is some selected text it is deleted. If there is no
    selected text the character to the right of the text cursor is
    deleted.

    \sa removeSelectedText() cut()

*/

/*! \fn void QTextEdit::indent()

  Re-indents the current paragraph.
*/

/*! \fn void QTextEdit::setItalic( bool b )

    If b is TRUE sets the current format to italic; otherwise sets the
    current format to non-italic.

    \sa italic()
*/

/*! \fn void QTextEdit::setBold( bool b )

    If b is TRUE sets the current format to bold; otherwise sets the
    current format to non-bold.

    \sa bold()
*/

/*! \fn void QTextEdit::setUnderline( bool b )

    If b is TRUE sets the current format to underline; otherwise sets the
    current format to non-underline.

    \sa underline()
*/

/*! \fn void QTextEdit::setFamily( const QString &f )

  Sets the font family of the current format to \a f.

  \sa family() setCurrentFont()
*/

/*! \fn void QTextEdit::setPointSize( int s )

  Sets the point size of the current format to \a s.

  Note that if \a s is zero or negative, the behaviour of this
  function is not defined.

  \sa pointSize() setCurrentFont() setFamily()
*/

/*! \fn void QTextEdit::setColor( const QColor &c )

  Sets the color of the current format to \a c.

  \sa color()
*/

/*! \fn void QTextEdit::setCurrentFont( const QFont &f )

  Sets the font of the current format to \a f.

  \sa font() setPointSize() setFamily()
*/

/*! \fn void QTextEdit::setAlignment( int a )

  Sets the alignment of the current paragraph to \a a. Valid alignments
  are \c Qt::AlignLeft, \c Qt::AlignRight and Qt::AlignJustify. (See
  Qt::AlignmentFlags.)

  \sa setParagType()
*/

/*! \fn void QTextEdit::setParagType( QStyleSheetItem::DisplayMode dm, QStyleSheetItem::ListStyle listStyle )

  Sets the paragraph style of the current paragraph
  to \a dm. If \a dm is QStyleSheetItem::DisplayListItem, the
  type of the list item is set to \a listStyle.

  \sa setAlignment()
*/

/*! \fn void QTextEdit::setCursorPosition( int parag, int index )

  Sets the cursor to position \a index in paragraph \a parag.

  \sa getCursorPosition()
*/

/*! \fn void QTextEdit::setSelection( int parag_from, int index_from, int parag_to, int index_to, int selNum )

  Sets a selection which starts at position \a index_from in
  paragraph \a parag_from and ends position \a index_to in
  paragraph \a parag_to.

  Uses the selection settings of selection \a selNum. If this is 0,
  this is the default selection.

  \sa getSelection() QTextView::selectedText()
*/

/*! \fn void QTextEdit::setSelectionAttributes( int selNum, const QColor &back, bool invertText )

  Sets the background color of selection \a selNum to \a back and
  specifies whether the text of this selection should be inverted with \a
  invertText.

*/

/*! \fn void QTextEdit::resetFormat()

    \internal

  Resets the current format to the default format.
*/

/*!
  \property QTextEdit::undoDepth
  \brief the depth of the undo history

  The maximum number of steps in the undo/redo history.

  \sa undo() redo()
*/

/*! \fn void QTextEdit::undoAvailable( bool yes )

  This signal is emitted when the availability of undo changes.  If \a
  yes is TRUE, then undo() will work until undoAvailable( FALSE ) is
  next emitted.

  \sa undo() undoDepth()
*/

/*! \fn void QTextEdit::modificationChanged( bool m )

  This signal is emitted when the modification of the document
  changed. If \a m is TRUE, the document was modified, otherwise the
  modification state has been reset to unmodified.

  \sa modified
*/

/*! \fn void QTextEdit::redoAvailable( bool yes )

  This signal is emitted when the availability of redo changes.  If \a
  yes is TRUE, then redo() will work until redoAvailable( FALSE ) is
  next emitted.

  \sa redo() undoDepth()
*/

/*! \fn void QTextEdit::currentFontChanged( const QFont &f )

  This signal is emitted if the font of the current format has changed.

  The new font is \a f.

  \sa setCurrentFont()
*/

/*! \fn void QTextEdit::currentColorChanged( const QColor &c )

  This signal is emitted if the color of the current format has changed.

  The new color is \a c.

  \sa setColor()
*/

/*! \fn void QTextEdit::currentAlignmentChanged( int a )

  This signal is emitted if the alignment of the current paragraph
  has changed.

  The new alignment is \a a.

  \sa setAlignment()
*/

/*! \fn void QTextEdit::cursorPositionChanged( QTextCursor *c )

  This signal is emitted if the position of the cursor changed. \a c
  points to the text cursor object.

  \sa setCursorPosition()
*/

/*! \fn void QTextEdit::returnPressed()

  This signal is emitted if the user pressed the RETURN or the ENTER key.
*/

/*! \fn void QTextEdit::setFormat( QTextFormat *f, int flags )

    \internal

  This functions sets the current format to \a f. Only the fields of \a
  f which are specified by the \a flags are used.
*/

/*! \fn void QTextEdit::ensureCursorVisible()

  Ensures that the cursor is visible by scrolling the text edit if
  necessary.

  \sa setCursorPosition()
*/

/*! \fn void QTextEdit::placeCursor( const QPoint &pos, QTextCursor *c )

  Places the cursor \a c at the character which is closest to \a pos
  (in contents coordinates). If \a c is 0, the default text cursor is
  used.

  \sa setCursorPosition()
*/

/*! \fn void QTextEdit::moveCursor( MoveDirection direction, bool shift, bool control )

  Moves the text cursor in \a direction. As this is normally
  used by some key event handler, the state of the \c Shift and \c Ctrl
  keys will influence how the cursor moves. For example, \e{Left Arrow}
  moves one character left, but \e{Ctrl+Left Arrow} moves one word left.
*/

/*! \overload void QTextEdit::moveCursor( MoveDirection direction, bool control )
*/

/*! \fn void QTextEdit::removeSelectedText()

  Deletes the selected text. If there is no selected text nothing
  happens.
*/

/*! \fn void QTextEdit::doKeyboardAction( KeyboardAction action )

  Executes the keyboard action \a action. This is normally called by
  a key event handler.
*/

/*! \fn QTextCursor *QTextEdit::textCursor() const

  Returns the text edit's text cursor. QTextCursor is not in the
  public API, but in special circumstances you might wish to use it.
  Note however that the API might change in an incompatible manner in
  the future.
*/

/*! \fn bool QTextEdit::getFormat( int para, int index, QFont *font, QColor *color )

  This function gets the format of the character at position \a index in
  paragraph \a para. Sets \a font to the character's font and \a color
  to the character's color.

  Returns FALSE if \a para or \a index is out of range otherwise
  returns TRUE.
*/

/*! Constructs a QTextEdit. The \a parent and \a name arguments are as
  for QWidget. */

QTextEdit::QTextEdit( QWidget *parent, const char *name )
    : QTextView( parent, name )
{
#ifndef QT_NO_CURSOR
    viewport()->setCursor( ibeamCursor );
#endif
}

/*! \reimp */

QTextEdit::~QTextEdit()
{
}

/*!
    \fn void QTextEdit::setFont( const QFont & )

    \obsolete

    Use setCurrentFont() instead.
*/

#endif //QT_NO_TEXTEDIT
