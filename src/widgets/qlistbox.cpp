/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.cpp#40 $
**
** Implementation of QListBox widget class
**
** Author  : Eirik Eng
** Created : 941121
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlistbox.h"
#include "qfontmet.h"
#include "qpainter.h"
#include "qstrlist.h"
#include "qkeycode.h"
#include "qpixmap.h"
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlistbox.cpp#40 $";
#endif


declare(QListM, QLBItem);

class QLBItemList : public QListM(QLBItem)	// internal class
{
    int compareItems( GCI i1, GCI i2);
};

int QLBItemList::compareItems( GCI i1, GCI i2)
{
    QLBItem * lbi1 = (QLBItem*) i1;
    QLBItem * lbi2 = (QLBItem*) i2;

    if ( lbi1->type == LBI_String && lbi2->type == LBI_String )
	return strcmp( lbi1->string, lbi2->string );
    if ( lbi1->type == LBI_String )
	return 1;   // A string is greater than an unknown
    if ( lbi2->type == LBI_String )
	return -1;  // An unknown is less than a string
    return 0;	    // An unknown equals an unknown
}


static inline bool checkInsertIndex( const char *method, int count, int *index)
{
    if ( *index > count ) {
#if defined(CHECK_RANGE)
	warning( "QListBox::%s Index %i out of range", method, *index );
#endif
	return FALSE;
    }
    if ( *index < 0 )				// append
	*index = count;
    return TRUE;
}

static inline bool checkIndex( const char *method, int count, int index )
{
    if ( index >= count ) {
#if defined(CHECK_RANGE)
	warning( "QListBox::%s Index %d out of range", method, index );
#endif
	return FALSE;
    }
    return TRUE;
}


/*----------------------------------------------------------------------------
  \class QListBox qlistbox.h
  \brief The QListBox widget provides a single-column list of items that
  can be scrolled.

  \ingroup realwidgets

  Each item in a QListBox can contain either a string (QString or
  char*) or a pixmap.  One of the items can be the current item.  The
  highlighted() signal is emitted when the user selects a new current
  item, and selected() is emitted when the user actually selects the
  current item.

  If the user does not select anything, no signals are emitted and
  currentItem() returns -1.

  New items may be inserted using either insertItem(), insertStrList()
  and inSort().

  By default, vertical and horizontal scroll bars are added and
  removed as necessary.  setAutoScrollBar() can be used to force a
  specific policy.

  If you need a list box that contains other types, you must inherit
  QListBox and reimplement itemWidth(), itemHeight() and paintItem().
  You must also call setUserItems( TRUE ) in the subclass constructor.

  You must use QLBItem quite a bit, see qlistbox.h for its declaration
  and nowhere (yet) for its documentation.  If you inherit QLBItem as
  well, you need to reimplement deleteItem() and newItem().
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Constructs a list box.  The arguments are passed directly to the
  QTableView constructor.
 ----------------------------------------------------------------------------*/

QListBox::QListBox( QWidget *parent, const char *name )
    : QTableView( parent, name )
{
    initMetaObject();
    copyStrings	  = TRUE;
    doDrag	  = TRUE;
    doAutoScroll  = TRUE;
    current	  = -1;
    isTiming	  = FALSE;
    stringsOnly	  = TRUE;
    ownerDrawn	  = FALSE;
    goingDown	  = FALSE;
    itemList	  = new QLBItemList;
    CHECK_PTR( itemList );
    setCellWidth( 0 );
    QFontMetrics fm = fontMetrics();
    setCellHeight( fm.lineSpacing() + 1 );
    setNumCols( 1 );
    setTableFlags( Tbl_smoothVScrolling );
    clearTableFlags( Tbl_clipCellPainting );
    setFrameStyle( QFrame::Sunken | QFrame::Panel );
    setLineWidth( 1 );
}

/*----------------------------------------------------------------------------
  Destroys the list box.  Deletes all list box items.
 ----------------------------------------------------------------------------*/

QListBox::~QListBox()
{
    goingDown = TRUE;
    clearList();
}


/*----------------------------------------------------------------------------
  Returns the number of items in the list box.
 ----------------------------------------------------------------------------*/

int QListBox::count() const
{
    return itemList->count();
}


/*----------------------------------------------------------------------------
  Inserts the string list \e list into the list at item \e index.

  If \e index is negative, \e list is inserted at the end of the list.  If
  \e index is too large, the operation is ignored.

  \sa insertItem(), inSort()
 ----------------------------------------------------------------------------*/

