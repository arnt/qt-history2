/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbt.cpp#102 $
**
** Implementation of QPushButton class
**
** Created : 940221
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpushbt.h"
#include "qdialog.h"
#include "qfontmet.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qpixmap.h"
#include "qpmcache.h"
#include "qbitmap.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qpushbt.cpp#102 $");


/*!
  \class QPushButton qpushbt.h
  \brief The QPushButton widget provides a push button with a text
	    or pixmap label.

  \ingroup realwidgets

  A default push button in a dialog emits the clicked signal if the user
  presses the Enter key.

  A push button has \c TabFocus as a default focusPolicy(), i.e. it can 
  get keyboard focus by tabbing but not by clicking.

  <img src=qpushbt-m.gif> <img src=qpushbt-w.gif>
*/


/*!
  Constructs a push button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*!
  Constructs a push button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( const char *text, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}

void QPushButton::init()
{
    autoDefButton = defButton = lastDown = lastDef = lastEnabled = FALSE;
}


/*!
  Makes the push button a toggle button if \e enable is TRUE, or a normal
  push button if \e enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A push button is initially not a toggle button.

  \sa setOn(), toggle(), toggleButton() toggled()
*/

void QPushButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*!
  Switches a toggle button on if \e enable is TRUE or off if \e enable is
  FALSE.
  \sa isOn(), toggle(), toggled(), toggleButton()
*/

void QPushButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*!
  Toggles the state of a toggle button.
  \sa isOn(), setOn(), toggled(), toggleButton()
*/

void QPushButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( !isOn() );
}


/*!
  \fn bool QPushButton::autoDefault() const
  Returns TRUE if the button is an auto-default button.

  \sa setAutoDefault()
*/

/*!
  Sets the push buttons to an auto-default button if \e enable is TRUE,
  or to a normal button if \e enable is FALSE.

  An auto-default button becomes the default push button automatically
  when it receives the keyboard input focus.

  \sa autoDefault(), setDefault()
*/

void QPushButton::setAutoDefault( bool enable )
{
    autoDefButton = enable;
}


/*!
  \fn bool QPushButton::isDefault() const
  Returns TRUE if the button is default.

  \sa setDefault()
*/

/*!
  Sets the button to be the default button if \e enable is TRUE, or
  to be a normal button if \e enable is FALSE.

  A default push button in a \link QDialog dialog\endlink emits the
  QButton::clicked() signal if the user presses the Enter key.	Only
  one push button in the dialog can be default.

  Default push buttons are only allowed in dialogs.

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
    int w, h;
    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w = pm->width()	 + 6;
	h = pm->height() + 6;
    } else {
	QString s( text() );
	if ( s.isEmpty() )
	    s = "XXXX";
	QFontMetrics fm = fontMetrics();
	QRect br = fm.boundingRect( s );
	w = br.width()	+ 6;
	h = fm.height() + fm.height()/8 + 10;
	w += h;
    }
    return QSize( w, h );
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


/*!
  Draws the push button, except its label.
  \sa drawButtonLabel()
*/

void QPushButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle gs = style();
    QColorGroup g  = colorGroup();
    bool updated = ( isDown() != (bool)lastDown ||
		     lastDef != defButton ||
		     isEnabled() != (bool)lastEnabled);

    int x1, y1, x2, y2;

    rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

    int w = x2 + 1;
    int h = y2 + 1;
    int dx = 0;
    int dy = 0;

    p->setPen( g.foreground() );
    p->setBrush( QBrush(g.background(),NoBrush) );

    if ( gs == WindowsStyle ) {		// Windows push button
	if ( isDown() ) {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
		p->setPen( g.dark() );
		p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	    } else {
		qDrawWinButton( p, x1, y1, w, h, g, TRUE );
	    }
	} else {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, w, h );
		x1++; y1++;
		x2--; y2--;
	    }
	    if ( isToggleButton() && isOn() && isEnabled() ) {
		QBrush fill(white, Dense4Pattern );
		qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, &fill );
		updated = FALSE;
	    } else {
		qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, isOn() );
	    }
	}
	// ### next two lines ignore backgroundPixmap()
	if ( updated )
	    p->fillRect( x1+2, y1+2, x2-x1-3, y2-y1-3, g.background() );
    } else if ( gs == MotifStyle ) {		// Motif push button
	QBrush fill;
	if ( isDown() )
	    fill = QBrush( g.mid() );
	else if ( isOn() )
	    fill = QBrush( g.mid(), Dense4Pattern );
	else
	    fill = QBrush( g.background() );

	qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, isOn() || isDown(),
			 2, &fill );

	if ( defButton ) {			// default Motif button
	    int by1, by2, by3, by4, by5, by6;	// top to bottom
	    int bx1, bx2, bx3, bx4;		// left to right

	    by4 = (y2-y1)/2;			// arrowhead
	    bx1 = x2 - 2 - (y2 - y1 - 4);
	    if ( x2 - bx1 - 3*by4/2 > 10 )	// but not too far from the
		bx1 = x2 - 3*by4/2 - 10; 	// right edge of the button
	    by2 = by4 / 2 + 1;			// top end of arrow
	    bx2 = bx1 + (by4-by2);
	    by6 = by4 + (by4-by2);		// bottom end of arrow
	    by3 = by4 - (by4-by2)/2;		// top end of stem
	    by5 = by4 + (by4-by3);		// bottom end of stem
	    bx3 = bx2 + (bx2-bx1) + 1;		// left side of stem
	    bx4 = bx3 + (by5-by3) - 1;		// right side of stem
	    by1 = by2 - 1;			// end of stem

	    QPointArray a;
	    p->setPen( g.dark() );
	    a.setPoints( 7,
			 bx4, by1,
			 bx3, by1,
			 bx3, by3,
			 bx2, by3,
			 bx2, by2,
			 bx1, by4,
			 bx2, by6 );
	    p->drawPolyline( a );

	    p->setPen( g.light() );
	    a.setPoints( 4, 
			 bx2, by6,
			 bx2, by5,
			 bx4, by5,
			 bx4, by1+1 );
	    p->drawPolyline( a );

	    dx = (y1-y2-4)/3;			// translate button label
	}
    }

    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );

    if ( dx || dy )
	p->translate( dx, dy );
    drawButtonLabel( p );
    if ( dx || dy )
	p->translate( -dx, -dy );

    if ( hasFocus() ) {
	if ( style() == WindowsStyle ) {
	    p->drawWinFocusRect( x1+3, y1+3, x2-x1-5, y2-y1-5, 
				 backgroundColor() );
	} else {
	    p->setPen( black );
	    p->drawRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
	}
    }

    lastDown = isDown();
    lastDef = defButton;
    lastEnabled = isEnabled();
}


/*!
  Draws the push button label.
  \sa drawButton()
*/

void QPushButton::drawButtonLabel( QPainter *paint )
{
    register QPainter *p = paint;

    QRect r = rect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( (isDown() || isOn()) && style() == WindowsStyle ) {
        // shift pixmap/text
	x++;
	y++;
    }
    x += 2;  y += 2;  w -= 4;  h -= 4;
    qDrawItem( p, style(), x, y, w, h,
	       AlignCenter|ShowPrefix,
	       colorGroup(), isEnabled(),
	       pixmap(), text() );
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

