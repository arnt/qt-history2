/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopupmenu.cpp#239 $
**
** Implementation of QPopupMenu class
**
** Created : 941128
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
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtimer.h"
#include "qwhatsthis.h"
#include <ctype.h>

// Motif style parameters

static const int motifArrowHMargin	= 6;	// arrow horizontal margin
static const int motifArrowVMargin	= 2;	// arrow vertical margin


/*

+-----------------------------
|      PopupFrame
|   +-------------------------
|   |	   ItemFrame
|   |	+---------------------
|   |	|
|   |	|			   \
|   |	|   ^	T E X T	  ^	    | ItemVMargin
|   |	|   |		  |	   /
|   |	      ItemHMargin
|

*/




// used for internal communication
static QPopupMenu * syncMenu = 0;
static int syncMenuId = 0;

// Used to detect motion prior to mouse-release
static int motion;

// used to provide ONE single-shot timer
static QTimer * singleSingleShot = 0;

static bool supressAboutToShow = FALSE;

static void popupSubMenuLater( int msec, QObject * receiver ) {
    if ( !singleSingleShot )
	singleSingleShot = new QTimer( qApp, "popup submenu timer" );
    singleSingleShot->disconnect( SIGNAL(timeout()) );
    QObject::connect( singleSingleShot, SIGNAL(timeout()),
		      receiver, SLOT(subMenuTimer()) );
    singleSingleShot->start( msec, TRUE );
}




/*!
  \class QPopupMenu qpopupmenu.h
  \brief The QPopupMenu class provides a popup menu widget.

  \ingroup realwidgets

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  \important insertItem clear text pixmap

  <img src=qpopmenu-m.png> <img src=qpopmenu-w.png>

  \sa QMenuBar
  <a href="guibooks.html#fowler">GUI Design Handbook: Menu, Drop-Down and
  Pop-Up</a>
*/


/*! \fn void QPopupMenu::aboutToShow()

  This signal is emitted just before the popup menu is displayed.  You
  can connect it to any slot that sets up the menu contents (e.g. to
  ensure that the right items are enabled).

  \sa setItemEnabled() setItemChecked() insertItem() removeItem()
*/




/*****************************************************************************
  QPopupMenu member functions
 *****************************************************************************/

/*!
  Constructs a popup menu with a parent and a widget name.

  Although a popup menu is always a top level widget, if a parent is
  passed, the popup menu will be deleted on destruction of that parent
  (as with any other QObject).

*/

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
    : QFrame( parent, name, WType_Popup )
{
    isPopupMenu	  = TRUE;
    parentMenu	  = 0;
    selfItem	  = 0;
    autoaccel	  = 0;
    accelDisabled = FALSE;
    popupActive	  = -1;
    tab = 0;
    checkable = 0;
    maxPMWidth = 0;

    tab = 0;
    setFrameStyle( QFrame::PopupPanel | QFrame::Raised );
    style().polishPopupMenu( this );
    setBackgroundMode( PaletteButton );
}

/*!
  Destroys the popup menu.
*/

QPopupMenu::~QPopupMenu()
{
    if ( syncMenu == this ) {
	qApp->exit_loop();
	syncMenu = 0;
    }
	
    delete autoaccel;
    if ( parentMenu )
	parentMenu->removePopup( this );	// remove from parent menu
}


void QPopupMenu::updateItem( int id )		// update popup menu item
{
    updateRow( indexOf(id) );
}


/*!
  Enables or disables display of check marks by the menu items.

  Notice that checking is always enabled when in windows-style.

  \sa isCheckable(), QMenuData::setItemChecked()
*/

void QPopupMenu::setCheckable( bool enable )
{
    if ( isCheckable() != enable ) {
	checkable = enable;
	badSize = TRUE;
    }
}

/*!
  Returns whether display of check marks by the menu items is enabled.

  \sa setCheckable(), QMenuData::setItemChecked()
*/

bool QPopupMenu::isCheckable() const
{
    return checkable;
}


void QPopupMenu::menuContentsChanged()
{
    badSize = TRUE;				// might change the size
    updateAccel( 0 );
    if ( isVisible() ) {
	updateSize();
	update();
    }
}

void QPopupMenu::menuStateChanged()
{
    repaint();
}

