/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qheader.cpp#10 $
**
**  Table header
**
**  Created:  961105
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
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
  \brief The QHeader widget class provides a table header.

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
    : QWidget( parent, name )
{
    orient = Horizontal;
    init( 0 );
}

/*!
  Constructs a horizontal header with \a n sections.

  The \e parent and \e name arguments are sent to the QWidget constructor.
  
*/

QHeader::QHeader( int n,  QWidget *parent, const char *name )
    : QWidget( parent, name )
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
  \fn void QHeader::sectionClicked (int) 
  
  This signal is emitted when a part of the header is clicked. In a
  list view, this signal would typically be connected to a slot which sorts
  the specified column.
*/

/*!
  \fn void QHeader::sizeChange() 

  This signal is emitted when the user has changed the size of some
  of the parts of the header. This signal is typically connected to a slot
  which resizes all columns, finding the sizes from cellSize().
  
*/

/*!
  \fn void QHeader::moved (int from, int to) 

  This signal is emitted when the user has moved column \a from to 
  position \a to. This signal is typically connected to a slot which 
  is similar to the following:

  \code
       //payload is an array containing the column data
       void MyTable::moveCol( int fromIdx, int toIdx )
       {
	   if ( fromIdx == toIdx )
	       return;
	   MyType tmp = payload[fromIdx];
	   if ( fromIdx < toIdx ) {
	       for ( int i = fromIdx; i < toIdx - 1; i++ ) {
		   payload[i] = payload[i+1];
	       }
	       payload[toIdx-1] = tmp;
	   } else {
	       for ( int i = fromIdx; i > toIdx ; i-- ) {
		   payload[i] = payload[i-1];
	       }
	       payload[toIdx] = tmp;
	   }
	   // redisplay logic goes here
	   // and probably a repaint
       }      

  \endcode
  */

/*!
  \fn int QHeader::cellSize( int i ) const

  Returns the size in pixels of section \a i of the header.
  */

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
    state = None;

    places.resize(n+1);
    recalc();
    labels.resize(n+1);
    for ( int i = 0; i < n ; i ++ )
	labels[i] = "Uninitialized";
    labels[n] = 0; 
    setMouseTracking( TRUE );
    trackingIsOn = FALSE;
}

