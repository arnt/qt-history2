/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.cpp#18 $
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

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlistbox.cpp#18 $";
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
    return 0;       // An unknown equals an unknown
}

static inline bool checkInsertIndex( const char *method, int count, int *index)
{
    if ( *index > count ) {
#if defined(CHECK_RANGE)
	warning( "QListBox::%s Index %i out of range", method, index );
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
	warning( "QListBox::%s Index %i out of range", method, index );
#endif
	return FALSE;
    }
    return TRUE;
}

/*! \class QListBox qlistbox.h

  \brief QListBox provides a single-column list of items.

  */

/*! Constructs a list box.  The arguments are passed to the parent
  class as usual. */

QListBox::QListBox( QWidget *parent, const char *name )
    : QTableWidget( parent, name )
{
    initMetaObject();
    copyStrings	  = TRUE;
    doDrag	  = TRUE;
    doAutoScroll  = TRUE;
    current	  = -1;
    isTiming	  = FALSE;
    stringsOnly	  = TRUE;
    ownerDrawn	  = FALSE;
    itemList	  = new QLBItemList;
    CHECK_PTR( itemList );
    setCellWidth( 0 );			// setup table params
    QFontMetrics fm( font() );
    setCellHeight( fm.lineSpacing() + 1 );
    setNumCols( 1 );
    setTableFlags( Tbl_smoothVScrolling ); 
    clearTableFlags( Tbl_clipCellPainting );
}

/*! Deletes the list box and frees the memory used. */

QListBox::~QListBox()
{
    clearList();
    delete itemList;
}

/*! Sets the list box' contents to \e l.  The list box is cleared in
  various ways; scrolled to the top of the list and so on.  \sa
  insertStrList(), insertItem(). */

void QListBox::setStrList( const QStrList *l )
{
    clearList();
    bool wasAuto = autoUpdate();
    setAutoUpdate( FALSE );
    if ( !l ) {
#if defined(CHECK_NULL)
	CHECK_PTR( l );
#endif
	setNumRows( 0 );
	return;
    }
    QStrListIterator iter( *l );
    const char *tmp;
    while ( (tmp = iter.current()) ) {
	itemList->append( newAny( tmp, 0 ) );
	++iter;
    }
    updateNumRows();
    setTopCell( 0 );
    if ( wasAuto ) {
        setAutoUpdate( TRUE );
        update();
    }
}

/*! Sets the list box' contents to the \e numStrings at \e l.  The
  list box is cleared in various ways; scrolled to the top of the list
  and so on.  \sa insertStrList(), insertItem(). */

void QListBox::setStrList( const char **strs,int numStrings )
{
    clearList();
    bool wasAuto = autoUpdate();
    setAutoUpdate( FALSE );
    if ( !strs ) {
#if defined ( CHECK_NULL )
	CHECK_PTR( strs );
#endif
	setNumRows( 0 );
	return;
    }
    for( int i = 0 ; i < numStrings ; i++ )
	itemList->append( newAny( strs[i], 0 ) );

    updateNumRows();
    setTopCell( 0 );
    if ( wasAuto ) {
        setAutoUpdate( TRUE );
        update();
    }
}

/*! Inserts the string list \e l into the list at item \e index.  If
  \e index is negative, \e l is inserted at the end of the list.  If
  \e index is too large, insertStrList() doesn't do anything.

  \sa insertItem(), inSort() */

void QListBox::insertStrList( const QStrList *l, int index )
{
    if ( !checkInsertIndex( "insertStrList", count(), &index ) )
	return;

#if defined(CHECK_NULL)
    CHECK_PTR( l );
#endif

    QStrListIterator iter( *l );
    const char *tmp;
    for( iter.toLast() ; (tmp=iter.current()) ; --iter )
	insertAny( tmp, 0, 0, index, FALSE );
    updateNumRows();
    if ( autoUpdate() && itemVisible( index ) ) {
	update(); // Optimize drawing ( only the ones after index )
    }
}

