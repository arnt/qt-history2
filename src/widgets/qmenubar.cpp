/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.cpp#48 $
**
** Implementation of QMenuBar class
**
** Author  : Haavard Nord
** Created : 941209
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 INCLUDE_MENUITEM_DEF
#include "qmenubar.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qapp.h"
#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qmenubar.cpp#48 $")


/*!
  \class QMenuBar qmenubar.h

  \brief The QMenuBar class provides a horizontal menu bar.

  \ingroup realwidgets

  It automatically sets its own geometry to the top of the parent
  widget and changes appropriately it when the parent widget is
  resized.

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  \sa QPopupMenu

  */


// Motif style parameters

static const motifBarFrame	= 2;		// menu bar frame width
static const motifBarHMargin	= 2;		// menu bar hor margin to item
static const motifBarVMargin	= 2;		// menu bar ver margin to item
static const motifItemFrame	= 2;		// menu item frame width
static const motifItemHMargin	= 8;		// menu item hor text margin
static const motifItemVMargin	= 8;		// menu item ver text margin

/*

+-----------------------------
|      BarFrame
|   +-------------------------
|   |	   V  BarMargin
|   |	+---------------------
|   | H |      ItemFrame
|   |	|  +-----------------
|   |	|  |			   \
|   |	|  |  ^	 T E X T   ^	    | ItemVMargin
|   |	|  |  |		   |	   /
|   |	|      ItemHMargin
|   |
|

*/


/*****************************************************************************
  QMenuBar member functions
 *****************************************************************************/


/*!
  Creates a menu bar with a \e parent and a \e name.
  */

QMenuBar::QMenuBar( QWidget *parent, const char *name )
    : QFrame( parent, name, 0, FALSE )
{
    initMetaObject();
    isMenuBar = TRUE;
    autoaccel = 0;
    irects    = 0;
    if ( parent )				// filter parent events
	parent->installEventFilter( this );
    move( 0, 0 );
    if ( style() == MotifStyle ) {
	setFrameStyle( QFrame::Panel | QFrame::Raised );
	setLineWidth( motifBarFrame );
    }
    else {
	setFrameStyle( QFrame::NoFrame );
    }
}

/*!
  Destroys the menu bar.
  */

QMenuBar::~QMenuBar()
{
    delete autoaccel;
    delete [] irects;
    if ( parent() )
	parent()->removeEventFilter( this );
}


void QMenuBar::updateItem( int )		// update menu bar item
{
    repaint( FALSE );
}


void QMenuBar::menuContentsChanged()
{
    badSize = TRUE;				// might change the size
    if ( isVisible() ) {
	updateRects();
	repaint();
    }
}

void QMenuBar::menuStateChanged()
{
    repaint();
}

void QMenuBar::menuInsPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;			// set parent menu
    connect( popup, SIGNAL(activatedRedirect(int)),
	     SLOT(subActivated(int)) );
    connect( popup, SIGNAL(highlightedRedirect(int)),
	     SLOT(subHighlighted(int)) );
}

void QMenuBar::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;
    popup->disconnect( SIGNAL(activatedRedirect(int)), this,
		       SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(highlightedRedirect(int)), this,
		       SLOT(subHighlighted(int)) );
}

void QMenuBar::frameChanged()
{
    menuContentsChanged();
}


/*!  This function is used to adjust the menu bar's geometry to the
  parent widget's.

  \internal
  Resizes the menu bar to fit in the parent widget when the parent receives
  a resize event.  */

bool QMenuBar::eventFilter( QObject *object, QEvent *event )
{
    if ( object == parent() && event->type() == Event_Resize ) {
	QResizeEvent *e = (QResizeEvent *)event;
	setGeometry( 0, y(), e->size().width(), height() );
	updateRects();
    }
    return FALSE;				// don't stop event
}



/*!
  \internal
  Receives signals from menu items.
  */

void QMenuBar::subActivated( int id )
{
    emit activated( id );
}

/*!
  \internal
  Receives signals from menu items.
  */

