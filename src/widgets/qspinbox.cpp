/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.cpp#15 $
**
** Implementation of QSpinBox widget class
**
** Created : yes
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qspinbox.h"
#include "qpushbt.h"
#include "qstrlist.h"
#include "qpainter.h"
#include "qkeycode.h"
#include "qbitmap.h"
#include "qlined.h"

/*! \class QSpinBox qspinbox.h

  \brief The QSpinBox class provides a spin box, sometimes called
  up-down widget, little arrows widget or spin button.

  The spin box deviates from Motif look a bit.

  <img src=qspinbox-m.gif> <img src=qspinbox-w.gif>
*/

struct QSpinBoxData {
    double b, t;
    int d;
    double a;
};


/*!  Creates an empty, non-wrapping spin box with TabFocus \link
  setFocusPolicy() focus policy. \endlink.
*/

QSpinBox::QSpinBox( QWidget * parent , const char * name )
    : QFrame( parent, name )
{
    d = 0; // not used
    wrap = FALSE;
    setFocusPolicy( TabFocus );

    up = new QPushButton( this, "up" );
    up->setFocusPolicy( QWidget::NoFocus );
    up->setAutoRepeat( TRUE );

    down = new QPushButton( this, "down" );
    down->setFocusPolicy( QWidget::NoFocus );
    down->setAutoRepeat( TRUE );

    vi = new QLineEdit( this, "this is not /usr/bin/vi" );
    vi->setFocusPolicy( QWidget::NoFocus );
    vi->setFrame( FALSE );

    if ( style() == WindowsStyle )
	setFrameStyle( WinPanel | Sunken );
    else
	setFrameStyle( Panel | Sunken );
    setLineWidth( 2 );

    connect( up, SIGNAL(clicked()), SLOT(next()) );
    connect( down, SIGNAL(clicked()), SLOT(prev()) );
    connect( vi, SIGNAL(returnPressed()), SLOT(textChanged()) );
}


/*!  Deletes the spin box, freeing all memory and other resoures.
*/

QSpinBox::~QSpinBox()
{
    delete d;
}


/*!  Sets the legal range of the spin box to \a bottom-top inclusive,
  with \a decimals decimal places.  \a decimals must be at most 8.
  Here are some examples:

  \code
    s->setRange( 42, 69 ); // integers from 42-69 inclusive
    s->setRange( 0, 1, 3 ); // 0.000, 0.001, 0.002, ..., 0.999, 1.000
    s->setRange( 0.1, 360, 1 ); // 0.1, ... 359.8, 359.9, 360.0
    s->setRange( -32768, 32767 ); // the integers -32768 to 32767 inclusive
  \endcode

  If you don't set a valid value using setCurrent(), QSpinBox picks
  one in show() - normally the lowest valid value.
*/

void QSpinBox::setRange( double bottom, double top, int decimals )
{
    if ( !d )
	d = new QSpinBoxData;
    d->b = bottom;
    d->t = top;
    d->d = 0;
    d->a = 1;
    while ( d->d < 8 && d->d < decimals ) {
	d->a = d->a / 10;
	d->d++;
    }
}


/*!  Makes the spin box wrap around from the last to the first item of
  \a w is TRUE, or not if not.

  \sa wrapping() setCurrent()
*/

void QSpinBox::setWrapping( bool w )
{
    wrap = w;
    enableButtons();
}

/*!  \fn bool QSpinBox::wrapping() const

  Returns the current setWrapping() value.
*/




/*!  Returns the current value of the spin box, or 0 is the current
  value is unparsable.
*/

double QSpinBox::current() const
{
    bool ok = FALSE;
    double result = QString( vi->text() ).toDouble( &ok );
    return ok ? result : 0;
}


/*!  Moves the spin box to the next value.  This is the same as
  clicking on the pointing-up button, and can be used for e.g.
  keyboard accelerators.

  \sa prev(), setCurrent(), current()
*/

void QSpinBox::next()
{
    if ( !d )
	return;

    bool ok;
    double c = QString(vi->text()).toDouble( &ok );

    c += d->a;

    QString f;
    f.sprintf( "%%.%df" );
    QString s;
    s.sprintf( f, c );
    if ( s.toDouble( &ok ) > d->t )
	c = wrapping() ? d->b : d->t;

    setCurrent( c );
}


/*!  Moves the spin box to the previous value.  This is the same as
  clicking on the pointing-down button, and can be used for e.g.
  keyboard accelerators.

  \sa next(), setCurrent(), current()
*/

