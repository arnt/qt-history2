/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qmultilinedit.cpp#69 $
**
** Definition of QMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
**
**
**
**
**
**
**
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

  <img src=qmlined-m.gif> <img src=qmlined-w.gif>
 */

static const int BORDER = 3;

static const int blinkTime  = 500;		// text cursor blink time
static const int scrollTime = 50;		// mark text scroll time


static int tabStopDist( const QFontMetrics &fm )
{
    return 8*fm.width( 'x' );
}

static int textWidthWithTabs( const QFontMetrics &fm,const char *s,int nChars )
{
    if ( !s )
	return 0;
    if ( nChars == -1 )
	nChars = strlen(s);

    int         dist = 0;
    const char *tmp  = s;
    if ( !tmp )
	return 0;
    int tabDist = tabStopDist(fm);
    while ( *tmp && tmp - s < nChars ) {
	if ( *tmp == '\t') {
	    dist = ( dist/tabDist + 1 ) * tabDist;
	} else {
	    dist += fm.width( tmp, 1 );
	}
	tmp++;
    }
    return dist;
}

static int xPosToCursorPos( const char *s, const QFontMetrics &fm,
			    int xPos, int width )
{
    const char *tmp;
    int	  dist;
    int tabDist;

    if ( !s )
	return 0;
    if ( xPos > width )
	xPos = width;
    if ( xPos <= 0 )
	return 0;

    int     distBeforeLastTab = 0;
    dist    = 0;
    tmp	    = s;
    tabDist = tabStopDist(fm);
    while ( *tmp && dist < xPos ) {
	if ( *tmp == '\t') {
	    distBeforeLastTab = dist;
	    dist = (dist/tabDist + 1) * tabDist;
	} else {
	    dist += fm.width( tmp, 1 );
	}
	tmp++;
    }
    if ( dist > xPos ) {
	if ( dist > width ) {
	    tmp--;
	} else {
	    if ( *(tmp - 1) == '\t' ) { // dist equals a tab stop position
		if ( xPos - distBeforeLastTab < (dist - distBeforeLastTab)/2 )
		    tmp--;
	    } else {
		if ( fm.width(tmp - 1, 1)/2 < dist-xPos )
		    tmp--;
	    }
	}
    }
    return tmp - s;
}

/*!
  Creates a new, empty, QMultiLineEdit.
*/

QMultiLineEdit::QMultiLineEdit( QWidget *parent , const char *name )
    :QTableView( parent, name)
{
    QFontMetrics fm( font() );
    setCellHeight( fm.lineSpacing() + 1 );
    setNumCols( 1 );

    setNumRows( 0 ); 
    setCellWidth( 1 ); // ### constant width
    contents = new QList<QString>;
    contents->setAutoDelete( TRUE );

    cursorX = 0; cursorY = 0;
    curXPos = 0;

    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   Tbl_smoothVScrolling |
		   Tbl_clipCellPainting
		   );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode( PaletteBase );
    setFocusPolicy( StrongFocus );
    setCursor( ibeamCursor );
    ((QScrollBar*)verticalScrollBar())->setCursor( arrowCursor );
    ((QScrollBar*)horizontalScrollBar())->setCursor( arrowCursor );
    dummy = FALSE;
    insertLine( "", -1 );
    readOnly 	   = FALSE;
    cursorOn	   = FALSE;
    dummy          = TRUE;
    markIsOn	   = FALSE;
    dragScrolling  = FALSE;
    dragMarking    = FALSE;
    textDirty	   = FALSE;
    wordMark	   = FALSE;
    overWrite	   = FALSE;
    markAnchorX    = 0;
    markAnchorY    = 0;
    markDragX      = 0;
    markDragY      = 0;
    blinkTimer     = 0;
    scrollTimer    = 0;
}

/*! \fn int QMultiLineEdit::numLines() const

  Returns the number of lines in the editor. The count includes any
  empty lines at top and bottom, so for an empty editor this method
  will return 1.
*/

/*! \fn bool QMultiLineEdit::atEnd() const

  Returns TRUE if the cursor is placed at the end of the text.
*/

