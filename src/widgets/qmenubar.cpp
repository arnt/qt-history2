/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.cpp#158 $
**
** Implementation of QMenuBar class
**
** Created : 941209
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

#define	 INCLUDE_MENUITEM_DEF
#include "qmenubar.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include <ctype.h>

/*!
  \class QMenuBar qmenubar.h
  \brief The QMenuBar class provides a horizontal menu bar.

  \ingroup realwidgets
  \ingroup application

  It automatically sets its own geometry to the top of the parent
  widget and changes appropriately it when the parent widget is
  resized.

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  <img src=qmenubar-m.png> <img src=qmenubar-w.png>

  \sa QPopupMenu
  <a href="guibooks.html#fowler">GUI Design Handbook: Menu Bar</a>
*/


/*! \enum QMenuBar::Separator

  This enum type is used to decide whether QMenuBar should draw a
  separator line at its bottom.  The possible values are: <ul>

  <li> \c Never - in many applications, there already is a separator,
  and two looks stupid.

  <li> \c InWindowsStyle - in some other applications, a separator
  looks good in Windows style but not else.

  </ul>
 */

/*!
  \fn void QMenuBar::activated( int id )

  This signal is emitted when a menu item is selected; \a id is the id
  of the selected item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa highlighted(), QMenuData::insertItem()
*/

/*!
  \fn void QMenuBar::highlighted( int id )

  This signal is emitted when a menu item is highlighted; \a id is the
  id of the highlighted item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa activated(), QMenuData::insertItem()
*/


// Motif style parameters

static const int motifBarFrame		= 2;	// menu bar frame width
static const int motifBarHMargin	= 2;	// menu bar hor margin to item
static const int motifBarVMargin	= 1;	// menu bar ver margin to item
static const int motifItemFrame		= 2;	// menu item frame width
static const int motifItemHMargin	= 5;	// menu item hor text margin
static const int motifItemVMargin	= 4;	// menu item ver text margin

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
    isMenuBar = TRUE;
    autoaccel = 0;
    irects    = 0;
    rightSide = 0; // Right of hear is rigth-aligned content
    mseparator = 0;
    windowsaltactive = 0;
    if ( parent ) {
	// filter parent events for resizing
	parent->installEventFilter( this );

	// filter top-level-widget events for accelerators
	QWidget *tlw = topLevelWidget();
	if ( tlw != parent )
	    tlw->installEventFilter( this );
    }

    QFontMetrics fm = fontMetrics();
    int gs = style();
    int h;
    if ( gs == WindowsStyle ) {
	h = 2 + fm.height() + motifItemVMargin + 2*motifBarFrame;
    } else {
	h =  motifBarFrame + motifBarVMargin + fm.height()
	    + motifItemVMargin + 2*motifBarFrame + 2*motifItemFrame;
    }

    move( 0, 0 );
    resize( width(), h );

    switch ( gs ) {
	case WindowsStyle:
	    setFrameStyle( QFrame::NoFrame );
	    setMouseTracking( TRUE );
	    break;
	case MotifStyle:
	    setFrameStyle( QFrame::Panel | QFrame::Raised );
	    setLineWidth( motifBarFrame );
	    break;
	default:
	    break;
    }
    setBackgroundMode( PaletteButton );
}



/*! \reimp */

void QMenuBar::styleChange( QStyle& old )
{
    switch ( style().guiStyle() ) {
	case WindowsStyle:
	    setFrameStyle( QFrame::NoFrame );
	    setMouseTracking( TRUE );
	    break;
	case MotifStyle:
	    setFrameStyle( QFrame::Panel | QFrame::Raised );
	    setLineWidth( motifBarFrame );
	    break;
	default:
	    break;
    }
    updateGeometry();
    QFrame::styleChange( old );
}



/*!
  Destroys the menu bar.
*/

QMenuBar::~QMenuBar()
{
    delete autoaccel;
    if ( irects )		// Avoid purify complaint.
	delete [] irects;
}

