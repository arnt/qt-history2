/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlineedit.cpp#245 $
**
** Implementation of QLineEdit widget class
**
** Created : 941011
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

#include "qlineedit.h"
#ifndef QT_NO_LINEEDIT
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
#include "qguardedptr.h"
#include <ctype.h>
#include "../kernel/qrichtext_p.h"


struct UndoRedoInfo {
    enum Type { Invalid, Insert, Delete, Backspace, RemoveSelected };
    UndoRedoInfo( QTextParag *p ) : type( Invalid ), parag( p ) {
	text = QString::null; index = -1;
    }
    void clear() {
	if ( valid() ) {
	    if ( type == Insert )
		parag->addCommand( new QTextInsertCommand( parag, index, text ) );
	    else if ( type != Invalid )
		parag->addCommand( new QTextDeleteCommand( parag, index, text ) );
	}
	text = QString::null;
	index = -1;
    }
    bool valid() const { return !text.isEmpty() && index >= 0; }

    QString text;
    int index;
    Type type;
    QTextParag *parag;
};

struct QLineEditPrivate {
    QLineEditPrivate( QLineEdit * l ):
	frame(TRUE), readonly( FALSE ),
	cursorOn( FALSE ), inDoubleClick( FALSE ),
	mousePressed( FALSE ),
	dnd_primed( FALSE ), ed( FALSE ),
	mode(QLineEdit::Normal),
	maxLen( 32767), offset( 0 ),
	selectionStart( 0 ),
	validator( 0 ),
	pm(0),
	blinkTimer( l, "QLineEdit blink timer" ),
	dndTimer( l, "DnD Timer" ),
	parag( new QTextParag( 0, 0, 0, FALSE ) ),
	dragTimer( l, "QLineEdit drag timer" ),
	undoRedoInfo( parag )
    {
	parag->formatter()->setWrapEnabled( FALSE );
	cursor = new QTextCursor( 0 );
	cursor->setParag( parag );
	pm = new QPixmap;
    }

    ~QLineEditPrivate()
    {
	delete parag;
	delete cursor;
	delete pm;
    }
    void getTextObjects( QTextParag **p, QTextCursor **c )
    {
	if ( mode == QLineEdit::Password ) {
	    *p = new QTextParag( 0, 0, 0, FALSE);
	    (*p)->formatter()->setWrapEnabled( FALSE );
	    *c = new QTextCursor( 0 );
	    (*c)->setParag( *p );
	    (*p)->append( displayText() );
	    (*c)->setIndex( cursor->index() );
	} else {
	    *p = parag;
	    *c = cursor;
	}
    }
    void releaseTextObjects( QTextParag **p, QTextCursor **c )
    {
	if ( mode == QLineEdit::Password ) {
	    cursor->setIndex( (*c)->index() );
	    delete *p;
	    delete *c;
	}
    }
    QString displayText() const
    {
	QString res;

	switch( mode ) {
	    case QLineEdit::Normal:
		res = parag->string()->toString();
		res.remove( res.length() - 1, 1 );
		break;
	    case QLineEdit::NoEcho:
		res = QString::fromLatin1("");
		break;
	    case QLineEdit::Password:
		res.fill( '*', parag->length() -1);
		break;
	}
	return res;
    }

    void checkUndoRedoInfo( UndoRedoInfo::Type t ) {
	if ( undoRedoInfo.valid() && t != undoRedoInfo.type ) {
	    undoRedoInfo.clear();
// 	emitUndoAvailable( doc->commands()->isUndoAvailable() );
// 	emitRedoAvailable( doc->commands()->isRedoAvailable() );
	}
	undoRedoInfo.type = t;
    }

    bool frame 			: 1;
    bool readonly 			: 1;
    bool cursorOn 			: 1;
    bool inDoubleClick 		: 1;
    bool mousePressed 		: 1;
    bool dnd_primed 			: 1;
    bool ed 			: 1;
    QLineEdit::EchoMode mode 	: 2;
    int maxLen;
    int offset;
    int selectionStart;
    const QValidator * validator;
    QPixmap *pm;
    QTimer blinkTimer;

    QTimer dndTimer;
    QTextParag *parag;
    QTextCursor *cursor;
    QPoint dnd_startpos;
    QTimer dragTimer;
    UndoRedoInfo undoRedoInfo;
    QPoint lastMovePos;

};


// REVISED: arnt
/*!
  \class QLineEdit qlineedit.h

  \brief The QLineEdit widget is a one-line text editor.

  \ingroup basic

  A line edit allows the user to enter and edit a single line of plain
  text, with a useful collection of editing functions, including undo
  & redo, cut & paste, and drag & drop.

  By changing the echoMode() of a line edit it can also be used as a
  "write-only" field, for inputs such as passwords.

  The length of the field can be constrained to a maxLength(), or the
  value can be arbitrarily constrained by setting a validator().

  A closely related class is QTextEdit which allows multi-line editing
  including rich text formatting of the text.

  The default QLineEdit object has its own frame as specified by the
  Windows/Motif style guides, you can turn off the frame by calling
  setFrame( FALSE ).

  The default key bindings are described in keyPressEvent().  A
  right-mouse-button menu presents a number of the editing commands to
  the user.

  <img src=qlined-m.png> <img src=qlined-w.png>

  \sa QTextEdit QLabel QComboBox
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

  \sa setEchoMode() echoMode()
*/


