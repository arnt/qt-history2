/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlined.cpp#19 $
**
** Implementation of QLineEdit widget class
**
** Author  : Eirik Eng
** Created : 941011
**
** Copyright (c) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlined.h"
#include "qpainter.h"
#include "qfontmet.h"
#include "qpixmap.h"
#include "qkeycode.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlined.cpp#19 $";
#endif

/*!
\class QLineEdit qlined.h

\brief QLineEdit is a simple line editor, suitable e.g. for asking the
user for his name.

The default key bindings are described in keyPressEvent(); they cannot
be customized except by inheriting the class.  When the user clicks on
the text, the cursor will jump to where the text is.

\todo cleaner focus
\todo clipboard, cut, paste
\todo mark and delete */

/*!
\fn void QLineEdit::textChanged( char * )

This signal is emitted every time the text has changed.  The argument
is the new text.
*/

static const int blinkTime = 500;

#define LEFT_MARGIN 4
#define RIGHT_MARGIN 4
#define TOP_MARGIN 4
#define BOTTOM_MARGIN 4


static uint xPosToCursorPos( char *s, const QFontMetrics &fm, 
			     uint xPos, uint width )
{
    char	 *tmp;
    int		  dist;

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

    char	 *tmp = &s[strlen( s ) - 1];

    do {
	width -= fm.width( tmp--, 1 );
    } while ( tmp >=s && width >=0 );

    return width < 0 ? tmp - s + 2 : 0;
}


/*!
Creates a new line editor inside widget \e parent, named \e name.
Both \e parent and name are as usual (in fact both are passed straight
to the QWidget constructor).

This constructor sets the cursor position to the start of the line,
the maximum length to 32767 characters, and the current contents of
the line to "".
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
    inTextFocus = FALSE;
    t		= "";
}

/*! Nohthing to look out for here. */
QLineEdit::~QLineEdit()
{
    delete pm;
}

/*!  Replaces the text currently in the line with \e s.  If necessary,
the line is resized to make space. */
void QLineEdit::setText( const char *s )
{
    if ( t == s )				// no change
	return;
    t = s;
    if ( t.length() > maxLen )
	t.resize( maxLen+1 );
    cursorPos = 0;
    offset    = 0;
    paint();
    emit textChanged( t.data() );
}

/*! Returns a reference to the text currently in the line.  This isn't
guaranteed to be valid for very long - it will tend to be, but may
become corrupted when the line editor exits, or even when the text in
the editor grows beyond a certain limit. */
char *QLineEdit::text() const
{
    return t.data();
}


/*! Set the maximum length of the text in the editor.  If the text is
currently too lone, it is chopped off at the limit. */
void QLineEdit::setMaxLength( int m )
{
    maxLen = (uint) m;
    if ( t.length() > maxLen ) {
	t.resize( maxLen + 1 ); // Include \0
	if ( cursorPos > maxLen ) {
	    offset = maxLen;
	    end();
	}
	paint();
    }
}

/*! Returns the current maximum length of the text in the editor. */
int QLineEdit::maxLength() const
{
    return maxLen;
}

/*! This function contains the guts of the line editor.

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

<strong><a href=mailto:all@troll.no>Comments solicited</a></strong>

All other keys insert themselves into the line.

\todo shift-arrow stuff
*/
void QLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->ascii() >= 32 && e->key() != Key_Delete ) {
	if ( t.length() < maxLen ) {
	    t.insert( cursorPos, e->ascii() );
	    cursorRight();
	    paint();
	    emit textChanged( t.data() );
	}
	return;
    }
    bool p = FALSE;
    switch ( e->key() ) {
	case Key_Left:
		 p = cursorLeft();
		 break;
	case Key_Right:
		 p = cursorRight();
		 break;
	case Key_Backspace:
		 p = backspace();
		 if ( p )
		     emit textChanged( t.data() );
		 break;
	case Key_Home:
		 p = home();
		 break;
	case Key_End:
		 p = end();
		 break;
	case Key_Delete:
		 p = remove();
		 if ( p )
		     emit textChanged( t.data() );
		 break;
	case Key_A:
		 if ( e->state() == ControlButton )
		     p = home();
		 break;
	case Key_B:
		 if ( e->state() == ControlButton )
		     p = cursorLeft();
		 break;
	case Key_D:
		 if ( e->state() == ControlButton ) {
		     p = remove();
		     if ( p )
			 emit textChanged( t.data() );
		 }
		 break;
	case Key_E:
		 if ( e->state() == ControlButton )
		     p = end();
		 break;
	case Key_F:
		 if ( e->state() == ControlButton )
		     p = cursorRight();
		 break;
	case Key_H:
		 if ( e->state() == ControlButton ) {
		     p = backspace();
		     if ( p )
			 emit textChanged( t.data() );
		 }
		 break;
	default:
		 e->ignore();
		 break;
    }
    if ( p )
	paint();
}

