/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qslider.cpp#17 $
**
** Implementation of QSlider class
**
** Created : 961019
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qslider.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qtimer.h"
#include "qkeycode.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qslider.cpp#17 $");

static const int motifBorder = 2;
static const int motifLength = 30;
static const int motifThickness = 16;
static const int winLength = 10;
static const int winThickness = 20;



static const int thresholdTime = 500;
static const int repeatTime    = 100;

static int sliderStartVal = 0; //##### class member?


/*!
  \class QSlider qslider.h

  \brief The QSlider widget provides a vertical or horizontal slider.


  A slider is used to let the user control a value within a
  program-definable range. In contrast to a QScrollBar, the QSlider
  widget has a constant size slider and no arrow buttons.

  QSlider only offers integer ranges.

  The recommended thickness of a slider is given by sizeHint().


  A slider can be controlled by the keyboard, but it has a
  default focusPolicy() of \a NoFocus. Use setFocusPolicy() to
  enable keyboard focus.

  \ingroup realwidgets
*/

/*!
  Constructs a vertical slider.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QSlider::QSlider( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    orient = Vertical;
    init();
}

/*!
  Constructs a slider.

  The \e orientation must be QSlider::Vertical or QSlider::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QSlider::QSlider( Orientation orientation, QWidget *parent,
			const char *name )
    : QWidget( parent, name )
{
    orient = orientation;
    init();
}

/*!
  Constructs a slider.

  \arg \e minValue is the minimum slider value.
  \arg \e maxValue is the maximum slider value.
  \arg \e step is the page step value.
  \arg \e value is the initial value.
  \arg \e orientation must be QSlider::Vertical or QSlider::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QSlider::QSlider( int minValue, int maxValue, int step,
			int value,  Orientation orientation,
			QWidget *parent, const char *name )
    : QWidget( parent, name ),
      QRangeControl( minValue, maxValue, 1, step, value )
{
    orient = orientation;
    init();
}

void QSlider::init()
{
    timer = 0;
    track = TRUE;
    sliderPos = 0;
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
    setFocusPolicy( NoFocus );
    tickOffset = 0; //###
    tickmarksAbove = tickmarksBelow = FALSE;
}


/*!
  Enables slider tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the slider emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the slider emits the valueChanged() signal
  when the user releases the mouse button (unless the value happens to
  be the same as before).

  \sa tracking()
*/

void QSlider::setTracking( bool enable )
{
    track = enable;
}


/*!
  \fn bool QSlider::tracking() const
  Returns TRUE if tracking is enabled, or FALSE if tracking is disabled.

  Tracking is initially enabled.

  \sa setTracking()
*/


/*!
  \fn void QSlider::valueChanged( int value )
  This signal is emitted when the slider value is changed, with the
  new slider value as an argument.
*/

/*!
  \fn void QSlider::sliderPressed()
  This signal is emitted when the user presses the slider with the mouse.
*/

/*!
  \fn void QSlider::sliderMoved( int value )
  This signal is emitted when the slider is dragged, with the
  new slider value as an argument.
*/

/*!
  \fn void QSlider::sliderReleased()
  This signal is emitted when the user releases the slider with the mouse.
*/

/*!
  Calculates slider position corresponding to value \a v. Does not perform
  rounding.
 */
int QSlider::positionFromValue( int v ) const
{
    int  a = available();
    int range = maxValue() - minValue();
    return range > 0 ? ( (v - minValue() ) * a ) / (range): 0;
}
/*!
  Returns the available space in which the slider can move.
  */
int QSlider::available() const
{
    int a;
    switch ( style() ) {
    case WindowsStyle:
	a = (orient == Horizontal) ? width() - winLength
	    : height() - winLength;
	break;
    default:
    case MotifStyle:
	a = (orient == Horizontal) ? width() -motifLength - 2*motifBorder 
	    : height() - motifLength - 2*motifBorder;
	break;
    }
    return a;
}

/*!
  Calculates value corresponding to slider position \a p. Performs rounding.
 */
int QSlider::valueFromPosition( int p ) const
{
    int a = available();
    int range = maxValue() - minValue();
    return   minValue() + ( a > 0 ? (2 * p * range + a ) / ( 2*a ): 0 );
}

/*!
  Implements the virtual QRangeControl function.
*/
void QSlider::rangeChange()
{
    int newPos = positionFromValue( value() );
    if ( newPos != sliderPos ) {
	reallyMoveSlider( newPos ); 
    }
}

