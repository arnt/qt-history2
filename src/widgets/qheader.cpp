/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qheader.cpp#22 $
**
**  Table header
**
**  Created:  961105
**
** Copyright (C) 1996-1997 by Troll Tech AS.	 All rights reserved.
*****************************************************************************/
#include "qheader.h"

#include "qpainter.h"
#include "qdrawutl.h"

#include "qcursor.h"
#include "qbitmap.h"
static const int MINSIZE  = 8;
static const int MARKSIZE = 32;




static QCursor *vSplitCur = 0;
static QCursor *hSplitCur = 0;

#define hsplit_width 32
#define hsplit_height 32
static unsigned char hsplit_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x82, 0x41, 0x00, 0x00, 0x83, 0xc1, 0x00, 0x80, 0x83, 0xc1, 0x01,
  0xc0, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x03, 0x80, 0x83, 0xc1, 0x01,
  0x00, 0x83, 0xc1, 0x00, 0x00, 0x82, 0x41, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static unsigned char hsplitm_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0xc4, 0x23, 0x00, 0x00, 0xc6, 0x63, 0x00,
  0x00, 0xc7, 0xe3, 0x00, 0x80, 0xc7, 0xe3, 0x01, 0xc0, 0xff, 0xff, 0x03,
  0xe0, 0xff, 0xff, 0x07, 0xe0, 0xff, 0xff, 0x07, 0xc0, 0xff, 0xff, 0x03,
  0x80, 0xc7, 0xe3, 0x01, 0x00, 0xc7, 0xe3, 0x00, 0x00, 0xc6, 0x63, 0x00,
  0x00, 0xc4, 0x23, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

#define vsplit_width 32
#define vsplit_height 32
static unsigned char vsplit_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xe0, 0x07, 0x00,
  0x00, 0xf0, 0x0f, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
  0xc0, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x03, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x80, 0x01, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xe0, 0x07, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static unsigned char vsplitm_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xf0, 0x0f, 0x00,
  0x00, 0xf8, 0x1f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xc0, 0x03, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0xe0, 0xff, 0xff, 0x07,
  0xe0, 0xff, 0xff, 0x07, 0xe0, 0xff, 0xff, 0x07, 0xe0, 0xff, 0xff, 0x07,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00,
  0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0xf0, 0x0f, 0x00,
  0x00, 0xe0, 0x07, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x80, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };


/*!
  \class QHeader qheader.h
  \brief The QHeader class provides a table header.

  This is a table heading of the type used in a list view. It gives
  the user the opportunity to resize and move the columns (or rows for
  vertical headings.)

  This class can be used without a table view, if you need to control
  table-like structures.

  <img src=qheader-m.gif> <img src=qheader-w.gif>
 */



/*!
  Constructs a horizontal header.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QHeader::QHeader( QWidget *parent, const char *name )
    : QTableView( parent, name )
{
    orient = Horizontal;
    init( 0 );
}

/*!
  Constructs a horizontal header with \a n sections.

  The \e parent and \e name arguments are sent to the QWidget constructor.
  
*/

QHeader::QHeader( int n,  QWidget *parent, const char *name )
    : QTableView( parent, name )
{
    orient = Horizontal;
    init( n );
}

/*!
  Destroys the header.
 */
QHeader::~QHeader() 
{
}

/*!
  \fn void QHeader::sectionClicked (int logical) 
  
  This signal is emitted when a part of the header is clicked. In a
  list view, this signal would typically be connected to a slot which sorts
  the specified column.
*/

/*!
  \fn void QHeader::sizeChange( int section, int newSize ) 

  This signal is emitted when the user has changed the size of some
  of the parts of the header. This signal is typically connected to a slot
  that repaints the table.  
*/

/*!
  \fn void QHeader::moved (int from, int to) 

  This signal is emitted when the user has moved column \a from to 
  position \a to. 
  */

/*!
  Returns the size in pixels of section \a i of the header. \a i is the
  actual index.
  */

int QHeader::cellSize( int i ) const
{
    int s = pSize( i );  
    return s;
}



/*!
  Returns the position in pixels of section \a i of the header.  \a i is the
  actual index.
*/

