/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsplitter.cpp#1 $
**
**  Splitter widget
**
**  Created:  980105
**
** Copyright (C) 1998 by Troll Tech AS.	 All rights reserved.
*****************************************************************************/
#include "qsplitter.h"

#include "qpainter.h"
#include "qdrawutl.h"

/*!
  \class QSplitter qsplitter.h
  \brief QSplitter implements a splitter widget.

  A splitter lets the user control the size of two child widgets.

  setFirstWidget() and setSecondWidget() specifies the two widgets.

 */

/*!
  Creates a horizontal splitter.
  */

QSplitter::QSplitter( QWidget *parent, const char *name )
    :QFrame(parent,name) 
{ 
     orient = Horizontal;
     init();
}
/*!
  Creates splitter with orientation \a o.
  */

QSplitter::QSplitter( Orientation o, QWidget *parent, const char *name )
    :QFrame(parent,name) 
{ 
     orient = o;
     init();
}


/*
QSplitter::setBorder2( int b )
{
    if ( bord == b )
	return;

    bord = b;
    recalc();
}

*/

void QSplitter::init()
{
    ratio = -1;

   setMouseTracking( TRUE );
    moving = 0;
    w1  = w2 = 0;
    if ( style() == WindowsStyle )
	bord = 3;
    else
	bord = 5;
}

/*!
  Lets \a w be the left (or top) widget.
 */
void QSplitter::setFirstWidget( QWidget *w ) {
    if ( w->parentWidget() != this ) {
	warning( "QSplitter::setFirstWidget(), must be child." );
	return;
    }
    w1 = w;
    if ( w2 == w1 )
	w2 = 0;
    recalc();
}

/*!
  Lets \a w be the right (or bottom) widget.
 */
void QSplitter::setSecondWidget( QWidget *w ) {
    if ( w->parentWidget() != this ) {
	warning( "QSplitter::setSecondWidget(), must be child." );
	return;
    }
    w2 = w;
    if ( w2 == w1 )
	w1 = 0;
    recalc();
}

/*!
  Sets the orientation to \a o.
 */
void QSplitter::setOrientation( Orientation o )
{
    if ( orient == o )
	return;
    orient = o;
    recalc();
}


QCOORD QSplitter::r2p( int r )
{
    return (pick(contentsRect().size()) * r) / 256;
}

int QSplitter::p2r( QCOORD p )
{
    int s = pick(contentsRect().size());
    
    return s ? ( p * 256 ) / s : 128;
}



/*!
  Reimplemented to provide childRemoveEvent() and childInsertEvent()
  without breaking binary compatibility.
 */
bool QSplitter::event( QEvent *e )
{
    switch( e->type() ) {
    case Event_ChildInserted:
	childInsertEvent( (QChildEvent*) e );
	return TRUE;
	break;
    case Event_ChildRemoved:
	childRemoveEvent( (QChildEvent*) e );
	return TRUE;
	break;
    case Event_LayoutHint:
	doResize();
	return TRUE;
	break;
    default:
	return QWidget::event( e );
    }
}

void QSplitter::resizeEvent( QResizeEvent * )
{
    doResize();
    //recalc(); //####
}



void QSplitter::leaveEvent( QEvent * )
{
    //    setCursor( arrowCursor );
}

void QSplitter::childRemoveEvent( QChildEvent *c )
{
    if ( c->child() == w1 ) {
	w1 = 0;
	recalc();
    } else if ( c->child() == w2 ) {
	w2 = 0;
	recalc();
    }
}

void QSplitter::childInsertEvent( QChildEvent * )
{
}

void QSplitter::mouseReleaseEvent( QMouseEvent * )
{
    moving = 0;
}

void QSplitter::mousePressEvent( QMouseEvent *m )
{
    if ( m->button() == LeftButton )
	moving = hit( m->pos() );
}

void QSplitter::mouseMoveEvent( QMouseEvent *m )
{
    if ( moving ) {
	moveSplitter( pick( m->pos() ) );
    } else {
	if ( hit( m->pos() ) )
	    setCursor( crossCursor );
	else
	    setCursor( arrowCursor );
    }
}

int QSplitter::hit( QPoint pnt )
{
    //### fancy 2-dim hit for Motif...
    QCOORD p = pick(pnt);
    if ( w1 && p > pick( w1->geometry().bottomRight() ) && 
	 w2 && p < pick( w2->pos() ) )
	return 1;
    else   
	return 0;
}


/*!
  Draws the splitter handle in the rectangle described by \a x, \a y, 
  \a w, \a h using painter \a p.
 */
