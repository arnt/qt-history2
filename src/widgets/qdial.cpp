/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qdial.cpp#12 $
**
** Implementation of something useful.
**
** Created : 979899
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

#include "qdial.h"

#include "qpainter.h"
#include "qpointarray.h"

#include <math.h> // sin(), cos(), atan()
//### Forutsetter linking med math lib - Jfr kommentar i qpainter_x11.cpp!

static const double m_pi = 3.14159265358979323846;


class QDialPrivate
{
public:
    QDialPrivate()
    {
	wrapping = FALSE;
	tracking = TRUE;
	doNotEmit = FALSE;
	target = 3.7;
    }

    bool wrapping;
    bool tracking;
    bool doNotEmit;
    double target;

    QPointArray lines;
};


/*! \class QDial qdial.h

  \brief The QDial widget provides a dial (or speedometer, or potentiometer) widget.

  \ingroup realwidgets

  Q dial is used when the user needs to control a value within a
  program-definable range, and the range either wraps around
  (typically, 0-359 degrees) or the dialog layout needs a square widget.

  Both API- and UI-wise, the dial is very like a \link QSlider
  slider. \endlink Indeed, when wrapping() is FALSE (the default)
  there is no hard difference between a slider and a dial.  They have
  the same signals, slots and member functions, all of which do the
  same things.  Which one to use depends only on your taste and
  on the application.

  The dial initially emits valueChanged() signals continuously while
  the slider is being moved; you can make it emit the signal less
  often by calling setTracking( FALSE ).  dialMoved() is emitted
  continuously even when tracking() is FALSE.

  The slider also emits dialPressed() and dialReleased() signals when
  the mouse button is pressed and released.  But note that the dial's
  value can change without these signals being emitted; the keyboard
  and wheel can be used to change the value.

  Unlike the slider, QDial attempts to draw a "nice" number of notches
  rather than one per lineStep().  If possible, that number \e is
  lineStep(), but if there aren't enough pixels to draw every, QDial
  will draw every second, third or something.  notchSize() returns the
  number of units per notch, hopefully a multiple of lineStep();
  setNotchTarget() sets the target distance between neighbouring
  notches in pixels.  The default is 3.75 pixels.

  Like the slider, the dial makes the QRangeControl functions
  setValue(), addLine(), substractLine(), addPage() and subtractPage()
  available as slots.

  The dial's keyboard interface is fairly simply: The left and right
  arrow keys move by lineStep(), page up and page down by pageStep()
  and Home and End to minValue() and maxValue().

  <img src=qdial-m.gif> <img src=qdial-w.gif>

  \sa QScrollBar QSpinBox
  <a href="guibooks.html#fowler">GUI Design Handbook: Slider</a>
*/




/*!  Constructs a dial with the default range of QRangeControl. */

QDial::QDial( QWidget *parent, const char *name )
    : QWidget( parent, name ), QRangeControl()
{
    d = new QDialPrivate;
}



/*!  Constructs a dial whose value can never be smaller than \a
  minValue or greater than \a maxValue, whose line step size is \a
  lineStep and page step size is \a pageStep, and whose value is
  initially \a value.

  \a value is forced to be within the legal range.

*/

QDial::QDial( int minValue, int maxValue, int pageStep, int value,
	      QWidget *parent, const char *name )
    : QWidget( parent, name ),
      QRangeControl( minValue, maxValue, 1, pageStep, value )
{
    d = new QDialPrivate;
}


/*!

*/

void QDial::setTracking( bool enable )
{
    d->tracking = enable;
}


/*!  Returns TRUE if tracking is enabled, or FALSE if tracking is disabled.

Tracking is initially enabled.

\sa setTracking().
*/

bool QDial::tracking() const
{
    return d ? d->tracking : TRUE;
}

QSize QDial::sizeHint() const
{
    return QSize(19,19);
}

/*!

*/

QSizePolicy QDial::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}


/*!  Makes QRangeControl::setValue() available as a slot.
*/

void QDial::setValue( int newValue )
{ // ### set doNotEmit?  Matthias?
    QRangeControl::setValue( newValue );
}