/*!
  \internal
  Needs documentation.
*/
void QMenuBar::updateItem( int )
{
 //   repaint( FALSE ); //###H: avoid this until we get a better solution
}


/*!
  Recomputes the menu bar's display data according to the new
  contents.

  You should never need to call this, it is called automatically by
  QMenuData whenever it needs to be called.
*/

void QMenuBar::menuContentsChanged()
{
    setupAccelerators();
    badSize = TRUE;				// might change the size
    if ( isVisible() ) {
	calculateRects();
	repaint();
    }
}

/*!
  Recomputes the menu bar's display data according to the new
  state.

  You should never need to call this, it is called automatically by
  QMenuData whenever it needs to be called.
*/

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
    popup->parentMenu = 0;
    popup->disconnect( SIGNAL(activatedRedirect(int)), this,
		       SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(highlightedRedirect(int)), this,
		       SLOT(subHighlighted(int)) );
}

void QMenuBar::frameChanged()
{
    menuContentsChanged();
}


/*!
  This function is used to adjust the menu bar's geometry to the
  parent widget's.  Note that this is \e not part of the public
  interface - the function is \c public only because
  QObject::eventFilter() is.

  \internal
  Resizes the menu bar to fit in the parent widget when the parent receives
  a resize event.
*/

bool QMenuBar::eventFilter( QObject *object, QEvent *event )
{
    if ( object == parent() && event->type() == QEvent::Resize ) {
	QResizeEvent *e = (QResizeEvent *)event;
	int w = e->size().width();
	setGeometry( 0, y(), w, heightForWidth(w) );
	return FALSE;
    }

    if ( style() != WindowsStyle ||
	 !isVisibleToTLW() ||
	 !object->isWidgetType() ||
	 !( event->type() == QEvent::Accel ||
	    event->type() == QEvent::KeyPress ||
	    event->type() == QEvent::KeyRelease ) )
	return FALSE;

    // look for Alt press and Alt-anything press
    if ( event->type() == QEvent::Accel ) {
	QWidget * f = ((QWidget *)object)->focusWidget();
	QKeyEvent * ke = (QKeyEvent *) event;
	if ( f ) { // ### this thinks alt and meta are the same
	    if ( ke->key() == Key_Alt || ke->key() == Key_Meta ) {
		if ( windowsaltactive || actItem >= 0 ) {
		    setWindowsAltMode( FALSE, -1 );
 		    ke->accept();
 		    return TRUE;
		} else {
		    windowsaltactive = 1;
		    if ( f != object )
			f->installEventFilter( this );
		}
	    } else if ( (ke->key() == Key_Control || ke->key() == Key_Shift) &&
			(windowsaltactive || actItem >= 0) ) {
		setWindowsAltMode( FALSE, -1 );
	    }
	}
	// ### ! block all accelerator events when the menu bar is active
	if ( qApp && qApp->focusWidget() == this ) {
	    return TRUE;
	}
    }

    // look for Alt release
    if ( ((QWidget*)object)->focusWidget() == object ||
	 (object->parent() == 0 && ((QWidget*)object)->focusWidget() == 0) ) {
	if ( windowsaltactive &&
	     event->type() == QEvent::KeyRelease &&
	     (((QKeyEvent *)event)->key() == Key_Alt ||
	      ((QKeyEvent *)event)->key() == Key_Meta) ) {
	    windowsaltactive = FALSE; // trigger update
	    setWindowsAltMode( TRUE, 0 );
	    if ( object->parent() )
		object->removeEventFilter( this );
	    QWidget * tlw = ((QWidget *)object)->topLevelWidget();
	    if ( tlw ) {
		// ### !
		// make sure to be the first event filter, so we can kill
		// accelerator events before the accelerators get to them.
		tlw->removeEventFilter( this );
		tlw->installEventFilter( this );
	    }
	    return TRUE;
	} else if ( (event->type() == QEvent::KeyPress ||
		     event->type() == QEvent::KeyRelease) &&
		    !(((QKeyEvent *)event)->key() == Key_Alt ||
		      ((QKeyEvent *)event)->key() == Key_Meta) ) {
	    if ( object->parent() )
		object->removeEventFilter( this );
	    setWindowsAltMode( FALSE, -1 );
	}
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
    if ( !isEnabled() )				// the menu bar is disabled
	return;
    QMenuItem *mi = findItem( id );
    if ( mi && mi->isEnabled() ) {
	setActItem( indexOf( id ), FALSE );
	QPopupMenu *popup = mi->popup();
	if ( popup ) {
	    emit highlighted( mi->id() );
	    if ( popup->isVisible() ) {		// sub menu already open
		popup->hidePopups();
		popup->repaint( FALSE );
	    } else {				// open sub menu
		hidePopups();
		openActPopup();
		popup->setFirstItemActive();
	    }
	} else {				// not a popup
	    setWindowsAltMode( FALSE, -1 );
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    emit activated( mi->id() );
	}
    }
}


/*!
  \internal
  This slot receives signals from menu accelerator when it is about to be
  destroyed.
*/

void QMenuBar::accelDestroyed()
{
    autoaccel = 0;				// don't delete it twice!
}


bool QMenuBar::tryMouseEvent( QPopupMenu *popup, QMouseEvent *e )
{
    QPoint pos = mapFromGlobal( popup->mapToGlobal( e->pos() ) );
    if ( !rect().contains( pos ) )		// outside
	return FALSE;
    int item = itemAtPos( pos );
    if ( item == -1 && (e->type() == QEvent::MouseButtonPress ||
			e->type() == QEvent::MouseButtonRelease) ) {
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


void QMenuBar::goodbye()
{
    setWindowsAltMode( FALSE, -1 );
    mouseBtDn = FALSE;
}


void QMenuBar::openActPopup()
{
    if ( actItem < 0 )
	return;
    setWindowsAltMode( FALSE, actItem );
    QPopupMenu *popup = mitems->at(actItem)->popup();
    if ( !popup || !popup->isEnabled() )
	return;

    QRect  r = itemRect( actItem );
    QPoint pos = r.bottomLeft() + QPoint(0,1);
    int ph = popup->sizeHint().height();
    pos = mapToGlobal(pos);
    int sh = QApplication::desktop()->height();
    if ( pos.y() + ph > sh ) {
	pos = mapToGlobal( r.topLeft() );
	pos.ry() -= (QCOORD)ph;
    }

    //avoid circularity
    if ( popup->isVisible() )
	return;

    if (popup->parentMenu != this ){
	// reuse
	if (popup->parentMenu)
	    popup->parentMenu->menuDelPopup(popup);
	popup->selfItem  = mitems->at(actItem);
	menuInsPopup(popup);
    }

    popup->popup( pos );
}

/*!
  \internal
  Hides all popup menu items.
*/

void QMenuBar::hidePopups()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() )
	    mi->popup()->hide();
    }
}


