/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.cpp#209 $
**
** Implementation of QListBox widget class
**
** Created : 941121
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qlistbox.h"
#include "qarray.h"
#include "qfontmetrics.h"
#include "qpainter.h"
#include "qstrlist.h"
#include "qscrollbar.h"
#include "qpixmap.h"
#include "qapplication.h"


class QListBoxPrivate
{
public:
    QListBoxPrivate():
	head( 0 ), current( 0 ),
	layoutDirty( TRUE ),
	dragging( FALSE ),
	variableHeight( TRUE /* !!! ### FALSE */ ),
	variableWidth( FALSE ),
	rowModeWins( FALSE ),
	rowMode( QListBox::FixedNumber ), columnMode( QListBox::FixedNumber ),
	numRows( 1 ), numColumns( 1 ),
	currentRow( 0 ), currentColumn( 0 ),
	mousePressRow( -1 ), mousePressColumn( -1 ),
	scrollTimer( 0 ), updateTimer( 0 ),
	selectionMode( QListBox::Single ),
	count( 0 )
    {}
    ~QListBoxPrivate();

    QListBoxItem * head;
    QListBoxItem * current;
    bool layoutDirty;
    bool dragging;
    bool variableHeight;
    bool variableWidth;

    QArray<int> columnPos;
    QArray<int> rowPos;

    bool rowModeWins;
    QListBox::LayoutMode rowMode;
    QListBox::LayoutMode columnMode;
    int numRows;
    int numColumns;

    int currentRow;
    int currentColumn;
    int mousePressRow;
    int mousePressColumn;

    QTimer * scrollTimer;
    QTimer * updateTimer;

    QPoint scrollPos;

    QListBox::SelectionMode selectionMode;

    int count;
};


QListBoxPrivate::~QListBoxPrivate()
{
    ASSERT( !head );
}


/*!
  \class QListBoxItem qlistbox.h
  \brief This is the base class of all list box items.

  This class is the abstract base class of all list box items. If you
  need to insert customized items into a QListBox, you must inherit
  this class and reimplement paint(), height() and width().

  The following shows how to define a list box item which shows a
  pixmap and a text:
  \code
    class MyListBoxItem : public QListBoxItem
    {
    public:
	MyListBoxItem( QString s, const QPixmap p )
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
    s = FALSE;
    dirty = TRUE;
    p = n = 0;

    // just something that'll look noticeable in the debugger
    x = y = 42;
}

/*!
  Destroys the list box item.
*/

QListBoxItem::~QListBoxItem()
{
    if ( p && p->n == this )
	p->n = p;
    if ( n && n->p == this )
	n->p = n;
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
  Returns the text of the item, which is used for sorting.

  \sa setText()
*/
QString QListBoxItem::text() const
{
    return txt;
}

/*!
  Returns the pixmap connected with the item, if any.

  The default implementation of this function returns a null pointer.
*/
const QPixmap *QListBoxItem::pixmap() const
{
    return 0;
}


/*!
  \fn void QListBoxItem::setText( const QString &text )

  Sets the text of the widget, which is used for sorting.
  The text is not shown unless explicitly drawn in paint().

  \sa text()
*/


/*!
  \class QListBoxText qlistbox.h
  \brief The QListBoxText class provides list box items with text.

  The text is drawn in the widget's current font. If you need several
  different fonts, you have to make your own subclass of QListBoxItem.

  \sa QListBox, QListBoxItem
*/


/*!
  Constructs a list box item showing the text \a text.
*/

QListBoxText::QListBoxText( const QString &text )
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
  Draws the text using painter \a p.
*/

void QListBoxText::paint( QPainter *p )
{
    QFontMetrics fm = p->fontMetrics();
    p->drawText( 3, fm.ascent() + fm.leading()/2, text() );
}

/*!
  Returns the height of a line of text.

  \sa paint(), width()
*/

int QListBoxText::height( const QListBox * l ) const
{
    return l ? l->fontMetrics().lineSpacing() + 2 : 0;
}

/*!
  Returns the width of this line.

  \sa paint(), height()
*/

int QListBoxText::width( const QListBox * l ) const
{
    return l ? l->fontMetrics().width( text() ) + 6 : 0;
}


/*!
  \class QListBoxPixmap qlistbox.h
  \brief The QListBoxPixmap class provides list box items with a pixmap.

  \sa QListBox, QListBoxItem
*/

/*!
  Creates a new list box item showing the pixmap \a pixmap.
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
  Draws the pixmap using painter \a p.
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
  \brief The QListBox widget provides a list of selectable, read-only items.

  \ingroup realwidgets

  The list of items can be arbitrarily big; if necessary, QListBox
  adds scroll bars.  It can be single-column (as most list boxes are)
  or multi-column, and offers both single and multiple selection.
  (QListBox does however not support multiple-column items; QListView
  does that job.)

  The list box items can be accessed both as QListBoxItem objects
  (recommended) and using integer indexes (the original QListBox
  implementation used an array of strings internally, and the API
  still supports this mode of operation).  Everything can be done
  using the new objects; most things can be done using the indexes too
  but unfortunately not everything.

  Each item in a QListBox contains a QListBoxItem.  One of the items
can be the current item.  The highlighted() signal is emitted when the
user highlights a new current item; selected() is emitted when the
user double-clicks on an item or presses return when an item is
highlighted.

  If the user does not select anything, no signals are emitted and
  currentItem() returns -1.

  A list box has \c StrongFocus as a default focusPolicy(), i.e. it can
  get keyboard focus both by tabbing and clicking.

  New items may be inserted using either insertItem(), insertStrList()
  and inSort().  The list box is automatically updated to reflect the
  change; if you are going to insert a lot of data it may be
  worthwhile to wrap the insertion in calls to setAutoUpdate():

  \code
      listBix->setAutoUpdate( FALSE );
      for( i=1; i< hugeArray->count(); i++ )
          listBox->insertItem( hugeArray[i] );
      listBox->setAutoUpdate( TRUE );
      listBox->repaint();
  \endcode

  Each change to insertItem() normally causes a screen update, and for
  a large change only a few of those updates are really necessary.  Be
  careful to call repaint() when you re-enable updates, so the widget
  is completely repainted.

  By default, vertical and horizontal scroll bars are added and
  removed as necessary.	 setAutoScrollBar() can be used to force a
  specific policy.

  If you need to insert other types than texts and pixmaps, you must
  define new classes which inherit QListBoxItem.

  \warning The list box assumes ownership of all list box items
  and will delete them when is does  not need them any more.

  <img src=qlistbox-m.gif> <img src=qlistbox-w.gif>

  \sa QListView QComboBox QButtonGroup
  <a href="guibooks.html#fowler">GUI Design Handbook: List Box (two
  sections)</a>
*/



/*!
  Constructs a list box.  The arguments are passed directly to the
  QScrollView constructor.

*/

QListBox::QListBox( QWidget *parent, const char *name, WFlags f )
    : QScrollView( parent, name, f )
{
    d = new QListBoxPrivate;
    d->updateTimer = new QTimer( this );
    connect( d->updateTimer, SIGNAL(timeout()),
	     this, SLOT(refreshSlot()) );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken ); // ### win/motif
    setBackgroundMode( PaletteBase );
    setFocusPolicy( StrongFocus );
    viewport()->setBackgroundMode( PaletteBase );
}