/*!  Moves the dial one lineStep() upwards. */

void QDial::addLine()
{
    QRangeControl::addLine();
}


/*!  Moves the dial one lineStep() downwards. */

void QDial::subtractLine()
{
    QRangeControl::subtractLine();
}


/*! \reimp */

void QDial::resizeEvent( QResizeEvent * e )
{
    d->lines.resize( 0 );
    QWidget::resizeEvent( e );
}


/*!

*/

void QDial::paintEvent( QPaintEvent * e )
{
    int r = QMIN( width(), height() )/2;

    int bigLineSize = r / 6;
    if ( bigLineSize < 4 )
	bigLineSize = 4;
    if ( bigLineSize > r/2 )
	bigLineSize = r/2;

    int xc = width()/2;
    int yc = height()/2;

    if ( !d->lines.size() ) {
	int ns = notchSize();
	int notches = (maxValue()+notchSize()-1-minValue()) / ns;
	d->lines.resize( 2+2*notches );
	int smallLineSize = bigLineSize / 2;
	int i;
	for( i=0; i<=notches; i++ ) {
	    double angle = d->wrapping
			   ? m_pi*3/2 - i*2*m_pi/notches
			   : (m_pi*8 - i*10*m_pi/notches)/6;
	    double s = sin( angle ); // sin/cos aren't defined as const...
	    double c = cos( angle );
	    if ( i == 0 ||
		 ns*i/pageStep() > ns*(i-1)/pageStep() ) {
		d->lines[2*i] = QPoint( (int)(0.5+xc+(r-bigLineSize)*c),
					(int)(0.5+yc-(r-bigLineSize)*s) );
		d->lines[2*i+1] = QPoint( (int)(0.5+xc+r*c),
					  (int)(0.5+yc-r*s) );
	    } else {
		d->lines[2*i] = QPoint( (int)(0.5+xc+(r-1-smallLineSize)*c),
					(int)(0.5+yc-(r-1-smallLineSize)*s) );
		d->lines[2*i+1] = QPoint( (int)(0.5+xc+(r-1)*c),
					  (int)(0.5+yc-(r-1)*s) );
	    }
	}
    }

    QPainter p( this );
    p.setClipRect( e->rect() );
    p.setPen( colorGroup().foreground() );

    p.drawLineSegments( d->lines );

    double a;
    if ( maxValue() == minValue() )
	a = m_pi/2;
    else if ( d->wrapping )
	a = m_pi*3/2 - (value()-minValue())*2*m_pi/(maxValue()-minValue());
    else
	a = (m_pi*8 - (value()-minValue())*10*m_pi/(maxValue()-minValue()))/6;
    int len = r - bigLineSize - 1;
    if ( len < 5 )
	len = 5;
    int back = len/4;
    if ( back < 1 )
	back = 1;
    QPointArray arrow( 3 );
    arrow[0] = QPoint( (int)(0.5+xc+len*cos(a)),
		       (int)(0.5+yc-len*sin(a)) );
    arrow[1] = QPoint( (int)(0.5+xc+back*cos(a+m_pi*5/6)),
		       (int)(0.5+yc-back*sin(a+m_pi*5/6)) );
    arrow[2] = QPoint( (int)(0.5+xc+back*cos(a-m_pi*5/6)),
		       (int)(0.5+yc-back*sin(a-m_pi*5/6)) );
    p.setPen( colorGroup().foreground() );
    p.setBrush( colorGroup().foreground() );
    //p.setPen( red );
    //p.setBrush( green );
    p.drawPolygon( arrow );
}


/*!

*/

void QDial::keyPressEvent( QKeyEvent * e )
{
    switch ( e->key() ) {
    case Key_Left:
	subtractLine();
	break;
    case Key_Right:
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
	break;
    }
}


/*!

*/

void QDial::mousePressEvent( QMouseEvent * e )
{
    setValue( valueFromPoint( e->pos() ) );
    emit dialPressed();
}


/*!

*/

void QDial::mouseReleaseEvent( QMouseEvent * e )
{
    setValue( valueFromPoint( e->pos() ) );
    emit dialReleased();
}


