/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlineedit.cpp#245 $
**
** Implementation of QLineEdit widget class
**
** Created : 941011
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qlineedit.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qfontmetrics.h"
#include "qpixmap.h"
#include "qclipboard.h"
#include "qapplication.h"
#include "qvalidator.h"
#include "qdragobject.h"
#include "qtimer.h"
#include "qpopupmenu.h"
#include "qstringlist.h"

#include <ctype.h>


struct QLineEditUndoItem
{
    QLineEditUndoItem(){pos=0;};
    QLineEditUndoItem( const QString& s, int p )
	: str(s),pos(p){}
    QString str;
    int pos;
};

struct QLineEditPrivate {
    QLineEditPrivate( QLineEdit * l ):
	frame(TRUE), mode(QLineEdit::Normal), validator( 0 ),
	pm(0), pmDirty( TRUE ),
	blinkTimer( l, "QLineEdit blink timer" ),
	dragTimer( l, "QLineEdit drag timer" ),
	dndTimer( l, "DnD Timer" ),
	inDoubleClick( FALSE ), offsetDirty( FALSE ),
	undo(TRUE), needundo( FALSE ), ignoreUndoWithDel( FALSE ), mousePressed( FALSE ) {}

    bool frame;
    QLineEdit::EchoMode mode;
    const QValidator * validator;
    QPixmap * pm;
    bool pmDirty;
    QTimer blinkTimer;
    QTimer dragTimer, dndTimer;
    QRect cursorRepaintRect;
    bool inDoubleClick;
    bool offsetDirty;
    QPopupMenu *popup;
    int id[ 7 ];
    QValueList<QLineEditUndoItem> undoList;
    QValueList<QLineEditUndoItem> redoList;
    bool undo;
    bool needundo;
    bool ignoreUndoWithDel;
    bool mousePressed;
};


// REVISED: warwick
/*!
  \class QLineEdit qlineedit.h

  \brief The QLineEdit widget is a one-line text editor.

  \ingroup realwidgets

  A line edit allows the user to enter and edit a single line of
  plain text, with a useful collection of editing functions, including
  undo & redo, cut & paste, and drag & drop.

  By changing the echoMode() of a line edit it can also be used
  as a "write-only" field, for inputs such as passwords.

  The length of the field can be constrained to a maxLength(),
  or the value can be arbitrarily constrained by setting a validator().

  A closely related class is QMultiLineEdit which allows multi-line
  editing.

  The default QLineEdit object has its own frame as specified by the
  Windows/Motif style guides, you can turn off the frame by calling
  setFrame( FALSE ).

  The default key bindings are described in keyPressEvent().
  A right-mouse-button menu presents a number of the editing commands
  to the user.

  <img src=qlined-m.png> <img src=qlined-w.png>

  \sa QMultiLineEdit QLabel QComboBox
  <a href="guibooks.html#fowler">GUI Design Handbook: Field, Entry,</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Field, Required.</a>
*/


/*! \enum QLineEdit::EchoMode

  This enum type describes how QLineEdit displays its
  contents.  The defined values are:
  <ul>
  <li> \c Normal - display characters as they are entered.  This is
	the default.
  <li> \c NoEcho - do not display anything. This may be appropriate
	for passwords where even the length of the password should
	be kept secret.
  <li> \c Password - display asterisks instead of the characters
	actually entered.
  </ul>

  \sa setEchoMode() echoMode() QMultiLineEdit::EchoMode
*/


/*!
  \fn void QLineEdit::textChanged( const QString& )
  This signal is emitted every time the text changes.
  The argument is the new text.
*/


static const int scrollTime = 40;		// mark text scroll time


/*!
  Constructs a line edit with no text.

  The maximum text length is set to 32767 characters.

  The \e parent and \e name arguments are sent to the QWidget constructor.

  \sa setText(), setMaxLength()
*/

QLineEdit::QLineEdit( QWidget *parent, const char *name )
    : QWidget( parent, name, WRepaintNoErase | WNorthWestGravity )
{
    init();
}


/*!
  Constructs a line edit containing the text \a contents.

  The cursor position is set to the end of the line and the maximum text
  length to 32767 characters.

  The \e parent and \e name arguments are sent to the QWidget constructor.

  \sa text(), setMaxLength()
*/

QLineEdit::QLineEdit( const QString & contents,
		      QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    init();
    setText( contents );
}


/*!
  Destructs the line edit.
*/

QLineEdit::~QLineEdit()
{
    if ( d->pm )
	delete d->pm;
    delete d;
}


/*! Contains initialization common to both constructors. */

void QLineEdit::init()
{
    d = new QLineEditPrivate( this );
    connect( &d->blinkTimer, SIGNAL(timeout()),
	     this, SLOT(blinkSlot()) );
    connect( &d->dragTimer, SIGNAL(timeout()),
	     this, SLOT(dragScrollSlot()) );
    connect( &d->dndTimer, SIGNAL(timeout()),
	     this, SLOT(doDrag()) );
    cursorPos = 0;
    offset = 0;
    maxLen = 32767;
    cursorOn = TRUE;
    markAnchor = 0;
    markDrag = 0;
    dragScrolling = FALSE;
    scrollingLeft = FALSE;
    tbuf = QString::fromLatin1("");
    setFocusPolicy( StrongFocus );
    setCursor( ibeamCursor );
    setBackgroundMode( PaletteBase );
    setKeyCompression( TRUE );
    alignmentFlag = Qt::AlignLeft;
    setAcceptDrops( TRUE );
    ed = FALSE;
    d->popup = new QPopupMenu( this );
    d->id[ 0 ] = d->popup->insertItem( tr( "Undo" ) );
    d->id[ 1 ] = d->popup->insertItem( tr( "Redo" ) );
    d->popup->insertSeparator();
    d->id[ 2 ] = d->popup->insertItem( tr( "Cut" ) );
    d->id[ 3 ] = d->popup->insertItem( tr( "Copy" ) );
    d->id[ 4 ] = d->popup->insertItem( tr( "Paste" ) );
    d->id[ 5 ] = d->popup->insertItem( tr( "Clear" ) );
    d->popup->insertSeparator();
    d->id[ 6 ] = d->popup->insertItem( tr( "Select All" ) );
}