/*!
  \fn void QLineEdit::textChanged( const QString& )
  This signal is emitted every time the text changes.
  The argument is the new text.
*/


/*!
  Constructs a line edit with no text.

  The maximum text length is set to 32767 characters.

  The \e parent and \e name arguments are sent to the QWidget constructor.

  \sa setText(), setMaxLength()
*/

QLineEdit::QLineEdit( QWidget *parent, const char *name )
    : QWidget( parent, name, WRepaintNoErase )
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
    : QWidget( parent, name, WRepaintNoErase )
{
    init();
    setText( contents );
}


/*!
  Destructs the line edit.
*/

QLineEdit::~QLineEdit()
{
    delete d;
}


/*! Contains initialization common to both constructors. */

void QLineEdit::init()
{
    d = new QLineEditPrivate( this );
    connect( &d->blinkTimer, SIGNAL(timeout()),
	     this, SLOT(blinkSlot()) );
    connect( &d->dragTimer, SIGNAL(timeout()),
	     this, SLOT(dragSlot()) );

#ifndef QT_NO_DRAGANDDROP
    connect( &d->dndTimer, SIGNAL(timeout()),
	     this, SLOT(doDrag()) );
     setAcceptDrops( TRUE );
#endif

#ifndef QT_NO_CURSOR
    setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif

    setFocusPolicy( StrongFocus );
    //   Specifies that this widget can use more, but is able to survive on
    //   less, horizontal space; and is fixed vertically.
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    setBackgroundMode( PaletteBase );
    setKeyCompression( TRUE );
}


/*!
  Sets the line edit text to \e text, clears the selection and moves
  the cursor to the end of the line.

  If necessary the text is truncated to maxLength().

  \sa text()
*/

void QLineEdit::setText( const QString &text )
{
    QString oldText = this->text();
    d->parag->truncate( 0 );
    d->parag->append( text );
    d->cursor->setIndex( d->parag->length() - 1 );
    deselect();
    update();
    if ( oldText != text )
	emit textChanged( text );
}


/*!
  Selects all text (i.e. marks it) and moves the cursor to the
  end. This is useful when a default value has been inserted,
  since if the user types before clicking on the widget the
  selected text will be erased.
*/

void QLineEdit::selectAll()
{
    d->selectionStart = 0;
    d->cursor->gotoEnd();
    updateSelection();
    update();
}



/*!
  Deselects all text (i.e. removes marking) and leaves the cursor at the
  current position.
*/

void QLineEdit::deselect()
{
    d->selectionStart = 0;
    d->parag->removeSelection( QTextDocument::Standard );
    update();
}


/*!
  Returns the text in the line.
  \sa setText()
*/

QString QLineEdit::text() const
{
    QString s = d->parag->string()->toString();
    s.remove( s.length() - 1, 1 ); // remove trailing space
    return s;
}



/*!  Returns the text that is displayed.  This is normally
the same as text(), but can be e.g. "*****" if EchoMode is Password or
"" if it is NoEcho.

\sa setEchoMode() text() EchoMode
*/

QString QLineEdit::displayText() const
{
    return d->displayText();
}



/*!
  Returns TRUE if part of the text has been marked by the user (e.g. by
  clicking and dragging).

  \sa markedText()
*/

bool QLineEdit::hasMarkedText() const
{
    return d->parag->hasSelection( QTextDocument::Standard ) &&
	d->parag->length() > 1 &&
	d->parag->selectionStart( QTextDocument::Standard ) > 0 &&
	d->parag->selectionEnd( QTextDocument::Standard ) > 0 &&
	d->parag->selectionStart( QTextDocument::Standard ) != d->parag->selectionEnd( QTextDocument::Standard );
}

/*!
  Returns the text marked by the user (e.g. by clicking and
  dragging), or a \link QString::operator!() null string\endlink
  if no text is marked.
  \sa hasMarkedText()
*/

QString QLineEdit::markedText() const
{
    return d->parag->string()->toString().mid( d->parag->selectionStart( 0 ), d->parag->selectionEnd( 0 ) - d->parag->selectionStart( 0 ) );
}

/*!
  Returns the maximum permitted length of the text in the editor.
  \sa setMaxLength()
*/

int QLineEdit::maxLength() const
{
    return d->maxLen;
}

/*!
  Set the maximum length of the text in the editor.  If the text is
  too long, it is chopped off at the limit. Any marked text will
  be unmarked.	The cursor position is set to 0 and the first part of the
  string is shown. \sa maxLength().
*/

void QLineEdit::setMaxLength( int m )
{
    d->maxLen = m;
    d->parag->truncate( d->maxLen );
    home( FALSE );
    update();
}

