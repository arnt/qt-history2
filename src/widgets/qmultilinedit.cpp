/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmultilinedit.cpp#21 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qmlined.h"

#include "qpainter.h"
#include "qscrbar.h"
#include "qkeycode.h"
#include "qclipbrd.h"
#include "qpixmap.h"
#include "qapp.h"

#include <ctype.h>

/*! \class QMultiLineEdit qmlined.h

  \brief The QMultiLineEdit widget is a simple editor for inputting text.

  \ingroup realwidgets

  The QMultiLineEdit widget provides multiple line text input and display.
  It is intended for moderate amounts of text. There are no arbitrary
  limitations, but if you try to handle megabytes of data, performance
  will suffer.

  This widget can be used to display text by calling setReadOnly(TRUE)

  The default key bindings are described in keyPressEvent(); they cannot
  be customized except by inheriting the class.
 */

static const int BORDER = 3;

static const int blinkTime  = 500;		// text cursor blink time
static const int scrollTime = 100;		// mark text scroll time

static int xPosToCursorPos( const char *s, const QFontMetrics &fm,
			    int xPos, int width )
{
    const char *tmp;
    int	  dist;
    if ( !s )
	return 0;
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

/*!
  Creates a new, empty, QMultiLineEdit.
*/

QMultiLineEdit::QMultiLineEdit( QWidget *parent , const char *name )
    :QTableView( parent, name)
{
    QFontMetrics fm = fontMetrics();
    setCellHeight( fm.lineSpacing() + 1 );
    setNumCols( 1 );

    setNumRows( 0 ); 
    setCellWidth( 1 ); // ### constant width
    contents = new QList<QString>;
    contents->setAutoDelete( TRUE );

    cursorX = 0; cursorY = 0;
    curXPos = 0;

    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   //Tbl_cutCellsV | //////###########
		   //Tbl_scrollLastVCell  |
		   Tbl_smoothVScrolling |
		   Tbl_clipCellPainting
		   );
    switch ( style() ) {
	case WindowsStyle:
	case MotifStyle:
	    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	    setBackgroundColor( colorGroup().base() );
	    break;
	default:
	    setFrameStyle( QFrame::Panel | QFrame::Plain );
	    setLineWidth( 1 );
    }
    readOnly = FALSE;
    setAcceptFocus( TRUE );
    setCursor( ibeamCursor );
    ((QScrollBar*)verticalScrollBar())->setCursor( arrowCursor );
    ((QScrollBar*)horizontalScrollBar())->setCursor( arrowCursor );
    insert( "", -1 );
    dummy          = TRUE;
    dragScrolling  = FALSE;
    dragMarking    = FALSE;
    markIsOn	   = FALSE;
    markAnchorX    = 0;
    markAnchorY    = 0;
    markDragX      = 0;
    markDragY      = 0;
    blinkTimer     = 0;
}

/*! \fn int QMultiLineEdit::numLines()

  Returns the number of lines in the editor.
*/

/*! \fn bool QMultiLineEdit::atEnd() const

  Returns TRUE if the cursor is placed at the end of the text.
*/

/*! \fn bool QMultiLineEdit::atBeginning() const

  Returns TRUE if the cursor is placed at the beginning of the text.
*/


/*!
  \fn int QMultiLineEdit::lineLength( int row ) const
  Returns the number of characters at line number \a row.
*/

/*! \fn QString *QMultiLineEdit::getString( int row ) const

  Returns a pointer to the text at line \a row.
*/

/*! \fn void QMultiLineEdit::textChanged()

  This signal should be emitted when the text is changed (not implemented)
*/


/*! \fn bool QMultiLineEdit::isReadOnly()

  Returns FALSE if this multi line edit accepts text input. 
  Scrolling and cursor movements are accepted in any case.

  \sa setReadOnly() QWidget::isEnabled()
*/


/*!
  If \a on is FALSE, this multi line edit accepts text input.
  Scrolling and cursor movements are accepted in any case.

  \sa inputEnabled() QWidget::setEnabled() 
*/

void QMultiLineEdit::setReadOnly( bool on ) 
{ 
    if ( readOnly != on ) {
	readOnly = on; 
	setCursor( on ? arrowCursor : ibeamCursor );
    }
}

/*!
  Destroys the QMultiLineEdit
*/

QMultiLineEdit::~QMultiLineEdit()
{
    
}

const int nBuffers = 3;
static QPixmap *buffer[nBuffers] = { 0, 0, 0 };   // ### delete ved avslutning
static int freeNext = 0;

static void cleanupMLBuffers()
{
    for( int i = 0 ; i < nBuffers ; i++ ) {
	delete buffer[i];
	buffer[i] = 0;
    }
}

