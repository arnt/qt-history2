/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlined.cpp#102 $
**
** Implementation of QLineEdit widget class
**
** Created : 941011
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlined.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qfontmet.h"
#include "qpixmap.h"
#include "qkeycode.h"
#include "qclipbrd.h"
#include "qapp.h"
#include "qvalidator.h"

#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qlined.cpp#102 $");

//### How to provide new member variables while keeping binary compatibility:
#if QT_VERSION == 200
#error "Remove QLineEdit dict!"
#endif

#include <qintdict.h>


struct QLineEditExtra {
    bool frame;
    QLineEdit::EchoMode mode;
    QValidator * validator;
};


static QIntDict<QLineEditExtra> *qle_extraStuff = 0;

static void cleanupLineEdit()
{
    delete qle_extraStuff;
    qle_extraStuff = 0;
}


static QLineEditExtra * makeLEDict( QLineEdit * that )
{
    if ( !qle_extraStuff ) {
	qle_extraStuff = new QIntDict<QLineEditExtra>;
	CHECK_PTR( qle_extraStuff );
	qAddPostRoutine( cleanupLineEdit );
    }

    QLineEditExtra * x = (QLineEditExtra *)qle_extraStuff->find( (long)that );
    if ( !x ) {
	x = new QLineEditExtra;
	x->frame = TRUE;
	x->mode = QLineEdit::Normal;
	x->validator = 0;
	qle_extraStuff->insert( (long)that, x );
    }

    return x;
}


static QLineEditExtra * lookInLEDict( const QLineEdit * that )
{
    QLineEditExtra * x = 0;
    if ( qle_extraStuff )
	x = (QLineEditExtra *)qle_extraStuff->find( (long)that );
    return x;
}


/*!
  \class QLineEdit qlined.h

  \brief The QLineEdit widget is a simple line editor for inputting text.

  \ingroup realwidgets

  \define QLineEdit::EchoMode

  The default QLineEdit object has its own frame as specified by the
  Windows/Motif style guides, you can turn off the frame by calling
  setFrame( FALSE ).

  It draws the text using its own \link QColorGroup color group:
  \endlink \link QColorGroup::text() colorGroup().text() \endlink on
  \link QColorGroup::base() colorGroup().base(). \endlink  The cursor
  and frame use other colors from same color group, of course.

  QLineEdit can display the content of itself in three ways, depending
  on the current \link setEchoMode() echo mode. \endlink The echo
  modes available are: <ul> <li> \c Normal - display characters as
  they are entered.  This is the default. <li> \c NoEcho - do not
  display anything. <li> \c Password - display asterisks instead of
  the characters actually entered. </ul>

  The default key bindings are described in keyPressEvent(); they cannot
  be customized except by inheriting the class.

  <img src=qlined-m.gif> <img src=qlined-w.gif>
*/


/*!
  \fn void QLineEdit::textChanged( const char * )
  This signal is emitted every time the text has changed.
  The argument is the new text.
*/


static const int blinkTime  = 500;		// text cursor blink time
static const int scrollTime = 100;		// mark text scroll time


static int xPosToCursorPos( char *s, const QFontMetrics &fm,
			    int xPos, int width )
{
    char *tmp;
    int	  dist;

    if ( xPos > width )
	xPos = width;
    if ( xPos <= 0 )
	return 0;
    dist = xPos;
    tmp	 = s;
    while ( *tmp && dist > 0 )
	dist -= fm.width( tmp++, 1 );
    if ( dist < 0 && ( xPos - dist > width || fm.width( tmp - 1, 1)/2 < -dist))
	tmp--;
    return tmp - s;
}

static int showLastPartOffset( char *s, const QFontMetrics &fm, int width )
{
    if ( !s || s[0] == '\0' )
	return 0;
    char *tmp = &s[strlen( s ) - 1];
    do {
	width -= fm.width( tmp--, 1 );
    } while ( tmp >=s && width >=0 );
    return width < 0 ? tmp - s + 2 : 0;
}