/*! Inserts the \e numStrings strings at \e strs into the list at item
  \e index.  If \e index is negative, \e l is inserted at the end of
  the list.  If \e index is too large, insertStrList() doesn't do
  anything.

  \sa insertItem(), inSort() */

void QListBox::insertStrList( const char **strs, int numStrings, int index )
{
    if ( !checkInsertIndex( "insertStrList", count(), &index ) )
	return;

#if defined(CHECK_NULL)
    CHECK_PTR( strs );
#endif

    for( int i = numStrings - 1 ; i >= 0 ; i-- )
	insertAny( strs[i], 0, 0, index, FALSE );
    updateNumRows();
    if ( autoUpdate() && itemVisible( index ) ) {
	update(); // Optimize drawing ( only the ones after index )
    }
}

/*! Inserts \e string into the list at \e index.  If \e index is
  negative, \e string is inserted at the end of the list.  \sa
  insertStrList(). */

void QListBox::insertItem( const char *string, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
#if defined(CHECK_NULL)
    CHECK_PTR( string );
#endif
    insertAny( string, 0, 0, index );
    updateNumRows( FALSE );
    if ( autoUpdate() && itemVisible( index ) ) {
	update(); // Optimize drawing ( only the ones after index ) ###
    }
}

/*! Inserts \e pixmap into the list at \e index.  If \e index is
  negative, \e pixmap is inserted at the end of the list.  \sa
  insertStrList(). */

void QListBox::insertItem( const QPixmap *pixmap, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
#if defined(CHECK_NULL)
    CHECK_PTR( pixmap );
#endif
    if ( stringsOnly ) {
	stringsOnly = FALSE;
	setCellHeight( 0 );
    }
    insertAny( 0, pixmap, 0, index );
    int w = pixmap->width() + 6;
    if ( w > cellWidth() )
        setCellWidth( w );
    updateNumRows( FALSE );
    if ( autoUpdate() && itemVisible( index ) ) {
	update(); // Optimize drawing ( only the ones after index )
    }

}

/*!
Inserts \e string into the list and sorts the list. \sa
  insertItem(). */

void QListBox::inSort( const char *string )
{
#if defined(CHECK_NULL)
    CHECK_PTR( string );
#endif
    itemList->inSort( newAny( string, 0 ) );
    QFontMetrics fm( font() );
    int w = fm.width( string ) + 6;
    if ( w > cellWidth() )
        setCellWidth( w );
    updateNumRows( FALSE );
    if ( autoUpdate() ) {
	update(); // Optimize drawing ( find index )
    }
}

/*! Removes the item at position \e index.  \sa insertItem(). */

void QListBox::removeItem( int index )
{
    if ( !checkIndex( "removeItem", count(), index ) )
	return;
    
    bool updt = autoUpdate() && itemVisible( index );

    QLBItem *tmp = itemList->take( index );
    QFontMetrics fm( font() );
    int w = internalItemWidth( tmp, fm );
    if ( w == cellWidth() )
        updateCellWidth();
    if ( copyStrings && tmp->type == LBI_String )
	delete [] (char*)tmp->string;
    delete tmp;
    if ( updt )
	update(); // Optimize this zucker!   ###
}

/*! Returns a pointer to the string at position \e index, or a null
  pointer if the position either doesn't exist or doesn't contain a
  string. \sa pixmap(). */

const char *QListBox::string( int index ) const
{
    if ( !checkIndex( "string", count(), index ) )
	return 0;
    QLBItem *tmp = itemList->at( index );
    return tmp->type == LBI_String ? tmp->string : 0;
}

/*! Returns a pointer to the pixmap at position \e index, or a null
  pointer if the position either doesn't exist or doesn't contain a
  pixmap. \sa string(). */

QPixmap *QListBox::pixmap( int index ) const
{
    if ( !checkIndex( "pixmap", count(), index ) )
	return 0;
    QLBItem *tmp = itemList->at( index );
    return tmp->type == LBI_Pixmap ? tmp->pixmap : 0;
}