QListBox * QListBox::changedListBox = 0;

/*!
  Destroys the list box.  Deletes all list box items.
*/

QListBox::~QListBox()
{
    if ( changedListBox == this )
	changedListBox = 0;
    clear();
    delete d;
    d = 0;
}

/*! \fn void QListBox::selectionChanged()

  This signal is emitted when the selection set of a multiple-choice
  listbox changes. If the user selects five items by drag-selecting,
  QListBox tries to emit just one selectionChanged() signal, so the
  signal can be connected to computationally expensive slots.

  \sa selected() currentItem()
*/

/*! \fn void QListBox::highlighted( int index )

  This signal is emitted when the user highlights a new current item.
  The argument is the index of the new item, which is already current.

  \sa selected() currentItem() selectionChanged()
*/

/*! \fn void QListBox::highlighted( const QString &)

  This signal is emitted when the user highlights a new current item
  and the new item is a string.  The argument is the text of the
  new current item.

  \sa selected() currentItem() selectionChanged()
*/

/*! \fn void QListBox::selected( int index )

  This signal is emitted when the user double-clicks on an item or
  presses return when an item is highlighted.  The argument is the
  index of the selected item.

  \sa highlighted() selectionChanged()
*/

/*! \fn void QListBox::selected( const QString &)

  This signal is emitted when the user double-clicks on an item or
  presses return while an item is highlighted, and the selected item
  is (or has) a string.  The argument is the text of the selected
  item.

  \sa highlighted() selectionChanged()
*/

/*! \reimp */

void QListBox::setFont( const QFont &font )
{
    QScrollView::setFont( font );
    triggerUpdate( TRUE );
}


/*! Returns the number of items in the list box. */

uint QListBox::count() const
{
    if ( d->count >= 0 )
	return d->count;

    int c = 0;
    QListBoxItem * i = d->head;
    while ( i ) {
	i = i->n;
	c++;
    }
    d->count = c;
    return c;
}


/*!
  Inserts the string list \a list into the list at item \a index.

  If \a index is negative, \a list is inserted at the end of the list.	If
  \a index is too large, the operation is ignored.

  \warning This function uses <code>const char *</code> rather than
  QString, so we recommend against using it.  It is provided so that
  legacy code will continue to work, and so that programs that
  certainly will not need to handle code outside a single 8-bit locale
  can use it.

  \warning This function is never significantly faster than a loop
  around insertItem().

  \sa insertItem(), inSort()
*/

void QListBox::insertStrList( const QStrList *list, int index )
{
    if ( !list ) {
#if defined(CHECK_NULL)
	ASSERT( list != 0 );
#endif
	return;
    }
    QStrListIterator it( *list );
    const char* txt;
    if ( index < 0 )
	index = count();
    while ( (txt=it.current()) ) {
	++it;
	insertItem( new QListBoxText(txt), index++ );
    }
}



/*!
  Inserts the string list \a list into the list at item \a index.

  If \a index is negative, \a list is inserted at the end of the list.	If
  \a index is too large, the operation is ignored.

  \warning This function uses <code>const char *</code> rather than
  QString, so we recommend against using it.  It is provided so that
  legacy code will continue to work, and so that programs that
  certainly will not need to handle code outside a single 8-bit locale
  can use it.

  \warning This function is never significantly faster than a loop
  around insertItem().

  \sa insertItem(), inSort()
*/

void QListBox::insertStrList( const QStrList & list, int index )
{
    QStrListIterator it( list );
    const char* txt;
    if ( index < 0 )
	index = count();
    while ( (txt=it.current()) ) {
	++it;
	insertItem( new QListBoxText(txt), index++ );
    }
}


/*!
  Inserts the \a numStrings strings of the array \a strings into the
  list at item\a index.

  If \a index is negative, insertStrList() inserts \a strings at the end
  of the list.	If \a index is too large, the operation is ignored.

  \warning This function uses <code>const char *</code> rather than
  QString, so we recommend against using it.  It is provided so that
  legacy code will continue to work, and so that programs that
  certainly will not need to handle code outside a single 8-bit locale
  can use it.

  \warning This function is never significantly faster than a loop
  around insertItem().

  \sa insertItem(), inSort()
*/

void QListBox::insertStrList( const char **strings, int numStrings, int index )
{
    if ( !strings ) {
#if defined(CHECK_NULL)
	ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = count();
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	insertItem( new QListBoxText(strings[i]), index + i );
	i++;
    }
}


/*!
  Inserts the item \a lbi into the list at \a index.

  If \a index is negative or larger than the number of items in the list
  box, \a lbi is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const QListBoxItem *lbi, int index )
{
#if defined ( CHECK_NULL )
    ASSERT( lbi != 0 );
#else
    if ( !lbi )
	return;
#endif

    d->count++;

    if ( index < 0 )
	index = count();

    QListBoxItem * item = (QListBoxItem *)lbi;
    if ( !d->head || index == 0 ) {
	item->n = d->head;
	item->p = 0;
	d->head = item;
	item->dirty = TRUE;
    } else {
	QListBoxItem * i = d->head;
	while ( i->n && index > 1 ) {
	    i = i->n;
	    index--;
	}
	if ( i->n ) {
	    item->n = i->n;
	    item->p = i;
	    item->n->p = item;
	    item->p->n = item;
	} else {
	    i->n = item;
	    item->p = i;
	    item->n = 0;
	}
    }
    triggerUpdate( TRUE );
}

/*!
  Inserts \a text into the list at \a index.

  If \a index is negative, \a text is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const QString &text, int index )
{
    insertItem( new QListBoxText(text), index );
}

/*!
  Inserts \a pixmap into the list at \a index.

  If \a index is negative, \a pixmap is inserted at the end of the list.

  \sa insertStrList()
*/

void QListBox::insertItem( const QPixmap &pixmap, int index )
{
    insertItem( new QListBoxPixmap(pixmap), index );
}


/*!  Removes the item at position \a index. If \a index is equal to
currentItem(), a new item gets selected and the highlighted() signal
is emitted.

\sa insertItem(), clear() */