/*!
  Constructs a line editor with an empty edit buffer.

  The cursor position is set to the start of the line, the maximum buffer
  size to 32767 characters, and the buffer contents to "".

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QLineEdit::QLineEdit( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    initMetaObject();
    pm = new QPixmap( width(), height() );   // used for flicker-free update
    CHECK_PTR( pm );
    cursorPos	  = 0;
    offset	  = 0;
    maxLen	  = 32767;
    cursorOn	  = TRUE;
    markAnchor	  = 0;
    markDrag	  = 0;
    dragScrolling = FALSE;
    scrollingLeft = FALSE;
    tbuf	  = "";
    setFocusPolicy( StrongFocus );
    setCursor( ibeamCursor );
    setBackgroundMode( PaletteBase );
}

/*!
  Destroys the line editor.
*/

QLineEdit::~QLineEdit()
{
    if ( qle_extraStuff )
	qle_extraStuff->remove( (long)this );
    delete pm;
}


/*!
  Sets the line editor text to \e text, clears the selection and moves
  the cursor to the end of the line.

  If necessary the text is truncated to fit maxLength().

  \sa text()
*/

void QLineEdit::setText( const char *text )
{
    QString oldText( tbuf );
    oldText.detach();
    tbuf = text ? text : "";
    if ( (int)tbuf.length() > maxLen ) {
	tbuf.resize( maxLen+1 );
	tbuf[maxLen] = '\0';
    }
    offset    = 0;
    cursorPos = 0;
    markAnchor = 0;
    markDrag = 0;
    end( FALSE );
    if ( validator() )
	(void)validator()->validate( tbuf, cursorPos );
    repaint( !hasFocus() );
    if ( oldText != tbuf )
	emit textChanged( tbuf );
}


/*!
  Selects all text (i.e. marks it) and moves the cursor to the
  end. Useful when a default value has been inserted. If the user
  types before clicking on the widget the selected text will be
  erased.
*/

void QLineEdit::selectAll()
{
    markAnchor = 0;
    markDrag   = 0;
    cursorPos  = 0;
    end( TRUE );
}



/*!
  Deselects all text (i.e. removes marking) and leaves the cursor at the
  current position.
*/

void QLineEdit::deselect()
{
    markAnchor = cursorPos;
    markDrag   = cursorPos;
    repaint( !hasFocus() );
}


/*!
  Returns a pointer to the text currently in the line.

  If you need to store the text, you should make a copy of it. This can
  conveniently be done with a QString object:
  \code
    QString s = lineEd->text();	 // makes a copy and stores it in s
  \endcode

  \sa setText()
*/

const char *QLineEdit::text() const
{
    return tbuf;
}

/*!
  Returns TRUE if part of the text has been marked by the user (e.g. by
  clicking and dragging).
*/

bool QLineEdit::hasMarkedText() const
{
    return markAnchor != markDrag;
}

/*!
  Returns the text marked by the user (e.g. by clicking and
  dragging), or 0 if no text is marked.
  \sa hasMarkedText()
*/

QString QLineEdit::markedText() const
{
    if ( markAnchor != markDrag ) {
	return tbuf.mid( minMark(), maxMark() - minMark() );
    } else {
	return 0;
    }
}

/*!
  Returns the current maximum length of the text in the editor.
  \sa setMaxLength()
*/

int QLineEdit::maxLength() const
{
    return maxLen;
}

/*!
  Set the maximum length of the text in the editor.  If the text is
  currently too long, it is chopped off at the limit. Any marked text will
  be unmarked.	The cursor position is set to 0 and the first part of the
  string is shown. \sa maxLength().
*/

void QLineEdit::setMaxLength( int m )
{
    maxLen = m;
    markAnchor = 0;
    markDrag   = 0;
    if ( (int)tbuf.length() > maxLen ) {
	tbuf.resize( maxLen + 1 );		// include \0
	tbuf[maxLen] = '\0';
    }
    offset    = 0;
    cursorPos = 0;
    repaint( !hasFocus() );
}

/*!
  \fn void  QLineEdit::returnPressed()
  This signal is emitted when the return or enter key is pressed.
*/

