/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopupmenu.cpp#274 $
**
** Implementation of QPopupMenu class
**
** Created : 941128
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
#include "qobjectlist.h"
#include "qguardedptr.h"
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

static void popupSubMenuLater( int msec, QPopupMenu * receiver ) {
    if ( !singleSingleShot )
	singleSingleShot = new QTimer( qApp, "popup submenu timer" );
    singleSingleShot->disconnect( SIGNAL(timeout()) );
    QObject::connect( singleSingleShot, SIGNAL(timeout()),
		      receiver, SLOT(subMenuTimer()) );
    singleSingleShot->start( msec, TRUE );
}


// NOT REVISED
/*!
  \class QPopupMenu qpopupmenu.h
  \brief The QPopupMenu class provides a popup menu widget.

  \ingroup application
  \ingroup basic

  A popup menu widget is a selection menu. It can be both, a pull-down
  menu in a menu bar or a standalone context menu.  Pull-down menus
  are shown by the menu bar when the user clicks on the respecitive
  item or hits the specified shortcut key. Use QMenuBar::insertItem()
  to insert a popup menu into a menu bar. Show a context menu either
  asynchronously with popup() or synchronously with exec().

  Technically, a popup menu consists of a list of menu items. You add
  items with insertItem().  An item is either a string, a pixmap or a
  custom item that provides its own drawing function (see
  QCustomMenuItem). In addition, items can have an optional icon drawn
  on the very left side and an accelerator key, like "Ctrl-X".

  There are three kind of menu items: seperators, those that perform
  an action and those that show a submenu.  Separators are inserted
  with insertSeparator(). For submenus, you pass a pointer to a
  QPopupMenu in your call to insertItem().  All other items are
  considered action items.

  When inserting actions items, you usually specify a receiver and a
  slot. The receiver will be notifed whenever the item was
  selected. In addition, QPopupMenu provides two signals activated()
  and highlighted() that signal the identifier of the respective menu
  item. Sometimes it is practical to connect several items to one
  slot. To distinguish between them, specify a slot that takes an
  integer argument and use setItemParameter() to associate a unique
  value with each item.

  You clear a popup menu with clear() and remove single items with
  removeItem() or removeItemAt().

  A popup menu can display check marks for certain items when enabled
  with setCheckable(TRUE). You check or uncheck items with
  setItemChecked().

  Items are either enabled or disabled. You toggle their state with
  setItemEnabled().  Just before a popup menu becomes visible, it
  emits the aboutToShow() signal. You can use this signal to set the
  correct enabled/disabled states of all menu items before the user
  sees it.

  You can provide What's This? help for single menu items with
  setWhatsThis(). See QWhatsThis for general information about this
  kind of light-weight online help.

  For ultimate flexibility, you can also add entire widgets as items
  into a popup menu, for example a color selector.

  A QPopupMenu can also provide a tear-off menu. A tear-off menu is a
  "torn off" copy of a menu that lives in a separate window. This
  makes it possible for the user to "tear off" frequently used menus
  and position them in a convenient place on the screen. If you want
  that functionality for a certain menu, insert a tear-off handle with
  insertTearOffHandle(). When using tear-off menus, keep in mind that
  the concept isn't typically used on MS-Windows, so users may not be
  familiar with it. Consider using a QToolBar instead.

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  \important insertItem removeItem removeItemAt clear text pixmap iconSet  insertSeparator
  changeItem whatsThis setWhatsThis accel setAccel setItemEnabled isItemEnabled
  setItemChecked isItemChecked connectItem disconnectItem setItemParameter itemParameter

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

class QMenuDataData {
    // attention: also defined in qmenudata.cpp
public:
    QMenuDataData();
    QGuardedPtr<QWidget> aWidget;
    int aInt;
};


static QPopupMenu* active_popup_menu = 0;
/*!
  Constructs a popup menu with a parent and a widget name.

  Although a popup menu is always a top level widget, if a parent is
  passed, the popup menu will be deleted on destruction of that parent
  (as with any other QObject).

*/

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
    : QFrame( parent, name, WType_Popup  | WRepaintNoErase )
{
    isPopupMenu	  = TRUE;
    parentMenu	  = 0;
    selfItem	  = 0;
    autoaccel	  = 0;
    accelDisabled = FALSE;
    popupActive	  = -1;
    tab = 0;
    checkable = 0;
    tearedOff = 0;
    maxPMWidth = 0;

    tab = 0;
    ncols = 1;
    setFrameStyle( QFrame::PopupPanel | QFrame::Raised );
    style().polishPopupMenu( this );
    setBackgroundMode( PaletteButton );
    connectModalRecursionSafety = 0;

    setFocusPolicy( StrongFocus );
}

