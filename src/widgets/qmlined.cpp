/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmlined.cpp#4 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qmlined.h"

#include "qpainter.h"
#include "qscrbar.h"
#include "qkeycode.h"




/*!
  \class QMultiLineEdit qmlined.h

  \brief The QMultiLineEdit widget is a simple editor for inputting text.

  \ingroup realwidgets

  This widget can be used to display text by calling enableInput(FALSE)

  The default key bindings are described in keyPressEvent(); they cannot
  be customized except by inheriting the class.

 */



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
		   Tbl_clipCellPainting );
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
    isInputEnabled = TRUE;
    setAcceptFocus( TRUE );
    setCursor( ibeamCursor );
    ((QScrollBar*)verticalScrollBar())->setCursor( sizeVerCursor );
    ((QScrollBar*)horizontalScrollBar())->setCursor( sizeHorCursor );
}


/*!
  \fn bool QMultiLineEdit::inputEnabled()

  Returns TRUE if this multi line edit accepts text input. 
  Scrolling and cursor movements are accepted in any case.

  \sa enableInput() QWidget::isEnabled()
 */


/*!
  If \a enable is TRUE, this multi line edit will accept text input.
  Scrolling and cursor movements are accepted in any case.

  \sa inputEnabled() QWidget::setEnabled() 
*/

void QMultiLineEdit::enableInput( bool enable ) 
{ 
    isInputEnabled = enable; 
    if ( enable ) {
	setCursor( ibeamCursor );
	setBackgroundColor( colorGroup().base() );
    }
    else {
	setCursor( arrowCursor );
	setBackgroundColor( colorGroup().background() );
    }
}




/*!

*/

QMultiLineEdit::~QMultiLineEdit()
{
    
}



/*!

*/

void QMultiLineEdit::paintCell( QPainter *p, int row, int )
{
    //debug( "paint cell %d", row );
    QFontMetrics fm = p->fontMetrics();
    QString *s = contents->at( row );
    if ( !s ) {
	warning( "QMultiLineEdit::paintCell, no text at line %d", row );
	return;
    }
    int yPos = fm.ascent() + fm.leading()/2 - 1;
    p->drawText( BORDER,  yPos , *s );
    if ( row == cursorY && cursorOn && isInputEnabled ) {
	int cursorPos = QMIN( (int)s->length(), cursorX );
	int curXPos   = BORDER +
			fm.width( *s, cursorPos ) - 1;
	int curYPos   = 0;
	if ( hasFocus() ) {
	    p->drawLine( curXPos - 2, curYPos,
			 curXPos + 2, curYPos );
	    p->drawLine( curXPos    , curYPos,
			 curXPos    , curYPos + fm.height() - 2);
	    p->drawLine( curXPos - 2, curYPos + fm.height() - 2,
			 curXPos + 2, curYPos + fm.height() - 2);
	}

    }
}




/*!

*/

int QMultiLineEdit::textWidth( QString *s )
{
    int w = fontMetrics().width( *s );
    return w + 2 * BORDER;
}

/*!

*/

int QMultiLineEdit::textWidth( int row )
{
    //possibilities of caching...
    QString *s = contents->at( row );
    if ( !s ) {
	warning( "Couldn't find contents at row %d", row );
	return 0;
    }
    return textWidth( s );
}



/*!

*/

void QMultiLineEdit::focusInEvent( QFocusEvent * )
{
    //debug( "startTimer" );
    //killTimers();
    startTimer( blinkTime );
    cursorOn = TRUE;
    updateCell( cursorY, 0 );
}


/*!

*/

void QMultiLineEdit::focusOutEvent( QFocusEvent * )
{
    killTimers();
}


/*!
  Cursor blinking
*/

void QMultiLineEdit::timerEvent( QTimerEvent * )
{
    if ( hasFocus() ) {
	/*
	if ( dragScrolling ) {
	    if ( scrollingLeft )
		cursorLeft( TRUE );	// mark left
	    else
		cursorRight( TRUE );	// mark right
	} else */{
	    cursorOn = !cursorOn;
	    updateCell( cursorY, 0 );
	}
    }
}



/*!

*/

bool QMultiLineEdit::hasMarkedText() const
{
    return FALSE;
}


/*!

*/

void QMultiLineEdit::clipboardChanged()
{
    
}


/*!

*/

const char * QMultiLineEdit::text() const
{
    return 0;
}


/*!

*/

void QMultiLineEdit::selectAll()
{
    
}


/*!

*/

void QMultiLineEdit::setText( const char * )
{
    
}