/*! \fn bool QMultiLineEdit::atBeginning() const

  Returns TRUE if the cursor is placed at the beginning of the text.
*/


/*!
  \fn int QMultiLineEdit::lineLength( int line ) const
  Returns the number of characters at line number \a line.
*/

/*! \fn QString *QMultiLineEdit::getString( int line ) const

  Returns a pointer to the text at line \a line.
*/

/*! \fn void QMultiLineEdit::textChanged()

  This signal is emitted when the text is changed by  
  an event or by a slot. Not that the signal is not emitted when 
  you call a non-slot function such as insertLine().

  \sa returnPressed()
*/

/*! \fn void QMultiLineEdit::returnPressed()

  This signal is emitted when the user presses the return or enter
  key. It is not emitted if isReadOnly() is TRUE.

  \sa textChanged()
*/


/*! \fn bool QMultiLineEdit::isReadOnly() const

  Returns FALSE if this multi line edit accepts text input. 
  Scrolling and cursor movements are accepted in any case.

  \sa setReadOnly() QWidget::isEnabled()
*/

/*! \fn bool QMultiLineEdit::isOverwriteMode() const

  Returns TRUE if this multi line edit is in overwrite mode, i.e.
  if characters typed replace characters in the editor.

  \sa setOverwriteMode()
*/


/*! \fn void QMultiLineEdit::setOverwriteMode( bool on )

  Sets overwrite mode if \a on is TRUE. Overwrite mode means
  that characters typed replace characters in the editor.

  \sa isOverwriteMode()
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
    QFontMetrics fm( painter->font() );
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

    p.setTabStops( tabStopDist(fm) );

    int yPos = fm.leading()/2 - 1;
    int markX1, markX2;				// in x-coordinate pixels
    markX1 = markX2 = 0;			// avoid gcc warning
    if ( markIsOn ) {
	int markBeginX, markBeginY;
	int markEndX, markEndY;
	getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX );
	if ( row >= markBeginY && row <= markEndY ) {
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
    p.setPen( g.text() );
    p.drawText( BORDER,  yPos, cellWidth() - BORDER, cellHeight( row ),
		ExpandTabs, *s );
    if ( markX1 != markX2 ) {
	int sLength = s->length();
	int xpos1   =  BORDER + textWidthWithTabs( fm, s->data(), markX1 );
	int xpos2   =  BORDER + textWidthWithTabs( fm, s->data(), markX2 ) - 1;
	int fillxpos1 = xpos1;
	int fillxpos2 = xpos2;
	if ( markX1 == 0 )
	    fillxpos1 -= 2;
	if ( markX2 == sLength )
	    fillxpos2 += 3;
	p.setClipping( TRUE );
	p.setClipRect( fillxpos1 - updateR.left(), 0, 
		       fillxpos2 - fillxpos1, cellHeight(row) );
	p.fillRect( fillxpos1, 0, fillxpos2 - fillxpos1, 
		    cellHeight(row), g.text() );
	p.setPen( g.base() );
	p.drawText( BORDER, yPos, xpos2 - BORDER, cellHeight( row ),
		    ExpandTabs, s->data() );
	p.setClipping( FALSE );
    }

    if ( row == cursorY && cursorOn && !readOnly ) {
	int cursorPos = QMIN( (int)s->length(), cursorX );
	int cXPos   = BORDER + textWidthWithTabs( fm, *s, cursorPos ) - 1;
	int cYPos   = 0;
	if ( hasFocus() ) {
	    p.drawLine( cXPos - 2, cYPos,
			cXPos + 2, cYPos );
	    p.drawLine( cXPos    , cYPos,
			cXPos    , cYPos + fm.height() - 2);
	    p.drawLine( cXPos - 2, cYPos + fm.height() - 2,
			cXPos + 2, cYPos + fm.height() - 2);
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
    int w = textWidthWithTabs( QFontMetrics( font() ), *s, -1 );
    return w + 2 * BORDER;
}


/*!
  Returns the width in pixels of the text at line \a line.
*/

