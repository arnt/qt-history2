/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.cpp#145 $
**
** Implementation of QPushButton class
**
** Created : 940221
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

#include "qpushbutton.h"
#include "qdialog.h"
#include "qfontmetrics.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qbitmap.h"

// NOT REVISED
/*!
  \class QPushButton qpushbutton.h
  \brief The QPushButton widget provides a push button with a text or pixmap label.

  \ingroup realwidgets

  The push button (command button in Windows parlance) is perhaps the
  most central widget: Push it to command the computer to perform some
  action.

  Being so central, the widget has grown to accomodate a great many
  variations in the past decade, and by now the
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui162.htm">"comand
  button" chapter</a> of the Microsoft style guide shows about ten
  different states of Windows push buttons, and the text implies that
  there are dozens more when all the combinations of features are taken
  into consideration.

  The most important modes or states are, sorted roughly by importance: <ul>
  <li> Available or not ("grayed out", disabled).
  <li> Standard push button or toggling push button.
  <li> On or off (only for toggling push buttons).
  <li> Default or normal.  The default button in a dialog can
  generally be "clicked" using the Enter or Return key.
  <li> Auto-repeat or not.
  <li> Pressed down or not.
  </ul>

  <img src="qpushbt-m.png"> <img src="qpushbt-w.png">

  In Qt, the QButton class provides most of the modes and other API,
  and QPushButton provides GUI logic.  See QButton for more
  information about the API.

  \sa QRadioButton QToolButton
  <a href="guibooks.html#fowler">GUI Design Handbook: Push Button</a>
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui162.htm">Microsoft
  Style Guide</a>
*/


/*!
  Constructs a push button with no text.

  The \a parent and \a name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*!
  Constructs a push button with a text.

  The \a parent and \a name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( const QString &text, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}


/*!
  Constructs a push button as arrow button. The ArrowType \a type
  defines the arrow direction. Possible values are LeftArrow,
  RightArrow, UpArrow and DownArrow.

  The \a parent and \a name arguments are sent to the QWidget constructor.
*/
QPushButton::QPushButton( ArrowType type, QWidget *parent, const char *name )
    : QButton( parent, name )
{
    init();

    setAutoRepeat( TRUE );
    // abuse text due to the lack of a bit flag ### fix 3.0
    switch ( type ) {
    case LeftArrow:
	setText("left");
	break;
    case RightArrow:
	setText("right");
	break;
    case UpArrow:
	setText("up");
	break;
    default: //LeftArrow:
	setText("down");
	break;
    }
    hasArrow = TRUE;
}


void QPushButton::init()
{
    defButton = FALSE;
    lastDown = FALSE;
    lastEnabled = FALSE;
    hasMenuArrow = FALSE;
    autoDefButton = TRUE;
    autoDefDefined = FALSE;
    hasArrow = FALSE;
    setBackgroundMode( PaletteButton );
}


/*!
  Makes the push button a toggle button if \a enable is TRUE, or a normal
  push button if \a enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A push button is initially not a toggle button.

  \sa setOn(), toggle(), isToggleButton() toggled()
*/

void QPushButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
    if ( !autoDefDefined )
	autoDefButton = FALSE;
}


/*!
  Switches a toggle button on if \a enable is TRUE or off if \a enable is
  FALSE.
  \sa isOn(), toggle(), toggled(), isToggleButton()
*/

void QPushButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*!
  Toggles the state of a toggle button.
  \sa isOn(), setOn(), toggled(), isToggleButton()
*/

void QPushButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( !isOn() );
}


/*! \fn bool QPushButton::autoDefault() const

  Returns TRUE if the button is an auto-default button.

  \sa setAutoDefault()
*/

/*!
  Sets the push buttons to an auto-default button if \a enable is TRUE,
  or to a normal button if \a enable is FALSE.

  An auto-default button becomes the default push button automatically
  when it receives the keyboard input focus.

  \sa autoDefault(), setDefault()
*/