/*!
  Sets the line edit text to \e text, clears the selection and moves
  the cursor to the end of the line.

  If necessary the text is truncated to maxLength().

  \sa text()
*/

void QLineEdit::setText( const QString &text )
{
    QString oldText( tbuf );
    tbuf = text;
    if ( (int)tbuf.length() > maxLen )
	tbuf.truncate( maxLen );
    offset    = 0;
    cursorPos = 0;
    markAnchor = 0;
    markDrag = 0;
    end( FALSE );
    if ( validator() )
	(void)validator()->validate( tbuf, cursorPos );
    d->pmDirty = TRUE;

    update();
    if ( d->undo ) {
	d->undoList.clear();
	d->redoList.clear();
	d->needundo = TRUE;
     }
    if ( oldText != tbuf )
	emit textChanged( tbuf );
}


/*!
  Selects all text (i.e. marks it) and moves the cursor to the
  end. This is useful when a default value has been inserted,
  since if the user types before clicking on the widget the
  selected text will be erased.
*/

void QLineEdit::selectAll()
{
    setSelection( 0, tbuf.length() );
    end( TRUE );
}



/*!
  Deselects all text (i.e. removes marking) and leaves the cursor at the
  current position.
*/

void QLineEdit::deselect()
{
    setSelection( cursorPos, 0 );
}


/*!
  Returns the text in the line.
  \sa setText()
*/

QString QLineEdit::text() const
{
    return tbuf;
}



/*!  Returns the text that is displayed.  This is normally
the same as text(), but can be e.g. "*****" if EchoMode is Password or
"" if it is NoEcho.

\sa setEchoMode() text() EchoMode
*/

QString QLineEdit::displayText() const
{
    QString res;

    switch( echoMode() ) {
    case Normal:
	res = tbuf;
	break;
    case NoEcho:
	res = QString::fromLatin1("");
	break;
    case Password:
	res.fill( '*', tbuf.length() );
	break;
    }
    return res;
}



/*!
  Returns TRUE if part of the text has been marked by the user (e.g. by
  clicking and dragging).

  \sa markedText()
*/

bool QLineEdit::hasMarkedText() const
{
    return markAnchor != markDrag;
}

/*!
  Returns the text marked by the user (e.g. by clicking and
  dragging), or a \link QString::operator!() null string\endlink
  if no text is marked.
  \sa hasMarkedText()
*/

QString QLineEdit::markedText() const
{
    return tbuf.mid( minMark(), maxMark() - minMark() );
}

/*!
  Returns the maximum permitted length of the text in the editor.
  \sa setMaxLength()
*/

int QLineEdit::maxLength() const
{
    return maxLen;
}

/*!
  Set the maximum length of the text in the editor.  If the text is
  too long, it is chopped off at the limit. Any marked text will
  be unmarked.	The cursor position is set to 0 and the first part of the
  string is shown. \sa maxLength().
*/

void QLineEdit::setMaxLength( int m )
{
    maxLen = m;
    markAnchor = 0;
    markDrag = 0;
    if ( (int)tbuf.length() > maxLen ) {
	tbuf.truncate( maxLen );
	d->pmDirty = TRUE;
    }
    setCursorPosition( 0 );
    if ( d->pmDirty )
	update();
}

/*!
  \fn void  QLineEdit::returnPressed()
  This signal is emitted when the return or enter key is pressed.
*/


/*!
  Converts a key press into a line edit action.

  If return or enter is pressed and the current text is valid (or can be
  \link QValidator::fixup() made valid\endlink bt the validator),
  the signal returnPressed is emitted.

  The default key bindings are:
  <ul>
  <li><i> Left Arrow </i> Move the cursor one character leftwards.
  <li><i> Right Arrow </i> Move the cursor one character rightwards.
  <li><i> Backspace </i> Delete the character to the left of the cursor.
  <li><i> Home </i> Move the cursor to the beginning of the line.
  <li><i> End </i> Move the cursor to the end of the line.
  <li><i> Delete </i> Delete the character to the right of the cursor.
  <li><i> Shift - Left Arrow </i> Move and mark text one character leftwards.
  <li><i> Shift - Right Arrow </i> Move and mark text one character rightwards.
  <li><i> Control-A </i> Move the cursor to the beginning of the line.
  <li><i> Control-B </i> Move the cursor one character leftwards.
  <li><i> Control-C </i> Copy the marked text to the clipboard.
  <li><i> Control-D </i> Delete the character to the right of the cursor.
  <li><i> Control-E </i> Move the cursor to the end of the line.
  <li><i> Control-F </i> Move the cursor one character rightwards.
  <li><i> Control-H </i> Delete the character to the left of the cursor.
  <li><i> Control-V </i> Paste the clipboard text into line edit.
  <li><i> Control-X </i> Move the marked text to the clipboard.
  <li><i> Control-Z </i> Undo the last operation.
  <li><i> Control-Y </i> Redo the last undone operation.
  </ul>

  All other keys producing text insert the text into the line.
*/

void QLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Enter || e->key() == Key_Return ) {
	const QValidator * v = validator();
	if ( !v || v->validate( tbuf, cursorPos ) == QValidator::Acceptable ) {
	    emit returnPressed();
	    e->ignore();
	} else if ( v ) {
	    QString old( tbuf );
	    v->fixup( tbuf );
	    if ( old != tbuf )
		update();
	    if ( v->validate( tbuf, cursorPos ) == QValidator::Acceptable )
		emit returnPressed();
	    e->ignore();
	}
	return;
    }
    QString t = e->text();
    if ( !t.isEmpty() && (!e->ascii() || e->ascii()>=32) &&
	 e->key() != Key_Delete &&
	 e->key() != Key_Backspace ) {
	insert( t );
	return;
    }
    bool needundo = d->needundo;
    d->needundo = TRUE;
    bool ignoreUndoWithDel = d->ignoreUndoWithDel;
    d->ignoreUndoWithDel = FALSE;
    int unknown = 0;
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
	    home( e->state() & ShiftButton );
	    break;
	case Key_B:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_C:
	    copy();
	    break;
	case Key_D:
	    d->ignoreUndoWithDel = ignoreUndoWithDel;
	    del();
	    break;
	case Key_E:
	    end( e->state() & ShiftButton );
	    break;
	case Key_F:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_H:
	    d->ignoreUndoWithDel = ignoreUndoWithDel;
	    backspace();
	    break;
	case Key_K:
	    if ( cursorPos < (int)tbuf.length() ) {
		QString t( tbuf );
		t.truncate( cursorPos );
		validateAndSet( t, cursorPos, cursorPos, cursorPos );
	    }
	    break;
	case Key_V:
	    insert( QApplication::clipboard()->text() );
	    break;
	case Key_X:
	    if ( hasMarkedText() && echoMode() == Normal ) {
		copy();
		del();
	    }
	    break;
	case Key_Right:
	    cursorWordForward( e->state() & ShiftButton );
	    break;
	case Key_Left:
	    cursorWordBackward( e->state() & ShiftButton );
	    break;
#if defined (_WS_WIN_)
	case Key_Insert:
	    copy();
#endif	
	case Key_Z:
	    undoInternal();
	    break;
	case Key_Y:
	    redoInternal();
	    break;
	default:
	    unknown++;
	}
    } else {
	switch ( e->key() ) {
	case Key_Left:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_Right:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_Backspace:
	    d->ignoreUndoWithDel = ignoreUndoWithDel;
	    backspace();
	    break;
	case Key_Home:
	    home( e->state() & ShiftButton );
	    break;
	case Key_End:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Delete:
#if defined (_WS_WIN_)
	    if ( e->state() & ShiftButton ) {
		cut();
		break;
	    }
#endif	
	    d->ignoreUndoWithDel = ignoreUndoWithDel;
	    del();
	    break;
#if defined (_WS_WIN_)
	case Key_Insert:
	    if ( e->state() & ShiftButton )
		paste();
	    else
		unknown++;
	    break;
#endif	
	default:
	    unknown++;
	}
    }

    if ( unknown ) {				// unknown key
	d->needundo = needundo;
	e->ignore();
	return;
    }
}


/*!
  Handles the cursor blinking.
*/

void QLineEdit::focusInEvent( QFocusEvent * e)
{
    d->pmDirty = TRUE;
    cursorOn = FALSE;
    blinkOn();
    if ( e->reason() == QFocusEvent::Tab )
	selectAll();
}


/*!
  Handles the cursor blinking and selection copying.
*/

void QLineEdit::focusOutEvent( QFocusEvent * e )
{
    if ( style() == WindowsStyle ) {
#if defined(_WS_X11_)
	// X11 users are very accustomed to "auto-copy"
	copy();
#endif
    }
    if ( e->reason() != QFocusEvent::ActiveWindow )
	deselect();
    d->dragTimer.stop();
    if ( cursorOn )
	blinkSlot();
}

/*!
  Handles selection copying.
*/
void QLineEdit::leaveEvent( QEvent * )
{
#if defined(_WS_X11_)
    if ( style() == WindowsStyle )
	copy(); // X11 users are very accustomed to "auto-copy"
#endif
}


/*!
  Handles paint events for the line edit.
*/

void QLineEdit::paintEvent( QPaintEvent *e )
{
    if ( d->offsetDirty )
	updateOffset();
    if ( !d->pm || d->pmDirty ) {
	makePixmap();
	QPainter p( d->pm, this );

	const QColorGroup & g = colorGroup();
	QColor bg = isEnabled() ? g.base() : g.background();
	QFontMetrics fm = fontMetrics();
	int markBegin = minMark();
	int markEnd = maxMark();

	p.fillRect( 0, 0, width(), height(), bg );

	QString display = displayText();
	QString before = display.mid( 0, markBegin );
	QString marked = display.mid( markBegin, markEnd - markBegin );
	QString after = display.mid( markEnd, display.length() );

	int y = (d->pm->height() + fm.height())/2 - fm.descent() - 1 ;

	int x = offset + 2;
	int w;

	w = fm.width( before );
	if ( x < d->pm->width() && x + w >= 0 ) {
	    p.setPen( g.text() );
	    p.drawText( x, y, before );
	}
	x += w;
	
	w = fm.width( marked );
	if ( x < d->pm->width() && x + w >= 0 ) {
	    p.fillRect( x, y-fm.ascent()-1, w, fm.height()+2,
			g.brush( QColorGroup::Highlight ) );
	    p.setPen( g.highlightedText() );
	    p.drawText( x, y, marked );
	}
	x += w;
	
	w = fm.width( after );
	if ( x < d->pm->width() && x + w >= 0 ) {
	    p.setPen( g.text() );
	    p.drawText( x, y, after );
	}
	// ... x += w;
	
	p.setPen( g.foreground() );

	d->cursorRepaintRect.setTop( y + (frame() ? 2 : 0) - fm.ascent() );
	d->cursorRepaintRect.setHeight( fm.height() );
	d->pmDirty = FALSE;
    }

    QPainter p( this );

    if ( frame() ) {
	style().drawPanel( &p, 0, 0, width(), height(), colorGroup(),
			   TRUE, style().defaultFrameWidth() );
	p.drawPixmap( 2, 2, *d->pm );
    } else {
	p.drawPixmap( 0, 0, *d->pm );
    }

    if ( hasFocus() ) {
	d->cursorRepaintRect
	    = QRect( offset + (frame() ? 2 : 0 ) +
		     fontMetrics().width( displayText().left( cursorPos ) ),
		     d->cursorRepaintRect.top(),
		     5, d->cursorRepaintRect.height() );

	int curYTop = d->cursorRepaintRect.y();
	int curYBot = d->cursorRepaintRect.bottom();
	int curXPos = d->cursorRepaintRect.x() + 2;
	if ( cursorOn && d->cursorRepaintRect.intersects( e->rect() ) ) {
	    p.drawLine( curXPos, curYTop, curXPos, curYBot );
	    if ( style() != WindowsStyle ) {
		p.drawLine( curXPos - 2, curYTop, curXPos + 2, curYTop );
		p.drawLine( curXPos - 2, curYBot, curXPos + 2, curYBot );
	    }
	}
	// Now is the optimal time to set this - all the repaint-minimization
	// then also reduces the number of calls to setMicroFocusHint().
	setMicroFocusHint( curXPos, curYTop, 1, curYBot-curYTop+1 );
    } else {
	delete d->pm;
	d->pm = 0;
    }

}


