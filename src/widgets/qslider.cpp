/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qslider.cpp#3 $
**
** Implementation of QSlider class
**
** Created : 961020
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qslider.h"
#include "qpainter.h"
#include "qdrawutl.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qslider.cpp#3 $");

#define SLIDE_BORDER	2
#define SLIDE_WIDTH	30
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

  The recommended thickness of a slider is 16 pixels.

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
    timerId   = 0;
    track     = TRUE;
    sliderPos = 0;
    if ( style() == MotifStyle )
	setBackgroundColor( colorGroup().mid() );
}


/*!
  \fn void QSlider::setTracking( bool enable )
  Enables slider tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the slider emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the slider emits the valueChanged() signal
  when the user relases the mouse button (unless the value happens to
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
    int  available = (orient == Horizontal) ? width() -SLIDE_WIDTH - 2*SLIDE_BORDER : height() - SLIDE_WIDTH - 2*SLIDE_BORDER;
    int range = maxValue() - minValue();
    return range > 0 ? ( v * available ) / (range): 0;
}

/*!
  Calculates value corresponding to slider position \a p. Performs rounding.
 */
int QSlider::valueFromPosition( int p ) const
{
    int  available = (orient == Horizontal) ? width() -SLIDE_WIDTH - 2*SLIDE_BORDER : height() - SLIDE_WIDTH - 2*SLIDE_BORDER; 
    int range = maxValue() - minValue();
    return available > 0 ? (2 * p * range + available ) / (2*available): 0;
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
    if (orient == Horizontal )
	return QRect ( SLIDE_BORDER + sliderPos, SLIDE_BORDER, 
		       SLIDE_WIDTH, height() - 2 * SLIDE_BORDER );
    else
	return QRect ( SLIDE_BORDER, SLIDE_BORDER + sliderPos, 
		       width() - 2 * SLIDE_BORDER, SLIDE_WIDTH );
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
  Removes the old slider and draws the new, with a minimum of flickering.
 */

void QSlider::paintSlider( int oldPos, int newPos )
{
    QPainter p;
    p.begin( this );

    QColorGroup g = colorGroup();
    QRect sliderR;
    if (orient == Horizontal )
	sliderR = QRect ( SLIDE_BORDER  + newPos, SLIDE_BORDER  , 
			  SLIDE_WIDTH, height() - 2 * SLIDE_BORDER );
    else
	sliderR = QRect ( SLIDE_BORDER, SLIDE_BORDER + newPos,
			  width() - 2 * SLIDE_BORDER, SLIDE_WIDTH );

    switch ( style() ) {
    case WindowsStyle:
    default:
    case MotifStyle:
	QBrush fill( g.background() );
	qDrawShadePanel( &p, sliderR, g, FALSE, 2, &fill );
	if ( orient == Horizontal ) {
	    QCOORD mid = ( sliderR.left() + sliderR.right() ) / 2;
	    qDrawShadeLine( &p, mid,  sliderR.top(), mid,  sliderR.bottom() - 1,
			    g, TRUE, 1);
	} else {
	    QCOORD mid = ( sliderR.top() + sliderR.bottom() ) / 2;
	    qDrawShadeLine( &p, sliderR.left(), mid,  sliderR.right() - 1, mid,
			    g, TRUE, 1);
	}
	int c,d;
	if ( oldPos < newPos ) {
	    c = oldPos + SLIDE_BORDER;
	    d = newPos - oldPos;
	} else {
	    c = newPos + SLIDE_WIDTH + SLIDE_BORDER;
	    d = oldPos - newPos;
	}
	if ( orient == Horizontal )
	    p.fillRect( c, SLIDE_BORDER, d, 
			height() - 2*SLIDE_BORDER, backgroundColor() );
	else
	    p.fillRect( SLIDE_BORDER, c, 
			width() - 2*SLIDE_BORDER, d, backgroundColor() ); 
    }
    p.end();
}





/*!
  Handles paint events for the slider.
*/

void QSlider::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin( this );
    qDrawShadePanel( &p, rect(), colorGroup(), TRUE );

    QRect sliderR = sliderRect();
    QColorGroup g = colorGroup();

    switch ( style() ) {
    case WindowsStyle:
    default:
    case MotifStyle:
	QBrush fill( g.background() );
	qDrawShadePanel( &p, sliderR, g, FALSE, 2, &fill );
	if ( orient == Horizontal ) {
	        QCOORD mid = ( sliderR.left() + sliderR.right() ) / 2;
		qDrawShadeLine( &p, mid,  sliderR.top(), mid,  sliderR.bottom() - 1,
				g, TRUE, 1);
	} else {
	        QCOORD mid = ( sliderR.top() + sliderR.bottom() ) / 2;
		qDrawShadeLine( &p, sliderR.left(), mid,  sliderR.right() - 1, mid,
				g, TRUE, 1);
	}
    }
    p.end();
}

/*!
  Handles mouse press events for the slider.
 */
void QSlider::mousePressEvent( QMouseEvent *e )
{
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
    if ( !(e->state() & LeftButton) )
	return;					// left mouse button is up
    if ( state != Dragging )
	return;
    int  available = (orient == Horizontal) ? width() - SLIDE_WIDTH - 2*SLIDE_BORDER : height() - SLIDE_WIDTH - 2*SLIDE_BORDER;
    int pos = (orient == Horizontal) ?  e->pos().x(): e->pos().y();
    int oldPos = sliderPos;
    sliderPos = QMIN( available, QMAX( 0, pos - clickOffset) );
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
	debug("QSlider::mouseReleaseEvent, not doing anything");
	break;
    default:
	warning("QSlider::mouseReleaseEvent, wrong state");
    }
    state = None;
}