void QListBox::removeItem( int index )
{
    d->count = -1;
    delete item( index );
}


/*! Deletes all items in the list.
  \sa removeItem(), setStrList() */

void QListBox::clear()
{
    d->count = 0;
    d->current = 0;
    QListBoxItem * i = d->head;
    d->head = 0;
    while ( i ) {
	QListBoxItem * n = i->n;
	i->n = i->p = 0;
	delete i;
	i = n;
    }
    triggerUpdate( TRUE );
}


/*!
  Returns the text at position \e index, or a
  \link QString::operator!() null string\endlink
  if there is no text at that position.

  \sa pixmap()
*/

QString QListBox::text( int index ) const
{
    QListBoxItem * i = item( index );
    if ( i )
	return i->text();
    return QString::null;
}


/*!
  Returns a pointer to the pixmap at position \a index, or 0 if there is no
  pixmap there.
  \sa text()
*/

const QPixmap *QListBox::pixmap( int index ) const
{
    QListBoxItem * i = item( index );
    if ( i )
	return i->pixmap();
    return 0;
}

/*!
  Replaces the item at position \a index with \a text.

  The operation is ignored if \a index is out of range.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QString &text, int index )
{
    changeItem( new QListBoxText(text), index );
}

/*!
  Replaces the item at position \a index with \a pixmap.

  The operation is ignored if \a index is out of range.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QPixmap &pixmap, int index )
{
    changeItem( new QListBoxPixmap(pixmap), index );
}


/*!
  Replaces the item at position \a index with \a lbi.	If \e
  index is negative or too large, changeItem() does nothing.

  \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QListBoxItem *lbi, int index )
{
    if ( !lbi )
	return;

#if defined(CHECK_RANGE)
    if ( index > (int)count() )
	warning( "QListBox::changeItem: (%s) Index %i out of range",
		 name(), index );
#endif

    QListBoxItem * i = item( index );
    if ( !i )
	return;

    QListBoxItem * item = (QListBoxItem *)lbi;
    item->n = i->n;
    item->p = i->p;
    if ( item->n )
	item->n->p = item;
    if ( item->p )
	item->p->n = item;
    i->n = i->p = 0;
    delete i;
    triggerUpdate( TRUE );
}


/*!
  Returns the number of visible items.  Both partially and entirely
  visible items are counted.

  \sa setFixedVisibleLines()
*/

int QListBox::numItemsVisible() const
{
    doLayout();

    int columns = 0;

    int x = contentsX();
    int i=0;
    while ( i < (int)d->columnPos.size()-1 &&
	   d->columnPos[i] < x )
	i++;
    if ( i < (int)d->columnPos.size()-1 &&
	 d->columnPos[i] > x )
	columns++;
    x += viewport()->width();
    while ( i < (int)d->columnPos.size()-1 &&
	   d->columnPos[i] < x ) {
	i++;
	columns++;
    }

    int y = contentsY();
    int rows = 0;
    while ( i < (int)d->rowPos.size()-1 &&
	   d->rowPos[i] < y )
	i++;
    if ( i < (int)d->rowPos.size()-1 &&
	 d->rowPos[i] > y )
	rows++;
    y += viewport()->height();
    while ( i < (int)d->rowPos.size()-1 &&
	   d->rowPos[i] < y ) {
	i++;
	rows++;
    }

    return rows*columns;
}

/*!
  Returns the index of the current (highlighted) item of the list box,
  or -1 if no item has been selected.

  \sa topItem()
*/

int QListBox::currentItem() const
{
    if ( !d->current || !d->head )
	return -1;

    return index( d->current );
}

/*!
  Sets the highlighted item to the item at position \a index in the list.
  The highlighting is moved and the list box scrolled as necessary.

  If autoUpdate() is enabled, the display is updated accordingly.

  \sa currentItem()
*/

void QListBox::setCurrentItem( int index )
{
    setCurrentItem( item( index ) );
}


void QListBox::setCurrentItem( QListBoxItem * i )
{
    if ( d->current == i )
	return;
    QListBoxItem * o = d->current;
    d->current = i;
    if ( o )
	updateItem( o );
    if ( i )
	updateItem( i );
    int ind = index( i );
    d->currentColumn = ind / numRows();
    d->currentRow = ind % numRows();
    ensureCurrentVisible();
    QString tmp;
    if ( i )
	tmp = i->text();
    int tmp2 = index( i );
    emit highlighted( i );
    if ( !tmp.isNull() )
	emit highlighted( tmp );
    emit highlighted( tmp2 );
}


/*!  Returns a pointer to the item at position \a index, or 0 if \a
index is out of bounds. \sa
*/

QListBoxItem *QListBox::item( int index ) const
{
    QListBoxItem * i = d->head;
    while ( i && index ) {
	index--;
	i = i->n;
    }
    return i;
}


/*!  Returns the index of \a lbi, or -1 if the item is not in this
list box.

\sa item()
*/

int QListBox::index( QListBoxItem * lbi ) const
{
    if ( !lbi )
	return -1;
    int c = 0;
    QListBoxItem * i = d->head;
    while ( i && i != lbi ) {
	c++;
	i = i->n;
    }
    return i ? c : -1;
}



/*!
  Returns TRUE if the item at position \a index is at least partly
  visible.
*/

bool QListBox::itemVisible( int index )
{
    QListBoxItem * i = item( index );
    return i ? itemVisible( i ) : FALSE;
}


/*!  Returns TRUE if \a item is at least partly visible, or else FALSE.
*/

bool QListBox::itemVisible( QListBoxItem * item )
{
    int i = index( item );
    int col = i/numRows();
    int row = i%numCols();
    return ( d->columnPos[col] < contentsX()+viewport()->width() &&
	     d->rowPos[row] < contentsY()+viewport()->height() &&
	     d->columnPos[col+1] > contentsX() &&
	     d->rowPos[row+1] > contentsY() );
}


/*! \reimp */

void QListBox::viewportMousePressEvent( QMouseEvent *e )
{
    QListBoxItem * i = itemAt( e->pos() );
    switch( selectionMode() ) {
    default:
    case Single:
	if ( i )
	    setSelected( i, TRUE );
	break;
    case Extended:
	if ( i ) {
	    if ( !(e->state() & QMouseEvent::ShiftButton) )
		clearSelection();
	    setSelected( i, TRUE );
	}
	break;
    case Multi:
	if ( i ) {
	    d->current = i;
	    setSelected( i, !i->s );
	}
	break;
    }
    if ( i )
	setCurrentItem( i );
    // for sanity, in case people are event-filtering or whatnot
    delete d->scrollTimer;
    d->scrollTimer = 0;
    if ( i ) {
	d->mousePressColumn = d->currentColumn;
	d->mousePressRow = d->currentRow;
    }
}


