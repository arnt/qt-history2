/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.cpp#23 $
**
** Implementation of QSpinBox widget class
**
** Created : yes
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qspinbox.h"

#include <qspinbox.h>
#include <qpushbt.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qbitmap.h>
#include <qlined.h>
#include <qvalidator.h>


/*! 
  \class QSpinBox qspinbox.h

  \brief The QSpinBox class provides a spin box widget, sometimes called
  up-down widget, little arrows widget or spin button.

  QSpinBox allows the user to choose a numeric value, either by
  clikcing the up/down buttons to increase/decrease the value
  currently displayed, or by typing the value directly into the spin
  box. 

  Every time the value changes, QSpinBox emits the valueChanged()
  signal. The current value can be fetched with value() and set with
  setValue().

  The spin box clamps the value within a numeric range, see
  QRangeControl for details. Clicking the up/down down buttons (or
  using the keyboard accelerators: Up-arrow and Down-arrow) will
  increase the value with the value of lineStep().

  Most spin boxes are directional, but QSpinBox can also operate as a
  circular spin box, i.e. if the range is 0-99 and the current value
  is 99, clicking Up will give 0. Use setWrapping() to if you want
  circular behavior.

  The default \link setFocusPolicy() focus policy \endlink is
  StrongFocus.

  QSpinBox can easily be subclassed to allow the user to input other
  things than a numeric value, as long as the allowed input can be
  mapped down to a range of integers. See updateDisplay(),
  interpretText() and setValidator().

  <img src=qspinbox-m.gif> <img src=qspinbox-w.gif>
*/


struct QSpinBoxData {
};


/*!
  Creates a spin box with the default QRangeControl range and step
  value.

  \sa minValue(), maxValue(), setRange(), lineStep(), setSteps()
*/

QSpinBox::QSpinBox( QWidget * parent , const char * name )
    : QFrame( parent, name )
{
    initSpinBox();
}


/*!
  Creates a spin box with range from \a minValue to \a maxValue
  inclusive, with step value \a step. The value is set to \a minValue.

  \sa minValue(), maxValue(), setRange(), lineStep(), setSteps()
*/

QSpinBox::QSpinBox( int minValue, int maxValue, int step, QWidget* parent,
		    const char* name )
    : QFrame( parent, name ),
      QRangeControl( minValue, maxValue, step, step, minValue )
{
    initSpinBox();
}

/*!  
  \internal Initialization.
*/

void QSpinBox::initSpinBox()
{
    extra = 0; 			// not used; reserved for future expansion
    wrap = FALSE;

    up = new QPushButton( this, "up" );
    up->setFocusPolicy( QWidget::NoFocus );
    up->setAutoRepeat( TRUE );

    down = new QPushButton( this, "down" );
    down->setFocusPolicy( QWidget::NoFocus );
    down->setAutoRepeat( TRUE );

    validator = new QIntValidator( minValue(), maxValue(), this, "validator" );
    vi = new QLineEdit( this, "this is not /usr/bin/vi" );
    vi->setFrame( FALSE );
    setFocusProxy( vi );
    setFocusPolicy( StrongFocus );
    vi->setValidator( validator );
    vi->installEventFilter( this );
    
    if ( style() == WindowsStyle )
	setFrameStyle( WinPanel | Sunken );
    else
	setFrameStyle( Panel | Sunken );
    setLineWidth( 2 );

    updateDisplay();

    connect( up, SIGNAL(pressed()), SLOT(stepUp()) );
    connect( down, SIGNAL(pressed()), SLOT(stepDown()) );
    connect( vi, SIGNAL(textChanged(const char *)), SLOT(textChanged()) );
}

/*!  
  Deletes the spin box, freeing all memory and other resoures.
*/

QSpinBox::~QSpinBox()
{
}


/*!  
  Returns the current text of the spin box.

  \sa value()
*/

const char * QSpinBox::text() const
{ 	
    return vi->text();
}



/*!
  Returns a copy of the current text of the spin box with any suffix
  and white space at the start and end removed.
*/