void QListBox::insertStrList( const QStrList *list, int index )
{
    if ( !checkInsertIndex( "insertStrList", count(), &index ) )
	return;
    if ( !list ) {
#if defined(CHECK_NULL)
	ASSERT( list != 0 );
#endif
	return;
    }
    QStrListIterator it( *list );
    const char *tmp;
    if ( index < 0 )
	index = itemList->count();
    while ( (tmp=it.current()) ) {
	++it;
	insertAny( tmp, 0, 0, index++, FALSE );
    }
    updateNumRows( TRUE );
    if ( autoUpdate() && itemVisible(index) )
	update();
}

/*----------------------------------------------------------------------------
  Inserts the \e numStrings strings of the array \e strings into the
  list at item\e index.

  If \e index is negative, insertStrList() inserts \e strings at the end
  of the list.  If \e index is too large, the operation is ignored.

  \sa insertItem(), inSort()
 ----------------------------------------------------------------------------*/

void QListBox::insertStrList( const char **strings, int numStrings, int index )
{
    if ( !checkInsertIndex( "insertStrList", count(), &index ) )
	return;
    if ( !strings ) {
#if defined(CHECK_NULL)
	ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = itemList->count();
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	insertAny( strings[i], 0, 0, index++, FALSE );
	i++;
    }
    updateNumRows( TRUE );
    if ( autoUpdate() && itemVisible(index) )
	update();
}

/*----------------------------------------------------------------------------
  Inserts \e string into the list at \e index.

  If \e index is negative, \e string is inserted at the end of the list.

  \sa insertStrList()
 ----------------------------------------------------------------------------*/

void QListBox::insertItem( const char *string, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
    if ( !string ) {
#if defined ( CHECK_NULL )
	ASSERT( string != 0 );
#endif
	return;
    }
    insertAny( string, 0, 0, index, TRUE );
    updateNumRows( FALSE );
    if ( autoUpdate() && itemVisible( index ) )
	update();
}

/*----------------------------------------------------------------------------
  Inserts \e pixmap into the list at \e index.

  If \e index is negative, \e pixmap is inserted at the end of the list.

  \sa insertStrList()
 ----------------------------------------------------------------------------*/

void QListBox::insertItem( const QPixmap &pixmap, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
    if ( stringsOnly ) {
	stringsOnly = FALSE;
	setCellHeight( 0 );
    }
    insertAny( 0, (QPixmap*)&pixmap, 0, index, TRUE );
    int w = pixmap.width() + 6;
    if ( w > cellWidth() )
	setCellWidth( w );
    updateNumRows( FALSE );
    if ( autoUpdate() && itemVisible( index ) )
	update();
}

/*----------------------------------------------------------------------------
  Inserts \e string into the list and sorts the list.

  inSort() treats any pixmap (or user-defined type) as lexicographically
  less than any string.

  \sa insertItem()
 ----------------------------------------------------------------------------*/

void QListBox::inSort( const char *string )
{
    if ( !string ) {
#if defined ( CHECK_NULL )
	ASSERT( string != 0 );
#endif
	return;
    }
    itemList->inSort( newAny( string, 0 ) );
    QFontMetrics fm = fontMetrics();
    int w = fm.width( string ) + 6;
    if ( w > cellWidth() )
	setCellWidth( w );
    updateNumRows( FALSE );
    if ( autoUpdate() ) {
	update(); // Optimize drawing ( find index )
    }
}


/*----------------------------------------------------------------------------
  Removes the item at position \e index.
  \sa insertItem(), clear()
 ----------------------------------------------------------------------------*/

void QListBox::removeItem( int index )
{
    if ( !checkIndex( "removeItem", count(), index ) )
	return;
    bool    updt = autoUpdate() && itemVisible( index );
    QLBItem *lbi = itemList->take( index );
    QFontMetrics fm = fontMetrics();
    int w = internalItemWidth( lbi, fm );
    if ( w == cellWidth() )
	updateCellWidth();
    if ( lbi->type == LBI_String && copyStrings )
	delete (char*)lbi->string;
    else if ( lbi->type == LBI_Pixmap )
	delete lbi->pixmap;
    delete lbi;
    if ( updt )
	update();
}

/*----------------------------------------------------------------------------
  Deletes all items in the list.
  \sa removeItem(), setStrList()
 ----------------------------------------------------------------------------*/

void QListBox::clear()
{
    clearList();
    if ( autoUpdate() )
	erase();
}


/*----------------------------------------------------------------------------
  Returns a pointer to the string at position \e index, or 0 if there is no
  string there.
  \sa pixmap()
 ----------------------------------------------------------------------------*/

const char *QListBox::string( int index ) const
{
    if ( !checkIndex( "string", count(), index ) )
	return 0;
    QLBItem *lbi = itemList->at( index );
    return lbi->type == LBI_String ? lbi->string : 0;
}

/*----------------------------------------------------------------------------
  Returns a pointer to the pixmap at position \e index, or 0 if there is no
  pixmap there.
  \sa string()
 ----------------------------------------------------------------------------*/

