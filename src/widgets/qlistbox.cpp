/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.cpp#93 $
**
** Implementation of QListBox widget class
**
** Created : 941121
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlistbox.h"
#include "qfontmet.h"
#include "qpainter.h"
#include "qstrlist.h"
#include "qkeycode.h"
#include "qpixmap.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qlistbox.cpp#93 $");


Q_DECLARE(QListM, QListBoxItem);

class QLBItemList : public QListM(QListBoxItem) // internal class
{
    int compareItems( GCI i1, GCI i2);
};

int QLBItemList::compareItems( GCI i1, GCI i2)
{
    QListBoxItem *lbi1 = (QListBoxItem *)i1;
    QListBoxItem *lbi2 = (QListBoxItem *)i2;
    return strcmp( lbi1->text(), lbi2->text() );
}


static inline bool checkInsertIndex( const char *method, int count, int *index)
{
    bool range_err = (*index > count);
#if defined(CHECK_RANGE)
    if ( *index > count )
	warning( "QListBox::%s: Index %i out of range", method, *index );
#endif
    if ( *index < 0 )				// append
	*index = count;
    return !range_err;
}

static inline bool checkIndex( const char *method, int count, int index )
{
    bool range_err = (index >= count);
#if defined(CHECK_RANGE)
    if ( range_err )
	warning( "QListBox::%s: Index %d out of range", method, index );
#endif
    return !range_err;
}


/*!
  \class QListBoxItem qlistbox.h
  \brief This is the base class of all list box items.

  \ingroup abstract

  This class is the abstract base class of all list box items. If you
  need to insert customized items into a QListBox, you must inherit
  this class and reimplement paint(), height() and width().

  The following shows how to define a list box item which shows a
  pixmap and a text:
  \code
    class MyListBoxItem : public QListBoxItem
    {
    public:
	MyListBoxItem( const char *s, const QPixmap p )
	    : QListBoxItem(), pm(p)
	    { setText( s ); }

    protected:
	virtual void paint( QPainter * );
	virtual int height( const QListBox * ) const;
	virtual int width( const QListBox * ) const;
	virtual const QPixmap *pixmap() { return &pm; }

    private:
	QPixmap pm;
    };

    void MyListBoxItem::paint( QPainter *p )
    {
	p->drawPixmap( 3, 0, pm );
	QFontMetrics fm = p->fontMetrics();
	int yPos;			// vertical text position
	if ( pm.height() < fm.height() )
	    yPos = fm.ascent() + fm.leading()/2;
	else
	    yPos = pm.height()/2 - fm.height()/2 + fm.ascent();
	p->drawText( pm.width() + 5, yPos, text() );
    }

    int MyListBoxItem::height(const QListBox *lb ) const
    {
	return QMAX( pm.height(), lb->fontMetrics().lineSpacing() + 1 );
    }

    int MyListBoxItem::width(const QListBox *lb ) const
    {
	return pm.width() + lb->fontMetrics().width( text() ) + 6;
    }
  \endcode

  \sa QListBox
*/


/*!
  Constructs an empty list box item.
*/

QListBoxItem::QListBoxItem()
{
    selected = FALSE;
}

/*!
  Destroys the list box item.
*/

QListBoxItem::~QListBoxItem()
{
}


/*!
  \fn void QListBoxItem::paint( QPainter *p )

  Implement this function to draw your item.

  \sa height(), width()
*/

/*!
  \fn int QListBoxItem::height( const QListBox * ) const

  Implement this function to return the height of your item

  \sa paint(), width()
*/

/*!
  \fn int QListBoxItem::width(	const QListBox * ) const

  Implement this function to return the width of your item

  \sa paint(), height()
*/

/*!
  \fn const char *QListBoxItem::text() const

  Returns the text of the item, which is used for sorting.

  \sa setText()
*/

/*!
  \fn const QPixmap *QListBoxItem::pixmap() const

  Returns the pixmap connected with the item, if any.

  The default implementation of this function returns a null pointer.
*/


/*!
  \fn void QListBoxItem::setText( const char *text )

  Sets the text of the widget, which is used for sorting.
  The text is not shown unless explicitly drawn in paint().

  \sa text()
*/