void QPopupMenu::menuInsPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;			// set parent menu
    connect( popup, SIGNAL(activatedRedirect(int)),
	     SLOT(subActivated(int)) );
    connect( popup, SIGNAL(highlightedRedirect(int)),
	     SLOT(subHighlighted(int)) );
}

void QPopupMenu::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = 0;
    popup->disconnect( SIGNAL(activatedRedirect(int)), this,
		       SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(highlightedRedirect(int)), this,
		       SLOT(subHighlighted(int)) );
}


void QPopupMenu::frameChanged()
{
    menuContentsChanged();
}


/*!
  Opens the popup menu so that the item number \a indexAtPoint will be
  at the specified \e global position \a pos.  To translate a widget's
  local coordinates into global coordinates, use QWidget::mapToGlobal().

  When positioning a popup with exec() or popup(), keep in mind that
  you cannot rely on the popup menu's current size(). For performance
  reasons, the popup adapts its size only when actually needed. So in
  many cases, the size before and after the show is
  different. Instead, use sizeHint(). It calculates the proper size
  depending on the menu's current contents.

*/

void QPopupMenu::popup( const QPoint &pos, int indexAtPoint )
{
    //avoid circularity
    if ( isVisible() )
	return;

    if (parentMenu && parentMenu->actItem == -1){
	//reuse
	parentMenu->menuDelPopup( this );
	selfItem = 0;
	parentMenu = 0;
    }
    // #### should move to QWidget - anything might need this functionality,
    // #### since anything can have WType_Popup window flag.

    if ( mitems->count() == 0 )			// oops, empty
	insertSeparator();			// Save Our Souls
    if ( badSize )
	updateSize();
    QWidget *desktop = QApplication::desktop();
    int sw = desktop->width();			// screen width
    int sh = desktop->height();			// screen height
    int x  = pos.x();
    int y  = pos.y();
    if ( indexAtPoint > 0 )			// don't subtract when < 0
	y -= itemPos( indexAtPoint );		// (would subtract 2 pixels!)
    int w  = width();
    int h  = height();
    if ( x+w > sw )				// the complete widget must
	x = sw - w;				//   be visible
    if ( y+h > sh )
	y = sh - h;
    if ( x < 0 )
	x = 0;
    if ( y < 0 )
	y = 0;
    move( x, y );
    show();
    motion=0;
}

/*!
  \fn void QPopupMenu::activated( int id )

  This signal is emitted when a menu item is selected; \a id is the id
  of the selected item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa highlighted(), QMenuData::insertItem()
*/

/*!
  \fn void QPopupMenu::highlighted( int id )

  This signal is emitted when a menu item is highlighted; \a id is the
  id of the highlighted item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa activated(), QMenuData::insertItem()
*/

/*! \fn void QPopupMenu::highlightedRedirect( int id )
  \internal
  Used internally to connect submenus to their parents.
*/

/*! \fn void QPopupMenu::activatedRedirect( int id )
  \internal
  Used internally to connect submenus to their parents.
*/

void QPopupMenu::subActivated( int id )
{
    emit activatedRedirect( id );
}

void QPopupMenu::subHighlighted( int id )
{
    emit highlightedRedirect( id );
}

void QPopupMenu::accelActivated( int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi && mi->isEnabled() ) {
	if ( mi->signal() ) // activate signal
	    mi->signal()->activate();
	actSig( mi->id() );
    }
}

void QPopupMenu::accelDestroyed()		// accel about to be deleted
{
    autoaccel = 0;				// don't delete it twice!
}


void QPopupMenu::actSig( int id, bool inwhatsthis )
{
    bool sync = FALSE;
    QPopupMenu * p = this;
    while( p && !sync ) {
	if ( p == syncMenu )
	    sync = TRUE;
	else if ( p->parentMenu && p->parentMenu->isPopupMenu )
	    p = (QPopupMenu*)(p->parentMenu);
	else
	    p = 0;
    }
    if ( sync && qApp ) {
	qApp->exit_loop();
	syncMenu = 0;
    }

    if ( !inwhatsthis )
	emit activated( id );
    else {
	int y = itemPos( indexOf( id ) ) + itemHeight( indexOf( id ) );
	QWhatsThis::leaveWhatsThisMode( findItem( id )->whatsThis(), mapToGlobal( QPoint(0,y) ) );
    }

    emit activatedRedirect( id );
}