const QPixmap *QListBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", count(), index ) )
	return 0;
    QLBItem *lbi = itemList->at( index );
    return lbi->type == LBI_Pixmap ? lbi->pixmap : 0;
}

/*----------------------------------------------------------------------------
  Replaces the item at position \e index with \e string.

  The operation is ignored if \e index is out of range.
  
  \sa insertItem(), removeItem()
 ----------------------------------------------------------------------------*/

void QListBox::changeItem( const char *string, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    changeAny( string, 0, 0, index );
}

/*----------------------------------------------------------------------------
  Replaces the item at position \e index with \e pixmap.

  The operation is ignored if \e index is out of range.

  \sa insertItem(), removeItem()
 ----------------------------------------------------------------------------*/

void QListBox::changeItem( const QPixmap &pixmap, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    changeAny( 0, &pixmap, 0, index );
}


/*----------------------------------------------------------------------------
  Returns TRUE if the list box makes copies of strings that are
  inserted.
  \sa setStringCopy()
 ----------------------------------------------------------------------------*/

bool QListBox::stringCopy() const
{
    return copyStrings;
}

/*----------------------------------------------------------------------------
  Specifies whether the list box should make copies of the strings
  that are inserted.

  If \e enable is TRUE, the list box makes copies of the inserted
  strings. If \e enable is FALSE, the list box keeps references to the
  inserted strings.

  \warning If you choose to use references instead of copies, you will be
  responsible for deleting the strings after the list box has been
  destroyed.  Be careful and do not modify any string that is referenced
  from the list box.  The advantage of using references is that it takes
  less memory than making copies.

  This function should only be called when the list box is empty.  If
  the list box not empty it will produce a warning and return
  immediately.

  The default setting is TRUE.

  \sa setStringCopy() 
 ----------------------------------------------------------------------------*/

void QListBox::setStringCopy( bool enable )
{
    if ( (bool)copyStrings == enable )
	return;
    if ( count() != 0 ) {
	warning( "QListBox::setStringCopy: Cannot change the value "
		 "when the list box is not empty." );
	return;
    }
}


/*----------------------------------------------------------------------------
  Returns TRUE if the list box updates itself automatically when
  items are inserted or removed.

  The default setting is TRUE.
  \sa setAutoUpdate() 
 ----------------------------------------------------------------------------*/

bool QListBox::autoUpdate() const
{
    return QTableView::autoUpdate();
}

/*----------------------------------------------------------------------------
  Specifies whether the list box should update itself automatically
  when items are inserted or removed.

  Auto-update is enabled by default.

  If \e enable is TRUE, the list box will update itself.  If \e enable
  is FALSE, the list box will not update itself.

  \warning Do not leave the view in this state for a long time
  (i.e. between events ). If the user interacts with the view when
  auto-update is off, strange things can happen.

  \sa autoUpdate()
 ----------------------------------------------------------------------------*/

void QListBox::setAutoUpdate( bool enable )
{
    QTableView::setAutoUpdate( enable );
}

/*----------------------------------------------------------------------------
  Returns the number of visible items.  This may change at any time
  since the user may resize the widget. 
 ----------------------------------------------------------------------------*/

int QListBox::numItemsVisible() const
{
    return (lastRowVisible() - topCell() + 1);
}

/*----------------------------------------------------------------------------
  Returns the index of the current (highlighted) item of the list box,
  or -1 if no item has been selected.

  \sa topItem()
 ----------------------------------------------------------------------------*/

int QListBox::currentItem() const
{
    return current;
}

/*----------------------------------------------------------------------------
  Sets the highlighted item to the item at position \e index in the list.
  The highlighting is moved and the list box scrolled as necessary.
  \sa currentItem()
 ----------------------------------------------------------------------------*/

void QListBox::setCurrentItem( int index )
{
    if ( index == current )
	return;
    if ( !checkIndex( "setCurrentItem", count(), index ) )
	return;
    int oldCurrent = current;
    current	   = index;
    updateItem( oldCurrent );
    updateItem( current, FALSE ); // Do not clear, current marker covers item
    emit highlighted( current );
}

/*----------------------------------------------------------------------------
  Scrolls the list box so the current (highlighted) item is
  centered in the list box.
  \sa currentItem(), setTopItem()
 ----------------------------------------------------------------------------*/

void QListBox::centerCurrentItem()
{
    int top;
    if ( stringsOnly )
	top = current - numItemsVisible() / 2; // ###
    else
	top = current - numItemsVisible() / 2;
    if ( top < 0 )
	top = 0;
    int max = maxRowOffset();
    if ( top > max )
	top = max;
    setTopItem( top );
}

/*----------------------------------------------------------------------------
  Returns index of the item that is on the top line of the list box.
  \sa setTopItem(), currentItem()
 ----------------------------------------------------------------------------*/