static QPixmap *getCacheBuffer( QSize sz )
{
    static bool firstTime = TRUE;
    if ( firstTime ) {
	firstTime = FALSE;
	qAddPostRoutine( cleanupMLBuffers );
    }

    for( int i = 0 ; i < nBuffers ; i++ ) {
	if ( buffer[i] ) {
	    if ( buffer[i]->size() == sz )
		return buffer[i];
	} else {
	    return buffer[i] = new QPixmap( sz );
	}
    }
    if ( ++freeNext == 3 )
	freeNext = 0;
    buffer[freeNext]->resize( sz );
    return buffer[freeNext];
}

/*!
  Implements the basic drawing logic.
*/
void QMultiLineEdit::paintCell( QPainter *painter, int row, int )
{
    QColorGroup	 g    = colorGroup();
    QFontMetrics fm = painter->fontMetrics();
    QString *s = contents->at( row );
    if ( !s ) {
	warning( "QMultiLineEdit::paintCell, no text at line %d", row );
	return;
    }
    QRect updateR   = cellUpdateRect();
    QPixmap *buffer = getCacheBuffer( updateR.size() );
    ASSERT(buffer);
    buffer->fill ( g.base() );

    QPainter p;
    p.begin( buffer );
    p.setFont( painter->font() );
    p.translate( -updateR.left(), -updateR.top() );

    int yPos = fm.ascent() + fm.leading()/2 - 1;
    bool hasMark = FALSE;
    int markX1, markX2;				// in x-coordinate pixels
    markX1 = markX2 = 0;			// avoid gcc warning
    if ( markIsOn ) {
	int markBeginX, markBeginY;
	int markEndX, markEndY;
	if ( markAnchorY < markDragY ) {
	    markBeginX = markAnchorX;
	    markBeginY = markAnchorY;
	    markEndX   = markDragX;
	    markEndY   = markDragY;
	} else {
	    markBeginX = markDragX;
	    markBeginY = markDragY;
	    markEndX   = markAnchorX;
	    markEndY   = markAnchorY;
	}
	if ( markAnchorY == markDragY && markBeginX > markEndX ) {
	    int tmp    = markBeginX;
	    markBeginX = markEndX;
	    markEndX   = tmp;
	}
	if ( row >= markBeginY && row <= markEndY ) {
	    hasMark = TRUE;
	    if ( row == markBeginY ) {
		markX1 = markBeginX;
		if ( row == markEndY ) 		// both marks on same row
		    markX2 = markEndX;
		else
		    markX2 = s->length();	// mark till end of line
	    } else {
		if ( row == markEndY ) {
		    markX1 = 0;
		    markX2 = markEndX;
		} else {
		    markX1 = 0;			// whole line is marked
		    markX2 = s->length();	// whole line is marked
		}
	    }
	}
    }
    if ( !hasMark ) {
	p.setPen( g.text() );
	p.drawText( BORDER,  yPos , *s );
    } else {
	int sLength = s->length();
	int xpos1, xpos2;
	if ( markX1 != markX2 ) {
	    	xpos1 =  BORDER + fm.width( s->data(), markX1 );
		xpos2 =  xpos1 + fm.width( s->data() + markX1, 
					       markX2 - markX1 ) - 1;
		int fillxpos1 = xpos1;
		int fillxpos2 = xpos2;
		if ( markX1 == 0 )
		    fillxpos1 -= 2;
		if ( markX2 == sLength )
		    fillxpos2 += 3;
		p.fillRect( fillxpos1, 0, fillxpos2 - fillxpos1, 
			    cellHeight(row), g.text() );
		p.setPen( g.base() );
		p.drawText( xpos1, yPos, s->data() + markX1, markX2 - markX1);
	}
	p.setPen( g.text() );
	if ( markX1 != 0 )
	    p.drawText( BORDER, yPos, *s, markX1 );
	if ( markX2 != (int)s->length() )
	    p.drawText( BORDER + fm.width( *s, markX2 ), yPos,  // ### length
			 s->data() + markX2, s->length() - markX2 );
    }

    if ( row == cursorY && cursorOn && !readOnly ) {
	int cursorPos = QMIN( (int)s->length(), cursorX );
	int curXPos   = BORDER +
			fm.width( *s, cursorPos ) - 1;
	int curYPos   = 0;
	if ( hasFocus() ) {
	    p.drawLine( curXPos - 2, curYPos,
			 curXPos + 2, curYPos );
	    p.drawLine( curXPos    , curYPos,
			 curXPos    , curYPos + fm.height() - 2);
	    p.drawLine( curXPos - 2, curYPos + fm.height() - 2,
			 curXPos + 2, curYPos + fm.height() - 2);
	}
    }
    p.end();
    painter->drawPixmap( updateR.left(), updateR.top(), *buffer );
}


/*!
  Returns the width in pixels of the string \a s.
*/

int QMultiLineEdit::textWidth( QString *s )
{
    int w = fontMetrics().width( *s );
    return w + 2 * BORDER;
}


/*!
  Returns the width in pixels of the text at line \a row.
*/

int QMultiLineEdit::textWidth( int row )
{
    //possibilities of caching...
    QString *s = contents->at( row );
    if ( !s ) {
	warning( 
	  "QMultiLineEdit::textWidth: Couldn't find contents at row %d", row );
	return 0;
    }
    return textWidth( s );
}