/*!
  \fn void  QLineEdit::returnPressed()
  This signal is emitted when the return or enter key is pressed.
*/


/*!
  Converts a key press into a line edit action.

  If return or enter is pressed and the current text is valid (or can be
  \link QValidator::fixup() made valid\endlink by the validator),
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
  <li><i> Control-K </i> Delete to end of line
  <li><i> Control-V </i> Paste the clipboard text into line edit.
  <li><i> Control-X </i> Move the marked text to the clipboard.
  <li><i> Control-Z </i> Undo the last operation.
  <li><i> Control-Y </i> Redo the last undone operation.
  </ul>
  In addition, the following key bindings are used on Windows:
  <ul>
  <li><i> Shift - Delete </i> Cut the marked text, copy to clipboard
  <li><i> Shift - Insert </i> Paste the clipboard text into line edit
  <li><i> Control - Insert </i> Copy the marked text to the clipboard
  </ul>

  All other keys with valid ASCII codes insert themselves into the line.
*/

void QLineEdit::keyPressEvent( QKeyEvent *e )
{
    int cursorPos = cursorPosition();
    if ( e->key() == Key_Enter || e->key() == Key_Return ) {
	const QValidator * v = validator();
	QString str = text();
	if ( !v || v->validate( str, cursorPos ) == QValidator::Acceptable ) {
	    emit returnPressed();
	    e->ignore();
	} else if ( v ) {
	    QString old = text();
	    QString vstr = old;
	    v->fixup( vstr );
	    if ( old != vstr ) {
		setText( vstr );
		update();
	    }
	    if ( v->validate( vstr, cursorPos ) == QValidator::Acceptable )
		emit returnPressed();
	    e->ignore();
	}
	return;
    }
    if ( !d->readonly ) {
	QString t = e->text();
	if ( !t.isEmpty() && (!e->ascii() || e->ascii()>=32) &&
	     e->key() != Key_Delete &&
	     e->key() != Key_Backspace ) {
	    insert( t );
	    return;
	}
    }
    bool unknown = FALSE;
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
	    home( e->state() & ShiftButton );
	    break;
	case Key_B:
	    cursorLeft( e->state() & ShiftButton );
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_C:
	    copy();
	    break;
#endif
	case Key_D:
	    if ( !d->readonly ) {
		del();
	    }
	    break;
	case Key_E:
	    end( e->state() & ShiftButton );
	    break;
	case Key_F:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_H:
	    if ( !d->readonly ) {
		backspace();
	    }
	    break;
	case Key_K:
	    if ( !d->readonly ) {
		QString t = text();
		int cursorPos = d->cursor->index();
		t.truncate( cursorPos );
		validateAndSet( t, cursorPos, cursorPos, cursorPos );
	    }
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_V:
	    if ( !d->readonly )
		insert( QApplication::clipboard()->text() );
	    break;
	case Key_X:
	    if ( !d->readonly && hasMarkedText() && echoMode() == Normal ) {
		copy();
		del();
	    }
	    break;
#if defined (Q_WS_WIN)
	case Key_Insert:
	    copy();
	    break;
#endif
#endif
	case Key_Right:
	    cursorWordForward( e->state() & ShiftButton );
	    break;
	case Key_Left:
	    cursorWordBackward( e->state() & ShiftButton );
	    break;
	case Key_Z:
 	    if ( !d->readonly )
 		undo();
	    break;
	case Key_Y:
 	    if ( !d->readonly )
 		redo();
	    break;
	default:
	    unknown = TRUE;
	}
    } else { // ### check for *no* modifier
	switch ( e->key() ) {
	case Key_Left:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_Right:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_Backspace:
	    if ( !d->readonly ) {
		backspace();
	    }
	    break;
	case Key_Home:
	    home( e->state() & ShiftButton );
	    break;
	case Key_End:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Delete:
	    if ( !d->readonly ) {
#if defined (Q_WS_WIN)
		if ( e->state() & ShiftButton ) {
		    cut();
		    break;
		}
#endif
		del();
	    }
	    break;
#if defined (Q_WS_WIN)
	case Key_Insert:
	    if ( !d->readonly && e->state() & ShiftButton )
		paste();
	    else
		unknown = TRUE;
	    break;
#endif
	case Key_F14: // Undo key on Sun keyboards
 	    if ( !d->readonly )
 		undo();
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_F16: // Copy key on Sun keyboards
	    copy();
	    break;
	case Key_F18: // Paste key on Sun keyboards
	    if ( !d->readonly )
		insert( QApplication::clipboard()->text() );
	    break;
	case Key_F20: // Cut key on Sun keyboards
	    if ( !d->readonly && hasMarkedText() && echoMode() == Normal ) {
		copy();
		del();
	    }
	    break;
#endif
	default:
	    unknown = TRUE;
	}
    }

    if ( unknown ) {				// unknown key
	e->ignore();
	return;
    }
}


/*!\reimp
*/

void QLineEdit::focusInEvent( QFocusEvent * e)
{
    d->cursorOn = FALSE;
    blinkOn();
    if ( e->reason() == QFocusEvent::Tab )
	selectAll();
    update();
}


