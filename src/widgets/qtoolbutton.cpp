/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbutton.cpp#5 $
**
** Implementation of something useful.
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtoolbutton.h"

#include "qdrawutl.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qwmatrix.h"
#include "qapp.h"
#include "qtooltip.h"
#include "qtoolbar.h"
#include "qimage.h"


RCSTAG("$Id: //depot/qt/main/src/widgets/qtoolbutton.cpp#5 $");


static QToolButton * threeDeeButton = 0;


class QToolButtonPrivate
{
};


/*! \class QToolButton qtoolbutton.h

  \brief The QToolButton class provides a push button whose appearance
  has been tailored for use in a QToolBar.

  This means that it implements the ridiculous Microsoft auto-raise
  shite.  And it isn't finished, either.
*/


/*!  Constructs an empty tool button. */

QToolButton::QToolButton( QWidget * parent, const char * name )
    : QButton( parent, name )
{
    init();
    setAutoMinimumSize( TRUE );
    setUsesBigPixmap( FALSE );
}


/*!  Set-up code common to all the constructors */

void QToolButton::init()
{
    d = 0;
    bpID = bp.serialNumber();
    spID = sp.serialNumber();

    utl = FALSE;
    ubp = TRUE;
}


/*!  Creates a tool button that is a child of \a parent (which must be
  a QToolBar) and named \a name.

  The tool button will display \a pm, with text label or tool tip \a
  textLabel and status-bar message \a grouptext, connected to \a slot
  in object \a receiver, and returns the button.

  Note that \a grouptext is not used unless \a parent is managed by a
  QMainWindow.
*/

QToolButton::QToolButton( const QPixmap & pm, const char * textLabel,
			  const char * grouptext,
			  QObject * receiver, const char * slot,
			  QToolBar * parent, const char * name )
    : QButton( parent, name )
{
    init();
    setAutoMinimumSize( TRUE );
    setPixmap( pm );
    setTextLabel( textLabel );
    setUsesBigPixmap( FALSE );
    connect( this, SIGNAL(clicked()), receiver, slot );
    connect( parent, SIGNAL(useBigPixmaps(bool)),
	     this, SLOT(setUsesBigPixmap(bool)) );
    debug( "Sex %s", grouptext );
}


/*! Destroys the object and frees any allocated resources. */

QToolButton::~QToolButton()
{
    delete d;
    threeDeeButton = 0;
}


/*!
  Makes the tool button a toggle button if \e enable is TRUE, or a normal
  tool button if \e enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A tool button is initially not a toggle button.

  \sa setOn(), toggle(), toggleButton() toggled()
*/

void QToolButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*!  Returns a size suitable for this tool button.  This depends on
  \link style() GUI style,\endlink usesBigPixmap(), textLabel() and
  usesTextLabel().
*/

QSize QToolButton::sizeHint() const
{
    int w, h;
    
    if ( text() ) {
	w = fontMetrics().width( text() );
	h = fontMetrics().height(); // boundingRect()?
    } else if ( usesBigPixmap() ) {
	w = h = 32;
    } else {
	w = h = 16;
    }
    
    if ( usesTextLabel() ) {
	h += 4 + fontMetrics().height();
	int tw = fontMetrics().width( textLabel() );
	if ( tw > w )
	    w = tw;
    }
    return QSize( w + 6, h + 6 );
}


/*!  Returns the pixmap used if usesBigPixmap() is TRUE.

  \sa smallPixmap() usesBigPixmap()
*/

QPixmap QToolButton::bigPixmap()
{
    if ( !pixmap() )
	return QPixmap();

    if ( bpID == pixmap()->serialNumber() )
	return sp;

    bpID = pixmap()->serialNumber();
    if ( pixmap()->width() < 21 && pixmap()->height() < 21 ) {
	QImage i( pixmap()->convertToImage() );
	i = i.smoothScale( 24, 24 );
	bp.convertFromImage( i );
    } else {
	bp = *pixmap();
    }
    return bp;
}


/*!

*/

QPixmap QToolButton::smallPixmap()
{
    if ( !pixmap() )
	return QPixmap();

    if ( spID == pixmap()->serialNumber() )
	return sp;

    spID = pixmap()->serialNumber();
    if ( pixmap()->width() < 21 && pixmap()->height() < 21 ) {
	sp = *pixmap();
    } else {
	QImage i( pixmap()->convertToImage() );
	sp.convertFromImage( i.smoothScale( 16, 16 ) );
    }
    return sp;
}


/* \fn bool QToolButton::usesBigPixmap() const

  Returns TRUE or FALSE.

*/


/* \fn bool QToolButton::usesTextLabel() const

  Returns TRUE or FALSE.

*/


/*! \fn const char * QToolButton::textLabel() const

  Returns the text label in use by this tool button, or 0.

  \sa setTextLabel() usesTextLabel() setUsesTextLabel() setText()
*/

/*!

*/