/*!
  Starts the cursor blinking.
*/

void QMultiLineEdit::focusInEvent( QFocusEvent * )
{
    //debug( "startTimer" );
    //killTimers();
    blinkTimer = startTimer( blinkTime );
    cursorOn = TRUE;
    updateCell( cursorY, 0, FALSE );
}


/*!
  stops the cursor blinking.
*/

void QMultiLineEdit::focusOutEvent( QFocusEvent * )
{
    killTimer( blinkTimer );
    blinkTimer = 0;
    if ( cursorOn )
	updateCell( cursorY, 0, FALSE );
}


/*!
  Cursor blinking
*/

void QMultiLineEdit::timerEvent( QTimerEvent *t )
{
    if ( hasFocus() && t->timerId() == blinkTimer ) {
	/*
	if ( dragScrolling ) {
	    if ( scrollingLeft )
		cursorLeft( TRUE );	// mark left
	    else
		cursorRight( TRUE );	// mark right
	} else */{
	    cursorOn = !cursorOn;
	    updateCell( cursorY, 0, FALSE );
	}
    }
}


/*!
  Returns TRUE if there is marked text.
*/

bool QMultiLineEdit::hasMarkedText() const
{
    return markIsOn;
}


/*!
  Returns a copy of the marked text.
*/

QString QMultiLineEdit::markedText() const
{
    if ( !markIsOn )
	return QString();
    if ( markAnchorY == markDragY ) { //just one line
	int minMark = markDragX < markAnchorX ? markDragX : markAnchorX;
	int maxMark = markDragX > markAnchorX ? markDragX : markAnchorX;
	QString *s  = getString( markAnchorY );
	ASSERT(s);
	return s->mid( minMark, maxMark - minMark );
    } else { //multiline
	int markBeginX, markBeginY;
	int markEndX, markEndY;
	if ( markAnchorY < markDragY ) {
	    markBeginX = markAnchorX;
	    markBeginY = markAnchorY;
	    markEndX   = markDragX;
	    markEndY   = markDragY;
	} else {
	    markBeginX = markDragX;
	    markBeginY = markDragY;
	    markEndX   = markAnchorX;
	    markEndY   = markAnchorY;
	}
	
	ASSERT( markBeginY >= 0);
	ASSERT( markEndY < (int)contents->count() );
	
	QString tmp;
	QString *firstS, *lastS;
	firstS = getString( markBeginY );
	lastS  = getString( markEndY );
	ASSERT( firstS != lastS );

	tmp = firstS->mid( markBeginX, firstS->length() - markBeginX  );

	for( int i = markBeginY + 1 ; i < markEndY ; i++ ) {
	    tmp += "\n";
	    tmp += *(contents->at( i ));
	}

	if ( markEndX != 0 ) {
	    tmp += "\n";
	    tmp += lastS->left( markEndX  );
	}
	return tmp;
    }
}


/*!
  Returns the text at line number \a row, or 0 if row is invalid.
*/

const char * QMultiLineEdit::textLine( int line ) const
{
    QString *s = contents->at( line );
    if ( s )
	return *s;
    else
	return 0;
}


/*!
  Returns a copy of the whole text. If the multi line edit contains no
  text, the empty string is returned.
*/

QString QMultiLineEdit::text() const
{
    if ( contents->count() == 0 )
	return QString( "" );

    QString tmp = *(contents->at( 0 ));
    for( int i = 1 ; i < (int)contents->count() ; i++ ) {
	tmp += "\n";
	tmp += *(contents->at( i ));
    }
    return tmp;
}


/*!
  Selects all text (unimplemented)
*/

void QMultiLineEdit::selectAll()
{
    
}

/*!
  Sets the text to \a s, removing old text, if any.
*/

void QMultiLineEdit::setText( const char *s )
{
    clear();
    insert( s, -1 );
    emit textChanged();
}


/*!
  Appends \a s to the text.
*/

void QMultiLineEdit::append( const char *s )
{
    insert( s, -1 );
    emit textChanged();
}


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
  <li><i> Control-K </i> Delete to end of line
  <li><i> Control-N </i> Move the cursor one line downwards
  <li><i> Control-P </i> Move the cursor one line upwards
  <li><i> Control-V </i> Paste the clipboard text into line edit.
  <li><i> Control-X </i> Cut the marked text, copy to clipboard.
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
  </ul>

*/

