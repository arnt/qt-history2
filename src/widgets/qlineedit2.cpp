/**********************************************************************
** $Id$
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
#include "qstyle.h"
#include "qwhatsthis.h"
#include <ctype.h>
#include "../kernel/qinternal_p.h"
#include "../kernel/qrichtext_p.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

#ifndef QT_NO_ACCEL
#include <qkeysequence.h>
#define ACCEL_KEY(k) "\t" + QString(QKeySequence( Qt::CTRL | Qt::Key_ ## k ))
#else
#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)
#endif

struct UndoRedoInfo {
    enum Type { Invalid, Insert, Delete, Backspace, RemoveSelected };
    UndoRedoInfo( QTextParagraph *p ) : type( Invalid ), parag( p ) {
	text = QString::null; index = -1;
    }
    bool valid() const { return !text.isEmpty() && index >= 0; }
    void clear( bool force = FALSE ) {
	if ( valid() || force ) {
	    QTextString s;
	    s.insert( 0, text, 0 );
	    if ( type == Insert )
		parag->addCommand( new QTextInsertCommand( parag, index, s.rawData() ) );
	    else if ( type != Invalid )
		parag->addCommand( new QTextDeleteCommand( parag, index, s.rawData() ) );
	}
	text = QString::null;
	index = -1;
    }

    QString text;
    int index;
    Type type;
    QTextParagraph *parag;
};

struct MaskInputData {
    enum Casemode { None, Upper, Lower };
    MaskInputData() {};
    MaskInputData ( QChar c, bool s, MaskInputData::Casemode cm )
	: maskChar( c ), separator( s ),  caseMode( cm ) {
    };

    QChar maskChar; // either the separator char or the inputmask
    bool separator;
    Casemode caseMode;

};

struct QLineEditPrivate {
    QLineEditPrivate( QLineEdit * l ):
	readonly( FALSE ),
	cursorOn( FALSE ),
	inDoubleClick( FALSE ),
	mousePressed( FALSE ),
	dnd_primed( FALSE ), ed( FALSE ),
	mode(QLineEdit::Normal),
	maxLen( 32767), offset( 0 ),
	selectionStart( 0 ),
	validator( 0 ),
	blinkTimer( l, "QLineEdit blink timer" ),
	dndTimer( l, "DnD Timer" ),
	parag( new QTextParagraph( 0, 0, 0, FALSE ) ),
	dragTimer( l, "QLineEdit drag timer" ),
	undoRedoInfo( parag ),
	dragEnabled( TRUE ),
	preeditStart(-1),
	preeditLength(-1),
	txtBuffer( "" ),
	passwordChar( '*' )
#ifndef QT_NO_CLIPBOARD
	,clipboard_mode( QClipboard::Clipboard )
#endif
    {
	parag->formatter()->setWrapEnabled( FALSE );
	cursor = new QTextCursor( 0 );
	cursor->setParagraph( parag );
	maskList = 0;
    }

    ~QLineEditPrivate()
    {
	delete parag;
	delete cursor;
	if ( maskList ) delete maskList;
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
		res.fill( passwordChar, parag->length() -1);
		break;
	}
	return res;
    }
    void getTextObjects( QTextParagraph **p, QTextCursor **c )
    {
	if ( mode == QLineEdit::Password ) {
	    *p = new QTextParagraph( 0, 0, 0, FALSE);
	    (*p)->formatter()->setWrapEnabled( FALSE );
	    *c = new QTextCursor( 0 );
	    (*c)->setParagraph( *p );
	    (*p)->append( displayText() );
	    (*c)->setIndex( cursor->index() );
	    (*p)->setAlignment( parag->alignment() );
	} else {
	    *p = parag;
	    *c = cursor;
	}
    }
    void releaseTextObjects( QTextParagraph **p, QTextCursor **c )
    {
	if ( mode == QLineEdit::Password ) {
	    cursor->setIndex( (*c)->index() );
	    delete *p;
	    delete *c;
	}
    }

    void checkUndoRedoInfo( UndoRedoInfo::Type t ) {
	bool compress = ( t == undoRedoInfo.type );
	if ( compress ) {
	    switch ( t ) {
	    case UndoRedoInfo::Insert:
		compress = ( undoRedoInfo.index + undoRedoInfo.text.length() ==
			     (uint) cursor->index() );
		break;
	    case UndoRedoInfo::Delete:
	    case UndoRedoInfo::Backspace:
		compress = ( undoRedoInfo.index == cursor->index() );
		break;
	    default:
		compress = FALSE;
	    }
	}

	if ( !compress ) {
	    undoRedoInfo.clear();
	    undoRedoInfo.type = t;
	    undoRedoInfo.index = cursor->index();
	}
    }

    bool readonly : 1;
    bool cursorOn : 1;
    bool inDoubleClick : 1;
    bool mousePressed : 1;
    bool dnd_primed : 1;
    bool ed : 1;
    QLineEdit::EchoMode mode;
    int maxLen;
    int offset;
    int selectionStart;
    const QValidator * validator;
    QTimer blinkTimer;
    QTimer dndTimer;
    QTextParagraph *parag;
    QTextCursor *cursor;
    QPoint dnd_startpos;
    QTimer dragTimer;
    UndoRedoInfo undoRedoInfo;
    QPoint lastMovePos;
    int id[ 7 ];
    bool dragEnabled;
    int preeditStart, preeditLength;
    QString txtBuffer;  // semi-persistant storage for text()
    QChar passwordChar;
#ifndef QT_NO_CLIPBOARD
    QClipboard::Mode clipboard_mode;
#endif
    QTimer trippleClickTimer;
    QPoint trippleClickPoint;
    // needed for the mask part
    QString maskFields;
    QString mask;
    QString initString;
    QChar blank;
    QValueList<MaskInputData> *maskList;
};


/* IGNORE!
    \class QLineEdit

    \brief The QLineEdit widget is a one-line text editor.

    \ingroup basic
    \mainclass

    A line edit allows the user to enter and edit a single line of
    plain text with a useful collection of editing functions,
    including undo and redo, cut and paste, and drag and drop.

    By changing the echoMode() of a line edit, it can also be used as
    a "write-only" field, for inputs such as passwords.

    The length of the field can be constrained to maxLength(), or the
    value can be arbitrarily constrained using a validator().

    A related class is QTextEdit which allows multi-line, rich-text
    editing.

    You can change the text with setText() or insert(). The text is
    retrieved with text(); the displayed text (which may be different,
    see \l{EchoMode}) is retrieved with displayText(). Text can be
    selected with setSelection() or selectAll(), and the selection can
    be cut(), copy()ied and paste()d. The text can be aligned with
    setAlignment().

    When the text changes the textChanged() signal is emitted; when
    the Return or Enter key is pressed the returnPressed() signal is
    emitted. Note that if there is a validator set on the line edit,
    the returnPressed() signal will only be emitted if the validator
    returns Acceptable.

    By default, QLineEdits have a frame as specified by the Windows
    and Motif style guides; you can turn it off by calling
    setFrame(FALSE).

    The default key bindings are described below. A right mouse
    button menu presents some of the editing commands to the user.
    \target desc
    \table
    \header \i Keypress \i Action
    \row \i Left Arrow \i moves the cursor one character to the left.
    \row \i Right Arrow \i moves the cursor one character to the right.
    \row \i Backspace \i deletes the character to the left of the cursor.
    \row \i Ctrl+Backspace \i Delete the word to the left of the cursor
    \row \i Delete \i Delete the character to the right of the cursor
    \row \i Ctrl+Delete \i Delete the word to the right of the cursor
    \row \i Home \i moves the cursor to the beginning of the line.
    \row \i End \i moves the cursor to the end of the line.
    \row \i Delete \i deletes the character to the right of the cursor.
    \row \i Shift+Left Arrow
	 \i moves and selects text one character to the left.
    \row \i Shift+Right Arrow
	 \i moves and selects text one character to the right.
    \row \i Ctrl+A \i moves the cursor to the beginning of the line.
    \row \i Ctrl+B \i moves the cursor one character to the left.
    \row \i Ctrl+C \i copies the selected text to the clipboard.
		      (Windows also supports Ctrl+Insert for this operation.)
    \row \i Ctrl+D \i deletes the character to the right of the cursor.
    \row \i Ctrl+E \i moves the cursor to the end of the line.
    \row \i Ctrl+F \i moves the cursor one character to the right.
    \row \i Ctrl+H \i deletes the character to the left of the cursor.
    \row \i Ctrl+K \i deletes to the end of the line.
    \row \i Ctrl+V \i pastes the clipboard text into line edit.
		      (Windows also supports Shift+Insert for this operation.)
    \row \i Ctrl+X \i deletes the selected text and copies it to the clipboard.
		      (Windows also supports Shift+Delete for this operation.)
    \row \i Ctrl+Z \i undoes the last operation.
    \row \i Ctrl+Y \i redoes the last undone operation.
    \endtable

    Any other key sequence, that represents a valid character, will
    cause the character to be inserted into the line.

    <img src=qlined-m.png> <img src=qlined-w.png>

    \sa QTextEdit QLabel QComboBox
	\link guibooks.html#fowler GUI Design Handbook: Field, Entry\endlink
*/


/* IGNORE!
    \enum QLineEdit::EchoMode

    This enum type describes how a line edit should display its
    contents.

    \value Normal   display characters as they are entered. This is the
		    default.
    \value NoEcho   do not display anything. This may be appropriate
		    for passwords where even the length of the
		    password should be kept secret.
    \value Password  display asterisks instead of the characters
		    actually entered.

    \sa setEchoMode() echoMode()
*/