int QMultiLineEdit::textWidth( int line )
{
    //possibilities of caching...
    QString *s = contents->at( line );
    if ( !s ) {
	warning( 
	  "QMultiLineEdit::textWidth: Couldn't find contents at line %d", line );
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
	cursorOn = !cursorOn;
	updateCell( cursorY, 0, FALSE );
    } else if ( t->timerId() == scrollTimer ) {
	QPoint p = mapFromGlobal( QCursor::pos() );
	if ( p.y() < 0 )
	    cursorUp( TRUE );
	else if ( p.y() > height() )
	    cursorDown( TRUE );
	else if ( p.x() < 0 )
	    cursorLeft( TRUE, FALSE );
	else if ( p.x() > width() )
	    cursorRight( TRUE, FALSE );
    }
}

/*!
  
 */
bool QMultiLineEdit::getMarkedRegion( int *line1, int *col1, 
				      int *line2, int *col2 ) const
{
    if ( !markIsOn )
	return FALSE;
    if ( markAnchorY < markDragY ||
	 markAnchorY == markDragY && markAnchorX < markDragX) {
	*line1 = markAnchorY;
	*col1 = markAnchorX;
	*line2 = markDragY;
	*col2 = markDragX;
    } else {
	*line1 = markDragY;
	*col1 = markDragX;
	*line2 = markAnchorY;
	*col2 = markAnchorX;
    }
    return TRUE;
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
    int markBeginX, markBeginY;
    int markEndX, markEndY;
    if ( !getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX ) )
	return QString();
    if ( markBeginY == markEndY ) { //just one line
	QString *s  = getString( markBeginY );
	ASSERT(s);
	return s->mid( markBeginX, markEndX - markBeginX );
    } else { //multiline
	ASSERT( markBeginY >= 0);
	ASSERT( markEndY < (int)contents->count() );
	
	QString *firstS, *lastS;
	firstS = getString( markBeginY );
	lastS  = getString( markEndY );
	ASSERT( firstS != lastS );

	int len = firstS->length() - markBeginX + 1;
	int i;

	for( i = markBeginY + 1 ; i < markEndY ; i++ ) {
	    len += lineLength( i ) + 1;
	}
	len += markEndX + 1;

	QString tmp( len + 1 );
	int idx = 0;

	if ( firstS ) {
	    const char *p = firstS->data() + markBeginX;
	    while (( tmp[idx++] = *p++ ))
		;
	    tmp[idx-1] = '\n';
	} else {
	    tmp[idx++] = '\n';
	}

	for( i = markBeginY + 1; i < markEndY ; i++ ) {
	    const char *p;
	    p = (contents->at( i ))->data();
	    if ( p ) {
		while (( tmp[idx++] = *p++ ))
		    ;
		tmp[idx-1] = '\n';
	    } else {
		tmp[idx++] = '\n';
	    }
	}
	if ( lastS ) {
	    int i = 0;
	    while ( i < markEndX )
		tmp[idx++] = (*lastS)[i++];
	    tmp[idx] = 0;
	} else {
	    tmp[idx++] = 0;
	}

	return tmp;
    }
}



static const char* emptyLine = "";

/*!
  Returns the text at line number \a line, or 0 if \a line is invalid.
*/

const char * QMultiLineEdit::textLine( int line ) const
{
    QString *s = contents->at( line );
    if ( s ) {
	if ( s->isNull() )
	    return emptyLine;
	else
	    return *s;
    } else
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

    int len = 0;
    int i;
    for( i = 0 ; i < (int)contents->count() ; i++ ) {
	len += lineLength( i ) + 1;
    }

    QString tmp( len + 1 );
    int idx = 0;
    for( i = 0 ; i < (int)contents->count() ; i++ ) {
	const char *p = (contents->at( i ))->data();
	if ( p ) {
	    while (( tmp[idx++] = *p++ ))
		;
	    tmp[idx-1] = '\n';
	} else {
	    tmp[idx++] = '\n';
	}
    }
    tmp[idx-1] = 0;
    return tmp;
}


/*!
  Selects all text without moving the cursor.
*/