/*!
  Reimplements QWidget::show() in order to set up the correct keyboard
  accelerators and raise itself to the top of the widget stack.
*/

void QMenuBar::show()
{
    setupAccelerators();
    if ( parentWidget() )
	resize( parentWidget()->width(), height() );
    calculateRects();
    QWidget::show();
    raise();
}

/*!
  Reimplements QWidget::hide() in order to deselect any selected item.
*/

void QMenuBar::hide()
{
    QWidget::hide();
    setWindowsAltMode( FALSE, -1 );
    hidePopups();
}


/*!
  \internal
  Needs to change the size of the menu bar when a new font is set.
*/

void QMenuBar::fontChange( const QFont & f )
{
    badSize = TRUE;
    updateGeometry();
    if ( isVisible() )
	calculateRects();
    QWidget::fontChange( f );
}


/*****************************************************************************
  Item geometry functions
 *****************************************************************************/

/*!
  This function serves two different purposes.  If the parameter is negative,
  it updates the irects member for the current width and resizes.  Otherwise,
  it does the same calculations for the GIVEN width, and returns the height
  to which it WOULD have resized.  A bit tricky, but both operations require
  almost identical steps.
*/
int QMenuBar::calculateRects( int max_width )
{
    bool update = ( max_width < 0 );

    if ( update ) {
	rightSide = 0;
	if ( !badSize )				// size was not changed
	    return 0;
	if ( irects )		// Avoid purify complaint.
	    delete [] irects;
	if ( mitems->isEmpty() ) {
	    irects = 0;
	    return 0;
	}
	irects = new QRect[ mitems->count() ];	// create rectangle array
	CHECK_PTR( irects );
	max_width = width();
    }
    QFontMetrics fm = fontMetrics();
    int max_height = 0;
    int nlitems = 0;				// number on items on cur line
    int gs = style();
    int x = motifBarFrame + motifBarHMargin;
    int y = motifBarFrame + motifBarVMargin;
    int i = 0;
    int separator = -1;
    if ( gs == WindowsStyle )	//###H
	x = y = 2;
    while ( i < (int)mitems->count() ) {	// for each menu item...
	QMenuItem *mi = mitems->at(i);
	int w=0, h=0;
	if ( mi->pixmap() ) {			// pixmap item
	    w = mi->pixmap()->width();
	    h = mi->pixmap()->height();
	} else if ( !mi->text().isNull() ) {	// text item
	    w = fm.width(mi->text()) + 2*motifItemHMargin;
	    h = fm.height() + motifItemVMargin;
	} else if ( mi->isSeparator() ) {	// separator item
	    separator = i;
	}
	if ( !mi->isSeparator() ) {
	    if ( gs == MotifStyle && !mi->isSeparator() ) {
		w += 2*motifItemFrame;
		h += 2*motifItemFrame;
	    }
	    if ( x + w + motifBarFrame - max_width > 0 && nlitems > 0 ) {
		nlitems = 0;
		x = motifBarFrame + motifBarHMargin;
		y += h + motifBarHMargin;
		separator = -1;
	    }
	    if ( y + h + 2*motifBarFrame > max_height )
		max_height = y + h + 2*motifBarFrame;
	}
	if ( update )
	    irects[i].setRect( x, y, w, h );
	x += w;
	nlitems++;
	i++;
    }
    if ( update ) {
	if ( separator >= 0 && style() == MotifStyle ) {
	    int moveBy = max_width - x;
	    rightSide = x;
	    while( --i > separator ) {
		irects[i].moveBy( moveBy, 0 );
	    }
	} else {
	    rightSide = width()-frameWidth();
	}
	if ( max_height != height() )
	    resize( max_width, max_height );
	badSize = FALSE;
    }
    return max_height;
}