/*! \reimp */

void QListBox::viewportMouseReleaseEvent( QMouseEvent *e )
{
    if ( d->scrollTimer )
	mouseMoveEvent( e );
    delete d->scrollTimer;
    d->scrollTimer = 0;
    emitChangedSignal( FALSE );
}


/*! \reimp */

void QListBox::viewportMouseDoubleClickEvent( QMouseEvent * )
{
    if ( selectionMode() == Single && d->current ) {
	QListBoxItem * i = d->current;
	QString tmp = d->current->text();
	emit selected( currentItem() );
	emit selected( i );
	if ( !tmp.isNull() )
	    emit selected( tmp );
    }
    // note: mouse move events may arrive and another release event will
}


/*! \reimp */

void QListBox::viewportMouseMoveEvent( QMouseEvent *e )
{
    if ( ( (e->state() & ( RightButton | LeftButton | MidButton ) ) == 0 ) ||
	 ( d->mousePressRow < 0 || d->mousePressColumn < 0 ) )
	return;

    int dx = 0;
    int x = e->x();
    if ( x >= viewport()->width() ) {
	x = viewport()->width()-1;
	dx = 1;
    } else if ( x < 0 ) {
	x = 0;
	dx = -1;
    }
    int col = columnAt( x + contentsX() );

    int dy = 0;
    int y = e->y();
    if ( y >= viewport()->height() ) {
	y = viewport()->height()-1;
	dy = 1;
    } else if ( y < 0 ) {
	y = 0;
	dy = -1;
    }
    d->scrollPos = QPoint( dx, dy );
    int row = rowAt( y + contentsY() );

    if ( col >= 0 && row >= 0 &&
	 d->mousePressColumn >= 0 && d->mousePressRow >= 0 ) {
	QListBoxItem * i = item( col * numRows() + row );
	if ( selectionMode() == Single ) {
	    if ( i )
		setSelected( i, TRUE );
	} else {
	    int c = QMIN( col, d->mousePressColumn );
	    int r = QMIN( row, d->mousePressRow );
	    int c2 = QMAX( col, d->mousePressColumn );
	    int r2 = QMAX( row, d->mousePressRow );
	    while( c < c2 ) {
		QListBoxItem * i = item( c*numRows()+r );
		while( i && r < r2 ) {
		    setSelected( i, d->current ? d->current->s : TRUE );
		    i = i->n;
		    r++;
		}
		c++;
	    }
	}
	if ( i )
	    setCurrentItem( i );
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
    if ( count() == 0 )
	return;
    if ( currentItem() == 0 )
	setCurrentItem( d->head );

    switch ( e->key() ) {
    case Key_Up:
	if ( currentRow() > 0 ) {
	    setCurrentItem( currentItem() - 1 );
	    if ( e->state() & ShiftButton )
		toggleCurrentItem();
	}
	e->accept();
	break;
    case Key_Down:
	if ( currentRow() < numRows()-1 ) {
	    setCurrentItem( currentItem()+1 );
	    if ( e->state() & ShiftButton )
		toggleCurrentItem();
	}
	e->accept();
	break;
    case Key_Left:
	if ( currentColumn() > 0 ) {
	    setCurrentItem( currentItem() - numRows() );
	    if ( e->state() & ShiftButton )
		toggleCurrentItem();
	} else {
	    QApplication::sendEvent( horizontalScrollBar(), e );
	}
	e->accept();
	break;
    case Key_Right:
	if ( currentColumn() < numColumns()-1 &&
	     currentItem() < (int)(count())-numRows() ) {
	    setCurrentItem( currentItem() + numRows() );
	    if ( e->state() & ShiftButton )
		toggleCurrentItem();
	} else {
	    QApplication::sendEvent( horizontalScrollBar(), e );
	}
	e->accept();
	break;
    case Key_Next:
	e->accept();
#if 0
	if ( style() == MotifStyle) {
	    if ( lastRowVisible() == (int) count() - 1){
		int o = yOffset();
		setBottomItem( lastRowVisible() );
		if ( currentItem() < lastRowVisible() &&
		     currentItem() == topItem() &&
		     yOffset() != o)
		    setCurrentItem(currentItem() + 1);
		break;
	    }
	    if (currentItem() != topItem() ){
		setTopItem( currentItem() );
		break;
	    }
	}
	else {
	    if ( currentItem() != lastRowVisible() ||
		 lastRowVisible() == (int) count() - 1) {
		ensureCurrentVisible(lastRowVisible());
		break;
	    }
	}
	oldCurrent = currentItem();
	setYOffset(yOffset() + viewHeight() );
	if ( style() == MotifStyle)
	    ensureCurrentVisible( topItem() );
	else
	    ensureCurrentVisible(lastRowVisible());
	if (oldCurrent == currentItem() && currentItem() + 1 <  (int) count() )
	    ensureCurrentVisible( currentItem() + 1 );
#endif
	break;
    case Key_Prior:
	e->accept();
#if 0
	if ( style() != MotifStyle) {
	    if (currentItem() != topItem() || topItem() == 0){
		ensureCurrentVisible(topItem());
		break;
	    }
	}
	else {
	    if ( topItem() == 0 ){
		int o = yOffset();
		setTopItem( topItem() );
		if ( currentItem() > 0 && currentItem() == lastRowVisible() && yOffset() != o)
		    setCurrentItem(currentItem()-1);
		break;
	    }
	    if ( currentItem() != lastRowVisible() ) {
		setBottomItem( currentItem() );
		break;
	    }
	}
	oldCurrent = currentItem();
	setYOffset(yOffset() - viewHeight() );
	if ( style() == MotifStyle)
	    ensureCurrentVisible( lastRowVisible() );
	else
	    ensureCurrentVisible( topItem() );
	if (oldCurrent == currentItem() && currentItem() > 0)
	    ensureCurrentVisible( currentItem() -1);
#endif
	break;

    case Key_Space:
	toggleCurrentItem();
	e->accept();
	break;

    case Key_Return:
    case Key_Enter:
	if ( currentItem() >= 0 ) {
	    QString tmp = item( currentItem() )->text();
	    emit selected( currentItem());
	    if ( !tmp.isEmpty() )
		emit selected( tmp );
	}
	// do NOT accept here.  qdialog.  yuck.
	break;
    default:
	break;
    }
    emitChangedSignal( FALSE );
}


/*!
  Handles focus events.  Repaints the current item (if not set,
  topItem() is made current).
  \sa keyPressEvent(), focusOutEvent()
*/

void QListBox::focusInEvent( QFocusEvent * )
{
    emitChangedSignal( FALSE );
    if ( currentItem() < 0 && numRows() > 0 )
	setCurrentItem( topItem() );
}


/*!
  Handles focus out events. Repaints the current item, if set.
  \sa keyPressEvent(), focusOutEvent()
*/

void QListBox::focusOutEvent( QFocusEvent * )
{
    emitChangedSignal( FALSE );
    if ( d->current )
	updateItem( currentItem() );
}


/*!
  Repaints the item at position \a index in the list.
*/

void QListBox::updateItem( int index )
{
    if ( index >= 0 )
	updateItem( item( index ) );
}


/*!
  Repaints \a i.
*/

void QListBox::updateItem( QListBoxItem * i )
{
    if ( !i )
	return;
    i->dirty = TRUE;
    d->updateTimer->start( 0, TRUE );
}


/*!  Sets the list box to selection mode \a mode, which may be one of
\c Single (the default), \a Extended and \c Multi.

\sa selectionMode()
*/

void QListBox::setSelectionMode( SelectionMode mode )
{
    if ( d->selectionMode == mode )
	return;

    d->selectionMode = mode;
    triggerUpdate( TRUE );
}


/*!  Returns the selection mode of the list box.  The initial mode is
\c Single.

\sa setSelectionMode()
*/

QListBox::SelectionMode QListBox::selectionMode() const
{
    return d->selectionMode;
}


/*!  Returns TRUE if the listbox is in multi-selection mode or
  extended selection mode, and FALSE if it is in single-selection mode.

  \sa selectionMode() setSelectionMode()
*/

bool QListBox::isMultiSelection() const
{
    return selectionMode() != Single;
}

/*!  Sets the list box to multi-selection mode if \a enable is TRUE, and
  to single-selection mode if \a enable is FALSE.  We recommend using
  setSelectionMode() instead; that function also offers a third mode
  of selection.

  \sa setSelectionMode() selectionMode()
*/

void QListBox::setMultiSelection( bool enable )
{
    setSelectionMode( enable ? Multi : Single );
}


/*!
  Toggles the selection status of currentItem() and repaints, if
  the listbox is a multi-selection listbox.

  Does nothing if the listbox is a single-selection listbox.

  \sa setMultiSelection()
*/

void QListBox::toggleCurrentItem()
{
    if ( selectionMode() == Single || !d->current )
	return;

    d->current->s = !d->current->s;
    updateItem( d->current );
    emitChangedSignal( TRUE );
}


/*!
  Selects the item at position \a index if \a select is TRUE, or
  unselects it if \a select is FALSE, and repaints the item
  appropriately.

  If the listbox is a single-selection listbox and and \a select is TRUE,
  setCurrentItem will be called.

  If the listbox is a single-selection listbox and and \a select is FALSE,
  clearSelection() will be called if \a index is the currently selected
  item.

  \sa setMultiSelection(), setCurrentItem(), clearSelection(), currentItem()
*/

void QListBox::setSelected( int index, bool select )
{
    setSelected( item( index ), select );
}


/*!
  Selects \a item if \a select is TRUE, or unselects it if \a select
  is FALSE, and repaints the item appropriately.

  If the listbox is a single-selection listbox and and \a select is TRUE,
  setCurrentItem will be called.

  If the listbox is a single-selection listbox and and \a select is FALSE,
  clearSelection() will be called if \a index is the currently selected
  item.

  \sa setMultiSelection(), setCurrentItem(), clearSelection(), currentItem()
*/

void QListBox::setSelected( QListBoxItem * item, bool select )
{
    if ( !item || item->s == (uint)select )
	return;

    if ( selectionMode() == Single && select &&
	 d->current && d->current->s ) {
	d->current->s = FALSE;
    }

    item->s = select;
    updateItem( item );
    emitChangedSignal( TRUE );
}


/*!
  Returns TRUE if item \a i is selected. Returns FALSE if it is not
  selected or if there is an error.
*/

bool QListBox::isSelected( int i ) const
{
    if ( i < 0 || i > (int)count() )
	return FALSE;

    if ( selectionMode() == Single )
	return i == currentItem();

    QListBoxItem * lbi = item( i );
    if ( !i )
	return FALSE; // should not happen
    return lbi->s;
}


/*!
  Returns TRUE if item \a i is selected. Returns FALSE if it is not
  selected or if there is an error.
*/

bool QListBox::isSelected( QListBoxItem * i ) const
{
    if ( !i )
	return FALSE;

    return i->s;
}


/*!
  Deselects all items, if possible.

  Note that a single-selection listbox will automatically select an
  item if it has keyboard focus.
*/

void QListBox::clearSelection()
{
    if ( selectionMode() != Single ) {
	for ( int i = 0; i < (int)count(); i++ )
	    setSelected( i, FALSE );
    } else {
	QListBoxItem * i = d->current;
	setCurrentItem( hasFocus() ? d->head : 0 );
	updateItem( i );
    }
}



/*!  If \a lazy is FALSE, maybe emit the changed() signal.  If \a lazy
  is TRUE, note that it's to be sent out at some later time.
*/

void QListBox::emitChangedSignal( bool lazy ) {
    if ( selectionMode() == Single )
	return;

    if ( changedListBox && (!lazy || changedListBox != this) )
	emit changedListBox->selectionChanged();

    changedListBox = lazy ? this : 0;
}


/*!
  Returns something...
*/
QSize QListBox::sizeHint() const
{
    return QSize( 50, 150 ); // ### !!! ### !!!
}


/*!
  Specifies that this widget can use additional space, and that it can
  survive on less than sizeHint().
*/

QSizePolicy QListBox::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}