int QListBox::topItem() const
{
    return topCell();
}

/*----------------------------------------------------------------------------
  Scrolls the list box so the item at position \e index in the list
  becomes the top row of the list box.
  \sa topItem(), centerCurrentItem() 
 ----------------------------------------------------------------------------*/

void QListBox::setTopItem( int index )
{
    setTopCell( index );
}


/*----------------------------------------------------------------------------
  Returns TRUE if drag-selection is enabled, otherwise FALSE.
  \sa setDragSelect(), autoScroll()
 ----------------------------------------------------------------------------*/

bool QListBox::dragSelect() const
{
    return doDrag;
}

/*----------------------------------------------------------------------------
  Sets drag-selection if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If drag-selection is enabled, the list box will highlight new items when
  the user drags the mouse inside the list box.

  The default setting is TRUE.

  \sa drawSelect(), setAutoScroll()
 ----------------------------------------------------------------------------*/

void QListBox::setDragSelect( bool enable )
{
    doDrag = enable;
}

/*----------------------------------------------------------------------------
  Returns TRUE if auto-scrolling is enabled, otherwise FALSE.
  \sa setAutoScroll, dragSelect()
 ----------------------------------------------------------------------------*/

bool QListBox::autoScroll() const
{
    return doAutoScroll;
}

/*----------------------------------------------------------------------------
  Sets auto-scrolling if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If auto-scrolling is enabled, the list box will scroll its contents when
  the user drags the mouse outside (below or above) the list  box.
  Auto-scrolling only works if \link setDragSelect() drag-selection\endlink
  is enabled.

  The default setting is TRUE.

  \sa autoScroll(), setDragSelect()
 ----------------------------------------------------------------------------*/

void QListBox::setAutoScroll( bool enable )
{
    doAutoScroll = enable;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the list box has an automatic (vertical) scroll bar.
  \sa setAutoScrollBar(), autoBottomScrollBar()
 ----------------------------------------------------------------------------*/

bool QListBox::autoScrollBar() const
{
    return testTableFlags( Tbl_autoVScrollBar );
}

/*----------------------------------------------------------------------------
  Enables an automatic (vertical) scroll bar if \e enable is TRUE, or disables
  it if \e enable is FALSE.
  
  If it is enabled, then the list box will get a (vertical) scroll bar if
  the list box items exceed the list box height.

  The default setting is TRUE.

  \sa autoScrollBar(), setScrollBar(), setAutoBottomScrollBar()
 ----------------------------------------------------------------------------*/

void QListBox::setAutoScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_autoVScrollBar );
    else
	clearTableFlags( Tbl_autoVScrollBar );
}

/*----------------------------------------------------------------------------
  Returns TRUE if the list box has a (vertical) scroll bar.
  \sa setScrollBar(), autoScrollBar(), bottomScrollBar()
 ----------------------------------------------------------------------------*/

bool QListBox::scrollBar() const
{
    return testTableFlags( Tbl_vScrollBar );
}

/*----------------------------------------------------------------------------
  Enables a (vertical) scroll bar if \e enable is TRUE, or disables it if
  \e enable is FALSE.
  
  The default setting is FALSE.

  \sa scrollBar(), setAutoScrollBar(), setBottomScrollBar()  
 ----------------------------------------------------------------------------*/

void QListBox::setScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_vScrollBar );
    else
	clearTableFlags( Tbl_vScrollBar );
}

/*----------------------------------------------------------------------------
  Returns TRUE if the list box has an automatic bottom scroll bar.
  \sa setAutoBottomScrollBar(), autoScrollBar()
 ----------------------------------------------------------------------------*/

bool QListBox::autoBottomScrollBar() const
{
    return testTableFlags( Tbl_autoHScrollBar );
}

/*----------------------------------------------------------------------------
  Enables an automatic bottom scroll bar if \e enable is TRUE, or disables
  it if \e enable is FALSE.
  
  If it is enabled, then the list box will get a bottom scroll bar if the
  maximum list box item width exceeds the list box width.

  The default setting is TRUE.

  \sa autoBottomScrollBar(), setBottomScrollBar(), setAutoScrollBar()
 ----------------------------------------------------------------------------*/

void QListBox::setAutoBottomScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_autoHScrollBar );
    else
	clearTableFlags( Tbl_autoHScrollBar );
}

/*----------------------------------------------------------------------------
  Returns TRUE if the list box has a bottom scroll bar.
  \sa setBottomScrollBar(), autoBottomScrollBar(), scrollBar()
 ----------------------------------------------------------------------------*/

bool QListBox::bottomScrollBar() const
{
    return testTableFlags( Tbl_hScrollBar );
}