/*!
  Returns the height that the menu would resize itself to if its parent
  (and hence itself) resized to the given width.  This can be useful for
  simple layout tasks where the height of the menubar is needed after
  items have been inserted.  See examples/showimg/showimg.cpp for an
  example of the usage.
*/
int QMenuBar::heightForWidth(int max_width) const
{
    // Okay to cast away const, as we are not updating.
    if ( max_width < 0 ) max_width = 0;
    return ((QMenuBar*)this)->calculateRects( max_width );
}

/*!
  \internal
  Return the bounding rectangle for the menu item at position \e index.
*/

QRect QMenuBar::itemRect( int index )
{
    calculateRects();
    return irects ? irects[index] : QRect(0,0,0,0);
}

/*!
  \internal
  Return the item at \e pos, or -1 if there is no item there, or if
  it is a separator item.
*/

int QMenuBar::itemAtPos( const QPoint &pos )
{
    calculateRects();
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


/*!
  When a menubar is used above an unframed widget, it may look better
  with a separating line when displayed with \link QWidget::style()
  WindowsStyle\endlink.

  This function sets the usage of such a separator to appear either
  QMenuBar::Never, or QMenuBar::InWindowsStyle.

  The default is QMenuBar::Never.

  \sa Separator separator()
*/
void QMenuBar::setSeparator( Separator when )
{
    mseparator = when;
}

/*!
  Returns the currently set Separator usage.

  \sa Separator setSeparator()
*/
QMenuBar::Separator QMenuBar::separator() const
{
    return mseparator ? InWindowsStyle : Never;
}

/*****************************************************************************
  Event handlers
 *****************************************************************************/


/*!
  Called from QFrame::paintEvent().
*/

void QMenuBar::drawContents( QPainter *p )
{
    QColorGroup g;
    GUIStyle gs = style().guiStyle();
    bool e;

    for ( int i=0; i<(int)mitems->count(); i++ ) {
	QMenuItem *mi = mitems->at( i );
	if ( !mi->text().isNull() || mi->pixmap() ) {
	    QRect r = irects[i];
	    e = mi->isEnabled();
	    if ( e )
		g = palette().normal();
	    else
		g = palette().disabled();
		
	    if ( gs == WindowsStyle || style().defaultFrameWidth() < 2) {
		p->fillRect( r,palette().normal().brush( QColorGroup::Button ) );
		if ( i == actItem ) {
		    QBrush b = palette().normal().brush( QColorGroup::Button );
		    bool sunken = !windowsaltactive ||
				  (mi->popup() && mi->popup()->isVisible());
		    if (sunken)
			p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
		    qDrawShadeRect( p,
				    r.left(), r.top(), r.width(), r.height(),
				    g, sunken, 1, 0, &b );
		    if ( sunken ) {
			r.setRect( r.left()+2, r.top()+2,
				   r.width()-2, r.height()-2 );
			p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
		    }
		}
	    } else { // motif
		if ( i == actItem ) // active item
		    qDrawShadePanel( p, r, palette().normal(), FALSE,
				     motifItemFrame,
			    &palette().normal().brush( QColorGroup::Button ));
		else // other item
		    p->fillRect(r,
			    palette().normal().brush( QColorGroup::Button ));
// 		    qDrawPlainRect( p, r, palette().normal().button(),
// 				    motifItemFrame, &palette().normal().fillButton() );
	    }
	    QColor btext = g.buttonText();
	    style().drawItem( p, r.left(), r.top(), r.width(), r.height(),
			      AlignCenter|ShowPrefix|DontClip|SingleLine,
			      g, e, mi->pixmap(), mi->text(), -1, &btext );

	}
    }
    if ( mseparator == InWindowsStyle && gs == WindowsStyle ) {
	p->setPen( g.light() );
	p->drawLine( 0, height()-1, width()-1, height()-1 );
	p->setPen( g.dark() );
	p->drawLine( 0, height()-2, width()-1, height()-2 );
    }
}


/*!
  Handles mouse press events for the menu bar.
*/

void QMenuBar::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton &&
	 e->button() != RightButton )
	return;
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	setWindowsAltMode( FALSE, -1 );
	return;
    }

    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem )			// new item highlighted
	emit highlighted( mi->id() );

    QPopupMenu *popup = mi->popup();
    if ( popup && mi->isEnabled() ) {
	setWindowsAltMode( FALSE, item );
	if ( popup->isVisible() ) {	// sub menu already open
	    popup->hidePopups();
	    popup->repaint( FALSE );
	} else {				// open sub menu
	    hidePopups();
	    openActPopup();
	}
    } else {
	setWindowsAltMode( FALSE, item );
	hidePopups();
    }
}