/*!
  The key press event handler converts a key press to some line editor
  action.

  If return or enter is pressed and the current text is valid (or if
  the validator can \link QValidator::fixup() make the text
  valid\endlink), the signal returnPressed is emitted.

  Here are the default key bindings:
  <ul>
  <li><i> Left Arrow </i> Move the cursor one character leftwards
  <li><i> Right Arrow </i> Move the cursor one character rightwards
  <li><i> Backspace </i> Delete the character to the left of the cursor
  <li><i> Home </i> Move the cursor to the beginning of the line
  <li><i> End </i>	 Move the cursor to the end of the line
  <li><i> Delete </i> Delete the character to the right of the cursor
  <li><i> Shift - Left Arrow </i> Mark text one character leftwards
  <li><i> Shift - Right Arrow </i> Mark text one character rightwards
  <li><i> Control-A </i> Move the cursor to the beginning of the line
  <li><i> Control-B </i> Move the cursor one character leftwards
  <li><i> Control-C </i> Copy the marked text to the clipboard.
  <li><i> Control-D </i> Delete the character to the right of the cursor
  <li><i> Control-E </i> Move the cursor to the end of the line
  <li><i> Control-F </i> Move the cursor one character rightwards
  <li><i> Control-H </i> Delete the character to the left of the cursor
  <li><i> Control-V </i> Paste the clipboard text into line edit.
  <li><i> Control-X </i> Cut the marked text, copy to clipboard.
  </ul>

  All other keys with valid ASCII codes insert themselves into the line.
*/

void QLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Enter || e->key() == Key_Return ) {
	QValidator * v = validator();
	if ( !v || v->validate( tbuf, cursorPos ) == QValidator::Acceptable ) {
	    emit returnPressed();
	} else if ( v ) {
	    v->fixup( tbuf );
	    if ( v->validate( tbuf, cursorPos ) == QValidator::Acceptable )
		 emit returnPressed();
	}
	e->ignore();
	return;
    }
    QValidator * v = validator();
    if ( e->ascii() >= 32 && e->key() != Key_Delete ) {
	QString test( tbuf.copy() );
	int cp = cursorPos;
	if ( hasMarkedText() ) {
	    test.remove( minMark(), maxMark() - minMark() );
	    cp = minMark();
	}
	if ( (int)test.length() < maxLen )
	    test.insert( cp, e->ascii() );
	if ( v &&
	     v->validate( test, cp ) == QValidator::Invalid &&
	     v->validate( tbuf, cursorPos ) != QValidator::Invalid ) {
	    // add stuff to indicate the error here and suggest remedies
	    return;
	}

	if ( test != tbuf ) {
	    cursorPos = cp;
	    tbuf = test;
	    cursorRight( FALSE );
	    emit textChanged( tbuf );
	}
	return;
    }
    int unknown = 0;
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
	case Key_Left:
	    home( e->state() & ShiftButton );
	    break;
	case Key_B:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_C:
	    if ( hasMarkedText() )
		copyText();
	    break;
	case Key_D:
	    del();
	    break;
	case Key_E:
	case Key_Right:
	    end( e->state() & ShiftButton );
	    break;
	case Key_F:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_H:
	    backspace();
	    break;
	case Key_V: {			// paste
	    QString t = QApplication::clipboard()->text();
	    if ( !t.isEmpty() ) {
		t.detach();
		int i = t.find( '\n' );	// no multiline text
		if ( i >= 0 )
		    t.truncate( i );
		uchar *p = (uchar *) t.data();
		while ( *p ) {		// unprintable becomes space
		    if ( *p < 32 )
			*p = 32;
		    p++;
		}
		int tlen = t.length();
		int blen;
		int cp = cursorPos;
		// do a test run without hurting tbuf and stuf
		QString test( tbuf.copy() );
		if ( hasMarkedText() ) {
		    test.remove( minMark(), maxMark() - minMark() );
		    if ( cp > maxMark() )
			cp -= (maxMark() - minMark());
		    else if ( cp > minMark() )
			cp = minMark();
		}
		blen = test.length();
		if ( tlen+blen >= maxLen ) {
		    if ( blen >= maxLen )
			break;
		    if ( tlen > maxLen )
			tlen = maxLen;
		    t.truncate( maxLen-blen+1 );
		    t[maxLen-blen] = '\0';
		}
		test.insert( cp, t );
		cp += t.length();

		if ( v &&
		     v->validate( test, cp ) == QValidator::Invalid &&
		     v->validate( tbuf, cursorPos ) != QValidator::Invalid )
		    break;

		// okay, it succeeded, so use those changes.
		tbuf = test;
		cursorPos = cp;
		repaint( FALSE );
		emit textChanged( tbuf );
	    }
	}
	case Key_X:
	    if ( hasMarkedText() ) {
		copyText();
		del();
	    }
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
	    backspace();
	    break;
	case Key_Home:
	    home( e->state() & ShiftButton );
	    break;
	case Key_End:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Delete:
	    del();
	    break;
	default:
	    unknown++;
	}
    }

    if ( unknown ) {				// unknown key
	e->ignore();
	return;
    }
}


