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

/*!  \fn void QTextEdit::undoAvailable (bool yes)

  This signal is emitted when the availability of undo changes.  If \a
  yes is TRUE, then undo() will work until undoAvailable( FALSE ) is
  next emitted.
*/

/*! \fn void QTextEdit::redoAvailable (bool yes)

  This signal is emitted when the availability of redo changes.  If \a
  yes is TRUE, then redo() will work until redoAvailable( FALSE ) is
  next emitted.
*/

/*! \fn bool QTextEdit::isOverwriteMode() const

  Returns TRUE if this multi line edit is in overwrite mode, i.e.
  if characters typed replace characters in the editor.

  \sa setOverwriteMode()
*/


/*! \fn void QTextEdit::setOverwriteMode( bool on )

  Sets overwrite mode if \a on is TRUE. Overwrite mode means
  that characters typed replace characters in the editor.

  \sa isOverwriteMode()
*/



/*!
  The key press event handler converts a key press to some line editor
  action.

  Here are the default key bindings when isReadOnly() is FALSE:
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
  All other keys with valid ASCII codes insert themselves into the line.

  Here are the default key bindings when isReadOnly() is TRUE:
  <ul>
  <li><i> Left Arrow </i> Scrolls the table rightwards
  <li><i> Right Arrow </i> Scrolls the table rightwards
  <li><i> Up Arrow </i> Scrolls the table one line downwards
  <li><i> Down Arrow </i> Scrolls the table one line upwards
  <li><i> Page Up </i> Scrolls the table one page downwards
  <li><i> Page Down </i> Scrolls the table one page upwards
  <li><i> Control-C </i> Copy the marked text to the clipboard
  </ul>

*/


QTextEdit::QTextEdit(QWidget *parent, const char *name )
    : QTextView( parent, name )
{
#ifndef QT_NO_CURSOR
    viewport()->setCursor( ibeamCursor );
#endif
}

QTextEdit::~QTextEdit()
{
}