/* IGNORE!
    \fn void QLineEdit::textChanged( const QString& )

    This signal is emitted whenever the text changes. The argument is
    the new text.
*/

/* IGNORE!
    \fn void QLineEdit::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa hasSelectedText(), selectedText()
*/

/*!
  \fn void QLineEdit::invalidInput()

  This signal is emitted whenever the QLineEdit loses focus and the input
  is invalid compared to the set mask and (if set) the current validator does
  not return Acceptable.
*/

/* IGNORE!
    Constructs a line edit with no text.

    The maximum text length is set to 32767 characters.

    The \a parent and \a name arguments are sent to the QWidget constructor.

    \sa setText(), setMaxLength()
*/

QLineEdit::QLineEdit( QWidget *parent, const char *name )
    : QFrame( parent, name, WRepaintNoErase | WResizeNoErase )
{
    init();
}


/* IGNORE!
    Constructs a line edit containing the text \a contents.

    The cursor position is set to the end of the line and the maximum
    text length to 32767 characters.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.

    \sa text(), setMaxLength()
*/

QLineEdit::QLineEdit( const QString & contents,
		      QWidget *parent, const char *name )
    : QFrame( parent, name, WRepaintNoErase )
{
    init();
    setText( contents );
}

/*!
  Constructs a  line edit with an input \a mask and the text \a contents.

  The cursor position is set to the end of the line and the maximum
  text length is set to the length of the mask (the number of mask
  characters and separators).

  The \a parent and \a name arguments are sent to the QWidget
  constructor.

  \sa setMask() text()
*/
QLineEdit::QLineEdit( const QString & mask, const QString & contents,
	   QWidget* parent, const char* name=0 ) : QFrame( parent, name, WRepaintNoErase )
{
    init();
    if ( !mask.isEmpty() ) parseMaskFields( mask );
    setText( contents );
}

/* IGNORE!
    Destroys the line edit.
*/

QLineEdit::~QLineEdit()
{
    delete d;
}

/* IGNORE!
    \internal

    Sets the password character to \a c.

    \sa passwordChar()
*/

void QLineEdit::setPasswordChar( QChar c )
{
    d->passwordChar = c;
}

/* IGNORE!
    \internal

    Returns the password character.

    \sa setPasswordChar()
*/
QChar QLineEdit::passwordChar() const
{
    return d->passwordChar;
}

/*
    Contains initialization common to both constructors.
*/

void QLineEdit::init()
{
    d = new QLineEditPrivate( this );
    d->parag->formatCollection()->setPaintDevice( this );
    d->parag->setPaintDevice( this );
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
    setMouseTracking( TRUE );
    setFrame( TRUE );
}


void QLineEdit::setText( const QString &text )
{
    QString maskText;
    if ( hasMask() ) {
	maskText = maskString( 0, text );
	maskText += clearString( maskText.length(), d->maskList->count() - maskText.length() );
    } else
	maskText = text;
    d->undoRedoInfo.clear();
    QString oldText = this->text( FALSE );
    d->parag->truncate( 0 );
    d->parag->append( maskText );
    d->parag->commands()->clear();
    d->cursor->setIndex( d->parag->length() - 1 );
    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    deselect();
    update();
    setEdited( FALSE );
    if ( oldText != maskText ) {
	emit textChanged( stripString( maskText ) );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
    }
}


/* IGNORE!
    Selects all the text (i.e. highlights it) and moves the cursor to
    the end. This is useful when a default value has been inserted
    because if the user types before clicking on the widget, the
    selected text will be erased.

    \sa setSelection() deselect()
*/

void QLineEdit::selectAll()
{
    d->selectionStart = 0;
    d->cursor->gotoEnd();
    updateSelection();
    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    update();
}



/* IGNORE!
    De-selects all text (i.e. removes highlighting) and leaves the
    cursor at the current position.

    \sa setSelection() selectAll()
*/

void QLineEdit::deselect()
{
    d->selectionStart = 0;
    d->parag->removeSelection( QTextDocument::Standard );
    update();
}


/* IGNORE!
    \property QLineEdit::text
    \brief the line edit's text

    Setting this property clears the selection, moves the cursor to
    the end of the line and resets the edited property to FALSE.

    The text is truncated to maxLength() length.
*/

QString QLineEdit::text() const
{
    // only change the text if we need to, this ensure that multiple
    // calls to text() will get the same shared value.
    QString s = d->parag->string()->toString();
    s.remove( s.length() - 1, 1 ); // get rid of trailing space

//     if ( d->txtBuffer != s )
// 	d->txtBuffer = s;
//     return d->txtBuffer;
    return stripString( s );
}



/* IGNORE!
    \property QLineEdit::displayText
    \brief the displayed text

    If \c EchoMode is \c Normal this returns the same as text(); if
    \c EchoMode is \c Password it returns a string of asterisks the
    text().length() characters long, e.g. "******"; if \c EchoMode is
    \c NoEcho returns an empty string, "".

    \sa setEchoMode() text() EchoMode
*/

QString QLineEdit::displayText() const
{
    return d->displayText();
}



/* IGNORE!
    \obsolete
    \property QLineEdit::hasMarkedText
    \brief whether part of the text has been selected by the user (e.g.
    by clicking and dragging).

  \sa selectedText()
*/

/* IGNORE!
    \property QLineEdit::hasSelectedText
    \brief whether there is any text selected

    hasSelectedText() returns TRUE if some or all of the text has been
    selected by the user (e.g. by clicking and dragging); otherwise
    returns FALSE.

    \sa selectedText()
*/


bool QLineEdit::hasSelectedText() const
{
    return d->parag->hasSelection( QTextDocument::Standard ) &&
	d->parag->length() > 1 &&
	d->parag->selectionStart( QTextDocument::Standard ) >= 0 &&
	d->parag->selectionEnd( QTextDocument::Standard ) >= 0 &&
	d->parag->selectionStart( QTextDocument::Standard ) != d->parag->selectionEnd( QTextDocument::Standard );
}

/* IGNORE!
    \obsolete
    \property QLineEdit::markedText
    \brief the text selected by the user (e.g. by clicking and
    dragging), or QString::null if no text is selected.

  \sa hasSelectedText()
*/

/* IGNORE!
    \property QLineEdit::selectedText
    \brief the selected text

    If there is no selected text this property's value is
    QString::null.

    \sa hasSelectedText()
*/


QString QLineEdit::selectedText() const
{
    return d->parag->string()->toString().mid( d->parag->selectionStart( 0 ), d->parag->selectionEnd( 0 ) - d->parag->selectionStart( 0 ) );
}

/* IGNORE!
    \property QLineEdit::maxLength
    \brief the maximum permitted length of the text

    If the text is too long, it is truncated at the limit.

    If truncation occurs any selected text will be unselected, the
    cursor position is set to 0 and the first part of the string is
    shown.
*/

int QLineEdit::maxLength() const
{
    return d->maxLen;
}

void QLineEdit::setMaxLength( int m )
{
    if ( hasMask() ) return;
    d->maxLen = m;
    d->parag->truncate( d->maxLen );
    home( FALSE );
    update();
}

/* IGNORE!
    \fn void  QLineEdit::returnPressed()

    This signal is emitted when the Return or Enter key is pressed.
    Note that if there is a validator or mask set on the line edit, the
    returnPressed() signal will only be emitted if the input follows
    the mask and/or the validator returns Acceptable.
*/


/* IGNORE!
    Converts key press event \a e into a line edit action.

    If Return or Enter is pressed and the current text is valid (or
    can be \link QValidator::fixup() made valid\endlink by the
    validator), the signal returnPressed() is emitted.

    The default key bindings are listed in the \link #desc detailed
    description.\endlink
*/