/*!  Ensures that a single paint event will occur at the end of the
  current event loop iteration.  If \a doLayout is TRUE, the layout is
  also redone.
*/

void QListBox::triggerUpdate( bool doLayout )
{
    if ( doLayout )
	d->layoutDirty = TRUE;
    d->updateTimer->start( 0, TRUE );
}


/*!  Sets the column layout mode to \a mode, and the number of displayed
columns accordingly.

The row layout mode implicitly becomes \c Variable.

If \a mode is \c Variable, this function returns without doing anything.

*/

void QListBox::setColumnMode( LayoutMode mode )
{
    if ( mode == Variable )
	return;
    d->rowModeWins = FALSE;
    d->columnMode = mode;
    triggerUpdate( TRUE );
}


/*!  Sets the column layout mode for this list box to \c FixedNumber, and
  sets the number of displayed columns accordingly.

  \setRowMode() columnMode() numColumns()
*/

void QListBox::setColumnMode( int columns )
{
    if ( columns < 1 )
	columns = 1;
    d->columnMode = FixedNumber;
    d->numColumns = columns;
    d->rowModeWins = FALSE;
    triggerUpdate( TRUE );
}


/*!  Sets the row layout mode to \a mode, and the number of displayed
rows accordingly.

The column layout mode implicitly becomes \c Variable.

If \a mode is \c Variable, this function returns without doing anything.

*/