void QPushButton::setAutoDefault( bool enable )
{
    autoDefButton = enable;
    autoDefDefined = TRUE;
}


/*!
  \fn bool QPushButton::isDefault() const
  Returns TRUE if the button is currently default.

  \sa setDefault()
*/

/*!
  Sets this button to be the current default button of a
  \link QDialog dialog\endlink if \a enable is TRUE, or to be a normal button
  if \a enable is FALSE.

  The current default button gets clicked when the user presses the "Enter"
  key, independently of which widget in the dialog currently has the keyboard
  input focus. Only one push button can at any time be the default button.

  The default button behaviour is only provided in dialogs. Buttons can always
  be clicked from the keyboard by pressing the spacebar whern the button has
  focus.

  \sa isDefault(), setAutoDefault(), QDialog
*/

void QPushButton::setDefault( bool enable )
{
    if ( (defButton && enable) || !(defButton || enable) )
	return;					// no change
    QWidget *p = topLevelWidget();
    if ( !p->inherits("QDialog") )		// not a dialog
	return;
    defButton = enable;
    if ( defButton )
	((QDialog*)p)->setDefault( this );
    if ( isVisible() )
	repaint( FALSE );
}


/*!
  Returns a size which fits the contents of the push button.
*/

QSize QPushButton::sizeHint() const
{
    int w = 0;
    int h = 0;
    if ( hasArrow )
	return QSize( 16, 16 );

    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w = pm->width()	 + 6;
	h = pm->height() + 6;
    } else {
	QString s( text() );
	if ( s.isEmpty() )
	    s = QString::fromLatin1("XXXX");
	QFontMetrics fm = fontMetrics();
	QSize sz = fm.size( ShowPrefix, s );
	w = sz.width()	+ 6;
	h = sz.height() + sz.height()/8 + 10;
	w += h;
    }

    bool adb = autoDefButton;
    if ( adb && !autoDefDefined && autoRepeat() ) {
	QPushButton* that  = (QPushButton*) this;
	that->autoDefButton = FALSE;
	adb = FALSE;
    }

    if ( adb || isDefault() ) {
	w += 2*style().buttonDefaultIndicatorWidth();
	h += 2*style().buttonDefaultIndicatorWidth();
    }

    if ( style() == WindowsStyle ) {
	// in windows style, try a little harder to conform to
	// microsoft's size specifications
	if ( h <= 25 )
	    h = 22;
	if ( w < 85 && !pixmap() &&
	     topLevelWidget() &&
	     topLevelWidget()->inherits( "QDialog" ) )
	    w = 80;
    }

    return QSize( w, h );
}


/*!
  Specifies that this widget may stretch horizontally, but is fixed
  vertically.
*/

QSizePolicy QPushButton::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}