void QMultiLineEdit::keyPressEvent( QKeyEvent *e ) 
{
    textDirty = FALSE;
    int unknown = 0;
    if ( readOnly ) {
	int pageSize = viewHeight() / cellHeight();

	switch ( e->key() ) {
	case Key_Left:
	    setXOffset( xOffset() - viewWidth()/10 );
	    break;
	case Key_Right:
	    setXOffset( xOffset() + viewWidth()/10 );
	    break;
	case Key_Up:
	    setTopCell( topCell() - 1 );
	    break;
	case Key_Down:
	    setTopCell( topCell() + 1 );
	    break;
	case Key_Next:
	    setTopCell( topCell() + pageSize );
	    break;
	case Key_Prior:
	    setTopCell( QMAX( topCell() - pageSize, 0 ) );
	    break;
	default:
	    unknown++;
	}
	if ( unknown ) 
	    e->ignore();
	return;
    }
    if ( e->ascii() >= 32 && e->key() != Key_Delete ) {
	insertChar( e->ascii() );
	if ( textDirty )
	    emit textChanged();
	return;
    }
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
	    if ( hasMarkedText() ) {
		copyText();
	    }
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
	case Key_K:
	    killLine();
	    break;
	case Key_N:
	    cursorDown( e->state() & ShiftButton );
	    break;
	case Key_P:
	    cursorUp( e->state() & ShiftButton );
	    break;
	case Key_V:
	    paste();
	    break;
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
	case Key_Up:
	    cursorUp( e->state() & ShiftButton );
	    break;
	case Key_Down:
	    cursorDown( e->state() & ShiftButton );
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
	case Key_Next:
	    pageDown( e->state() & ShiftButton );
	    break;
	case Key_Prior:
	    pageUp( e->state() & ShiftButton );
	    break;
	case Key_Enter:
	case Key_Return:
	    newLine();
	    break;
	default:
	    unknown++;
	}
    }
    if ( textDirty )
	emit textChanged();

    if ( unknown ) {				// unknown key
	e->ignore();
	return;
    }
}


/*!
  Moves the cursor one page up.
*/

void QMultiLineEdit::pageDown( bool mark )
{ 
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    int newTopCell = QMIN( topCell() + pageSize, numLines() - 1 - pageSize );

    if ( pageSize > numLines() ) { // quick hack to handle small texts
	newTopCell = topCell();
    }
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;

    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    if ( newTopCell != topCell() ) {
	cursorY = newTopCell + delta;
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY );
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMIN( cursorY + pageSize, numLines() - 1);
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY );
	makeVisible();
    }
    if ( mark )
	repaint( FALSE );
    else
	updateCell( oldY, 0, FALSE );
}


/*!
  Moves the cursor one page down.
*/

void QMultiLineEdit::pageUp( bool mark )
{
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    bool partial = delta == pageSize && viewHeight() != pageSize * cellHeight();
    int newTopCell = QMAX( topCell() - pageSize, 0 );
    if ( pageSize > numLines() ) { // quick hack to handle small texts
	newTopCell = 0;
	delta = 0;
    }
    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;
    if ( newTopCell != topCell() ) {
	cursorY = newTopCell + delta;
	if ( partial )
	    cursorY--;
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY );
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMAX( cursorY - pageSize, 0 );
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY );
    }
    if ( mark )
	repaint( FALSE );
    else
	updateCell( oldY, 0, FALSE );
}

/*!
  Inserts \a s at line number \a row. If \a row is less than zero, or
  larger than the number of rows, the new text is put at the end.
  If \a s contains newline characters, several lines are inserted.
*/

void QMultiLineEdit::insert( const char *txt, int line )
{
    if ( dummy && numLines() == 1 && getString( 0 )->isEmpty() ) {
	contents->remove( (uint)0 );
	//debug ("insert: removing dummy, %d", count() );
	dummy = FALSE;
    }

    QString s;
    if ( txt )
	s.setRawData( txt, strlen(txt) + 1 );
    else
	s = "";

    int to = s.find( '\n' );
    if ( to < 0 ) { //just one line
	QString *txtCopy = new QString( txt );
	if ( line < 0 || !contents->insert( line, txtCopy ) )
	    contents->append( txtCopy );
	bool updt = autoUpdate() && rowIsVisible( line );
	int w = textWidth( txtCopy );
	setWidth( QMAX( cellWidth(), w ) );
	setNumRows( contents->count() );

	if ( updt )
	    repaint( FALSE );
    } else { //multiline
	int from = 0;
	if ( line < 0 || line >= numLines() )
	    line = numLines();
	while ( to > 0 ) {
	    insert( s.mid( from, to - from ), line++ );
	    from = to + 1;
	    to = s.find( '\n', from );
	}
	int lastLen = s.length() - from;
	insert( s.right( lastLen ), line );
	setNumRows( contents->count() );
	updateCellWidth();
    }
    textDirty = TRUE;
    ASSERT( numLines() != 0 );
    makeVisible();
    if ( txt )
	s.resetRawData( txt, strlen(txt) + 1 );
}

/*!
  Deletes the line at line number \a row. If \a
  row is less than zero, or larger than the number of rows, 
  no line is deleted.
*/

void QMultiLineEdit::remove( int row )
{
    if ( row >= numLines()  )
	return;
    if ( cursorY >= row && cursorY > 0 )
	cursorY--;
    bool updt = autoUpdate() && rowIsVisible( row );
    bool recalc = textWidth( row ) == cellWidth();
    contents->remove( row );
    if ( contents->count() == 0 ) {
	//debug( "remove: last one gone, inserting dummy" );
	insert( "", -1 );
	dummy = TRUE;
    }
    setNumRows( contents->count() );
    if ( recalc )
	updateCellWidth();
    makeVisible();
    bool clear = numLines() < viewHeight() / cellHeight() || recalc;
    if ( updt )
	repaint( clear );
    textDirty = TRUE;
}