/*!
  \class QListBoxText qlistbox.h
  \brief QListBoxText provides list box items with text.

  The text is drawn in the widget's current font. If you need several
  different fonts, you have to make your own subclass of QListBoxItem.

  \sa QListBox, QListBoxItem
*/


/*!
  Constructs a list box item showing the text \e text.
*/

QListBoxText::QListBoxText( const char *text )
    :QListBoxItem()
{
    setText( text );
}

/*!
  Destroys the item.
*/

QListBoxText::~QListBoxText()
{
}

/*!
  Draws the text using painter \e p.
*/

void QListBoxText::paint( QPainter *p )
{
    QFontMetrics fm = p->fontMetrics();
    p->drawText( 3,  fm.ascent() + fm.leading()/2, text() );
}

/*!
  Returns the height of a line of text.

  \sa paint(), width()
*/

int QListBoxText::height( const QListBox *lb ) const
{
    if ( lb )
	return lb->fontMetrics().lineSpacing() + 2;
    return -1;
}

/*!
  Returns the width of this line.

  \sa paint(), height()
*/

int QListBoxText::width( const QListBox *lb ) const
{
    if ( lb )
	return lb->fontMetrics().width( text() ) + 6;
    return -1;
}


/*!
  \class QListBoxPixmap qlistbox.h
  \brief QListBoxPixmap provides list box items with a pixmap.

  \sa QListBox, QListBoxItem
*/

/*!
  Creates a new list box item showing the pixmap \e pixmap.
*/

QListBoxPixmap::QListBoxPixmap( const QPixmap &pixmap )
    : QListBoxItem()
{
    pm = pixmap;
}

/*!
  Destroys the item.
*/

QListBoxPixmap::~QListBoxPixmap()
{
}

/*!
  \fn const QPixmap *QListBoxPixmap::pixmap() const

  Returns the pixmap connected with the item.
*/


/*!
  Draws the pixmap using painter \e p.
*/

void QListBoxPixmap::paint( QPainter *p )
{
    p->drawPixmap( 3, 0, pm );
}

/*!
  Returns the height of the pixmap.

  \sa paint(), width()
*/

int QListBoxPixmap::height( const QListBox * ) const
{
    return pm.height();
}

/*!
  Returns the width of the pixmap.

  \sa paint(), height()
*/

int QListBoxPixmap::width( const QListBox * ) const
{
    return pm.width() + 6;
}


/*!
  \class QListBox qlistbox.h
  \brief The QListBox widget provides a single-column list of items that
  can be scrolled.

  \ingroup realwidgets

  Each item in a QListBox contains a QListBoxItem.  One of the items
  can be the current item.  The highlighted() signal is emitted when
  the user highlights a new current item; selected() is emitted when
  the user double-clicks on an item or presses return when an item is
  highlighted.

  If the user does not select anything, no signals are emitted and
  currentItem() returns -1.

  A list box has \c StrongFocus as a default focusPolicy(), i.e. it can 
  get keyboard focus both by tabbing and clicking.

  New items may be inserted using either insertItem(), insertStrList()
  and inSort().

  By default, vertical and horizontal scroll bars are added and
  removed as necessary.	 setAutoScrollBar() can be used to force a
  specific policy.

  If you need to insert other types than texts and pixmaps, you must
  define new classes which inherit QListBoxItem.

  \warning The list box assumes ownership of all list box items
  and will delete them when they are not needed.
*/



//### How to provide new member variables while keeping binary compatibility:

#include <qintdict.h>

static QIntDict<int> *qlb_maxLenDict;

/*!
  Constructs a list box.  The arguments are passed directly to the
  QTableView constructor.
*/

QListBox::QListBox( QWidget *parent, const char *name, WFlags f )
    : QTableView( parent, name, f )
{
    initMetaObject();
    doDrag	  = TRUE;
    doAutoScroll  = TRUE;
    current	  = -1;
    isTiming	  = FALSE;
    stringsOnly	  = TRUE;
    goingDown	  = FALSE;
    itemList	  = new QLBItemList;
    CHECK_PTR( itemList );
    setCellWidth( 0 );
    QFontMetrics fm = fontMetrics();
    setCellHeight( fm.lineSpacing() + 1 );
    setNumCols( 1 );
    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   Tbl_smoothVScrolling | Tbl_clipCellPainting );
    switch ( style() ) {
	case WindowsStyle:
	case MotifStyle:
	    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	    setBackgroundColor( colorGroup().base() );
	    break;
	default:
	    setFrameStyle( QFrame::Panel | QFrame::Plain );
	    setLineWidth( 1 );
    }
    setFocusPolicy( StrongFocus );
    if ( !qlb_maxLenDict )
	qlb_maxLenDict = new QIntDict<int>;
    ASSERT( qlb_maxLenDict );
}

