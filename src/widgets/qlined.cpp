/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlined.cpp#26 $
**
** Implementation of QLineEdit widget class
**
** Author  : Eirik Eng
** Created : 941011
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlined.h"
#include "qpainter.h"
#include "qfontmet.h"
#include "qpixmap.h"
#include "qkeycode.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlined.cpp#26 $";
#endif


/*!
\class QLineEdit qlined.h
\brief The QLineEdit widget is a simple line editor for inputting text.

The default key bindings are described in keyPressEvent(); they cannot
be customized except by inheriting the class.

\todo inherit QFrame
\todo cleaner focus
\todo clipboard, cut, paste
\todo mark and delete
*/


/*!
\fn void QLineEdit::textChanged( char * )
This signal is emitted every time the text has changed.	 The argument
is the new text.
*/


static const int blinkTime = 500;		// text cursor blink time

#define LEFT_MARGIN 4
#define RIGHT_MARGIN 4
#define TOP_MARGIN 4
#define BOTTOM_MARGIN 4


static uint xPosToCursorPos( char *s, const QFontMetrics &fm, 
			     uint xPos, uint width )
{
    char *tmp;
    int	  dist;

    if( xPos > width )
	xPos = width;
    dist = xPos;
    tmp	 = s;
    while( *tmp && dist >= 0 )
	dist -= fm.width( tmp++, 1 );
    if( dist < 0 && ( xPos - dist > width ||
		      fm.width( tmp - 1, 1)/2 < -dist ) )
	tmp--;
    return tmp - s;
}

static uint showLastPartOffset( char *s, const QFontMetrics &fm, int width )
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
    pm		= 0;
    cursorPos	= 0;
    offset	= 0;
    maxLen	= 32767;
    cursorOn	= TRUE;
    tbuf	= "";
    setAcceptFocus( TRUE );
}

/*!
Destroys the line editor.
*/

QLineEdit::~QLineEdit()
{
    delete pm;
}

/*!
Sets the line editor text to \e text.

If necessary the text is truncated to fit maxLength().

\sa text().
*/

void QLineEdit::setText( const char *text )
{
    if ( tbuf == text )				// no change
	return;
    tbuf = text;
    if ( tbuf.length() > maxLen )
	tbuf.resize( maxLen+1 );
    cursorPos = 0;
    offset    = 0;
    paint();
    emit textChanged( tbuf.data() );
}

/*! 
Returns a pointer to the text currently in the line.  

If you need to store the text, you should make a copy of it. This can
conveniently be done with a QString object:
\code
  QString s = lineEd->text();  // makes a copy and stores it in s
\endcode

\sa setText().
*/

char *QLineEdit::text() const
{
    return tbuf.data();
}


/*!
Set the maximum length of the text in the editor.  If the text is
currently too long, it is chopped off at the limit.
\sa maxLength().
*/

void QLineEdit::setMaxLength( int m )
{
    maxLen = (uint) m;
    if ( tbuf.length() > maxLen ) {
	tbuf.resize( maxLen + 1 );		// include \0
	if ( cursorPos > maxLen ) {
	    offset = maxLen;
	    end();
	}
	paint();
    }
}

/*!
Returns the current maximum length of the text in the editor.
\sa setMaxLength().
*/

int QLineEdit::maxLength() const
{
    return maxLen;
}

/*!
The key press event handler converts a key press to some line editor
action.

Here are the default key bindings:
<dl compact>
<dt> Left Arrow <dd> Move the cursor one character leftwards
<dt> Right Arrow <dd> Move the cursor one character rightwards
<dt> Backspace <dd> Delete the character to the left of the cursor
<dt> Home <dd> Move the cursor to the beginning of the line
<dt> End <dd>  Move the cursor to the end of the line
<dt> Delete <dd> Delete the character to the right of the cursor
<dt> Control-A <dd> Move the cursor to the beginning of the line
<dt> Control-B <dd> Move the cursor one character leftwards
<dt> Control-D <dd> Delete the character to the right of the cursor
<dt> Control-E <dd> Move the cursor to the end of the line
<dt> Control-F <dd> Move the cursor one character rightwards
<dt> Control-H <dd> Delete the character to the left of the cursor
</dl>

<strong><a href=mailto:qt-bugs@troll.no>Comments solicited</a></strong>

All other keys with valid ASCII codes insert themselves into the line.

\todo shift-arrow stuff
*/

void QLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->ascii() >= 32 && e->key() != Key_Delete ) {
	if ( tbuf.length() < maxLen ) {
	    tbuf.insert( cursorPos, e->ascii() );
	    cursorRight();			// will repaint
	    emit textChanged( tbuf.data() );
	}
	return;
    }
    int unknown = 0;
    if ( e->state() == 0 ) {
	switch ( e->key() ) {
	    case Key_Left:
		cursorLeft();
		break;
	    case Key_Right:
		cursorRight();
		break;
	    case Key_Backspace:
		backspace();
		break;
	    case Key_Home:
		home();
		break;
	    case Key_End:
		end();
		break;
	    case Key_Delete:
		del();
		break;
	    default:
		unknown++;
	}
    }
    else if ( e->state() == ControlButton ) {
	switch ( e->key() ) {
	    case Key_A:
		home();
		break;
	    case Key_B:
		cursorLeft();
		break;
	    case Key_D:
		del();
		break;
	    case Key_E:
		end();
		break;
	    case Key_F:
		cursorRight();
		break;
	    case Key_H:
		backspace();
		break;
	    default:
		unknown++;
	}
    }
    else
	unknown++;

    if ( unknown ) {				// unknown key
	e->ignore();
	return;
    }
}


/*!
\internal
Starts cursor blinking.
*/

void QLineEdit::focusInEvent( QFocusEvent * )
{
    pm = new QPixmap( width(), height() );   // used for flicker-free update
    CHECK_PTR( pm );
    startTimer( blinkTime );
    cursorOn = TRUE;
    paint();
}

/*!
\internal
Stops text cursor blinking.
*/

void QLineEdit::focusOutEvent( QFocusEvent * )
{
    killTimers();
    delete pm;
    pm = 0;
    cursorOn = FALSE;
    paint();
}


void QLineEdit::paintEvent( QPaintEvent * )
{
    paint( TRUE );
}


/*!
\internal
This event is used to implement the blinking text cursor.
*/

void QLineEdit::timerEvent( QTimerEvent * )
{
    if ( hasFocus() ) {
	cursorOn = !cursorOn;
	paint();
    }
}


void QLineEdit::resizeEvent( QResizeEvent *e )
{
    if ( hasFocus() ) {
	delete pm;
	pm = new QPixmap( e->size().width(), e->size().height() );
    }
    paint();
}

/*!
\internal
Sets the text cursor.
\todo drag-to-mark, paste
*/

void QLineEdit::mousePressEvent( QMouseEvent *e )
{
    killTimers();
    cursorPos = offset +
	xPosToCursorPos( &tbuf[offset], fontMetrics(),
			 e->pos().x() - LEFT_MARGIN,
			 width() - LEFT_MARGIN - RIGHT_MARGIN );
    cursorOn = TRUE;
    startTimer( blinkTime );
    paint();
}


/*!
\internal
Repaints the line editor as needed. If the line editor is in
focus, the line is painted using a pixmap buffer. If not, a faster
but flickering drawing method is used.
*/

void QLineEdit::paint( bool frame )
{
    if ( hasFocus() ) {
	pixmapPaint();
    } else {
	QPainter p;
	p.begin( this );
	if ( !frame )
	    p.eraseRect( LEFT_MARGIN, TOP_MARGIN,
			 width()  - LEFT_MARGIN - RIGHT_MARGIN,
			 height() - TOP_MARGIN	- BOTTOM_MARGIN );
	paintText( &p, size(), frame );
	p.end();
    }
}

/*!
\internal
Paints the line editor in a pixmap and then blts the pixmap onto the screen.
*/

void QLineEdit::pixmapPaint()
{
    pm->fill( backgroundColor() );
    QPainter p;
    p.begin( pm );
    p.setFont( font() );
    paintText( &p, pm->size() , TRUE );
    p.end();
    bitBlt( this, 0, 0, pm, 0, 0, -1, -1 );
}


/*!
\internal
Paints the line editor.
*/