/*! This event occurs whenever the mouse enters the line editor.  It
  starts the blink timer, etc. */

void QLineEdit::focusInEvent( QFocusEvent * )
{
    if ( inTextFocus )
        return;
    inTextFocus = TRUE;
//    debug( "IN focus" );
    pm = new QPixmap( width(), height() );
    CHECK_PTR( pm );
    startTimer( blinkTime );
    cursorOn = TRUE;
    paint();
}

/*!  This event occurs whenever the mouse leaves the line editor.  It
  stops the editor's cursor from blinking, etc. */

void QLineEdit::focusOutEvent( QFocusEvent * )
{
    if ( !inTextFocus )
        return;
    inTextFocus = FALSE;
//    debug( "OUT focus" );
    killTimers();
    delete pm;
    pm = 0;
    cursorOn = TRUE;
    paint();
}

/*!  This event occurs whenever the widget needs repainting. */

void QLineEdit::paintEvent( QPaintEvent * )
{
    paint( TRUE );
}


/*!  This event is used to implement the blinking text cursor. */

void QLineEdit::timerEvent( QTimerEvent * )
{
    if ( inTextFocus ) {
	cursorOn = !cursorOn;
	paint();
    }
}

/*!  This even occurs whenever the widget is resized; if necessary it
  will move the cursor, scroll the text and repaint. */
void QLineEdit::resizeEvent( QResizeEvent *e )
{
    if ( inTextFocus ) {
	delete pm;
	pm = new QPixmap( e->size().width(), e->size().height() );
    }
    paint();
}

/*! Handles mouse clicks.

At present, this event handler only moves the text cursor to the mouse
position and accepts the focus.

\todo drag-to-mark, paste
*/

void QLineEdit::mousePressEvent( QMouseEvent *e )
{
    killTimers();
    cursorPos = offset +
	xPosToCursorPos( &t[ offset ], fontMetrics(),
			 e->pos().x() - LEFT_MARGIN,
			 width() - LEFT_MARGIN - RIGHT_MARGIN );
    cursorOn = TRUE;
    startTimer( blinkTime );
    if ( !inTextFocus )
        focusInEvent( 0 );   // will call paint()
    else
        paint();
}

/*! Repaints the line editor as needed.  The line is painted nicely
(using a pixmap buffer to avoid flickering) if the line editor is in
focus, otherwise, a faster direct drawing method is used. */
void QLineEdit::paint( bool frame )
{
    QPainter p;

    if ( inTextFocus ) {
	pixmapPaint();
    } else {
	p.begin( this );
	if ( !frame )
	    p.eraseRect( LEFT_MARGIN, TOP_MARGIN,
			 width()  - LEFT_MARGIN - RIGHT_MARGIN,
			 height() - TOP_MARGIN  - BOTTOM_MARGIN );
	paintText( &p, size(), frame );
	p.end();
    }
}

/*! Paints the line editor slowly, prettily and without flicker */
void QLineEdit::pixmapPaint()
{
    QPainter p;
    p.begin( pm );
    p.setFont( fontRef() );
    p.fillRect( rect(), colorGroup().background() );
    paintText( &p, pm->size() , TRUE );
    p.end();
    bitBlt( this, 0, 0, pm, 0, 0, -1, -1 );
}