/*----------------------------------------------------------------------------
  Enables a bottom scroll bar if \e enable is TRUE, or disables it if
  \e enable is FALSE.
  
  The default setting is FALSE.

  \sa bottomScrollBar(), setAutoBottomScrollBar(), setScrollBar()
 ----------------------------------------------------------------------------*/

void QListBox::setBottomScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_hScrollBar );
    else
	clearTableFlags( Tbl_hScrollBar );
}

/*----------------------------------------------------------------------------
  Returns TRUE if smooth list box scrolling is enabled, otherwise FALSE.
  \sa setSmoothScrolling()
 ----------------------------------------------------------------------------*/

bool QListBox::smoothScrolling() const
{
    return testTableFlags( Tbl_smoothVScrolling );
}

/*----------------------------------------------------------------------------
  Enables smooth list box scrolling if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  The default setting is TRUE.

  \sa smoothScrolling()
 ----------------------------------------------------------------------------*/

void QListBox::setSmoothScrolling( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_smoothVScrolling );
    else
	clearTableFlags( Tbl_smoothVScrolling );
}


/*----------------------------------------------------------------------------
  Returns TRUE if the list box contains user-defined items, otherwise FALSE.
  \sa setUserItems()
 ----------------------------------------------------------------------------*/

bool QListBox::userItems() const
{
    return ownerDrawn;
}

/*----------------------------------------------------------------------------
  Specifies that the list box should contains user-defined items if \e
  enable is TRUE, or only standard items (strings and pixmaps) if \e
  enable is FALSE.

  The default setting is FALSE.

  A list box with custom items can only be made through inheritance;
  you must make a new class that inherits QListBox and reimplement
  the virtual functions itemWidth(QLBItem*), itemHeight(QLBItem*),
  newItem(), deleteItem() and paintItem().
 ----------------------------------------------------------------------------*/

void QListBox::setUserItems( bool enable )
{
    if ( (bool)ownerDrawn == enable )
	return;
    if ( itemList->count() != 0 ) {
#if defined(CHECK_STATE)
	warning( "QListBox::setUserItems: List box not empty");
#endif
	return;
    }
    ownerDrawn = enable;
}


/*----------------------------------------------------------------------------
  Returns a new list box item.

  This virtual function can be reimplemented in a subclass if the list box
  contains user-defined items, i.e. you have called setUserItems( TRUE ).
  You do not need to reimplement this function if you put the user-defined
  items into a QLBItem.  You must reimplement this function if you make
  your own class that inherits QLBItem.

  The default implementation:
  \code
    return new QLBItem;		// data=0, type=LBI_Undefined
  \endcode

  \sa deleteItem(), setUserItems()
 ----------------------------------------------------------------------------*/

QLBItem *QListBox::newItem()
{
    return new QLBItem;
}

/*----------------------------------------------------------------------------
  Deletes a list box item \e lbi.

  This virtual function should be reimplemented in a subclass if the list
  box is contains user-defined items, i.e. you have called
  setUserItems( TRUE ).

  Notice that C++ does not allow calling virtual functions from the 
  list box constructor or destructor.  If your list box contains user items,
  your class that inherits QListBox should have a destructor which calls
  clearList().

  \sa newItem(), setUserItems()
 ----------------------------------------------------------------------------*/

void QListBox::deleteItem( QLBItem *lbi )
{
    delete lbi;
}


/*----------------------------------------------------------------------------
  Paints a user-defined list box item.

  All subclasses that setUserItems(TRUE) must reimplement this function.

  \warning Do not paint outside the area that your itemHeight() and
  itemWidth() functions indicate.  QListBox does not guarantee correct
  clipping.

  \sa itemHeight(), itemWidth(), setUserItems(), paintCell()
 ----------------------------------------------------------------------------*/

void QListBox::paintItem( QPainter *, int )
{
}


/*----------------------------------------------------------------------------
  Inserts \e lbi into the list at \e index.

  If \e index is negative or larger than the number of items in the list
  box, \e lbi is inserted at the end of the list.

  \sa insertStrList()
 ----------------------------------------------------------------------------*/

void QListBox::insertItem( const QLBItem *lbi, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
    if ( !lbi ) {
#if defined ( CHECK_NULL )
	ASSERT( lbi != 0 );
#endif
	return;
    }
    stringsOnly = FALSE;
    insertAny( 0, 0, lbi, index, TRUE );
    updateNumRows( FALSE );
    if ( autoUpdate() )
	update();
}

#if 0
void QListBox::inSort( const QLBItem *lbi )
{
#if defined(CHECK_NULL)
    CHECK_PTR( lbi );
#endif
    itemList->inSort( lbi );
//###	 updateNumRows( FALSE );
    if ( autoUpdate() ) {
	update(); // Optimize drawing ( find index )
    }
}
#endif