/*!\reimp
*/

void QLineEdit::focusOutEvent( QFocusEvent * e )
{
    if ( e->reason() != QFocusEvent::ActiveWindow
	 && e->reason() != QFocusEvent::Popup )
	deselect();
    d->dragTimer.stop();
    if ( d->cursorOn )
	blinkSlot();
    update();
}

/*!\reimp
*/

void QLineEdit::paintEvent( QPaintEvent * )
{
    updateOffset();
    const QColorGroup & g = colorGroup();
    QPainter p( d->pm );
    QBrush bg = g.brush((isEnabled()) ? QColorGroup::Base :
			QColorGroup::Background);
    p.fillRect( 0, 0, width(), height(), bg );
    QTextParag *parag;
    QTextCursor *cursor;
    d->getTextObjects( &parag, &cursor );
    QTextFormat *f = parag->formatCollection()->format( font(), p.pen().color() );
    parag->setFormat( 0, parag->length(), f );
    f->removeRef();
    int fw = 0;
    if ( frame() )
	fw = style().defaultFrameWidth();
    QRect r( rect().x(), rect().y(), width() - 2 * ( 2 + fw ), rect().height() );
    parag->setDocumentRect( r );
    parag->invalidate(0);
    parag->format();
    int xoff = -d->offset + 2 + fw;
    int yoff = (height() - parag->rect().height())/2;
    if ( yoff < 0 )
	yoff = 0;
    // Reggie: Strange, if we add fw the text is drawn too much at the
    // bottom, might be that parag->paint() does some translation
    // yoff += fw;
    p.translate( xoff, yoff );
    if ( d->mode != NoEcho )
	parag->paint( p, colorGroup(), d->cursorOn && !d->readonly ? cursor : 0, TRUE );
    if ( frame() ) {
	p.translate( -xoff, -yoff );
	style().drawPanel( &p, 0, 0, width(), height(), colorGroup(),
			   TRUE, style().defaultFrameWidth() );
    }
    p.end();

    bitBlt( this, 0, 0, d->pm );
    if ( d->mode == Password ) {
	delete parag;
	delete cursor;
    }
}


/*!\reimp
*/

void QLineEdit::resizeEvent( QResizeEvent *e )
{
    d->pm->resize( e->size() );
}


/*! \reimp
*/
bool QLineEdit::event( QEvent * e )
{
    if ( e->type() == QEvent::AccelOverride && !d->readonly ) {
	QKeyEvent* ke = (QKeyEvent*) e;
	if ( ke->state() & ControlButton ) {
	    switch ( ke->key() ) {
	    case Key_A:
	    case Key_E:
#if defined (Q_WS_WIN)
	    case Key_Insert:
#endif
	    case Key_X:
	    case Key_V:
	    case Key_C:
	    case Key_Left:
	    case Key_Right:
	    ke->accept();
	    default:
		break;
	    }
	} else {
	    switch ( ke->key() ) {
	    case Key_Delete:
	    case Key_Home:
	    case Key_End:
	    case Key_Backspace:
		ke->accept();
	    default:
		break;
	    }
	}
    }
    return QWidget::event( e );
}


enum {
    IdUndo = 0,
    IdRedo = 1,
    IdCut = 2,
    IdCopy = 3,
    IdPaste = 4,
    IdClear = 5,
    IdSelectAll = 6
};