void QPopupMenu::hilitSig( int id )
{
    emit highlighted( id );
    emit highlightedRedirect( id );
}


void QPopupMenu::setFirstItemActive()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    int lastActItem = actItem;
    actItem = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( !mi->isSeparator() ) {
	    if ( lastActItem != actItem )
		updateRow( lastActItem );
	    updateRow( actItem );
	    hilitSig( mi->id() );
	    return;
	}
	actItem++;
    }
    actItem = -1;
}

/*!
  \internal
  Hides all popup menus (in this menu tree) that are currently open.
*/

void QPopupMenu::hideAllPopups()
{
    register QMenuData *top = this;		// find top level popup
    while ( top->parentMenu && top->parentMenu->isPopupMenu )
	top = top->parentMenu;
    ((QPopupMenu*)top)->hide();			// cascade from top level
}

/*!
  \internal
  Hides all popup sub-menus.
*/

void QPopupMenu::hidePopups()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() && mi->popup()->parentMenu == this ) //avoid circularity
	    mi->popup()->hide();
    }
    popupActive = -1;				// no active sub menu
}


/*!
  \internal
  Sends the event to the menu bar.
*/

bool QPopupMenu::tryMenuBar( QMouseEvent *e )
{
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    return top->isMenuBar ?
	((QMenuBar *)top)->tryMouseEvent( this, e ) : FALSE;
}

/*!
  \internal
  Tells the menu bar to go back to idle state.
*/

void QPopupMenu::byeMenuBar()
{
    hideAllPopups();
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    if ( top->isMenuBar )
	((QMenuBar *)top)->goodbye();
}


/*!
  \internal
  Return the item at \e pos, or -1 if there is no item there, or if
  it is a separator item.
*/

int QPopupMenu::itemAtPos( const QPoint &pos ) const
{
    if ( !contentsRect().contains(pos) )
	return -1;

    int row = 0;
    int y = contentsRect().y();
    QMenuItem *mi;
    QMenuItemListIt it( *mitems );
    while ( (mi=it.current()) && y + itemHeight( row ) < pos.y() ) {
	++it;
	y += itemHeight( row );
	++row;
    }

    if ( mi && !mi->isSeparator() )
	return row;
    return -1;
}

/*!
  \internal
  Returns the y (top) position of item number \e index.
*/

int QPopupMenu::itemPos( int index )		// get y coord for item
{
    QMenuItem *mi;
    int row = 0;
    int y = contentsRect().y();
    QMenuItemListIt it( *mitems );
    while ( (mi=it.current()) && row < index ) {
	++it;
	y += itemHeight( mi );
	++row;
    }

    if ( row == index )
	return y;
    else
	return 0;
}


/*!
  \internal
  Calculates and sets the size of the popup menu, based on the size
  of the items.
*/

void QPopupMenu::updateSize()
{
    polish();
    int height = 0;
    int max_width = 0;
    QFontMetrics fm = fontMetrics();
    register QMenuItem *mi;
    maxPMWidth = 0;
    tab = 0;
    int arrow_width = style().popupSubmenuIndicatorWidth( fm );

    for ( QMenuItemListIt it( *mitems ); it.current(); ++it ) {
	mi = it.current();
	if ( mi->iconSet() != 0)
	    maxPMWidth = QMAX( maxPMWidth,
			       mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4 );
    }


    for ( QMenuItemListIt it2( *mitems ); it2.current(); ++it2 ) {
	mi = it2.current();
	int w = 0;
	int itemHeight = style().popupMenuItemHeight( checkable, mi, fm );
	
	if ( !mi->text().isNull() && !mi->isSeparator() ) {
	    QString s = mi->text();
	    int t;
	    if ( (t=s.find('\t')) >= 0 ) {	// string contains tab
		w = fm.width( s, t );
		w -= s.contains('&')*fm.width('&');
		int tw = fm.width( s.mid(t+1) );
		if ( tw > tab)
		    tab = tw;
	    } else {
		w += fm.width( s );
		w -= s.contains('&')*fm.width('&');
	    }
	}

	w += style().extraPopupMenuItemWidth( checkable, maxPMWidth, mi, fm );
	
	if ( mi->popup() ) { // submenu indicator belong in the right tab area
	    if ( arrow_width > tab )
		tab = arrow_width;
	}
	

#if defined(CHECK_NULL)
	if ( mi->text().isNull() && !mi->pixmap() && !mi->isSeparator() )
	    qWarning( "QPopupMenu: (%s) Popup has invalid menu item",
		     name( "unnamed" ) );
#endif
	
	height += itemHeight;
	if ( w > max_width )
	    max_width = w;
    }

    if ( tab )
	tab -= fontMetrics().minRightBearing();
    else
	max_width -= fontMetrics().minRightBearing();

    resize( max_width + tab + 2*frameWidth(), height+2*frameWidth() );
    badSize = FALSE;
}