/*!
  Destroys the list box.  Deletes all list box items.
*/

QListBox::~QListBox()
{
    goingDown = TRUE;
    clearList();
    if ( qlb_maxLenDict )
	qlb_maxLenDict->remove( (long)this );
    delete itemList;
}

/*!
  \fn void QListBox::highlighted( int index )
  This signal is emitted when the user highlights a new current item.

  \sa selected()
*/

/*!
  \fn void QListBox::selected( int index )
  This signal is emitted when the user double-clicks on an item
  or presses return when an item is highlighted.

  \sa highlighted()
*/

/*!
  Reimplements QWidget::setFont() to update the list box line height.
*/

void QListBox::setFont( const QFont &font )
{
    QWidget::setFont( font );
    if ( stringsOnly )
	setCellHeight( fontMetrics().lineSpacing() + 1 );
}


/*!
  Returns the number of items in the list box.
*/

uint QListBox::count() const
{
    return itemList->count();
}


/*!
  Inserts the string list \e list into the list at item \e index.

  If \e index is negative, \e list is inserted at the end of the list.	If
  \e index is too large, the operation is ignored.

  \sa insertItem(), inSort()
*/

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
    const char *txt;
    if ( index < 0 )
	index = itemList->count();
    while ( (txt=it.current()) ) {
	++it;
	QListBoxText *tmp = new QListBoxText( txt );
	insertDangerously( tmp, index++, FALSE );
    }
    updateNumRows( TRUE );
    if ( autoUpdate() && isVisible() )
	repaint();
}

/*!
  Inserts the \e numStrings strings of the array \e strings into the
  list at item\e index.

  If \e index is negative, insertStrList() inserts \e strings at the end
  of the list.	If \e index is too large, the operation is ignored.

  \sa insertItem(), inSort()
*/

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
	QListBoxText *tmp = new QListBoxText( strings[i] );
	insertDangerously( tmp, index + i, FALSE );
	i++;
    }
    updateNumRows( TRUE );
    if ( autoUpdate() && isVisible() )
	repaint();
}


/*!
  Inserts the item \e lbi into the list at \e index.

  If \e index is negative or larger than the number of items in the list
  box, \e lbi is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const QListBoxItem *lbi, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
    if ( !lbi ) {
#if defined ( CHECK_NULL )
	ASSERT( lbi != 0 );
#endif
	return;
    }
    if ( stringsOnly ) {
	stringsOnly = FALSE;
	setCellHeight( 0 );
    }
    insertDangerously( lbi, index, TRUE );
    updateNumRows( FALSE );
    if ( autoUpdate() )
	repaint();
}

/*!
  Inserts \e text into the list at \e index.

  If \e index is negative, \e text is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const char *text, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
    if ( !text ) {
#if defined ( CHECK_NULL )
	ASSERT( text != 0 );
#endif
	return;
    }
    QListBoxText *tmp = new QListBoxText( text );
    insertDangerously( tmp, index, TRUE );
    updateNumRows( FALSE );
    if ( autoUpdate() && itemVisible(index) ) {
	int x, y;
	colXPos( 0, &x );
	rowYPos( index, &y );
	repaint( x, y, -1, -1 );
    }
}

/*!
  Inserts \e pixmap into the list at \e index.

  If \e index is negative, \e pixmap is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const QPixmap &pixmap, int index )
{
    if ( !checkInsertIndex( "insertItem", count(), &index ) )
	return;
    if ( stringsOnly ) {
	stringsOnly = FALSE;
	setCellHeight( 0 );
    }
    QListBoxPixmap *tmp = new QListBoxPixmap( pixmap );
    insertDangerously( tmp, index, TRUE );
    updateNumRows( FALSE );
    if ( autoUpdate() && itemVisible(index) ) {
	int x, y;
	colXPos( index, &x );
	rowYPos( index, &y );
	repaint( x, y, -1, -1 );
    }
}


/*!
  Inserts \e lbi at its sorted position in the list box.

  All items must be inserted with inSort() to maintain the sorting
  order.  inSort() treats any pixmap (or user-defined type) as
  lexicographically less than any string.

  \sa insertItem()
*/