QString QSpinBox::textWithoutSuffix() const
{
    QString s = QString(text()).stripWhiteSpace();
    if ( suffix() ) {
	QString x = QString(suffix()).stripWhiteSpace();
	int len = x.length();
	if ( len && s.right(len) == x )  // Remove _only_ if it is the suffix
	    s.truncate( s.length() - len );
    }
    return s;
}

/*!
  Sets the suffix to \a text. The suffix is appended to the end of the
  displayed value. Typical use is to indicate the unit of measurement
  to the user. 

  To turn off the suffix display, call this function with 0 or an
  empty string as parameter. The default is no suffix.

  \sa suffix()
*/

void QSpinBox::setSuffix( const char* text )
{
    sfix = text;
    updateDisplay();
}


/*!
  Returns the currently set suffix, or 0 if no suffix is currently
  set.
*/

const char* QSpinBox::suffix() const
{
    if ( sfix.isEmpty() )
	return 0;
    else
	return sfix;
}


/*!
  Setting wrapping to TRUE will allow the value to be wrapped from the
  highest value to the lowest, and vice versa. By default, wrapping is
  turned off.

  \sa wrapping(), minValue(), maxValue(), setRange()
*/

void QSpinBox::setWrapping( bool on )
{
    wrap = on;
    updateDisplay();
}


/*!
  Returns the current setWrapping() value.
*/

bool QSpinBox::wrapping() const
{
    return wrap;
}



/*!  
  Returns a good-looking size for the spin box.
*/

QSize QSpinBox::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    int h = fm.height();
    if ( h < 12 ) 	// ensure enough space for the button pixmaps
	h = 12;
    int w = 28; 	// minimum width for the value
    QString s;
    s.setNum( minValue() );
    s.append( suffix() );
    w = QMAX( w, fm.width( s ) );
    s.setNum( maxValue() );
    s.append( suffix() );
    w = QMAX( w, fm.width( s ) );

    return QSize( h // buttons AND frame both sides - see resizeevent()
		  + 6 // right/left margins
		  + w, // widest value
		  frameWidth() * 2 // top/bottom frame
		  + 4 // top/bottom margins
		  + h // font height
		  );
}


/*!
  Sets the current value of the spin box to \a value. This is
  QRangeControl::setValue() made available as a slot.
*/

void QSpinBox::setValue( int value )
{
    QRangeControl::setValue( value );
}


/*!
  Increases the current value one step, wrapping as necessary. This is
  the same as clicking on the pointing-up button, and can be used for
  e.g. keyboard accelerators.

  \sa stepDown(), addLine(), lineStep(), setSteps(), setValue(), value()
*/

void QSpinBox::stepUp()
{
    if ( edited )
	interpretText();
    if ( wrapping() && ( value()+lineStep() > maxValue() ) )
	setValue( minValue() );
    else
	addLine();
}


/*!
  Decreases the current value one step, wrapping as necessary. This is
  the same as clicking on the pointing-down button, and can be used
  for e.g. keyboard accelerators.

  \sa stepUp(), subtractLine(), lineStep(), setSteps(), setValue(), value()
*/

void QSpinBox::stepDown()
{
    if ( edited )
	interpretText();
    if ( wrapping() && ( value()-lineStep() < minValue() ) )
	setValue( maxValue() );
    else
	subtractLine();
}


/*! 
  \fn void QSpinBox::valueChanged( int value )

  This signal is emitted every time the value of the spin box changes
  (whatever the cause - by setValue(), by a keyboard accelerator, by
  mouse clicks etc.).

  Note that it is emitted \e every time, not just for the "final" step
  - if the user clicks 'up' three times, this signal is emitted three
  times.

  \sa value()
*/



/*!  
  Intercepts and handles those events coming to the embedded QLineEdit
  which have special meaning for the QSpinBox.
*/

bool QSpinBox::eventFilter( QObject* obj, QEvent* ev )
{
    if ( obj != vi )
	return FALSE;

    if ( ev->type() == Event_FocusOut ) {
	interpretText();
    }
    else if ( ev->type() == Event_KeyPress ) {
	QKeyEvent* k = (QKeyEvent*)ev;
	if ( k->key() == Key_Up ) {
	    stepUp();
	    k->accept();
	    return TRUE;
	} 
	else if ( k->key() == Key_Down ) {
	    stepDown();
	    k->accept();
	    return TRUE;
	}
	else if ( k->key() == Key_Return ) { // Workaround for use in dialogs
	    interpretText();
	    k->accept();
	    return TRUE;
	}
    }
    return FALSE;
}