void QMenuBar::subHighlighted( int id )
{
    emit highlighted( id );
}

/*!
  \internal
  Receives signals from menu accelerator.
  */

void QMenuBar::accelActivated( int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi && !mi->isDisabled() ) {
	actItem = indexOf( id );
	repaint( FALSE );
	QPopupMenu *popup = mi->popup();
	if ( popup ) {
	    emit highlighted( mi->id() );
	    if ( popup->isVisible() ) {		// sub menu already open
		popup->hidePopups();
		popup->repaint( FALSE );
	    }
	    else {				// open sub menu
		hidePopups();
		openActPopup();
		popup->setFirstItemActive();
	    }
	}
	else {					// not a popup
	    actItem = -1;
	    repaint( FALSE );
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    else				// normal connection
		emit activated( mi->id() );
	}
    }
}

void QMenuBar::accelDestroyed()			// accel about to be deleted
{
    autoaccel = 0;				// don't delete it twice!
}


bool QMenuBar::tryMouseEvent( QPopupMenu *popup, QMouseEvent *e )
{
    QPoint pos = mapFromGlobal( popup->mapToGlobal( e->pos() ) );
    if ( !rect().contains( pos ) )		// outside
	return FALSE;
    int item = itemAtPos( pos );
    if ( item == -1 && (e->type() == Event_MouseButtonPress ||
			e->type() == Event_MouseButtonRelease) ) {
	hidePopups();
	goodbye();
	return FALSE;
    }
    QMouseEvent ee( e->type(), pos, e->button(), e->state() );
    event( &ee );
    return TRUE;
}

void QMenuBar::tryKeyEvent( QPopupMenu *, QKeyEvent *e )
{
    event( e );
}


void QMenuBar::goodbye()			// set to idle state
{
    actItem = -1;
    repaint( FALSE );
}


void QMenuBar::openActPopup()			// open active popup menu
{
    if ( actItem < 0 )
	return;
    QPopupMenu *popup = mitems->at(actItem)->popup();
    if ( popup ) {
	QRect  r = itemRect( actItem );
	QPoint pos = r.bottomLeft() + QPoint(0,1);
	if ( popup->badSize )
	    popup->updateSize();
	pos = mapToGlobal(pos);
	int sh = QApplication::desktop()->height();
	int ph = popup->height();
	if ( pos.y() + ph > sh ) {
	    pos = mapToGlobal( r.topLeft() );
	    pos.ry() -= (QCOORD)ph;
	}
	popup->popup( pos );
    }
}

/*!
  \internal
  Hides all popup menu items.
  */

void QMenuBar::hidePopups()			// hide popup items
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() )
	    mi->popup()->hide();
    }
}


/*! Reimplements QWidget::setFont() for performance purposes. */

void QMenuBar::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    update();
}

/*!  Reimplements QWidget::show() in order to set up the correct
  keyboard accelerators and raise itself to the top of the widget
  stack.  */

void QMenuBar::show()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    QWidget *w = parentWidget();
    while ( (mi=it.current()) ) {
	++it;
	QString s = mi->string();
	if ( !s.isEmpty() ) {
	    int i = s.find( '&' );
	    if ( i >= 0 && isalnum(s[i+1]) ) {
		int k = s[i+1];
		if ( isalpha(k) )
		    k = toupper(k) - 'A' + Key_A;		
		if ( !autoaccel ) {
		    autoaccel = new QAccel( this );
		    CHECK_PTR( autoaccel );
		    connect( autoaccel, SIGNAL(activated(int)),
			     SLOT(accelActivated(int)) );
		    connect( autoaccel, SIGNAL(destroyed()),
			     SLOT(accelDestroyed()) );
		}
		autoaccel->insertItem( ALT+k, mi->id() );
	    }
	}
	if ( mi->popup() ) {
	    mi->popup()->updateAccel( this );
	    if ( mi->popup()->isDisabled() )
		mi->popup()->enableAccel( FALSE );
	}
    }
    if ( w )
	resize( w->width(), height() );
    updateRects();
    QWidget::show();
    raise();
}