/*!
  Destructs the popup menu.
*/

QPopupMenu::~QPopupMenu()
{
    if ( syncMenu == this ) {
	qApp->exit_loop();
	syncMenu = 0;
    }

    delete (QWidget*) QMenuData::d->aWidget;  // tear-off menu

    if ( parentMenu )
	parentMenu->removePopup( this );	// remove from parent menu
}


/*!
  Updates the item with identity \a id.

  \sa updateRow()
*/
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
	if ( QMenuData::d->aWidget )
	    ( (QPopupMenu*)(QWidget*)QMenuData::d->aWidget)->setCheckable( enable );
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
    if ( isVisible() ) {
	if ( tearedOff )
	    return;
	updateSize();
	update();
    }
    QPopupMenu* p = (QPopupMenu*)(QWidget*)QMenuData::d->aWidget;
    if ( p && p->isVisible() ) {
	p->mitems->clear();
	for ( QMenuItemListIt it( *mitems ); it.current(); ++it ) {
	    if ( it.current()->id() != QMenuData::d->aInt && !it.current()->widget() )
		p->mitems->append( it.current() );
	}
	p->updateSize();
	p->update();
    }
}

void QPopupMenu::menuStateChanged()
{
    update();
    if ( QMenuData::d->aWidget )
	QMenuData::d->aWidget->update();
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
    if ( !isPopup() && isVisible() )
	hide();

    //avoid circularity
    if ( isVisible() || !isEnabled() )
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
	y -= itemGeometry( indexAtPoint ).y();		// (would subtract 2 pixels!)
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
	actSig( mi->id() );
	if ( mi->signal() )
	    mi->signal()->activate();
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
	QRect r( itemGeometry( indexOf( id ) ) );
	QWhatsThis::leaveWhatsThisMode( findItem( id )->whatsThis(), mapToGlobal( r.bottomLeft() ) );
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
    int ai = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( !mi->isSeparator() && mi->id() != QMenuData::d->aInt ) {
	    setActiveItem( ai );
	    return;
	}
	ai++;
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
    if ( !isPopup() )
	return; // nothing to do

    while ( top->parentMenu && top->parentMenu->isPopupMenu
	    && ((QPopupMenu*)top->parentMenu)->isPopup() )
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
	((QMenuBar *)top)->tryMouseEvent( this, e ) :
			      ((QPopupMenu*)top)->tryMouseEvent(this, e );
}


/*!
  \internal
*/
bool QPopupMenu::tryMouseEvent( QPopupMenu *p, QMouseEvent * e)
{
    if ( p == this )
	return FALSE;
    QPoint pos = mapFromGlobal( e->globalPos() );
    if ( !rect().contains( pos ) )		// outside
	return FALSE;
    QMouseEvent ee( e->type(), pos, e->globalPos(), e->button(), e->state() );
    event( &ee );
    return TRUE;
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
    int x = contentsRect().x();
    int y = contentsRect().y();
    int itemw = contentsRect().width() / ncols;

    QMenuItem *mi;
    QMenuItemListIt it( *mitems );
    while ( (mi=it.current()) ) {
	++it;
	int itemh = itemHeight( row );
	if ( ncols > 1 && y + itemh > contentsRect().bottom() ) {
	    y = contentsRect().y();
	    x +=itemw;
	}
	if ( QRect( x, y, itemw, itemh ).contains( pos ) )
	    break;
	y += itemh;
	++row;
    }

    if ( mi && !mi->isSeparator() )
	return row;
    return -1;
}

/*!
  \internal
  Returns the geometry of item number \e index.
*/