void QHeader::recalc()
{
    int i;
    int n = count();
    if ( n <= 0 ) return;
    int w = orient == Horizontal ? width() : height();
    for ( i = 0; i < n; i ++ )
	places[i] = i*w/n;
    places[n] = w;
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
  Returns the index of the section at position \a c, or -1 if outside.
 */
int QHeader::pos2idx( int c )
{
    int i = 0;
    while ( i < (int) count() - 1 && c > pPos( i + 1 ) )
	i++;
    return i;
}

/*!
  Tries to find a line that is not a neighbour of  \c handleIdx.
 */
int QHeader::findLine( int c )
{
    int i = pos2idx( c );
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
    debug( "moving from %d to %d", fromIdx, toIdx );
    QArray<QCOORD> sizes( count() );
    int i;
    for ( i = 0; i < count(); i++ )
	sizes[i] = pSize( i );
    const char *p = labels[fromIdx];
    int s = sizes[fromIdx];
    if ( fromIdx < toIdx ) {
	for ( i = fromIdx; i < toIdx - 1; i++ ) {
	    sizes[i] = sizes[i+1];
	    labels[i] = labels[i+1];
	}
	sizes[toIdx-1] = s;
	labels[toIdx-1] = p;
    } else {
	for ( i = fromIdx; i > toIdx ; i-- ) {
	    sizes[i] = sizes[i-1];
	    labels[i] = labels[i-1];
	}
	sizes[toIdx] = s;
	labels[toIdx] = p;
    }
    int place = 0;
    for ( i = 0; i < count(); i++ ) {
	places[i] = place;
	place += sizes[i];
    }
    places[ count() ] = orient == Horizontal? width() : height();
}

void QHeader::paintEvent( QPaintEvent *event )
{
    QPainter p;
    p.begin( this );
    p.setClipRect( event->rect() );

    QColorGroup g = colorGroup();
    QRect r( 0, 0, width(), height() );
    switch ( style() ) {
    case WindowsStyle:
	{
	    for ( int i = 0; i < (int) count(); i++ ) {
		bool down = (i==handleIdx) && ( state == Pressed || state == Moving );
		qDrawWinButton( &p, sRect(i), g, down );
	    }
	    break;
	}
    default:
    case MotifStyle:
	{
	    qDrawShadePanel( &p, r, g, FALSE, 2 );
	    for ( int i = 1; i < (int) count(); i++ ) {
		if ( orient == Horizontal ) {
		    int x = pPos( i );
		    p.setPen( g.dark() );
		    p.drawLine( x,  r.top()+1, x, r.bottom() );
		    p.setPen( g.light() );
		    p.drawLine( x+1,  r.top(), x+1, r.bottom() - 1 );
		} else {
		    int y = pPos( i );
		    p.setPen( g.dark() );
		    p.drawLine( r.left()+1, y, r.right(), y );
		    p.setPen( g.light() );
		    p.drawLine( r.left(), y+1, r.right() - 1, y+1 );
		}
	    }
	    if ( state == Pressed || state == Moving ) {
		if ( orient == Horizontal ) {
		    QRect q( pPos( handleIdx ) + 1, r.top()+1, 
			     pSize( handleIdx ), r.height()-2 );
		    qDrawShadePanel( &p, q, g, TRUE, 2 );
		} else {
		    QRect q( r.left()+1, pPos( handleIdx ) + 1, 
			     r.width()-2, pSize( handleIdx ) );
		    qDrawShadePanel( &p, q, g, TRUE, 2 );
		}
	    }
	    break;
	}
    }
    p.setPen( g.text() );

    for ( int i = 0; i < (int) count(); i++ ) {
	const char *s = labels[i];
	int d = 0;
	if ( style() == WindowsStyle  &&
	     i==handleIdx && ( state == Pressed || state == Moving ) )
	    d = 1;
	QRect r( pPos(i)+4+d, 2+d, pSize(i) - 6, 
		 orient == Horizontal ?  height() - 4 : width() - 4 );

	if ( orient == Vertical ) {
	    QWMatrix m;       
	    m.rotate( 90 );
	    m.translate( 0, -width() ); //###########  
	    p.setWorldMatrix( m );  
	}
	p.drawText ( r, AlignLeft| AlignVCenter|SingleLine, s );
    }
    p.end();
}

void QHeader::mousePressEvent( QMouseEvent *m )
{
    handleIdx = 0;
    int c = orient == Horizontal ? m->pos().x() : m->pos().y();
    int i = 0;
    while ( i < (int) count() ) { 
	if ( i+1 < count() &&  pPos(i+1) - MINSIZE/2 < c && 
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
    state = None;
    switch ( oldState ) {
    case Pressed:
	repaint(sRect( handleIdx ));
	if ( sRect( handleIdx ).contains( m->pos() ) )
	    emit sectionClicked( handleIdx );
	break;
    case Sliding:
	// setCursor( arrowCursor ); // We're probably still there...
	emit sizeChange();
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
	debug("no state");
	break;
    }
}

void QHeader::mouseMoveEvent( QMouseEvent *m )
{
    int s = orient == Horizontal ? m->pos().x() : m->pos().y();
    if ( state == None ) {
	bool hit = FALSE;
	int i = 0;
	while ( i < (int) count() ) { 
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
	case None:
	    debug( "No state" );
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
		for ( int i = handleIdx; i < (int) count(); i++ )
		    places[i] += delta;

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
  Returns the rectangle covered by section \a i.
*/

QRect QHeader::sRect( int i )
{
    if ( orient == Horizontal )
	return QRect( pPos( i ), 0, pSize( i ), height() );
    else
	return QRect( 0, pPos( i ), width(), pSize( i ) );
}

/*!
  Sets the text on section \a i to \a s. If the section does not exist,
  nothing happens.
*/

void QHeader::setLabel( int i, const char *s )
{
    if ( i >= 0 && i < count() )
	labels[i] = s;
    repaint();
}


/*!
  Adds a new section, with label text \a s. Returns the index.
*/

int QHeader::addLabel( const char *s )
{
    int n = count() + 1;
    labels.resize( n + 1 );
    labels[n-1] = s;
    places.resize( n + 1 );
    recalc();
    repaint();
    return n - 1;
}


/*!
  Handles resize events.
*/

void QHeader::resizeEvent( QResizeEvent * )
{
    recalc();
}

/*!
  Returns the recommended size of the QHeader. Only the thickness is
  interesting, the other dimension is taken from the current size.
*/
QSize QHeader::sizeHint() const
{
    if ( orient == Horizontal )
	return QSize( width(), fontMetrics().lineSpacing() + 7 );
    else
	return QSize( fontMetrics().lineSpacing() + 7, height() );
}