/*!
  Handles mouse release events for the menu bar.
*/

void QMenuBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton &&
	 e->button() != RightButton )
	return;
    mouseBtDn = FALSE;				// mouse button up
    int item = itemAtPos( e->pos() );
    if ( item >= 0 && !mitems->at(item)->isEnabled() ||
	 actItem >= 0 && !mitems->at(actItem)->isEnabled() ) {
	setWindowsAltMode( FALSE, -1 );
	hidePopups();
	return;
    }
    if ( actItem == -1 || item != actItem )	// ignore mouse release
	return;
    setActItem( item, FALSE );
    if ( actItem >= 0 ) {			// selected a menu item
	QMenuItem  *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	if ( popup ) {
	    if (!hasMouseTracking() )
		popup->setFirstItemActive();
	} else {				// not a popup
	    setWindowsAltMode( FALSE, -1 );
	    if ( mi->signal() )			// activate signal
		mi->signal()->activate();
	    emit activated( mi->id() );
	}
    }
}


/*!
  Handles mouse move events for the menu bar.
*/

void QMenuBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( style() == WindowsStyle && !mouseBtDn ) {
	int item = itemAtPos( e->pos() );
	QMenuItem *mi = actItem < 0 ? 0 : mitems->at(actItem);
	if ( item != actItem &&
	     (!mi || !mi->popup() || !mi->popup()->isVisible()) ) {
	    if ( item >= 0 ) {
		setWindowsAltMode( TRUE, item );
		mi = mitems->at( item );
		emit highlighted( mi->id() );
	    } else if (actItem >= 0 && irects ) {
		QRect r( irects[actItem].left(), irects[actItem].top()-4,
			 irects[actItem].width(),irects[actItem].height()+8 );
		if ( !r.contains(e->pos()) )
		    setWindowsAltMode( FALSE, -1 );
	    } else {
		setWindowsAltMode( FALSE, -1 );
	    }
	    return;
	}
    }

    if ( actItem < 0 && !mouseBtDn )
	return;

    int item = itemAtPos( e->pos() );
    if ( item == -1 )
	return;
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	setActItem( item, FALSE );
	hidePopups();
	emit highlighted( mi->id() );
	if ( mi->popup() && mi->isEnabled() )
	    openActPopup();
    }
}