void QLineEdit::keyPressEvent( QKeyEvent *e )
{
    d->blinkTimer.stop();
    blinkOn();
    int cursorPos = cursorPosition();
    if ( e->key() == Key_Enter || e->key() == Key_Return ) {
#ifdef QT_NO_VALIDATOR
	emit returnPressed();
	e->ignore();
#else
	const QValidator * v = validator();
	QString str = text( FALSE );
	if ( !v || v->validate( str, cursorPos ) == QValidator::Acceptable ) {
	    if ( hasMask() ) {
		if ( isValidInput() )
		    emit returnPressed();
	    } else
		emit returnPressed();
	    e->ignore();
	} else if ( v ) {
	    QString old = text( FALSE );
	    QString vstr = old;
	    v->fixup( vstr );
	    if ( old != vstr ) {
		setText( vstr );
		update();
	    }
	    if ( hasMask() ) {
		if ( isValidInput() ) emit returnPressed();
	    } else {
		if ( v->validate( vstr, cursorPos ) == QValidator::Acceptable )
		    emit returnPressed();
	    }
	    e->ignore();
	}
#endif
	return;
    }
    if ( !d->readonly ) {
	QString t = e->text();
	if ( !t.isEmpty() && (!e->ascii() || e->ascii()>=32) &&
	     e->key() != Key_Delete &&
	     e->key() != Key_Backspace ) {
	    if ( d->parag && d->parag->string() && d->parag->string()->isRightToLeft() ) {
		QChar *c = (QChar *)t.unicode();
		int l = t.length();
		while( l-- ) {
		    if ( c->mirrored() )
			*c = c->mirroredChar();
		    c++;
		}
	    }
	    insert( t );
	    return;
	}
    }
    bool unknown = FALSE;
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
#if defined(Q_WS_X11)
	    home( e->state() & ShiftButton );
#else
	    selectAll();
#endif
	    break;
	case Key_B:
	    cursorForward( e->state() & ShiftButton, -1 );
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
	    cursorForward( e->state() & ShiftButton, 1 );
	    break;
	case Key_H:
	    if ( !d->readonly ) {
		backspace();
	    }
	    break;
	case Key_K:
	    if ( !d->readonly ) {
		d->selectionStart = d->cursor->index();
		d->cursor->setIndex( d->parag->length() - 1 );
		updateSelection();
		removeSelectedText();
	    }
	    break;
#if defined(Q_WS_X11)
        case Key_U:
	    if ( !d->readonly )
		clear();
	    break;
#endif
#ifndef QT_NO_CLIPBOARD
	case Key_V:
	    if ( !d->readonly )
		paste();
	    break;
	case Key_X:
	    if ( !d->readonly && hasSelectedText() && echoMode() == Normal ) {
		copy();
		del();
	    }
	    break;
#if defined (Q_WS_WIN)
	case Key_Insert:
	    copy();
	    break;
	case Key_Delete:
	    if ( !d->readonly )
		del();
	    break;
#endif
#endif
	case Key_Right:
	case Key_Left:
	    if ( d->parag->string()->isRightToLeft() == (e->key() == Key_Right) ) {
	        if ( echoMode() == Normal )
		    cursorWordBackward( e->state() & ShiftButton );
		else
		    home( e->state() & ShiftButton );
	    } else {
		if ( echoMode() == Normal )
		    cursorWordForward( e->state() & ShiftButton );
		else
		    end( e->state() & ShiftButton );
	    }
	    break;
	case Key_Z:
	    if ( !d->readonly ) {
		if(e->state() & ShiftButton)
		    redo();
		else
		    undo();
	    }
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
	case Key_Shift:
	    if ( !d->parag->hasSelection( QTextDocument::Standard ) )
		d->selectionStart = d->cursor->index();
	    break;
	case Key_Left:
	case Key_Right: {
	    int step =  (d->parag->string()->isRightToLeft() == (e->key() == Key_Right)) ? -1 : 1;
	    cursorForward( e->state() & ShiftButton, step );
	}
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
		paste();
	    break;
	case Key_F20: // Cut key on Sun keyboards
	    if ( !d->readonly && hasSelectedText() && echoMode() == Normal ) {
		copy();
		del();
	    }
	    break;
#endif
	default:
	    unknown = TRUE;
	}
    }
    if ( e->key() == Key_Direction_L && d->parag->direction() != QChar::DirL ) {
	d->parag->setDirection( QChar::DirL );
	d->parag->invalidate( 0 );
	d->parag->format( -1, TRUE );
	unknown = FALSE;
    } else if ( e->key() == Key_Direction_R && d->parag->direction() != QChar::DirR ) {
	d->parag->setDirection( QChar::DirR );
	d->parag->invalidate( 0 );
	d->parag->format( -1, TRUE );
	unknown = FALSE;
    }

    if ( unknown ) {				// unknown key
	e->ignore();
	return;
    }
}


/* IGNORE! \reimp
 */
void QLineEdit::imStartEvent( QIMEvent *e )
{
    if ( isReadOnly() ) {
	e->ignore();
	return;
    }

    if ( hasSelectedText() )
	removeSelectedText();

    d->preeditStart = cursorPosition();
    d->preeditLength = 0;
    setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0,
	d->cursor->paragraph()->rect().height(), TRUE );
    e->accept();
}


/* IGNORE! \reimp
 */
void QLineEdit::imComposeEvent( QIMEvent *e )
{
    if ( isReadOnly() ) {
	e->ignore();
	return;
    }

    d->parag->removeSelection( QTextDocument::IMCompositionText );
    d->parag->removeSelection( QTextDocument::IMSelectionText );

    if (d->preeditLength > 0)
	d->parag->remove(d->preeditStart, d->preeditLength);
    d->cursor->setIndex( d->preeditStart );
    insert( e->text() );
    d->preeditLength = e->text().length();

    d->parag->setSelection( QTextDocument::IMCompositionText,
			    d->preeditStart, d->preeditStart + d->preeditLength );

    d->cursor->setIndex(d->preeditStart + e->cursorPos());

    int sellen = e->selectionLength();
    if ( sellen > 0 ) {
	d->parag->setSelection( QTextDocument::IMSelectionText,
				d->preeditStart + e->cursorPos(),
				d->preeditStart + e->cursorPos() + sellen );
    }

    e->accept();
}


/* IGNORE! \reimp
 */
void QLineEdit::imEndEvent( QIMEvent *e )
{
    if ( isReadOnly() ) {
	e->ignore();
	return;
    }

    d->parag->removeSelection( QTextDocument::IMCompositionText );
    d->parag->removeSelection( QTextDocument::IMSelectionText );

    if (d->preeditLength > 0)
	d->parag->remove(d->preeditStart, d->preeditLength);
    d->cursor->setIndex( d->preeditStart );
    insert( e->text() );
    d->preeditStart = d->preeditLength = -1;

    e->accept();
}


/* IGNORE!\reimp
*/

void QLineEdit::focusInEvent( QFocusEvent * e)
{
    Q_UNUSED(e) // I need this to get rid of a Borland warning
    d->cursorOn = FALSE;
    blinkOn();
    if ( e->reason() == QFocusEvent::Tab || e->reason() == QFocusEvent::Backtab  || e->reason() == QFocusEvent::Shortcut )
	selectAll();
    update();
    setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
}


/* IGNORE!\reimp
*/

void QLineEdit::focusOutEvent( QFocusEvent * e )
{
    Q_UNUSED(e) // I need this to get rid of a Borland warning
    if ( e->reason() != QFocusEvent::ActiveWindow
	 && e->reason() != QFocusEvent::Popup )
	deselect();
    d->dragTimer.stop();
    if ( d->cursorOn )
	blinkSlot();
    if ( ( hasMask() ) && ( !isValidInput() ) ) emit invalidInput();
    update();
}

/* IGNORE!\reimp
*/

void QLineEdit::drawContents( QPainter *painter )
{
    int marg = frameWidth();
    painter->translate( marg, 0 );
    const QColorGroup & g = colorGroup();

    int lineheight = QMIN( fontMetrics().lineSpacing() + 4, height() );
    int linetop = (height() - lineheight ) / 2;

    QSharedDoubleBuffer buffer( painter, 0, linetop, width(), lineheight,
				hasFocus() ? QSharedDoubleBuffer::Force : 0 );
    buffer.painter()->setPen( colorGroup().text() );

    QBrush bg = isEnabled() ? QBrush( paletteBackgroundColor() ) :
  			      g.brush( QColorGroup::Background);
    buffer.painter()->fillRect( 0, 0, width(), height(), bg );
    if ( linetop ) {
	painter->fillRect( 0, 0, width(), linetop, bg );
	painter->fillRect( 0, linetop + lineheight, width(), linetop, bg );
    }

    QTextParagraph *parag;
    QTextCursor *cursor;
    d->getTextObjects( &parag, &cursor );
    if ( echoMode() == Password ) {
	// in password mode, make sure that the parag which draws the
	// *** contains the same selecion as the parag which contains
	// the actual text
	if ( d->parag->hasSelection( QTextDocument::Standard ) ) {
	    int start = d->parag->selectionStart( QTextDocument::Standard );
	    int end = d->parag->selectionEnd( QTextDocument::Standard );
	    parag->setSelection( QTextDocument::Standard, start, end );
	} else {
	    parag->removeSelection( QTextDocument::Standard );
	}
    }
    QTextFormat *f = parag->formatCollection()->format( font(), buffer.painter()->pen().color() );
    parag->setFormat( 0, parag->length(), f );
    f->removeRef();
    QRect r( rect().x(), rect().y(), width() - 2 * marg, rect().height() );
    parag->pseudoDocument()->docRect = r;
    parag->invalidate( 0 );
    parag->format();
    updateOffset( parag, cursor );
    int xoff = 1 - d->offset;
    int yoff = ( height() - parag->rect().height()  + 1 ) / 2;
    if ( yoff < 0 )
	yoff = 0;
    buffer.painter()->translate( xoff, yoff );
    if ( d->mode != NoEcho )
	parag->paint( *buffer.painter(), colorGroup(),
		      d->cursorOn && !d->readonly ? cursor : 0,
		      TRUE,
		      -xoff, 0, width(), height() );

    buffer.end();

    d->releaseTextObjects( &parag, &cursor );
    painter->fillRect( -1, 0, 1, height(), bg );
    painter->translate( -marg, 0 );
}


/* IGNORE!\reimp
*/

void QLineEdit::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
}