/*!
  Handles resize events for this widget.
*/

void QLineEdit::resizeEvent( QResizeEvent * )
{
    delete d->pm;
    d->pm = 0;
    updateOffset();
}


/*!
  Handles mouse press events for this widget.

  The left mouse button is used for moving the cursor, selecting text,
  and initiating drag-and-drop.  The right mouse button brings up an
  edit menu.
*/

void QLineEdit::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton ) {
	d->popup->setItemEnabled( d->id[ 0 ], !d->undoList.isEmpty() );
	d->popup->setItemEnabled( d->id[ 1 ], !d->redoList.isEmpty() );
	d->popup->setItemEnabled( d->id[ 2 ], hasMarkedText() );
	d->popup->setItemEnabled( d->id[ 3 ], hasMarkedText() );
	d->popup->setItemEnabled( d->id[ 4 ], (bool)QApplication::clipboard()->text().length() );
	d->popup->setItemEnabled( d->id[ 5 ], (bool)text().length() );
	int id = d->popup->exec( e->globalPos() );
	if ( id == d->id[ 0 ] )
	    undoInternal();
	else if ( id == d->id[ 1 ] )
	    redoInternal();
	else if ( id == d->id[ 2 ] )
	    cut();
	else if ( id == d->id[ 3 ] )
	    copy();
	else if ( id == d->id[ 4 ] )
	    paste();
	else if ( id == d->id[ 5 ] )
	    clear();
	else if ( id == d->id[ 6 ] )
	    selectAll();

	return;
    }

    d->inDoubleClick = FALSE;
    int newCP = xPosToCursorPos( e->pos().x() );
    int m1 = minMark();
    int m2 = maxMark();
    if ( hasMarkedText() && echoMode() == Normal &&
	 e->button() == LeftButton && m1 < newCP && m2 > newCP ) {
	d->dndTimer.start( QApplication::startDragTime(), TRUE );
	return;
    }

    m1 = QMIN( m1, cursorPos );
    m2 = QMAX( m2, cursorPos );
    markAnchor = newCP;
    newMark( markAnchor, FALSE );
    repaintArea( m1, m2 );
    dragScrolling = FALSE;
    d->mousePressed = TRUE;
}

/*
  \internal
*/

void QLineEdit::doDrag()
{
    QTextDrag *tdo = new QTextDrag( markedText(), this );
    if ( tdo->drag() ) {
	// ##### Delete original (but check if it went to me)
    }
}

/*!
  Handles mouse move events for the line edit, primarily for
  marking text.
*/

void QLineEdit::mouseMoveEvent( QMouseEvent *e )
{
    if ( d->dndTimer.isActive() ) {
	doDrag();
	return;
    }

    if ( !(e->state() & LeftButton) )
	return;

    int margin = frame() ? 4 : 2;

    if ( e->pos().x() < margin || e->pos().x() > width() - margin ) {
	if ( !dragScrolling ) {
	    dragScrolling = TRUE;
	    scrollingLeft = e->pos().x() < margin;
	    if ( scrollingLeft )
		newMark( xPosToCursorPos( 0 ), FALSE );
	    else
		newMark( xPosToCursorPos( width() ), FALSE );
	    d->dragTimer.start( scrollTime );
	}
    } else {
	dragScrolling = FALSE;
	int mousePos = xPosToCursorPos( e->pos().x() );
	int m1 = markDrag;
	newMark( mousePos, FALSE );
	repaintArea( m1, mousePos );
    }
}

/*!
  Handles mouse release events for this widget.
*/

void QLineEdit::mouseReleaseEvent( QMouseEvent * e )
{
    dragScrolling = FALSE;
    if ( d->dndTimer.isActive() ) {
	d->dndTimer.stop();
	deselect();
	setCursorPosition( xPosToCursorPos( e->pos().x() ) );
	return;
    }
    if ( d->inDoubleClick ) {
	d->inDoubleClick = FALSE;
	return;
    }

    if ( !d->mousePressed )
	return;
    d->mousePressed = FALSE;

#if defined(_WS_X11_)
    copy();
#else
    if ( style() == MotifStyle )
	copy();
#endif

    if ( e->button() == MidButton ) {
#if defined(_WS_X11_)
	insert( QApplication::clipboard()->text() );
#else
	if ( style() == MotifStyle )
	    insert( QApplication::clipboard()->text() );
#endif
	return;
    }

    if ( e->button() != LeftButton )
	return;

    int margin = frame() ? 4 : 2;
    if ( !QRect( margin, margin,
		 width() - 2*margin,
		 height() - 2*margin ).contains( e->pos() ) )
	return;

    int mousePos = xPosToCursorPos( e->pos().x() );
    int m1 = markDrag;
    newMark( mousePos, FALSE );
    repaintArea( m1, mousePos );
}


