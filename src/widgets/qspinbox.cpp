/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.cpp#24 $
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
  clicking the up/down buttons to increase/decrease the value
  currently displayed, or by typing the value directly into the spin
  box. 

  Every time the value changes, QSpinBox emits the valueChanged()
  signal. The current value can be fetched with value() and set with
  setValue().

  The spin box clamps the value within a numeric range, see
  QRangeControl for details. Clicking the up/down down buttons (or
  using the keyboard accelerators: Up-arrow and Down-arrow) will
  increase or decrease the current value in steps of size lineStep().

  Most spin boxes are directional, but QSpinBox can also operate as a
  circular spin box, i.e. if the range is 0-99 and the current value
  is 99, clicking Up will give 0. Use setWrapping() to if you want
  circular behavior.

  The displayed value can be appended with an arbitray string
  indicating for example the unit of measurement. See setSuffix().

  It is often desirable to give the user a special, often default,
  choice in addition to the range of numeric values. See
  setMinValueText() for how to do this with QSpinBox.

  The default \link setFocusPolicy() focus policy \endlink is
  StrongFocus.

  QSpinBox can easily be subclassed to allow the user to input other
  things than a numeric value, as long as the allowed input can be
  mapped down to a range of integers. Override the virtual functions
  mapValueToText() and mapTextToValue() and set another, suitable
  validator using setValidator().

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

  \sa text(), suffix(), setSuffix()
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
    return s.stripWhiteSpace();
}


/*!
  Sets the minimum-value text to \a text. If set, the spin box will
  display this text in stead of a numeric value whenever the current
  value is equal to minVal(). Typically used for indicating that this
  choice has a special (default) meaning. 
  
  For example, if you use a spin box for letting the user choose
  margin width in a print dialog, and your application is able to
  automatically choose a good margin width, you can set up the spin
  box like this:
  \code
    QSpinBox marginBox( -1, 20, 1, parent, "marginBox" );
    marginBox->setSuffix( " mm" );
    marginBox->setMinValueText( "Auto" );
  \endcode 
  The user will then be able to choose a margin width from 0-20
  millimeters, or select "Auto" to leave it to the application to
  choose. Your code must then interpret the spin box value of -1 as
  the user requesting automatic margin width.

  The \link setSuffix suffix,\endlink if set, is not appended to the
  minimum-value text when displayed.

  To turn off the minimum-value text display, call this function with
  0 or an empty string as parameter. The default is no minimum-value
  text, i.e. the numeric value is shown as usual.
  
  \sa minValueText()
*/

void QSpinBox::setMinValueText( const char* text )
{
    minText = text;
    updateDisplay();
}


/*!
  Returns the currently minimum-value text, or 0 if no minimum-value
  text is currently set.

  \sa setMinValueText()
*/

const char* QSpinBox::minValueText() const
{
    if ( minText.isEmpty() )
	return 0;
    else
	return minText;
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

  \sa setSuffix()
*/

const char* QSpinBox::suffix() const
{
    if ( sfix.isEmpty() )
	return 0;
    else
	return sfix;
}


/*!
  Setting wrapping to TRUE will allow the value to be stepped from the
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
    int w = 35; 	// minimum width for the value
    int wx = fm.width( "  " );
    QString s;
    s.setNum( minValue() );
    s.append( suffix() );
    w = QMAX( w, fm.width( s ) + wx );
    s.setNum( maxValue() );
    s.append( suffix() );
    w = QMAX( w, fm.width( s ) + wx );
    s = minValueText();
    w = QMAX( w, fm.width( s ) + wx );
    
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
  value, using mapValueToText(). Also enables/disables the push
  buttons accordingly.

  \sa mapValueToText()
*/

void QSpinBox::updateDisplay()
{    
    if ( (value() == minValue()) && minValueText() ) {
	vi->setText( minText );
    }
    else { 
	QString s = mapValueToText( value() );
	if ( suffix() )
	    s.append( suffix() );
	vi->setText( s );
    }
    edited = FALSE;
    up->setEnabled( wrapping() || value() < maxValue() );
    down->setEnabled( wrapping() || value() > minValue() );
}


/*!
  Called after the user has manually edited the contents of the spin
  box. Interprets the text using mapTextToValue(), and calls
  setValue() if successful.
*/

void QSpinBox::interpretText()
{
    bool ok = TRUE;
    bool done = FALSE;
    int newVal = 0;
    if ( minValueText() ) {
	QString s = QString(text()).stripWhiteSpace();
	QString t = minText.stripWhiteSpace();
	if ( s == t ) {
	    newVal = minValue();
	    done = TRUE;
	}
    }
    if ( !done )
	newVal = mapTextToValue( &ok );
    if ( ok )
	setValue( newVal );
    updateDisplay();		// Sometimes redundant
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




/*!
  This virtual function is used by the spin box whenever it needs to
  display value \a v. The default implementation returns a string
  containing \a v printed in the standard way.

  Override this in in a subclass if you want a specialized spin box,
  handling something else than integers. This function need not be
  concerned with \link setSuffix() suffix \endlink or \link
  setMinValueText() minimum-value text, \endlink the QSpinBox handles
  that automatically.

  \sa updateDisplay(), mapTextToValue()
*/

QString QSpinBox::mapValueToText( int v )
{
    QString s;
    s.setNum( v );
    return s;
}


/*!
  This virtual function is used by the spin box whenever it needs to
  interpret the text entered by the user as a value. The default
  implementation tries to interpret it as an integer in the standard
  way, and returns the integer value.

  Override this in in a subclass if you want a specialized spin box,
  handling something else than integers. It should call text() (or
  textWithoutSuffix() ) and return the value corresponding to that
  text. If the text does not represent a legal value
  (uninterpretable), the bool pointed to by \a ok should be set to
  FALSE.

  This function need not be concerned with \link setMinValueText()
  minimum-value text, \endlink the QSpinBox handles that
  automatically.
  
  \sa interpretText(), mapValueToText()
*/

int QSpinBox::mapTextToValue( bool* ok )
{
    QString s = text();
    int newVal = s.toInt( ok );
    if ( !(*ok) && suffix() ) {	// Try removing any suffix
	s = textWithoutSuffix();
	newVal = s.toInt( ok );
    }
    return newVal;
}


/*!
  Sets the palette of the spin box to \a p.
*/

void QSpinBox::setPalette( const QPalette& p )
{
    // Currently just uses default implementation:
    QWidget::setPalette( p );
}