/* IGNORE! \reimp
*/
bool QLineEdit::event( QEvent * e )
{
    if ( e->type() == QEvent::AccelOverride && !d->readonly ) {
	QKeyEvent* ke = (QKeyEvent*) e;
	if ( ke->state() == NoButton || ke->state() == Keypad ) {
	    if ( ke->key() < Key_Escape ) {
		ke->accept();
	    } else {
		switch ( ke->key() ) {
  		case Key_Delete:
  		case Key_Home:
  		case Key_End:
  		case Key_Backspace:
 		case Key_Left:
		case Key_Right:
 		    ke->accept();
 		default:
  		    break;
  		}
	    }
	} else if ( ke->state() & ControlButton ) {
	    switch ( ke->key() ) {
// Those are too frequently used for application functionality
/*	    case Key_A:
	    case Key_B:
	    case Key_D:
	    case Key_E:
	    case Key_F:
	    case Key_H:
	    case Key_K:
*/
	    case Key_C:
	    case Key_V:
	    case Key_X:
	    case Key_Y:
	    case Key_Z:
	    case Key_Left:
	    case Key_Right:
#if defined (Q_WS_WIN)
	    case Key_Insert:
	    case Key_Delete:
#endif
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

static bool inSelection( int x, QTextParagraph *p )
{
    return ( x >= p->at( p->selectionStart( QTextDocument::Standard ) )->x &&
	     x <= p->at( p->selectionEnd( QTextDocument::Standard ) )->x );
}

/* IGNORE! \reimp
*/
void QLineEdit::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton )
	return;

    if ( d->trippleClickTimer.isActive() &&
	 ( e->globalPos() - d->trippleClickPoint ).manhattanLength() <
	 QApplication::startDragDistance() ) {
	selectAll();
	return;
    }

    bool oldHST = hasSelectedText();

    d->undoRedoInfo.clear();

    d->inDoubleClick = FALSE;
    QPoint p( e->pos().x() + d->offset - frameWidth() - margin() - 1, 0 );
    QTextParagraph *par;
    QTextCursor *c;
    d->getTextObjects(&par, &c);
    int oldPos = c->index();
    c->place( p, par );
#ifndef QT_NO_DRAGANDDROP
    if ( dragEnabled() && hasSelectedText() && echoMode() == Normal && !( e->state() & ShiftButton ) &&
	 e->button() == LeftButton && inSelection( e->pos().x() + d->offset - frameWidth() - margin() - 1, d->parag ) ) {
	d->dndTimer.start( QApplication::startDragTime(), TRUE );
	d->dnd_primed = TRUE;
	d->dnd_startpos = e->pos();
	d->releaseTextObjects( &par, &c );
	if ( oldHST != hasSelectedText() )
	    emit selectionChanged();

	return;
    }
#endif
    if ( !( e->state() & ShiftButton ) ) {
	d->selectionStart = c->index();
	d->parag->setSelection( QTextDocument::Standard, d->selectionStart, d->selectionStart );
    } else {
	if ( d->parag->selectionEnd( QTextDocument::Standard ) != oldPos &&
	     d->parag->selectionStart( QTextDocument::Standard ) != oldPos )
	    d->selectionStart = oldPos;
	int s = d->selectionStart;
	int e = c->index();
	if ( s > e ) {
	    s = c->index();
	    e = d->selectionStart;
	}
	d->parag->setSelection( QTextDocument::Standard, s, e );
    }
    d->releaseTextObjects( &par, &c);

    if ( oldHST != hasSelectedText() )
	emit selectionChanged();

    update();
    d->mousePressed = TRUE;
}

#ifndef QT_NO_DRAGANDDROP

/*
  \internal
*/

void QLineEdit::doDrag()
{
    if ( !dragEnabled() )
	return;
    d->dnd_primed = FALSE;
    QTextDrag *tdo = new QTextDrag( selectedText(), this );
    if ( tdo->drag() && !isReadOnly() )
	del();
#ifndef QT_NO_CURSOR
    setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
    d->mousePressed = FALSE;
}

#endif // QT_NO_DRAGANDDROP

/* IGNORE!\reimp
*/
void QLineEdit::mouseMoveEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton )
	return;
#ifndef QT_NO_CURSOR
    if ( !d->mousePressed ) {
	if ( !isReadOnly() && dragEnabled()
#ifndef QT_NO_WHATSTHIS
	    && !QWhatsThis::inWhatsThisMode()
#endif
	) {
	    if ( hasSelectedText() &&
		 inSelection( e->pos().x() + d->offset - frameWidth() - margin() - 1, d->parag ) )
		setCursor( arrowCursor );
	    else
		setCursor( ibeamCursor );
	}
    }
#endif

#ifndef QT_NO_DRAGANDDROP
    if ( dragEnabled() ) {
	if ( d->dndTimer.isActive() ) {
	    d->dndTimer.stop();
	    return;
	}

	if ( d->dnd_primed ) {
	    if ( ( d->dnd_startpos - e->pos() ).manhattanLength() > QApplication::startDragDistance() )
		doDrag();
	    return;
	}
    }
#endif

    if ( d->mousePressed ) {
	d->dragTimer.stop();
	d->lastMovePos = e->pos();
	dragSlot();
    }
}

void QLineEdit::dragSlot()
{
    QPoint p( d->lastMovePos.x() + d->offset - frameWidth() - margin() - 1, 0 );
    QTextParagraph *par;
    QTextCursor *c;
    d->getTextObjects(&par, &c);
    c->place( p, par );
    d->releaseTextObjects( &par, &c );
    updateSelection();
    update();
    if ( d->lastMovePos.x() < 0 || d->lastMovePos.x() > width() )
	d->dragTimer.start( 100, TRUE );
}

/* IGNORE!\reimp
*/
void QLineEdit::mouseReleaseEvent( QMouseEvent * e )
{
    if ( e->button() == RightButton )
	return;
    d->dnd_primed = FALSE;
    d->dragTimer.stop();
    if ( d->dndTimer.isActive() ) {
	d->dndTimer.stop();
	QPoint p( e->pos().x() + d->offset - frameWidth() - margin() - 1, 0 );
	d->cursor->place( p, d->parag );
	bool oldHST = hasSelectedText();
	deselect(); // does a repaint
	if ( oldHST != hasSelectedText() )
	    emit selectionChanged();

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
	d->clipboard_mode = QClipboard::Selection;
	copy();
	d->clipboard_mode = QClipboard::Clipboard;
    }

    if ( !d->readonly && e->button() == MidButton ) {
	if (QApplication::clipboard()->supportsSelection()) {
	    d->clipboard_mode = QClipboard::Selection;
	    paste();
	    d->clipboard_mode = QClipboard::Clipboard;
	}
	return;
    }
#endif

    if ( e->button() != LeftButton )
	return;

    QPoint p( e->pos().x() + d->offset - frameWidth() - margin() - 1, 0 );
    QTextParagraph *par;
    QTextCursor *c;
    d->getTextObjects(&par, &c);
    c->place( p, par );
    d->releaseTextObjects( &par, &c );
    update();
}


/* IGNORE!\reimp
*/
void QLineEdit::mouseDoubleClickEvent( QMouseEvent *e )
{
    bool oldHST = hasSelectedText();
    d->inDoubleClick = TRUE;

    if ( echoMode() == Password ) {
	selectAll();
    } else {
	QTextCursor c1 = *d->cursor;
	QTextCursor c2 = *d->cursor;
	c1.gotoPreviousWord();
	c2.gotoNextWord();

	d->parag->setSelection( QTextDocument::Standard, c1.index(), c2.index() );
	*d->cursor = c2;

	d->trippleClickTimer.start( qApp->doubleClickInterval(), TRUE );
	d->trippleClickPoint = e->globalPos();
    }
#ifndef QT_NO_CLIPBOARD
    if (! d->mousePressed && QApplication::clipboard()->supportsSelection()) {
	d->clipboard_mode = QClipboard::Selection;
	copy();
	d->clipboard_mode = QClipboard::Clipboard;
    }
#endif // QT_NO_CLIPBOARD
    if ( oldHST != hasSelectedText() )
	emit selectionChanged();

    update();
}

/* IGNORE!\reimp
*/
void QLineEdit::contextMenuEvent( QContextMenuEvent* e )
{
#ifndef QT_NO_POPUPMENU
    d->undoRedoInfo.clear();
    d->mousePressed = FALSE;

    QPopupMenu *popup = createPopupMenu();
    QPoint pos = e->reason() == QContextMenuEvent::Mouse ? e->globalPos() :
		 mapToGlobal( QPoint(e->pos().x(), 0) ) + QPoint( width() / 2, height() / 2 );
    connect( popup, SIGNAL(activated(int)), SLOT(popupActivated(int)) );
    popup->exec( pos );
    delete popup;

    // WARNING: do not add any code here that accesses members of this QLineEdit since
    // it could already be destroyed

    e->accept();
#endif //QT_NO_POPUPMENU
}

void QLineEdit::popupActivated( int r )
{
    if ( r == d->id[ IdClear ] )
	clear();
    else if ( r == d->id[ IdSelectAll ] )
	selectAll();
    else if ( r == d->id[ IdUndo ] )
	undo();
    else if ( r == d->id[ IdRedo ] )
	redo();
#ifndef QT_NO_CLIPBOARD
    else if ( r == d->id[ IdCut ] )
	cut();
    else if ( r == d->id[ IdCopy ] )
	copy();
    else if ( r == d->id[ IdPaste ] )
	paste();
#endif
}


/* IGNORE!
  \obsolete
  \fn void QLineEdit::cursorRight( bool, int )

  Use cursorForward() instead.

  \sa cursorForward()
*/

/* IGNORE!
  \obsolete
  \fn void QLineEdit::cursorLeft( bool, int )
  For compatibilty with older applications only. Use cursorBackward()
  instead.
  \sa cursorBackward()
*/

