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
#include "qcursor.h"

/*!
  \class QTextEdit qtextedit.h
  \brief A sophisticated single-page rich text editor.
  \ingroup basic

  QTextEdit is an advanced WYSIWYG editor supporting rich text
  formatting. It is optimized to handle large text
  documents and responds quickly to user input.

  QTextEdit extends QTextView with keyboard and mouse handling for
  user input and functions to set/get/modify formatting, text, etc.

  As it is derived from QTextView it supports the same functions to
  set and load plain and HTML text. Using the save() function, the
  current contents can also be saved in HTML.

  QTextEdit internally works on paragraphs. A paragraph is a formatted
  string which is word-wrapped to fit into the width of the
  widget. Paragraphs are seperated by hard breaks.

  For user input selections are important. To work with selections use
  setSelection() and QTextView::getSelection(). To set or get the
  position of the cursor use setCursorPosition() or
  getCursorPosition().

  To change the current format (at the cursor position) or the formats
  of a selection, use setBold(), setItalic(), setUnderline(),
  setFamily(), setPointSize(), setFont() or setColor(). To change
  paragraph formatting use setAlignment() and setParagType(). When the
  cursor is moved, the signals currentFontChanged(),
  currentColorChanged() and currentAlignmentChanged() are emitted to
  inform about the format at the cursor position.

  To insert text at the cursor position use insert(). Also cut(),
  copy() and paste() can be done through the API. If the text changes,
  the textChanged() signal is emitted, if the user inserted a new line
  by pressing return/enter returnPressed() is emitted.

   QTextEdit also implements a command based undo/redo
   functionality. To set the depth of the command history use
   setUndoDepth() - it defaults to 100 steps. To undo or redo the last
   opetation call undo() or redo(). The signals undoAvailable() and
   redoAvailable() inform about when undo or redo operations can be
   executed.

  Here is a list of key-bindings which are inplemented for editing

  <ul>
  <li><i> Left Arrow </i> Move the cursor one character leftwards
  <li><i> Right Arrow </i> Move the cursor one character rightwards
  <li><i> Up Arrow </i> Move the cursor one line upwards
  <li><i> Down Arrow </i> Move the cursor one line downwards
  <li><i> Page Up </i> Move the cursor one page upwards
  <li><i> Page Down </i> Move the cursor one page downwards
  <li><i> Backspace </i> Delete the character to the left of the cursor
  <li><i> Home </i> Move the cursor to the beginning of the line
  <li><i> End </i> Move the cursor to the end of the line
  <li><i> Delete </i> Delete the character to the right of the cursor
  <li><i> Shift - Left Arrow </i> Mark text one character leftwards
  <li><i> Shift - Right Arrow </i> Mark text one character rightwards
  <li><i> Control-A </i> Move the cursor to the beginning of the line
  <li><i> Control-B </i> Move the cursor one character leftwards
  <li><i> Control-C </i> Copy the marked text to the clipboard
  <li><i> Control-D </i> Delete the character to the right of the cursor
  <li><i> Control-E </i> Move the cursor to the end of the line
  <li><i> Control-F </i> Move the cursor one character rightwards
  <li><i> Control-H </i> Delete the character to the left of the cursor
  <li><i> Control-K </i> Delete to end of line
  <li><i> Control-N </i> Move the cursor one line downwards
  <li><i> Control-P </i> Move the cursor one line upwards
  <li><i> Control-V </i> Paste the clipboard text into line edit
  <li><i> Control-X </i> Cut the marked text, copy to clipboard
  <li><i> Control-Z </i> Undo the last operation
  <li><i> Control-Y </i> Redo the last operation
  <li><i> Control - Left Arrow </i> Move the cursor one word leftwards
  <li><i> Control - Right Arrow </i> Move the cursor one word rightwards
  <li><i> Control - Up Arrow </i> Move the cursor one word upwards
  <li><i> Control - Down Arrow </i> Move the cursor one word downwards
  <li><i> Control - Home Arrow </i> Move the cursor to the beginning of the text
  <li><i> Control - End Arrow </i> Move the cursor to the end of the text
  </ul>

  In addition, the following key bindings are used on Windows:

  <ul>
  <li><i> Shift - Delete </i> Cut the marked text, copy to clipboard
  <li><i> Shift - Insert </i> Paste the clipboard text into line edit
  <li><i> Control - Insert </i> Copy the marked text to the clipboard
  </ul>

  All other keys with valid ASCII codes insert themselves into the
  text at the cursor posi.



*/

/*! \enum QTextEdit::KeyboardAction

  This enum is used by doKeyboardAction() to specify which action
  should be exectuted:
	
  <ul>

  <li>\c ActionBackspace - Delete the character at the left of the cursor

  <li> \c ActionDelete - Delete the character at the right of the cursor

  <li> \c ActionReturn - Split the paragraph at the cursor position

  <li> \c ActionKill - Delete the text until the end of the paragraph,
  or if the cursor is at the end of the paragraph, join this and the next paragraph.

  </ul>
*/

