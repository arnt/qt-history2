/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenudta.cpp#47 $
**
** Implementation of QMenuData class
**
** Created : 941128
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define	 INCLUDE_MENUITEM_DEF
#include "qmenudta.h"
#include "qpopmenu.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qmenudta.cpp#47 $");


/*!
  \class QMenuData qmenudta.h
  \brief The QMenuData class is a base class for QMenuBar and QPopupMenu.

  QMenuData has an internal list of menu items.	 A menu item is a text,
  pixmap or a separator, and may also have a popup menu (separators
  have no popup menus).

  The menu item sends out an activated() signal when it is selected, and
  a highlighted() signal when it receives the user input focus.

  Menu items can be accessed through identifiers.

  \sa QAccel
*/


/*****************************************************************************
  QMenuItem member functions
 *****************************************************************************/

QMenuItem::QMenuItem()
{
    ident	 = -1;
    is_separator = is_checked = FALSE;
    is_enabled	 = TRUE;
    is_dirty	 = TRUE;
    pixmap_data	 = 0;
    popup_menu	 = 0;
    accel_key	 = 0;
    signal_data	 = 0;
}

QMenuItem::~QMenuItem()
{
    delete pixmap_data;
    delete signal_data;
}


/*****************************************************************************
  QMenuData member functions
 *****************************************************************************/


static int get_seq_id()
{
    static int seq_no = -2;
    return seq_no--;
}


/*!
  Constructs an empty list.
*/

QMenuData::QMenuData()
{
    actItem = -1;				// no active menu item
    mitems = new QMenuItemList;			// create list of menu items
    CHECK_PTR( mitems );
    mitems->setAutoDelete( TRUE );
    parentMenu = 0;				// assume top level
    isPopupMenu = isMenuBar = mouseBtDn = FALSE;
    badSize = TRUE;
}

/*!
  Removes all menu items and disconnects any signals that have been connected.
*/

QMenuData::~QMenuData()
{
    register QMenuItem *mi = mitems->first();
    while ( mi ) {
	if ( mi->popup_menu )			// reset parent pointer for all
	    mi->popup_menu->parentMenu = 0;	//   child menus
	mi = mitems->next();
    }
    delete mitems;				// delete menu item list
}


/*!
  Virtual function; notifies subclasses about an item that has been changed.
*/

void QMenuData::updateItem( int )		// reimplemented in subclass
{
}

/*!
  Virtual function; notifies subclasses that one or more items have been
  inserted or removed.
*/

void QMenuData::menuContentsChanged()		// reimplemented in subclass
{
}

/*!
  Virtual function; notifies subclasses that one or more items have changed
  state (enabled/disabled or checked/unchecked).
*/

void QMenuData::menuStateChanged()		// reimplemented in subclass
{
}

/*!
  Virtual function; notifies subclasses that a popup menu item has been
  inserted.
*/

void QMenuData::menuInsPopup( QPopupMenu * )	// reimplemented in subclass
{
}

/*!
  Virtual function; notifies subclasses that a popup menu item has been
  removed.
*/

void QMenuData::menuDelPopup( QPopupMenu * )	// reimplemented in subclass
{
}


/*!
  Returns the number of items in the menu.
*/

uint QMenuData::count() const
{
    return mitems->count();
}


/*!
  Internal function that insert a menu item.  Called by all insertItem()
  functions.
*/

int QMenuData::insertAny( const char *text, const QPixmap *pixmap,
			  QPopupMenu *popup, int id, int index )
{
    if ( index > (int)mitems->count() ) {
#if defined(CHECK_RANGE)
	warning( "QMenuData::insertItem: Index %d out of range", index );
#endif
	return 0;
    }
    if ( index < 0 )				// append
	index = mitems->count();
    if ( id < -1 )				// -2, -3 etc.
	id = get_seq_id();
    if ( popup && popup->parentMenu )		// popup already in use
	return 0;
    register QMenuItem *mi = new QMenuItem;
    CHECK_PTR( mi );
    mi->ident = id == -1 ? index : id;
    if ( text == 0 && pixmap == 0 && popup == 0 ) {
	mi->is_separator = TRUE;		// separator
	mi->ident	 = -1;
    } else {
	mi->text_data = text;
	if ( pixmap )
	    mi->pixmap_data = new QPixmap( *pixmap );
	mi->popup_menu = popup;
	if ( popup ) {
	    popup->selfItem = mi;
	    menuInsPopup( popup );
	    if ( isPopupMenu ) {		// circular popups?
		QPopupMenu *p = (QPopupMenu*)this;
		while ( p && p != popup )
		    p = (QPopupMenu*)p->parentMenu;
		if ( p ) {
#if defined(CHECK_STATE)
		    warning( "QMenuData::insertItem: "
			     "Circular popup menu ignored");
#endif
		    return 0;
		}
	    }
	}
    }
    mitems->insert( index, mi );
    menuContentsChanged();			// menu data changed
    return mi->ident;
}