void QMultiLineEdit::selectAll()
{
    markAnchorX    = 0;
    markAnchorY    = 0;
    markDragY = numLines() - 1;
    markDragX = lineLength( markDragY );
    markIsOn = ( markDragX != markAnchorX ||  markDragY != markAnchorY );
    repaint( TRUE );
}



/*!
  Deselects all text (i.e. removes marking) and leaves the cursor at the
  current position.
*/

void QMultiLineEdit::deselect()
{
    turnMarkOff();
}


/*!
  Sets the text to \a s, removing old text, if any.
*/

void QMultiLineEdit::setText( const char *s )
{
    clear();
    insertLine( s, -1 );
    emit textChanged();
}


/*!
  Appends \a s to the text.
*/

void QMultiLineEdit::append( const char *s )
{
    insertLine( s, -1 );
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
	    cut();
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
	    emit returnPressed();
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
  Moves the cursor one page down.  If \a mark is TRUE, the text 
  is marked. 
*/

void QMultiLineEdit::pageDown( bool mark )
{ 
    bool oldAuto = autoUpdate();
    if ( mark )
	setAutoUpdate( FALSE );

    if ( partiallyInvisible( cursorY ) )
	cursorY = topCell();
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
	    newMark( cursorX, cursorY, FALSE );
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMIN( cursorY + pageSize, numLines() - 1);
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	makeVisible();
    }
    if ( oldAuto )
	if ( mark ) {
	    setAutoUpdate( TRUE );
	    repaint( FALSE );
	} else {
	    updateCell( oldY, 0, FALSE );
	}
    if ( !mark )
	turnMarkOff();
}


/*!
  Moves the cursor one page up.  If \a mark is TRUE, the text 
  is marked. 
*/

void QMultiLineEdit::pageUp( bool mark )
{
    bool oldAuto = autoUpdate();
    if ( mark )
	setAutoUpdate( FALSE );
    if ( partiallyInvisible( cursorY ) )
	cursorY = topCell();
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
	cursorY = QMIN( newTopCell + delta, numLines() - 1 );
	if ( partial )
	    cursorY--;
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMAX( cursorY - pageSize, 0 );
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
    }
    if ( oldAuto )
	if ( mark ) {
	    setAutoUpdate( TRUE );
	    repaint( FALSE );
	} else {
	    updateCell( oldY, 0, FALSE );
	}
    if ( !mark )
	turnMarkOff();
}

/*
  Scans txt, returning one line in s, returns the address of the next
  line, 0 if end of text.

 */
static const char *getOneLine( const char *txt, QString **s )
{
    if ( !txt ) {
	*s = new QString;
	return 0;
    }
    int len = 0;
    const char *p = txt;
    while ( *p && *p != '\n' ) {
	p++;
	len++;
    }
    *s = new QString( len + 1 );
    memmove( (*s)->data(), txt, len );
    (**s)[len] = 0;
    if (*p)
	return p+1;
    else
	return 0;
}


/*!
  Inserts \a txt at line number \a line, after character number \a col
  in the line. 
  If \a txt contains newline characters, new lines are inserted.

  The cursor position is adjusted. If the insertion position is equal to
  the cursor poition, the cursor is placed after the end of the new text.

 */

void QMultiLineEdit::insertAt( const char *txt, int line, int col )
{
    line = QMAX( QMIN( line, numLines() - 1), 0 );
    col = QMAX( QMIN( col,  lineLength( line )), 0 );

    QString *oldLine = getString( line );
    ASSERT( oldLine );
    bool cursorAfter = atEnd() || cursorY > line 
	  || cursorY == line && cursorX >= col;
    bool onLineAfter = cursorY == line && cursorX >= col;

    QString *textLine;
    const char *p = getOneLine( txt, &textLine );

    if ( !p ) { //single line
	oldLine->insert( col, *textLine );
	int w = textWidth( oldLine );
	setWidth( QMAX( cellWidth(), w ) );
	if ( onLineAfter )
	    cursorX += textLine->length();
    } else {
	int w = cellWidth();
	QString newString = oldLine->mid( col, oldLine->length() );
	oldLine->remove( col, oldLine->length() );
	if ( onLineAfter )
	    cursorX -= oldLine->length();
	*oldLine += *textLine;
	w = QMAX( textWidth( oldLine ), w );
	line++;
	cursorY++;
	while (( p = getOneLine( p, &textLine ) )) {
	    ASSERT ( textLine );
	    contents->insert( line++, textLine );
	    w = QMAX( textWidth( textLine ), w );
	    if ( cursorAfter )
		cursorY++;
	}
	int lastLen = textLine->length();
	if ( onLineAfter )
	    cursorX += lastLen;
	newString.prepend( *textLine );
	w = QMAX( textWidth( textLine ), w );
	insertLine( newString, line );
	setWidth( w );
    }
    if ( autoUpdate() )
	repaint( FALSE );
    textDirty = TRUE;
}


