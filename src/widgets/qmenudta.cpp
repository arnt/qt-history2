/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenudta.cpp#2 $
**
** Implementation of QMenuData class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define  INCLUDE_MENUITEM_DEF
#include "qmenudta.h"
#include "qpopmenu.h"


// ---------------------------------------------------------------------------
// QMenuItem member functions
//

QMenuItem::QMenuItem()				// initialize menu item
{
    ident = -1;
    is_separator = is_disabled = is_checked = FALSE;
    bitmap_data = 0;
    submenu = 0;
    signal_data = 0;
}

QMenuItem::~QMenuItem()
{
    delete bitmap_data;
    delete submenu;
    delete signal_data;
}


// ---------------------------------------------------------------------------
// QMenuData member functions
//

QMenuData::QMenuData()
{
    mitems = new QMenuItemList;			// create list of menu items
    CHECK_PTR( mitems );
    mitems->setAutoDelete( TRUE );
}

QMenuData::~QMenuData()
{
    delete mitems;				// delete menu item list
}


void QMenuData::menuContentsChanged()		// reimplemented in subclass
{
}

void QMenuData::menuStateChanged()		// reimplemented in subclass
{
}

void QMenuData::menuInitSubMenu( QPopupMenu * )	// reimplemented in subclass
{
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
	++it;
    }
    return 0;					// not found
}


void QMenuData::insertAny( const char *string, QBitMap *bitmap,
			   QPopupMenu *sub, int id, int index )
{						// insert bitmap + sub menu
    if ( index > (int)mitems->count() ) {
#if defined(CHECK_RANGE)
	warning( "QMenuData::insertItem: Index %d out of range", index );
#endif
	return;
    }
    if ( index < 0 )
	index = mitems->count();
    QMenuItem *mi = new QMenuItem;
    CHECK_PTR( mi );
    mi->ident = id != -1 ? id : index;
    if ( string == 0 && bitmap == 0 && sub == 0 )	// separator
	mi->is_separator = TRUE;
    else {
	mi->string_data = string;
	mi->bitmap_data = bitmap;
	mi->submenu = sub;
	if ( sub )
	    menuInitSubMenu( sub );
    }
    mitems->insert( index, mi );
    menuContentsChanged();			// menu data changed
}

void QMenuData::insertItem( const char *string, int id, int index )
{						// insert text item
    insertAny( string, 0, 0, id, index );
}

void QMenuData::insertItem( const char *string, QPopupMenu *sub, int index )
{						// insert text + sub menu
    insertAny( string, 0, sub, -1, index );
}

void QMenuData::insertItem( QBitMap *bitmap, int id, int index )
{						// insert bitmap item
    insertAny( 0, bitmap, 0, id, index );
}

void QMenuData::insertItem( QBitMap *bitmap, QPopupMenu *sub, int index )
{						// insert bitmap + sub menu
    insertAny( 0, bitmap, sub, -1, index );
}

void QMenuData::insertSeparator( int index )	// insert menu separator
{
    insertAny( 0, 0, 0, -1, index );
}


void QMenuData::removeItem( int index )		// insert menu separator
{
    if ( index < 0 || index >= mitems->count() ) {
#if defined(CHECK_RANGE)
	warning( "QMenuData::removeItem: Index %d out of range" );
#endif
	return;
    }
    mitems->remove( index );
    menuContentsChanged();
}


const char *QMenuData::string( int id ) const	// get string at index
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->string() : 0;
}

QBitMap *QMenuData::bitmap( int id ) const	// get bitmap at index
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->bitmap() : 0;
}

void QMenuData::changeItem( const char *string, int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {					// item found
	if ( mi->string_data == string )	// same string
	    return;
	if ( mi->bitmap ) {			// delete bitmap
	    delete mi->bitmap_data;
	    mi->bitmap_data = 0;
	}
	mi->string_data = string;
	menuContentsChanged();
    }
}

void QMenuData::changeItem( QBitMap *bitmap, int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi ) {					// item found
	if ( !mi->string_data.isNull() )	// delete string
	    mi->string_data.resize( 0 );
	delete mi->bitmap_data;
	mi->bitmap_data = bitmap;
	menuContentsChanged();
    }
}


bool QMenuData::isItemDisabled( int id ) const
{						// is menu item disabled?
    QMenuItem *mi = findItem(id);
    return mi ? mi->isDisabled() : FALSE;
}

void QMenuData::setItemEnabled( int id, bool enable )
{						// enable/disable item
    QMenuItem *mi = findItem(id);
    bool disable = !enable;
    if ( mi && mi->is_disabled != disable ) {
	mi->is_disabled = disable;
	menuStateChanged();
    }
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