void QMenuData::removePopup( QPopupMenu *popup )
{
    int index = 0;
    register QMenuItem *mi = mitems->first();
    while ( mi ) {
	if ( mi->popup_menu == popup )		// found popup
	    break;
	index++;
	mi = mitems->next();
    }
    if ( mi )
	removeItemAt( index );
}


/*!
  Sets the dirty flag of all menu items to \e dirty.
*/

void QMenuData::setAllDirty( bool dirty )
{
    register QMenuItem *mi = mitems->first();
    while ( mi ) {
	mi->is_dirty = dirty;
	mi = mitems->next();
    }
}


/*!
  Inserts a menu item with a text and optional accelerator key, and
  connects it to an object/slot.

  Returns a unique menu item identifier (negative integer \<= -2).

  Example:
  \code
    QMenuBar   *mainMenu = new QMenuBar;
    QPopupMenu *fileMenu = new QPopupMenu;
    fileMenu->insert( "New",  myView, SLOT(newFile()), CTRL+Key_N );
    fileMenu->insert( "Open", myView, SLOT(open()),    CTRL+Key_O );
    mainMenu->insert( "File", fileMenu );
  \endcode

  In the example above, pressing CTRL+N or selecting "open" from the
  menu activates the myView->open() function.

  Note that accelerators only work for QPopupMenu items that live in a
  menu bar. For stand-alone popup menus, use an independent QAccel object.

  \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
  qkeycode.h
*/

int QMenuData::insertItem( const char *text,
			   const QObject *receiver, const char *member,
			   int accel )
{
    int id = insertAny( text, 0, 0, -2, -1 );
    connectItem( id, receiver, member );
    if ( accel )
	setAccel( accel, id );
    return id;
}

/*!
  Inserts a menu item with a pixmap and optional accelerator key, and
  connects it to an object/slot.

  Returns a unique menu item identifier (negative integer \<= -2).

  \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
  qkeycode.h
*/

int QMenuData::insertItem( const QPixmap &pixmap,
			   const QObject *receiver, const char *member,
			   int accel )
{
    int id = insertAny( 0, &pixmap, 0, -2, -1 );
    connectItem( id, receiver, member );
    if ( accel )
	setAccel( accel, id );
    return id;
}

/*!
  Inserts a menu item with a text.  Returns the menu item identifier.

  The menu item is assigned the identifier \e id or an automatically
  generated identifier.	 It works as follows: If \e id \>= 0, this
  identifier is assigned.  If \e id == -1 (default), the identifier is
  set equal to the menu item's real index (see below).	If \e id is
  any other negative integer, for instance -2, a unique identifier
  (negative integer \<= -2) is generated.

  The \e index specifies the position in the menu.  The menu item is
  appended at the end of the list if \e index is negative.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const char *text, int id, int index )
{
    return insertAny( text, 0, 0, id, index );
}

/*!
  Inserts a menu item with a text and a sub menu.
  Returns the menu item identifier.

  The menu item is assigned the identifier \e id or an automatically
  generated identifier.	 It works as follows: If \e id \>= 0, this
  identifier is assigned.  If \e id == -1 (default), the identifier is
  set equal to the menu item's real index (see below).	If \e id is
  any other negative integer, for instance -2, a unique identifier
  (negative integer \<= -2) is generated.

  The \e index specifies the position in the menu.  The menu item is
  appended at the end of the list if \e index is negative.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const char *text, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( text, 0, popup, id, index );
}

/*!
  Inserts a menu item with a pixmap.  Returns the menu item identifier.

  The menu item is assigned the identifier \e id or an automatically
  generated identifier.	 It works as follows: If \e id \>= 0, this
  identifier is assigned.  If \e id == -1 (default), the identifier is
  set equal to the menu item's real index (see below).	If \e id is
  any other negative integer, for instance -2, a unique identifier
  (negative integer \<= -2) is generated.

  The \e index specifies the position in the menu.  The menu item is
  appended at the end of the list if \e index is negative.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QPixmap &pixmap, int id, int index )
{
    return insertAny( 0, &pixmap, 0, id, index );
}

/*!
  Inserts a menu item with a pixmap and a sub menu.
  Returns the menu item identifier.

  The menu item is assigned the identifier \e id or an automatically
  generated identifier.	 It works as follows: If \e id \>= 0, this
  identifier is assigned.  If \e id == -1 (default), the identifier is
  set equal to the menu item's real index (see below).	If \e id is
  any other negative integer, for instance -2, a unique identifier
  (negative integer \<= -2) is generated.

  The \e index specifies the position in the menu.  The menu item is
  appended at the end of the list if \e index is negative.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QPixmap &pixmap, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( 0, &pixmap, popup, id, index );
}

/*!
  Inserts a separator at position \e index.
  The separator becomes the last menu item if \e index is negative.
*/