/*!
  Handles the cursor blinking.
*/

void QLineEdit::focusInEvent( QFocusEvent * )
{
    if ( style() == WindowsStyle && markAnchor == markDrag )
	selectAll(); // calls repaint
    killTimers();
    startTimer( blinkTime );
    cursorOn = TRUE;
    repaint( !hasFocus() );
}


/*!
  Handles the cursor blinking.
*/

void QLineEdit::focusOutEvent( QFocusEvent * )
{
    if ( style() == WindowsStyle && 
	 ( focusWidget() != this ||
	   qApp->focusWidget() == 0 ||
	   qApp->focusWidget()->topLevelWidget() != topLevelWidget() ) )
	deselect();
    killTimers();
    cursorOn	  = FALSE;
    dragScrolling = FALSE;
    repaint( !hasFocus() );
}


/*!
  Handles paint events for the line editor.
*/  

void QLineEdit::paintEvent( QPaintEvent *event )
{
    paint( event->rect(), TRUE /* remove for 2.0 */ );
}


/*!
  This event is used to implement the blinking text cursor
  and scrolling when marking text.
*/

void QLineEdit::timerEvent( QTimerEvent * )
{
    if ( hasFocus() ) {
	if ( dragScrolling ) {
	    if ( scrollingLeft )
		cursorLeft( TRUE );	// mark left
	    else
		cursorRight( TRUE );	// mark right
	} else {
	    cursorOn = !cursorOn;
	    repaint( !hasFocus() );
	}
    }
}


/*!
  Handles resize events for this widget.
*/

void QLineEdit::resizeEvent( QResizeEvent *e )
{
    pm->resize( e->size() );
    int max = lastCharVisible();
    if ( cursorPos > max ) {
	QFontMetrics fm = fontMetrics();
	int w = width() - (frame() ? 8 : 4);
	int i = cursorPos;
	while ( w > 0 && i > 0 ) {
	    i--;
	    w -= fm.width( tbuf.at(i) );
	}
	if ( w < 0 && i != cursorPos )
	    i++;
	offset = i;
    } else if ( offset ) {
	int i = showLastPartOffset( tbuf.data(), fontMetrics(),
				    width() - (frame() ? 8 : 4) );
	if ( i < offset )
	    offset = i;
    }
    repaint( !hasFocus() );
}


/*!
  Handles mouse press events for this widget.
*/

void QLineEdit::mousePressEvent( QMouseEvent *e )
{
    killTimers();
    int margin = frame() ? 4 : 2;
    cursorPos = offset + xPosToCursorPos( &tbuf[(int)offset], fontMetrics(),
				       e->pos().x() - margin,
				       width() - 2*margin );
    if ( e->button() == MidButton ) {		// paste
	QKeyEvent k( Event_KeyPress, Key_V, 0, ControlButton );
	keyPressEvent( &k );
	return;
    }
    markAnchor = cursorPos;
    newMark( markAnchor, FALSE );
    cursorOn	  = TRUE;
    dragScrolling = FALSE;
    startTimer( blinkTime );
    repaint( !hasFocus() );
}


