/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qslider.cpp#11 $
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
#include "qkeycode.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qslider.cpp#11 $");

#define SLIDE_BORDER	2
#define MOTIF_WIDTH	30
#define WIN_WIDTH	10
//#define WIN_LENGTH	20

static const int thresholdTime = 500;
static const int repeatTime    = 100;


/*!
  \class QSlider qslider.h

  \brief The QSlider widget provides a vertical or horizontal slider
  (stripped down scrollbar).

  A slider is used to let the user control a value within a
  program-definable range. In contrast to a QScrollBar, the QSlider
  widget has a constant size slider and no arrow buttons.

  QSlider only offers integer ranges.

  The recommended thickness of a slider is given by sizeHint().


  A slider can be controlled by the keyboard, but it has a
  default focusPolicy() of \a NoFocus. Use setFocusPolicy() to
  enable keyboard focus.

  \ingroup realwidgets
  \internal
  sizeHint is not implemented yet!

  WinStyle is not finished! 
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
    timerId   = 0;
    track     = TRUE;
    sliderPos = 0;
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
    setFocusPolicy( NoFocus );
}


/*!
  \fn void QSlider::setTracking( bool enable )
  Enables slider tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the slider emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the slider emits the valueChanged() signal
  when the user releases the mouse button (unless the value happens to
  be the same as before).

  \sa tracking()
*/

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
    return range > 0 ? ( v * a ) / (range): 0;
}
/*!
  Returns the available space in which the slider can move.
  */