void QSpinBox::prev()
{
    if ( !d )
	return;

    bool ok;
    double c = QString(vi->text()).toDouble( &ok );

    c -= d->a;

    QString f;
    f.sprintf( "%%.%df" );
    QString s;
    s.sprintf( f, c );
    if ( s.toDouble( &ok ) < d->b )
	c = wrapping() ? d->t : d->b;

    setCurrent( c );
}


/*!  Sets the current value of the spin box to \a value.  \a value is
  forced into the legal range.
*/

void QSpinBox::setCurrent( double value )
{
    if ( d && value > d->t )
	value = d->t;
    else if ( d && value < d->b )
	value = d->b;
    QString f;
    f.sprintf( "%%.%df" );
    QString s;
    s.sprintf( f, value );
    vi->setText( s );
}


/*! \fn void QSpinBox::selected( double )

  This signal is emitted every time the value of the spin box changes
  (by setCurrent(), by a keyboard accelerator, by mouse clicks, or by
  telepathy).

  Note that it is emitted \e every time, not just for the \"final\"
  step - if the user clicks 'up' three times, this signal is emitted
  three times.
*/


/*!  Returns a good-looking size for the spin box.
*/

QSize QSpinBox::sizeHint() const
{ // maybe write this around QLineEdit::sizeHint()
    QFontMetrics fm = fontMetrics();
    int h = fm.height();
    if ( h < 22 ) // enough space for the button pixmaps
	h = 22;
    int w = 40; // never less than 40 pixels for the value

    QString s( "999.99" );
    if ( d ) {
	QString f;
	f.sprintf( "%%.%df", d->d );
	if ( QABS(d->b) > QABS(d->t) )
	    s.sprintf( f, d->b );
	else
	    s.sprintf( f, d->t );
    }
    w = fm.width( s );
    
    return QSize( frameWidth() * 2 // right/left frame
		  + (8*h)/5 // buttons - approximate golden ratio
		  + 6 // right/left margins
		  + w, // longest value
		  frameWidth() * 2 // top/bottom frame
		  + 4 // top/bottom margins
		  + h // font height
		  );
}


/*!  Interprets the up and down keys; ignores everything else.
*/

void QSpinBox::keyPressEvent( QKeyEvent * e )
{
    if ( e->key() == Key_Up ) {
	next();
	e->accept();
    } else if ( e->key() == Key_Down ) {
	prev();
	e->accept();
    } else {
	e->ignore();
    }
}


/*!  Handles resize events for the spin box.  
*/

void QSpinBox::resizeEvent( QResizeEvent * e )
{
    if ( !up || !down ) // happens if the application has a pointer error
	return;

    QSize bs; // no, it's short for 'button size'
    bs.setHeight( e->size().height()/2 - frameWidth() );
    if ( bs.height() < 9 )
	bs.setHeight( 9 );
    bs.setWidth( bs.height() * 8 / 5 );

    if ( up->size() != bs ) {
	up->resize( bs );
	QBitmap bm( (bs.height() - 6) * 2 - 1, bs.height() - 6 );
	QPointArray a;
	a.setPoints( 3,
		     bm.height()-1, 0,
		     0, bm.height()-1,
		     bm.width()-1, bm.height()-1 );
	QPainter p;
	p.begin( &bm );
	p.eraseRect( 0, 0, bm.width(), bm.height() );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
	up->setPixmap( bm );
    }

    if ( down->size() != bs ) {
	down->resize( bs );
	QBitmap bm( (bs.height() - 6) * 2 - 1, bs.height() - 6 );
	QPointArray a;
	a.setPoints( 3,
		     bm.height()-1, bm.height()-1,
		     0, 0,
		     bm.width()-1, 0 );
	QPainter p;
	p.begin( &bm );
	p.eraseRect( 0, 0, bm.width(), bm.height() );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
	down->setPixmap( bm );
    }

    int x = e->size().width() - frameWidth() - bs.width();

    up->move( x, frameWidth() );
    down->move( x, height() - frameWidth() - up->height() );

    vi->setGeometry( frameWidth(), frameWidth(),
		     x - frameWidth(), height() - 2*frameWidth() );
}


/*!  Set the up and down buttons to enabled or disabled state, as
  appropriate.
*/

void QSpinBox::enableButtons()
{
    up->setEnabled( wrapping() || (d && current() < d->t) );
    down->setEnabled( wrapping() || (d && current() > d->b) );
}


/*!  

*/

void QSpinBox::textChanged()
{
    
}
