/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmultilineedit.cpp#102 $
**
** Implementation of QMultiLineEdit widget class
**
** Created : 961005
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
#include "qdict.h"
#include "../kernel/qrichtext_p.h"

#include <ctype.h>


// NOT REVISED
/*!
  \class QMultiLineEdit qmultilineedit.h

  \brief The QMultiLineEdit widget is a simple editor for inputting text.

  \ingroup advanced

  The QMultiLineEdit was a simple editor widget in former Qt
  versions. Since Qt 3.0, which comes with a new richtext engine,
  which also supports editing, QMultiLineEdit is obsolete. It is still
  included for compatibility reasons. It is now a subclass of
  QTextEdit which wrappes the old QMultiLineEdit so that it is mostly
  source compatible to keep old applications working.
  
  If you implement something new with QMultiLineEdit, rather use
  QTextEdit instead.
  
  Although most of the old QMultiLineEdit API is still available,
  there is one important difference. Because of a design flaw of the
  old QMultiLineEdit it operated on lines and not on paragraphs. As
  lines do change all the time during wordwrap, the new richtext
  engine only knows paragraphs as elements in the data structure. So
  all functions (like numLines(), textLine()), which worked on lines,
  now work on paragraphs.
  
  Also the function getString() has been removed as this one published
  the internal data structure.
  
  So, applications which made normal usage of the QMultiLineEdit,
  should keep working without problems. Programs which did some
  special stuff with it might require some porting.

  <img src=qmlined-m.png> <img src=qmlined-w.png>
*/

struct QMultiLineData
{
};


/*!
  Constructs a new, empty, QMultiLineEdit.
*/

QMultiLineEdit::QMultiLineEdit( QWidget *parent , const char *name )
    : QTextEdit( parent, name )
{
    d = new QMultiLineData;
    readOnly = FALSE;
    setTextFormat( Qt::PlainText );
}

/*! \fn int QMultiLineEdit::numLines() const

  Returns the number of paragraphs in the editor. The count includes
  any empty paragraph at top and bottom, so for an empty editor this
  method will return 1.
*/

int QMultiLineEdit::numLines() const
{
    return document()->lastParag()->paragId() + 1;
}

/*! \fn bool QMultiLineEdit::atEnd() const

  Returns TRUE if the cursor is placed at the end of the text.
*/

bool QMultiLineEdit::atEnd() const
{
    return textCursor()->parag() == document()->lastParag() && textCursor()->atParagEnd();
}


/*! \fn bool QMultiLineEdit::atBeginning() const

  Returns TRUE if the cursor is placed at the beginning of the text.
*/

bool QMultiLineEdit::atBeginning() const
{
    return textCursor()->parag() == document()->firstParag() && textCursor()->atParagStart();
}

/*!  Returns the number of characters at paragraph number \a line. If
  \a line is out of range, -1 is returned.
*/

int QMultiLineEdit::lineLength( int row ) const
{
    if ( row < 0 || row > numLines() )
	return -1;
    return document()->paragAt( row )->length();
}

/*! \fn bool QMultiLineEdit::isReadOnly() const

  Returns FALSE if this multi line edit accepts text input.
  Scrolling and cursor movements are accepted in any case.

  \sa setReadOnly() QWidget::isEnabled()
*/

/*!
  If \a on is FALSE, this multi line edit accepts text input.
  Scrolling and cursor movements are accepted in any case.

  \sa isReadOnly() QWidget::setEnabled()
*/

void QMultiLineEdit::setReadOnly( bool on )
{
    if ( readOnly != on ) {
	readOnly = on;
#ifndef QT_NO_CURSOR
	viewport()->setCursor( on ? arrowCursor : ibeamCursor );
#endif
    }
}

/*! \reimp
*/

QMultiLineEdit::~QMultiLineEdit()
{
    delete d;
}

/*!
  If there is marked text, sets \a line1, \a col1, \a line2 and \a col2
  to the start and end of the marked region and returns TRUE. Returns
  FALSE if there is no marked text.
 */
bool QMultiLineEdit::getMarkedRegion( int *line1, int *col1,
				      int *line2, int *col2 ) const
{
    int p1,c1, p2, c2;
    getSelection( p1, c1, p2, c2 );
    if ( p1 == -1 && c1 == -1 && p2 == -1 && c2 == -1 )
	return FALSE;
    if ( line1 )
	*line1 = p1;
    if ( col1 )
	*col1 = c1;
    if ( line2 )
	*line2 = p2;
    if ( col2 )
	*col2 = c2;
    return TRUE;
}


/*!
  Returns TRUE if there is marked text.
*/