/*!
  Reimplements QWidget::hide() in order to deselect any selected item.
  */

void QMenuBar::hide()
{
    actItem = -1;
    hidePopups();
    QWidget::hide();
}


/*****************************************************************************
  Item geometry functions
 *****************************************************************************/

void QMenuBar::updateRects()
{
    if ( !badSize )				// size was not changed
	return;
    delete [] irects;
    if ( mitems->isEmpty() ) {
	irects = 0;
	return;
    }
    irects = new QRect[ mitems->count() ];	// create rectangle array
    CHECK_PTR( irects );
    QFontMetrics fm = fontMetrics();
    int max_width = width();
    int max_height = 0;
    int nlitems = 0;				// number on items on cur line
    int gs = style();
    int x = motifBarFrame + motifBarHMargin;
    int y = motifBarFrame + motifBarVMargin;
    int i = 0;
    if ( gs == WindowsStyle )	// !!!hanord
	x = y = 2;
    while ( i < (int)mitems->count() ) {	// for each menu item...
	QMenuItem *mi = mitems->at(i);
	int w, h;
	if ( mi->pixmap() ) {			// pixmap item
	    w = mi->pixmap()->width();
	    h = mi->pixmap()->height();
	}
	else {					// text item
	    w = fm.width( mi->string() ) + 2*motifItemHMargin;
	    h = fm.height() + motifItemVMargin;
	}
	if ( gs == MotifStyle ) {
	    w += 2*motifItemFrame;
	    h += 2*motifItemFrame;
	}
	if ( x + w + motifBarFrame > max_width && nlitems > 0 ) {
	    nlitems = 0;
	    x = motifBarFrame + motifBarHMargin;
	    y += h + motifBarHMargin;
	}
	if ( y + h + 2*motifBarFrame > max_height )
	    max_height = y + h + 2*motifBarFrame;
	irects[i].setRect( x, y, w, h );
	x += w;
	nlitems++;
	i++;
    }
    if ( max_height != height() )
	resize( max_width, max_height );
    badSize = FALSE;
}

/*!
  \internal
  Return the bounding rectangle for the menu item at position \e index.
  */

QRect QMenuBar::itemRect( int index )
{
    updateRects();
    return irects ? irects[index] : QRect(0,0,0,0);
}

/*!
  \internal
  Return the item at \e pos, or -1 if there is no item there, or if
  it is a separator item.
  */

int QMenuBar::itemAtPos( const QPoint &pos )
{
    updateRects();
    if ( !irects )
	return -1;
    int i = 0;
    while ( i < (int)mitems->count() ) {
	if ( irects[i].contains( pos ) ) {
	    QMenuItem *mi = mitems->at(i);
	    return mi->isSeparator() ? -1 : i;
	}
	++i;
    }
    return -1;					// no match
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/


/*!
  Called from QFrame::paintEvent().
  */

void QMenuBar::drawContents( QPainter *p )	// draw menu bar
{
    QColorGroup	 g  = colorGroup();
    QFontMetrics fm = fontMetrics();
    int		 fw = frameWidth();
    int		 gs = style();

    p->setClipRect( fw, fw, width() - 2*fw, height() - 2*fw );

    for ( int i=0; i<(int)mitems->count(); i++ ) {
	QMenuItem *mi = mitems->at( i );
	QRect r = irects[i];
	if ( gs == WindowsStyle )
	    p->fillRect( r, i == actItem ? darkBlue : g.background() );
	else if ( gs == MotifStyle ) {
	    if ( i == actItem )				// active item frame
		qDrawShadePanel( p, r, g, FALSE, motifItemFrame );
	    else					// incognito frame
		qDrawPlainRect( p, r, g.background(), motifItemFrame );
	}
	if ( mi->pixmap() )
	    p->drawPixmap( r.left() + motifItemFrame,
			   r.top() + motifItemFrame,
			   *mi->pixmap() );
	else if ( mi->string() ) {
	    if ( mi->isDisabled() && mi->popup() == 0 )
		p->setPen( palette().disabled().text() );
	    else {
		if ( i == actItem && gs == WindowsStyle )
		    p->setPen( white );
		else
		    p->setPen( g.text() );
	    }
	    p->drawText( r, AlignCenter | ShowPrefix | DontClip,
			 mi->string() );
	}
    }
}

/*!
  Handles mouse press events for the menu bar.

  All the mouse buttons are treated equally.
  */

void QMenuBar::mousePressEvent( QMouseEvent *e )
{
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	actItem = -1;
	repaint( FALSE );
	return;
    }
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item highlighted
	actItem = item;
	repaint( FALSE );
	if ( mi->id() >= 0 )
	    emit highlighted( mi->id() );
    }
    QPopupMenu *popup = mi->popup();
    if ( popup ) {
	popup->actItem = -1;
	if ( popup->isVisible() ) {		// sub menu already open
	    popup->hidePopups();
	    popup->repaint( FALSE );
	}
	else {					// open sub menu
	    hidePopups();
	    openActPopup();
	}
    }
    else
	hidePopups();
}