/*! Replaces the item at posistion \e index with \e string.  If \e
  index is negative or too large, changeItem() does nothing.

  \sa insertItem(), removeItem(). */

void QListBox::changeItem( const char *string, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    changeAny( string, 0, 0, index );
}

/*! Replaces the item at posistion \e index with \e pixmap.  If \e
  index is negative or too large, changeItem() does nothing.

  \sa insertItem(), removeItem(). */

void QListBox::changeItem( const QPixmap *pixmap, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    changeAny( 0, pixmap, 0, index );
}

/*! Deletes all items in the list.  \sa removeItems(), setStrList(). */

void QListBox::clear()
{
    clearList();
    if ( autoUpdate() )
	erase();
}

void QListBox::setStringCopy( bool b )
{
    if ( b == copyStrings )
	return;
    if ( count() != 0 ) {
//	if ( !b ) {
	    warning("QListBox::setStringCopy: Cannot change the value "
		    "when the list box is not empty.");
	    return;
//	}
    }
    
}

/*! Scrolls the list box so the highlighted item is in the middle of
  the visible box. */

void QListBox::centerCurrentItem()
{
    int top;
    if ( stringsOnly ) 
	top = current - numItemsVisible() / 2; // ###
    else
	top = current - numItemsVisible() / 2;
    if ( top < 0 )
	top = 0;
    int max  = maxRowOffset();
    if ( top > max )
	top = max;
    setTopItem( top );
}

/*! Returns the number of visible items.  This may change at any time
  since the user may resize the widget. */

int QListBox::numItemsVisible()
{
    return lastRowVisible() - topCell() + 1;
}

/*! Not implemented yet. */

void QListBox::setUserItems( bool b )
{
    if ( b == ownerDrawn )
	return;
    if ( itemList->count() != 0 ) {
	warning( "QListBox::setUserItems: List box not empty.");
	return;
    }
    ownerDrawn = b;
}

bool QListBox::userItems()
{
    return ownerDrawn;
}

QLBItem *QListBox::newItem()
{
    return new QLBItem;
}

void QListBox::deleteItem( QLBItem *i )
{
    delete i;
}

void QListBox::paintItem( QPainter *, int )
{
}

/*! Inserts \e lbi into the list at \e index.  If \e index is
  negative, \e lbi is inserted at the end of the list.  \sa
  insertStrList(). */

void QListBox::insertItem( const QLBItem *lbi, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
#if defined(CHECK_NULL)
    CHECK_PTR( lbi );
#endif
    insertAny( 0, 0, lbi, index );
    updateNumRows( FALSE );
    if ( autoUpdate() )
	update();
}

/*
void QListBox::inSort( const QLBItem *lbi )
{
#if defined(CHECK_NULL)
    CHECK_PTR( lbi );
#endif
    itemList->inSort( lbi );
//###    updateNumRows( FALSE );
    if ( autoUpdate() ) {
	update(); // Optimize drawing ( find index )
    }
} */

/*! Replaces the item at posistion \e index with \e lbi.  If \e
  index is negative or too large, changeItem() does nothing.

  \sa insertItem(), removeItem(). */

void QListBox::changeItem( const QLBItem *lbi, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    changeAny( 0, 0, lbi, index );
}

/*! Returns a pointer to the item at position \e index. */

QLBItem *QListBox::item( int index ) const
{
#if defined(CHECK_RANGE)
    if (!checkIndex( "item", count(), index ) )
	return 0;
#endif
    return itemList->at( index );
}

/*! Returns the height of the item at position \e l in pixels. */