int QHeader::cellPos( int i ) const
{
    /* cvs conflict here */

    int r = pPos( i ); 
    return r + offset();
}


/*!
  \fn int QHeader::count() const
  
  Returns the number of sections in the header.
*/

/*!
  \fn QHeader::Orientation QHeader::orientation() const

  Returns \c Horizontal if the header is horizontal, \c Vertical if
  the header is vertical.

  */

/*!
  \fn void QHeader::setTracking( bool enable )

  Sets tracking if \a enable is TRUE, otherwise turns off tracking.
  If tracking is on, the sizeChange() signal is emitted continuously
  while the mouse is moved, otherwise it is only emitted when the 
  mouse button is released.

  \sa tracking()
  */

/*!
  \fn bool QHeader::tracking() const

  Returns TRUE if tracking is on, FALSE otherwise.

  \sa setTracking()
  */

/*!
  What do you think it does?
 */
void QHeader::init( int n )
{
    if ( !hSplitCur )
	hSplitCur = new QCursor( QBitmap( hsplit_width, hsplit_height, hsplit_bits, TRUE),
				 QBitmap( hsplit_width, hsplit_height, hsplitm_bits, TRUE)
				 );
    if ( !vSplitCur )
	vSplitCur = new QCursor( QBitmap( vsplit_width, vsplit_height, vsplit_bits, TRUE),
				 QBitmap( vsplit_width, vsplit_height, vsplitm_bits, TRUE)
				 );
    state = Idle;

    sizes.resize(n+1);
    labels.resize(n+1);
    a2l.resize(n+1);
    l2a.resize(n+1);
    for ( int i = 0; i < n ; i ++ ) {
	labels[i] = 0;
	sizes[i] = 88;
	a2l[i] = i;
	l2a[i] = i;
    }
    setFrameStyle( QFrame::NoFrame );

    if ( orient == Horizontal ) {
	setCellWidth( 0 );
	setCellHeight( height() );
	setNumCols( n );
	setNumRows( 1 );
    } else {
	setCellWidth( width() );
	setCellHeight( 0 );
	setNumCols( 1 );
	setNumRows( n );
    }
    handleIdx = 0;
    //################
    labels[n] = 0; 
    sizes[n] = 0;
    a2l[n] = 0;
    l2a[n] = 0;
    //#############
    setMouseTracking( TRUE );
    trackingIsOn = FALSE;
}

/*!
  Sets the header orientation.  The \e orientation must be
  QQHeader::Vertical or QQHeader::Horizontal.
  \sa orientation()
*/

void QHeader::setOrientation( Orientation orientation )
{
    orient = orientation;
    repaint();
}

static QPen  ppppen( black, 1, DotLine );

/*!
  Paints a rectangle starting at \a p, with length \s.
  */
void QHeader::paintRect( int p, int s )
{
    QPainter paint;
    paint.begin( this );
    paint.setPen( ppppen );
    if ( orient == Horizontal )
	paint.drawRect( p, 3, s, height() - 5 );
    else	
	paint.drawRect( 3, p, height() - 5, s );
    paint.end();
}

/*!
  Marks the division line at \a idx.
  */
void QHeader::markLine( int idx )
{
    QPainter paint;
    paint.begin( this );
    paint.setPen( ppppen );
    int p = pPos( idx );
#if 0    
    paint.drawLine(  p, 0, p, height() );
    paint.drawLine(  p-3, 1, p+4, 1 );
    paint.drawLine(  p-3, height()-3, p+4, height()-3 );
#else
    int x = p - MARKSIZE/2;
    int y = 2;
    int x2 = p + MARKSIZE/2;
    int y2 = height() - 3;
    if ( orient == Vertical ) {
	int t = x; x = y; y = t;
	t = x2; x2 = y2; y2 = t;
    }

    paint.drawLine( x, y, x2, y );
    paint.drawLine( x, y+1, x2, y+1 );

    paint.drawLine( x, y2, x2, y2 );
    paint.drawLine( x, y2-1, x2, y2-1 );

    paint.drawLine( x, y, x, y2 );
    paint.drawLine( x+1, y, x+1, y2 );

    paint.drawLine( x2, y, x2, y2 );
    paint.drawLine( x2-1, y, x2-1, y2 );
#endif
    paint.end();

}