QRect QPopupMenu::itemGeometry( int index )
{
    QMenuItem *mi;
    int row = 0;
    int x = contentsRect().x();
    int y = contentsRect().y();
    int itemw = contentsRect().width() / ncols;
    QMenuItemListIt it( *mitems );
    while ( (mi=it.current()) ) {
	++it;
	int itemh = itemHeight( mi );
	if ( ncols > 1 && y + itemh > contentsRect().bottom() ) {
	    y = contentsRect().y();
	    x +=itemw;
	}
	if ( row == index )
	    return QRect( x,y,itemw,itemh );
	y += itemh;
	++row;
    }

    return QRect(0,0,0,0);
}


/*!
  \internal
  Calculates and sets the size of the popup menu, based on the size
  of the items.
*/

void QPopupMenu::updateSize()
{
    polish();
    updateAccel( 0 );
    int height = 0;
    int max_width = 0;
    QFontMetrics fm = fontMetrics();
    register QMenuItem *mi;
    maxPMWidth = 0;
    int maxWidgetWidth = 0;
    tab = 0;
    int arrow_width = style().popupSubmenuIndicatorWidth( fm );

    bool hasWidgetItems = FALSE;

    for ( QMenuItemListIt it( *mitems ); it.current(); ++it ) {
	mi = it.current();
	if ( mi->widget() && mi->widget()->parentWidget() != this ) {
	    mi->widget()->reparent( this, QPoint(0,0), TRUE );
	}
	if ( mi->custom() )
	    mi->custom()->setFont( font() );
	if ( mi->iconSet() != 0)
	    maxPMWidth = QMAX( maxPMWidth,
			       mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4 );
    }

    int dh = QApplication::desktop()->height();
    ncols = 1;

    for ( QMenuItemListIt it2( *mitems ); it2.current(); ++it2 ) {
	mi = it2.current();
	int w = 0;
	int itemHeight = QPopupMenu::itemHeight( mi );
	
	if ( mi->widget() ) {
	    hasWidgetItems = TRUE;
	    QSize s( mi->widget()->sizeHint() );
	    s.expandedTo( mi->widget()->minimumSize() );
	    mi->widget()->resize( s );
	    if ( s.width()  > maxWidgetWidth )
		maxWidgetWidth = s.width();
	    itemHeight = s.height();
	} else {
	    if ( mi->custom() ) {
		if ( mi->custom()->fullSpan() ) {
		    maxWidgetWidth = QMAX( maxWidgetWidth, mi->custom()->sizeHint().width() );
		} else {
		    QSize s ( mi->custom()->sizeHint() );
		    w += s.width();
		}
	    }
	
	    if ( !mi->text().isNull() && !mi->isSeparator() ) {
		QString s = mi->text();
		int t;
		if ( (t=s.find('\t')) >= 0 ) {	// string contains tab
		    w += fm.width( s, t );
		    w -= s.contains('&')*fm.width('&');
		    w += s.contains("&&")*fm.width('&');
		    int tw = fm.width( s.mid(t+1) );
		    if ( tw > tab)
			tab = tw;
		} else {
		    w += fm.width( s );
		    w -= s.contains('&')*fm.width('&');
		    w += s.contains("&&")*fm.width('&');
		}
	    }
	}

	w += style().extraPopupMenuItemWidth( checkable, maxPMWidth, mi, fm );
	
	if ( mi->popup() ) { // submenu indicator belong in the right tab area
	    if ( arrow_width > tab )
		tab = arrow_width;
	}
	

#if defined(CHECK_NULL)
	if ( mi->text().isNull() && !mi->pixmap() && !mi->isSeparator() && !mi->widget() && !mi->custom() )
	    qWarning( "QPopupMenu: (%s) Popup has invalid menu item",
		     name( "unnamed" ) );
#endif
	
	height += itemHeight;
	if ( height + 2*frameWidth()  >= dh ) {
	    ncols++;
	    height = 0;
	}
	
	if ( w > max_width )
	    max_width = w;
    }

    if ( tab )
	tab -= fontMetrics().minRightBearing();
    else
	max_width -= fontMetrics().minRightBearing();

    if ( max_width + tab < maxWidgetWidth )
	max_width = maxWidgetWidth - tab;

    if ( ncols == 1 ) {
	setFixedSize( max_width + tab + 2*frameWidth(), height + 2*frameWidth() );
    }
    else {
	setFixedSize( (ncols*(max_width + tab)) + 2*frameWidth(), dh );
    }
	
    badSize = FALSE;


    if ( hasWidgetItems ) {
	// Position the widget items. It could be done in drawContents
	// but this way we get less flicker.
	QMenuItemListIt it(*mitems);
	QMenuItem *mi;
	int row = 0;
	int x = contentsRect().x();
	int y = contentsRect().y();
	int itemw = contentsRect().width() / ncols;
	while ( (mi=it.current()) ) {
	    ++it;
	    int itemh = itemHeight( row );
	    if ( ncols > 1 && y + itemh > contentsRect().bottom() ) {
		y = contentsRect().y();
		x +=itemw;
	    }
	    if ( mi->widget() )
		mi->widget()->setGeometry( x, y, itemw, mi->widget()->height() );
	    y += itemh;
	    ++row;
	}
    }
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

    if ( parent ) {
	delete autoaccel;
	autoaccel = 0;
    } else if ( !autoaccel ) {
	// we have no parent. Rather than ignoring any accelerators we try to find this popup's main window
	QWidget *w = (QWidget *) this;
	parent = w->parentWidget();
	while ( (!w->testWFlags(WType_TopLevel) || !w->testWFlags(WType_Popup)) && parent ) {
	    w = parent;
	    parent = parent->parentWidget();
	}
    }

    if ( parent == 0 && autoaccel == 0 )
 	return;

    if ( autoaccel )				// build it from scratch
	autoaccel->clear();
    else {
	// create an autoaccel in any case, even if we might not use
	// it immediately. Maybe the user needs it later.
	autoaccel = new QAccel( parent, this );
	connect( autoaccel, SIGNAL(activated(int)),
		 SLOT(accelActivated(int)) );
	connect( autoaccel, SIGNAL(destroyed()),
		 SLOT(accelDestroyed()) );
	if ( accelDisabled )
	    autoaccel->setEnabled( FALSE );
    }
    while ( (mi=it.current()) ) {
	++it;
	int k = mi->key();
	if ( k ) {
	    int id = autoaccel->insertItem( k, mi->id() );
	    autoaccel->setWhatsThis( id, mi->whatsThis() );
	}
	if ( !mi->text().isNull() || mi->custom() ) {
	    QString s = mi->text();
	    int i = s.find('\t');
	    if (k) {
		QString t = QAccel::keyToString( k );
		if ( i >= 0 )
		    s.replace( i+1, s.length()-i, t );
		else {
		    s += '\t';
		    s += t;
		}
	    } else {
		if ( i >= 0 )
		    s.truncate( i );
	    }
	    if ( s != mi->text() ) {
		mi->setText( s );
		badSize = TRUE;
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


/*!\reimp
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

/*!\reimp
*/
void QPopupMenu::show()
{
    if ( !isPopup() && isVisible() )
	hide();

    if ( isVisible() ) {
	supressAboutToShow = FALSE;
	QWidget::show();
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

/*!\reimp
*/

void QPopupMenu::hide()
{	
    if ( !isVisible() ) {
	QWidget::hide();
  	return;
    }

    actItem = popupActive = -1;
    mouseBtDn = FALSE;				// mouse button up
    hidePopups();
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
    if  ( mi->widget() )
	return mi->widget()->height();
    if ( mi->custom() && mi->custom()->fullSpan() )
	return mi->custom()->sizeHint().height();
    return style().popupMenuItemHeight( checkable, mi, fontMetrics() );
}


/*!
  Draws item \a mi in the area \a x, \a y, \a w, \a h,
  using painter \a p.  The item is drawn active or inactive according
  to \a act, and using the rightmost \a tab_ pixels for accelerator
  text.

  \sa QStyle::drawPopupMenuItem()
*/
void QPopupMenu::drawItem( QPainter* p, int tab_, QMenuItem* mi,
			   bool act, int x, int y, int w, int h)
{
    bool dis = (selfItem && !selfItem->isEnabled()) || !mi->isEnabled();
    if ( mi->custom() && mi->custom()->fullSpan() ) {
	QMenuItem dummy;
	style().drawPopupMenuItem(p, checkable, maxPMWidth, tab_, &dummy, palette(),
				  act, !dis, x, y, w, h);
	mi->custom()->paint( p, colorGroup(), act, !dis, x, y, w, h );
    }
    else
	style().drawPopupMenuItem(p, checkable, maxPMWidth, tab_, mi, palette(),
				  act, !dis, x, y, w, h);
}


/*!
  Draws all menu items.
*/
void QPopupMenu::drawContents( QPainter* p )
{
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    int row = 0;
    int x = contentsRect().x();
    int y = contentsRect().y();
    int itemw = contentsRect().width() / ncols;
    while ( (mi=it.current()) ) {
	++it;
	int itemh = itemHeight( row );
	if ( ncols > 1 && y + itemh > contentsRect().bottom() ) {
	    if ( y < contentsRect().bottom() ) {
		style().drawPopupMenuItem(p, checkable, maxPMWidth, tab, 0, palette(),
					  FALSE, TRUE, x, y, itemw, contentsRect().bottom()-y);
	    }
	    y = contentsRect().y();
	    x +=itemw;
	}
	drawItem( p, tab, mi, row == actItem, x, y, itemw, itemh );
	y += itemh;
	++row;
    }
    if ( y < contentsRect().bottom() )
	style().drawPopupMenuItem(p, checkable, maxPMWidth, tab, 0, palette(),
				  FALSE, TRUE, x, y, itemw, contentsRect().bottom()-y);
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*!\reimp
*/

void QPopupMenu::paintEvent( QPaintEvent *e )
{
    QFrame::paintEvent( e );
}

/*!\reimp
*/

void QPopupMenu::closeEvent( QCloseEvent * e) {
    e->accept();
    hide();
    byeMenuBar();
}


/*!\reimp
*/

void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains(e->pos()) && !tryMenuBar(e) ) {
	    byeMenuBar();
	}
	return;
    }
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem )			// new item activated
	setActiveItem( item );

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

/*!\reimp
*/

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !mouseBtDn && !parentMenu && actItem < 0 && motion < 6 )
	return;

    mouseBtDn = FALSE;

    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains( e->pos() ) && tryMenuBar(e) )
	    return;
    }
    if ( actItem >= 0 ) {			// selected menu item!
	register QMenuItem *mi = mitems->at(actItem);
	if ( mi ->widget() ) {
	    QWidget* widgetAt = QApplication::widgetAt( e->globalPos(), TRUE );
	    if ( widgetAt && widgetAt != this ) {
		QMouseEvent me( e->type(), widgetAt->mapFromGlobal( e->globalPos() ),
				e->globalPos(), e->button(), e->state() );
		QApplication::sendEvent( widgetAt, &me );
	    }
	}
	QPopupMenu *popup = mi->popup();
	bool b = QWhatsThis::inWhatsThisMode();
	if ( !mi->isEnabled() ) {
	    if ( b ) {
		actItem = -1;
		updateItem( mi->id() );
		byeMenuBar();
		actSig( mi->id(), b);
	    }
	} else 	if ( popup ) {
	    popup->setFirstItemActive();
	} else {				// normal menu item
	    byeMenuBar();			// deactivate menu bar
	    if ( mi->isEnabled() ) {
		actItem = -1;
		updateItem( mi->id() );
		active_popup_menu = this;
		actSig( mi->id(), b );
		if ( mi->signal() && !b )
		    mi->signal()->activate();
		active_popup_menu = 0;
	    }
	}
    } else {
	byeMenuBar();
    }
}