/*!
  \internal
  The \e parent is 0 when it is updated when a menu item has
  changed a state, or it is something else if called from the menu bar.
*/

void QPopupMenu::updateAccel( QWidget *parent )
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    if ( parent == 0 && autoaccel == 0 )
	return;
    if ( autoaccel )				// build it from scratch
	autoaccel->clear();
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->key() ) {
	    if ( !autoaccel ) {
		autoaccel = new QAccel( parent );
		CHECK_PTR( autoaccel );
		connect( autoaccel, SIGNAL(activated(int)),
			 SLOT(accelActivated(int)) );
		connect( autoaccel, SIGNAL(destroyed()),
			 SLOT(accelDestroyed()) );
		if ( accelDisabled )
		    autoaccel->setEnabled( FALSE );
	    }
	    int k = mi->key();
	    int id = autoaccel->insertItem( k, mi->id() );
	    autoaccel->setWhatsThis( id, mi->whatsThis() );
	    if ( !mi->text().isNull() ) {
		QString s = mi->text();
		int i = s.find('\t');
		QString t = QAccel::keyToString( k );
		if ( i >= 0 )
		    s.replace( i+1, s.length()-i, t );
		else {
		    s += '\t';
		    s += t;
		}
		if ( s != mi->text() ) {
		    mi->setText( s );
		    badSize = TRUE;
		}
	    }
	}
	if ( mi->popup() && parent ) {		// call recursively
	    // reuse
	    QPopupMenu* popup = mi->popup();
	    if (!popup->avoid_circularity) {
		popup->avoid_circularity = 1;
		if (popup->parentMenu)
		    popup->parentMenu->menuDelPopup(popup);
		popup->selfItem  = mi;
		menuInsPopup(popup);
		popup->updateAccel( parent );
		popup->avoid_circularity = 0;
	    }
	}
    }
}

/*!
  \internal
  It would be better to check in the slot.
*/

void QPopupMenu::enableAccel( bool enable )
{
    if ( autoaccel )
	autoaccel->setEnabled( enable );
    else
	accelDisabled = TRUE;			// rememeber when updateAccel
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {		// do the same for sub popups
	++it;
	if ( mi->popup() )			// call recursively
	    mi->popup()->enableAccel( enable );
    }
}


/*!
  Reimplements QWidget::setFont() to be able to refresh the popup menu
  when its font changes.
*/

void QPopupMenu::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    if ( isVisible() ) {
	updateSize();
	update();
    }
}

/*!
  Reimplements QWidget::show() for internal purposes.
*/

void QPopupMenu::show()
{
    if ( testWState(WState_Visible) ){
	supressAboutToShow = FALSE;
	return;
    }
    if (!supressAboutToShow)
	emit aboutToShow();
    else
	supressAboutToShow = FALSE;
    if ( badSize )
	updateSize();
    QWidget::show();
    popupActive = -1;
}

/*!
  Reimplements QWidget::hide() for internal purposes.
*/

void QPopupMenu::hide()
{
    if ( !isVisible() )
	return;
    actItem = popupActive = -1;
    mouseBtDn = FALSE;				// mouse button up
    hidePopups();
    killTimers();
    QWidget::hide();
    if ( syncMenu == this && qApp ) {
	qApp->exit_loop();
	syncMenu = 0;
    }
}


/*!
  Calculates the height in pixels of the item in row \a row.
 */
int QPopupMenu::itemHeight( int row ) const
{
    return itemHeight( mitems->at( row ) );
}

/*!
  Calculates the height in pixels of the item \a mi.
 */