/*!
  Removes the mark at the division line at \a idx.
  */
void QHeader::unMarkLine( int idx )
{
    if ( idx < 0 )
	return;
    int p = pPos( idx );
    int x = p - MARKSIZE/2;
    int y = 2;
    int x2 = p + MARKSIZE/2;
    int y2 = height() - 3;
    if ( orient == Vertical ) {
	int t = x; x = y; y = t;
	t = x2; x2 = y2; y2 = t;
    }
    repaint( x, y, x2-x+1, y2-y+1);
}

/*!
  Returns the actual index of the section at position \a c, or -1 if outside.
 */
int QHeader::cellAt( int c ) const
{
    if ( orient == Horizontal )
	return findCol( c );
    else
	return findRow( c );
}


/*!
  Tries to find a line that is not a neighbour of  \c handleIdx.
 */
int QHeader::findLine( int c )
{
    int i = cellAt( c );
    if ( i == -1 )
	return handleIdx; //####### frustrating, but safe behaviour.
    if ( i == handleIdx )
	return i;
    if ( i == handleIdx - 1 &&  pPos( handleIdx ) - c > MARKSIZE/2 )
	return i;
    if ( i == handleIdx + 1 && c - pPos( i ) > MARKSIZE/2 ) 
	return i + 1;
    if ( c - pPos( i ) > pSize( i ) / 2 )
	return i + 1;
    else 
	return i;
}

void QHeader::moveAround( int fromIdx, int toIdx )
{
    if ( fromIdx == toIdx )
	return;
    int i;

    int idx = a2l[fromIdx];
    if ( fromIdx < toIdx ) {
	for ( i = fromIdx; i < toIdx - 1; i++ ) {
	    int t;
	    a2l[i] = t = a2l[i+1];
	    l2a[t] = i;
	}
	a2l[toIdx-1] = idx;
	l2a[idx] = toIdx-1;
    } else {
	for ( i = fromIdx; i > toIdx ; i-- ) {
	    int t;
	    a2l[i] = t = a2l[i-1];
	    l2a[t] = i;
	}
	a2l[toIdx] = idx;
	l2a[idx] = toIdx;
    }
}

/*!
  sets up the painter
*/

void QHeader::setupPainter( QPainter *p )
{
    p->setPen( colorGroup().text() );
    p->setFont( font() );
}


/*!
  paints a section of the header
*/

void QHeader::paintCell( QPainter *p, int row, int col )
{
    int i = ( orient == Horizontal ) ? col : row;
    int size = pSize( i );
    bool down = (i==handleIdx) && ( state == Pressed || state == Moving );
    //    debug( "i = %d, handleIdx= %d, down = %d ",i ,handleIdx, down );

    QRect fr( 0, 0, orient == Horizontal ?  size : width(),
	      orient == Horizontal ?  height() : size );

    if ( style() == WindowsStyle )
	qDrawWinButton( p, fr, colorGroup(), down );
    else
	qDrawShadePanel( p, fr, colorGroup(), down );

    int logIdx = mapToLogical(i);

    const char *s = labels[logIdx];
    int d = 0;
    if ( style() == WindowsStyle  &&
	 i==handleIdx && ( state == Pressed || state == Moving ) )
	d = 1;
    QRect r( 4+d, 2+d, size - 6, height() - 4 ); //#####

    if ( 0 && orient == Vertical ) { // we can't do that...
	QWMatrix m;       
	m.rotate( 90 );
	m.translate( 0, -width() ); //###########  
	p->setWorldMatrix( m );  
    }

    if ( s ) {
	p->drawText ( r, AlignLeft| AlignVCenter|SingleLine, s );
    } else {
	QString str;
	if ( orient == Horizontal )
	    str.sprintf( "Col %d", logIdx );
	else
	    str.sprintf( "Row %d", logIdx );
	p->drawText ( r, AlignLeft| AlignVCenter|SingleLine, str );
    }
}