/*!
  The key press event handler converts a key press to some line editor
  action.

  Here are the default key bindings when inputEnabled() is TRUE:
  <dl compact>
  <dt> Left Arrow <dd> Move the cursor one character leftwards
  <dt> Right Arrow <dd> Move the cursor one character rightwards
  <dt> Up Arrow <dd> Move the cursor one line upwards
  <dt> Down Arrow <dd> Move the cursor one line downwards
  <dt> Page Up <dd> Move the cursor one page upwards
  <dt> Page Down <dd> Move the cursor one page downwards
  <dt> Backspace <dd> Delete the character to the left of the cursor
  <dt> Home <dd> Move the cursor to the beginning of the line
  <dt> End <dd>	 Move the cursor to the end of the line
  <dt> Delete <dd> Delete the character to the right of the cursor
  <dt> Shift - Left Arrow <dd> Mark text one character leftwards
  <dt> Shift - Right Arrow <dd> Mark text one character rightwards
  <dt> Control-A <dd> Move the cursor to the beginning of the line
  <dt> Control-B <dd> Move the cursor one character leftwards
  <dt> Control-C <dd> Copy the marked text to the clipboard.
  <dt> Control-D <dd> Delete the character to the right of the cursor
  <dt> Control-E <dd> Move the cursor to the end of the line
  <dt> Control-F <dd> Move the cursor one character rightwards
  <dt> Control-H <dd> Delete the character to the left of the cursor
  <dt> Control-K <dd> Delete to end of line
  <dt> Control-V <dd> Paste the clipboard text into line edit.
  <dt> Control-X <dd> Cut the marked text, copy to clipboard.
  </dl>
  All other keys with valid ASCII codes insert themselves into the line.

  Here are the default key bindings when inputEnabled() is FALSE:
  <dl compact>
  <dt> Left Arrow <dd> Scrolls the table rightwards
  <dt> Right Arrow <dd> Scrolls the table rightwards
  <dt> Up Arrow <dd> Scrolls the table one line downwards
  <dt> Down Arrow <dd> Scrolls the table one line upwards
  <dt> Page Up <dd> Scrolls the table one page downwards
  <dt> Page Down <dd> Scrolls the table one page upwards
  </dl>

*/