void QSplitter::drawSplitter( QPainter *p, QCOORD x, QCOORD y, QCOORD w, QCOORD h )
{
    static const int motifOffset = 10;
    if ( style() == WindowsStyle ) {
	qDrawWinPanel( p, x, y, w, h, colorGroup() );
    } else {
    	if ( orient == Horizontal ) {
	    QCOORD xPos = x + w/2;
	    QCOORD kPos = motifOffset;
	    QCOORD kSize = bord*2 - 1;

	    qDrawShadeLine( p, xPos, kPos + kSize - 1 ,
			    xPos, h, colorGroup() );
	    qDrawShadePanel( p, xPos-bord+1, kPos,
			     kSize, kSize, colorGroup() );
	    qDrawShadeLine( p, xPos, 0, xPos, kPos ,colorGroup() );
	} else {
	    QCOORD yPos = y + h/2;
	    QCOORD kPos = w - motifOffset - 2*bord;
	    QCOORD kSize = bord*2 - 1;

	    qDrawShadeLine( p, 0, yPos, kPos, yPos, colorGroup() );
	    qDrawShadePanel( p, kPos, yPos-bord+1,
			     kSize, kSize, colorGroup() );
	    qDrawShadeLine( p, kPos + kSize -1, yPos, 
			    w, yPos, colorGroup() );
	}
    }
}


void QSplitter::drawContents( QPainter *p )
{
    if ( !w1 || !w2 )
	return;

    QRect r = contentsRect();

    if ( orient == Horizontal ) {
	QCOORD xPos = w2->x() - bord*2;
	drawSplitter( p, xPos, r.y(), bord*2, r.height() );
    } else {
	QCOORD yPos = w2->y() - bord*2;
	drawSplitter( p, r.x(), yPos, r.width(), bord*2 );

    }
}

/*!
  Moves the center of the splitter handle as close as possible to
  \a p which is the distance from the left (or top) edge of the widget.

  Only has effect if both widgets are set.

 */
void QSplitter::moveSplitter( QCOORD p )
{
    if ( !w1 || !w2 )
	return;

    QWidget *prev = w1;
    QWidget *w = w2;

    ratio = p2r( p );


    QRect r = contentsRect();

    QCOORD p0 = pick( r.topLeft() );
    QCOORD p1 = pick( r.bottomRight() );

    QCOORD min = p0 + 1; //### no zero size widgets
    min = QMAX( min, p0 + pick( prev->minimumSize() ) );
    min = QMAX( min, p1 - pick( w->maximumSize() ) );

    QCOORD max = p1 - 1; //### no zero size widgets
    max = QMIN( max, p1 - pick( w->minimumSize() ) );
    max = QMIN( max, p0 + pick( prev->maximumSize() ) );

    p -= bord; // measure from prev->right
    p = QMAX( min, QMIN( p, max - 2*bord ) );

    if ( orient == Horizontal ) {
	w1->setGeometry( r.x(), r.y(), p, r.height() );
	p += 2*bord;
	w2->setGeometry( p, r.y(), r.width() - p + 1, r.height() );
	repaint( p - 2*bord , r.y(), 2*bord, r.height() );
    } else {
	w1->setGeometry( r.x(), r.y(), r.width(), p );
	p += 2*bord;
	w2->setGeometry( r.x(), p, r.width(), r.height() - p + 1 );
	repaint( r.x(), p - 2*bord, r.width(), 2*bord );
    }
}

void QSplitter::doResize()
{
    if ( !w1 || !w2 ) {
	QRect r = contentsRect();
	if ( w1 )
	    w1->setGeometry( r.x(), r.y(), r.width(), r.height() );
	else if ( w2 )
	    w2->setGeometry( r.x(), r.y(), r.width(), r.height() );
	return;
    }

    QCOORD p = r2p( ratio );
    //         pick(w1->size()) + bord; //current center of splitter.
    int r = ratio;
    moveSplitter( p );
    ratio = r; //resize should not affect the ratio set by the user
}


void QSplitter::recalc()
{
    if ( !w1 || !w2 ) {
	QRect r = contentsRect();
	if ( w1 ) {
	    setMaximumSize( w1->maximumSize() );
	    setMinimumSize( w1->minimumSize() );
	    w1->setGeometry( r.x(), r.y(), r.width(), r.height() );
	}
	else if ( w2 ){
	    setMaximumSize( w2->maximumSize() );
	    setMinimumSize( w2->minimumSize() );
	    w2->setGeometry(r.x(), r.y(), r.width(), r.height() );
	}
	return;
    }

    int maxl = pick(w1->maximumSize()) + pick(w2->maximumSize()) + bord*2;
    maxl = QMIN( maxl, QCOORD_MAX );
    int minl = pick(w1->minimumSize()) + pick(w2->minimumSize()) + bord*2;

    int maxt = QMIN( trans(w1->maximumSize()),trans(w2->maximumSize()) );
    int mint = QMAX( trans(w1->minimumSize()), trans(w2->minimumSize()) );

    if ( maxt < mint ) 
	maxt = mint;


    if ( orient == Horizontal ) {
	setMaximumSize( maxl, maxt );
	setMinimumSize( minl, mint );
    } else {
	setMaximumSize( maxt, maxl );
	setMinimumSize( mint, minl );
    }
    if ( ratio >= 0 ) {
	int r = ratio;
	moveSplitter( r2p(ratio) );
	ratio = r; //keep old ratio
    } else {
	moveSplitter( pick(size())/2 );
    }
}