void QListBox::setRowMode( LayoutMode mode )
{
    if ( mode == Variable )
	return;
    d->rowModeWins = TRUE;
    d->rowMode = mode;
    triggerUpdate( TRUE );
}


/*!  Sets the row layout mode for this list box to \c FixedNumber and sets the
  number of displayed rows accordingly.

  \setColumnMode() rowMode() numRows()
*/

void QListBox::setRowMode( int rows )
{
    if ( rows < 1 )
	rows = 1;
    d->rowMode = FixedNumber;
    d->numRows = rows;
    d->rowModeWins = TRUE;
    triggerUpdate( TRUE );
}


/*!  Returns the column layout mode for this list box.  This is
  normally \c FixedNumber, but can be changed by calling setColumnMode()

  \sa rowMode(), setColumnMode(), numColumns()
*/

QListBox::LayoutMode QListBox::columnMode() const
{
    if ( d->rowModeWins )
	return Variable;
    else
	return d->columnMode;
}


/*!  Returns the row layout mode for this list box.  This is normally
  \c Variable, but can be changed by calling setRowMode().

  \sa columnMode(), setRowMode(), numRows()
*/

QListBox::LayoutMode QListBox::rowMode() const
{
    if ( d->rowModeWins )
	return d->rowMode;
    else
	return Variable;
}


/*!  Returns the number of columns in the list box.  This is normally
1, but can be different if setColumnMode() or setRowMode() has been
called.

\sa setColumnMode() setRowMode() numRows()
*/

int QListBox::numColumns() const
{
    if ( !d->rowModeWins && d->columnMode == FixedNumber )
	return d->numColumns;
    doLayout();
    return d->columnPos.size()-1;
}


/*!  Returns the number of rows in the list box.  This is equal to the
number of items in the default single-column layout, but can be
different.

\sa setRowMode() numColumns()
*/

int QListBox::numRows() const
{
    if ( d->rowModeWins && d->rowMode == FixedNumber )
	return d->numRows;
    doLayout();
    return d->rowPos.size()-1;
}


/*!  This function does the hard layout work.  You should never need
  to call it.
*/

void QListBox::doLayout() const
{
    if ( !d->layoutDirty )
	return;

    int c = count();
    switch( rowMode() ) {
    case FixedNumber:
	// columnMode() is known to be Variable
	tryGeometry( d->numRows, (c+d->numRows-1)/c );
	break;
    case FitToHeight:
	// columnMode() is known to be Variable
	if ( d->head ) {
	    // this is basically the FitToWidth code, but edited to use rows.
	    int maxh = 0;
	    QListBoxItem * i = d->head;
	    while ( i ) {
		int h = i->height( this );
		if ( maxh < h )
		    maxh = h;
		i = i->n;
	    }
	    int vh = viewportSize( QSize( 1, 1 ) ).height();
	    do {
		int rows = vh / maxh;
		if ( rows > c )
		    rows = c;
		if ( rows < 1 )
		    rows = 1;
		if ( variableHeight() && rows < c ) {
		    do {
			++rows;
			tryGeometry( rows, (c+rows-1)/rows );
		    } while ( rows <= c &&
			      d->rowPos[(int)d->rowPos.size()-1] <= vh );
		    --rows;
		}
		tryGeometry( rows, (c+rows-1)/rows );
		int nvh = viewportSize(
			QSize( d->columnPos[(int)d->columnPos.size()-1],
			       d->rowPos[(int)d->rowPos.size()-1] ) ).height();
		if ( nvh < vh )
		    vh = nvh;
	    } while ( vh < d->rowPos[(int)d->rowPos.size()-1] );
	} else {
	    tryGeometry( 1, 1 );
	}
	break;
    case Variable:
	if ( columnMode() == FixedNumber ) {
	    tryGeometry( (count()+d->numColumns-1)/d->numColumns,
			 d->numColumns );
	} else if ( d->head ) { // FitToWidth, at least one item
	    int maxw = 0;
	    QListBoxItem * i = d->head;
	    while ( i ) {
		int w = i->width( this );
		if ( maxw < w )
		    maxw = w;
		i = i->n;
	    }
	    int vw = viewportSize( QSize( 1,1 ) ).width();
	    do {
		int columns = vw / maxw;
		if ( columns > c )
		    columns = c;
		if ( columns < 1 )
		    columns = 1;
		if ( variableWidth() && columns < c ) {
		    // variable width means we have to Work Harder.
		    do {
			columns++;
			tryGeometry( (c+columns-1)/columns, columns );
		    } while ( columns <= c &&
			      d->columnPos[(int)d->columnPos.size()-1] <= vw );
		    // and as the perceptive reader will understand, we Work
		    // even Harder than we really have to...
		    --columns;
		}
		tryGeometry( (c+columns-1)/columns, columns );
		int nvw = viewportSize( QSize( d->columnPos[(int)d->columnPos.size()-1],
					       d->rowPos[(int)d->rowPos.size()-1] ) ).width();
		if ( nvw < vw )
		    vw = nvw;
	    } while ( vw < d->columnPos[(int)d->columnPos.size()-1] );
	} else {
	    tryGeometry( 1, 1 );
	}
	break;
    }
    d->layoutDirty = FALSE;
    QSize s( viewportSize( QSize( d->columnPos[numColumns()],
				  d->rowPos[numRows()] ) ) );
    int x, y;
    x = QMAX( d->columnPos[numColumns()], s.width() );
    y = QMAX( d->rowPos[numRows()], s.height() );
    ((QListBox *)this)->resizeContents( x, y );
}


/*! Lay the items out in a \a columns by \a rows array.  The array may
  be too big or whatnot: doLayout() is expected to call this with the
  right values. */