void QMultiLineEdit::keyPressEvent( QKeyEvent *e ) 
{
    int unknown = 0;
    if ( !isInputEnabled ) {
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
		//copyText();
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
	case Key_V:/* {			// paste
		      QString t = QApplication::clipboard()->text();
		      if ( !t.isEmpty() ) {
		      int i = t.find( '\n' );	// no multiline text
		      if ( i >= 0 )
		      t.truncate( i );
		      uchar *p = (uchar *) t.data();
		      while ( *p ) {		// unprintable becomes space
		      if ( *p < 32 )
		      *p = 32;
		      p++;
		      }
		      if ( hasMarkedText() ) {
		      tbuf.remove( minMark(), maxMark() - minMark() );
		      cursorPos = minMark();
		      if ( cursorPos < offset )
		      offset = cursorPos;
		      }
		      int tlen = t.length();
		      int blen = tbuf.length();
		      if ( tlen+blen >= maxLen ) {
		      if ( blen >= maxLen )
		      break;
		      t.truncate( maxLen-tlen );
		      }
		      tbuf.insert( cursorPos, t );
		      cursorRight( FALSE, tlen );
		      emit textChanged( tbuf.data() );
		      }
		      }*/
	case Key_X:
	    if ( hasMarkedText() ) {
		//copyText();
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
	    pageDown();
	    break;
	case Key_Prior:
	    pageUp();
	    break;
	case Key_Enter:
	case Key_Return:
	    newLine();
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

void QMultiLineEdit::pageDown()
{
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    int newTopCell = QMIN( topCell() + pageSize, count() - 1 - pageSize );
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;
    if ( newTopCell != topCell() ) {
	cursorY = newTopCell + delta;
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMIN( cursorY + pageSize, count() - 1);
	makeVisible();
    }
    cursorX = mapFromView( curXPos, cursorY );
    //makeVisible();
    updateCell( oldY, 0 );
}

void QMultiLineEdit::pageUp()
{
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    bool partial = delta == pageSize && viewHeight() != pageSize * cellHeight();
    int newTopCell = QMAX( topCell() - pageSize, 0 );
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;
    if ( newTopCell != topCell() ) {
	cursorY = newTopCell + delta;
	if ( partial )
	    cursorY--;
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMAX( cursorY - pageSize, 0 );
    }
    //makeVisible();
    updateCell( oldY, 0 );
}


/*!
  Inserts a new line containing \a s at line number \a row. If \a
  row is less than zero, or larger than the number of rows, the new line
  is put at the end.
*/

void QMultiLineEdit::insert( QString s, int row )
{
    QString *line = new QString( s );
    if ( row < 0 || !contents->insert( row, line ) )
	contents->append( line );

    bool    updt = autoUpdate() && rowIsVisible( row );

    setNumRows( contents->count() );
    int w = textWidth( line );
    setCellWidth( QMAX( cellWidth(), w ) );
    makeVisible();
    if ( updt )
	repaint();
}


/*!
  Deletes the line at line number \a row. If \a
  row is less than zero, or larger than the number of rows, 
  no line is deleted.
*/

void QMultiLineEdit::remove( int row )
{
    if ( row >= count()  )
	return;
    if ( cursorY >= row && cursorY > 0 )
	cursorY--;
    bool updt = autoUpdate() && rowIsVisible( row );
    bool recalc = textWidth( row ) == cellWidth();
    contents->remove( row );
    setNumRows( contents->count() );
    if ( recalc )
	updateCellWidth();
    makeVisible();
    if ( updt )
	repaint();
}

/*!
  Inserts \a c at the current cursor position.
 */

void QMultiLineEdit::insertChar( char c )
{
    /*
      if ( hasMarkedText() ) {
      tbuf.remove( minMark(), maxMark() - minMark() );
      cursorPos = minMark();
      if ( cursorPos < offset )
      offset = cursorPos;
      }
      */
    QString *s = getString( cursorY );
    if ( cursorX > (int)s->length() )
	cursorX = s->length();
    s->insert( cursorX, c);
    cursorRight( FALSE );			// will repaint
    //emit textChanged( tbuf.data() );

    //updateLineLength();
    int w = textWidth( s );
    setCellWidth( QMAX( cellWidth(), w ) );
    curXPos = 0;
    makeVisible();
}

/*!
  Makes a line break at the current cursor position.
 */

void QMultiLineEdit::newLine()
{
    QString *s = getString( cursorY );
    bool recalc = cursorX != (int)s->length() && textWidth( s ) == cellWidth();
    QString newString = s->mid( cursorX, s->length() );
    s->remove( cursorX, s->length() );
    insert( newString, cursorY + 1 );
    cursorRight( FALSE );
    curXPos = 0;
    if ( recalc )
	updateCellWidth();
    makeVisible();
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
	if ( recalc )
	    updateCellWidth();
	updateCell( cursorY, 0 );
    }
    curXPos = 0;
    makeVisible();
}


/*!
  Moves the cursor leftwards one or more characters.
  \sa cursorRight()
*/

void QMultiLineEdit::cursorLeft( bool mark, int steps )
{
    if ( steps != 1 ) {
	warning( "cursorLeft %d steps", steps );
    }
    if ( steps < 0 ) {
	cursorRight( mark, -steps );
	return;
    }
    if ( cursorX != 0 || cursorY != 0 ) {
	killTimers();
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
	    updateCell( oldY, 0 );
	}
	//###scrolling &c
	updateCell( cursorY, 0 );
    }
    curXPos = 0;
    makeVisible();
    startTimer( blinkTime );
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
	killTimers();
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
	    updateCell( oldY, 0 );
	}
	//###scrolling &c
	updateCell( cursorY, 0 );
    }
    curXPos = 0;
    makeVisible();
    startTimer( blinkTime );
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
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	killTimers();
	cursorOn = TRUE;
	cursorY -= steps;
	if ( cursorY < 0 ) {
	    cursorY = 0;
	}
        cursorX = mapFromView( curXPos, cursorY );
	updateCell( oldY, 0 );
	updateCell( cursorY, 0 );
	startTimer( blinkTime );
    }
    makeVisible();
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
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	killTimers();
	cursorOn = TRUE;
	cursorY += steps;
	if ( cursorY > lastLin ) {
	    cursorY = lastLin;
	}
        cursorX = mapFromView( curXPos, cursorY );
	updateCell( oldY, 0 );
	updateCell( cursorY, 0 );
	startTimer( blinkTime );
    }
    makeVisible();
}


/*!
  Deletes the character on the left side of the text cursor and moves the
  cursor one position to the left. If a text has been marked by the user
  (e.g. by clicking and dragging) the cursor will be put at the beginning
  of the marked text and the marked text will be removed.  \sa del()
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
  Deletes the character on the right side of the text cursor. If a text
  has been marked by the user (e.g. by clicking and dragging) the cursor
  will be put at the beginning of the marked text and the marked text will
  be removed.  \sa backspace()
*/

