/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsplitter.cpp#8 $
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
#include "qbitmap.h"


//#############################################

#define split_width 32
#define split_height 32
static unsigned char split_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00,
  0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00,
  0x00, 0x82, 0x82, 0x00, 0x00, 0x83, 0x82, 0x01, 0x80, 0xff, 0xfe, 0x03,
  0x00, 0x83, 0x82, 0x01, 0x00, 0x82, 0x82, 0x00, 0x00, 0x80, 0x02, 0x00,
  0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00,
  0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static unsigned char splitm_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xc0, 0x07, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0xc0, 0x07, 0x00,
  0x00, 0xc0, 0x07, 0x00, 0x00, 0xc4, 0x47, 0x00, 0x00, 0xc6, 0xc7, 0x00,
  0x00, 0xc7, 0xc7, 0x01, 0x80, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x07,
  0x80, 0xff, 0xff, 0x03, 0x00, 0xc7, 0xc7, 0x01, 0x00, 0xc6, 0xc7, 0x00,
  0x00, 0xc4, 0x47, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0xc0, 0x07, 0x00,
  0x00, 0xc0, 0x07, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

#define vsplit_width 32
#define vsplit_height 32
static unsigned char vsplit_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0x80, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00,
  0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

#define vsplitm_width 32
#define vsplitm_height 32
static unsigned char vsplitm_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
  0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
  0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
  0x00, 0xc0, 0x01, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
  0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
  0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
  0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xe0, 0x03, 0x00,
  0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };


static QCursor *vSplitterCur = 0;
static QCursor *hSplitterCur = 0;



class QInternalSplitter : public QWidget
{
public:
    QInternalSplitter( QSplitter::Orientation o, 
		       QSplitter *parent, const char *name=0 );
    void setOrientation( QSplitter::Orientation o );
    QSplitter::Orientation orientation() const { return orient; }

protected:
    //    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );


private:
    QSplitter::Orientation orient;
    QSplitter *s;
};

QInternalSplitter::QInternalSplitter( QSplitter::Orientation o, 
				      QSplitter *parent, const char *name=0 )
    : QWidget( parent, name ) 
{ 
    if ( !hSplitterCur )
	hSplitterCur = new QCursor( QBitmap( split_width, split_height,
					     split_bits, TRUE),
				 QBitmap( split_width, split_height,
					  splitm_bits, TRUE) );
    if ( !vSplitterCur )
	vSplitterCur = new QCursor( QBitmap( vsplit_width, vsplit_height,
					     vsplit_bits, TRUE),
				 QBitmap( vsplit_width, vsplit_height,
					  vsplitm_bits, TRUE) );
    orient = o;
    s = parent; 
    if ( o == QSplitter::Horizontal )
	setCursor( *hSplitterCur );
    else
	setCursor( *vSplitterCur );
}

#if 0
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
#endif


void QInternalSplitter::setOrientation( QSplitter::Orientation o )
{
    orient = o; 
    if ( o == QSplitter::Horizontal )
	setCursor( *hSplitterCur );
    else if ( vSplitterCur )
	setCursor( *vSplitterCur );
}

void QInternalSplitter::mouseMoveEvent( QMouseEvent *e )
{
    s->moveTo( mapToParent( e->pos() ));
}
void QInternalSplitter::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton )
	s->startMoving();
}
void QInternalSplitter::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton )
	s->stopMoving();
}

void QInternalSplitter::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    s->drawSplitter( &p, 0, 0, width(), height() ); 
}



/*!
  \class QSplitter qsplitter.h
  \brief QSplitter implements a splitter widget.

  A splitter lets the user control the size of two child widgets.

  setFirstWidget() and setSecondWidget() specifies the two widgets.

 */

static int opaqueOldPos = -1; //### there's only one mouse, but this is a bit risky

/*!
  Creates a horizontal splitter.
  */

QSplitter::QSplitter( QWidget *parent, const char *name )
    :QFrame(parent,name,WPaintUnclipped)
{
     orient = Horizontal;
     init();
}
/*!
  Creates splitter with orientation \a o.
  */

