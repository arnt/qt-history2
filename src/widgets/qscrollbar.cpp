/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollbar.cpp#141 $
**
** Implementation of QScrollBar class
**
** Created : 940427
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qscrollbar.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qtimer.h"
#include <limits.h>

// NOT REVISED
/*!
  \class QScrollBar qscrollbar.h

  \brief The QScrollBar widget provides a vertical or horizontal scroll bar.

  \ingroup realwidgets

  A scroll bar is used to let the user control a value within a
  program-definable range, and to give the user visible indication of
  the current value of a \link QRangeControl range control \endlink.

  Scroll bars include four separate controls, in order: <ul> <li> The
  \e line-up control is a little triangle with which the user can move
  one line up.  The meaning of line is configurable. In e.g. editors
  and list boxes means one line of text.  <li> The \e slider is the
  handle that indicates the current value of the scroll bar, and which
  the user can drag to change the value.  <li> The \a page-up/down
  control is the area on which the slider slides (the scroll bar's
  background).  Clicking here moves the scroll bar up or down one
  page.  The meaning of page too is configurable - in editors and list
  boxes it means as many lines as there is space for in the widget.
  <li> Finally, the line-down control is the arrow on the other end of
  the scroll bar.  Clicking there moves the scroll bar down/rightwards
  one line.</ul>

  QScrollBar has not much of an API of its own; it mostly relies on
  QRangeControl.  The most useful functions are setValue() to set the
  scrollbar directly to some value; addPage(), addLine(), subtractPage()
  and subtractLine() to simulate the effects of clicking (neat for
  accelerator keys; setSteps() to define the values of pageStep() and
  lineStep(); and last but NOT least setRange() to set the minValue()
  and maxValue() of the scrollbar.  (QScrollBar has a convenience
  constructor with which you can set most of that.)

  Some GUI styles, for example the provided Windows and Motif styles,
  also use the pageStep() value to calculate the size of the sliding
  thumb (scroll indicator).

  In addition to the access functions from QRangeControl, QScrollBar
  has a comprehensive set of signals: <ul>

  <li> valueChanged() - emitted when the scroll bar's value has changed.

  <li> sliderPressed() - emitted when the user starts to drag the
  slider

  <li> sliderMoved() - emitted when the user drags the slider

  <li> sliderReleased() - emitted when the user releases the slider

  <li> nextLine() - emitted when the scroll bar has moved one line
  down/rightwards.  Line is defined in QRangeControl.

  <li> prevLine() - emitted when the scroll bar has moved one line
  up/leftwards.

  <li> nextPage() - emitted when the scroll bar has moved one page
  down/rightwards.

  <li> prevPage() - emitted when the scroll bar has moved one page
  up/leftwards.

  </ul>

  QScrollBar only offers integer ranges.  Note that while QScrollBar
  handles really big numbers, scroll bars on today's screens cannot
  usefully control ranges above, say, 100,000 pixels.  Somewhere in
  the vicinity of that number, it becomes very hard to control the
  scrollbar using either keyboard or mouse.

  A scroll bar can be controlled by the keyboard, but it has a
  default focusPolicy() of \c NoFocus. Use setFocusPolicy() to
  enable keyboard focus.

  <img src=qscrbar-m.png> <img src=qscrbar-w.png>

  \sa QSlider QSpinBox
  <a href="guibooks.html#fowler">GUI Design Handbook: Scroll Bar</a>
*/


/*!
  \fn void QScrollBar::valueChanged( int value )
  This signal is emitted when the scroll bar value is changed, with the
  new scroll bar value as an argument.
*/

/*!
  \fn void QScrollBar::sliderPressed()
  This signal is emitted when the user presses the slider with the mouse.
*/

/*!
  \fn void QScrollBar::sliderMoved( int value )

  This signal is emitted when the slider is moved by the user, with
  the new scroll bar value as an argument.

  This signal is emitted even when tracking is turned off.

  \sa tracking() valueChanged() nextLine() prevLine() nextPage() prevPage()
*/