void QListBox::inSort( const QListBoxItem *lbi )
{
    if ( !lbi->text() ) {
#if defined (CHECK_NULL)
	ASSERT( lbi->text() != 0 );
#endif
	return;
    }

    itemList->inSort( lbi );
    int index = itemList->at();
    itemList->remove();
    insertItem( lbi, index );
}


/*!
  /overload void QListBox::inSort( const char *text )
*/

void QListBox::inSort( const char *text )
{
    if ( !text ) {
#if defined ( CHECK_NULL )
	ASSERT( text != 0 );
#endif
	return;
    }
    QListBoxText lbi( text );
    itemList->inSort(&lbi);
    int index = itemList->at();
    itemList->remove();
    insertItem( text, index );
}


/*!
  Removes the item at position \e index.
  \sa insertItem(), clear()
*/

void QListBox::removeItem( int index )
{
    if ( !checkIndex( "removeItem", count(), index ) )
	return;
    if ( current >= index && current > 0 )
	current--;
    bool    updt = autoUpdate() && itemVisible( index );
    QListBoxItem *lbi = itemList->take( index );
    QFontMetrics fm   = fontMetrics();
    int w             = lbi->width( this );
    updateNumRows( w == cellWidth() );
    delete lbi;
    if ( count() == 0 )
	current = -1;
    if ( updt )
	repaint();
}

/*!
  Deletes all items in the list.
  \sa removeItem(), setStrList()
*/

void QListBox::clear()
{
    clearList();
    updateNumRows( TRUE );
    if ( autoUpdate() )
	erase();
}


/*!
  Returns a pointer to the text at position \e index, or 0 if there is no
  text there.
  \sa pixmap()
*/

const char *QListBox::text( int index ) const
{
    if ( index >= (int)count() )
	return 0;
    QListBoxItem *lbi = itemList->at( index );
    return lbi->text();
}

/*!
  Returns a pointer to the pixmap at position \e index, or 0 if there is no
  pixmap there.
  \sa text()
*/

const QPixmap *QListBox::pixmap( int index ) const
{
    if ( index >= (int)count() )
	return 0;
    QListBoxItem *lbi = itemList->at( index );
    return lbi->pixmap();
}

/*!
  Replaces the item at position \e index with \e text.

  The operation is ignored if \e index is out of range.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const char *text, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    QListBoxText *tmp = new QListBoxText( text );
    changeDangerously( tmp, index );
}

/*!
  Replaces the item at position \e index with \e pixmap.

  The operation is ignored if \e index is out of range.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QPixmap &pixmap, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    QListBoxPixmap *tmp = new QListBoxPixmap( pixmap );
    changeDangerously( tmp, index );
}


/*!
  Replaces the item at posistion \e index with \e lbi.	If \e
  index is negative or too large, changeItem() does nothing.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QListBoxItem *lbi, int index )
{
    if ( !checkIndex( "changeItem", count(), index ) )
	return;
    changeDangerously( lbi, index );
}


/*!
  Returns TRUE if the list box updates itself automatically when
  items are inserted or removed.

  The default setting is TRUE.

  \sa setAutoUpdate()
*/

bool QListBox::autoUpdate() const
{
    return QTableView::autoUpdate();
}

/*!
  Specifies whether the list box should update itself automatically
  when items are inserted or removed.

  Auto-update is enabled by default.

  If \e enable is TRUE, the list box will update itself.  If \e enable
  is FALSE, the list box will not update itself.

  \warning Do not leave the view in this state for a long time
  (i.e. between events ). If the user interacts with the view when
  auto-update is off, strange things can happen.

  \sa autoUpdate()
*/

void QListBox::setAutoUpdate( bool enable )
{
    QTableView::setAutoUpdate( enable );
}

