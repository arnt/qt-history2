/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.cpp#81 $
**
** Implementation of QMenuBar class
**
** Created : 941209
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 INCLUDE_MENUITEM_DEF
#include "qmenubar.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qapp.h"
#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qmenubar.cpp#81 $");


/*!
  \class QMenuBar qmenubar.h
  \brief The QMenuBar class provides a horizontal menu bar.

  \ingroup realwidgets

  It automatically sets its own geometry to the top of the parent
  widget and changes appropriately it when the parent widget is
  resized.

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  <img src=qmenubar-m.gif> <img src=qmenubar-w.gif>

  \sa QPopupMenu
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
    initMetaObject();
    isMenuBar = TRUE;
    autoaccel = 0;
    irects    = 0;
    mseparator = 0;
    windowsaltactive = 0;
    if ( parent )				// filter parent events
	topLevelWidget()->installEventFilter( this );

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
}

/*!
  Destroys the menu bar.
*/

QMenuBar::~QMenuBar()
{
    delete autoaccel;
    delete [] irects;
}

/*!
  \internal
  Needs documentation.
*/
void QMenuBar::updateItem( int )
{
 //   repaint( FALSE ); !!!hanord: avoid this until we get a better solution
}


/*!
  Recomputes the menu bar's display data according to the new
  contents.

  You should never need to call this, it is called automatically by
  QMenuData whenever it needs to be called.
*/