/*!
  Handles mouse move events for the line editor, primarily for
  marking text.
*/

void QLineEdit::mouseMoveEvent( QMouseEvent *e )
{
    int margin = frame() ? 4 : 2;

    if ( e->pos().x() < margin || e->pos().x() > width() - margin) {
	scrollingLeft =	 ( e->pos().x() < margin );
	if ( !dragScrolling ) {
	    if ( scrollingLeft )
		newMark( offset, FALSE );
	    else
		newMark( lastCharVisible(), FALSE );
	    killTimers();
	    dragScrolling = TRUE;
	    cursorOn	  = TRUE;
	    startTimer( scrollTime );
	    repaint( !hasFocus() );
	} else {
	    if ( scrollingLeft ) {
		int steps = -(e->pos().x() + margin) / 15 + 2;
		cursorLeft( TRUE, steps );
	    } else {
		int steps = (e->pos().x() - width() +  margin) / 15 + 2;
		cursorRight( TRUE, steps );
	    }
	}
    } else {
	dragScrolling = FALSE;
	int mousePos  = offset +
	    xPosToCursorPos( &tbuf[(int)offset], fontMetrics(),
			     e->pos().x() - margin,
			     width() - margin - margin );
	newMark( mousePos, FALSE );
	cursorOn  = TRUE;
	killTimers();
	startTimer( blinkTime );
	repaint( !hasFocus() );
    }
}

/*!
  Handles mouse release events for this widget.
*/

void QLineEdit::mouseReleaseEvent( QMouseEvent * )
{
    if ( style() == MotifStyle )
	copyText();
    if ( dragScrolling ) {
	dragScrolling = FALSE;
	killTimers();
	startTimer( blinkTime );
    }
}

/*!
  Handles mouse double click events for this widget.
*/

void QLineEdit::mouseDoubleClickEvent( QMouseEvent * )
{
    if ( dragScrolling ) {
	dragScrolling = FALSE;
	killTimers();
	startTimer( blinkTime );
    }
    markWord( cursorPos );
    repaint( !hasFocus() );
}

/*!
  \internal
  Repaints the line editor as needed. If the line editor is in
  focus, the line is painted using a pixmap buffer. If not, a faster
  but flickering drawing method is used.
*/

void QLineEdit::paint( const QRect& clip, bool )
{
    if ( hasFocus() ) {
	pixmapPaint( clip );
    } else {
	QPainter p;
	p.begin( this );
	p.setClipRect( clip );
	paintText( &p, size(), TRUE /* remove for 2.0 */ );
	p.end();
    }
}

/*!
  \internal
  Paints the line editor in a pixmap and then blts the pixmap onto the screen.
*/

void QLineEdit::pixmapPaint( const QRect& clip )
{
    QPainter p( pm );
    p.setClipRect( clip );
    p.setFont( font() );
    paintText( &p, pm->size(), TRUE /* remove for 2.0 */ );
    p.end();
    bitBlt( this, 0, 0, pm, 0, 0, width(), height() );
}


/*!
  \internal
  Paints the line editor.
*/

