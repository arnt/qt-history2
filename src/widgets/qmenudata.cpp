/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenudata.cpp#81 $
**
** Implementation of QMenuData class
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
#include "qmenudata.h"
#include "qpopupmenu.h"
#include "qapplication.h"


// Not used yet...
class QMenuItemData { };
class QMenuDataData { };

/*!
  \class QMenuData qmenudata.h
  \brief The QMenuData class is a base class for QMenuBar and QPopupMenu.

  \ingroup misc

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
    iconset_data	 = 0;
    pixmap_data	 = 0;
    popup_menu	 = 0;
    accel_key	 = 0;
    signal_data	 = 0;
    d = 0; // FOR EXTENSION (eg. non-ascii accels)
}

QMenuItem::~QMenuItem()
{
    delete iconset_data;
    delete pixmap_data;
    delete signal_data;
    delete d;
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
    avoid_circularity = 0;
    d = 0; // FOR EXTENSION
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
    delete d;
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
  Internal function that insert a menu item.  Called by all insert()
  functions.
*/

int QMenuData::insertAny( const QString *text, const QPixmap *pixmap,
			  QPopupMenu *popup, const QIconSet* iconset, int id, int index )
{
    if ( index > (int)mitems->count() ) {
#if defined(CHECK_RANGE)
	qWarning( "QMenuData::insertItem: Index %d out of range", index );
#endif
	return 0;
    }
    if ( index < 0 )				// append
	index = mitems->count();
    if ( id < 0 )				// -2, -3 etc.
	id = get_seq_id();

    register QMenuItem *mi = new QMenuItem;
    CHECK_PTR( mi );
    mi->ident = id;
    if ( text == 0 && pixmap == 0 && popup == 0 ) {
	mi->is_separator = TRUE;		// separator
	mi->ident	 = -1;
    } else {
	mi->text_data = text?*text:QString::null;
	if ( pixmap )
	    mi->pixmap_data = new QPixmap( *pixmap );
	mi->popup_menu = popup;
	if ( popup )
	    popup->selfItem = mi;
	if ( iconset )
	    mi->iconset_data = new QIconSet( *iconset );
    }

    mitems->insert( index, mi );
    menuContentsChanged();			// menu data changed
    return mi->ident;
}

/*!
  Internal function that finds the menu item where \a popup is located,
  storing its index at \a index if \a index is not NULL.
*/
QMenuItem *QMenuData::findPopup( QPopupMenu *popup, int *index )
{
    int i = 0;
    QMenuItem *mi = mitems->first();
    while ( mi ) {
	if ( mi->popup_menu == popup )		// found popup
	    break;
	i++;
	mi = mitems->next();
    }
    if ( index && mi )
	*index = i;
    return mi;
}

void QMenuData::removePopup( QPopupMenu *popup )
{
    int index = 0;
    QMenuItem *mi = findPopup( popup, &index );
    if ( mi )
	removeItemAt( index );
}


/*!
  Sets the dirty flag of all menu items to \a dirty.
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
  Inserts a menu item with a text, an accelerator key, an id and an
  optional index and connects it to an object/slot.

  Example:
  \code
    QMenuBar   *mainMenu = new QMenuBar;
    QPopupMenu *fileMenu = new QPopupMenu;
    fileMenu->insertItem( "New",  myView, SLOT(newFile()), CTRL+Key_N );
    fileMenu->insertItem( "Open", myView, SLOT(open()),    CTRL+Key_O );
    mainMenu->insertItem( "File", fileMenu );
  \endcode

  In the example above, pressing CTRL+N or selecting "open" from the
  menu activates the myView->open() function.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  Note that accelerators only work for QPopupMenu items that live in a
  menu bar. For stand-alone popup menus, use an independent QAccel
  object.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
  qnamespace.h
*/

int QMenuData::insertItem( const QString &text,
			   const QObject *receiver, const char* member,
			   int accel, int id, int index )
{
    int actualID = insertAny( &text, 0, 0, 0, id, index );
    connectItem( actualID, receiver, member );
    if ( accel )
	setAccel( accel, actualID );
    return actualID;
}

