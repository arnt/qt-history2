/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenudata.cpp#19 $
**
** Implementation of QMenuData class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define  INCLUDE_MENUITEM_DEF
#include "qmenudta.h"
#include "qpopmenu.h"
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qmenudata.cpp#19 $";
#endif


/*!
\class QMenuData qmenudta.h

The QMenuData class is a base class for QMenuBar and QPopupMenu.
QMenuData organizes a list of menu items.
*/

// ---------------------------------------------------------------------------
// QMenuItem member functions
//

QMenuItem::QMenuItem()				// initialize menu item
{
    ident        = -1;
    is_separator = is_disabled = is_checked = FALSE;
    pixmap_data  = 0;
    popup_menu   = 0;
    accel_key    = 0;
    signal_data  = 0;
}

QMenuItem::~QMenuItem()
{
    delete pixmap_data;
    delete signal_data;
}


// ---------------------------------------------------------------------------
// QMenuData member functions
//

QMenuData::QMenuData()
{
    actItem = -1;				// no active menu item
    mitems = new QMenuItemList;			// create list of menu items
    CHECK_PTR( mitems );
    mitems->setAutoDelete( TRUE );
    parentMenu = 0;				// assume top level
    isPopup = isMenuBar = mouseBtDn = FALSE;
    badSize = TRUE;
}

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


void QMenuData::updateItem( int )		// reimplemented in subclass
{
}

void QMenuData::menuContentsChanged()		// reimplemented in subclass
{
}

void QMenuData::menuStateChanged()		// reimplemented in subclass
{
}

void QMenuData::menuInsPopup( QPopupMenu * )	// reimplemented in subclass
{
}

void QMenuData::menuDelPopup( QPopupMenu * )	// reimplemented in subclass
{
}


void QMenuData::insertAny( const char *string, const QPixmap *pixmap,
			   QPopupMenu *popup, int id, int index )
{						// insert pixmap + sub menu
    if ( index > (int)mitems->count() ) {
#if defined(CHECK_RANGE)
	warning( "QMenuData::insertItem: Index %d out of range", index );
#endif
	return;
    }
    if ( index < 0 )				// append
	index = mitems->count();
    if ( popup && popup->parentMenu )		// popup already in use
	return;
    register QMenuItem *mi = new QMenuItem;
    CHECK_PTR( mi );
    mi->ident = id == -1 ? index : id;
    if ( mi->ident == -2 )
	mi->ident = -1;
    if ( string == 0 && pixmap == 0 && popup == 0 )
	mi->is_separator = TRUE;		// separator
    else {
	mi->string_data = string;
	if ( pixmap )
	    mi->pixmap_data = new QPixmap( *pixmap );
	mi->popup_menu = popup;
	if ( popup ) {
	    menuInsPopup( popup );
	    QPopupMenu *p = (QPopupMenu*)this;
	    while ( p && p != popup )
		p = (QPopupMenu*)p->parentMenu;
	    if ( p ) {
#if defined(CHECK_STATE)
		warning( "QMenuData::insertItem: Circular popup menu ignored");
#endif
		delete mi;
		return;
	    }
	}
    }
    mitems->insert( index, mi );
    menuContentsChanged();			// menu data changed
}

void QMenuData::removePopup( QPopupMenu *popup )
{						// remove sub menu
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

void QMenuData::insertItem( const char *string, int id, int index )
{						// insert string item
    insertAny( string, 0, 0, id, index );
}

void QMenuData::insertItem( const char *string, QPopupMenu *popup,
			    int id, int index )
{						// insert string + popup menu
    insertAny( string, 0, popup, id, index );
}

void QMenuData::insertItem( const QPixmap &pixmap, int id, int index )
{						// insert pixmap item
    insertAny( 0, &pixmap, 0, id, index );
}

void QMenuData::insertItem( const QPixmap &pixmap, QPopupMenu *popup,
			    int id, int index )
{						// insert bitmap + popup menu
    insertAny( 0, &pixmap, popup, id, index );
}

/*!
Inserts a separator at position \e index.  The separator will become the last
menu item if \e index is negative.
*/
void QMenuData::insertSeparator( int index )	// insert menu separator
{
    insertAny( 0, 0, 0, -2, index );
}

/*!
\fn void QMenuData::removeItem( int id )
Removes the menu item which has the identifier \e id.
*/

/*!
Removes the menu item at position \e index.
*/
void QMenuData::removeItemAt( int index )	// remove menu item
{
    if ( index < 0 || index >= mitems->count() ) {
#if defined(CHECK_RANGE)
	warning( "QMenuData::removeItem: Index %d out of range" );
#endif
	return;
    }
    QMenuItem *mi = mitems->at( index );
    if ( mi->popup_menu )
	menuDelPopup( mi->popup_menu );
    mitems->remove();
    if ( !QApplication::closingDown() )		// avoid trouble
	menuContentsChanged();
}


long QMenuData::accel( int id ) const	// get accelerator key
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->key() : 0;
}