int QListBox::cellHeight( long l )
{
    if ( stringsOnly ) {
        QFontMetrics fm( font() );
        return fm.lineSpacing() + 1;
    }
    QLBItem *tmp = item( l );
    if ( tmp ) {
        switch( tmp->type ) {
  	    case LBI_String: {
                QFontMetrics fm( font() );
                return fm.lineSpacing() + 1;
	    }
	    case LBI_Pixmap: {
                if ( tmp->pixmap )
                    return tmp->pixmap->height();
                else
                    return 0;
	    }
            default:
                return itemHeight( tmp );
	}
    } else {
        return 0;
    }
}

/*! Returns the walue set by setCellWidth(). \sa cellHeight(). */

int QListBox::cellWidth( long l )
{
    return QTableWidget::cellWidth();  // cellWidth is always constant
}

/*! This virtual function returns 0 in QListBox and must
  be reimplemented by subclasses that use other types. */

int QListBox::itemHeight( QLBItem *lbi )
{
    warning("QListBox::itemHeight: You must reimplement itemHeight() when you"
            " use item types different from LBI_String and LBI_Pixmap");
    return 0;
}

/*! This virtual function returns 0 in QListBox and must
  be reimplemented by subclasses that use other types. */

int QListBox::itemWidth( QLBItem *lbi )
{
    warning("QListBox::itemWidth: You must reimplement itemWidth() when you"
            " use item types different from LBI_String and LBI_Pixmap");
    return 0;
}

/*! This event handler knows about the up and down arrow keys and
  about enter.

  Up and down arrow keys make the highlighted item move and if
  necessary the list scroll.

  Enter makes the list box emit the selected() signal.

  \sa selected(), setTopItem(). */

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

/*! Sets the highlighted item to the item at position \e index.  The
  highlighting is moved and list scrolled as necessary.  */

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

/*! Returns the number of items in the list box. */

int QListBox::count() const
{
    return itemList->count();
}

/*! Returns TRUE if the item at position \e index is at least partly
  visible. */
bool QListBox::itemVisible( int index )
{
    return rowIsVisible( index );
}

/*! Repaints the cell at position \e row using \e p.  The \e column
  argument is ignored, it is present because QTableWidget is more
  general.

  \sa QTableWidget::paintCell(). */

void QListBox::paintCell( QPainter *p, long row, long column )
{
    QFontMetrics fm = fontMetrics();

#if defined( DEBUG )
//    if ( column )
//	debug( "QListBox::paintCell: Column = %i!", column );
#endif

    if ( ownerDrawn ) {
	paintItem( p, row );
	return;
    }

    QLBItem *lbi = itemList->at( row );
    if ( !lbi )
	return;
    if ( lbi->type != LBI_String && lbi->type != LBI_Pixmap ) {
        warning( "QListBox::paintCell: illegal item type (%i) in"
                 " non-ownerdrawn list box." );
        return;
    }
    QColorGroup g = colorGroup();
    if ( current == row ) {
	p->fillRect( 0, 0, cellWidth( column ), cellHeight( row ), g.text() );
	p->pen().setColor( g.background() );
    } else {
	p->pen().setColor( g.text() );
    }
    if ( lbi->type == LBI_String )
        p->drawText( 3, cellHeight( row ) - fm.descent() - 1, lbi->string );
    if ( lbi->type == LBI_Pixmap )
        p->drawPixmap( 3, 2, *lbi->pixmap );
}

/*! This event handler moves the highlight to the clicked item, and
  ignores mouse clicks that do not fall on an item. */

void QListBox::mousePressEvent( QMouseEvent *e )
{
    int itemClicked = findItem( e->pos().y() );
    if ( itemClicked != -1 ) {
	setCurrentItem( itemClicked );
    }
}