/*! \reimp
*/
void QLineEdit::mousePressEvent( QMouseEvent *e )
{
    d->undoRedoInfo.clear();
    if ( e->button() == RightButton ) {
	QGuardedPtr<QPopupMenu> popup = new QPopupMenu( this );
	int id[ 7 ];
 	id[ IdUndo ] = popup->insertItem( tr( "Undo" ) );
 	id[ IdRedo ] = popup->insertItem( tr( "Redo" ) );
	popup->insertSeparator();
#ifndef QT_NO_CLIPBOARD
	id[ IdCut ] = popup->insertItem( tr( "Cut" ) );
	id[ IdCopy ] = popup->insertItem( tr( "Copy" ) );
	id[ IdPaste ] = popup->insertItem( tr( "Paste" ) );
#endif
	id[ IdClear ] = popup->insertItem( tr( "Clear" ) );
	popup->insertSeparator();
	id[ IdSelectAll ] = popup->insertItem( tr( "Select All" ) );
 	popup->setItemEnabled( id[ IdUndo ],
 				  !d->readonly && d->parag->commands()->isUndoAvailable() );
 	popup->setItemEnabled( id[ IdRedo ],
 				  !d->readonly && d->parag->commands()->isRedoAvailable() );
#ifndef QT_NO_CLIPBOARD
	popup->setItemEnabled( id[ IdCut ],
				  !d->readonly && hasMarkedText() );
	popup->setItemEnabled( id[ IdCopy ], hasMarkedText() );
	popup->setItemEnabled( id[ IdPaste ],
				  !d->readonly
				  && !QApplication::clipboard()->text().isEmpty() );
#endif
	popup->setItemEnabled( id[ IdClear ],
				  !d->readonly && !text().isEmpty() );
	bool allSelected = (d->parag->selectionStart( 0 ) == 0 && d->parag->selectionEnd( 0 ) == (int)text().length() );
	popup->setItemEnabled( id[ IdSelectAll ],
				  (bool)text().length() && !allSelected );

	int r = popup->exec( e->globalPos() );
	delete popup;

	if ( r == id[ IdClear ] )
	    clear();
	else if ( r == id[ IdSelectAll ] )
	    selectAll();
 	else if ( r == id[ IdUndo ] )
 	    undo();
 	else if ( r == id[ IdRedo ] )
 	    redo();
#ifndef QT_NO_CLIPBOARD
	else if ( r == id[ IdCut ] )
	    cut();
	else if ( r == id[ IdCopy ] )
	    copy();
	else if ( r == id[ IdPaste ] )
	    paste();
#endif
	return;
    }

    d->inDoubleClick = FALSE;
    QPoint p( e->pos().x() + d->offset - 2 - style().defaultFrameWidth(), 0 );
    QTextParag *par;
    QTextCursor *c;
    d->getTextObjects(&par, &c);
    int oldPos = c->index();
    c->place( p, par );
#ifndef QT_NO_DRAGANDDROP
    if ( hasMarkedText() && echoMode() == Normal && !( e->state() & ShiftButton ) &&
	 e->button() == LeftButton ) {
	d->dndTimer.start( QApplication::startDragTime(), TRUE );
	d->dnd_primed = TRUE;
	d->dnd_startpos = e->pos();
	d->releaseTextObjects( &par, &c );
	return;
    }
#endif
    if ( !( e->state() && ShiftButton ) ) {
	d->selectionStart = d->cursor->index();
	par->setSelection( QTextDocument::Standard, d->selectionStart, d->selectionStart );
    } else {
	if ( par->selectionEnd( QTextDocument::Standard ) != oldPos &&
	     par->selectionStart( QTextDocument::Standard ) != oldPos )
	    d->selectionStart = oldPos;
	int s = d->selectionStart;
	int e = c->index();
	if ( s > e ) {
	    s = c->index();
	    e = d->selectionStart;
	}
	par->setSelection( QTextDocument::Standard, s, e );
    }
    d->releaseTextObjects( &par, &c);
    update();
    d->mousePressed = TRUE;
}

#ifndef QT_NO_DRAGANDDROP

/*
  \internal
*/

void QLineEdit::doDrag()
{
    d->dnd_primed = FALSE;
    QTextDrag *tdo = new QTextDrag( markedText(), this );
    if ( tdo->drag() ) {
	// ##### Delete original (but check if it went to me)
    }
}

#endif // QT_NO_DRAGANDDROP

/*!\reimp
*/
void QLineEdit::mouseMoveEvent( QMouseEvent *e )
{
#ifndef QT_NO_DRAGANDDROP
    if ( d->dndTimer.isActive() ) {
	d->dndTimer.stop();
	return;
    }

    if ( d->dnd_primed ) {
	if ( ( d->dnd_startpos - e->pos() ).manhattanLength() > QApplication::startDragDistance() )
	    doDrag();
	return;
    }
#endif

    if ( !(e->state() & LeftButton) )
	return;

    d->dragTimer.stop();
    d->lastMovePos = e->pos();
    dragSlot();
}

void QLineEdit::dragSlot()
{
    QPoint p( d->lastMovePos.x() + d->offset - 2 - style().defaultFrameWidth(), 0 );
    QTextParag *par;
    QTextCursor *c;
    d->getTextObjects(&par, &c);
    c->place( p, par );
    d->releaseTextObjects( &par, &c );
    updateSelection();
    update();
    if ( d->lastMovePos.x() < 0 || d->lastMovePos.x() > width() )
	d->dragTimer.start( 100, TRUE );
}

/*!\reimp
*/
void QLineEdit::mouseReleaseEvent( QMouseEvent * e )
{
    d->dnd_primed = FALSE;
    d->dragTimer.stop();
    if ( d->dndTimer.isActive() ) {
	d->dndTimer.stop();
	QPoint p( e->pos().x() + d->offset - 2 - style().defaultFrameWidth(), 0 );
	d->cursor->place( p, d->parag );
	deselect(); // does a repaint
	return;
    }
    if ( d->inDoubleClick ) {
	d->inDoubleClick = FALSE;
	return;
    }

    if ( !d->mousePressed )
	return;
    d->mousePressed = FALSE;

#ifndef QT_NO_CLIPBOARD
    if (QApplication::clipboard()->supportsSelection()) {
     	QApplication::clipboard()->setSelectionMode(TRUE);
	copy();
	QApplication::clipboard()->setSelectionMode(FALSE);
    }

    if ( !d->readonly && e->button() == MidButton ) {
	if (QApplication::clipboard()->supportsSelection()) {
	    QApplication::clipboard()->setSelectionMode(TRUE);
	    insert( QApplication::clipboard()->text() );
	    QApplication::clipboard()->setSelectionMode(TRUE);
	}
	return;
    }
#endif

    if ( e->button() != LeftButton )
	return;

    QPoint p( e->pos().x() + d->offset - 2 - style().defaultFrameWidth(), 0 );
    QTextParag *par;
    QTextCursor *c;
    d->getTextObjects(&par, &c);
    c->place( p, par );
    d->releaseTextObjects( &par, &c );
    update();
}