/*!  Handles leave events for the menu bar.
*/

void QMenuBar::leaveEvent( QEvent * e )
{
    if ( windowsaltactive && actItem >= 0 ) {
	QMenuItem *mi = mitems->at( actItem );
	if ( mi && (!mi->popup() || !mi->popup()->isVisible()) )
	    setWindowsAltMode( FALSE, -1 );
    }
    QFrame::leaveEvent( e );
}


/*!
  Handles key press events for the menu bar.
*/

void QMenuBar::keyPressEvent( QKeyEvent *e )
{
    if ( actItem < 0 )
	return;

    QMenuItem  *mi = 0;
    QPopupMenu *popup;
    int dx = 0;

    switch ( e->key() ) {
    case Key_Left:
	dx = -1;
	break;

    case Key_Right:
	dx = 1;
	break;

    case Key_Up:
    case Key_Down:
	if ( style() == WindowsStyle ) {
	    mi = mitems->at( actItem );
	    popup = mi->popup();
	    if ( popup && mi->isEnabled() ) {
		windowsaltactive = 0;
		hidePopups();
		popup->setFirstItemActive();
		openActPopup();
		repaint( FALSE );
	    }
	}
	break;

    case Key_Escape:
	setWindowsAltMode( FALSE, -1 );
	break;
    }

    if ( dx ) {					// highlight next/prev
	register int i = actItem;
	int c = mitems->count();
	int n = c;
	while ( n-- ) {
	    i = i + dx;
	    if ( i == c )
		i = 0;
	    else if ( i < 0 )
		i = c - 1;
	    mi = mitems->at( i );
	    // ### fix windows-style traversal - currently broken due to
	    // QMenuBar's reliance on QPopupMenu
	    if ( /* (style() == WindowsStyle || */ mi->isEnabled() /* ) */
		 && !mi->isSeparator() )
		break;
	}
	if ( i != actItem ) {
	    setActItem( i, FALSE );
	    if ( !windowsaltactive ) {
		hidePopups();
		popup = mi->popup();
		if ( popup && mi->isEnabled() ) {
		    popup->setFirstItemActive();
		    openActPopup();
		} else {
		    emit highlighted( mi->id() );
		}
	    }
	}
    } else if ( !e->state() && e->text().length()==1 ) {
	QChar c = e->text()[0].upper();

	QMenuItemListIt it(*mitems);
	register QMenuItem *m;
	int indx = 0;
	while ( (m=it.current()) ) {
	    ++it;
	    QString s = m->text();
	    if ( !s.isEmpty() ) {
		int i = s.find( '&' );
		if ( i >= 0 )
		{
		    if ( s[i+1].upper() == c ) {
			mi = m;
			break;
		    }
		}
	    }
	    indx++;
	}
	if ( mi ) {
	    setWindowsAltMode( FALSE, indx );
	    hidePopups();
	    popup = mi->popup();
	    if ( popup && mi->isEnabled() ) {
		popup->setFirstItemActive();
		openActPopup();
	    } else {
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
    QRect fr = frameRect();
    if ( !fr.isNull() ) {
	QRect r( fr.x(), fr.y(), width(), height() );
	setFrameRect( r );
    }

    if ( badSize )
	return;
    badSize = TRUE;
    if ( isVisible() )
	calculateRects();
}


/*!  Set actItem to \a i and repaint( \a clear ).

  Takes care to optimize the repainting.  Assumes that
  calculateRects() has been called as appropriate.
*/

void QMenuBar::setActItem( int i, bool clear )
{
    if ( i == actItem )
	return;

    if ( i < 0 || actItem < 0 ) {
	// just one item needs repainting
	int n = QMAX( actItem, i );
	actItem = i;
	if ( irects )
	    repaint( irects[n], clear );
    } else if ( QABS(i-actItem) == 1 ) {
	// two neighbouring items need repainting
	int o = actItem;
	actItem = i;
	if ( irects )
	    repaint( irects[i].unite( irects[o] ), clear );
    } else {
	// two non-neighbouring items need repainting
	int o = actItem;
	actItem = i;
	if ( irects ) {
	    repaint( irects[o], clear );
	    repaint( irects[i], clear );
	}
    }
}


/*!  Sets the windows alt-key mode to \a enable, optionally setting
  item \a index to be the active item.

  Windows alt-key mode lets the user use the keys to move along the
  menu bar.  You get into this mode by pressing and releasing alt or
  by moving the mouse onto the menu bar.
*/

void QMenuBar::setWindowsAltMode( bool enable, int index )
{
    if ( enable ) {
	if ( !windowsaltactive ) {
	    windowsaltactive = 1;
	    if ( style() == WindowsStyle ) {
		setUpdatesEnabled( FALSE );
		setFocus();
		setUpdatesEnabled( TRUE );
	    }
	}
	setActItem( index, FALSE );
    } else {
	if ( windowsaltactive ) {
	    if ( style() == WindowsStyle && focusWidget() ) {
		setUpdatesEnabled( FALSE );
		focusWidget()->setFocus();
		setUpdatesEnabled( TRUE );
	    }
	    windowsaltactive = 0;
	}
	setActItem( index, FALSE );
    }
}


/*!  Sets up keyboard accelerators for the menu bar. */

void QMenuBar::setupAccelerators()
{
    delete autoaccel;
    autoaccel = 0;

    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	QString s = mi->text();
	if ( !s.isEmpty() ) {
	    int i = QAccel::shortcutKey( s );
	    if ( i ) {
		if ( !autoaccel ) {
		    autoaccel = new QAccel( this );
		    CHECK_PTR( autoaccel );
		    autoaccel->setIgnoreWhatsThis( TRUE );
		    connect( autoaccel, SIGNAL(activated(int)),
			     SLOT(accelActivated(int)) );
		    connect( autoaccel, SIGNAL(destroyed()),
			     SLOT(accelDestroyed()) );
		}
		autoaccel->insertItem( i, mi->id() );
	    }
	}
	if ( mi->popup() ) {
	    // reuse
	    QPopupMenu* popup = mi->popup();
	    if (popup->parentMenu)
		popup->parentMenu->menuDelPopup(popup);
	    popup->selfItem  = mi;
	    menuInsPopup(popup);
	    popup->updateAccel( this );
	    if ( !popup->isEnabled() )
		popup->enableAccel( FALSE );
	}
    }
}

/*!\reimp
 */
bool QMenuBar::customWhatsThis() const
{
    return TRUE;
}


/*!\reimp
 */
void QMenuBar::focusOutEvent( QFocusEvent * )
{
    if ( windowsaltactive )
	setWindowsAltMode( FALSE, -1 );
}