void QLineEdit::paintText( QPainter *p, const QSize &s, bool )
{
    QColorGroup	 g    = colorGroup();
    QColor	 bg   = isEnabled() ? g.base() : g.background();
    QFontMetrics fm   = fontMetrics();
    int markBegin     = minMark();
    int markEnd	      = maxMark();
    int margin	     =  this->frame() ? 4 : 2;

    if ( frame() ) {
	QBrush fill( bg );
	qDrawWinPanel( p, 0, 0, s.width(), s.height(), g, TRUE, &fill );
    } else {
	p->fillRect( 0, 0, width(), height(), bg );
    }

    QString displayText;

    switch( echoMode() ) {
    case Normal:
	displayText = tbuf.mid( offset, tbuf.length() );
	break;
    case NoEcho:
	displayText = "";
	break;
    case Password:
	displayText.fill( '*', tbuf.length() - offset );
	break;
    }

    int ypos = s.height() - margin - fm.descent() - 1 -
	       (s.height() - 2*margin - fm.height())/2;

    if ( !displayText.isEmpty() ) {
	p->setClipRect( margin, margin,
			s.width()  - 2*margin,
			s.height() - 2*margin );

	int charsVisible = lastCharVisible() - offset;
	if ( displayText[ charsVisible ] != '\0' )
	    charsVisible++;

	int mark1,mark2; // start and end of inverted text, 0 = leftmost char

	if ( markBegin > offset ) {
	    if ( markBegin <  offset + charsVisible )
		mark1 = markBegin - offset;
	    else
		mark1 = charsVisible;
	} else {
	    mark1 = 0;
	}

	if ( markEnd > offset ) {
	    if ( markEnd <	offset + charsVisible )
		mark2 = markEnd - offset;
	    else
		mark2 = charsVisible;
	} else {
	    mark2 = 0;
	}

	// display code comes here - a bit yucky but it works
	if ( mark1 != mark2 ) {
	    QString marked( displayText.mid( mark1, mark2 - mark1 ) );
	    int xpos1 =  margin + fm.width( displayText, mark1 );
	    int xpos2 =  xpos1 + fm.width( marked ) - 1;
	    p->fillRect( xpos1, ypos - fm.ascent(),
			 xpos2 - xpos1, fm.height(),
			 style() == WindowsStyle ? darkBlue : g.text() );
	    p->setPen( g.base() );
	    p->drawText( xpos1, ypos, marked );
	}
	p->setPen( g.text() );
	if ( mark1 != 0 )
	    p->drawText( margin, ypos, displayText, mark1 );
	if ( mark2 != charsVisible ) {
	    QString rest( displayText.mid( mark2, charsVisible - mark2 ) );
	    p->drawText( margin + fm.width( displayText.left( mark2) ), ypos,
			 rest );
	}
    }

    p->setPen( g.foreground() );

    p->setClipping( FALSE );
    if ( cursorOn ) {
	int curXPos = margin;
	if ( echoMode() != NoEcho )
	    curXPos += fm.width( displayText, cursorPos - offset ) - 1;
	int curYPos   = ypos - fm.ascent();
	if ( hasFocus() ) {
	    p->drawLine( curXPos, curYPos, curXPos, curYPos + fm.height() - 1);
	    if ( style() != WindowsStyle ) {
		p->drawLine( curXPos - 2, curYPos,
			     curXPos + 2, curYPos );
		p->drawLine( curXPos - 2, curYPos + fm.height() - 1,
			     curXPos + 2, curYPos + fm.height() - 1);
	    }
	}
    }
}


/*!
  Moves the cursor leftwards one or more characters.
  \sa cursorRight()
*/

void QLineEdit::cursorLeft( bool mark, int steps )
{
    if ( steps < 0 ) {
	cursorRight( mark, -steps );
	return;
    }
    if ( cursorPos != 0 || !mark && hasMarkedText() ) {
	killTimers();
	cursorOn = TRUE;
	cursorPos -= steps;
	if ( cursorPos < 0 )
	    cursorPos = 0;
	if ( mark ) {
	    newMark( cursorPos );
	} else {
	    markAnchor = cursorPos;
	    markDrag   = markAnchor;
	}
	if ( cursorPos < offset )
	    offset = cursorPos;
	startTimer( dragScrolling ? scrollTime : blinkTime );
	repaint( !hasFocus() );
    }
}

/*!
  Moves the cursor rightwards one or more characters.
  \sa cursorLeft()
*/