/*!\reimp
*/
void QLineEdit::mouseDoubleClickEvent( QMouseEvent * )
{
    d->inDoubleClick = TRUE;

    QTextCursor c1 = *d->cursor;
    QTextCursor c2 = *d->cursor;
    c1.gotoWordLeft();
    c2.gotoWordRight();

    d->parag->setSelection( QTextDocument::Standard, c1.index(), c2.index() );
    *d->cursor = c2;
    update();
}

/*!
  \obsolete
  For compatibilty with older applications only. Use cursorForward
  instead.
  \sa cursorForward()
*/
void QLineEdit::cursorRight( bool mark, int steps )
{
    cursorForward( mark, steps );
}

/*!
  \obsolete
  For compatibilty with older applications only. Use cursorBack
  instead.
  \sa cursorBackward()
*/
void QLineEdit::cursorLeft( bool mark, int steps )
{
    cursorForward( mark, -steps );
}

/*!
  Moves the cursor back one or more characters.
  \sa cursorForward()
*/
void QLineEdit::cursorBackward( bool mark, int steps )
{
    cursorRight( mark, -steps );
}

/*!
  Moves the cursor forward one or more characters.
  \sa cursorBackward()
*/

void QLineEdit::cursorForward( bool mark, int steps )
{
    if( steps > 0 )
	while( steps-- )
	    d->cursor->gotoRight();
    else
	while( steps++ )
	    d->cursor->gotoLeft();
    if ( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    update();
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
	removeSelectedText();
    } else if ( d->cursor->index() > 0 ) {
	d->checkUndoRedoInfo( UndoRedoInfo::Delete );
	if ( !d->undoRedoInfo.valid() ) {
	    d->undoRedoInfo.index = d->cursor->index();
	    d->undoRedoInfo.text = QString::null;
	}
	d->cursor->gotoLeft();
	d->undoRedoInfo.text.prepend( QString( d->cursor->parag()->at( d->cursor->index() )->c ) );
	d->undoRedoInfo.index = d->cursor->index();
	d->cursor->remove();
    }
    d->selectionStart = d->cursor->index();
    update();
    emit textChanged( text() );
}

/*!
  Deletes the character on the right side of the text cursor. If a text
  has been marked by the user (e.g. by clicking and dragging) the cursor
  will be put at the beginning of the marked text and the marked text will
  be removed.  \sa backspace()
*/

void QLineEdit::del()
{
    if ( hasMarkedText() ) {
	removeSelectedText();
    } else {
	d->checkUndoRedoInfo( UndoRedoInfo::Delete );
	if ( !d->undoRedoInfo.valid() ) {
	    d->undoRedoInfo.index = d->cursor->index();
	    d->undoRedoInfo.text = QString::null;
	}
	d->undoRedoInfo.text += d->cursor->parag()->at( d->cursor->index() )->c;
	d->cursor->remove();
    }
    d->selectionStart = d->cursor->index();
    update();
    emit textChanged( text() );
}

/*!
  Moves the text cursor to the beginning of the line. If mark is TRUE text
  will be marked towards the first position, if not any marked text will
  be unmarked if the cursor is moved.  \sa end()
*/