/*!
  \fn void QScrollBar::sliderReleased()
  This signal is emitted when the user releases the slider with the mouse.
*/

/*!
  \fn void QScrollBar::nextLine()
  This signal is emitted when the scroll bar scrolls one line down/right.
*/

/*!
  \fn void QScrollBar::prevLine()
  This signal is emitted when the scroll bar scrolls one line up/left.
*/

/*!
  \fn void QScrollBar::nextPage()
  This signal is emitted when the scroll bar scrolls one page down/right.
*/

/*!
  \fn void QScrollBar::prevPage()
  This signal is emitted when the scroll bar scrolls one page up/left.
*/



static const int thresholdTime = 500;
static const int repeatTime	= 50;

#define HORIZONTAL	(orientation() == Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2
#define SLIDER_MIN	9


/*!
  Constructs a vertical scroll bar.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( QWidget *parent, const char *name )
    : QWidget( parent, name, WResizeNoErase )
{
    orient = Vertical;
    init();
}

/*!
  Constructs a scroll bar.

  The \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( Orientation orientation, QWidget *parent,
			const char *name )
    : QWidget( parent, name, WResizeNoErase )
{
    orient = orientation;
    init();
}

/*!
  Constructs a scroll bar.

  \arg \e minValue is the minimum scroll bar value.
  \arg \e maxValue is the maximum scroll bar value.
  \arg \e lineStep is the line step value.
  \arg \e pageStep is the page step value. It is also used to calculate the size of the sliding thumb (scroll indicator).
  \arg \e value is the initial value.
  \arg \e orientation must be QScrollBar::Vertical or QScrollBar::Horizontal.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QScrollBar::QScrollBar( int minValue, int maxValue, int lineStep, int pageStep,
			int value,  Orientation orientation,
			QWidget *parent, const char *name )
    : QWidget( parent, name, WResizeNoErase ),
      QRangeControl( minValue, maxValue, lineStep, pageStep, value )
{
    orient = orientation;
    init();
}

void QScrollBar::init()
{
    track = TRUE;
    sliderPos = 0;
    pressedControl = QStyle::NoScroll;
    clickedAt = FALSE;
    setFocusPolicy( NoFocus );

    repeater = 0;
    d = 0;

    if ( style() == MotifStyle )
	setBackgroundMode( PaletteMid );
    else
	setBackgroundMode( PaletteBackground );
}


/*!
  Sets the scroll bar orientation.  The \e orientation must be
  QScrollBar::Vertical or QScrollBar::Horizontal.
  \sa orientation()
*/

void QScrollBar::setOrientation( Orientation orientation )
{
    orient = orientation;
    positionSliderFromValue();
    update();
}

/*!
  \fn Orientation QScrollBar::orientation() const
  Returns the scroll bar orientation; QScrollBar::Vertical or
  QScrollBar::Horizontal.
  \sa setOrientation()
*/

/*!
  \fn void QScrollBar::setTracking( bool enable )
  Enables scroll bar tracking if \e enable is TRUE, or disables tracking
  if \e enable is FALSE.

  If tracking is enabled (default), the scroll bar emits the
  valueChanged() signal whenever the slider is being dragged.  If
  tracking is disabled, the scroll bar emits the valueChanged() signal
  when the user releases the mouse button (unless the value happens to
  be the same as before).

  \sa tracking()
*/

/*!
  \fn bool QScrollBar::tracking() const
  Returns TRUE if tracking is enabled, or FALSE if tracking is disabled.

  Tracking is initially enabled.

  \sa setTracking()
*/


/*!
  Returns TRUE if the user has clicked the mouse on the slider
  and is currently dragging it, or FALSE if not.
*/

bool QScrollBar::draggingSlider() const
{
    return pressedControl == QStyle::Slider;
}


/*!
  Reimplements the virtual function QWidget::setPalette().

  Sets the background color to the mid color for Motif style scroll bars.
*/