/*!
  Inserts a menu item with an icon, a text, an accelerator key, an id
  and an optional index and connects it to an object/slot. The icon
  will be displayed to the left of the text in the item.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  Note that accelerators only work for QPopupMenu items that live in a
  menu bar. For stand-alone popup menus, use an independent QAccel
  object.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
  qnamespace.h
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QString &text,
			   const QObject *receiver, const char* member,
			   int accel, int id, int index )
{
    int actualID = insertAny( &text, 0, 0, &icon, id, index );
    connectItem( actualID, receiver, member );
    if ( accel )
	setAccel( accel, actualID );
    return actualID;
}

/*!
  Inserts a menu item with a pixmap, an accelerator key, an id and an
  optional index and connects it to an object/slot.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  Note that accelerators only work for QPopupMenu items that live in a
  menu bar. For stand-alone popup menus, use an independent QAccel
  object.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
  qnamespace.h
*/

int QMenuData::insertItem( const QPixmap &pixmap,
			   const QObject *receiver, const char* member,
			   int accel, int id, int index )
{
    int actualID = insertAny( 0, &pixmap, 0, 0, id, index );
    connectItem( actualID, receiver, member );
    if ( accel )
	setAccel( accel, actualID );
    return actualID;
}


/*!
  Inserts a menu item with an icon, a pixmap, an accelerator key, an id
  and an optional index and connects it to an object/slot. The icon
  will be displayed to the left of the pixmap in the item.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  Note that accelerators only work for QPopupMenu items that live in a
  menu bar. For stand-alone popup menus, use an independent QAccel
  object.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
  qnamespace.h
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QPixmap &pixmap,
			   const QObject *receiver, const char* member,
			   int accel, int id, int index )
{
    int actualID = insertAny( 0, &pixmap, 0, &icon, id, index );
    connectItem( actualID, receiver, member );
    if ( accel )
	setAccel( accel, actualID );
    return actualID;
}


/*!
  Inserts a menu item with a text.  Returns the menu item identifier.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QString &text, int id, int index )
{
    return insertAny( &text, 0, 0, 0, id, index );
}

/*!
  Inserts a menu item with an icon and a text.  The icon will be
  displayed to the left of the text in the item. Returns the menu
  item identifier.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QString &text, int id, int index )
{
    return insertAny( &text, 0, 0, &icon, id, index );
}

/*!
  Inserts a menu item with a text and a sub menu.
  Returns the menu item identifier.

  The \a popup must be deleted by the programmer or by its parent
  widget.  It is not deleted when this menu item is removed or when
  the menu is deleted.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QString &text, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( &text, 0, popup, 0, id, index );
}

/*!
  Inserts a menu item with an icon, a text and a sub menu. The icon
  will be displayed to the left of the text in the item. Returns the
  menu item identifier.

  The \a popup must be deleted by the programmer or by its parent
  widget.  It is not deleted when this menu item is removed or when
  the menu is deleted.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QString &text, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( &text, 0, popup, &icon, id, index );
}

/*!
  Inserts a menu item with a pixmap.  Returns the menu item identifier.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QPixmap &pixmap, int id, int index )
{
    return insertAny( 0, &pixmap, 0, 0, id, index );
}

/*!
  Inserts a menu item with an icon and a pixmap.  The icon will be
  displayed to the left of the pixmap in the item. Returns the menu
  item identifier.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QPixmap &pixmap, int id, int index )
{
    return insertAny( 0, &pixmap, 0, &icon, id, index );
}


/*!
  Inserts a menu item with a pixmap and a sub menu. The icon
  will be displayed to the left of the pixmap in the item.  Returns
  the menu item identifier.

  The \a popup must be deleted by the programmer or by its parent
  widget.  It is not deleted when this menu item is removed or when
  the menu is deleted.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QPixmap &pixmap, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( 0, &pixmap, popup, 0, id, index );
}


/*!
  Inserts a menu item with an icon, a pixmap and a sub menu. The icon
  will be displayed to the left of the pixmap in the item.  Returns
  the menu item identifier.

  The \a popup must be deleted by the programmer or by its parent
  widget.  It is not deleted when this menu item is removed or when
  the menu is deleted.

  The menu item is assigned the identifier \a id or an automatically
  generated identifier if \a id is < 0. The generated identifiers
  (negative integers) are guaranteed to be unique within the entire
  application.

  The \a index specifies the position in the menu.  The menu item is
  appended at the end of the list if \a index is negative.

  \warning Be careful when passing a literal 0 to insertItem(), as
	some C++ compilers choose the wrong overloaded function.
	Cast the 0 to what you mean, eg. <tt>(QObject*)0</tt>.

  \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QPixmap &pixmap, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( 0, &pixmap, popup, &icon, id, index );
}



/*!
  Inserts a separator at position \a index.
  The separator becomes the last menu item if \a index is negative.

  In a popup menu, a separator is rendered as a horizontal line.  In a
  Motif menubar, a separator is spacing, so the rest of the items
  (just "Help", normally) are drawn right-justified.  In a Windows
  menubar, all separator are ignored (to comply with the Windows style
  guide).
*/