/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QPushButton::move( int x, int y )
{
    QWidget::move( x, y );
}

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QPushButton::move( const QPoint &p )
{
    move( p.x(), p.y() );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QPushButton::resize( int w, int h )
{
    QWidget::resize( w, h );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QPushButton::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QPushButton::setGeometry( int x, int y, int w, int h )
{
    QWidget::setGeometry( x, y, w, h );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QPushButton::setGeometry( const QRect &r )
{
    QWidget::setGeometry( r );
}

void QPushButton::resizeEvent( QResizeEvent * )
{
    if ( autoMask() )
	updateMask();
}

/*!
  Draws the push button, except its label.
  \sa drawButtonLabel()
*/

void QPushButton::drawButton( QPainter *paint )
{
    int diw = 0;
    if ( isDefault() || autoDefault() ) {
	diw = style().buttonDefaultIndicatorWidth();
	if ( !autoDefDefined && autoRepeat() ) {
	    autoDefButton = FALSE;
	    diw = 0;
	}
	if ( diw > 0 ) {
	    if ( parentWidget() && parentWidget()->backgroundPixmap() ){
		// pseudo tranparency
		paint->drawTiledPixmap( 0, 0, width(), diw,
				    *parentWidget()->backgroundPixmap(),
				    x(), y() );
		paint->drawTiledPixmap( 0, 0, diw, height(),
				    *parentWidget()->backgroundPixmap(),
				    x(), y() );
		paint->drawTiledPixmap( 0, height()-diw, width(), diw,
				    *parentWidget()->backgroundPixmap(),
				    x(), y() );
		paint->drawTiledPixmap( width()-diw, 0, diw, height(),
				    *parentWidget()->backgroundPixmap(),
				    x(), y() );
	    } else {
		paint->fillRect( 0, 0, width(), diw, colorGroup().brush(QColorGroup::Background) );
		paint->fillRect( 0, 0, diw, height(), colorGroup().brush(QColorGroup::Background) );
		paint->fillRect( 0, height()-diw, width(), diw, colorGroup().brush(QColorGroup::Background) );
		paint->fillRect( width()-diw, 0, diw, height(), colorGroup().brush(QColorGroup::Background) );
	    }
	}
    }

    style().drawPushButton(this, paint);
    drawButtonLabel( paint );
    int x1, y1, x2, y2;
    style().buttonRect( 0,0,width(),height()).coords( &x1, &y1, &x2, &y2 );	// get coordinates
    if ( hasFocus() ) {
 	QRect r(x1+3+diw, y1+3+diw, x2-x1-5-2*diw, y2-y1-5-2*diw);
 	style().drawFocusRect( paint, r , colorGroup(), &colorGroup().button() );
     }
    lastDown = isDown();
    lastEnabled = isEnabled();
}


/*!
  Draws the push button label.
  \sa drawButton()
*/

void QPushButton::drawButtonLabel( QPainter *paint )
{
    if ( hasArrow ) {
	int x, y, w, h;
	rect().rect( &x, &y, &w, &h );
	if ( isDown() || isOn() ){
	    int sx = 0;
	    int sy = 0;
	    style().getButtonShift(sx, sy);
	    x+=sx;
	    y+=sy;
	}
	int fw = style().defaultFrameWidth();
	x += fw;  y += fw;  w -= 2*fw;  h -= 2*fw;
	ArrowType arrow = DownArrow;
	if ( text()[0] == 'l' )
	    arrow = LeftArrow;
	else if ( text()[0] == 'r' )
	    arrow = RightArrow;
	else if ( text()[0] == 'u' )
	    arrow = UpArrow;
	style().drawArrow( paint, arrow, isDown(), x, y, w, h, colorGroup(), isEnabled() );
	return;
    }
    style().drawPushButtonLabel( this, paint );
}


void QPushButton::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );

    {
	QPainter p( &bm, this );
	p.setPen( color1 );
	p.setBrush( color1 );
	style().drawButtonMask( &p, 0, 0, width(), height() );
    }
    setMask( bm );
}

/*!
  Handles focus in events for the push button.
*/

void QPushButton::focusInEvent( QFocusEvent *e )
{
    if ( autoDefButton )
	setDefault( TRUE );
    QButton::focusInEvent( e );
}

/*!
  Handles focus out events for the push button.
*/

void QPushButton::focusOutEvent( QFocusEvent *e )
{
    if ( defButton && autoDefButton ) {
	QWidget *p = topLevelWidget();
	if ( p->inherits("QDialog") )
	    ((QDialog*)p)->setDefault( 0 );
    }

    QButton::focusOutEvent( e );
}



/*!  Tells this button to draw a menu indication triangle if \a enable
  is TRUE,  and to not draw one if \a enable is FALSE (the default).

  setIsMenuButton() does not cause the button to do anything other
  than draw the menu indication.

  \sa isMenuButton()
*/

void QPushButton::setIsMenuButton( bool enable )
{
    if ( (bool)hasMenuArrow == enable )
	return;
    hasMenuArrow = enable ? 1 : 0;
    repaint( FALSE );
}


/*!  Returns TRUE if this button indicates to the user that pressing
  it will pop up a menu, and FALSE otherwise.  The default is FALSE.

  \sa setIsMenuButton()
*/

bool QPushButton::isMenuButton() const
{
    return hasMenuArrow;
}