bool QMultiLineEdit::hasMarkedText() const
{
    return hasSelectedText();
}


/*!
  Returns a copy of the marked text.
*/

QString QMultiLineEdit::markedText() const
{
    return selectedText();
}

/*!
  Moves the cursor one page down.  If \a mark is TRUE, the text
  is marked.
*/

void QMultiLineEdit::pageDown( bool mark )
{
    moveCursor( MoveDown, mark, FALSE );
}


/*!
  Moves the cursor one page up.  If \a mark is TRUE, the text
  is marked.
*/

void QMultiLineEdit::pageUp( bool mark )
{
    moveCursor( MovePgUp, mark, FALSE );
}


/*!  Inserts \a txt at paragraph number \a line. If \a line is less
  than zero, or larger than the number of paragraphs, the new text is
  put at the end.  If \a txt contains newline characters, several
  paragraphs are inserted.

  The cursor position is not changed.
*/

void QMultiLineEdit::insertLine( const QString &txt, int line )
{
    QString s = txt;
    QTextCursor tmp = *textCursor();
    if ( line < 0 || line >= numLines() ) {
	textCursor()->setParag( document()->lastParag() );
	textCursor()->gotoEnd();
    } else {
	s.append('\n');
	textCursor()->setParag( document()->paragAt( line ) );
	textCursor()->gotoLineStart();
    }
    insert( s );
    QTextCursor *c = textCursor();
    *c = tmp;
}

/*!  Deletes the paragraph at paragraph number \a line. If \a line is
  less than zero, or larger than the number of paragraphs, no line is
  deleted.
*/

void QMultiLineEdit::removeLine( int line )
{
    if ( line < 0 || line >= numLines() )
	return;
    QTextCursor tmp = *textCursor();
    tmp.killLine(); // until end
    tmp.killLine(); // join
}

/*!  Inserts \a str at the current cursor position and selects the
  text if \a mark is TRUE.
*/

void QMultiLineEdit::insertAndMark( const QString& str, bool mark )
{
    insert( str );
    if ( mark )
	document()->setSelectionEnd( QTextDocument::Standard, textCursor() );
}

/*!  Splits the paragraph at the current cursor position.
*/

void QMultiLineEdit::newLine()
{
    insert( "\n" );
}


/*!  Deletes the character on the left side of the text cursor and
  moves the cursor one position to the left. If a text has been marked
  by the user (e.g. by clicking and dragging) the cursor is put at the
  beginning of the marked text and the marked text is removed.  \sa
  del()
*/

void QMultiLineEdit::backspace()
{
    if ( document()->hasSelection( QTextDocument::Standard ) ) {
	removeSelectedText();
	return;
    }

    if ( !textCursor()->parag()->prev() &&
	 textCursor()->atParagStart() )
	return;

    doKeyboardAction( ActionBackspace );
}


/*!  Moves the text cursor to the left end of the line. If \a mark is
  TRUE, text is marked towards the first position. If it is FALSE and
  the cursor is moved, all marked text is unmarked.

  \sa end()
*/

void QMultiLineEdit::home( bool mark )
{
    moveCursor( MoveHome, mark, FALSE );
}

/*!  Moves the text cursor to the right end of the line. If mark is
  TRUE text is marked towards the last position.  If it is FALSE and
  the cursor is moved, all marked text is unmarked.

  \sa home()
*/

void QMultiLineEdit::end( bool mark )
{
    moveCursor( MoveEnd, mark, FALSE );
}

/*!  Sets the cursor position to character number \a col in paragraph
  number \a line.  The parameters are adjusted to lie within the legal
  range.

  If \a mark is FALSE, the selection is cleared. otherwise it is extended

  \sa cursorPosition()
*/

void QMultiLineEdit::setCursorPosition( int line, int col, bool mark )
{
    if ( !mark )
	selectAll( FALSE );
    setCursorPosition( line, col );
    if ( mark )
	document()->setSelectionEnd( QTextDocument::Standard, textCursor() );
}


/*!  Returns the current paragraph and character position within that
  paragraph, in the variables pointed to by \a line and \a col
  respectively.

  \sa setCursorPosition()
*/

void QMultiLineEdit::getCursorPosition( int *line, int *col ) const
{
    int l, c;
    QTextEdit::getCursorPosition( l, c );
    if ( line )
	*line = l;
    if ( col )
	*col = c;
}

/*!  Returns the top center point where the cursor is drawn
*/

QPoint QMultiLineEdit::cursorPoint() const
{
    return QPoint( textCursor()->totalOffsetX(), textCursor()->totalOffsetY() );
}