void QMenuData::insertSeparator( int index )
{
    insertAny( 0, 0, 0, 0, 0, index );
}

/*!
  \fn void QMenuData::removeItem( int id )
  Removes the menu item which has the identifier \a id.
  \sa removeItemAt(), clear()
*/

/*!
  Removes the menu item at position \a index.
  \sa removeItem(), clear()
*/

void QMenuData::removeItemAt( int index )
{
    if ( index < 0 || index >= (int)mitems->count() ) {
#if defined(CHECK_RANGE)
	qWarning( "QMenuData::removeItem: Index %d out of range", index );
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
  Returns the accelerator key that has been defined for the menu item \a id,
  or 0 if it has no accelerator key.
  \sa setAccel(), QAccel, qnamespace.h
*/

int QMenuData::accel( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->key() : 0;
}

/*!
  Defines an accelerator key for the menu item \a id.

  An accelerator key consists of a key code and a combination of the modifiers
  \c SHIFT, \c CTRL and \c ALT (OR'ed or added).
  The header file qnamespace.h contains a list of key codes.

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

  \sa setAccel(), accel(), insertItem(), QAccel, qnamespace.h
*/

void QMenuData::setAccel( int key, int id )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {
	mi->accel_key = key;
	parent->menuContentsChanged();
    }
}

/*!
  Returns the icon set that has been set for menu item \a id, or 0 if no icon
  set has been set.
  \sa changeItem(), text(), pixmap()
*/

QIconSet* QMenuData::iconSet( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->iconSet() : 0;
}

/*!
  Returns the text that has been set for menu item \a id, or a
  \link QString::operator!() null string\endlink
  if no text has been set.
  \sa changeItem(), pixmap(), iconSet()
*/

QString QMenuData::text( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->text() : QString::null;
}

/*!
  Returns the pixmap that has been set for menu item \a id, or 0 if no pixmap
  has been set.
  \sa changeItem(), text(), iconSet()
*/

QPixmap *QMenuData::pixmap( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->pixmap() : 0;
}

/*!\obsolete
  Changes the text of the menu item \a id. If the item has an icon,
  the icon remains unchanged.
  \sa text()
*/

void QMenuData::changeItem( const QString &text, int id )
{
    changeItem( id, text);
}

/*!\obsolete
  Changes the pixmap of the menu item \a id. If the item has an icon,
  the icon remains unchanged.
  \sa pixmap()
*/

void QMenuData::changeItem( const QPixmap &pixmap, int id )
{
    changeItem( id, pixmap );
}

/*!\obsolete
  Changes the icon and text of the menu item \a id.
  \sa pixmap()
*/

void QMenuData::changeItem( const QIconSet &icon, const QString &text, int id )
{
    changeItem( id, icon, text );
}


/*!
  Changes the text of the menu item \a id. If the item has an icon,
  the icon remains unchanged.
  \sa text()
*/

void QMenuData::changeItem( int id, const QString &text )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {					// item found
	if ( mi->text_data == text )		// same string
	    return;
	if ( mi->pixmap_data ) {		// delete pixmap
	    delete mi->pixmap_data;
	    mi->pixmap_data = 0;
	}
	mi->text_data = text;
	parent->menuContentsChanged();
    }
}

/*!
  Changes the pixmap of the menu item \a id. If the item has an icon,
  the icon remains unchanged.
  \sa pixmap()
*/

void QMenuData::changeItem( int id, const QPixmap &pixmap )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {					// item found
	register QPixmap *i = mi->pixmap_data;
	bool fast_refresh = i != 0 &&
	    i->width() == pixmap.width() &&
	    i->height() == pixmap.height() &&
	    !mi->text();
	if ( !mi->text_data.isNull() )		// delete text
	    mi->text_data = QString::null;
	mi->pixmap_data = new QPixmap( pixmap );
	delete i; // old mi->pixmap_data, could be &pixmap
	if ( fast_refresh )
	    parent->updateItem( id );
	else
	    parent->menuContentsChanged();
    }
}

/*!
  Changes the icon and text of the menu item \a id.
  \sa pixmap()
*/

void QMenuData::changeItem( int id, const QIconSet &icon, const QString &text )
{
    changeItem(id, text);
    changeItemIconSet(id,  icon);
}