/*!
  Inserts \a c at the current cursor position.
*/

void QMultiLineEdit::insertChar( char c )
{
    dummy = FALSE;
    if ( hasMarkedText() ) {
	del();					// ## Will flicker
    }
   
    QString *s = getString( cursorY );
    if ( cursorX > (int)s->length() )
	cursorX = s->length();
    s->insert( cursorX, c);
    int w = textWidth( s );
    setWidth( QMAX( cellWidth(), w ) );
    cursorRight( FALSE );			// will repaint
    curXPos  = 0;
    makeVisible();
    textDirty = TRUE;
}

/*!
  Makes a line break at the current cursor position.
*/

void QMultiLineEdit::newLine()
{
    dummy = FALSE;
    QString *s = getString( cursorY );
    bool recalc = cursorX != (int)s->length() && textWidth( s ) == cellWidth();
    QString newString = s->mid( cursorX, s->length() );
    s->remove( cursorX, s->length() );
    insert( newString, cursorY + 1 );
    cursorRight( FALSE );
    curXPos  = 0;
    if ( recalc )
	updateCellWidth();
    makeVisible();
    turnMarkOff();
    textDirty = TRUE;
}

/*!
  Deletes text from the current cursor position to the end of the line.
*/

void QMultiLineEdit::killLine()
{
    QString *s = getString( cursorY );
    if ( cursorX == (int)s->length() ) {
	del();
	return;
    } else {    
	bool recalc = textWidth( s ) == cellWidth();
	s->remove( cursorX, s->length() );
	updateCell( cursorY, 0, TRUE ); //Quick fix; whole line needs update
	if ( recalc )
	    updateCellWidth();
	textDirty = TRUE;
    }
    curXPos  = 0;
    makeVisible();
    turnMarkOff();
}

/*!
  Moves the cursor leftwards one or more characters.
  \sa cursorRight()
*/

void QMultiLineEdit::cursorLeft( bool mark, int steps )
{
    if ( steps < 0 ) {
	cursorRight( mark, -steps );
	return;
    }
    if ( cursorX != 0 || cursorY != 0 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	int ll = lineLength( cursorY );
	if ( cursorX > ll )
	    cursorX = ll;
	cursorOn = TRUE;
	cursorX -= steps;
	if ( cursorX < 0 ) {
	    int oldY = cursorY;
	    if ( cursorY > 0 ) {
		cursorY--;
		cursorX = lineLength( cursorY );
	    } else {
		cursorY = 0; //### ?
		cursorX = 0;
	    }
	    updateCell( oldY, 0, FALSE );
	}
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	blinkTimer = startTimer( blinkTime );
	updateCell( cursorY, 0, FALSE );
    }
    curXPos  = 0;
    makeVisible();
    if ( !mark )
	turnMarkOff();
}

/*!
  Moves the cursor rightwards one or more characters.
  \sa cursorLeft()
*/

void QMultiLineEdit::cursorRight( bool mark, int steps )
{

    if ( steps != 1 ) {
	warning( "cursorRight %d steps", steps );
    }
    if ( steps < 0 ) {
	cursorLeft( mark, -steps );
	return;
    }
    int strl = lineLength( cursorY );

    if ( cursorX < strl || cursorY < (int)contents->count() - 1 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	cursorOn = TRUE;
	cursorX += steps;
	if ( cursorX > strl ) {
	    int oldY = cursorY;
	    if ( cursorY < (int) contents->count() - 1 ) {
		cursorY++;
		cursorX = 0;
	    } else {
		cursorX = lineLength( cursorY );
	    }
	    updateCell( oldY, 0, FALSE );
	}
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer( blinkTime );
    }
    curXPos  = 0;
    makeVisible();
    if ( !mark )
	turnMarkOff();
}

/*!
  Moves the cursor upwards one or more characters.
  \sa cursorDown() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorUp( bool mark, int steps )
{
    if ( steps != 1 ) {
	warning( "cursorUp %d steps", steps );
    }
    if ( steps < 0 ) {
	cursorDown( mark, -steps );
	return;
    }

    if ( cursorY != 0 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	killTimer( blinkTimer );
	cursorOn = TRUE;
	cursorY -= steps;
	if ( cursorY < 0 ) {
	    cursorY = 0;
	}
        cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( oldY, 0, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer( blinkTime );
    }
    makeVisible();
    if ( !mark )
	turnMarkOff();
}

/*!
  Moves the cursor downwards one or more characters.
  \sa cursorDown() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorDown( bool mark, int steps )
{
    if ( steps != 1 ) {
	warning( "cursorDown %d steps", steps );
    }
    if ( steps < 0 ) {
	cursorUp( mark, -steps );
	return;
    }
    int lastLin = contents->count() - 1;
    if ( cursorY != lastLin ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	killTimer( blinkTimer );
	cursorOn = TRUE;
	cursorY += steps;
	if ( cursorY > lastLin ) {
	    cursorY = lastLin;
	}
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
        cursorX = mapFromView( curXPos, cursorY );
	updateCell( oldY, 0, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer( blinkTime );
    }
    makeVisible();
    if ( !mark )
	turnMarkOff();
}

/*!
  Turns off marked text
*/
void QMultiLineEdit::turnMarkOff()
{
    if ( markIsOn ) {
	markIsOn = FALSE;
	repaint( FALSE );
    }
}