void QLineEdit::home( bool mark )
{
    d->cursor->gotoHome();
    if( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    update();
}

/*!
  Moves the text cursor to the end of the line. If mark is TRUE text
  will be marked towards the last position, if not any marked text will
  be unmarked if the cursor is moved.
  \sa home()
*/

void QLineEdit::end( bool mark )
{
    d->cursor->gotoEnd();
    if( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    update();
}


#ifndef QT_NO_CLIPBOARD

/*! Copies the marked text to the clipboard, if there is any and
  if echoMode() is Normal.

  \sa cut() paste()
*/

void QLineEdit::copy() const
{
    QString t = markedText();
    if ( !t.isEmpty() && echoMode() == Normal ) {
	disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);
	QApplication::clipboard()->setText( t );
	connect( QApplication::clipboard(), SIGNAL(dataChanged()),
		 this, SLOT(clipboardChanged()) );
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()),
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
    deselect();
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

#endif

/*!
  Sets the alignment of the line edit. Possible Values are Qt::AlignAuto, Qt::AlignLeft,
  Qt::AlignRight and Qt::Align(H)Center - see Qt::AlignmentFlags.
  \sa alignment()
*/
void QLineEdit::setAlignment( int flag )
{
    if ( flag == d->parag->alignment() )
	return;
    if ( !(flag & Qt::AlignVertical_Mask ) || flag & Qt::AlignCenter ) {
	d->parag->setAlignment( flag );
	d->parag->invalidate( 0 );
	d->parag->format();
	updateOffset();
	update();
    }
}

/*!
  Returns the alignment of the line edit. Possible Values
  are Qt::AlignAuto, Qt::AlignLeft, Qt::AlignRight and Qt::Align(H)Center.

  \sa setAlignment(), Qt::AlignmentFlags
*/

int QLineEdit::alignment() const
{
    return d->parag->alignment();
}

/*!
  This private slot is activated when this line edit owns the clipboard and
  some other widget/application takes over the clipboard. (X11 only)
*/

void QLineEdit::clipboardChanged()
{
#if defined(Q_WS_X11)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()),
		this, SLOT(clipboardChanged()) );
    deselect();
#endif
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
    update();
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
    update();
}


/*!
  Returns the echo mode of the line edit.

  \sa setEchoMode() EchoMode
*/

QLineEdit::EchoMode QLineEdit::echoMode() const
{
    return d->mode;
}

/*!
  Enables or disables read-only mode, where the user can cut-and-paste
  or drag-and-drop the text, but cannot edit it.
  They never see a cursor in this case.

  \sa setEnabled(), isReadOnly()
*/
void QLineEdit::setReadOnly( bool enable )
{
    d->readonly = enable;
#ifndef QT_NO_CURSOR
    setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
}

/*!
  Returns whether the line-edit is read-only.
  \sa setReadOnly()
*/
bool QLineEdit::isReadOnly() const
{
    return d->readonly;
}



/*!
  Returns a recommended size for the widget.

  The width returned is enough for a few characters, typically 15 to 20.
*/
QSize QLineEdit::sizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int h = fm.height();
    int w = fm.width( 'x' ) * 17; // "some"
    if ( frame() ) {
	h += 8;
	if ( style() == WindowsStyle && h < 22 )
	    h = 22;
	return QSize( w + 8, h ).expandedTo( QApplication::globalStrut() );
    } else {
	return QSize( w + 4, h + 4 ).expandedTo( QApplication::globalStrut() );
    }
}



/*!
  Returns a minimum size for the line edit.

  The width returned is enough for at least one character.
*/

QSize QLineEdit::minimumSizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int h = fm.height();
    int w = fm.maxWidth();
    if ( frame() ) {
	h += 8;
	if ( style() == WindowsStyle && h < 22 )
	    h = 22;
	return QSize( w + 8, h );
    } else {
	return QSize( w + 4, h + 4 );
    }
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

#ifndef QT_NO_DRAGANDDROP

/*! \reimp
*/
void QLineEdit::dragEnterEvent( QDragEnterEvent *e )
{
    if ( !d->readonly && QTextDrag::canDecode(e) )
	e->accept( rect() );
}


/*!\reimp
*/
void QLineEdit::dropEvent( QDropEvent *e )
{
    QString str;
    QCString plain = "plain";
    if ( !d->readonly && QTextDrag::decode( e, str, plain ) ) {
	if ( e->source() == this && hasMarkedText() )
	    deselect();
	if ( !hasMarkedText() ) {
	    QPoint p( e->pos().x() + d->offset - 2 - style().defaultFrameWidth(), 0 );
	    d->cursor->place( p, d->parag );
	}
	insert( str );
	e->accept();
    } else {
	e->ignore();
    }
}

#endif // QT_NO_DRAGANDDROP

/*!  This private slot handles cursor blinking. */

void QLineEdit::blinkSlot()
{
    if ( hasFocus() || d->cursorOn ) {
	d->cursorOn = !d->cursorOn;
	update();
    }
    if ( hasFocus() )
	d->blinkTimer.start( QApplication::cursorFlashTime()/2, TRUE );
    else
	d->blinkTimer.stop();
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
    QString t = newText;
    for ( uint i=0; i<t.length(); i++ ) {
	if ( t[(int)i] < ' ' )  // unprintable/linefeed becomes space
	    t[(int)i] = ' ';
    }
    t.truncate( maxLength() );

    QString old = d->parag->string()->toString();
    old.remove( old.length() - 1, 1 );

    const QValidator * v = validator();

    int pos = d->cursor->index();
    if ( v && v->validate( t, newPos ) == QValidator::Invalid &&
	 v->validate( old, pos ) != QValidator::Invalid ) {
	return FALSE;
    }

    // okay, it succeeded
    if( t != old )
	setText( t );

    d->cursor->setIndex( newPos );
    d->selectionStart = newMarkAnchor;
    d->parag->setSelection( QTextDocument::Standard, newMarkAnchor, newMarkDrag );
    repaint( FALSE );
    d->selectionStart = d->cursor->index();
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

    d->checkUndoRedoInfo( UndoRedoInfo::Insert );
    if ( !d->undoRedoInfo.valid() ) {
	d->undoRedoInfo.index = d->cursor->index();
	d->undoRedoInfo.text = QString::null;
    }

    d->ed = TRUE;

    for ( int i=0; i<(int)t.length(); i++ )
	if ( t[i] < ' ' )  // unprintable/linefeed becomes space
	    t[i] = ' ';

    if ( !d->validator ) {
	if( hasMarkedText() )
	    removeSelectedText();
	d->cursor->insert( t, FALSE );
	emit textChanged( text() );
    } else {
	QString text = d->parag->string()->toString();
	text.remove( text.length() - 1, 1 );
	int cp = d->cursor->index();
	if ( hasMarkedText() ) {
	    text.remove( d->parag->selectionStart(0), d->parag->selectionEnd(0) - d->parag->selectionStart( 0 ) );
	    cp = d->parag->selectionStart(0);
	}
	text.insert( cp, t );
	cp = QMIN( cp+t.length(), (uint)maxLength() );
	blinkOn();
	validateAndSet( text, cp, cp, cp );
    }
    update();
    d->selectionStart = d->cursor->index();
    d->undoRedoInfo.text += t;
}