int QPopupMenu::itemHeight( QMenuItem *mi ) const
{
    return style().popupMenuItemHeight( checkable, mi, fontMetrics() );
}


void QPopupMenu::drawItem( QPainter* p, int tab_, QMenuItem* mi,
			   bool act, int x, int y, int w, int h)
{

    bool dis = (selfItem && !selfItem->isEnabled()) || !mi->isEnabled();
    style().drawPopupMenuItem(p, checkable, maxPMWidth, tab_, mi, palette(),
			      act, !dis, x, y, w, h);
}


void QPopupMenu::drawContents( QPainter* p )
{
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    int row = 0;
    int y = contentsRect().y();
    while ( (mi=it.current()) ) {
	++it;
	drawItem(p, tab, mi, row == actItem, contentsRect().x(), y, contentsRect().width(), itemHeight(row) );
	y += itemHeight( row );
	++row;
    }
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*!
  Handles paint events for the popup menu.
*/

void QPopupMenu::paintEvent( QPaintEvent *e )
{
    QFrame::paintEvent( e );
}

/*!
  Handles close events for the popup menu.
*/

void QPopupMenu::closeEvent( QCloseEvent * e) {
    e->accept();
    hide();
     if ( parentMenu && parentMenu->isMenuBar )
 	byeMenuBar();
}


/*!
  Handles mouse press events for the popup menu.
*/

void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    QWidget* w;
    while ( (w = qApp->activePopupWidget() ) && w != this ){
	    w->close();
	    if (qApp->activePopupWidget() == w) // widget does not want to dissappear
		w->hide(); // hide at least
    }
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains(e->pos()) && !tryMenuBar(e) ) {
	    byeMenuBar();
	}
	return;
    }
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	actItem = item;
	updateRow( actItem );
	hilitSig( mi->id() );
    }
    QPopupMenu *popup = mi->popup();
    if ( popup ) {
	if ( popup->isVisible() ) {		// sub menu already open
	    int pactItem = popup->actItem;
	    popup->actItem = -1;
	    popup->hidePopups();
	    popup->updateRow( pactItem );
	} else {				// open sub menu
	    hidePopups();
	    popupSubMenuLater( 20, this );
	}
    } else {
	hidePopups();
    }
}

/*!
  Handles mouse release events for the popup menu.
*/

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !mouseBtDn && !parentMenu && actItem < 0 && motion < 5 )
	return;

    mouseBtDn = FALSE;

    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains( e->pos() ) && tryMenuBar(e) )
	    return;
    }
    if ( actItem >= 0 ) {			// selected menu item!
	register QMenuItem *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	bool b = QWhatsThis::inWhatsThisMode();
	if ( !mi->isEnabled() ) {
	    if ( b ) {
		byeMenuBar();
		actSig( mi->id(), b);
	    }
	} else 	if ( popup ) {
	    popup->setFirstItemActive();
	} else {				// normal menu item
	    byeMenuBar();			// deactivate menu bar
	    if ( mi->isEnabled() ) {
		if ( mi->signal() && !b ) // activate signal
		    mi->signal()->activate();
		actSig( mi->id(), b );
	    }
	}
    } else {
	byeMenuBar();
    }
}