/*!
  Returns the number of visible items.	This may change at any time
  since the user may resize the widget.
*/

int QListBox::numItemsVisible() const
{
    return (lastRowVisible() - topCell() + 1);
}

/*!
  Returns the index of the current (highlighted) item of the list box,
  or -1 if no item has been selected.

  \sa topItem()
*/

int QListBox::currentItem() const
{
    return current;
}

/*!
  Sets the highlighted item to the item at position \e index in the list.
  The highlighting is moved and the list box scrolled as necessary.
  \sa currentItem()
*/

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

/*!
  Scrolls the list box so the current (highlighted) item is
  centered in the list box.
  \sa currentItem(), setTopItem()
*/

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

/*!
  Returns index of the item that is on the top line of the list box.
  \sa setTopItem(), currentItem()
*/

int QListBox::topItem() const
{
    return topCell();
}

/*!
  Scrolls the list box so the item at position \e index in the list
  becomes the top row of the list box.
  \sa topItem(), centerCurrentItem()
*/

void QListBox::setTopItem( int index )
{
    //debug( "Set top item %d", index );
    setTopCell( index );
}


/*!
  Returns TRUE if drag-selection is enabled, otherwise FALSE.
  \sa setDragSelect(), autoScroll()
*/

bool QListBox::dragSelect() const
{
    return doDrag;
}

/*!
  Sets drag-selection if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If drag-selection is enabled, the list box will highlight new items when
  the user drags the mouse inside the list box.

  The default setting is TRUE.

  \sa drawSelect(), setAutoScroll()
*/

void QListBox::setDragSelect( bool enable )
{
    doDrag = enable;
}

/*!
  Returns TRUE if auto-scrolling is enabled, otherwise FALSE.
  \sa setAutoScroll, dragSelect()
*/

bool QListBox::autoScroll() const
{
    return doAutoScroll;
}

/*!
  Sets auto-scrolling if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If auto-scrolling is enabled, the list box will scroll its contents when
  the user drags the mouse outside (below or above) the list  box.
  Auto-scrolling only works if \link setDragSelect() drag-selection\endlink
  is enabled.

  The default setting is TRUE.

  \sa autoScroll(), setDragSelect()
*/

void QListBox::setAutoScroll( bool enable )
{
    doAutoScroll = enable;
}

/*!
  Returns TRUE if the list box has an automatic (vertical) scroll bar.
  \sa setAutoScrollBar(), autoBottomScrollBar()
*/

bool QListBox::autoScrollBar() const
{
    return testTableFlags( Tbl_autoVScrollBar );
}

/*!
  Enables an automatic (vertical) scroll bar if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  If it is enabled, then the list box will get a (vertical) scroll bar if
  the list box items exceed the list box height.

  The default setting is TRUE.

  \sa autoScrollBar(), setScrollBar(), setAutoBottomScrollBar()
*/

void QListBox::setAutoScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_autoVScrollBar );
    else
	clearTableFlags( Tbl_autoVScrollBar );
}

/*!
  Returns TRUE if the list box has a (vertical) scroll bar.
  \sa setScrollBar(), autoScrollBar(), bottomScrollBar()
*/

bool QListBox::scrollBar() const
{
    return testTableFlags( Tbl_vScrollBar );
}

/*!
  Enables a (vertical) scroll bar if \e enable is TRUE, or disables it if
  \e enable is FALSE.

  The default setting is FALSE.

  \sa scrollBar(), setAutoScrollBar(), setBottomScrollBar()
*/

void QListBox::setScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_vScrollBar );
    else
	clearTableFlags( Tbl_vScrollBar );
}

/*!
  Returns TRUE if the list box has an automatic bottom scroll bar.
  \sa setAutoBottomScrollBar(), autoScrollBar()
*/

bool QListBox::autoBottomScrollBar() const
{
    return testTableFlags( Tbl_autoHScrollBar );
}

/*!
  Enables an automatic bottom scroll bar if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  If it is enabled, then the list box will get a bottom scroll bar if the
  maximum list box item width exceeds the list box width.

  The default setting is TRUE.

  \sa autoBottomScrollBar(), setBottomScrollBar(), setAutoScrollBar()
*/