void QHeader::mousePressEvent( QMouseEvent *m )
{
    handleIdx = 0;
    int c = orient == Horizontal ? m->pos().x() : m->pos().y();
    int i = 0;
    while ( i <= (int) count() ) { 
	if ( i+1 <= count() &&  pPos(i+1) - MINSIZE/2 < c && 
	     c < pPos(i+1) + MINSIZE/2 ) {
	    handleIdx = i+1;
	    state = Sliding;
	    break;
	} else if ( pPos(i)  < c && c < pPos( i+1 ) ) {  
	    handleIdx = i;
	    moveToIdx = -1;
	    state = Pressed;
	    clickPos = c;
	    repaint(sRect( handleIdx ));
	    break;
	}
	i++;
    }
}

void QHeader::mouseReleaseEvent( QMouseEvent *m )
{
    State oldState = state;
    state = Idle;
    switch ( oldState ) {
    case Pressed:
	repaint(sRect( handleIdx ));
	if ( sRect( handleIdx ).contains( m->pos() ) )
	    emit sectionClicked( handleIdx );
	break;
    case Sliding:
	// setCursor( arrowCursor ); // We're probably still there...
	emit sizeChange( 0, 0 ); //######################################
	break;
    case Moving: {
	setCursor( arrowCursor );
	if ( handleIdx != moveToIdx && moveToIdx != -1 ) {
	    moveAround( handleIdx, moveToIdx );
	    emit moved( handleIdx, moveToIdx );
	    repaint();
	} else { 
	    if ( sRect( handleIdx).contains( m->pos() ) )
		emit sectionClicked( handleIdx );
	    repaint(sRect( handleIdx ));
	}
	break;
    }
    default:
	debug("QHeader::mouseReleaseEvent() no state");
	break;
    }
}

void QHeader::mouseMoveEvent( QMouseEvent *m )
{
    int s = orient == Horizontal ? m->pos().x() : m->pos().y();
    if ( state == Idle ) {
	bool hit = FALSE;
	int i = 0;
	while ( i <= (int) count() ) { 
	    if ( i && pPos(i) - MINSIZE/2 < s && s < pPos(i) + MINSIZE/2 ) {
		hit = TRUE;
		if ( orient == Horizontal )
		    setCursor( *hSplitCur );
		else
		    setCursor( *vSplitCur );
		break;
	    }
	    i++;
	}
	if ( !hit )
	    setCursor( arrowCursor );
    } else {
	switch ( state ) {
	case Idle:
	    debug( "QHeader::mouseMoveEvent() Idle state" );
	    break;
	case Pressed:
	    if ( QABS( s - clickPos ) > 4 ) {
		state = Moving;
		moveToIdx = -1;
		if ( orient == Horizontal )
		    setCursor( sizeHorCursor );
		else
		    setCursor( sizeVerCursor );
	    }
	    break;
	case Sliding:
	    if ( s > pPos(handleIdx-1) + MINSIZE ) {

		int oldPos = pPos( handleIdx );
		int delta = s - oldPos;
		sizes[mapToLogical(handleIdx - 1)] += delta;
		int repaintPos = QMIN( oldPos, s ); 
		repaint(repaintPos-2, 0, width(), height());
		/*
		int us, uw;
		if ( oldPos < s ) {
		    us = oldPos;
		    uw = s - oldPos;
		    uw += 3; //#######
		    us -= 3; //#######
		} else {
		    int w = ( orient == Horizontal ) ? width() : height();
		    uw = (oldPos - s);
		    us = w - uw;
		    uw += 3; //#######
		    us -= 3; //#######
		}
		if ( orient == Horizontal ) {
		    bitBlt( this, s, 0, this, oldPos, 0 );
		    repaint( us, 0, uw, height() ); 
		} else {
		    bitBlt( this, 0, s, this, 0, oldPos );
		    repaint( 0, us, width(), uw ); 
		}
		*/
		if ( tracking() )
		    emit sizeChange( handleIdx - 1, pSize( handleIdx - 1 ) );
	    }
	    break;
	case Moving: {
	    int newPos = findLine( s );
	    if ( newPos != moveToIdx ) {
		if ( moveToIdx == handleIdx || moveToIdx == handleIdx + 1 )
		    repaint( sRect(handleIdx) );
		else
		    unMarkLine( moveToIdx );
		moveToIdx = newPos;
		if ( moveToIdx == handleIdx || moveToIdx == handleIdx + 1 )
		    paintRect( pPos( handleIdx ), pSize( handleIdx ) );
		else
		    markLine( moveToIdx );
	    }
	    break;
	}
	default:
	    warning( "QHeader::mouseMoveEvent, unknown state" );
	    break;
	}
    }
}