QSplitter::QSplitter( Orientation o, QWidget *parent, const char *name )
    :QFrame(parent,name,WPaintUnclipped)
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
    fixedWidget = 0;
    opaque = 0;

    d = new QInternalSplitter( orient, this );

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
    if ( w1 )
	w1->hide();
    if ( w->parentWidget() != this ) {
	w->recreate( this, 0, QPoint(0,0) );
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
    if ( w2 )
	w1->hide();
    if ( w->parentWidget() != this ) {
	w->recreate( this, 0, QPoint(0,0) );
    }
    w2 = w;
    if ( w2 == w1 )
	w1 = 0;
    recalc();
}

/*!
  Sets the orientation to \a o.
   \sa orientation()
 */
void QSplitter::setOrientation( Orientation o )
{
    if ( orient == o )
	return;
    orient = o;
    d->setOrientation( o );
    recalc();
}

/*!
   \fn QSplitter::Orientation QSplitter::orientation() const

   Returns the orientation (\c Horizontal or \c Vertical) of the splitter.
   \sa setOrientation()
   */


QCOORD QSplitter::r2p( int r ) const
{
    int s = pick(contentsRect().size());
    if ( fixedWidget ) 
	return fixedWidget == 1 ? r : s - r;
    else
	return ( s * r) / 256;
}

int QSplitter::p2r( QCOORD p ) const
{
    int s = pick(contentsRect().size());
    if ( fixedWidget )
	return fixedWidget == 1 ? p : s - p;
    else
	return s ? ( p * 256 ) / s : 128;
}



/*!
  Reimplemented to provide childRemoveEvent(), childInsertEvent() and
  layoutHintEvent()  without breaking binary compatibility.
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
	layoutHintEvent( e );
	return TRUE;
	break;
    default:
	return QWidget::event( e );
    }
}

void QSplitter::resizeEvent( QResizeEvent * )
{
    doResize();
}



void QSplitter::leaveEvent( QEvent * )
{
    //    setCursor( arrowCursor );
}


/*!
  Tells the splitter that a child widget has been removed.
 */
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

/*!
  Tells the splitter that a child widget has been inserted.
 */
void QSplitter::childInsertEvent( QChildEvent * )
{
}



/*!
  Tells the splitter that a child widget has changed layout parameters
*/

void QSplitter::layoutHintEvent( QEvent * )
{
    recalc();    
}



void QSplitter::stopMoving()
{
    moving = 0;
    if ( !opaque && opaqueOldPos >= 0 ) {
	int p = opaqueOldPos;
	setRubberband( -1 );
	moveSplitter( p );
    }
}

void QSplitter::startMoving()
{
    moving = TRUE;
}

void QSplitter::moveTo( QPoint mp )
{
    if ( moving ) {
	int p = adjustPos( pick( mp ) );
	if ( opaque )
	    moveSplitter( p );
	else
	    setRubberband( p );
    } else {
    }
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
	    QCOORD kSize = bord*2 - 2;

	    qDrawShadeLine( p, xPos, kPos + kSize - 1 ,
			    xPos, h, colorGroup() );
	    qDrawShadePanel( p, xPos-bord+1, kPos,
			     kSize, kSize, colorGroup() );
	    qDrawShadeLine( p, xPos, 0, xPos, kPos ,colorGroup() );
	} else {
	    QCOORD yPos = y + h/2;
	    QCOORD kPos = w - motifOffset - 2*bord;
	    QCOORD kSize = bord*2 - 2;

	    qDrawShadeLine( p, 0, yPos, kPos, yPos, colorGroup() );
	    qDrawShadePanel( p, kPos, yPos-bord+1,
			     kSize, kSize, colorGroup() );
	    qDrawShadeLine( p, kPos + kSize -1, yPos,
			    w, yPos, colorGroup() );
	}
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
    QRect r = contentsRect();
    if ( orient == Horizontal ) {
	w1->setGeometry( r.x(), r.y(), p, r.height() );
	p += 2*bord;
	w2->setGeometry( p, r.y(), r.width() - p + 1, r.height() );
	d->setGeometry( p - 2*bord , r.y(), 2*bord, r.height() );
    } else {
	w1->setGeometry( r.x(), r.y(), r.width(), p );
	p += 2*bord;
	w2->setGeometry( r.x(), p, r.width(), r.height() - p + 1 );
	d->setGeometry( r.x(), p - 2*bord, r.width(), 2*bord );
    }
}