void QMenuData::insertSeparator( int index )
{
    insertAny( 0, 0, 0, 0, index );
}

/*!
  \fn void QMenuData::removeItem( int id )
  Removes the menu item which has the identifier \e id.
  \sa removeItemAt(), clear()
*/

/*!
  Removes the menu item at position \e index.
  \sa removeItem(), clear()
*/

void QMenuData::removeItemAt( int index )
{
    if ( index < 0 || index >= (int)mitems->count() ) {
#if defined(CHECK_RANGE)
	warning( "QMenuData::removeItem: Index %d out of range", index );
#endif
	return;
    }
    QMenuItem *mi = mitems->at( index );
    if ( mi->popup_menu ) {
	mi->popup_menu->selfItem = 0;
	menuDelPopup( mi->popup_menu );
    }
    mitems->remove();
    if ( !QApplication::closingDown() )		// avoid trouble
	menuContentsChanged();
}


/*!
  Removes all menu items.
  \sa removeItem(), removeItemAt()
*/

void QMenuData::clear()
{
    register QMenuItem *mi = mitems->first();
    while ( mi ) {
	if ( mi->popup_menu ) {
	    mi->popup_menu->selfItem = 0;
	    menuDelPopup( mi->popup_menu );
	}
	mitems->remove();
	mi = mitems->current();
    }
    if ( !QApplication::closingDown() )		// avoid trouble
	menuContentsChanged();
}


/*!
  Returns the accelerator key that has been defined for the menu item \e id,
  or 0 if it has no accelerator key.
  \sa setAccel(), QAccel, qkeycode.h
*/

int QMenuData::accel( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->key() : 0;
}

/*!
  Defines an accelerator key for the menu item \e id.

  An accelerator key consists of a key code and a combination of the modifiers
  \c SHIFT, \c CTRL and \c ALT (OR'ed or added).
  The header file qkeycode.h contains a list of key codes.

  Defining an accelerator key generates a text which is added to the
  menu item, for instance, \c CTRL + \c Key_O generates "Ctrl+O".  The
  text is formatted differently for different platforms.

  Note that accelerators only work for QPopupMenu items that live in a
  menu bar. For stand-alone popup menus, use an independent QAccel object.

  Example:
  \code
    QMenuBar   *mainMenu = new QMenuBar;
    QPopupMenu *fileMenu = new QPopupMenu;	// file sub menu
    fileMenu->insertItem( "Open Document", 67 );// add "Open" item
    fileMenu->setAccel( CTRL + Key_O, 67 );
    fileMenu->insertItem( "Quit", 69 );		// add "Quit" item
    fileMenu->setAccel( CTRL + ALT + Key_Delete, 69 );
    mainMenu->insertItem( "File", fileMenu );	// add the file menu
  \endcode

  You can also specify the accelerator in the insertItem() function.

  \sa accel(), insertItem(), QAccel, qkeycode.h
*/

void QMenuData::setAccel( int key, int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {
	mi->accel_key = key;
	menuContentsChanged();
    }
}


/*!
  Returns the text that has been set for menu item \e id, or 0 if no text
  has been set.
  \sa changeItem(), pixmap()
*/

const char *QMenuData::text( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->text() : 0;
}

/*!
  Returns the pixmap that has been set for menu item \e id, or 0 if no pixmap
  has been set.
  \sa changeItem(), text()
*/

QPixmap *QMenuData::pixmap( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->pixmap() : 0;
}

/*!
  Changes the text of the menu item \e id.
  \sa text()
*/

void QMenuData::changeItem( const char *text, int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {					// item found
	if ( mi->text_data == text )		// same string
	    return;
	if ( mi->pixmap_data ) {		// delete pixmap
	    delete mi->pixmap_data;
	    mi->pixmap_data = 0;
	}
	mi->text_data = text;
	menuContentsChanged();
    }
}

/*!
  Changes the pixmap of the menu item \e id.
  \sa pixmap()
*/