/*! \enum QTextEdit::MoveDirection

  This enum is used by moveCursor() to specify into which direction
  the cursor should be moved:

  <ul>

  <li> \c MoveLeft - Moves the cursor to the left

  <li> \c MoveRight - Moves the cursor to the right

  <li> \c MoveUp - Moves the cursor up one line

  <li> \c MoveDown - Moves the cursor down one line

  <li> \c MoveHome - Moves the cursor to the begin of the line

  <li> \c MoveEnd - Moves the cursor to the end of the line

  <li> \c MovePgUp - Moves the cursor one page up

  <li> \c MovePgDown - Moves the cursor one page down

  </ul>
*/


/*! \fn void QTextEdit::getCursorPosition( int &parag, int &index ) const

  This functions sets the \a parag and \a index parameters to the
  current cursor position.
 */

/*! \fn bool QTextEdit::isModified() const

  This function returns whether the document has been modified by the
  user.
 */

/*! \fn bool QTextEdit::italic() const

  Returns wheather the current format (the position where the cursor
  is placed) is italic or not.
 */

/*! \fn bool QTextEdit::bold() const

  Returns wheather the current format (the position where the cursor
  is placed) is bold or not.
 */

/*! \fn bool QTextEdit::underline() const

  Returns wheather the current format (the position where the cursor
  is placed) is underlined or not.
 */

/*! \fn QString QTextEdit::family() const

  Returns the font family of the current format (the position where
  the cursor is placed).
 */

/*! \fn int QTextEdit::pointSize() const

  Returns the poit size of the font of the current format (the
  position where the cursor is placed).
 */

/*! \fn QColor QTextEdit::color() const

  Returns the color of the current format (the position where the
  cursor is placed).
 */

/*! \fn QFont QTextEdit::font() const

  Returns the font of the current format (the position where the
  cursor is placed).
 */

/*! \fn int QTextEdit::alignment() const

  Returns the alignment of the paragraph at which the cursor is
  currently placed.
 */

/*! \fn bool QTextEdit::isOverwriteMode() const

  Returns TRUE if this editor is in overwrite mode, i.e.  if
  characters typed replace characters in the editor.
*/

/*! \fn int QTextEdit::undoDepth() const

  Returns how many steps the undo/redo history can maximally store.
 */

/*! \fn void QTextEdit::insert( const QString &text, bool indent, bool checkNewLine )

  Inserts \a text at the current cursor position. If \a indent is
  TRUE, the paragraph gets re-indented. \a checkNewLine specifies
  whether the text should be checked for new-lines, and in this case
  multiple paragraphs should be inserted.
 */

/*! \fn void QTextEdit::setOverwriteMode( bool b )

  Sets overwrite mode if \a a on is TRUE. Overwrite mode means that
  characters typed replace characters in the editor.

 */

/*! \fn void QTextEdit::undo()

  Undoes the last operation.
 */

/*! \fn void QTextEdit::redo()

  Redoes the last operation.
 */

/*! \fn void QTextEdit::cut()

  Cuts the selected text (if there is any) and puts it on the
  clipboard.
 */

/*! \fn void QTextEdit::paste()

  Pastes the text from the clipboard (if there is any) at the current
  cursor position. Only pastes plain text.

  \sa pasteSubType()
 */

/*! \fn void QTextEdit::pasteSubType( const QCString &subtype )

  Pastes the text from the clipboard (if there is any) of the format
  \a subtype (this can be e.g. "plain", "html" ) at the current cursor
  position.
 */

/*! \fn void QTextEdit::indent()

  Re-indents the current paragraph.
 */

/*! \fn void QTextEdit::setItalic( bool b )

  Sets the current format and the selected text (if there is any) to
  italic, if \a b is TRUE, otherwise unsets the italic flag.
 */

/*! \fn void QTextEdit::setBold( bool b )

  Sets the current format and the selected text (if there is any) to
  bold, if \a b is TRUE, otherwise unsets the bold flag.
 */

/*! \fn void QTextEdit::setUnderline( bool b )

  Sets the current format and the selected text (if there is any) to
  underlined, if \a b is TRUE, otherwise unsets the underlined flag.
 */

/*! \fn void QTextEdit::setFamily( const QString &f )

  Sets the family of the current format and the selected text (if
  there is any).
 */

/*! \fn void QTextEdit::setPointSize( int s )

  Sets the point size of the current format and the selected text (if
  there is any).
 */

/*! \fn void QTextEdit::setColor( const QColor &c )

  Sets the color of the current format and the selected text (if there
  is any).
 */

/*! \fn void QTextEdit::setFont( const QFont &f )

  Sets the font of the current format and the selected text (if there
  is any).
 */

