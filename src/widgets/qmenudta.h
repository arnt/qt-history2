/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenudta.h#4 $
**
** Definition of QMenuData class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMENUDTA_H
#define QMENUDTA_H

#include "qglobal.h"


class QPopupMenu;

#if defined(INCLUDE_MENUITEM_DEF)

#include "qstring.h"
#include "qbitmap.h"
#include "qsignal.h"

class QMenuItem					// internal menu item class
{
friend class QMenuData;
public:
    QMenuItem();
   ~QMenuItem();

    int		id()	      const { return ident; }
    const char *string()      const { return string_data; }
    QBitMap    *bitmap()      const { return bitmap_data; }
    QPopupMenu *popup()	      const { return popup_menu; }
    QSignal    *signal()      const { return signal_data; }
    bool	isSeparator() const { return is_separator; }
    bool	isDisabled()  const { return is_disabled; }
    bool	isChecked()   const { return is_checked; }

private:
    int		ident;				// item identifier
    QString	string_data;			// item text
    QBitMap    *bitmap_data;			// item bitmap
    QPopupMenu *popup_menu;			// item popup menu
    QSignal    *signal_data;			// connection
    uint	is_separator : 1;		// separator flag
    uint	is_disabled  : 1;		// disabled flag
    uint	is_checked   : 1;		// checked flag
};

#include "qlist.h"
typedef declare(QListM,QMenuItem)	  QMenuItemList;
typedef declare(QListIteratorM,QMenuItem) QMenuItemListIt;

#else

class QMenuItem;
class QMenuItemList;

#endif


class QMenuData					// menu data class
{
friend class QMenuBar;
friend class QPopupMenu;
public:
    QMenuData();
   ~QMenuData();

    void	insertItem( const char *string, int id=-1, int index=-1 );
    void	insertItem( const char *string, QPopupMenu *popup,
			    int index=-1 );

    void	insertItem( QBitMap *bitmap, int id=-1, int index=-1 );
    void	insertItem( QBitMap *bitmap, QPopupMenu *popup,
			    int index=-1 );

    void	insertSeparator( int index=-1 );

    void	removeItem( int index );

    const char *string( int id ) const;		// get string of item id
    QBitMap    *bitmap( int id ) const;		// get bitmap of item id
    void	changeItem( const char *string, int id );
    void	changeItem( QBitMap *bitmap, int id );

    bool	isItemDisabled( int id ) const;
    bool	isItemEnabled( int id )	 const	{ return !isItemDisabled(id); }
    void	setItemEnabled( int id, bool enable );
    void	enableItem( int id )		{ setItemEnabled( id, TRUE ); }
    void	disableItem( int id )		{ setItemEnabled( id, FALSE );}

    bool	isItemChecked( int id ) const;
    void	setItemChecked( int id, bool check );
    void	checkItem( int id )		{ setItemChecked( id, TRUE ); }
    void	uncheckItem( int id )		{ setItemChecked( id, FALSE );}

    int		indexOf( int id ) const;	// get index of specified item
    int		idAt( int index ) const;	// get id of item at index

    bool	connectItem( int id,		// connect item to method
			     const QObject *receiver, const char *member );
    bool	disconnectItem( int id,
				const QObject *receiver, const char *member );

protected:
    int		   actItem;			// active menu item
    QMenuItemList *mitems;			// list of menu items
    QMenuData     *parentMenu;
    uint	   isPopup	: 1;
    uint	   isMenuBar	: 1;
    uint	   badSize	: 1;
    QMenuItem     *findItem( int id ) const;
    virtual void   menuContentsChanged();
    virtual void   menuStateChanged();
    virtual void   menuInsPopup( QPopupMenu * );
    virtual void   menuDelPopup( QPopupMenu * );

private:
    void	insertAny( const char *, QBitMap *, QPopupMenu *, int, int );
};


#endif // QMENUDTA_H