void QListBox::setAutoBottomScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_autoHScrollBar );
    else
	clearTableFlags( Tbl_autoHScrollBar );
}

/*!
  Returns TRUE if the list box has a bottom scroll bar.
  \sa setBottomScrollBar(), autoBottomScrollBar(), scrollBar()
*/

bool QListBox::bottomScrollBar() const
{
    return testTableFlags( Tbl_hScrollBar );
}

/*!
  Enables a bottom scroll bar if \e enable is TRUE, or disables it if
  \e enable is FALSE.

  The default setting is FALSE.

  \sa bottomScrollBar(), setAutoBottomScrollBar(), setScrollBar()
*/

void QListBox::setBottomScrollBar( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_hScrollBar );
    else
	clearTableFlags( Tbl_hScrollBar );
}

/*!
  Returns TRUE if smooth list box scrolling is enabled, otherwise FALSE.
  \sa setSmoothScrolling()
*/

bool QListBox::smoothScrolling() const
{
    return testTableFlags( Tbl_smoothVScrolling );
}

/*!
  Enables smooth list box scrolling if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  The default setting is TRUE.

  \sa smoothScrolling()
*/

void QListBox::setSmoothScrolling( bool enable )
{
    if ( enable )
	setTableFlags( Tbl_smoothVScrolling );
    else
	clearTableFlags( Tbl_smoothVScrolling );
}


/*!
  Returns a pointer to the item at position \e index.
*/

QListBoxItem *QListBox::item( int index ) const
{
    if (!checkIndex( "item", count(), index ) )
	return 0;
    return itemList->at( index );
}

/*!
  Returns the height of the item at position \e index in pixels.
*/

int QListBox::cellHeight( int index )
{
    if ( stringsOnly )
	return QTableView::cellHeight();
    QListBoxItem *lbi = item( index );
    return lbi ? lbi->height(this) : 0;
}


/*!
  Returns the standard item height (in pixels), or -1 if the list box has
  variable item height.
*/

int QListBox::itemHeight() const
{
    return stringsOnly ? ((QListBox*)this)->cellHeight( 0 ) : -1;
}


/*!
  Returns the height (in pixels) of item at \e index.
*/

int QListBox::itemHeight( int index ) const
{
    return ((QListBox*)this)->cellHeight( index );
}

/*!
  Returns TRUE if the item at position \e index is at least partly
  visible.
*/

bool QListBox::itemVisible( int index )
{
    return rowIsVisible( index );
}

/*!
  Repaints the cell at position \e row using \e p.  The \e col
  argument is ignored, it is present because QTableView is more
  general. This function has the responsibility of showing focus
  and highlighting.

  \sa QTableView::paintCell()
*/

void QListBox::paintCell( QPainter *p, int row, int col )
{
    QListBoxItem *lbi = itemList->at( row );
    if ( !lbi )
	return;

    //debug( "painting cell %d, focus %u, select %u, %s", row, hasFocus(),
    //	      row == current, lbi->text() );

    QColorGroup g = colorGroup();
    if ( current == row ) {
	QColor	 fc;				// fill color
	if ( style() == WindowsStyle )
	    fc = darkBlue;			// !!!hardcoded
	else
	    fc = g.text();
	if ( hasFocus() ) {
	    //bool clip =  p->hasClipping();
	    //p->setClipping( FALSE );
	    p->fillRect( 0, 0, cellWidth(col), cellHeight(row), fc );
	    //p->setClipping( clip );
	}
	else
	    p->fillRect( 1, 1, cellWidth(col) - 2, cellHeight(row) - 2, fc );
	p->setPen( g.base() );
	p->setBackgroundColor( g.text() );
    } else {
	p->setBackgroundColor( g.base() );
	p->setPen( g.text() );
    }
    lbi->paint( p );
    if ( current == row && hasFocus() ) {
	if ( style() == WindowsStyle ) {
	    p->drawWinFocusRect( 1, 1, viewWidth()-2 , cellHeight(row)-2 );
	} else {
	    p->setPen( g.base() );
	    p->setBrush( NoBrush );
	    p->drawRect( 1, 1, viewWidth()-2 , cellHeight(row)-2 );
	}
    }
    p->setBackgroundColor( g.base() );
    p->setPen( g.text() );
}