int QSlider::available() const
{
    int a;
    switch ( style() ) {
    case WindowsStyle:
	a = (orient == Horizontal) ? width() - WIN_WIDTH
	    : height() - WIN_WIDTH;
	break;
    default:
    case MotifStyle:
	a = (orient == Horizontal) ? width() -MOTIF_WIDTH - 2*SLIDE_BORDER 
	    : height() - MOTIF_WIDTH - 2*SLIDE_BORDER;
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
    return a > 0 ? (2 * p * range + a ) / ( 2*a ): 0;
}

/*!
  Implements the virtual QRangeControl function.
*/
void QSlider::rangeChange()
{
    int newPos = positionFromValue( value() );
    if ( newPos != sliderPos ) {
	paintSlider( sliderPos, newPos );
	sliderPos = newPos;
    }
}

/*!
  Implements the virtual QRangeControl function.
*/
void QSlider::valueChange()
{
    if ( sliderVal != value() ) {
	int oldPos = sliderPos;
	sliderPos = positionFromValue( value() );
	sliderVal = value();
	paintSlider( oldPos, sliderPos );
    }
    emit valueChanged(value());
}


/*!
    Handles resize events for the slider.
*/

void QSlider::resizeEvent( QResizeEvent * )
{
    rangeChange();
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
  Returns the slider rectangle.
  */
QRect QSlider::sliderRect() const
{
    switch ( style() ) {
    case WindowsStyle:
	if (orient == Horizontal )
	    return QRect ( sliderPos, 0, 
			   WIN_WIDTH, height()  );
	else
	    return QRect ( 0, sliderPos,
			   width(), WIN_WIDTH  );
	break;
    default:
    case MotifStyle:
	if (orient == Horizontal )
	    return QRect ( SLIDE_BORDER + sliderPos, SLIDE_BORDER, 
			   MOTIF_WIDTH, height() - 2 * SLIDE_BORDER );
	else
	    return QRect ( SLIDE_BORDER, SLIDE_BORDER + sliderPos, 
			   width() - 2 * SLIDE_BORDER, MOTIF_WIDTH );
	break;
    }
}

/*
  Handles slider auto-repeat.
  */
void QSlider::timerEvent( QTimerEvent *t )
{
    if ( t->timerId() != timerId )
	return; // Hmm, someone must have inherited us...
    switch ( state ) {
    case TimingDown:
	subtractPage();
	break;
    case TimingUp:
	addPage();
	break;
    default:
	warning("QSlider::timerEvent, bad state");
    }
    killTimer( timerId );
    timerId = startTimer( repeatTime );
}



/*!
  Paints the slider button using painter \a p with size and
  posistion given by \a r. Reimplement this function to change the
  look of the slider button.  
*/

void QSlider::paintSlider( QPainter *p, const QRect &r )
{
    QColorGroup g = colorGroup();
    QBrush fill( g.background() );

    switch ( style() ) {
    case WindowsStyle:
	qDrawWinButton( p, r, g, FALSE, &fill );
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
  Removes the old slider and draws the new, with a minimum of flickering.
 */

void QSlider::paintSlider( int oldPos, int newPos )
{
    QPainter p;
    p.begin( this );

    QColorGroup g = colorGroup();
    QRect sliderR = sliderRect();
    int c,d;
    //### a bit wasteful if the slider moves more than one slider width
    switch ( style() ) {
    case WindowsStyle:
	d = newPos - oldPos;
	if ( oldPos < newPos ) {
	    c = oldPos;
	} else {
	    c = newPos + WIN_WIDTH;
	}
	if ( orient == Horizontal )
	    p.fillRect( c, 0, d, height(), backgroundColor() );
	else
	    p.fillRect( 0, c, width(), d, backgroundColor() ); 
	drawWinBackground( &p, g );
	paintSlider( &p, sliderR );
	break;
    default:
    case MotifStyle:
	d = newPos - oldPos;
	if ( oldPos < newPos ) {
	    c = oldPos + SLIDE_BORDER;
	} else {
	    c = newPos + MOTIF_WIDTH + SLIDE_BORDER;
	}
	if ( orient == Horizontal )
	    p.fillRect( c, SLIDE_BORDER, d, 
			height() - 2*SLIDE_BORDER, backgroundColor() );
	else
	    p.fillRect( SLIDE_BORDER, c, 
			width() - 2*SLIDE_BORDER, d, backgroundColor() ); 
	paintSlider( &p, sliderR );
	break;
    }
    p.end();
}



/*!
  Draws the "groove" on which the slider moves.
*/

void QSlider::drawWinBackground( QPainter *p, QColorGroup &g )
{
	if ( orient == Horizontal ) {
	    qDrawWinPanel( p, 0, height()/2 - 2,  width(), 4 , g, TRUE );
	    p->setPen( black );
	    p->drawLine( 1, height()/2 - 1, width() - 3, height()/2 - 1 );
	} else {
	    qDrawWinPanel( p, width()/2 - 2, 0, 4, height(), g, TRUE );
	    p->setPen( black );
	    p->drawLine( width()/2 - 1, 1, width()/2 - 1, height() - 3 );
	}
}



/*!
  Handles paint events for the slider.
*/

void QSlider::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin( this );

    QRect sliderR = sliderRect();
    QColorGroup g = colorGroup();
    switch ( style() ) {
    case WindowsStyle:
	if ( hasFocus() )
	    p.drawWinFocusRect( 0, 0, width(), height() );
	drawWinBackground( &p, g );
	paintSlider( &p, sliderR );
	break;
    default:
    case MotifStyle:
	qDrawShadePanel( &p, rect(), colorGroup(), TRUE );
	if ( hasFocus() ) {
	    p.setPen( black );
	    p.drawRect(  1, 1, width() - 2, height() - 2 );
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
    if ( e->button() == MidButton ) {
	int pos = (orient == Horizontal) ?  e->pos().x(): e->pos().y();
	moveSlider( pos - slideWidth() / 2 );
	return;
    }
    if ( e->button() != LeftButton )
	return;
    QRect r = sliderRect();

    if ( r.contains( e->pos() ) ) {
	state = Dragging;
	clickOffset = (QCOORD)( ( (orient == Horizontal) ?
				  e->pos().x() : e->pos().y())
				- sliderPos );
	emit sliderPressed();
    } else if ( orient == Horizontal && e->pos().x() < r.left() 
		|| orient == Vertical && e->pos().y() < r.top() ) {
	state = TimingDown;
        subtractPage();
	timerId = startTimer( thresholdTime );
    } else if ( orient == Horizontal && e->pos().x() > r.right() 
		|| orient == Vertical && e->pos().y() > r.bottom() ) {
	state = TimingUp;
	addPage();
	timerId = startTimer( thresholdTime );
    }
}

/*!
  Handles mouse move events for the slider.
*/

void QSlider::mouseMoveEvent( QMouseEvent *e )
{
    if ( (e->state() & MidButton) ) { 		// middle button wins
	int pos = (orient == Horizontal) ?  e->pos().x(): e->pos().y();
	moveSlider( pos - slideWidth() / 2 );
	return;	
    }
    if ( !(e->state() & LeftButton) )
	return;					// left mouse button is up
    if ( state != Dragging )
	return;
    int  a = available();
    int pos = (orient == Horizontal) ?  e->pos().x(): e->pos().y();
    int oldPos = sliderPos;
    sliderPos = QMIN( a, QMAX( 0, pos - clickOffset) );
    int newVal = valueFromPosition( sliderPos );
    if ( sliderVal != newVal ) {
	sliderVal = newVal;
	emit sliderMoved( sliderVal );
    }
    if ( tracking() && sliderVal != value() ) {
	directSetValue( sliderVal );
	emit valueChanged( sliderVal );
    }
    paintSlider( oldPos, sliderPos );
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
  \a pos.
*/

void QSlider::moveSlider( int pos )
{
        int  a = available();
	int oldPos = sliderPos;
	sliderPos = QMIN( a, QMAX( 0, pos ) );
	int newVal = valueFromPosition( sliderPos );
	if ( sliderVal != newVal ) {
	    sliderVal = newVal;
	    emit sliderMoved( sliderVal );
	}
	if ( sliderVal != value() ) {
	    directSetValue( sliderVal );
	    emit valueChanged( sliderVal );
	}
	paintSlider( oldPos, sliderPos );

}


/*!
  Resets all state information and kills my timer.
*/

void QSlider::resetState()
{
    switch ( state ) {
    case TimingUp:
    case TimingDown:
	killTimer( timerId );
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
	    setValue( value() - 1 );
	break;
    case Key_Right:
	if ( orient == Horizontal )
	    setValue( value() + 1 );
	break;
    case Key_Up:
	if ( orient == Vertical )
	    setValue( value() - 1 );
	break;
    case Key_Down:
	if ( orient == Vertical )
	    setValue( value() + 1 );
	break;
    case Key_Home:
	setValue( minValue() );
	break;
    case Key_End:
	setValue( maxValue() );
	break;
    default:
	e->ignore();
	break;
    }
}


/*!
  Returns the width of the slider.
*/

int QSlider::slideWidth() const 
{
    switch ( style() ) {
    case WindowsStyle:
	return WIN_WIDTH;
    default:
    case MotifStyle:
	return MOTIF_WIDTH;
    }
}