/*!
  Handles mouse double click events for this widget.

  Double clicking begins word marking rather than
  the normal character marking.
*/

void QLineEdit::mouseDoubleClickEvent( QMouseEvent * )
{
    d->inDoubleClick = TRUE;
    dragScrolling = FALSE;

    markWord( cursorPos );
}

/*!
  Moves the cursor leftwards one or more characters.
  \sa cursorRight()
*/

void QLineEdit::cursorLeft( bool mark, int steps )
{
    cursorRight( mark, -steps );
}

/*!
  Moves the cursor rightwards one or more characters.
  \sa cursorLeft()
*/

void QLineEdit::cursorRight( bool mark, int steps )
{
    int cp = cursorPos + steps;
    cp = QMAX( cp, 0 );
    cp = QMIN( cp, (int)tbuf.length() );
    if ( cp == cursorPos ) {
	if ( !mark )
	    deselect();
    } else if ( mark ) {
	newMark( cp );
	blinkOn();
    } else {
	setCursorPosition( cp );
	setSelection( cp, 0 );
    }
}

/*!
  Deletes the character to the left of the text cursor and moves the
  cursor one position to the left. If a text has been marked by the user
  (e.g. by clicking and dragging) the cursor will be put at the beginning
  of the marked text and the marked text will be removed.  \sa del()
*/

void QLineEdit::backspace()
{
    if ( hasMarkedText() ) {
	del();
    } else if ( cursorPos > 0 ) {
	if ( d->undo && d->needundo && !d->ignoreUndoWithDel ) {
	    if ( d->undoList.isEmpty() || d->undoList.last().str != tbuf ) {
		d->undoList += QLineEditUndoItem(tbuf, cursorPos );
		d->redoList.clear();
	    }
	}
	cursorLeft( FALSE );
	del();
    }
}

/*!
  Deletes the character on the right side of the text cursor. If a text
  has been marked by the user (e.g. by clicking and dragging) the cursor
  will be put at the beginning of the marked text and the marked text will
  be removed.  \sa backspace()
*/

void QLineEdit::del()
{
    QString test( tbuf);
    d->ignoreUndoWithDel = TRUE;
    if ( d->undo && ( (d->needundo && !d->ignoreUndoWithDel) || hasMarkedText() ) ) {
	if ( d->undoList.isEmpty() || d->undoList.last().str != tbuf ) {
	    d->undoList += QLineEditUndoItem(tbuf, cursorPos );
	    d->redoList.clear();
	}
    }

    if ( hasMarkedText() ) {
	test.remove( minMark(), maxMark() - minMark() );
	validateAndSet( test, minMark(), minMark(), minMark() );
    } else if ( cursorPos != (int)tbuf.length() ) {
	test.remove( cursorPos, 1 );
	validateAndSet( test, minMark(), minMark(), minMark() );
    }
}

/*!
  Moves the text cursor to the left end of the line. If mark is TRUE text
  will be marked towards the first position, if not any marked text will
  be unmarked if the cursor is moved.  \sa end()
*/

void QLineEdit::home( bool mark )
{
    cursorRight( mark, -cursorPos );
}

/*!
  Moves the text cursor to the right end of the line. If mark is TRUE text
  will be marked towards the last position, if not any marked text will
  be unmarked if the cursor is moved.
  \sa home()
*/

void QLineEdit::end( bool mark )
{
    cursorRight( mark, tbuf.length()-cursorPos );
}


void QLineEdit::newMark( int pos, bool c )
{
    if ( markDrag != pos || cursorPos != pos )
	d->pmDirty = TRUE;
    markDrag = pos;
    setCursorPosition( pos );
    if ( c && style() == MotifStyle )
	copy();
}


void QLineEdit::markWord( int pos )
{
    int i = pos - 1;
    while ( i >= 0 && tbuf[i].isPrint() && !tbuf[i].isSpace() )
	i--;
    i++;
    int newAnchor = i;

    i = pos;
    while ( tbuf[i].isPrint() && !tbuf[i].isSpace() )
	i++;
    if ( style() != MotifStyle ) {
	while( tbuf[i].isSpace() )
	    i++;
	setCursorPosition( i );
    }
    int newDrag = i;
    setSelection( newAnchor, newDrag - newAnchor );

    if ( style() == MotifStyle )
	copy();
}


/*! Copies the marked text to the clipboard, if there is any and
  if echoMode() is Normal.

  \sa cut() paste()
*/

void QLineEdit::copy() const
{
    QString t = markedText();
    if ( !t.isEmpty() && echoMode() == Normal ) {
	disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
	QApplication::clipboard()->setText( t );
	connect( QApplication::clipboard(), SIGNAL(dataChanged()),
		 this, SLOT(clipboardChanged()) );
    }
}


/*!
  Inserts the clipboard's text at the cursor position, deleting any
  previous marked text.

  If the end result is not acceptable for the current validator,
  nothing happens.

  \sa copy() cut()
*/

void QLineEdit::paste()
{
    insert( QApplication::clipboard()->text() );
}

/*!
  Copies the marked text to the clipboard and deletes it, if there is
  any.

  If the current validator disallows deleting the marked text, cut()
  will copy it but not delete it.

  \sa copy() paste()
*/

void QLineEdit::cut()
{
    QString t = markedText();
    if ( !t.isEmpty() ) {
	copy();
	del();
    }
}