/*!
  Handles mouse release events for the menu bar.
  */

void QMenuBar::mouseReleaseEvent( QMouseEvent *e )
{
    mouseBtDn = FALSE;				// mouse button up
    int item = itemAtPos( e->pos() );
    if ( actItem == -1 && item != -1 )		// ignore mouse release
	return;
    actItem = item;
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected menu item!
	QMenuItem  *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	if ( popup ) {
	    if ( style() == MacStyle )
		popup->hide();
	    else
		popup->setFirstItemActive();
	}
	else {					// not a popup
	    actItem = -1;
	    repaint( FALSE );
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    else				// normal connection
		emit activated( mi->id() );
	}
    }
}

/*!  Handles mouse move events for the menu bar.  The selected item
  moves along with the mouse automatically.  \sa keyPressEvent() */

void QMenuBar::mouseMoveEvent( QMouseEvent *e )
{
    int item = itemAtPos( e->pos() );
    if ( item == -1 )
	return;
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	hidePopups();
	if ( mi->id() != -1 )
	    emit highlighted( mi->id() );
	if ( mi->popup() )
	    openActPopup();
    }
}

/*!  Handles key press events for the menu bar.  Left and Up move to
  the left, Right/Down move to the right, Escape closes the menu bar,
  and Return and Enter pass control to the selected item.

  \sa QPopupMenu mouseEvent() */

void QMenuBar::keyPressEvent( QKeyEvent *e )
{
    if ( actItem < 0 || mouseBtDn )		// cannot handle key event
	return;

    QMenuItem  *mi = 0;
    QPopupMenu *popup;
    int d = 0;

    switch ( e->key() ) {
	case Key_Left:
	case Key_Up:
	    d = -1;
	    break;

	case Key_Right:
	case Key_Down:
	    d = 1;
	    break;

	case Key_Escape:
	    actItem = -1;
	    repaint( FALSE );
	    break;

	case Key_Return:
	case Key_Enter:
	    mi = mitems->at( actItem );
//	    popup = mi->popup();
	    // ... what to do
	    break;
    }

    if ( d ) {					// highlight next/prev
	register int i = actItem;
	int c = mitems->count();
	int n = c;
	while ( n-- ) {
	    i = i + d;
	    if ( i == c )
		i = 0;
	    else if ( i < 0 )
		i = c - 1;
	    mi = mitems->at( i );
	    if ( !(mi->isSeparator() || mi->isDisabled()) )
		break;
	}
	if ( i != actItem ) {
	    actItem = i;
	    repaint( FALSE );
	    hidePopups();
	    popup = mi->popup();
	    if ( popup ) {
		popup->setFirstItemActive();
		openActPopup();
	    }
	    else {
		if ( mi->id() >= 0 )
		    emit highlighted( mi->id() );
	    }
	}
    }
}

/*!
  Handles resize events for the menu bar.
  */

void QMenuBar::resizeEvent( QResizeEvent * )
{
    badSize = TRUE;
}