void QToolButton::setUsesBigPixmap( bool enable )
{
    if ( ubp == enable )
	return;

    ubp = enable;

    if ( autoMinimumSize() )
	setMinimumSize( sizeHint() );
    else if ( parent() )
	QApplication::postEvent( parent(), new QEvent( Event_LayoutHint ) );
}


/*!  \fn bool QToolButton::usesBigPixmap() const

  Returns TRUE if this tool button uses the big (32-pixel) pixmaps,
  and FALSE if it does not.  \sa setUsesBigPixmap(), setPixmap(),
  usesTextLabel
*/


/*!

*/

void QToolButton::setUsesTextLabel( bool enable )
{
    if ( utl == enable )
	return;

    utl = enable;

    if ( autoMinimumSize() )
	setMinimumSize( sizeHint() );
    else if ( parent() )
	QApplication::postEvent( parent(), new QEvent( Event_LayoutHint ) );
}


/*! \fn bool QToolButton::usesTextLabel() const

  Returns TRUE if this tool button puts a text label below the button
  pixmap, and FALSE if it does not. \sa setUsesTextLabel()
  setTextLabel() usesBigPixmap()
*/


/*!  Sets this tool button to be on if \a enable is TRUE, and off it
  \a enable is FALSE.

  This function has no effect on \link isToggleButton() non-toggling
  buttons. \endlink

  \sa isToggleButton() toggle()
*/

void QToolButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*!  Toggles the state of this tool button.

  This function has no effect on \link isToggleButton() non-toggling
  buttons. \endlink

  \sa isToggleButton() toggle()
*/

void QToolButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( !isOn() );
}


/*!  Draws the edges and decoration of the button (pretty much
  nothing) and calls drawButtonLabel().

  \sa drawButtonLabel() QButton::paintEvent() */

void QToolButton::drawButton( QPainter * p )
{
    // ### must do something about motif style
    if ( uses3D() ) {
	QPointArray a;
	a.setPoints( 3, 0, height()-1, 0, 0, width()-1, 0 );
	p->setPen( isDown() ? colorGroup().dark() : colorGroup().light() );
	p->drawPolyline( a );
	a[1] = QPoint( width()-1, height()-1 );
	p->setPen( isDown() ? colorGroup().light() : colorGroup().dark() );
	p->drawPolyline( a );
    }
    drawButtonLabel( p );
}


/*!  Draws the contents of the button (pixmap and optionally text).

  \sa drawButton() QButton::paintEvent() */

void QToolButton::drawButtonLabel( QPainter * p )
{
    if ( text() ) {
	qDrawItem( p, style(), 1, 1, width()-2, height()-2,
		   AlignCenter + ShowPrefix,
		   colorGroup(), isEnabled(),
		   0, text() );
    } else {
	int x, y, fh;
	fh = fontMetrics().height();
	x = width()/2 - (usesBigPixmap() ? 16 : 8);
	y = height()/2 - (usesBigPixmap() ? 16 : 8);
	if ( usesTextLabel() )
	    y = y - fh/2 - 2;

	if ( usesBigPixmap() )
	    qDrawItem( p, style(), x, y, 32, 32,
		       AlignCenter + ShowPrefix,
		       colorGroup(), isEnabled(),
		       &bigPixmap(), 0 );
	else
	    qDrawItem( p, style(), x, y, 16, 16,
		       AlignCenter + ShowPrefix,
		       colorGroup(), isEnabled(),
		       &smallPixmap(), 0 );

	if ( usesTextLabel() ) {
	    y += (usesBigPixmap() ? 32 : 16) + 4;
	    p->setFont( font() );
	    qDrawItem( p, style(), 3, y, width()-6, fh,
		       AlignCenter + ShowPrefix,
		       colorGroup(), isEnabled(),
		       0, textLabel() );
	}
    }
}


/*! Reimplemented to handle the automatic 3D effects in Windows style. */

void QToolButton::enterEvent( QEvent * e )
{
    threeDeeButton = this;
    if ( isEnabled() )
	repaint();
    QButton::enterEvent( e );
}


/*! Reimplemented to handle the automatic 3D effects in Windows style. */

void QToolButton::leaveEvent( QEvent * e )
{
    QToolButton * o = threeDeeButton;
    threeDeeButton = 0;
    if ( o && o->isEnabled() )
	o->repaint();
    QButton::leaveEvent( e );
}



/*!  Returns TRUE if this button should be drawn using raised edges in
  Windows style.

  \sa drawButton() */

bool QToolButton::uses3D() const
{
    return threeDeeButton == this && isEnabled();
}


/*!  Sets the label of this button to \a newLabel, and automatically
  sets it as tool tip too if \a tipToo is TRUE.
*/

void QToolButton::setTextLabel( const char * newLabel , bool tipToo )
{
    tl = newLabel;
    if ( !tipToo )
	return;

    if ( usesTextLabel() )
	QToolTip::remove( this );
    else
	QToolTip::add( this, newLabel );
}