/* IGNORE!
    Moves the cursor back \a steps characters. If \a mark is TRUE each
    character moved over is added to the selection; if \a mark is
    FALSE the selection is cleared.

    \sa cursorForward()
*/
void QLineEdit::cursorBackward( bool mark, int steps )
{
    cursorForward( mark, -steps );
}

/* IGNORE!
    Moves the cursor forward \a steps characters. If \a mark is TRUE
    each character moved over is added to the selection; if \a mark is
    FALSE the selection is cleared.

    \sa cursorBackward()
*/

void QLineEdit::cursorForward( bool mark, int steps )
{
    if( steps > 0 )
	while( steps-- ) {
	    d->cursor->gotoNextLetter();
	    if ( hasMask() )
		while ( (*d->maskList)[ d->cursor->index() ].separator ) {
		    d->cursor->gotoNextLetter();
		    if ( d->cursor->atParagEnd() ) break;
		}
	}
    else
	while( steps++ ) {
	    d->cursor->gotoPreviousLetter();
	    if ( hasMask() )
		while ( (*d->maskList)[ d->cursor->index() ].separator ) {
		    d->cursor->gotoPreviousLetter();
		    if ( d->cursor->atParagStart() ) break;
		}
	}
    if ( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    update();
}

/* IGNORE!
    Deletes the character to the left of the text cursor and moves the
    cursor one position to the left. If any text has been selected by
    the user (e.g. by clicking and dragging), the cursor will be put
    at the beginning of the selected text and the selected text will
    be removed.

    \sa del()
*/
void QLineEdit::backspace()
{
    delOrBackspace( TRUE );
}

/* IGNORE!
    Deletes the character to the right of the text cursor. If any text
    has been selected by the user (e.g. by clicking and dragging), the
    cursor will be put at the beginning of the selected text and the
    selected text will be removed.

    \sa backspace()
*/

void QLineEdit::del()
{
    delOrBackspace( FALSE );
}

/* IGNORE!
    Moves the text cursor to the beginning of the line. If \a mark is
    TRUE, text is selected towards the first position; otherwise, any
    selected text is unselected if the cursor is moved.

    \sa end()
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
    setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    updateOffset();
    update();
}

/* IGNORE!
    Moves the text cursor to the end of the line. If \a mark is TRUE,
    text is selected towards the last position; otherwise, any
    selected text is unselected if the cursor is moved.

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
    setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    update();
}


#ifndef QT_NO_CLIPBOARD

/* IGNORE!
    Copies the selected text to the clipboard, if there is any, and if
    echoMode() is \c Normal.

    \sa cut() paste()
*/

void QLineEdit::copy() const
{
    QString t = selectedText();
    if ( !t.isEmpty() && echoMode() == Normal ) {
	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);
	QApplication::clipboard()->setText( t, d->clipboard_mode );
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()),
		 this, SLOT(clipboardChanged()) );
    }
}

/* IGNORE!
    Inserts the clipboard's text at the cursor position, deleting any
    selected text.

    If the end result is not acceptable for the current validator,
    nothing happens.

    \sa copy() cut()
*/

void QLineEdit::paste()
{
    insert( QApplication::clipboard()->text( d->clipboard_mode ) );
    deselect();
}

/* IGNORE!
    Copies the selected text to the clipboard and deletes it, if there
    is any, and if echoMode() is \c Normal.

    If the current validator disallows deleting the selected text,
    cut() will copy it but not delete it.

    \sa copy() paste()
*/

void QLineEdit::cut()
{
    QString t = selectedText();
    if ( !t.isEmpty() ) {
	copy();
	del();
    }
}

#endif

void QLineEdit::setAlignment( int flag )
{
    if ( flag == AlignCenter )
	flag = AlignHCenter;
    if ( flag == d->parag->alignment() )
	return;
    d->parag->setAlignment( flag );
    d->parag->invalidate( 0 );
    d->parag->format();
    updateOffset();
    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    update();
}

/* IGNORE!
    \property QLineEdit::alignment
    \brief the alignment of the line edit

    Possible Values are \c Qt::AlignAuto, \c Qt::AlignLeft, \c
    Qt::AlignRight and \c Qt::AlignHCenter.

    Attempting to set the alignment to an illegal flag combination
    does nothing.

    \sa Qt::AlignmentFlags
*/

int QLineEdit::alignment() const
{
    return d->parag->alignment();
}

/*
  This private slot is activated when this line edit owns the clipboard and
  some other widget/application takes over the clipboard. (X11 only)
*/

void QLineEdit::clipboardChanged()
{
#if defined(Q_WS_X11)
    disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()),
		this, SLOT(clipboardChanged()) );
    deselect();
#endif
}

void QLineEdit::setFrame( bool enable )
{
    setFrameStyle( enable ? ( LineEditPanel | Sunken ) : NoFrame  );
}


/* IGNORE!
    \property QLineEdit::frame
    \brief whether the line edit draws itself with a frame

    If enabled (the default) the line edit draws itself inside a
    two-pixel frame, otherwise the line edit draws itself without any
    frame.
*/

bool QLineEdit::frame() const
{
    return frameShape() != NoFrame;
}

void QLineEdit::setEchoMode( EchoMode mode )
{
    if ( d->mode == mode )
	return;

    d->mode = mode;
    update();
}


/* IGNORE!
    \property QLineEdit::echoMode
    \brief the line edit's echo mode

    The initial setting is \c Normal, but QLineEdit also supports \c
    NoEcho and \c Password modes.

    The widget's display and the ability to copy or drag the text is
    affected by this setting.

    \sa EchoMode displayText()
*/

QLineEdit::EchoMode QLineEdit::echoMode() const
{
    return d->mode;
}


/* IGNORE!
    \property QLineEdit::readOnly
    \brief whether the line edit is read only.

    In read-only mode, the user can still copy the text to the
    clipboard or drag-and-drop the text, but cannot edit it.

    QLineEdit does not show a cursor in read-only mode.

    \sa setEnabled()
*/
void QLineEdit::setReadOnly( bool enable )
{
    d->readonly = enable;
#ifndef QT_NO_CURSOR
    setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif

    update();
}

bool QLineEdit::isReadOnly() const
{
    return d->readonly;
}

/* IGNORE!
    Returns a recommended size for the widget.

    The width returned, in pixels, is usually enough for about 15 to
    20 characters.
*/
QSize QLineEdit::sizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int h = fm.lineSpacing();
    int w = fm.width( 'x' ) * 17; // "some"
    return (style().sizeFromContents(QStyle::CT_LineEdit, this,
				     QSize(w + 2 + 2*frameWidth(), QMAX( h, 14 ) + 2 + 2*frameWidth()).
	    expandedTo(QApplication::globalStrut())));
}



/* IGNORE!
    Returns a minimum size for the line edit.

    The width returned is enough for at least one character.
*/

QSize QLineEdit::minimumSizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int h = fm.lineSpacing();
    int w = fm.maxWidth();
    return QSize( w + 2 + 2*frameWidth(), h + 2 + 2 * frameWidth() );
}


/* IGNORE!
    Sets this line edit to only accept input that the validator, \a v,
    will accept. This allows you to place any arbitrary constraints on
    the text which may be entered.

    If \a v == 0, setValidator() removes the current input validator.
    The initial setting is to have no input validator (i.e. any input
    is accepted up to maxLength()).

    \sa validator() QValidator
*/

void QLineEdit::setValidator( const QValidator * v )
{
    if ( d->validator )
	disconnect( (QObject*)d->validator, SIGNAL( destroyed() ),
		    this, SLOT( clearValidator() ) );
    d->validator = v;
    if ( d->validator )
	connect( (QObject*)d->validator, SIGNAL( destroyed() ),
	         this, SLOT( clearValidator() ) );
}

/* IGNORE!
    Returns a pointer to the current input validator, or 0 if no
    validator has been set.

    \sa setValidator()
*/

const QValidator * QLineEdit::validator() const
{
    return d ? d->validator : 0;
}


/* IGNORE!
    This slot is equivalent to setValidator( 0 ).
*/

void QLineEdit::clearValidator()
{
    setValidator( 0 );
}

#ifndef QT_NO_DRAGANDDROP

/* IGNORE! \reimp
*/
void QLineEdit::dragEnterEvent( QDragEnterEvent *e )
{
    if ( !d->readonly && QTextDrag::canDecode(e) )
	e->acceptAction();
    else
	return;
    d->cursorOn = TRUE;
}

/* IGNORE! \reimp
*/
void QLineEdit::dragLeaveEvent( QDragLeaveEvent * )
{
    d->cursorOn = hasFocus();
    update();
}

/* IGNORE!\reimp
*/

void QLineEdit::dragMoveEvent( QDragMoveEvent *e )
{
    if ( !d->readonly && QTextDrag::canDecode(e) )
	e->acceptAction();
    else
	return;
    QPoint p( e->pos().x() + d->offset - frameWidth() - 1, 0 );
    d->cursor->place( p, d->parag );
    update();
}

/* IGNORE!\reimp
*/
void QLineEdit::dropEvent( QDropEvent *e )
{
    QString str;
    QCString plain = "plain";

    // try text/plain
    bool decoded = QTextDrag::decode(e, str, plain);
    // otherwise we'll accept any kind of text (like text/uri-list)
    if (! decoded) decoded = QTextDrag::decode(e, str);

    d->cursorOn = hasFocus();

    if ( !d->readonly && decoded ) {
	if ( e->source() == this && hasSelectedText() )
	    deselect();
	if ( !hasSelectedText() ) {
	    QPoint p( e->pos().x() + d->offset - frameWidth() - 1, 0 );
	    d->cursor->place( p, d->parag );
	}
	insert( str );
	e->acceptAction();
    } else {
	e->ignore();
	update();
    }
}