/*!
  Inserts \a s at line number \a line. If \a line is less than zero, or
  larger than the number of rows, the new text is put at the end.
  If \a s contains newline characters, several lines are inserted.

  The cursor position is not changed.
*/

void QMultiLineEdit::insertLine( const char *txt, int line )
{
    if ( dummy && numLines() == 1 && getString( 0 )->isEmpty() ) {
	contents->remove( (uint)0 );
	//debug ("insertLine: removing dummy, %d", count() );
	dummy = FALSE;
    }
    if ( line < 0 || line >= numLines() )
	line = numLines();
    QString *textLine;
    int w = cellWidth();
    const char *p = txt;
    do {
	p = getOneLine( p, &textLine );
	ASSERT ( textLine );
	contents->insert( line++, textLine );
	w = QMAX( textWidth( textLine ), w );
    } while ( p );

    setWidth( w );
    setNumRows( contents->count() );
    if ( autoUpdate() ) //### && visibleChanges
	repaint( FALSE );

    ASSERT( numLines() != 0 );
    makeVisible();
    textDirty = TRUE;
}

/*!
  Deletes the line at line number \a line. If \a
  line is less than zero, or larger than the number of lines, 
  no line is deleted.
*/

void QMultiLineEdit::removeLine( int line )
{
    if ( line >= numLines()  )
	return;
    if ( cursorY >= line && cursorY > 0 )
	cursorY--;
    bool updt = autoUpdate() && rowIsVisible( line );
    bool recalc = textWidth( line ) == cellWidth();
    contents->remove( line );
    if ( contents->count() == 0 ) {
	//debug( "remove: last one gone, inserting dummy" );
	insertLine( "", -1 );
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
    bool wasMarkedText = hasMarkedText();
    if ( wasMarkedText ) {
	del();					// ## Will flicker
    }
    QString *s = getString( cursorY );
    if ( cursorX > (int)s->length() )
	cursorX = s->length();
    if ( overWrite && !wasMarkedText && cursorX < (int)s->length() )
	del();                                 // ## Will flicker
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
    insertLine( newString, cursorY + 1 );
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
  Moves the cursor one character to the left. If \a mark is TRUE, the text 
  is marked. If \a wrap is TRUE, the cursor moves to the end of the 
  previous line  if it is placed at the beginning of the current line.

  \sa cursorRight() cursorUp() cursorDown()
*/

void QMultiLineEdit::cursorLeft( bool mark, bool wrap )
{
    if ( cursorX != 0 || cursorY != 0 && wrap ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	int ll = lineLength( cursorY );
	if ( cursorX > ll )
	    cursorX = ll;
	cursorOn = TRUE;
	cursorX--;
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
  Moves the cursor one character to the right.  If \a mark is TRUE, the text 
  is marked. If \a wrap is TRUE, the cursor moves to the beginning of the next
  line if it is placed at the end of the current line.
  \sa cursorLeft() cursorUp() cursorDown()
*/

void QMultiLineEdit::cursorRight( bool mark, bool wrap )
{
    int strl = lineLength( cursorY );

    if ( cursorX < strl || cursorY < (int)contents->count() - 1 && wrap ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	cursorOn = TRUE;
	cursorX++;
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
  Moves the cursor up one line.  If \a mark is TRUE, the text 
  is marked. 
  \sa cursorDown() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorUp( bool mark )
{
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
	cursorY--;
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
  Moves the cursor one line down.  If \a mark is TRUE, the text 
  is marked. 
  \sa cursorDown() cursorLeft() cursorRight()
*/

void QMultiLineEdit::cursorDown( bool mark )
{
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
	cursorY++;
	if ( cursorY > lastLin ) {
	    cursorY = lastLin;
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
    int markBeginX, markBeginY;
    int markEndX, markEndY;
    if ( getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX ) ) {
	textDirty = TRUE;
	setAutoUpdate( FALSE );
	if ( markBeginY == markEndY ) { //just one line
	    QString *s  = getString( markBeginY );
	    ASSERT(s);
	    s->remove( markBeginX, markEndX - markBeginX );
	    cursorX  = markBeginX;
	    cursorY  = markBeginY;
	    markIsOn    = FALSE;
	    updateCellWidth();
	} else { //multiline
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
		insertLine( "", -1 );

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
		removeLine( cursorY + 1 );
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
    wordMark = FALSE;
    int newY = findRow( m->pos().y() );
    if ( newY < 0 )
	newY = lastRowVisible();
    newY = QMIN( (int)contents->count() - 1, newY );
    QFontMetrics fm( font() );
    cursorX = xPosToCursorPos( *getString( newY ), fm,
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
    if ( readOnly )
    	return;
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
    if ( !dragMarking )
	return;
    if ( rect().contains( e->pos() ) ) {
	if ( dragScrolling ) {
	    killTimer( scrollTimer );
	    dragScrolling = FALSE;
	}
    } else if ( !dragScrolling ) {
	scrollTimer = startTimer( scrollTime );
	dragScrolling = TRUE;
    }

    int newY = findRow( e->pos().y() );
    if ( newY < 0 ) {
	if ( e->pos().y() < lineWidth() )
	    newY = topCell();
	else
	    newY = lastRowVisible();
    }
    newY = QMIN( (int)contents->count() - 1, newY );
    QFontMetrics fm( font() );
    QString *s = getString( newY );
    int newX = xPosToCursorPos( *s, fm,
				e->pos().x() - BORDER + xOffset(),
				cellWidth() - 2 * BORDER );

    if ( wordMark ) {
	int i = newX;
	int lim = s->length();
	int startclass = i < 0 || i >= lim ? -1 : charClass(s->at(i));
	if ( markAnchorY < markDragY || markAnchorY == markDragY
	     && markAnchorX < markDragX ) {
	    // going right
	    while ( i < lim && charClass(s->at(i)) == startclass )
		i++;
	} else {
	    // going left
	    while ( i >= 0 && charClass(s->at(i)) == startclass )
		i--;
	    i++;
	}
	newX = i;
    }

    if ( markDragX == newX && markDragY == newY )
	return;
    int oldY = markDragY;
    newMark( newX, newY, FALSE );
    for ( int i = QMIN(oldY,newY); i <= QMAX(oldY,newY); i++ )
	updateCell( i, 0, FALSE );
}


/*!
  Handles mouse release events.
*/
void QMultiLineEdit::mouseReleaseEvent( QMouseEvent * )
{
    if ( dragScrolling ) {
	killTimer( scrollTimer );
	dragScrolling = FALSE;
    }
    wordMark = FALSE;
    dragMarking   = FALSE;
    if ( markAnchorY == markDragY && markAnchorX == markDragX )
	markIsOn = FALSE;
    else
	copyText();
}


/*!
  Handles double click events.
*/

void QMultiLineEdit::mouseDoubleClickEvent( QMouseEvent *m )
{
    if ( m->button() ==  LeftButton ) {
	dragMarking    = TRUE;
	markWord( cursorX, cursorY );
	wordMark = TRUE;
	updateCell( cursorY, 0, FALSE );
    }
}


/*!
  Returns TRUE if line \a line is invisible or partially invisible.
*/

bool QMultiLineEdit::partiallyInvisible( int line )
{
    int y;
    if ( !rowYPos( line, &y ) )
	return TRUE;
    if ( y < 0 ) {
	//debug( "line %d occluded at top", line );
	return TRUE;
    } else if ( y + cellHeight() - 2 > viewHeight() ) {
	//debug( "line %d occluded at bottom", line );
	return TRUE;
    }
    return FALSE;
}

/*!
  Scrolls such that the cursor is visible
*/

void QMultiLineEdit::makeVisible()
{
    if ( !autoUpdate() )
	return;

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
  Computes the character position in line \a line which corresponds 
  to pixel \a xPos
*/

int QMultiLineEdit::mapFromView( int xPos, int line )
{
    QString *s = getString( line );
    if ( !s )
	return 0;
    QFontMetrics fm( font() );
    int index = xPosToCursorPos( *s, fm,
				 xPos - BORDER,
				 cellWidth() - 2 * BORDER );
    return index;
}

/*!
  Computes the pixel position in line \a line which corresponds to 
  character position \a xIndex
*/

int QMultiLineEdit::mapToView( int xIndex, int line )
{
    QString *s = getString( line );
    ASSERT( s );
    xIndex = QMIN( (int)s->length(), xIndex );
    QFontMetrics fm( font() );
    return BORDER + textWidthWithTabs( fm, *s, xIndex ) - 1;
}

/*!
  Traverses the list and finds an item with the maximum width, and
  updates the internal list box structures accordingly.
*/

void QMultiLineEdit::updateCellWidth()
{
    QString *s = contents->first();
    QFontMetrics fm( font() );
    int maxW = 0;
    int w;
    while ( s ) {
	w = textWidth( s );
	if ( w > maxW )
	    maxW = w;
	s = contents->next();
    }
    setWidth( maxW );
}


/*!
  Sets the bottommost visible line to \a line.
*/

void QMultiLineEdit::setBottomCell( int line )
{
    //debug( "setBottomCell %d", line );
    int rowY = cellHeight() * line;
    int newYPos = rowY +  cellHeight() - viewHeight();
    setYOffset( QMAX( newYPos, 0 ) );
}

/*!
  Copies text from the clipboard onto the current cursor position.
  Any marked text is unmarked.
*/
void QMultiLineEdit::paste()
{
    //debug( "paste" );
    QString t = QApplication::clipboard()->text();
    if ( !t.isEmpty() ) {
	if ( hasMarkedText() )
	    turnMarkOff();

	uchar *p = (uchar *) t.data();
	while ( *p ) {		// unprintable becomes space
	    if ( *p < 32 && *p != '\n' && *p != '\t' )
		*p = 32;
	    p++;
	}
	insertAt( t, cursorY, cursorX );
	markIsOn = FALSE;
	curXPos  = 0;
	makeVisible();
    }
}


/*!
  Removes all text.
*/

void QMultiLineEdit::clear()
{
    contents->first();
    while ( contents->remove() ) 
	;
    cursorX = cursorY = 0;
    insertLine( "", -1 );
    dummy = TRUE;
    repaint( TRUE );
}


/*!
  Reimplements QWidget::setFont() to update the list box line height.
*/

void QMultiLineEdit::setFont( const QFont &font )
{
    QWidget::setFont( font );
    QFontMetrics fm( font );
    setCellHeight( fm.lineSpacing() + 1 );
    updateCellWidth();
}

/*!
  Sets a new marked text limit, does not repaint the widget.
*/

void QMultiLineEdit::newMark( int posx, int posy, bool copy )
{
    if ( markDragX == posx && markDragY == posy &&
	 cursorX   == posx && cursorY   == posy )
	return;
    markDragX  = posx;
    markDragY  = posy;
    cursorX    = posx;
    cursorY    = posy;
    markIsOn = ( markDragX != markAnchorX ||  markDragY != markAnchorY );
    if ( copy )
	copyText();
}

/*!
  Marks the word at character position \a posx, \a posy.
 */
void QMultiLineEdit::markWord( int posx, int posy )
{
    QString *s = contents->at( posy );
    ASSERT( s );
    int lim = s->length();
    int i = posx - 1;

    int startclass = i < 0 || i >= lim ? -1 : charClass( s->at(i) );

    while ( i >= 0 && charClass(s->at(i)) == startclass )
	i--;
    i++;
    markAnchorY = posy;
    markAnchorX = i;

    i = posx;
    while ( i < lim && charClass(s->at(i)) == startclass )
	i++;
    markDragX = i;
    markDragY = posy;
    markIsOn = ( markDragX != markAnchorX ||  markDragY != markAnchorY );
    copyText();
}

/*!
  This may become a protected virtual member in a future Qt.
  This implementation is an example of a useful classification
  that aides selection of common units like filenames and URLs.
*/
int QMultiLineEdit::charClass( char ch )
{
    if ( !isprint(ch) || isspace(ch) ) return 1;
    else if ( isalnum(ch) || ch=='-' || ch=='+' || ch==':'
	    || ch=='.' || ch=='/' || ch=='\\'
	    || ch=='@' || ch=='$' || ch=='~' ) return 2;
    else return 3;
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
  Copies the selected text to the clipboard and deletes the selected text.
*/

void QMultiLineEdit::cut()
{
    if ( hasMarkedText() ) {
	copyText();
	del();
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


/*!
  Sets the cursor position to character number \a col in line number \a line.
  The parameters are adjusted to lie within the legal range.

  If \a mark is FALSE, the selection is cleared. otherwise it is extended

  \sa cursorPosition()
*/

void QMultiLineEdit::setCursorPosition( int line, int col, bool mark )
{
    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    int oldY = cursorY;
    cursorY = QMAX( QMIN( line, numLines() - 1), 0 );
    cursorX = QMAX( QMIN( col,  lineLength( cursorY )), 0 );
    curXPos = 0;
    makeVisible();
    updateCell( oldY, 0, FALSE );
    if ( mark )
	newMark( cursorX, cursorY, FALSE );
    else
	turnMarkOff();
}



/*!
  Sets \a line to the current line and \a col to the current character 
  position within that line.

  \sa setCursorPosition()
*/

void QMultiLineEdit::cursorPosition( int *line, int *col ) const
{
    if ( line )
	*line = cursorY;
    if ( col )
	*col = cursorX;
}


/*!
  Sets \a line to the current line and \a col to the current character 
  position within that line.

  OBSOLETE - Use cursorPosition() instead.
*/

void QMultiLineEdit::getCursorPosition( int *line, int *col )
{
    if ( line )
	*line = cursorY;
    if ( col )
	*col = cursorX;
}


/*!
  Returns TRUE if the view updates itself automatically whenever it
  is changed in some way.

  \sa setAutoUpdate()
*/

bool QMultiLineEdit::autoUpdate() const
{
    return QTableView::autoUpdate();
}


/*!
  Sets the auto-update option of multi-line editor to \e enable.

  If \e enable is TRUE (this is the default) then the editor updates
  itself automatically whenever it has changed in some way (generally,
  when text has been inserted or deleted).

  If \e enable is FALSE, the view does NOT repaint itself, or update
  its internal state variables itself when it is changed.  This can be
  useful to avoid flicker during large changes, and is singularly
  useless otherwise: Disable auto-update, do the changes, re-enable
  auto-update, and call repaint().

  \warning Do not leave the view in this state for a long time
  (i.e. between events ). If, for example, the user interacts with the
  view when auto-update is off, strange things can happen.

  Setting auto-update to TRUE does not repaint the view, you must call
  repaint() to do this.

  \sa autoUpdate() repaint()
*/

void QMultiLineEdit::setAutoUpdate( bool enable )
{
    QTableView::setAutoUpdate( enable );
}



/*!
  Returns the top center point where the cursor is drawn
*/

QPoint QMultiLineEdit::cursorPoint() const
{
    QPoint cp( 0, 0 );

    QFontMetrics fm( font() );
    int col, row;
    col = row = 0;
    cursorPosition( &row, &col );
    const char* line = textLine( row );
    ASSERT( line );
    cp.setX( BORDER + textWidthWithTabs( fm, line, col ) - 1 );
    cp.setY( (row * cellHeight()) + viewRect().y() );
    return cp;
}