/*----------------------------------------------------------------------------
  Replaces the item at posistion \e index with \e lbi.  If \e
  index is negative or too large, changeItem() does nothing.

  \sa insertItem(), removeItem()
 ----------------------------------------------------------------------------*/

void QListBox::changeItem( const QLBItem *lbi, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    changeAny( 0, 0, lbi, index );
}

/*----------------------------------------------------------------------------
  Returns a pointer to the item at position \e index.
 ----------------------------------------------------------------------------*/

QLBItem *QListBox::item( int index ) const
{
    if (!checkIndex( "item", count(), index ) )
	return 0;
    return itemList->at( index );
}

/*----------------------------------------------------------------------------
  Returns the height of the item at position \e index in pixels.
 ----------------------------------------------------------------------------*/

int QListBox::cellHeight( int index )
{
    if ( stringsOnly ) {
	QFontMetrics fm = fontMetrics();
	return fm.lineSpacing() + 1;
    }
    QLBItem *lbi = item( index );
    if ( lbi ) {
	switch( lbi->type ) {
	    case LBI_String: {
		QFontMetrics fm = fontMetrics();
		return fm.lineSpacing() + 1;
	    }
	    case LBI_Pixmap: {
		if ( lbi->pixmap )
		    return lbi->pixmap->height();
		else
		    return 0;
	    }
	    default:
		return itemHeight( lbi );
	}
    } else {
	return 0;
    }
}


/*----------------------------------------------------------------------------
  Returns the standard item height (in pixels), or -1 if the list box has
  variable item height.
 ----------------------------------------------------------------------------*/

int QListBox::itemHeight() const
{
    return stringsOnly ? ((QListBox*)this)->cellHeight( 0 ) : -1;
}


/*----------------------------------------------------------------------------
  Returns the height (in pixels) of item at \e index.
 ----------------------------------------------------------------------------*/

int QListBox::itemHeight( int index ) const
{
    return ((QListBox*)this)->cellHeight( index );
}


/*----------------------------------------------------------------------------
  This virtual function returns 0 in QListBox and must
  be reimplemented by subclasses that use other types.

  It must return the height of \e item in pixels.
 ----------------------------------------------------------------------------*/

int QListBox::itemHeight( QLBItem * item )
{
    warning( "QListBox::itemHeight: You must reimplement itemHeight() when you"
	     " use item types different from LBI_String and LBI_Pixmap" );
    return 0;
}

/*----------------------------------------------------------------------------
  This virtual function returns 0 in QListBox and must
  be reimplemented by subclasses that use other types.

  It must return the width of \e item in pixels.
 ----------------------------------------------------------------------------*/

int QListBox::itemWidth( QLBItem * item )
{
    warning( "QListBox::itemWidth: You must reimplement itemWidth() when you"
	     " use item types different from LBI_String and LBI_Pixmap" );
    return 0;
}


/*----------------------------------------------------------------------------
  Returns TRUE if the item at position \e index is at least partly
  visible.
 ----------------------------------------------------------------------------*/

bool QListBox::itemVisible( int index )
{
    return rowIsVisible( index );
}

/*----------------------------------------------------------------------------
  Repaints the cell at position \e row using \e p.  The \e column
  argument is ignored, it is present because QTableView is more
  general.

  \bug When userItems() is TRUE, this function will call paintItem()
  for \e all items.  This is possibly not the correct behaviour.
  Feedback appreciated.

  \sa QTableView::paintCell()
 ----------------------------------------------------------------------------*/

void QListBox::paintCell( QPainter *p, int row, int column )
{
    if ( ownerDrawn ) {
	paintItem( p, row );
	return;
    }

    QLBItem *lbi = itemList->at( row );
    if ( !lbi )
	return;
    if ( lbi->type != LBI_String && lbi->type != LBI_Pixmap ) {
	warning( "QListBox::paintCell: illegal item type (%d) in"
		 " non-ownerdrawn list box", lbi->type );
	return;
    }

    QFontMetrics fm = fontMetrics();
    QColorGroup  g  = colorGroup();
    if ( current == row ) {
	p->fillRect( 0, 0, cellWidth( column ), cellHeight( row ), g.text() );
	p->setPen( g.background() );
    } else {
	p->setPen( g.text() );
    }
    if ( lbi->type == LBI_String )
	p->drawText( 3, fm.ascent() + fm.leading()/2, lbi->string );
    if ( lbi->type == LBI_Pixmap )
	p->drawPixmap( 3, 0, *lbi->pixmap );
}

/*----------------------------------------------------------------------------
  Handles mouse press events.  Makes the clicked item the current item.
  \sa currentItem() 
 ----------------------------------------------------------------------------*/

void QListBox::mousePressEvent( QMouseEvent *e )
{
    int itemClicked = findItem( e->pos().y() );
    if ( itemClicked != -1 ) {
	setCurrentItem( itemClicked );
    }
}