#endif // QT_NO_DRAGANDDROP

/*  This private slot handles cursor blinking. */

void QLineEdit::blinkSlot()
{
    if( hasSelectedText() && !style().styleHint( QStyle::SH_BlinkCursorWhenTextSelected )) {
	if(!d->cursorOn) {
	    d->cursorOn = TRUE;
	    update();
	}
	d->blinkTimer.stop();
    }
    if ( hasFocus() || d->cursorOn ) {
	d->cursorOn = !d->cursorOn;
	update();
    }
    if( hasFocus() )
	d->blinkTimer.start( QApplication::cursorFlashTime()/2, TRUE );
    else
	d->blinkTimer.stop();
}



/* IGNORE!
    Validates and perhaps sets this line edit to contain \a newText
    with the cursor at position \a newPos, with selected text from \a
    newMarkAnchor to \a newMarkDrag. Returns TRUE if it changes the
    line edit; otherwise returns FALSE.

    Linebreaks in \a newText are converted to spaces, and the text is
    truncated to maxLength() before its validity is tested.

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

    QString old = this->text( FALSE );

#ifndef QT_NO_VALIDATOR
    const QValidator * v = validator();

    int pos = d->cursor->index();
    if ( v && v->validate( t, newPos ) == QValidator::Invalid &&
	 v->validate( old, pos ) != QValidator::Invalid ) {
	return FALSE;
    }
#endif

    // okay, it succeeded
    bool text_changed = ( t != old );
    if ( text_changed ) {
	d->parag->truncate( 0 );
	d->parag->append( t );
    }

    d->cursor->setIndex( newPos );
    d->selectionStart = newMarkAnchor;
    d->parag->setSelection( QTextDocument::Standard, newMarkAnchor, newMarkDrag );
    repaint( FALSE );

    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(),
			   0, d->cursor->paragraph()->rect().height(), TRUE );

    if ( text_changed ) {
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
	emit textChanged( stripString( t ) );
    }
    return TRUE;
}


/* IGNORE!
    Removes any selected text, inserts \a newText, and validates the
    result. If it is valid, it sets it as the new contents of the line
    edit.
*/
void QLineEdit::insert( const QString &newText )
{
    QString t( newText );
    if ( hasMask() )
	if ( hasSelectedText() )
	    t = maskString( d->parag->selectionStart( 0 ), newText );
        else
	    t = maskString( d->cursor->index(), newText );
    if ( t.isEmpty() && !hasSelectedText() )
	return;

    for ( int i=0; i<(int)t.length(); i++ )
	if ( t[i] < ' ' )  // unprintable/linefeed becomes space
	    t[i] = ' ';

    QString t1 = d->parag->string()->toString();
    t1.remove( t1.length() - 1, 1 );
    int cp1 = d->cursor->index();

    if ( hasSelectedText() ) {
	int start = d->parag->selectionStart( 0 );
	int len = d->parag->selectionEnd( 0 ) - d->parag->selectionStart( 0 );
	d->checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
	d->undoRedoInfo.index = start;
	d->undoRedoInfo.text = t1.mid( start, len );
	if ( hasMask() ) {
	    QString clear = clearString( start, len );
	    t1.replace( start, clear.length(), clear );
	} else
	    t1.remove( start, len );
	cp1 = start;
    }

    d->checkUndoRedoInfo( UndoRedoInfo::Insert );
    d->undoRedoInfo.index = cp1;

    QString t2 = t1;
    if ( hasMask() )
	t2.replace( cp1, t.length(), t );
    else
	t2.insert( cp1, t );

    int cp2 = QMIN( cp1 + t.length(), (uint)maxLength() );

    d->ed = TRUE;
    if ( !validateAndSet( t2, cp2, cp2, cp2 ) ) {
	if ( !validateAndSet( t1, cp1, cp1, cp1 ) )
	    return;
    }

    blinkOn();

    if ( t2 == this->text( FALSE ) )
 	d->undoRedoInfo.text += t;
    update();
    d->selectionStart = d->cursor->index();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0,
			   d->cursor->paragraph()->rect().height(), TRUE );
}


/* IGNORE!
  \obsolete
  \fn void QLineEdit::repaintArea( int from, int to )
  Repaints all characters from \a from to \a to. If cursorPos is
  between from and to, ensures that cursorPos is visible.
*/


/* IGNORE! \reimp */

void QLineEdit::setFont( const QFont & f )
{
    QWidget::setFont( f );
}


/* IGNORE!
    Clears the contents of the editor. This is equivalent to setText("").
*/

void QLineEdit::clear()
{
    if ( hasMask() )
	setText( d->initString );
    else
	setText( QString::fromLatin1("") );
}


/* IGNORE!
    Sets the selected area of this line edit to start at position \a
    start and be \a length characters long.

    \sa deselect() selectAll() getSelection()
*/

void QLineEdit::setSelection( int start, int length )
{
    d->selectionStart = start;
    d->cursor->setIndex( start + length );
    updateSelection();
    update();
}


void QLineEdit::setCursorPosition( int newPos )
{
    d->cursor->setIndex( newPos );
    deselect();
}


/* IGNORE!
    \property QLineEdit::cursorPosition
    \brief the current cursor position for this line edit

    Setting the cursor position causes a repaint when appropriate.
*/

int QLineEdit::cursorPosition() const
{
    return d->cursor->index();
}


/* IGNORE! \reimp */

void QLineEdit::setPalette( const QPalette & p )
{
    QWidget::setPalette( p );
    update();
}

void QLineEdit::setEdited( bool on )
{
    d->ed = on;
}


/* IGNORE!
    \property QLineEdit::edited
    \brief whether the line edit has been edited

    The edited flag is never read by QLineEdit; it has a default value
    of FALSE and is changed to TRUE whenever the user changes the line
    edit's contents.

    This is useful for things that need to provide a default value but
    cannot find the default at once. Just start the line edit without
    the best default; when the default is known, check the edited()
    return value and set the line edit's contents if the user has not
    started editing the line edit.

    Calling setText() resets the edited flag to FALSE.
*/

bool QLineEdit::edited() const
{
    return d->ed;
}