/*!  Repaints all characters from \a from to \a to.  If cursorPos is
  between from and to, ensures that cursorPos is visible.

  Obsolete and provided for backwards compatibilty only.
*/
#ifndef QT_NO_COMPAT
void QLineEdit::repaintArea( int, int )
{
    update();
}
#endif

/*!  \reimp */

void QLineEdit::setEnabled( bool e )
{
    QWidget::setEnabled( e );
}


/*! \reimp */

void QLineEdit::setFont( const QFont & f )
{
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
    d->selectionStart = start;
    d->cursor->setIndex( start + length );
    updateSelection();
    update();
}


/*!  Sets the cursor position for this line edit to \a newPos and
  repaints accordingly.  \sa cursorPosition() */

void QLineEdit::setCursorPosition( int newPos )
{
    d->cursor->setIndex( newPos );
    deselect();
}


/*!  Returns the current cursor position for this line edit.  \sa
  setCursorPosition() */

int QLineEdit::cursorPosition() const
{
    return d->cursor->index();
}


/*! \reimp */

void QLineEdit::setPalette( const QPalette & p )
{
    QWidget::setPalette( p );
    update();
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
    d->ed = on;
}


/*!  Returns the edited flag of the line edit.  If this returns FALSE,
the line edit's contents have not been changed since the construction
of the QLineEdit (or the last call to either setText() or setEdited( FALSE ),
if any).  If it returns true, the contents have been edited, or
setEdited( TRUE ) has been called.

\sa setEdited()
*/

bool QLineEdit::edited() const
{
    return d->ed;
}

/*!
  Moves the cursor one word forward.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordBackward()
*/
void QLineEdit::cursorWordForward( bool mark )
{
    d->cursor->gotoWordRight();
    if( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    update();
}


/*!
  Moves the cursor one word backward.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordForward()
*/
void QLineEdit::cursorWordBackward( bool mark )
{
    d->cursor->gotoWordLeft();
    if( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    update();
}


/*!  Starts the thing blinking, or makes sure it's displayed at once. */

void QLineEdit::blinkOn()
{
    if ( !hasFocus() )
	return;

    d->blinkTimer.start( d->cursorOn?QApplication::cursorFlashTime() / 2 : 0, TRUE );
    blinkSlot();
}

void QLineEdit::updateOffset()
{ // must not call repaint() - paintEvent() calls this
    int textWidth = d->parag->rect().width();
    int w = width();
    int fw = 0;
    if ( frame() )
	fw = style().defaultFrameWidth();
    w -= 2*fw + 4;
    int cursorPos = d->cursor->x();

    if ( textWidth > w ) {
	if ( cursorPos < d->offset )
	    d->offset = cursorPos;
	else if ( cursorPos > d->offset + w )
	    d->offset = cursorPos - w;
    } else {
	d->offset = 0;
    }
}

void QLineEdit::updateSelection()
{
    int pos = d->cursor->index();
    int selectionStart = d->selectionStart;
    int selectionEnd;
    if ( pos > selectionStart ) {
	selectionEnd = pos;
    } else {
	selectionEnd = selectionStart;
	selectionStart = pos;
    }
    d->parag->setSelection( QTextDocument::Standard, selectionStart, selectionEnd );
}

void QLineEdit::removeSelectedText()
{
    d->checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
    int start = d->parag->selectionStart( 0 );
    int len = d->parag->selectionEnd( 0 ) - start;
    if ( !d->undoRedoInfo.valid() ) {
	d->undoRedoInfo.index = start;
	d->undoRedoInfo.text = QString::null;
    }
    d->undoRedoInfo.text = d->parag->string()->toString().mid( start, len );
    d->parag->remove( start, len );
    d->cursor->setIndex( start );
    deselect();
    d->undoRedoInfo.clear();
}

/*! Undoes the last operation */

void QLineEdit::undo()
{
    d->undoRedoInfo.clear();
    d->parag->undo( d->cursor );
    update();
}

/*! Redoes the last operation */

void QLineEdit::redo()
{
    d->undoRedoInfo.clear();
    d->parag->redo( d->cursor );
    update();
}

#endif