/*!\reimp
*/

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    motion++;

    if ( (e->state() & Qt::MouseButtonMask) == 0 &&
	 !hasMouseTracking() )
	return;

    int	 item = itemAtPos( e->pos() );
    if ( item == -1 ) {				// no valid item
	int lastActItem = actItem;
	actItem = -1;
	if ( lastActItem >= 0 )
	    updateRow( lastActItem );

	if ( !rect().contains( e->pos() )  && !tryMenuBar( e ) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
    } else {					// mouse on valid item
	// but did not register mouse press
	if ( (e->state() & Qt::MouseButtonMask) && !mouseBtDn )
	    mouseBtDn = TRUE; // so mouseReleaseEvent will pop down

	register QMenuItem *mi = mitems->at( item );
	
	if ( mi ->widget() ) {
	    QWidget* widgetAt = QApplication::widgetAt( e->globalPos(), TRUE );
	    if ( widgetAt && widgetAt != this ) {
		QMouseEvent me( e->type(), widgetAt->mapFromGlobal( e->globalPos() ),
				e->globalPos(), e->button(), e->state() );
		QApplication::sendEvent( widgetAt, &me );
	    }
	}
	
	if ( actItem == item )
	    return;

	if ( mi->popup() || (popupActive >= 0 && popupActive != item) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
	else if ( singleSingleShot )
	    singleSingleShot->stop();

	if ( item != actItem )
	    setActiveItem( item );
    }
}


/*!\reimp
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
	if ( tearedOff ) {
	    close();
	    return;
	}
	// just hide one
	hide();
  	if ( parentMenu && parentMenu->isMenuBar )
	    ((QMenuBar*) parentMenu)->goodbye( TRUE );
	break;

    case Key_Left:
	if ( ncols > 1 && actItem >= 0 ) {
	    QRect r( itemGeometry( actItem ) );
	    int newActItem = itemAtPos( QPoint( r.left() - 1, r.center().y() ) );
	    if ( newActItem >= 0 ) {
		setActiveItem( newActItem );
		break;
	    }
	}
	if ( parentMenu && parentMenu->isPopupMenu ) {
	    ((QPopupMenu *)parentMenu)->hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	    break;
	}

	ok_key = FALSE;
    	break;

    case Key_Right:
	if ( actItem >= 0 && (popup=mitems->at( actItem )->popup()) ) {
	    hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	    popup->setFirstItemActive();
	    subMenuTimer();
	    break;
	}
	if ( ncols > 1 && actItem >= 0 ) {
	    QRect r( itemGeometry( actItem ) );
	    int newActItem = itemAtPos( QPoint( r.right() + 1, r.center().y() ) );
	    if ( newActItem >= 0 ) {
		setActiveItem( newActItem );
		break;
	    }
	}
	ok_key = FALSE;
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
	    actItem = -1;
	    updateItem( mi->id() );
	    byeMenuBar();
	    bool b = QWhatsThis::inWhatsThisMode();
	    if ( mi->isEnabled() || b ) {
		active_popup_menu = this;
		actSig( mi->id(), b );
		if ( mi->signal() && !b )
		    mi->signal()->activate();
		active_popup_menu = 0;
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
	    QRect r( itemGeometry( actItem) );
	    QWhatsThis::leaveWhatsThisMode( mi->whatsThis(), mapToGlobal( r.bottomLeft()) );
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
		    active_popup_menu = this;
		    actSig( mi->id(), b );
		    if ( mi->signal() && !b  )
			mi->signal()->activate();
		    active_popup_menu = 0;
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
	if ( i != actItem )
	    setActiveItem( i );
    }
}


/*!\reimp
*/

void QPopupMenu::timerEvent( QTimerEvent *e )
{
    QFrame::timerEvent( e );
}

/*!\reimp
*/
void  QPopupMenu::styleChange( QStyle& old )
{
    style().polishPopupMenu( this );
    QFrame::styleChange( old );
}


/*! This private slot handles the delayed submenu effects */

void QPopupMenu::subMenuTimer() {

    if ( !isVisible() || (actItem < 0 && popupActive < 0) || actItem == popupActive )
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


    QRect r( itemGeometry( actItem ) );
    QPoint p( r.right() - motifArrowHMargin, r.top() + motifArrowVMargin );
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
    if ( left && (ps.width() > mapToGlobal( r.topLeft() ).x() ) )
	left = FALSE;
    if ( left )
	p.setX( mapToGlobal( r.topLeft() ).x() - ps.width() );
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
    int x = contentsRect().x();
    int y = contentsRect().y();
    int itemw = contentsRect().width() / ncols;
    while ( (mi=it.current()) ) {
	++it;
	int itemh = itemHeight( r );
	if ( ncols > 1 && y + itemh > contentsRect().bottom() ) {
	    y = contentsRect().y();
	    x +=itemw;
	}
	if ( r == row )
	    drawItem(&p, tab, mi, r == actItem, x, y, itemw, itemh );
	y += itemh;
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

  When positioning a popup with exec() or popup(), keep in mind that
  you cannot rely on the popup menu's current size(). For performance
  reasons, the popup adapts its size only when actually needed. So in
  many cases, the size before and after the show is
  different. Instead, use sizeHint(). It calculates the proper size
  depending on the menu's current contents.

  \sa popup(), sizeHint()
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
    connectModalRecursionSafety = doConnect;

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
	if ( mi->popup() && mi->popup() != receiver
	     && (bool)(mi->popup()->connectModalRecursionSafety) != doConnect )
	    mi->popup()->connectModal( receiver, doConnect ); //avoid circular
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
    QMenuItem *mi = mitems->at( actItem );
    if ( !mi )
	return;
    if ( mi->widget() && mi->widget()->isFocusEnabled() )
	mi->widget()->setFocus();
    else
	setFocus();
    if ( mi->id() != 0 )
	hilitSig( mi->id() );
}


/*!\reimp
 */
QSize QPopupMenu::sizeHint() const
{
    constPolish();
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


/*!\reimp
 */
bool QPopupMenu::focusNextPrevChild( bool next )
{
    register QMenuItem *mi;
    int dy = next? 1 : -1;
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
	if ( i != actItem )
	    setActiveItem( i );
    }
    return TRUE;
}


/*!\reimp
 */
void QPopupMenu::focusInEvent( QFocusEvent * )
{
}

/*!\reimp
 */
void QPopupMenu::focusOutEvent( QFocusEvent * )
{
}


class QTearOffMenuItem : public QCustomMenuItem
{
public:
    QTearOffMenuItem()
    {
    }
    ~QTearOffMenuItem()
    {
    }
    void paint( QPainter* p, const QColorGroup& cg, bool /* act*/,
		bool /*enabled*/, int x, int y, int w, int h )
    {
	p->setPen( QPen( cg.dark(), 1, DashLine ) );
	p->drawLine( x+2, y+h/2-1, x+w-4, y+h/2-1 );
	p->setPen( QPen( cg.light(), 1, DashLine ) );
	p->drawLine( x+2, y+h/2, x+w-4, y+h/2 );
    }
    bool fullSpan() const
    {
	return TRUE;
    }

    QSize sizeHint()
    {
	return QSize( 20, 6 );
    }
};



/*!
  Inserts a tear-off handle into the menu. A tear-off handle is a
  special menu item, that - when selected - creates a copy of the
  menu. This "torn off" copy lives in a separate window. It contains
  the same choices as the original menu, with the exception of the
  tear-off handle.

  You may also want to set a proper window title for the tear-off menu
  with setCaption().

  The handle item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The tear-off
  handle is appended at the end of the list if \a index is negative.
 */
int QPopupMenu::insertTearOffHandle( int id, int index )
{
    int myid = insertItem( new QTearOffMenuItem, id, index );
    connectItem( myid, this, SLOT( toggleTearOff() ) );
    QMenuData::d->aInt = myid;
    return myid;
}


/*!\internal

  implements tear-off menus
 */
void QPopupMenu::toggleTearOff()
{
    if ( active_popup_menu && active_popup_menu->tearedOff ) {
	active_popup_menu->close();
    } else  if (QMenuData::d->aWidget ) {
	delete (QWidget*) QMenuData::d->aWidget; // delete the old one
    } else {
	// create a tear off menu
	QPopupMenu* p = new QPopupMenu( 0, "tear off menu" );
	p->setCaption( caption() );
	p->setCheckable( isCheckable() );
	p->reparent( parentWidget(), WType_TopLevel | WStyle_Tool |
		     WRepaintNoErase | WDestructiveClose,
		     geometry().topLeft(), FALSE );
	style().polishPopupMenu( p );
	p->mitems->setAutoDelete( FALSE );
	p->tearedOff = TRUE;
	for ( QMenuItemListIt it( *mitems ); it.current(); ++it ) {
	    if ( it.current()->id() != QMenuData::d->aInt && !it.current()->widget() )
		p->mitems->append( it.current() );
	}
	p->show();
	QMenuData::d->aWidget = p;
    }
}