/*!
  Deletes the character on the left side of the text cursor and moves
  the cursor one position to the left. If a text has been marked by
  the user (e.g. by clicking and dragging) the cursor is put at the
  beginning of the marked text and the marked text is removed.
  \sa del()
*/

void QMultiLineEdit::backspace()
{
    if ( hasMarkedText() ) {
	del();
    } else {
	if ( !atBeginning() ) {
	    cursorLeft( FALSE );
	    del();
	}
    }
    makeVisible();
}

/*!
  Deletes the character on the right side of the text cursor. If a
  text has been marked by the user (e.g. by clicking and dragging) the
  cursor is put at the beginning of the marked text and the marked
  text is removed.  \sa backspace()
*/

void QMultiLineEdit::del()
{
    if ( hasMarkedText() ) {
	textDirty = TRUE;
	setAutoUpdate( FALSE );
	if ( markAnchorY == markDragY ) { //just one line
	    int minMark = markDragX < markAnchorX ? markDragX : markAnchorX;
	    int maxMark = markDragX > markAnchorX ? markDragX : markAnchorX;
	    QString *s  = getString( markAnchorY );
	    ASSERT(s);
	    s->remove( minMark, maxMark - minMark );
	    cursorX  = minMark;
	    cursorY  = markAnchorY;
	    markIsOn    = FALSE;
	    updateCellWidth();
	} else { //multiline
	    int markBeginX, markBeginY;
	    int markEndX, markEndY;
	    if ( markAnchorY < markDragY ) {
		markBeginX = markAnchorX;
		markBeginY = markAnchorY;
		markEndX   = markDragX;
		markEndY   = markDragY;
	    } else {
		markBeginX = markDragX;
		markBeginY = markDragY;
		markEndX   = markAnchorX;
		markEndY   = markAnchorY;
	    }

	    ASSERT( markBeginY >= 0);
	    ASSERT( markEndY < (int)contents->count() );

	    QString *firstS, *lastS;
	    firstS = getString( markBeginY );
	    lastS  = getString( markEndY );
	    ASSERT( firstS != lastS );
	    firstS->remove( markBeginX, firstS->length() - markBeginX  );
	    lastS->remove( 0, markEndX  );
	    firstS->append( *lastS );  // lastS will be removed in loop below

	    for( int i = markBeginY + 1 ; i <= markEndY ; i++ )
		contents->remove( markBeginY + 1 );
	    markIsOn = FALSE;
	    if ( contents->isEmpty() )
		insert( "", -1 );

	    cursorX  = markBeginX;
	    cursorY  = markBeginY;
	    curXPos  = 0;

	    setNumRows( contents->count() );
	}
	updateCellWidth();
	setAutoUpdate( TRUE );
	repaint();
    } else {
	if ( !atEnd() ) {
	    textDirty = TRUE;
	    QString *s = getString( cursorY );
	    if ( cursorX == (int) s->length() ) { // remove newline
		*s += *getString( cursorY + 1 );
		int w = textWidth( s );
		setWidth( QMAX( cellWidth(), w ) );
		remove( cursorY + 1 );
	    } else {
		bool recalc = textWidth( s ) == cellWidth();
		s->remove( cursorX, 1 );
		updateCell( cursorY, 0, FALSE );
		if ( recalc )
		    updateCellWidth();
	    }
	}
    }
    curXPos  = 0;
    makeVisible();
}

/*!
  Moves the text cursor to the left end of the line. If \a mark is
  TRUE, text is marked towards the first position. If it is FALSE and
  the cursor is moved, all marked text is unmarked.
  
  \sa end() 
*/

void QMultiLineEdit::home( bool mark )
{
    if ( cursorX != 0 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	cursorX = 0;
	cursorOn = TRUE;
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer( blinkTime );
    }
    curXPos  = 0;
    if ( !mark )
	turnMarkOff();
    makeVisible();
}

/*!
  Moves the text cursor to the right end of the line. If mark is TRUE
  text is marked towards the last position.  If it is FALSE and the
  cursor is moved, all marked text is unmarked.

  \sa home()
*/

void QMultiLineEdit::end( bool mark ) 
{
    int tlen = lineLength( cursorY );
    if ( cursorX != tlen ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	cursorX = tlen;
	cursorOn  = TRUE;
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	blinkTimer = startTimer( blinkTime );
	updateCell( cursorY, 0, FALSE );
    }
    curXPos  = 0;
    makeVisible();
    if ( !mark )
	turnMarkOff();
}