void QScrollBar::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    if ( style() == MotifStyle )
	setBackgroundMode( PaletteMid );
    else
	setBackgroundMode( PaletteBackground );
}


/*!
  Returns a size hint for this scroll bar.
*/

QSize QScrollBar::sizeHint() const
{
    int sbextent = style().scrollBarExtent();

    if ( orient == Horizontal ) {
	return QSize( 30, sbextent );
    } else {
	return QSize( sbextent, 30 );
    }
}

/*!
  Specifies that this widget can use more, but is able to survive on
  less, space in the orientation() direction; and is fixed in the other
  direction.
*/

QSizePolicy QScrollBar::sizePolicy() const
{
    if ( orient == Horizontal )
	return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
    else
	return QSizePolicy(  QSizePolicy::Fixed, QSizePolicy::Minimum );
}


/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::valueChange()
{
    int tmp = sliderPos;
    positionSliderFromValue();
    if ( tmp != sliderPos )
	drawControls( QStyle::AddPage | QStyle::Slider | QStyle::SubPage , pressedControl );
    emit valueChanged(value());
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::stepChange()
{
    rangeChange();
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::rangeChange()
{
    positionSliderFromValue();
    drawControls( QStyle::AddLine | QStyle::AddPage | QStyle::Slider |
		  QStyle::SubPage | QStyle::SubLine,
		  pressedControl );
}


/*!
  Handles timer events for the scroll bar.
*/

void QScrollBar::doAutoRepeat()
{
    if ( clickedAt ){
	if ( repeater )
	    repeater->changeInterval( repeatTime );
	action( (QStyle::ScrollControl) pressedControl );
	QApplication::syncX();
    } else {
	stopAutoRepeat();
    }
}


/*! Starts the auto-repeat logic.  Some time after this function is
called, the auto-repeat starts taking effect, and from then on repeats
until stopAutoRepeat() is called.
*/

void QScrollBar::startAutoRepeat()
{
    if ( !repeater ) {
	repeater = new QTimer( this, "auto-repeat timer" );
	connect( repeater, SIGNAL(timeout()),
		 this, SLOT(doAutoRepeat()) );
    }
    repeater->start( thresholdTime, FALSE );
}


/*! Stops the auto-repeat logic. */

void QScrollBar::stopAutoRepeat()
{
    delete repeater;
    repeater = 0;
}


/*!
  Handles wheel events for the scroll bar.
*/
void QScrollBar::wheelEvent( QWheelEvent *e ){
    static float offset = 0;
    static QScrollBar* offset_owner = 0;
    if (offset_owner != this){
	offset_owner = this;
	offset = 0;
    }
    e->accept();
    offset += -e->delta()*QMAX(pageStep()/8,2*lineStep())/120;
    if (QABS(offset)<1)
	return;
    setValue( value() + int(offset) );
    offset -= int(offset);
}


/*!
  Handles key press events for the scroll bar.
*/

void QScrollBar::keyPressEvent( QKeyEvent *e )
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
    case Key_PageUp:
	subtractPage();
	break;
    case Key_PageDown:
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
	break;
    }
}


/*!
  Handles resize events for the scroll bar.
*/

void QScrollBar::resizeEvent( QResizeEvent * )
{
    positionSliderFromValue();
    repaint(rect());
}


/*!
  Handles paint events for the scroll bar.
*/

void QScrollBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    drawControls( QStyle::AddLine | QStyle::SubLine | QStyle::AddPage | QStyle::SubPage | QStyle::Slider,
			pressedControl, &p );
}

static QCOORD sliderStartPos = 0;

/*!
  Handles mouse press events for the scroll bar.
*/