/*!
  Handles mouse move events for the popup menu.
*/

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    motion++;
    if ( parentMenu && parentMenu->isPopupMenu &&
	 (parentMenu->actItem != ((QPopupMenu *)parentMenu)->popupActive ) ) {
	// hack it to work: if there's a parent popup, and its active
	// item is not the same as its popped-up child, make the
	// popped-up child active
	QPopupMenu * p = (QPopupMenu *)parentMenu;
	int lastActItem = p->actItem;
	p->actItem = p->popupActive;
	if ( lastActItem >= 0 )
	    p->updateRow( lastActItem );
	if ( p->actItem >= 0 )
	    p->updateRow( p->actItem );
    }

    if ( (e->state() & Qt::MouseButtonMask) == 0 &&
	 !hasMouseTracking() )
	return;

    int	 item = itemAtPos( e->pos() );
    if ( item == -1 ) {				// no valid item
	int lastActItem = actItem;
	actItem = -1;
	if ( lastActItem >= 0 )
	    updateRow( lastActItem );

	if ( !rect().contains( e->pos() ) && !tryMenuBar( e ) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
	else if ( singleSingleShot )
	    singleSingleShot->stop();
    } else {					// mouse on valid item
	// but did not register mouse press
	if ( (e->state() & Qt::MouseButtonMask) && !mouseBtDn )
	    mouseBtDn = TRUE; // so mouseReleaseEvent will pop down

	register QMenuItem *mi = mitems->at( item );
	if ( actItem == item )
	    return;

	if ( mi->popup() || (popupActive >= 0 && popupActive != item) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
	else if ( singleSingleShot )
	    singleSingleShot->stop();

	int lastActItem = actItem;
	actItem = item;
	if ( lastActItem >= 0 )
	    updateRow( lastActItem );
	updateRow( actItem );
	hilitSig( mi->id() );
    }
}


/*!
  Handles key press events for the popup menu.
*/

void QPopupMenu::keyPressEvent( QKeyEvent *e )
{
    QMenuItem  *mi = 0;
    QPopupMenu *popup;
    int dy = 0;
    bool ok_key = TRUE;

    switch ( e->key() ) {
    case Key_Tab:
	// ignore tab, otherwise it will be passed to the menubar
	break;
	
    case Key_Up:
	dy = -1;
	break;

    case Key_Down:
	dy = 1;
	break;

    case Key_Alt:
	if ( style() == WindowsStyle ) {
	    byeMenuBar();
	}
	break;

    case Key_Escape:
	// just hide one
	hide();
	if ( parentMenu && parentMenu->isMenuBar )
	    ((QMenuBar *)parentMenu)->setWindowsAltMode( FALSE, -1);
	break;

    case Key_Left:
	if ( parentMenu && parentMenu->isPopupMenu ) {
	    ((QPopupMenu *)parentMenu)->hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	} else {
	    ok_key = FALSE;
	}
	break;

    case Key_Right:
	if ( actItem >= 0 && (popup=mitems->at( actItem )->popup()) ) {
	    hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	    popup->setFirstItemActive();
	    subMenuTimer();
	} else {
	    ok_key = FALSE;
	}
	break;

    case Key_Space:
	if ( style() != MotifStyle )
	    break;
	// for motif, fall through

    case Key_Return:
    case Key_Enter:
	if ( actItem < 0 )
	    break;
	mi = mitems->at( actItem );
	popup = mi->popup();
	if ( popup ) {
	    hidePopups();
	    popupSubMenuLater( 20, this );
	    popup->setFirstItemActive();
	} else {
	    byeMenuBar();
	    bool b = QWhatsThis::inWhatsThisMode();
	    if ( mi->isEnabled() || b ) {
		if ( mi->signal() && !b ) // activate signal
		    mi->signal()->activate();
		actSig( mi->id(), b );
	    }
	}
	break;

    case Key_F1:
	if ( actItem < 0 || e->state() != ShiftButton)
	    break;
	mi = mitems->at( actItem );
	if ( !mi->whatsThis().isNull() ){
	    if ( !QWhatsThis::inWhatsThisMode() )
		QWhatsThis::enterWhatsThisMode();
	    int y = itemPos( actItem) + itemHeight( actItem );
	    QWhatsThis::leaveWhatsThisMode( mi->whatsThis(), mapToGlobal( QPoint(0,y) ) );
	}
    default:
	ok_key = FALSE;

    }
    if ( !ok_key && !e->state() && e->text().length()==1 ) {
	QChar c = e->text()[0].upper();

	QMenuItemListIt it(*mitems);
	register QMenuItem *m;
	int indx = 0;
	while ( (m=it.current()) ) {
	    ++it;
	    QString s = m->text();
	    if ( !s.isEmpty() ) {
		int i = s.find( '&' );
		if ( i >= 0 ) {
		    if ( s[i+1].upper() == c ) {
			mi = m;
			ok_key = TRUE;
			break;
		    }
		}
	    }
	    indx++;
	}
	if ( mi ) {
	    popup = mi->popup();
	    if ( popup ) {
		setActiveItem( indx );
		hidePopups();
		popupSubMenuLater( 20, this );
		popup->setFirstItemActive();
	    } else {
		byeMenuBar();
		bool b = QWhatsThis::inWhatsThisMode();
		if ( mi->isEnabled() || b ) {
		    if ( mi->signal() && !b  ) // activate signal
			mi->signal()->activate();
		    actSig( mi->id(), b );
		}
	    }
	}
    }
    if ( !ok_key ) {				// send to menu bar
	register QMenuData *top = this;		// find top level
	while ( top->parentMenu )
	    top = top->parentMenu;
	if ( top->isMenuBar )
	    ((QMenuBar*)top)->tryKeyEvent( this, e );
    }

    if ( dy && actItem < 0 ) {
	setFirstItemActive();
    } else if ( dy ) {				// highlight next/prev
	register int i = actItem;
	int c = mitems->count();
	int n = c;
	while ( n-- ) {
	    i = i + dy;
	    if ( i == c )
		i = 0;
	    else if ( i < 0 )
		i = c - 1;
	    mi = mitems->at( i );
	    if ( !mi->isSeparator()
		 && ( style() != MotifStyle || mi->isEnabled() ) )
		break;
	}
	if ( i != actItem ) {
	    int lastActItem = actItem;
	    actItem = i;
	    if ( mi->id() != 0 )
		hilitSig( mi->id() );
	    updateRow( lastActItem );
	    updateRow( actItem );
	}
    }
}


/*!
  Handles timer events for the popup menu.
*/

void QPopupMenu::timerEvent( QTimerEvent *e )
{
    QFrame::timerEvent( e );
}

/*!
  Reimplemented for internal purposes.
*/
void  QPopupMenu::styleChange( QStyle& old )
{
    style().polishPopupMenu( this );
    QFrame::styleChange( old );
}


/*! This private slot handles the delayed submenu effects */

void QPopupMenu::subMenuTimer() {
    if ( (actItem < 0 && popupActive < 0) || actItem == popupActive )
	return;

    if ( popupActive >= 0 ) {
	hidePopups();
	popupActive = -1;
    }

    QMenuItem *mi = mitems->at(actItem);
    if ( !mi || !mi->isEnabled() )
	return;

    QPopupMenu *popup = mi->popup();
    if ( !popup || !popup->isEnabled() )
	return;

    //avoid circularity
    if ( popup->isVisible() )
	return;

    if (popup->parentMenu != this ){
	// reuse
	if (popup->parentMenu)
	    popup->parentMenu->menuDelPopup(popup);
	popup->selfItem  = mi;
	menuInsPopup(popup);
    }

    emit popup->aboutToShow();
    supressAboutToShow = TRUE;

    QPoint p( width() - motifArrowHMargin, frameWidth() + motifArrowVMargin );
    for ( int i=0; i<actItem; i++ )
	p.setY( p.y() + (QCOORD)itemHeight( i ) );
    p = mapToGlobal( p );
    QSize ps = popup->sizeHint();
    if (p.y() + ps.height() > QApplication::desktop()->height()
	&& p.y() - ps.height()
	+ (QCOORD)(popup->itemHeight( popup->count()-1)) >= 0)
	p.setY( p.y() - ps.height()
		+ (QCOORD)(popup->itemHeight( popup->count()-1)));
    popupActive = actItem;
    bool left = FALSE;
    if ( ( parentMenu && parentMenu->isPopupMenu &&
	   ((QPopupMenu*)parentMenu)->geometry().x() > geometry().x() ) ||
	 p.x() + ps.width() > QApplication::desktop()->width() )
	left = TRUE;
    if ( left && ps.width() > mapToGlobal( QPoint(0,0) ).x() )
	left = FALSE;
    if ( left )
	p.setX( mapToGlobal(QPoint(0,0)).x() - ps.width() + frameWidth() );
    popup->popup( p );
}


void QPopupMenu::updateRow( int row )
{
    if ( badSize ){
	updateSize();
	update();
	return;
    }
    if ( !isVisible() )
	return;
    QPainter p(this);
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    int r = 0;
    int y = contentsRect().y();
    while ( (mi=it.current()) ) {
	++it;
	if ( r == row )
	    drawItem(&p, tab, mi, r == actItem, contentsRect().x(), y, contentsRect().width(), itemHeight(r) );
	y += itemHeight( r );
	++r;
    }
}


/*!  Execute this popup synchronously.

  Opens the popup menu so that the item number \a indexAtPoint will be
  at the specified \e global position \a pos.  To translate a widget's
  local coordinates into global coordinates, use QWidget::mapToGlobal().

  The return code is the ID of the selected item in either the popup
  menu or one of its submenus, or -1 if no item is selected (normally
  because the user presses Escape).

  Note that all signals are emitted as usual.  If you connect a menu
  item to a slot and call the menu's exec(), you get the result both
  via the signal-slot connection and in the return value of exec().

  Common usage is to position the popup at the current
  mouse position:
  \code
      exec(QCursor::pos());
  \endcode
  or aligned to a widget:
  \code
      exec(somewidget.mapToGlobal(QPoint(0,0)));
  \endcode
  \sa popup()

  When positioning a popup with exec() or popup(), keep in mind that
  you cannot rely on the popup menu's current size(). For performance
  reasons, the popup adapts its size only when actually needed. So in
  many cases, the size before and after the show is
  different. Instead, use sizeHint(). It calculates the proper size
  depending on the menu's current contents.

  \sa sizeHint()
*/

int QPopupMenu::exec( const QPoint & pos, int indexAtPoint )
{
    if ( !qApp )
	return -1;

    QPopupMenu* priorSyncMenu = syncMenu;

    syncMenu = this;
    syncMenuId = -1;

    connectModal( this, TRUE );
    popup( pos, indexAtPoint );
    qApp->enter_loop();
    connectModal( this, FALSE );

    syncMenu = priorSyncMenu;
    return syncMenuId;
}



/*
  connect the popup and all its submenus to modalActivation() if
  \a doConnect is true, otherwise disconnect.
 */
void QPopupMenu::connectModal( QPopupMenu* receiver, bool doConnect )
{
    if ( doConnect )
	connect( this, SIGNAL(activated(int)),
		 receiver, SLOT(modalActivation(int)) );
    else
	disconnect( this, SIGNAL(activated(int)),
		    receiver, SLOT(modalActivation(int)) );

    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() && mi->popup() != receiver && mi->popup()->parentMenu == this ) //avoid circularity
	    mi->popup()->connectModal( receiver, doConnect );
    }
}