void QListBox::tryGeometry( int rows, int columns ) const
{
    if ( columns < 1 )
	columns = 1;
    d->columnPos.resize( columns+1 );

    if ( rows < 1 )
	rows = 1;
    d->rowPos.resize( rows+1 );

    // funky hack I: dump the height/width of each column/row in
    // {column,row}Pos for later conversion to positions.
    int c;
    for( c=0; c<=columns; c++ )
	d->columnPos[c] = 0;
    int r;
    for( r=0; r<=rows; r++ )
	d->rowPos[r] = 0;
    r = c = 0;
    QListBoxItem * i = d->head;
    while ( i && c < columns) {
	if ( i == d->current ) {
	    d->currentRow = r;
	    d->currentColumn = c;
	}
	int w = i->width( this );
	if ( d->columnPos[c] < w )
	    d->columnPos[c] = w;
	int h = i->height( this );
	if ( d->rowPos[r] < h )
	    d->rowPos[r] = h;
	i = i->n;
	r++;
	if ( r == rows ) {
	    r = 0;
	    c++;
	}
    }

    // funky hack II: if not variable {width,height}, unvariablify it.
    if ( !variableWidth() ) {
	int w = 0;
	for( c=0; c<columns; c++ )
	    if ( w < d->columnPos[c] )
		w = d->columnPos[c];
	for( c=0; c<columns; c++ )
	    d->columnPos[c] = w;
    }
    if ( !variableHeight() ) {
	int h = 0;
	for( r=0; r<rows; r++ )
	    if ( h < d->rowPos[r] )
		h = d->rowPos[r];
	for( r=0; r<rows; r++ )
	    d->rowPos[r] = h;
    }

    // repair the hacking.
    int x = 0;
    for( c=0; c<=columns; c++ ) {
	int w = d->columnPos[c];
	d->columnPos[c] = x;
	x += w;
    }
    int y = 0;
    for( r=0; r<=rows; r++ ) {
	int h = d->rowPos[r];
	d->rowPos[r] = y;
	y += h;
    }
}


/*!  Returns the row index of the current item, or -1 if no item is
  currently the current item.
*/

int QListBox::currentRow() const
{
    if ( !d->current )
	return -1;
    if ( d->currentRow < 0 )
	d->layoutDirty = TRUE;
    if ( d->layoutDirty )
	doLayout();
    return d->currentRow;
}


/*!  Returns the column index of the current item, or -1 if no item is
  currently the current item.
*/

int QListBox::currentColumn() const
{
    if ( !d->current )
	return -1;
    if ( d->currentColumn < 0 )
	d->layoutDirty = TRUE;
    if ( d->layoutDirty )
	doLayout();
    return d->currentColumn;
}


/*!
  Scrolls the list box so the item at position \a index in the list
  is displayed in the top row of the list box.

  \sa topItem(), ensureCurrentVisible()
*/

void QListBox::setTopItem( int index )
{
    if ( index >= (int)count() )
	return;
    int col = index / numRows();
    int y = d->rowPos[index-col*numRows()];
    if ( d->columnPos[col] >= contentsX() &&
	 d->columnPos[col+1] <= contentsX() + viewport()->width() )
	setContentsPos( contentsX(), y );
    else
	setContentsPos( d->columnPos[col], y );
}

/*!
  Scrolls the list box so the item at position \a index in the list
  is displayed in the bottom row of the list box.

  \sa setTopItem()
*/

void QListBox::setBottomItem( int index )
{
    if ( index >= (int)count() )
	return;
    int col = index / numRows();
    int y = d->rowPos[1+index-col*numRows()] - viewport()->height();
    if ( y < 0 )
	y = 0;
    if ( d->columnPos[col] >= contentsX() &&
	 d->columnPos[col+1] <= contentsX() + viewport()->width() )
	setContentsPos( contentsX(), y );
    else
	setContentsPos( d->columnPos[col], y );
}


/*!  Returns a pointer to the item at \a p, which is in on-screen
coordinates, or a null pointer if there is no item at \a p.

*/

QListBoxItem * QListBox::itemAt( QPoint p ) const
{
    if ( d->layoutDirty )
	doLayout();
    int x = p.x() + contentsX();
    int y = p.y() + contentsY();

    int col=0;
    while ( col < numColumns() &&
	   x > d->columnPos[col] )
	col++;
    if ( col ) // && x < d->columnPos[col] )
	col--;

    int row=0;
    while ( row < numRows() &&
	   y > d->rowPos[row] )
	row++;
    if ( row && y < d->rowPos[row] )
	row--;

    return item( col*numRows()+row );
}


/*!  Ensures that the currently selected item is visible.  If the list
box is in multi-selection mode, this function has no effect.
*/

void QListBox::ensureCurrentVisible()
{
    if ( !d->current )
	return;

    doLayout();
    int row = currentRow();
    int column = currentColumn();

    if ( d->rowPos[row] >= contentsY() &&
	 d->rowPos[row+1] <= contentsY()+viewport()->height() &&
	 d->columnPos[column] >= contentsX() &&
	 d->columnPos[column+1] <= contentsX()+viewport()->width() )
	return;

    int y = (d->rowPos[row] + d->rowPos[row+1] - viewport()->height())/2;
    // fuddle y to get good-looking alignment?

    // see whether mere vertical scrolling will do
    if ( contentsX() < d->columnPos[column] &&
	 contentsX()+contentsWidth() < d->columnPos[column+1] &&
	 contentsY() < d->rowPos[row] &&
	 contentsY()+contentsHeight() < d->rowPos[row+1] ) {
	setContentsPos( contentsX(), y );
    } else {
	int x = (d->columnPos[column] +
		 d->columnPos[column+1] -
		 viewport()->width())/2;
	// fuddle x too?
	setContentsPos( x, y );
    }
}


/*! \internal */

void QListBox::autoScroll()
{
    debug( "as" );
    if ( d->scrollPos.x() < 0 ) {
	// scroll left
	int x = contentsX() - horizontalScrollBar()->lineStep();
	if ( x < 0 )
	    x = 0;
	setContentsPos( x, contentsY() );
    } else if ( d->scrollPos.x() >= viewport()->width() ) {
	// scroll left
	int x = contentsX() + horizontalScrollBar()->lineStep();
	if ( x + viewport()->width() > contentsWidth() )
	    x = contentsWidth() - viewport()->width();
	setContentsPos( x, contentsY() );
    } else if ( d->scrollPos.y() < 0 ) {
	// scroll up
	int y = contentsY() - verticalScrollBar()->lineStep();
	if ( y < 0 )
	    y = 0;
	setContentsPos( contentsX(), y );
    } else if ( d->scrollPos.y() >= viewport()->height() ) {
	// scroll down
	int y = contentsY() + verticalScrollBar()->lineStep();
	if ( y + viewport()->height() > contentsHeight() )
	    y = contentsHeight() - viewport()->height();
	setContentsPos( contentsX(), y );
    } else {
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }
}


/*!  Returns the index of an item at the top of the screen.  If there
are more than one of them, an arbitrary item is selected and returned.

*/

int QListBox::topItem() const
{
    doLayout();
    int col=0;
    while ( col < numColumns() && d->columnPos[col] < contentsX() )
	col++;
    if ( ( col < numColumns() &&
	   d->columnPos[col+1] <= contentsX()+viewport()->width() ) ||
	 col == 0 ||
	 d->columnPos[col] < contentsX()+viewport()->width()/2 )
	return col*numRows();
    return (col-1)*numRows();
}