/*!

*/

void QDial::mouseMoveEvent( QMouseEvent * e )
{
    if ( !d->tracking || (e->state() & LeftButton) == 0 )
	return;
    d->doNotEmit = TRUE;
    setValue( valueFromPoint( e->pos() ) );
    emit dialMoved( value() );
    d->doNotEmit = FALSE;
}


/*!

*/

void QDial::wheelEvent( QWheelEvent * )
{

}


/*!

*/

void QDial::valueChange()
{
    d->lines.resize( 0 );
    repaint();
    if ( d->tracking || !d->doNotEmit )
	emit valueChanged( value() );
}


/*!

*/

void QDial::rangeChange()
{
    d->lines.resize( 0 );
    repaint();
}


/*!

*/

int QDial::valueFromPoint( const QPoint & p ) const
{
    double a = atan2( height()/2 - p.y(), p.x() - width()/2 );
    if ( a < m_pi/-2 )
	a = a + m_pi*2;

    int r = maxValue()-minValue();
    if ( d->wrapping )
	return bound((int)(0.5 + minValue() + r*(m_pi*3/2-a)/(2*m_pi)));
    else
	return bound((int)(0.5 + minValue() + r*(m_pi*4/3-a)/(m_pi*10/6)));
}


/*!

*/

void QDial::setWrapping( bool enable )
{
    if ( d->wrapping == enable )
	return;
    d->lines.resize( 0 );
    d->wrapping = enable;
    repaint();
}


/*!

*/

bool QDial::wrapping() const
{
    return d->wrapping;
}


/*!  Returns the current notch size.  This is in range control units,
not pixels, and if possible it is a multiple of lineStep() that
results in an on-screen notch size near notchTarget().

\sa notchTarget() lineStep()
*/

int QDial::notchSize() const
{
    // radius of the arc
    int r = QMIN( width(), height() )/2;
    // length of the whole arc
    int l = (int)(r*(d->wrapping ? 6 : 5)*m_pi/6);
    // length of the arc from minValue() to minValue()+pageStep()
    if ( maxValue() > minValue()+pageStep() )
	l = (int)(0.5 + l * pageStep() / (maxValue()-minValue()));
    // length of a lineStep() arc
    l = l * lineStep() / pageStep();
    if ( l < 1 )
	l = 1;
    // how many times lineStep can be draw in d->target pixels
    l = (int)(0.5 + d->target / l);
    // we want notchSize() to be a non-zero multiple of lineStep()
    if ( !l )
	l = 1;
    return lineStep() * l;
}


/*!  Sets the dial to use a notch size as close to \a target pixels as
  possible.  QDial will find a suitable number close to this, based on
  the dial's on-screen size, range and lineStep().

  \sa notchTarget() notchSize()
*/

void QDial::setNotchTarget( double target )
{
    d->lines.resize( 0 );
    d->target = target;
    repaint();
}


/*!  Returns the target size of the notch; this is the number of
  pixels QDial attempts to put between each little line.

  The actual size differs a bit from the target.
*/

double QDial::notchTarget() const
{
    return d->target;
}


/*!  Moves the dial one pageStep() upwards. */

void QDial::addPage()
{
    QRangeControl::addPage();
}


/*!  Moves the dial one pageStep() downwards. */

void QDial::subtractPage()
{
    QRangeControl::subtractPage();
}


/*!
    \fn void QDial::valueChanged( int value )

    This signal is emitted whenever the dial value changes.  The frequency
    of this signal is influenced by setTracking().
*/

/*!
    \fn void QDial::dialPressed()

    This signal is emitted when the use begins mouse interaction with
    the dial.

    \sa dialReleased()
*/

/*!
    \fn void QDial::dialMoved( int value )

    This signal is emitted whenever the dial value changes.  The frequency
    of this signal is \e not influenced by setTracking().

    \sa valueChanged(int)
*/

/*!
    \fn void QDial::dialReleased()

    This signal is emitted when the use ends mouse interaction with
    the dial.

    \sa dialPressed()
*/