void QMenuData::setAccel( long key, int id )	// set accelerator key
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {
	mi->accel_key = key;
	menuContentsChanged();
    }
}


const char *QMenuData::string( int id ) const	// get string
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->string() : 0;
}

QPixmap *QMenuData::pixmap( int id ) const	// get pixmap
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->pixmap() : 0;
}

void QMenuData::changeItem( const char *string, int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {					// item found
	if ( mi->string_data == string )	// same string
	    return;
	if ( mi->pixmap ) {			// delete pixmap
	    delete mi->pixmap_data;
	    mi->pixmap_data = 0;
	}
	mi->string_data = string;
	menuContentsChanged();
    }
}

void QMenuData::changeItem( const QPixmap &pixmap, int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {					// item found
	if ( !mi->string_data.isNull() )	// delete string
	    mi->string_data.resize( 0 );
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


bool QMenuData::isItemDisabled( int id ) const
{						// is menu item disabled?
    QMenuItem *mi = findItem( id );
    return mi ? mi->isDisabled() : FALSE;
}

void QMenuData::setItemEnabled( int id, bool enable )
{						// enable/disable item
    QMenuItem *mi = findItem( id );
    bool disable = !enable;
    if ( mi && mi->is_disabled != disable ) {
	mi->is_disabled = disable;
	if ( mi->popup() )
	    mi->popup()->enableAccel( enable );
	menuStateChanged();
    }
}


bool QMenuData::isItemChecked( int id ) const
{						// is menu item checked?
    QMenuItem *mi = findItem( id );
    return mi ? mi->isChecked() : FALSE;
}

void QMenuData::setItemChecked( int id, bool check )
{						// enable/disable item
    QMenuItem *mi = findItem( id );
    if ( mi && mi->is_checked != check ) {
	mi->is_checked = check;
	menuStateChanged();
    }
}


QMenuItem *QMenuData::findItem( int id ) const	// find menu item, ident==id
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

int QMenuData::indexOf( int id ) const		// get index of item, ident==id
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

int QMenuData::idAt( int index ) const		// get menu identifier at index
{
    return ((uint)index < mitems->count()) ?
	   mitems->at(index)->id() : -1;
}

  /*!
  Sets the menu identifier of the item at \e index. If index is out of range
  the operation is ignored. 
  */

void QMenuData::setId( int index, int id )	// set menu identifier at index
{
    if ((uint)index < mitems->count())
        mitems->at(index)->ident = id;
}


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

bool QMenuData::disconnectItem( int id, const QObject *receiver,
				const char *member )
{
    QMenuItem *mi = findItem( id );
    if ( !mi || !mi->signal_data )		// no identifier or no signal
	return FALSE;
    return mi->signal_data->disconnect( receiver, member );
}

  /*!
  Returns the number of items in the menu.
  */
int QMenuData::count() const
{ 
    return mitems->count(); 
}


  /*!
  Removes all menu items.
  */
void QMenuData::clear()				// remove all menu items
{
    register QMenuItem *mi = mitems->first();
    while ( mi ) {
	if ( mi->popup_menu )
            menuDelPopup( mi->popup_menu );
        mitems->remove();
	mi = mitems->current();
    }
    if ( !QApplication::closingDown() )		// avoid trouble
	menuContentsChanged();

}