void QMenuBar::menuContentsChanged()
{
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
 //   repaint(); !!!hanord: avoid this until we get a better solution
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
    if ( object == parent() && event->type() == Event_Resize ) {
	QResizeEvent *e = (QResizeEvent *)event;
	setGeometry( 0, y(), e->size().width(), height() );
	calculateRects();
	return FALSE;
    }

    if ( style() != WindowsStyle || 
	 !( event->type() == Event_Accel || 
	    event->type() == Event_KeyRelease ) )
	return FALSE;

    QWidget * tlw = topLevelWidget();
    QWidget * f = tlw->focusWidget();

    // look for Alt press and Alt-anything press
    if ( object == tlw && event->type() == Event_Accel ) {
	QKeyEvent * ke = (QKeyEvent *) event;
	if ( f ) {
	    if ( ke->key() == Key_Alt ) {
		if ( windowsaltactive || actItem >= 0 ) {
		    windowsaltactive = 0;
		    actItem = -1;
		    if ( focusWidget() )
			focusWidget()->setFocus();
		} else {
		    windowsaltactive = 1;
		    if ( f != tlw )
			f->installEventFilter( this );
		}
	    }
	}
    }

    // look for Alt release
    if ( object == f ) {
	if ( windowsaltactive && event->type() == Event_KeyRelease &&
	     ((QKeyEvent *)event)->key() == Key_Alt ) {
	    actItem = 0;
	    setFocus();
	} else if ( event->type() == Event_KeyPress && f != tlw ) {
	    f->removeEventFilter( this );
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
	actItem = indexOf( id );
	repaint( FALSE );
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
	    if ( focusWidget() )
		focusWidget()->setFocus();
	    windowsaltactive = 0;
	    actItem = -1;
	    repaint( FALSE );
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


void QMenuBar::goodbye()
{
    if ( focusWidget() )
	focusWidget()->setFocus();
    windowsaltactive = 0;
    actItem = -1;
    mouseBtDn = FALSE;
    repaint( FALSE );
}


void QMenuBar::openActPopup()
{
    if ( actItem < 0 )
	return;
    QPopupMenu *popup = mitems->at(actItem)->popup();
    if ( !popup )
	return;

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
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    QWidget *w = parentWidget();
    while ( (mi=it.current()) ) {
	++it;
	QString s = mi->text();
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
	    if ( !mi->popup()->isEnabled() )
		mi->popup()->enableAccel( FALSE );
	}
    }
    if ( w )
	resize( w->width(), height() );
    calculateRects();
    QWidget::show();
    raise();
}

/*!
  Reimplements QWidget::hide() in order to deselect any selected item.
*/

void QMenuBar::hide()
{
    if ( focusWidget() )
	focusWidget()->setFocus();
    windowsaltactive = 0;
    actItem = -1;
    hidePopups();
    QWidget::hide();
}


/*!
  \internal
  Needs to change the size of the menu bar when a new font is set.
*/

void QMenuBar::fontChange( const QFont & )
{
    badSize = TRUE;
    repaint();
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
	if ( !badSize )				// size was not changed
	    return 0;
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
    if ( gs == WindowsStyle )	// !!!hanord
	x = y = 2;
    while ( i < (int)mitems->count() ) {	// for each menu item...
	QMenuItem *mi = mitems->at(i);
	int w=0, h=0;
	if ( mi->pixmap() ) {			// pixmap item
	    w = mi->pixmap()->width();
	    h = mi->pixmap()->height();
	} else if ( mi->text() ) {		// text item
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
	    if ( x + w + motifBarFrame > max_width && nlitems > 0 ) {
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
	    while( --i > separator ) {
		irects[i].moveBy( moveBy, 0 );
	    }
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


/*# QT_VERSION 200
  Move the flag for separator usage down from QMenuData to QMenuBar.
*/

/*!
  When a menubar is used above an unframed widget, it may look better
  with a separating line when displayed with \link QWidget::style()
  WindowsStyle\endlink.

  This function sets the usage of such a separator to appear either
  QMenuBar::Never, or QMenuBar::InWindowsStyle.

  The default is QMenuBar::Never.
*/
void QMenuBar::setSeparator( Separator when )
{
    mseparator = when;
}

/*!
  Returns the currently set \link setSeparator() separator usage\endlink.
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
    int fw = frameWidth();
    GUIStyle gs = style();
    bool e;

    p->setClipRect( fw, fw, width() - 2*fw, height() - 2*fw );

    for ( int i=0; i<(int)mitems->count(); i++ ) {
	QMenuItem *mi = mitems->at( i );
	if ( mi->text() || mi->pixmap() ) {
	    QRect r = irects[i];
	    e = mi->isEnabled();
	    if ( e )
		g = palette().normal();
	    else
		g = palette().disabled();
		     
	    if ( gs == WindowsStyle && i == actItem ) {
		if ( e ) {
		    g = QColorGroup ( g.foreground(), g.background(),
				      g.light(), g.dark(), g.mid(),
				      white, g.base() );
		} else {
		    g = QColorGroup ( g.foreground(), g.background(),
				      g.light(), g.dark(), g.mid(),
				      palette().disabled().text(), g.base() );
		    e = TRUE;
		}
	    }

	    if ( gs == WindowsStyle && i == actItem )
		p->fillRect( r, darkBlue );
	    else if ( gs == WindowsStyle )
		p->fillRect( r, palette().normal().background() );
	    else if ( i == actItem ) // motif, active item
		qDrawShadePanel( p, r, palette().normal(), FALSE,
				 motifItemFrame );
	    else // motif, other item
		qDrawPlainRect( p, r, palette().normal().background(),
				motifItemFrame );

	    qDrawItem( p, gs, r.left(), r.top(), r.width(), r.height(),
		       AlignCenter|ShowPrefix|DontClip|SingleLine,
		       g, e, mi->pixmap(), mi->text() );

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
    if ( e->button() != LeftButton &&  e->button() != RightButton )
	return;
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( focusWidget() )
	    focusWidget()->setFocus();
	windowsaltactive = 0;
	actItem = -1;
	repaint( FALSE );
	return;
    }
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item highlighted
	actItem = item;
	repaint( FALSE );
	emit highlighted( mi->id() );
    }

    QPopupMenu *popup = mi->popup();
    if ( popup && mi->isEnabled() ) {
	if ( popup->isVisible() ) {	// sub menu already open
	    popup->hidePopups();
	    popup->repaint( FALSE );
	} else {				// open sub menu
	    hidePopups();
	    openActPopup();
	}
    } else {
	hidePopups();
    }
}


/*!
  Handles mouse release events for the menu bar.
*/

void QMenuBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton &&  e->button() != RightButton )
	return;
    mouseBtDn = FALSE;				// mouse button up
    int item = itemAtPos( e->pos() );
    if ( item >= 0 && !mitems->at(item)->isEnabled() ||
	 actItem >= 0 && !mitems->at(actItem)->isEnabled() ) {
	if ( focusWidget() )
	    focusWidget()->setFocus();
	windowsaltactive = 0;
	actItem = -1;
	hidePopups();
	repaint();
	return;
    }
    if ( actItem == -1 || item != actItem )	// ignore mouse release
	return;
    actItem = item;
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected a menu item
	QMenuItem  *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	if ( popup ) {
	    if (!hasMouseTracking() )
		popup->setFirstItemActive();
	} else {				// not a popup
	    if ( focusWidget() )
		focusWidget()->setFocus();
	    windowsaltactive = 0;
	    actItem = -1;
	    repaint( FALSE );
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
    if ( actItem < 0 && !mouseBtDn )
	return;
    //    if ( !(mouseBtDn || (actItem >= 0 && hasMouseTracking())) )
    //-	return;

    int item = itemAtPos( e->pos() );
    if ( item == -1 )
	return;
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	hidePopups();
	emit highlighted( mi->id() );
	if ( mi->popup() && mi->isEnabled() )
	    openActPopup();
    }
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
    int d = 0;

    switch ( e->key() ) {
    case Key_Left:
	d = -1;
	break;

    case Key_Right:
	d = 1;
	break;

    case Key_Up:
    case Key_Down:
	if ( style() == WindowsStyle ) {
	    mi = mitems->at( actItem );
	    popup = mi->popup();
	    if ( popup && mi->isEnabled() ) {
		hidePopups();
		popup->setFirstItemActive();
		openActPopup();
		windowsaltactive = 0;
	    }
	}
	break;

    case Key_Escape:
	if ( focusWidget() )
	    focusWidget()->setFocus();
	windowsaltactive = 0;
	actItem = -1;
	repaint( FALSE );
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
	    // ### fix windows-style traversal - currently broken due to 
	    // QMenuBar's reliance on QPopupMenu
	    if ( /* (style() == WindowsStyle || */ mi->isEnabled() /* ) */
		 && !mi->isSeparator() )
		break;
	}
	if ( i != actItem ) {
	    actItem = i;
	    repaint( FALSE );
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
    }
}


/*!
  Handles resize events for the menu bar.
*/

void QMenuBar::resizeEvent( QResizeEvent *e )
{
    QSize oldSize = e->oldSize();
    QSize newSize = e->size();
    if(oldSize == newSize) return;
  
    badSize = TRUE;
    calculateRects();
}