/*----------------------------------------------------------------------------
  Handles mouse release events.
 ----------------------------------------------------------------------------*/

void QListBox::mouseReleaseEvent( QMouseEvent *e )
{
    if ( doDrag )
	mouseMoveEvent( e );
    if ( isTiming ) {
	killTimers();
	isTiming = FALSE;
    }
}

/*----------------------------------------------------------------------------
  Handles mouse double click events.  Emits the selected() signal for
  the item that was double-clicked.
 ----------------------------------------------------------------------------*/

void QListBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mouseReleaseEvent( e );
    if ( currentItem() >= 0 )
	emit selected( currentItem());
}

/*----------------------------------------------------------------------------
  Handles mouse move events.  Scrolls the list box is auto-scroll
  is enabled.
  \sa autoScroll()
 ----------------------------------------------------------------------------*/

void QListBox::mouseMoveEvent( QMouseEvent *e )
{
    if ( doDrag ) {
	int itemClicked = findItem( e->pos().y() );
	if ( itemClicked != -1 ) {
	    if ( isTiming ) {
		killTimers();
		isTiming = FALSE;
	    }
	    setCurrentItem( itemClicked );	// already current -> return
	    return;
	} else {
	    if ( !doAutoScroll )
		return;
	    if ( e->pos().y() < frameWidth() )
		scrollDown = FALSE;
	    else
		scrollDown = TRUE;
	    if ( !isTiming ) {
		isTiming = TRUE;
		startTimer( 100 );
	    }
	}
    }
}

/*----------------------------------------------------------------------------
  Handles key press events.

  \c Up and \c down arrow keys make the highlighted item move and if
  necessary scroll the list box.

  \c Enter makes the list box emit the selected() signal.

  \sa selected(), setCurrentItem()
 ----------------------------------------------------------------------------*/

void QListBox::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
	case Key_Up:
	    if ( currentItem() != 0 ) {
		setCurrentItem( currentItem() - 1 );
		if ( currentItem() == topItem() - 1 )
		    setTopItem( topItem() - 1 );
	    }
	    break;
	case Key_Down:
	    if ( currentItem() < count() - 1 ) {
		setCurrentItem( currentItem() + 1 );
		if ( currentItem() == lastRowVisible() + 1 )
		    setTopItem( topItem() + 1 );
	    }
	    break;
	case Key_Return:
	case Key_Enter:
	    if ( currentItem() >= 0 )
	    emit selected( currentItem());
	    break;
	default:
	    break;
    }
}

/*----------------------------------------------------------------------------
  Handles resize events.  Updates internal parameters for the new list box
  size.
 ----------------------------------------------------------------------------*/

void QListBox::resizeEvent( QResizeEvent *e )
{
    QTableView::resizeEvent( e );
    updateCellWidth();
}

/*----------------------------------------------------------------------------
  Handles timer events.  Does auto-scrolling.
 ----------------------------------------------------------------------------*/

void QListBox::timerEvent( QTimerEvent * )
{
    if ( scrollDown ) {
	if ( topItem() != maxRowOffset() ) {
	    setTopItem( topItem() + 1 );
	    setCurrentItem( lastRowVisible() );
	}
    } else {
	if ( topItem() != 0 ) {
	    setTopItem( topItem() - 1 );
	    setCurrentItem( topItem() );
	}
    }
}


/*----------------------------------------------------------------------------
  Returns the vertical pixel-coordinate in \e *yPos, of the list box
  item at position \e index in the list.  Returns FALSE if the item is
  outside the visible area.

  \sa findItem
 ----------------------------------------------------------------------------*/

bool QListBox::itemYPos( int index, int *yPos ) const
{

    return rowYPos( index, yPos );
}

/*----------------------------------------------------------------------------
  Returns the index of the list box item at the vertical pixel-coordinate
  \e yPos.

  \sa itemYPos()
 ----------------------------------------------------------------------------*/

int QListBox::findItem( int yPos ) const
{
    return findRow( yPos );
}

/*----------------------------------------------------------------------------
  Repaints the item at position \e index in the list.  Erases the line
  first if \e erase is TRUE.
 ----------------------------------------------------------------------------*/

void QListBox::updateItem( int index, bool erase )
{
    updateCell( index, 0,  erase );
}

/*----------------------------------------------------------------------------
  Deletes all items in the list.  Protected function that does NOT
  update the list box. 
 ----------------------------------------------------------------------------*/