void QLineEdit::cursorRight( bool mark, int steps )
{
    int margin = frame() ? 4 : 2;
    if ( steps < 0 ) {
	cursorLeft( mark, -steps );
	return;
    }
    int len = (int)strlen( tbuf );
    if ( len > cursorPos || !mark && hasMarkedText() ) {
	QFontMetrics fm = fontMetrics();
	killTimers();
	cursorOn   = TRUE;
	cursorPos += steps;
	if ( cursorPos > len )
	    cursorPos = len;
	if ( mark ) {
	    newMark( cursorPos );
	} else {
	    markAnchor = cursorPos;
	    markDrag   = markAnchor;
	}
	int surplusWidth = width() - 2*margin
			   - fm.width( &tbuf[ offset ], cursorPos - offset);
	if ( surplusWidth < 0 ) {
	    while ( surplusWidth < 0 && offset < cursorPos - 1 ) {
		surplusWidth += fm.width( &tbuf[ offset ], 1 );
		offset++;
	    }
	}
	startTimer( dragScrolling ? scrollTime : blinkTime );
	repaint( !hasFocus() );
    }
}

/*!
  Deletes the character on the left side of the text cursor and moves the
  cursor one position to the left. If a text has been marked by the user
  (e.g. by clicking and dragging) the cursor will be put at the beginning
  of the marked text and the marked text will be removed.  \sa del()
*/