/*!  Sets the alignment. Possible values are \c AlignLeft, \c
  Align(H)Center and \c AlignRight.

  \sa alignment(), Qt::AlignmentFlags
*/
void QMultiLineEdit::setAlignment( int flags )
{
    QTextParag *p = document()->firstParag();
    while ( p ) {
	p->setAlignment( flags );
	p = p->next();
    }
}

/*!  Returns the alignment.

  \sa setAlignment(), Qt::AlignmentFlags.
*/

int QMultiLineEdit::alignment() const
{
    return document()->firstParag()->alignment();
}


/*!  Sets the edited flag of this line edit to \a on.  The edited flag
  is never read by QMultiLineEdit, but is changed to TRUE whenever the
  user changes its contents.

  This is useful e.g. for things that need to provide a default value,
  but cannot find the default at once.  Just open the widget without
  the best default and when the default is known, check the edited()
  return value and set the line edit's contents if the user has not
  started editing the line edit.  Another example is to detect whether
  the contents need saving.

  \sa edited()
*/
void QMultiLineEdit::setEdited( bool e )
{
    setModified( e );
}

/*!  Returns the edited flag of the line edit.  If this returns FALSE,
  the contents has not been changed since the construction of the
  QMultiLineEdit (or the last call to setEdited( FALSE ), if any).  If
  it returns TRUE, the contents have been edited, or setEdited( TRUE )
  has been called.

  \sa setEdited()
*/
bool QMultiLineEdit::edited() const
{
    return isModified();
}

/*!  Moves the cursor one word to the right.  If \a mark is TRUE, the
  text is marked.  
  
  \sa cursorWordBackward()
*/
void QMultiLineEdit::cursorWordForward( bool mark )
{
    moveCursor( MoveRight, mark, TRUE );
}

/*!  Moves the cursor one word to the left.  If \a mark is TRUE, the
  text is marked.  
  
  \sa cursorWordForward()
*/
void QMultiLineEdit::cursorWordBackward( bool mark )
{
    moveCursor( MoveLeft, mark, TRUE );
}

/*!  Inserts \a txt at paragraph number \a line, after character
  number \a col in the paragraph.  If \a txt contains newline
  characters, new lines are inserted.

  The cursor position is adjusted.
 */

void QMultiLineEdit::insertAt( const QString &s, int line, int col, bool mark )
{
    if ( line < 0 || line >= numLines() )
	return;
    QTextCursor tmp = *textCursor();
    textCursor()->setParag( document()->paragAt( line ) );
    textCursor()->setIndex( col );
    insert( s );
    QTextCursor *c = textCursor();
    *c = tmp;
    if ( mark )
	setSelection( line, col, line, col + s.length() );
}

/*!  Deletes text from the current cursor position to the end of the
  line.
*/

void QMultiLineEdit::killLine()
{
    doKeyboardAction( ActionKill );
}

/*!  Deletes the character on the right side of the text cursor. If a
  text has been marked by the user (e.g. by clicking and dragging) the
  cursor is put at the beginning of the marked text and the marked
  text is removed.  
  
  \sa backspace()
*/

void QMultiLineEdit::del()
{
    if ( document()->hasSelection( QTextDocument::Standard ) ) {
	removeSelectedText();
	return;
    }

    doKeyboardAction( ActionDelete );
}

/*!  Moves the cursor one character to the left. If \a mark is TRUE,
  the text is marked.

  \sa cursorRight() cursorUp() cursorDown()
*/

void QMultiLineEdit::cursorLeft( bool mark, bool )
{
    moveCursor( MoveLeft, mark, FALSE );
}

/*!  Moves the cursor one character to the right.  If \a mark is TRUE,
  the text is marked.

  \sa cursorLeft() cursorUp() cursorDown()
*/

void QMultiLineEdit::cursorRight( bool mark, bool )
{
    moveCursor( MoveRight, mark, FALSE );
}

/*!  Moves the cursor up one line.  If \a mark is TRUE, the text is
  marked.  
  
  \sa cursorDown() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorUp( bool mark )
{
    moveCursor( MoveUp, mark, FALSE );
}

/*!
  Moves the cursor one line down.  If \a mark is TRUE, the text
  is marked.
  \sa cursorUp() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorDown( bool mark )
{
    moveCursor( MoveDown, mark, FALSE );
}

/*!  Returns the text at line number \a line (possibly the empty
  string), or a \link QString::operator!() null string\endlink if \a
  line is invalid.
*/

QString QMultiLineEdit::textLine( int line ) const
{
    if ( line < 0 || line >= numLines() )
	return QString::null;
    return document()->paragAt( line )->string()->toString();
}

#endif