/*!
  Sets the alignment of the line edit. Possible Values are Qt::AlignLeft,
  Qt::AlignRight and Qt::AlignCenter - see Qt::AlignmentFlags.
  \sa alignment()
*/
void QLineEdit::setAlignment( int flag ){
    if ( flag == Qt::AlignRight ||
	 flag == Qt::AlignCenter ||
	 flag == Qt::AlignLeft ) {
	alignmentFlag = flag;
	updateOffset();
    }
}

/*!
  Returns the alignment of the line edit. Possible Values
  are Qt::AlignLeft, Qt::AlignRight and Qt::AlignCenter.

  \sa setAlignment(), Qt::AlignmentFlags
*/

int QLineEdit::alignment() const
{
    return alignmentFlag;
}

/*!
  This private slot is activated when this line edit owns the clipboard and
  some other widget/application takes over the clipboard. (X11 only)
*/

void QLineEdit::clipboardChanged()
{
#if defined(_WS_X11_)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    deselect();
#endif
}



int QLineEdit::lastCharVisible() const
{
    int tDispWidth = width() - (frame() ? 8 : 4);
    return xPosToCursorPos( tDispWidth );
}


int QLineEdit::minMark() const
{
    return markAnchor < markDrag ? markAnchor : markDrag;
}


int QLineEdit::maxMark() const
{
    return markAnchor > markDrag ? markAnchor : markDrag;
}



/*!  Sets the line edit to draw itself inside a two-pixel frame if \a
  enable is TRUE, and to draw itself without any frame if \a enable is
  FALSE.

  The default is TRUE.

  \sa frame()
*/

void QLineEdit::setFrame( bool enable )
{
    if ( d->frame == enable )
	return;

    d->frame = enable;
    d->pmDirty = TRUE;
    updateOffset();
}


/*!  Returns TRUE if the line edit draws itself inside a frame, FALSE
  if it draws itself without any frame.

  The default is to use a frame.

  \sa setFrame()
*/

bool QLineEdit::frame() const
{
    return d ? d->frame : TRUE;
}


/*!  Sets the echo mode of the line edit widget.

  The echo modes available are:
  <ul>
  <li> \c Normal - display characters as they are entered.  This is
	the default.
  <li> \c NoEcho - do not display anything. This may be appropriate
	for passwords where even the length of the password should
	be kept secret.
  <li> \c Password - display asterisks instead of the characters
	actually entered.
  </ul>

  The widget's display, and the ability to copy or drag the
  text is affected by this setting.

  \sa echoMode() EchoMode displayText()
*/

void QLineEdit::setEchoMode( EchoMode mode )
{
    if ( d->mode == mode )
	return;

    d->mode = mode;
    d->pmDirty = TRUE;
    updateOffset();
}


/*!
  Returns the echo mode of the line edit.

  \sa setEchoMode() EchoMode
*/

QLineEdit::EchoMode QLineEdit::echoMode() const
{
    return d ? d->mode : Normal;
}


/*!
  Returns a size which fits the contents of the line edit.

  The width returned tends to be enough for about 15-20 characters.
*/

QSize QLineEdit::sizeHint() const
{
    QFontMetrics fm( font() );
    int h = fm.height();
    int w = fm.width( 'x' ) * 17; // "some"
    if ( frame() ) {
	h += 8;
	if ( style() == WindowsStyle && h < 26 )
	    h = 22;
	return QSize( w + 8, h );
    } else {
	return QSize( w + 4, h + 4 );
    }
}



/*!
  Returns a minimum size for the line edit.

  The width returned is enough for at least one character.
*/

QSize QLineEdit::minimumSizeHint() const
{
    QFontMetrics fm( font() );
    int h = fm.height();
    int w = fm.maxWidth();
    if ( frame() ) {
	h += 8;
	if ( style() == WindowsStyle && h < 26 )
	    h = 22;
	return QSize( w + 8, h );
    } else {
	return QSize( w + 4, h + 4 );
    }
}



/*!
  Specifies that this widget can use more, but is able to survive on
  less, horizontal space; and is fixed vertically.
*/

QSizePolicy QLineEdit::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
}


/*!
  Sets this line edit to accept input only as accepted by \a v,
  allowing arbitrary constraints on the text which the user can edit.

  If \a v == 0, remove the current input validator.  The default
  is no input validator (ie. any input is accepted up to maxLength()).

  \sa validator() QValidator
*/


void QLineEdit::setValidator( const QValidator * v )
{
    d->validator = v;
}

/*!
  Returns a pointer to the current input validator, or 0 if no
  validator has been set.

  \sa setValidator()
*/

const QValidator * QLineEdit::validator() const
{
    return d ? d->validator : 0;
}


/*!  This slot is equivalent to setValidator( 0 ). */

void QLineEdit::clearValidator()
{
    setValidator( 0 );
}



/*!
  Reimplemented to accept text drags entering the line edit.
*/
void QLineEdit::dragEnterEvent( QDragEnterEvent *e )
{
    if ( QTextDrag::canDecode(e) )
	e->accept( rect() );
}


/*!
  Reimplemented to accept text drops into the line edit.
*/
void QLineEdit::dropEvent( QDropEvent *e )
{
    QString str;
    if ( QTextDrag::decode( e, str ) ) {
	if ( !hasMarkedText() )
	    setCursorPosition( e->pos().x() );
	insert( str );
	e->accept();
    } else {
	e->ignore();
    }
}


/*!  This private slot handles cursor blinking. */