/*!
  Handles mouse press events.
*/

void QMultiLineEdit::mousePressEvent( QMouseEvent *m )
{
    textDirty = FALSE;
    if ( readOnly )
	return;
    int newY = findRow( m->pos().y() );
    if ( newY < 0 )
	newY = lastRowVisible();
    newY = QMIN( (int)contents->count() - 1, newY );
    cursorX = xPosToCursorPos( *getString( newY ), fontMetrics(),
			       m->pos().x() - BORDER + xOffset(),
			       cellWidth() - 2 * BORDER );
    if ( m->button() ==  LeftButton ) {
	dragMarking    = TRUE;
	curXPos        = 0;
	markAnchorX    = cursorX;
	markAnchorY    = newY;
	bool markWasOn = markIsOn;
	markIsOn       = FALSE;
	if ( markWasOn ) {
	    cursorY = newY;
	    repaint( FALSE );
	    return;
	}	
    }

    if ( m->button() ==  MidButton || m->button() ==  LeftButton) {
	if ( cursorY != newY ) {
	    int oldY = cursorY;
	    cursorY = newY;
	    updateCell( oldY, 0, FALSE );
	}
	updateCell( cursorY, 0, FALSE );		// ###
    }
    if ( m->button() ==  MidButton )
	paste();		// Will repaint the cursor line.
    if ( textDirty )
	emit textChanged();
}

/*!
  Handles mouse move events.
*/
void QMultiLineEdit::mouseMoveEvent( QMouseEvent *e )
{
    if ( readOnly || !dragMarking )
	return;
    if ( rect().contains( e->pos() ) ) {
	int newY = findRow( e->pos().y() );
	if ( newY < 0 ) {
	    if ( e->pos().y() < 0 )
		newY = topCell();
	    else
		newY = lastRowVisible();
	}
	newY = QMIN( (int)contents->count() - 1, newY );
	int newX = xPosToCursorPos( *getString( newY ), fontMetrics(),
				   e->pos().x() - BORDER + xOffset(),
				   cellWidth() - 2 * BORDER );
	newMark( newX, newY, FALSE );
	repaint( FALSE ); //###
    }
}


/*!
  Handles mouse release events.
*/
void QMultiLineEdit::mouseReleaseEvent( QMouseEvent * )
{
    dragScrolling = FALSE;
    dragMarking   = FALSE;
    if ( markAnchorY == markDragY && markAnchorX == markDragX )
	markIsOn = FALSE;
    else
	copyText();
}


/*!
  Handles double click events.
*/

void QMultiLineEdit::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );
}


/*!
  Returns TRUE if line \a row is invisible or partially invisible.
*/

bool QMultiLineEdit::partiallyInvisible( int row )
{
    int y;
    if ( !rowYPos( row, &y ) )
	return TRUE;
    if ( y < 0 ) {
	//debug( "row %d occluded at top", row );
	return TRUE;
    } else if ( y + cellHeight() - 2 > viewHeight() ) {
	//debug( "row %d occluded at bottom", row );
	return TRUE;
    }
    return FALSE;
}

/*!
  Scrolls such that the cursor is visible
*/

void QMultiLineEdit::makeVisible()
{
    if ( partiallyInvisible( cursorY ) ) {
	if ( cursorY >= lastRowVisible() )
	    setBottomCell( cursorY );
	else
	    setTopCell( cursorY );
    }
    int xPos = mapToView( cursorX, cursorY );
    //debug( "xpos %d, offset %d, width %d", xPos, xOffset(), viewWidth() );
    if ( xPos < xOffset() ) {
	int of = xPos - 10; //###
	//debug( "left: new offset = %d", of );
	setXOffset( of ); 
    } else if ( xPos > xOffset() + viewWidth() ) {
	int of = xPos - viewWidth() + 10; //###
	//debug( "right: new offset = %d", of );
	setXOffset( of );
    }
}

/*!
  Computes the character position in line \a row which corresponds 
  to pixel \a xPos
*/

int QMultiLineEdit::mapFromView( int xPos, int row )
{
    QString *s = getString( row );
    if ( !s )
	return 0;
    int index = xPosToCursorPos( *s, fontMetrics(),
				 xPos - BORDER,
				 cellWidth() - 2 * BORDER );
    return index;
}

/*!
  Computes the pixel position in line \a row which corresponds to 
  character position \a xIndex
*/

int QMultiLineEdit::mapToView( int xIndex, int row )
{
    QString *s = getString( row );
    xIndex = QMIN( (int)s->length(), xIndex );
    int curXPos   = BORDER +
		    fontMetrics().width( *s, xIndex ) - 1;
    return curXPos;
}

/*!
  Traverses the list and finds an item with the maximum width, and
  updates the internal list box structures accordingly.
*/