void QScrollBar::mousePressEvent( QMouseEvent *e )
{
    if ( !(e->button() == LeftButton ||
	   (/*style() == MotifStyle &&*/ e->button() == MidButton) ) )
	return;

    if ( maxValue() == minValue() ) // nothing to be done
	return;

    clickedAt	   = TRUE;
    pressedControl = pointOver( e->pos() );

    if ( (pressedControl == QStyle::AddPage ||
	  pressedControl == QStyle::SubPage ||
	  pressedControl == QStyle::Slider ) &&
	 /*style() == MotifStyle &&*/
	 e->button() == MidButton ) {
	int sliderMin, dummy2, dummy3, sliderLength;
	metrics( sliderMin, dummy2, sliderLength, dummy3 );
	int newSliderPos = (HORIZONTAL ? e->pos().x() : e->pos().y())
			   - sliderLength/2;
	newSliderPos = QMAX( sliderMin, newSliderPos );
	setValue( sliderPosToRangeValue(newSliderPos) );
	sliderPos = newSliderPos;
	pressedControl = QStyle::Slider;
    }

    if ( pressedControl == QStyle::Slider ) {
	clickOffset = (QCOORD)( (HORIZONTAL ? e->pos().x() : e->pos().y())
				- sliderPos );
	slidePrevVal   = value();
	sliderStartPos = sliderPos;
	emit sliderPressed();
    } else if ( pressedControl != QStyle::NoScroll ) {
	drawControls( pressedControl, pressedControl );
	action( (QStyle::ScrollControl) pressedControl );
	startAutoRepeat();
    }
}


/*!
  Handles mouse release events for the scroll bar.
*/

void QScrollBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !clickedAt || !( e->button() == LeftButton ||
			  e->button() == MidButton ) )
	return;
    QStyle::ScrollControl tmp = (QStyle::ScrollControl) pressedControl;
    clickedAt = FALSE;
    stopAutoRepeat();
    mouseMoveEvent( e );  // Might have moved since last mouse move event.
    pressedControl = QStyle::NoScroll;

    if (tmp == QStyle::Slider) {
	directSetValue( calculateValueFromSlider() );
	emit sliderReleased();
	if ( value() != prevValue() )
	    emit valueChanged( value() );
    }
    drawControls( tmp, pressedControl );
}


/*!
  Handles mouse move events for the scroll bar.
*/

void QScrollBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !isVisible() ) {
	clickedAt = FALSE;
	return;
    }

    if ( !clickedAt || !(e->state() & LeftButton ||
			 ((e->state() & MidButton) /*&&
			  style() == MotifStyle*/)) )
	return;
    int newSliderPos;
    if ( pressedControl == QStyle::Slider ) {
	int sliderMin, sliderMax;
	sliderMinMax( sliderMin, sliderMax );
	QRect r = rect();
	int m = style().maximumSliderDragDistance();
	if ( m >= 0 ) {
	    if ( orientation() == Horizontal )
		r.setRect( r.x() - m, r.y() - 2*m, r.width() + 2*m, r.height() + 4*m );
	    else
		r.setRect( r.x() - 2*m, r.y() - m, r.width() + 4*m, r.height() + 2*m );
	    if ( style() == WindowsStyle && !r.contains( e->pos() ) )
		newSliderPos = sliderStartPos;
	    else
		newSliderPos = (HORIZONTAL ? e->pos().x() :
				e->pos().y()) -clickOffset;
	}
	else
	    newSliderPos = (HORIZONTAL ? e->pos().x() :
			    e->pos().y()) -clickOffset;
	
	if ( newSliderPos < sliderMin )
	    newSliderPos = sliderMin;
	else if ( newSliderPos > sliderMax )
	    newSliderPos = sliderMax;
	if ( newSliderPos == sliderPos )
	    return;
	int newVal = sliderPosToRangeValue(newSliderPos);
	if ( newVal != slidePrevVal )
	    emit sliderMoved( newVal );
	if ( track && newVal != value() ) {
	    directSetValue( newVal ); // Set directly, painting done below
	    emit valueChanged( value() );
	}
	slidePrevVal = newVal;
	sliderPos = (QCOORD)newSliderPos;
	drawControls( QStyle::AddPage | QStyle::Slider | QStyle::SubPage, pressedControl );
    }
    else if ( style() == WindowsStyle ) {
	// stop scrolling when the mouse pointer leaves a control
	// similar to push buttons
	if ( (int)pressedControl != pointOver( e->pos() ) ) {
	    drawControls( pressedControl, QStyle::NoScroll );
	    stopAutoRepeat();
	} else if ( !repeater ) {
	    drawControls( pressedControl, pressedControl );
	    action( (QStyle::ScrollControl) pressedControl );
	    startAutoRepeat();
	}
    }
}