void QListBox::mouseMoveEvent( QMouseEvent *e )
{
    if ( doDrag ) {
	int itemClicked = findItem( e->pos().y() );
	if ( itemClicked != -1 ) {
	    if ( isTiming ) {
		killTimers();
		isTiming = FALSE;
	    }
            setCurrentItem( itemClicked ); //returns at once if already current
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

void QListBox::mouseReleaseEvent( QMouseEvent *e )
{
    if ( doDrag )
	mouseMoveEvent( e );
    if ( isTiming ) {
	killTimers();
	isTiming = FALSE;
    }
}

/*! This event handler emits the selected() signal for the
  double-clicked item.  It ignores clicks that don't fall on an
  item. */

void QListBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mouseReleaseEvent( e );
    if ( currentItem() >= 0 )
	emit selected( currentItem());
}

void QListBox::resizeEvent( QResizeEvent *e )
{
    QTableWidget::resizeEvent( e );
    updateCellWidth();
}

void QListBox::timerEvent( QTimerEvent *e )
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

/*! Deletes all items in the list box. */

void QListBox::clearList()
{
    stringsOnly = TRUE;
    QLBItem *tmp;
    while( itemList->count() ) {
	tmp = itemList->take( 0 );
	if ( !ownerDrawn && copyStrings && tmp->type == LBI_String )
	    delete (char *)tmp->string;
	deleteItem( tmp );
    }
    bool a = autoUpdate();
    setAutoUpdate( FALSE );
    setTopCell( 0 );
    setAutoUpdate( a );
}

QLBItem *QListBox::newAny( const char *s, const QPixmap *bm )
{
#if defined(DEBUG)
    if ( !s && !bm )
	debug( "QListBox::newAny: Both s and bm are NULL" );
#endif
    QLBItem *tmp = newItem();
    if ( s ) {
	if ( copyStrings )
	    tmp->string = strdup( s );
	else
	    tmp->string = s;
	tmp->type = LBI_String;
    } else {
	tmp->pixmap = (QPixmap *)bm;
	tmp->type   = LBI_Pixmap;
    }
    return tmp;
}

void QListBox::insertAny( const char *s, const QPixmap *bm, 
			  const QLBItem *lbi, int index,
                          bool updateCellWidth  )
{
#if defined(DEBUG)
    if ( index > count() )
	debug( "QListBox::insertAny: index %i, out of range", index );
    if ( !s && !bm && !lbi )
	debug( "QListBox::insertAny: All arguments are NULL " );
#endif
    if ( !lbi )
	lbi = newAny( s, bm );
    itemList->insert( index, lbi );
    QFontMetrics fm( font() );
    if ( updateCellWidth ) {
        int w = internalItemWidth( lbi, fm );
        if ( w > cellWidth() )
            setCellWidth( w );
    }
}

void QListBox::changeAny( const char *s, const QPixmap *bm, 
			  const QLBItem *lbi, int index )
{
#if defined(DEBUG)
    if ( index > count() )
	debug( "QListBox::changeAny: index %i, out of range", index );
    if ( !s && !bm )
	debug( "QListBox::changeAny: Both s and bm are NULL " );
#endif

    QLBItem *tmp = itemList->take( index );
    QFontMetrics fm( font() );
    int w = internalItemWidth( tmp, fm );
    if ( w == cellWidth() )
        updateCellWidth();
    if ( copyStrings && tmp->type == LBI_String )
	delete [] (char*)tmp->string;
    deleteItem( tmp );
    insertAny( s, bm, lbi, index );
}

void QListBox::updateNumRows( bool updateWidth )
{
    bool autoU = autoUpdate();
    if ( autoU )
	setAutoUpdate( FALSE );
    setNumRows( itemList->count() );
    if ( updateWidth ) 
        updateCellWidth();
    if ( autoU )
        setAutoUpdate( TRUE );
}

void QListBox::updateCellWidth()
{
    QLBItem *tmp = itemList->first();
    QFontMetrics fm( font() );
    int maxW = windowWidth();
    int w;
    while ( tmp ) {
	w = internalItemWidth( tmp, fm );
	if ( w > maxW )
	    maxW = w;
	tmp = itemList->next();
    }
    setCellWidth( maxW );    
}

int QListBox::internalItemWidth( const QLBItem      *lbi,
                                 const QFontMetrics &fm ) const
{
    int w;
    switch( lbi->type ) {
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

void QListBox::init()
{
}