void QMultiLineEdit::del()
{
    if ( hasMarkedText() ) {
	//###
    } else {
	if ( !atEnd() ) {
	    QString *s = getString( cursorY );
	    if ( cursorX == (int) s->length() ) { // remove newline
		*s += *getString( cursorY + 1 );
		int w = textWidth( s );
		setCellWidth( QMAX( cellWidth(), w ) );
		remove( cursorY + 1 );
	    } else {
		bool recalc = textWidth( s ) == cellWidth();
		s->remove( cursorX, 1 );
		if ( recalc )
		    updateCellWidth();
		updateCell( cursorY, 0 );
	    }
	    //emit textChanged();
	}
    }
    curXPos = 0;
    makeVisible();
}

/*!
  Moves the text cursor to the left end of the line. If \a mark is TRUE,
  text will be marked towards the first position. If not, any marked
  text will be unmarked if the cursor is moved.
  
  \sa end() 
*/

void QMultiLineEdit::home( bool ) //mark
{
    if ( cursorX != 0 ) {
	killTimers();
	cursorX = 0;
	/*
	if ( mark ) {
	    newMark( cursorPos );
	} else {
	    markAnchor = 0;
	    markDrag   = markAnchor;
	}
	*/
	cursorOn = TRUE;
	updateCell( cursorY, 0 );
	//startTimer( dragScrolling ? scrollTime : blinkTime );
	startTimer( blinkTime );
    }
    curXPos = 0;
    makeVisible();
}

/*!
  Moves the text cursor to the right end of the line. If mark is TRUE text
  will be marked towards the last position, if not any marked text will
  be unmarked if the cursor is moved.
  \sa home()
*/

void QMultiLineEdit::end( bool ) //mark
{
    int tlen = lineLength( cursorY );
    if ( cursorX != tlen ) {
	killTimers();
	cursorX = tlen;
	/*
	if ( mark ) {
	    newMark( cursorPos );
	} else {
	    markAnchor = cursorPos;
	    markDrag   = markAnchor;
	}
	*/
	cursorOn  = TRUE;
	//startTimer( dragScrolling ? scrollTime : blinkTime );
	startTimer( blinkTime );
	updateCell( cursorY, 0 );
    }
    curXPos = 0;
    makeVisible();
}

#if 0
/*!
  Sets a new marked text limit, does not repaint the widget.
*/

void QMultiLineEdit::newMark( int pos, bool copy )
{
    markDrag  = pos;
    cursorPos = pos;
    if ( copy )
	copyText();
}

void QMultiLineEdit::markWord( int pos )
{
    int i = pos;
    while ( i >= 0 && isprint(tbuf.at(i)) && !isspace(tbuf.at(i)) )
	i--;
    if ( i != pos )
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
    copyText();
}
#endif


/*!

*/

void QMultiLineEdit::mousePressEvent( QMouseEvent *m )
{
    
    int newY = findRow( m->pos().y() );
    if ( newY < 0 )
	return;
    newY = QMIN( (int)contents->count() - 1, newY );
    cursorX = xPosToCursorPos( *getString( newY ), fontMetrics(),
			       m->pos().x() - BORDER + xOffset(),
			       cellWidth() - 2 * BORDER );
    if ( cursorY != newY ) {
	int oldY = cursorY;
	cursorY = newY;
	updateCell( oldY, 0 );
    }
    updateCell( cursorY, 0 );
}

bool QMultiLineEdit::partiallyInvisible( int row )
{
    int y;
    if ( !rowYPos( row, &y ) )
	return TRUE;
    if ( y < 0 ) {
	//debug( "row %d occluded at top", row );
	return TRUE;
    } else if ( y + cellHeight() - 1 > viewHeight() ) {
	//debug( "row %d occluded at bottom", row );
	return TRUE;
    }
    return FALSE;
}

/*!

*/

void QMultiLineEdit::makeVisible()
{
    if ( partiallyInvisible( cursorY ) ) {
	if ( cursorY > lastRowVisible() )
	    // ### one too many...
	    setTopCell( topCell() + cursorY - lastRowVisible() + 1 ); 
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

*/

int QMultiLineEdit::mapFromView( int xPos, int row )
{
    QString *s = getString( row );
    int index = xPosToCursorPos( *s, fontMetrics(),
				 xPos - BORDER,
				 cellWidth() - 2 * BORDER );
    return index;
}


/*!

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
    setCellWidth( maxW );
}