/*!
  Implements the virtual QRangeControl function.
*/
void QSlider::valueChange()
{
    if ( sliderVal != value() ) {
	int newPos = positionFromValue( value() );
	sliderVal = value();
	reallyMoveSlider( newPos );
    }
    emit valueChanged(value());
}


/*!
    Handles resize events for the slider.
*/

void QSlider::resizeEvent( QResizeEvent * )
{
    rangeChange();

    //#### put this into tickChange function...
    int thick;
    switch ( style() ) {
    case WindowsStyle:
	thick = winThickness;
	break;
    case MotifStyle:
    default:
	thick = motifThickness;
	break;
    }
    int space = (orient == Horizontal) ? height() : width();
    if ( tickmarksBelow == tickmarksAbove ) {
	tickOffset = ( space - thick ) / 2;
    } else if ( tickmarksAbove ) {
	tickOffset = space - thick;
    } else {
	tickOffset = 0;
    }
	
}


/*!
  Reimplements the virtual function QWidget::setPalette().

  Sets the background color to the mid color for Motif style sliders.
*/

void QSlider::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
}



/*!
  Sets the slider orientation.  The \e orientation must be
  QSlider::Vertical or QSlider::Horizontal.
  \sa orientation()
*/

void QSlider::setOrientation( Orientation orientation )
{
    orient = orientation;
    rangeChange();
    repaint();	//slightly inefficient...
}


/*!
  \fn Orientation QSlider::orientation() const
  Returns the slider orientation; QSlider::Vertical or
  QSlider::Horizontal.
  \sa setOrientation()
*/


/*!
  Returns the slider handle rectangle. (The actual moving-around thing.)
  */
QRect QSlider::sliderRect() const
{
    QRect r;
    switch ( style() ) {
    case WindowsStyle:
	if (orient == Horizontal )
	    r.setRect( sliderPos, tickOffset, 
		       winLength, winThickness  );
	else
	    r.setRect ( tickOffset, sliderPos,
			winThickness, winLength  );
	break;
    default:
    case MotifStyle:
	if (orient == Horizontal )
	    r.setRect ( sliderPos + motifBorder, tickOffset + motifBorder, 
			motifLength, motifThickness - 2 * motifBorder );
	else
	    r.setRect ( tickOffset + motifBorder, sliderPos + motifBorder, 
			motifThickness - 2 * motifBorder, motifLength );
	break;
    }
    return r;
}

/*!
  Paints the slider button using painter \a p with size and
  position given by \a r. Reimplement this function to change the
  look of the slider button.  
*/

void QSlider::paintSlider( QPainter *p, const QRect &r )
{
    QColorGroup g = colorGroup();
    QBrush fill( g.background() );

    switch ( style() ) {
    case WindowsStyle:
	if ( orient == Horizontal ) {
	    int d = r.width() / 4;
	    qDrawWinButton( p, r.x() , r.y() + d, r.width(), r.height() -2*d,
			    g, FALSE, &fill );
	} else {
	    int d = r.height() / 4;
	    qDrawWinButton( p, r.x() + d, r.y(), r.width() - 2*d, r.height(),
			    g, FALSE, &fill );
	}
	break;
    default:
    case MotifStyle:
	qDrawShadePanel( p, r, g, FALSE, 2, &fill );
	if ( orient == Horizontal ) {
	    QCOORD mid = ( r.left() + r.right() ) / 2;
	    qDrawShadeLine( p, mid,  r.top(), mid,  r.bottom() - 1,
			    g, TRUE, 1);
	} else {
	    QCOORD mid = ( r.top() + r.bottom() ) / 2;
	    qDrawShadeLine( p, r.left(), mid,  r.right() - 1, mid,
			    g, TRUE, 1);
	}
	break;
    }
}

/*!
  Performs the actual moving of the slider.
 */

void QSlider::reallyMoveSlider( int newPos )
{
    QRect updateR = sliderRect();
    sliderPos = newPos;
    updateR.unite( sliderRect() );
    repaint( updateR );
}



/*!
  Draws the "groove" on which the slider moves, using the painter \a p.
  \a c gives the distance from the top (or left) edge of the widget to
  the center of the groove.
*/