void QLineEdit::blinkSlot()
{
    if ( hasFocus() || cursorOn ) {
	cursorOn = !cursorOn;
	if ( d->pm && !d->pmDirty && d->cursorRepaintRect.isValid() )
	    repaint( d->cursorRepaintRect, FALSE );
	else
	    repaint( FALSE );
    }
    if ( hasFocus() )
	d->blinkTimer.start( QApplication::cursorFlashTime()/2, TRUE );
    else
	d->blinkTimer.stop();
}


/*!  This private slot handles drag-scrolling. */

void QLineEdit::dragScrollSlot()
{
    if ( !hasFocus() || !dragScrolling )
	d->dragTimer.stop();
    else if ( scrollingLeft )
	cursorLeft( TRUE );
    else
	cursorRight( TRUE );
}


/*!  Validates and perhaps sets this line edit to contain \a newText
  with the cursor at position newPos, with marked text from \a
  newMarkAnchor to \a newMarkDrag.  Returns TRUE if it changes the line
  edit and FALSE if it doesn't.

  Linebreaks in \a newText are converted to spaces, and it is
  truncated to maxLength() before testing its validity.

  Repaints and emits textChanged() if appropriate.
*/

bool QLineEdit::validateAndSet( const QString &newText, int newPos,
				int newMarkAnchor, int newMarkDrag )
{
    QString t( newText );
    for ( uint i=0; i<t.length(); i++ ) {
	if ( t[(int)i] < ' ' )  // unprintable/linefeed becomes space
	    t[(int)i] = ' ';
    }
    t.truncate( maxLength() );

    const QValidator * v = validator();

    if ( v && v->validate( t, newPos ) == QValidator::Invalid &&
	 v->validate( tbuf, cursorPos ) != QValidator::Invalid ) {
	return FALSE;
    }

    bool tc = ( t != tbuf );

    // okay, it succeeded
    if ( newMarkDrag != markDrag ||
	 newMarkAnchor |! markAnchor ||
	 newPos != cursorPos ||
	 tc ) {
	int minP = QMIN( cursorPos, minMark() );
	int maxP = QMAX( cursorPos, maxMark() );

	cursorPos = newPos;
	markAnchor = newMarkAnchor;
	markDrag = newMarkDrag;

	minP = QMIN( minP, QMIN( cursorPos, minMark() ) );
	int i = 0;
	while( i < minP && t[i] == tbuf[i] )
	    i++;
	minP = i;

	maxP = QMAX( maxP, QMAX( cursorPos, maxMark() ) );
	if ( fontMetrics().width( t ) < fontMetrics().width( tbuf ) )
	    maxP = t.length();
	tbuf = t;

	repaintArea( minP, maxP );
    }
    if ( tc ) {
	ed = TRUE;
	emit textChanged( tbuf );
    }
    return TRUE;
}


/*!  Removes any selected text, inserts \a newText,
  validates the result and if it is valid, sets it as the new contents
  of the line edit.

*/

void QLineEdit::insert( const QString &newText )
{
    QString t( newText );
    if ( t.isEmpty() )
	return;

    for ( int i=0; i<(int)t.length(); i++ )
	if ( t[i] < ' ' )  // unprintable/linefeed becomes space
	    t[i] = ' ';

    QString test( tbuf );
    int cp = cursorPos;
    if ( d->undo && ( d->needundo || hasMarkedText() ) ) {
	if ( d->undoList.isEmpty() || d->undoList.last().str != tbuf ) {
	    d->undoList += QLineEditUndoItem(tbuf, cursorPos );
	    d->redoList.clear();
	    d->needundo = FALSE;
	}
    }
    if ( hasMarkedText() ) {
	test.remove( minMark(), maxMark() - minMark() );
	cp = minMark();
    }
    test.insert( cp, t );
    int ncp = QMIN( cp+t.length(), (uint)maxLength() );
    blinkOn();
    validateAndSet( test, ncp, ncp, ncp );
}


/*!  Repaints all characters from \a from to \a to.  If cursorPos is
  between from and to, ensures that cursorPos is visible.  */

void QLineEdit::repaintArea( int from, int to )
{
    QString buf = displayText();

    int a, b;
    if ( from < to ) {
	a = from;
	b = to;
    } else {
	a = to;
	b = from;
    }

    d->pmDirty = TRUE;
    if ( d->offsetDirty || cursorPos >= a && cursorPos <= b )
	updateOffset();
    if ( !d->pmDirty )
	return;

    QFontMetrics fm = fontMetrics();
    int x = fm.width( buf.left( a ) ) + offset - 2 + (frame() ? 2 : 0);
    QRect r( x, 0, fm.width( buf.mid( a, b-a ) ) + 5, height() );
    r = r.intersect( rect() );
    if ( !r.isValid() )
	return;
    if ( b >= (int)buf.length() )
	r.setRight( width() );
    repaint( r, FALSE );
}


/*!  \reimp */

void QLineEdit::setEnabled( bool e )
{
    d->pmDirty = TRUE;
    QWidget::setEnabled( e );
}


/*! \reimp */

void QLineEdit::setFont( const QFont & f )
{
    d->pmDirty     = TRUE;
    d->offsetDirty = TRUE;
    QWidget::setFont( f );
}


/*!  Syntactic sugar for setText( "" ), provided to match no-argument
  signals.
*/

void QLineEdit::clear()
{
    setText( QString::fromLatin1("") );
}


/*!  Sets the marked area of this line edit to start at \a start and
  be \a length characters long. */

void QLineEdit::setSelection( int start, int length )
{
    int b, e;
    b = QMIN( markAnchor, markDrag );
    e = QMAX( markAnchor, markDrag );
    b = QMIN( b, start );
    e = QMAX( e, start + length );
    markAnchor = start;
    markDrag = start + length;
    repaintArea( b, e );
}


/*!  Sets the cursor position for this line edit to \a newPos and
  repaints accordingly.  \sa cursorPosition() */