/*!  Execute this popup synchronously.

  Similar to the above function, but the position of the
  popup is not set, so you must choose an appropriate position.
  The function move the popup if it is partially off-screen.

  More common usage is to position the popup at the current
  mouse position:
  \code
      exec(QCursor::pos());
  \endcode
  or aligned to a widget:
  \code
      exec(somewidget.mapToGlobal(QPoint(0,0)));
  \endcode
*/

int QPopupMenu::exec()
{
    return exec(mapToGlobal(QPoint(0,0)));
}


/*!  Internal slot used for exec(). */

void QPopupMenu::modalActivation( int id )
{
    syncMenuId = id;
}


/*!  Sets the currently active item to \a i and repaints as necessary.
*/

void QPopupMenu::setActiveItem( int i )
{
    int lastActItem = actItem;
    actItem = i;
    if ( lastActItem >= 0 )
	updateRow( lastActItem );
    if ( i >= 0 && i != lastActItem )
	updateRow( i );
}


/*!
  Returns the size the popupmenu would use shall it become visible
  now. (##### is that english??)

  Note that this size may be different from the popup's actual
  size. It changes all the time a new item is added or an existing one
  is modified. For performance reasons, QPopupMenu doesn't change its
  physical size each time this happens but only once before it is
  shown.

  \sa exec(), show()
 */
QSize QPopupMenu::sizeHint() const
{
    if ( !testWState(WState_Polished) ) {
	QPopupMenu* that = (QPopupMenu*) this;
	that->polish();
    }

    if ( badSize ) {
	QPopupMenu* that = (QPopupMenu*) this;
	that->updateSize();
    }
    return size();
}


/*!
  Return the id of the item at \e pos, or -1 if there is no item
  there, or if it is a separator item.
 */
int QPopupMenu::idAt( const QPoint& pos ) const
{
    return idAt( itemAtPos( pos ) );
}


/*!\fn int QPopupMenu::idAt( int index ) const

  Returns the identifier of the menu item at position \a index in the internal
  list, or -1 if \a index is out of range.

  \sa QMenuData::setId(), QMenuData::indexOf()
*/


/*!\reimp
 */
bool QPopupMenu::customWhatsThis() const
{
    return TRUE;
}