void QMenuData::changeItem( const QPixmap &pixmap, int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {					// item found
	if ( !mi->text_data.isNull() )		// delete text
	    mi->text_data.resize( 0 );
	register QPixmap *i = mi->pixmap_data;
	bool fast_refresh = i != 0 &&
	    i->width() == pixmap.width() &&
	    i->height() == pixmap.height();
	delete mi->pixmap_data;
	mi->pixmap_data = new QPixmap( pixmap );
	if ( fast_refresh )			// fast update
	    updateItem( id );
	else
	    menuContentsChanged();
    }
}


/*!
  Returns TRUE if the item with identifier \e id is enabled or FALSE if
  it is disabled.
  \sa setItemEnabled()
*/

bool QMenuData::isItemEnabled( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->isEnabled() : FALSE;
}

/*!
  Enables the menu item with identifier \e id if \e enable is TRUE, or
  disables the item if \e enable is FALSE.
  \sa isItemEnabled()
*/

void QMenuData::setItemEnabled( int id, bool enable )
{
    QMenuItem *mi = findItem( id );
    if ( mi && (bool)mi->is_enabled != enable ) {
	mi->is_enabled = enable;
	if ( mi->popup() )
	    mi->popup()->enableAccel( enable );
	menuStateChanged();
    }
}


/*!
  Returns TRUE if the menu item has been checked, otherwise FALSE.
  \sa setItemChecked()
*/

bool QMenuData::isItemChecked( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->isChecked() : FALSE;
}

/*!
  Checks a menu item if \e check is TRUE, or unchecks it if \e check is
  FALSE.
  \sa isItemChecked()
*/

void QMenuData::setItemChecked( int id, bool check )
{
    QMenuItem *mi = findItem( id );
    if ( mi && (bool)mi->is_checked != check ) {
	mi->is_checked = check;
	menuStateChanged();
    }
}


/*!
  Returns a pointer to the menu item with identifier \e id, or 0 if
  there is no item with such an identifier.
  \sa indexOf()
*/

QMenuItem *QMenuData::findItem( int id ) const
{
    if ( id == -1 )				// bad identifier
	return 0;
    QMenuItemListIt it( *mitems );
    QMenuItem *mi;
    while ( (mi=it.current()) ) {
	if ( mi->ident == id )			// this one?
	    return mi;
	if ( mi->popup_menu ) {			// recursive search
	    mi = mi->popup_menu->findItem( id );
	    if ( mi )
		return mi;
	}
	++it;
    }
    return 0;					// not found
}

/*!
  Returns the index of the menu item with identifier \e id, or -1 if
  there is no item with such an identifier.
  \sa idAt(), findItem()
*/

int QMenuData::indexOf( int id ) const
{
    if ( id == -1 )				// bad identifier
	return -1;
    QMenuItemListIt it( *mitems );
    QMenuItem *mi;
    int index = 0;
    while ( (mi=it.current()) ) {
	if ( mi->ident == id )			// this one?
	    return index;
	++index;
	++it;
    }
    return -1;					// not found
}

/*!
  Returns the identifier of the menu item at position \e index in the internal
  list, or -1 if \e index is out of range.
  \sa setId(), indexOf()
*/

int QMenuData::idAt( int index ) const
{
    return index < (int)mitems->count() ?
	   mitems->at(index)->id() : -1;
}

/*!
  Sets the menu identifier of the item at \e index to \e id.

  If index is out of range the operation is ignored.

  \sa idAt()
*/

void QMenuData::setId( int index, int id )
{
    if ( index < (int)mitems->count() )
	mitems->at(index)->ident = id;
}


/*!
  Connects a menu item to a receiver and a slot or signal.

  The receiver's slot/signal is activated when the menu item is activated.
*/

bool QMenuData::connectItem( int id, const QObject *receiver,
			     const char *member )
{
    QMenuItem *mi = findItem( id );
    if ( !mi )					// no such identifier
	return FALSE;
    if ( !mi->signal_data ) {			// create new signal
	mi->signal_data = new QSignal;
	CHECK_PTR( mi->signal_data );
    }
    return mi->signal_data->connect( receiver, member );
}

/*!
  Disconnects a receiver/member from a menu item.

  All connections are removed when the menu data object is destroyed.
*/

bool QMenuData::disconnectItem( int id, const QObject *receiver,
				const char *member )
{
    QMenuItem *mi = findItem( id );
    if ( !mi || !mi->signal_data )		// no identifier or no signal
	return FALSE;
    return mi->signal_data->disconnect( receiver, member );
}