/*!
  Returns the legal position of the splitter closest to \a p, and sets the
  chosen ratio.
*/

int QSplitter::adjustPos( int p )
{
    ratio = p2r( p );

    QRect r = contentsRect();

    QCOORD p0 = pick( r.topLeft() );
    QCOORD p1 = pick( r.bottomRight() );

    QCOORD min = p0 + 1; //### no zero size widgets
    min = QMAX( min, p0 + pick( w1->minimumSize() ) );
    min = QMAX( min, p1 - pick( w2->maximumSize() ) );

    QCOORD max = p1 - 1; //### no zero size widgets
    max = QMIN( max, p1 - pick( w2->minimumSize() ) );
    max = QMIN( max, p0 + pick( w1->maximumSize() ) );

    p -= bord; // measure from prev->right
    p = QMAX( min, QMIN( p, max - 2*bord ) );    

    return p;
}


/*!
  Shows a rubber band at position \a p. If \a p is negative, the rubber band is removed.
*/

void QSplitter::setRubberband( int p )
{
    QPainter paint( this );
    paint.setPen( gray );
    paint.setBrush( gray );
    paint.setRasterOp( XorROP );
    QRect r = contentsRect();
    const int rBord = 3; //###

    if ( orient == Horizontal ) {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( opaqueOldPos + bord - rBord , r.y(), 
			    2*rBord, r.height() );
	if ( p >= 0 )
	    paint.drawRect( p  + bord - rBord, r.y(), 2*rBord, r.height() );
    } else {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( r.x(), opaqueOldPos + bord - rBord,
			    r.width(), 2*rBord );
	if ( p >= 0 )
	    paint.drawRect( r.x(), p + bord - rBord, r.width(), 2*rBord );
    }
    opaqueOldPos = p;
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
    moveSplitter( adjustPos( p ) );
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
	moveSplitter( adjustPos( r2p(ratio) ) );
	ratio = r; //keep old ratio
    } else {
	moveSplitter( adjustPos( pick(size())/2 ) );
    }
}





/*!
  Sets the proportion of available space allocated for the first widget to
  \a f.  A value of 0.5 means equal space for the two widgets, 
  1.0 means the second widget receives all the space.

  The user can change the ratio by adjusting  the splitter.

  The widgets' minimum/maximum sizes, if any, have precedence over the ratio.

  \sa setFixed()
*/

void QSplitter::setRatio( float f )
{
    fixedWidget = 0;
    ratio = (int)(f*256);
    doResize();
}




/*!
  Sets the size of widget number \a w to \a size. \a w must be 1 or 2. 
  The specified widget will have a fixed size. 
  The user can change the fixed size by adjusting  the splitter.

  The widgets' minimum/maximum sizes, if any, have precedence over the size
  set.

  \sa setRatio()
*/

void QSplitter::setFixed( int w, int size )
{
    if ( w > 2 || w < 1 ) {
#ifdef CHECK_RANGE
	debug( "QSplitter::setFixed(), unsupported widget number %d "
	       "(must be 1 or 2).", w );
#endif	
	return;
    }
    fixedWidget = w;
    ratio = size;
    doResize();
}


/*!
  \fn bool QSplitter::opaqueResize() const 
  
  Returns TRUE if opaque resize is on, FALSE otherwise.

  \sa setOpaqueResize()
*/

/*!
  Sets opaque resize to \a on. Opaque resize is initially turned off.

  \sa opaqueResize()
*/

void QSplitter::setOpaqueResize( bool on )
{
    opaque = on;
}

QWidget * QSplitter::splitterWidget()
{
    return (QWidget*)d;
}