/*! \fn void QTextEdit::setAlignment( int a )

  Sets the alignment of the paragraph, at which the cursor is placed,
  to \a a.
 */

/*! \fn void QTextEdit::setParagType( QStyleSheetItem::DisplayMode dm, QStyleSheetItem::ListStyle listStyle )

  Sets the paragraph style of the paragraph ate which the cursor is
  placed to \a dm. If \a dm is QStyleSheetItem::DisplayListItem, the
  type of the list item is set to \a listStyle.
 */

/*! \fn void QTextEdit::setCursorPosition( int parag, int index )

  Sets the cursor to the index \a index in the paragraph \a parag.
 */

/*! \fn void QTextEdit::setSelection( int parag_from, int index_from, int parag_to, int index_to )

  Sets a selections which starts at the index \a index_from in the
  paragraph \a parag_from and ends at index \a index_to in the
  paragraph \a parag_to.
 */

/*! \fn void QTextEdit::setModified( bool m )

  Sets the modified flag of the document to \a m.
 */

/*! \fn void QTextEdit::resetFormat()

  Resets the current format to the default format.
 */

/*! \fn void QTextEdit::setUndoDepth( int d )

  Sets the number of steps the undo/redo history can maximally store
  to \a d.
*/

/*! \fn void QTextEdit::save( const QString &fn )

  Saves the document to the file \a fn. If \fn is valid, the filename
  of the document is set to this name. If \a fn is an empty string,
  and the document has already a filename, this one is used, otherwise
  nothing happens.
*/

/*! \fn void QTextEdit::undoAvailable( bool yes )

  This signal is emitted when the availability of undo changes.  If \a
  yes is TRUE, then undo() will work until undoAvailable( FALSE ) is
  next emitted.
 */

/*! \fn void QTextEdit::modificationChanged( bool m )

  This signal is emitted when the modification of the document
  changed. If \a m is TRUE, the document got modified, else the
  modification state has been reset to unmodified.
 */

/*! \fn void QTextEdit::redoAvailable( bool yes )

  This signal is emitted when the availability of redo changes.  If \a
  yes is TRUE, then redo() will work until redoAvailable( FALSE ) is
  next emitted.
*/

/*! \fn void QTextEdit::currentFontChanged( const QFont &f )

  This signal is emitted if the font of the current format (the format
  at the position where the cursor is placed) has changed.

  \a f contains the new font.
 */

/*! \fn void QTextEdit::currentColorChanged( const QColor &c )

  This signal is emitted if the color of the current format (the
  format at the position where the cursor is placed) has changed.

  \a c contains the new color.
*/

/*! \fn void QTextEdit::currentAlignmentChanged( int a )

  This signal is emitted if the alignment of the current paragraph
  (the paragraph at which the cursor is placed) has changed.

  \a a contains the new alignment.
*/

/*! \fn void QTextEdit::cursorPositionChanged( QTextCursor *c )

  This signal is emitted if the position of the cursor changed. \a c
  points to the text cursor object.
 */

/*! \fn void QTextEdit::returnPressed()

  This signal is emitted if the user pressed the RETURN or ENTER key.
 */

/*! \fn void QTextEdit::setFormat( QTextFormat *f, int flags )

  This functions sets the current format and the selected text (if
  there is any) to \a f. Only the fields of \a f which are specified
  by the \a flags are used.
 */

/*! \fn void QTextEdit::ensureCursorVisible()

  Ensures that the cursor is visible by scrolling the view if needed.
 */

/*! \fn void QTextEdit::placeCursor( const QPoint &pos, QTextCursor *c )

  Places the cursor \a c at the character which is closest to \a pos
  (in contents coordinates). If \a c is 0, the default text cursor is
  used.
 */

/*! \fn void QTextEdit::moveCursor( MoveDirection direction, bool shift, bool control )

  Moves the text cursor into the \a direction. As this is normally
  used by some keyevent handler, \a shift and \a control specify the
  state of the key modifiers which have an influence on the cursor
  moving.
 */

/*! \fn void QTextEdit::moveCursor( MoveDirection direction, bool control )

  Moves the text cursor into the \a direction. As this is normally
  used by some keyevent handler, \a control specifies the state of the
  control key modifiers which has an influence on the cursor moving.
 */

/*! \fn void QTextEdit::removeSelectedText()

  Deletes the text which is currently selected (if there is any
  selected).
 */

/*! \fn void QTextEdit::doKeyboardAction( KeyboardAction action )

  Exectutes the keyboard action \a action. This is normally called by
  a key event handler.
 */

/*! \fn QTextCursor *QTextEdit::textCursor() const

  Returns the text cursor if the editor. QTextCursor is not in the
  public API, but for special cases you might use it anyway. But the
  API of it might change in an incompatible manner in the future.
 */

/*! Constructs a QTextEdit */

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