/*!
  \fn int QScrollBar::sliderStart() const
  Returns the pixel position where the scroll bar slider starts.

  It is equivalent to sliderRect().y() for vertical
  scroll bars or sliderRect().x() for horizontal scroll bars.
*/

/*!
  Returns the scroll bar slider rectangle.
  \sa sliderStart()
*/

QRect QScrollBar::sliderRect() const
{
    int sliderMin, sliderMax, sliderLength, buttonDim;
    metrics( sliderMin, sliderMax, sliderLength, buttonDim );
    int b = style() == MotifStyle ? MOTIF_BORDER : 0;

    if ( HORIZONTAL )
	return QRect( sliderStart(), b,
		      sliderLength, height() - b*2 );
    else
	return QRect( b, sliderStart(),
		      width() - b*2, sliderLength );
}

void QScrollBar::positionSliderFromValue()
{
    sliderPos = (QCOORD)rangeValueToSliderPos( value() );
}

int QScrollBar::calculateValueFromSlider() const
{
    return sliderPosToRangeValue( sliderPos );
}


void QScrollBar::sliderMinMax( int &sliderMin, int &sliderMax) const
{
    int dummy1, dummy2;
    metrics( sliderMin, sliderMax, dummy1, dummy2 );
}


void QScrollBar::metrics( int &sliderMin, int &sliderMax,
			  int &sliderLength, int& buttonDim ) const
{

    style().scrollBarMetrics( this, sliderMin, sliderMax, sliderLength, buttonDim);
}


QStyle::ScrollControl QScrollBar::pointOver(const QPoint &p) const
{
    return style().scrollBarPointOver(this, sliderStart(), p);
}


int QScrollBar::rangeValueToSliderPos( int v ) const
{
    int sliderMin, sliderMax;
    sliderMinMax( sliderMin, sliderMax );
    return positionFromValue( v, sliderMax-sliderMin ) + sliderMin;
}

int QScrollBar::sliderPosToRangeValue( int pos ) const
{
    int sliderMin, sliderMax;
    sliderMinMax( sliderMin, sliderMax );
    return  valueFromPosition( pos - sliderMin, sliderMax - sliderMin );
}


void QScrollBar::action( QStyle::ScrollControl control )
{
    switch( control ) {
	case QStyle::AddLine:
	    emit nextLine();
	    addLine();
	    break;
	case QStyle::SubLine:
	    emit prevLine();
	    subtractLine();
	    break;
	case QStyle::AddPage:
	    emit nextPage();
	    addPage();
	    break;
	case QStyle::SubPage:
	    emit prevPage();
	    subtractPage();
	    break;
	default:
#if defined(CHECK_RANGE)
	    qWarning( "QScrollBar::action: (%s) internal error",
		     name( "unnamed" ) );
#else
	    ;
#endif
    }
}


void QScrollBar::drawControls( uint controls, uint activeControl ) const
{
    QPainter p ( this );
    drawControls( controls, activeControl, &p );
}


void QScrollBar::drawControls( uint controls, uint activeControl,
			       QPainter *p ) const
{

    style().drawScrollBarControls(p, this, sliderStart(), controls, activeControl);
    return;

}

/*!\reimp
 */
void QScrollBar::styleChange( QStyle& old )
{
    positionSliderFromValue();
    if ( style() == MotifStyle )
	setBackgroundMode( PaletteMid );
    else
	setBackgroundMode( PaletteBackground );

    QWidget::styleChange( old );
}



#undef ADD_LINE_ACTIVE
#undef SUB_LINE_ACTIVE