/* IGNORE!
    Moves the cursor one word forward. If \a mark is TRUE, the word is
    also selected.

    \sa cursorWordBackward()
*/
void QLineEdit::cursorWordForward( bool mark )
{
    d->cursor->gotoNextWord();
    if( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    update();
}


/* IGNORE!
    Moves the cursor one word backward. If \a mark is TRUE, the word
    is also selected.

    \sa cursorWordForward()
*/
void QLineEdit::cursorWordBackward( bool mark )
{
    d->cursor->gotoPreviousWord();
    if( mark )
	updateSelection();
    else {
	deselect();
	d->selectionStart = d->cursor->index();
    }
    if ( hasFocus() )
	setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
    update();
}


/*  Starts the thing blinking, or makes sure it's displayed at once. */

void QLineEdit::blinkOn()
{
    if ( !hasFocus() )
	return;

    d->blinkTimer.start( d->cursorOn
			 ? QApplication::cursorFlashTime() / 2
			 : 0,
			 TRUE );
    blinkSlot();
}

void QLineEdit::updateOffset( QTextParagraph *parag, QTextCursor *cursor )
{
    // must not call repaint() - paintEvent() calls this
    int parWidth = parag->rect().width() - 4; // QTextParagraph adds 4 pixels to the real width
    int leftGap = parag->leftGap();
    int textWidth = parWidth - leftGap;
    int w = width();
    int fw = 0;
    fw = frameWidth() + 1;
    w -= 2*fw + 4;
    int cursorPos = cursor->x();

    if ( textWidth > w ) {
	if ( d->offset + w > parWidth )
	    d->offset = parWidth - w;
	else if ( d->offset < leftGap )
	    d->offset = leftGap;
	else if ( cursorPos < d->offset )
	    d->offset = cursorPos;
	if ( cursorPos > d->offset + w )
	    d->offset = cursorPos - w;
    } else {
	int align = parag->alignment() & Qt::AlignHorizontal_Mask;
	if ( align == Qt::AlignAuto ) {
	    if ( parag->string()->isRightToLeft() )
		align = Qt::AlignRight;
	    else
		align = Qt::AlignLeft;
	}
	switch( align ) {
	    case Qt::AlignCenter:
		d->offset = leftGap - (w - textWidth)/2;
		break;
	    case Qt::AlignRight:
		d->offset = leftGap - (w- textWidth);
		break;
	    case Qt::AlignLeft:
	    default:
		d->offset = 0;
		break;
	}
    }
}

void QLineEdit::updateOffset()
{
    updateOffset( d->parag, d->cursor );
}


void QLineEdit::updateSelection()
{
    bool oldHST = hasSelectedText();
    int pos = d->cursor->index();
    int selectionStart = d->selectionStart;
    int selectionEnd;
    int oldStart = d->parag->selectionStart( QTextDocument::Standard ),
          oldEnd = d->parag->selectionEnd( QTextDocument::Standard );
    if ( pos > selectionStart ) {
	selectionEnd = pos;
    } else {
	selectionEnd = selectionStart;
	selectionStart = pos;
    }
    d->parag->setSelection( QTextDocument::Standard,
			    selectionStart, selectionEnd );

#ifndef QT_NO_CLIPBOARD
    if (! d->mousePressed && QApplication::clipboard()->supportsSelection()) {
	d->clipboard_mode = QClipboard::Selection;
	copy();
	d->clipboard_mode = QClipboard::Clipboard;
    }
#endif // QT_NO_CLIPBOARD

    if ( (oldHST != hasSelectedText()) ||
  	 (oldStart != selectionStart) ||
  	 (oldEnd != selectionEnd) )
	emit selectionChanged();
}


void QLineEdit::removeSelectedText()
{
    insert( QString::null );
    emit selectionChanged();
}


/* IGNORE!
    Undoes the last operation.
*/

void QLineEdit::undo()
{
    if ( hasMask() ) return;
    QString oldText = text( FALSE );
    d->undoRedoInfo.clear();
    d->parag->undo( d->cursor );
    if ( oldText != text( FALSE ) )
	emit textChanged( text( FALSE ) );
    update();
}


/* IGNORE!
    Redoes the last operation.
*/

void QLineEdit::redo()
{
    if ( hasMask() ) return;
    QString oldText = text( FALSE );
    d->undoRedoInfo.clear();
    d->parag->redo( d->cursor );
    if ( oldText != text( FALSE ) )
	emit textChanged( text( FALSE ) );
    update();
}

/* IGNORE!
    This function is called to create the popup menu which is shown
    when the user clicks on the line edit with the right mouse button.
    If you want to create a custom popup menu, reimplement this
    function and return the popup menu you create. The popup menu's
    ownership is transferred to the caller.
*/

QPopupMenu *QLineEdit::createPopupMenu()
{
#ifndef QT_NO_POPUPMENU
    QPopupMenu *popup = new QPopupMenu( 0, "qt_edit_menu" );
    if ( !hasMask() ) {
    d->id[ IdUndo ] = popup->insertItem( tr( "&Undo" ) + ACCEL_KEY( Z ) );
    d->id[ IdRedo ] = popup->insertItem( tr( "&Redo" ) + ACCEL_KEY( Y ) );
    popup->insertSeparator();
    }
#ifndef QT_NO_CLIPBOARD
    d->id[ IdCut ] = popup->insertItem( tr( "Cu&t" ) + ACCEL_KEY( X ) );
    d->id[ IdCopy ] = popup->insertItem( tr( "&Copy" ) + ACCEL_KEY( C ) );
    d->id[ IdPaste ] = popup->insertItem( tr( "&Paste" ) + ACCEL_KEY( V ) );
#endif
    d->id[ IdClear ] = popup->insertItem( tr( "Clear" ) );
    popup->insertSeparator();
#if defined(Q_WS_X11)
    d->id[ IdSelectAll ] = popup->insertItem( tr( "Select All" ) );
#else
    d->id[ IdSelectAll ] = popup->insertItem( tr( "Select All" ) + ACCEL_KEY( A ) );
#endif
    bool enableUndo = !d->readonly && isUndoAvailable();
    popup->setItemEnabled( d->id[ IdUndo ], enableUndo );
    bool enableRedo = !d->readonly && isRedoAvailable();
    popup->setItemEnabled( d->id[ IdRedo ], enableRedo );
#ifndef QT_NO_CLIPBOARD
    bool enableCut = !d->readonly && hasSelectedText();
    popup->setItemEnabled( d->id[ IdCut ], enableCut );
    popup->setItemEnabled( d->id[ IdCopy ], hasSelectedText() );
    bool enablePaste = !d->readonly &&
		       !QApplication::clipboard()->text( d->clipboard_mode ).isEmpty();
    popup->setItemEnabled( d->id[ IdPaste ], enablePaste );
#endif
    bool enableClear = !d->readonly && !text( FALSE ).isEmpty();
    popup->setItemEnabled( d->id[ IdClear ], enableClear );
    bool allSelected = (d->parag->selectionStart( 0 ) == 0 && d->parag->selectionEnd( 0 ) == (int)text( FALSE ).length() );
    popup->setItemEnabled( d->id[ IdSelectAll ], (bool)text( FALSE ).length() && !allSelected );

    return popup;
#else
    return 0;
#endif
}

void QLineEdit::setDragEnabled( bool b )
{
    d->dragEnabled = b;
}

/* IGNORE!
    \property QLineEdit::dragEnabled
    \brief whether the lineedit starts a drag if the user presses and
    moves the mouse on some selected text
*/

bool QLineEdit::dragEnabled() const
{
    return d->dragEnabled;
}

/* IGNORE!
    This function sets \a *start to the position in the text where
    the selection starts and \a *end to the position where the
    selection ends. Returns TRUE if both \a start and \a end are not 0
    and if there is some selected text; otherwise returns FALSE.

    \sa setSelection()
*/

bool QLineEdit::getSelection( int *start, int *end )
{
    if ( !start || !end )
	return FALSE;
    if ( !hasSelectedText() )
	return FALSE;
    *start = d->parag->selectionStart( QTextDocument::Standard );
    *end = d->parag->selectionEnd( QTextDocument::Standard );
    return TRUE;
}

/* IGNORE! \reimp */
void QLineEdit::windowActivationChange( bool )
{
    if ( !isVisible() )
	return;

    if ( palette().active() != palette().inactive() )
	update();
}

/* IGNORE!
    Returns the index position of the character which is at \a xpos
    (in logical coordinates from the left). If \a chr is not 0, \a
    *chr is populated with the character at this position.
*/

int QLineEdit::characterAt( int xpos, QChar *chr ) const
{
    QTextCursor c;
    c.setParagraph( d->parag );
    c.setIndex( 0 );
    c.place( QPoint( xpos, 0 ), c.paragraph() );
    if ( chr )
	*chr = c.paragraph()->at( c.index() )->c;
    return c.index();
}

/* IGNORE!
    \property QLineEdit::undoAvailable
    \brief whether undo is available
*/
bool QLineEdit::isUndoAvailable() const
{
    if ( hasMask() )
	return FALSE;
    else
	return d->parag->commands()->isUndoAvailable();
}

/* IGNORE!
    \property QLineEdit::redoAvailable
    \brief whether redo is available
*/
bool QLineEdit::isRedoAvailable() const
{
    if ( hasMask() )
	return FALSE;
    else
	return d->parag->commands()->isRedoAvailable();
}

/*!
  Sets the mask for this QLineEdit.

  Unset the mask and return to normal QLineEdit operation by
  passing an empty string ("") or just calling it with no arguments.

  The mask format takes these mask characters:
  \list
  \i \c L - alphabetic character required. A-Z, a-z.
  \i \c l - alphabetic character permitted but not required.
  \i \c A - alphanumeric character required. A-Z, a-z, 0-9.
  \i \c a - alphanumeric character permitted but not required.
  \i \c C - printable character required.
  \i \c c - printable character permitted but not required.
  \i \c 0 - numeric character required. 0-9.
  \i \c 9 - numeric character permitted but not required.
  \i \c # - numeric character or plus/minus sign permitted but not required.

  \i \c > - All following alphabetic characters are lowercased.
  \i \c < - All following alphabetic characters are uppercased.
  \i \c <> - No case conversion.

  \i <tt>\\</tt> - Use <tt>\\</tt> to escape the above characters to use them as separators.
  \endlist

  The \a mask string has mask characters, separators and then optionally
  the character used for blanks (separated by a semi-colon). The default blank character
  is space.

  Examples:
  \list
   \i \c "999.999.999.999;_" IP address, blanks are \c _
  \i \c "99/99/9999;0" Date, blanks are \c 0
  \i \c ">AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;#" License number, blanks are \c -
  and all characters (alphabetic) are converted to uppercase.
  \endlist

  To get range control (like with an IP address) use masks together with validators.
*/
void QLineEdit::setMask( const QString &mask = "")
{
    parseMaskFields( mask );
}

/*!
  Returns TRUE if a mask has been set, FALSE otherwise.
*/
bool QLineEdit::hasMask() const
{
    return !d->mask.isEmpty();
}


/*!
  Checks the content of the QLineEdit compared to the set mask
  and returns TRUE if the input fits with the mask. The validator (if set)
  is also checked and needs to be Acceptable for this function to return TRUE.

  Returns FALSE on either invalid input or if no mask specified.

*/
bool QLineEdit::isValidInput() const
{
    QString str = d->parag->string()->toString();
    str.remove( str.length() - 1, 1 );

    if ( ( !hasMask() ) || ( str.length() != d->maskList->count() ) )
 	return FALSE;

    for ( uint i=0; i < d->maskList->count(); i++) {
	if ( (*d->maskList)[i].separator ) {
	    if ( str[i] != (*d->maskList)[i].maskChar )
		return FALSE;
	} else {
	    if ( !isValidInput( str[i], (*d->maskList)[i].maskChar ) )
		return FALSE;
	}
    }
    const QValidator * v = validator();
    int pos = d->cursor->index();

    if ( v )
	return ( v->validate( str, pos ) == QValidator::Acceptable );
    else
	return TRUE;
}



/*
  Implements del() and backspace().
*/
void QLineEdit::delOrBackspace( bool backspace )
{
    if ( hasSelectedText() ) {
	removeSelectedText();
    } else {
	int oldPos = d->cursor->index();
	int newPos = oldPos;
	QString newText;

	if ( backspace )
	    newPos--;
	if ( newPos >= 0 ) {
	    bool ok = TRUE;
#ifndef QT_NO_VALIDATOR
	    if ( d->validator ) {
		newText = text( FALSE );
		if ( hasMask() )
		    newText.replace( newPos, 1, clearString( newPos, 1 ) );
		else
		    newText.remove( newPos, 1 );
		QString oldText = text( FALSE );
		ok = ( ( d->validator->validate( oldText, oldPos ) == QValidator::Invalid ) ||
		       ( d->validator->validate( newText, newPos ) != QValidator::Invalid ) );
	    }
#endif

	    if ( ok ) {
		d->checkUndoRedoInfo( backspace ? UndoRedoInfo::Backspace :
				      UndoRedoInfo::Delete );
		if ( backspace ) {
		    d->cursor->gotoPreviousLetter();
		    d->undoRedoInfo.index = d->cursor->index();
		}
		QChar ch = d->cursor->paragraph()->at( d->cursor->index() )->c;
		if ( backspace ) {
		    d->undoRedoInfo.text.prepend( ch );
		} else {
		    d->undoRedoInfo.text.append( ch );
		}
		if ( hasMask() ) {
		    QString t1 = d->parag->string()->toString();
		    t1.remove( t1.length() - 1, 1 );
		    int idx = d->cursor->index();
		    t1.replace( idx, 1, clearString( idx, 1 ) );
		    setText( t1 );
		    d->cursor->setIndex( idx );
		} else
		    d->cursor->remove();

#ifndef QT_NO_VALIDATOR
		if ( d->validator ) {
		    if ( newText != text( FALSE ) )
			setText( newText );
		    d->cursor->setIndex( newPos );
		}
#endif
		d->selectionStart = d->cursor->index();
		d->ed = TRUE;
		update();
		setMicroFocusHint( d->cursor->x() - d->offset, d->cursor->y(), 0, d->cursor->paragraph()->rect().height(), TRUE );
		emit textChanged( text() );
#if defined(QT_ACCESSIBILITY_SUPPORT)
		QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
	    }
	}
    }
}

/* parses the maskfields, sets the mask, if to save literals and the space/blank character */
void QLineEdit::parseMaskFields( const QString &maskFields ) {

    if ( maskFields.isEmpty() ) {
	if ( d->maskList ) {
	    delete d->maskList;
	    d->maskList = 0;
	}
	d->maskFields = "";
	d->mask = "";
	setMaxLength( 32767 );
	d->undoRedoInfo.clear( TRUE );
	return;
    }

    if ( !d->maskList )
	d->maskList = new QValueList<MaskInputData>;

    d->maskList->clear();

    d->maskFields = maskFields;

    d->mask =  d->maskFields.section( ';', 0, 0 );
    d->blank = d->maskFields.section( ';', 1, 1 ).at(0);
    if ( d->blank == 0 )
	d->blank = ' ';

    MaskInputData::Casemode m = MaskInputData::None;
    QChar c = 0;
    QChar p = 0;
    bool s;
    bool escape = FALSE;
    for ( uint i = 0; i < d->mask.length(); i++ ) {
	c = d->mask.at(i);


	if ( escape ) {
	    s = TRUE;
	    d->maskList->append( MaskInputData(c, TRUE, m));
	    escape = FALSE;
	} else if ( ( c == '<' ) || ( c == '>' ) ) {
	    switch ( c ) {
	    case '<':
		m = MaskInputData::Lower;
		break;
	    case '>':
		if ( p == '<' )
		    m = MaskInputData::None;
		else
		    m = MaskInputData::Upper;
		break;
	    }
	} else {
	    switch ( c ) {
	    case 'L':
	    case 'l':
	    case 'A':
	    case 'a':
	    case 'C':
	    case 'c':
	    case '0':
	    case '9':
	    case '#':
		s = FALSE;
		break;
	    case '\\':
		escape = TRUE;
	    default:
		s = TRUE;
		break;
	    }

	    if ( !escape )
		d->maskList->append( MaskInputData(c, s, m));
	}
	p = c;
    }
    setMaxLength( d->maskList->count() );

    d->initString = "";
    for ( uint i=0; i < d->maskList->count(); i++) {
	if ( (*d->maskList)[ i ].separator ) d->initString.append( (*d->maskList)[ i ].maskChar );
	else d->initString.append( d->blank );
    }
    setText( d->initString );
}


/*
  Finds position of next separator (inclusive).
  Calling this when no mask is set is undefined.
*/
int QLineEdit::nextSeparator( uint pos )
{
    if (pos >= d->maskList->count()) return -1;
    for ( uint i=pos; i < d->maskList->count(); i++) {
	if ( (*d->maskList)[ i ].separator ) return i;
    }
    return -1;
}

/*
  Finds position of next separator of the specified char.
  Calling this when no mask is set is undefined.
*/
int QLineEdit::nextSeparator( uint pos, QChar sep )
{
    if ( pos >= d->maskList->count() ) return -1;
    int p = nextSeparator( pos );
    if ( p == -1 ) return -1;

    if ( (*d->maskList)[ p ].maskChar == sep ) return p;
    else return nextSeparator( p + 1, sep );
}

/*
  Finds position of next blank (inclusive)
  Calling this when no mask is set is undefined.
*/
int QLineEdit::nextBlank( uint pos )
{
    if (pos >= d->maskList->count())
	return -1;
    for ( uint i=pos; i < d->maskList->count(); i++) {
	if ( !(*d->maskList)[ i ].separator ) return i;
    }
    return -1;
}

/* checks if the key is valid compared to the mask */
bool QLineEdit::isValidInput(QChar key, QChar mask) const
{
    switch ( mask ) {
    case 'L':
	if ( key.isLetter() )
	    return TRUE;
	break;
    case 'l':
	if ( key.isLetter() || ( key == d->blank ) )
	    return TRUE;
	break;
    case '0':
	if ( key.isNumber() )
	    return TRUE;
	break;
    case '9':
	if ( key.isNumber() || ( key == d->blank ) )
	    return TRUE;
	break;
    case 'A':
	if ( key.isLetterOrNumber() )
	    return TRUE;
	break;
    case 'a':
	if ( key.isLetterOrNumber() || ( key == d->blank ) )
	    return TRUE;
	break;
    case 'C':
	if ( key.isPrint() )
	    return TRUE;
	break;
    case 'c':
	if ( key.isPrint() || ( key == d->blank ) )
	    return TRUE;
	break;
    case '#':
	if ( ( key.isNumber() ) || ( key == '+' ) || ( key == '-' ) )
	    return TRUE;
	break;
    default:
	break;
    }
    return FALSE;
}

/*
  Applies the mask on a string starting from position \a pos in the mask.
  Calling this when no mask is set is undefined.
*/
QString QLineEdit::maskString( uint pos, const QString &str ) {
    if (pos >= d->maskList->count()) return "";

    QString t1 = d->parag->string()->toString();
    t1.remove( t1.length() - 1, 1 );

    uint strIndex = 0;
    QString s = "";
    for ( uint i=pos; i<d->maskList->count(); i++) {
	if ( strIndex < str.length() ) {
	    if ( (*d->maskList)[i].separator ) {
		s += (*d->maskList)[i].maskChar;
		if ( str[strIndex] == (*d->maskList)[i].maskChar ) strIndex++;
	    } else {
		if ( isValidInput( str[strIndex], (*d->maskList)[i].maskChar ) ) {
		    switch ( (*d->maskList)[i].caseMode ) {
		    case MaskInputData::Upper:
			s += str[strIndex].upper();
			break;
		    case MaskInputData::Lower:
			s += str[strIndex].lower();
			break;
		    default:
			s += str[strIndex];
		    }
		} else {
		    int n = nextSeparator( i, str[strIndex] );
		    if (n != -1 ) {
			s += t1.mid( i, n-i+1 );
			i = n; // updates new pos since we might have advanced more then one char
		    } else
			if ( str.length() > 1 ) s += d->blank; // only blanks if more then one char in str
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
  Calling this when no mask is set is undefined.
*/
QString QLineEdit::clearString( uint pos, uint len ) {
    if (pos >= d->maskList->count()) return "";

    QString s = "";
    uint end = QMIN( d->maskList->count(), pos + len );
    for ( uint i=pos; i<end; i++ )
	if ( (*d->maskList)[i].separator )
	    s += (*d->maskList)[i].maskChar;
	else
	    s += d->blank;

    return s;
}

/*
  Strips blank parts of the input in a QLineEdit when a mask is set,
  separators are still included. Typically "127.0__.0__.1__" becomes "127.0.0.1".
*/
QString QLineEdit::stripString( const QString &str ) const
{
    if ( !hasMask() ) return str;
    QString s;
    for (uint i=0; i<QMIN( d->maskList->count(), str.length() ); i++)
	if ( (*d->maskList)[i].separator )
	    s += (*d->maskList)[i].maskChar;
	else
	    if ( str[i] != d->blank )
		s += str[i];

    return s;
}

/*
  If \a strip is TRUE it just calls the public text() function (which strips if a
  mask is set) on FALSE it returns the string with no stripping. For internal QLineEdit usage
  text( FALSE ); is probably the best.
*/
QString QLineEdit::text( bool strip ) const
{
    if ( strip ) return text();
    else {
	QString s = d->parag->string()->toString();
	s.remove( s.length() - 1, 1 ); // get rid of trailing space
	return s;
    }
}

#endif