void QMultiLineEdit::updateCellWidth()
{
    QString *s = contents->first();
    QFontMetrics fm = fontMetrics();
    int maxW = 0;
    int w;
    while ( s ) {
	w = fontMetrics().width( *s ) + 2 * BORDER; 
	if ( w > maxW )
	    maxW = w;
	s = contents->next();
    }
    setWidth( maxW );
}


/*!
  Sets the bottommost visible line to \a row.
*/

void QMultiLineEdit::setBottomCell( int row )
{
    //debug( "setBottomCell %d", row );
    int rowY = cellHeight() * row;
    int newYPos = rowY +  cellHeight() - viewHeight();
    setYOffset( QMAX( newYPos, 0 ) );
}

/*!
  Copies text from the clipboard onto the current cursor position.
  Any marked text is unmarked;
*/
void QMultiLineEdit::paste()
{
    //debug( "paste" );
    QString t = QApplication::clipboard()->text();
    if ( !t.isEmpty() ) {
	if ( hasMarkedText() )
	    turnMarkOff();
	QString *s = getString( cursorY );
	ASSERT( s );
	uchar *p = (uchar *) t.data();
	while ( *p ) {		// unprintable becomes space
	    if ( *p < 32 && *p != '\n')
		*p = 32;
	    p++;
	}

	int from = 0;

	int to = t.find( '\n' );
	if ( to < 0 ) { //just one line
	    s->insert( cursorX, t );
	    int w = textWidth( s );
	    setWidth( QMAX( cellWidth(), w ) );
	    cursorX  = t.length();
	} else { //multiline
	    setAutoUpdate( FALSE );
	    QString newString = s->mid( cursorX, s->length() );
	    s->remove( cursorX, s->length() );
	    *s += t.left( to );
	    cursorY++;
	    if ( cursorY >= numLines() ) {
		insert( "", numLines() );
	    }
	    from = to + 1;
	    while ( (to = t.find( '\n', from )) > 0 ) {
		insert( t.mid( from, to - from ), cursorY++ );
		from = to + 1;
	    }
	    int lastLen = t.length() - from;
	    newString.prepend( t.right( lastLen ) );
	    insert( newString, cursorY );
	    cursorX = lastLen;
	    updateCellWidth();
	    setAutoUpdate( TRUE );
	    repaint( FALSE );
	}
	markIsOn = FALSE;
    }
    curXPos  = 0;
}


/*!
  Removes all text.
*/

void QMultiLineEdit::clear()
{
    contents->first();
    while ( contents->remove() ) 
	;
    insert( "", -1 );
    dummy = TRUE;
}


/*!
  Reimplements QWidget::setFont() to update the list box line height.
*/

void QMultiLineEdit::setFont( const QFont &font )
{
    QWidget::setFont( font );
    setCellHeight( fontMetrics().lineSpacing() + 1 );
    updateCellWidth();
}

/*!
  Sets a new marked text limit, does not repaint the widget.
*/

void QMultiLineEdit::newMark( int posx, int posy, bool copy )
{
    markDragX  = posx;
    markDragY  = posy;
    cursorX    = posx;
    cursorY    = posy;
    markIsOn = ( markDragX != markAnchorX ||  markDragY != markAnchorY );
    if ( copy )
	copyText();
}

void QMultiLineEdit::markWord( int posx, int posy )
{
    QString *s = contents->at( posy );
    ASSERT( s );
    int i = posx;
    while ( i >= 0 && isprint(s->at(i)) && !isspace(s->at(i)) )
	i--;
    if ( i != posx )
	i++;
    markAnchorX = i;

    int lim = s->length();
    i = posx;
    while ( i < lim && isprint(s->at(i)) && !isspace(s->at(i)) )
	i++;
    markDragX = i;

    /*
    int tDispWidth = width() - 2*BORDER;
    int maxVis = offset + xPosToCursorPos( &s[(int)offset], fontMetrics(),
					   tDispWidth, tDispWidth );

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
    */
    copyText();
}


/*!
  Copies the marked text to the clipboard.
*/

void QMultiLineEdit::copyText()
{
    QString t = markedText();
    if ( !t.isEmpty() ) {
#if defined(_WS_X11_)
	disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
#endif
	QApplication::clipboard()->setText( t );
#if defined(_WS_X11_)
	connect( QApplication::clipboard(), SIGNAL(dataChanged()),
		 this, SLOT(clipboardChanged()) );
#endif
    }
}


/*!
  This private slot is activated when this line edit owns the clipboard and
  some other widget/application takes over the clipboard. (X11 only)
*/

void QMultiLineEdit::clipboardChanged()
{
#if defined(_WS_X11_)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    markIsOn = FALSE;
    repaint( FALSE );
#endif
}



/*!
  Sets cellWidth() to \a w without updating the entire widget.
*/

void QMultiLineEdit::setWidth( int w )
{
    if ( w == cellWidth() )
	return;
    bool u = autoUpdate();
    setAutoUpdate( FALSE );
    setCellWidth( w );
    setAutoUpdate( u );
    
}