/*!
  Handles mouse press events.  Makes the clicked item the current item.
  \sa currentItem()
*/

void QListBox::mousePressEvent( QMouseEvent *e )
{
    int itemClicked = findItem( e->pos().y() );
    if ( itemClicked != -1 ) {
	setCurrentItem( itemClicked );
    } else if ( contentsRect().contains( e->pos() ) &&
		lastRowVisible() >= (int) count() ) {
	setCurrentItem( count()-1 );
    }
}

/*!
  Handles mouse release events.
*/

void QListBox::mouseReleaseEvent( QMouseEvent *e )
{
    if ( doDrag )
	mouseMoveEvent( e );
    if ( isTiming ) {
	killTimers();
	isTiming = FALSE;
    }
}

/*!
  Handles mouse double click events.  Emits the selected() signal for
  the item that was double-clicked.
*/

void QListBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    mouseReleaseEvent( e );
    if ( currentItem() >= 0 )
	emit selected( currentItem());
}

/*!
  Handles mouse move events.  Scrolls the list box if auto-scroll
  is enabled.
  \sa autoScroll()
*/

void QListBox::mouseMoveEvent( QMouseEvent *e )
{
    if ( doDrag && (e->state() & (RightButton|LeftButton|MidButton)) != 0 ) {
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

/*!
  Handles key press events.

  \c Up and \c down arrow keys make the highlighted item move and if
  necessary scroll the list box.

  \c Enter makes the list box emit the selected() signal.

  \sa selected(), setCurrentItem()
*/

void QListBox::keyPressEvent( QKeyEvent *e )
{
    if ( numRows() == 0 )
	return;
    if ( currentItem() < 0 )
	setCurrentItem( topItem() );

    //debug("Last row visible is %d", lastRowVisible() );
    //debug("Top row visible is %d", topItem() );
    //debug("Current item is %d", currentItem() );

    int pageSize, delta;

    switch ( e->key() ) {
	case Key_Up:
	    if ( currentItem() > 0 ) {
		setCurrentItem( currentItem() - 1 );
		//debug("Current item is %d", currentItem() );
		if ( currentItem() < topItem()	)
		    setTopItem( currentItem() );
	    }
	    break;
	case Key_Down:
	    if ( currentItem() < (int)count() - 1 ) {
		setCurrentItem( currentItem() + 1 );
		//debug("Current item is %d", currentItem() );
		if ( currentItem() > lastRowVisible() )
		    setTopItem( topItem() + currentItem() - lastRowVisible() );
	    }
	    //### tableview !!!
	    //	    while (  currentItem() > lastRowVisible() ) {
	    //setTopItem( topItem() + 1 );
	    //}
	    break;
	case Key_Next:
	    delta = currentItem() - topItem();
	    pageSize = lastRowVisible() - topItem();
	    setTopItem( QMIN( topItem() + pageSize, (int)count() - 1 ) );
	    setCurrentItem( QMIN( topItem() + delta, (int)count() - 1 ) );
	    //debug("Current item is %d", currentItem() );
	    break;
	case Key_Prior:
	    delta = currentItem() - topItem();
	    pageSize = lastRowVisible() - topItem();
	    setTopItem( QMAX( topItem() - pageSize, 0 ) );
	    setCurrentItem( QMAX( topItem() + delta, 0 ) );
	    //debug("Current item is %d", currentItem() );
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


/*!
  Handles focus events.	 Repaints, and sets the current item to
  first one if there is no current item.
  \sa keyPressEvent(), focusOutEvent()
*/

void QListBox::focusInEvent( QFocusEvent * )
{
    if ( currentItem() < 0 && numRows() > 0 )
	setCurrentItem( topItem() );
    updateCell( currentItem(), 0); //show focus
}


/*!
  Handles resize events.  Updates internal parameters for the new list box
  size.
*/

void QListBox::resizeEvent( QResizeEvent *e )
{
    QTableView::resizeEvent( e );
    setCellWidth( QMAX(maxItemWidth(), viewWidth()) );
}

/*!
  Handles timer events.	 Does auto-scrolling.
*/

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


/*!
  Returns the vertical pixel-coordinate in \e *yPos, of the list box
  item at position \e index in the list.  Returns FALSE if the item is
  outside the visible area.

  \sa findItem
*/

bool QListBox::itemYPos( int index, int *yPos ) const
{

    return rowYPos( index, yPos );
}

/*!
  Returns the index of the list box item at the vertical pixel-coordinate
  \e yPos.

  \sa itemYPos()
*/

int QListBox::findItem( int yPos ) const
{
    return findRow( yPos );
}

/*!
  Repaints the item at position \e index in the list.  Erases the line
  first if \e erase is TRUE.
*/

void QListBox::updateItem( int index, bool erase )
{
    updateCell( index, 0,  erase );
}

/*!
  Deletes all items in the list.  Protected function that does NOT
  update the list box.
*/

void QListBox::clearList()
{
    stringsOnly = TRUE;
    QListBoxItem *lbi;
    while ( itemList->count() ) {
	lbi = itemList->take( 0 );
	delete lbi;
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

/*!
  Traverses the list and finds an item with the maximum width, and
  updates the internal list box structures accordingly.
*/

void QListBox::updateCellWidth()
{
    QListBoxItem *lbi = itemList->first();
    QFontMetrics fm = fontMetrics();
    int maxW = 0;
    int w;
    while ( lbi ) {
	w = lbi->width( this );
	if ( w > maxW )
	    maxW = w;
	lbi = itemList->next();
    }
    setMaxItemWidth( maxW );
    setCellWidth( QMAX( maxW, viewWidth() ) );
}

/*!
  \internal
  Inserts a new list box item.

  The caller must also call update() if autoUpdate() is TRUE.
*/

void QListBox::insertDangerously( const QListBoxItem *lbi, int index,
				  bool updateCellWidth	)
{
#if defined(CHECK_RANGE)
    if ( (uint)index > count() )
	warning( "QListBox::insert: Index %d out of range", index );
#endif
    itemList->insert( index, lbi );
    if ( current == index )
	current++;
    if ( updateCellWidth ) {
	int w = lbi->width( this );
	if ( w > maxItemWidth() )
	    setMaxItemWidth( w );
	if ( w > cellWidth() ) {
	    setCellWidth( w );
	}
    }
}

/*!
  \internal
  Changes a list box item.
*/

void QListBox::changeDangerously( const QListBoxItem *lbi, int index )
{
#if defined(CHECK_RANGE)
    if ( (uint)index > count() )
	warning( "QListBox::change: Index %d out of range", index );
#endif
    QListBoxItem *old = itemList->take( index );
    QFontMetrics fm = fontMetrics();
    int w = old->width( this );
    if ( w == cellWidth() )
	updateCellWidth();
    int h = cellHeight( index );
    delete old;
    int oldpos = current;
    insertDangerously( lbi, index, TRUE );
    current = oldpos;
    int nh = cellHeight( index );
    int y;
    // ### the update rectangles are dubious
    if ( autoUpdate() && rowYPos( index, &y ) ) {
	if ( nh == h )
	    repaint( frameWidth(), y, viewWidth(), h );
	else
	    repaint( frameWidth(), y, viewWidth(), viewHeight() - y );
    }
}

/*!
  \internal
  Updates the num-rows setting in the table view.
*/

void QListBox::updateNumRows( bool updateWidth )
{
    bool autoU = autoUpdate();
    if ( autoU )
	setAutoUpdate( FALSE );
    bool sbBefore = testTableFlags( Tbl_vScrollBar );
    setNumRows( itemList->count() );
    if ( updateWidth || sbBefore != testTableFlags(Tbl_vScrollBar) )
	updateCellWidth();
    if ( autoU )
	setAutoUpdate( TRUE );
}


/*!
  Returns the width in pixels of the longest item. 
*/

long QListBox::maxItemWidth()
{
    if ( !qlb_maxLenDict )
	return 0;
    return (long) qlb_maxLenDict->find( (long)this );
}


/*!
  Updates the cached value used by maxItemWidth().
*/

void QListBox::setMaxItemWidth( int len )
{
    ASSERT( qlb_maxLenDict );
    qlb_maxLenDict->remove( (long)this );
    if ( len )
	qlb_maxLenDict->insert( (long)this, (int*)len );
}