void QSlider::drawWinGroove( QPainter *p, QCOORD c )
{
    if ( orient == Horizontal ) {
	qDrawWinPanel( p, 0, c - 2,  width(), 4, colorGroup(), TRUE );
	p->setPen( black );
	p->drawLine( 1, c - 1, width() - 3, c - 1 );
    } else {
	qDrawWinPanel( p, c - 2, 0, 4, height(), colorGroup(), TRUE );
	p->setPen( black );
	p->drawLine( c - 1, 1, c - 1, height() - 3 );
    }
}



/*!
  Handles paint events for the slider.
*/

void QSlider::paintEvent( QPaintEvent * )
{
    //QRect paintRect = e->rect();
    
    QPainter p;
    p.begin( this );

    QRect sliderR = sliderRect();
    QColorGroup g = colorGroup();
    switch ( style() ) {
    case WindowsStyle:
	if ( hasFocus() ) {
	    QRect r;
	    if ( orient == Horizontal )
		r.setRect( 0, tickOffset, width(), winThickness );
	    else
		r.setRect( tickOffset, 0, winThickness, height() );
	    r = r.intersect( rect() );
	    qDrawPlainRect( &p, r, g.background() );
	    p.drawWinFocusRect( r );
	}
	drawWinGroove( &p, tickOffset + winThickness/2 );
	paintSlider( &p, sliderR );
	break;
    default:
    case MotifStyle:
	if ( orient == Horizontal ) {
	    qDrawShadePanel( &p, 0, tickOffset, width(), motifThickness,
			     g, TRUE );
	    p.fillRect( 0, 0, width(), tickOffset, g.background() );
	    p.fillRect( 0, tickOffset + motifThickness + 1,
			width(), height()/*###*/, g.background() ); 
	} else {
	    qDrawShadePanel( &p, tickOffset, 0, motifThickness, height(),
			     g, TRUE );
	    p.fillRect( 0, 0,  tickOffset, height(), g.background() );
	    p.fillRect( tickOffset + motifThickness + 1, 0,
			width()/*###*/, height(), g.background() ); 
	}

	if ( hasFocus() ) {
	    p.setPen( black );
	    if ( orient == Horizontal )
		p.drawRect(  1, tickOffset + 1, width() - 2, motifThickness - 2 );
	    else
		p.drawRect( tickOffset + 1, 1, motifThickness - 2, height() - 2 );
	}
	paintSlider( &p, sliderR );
	break;
    }
    p.end();
}

/*!
  Handles mouse press events for the slider.
 */
void QSlider::mousePressEvent( QMouseEvent *e )
{
    resetState();
    sliderStartVal = sliderVal;
    if ( e->button() == MidButton ) {
	int pos = goodPart( e->pos() );
	moveSlider( pos - slideLength() / 2 );
	return;
    }
    if ( e->button() != LeftButton )
	return;
    QRect r = sliderRect();

    if ( r.contains( e->pos() ) ) {
	state = Dragging;
	clickOffset = (QCOORD)( goodPart( e->pos() ) - sliderPos );
	emit sliderPressed();
    } else if ( style()   == WindowsStyle) {
	int pos = goodPart( e->pos() );
	moveSlider( pos - slideLength() / 2 );
	state = Dragging;
	clickOffset = slideLength() / 2;     
    } else if ( orient == Horizontal && e->pos().x() < r.left() //### goodPart
		|| orient == Vertical && e->pos().y() < r.top() ) {
	state = TimingDown;
        subtractPage();
	if ( !timer )
	    timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), SLOT(repeatTimeout()) );
	timer->start( thresholdTime, TRUE ); 
    } else if ( orient == Horizontal && e->pos().x() > r.right() //### goodPart
		|| orient == Vertical && e->pos().y() > r.bottom() ) {
	state = TimingUp;
	addPage();
	if ( !timer )
	    timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), SLOT(repeatTimeout()) );
	timer->start( thresholdTime, TRUE ); 
    }
}

/*!
  Handles mouse move events for the slider.
*/