/*! Paints the line editor quickly, efficently and without the
slightest thought about flicker or other luxury problems */
void QLineEdit::paintText( QPainter *p, const QSize &sz, bool frame)
{
    QColorGroup  g  = colorGroup();
    QFontMetrics fm = fontMetrics();
    char *displayText = &t[ offset ];

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
	    if ( !inTextFocus ) {
		p->pen().setStyle( DotLine );
		p->setBackgroundMode( OpaqueMode );
	    }
	    p->drawLine( curPos - 2, TOP_MARGIN, curPos + 2, TOP_MARGIN );
	    p->drawLine( curPos	  , TOP_MARGIN,
			 curPos	  , sz.height() - BOTTOM_MARGIN );
	    p->drawLine( curPos - 2, sz.height() - BOTTOM_MARGIN,
			 curPos + 2, sz.height() - BOTTOM_MARGIN );
	} else {
	    p->drawLine( curPos	  , TOP_MARGIN,
			 curPos	  , sz.height() - BOTTOM_MARGIN );
	}
    }
}


/*! Moves the cursor leftwards one character, restarts the blink
  timer, and returns TRUE.

  Or finds that it can't move the cursor leftwards, and returns FALSE. */

bool QLineEdit::cursorLeft()
{
    if ( cursorPos != 0 ) {
	killTimers();
	cursorOn = TRUE;
	cursorPos--;
	if ( cursorPos < offset )
	    offset = cursorPos;
	startTimer( blinkTime );
	return TRUE;
    }
    return FALSE;
}

  /*! Moves the cursor rightwards one character, restarts the blink
  timer, and returns TRUE.

  Or finds that it can't move the cursor rightwards, and returns FALSE.
  */


bool QLineEdit::cursorRight()
{
    QFontMetrics fm = fontMetrics();

    if ( strlen( t ) > cursorPos ) {
	killTimers();
	cursorOn = TRUE;
	cursorPos++;
	int surplusWidth = width() - LEFT_MARGIN - RIGHT_MARGIN
			   - fm.width( &t[ offset ], cursorPos - offset);
	if ( surplusWidth < 0 ) {
	    while ( surplusWidth < 0 && offset < cursorPos - 1 ) {
		surplusWidth += fm.width( &t[ offset ], 1 );
		offset++;
	    }
	}
	startTimer( blinkTime );
	return TRUE;
    }
    return FALSE;
}


/*! If it can delete leftwards, does that and returns TRUE, othewise
  it returns FALSE */
bool QLineEdit::backspace()
{
    return cursorLeft() ? remove() : FALSE;
}


/*! If it can delete rightwards, does that and returns TRUE, othewise
  it returns FALSE */
bool QLineEdit::remove()
{
    if ( cursorPos != strlen(t) ) {
	t.remove( cursorPos, 1 );
	return TRUE;
    }
    return FALSE;
}

/*! Moves the text cursor to the left end of the line, restarts the
  blink timer, and returns TRUE.  Or, if the cursor was already there,
  returns FALSE. */

bool QLineEdit::home()
{
    if ( cursorPos != 0 ) {
	killTimers();
	cursorPos = 0;
	offset	  = 0;
	cursorOn = TRUE;
	startTimer( blinkTime );
	return TRUE;
    }
    return FALSE;
}


/*! Moves the text cursor to the right end of the line, restarts the
  blink timer, and returns TRUE.  Or, if the cursor was already there,
  returns FALSE. */

bool QLineEdit::end()
{
    if ( cursorPos != strlen(t) ) {
	killTimers();
	offset += showLastPartOffset( &t[offset], fontMetrics(),
		      width() - LEFT_MARGIN - RIGHT_MARGIN );
	cursorPos = strlen( t );
	cursorOn = TRUE;
	startTimer( blinkTime );
	return TRUE;
    }
    return FALSE;
}