/*!
  Returns the rectangle covered by actual section \a i.
*/

QRect QHeader::sRect( int i )
{
    if ( orient == Horizontal )
	return QRect( pPos( i ), 0, pSize( i ), height() );
    else
	return QRect( 0, pPos( i ), width(), pSize( i ) );
}

/*!
  Sets the text on logical section \a i to \a s. If the section does not exist,
  nothing happens.
*/

void QHeader::setLabel( int i, const char *s, int size )
{
    if ( i >= 0 && i < count() ) {
	labels[i] = s;
	if ( size )
	    sizes[i] = size;
    }
    repaint();
}


/*!
  Adds a new section, with label text \a s. Returns the index.
*/

int QHeader::addLabel( const char *s, int size )
{
    int n = count() + 1; //###########
    labels.resize( n + 1 );
    labels[n-1] = s;
    sizes.resize( n + 1 );
    sizes[n-1] = size ? size : 88; //###
    a2l.resize( n + 1 );
    l2a.resize( n + 1 );
    a2l[n-1] = n-1;
    l2a[n-1] = n-1;
    //    recalc();
    if ( orient == Horizontal )
	setNumCols( n );
    else
	setNumRows( n );
    repaint(); //####
    return n - 1;
}


/*!
  Handles resize events.
*/

void QHeader::resizeEvent( QResizeEvent * )
{
    setCellHeight( height() ); //######
}

/*!
  Returns the recommended size of the QHeader. Only the thickness is
  interesting, the other dimension is taken from the current size.
*/
QSize QHeader::sizeHint() const
{
    QFontMetrics fm( font() );
    if ( orient == Horizontal )
	return QSize( width(), fm.lineSpacing() + 6 );
    else
	return QSize( fm.lineSpacing() + 6, height() );
}


/*!
  Scrolls the header such that \a x becomes the leftmost (or uppermost
  for vertical headers) visible pixel.
*/

void QHeader::setOffset( int x )
{
    if ( orient == Horizontal )
	setXOffset( x );
    else
	setYOffset( x );
}



/*!
  Returns the position of actual division line \a i. May return a position 
  outside the widget.
 */
int QHeader::pPos( int i ) const 
{ 
    int r = 0;
    bool ok;
    if ( orient == Horizontal )
	ok = colXPos( i, &r );
    else
	ok = rowYPos( i, &r );
    if ( !ok ) {
	r = 0;
	for ( int j = 0; j < i; j++ ) 
	    r += pSize( j );
	r -= offset();
    }
    return r;
}


/*!
  Returns the size of actual section \a i.
 */
int QHeader::pSize( int i ) const 
{
    return sizes[mapToLogical(i)];
}



/*!
  int QHeader::offset() const
  Returns the leftmost (or uppermost for vertical headers) visible pixel.
 */


int QHeader::offset() const
{
     if ( orient == Horizontal )
	return xOffset();
    else
	return yOffset();
}


int QHeader::cellHeight( int row )
{
    if ( orient == Vertical )
	return pSize( row );
    else
	return QTableView::cellHeight();
}


int QHeader::cellWidth( int col )
{
    if ( orient == Horizontal )
	return pSize( col );
    else
	return QTableView::cellWidth();
}


/*!
  Translates from actual indexes to logical indexes
*/

int QHeader::mapToLogical( int a ) const
{
    return a2l[ a ];
}


/*!
  Translates from logical indexes to actual indexes
*/

int QHeader::mapToActual( int l ) const
{
    return l2a[ l ];
}


/*!
  Sets the size of logical cell \a i to \a s pixels.

  \warning does not repaint or send out signals at present.
*/

void QHeader::setCellSize( int i, int s )
{
    sizes[i] = s;
}


/*!
  Not implemented
*/

void QHeader::setResizeEnabled( bool, int )
{
    
}


/*!
  Not implemented
*/

void QHeader::setMovingEnabled( bool )
{
    
}


/*!
  Not implemented
*/

void QHeader::setClickEnabled( bool, int )
{
    
}