void QSlider::mouseMoveEvent( QMouseEvent *e )
{
    
    if ( style() == WindowsStyle ) {
	QRect r = rect();
	if ( orientation() == Horizontal )
	    r.setRect( r.x() - 20, r.y() - 30, r.width() + 40, r.height() + 60 );
	else
	    r.setRect( r.x() - 30, r.y() - 20, r.width() + 60, r.height() + 40 );
	if ( !r.contains( e->pos() ) ) { 
	    moveSlider( positionFromValue( sliderStartVal) );
	    return;
	}
    }

    if ( (e->state() & MidButton) ) { 		// middle button wins
	int pos = goodPart( e->pos() );
	moveSlider( pos - slideLength() / 2 );
	return;	
    }
    if ( !(e->state() & LeftButton) )
	return;					// left mouse button is up
    if ( state != Dragging )
	return;

    int pos = goodPart( e->pos() );
    
    moveSlider( pos - clickOffset );
}


/*!
  Handles mouse release events for the slider.
*/

void QSlider::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !(e->state() & LeftButton) )
	return;					// left mouse button is up
    resetState();
}

/*!
  Moves the left (or top) edge of the slider to position 
  \a pos. Performs snapping.
*/

void QSlider::moveSlider( int pos )
{
    int  a = available();
    int newPos = QMIN( a, QMAX( 0, pos ) );
    int newVal = valueFromPosition( newPos );
    if ( sliderVal != newVal ) {
	sliderVal = newVal;
	emit sliderMoved( sliderVal );
    }
    if ( tracking() && sliderVal != value() ) {
	directSetValue( sliderVal );
	emit valueChanged( sliderVal );
    }

    switch ( style() ) {
    case WindowsStyle:
	newPos = positionFromValue( newVal );
	break;
    default:
    case MotifStyle:
	break;
    }	    

    if ( sliderPos != newPos )
	reallyMoveSlider( newPos );
}


/*!
  Resets all state information and stops my timer.
*/

void QSlider::resetState()
{
    if ( timer ) {
	timer->stop();
	timer->disconnect();
    }
    switch ( state ) {
    case TimingUp:
    case TimingDown:
	break;
    case Dragging: {
	setValue( valueFromPosition( sliderPos ) );
	emit sliderReleased();
	break;
    }
    case None:
	break;
    default:
	warning("QSlider: in wrong state");
    }
    state = None;
}


/*!
  Handles key press events for the slider.
*/

void QSlider::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Left:
	if ( orient == Horizontal )
	    subtractLine();
	break;
    case Key_Right:
	if ( orient == Horizontal )
	    addLine();
	break;
    case Key_Up:
	if ( orient == Vertical )
	    subtractLine();
	break;
    case Key_Down:
	if ( orient == Vertical )	
	    addLine();
	break;
    case Key_Prior:
	subtractPage();
	break;
    case Key_Next:
	addPage();
	break;
    case Key_Home:
	setValue( minValue() );
	break;
    case Key_End:
	setValue( maxValue() );
	break;
    default:
	e->ignore();
	return;
    }
    e->accept();
}


/*!
  Returns the length of the slider.
*/

int QSlider::slideLength() const 
{
    switch ( style() ) {
    case WindowsStyle:
	return winLength;
    default:
    case MotifStyle:
	return motifLength;
    }
}



/*!
  Makes QRangeControl::setValue() available as a slot.
*/

void QSlider::setValue( int value )
{
    QRangeControl::setValue( value );
}


/*!
  Moves the slider one pageStep() upwards.
*/

void QSlider::pageUp()
{
    addPage();
}


/*!
  Moves the slider one pageStep() downwards.
*/

void QSlider::pageDown()
{
    subtractPage();
}


/*!
  Waits for autorepeat.
*/

void QSlider::repeatTimeout()
{
    ASSERT( timer );
    timer->disconnect();
    if ( state == TimingDown )
	connect( timer, SIGNAL(timeout()), SLOT(pageDown()) );
    else if ( state == TimingUp )
	connect( timer, SIGNAL(timeout()), SLOT(pageUp()) );
    timer->start( repeatTime, FALSE );   
}


/*!
  Returns the relevant dimension of \a p.
*/

int QSlider::goodPart( const QPoint &p )
{
    return (orient == Horizontal) ?  p.x() : p.y();
}


/*!  

  Returns the recommended size of the slider. Only the thickness is
  relevant.

*/

QSize QSlider::sizeHint() const
{
    const int length = 42;
    int thickness;
    switch ( style() ) {
    case WindowsStyle:
	thickness = winThickness;
	break;
    case MotifStyle:
    default:
	thickness = motifThickness;
	break;
    }
    if ( orient == Horizontal )
	return QSize( length, thickness );
    else
	return QSize( thickness, length );
}