void QListBox::clearList()
{
    stringsOnly = TRUE;
    QLBItem *lbi;
    while ( itemList->count() ) {
	lbi = itemList->take( 0 );
	if ( ownerDrawn )
	    deleteItem( lbi );
	else {
	    if ( lbi->type == LBI_String && copyStrings )
		delete (char *)lbi->string;
	    else if ( lbi->type == LBI_Pixmap )
		delete lbi->pixmap;
	    delete lbi;
	}
    }
    if ( goingDown || QApplication::closingDown() )
	return;
    bool a = autoUpdate();
    setAutoUpdate( FALSE );
    updateNumRows( TRUE );
    current = -1;
    setTopCell( 0 );
    setAutoUpdate( a );
}

/*----------------------------------------------------------------------------
  Traverses the list and finds an item with the maximum width, and
  updates the internal list box structures accordingly.
 ----------------------------------------------------------------------------*/

void QListBox::updateCellWidth()
{
    QLBItem *lbi = itemList->first();
    QFontMetrics fm = fontMetrics();
    int maxW = viewWidth();
    int w;
    while ( lbi ) {
	w = internalItemWidth( lbi, fm );
	if ( w > maxW )
	    maxW = w;
	lbi = itemList->next();
    }
    setCellWidth( maxW );
}


/*----------------------------------------------------------------------------
  \internal
  Returns a new string or pixmap item.
 ----------------------------------------------------------------------------*/

QLBItem *QListBox::newAny( const char *str, const QPixmap *pm )
{
#if defined(CHECK_NULL)
    ASSERT( str || pm );
#endif
    QLBItem *lbi = newItem();
    CHECK_PTR( lbi );
    if ( str ) {
	if ( copyStrings )
	    lbi->string = qstrdup( str );
	else
	    lbi->string = str;
	lbi->type = LBI_String;
    }
    else if ( pm ) {
	lbi->pixmap = new QPixmap( *pm );
	lbi->type   = LBI_Pixmap;
    }
    return lbi;
}

/*----------------------------------------------------------------------------
  \internal
  Inserts a new list box item.
 ----------------------------------------------------------------------------*/

void QListBox::insertAny( const char *str, const QPixmap *pm,
			  const QLBItem *lbi, int index,
			  bool updateCellWidth	)
{
#if defined(CHECK_RANGE)
    if ( index > count() )
	warning( "QListBox::insertAny: Index %d out of range", index );
#endif
#if defined(CHECK_NULL)
    if ( !str && !pm && !lbi )
	warning( "QListBox::insertAny: Unexpected null argument" );
#endif
    if ( !lbi )
	lbi = newAny( str, pm );
    itemList->insert( index, lbi );
    QFontMetrics fm = fontMetrics();
    if ( updateCellWidth ) {
	int w = internalItemWidth( lbi, fm );
	if ( w > cellWidth() )
	    setCellWidth( w );
    }
}

/*----------------------------------------------------------------------------
  \internal
  Changes a list box item.
 ----------------------------------------------------------------------------*/

void QListBox::changeAny( const char *str, const QPixmap *pm,
			  const QLBItem *lbi, int index )
{
#if defined(CHECK_RANGE)
    if ( index > count() )
	warning( "QListBox::changeAny: Index %d out of range", index );
#endif
#if defined(CHECK_NULL)
    if ( !str && !pm )
	warning( "QListBox::changeAny: Unexpected null argument" );
#endif
    QLBItem *old = itemList->take( index );
    QFontMetrics fm = fontMetrics();
    int w = internalItemWidth( old, fm );
    if ( w == cellWidth() )
	updateCellWidth();
    if ( old->type == LBI_String && copyStrings )
	delete (char*)old->string;
    else if ( old->type == LBI_Pixmap )
	delete old->pixmap;
    deleteItem( old );
    insertAny( str, pm, lbi, index, TRUE );
}

/*----------------------------------------------------------------------------
  \internal
  Updates the num-rows setting in the table view.
 ----------------------------------------------------------------------------*/

void QListBox::updateNumRows( bool updateWidth )
{
    bool autoU = autoUpdate();
    if ( autoU )
	setAutoUpdate( FALSE );
    bool sbBefore = testTableFlags( Tbl_vScrollBar );
    setNumRows( itemList->count() );
    if ( updateWidth || sbBefore != testTableFlags( Tbl_vScrollBar ))
	updateCellWidth();
    if ( autoU )
	setAutoUpdate( TRUE );
}

/*----------------------------------------------------------------------------
  \internal
  Returns the width of a list box item.
 ----------------------------------------------------------------------------*/

int QListBox::internalItemWidth( const QLBItem	    *lbi,
				 const QFontMetrics &fm ) const
{
    int w;
    switch ( lbi->type ) {
	case LBI_String:
	    w = fm.width( lbi->string ) + 6;
	    break;
	case LBI_Pixmap:
	    if ( lbi->pixmap )
		w = lbi->pixmap->width();
	    else
		w = 0;
	    break;
	default:
	    w = ((QListBox*)this)->itemWidth( (QLBItem*) lbi );
	    break;
    }
    return w;
}