/*!  
  Handles resize events for the spin box.  
*/

void QSpinBox::resizeEvent( QResizeEvent* ev )
{
    if ( !up || !down ) // happens if the application has a pointer error
	return;

    QSize bs; // no, it's short for 'button size'
    bs.setHeight( ev->size().height()/2 - frameWidth() );
    if ( bs.height() < 8 )
	bs.setHeight( 8 );
    bs.setWidth( bs.height() * 2 );
    QSize bms( (bs.height()-5)*2-1, bs.height()-4 );
    
    if ( up->size() != bs ) {
	up->resize( bs );
	QBitmap bm( bms );
	QPointArray a;
	a.setPoints( 3,
		     bms.height()-2, 0,
		     0, bms.height()-2,
		     bms.width()-1, bms.height()-2 );
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
	QBitmap bm( bms );
	QPointArray a;
	a.setPoints( 3,
		     bms.height()-2, bms.height()-1,
		     0, 1,
		     bms.width()-1, 1 );
	QPainter p;
	p.begin( &bm );
	p.eraseRect( 0, 0, bm.width(), bm.height() );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
	down->setPixmap( bm );
    }

    int x = ev->size().width() - frameWidth() - bs.width();

    up->move( x, frameWidth() );
    down->move( x, height() - frameWidth() - up->height() );

    vi->setGeometry( frameWidth(), frameWidth(),
		     (x - frameWidth())-1, height() - 2*frameWidth() );
}


/*!  
  This method gets called by QRangeControl whenever the value has changed.
  Updates the display and emits the valueChanged() signal.
*/

void QSpinBox::valueChange()
{
    updateDisplay();
    emit valueChanged( value() );
}


/*!
  This method gets called by QRangeControl whenever the range has
  changed. Adjusts the default validator of the embedded QLineEdit
  and updates the display.
*/

void QSpinBox::rangeChange()
{
    ((QIntValidator*)validator)->setRange( minValue(), maxValue() );
    updateDisplay();
}


/*!  
  Sets the validator of the embedded QLineEdit to \a v. The default is to use
  a suitable QIntValidator.
*/

void QSpinBox::setValidator( QValidator* v )
{
    if ( vi )
	vi->setValidator( v );
}


/*!
  Updates the contents of the embedded QLineEdit to reflect current
  value. Also enables/disables the push buttons accordingly.
*/

void QSpinBox::updateDisplay()
{    
    QString s;
    s.setNum( value() );
    if ( suffix() )
	s.append( suffix() );
    vi->setText( s );
    edited = FALSE;
    up->setEnabled( wrapping() || value() < maxValue() );
    down->setEnabled( wrapping() || value() > minValue() );
}


/*!
  Called after the user has manually edited the contents of the spin
  box. Tries to interpret the text as a legal value, and calls
  setValue() if successful.
*/

void QSpinBox::interpretText()
{
    QString s = text();
    bool ok = FALSE;
    int newVal = s.toInt( &ok );
    if ( !ok && suffix() ) {	// Try removing any suffix
	s = textWithoutSuffix();
	newVal = s.toInt( &ok );
    }
    if ( ok )
	setValue( newVal );
    updateDisplay();		// May be redundant, but sometimes it's not.
}


/*!  
  Returns a pointer to the embedded 'up' button.
*/

QPushButton* QSpinBox::upButton() const
{
    return up;
}


/*!  
  Returns a pointer to the embedded 'down' button.
*/

QPushButton* QSpinBox::downButton() const
{
    return down;
}


/*!  
  Returns a pointer to the embedded QLineEdit.
*/

QLineEdit* QSpinBox::editor() const
{
    return vi;
}


/*!  
  This slot gets called whenever the user edits the text of the spin box.
*/

void QSpinBox::textChanged()
{
    edited = TRUE;	// This flag is cleared in updateDisplay()
};