void QLineEdit::setCursorPosition( int newPos )
{
    if ( newPos == cursorPos )
	return;
    newPos = QMIN( newPos, (int)tbuf.length() );
    newPos = QMAX( newPos, 0 );
    int b, e;
    b = QMIN( newPos, cursorPos );
    e = QMAX( newPos, cursorPos );
    cursorPos = newPos;
    blinkOn();
    repaintArea( b, e );
}


/*!  Returns the current cursor position for this line edit.  \sa
  setCursorPosition() */

int QLineEdit::cursorPosition() const
{
    return cursorPos;
}


/*! \reimp */

void QLineEdit::setPalette( const QPalette & p )
{
    d->pmDirty = TRUE;
    QWidget::setPalette( p );
}


/*!  Sets the edited flag of this line edit to \a on.  The edited flag
is never read by QLineEdit, and is changed to TRUE whenever the user
changes its contents.

This is useful e.g. for things that need to provide a default value,
but cannot find the default at once.  Just open the line edit without
the best default and when the default is known, check the edited()
return value and set the line edit's contents if the user has not
started editing the line edit.

\sa edited()
*/

void QLineEdit::setEdited( bool on )
{
    ed = on;
}


/*!  Returns the edited flag of the line edit.  If this returns FALSE,
the line edit's contents have not been changed since the construction
of the QLineEdit (or the last call to setEdited( FALSE ), if any).  If
it returns true, the contents have been edited, or setEdited( TRUE )
has been called.

\sa setEdited()
*/

bool QLineEdit::edited() const
{
    return ed;
}

/*!
  Moves the cursor one word to the right.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordBackward()
*/
void QLineEdit::cursorWordForward( bool mark )
{
    int i = cursorPos;
    while ( i < (int) tbuf.length() && !tbuf[i].isSpace() )
	++i;
    while ( i < (int) tbuf.length() && tbuf[i].isSpace() )
	++i;
    cursorRight( mark, i - cursorPos );
}


/*!
  Moves the cursor one word to the left.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordForward()
*/
void QLineEdit::cursorWordBackward( bool mark )
{
    int i = cursorPos;
    while ( i > 0 && tbuf[i-1].isSpace() )
	--i;
    while ( i > 0 && !tbuf[i-1].isSpace() )
	--i;
    cursorLeft( mark, cursorPos - i );
}


void QLineEdit::updateOffset()
{
    if ( !isVisible() ) {
	d->offsetDirty = TRUE;
	return;
    }
    d->offsetDirty = FALSE;
    makePixmap();
    QFontMetrics fm = fontMetrics();
    int textWidth = fm.width( displayText() )+4;
    int w = d->pm->width();
    int old = offset;

    // there are five cases, so we consider each in turn.
    if ( textWidth < 5 ) {
	// nothing is to be drawn.  okay.
	offset = 0;
    } else if ( textWidth > w ) {
	// may need to scroll.

	QString dt = displayText();
	dt += QString::fromLatin1( "  " );
	dt = dt.left( cursorPos + 2 );
	if ( cursorPos < 3 )
	    offset = 0;
	else if ( fm.width( dt.left( cursorPos - 2 ) ) + offset < 0 )
	    offset = -fm.width( dt.left( cursorPos - 2 ) );
	else if ( fm.width( dt ) + offset > w )
	    offset = w - fm.width( dt );
    } else if ( alignmentFlag == Qt::AlignRight ) {
	// right-aligned text, space for all of it
	offset = w - textWidth;
    } else if ( alignmentFlag == Qt::AlignCenter ) {
	// center-aligned text, space for all of it
	offset = (w - textWidth)/2;
    } else {
	// default: left-aligned, space for all of it.
	offset = 0;
    }

    if ( old == offset && !d->pmDirty )
	return;

    d->pmDirty = TRUE;
    update();
}


/*! Returns the index of the character to whose left edge \a goalx is
  closest.
*/

int QLineEdit::xPosToCursorPos( int goalx ) const
{
    int x1, x2;
    x1 = offset;
    int i = 0;
    QFontMetrics fm = fontMetrics();
    QString s = displayText();
    goalx -= (frame() ? 4 : 2);

    while( i < (int) s.length() ) {
	x2 = x1 + fm.width( s[i] );
	if ( QABS( x1 - goalx ) < QABS( x2 - goalx ) )
	    return i;
	i++;
	x1 = x2;
    }
    return i;
}


/*!  Starts the thing blinking, or makes sure it's displayed at once. */

void QLineEdit::blinkOn()
{
    if ( !hasFocus() )
	return;

    d->blinkTimer.start( cursorOn?QApplication::cursorFlashTime() / 2 : 0, TRUE );
    blinkSlot();
}


void QLineEdit::makePixmap() const
{
    if ( d->pm )
	return;
    d->pm = new QPixmap(frame()?QSize(width()-4,height()-4):size());
    d->pmDirty = TRUE;
}


void QLineEdit::undoInternal()
{
    if ( d->undoList.isEmpty() )
	return;
    d->undo = FALSE;	

    d->redoList += QLineEditUndoItem(tbuf, cursorPos );
    setText( d->undoList.last().str );
    setCursorPosition( d->undoList.last().pos );
    d->undoList.remove( d->undoList.fromLast() );
    if ( d->undoList.count() > 10 )
	d->undoList.remove( d->undoList.begin() );
    d->undo = TRUE;
    d->needundo = TRUE;
}

void QLineEdit::redoInternal()
{
    if ( d->redoList.isEmpty() )
	return;
    d->undo = FALSE;	
    d->undoList += QLineEditUndoItem(tbuf, cursorPos );
    setText( d->redoList.last().str );
    setCursorPosition( d->redoList.last().pos );
    d->redoList.remove( d->redoList.fromLast() );
    d->undo = TRUE;
    d->needundo = TRUE;
}