/*!
  Changes the icon and pixmap of the menu item \a id.
  \sa pixmap()
*/

void QMenuData::changeItem( int id, const QIconSet &icon, const QPixmap &pixmap )
{
    changeItem(id, pixmap);
    changeItemIconSet(id,  icon);
}



/*!
  Changes the icon of the menu item \a id.
  \sa pixmap()
*/

void QMenuData::changeItemIconSet( int id, const QIconSet &icon )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {					// item found
	register QIconSet *i = mi->iconset_data;
	bool fast_refresh = i != 0;
	mi->iconset_data = new QIconSet( icon );
	delete i; // old mi->iconset_data, could be &icon
	if ( fast_refresh )
	    parent->updateItem( id );
	else
	    parent->menuContentsChanged();
    }
}


/*!
  Returns TRUE if the item with identifier \a id is enabled or FALSE if
  it is disabled.
  \sa setItemEnabled()
*/

bool QMenuData::isItemEnabled( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->isEnabled() : FALSE;
}

/*!
  Enables the menu item with identifier \a id if \a enable is TRUE, or
  disables the item if \a enable is FALSE.
  \sa isItemEnabled()
*/

void QMenuData::setItemEnabled( int id, bool enable )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi && (bool)mi->is_enabled != enable ) {
	mi->is_enabled = enable;
	if ( mi->popup() )
	    mi->popup()->enableAccel( enable );
	parent->menuStateChanged();
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
  Checks the menu item with id \a id if \a check is TRUE, or unchecks
  it if \a check is FALSE, and calls setCheckable( TRUE ) if
  necessary.

  \sa isItemChecked()
*/

void QMenuData::setItemChecked( int id, bool check )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi && (bool)mi->is_checked != check ) {
	mi->is_checked = check;
	if ( parent->isPopupMenu && !((QPopupMenu *)parent)->isCheckable() )
	    ((QPopupMenu *)parent)->setCheckable( TRUE );
	parent->menuStateChanged();
    }
}


/*!
  Returns a pointer to the menu item with identifier \a id, or 0 if
  there is no item with such an identifier.
  \sa indexOf()
*/

QMenuItem *QMenuData::findItem( int id ) const
{
    return findItem( id, 0 );
}


/*!
  Returns a pointer to the menu item with identifier \a id, or 0 if
  there is no item with such an identifier, and changes \a parent to
  point to the parent of the return value.

  \sa indexOf()
*/

QMenuItem * QMenuData::findItem( int id, QMenuData ** parent ) const
{
    if ( parent )
	*parent = (QMenuData *)this;		// ###

    if ( id == -1 )				// bad identifier
	return 0;
    QMenuItemListIt it( *mitems );
    QMenuItem *mi;
    while ( (mi=it.current()) ) {		// search this menu
	++it;
	if ( mi->ident == id )			// found item
	    return mi;
    }
    it.toFirst();
    while ( (mi=it.current()) ) {		// search submenus
	++it;
	if ( mi->popup_menu ) {
	    mi = mi->popup_menu->findItem( id, parent );
	    if ( mi )				// found item
		return mi;
	}
    }
    return 0;					// not found
}

/*!
  Returns the index of the menu item with identifier \a id, or -1 if
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
  Returns the identifier of the menu item at position \a index in the internal
  list, or -1 if \a index is out of range.
  \sa setId(), indexOf()
*/

int QMenuData::idAt( int index ) const
{
    return index < (int)mitems->count() ?
	   mitems->at(index)->id() : -1;
}

/*!
  Sets the menu identifier of the item at \a index to \a id.

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
			     const char* member )
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
				const char* member )
{
    QMenuItem *mi = findItem( id );
    if ( !mi || !mi->signal_data )		// no identifier or no signal
	return FALSE;
    return mi->signal_data->disconnect( receiver, member );
}

/*!
  Sets a Whats This help for a certain menu item.

  \arg \e id is the menu item id.
  \arg \e text is the Whats This help text in QML format

  \sa whatsThis()
 */
void QMenuData::setWhatsThis( int id, const QString& text )
{

    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {
	mi->setWhatsThis( text );
	parent->menuContentsChanged();
    }
}

/*!
  Returns the Whats This help text for the specified item \e id or
  QString::null if no text has been defined yet.

  \sa setWhatsThis()
 */
QString QMenuData::whatsThis( int id ) const
{

    QMenuItem *mi = findItem( id );
    return mi? mi->whatsThis() : QString::null;
}