/*!

*/

bool QListBox::variableHeight() const
{
    return d->variableHeight;
}


/*!

*/

void QListBox::setVariableHeight( bool enable )
{
    if ( d->variableHeight == enable )
	return;

    d->variableHeight = enable;
    triggerUpdate( TRUE );
}


/*!
*/

bool QListBox::variableWidth() const
{
    return d->variableWidth;
}


/*!

*/

void QListBox::setVariableWidth( bool enable )
{
    if ( d->variableWidth == enable )
	return;

    d->variableWidth = enable;
    triggerUpdate( TRUE );
}


/*!  Repaints just what really needs to be repainted. */

void QListBox::refreshSlot()
{
    if ( d->layoutDirty ) {
	doLayout();
	viewport()->repaint( FALSE );
	return;
    }

    QRegion r;
    int x = contentsX();
    int y = contentsY();
    int col = 0;
    int row = 0;
    int top = row;
    while( col < (int)d->columnPos.size()-1 && d->columnPos[col+1] < x )
	col++;
    while( top < (int)d->rowPos.size()-1 && d->rowPos[top+1] < y )
	top++;
    QListBoxItem * i = item( col*numRows() );

    while ( i && (int)col < numCols() &&
	    d->columnPos[col] < x + viewport()->width()  ) {
	int cw = d->columnPos[col+1] - d->columnPos[col];
	while ( i && row < top ) {
	    i = i->n;
	    row++;
	}
	while ( i && row < numRows() && d->rowPos[row] <
		y + viewport()->height() ) {
	    if ( i->dirty )
		r = r.unite( QRect( d->columnPos[col] - x, d->rowPos[row] - y,
				    cw, d->rowPos[row+1] - d->rowPos[row] ) );
	    row++;
	    i = i->n;
	}
	while ( i && row < numRows() )
	    i = i->n;
	row = 0;
	col++;
    }
    if ( r.isEmpty() )
	viewport()->repaint( FALSE );
    else
	viewport()->repaint( r, FALSE );
}


/*! \reimp */

void QListBox::viewportPaintEvent( QPaintEvent * e )
{
    QWidget* vp = viewport();
    QPainter p( vp );
    QRegion r = e->region();

/*
    Arnt!  REMEMBER: On Window there is no event queue protecting you
    from re-entrancy.  This causes resizes, causes layouts...
    You've got enough experience to know that a "const" function with
    "do" in its name (doLayout()) is just plain asking for trouble.

    if ( d->layoutDirty )
	doLayout();
*/

    int x = contentsX();
    int y = contentsY();
    int w = vp->width();
    int h = vp->height();

    int col = 0;
    int row = 0;
    int top = row;
    while( col < (int)d->columnPos.size()-1 && d->columnPos[col+1] < x )
	col++;
    while( top < (int)d->rowPos.size()-1 && d->rowPos[top+1] < y )
	top++;
    QListBoxItem * i = item( col*numRows() );

    const QColorGroup & g = colorGroup();
    p.setPen( g.text() );
    p.setBackgroundColor( g.base() );
    while ( i && (int)col < numCols() && d->columnPos[col] < x + w ) {
	int cw = d->columnPos[col+1] - d->columnPos[col];
	while ( i && row < top ) {
	    i = i->n;
	    row++;
	}
	while ( i && (int)row < numRows() && d->rowPos[row] < y + h ) {
	    int ch = d->rowPos[row+1] - d->rowPos[row];
	    QRect itemRect( d->columnPos[col]-x,  d->rowPos[row]-y, cw, ch );
	    QRegion itemPaintRegion( QRegion( itemRect ).intersect( r  ) );
	    if ( !itemPaintRegion.isEmpty() ) {
		p.save();
		p.setClipRegion( itemPaintRegion );
		p.translate( d->columnPos[col]-x, d->rowPos[row]-y );
		if ( i->s ) {
		    p.fillRect( 0, 0, cw, ch,
				 g.brush( QColorGroup::Highlight ) );
		    p.setPen( g.highlightedText() );
		    p.setBackgroundColor( g.highlight() );
		} else {
		    p.fillRect( 0, 0, cw, ch, g.base() );
		}
		i->paint( &p );
		if ( d->current == i && hasFocus() )
		    style().drawFocusRect( &p, QRect( d->columnPos[col]-x,
						      d->rowPos[row]-y,
						      cw, ch ),
					   g, &g.highlight(), TRUE );
		p.restore();
		r = r.subtract( itemPaintRegion );
	    }
	    row++;
	    i->dirty = FALSE;
	    i = i->n;
	}
	while ( i && row < numRows() )
	    i = i->n;
	row = 0;
	col++;
    }
    if ( r.isEmpty() )
	return;
    p.setClipRegion( r );
    p.fillRect( 0, 0, w, h, g.brush( QColorGroup::Base ) );
}


/*!  Returns the height in pixels of the item with index \a index.  \a
index defaults to 0.

If \a index is too large, this function returns 0.
*/

int QListBox::itemHeight( int index ) const
{
    if ( !d->layoutDirty )
	doLayout();
    if ( index >= (int)count() || index < 0 )
	return 0;
    int r = index % numRows();
    return d->columnPos[r+1] - d->columnPos[r];
}


/*!  Returns the index of the column at \a x, which is in the listbox'
  coordinates, not in on-screen coordinates.

  If there is no column that spans \a x, columnAt() returns -1.
*/

int QListBox::columnAt( int x ) const
{
    if ( !d->layoutDirty )
	doLayout();
    if ( x < 0 || x >= d->columnPos[(int)d->columnPos.size()-1] )
	return -1;
    int col = 0;
    while( col < (int)d->columnPos.size()-1 && d->columnPos[col+1] < x )
	col++;
    return col;
}


/*!  Returns the index of the row at \a y, which is in the listbox'
  coordinates, not in on-screen coordinates.

  If there is no row that spans \a y, rowAt() returns -1.
*/

int QListBox::rowAt( int y ) const
{
    if ( !d->layoutDirty )
	doLayout();
    if ( y < 0 || y >= d->rowPos[(int)d->rowPos.size()-1] )
	return -1;
    int row = 0;
    while( row < (int)d->rowPos.size()-1 && d->rowPos[row+1] < y )
	row++;
    return row;
}


void QListBox::inSort( const QListBoxItem * lbi )
{
    if ( !lbi )
	return;

    QListBoxItem * i = d->head;
    int c = 0;

    while( i && i->text() < lbi->text() ) {
	i = i->n;
	c++;
    }
    insertItem( lbi, c );
}


void QListBox::inSort( const char *text )
{
    inSort( new QListBoxText(text) );
}