void QLineEdit::backspace()
{
    if ( hasMarkedText() ) {
	del();
    } else {
	if ( cursorPos > 0 ) {
	    cursorLeft( FALSE );
	    del();
	}
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
    QString test( tbuf.copy() );
    QValidator * v = validator();

    if ( hasMarkedText() ) {
	test.remove( minMark(), maxMark() - minMark() );
	int cp = minMark();
	if ( v &&
	     v->validate( test, cp ) == QValidator::Invalid &&
	     v->validate( tbuf, cursorPos ) != QValidator::Invalid )
	    return;
	tbuf = test;
	cursorPos = cp;
	markAnchor = cursorPos;
	markDrag   = cursorPos;
	if ( cursorPos < offset )
	    offset = cursorPos;
	repaint( !hasFocus() );
	emit textChanged( tbuf );
    } else if ( cursorPos != (int)strlen(tbuf) ) {
	test.remove( cursorPos, 1 );
	int cp = cursorPos;
	if ( v &&
	     v->validate( test, cp ) == QValidator::Invalid &&
	     v->validate( tbuf, cursorPos ) != QValidator::Invalid )
	    return;
	tbuf = test;
	cursorPos = cp;
	repaint( !hasFocus() );
	emit textChanged( tbuf );
    }
}

/*!
  Moves the text cursor to the left end of the line. If mark is TRUE text
  will be marked towards the first position, if not any marked text will
  be unmarked if the cursor is moved.  \sa end()
*/

void QLineEdit::home( bool mark )
{
    if ( cursorPos != 0 || !mark && hasMarkedText() ) {
	killTimers();
	cursorPos = 0;
	if ( mark ) {
	    newMark( cursorPos );
	} else {
	    markAnchor = 0;
	    markDrag   = markAnchor;
	}
	offset	 = 0;
	cursorOn = TRUE;
	startTimer( dragScrolling ? scrollTime : blinkTime );
	repaint( !hasFocus() );
    }
}

/*!
  Moves the text cursor to the right end of the line. If mark is TRUE text
  will be marked towards the last position, if not any marked text will
  be unmarked if the cursor is moved.
  \sa home()
*/

void QLineEdit::end( bool mark )
{
    int tlen = strlen( tbuf );
    if ( cursorPos != tlen || !mark && hasMarkedText() ) {
	killTimers();
	offset += showLastPartOffset( &tbuf[offset], fontMetrics(),
				      width() - (frame() ? 8 : 4) );
	cursorPos = tlen;
	if ( mark ) {
	    newMark( cursorPos );
	} else {
	    markAnchor = cursorPos;
	    markDrag   = markAnchor;
	}
	cursorOn  = TRUE;
	startTimer( dragScrolling ? scrollTime : blinkTime );
	repaint( !hasFocus() );
    }
}


/*!
  Sets a new marked text limit, does not repaint the widget.
*/

void QLineEdit::newMark( int pos, bool copy )
{
    markDrag  = pos;
    cursorPos = pos;
    if ( copy && style() == MotifStyle ) // ### ?
	copyText();
}


void QLineEdit::markWord( int pos )
{
    int i = pos - 1;
    while ( i >= 0 && isprint(tbuf.at(i)) && !isspace(tbuf.at(i)) )
	i--;
    i++;
    markAnchor = i;

    int lim = tbuf.length();
    i = pos;
    while ( i < lim && isprint(tbuf.at(i)) && !isspace(tbuf.at(i)) )
	i++;
    markDrag = i;

    int maxVis	  = lastCharVisible();
    int markBegin = minMark();
    int markEnd	  = maxMark();
    if ( markBegin < offset || markBegin > maxVis ) {
	if ( markEnd >= offset && markEnd <= maxVis ) {
	    cursorPos = markEnd;
	} else {
	    offset    = markBegin;
	    cursorPos = markBegin;
	}
    } else {
	cursorPos = markBegin;
    }
    if ( style() == MotifStyle )
	copyText();
}


/*!
  Copies the marked text to the clipboard.
*/

void QLineEdit::copyText()
{
    QString t = markedText();
    if ( !t.isEmpty() ) {
	disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
	QApplication::clipboard()->setText( t );
	connect( QApplication::clipboard(), SIGNAL(dataChanged()),
		 this, SLOT(clipboardChanged()) );
    }
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
    markDrag = markAnchor = cursorPos;
    repaint( !hasFocus() );
#endif
}



int QLineEdit::lastCharVisible() const
{
    int tDispWidth = width() - (frame() ? 8 : 4);
    return offset + xPosToCursorPos( &tbuf[(int)offset], fontMetrics(),
			       tDispWidth, tDispWidth );
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

  \sa frame() QComboBox
*/

void QLineEdit::setFrame( bool enable )
{
    if ( enable && !qle_extraStuff )
	return;

    QLineEditExtra * x = makeLEDict( this );
    x->frame = enable;
    repaint();
}


/*!  Returns TRUE if the line edit draws itself inside a frame, FALSE
  if it draws itself without any frame.

  The default is to use a frame.

  \sa setFrame()
*/

bool QLineEdit::frame() const
{
    QLineEditExtra * x = lookInLEDict( this );
    return x ? x->frame : TRUE;
}


/*!  Sets the echo mode of the line edit widget.

  The echo modes available are: <ul> <li> \c Normal - display
  characters as they are entered.  This is the default. <li> \c NoEcho
  - do not display anything. <li> \c Password - display asterisks
  instead of the characters actually entered. </ul>

  It is always possible to cut and paste any marked text; only the
  widget's own display is affected.

  \sa echoMode()
*/

void QLineEdit::setEchoMode( EchoMode mode )
{
    if ( mode == Normal && !qle_extraStuff )
	return;

    QLineEditExtra * x = makeLEDict( this );
    x->mode = mode;
    repaint();
}


/*!
  Returns the current echo mode of the line edit.

  \sa setEchoMode()
*/

QLineEdit::EchoMode QLineEdit::echoMode() const
{
    QLineEditExtra * x = lookInLEDict( this );
    return x ? x->mode : Normal;
}


/*!
  Returns a size which fits the contents of the line edit.

  The width returned tends to be enough for about 15-20 characters.
*/

QSize QLineEdit::sizeHint() const
{
    int h = fontMetrics().height();
    int margin = frame() ? 8 : 4;
    return QSize( 10*h + margin, h + margin );
}


/*!
  Sets this line edit to accept input only as accepted by \a v.

  If \a v == 0, remove the currently set input validator.  The default
  is no input validator (ie. any input is accepted up to maxLength()).

  \sa validator() QValidator
*/


void QLineEdit::setValidator( QValidator * v )
{
    if ( v == 0 && !qle_extraStuff )
	return;

    QLineEditExtra * x = makeLEDict( this );
    x->validator = v;
    repaint();
}

/*!
  Returns a pointer to the current input validator, or 0 if no
  validator has been set.
*/

QValidator * QLineEdit::validator() const
{
    QLineEditExtra * x = lookInLEDict( this );
    return x ? x->validator : 0;
}


/*!  This slot is equivalent to setValidator( 0 ). */

void QLineEdit::clearValidator()
{
    setValidator( 0 );
}
