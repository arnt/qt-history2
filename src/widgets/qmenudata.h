/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenudata.h#32 $
**
** Definition of QMenuData class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMENUDTA_H
#define QMENUDTA_H

#include "qglobal.h"


class QPopupMenu;

#if defined(INCLUDE_MENUITEM_DEF)

#include "qstring.h"
#include "qpixmap.h"
#include "qsignal.h"

class QMenuItem					// internal menu item class
{
friend class QMenuData;
public:
    QMenuItem();
   ~QMenuItem();

    int		id()		const	{ return ident; }
    const char *text()		const	{ return text_data; }
    QPixmap    *pixmap()	const	{ return pixmap_data; }
    QPopupMenu *popup()		const	{ return popup_menu; }
    int		key()		const	{ return accel_key; }
    QSignal    *signal()	const	{ return signal_data; }
    bool	isSeparator()	const	{ return is_separator; }
    bool	isEnabled()	const	{ return is_enabled; }
    bool	isChecked()	const	{ return is_checked; }
    bool	isDirty()	const	{ return is_dirty; }

    void	setText( const char *text ) { text_data = text; }
    void	setDirty( bool d )	    { is_dirty = d; }

private:
    int		ident;				// item identifier
    QString	text_data;			// item text
    QPixmap    *pixmap_data;			// item pixmap
    QPopupMenu *popup_menu;			// item popup menu
    int		accel_key;			// accelerator key
    QSignal    *signal_data;			// connection
    uint	is_separator : 1;		// separator flag
    uint	is_enabled   : 1;		// disabled flag
    uint	is_checked   : 1;		// checked flag
    uint	is_dirty     : 1;		// dirty (update) flag
};

#include "qlist.h"
typedef declare(QListM,QMenuItem)	  QMenuItemList;
typedef declare(QListIteratorM,QMenuItem) QMenuItemListIt;

#else

class QMenuItem;
class QMenuItemList;
class QPixmap;

#endif


class QMenuData					// menu data class
{
friend class QMenuBar;
friend class QPopupMenu;
public:
    QMenuData();
    virtual ~QMenuData();

    uint	count() const;

    int		insertItem( const char *text,
			    const QObject *receiver, const char *member,
			    int accel=0 );
    int		insertItem( const QPixmap &pixmap,
			    const QObject *receiver, const char *member,
			    int accel=0 );
    int		insertItem( const char *text, int id=-1, int index=-1 );
    int		insertItem( const char *text, QPopupMenu *popup,
			    int id=-1, int index=-1 );
    int		insertItem( const QPixmap &pixmap, int id=-1, int index=-1 );
    int		insertItem( const QPixmap &pixmap, QPopupMenu *popup,
			    int id=-1, int index=-1 );

    void	insertSeparator( int index=-1 );

    void	removeItem( int id )		{ removeItemAt(indexOf(id)); }
    void	removeItemAt( int index );
    void	clear();

    int		accel( int id )		const;
    void	setAccel( int key, int id );

    const char *text( int id )		const;
#if defined(OBSOLETE)
    const char *string( int id )	const;
#endif
    QPixmap    *pixmap( int id )	const;
    void	changeItem( const char *text, int id );
    void	changeItem( const QPixmap &pixmap, int id );

    bool	isItemEnabled( int id ) const;
    void	setItemEnabled( int id, bool enable );
#if defined(OBSOLETE)
    bool	isItemDisabled( int id ) const;
    void	enableItem( int id );
    void	disableItem( int id );
#endif

    bool	isItemChecked( int id ) const;
    void	setItemChecked( int id, bool check );
#if defined(OBSOLETE)
    void	checkItem( int id );
    void	uncheckItem( int id );
#endif

    virtual void updateItem( int id );

    int		indexOf( int id )	const;
    int		idAt( int index )	const;
    void	setId( int index, int id );

    bool	connectItem( int id,
			     const QObject *receiver, const char *member );
    bool	disconnectItem( int id,
				const QObject *receiver, const char *member );

    QMenuItem  *findItem( int id )	const;

protected:
    int		   actItem;
    QMenuItemList *mitems;
    QMenuData	  *parentMenu;
    uint	   isPopupMenu	: 1;
    uint	   isMenuBar	: 1;
    uint	   badSize	: 1;
    uint	   mouseBtDn	: 1;
    virtual void   menuContentsChanged();
    virtual void   menuStateChanged();
    virtual void   menuInsPopup( QPopupMenu * );
    virtual void   menuDelPopup( QPopupMenu * );

private:
    int		insertAny( const char *, const QPixmap *, QPopupMenu *,
			   int, int );
    void	removePopup( QPopupMenu * );
    void	setAllDirty( bool );
};


#if defined(OBSOLETE)
inline const char *QMenuData::string( int id ) const
{
    qObsolete("QMenuData","string","text" );
    return text(id); 
}

inline bool QMenuData::isItemDisabled( int id ) const
{
    qObsolete("QMenuData","isItemDisabled","!isItemEnabled()" );
    return !isItemEnabled(id);
}

inline void QMenuData::enableItem( int id )
{
    qObsolete("QMenuData","enableITem","setItemEnabled(id,TRUE)" );
    setItemEnabled( id, TRUE );
}

inline void QMenuData::disableItem( int id )
{
    qObsolete("QMenuData","disableItem","setItemEnabled(id,FALSE)" );
    setItemEnabled( id, FALSE );
}

inline void QMenuData::checkItem( int id )
{
    qObsolete("QMenuData","checkItem","setItemChecked(id,TRUE)" );
    setItemChecked( id, TRUE );
}

inline void QMenuData::uncheckItem( int id )
{
    qObsolete("QMenuData","uncheckItem","setItemChecked(id,FALSE)" );
    setItemChecked( id, FALSE );
}
#endif


#endif // QMENUDTA_H