void QLineEdit::paintText( QPainter *p, const QSize &sz, bool frame)
{
    QColorGroup	 g  = colorGroup();
    QFontMetrics fm = fontMetrics();
    char *displayText = &tbuf[offset];

    if ( frame )
	p->drawShadePanel( 0, 0, sz.width(), sz.height(),
			   g.dark(), g.light() );
    p->setClipRect( LEFT_MARGIN, TOP_MARGIN,
		    sz.width()	- LEFT_MARGIN - RIGHT_MARGIN + 1,
		    sz.height() - TOP_MARGIN - BOTTOM_MARGIN + 1 );

    int tDispWidth = sz.width() - LEFT_MARGIN - RIGHT_MARGIN;
    int displayLength = xPosToCursorPos( displayText, fontMetrics(),
					 tDispWidth, tDispWidth );
    if ( displayText[ displayLength ] != '\0' )
	displayLength++;

    p->setPen( g.text() );
    p->drawText( LEFT_MARGIN, sz.height() - BOTTOM_MARGIN - fm.descent(),
		 displayText, displayLength );
    p->setPen( g.foreground() );

    p->setClipping( FALSE );
    if( cursorOn ) {
	uint curPos = LEFT_MARGIN +
		      fm.width( displayText, cursorPos - offset ) - 1;
	if ( style() == MotifStyle ) {
	    if ( !hasFocus() ) {
		p->pen().setStyle( DotLine );
		p->setBackgroundMode( OpaqueMode );
	    }
	    p->drawLine( curPos - 2, TOP_MARGIN, curPos + 2, TOP_MARGIN );
	    p->drawLine( curPos	  , TOP_MARGIN,
			 curPos	  , sz.height() - BOTTOM_MARGIN );
	    p->drawLine( curPos - 2, sz.height() - BOTTOM_MARGIN,
			 curPos + 2, sz.height() - BOTTOM_MARGIN );
	}
	else if ( hasFocus() ) {
	    p->drawLine( curPos, TOP_MARGIN,
			 curPos, sz.height() - BOTTOM_MARGIN );
	}
    }
}


/*!
Moves the cursor leftwards one character.
\sa cursorRight().
*/

void QLineEdit::cursorLeft()
{
    if ( cursorPos != 0 ) {
	killTimers();
	cursorOn = TRUE;
	cursorPos--;
	if ( cursorPos < offset )
	    offset = cursorPos;
	startTimer( blinkTime );
	paint();
    }
}

/*!
Moves the cursor rightwards one character.
\sa cursorLeft().
*/

void QLineEdit::cursorRight()
{
    if ( strlen(tbuf) > cursorPos ) {
	QFontMetrics fm = fontMetrics();
	killTimers();
	cursorOn = TRUE;
	cursorPos++;
	int surplusWidth = width() - LEFT_MARGIN - RIGHT_MARGIN
			   - fm.width( &tbuf[ offset ], cursorPos - offset);
	if ( surplusWidth < 0 ) {
	    while ( surplusWidth < 0 && offset < cursorPos - 1 ) {
		surplusWidth += fm.width( &tbuf[ offset ], 1 );
		offset++;
	    }
	}
	startTimer( blinkTime );
	paint();
    }
}


/*!
Deletes the character on the left side of the text cursor and
moves the cursor one position to the left.
\sa del().
*/

void QLineEdit::backspace()
{
    if ( cursorPos > 0 ) {
	cursorLeft();
	del();
    }
}

/*!
Deletes the character on the right side of the text cursor.
\sa backspace().
*/

void QLineEdit::del()
{
    if ( cursorPos != strlen(tbuf) ) {
	tbuf.remove( cursorPos, 1 );
	paint();
	emit textChanged( tbuf.data() );
    }
}

/*!
Moves the text cursor to the left end of the line.
\sa end()
*/

void QLineEdit::home()
{
    if ( cursorPos != 0 ) {
	killTimers();
	cursorPos = 0;
	offset	  = 0;
	cursorOn = TRUE;
	startTimer( blinkTime );
	paint();
    }
}

/*!
Moves the text cursor to the right end of the line.
\sa home().
*/

void QLineEdit::end()
{
    int tlen = strlen( tbuf );
    if ( cursorPos != tlen ) {
	killTimers();
	offset += showLastPartOffset( &tbuf[offset], fontMetrics(),
				      width() - LEFT_MARGIN - RIGHT_MARGIN );
	cursorPos = tlen;
	cursorOn = TRUE;
	startTimer( blinkTime );
	paint();
    }
}
