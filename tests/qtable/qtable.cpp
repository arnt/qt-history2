/****************************************************************************
**
** Implementation of QTable widget class
**
** Created : 000607
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qtable.h"
#include <qpainter.h>
#include <qkeycode.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <qtimer.h>
#include <stdlib.h>

/*!
  \class QTableItem qtable.h

  \brief Implementation of a item for a QTable.

  This item contains the data of a table cell, specifies the editor
  and edit type for that cell, defines some other behaviour and
  provides the API neede for sorting table items. By default it can
  contain a text and pixmaps and offers a QLineEdit for editing.

  By reimplementing paint(), key(), editor() and
  setContentFromEditor() you can change this.

  As it is possible to put one item multiple times into a QTable (this
  is internally used e.g. for multicells), QTableItem is derived from
  QShared to do reference counting. So never do a

  \code
  QTableItem *item = .....
  delete item;
  \endcode

  but always do instead

  \code
  QTableItem *item = .....
  if ( item && item->deref() == 0 )
      delete item;
  \endcode
*/

/*! \fn QTable *QTableItem::table() const

  Returns the QTable of this item.
*/

/*! \enum QTableItem::EditType

  <ul>
  <li>\c Always
  <li>\c OnActivate
  <li>\c OnCurrent
  <li>\c Never
  </ul>
*/

/*!  Creates an item for the table \a table with the text \a t.
*/

QTableItem::QTableItem( QTable *table, const QString &t )
    : txt( t ), pix(), t( table ), edType( OnActivate ), wordwrap( FALSE ),
      tcha( TRUE ), lastEditor( 0 ), row( -1 ), col( -1 ), rowspan( 1 ), colspan( 1 )
{
}

/*!  Creates an item for the table \a table with the text \a t and the
  pixmap \a p.
*/

QTableItem::QTableItem( QTable *table, const QString &t, const QPixmap &p )
    : txt( t ), pix( p ), t( table ), edType( OnActivate ), wordwrap( FALSE ),
      tcha( TRUE ), lastEditor( 0 ), row( -1 ), col( -1 ), rowspan( 1 ), colspan( 1 )
{
}

/*!  Destructor.
*/

QTableItem::~QTableItem()
{
}

/*!  Returns the pixmap of that item.
*/

QPixmap QTableItem::pixmap() const
{
    return pix;
}


/*!  Returns the text of that item.
*/

QString QTableItem::text() const
{
    if ( edType == Always )
	( (QTableItem*)this )->setContentFromEditor( lastEditor );
    return txt;
}

/*!  Sets the pixmap of the item to \a pix. Does not repaint the cell!
*/

void QTableItem::setPixmap( const QPixmap &p )
{
    pix = p;
}

/*!  Sets the text of the item to \a t. Does not repaint the cell!
*/

void QTableItem::setText( const QString &t )
{
    txt = t;
}

/*!  This is called to paint the contents of the item.
*/

void QTableItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected )
{
    int w = cr.width();
    int h = cr.height();

    int x = 0;
    if ( !pix.isNull() ) {
	p->drawPixmap( 0, ( cr.height() - pix.height() ) / 2, pix );
	x = pix.width() + 2;
    }

    if ( selected )
	p->setPen( cg.highlightedText() );
    p->drawText( x, 0, w - x, h, wordwrap ? alignment() | WordBreak : alignment() , txt );
}

/*!  Returns the editor which should be used for editing that
  cell. Returning 0 means that this cell is not editable. If you
  reimplement that to use a custom editor widget always create a new
  widget here as parent of table()->viewport(), as the ownership of it
  is trasferred to the caller.

  \sa QTable::editor()
*/

QWidget *QTableItem::editor() const
{
    QLineEdit *e = new QLineEdit( table()->viewport() );
    e->setFrame( FALSE );
    e->setText( text() );
    return e;
}

/*!  This function is called to set the cell contents from the editor
  \a w.

  \sa QTable::setContentFromEditor()
*/

void QTableItem::setContentFromEditor( QWidget *w )
{
    if ( w->inherits( "QLineEdit" ) )
	setText( ( (QLineEdit*)w )->text() );
}

/*!  This function returns the alignment which should be used inside
  the cell to draw the contents. The default implementation aligns
  normal text to the left and numbers to the right.
*/

int QTableItem::alignment() const
{
    bool num;
    bool ok1 = FALSE, ok2 = FALSE;
    txt.toInt( &ok1 );
    txt.toDouble( &ok2 );
    num = ok1 || ok2;

    return ( num ? AlignRight : AlignLeft ) | AlignVCenter;
}

/*! If \a b is TRUE, the cell's text is wrapped into multible lines,
  else not.
*/

void QTableItem::setWordWrap( bool b )
{
    wordwrap = b;
}

/*! Returns TRUE, of word-wrap is turned on for this cell, else FALSE.
 */

bool QTableItem::wordWrap() const
{
    return wordwrap;
}

/*!  Sets the edit type for that cell to \a t. This can be one of \c
  Never (not editable), \c OnActivate (normal in-place editing), \c
  OnCurrent (start in-place editing when this cell gets the current
  one) and \c Always (always show the editor).
*/

void QTableItem::setEditType( EditType t )
{
    EditType old = edType;
    edType = t;
    table()->editTypeChanged( this, old );
}

/*!  Returns the edit type of that item

  \sa setEditType()
*/

QTableItem::EditType QTableItem::editType() const
{
    return edType;
}

/*! If this item is set to a cell it might be that it should be not
  possible anymore to replace the contents of that cell by another
  QTableItem. If this should be the case, set \a b to TRUE here.
*/

void QTableItem::setTypeChangeAllowed( bool b )
{
    tcha = b;
}

/*!  Returns if it is allowed to replace this item if it is set to a
  cell once.

  \sa setTypeChangeAllowed()
*/

bool QTableItem::isTypeChangeAllowed() const
{
    if ( rowspan > 1 || colspan > 1 )
	return FALSE;
    return tcha;
}

/*! Returns the key which should be used for sorting. The default
  implementation returns the text() if the item here. You might
  reimplement this in custom table items.
*/

QString QTableItem::key() const
{
    return text();
}

/*!  Returns the size which the cell needs to have to show the whole
  contents. If you subclass and implement a custom item width custom
  data, you should reimplement this function too.
*/

QSize QTableItem::sizeHint() const
{
    if ( edType == Always && lastEditor )
	return lastEditor->sizeHint();

    QSize s;
    if ( !pix.isNull() ) {
	s = pix.size();
	s.setWidth( s.width() + 2 );
    }

    s.setWidth( s.width() + table()->fontMetrics().width( text() ) + 10 );
    return s;
}

/*!  Sets this cell to fill \a rs rows and \a cs columns. Using that
  you can make a QTableItem to a multi-cell.
*/

void QTableItem::setSpan( int rs, int cs )
{
    if ( rowspan > 1 || colspan > 1 ) {
	for ( int r = 0; r < rowspan; ++r ) {
	    for ( int c = 0; c < colspan; ++c ) {
		if ( r == 0 && c == 0 )
		    continue;
		table()->clearCell( r + row, c + col );
	    }
	}
    }

    rowspan = rs;
    colspan = cs;

    for ( int r = 0; r < rowspan; ++r ) {
	for ( int c = 0; c < colspan; ++c ) {
	    if ( r == 0 && c == 0 )
		continue;
	    int rrow = row;
	    int rcol = col;
	    table()->setCellContent( r + row, c + col, this );
	    ref();
	    row = rrow;
	    col = rcol;
	}
    }
}

/*! Returns the row-span of this item
 */

int QTableItem::rowSpan() const
{
    return rowspan;
}

/*! Returns the col-span of this item
 */

int QTableItem::columnSpan() const
{
    return colspan;
}


/*!
  \class QTable qtable.h

  \brief Implementation of a flexible and editable table widget.

  This widget is the implementation of an editable table widget. The
  conecpt is that initially there is no memory allocated for any
  cell. But if a table cell should get a content create a QTableItem
  for that and set it using setCellContent(). There exist convenience
  functions for setting table text and pixmaps (setCellText(),
  setCellPixmap()). These functions just create an item for the
  required cell on demand.

  If you want to draw a custom content for a cell normally you
  implement your own subclass of QTableItem and reimplement the
  QTableItem::paint() method which is then used for drawing the contents.

  If you have your data already in a datastructure in your application
  and do not want to allocate a QTableItem for each cell with a
  contents, you can reimplement QTable::paintCell() and draw there
  directly the contents without the need of any QTableItems. If you do
  this you can't of course use setCellText() and friends for these
  cells (except you reimplement them and do something different). Also
  you have of course to repaint the cells which changed yourself using
  updateCell().

  The in-place editing is also made in an abstract way so that it is
  possible to have custom edit widgets for certain cells or types or
  cells. When in-plcae editing is started beginEdit() is called. This
  creates the editor widget for the required cell, places and shows
  it. To create an editor widget this function calls editor() for the
  required cell. See the documentation of editor() for more detailed
  documentation of that.

  Now there exist two different ways to edit a cell. Either offer an
  edit widget to enter a contents which should replace the current
  cell's contents or offer an editor to edit the current cell's
  contents. If it shouldn't be possible to replace the contents of a
  cell, but just edit the current content, set a QTableItem for that
  cell and set isTypeChangeAllowed() of that item to FALSE.

  There are also different ways for starting in-place
  editing. Normally if the user starts typing text in-place editing
  (Replacing) for the current cell is started. If the user
  double-clicks on a cell also in-place editing is started for the
  cell he doubleclicked on (Editing).  But it is sometimes required
  that a cell always shows an editor, shows the editor as soon as it
  gets the current cell or that it is not editable at all. Use
  QTableItem::setEditType() for specifying this behaviour of a cell.

  Now when the user finishes editing endEdit() is called. Look at the
  documentation of endEdit() for more information on that (e.g. how
  the contents from the editor is transferred to the item.)

  If you want to make a cell not editable and do not want to waste a
  QTableItem for this cell, reimplement editor() and return 0 there
  for the cells which should be not editable.

  QTable supports also all needed selection types like range
  selections, selectiongs through the header, selection with keyboard
  and mouse, etc.

  QTable offers also an API for sorting columns. See setSorting(),
  sortColumn() and QTableItem::key() for more details.
*/

/*! \fn void QTable::currentChanged( int row, int col )

  This signal is emitted if the current cell has been changed to \a
  row, \a col.
*/

/*! \fn int QTable::currentRow() const

  Returns the current row.
*/

/*! \fn int QTable::currentColumn() const

  Returns the current column.
*/

/*! \enum QTable::EditMode
  <ul>
  <li>\c NotEditing
  <li>\c Editing
  <li>\c Replacing
  </ul>
 */

/*!  Constructs a table with a range of 10 * 10 cells.
*/

QTable::QTable( QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase | WNorthWestGravity ),
      currentSelection( 0 ), sGrid( TRUE ), mRows( FALSE ), mCols( FALSE ),
      lastSortCol( -1 ), asc( TRUE ), doSort( TRUE )
{
    init( 10, 10 );
}

/*!  Constructs a table with a range of \a numRows * \a numCols cells.
*/

QTable::QTable( int numRows, int numCols, QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase | WNorthWestGravity ),
      currentSelection( 0 ), sGrid( TRUE ), mRows( FALSE ), mCols( FALSE ),
      lastSortCol( -1 ), asc( TRUE ), doSort( TRUE )
{
    init( numRows, numCols );
}

/*! \internal
 */

void QTable::init( int numRows, int numCols )
{
    // Enable clipper and set background mode
    enableClipper( TRUE );
    viewport()->setBackgroundMode( PaletteBase );
    setResizePolicy( Manual );
    selections.setAutoDelete( TRUE );

    // Create headers
    leftHeader = new QTableHeader( numRows, this, this );
    leftHeader->setOrientation( Vertical );
    leftHeader->setTracking( TRUE );
    leftHeader->setMovingEnabled( TRUE );
    topHeader = new QTableHeader( numCols, this, this );
    topHeader->setOrientation( Horizontal );
    topHeader->setTracking( TRUE );
    topHeader->setMovingEnabled( TRUE );
    setMargins( 30, fontMetrics().height() + 4, 0, 0 );

    // Initialize headers
    int i = 0;
    for ( i = 0; i < columns(); ++i ) {
	topHeader->setLabel( i, QString::number( i + 1 ) );
	topHeader->resizeSection( i, 100 );
    }
    for ( i = 0; i < rows(); ++i ) {
	leftHeader->setLabel( i, QString::number( i + 1 ) );
	leftHeader->resizeSection( i, 20 );
    }

    // Prepare for contents
    contents.resize( numRows * numCols );
    contents.setAutoDelete( FALSE );

    // Connect header, table and scrollbars
    connect( horizontalScrollBar(), SIGNAL( valueChanged( int ) ),
	     topHeader, SLOT( setOffset( int ) ) );
    connect( verticalScrollBar(), SIGNAL( valueChanged( int ) ),
	     leftHeader, SLOT( setOffset( int ) ) );
    connect( topHeader, SIGNAL( sectionSizeChanged( int ) ),
	     this, SLOT( columnWidthChanged( int ) ) );
    connect( topHeader, SIGNAL( indexChange( int, int, int ) ),
	     this, SLOT( columnIndexChanged( int, int, int ) ) );
    connect( topHeader, SIGNAL( sectionClicked( int ) ),
	     this, SLOT( columnClicked( int ) ) );
    connect( leftHeader, SIGNAL( sectionSizeChanged( int ) ),
	     this, SLOT( rowHeightChanged( int ) ) );
    connect( leftHeader, SIGNAL( indexChange( int, int, int ) ),
	     this, SLOT( rowIndexChanged( int, int, int ) ) );

    // Initialize variables
    autoScrollTimer = new QTimer( this );
    connect( autoScrollTimer, SIGNAL( timeout() ),
	     this, SLOT( doAutoScroll() ) );
    curRow = curCol = -1;
    setCurrentCell( 0, 0 );
    edMode = NotEditing;
    editRow = editCol = -1;
    defValidator = 0;
    editorWidget = 0;

    installEventFilter( this );

    // Initial size
    resize( 640, 480 );
}

/*!
  Destructor.
*/

QTable::~QTable()
{
    for ( int i = 0; i < (int)contents.count(); ++i ) {
	QTableItem *item = contents[ i ];
	if ( item && item->deref() == 0 )
	    delete item;
    }
}

/*!  Returns the QHeader which is used on the top.
*/

QHeader *QTable::horizontalHeader() const
{
    return (QHeader*)topHeader;
}

/*!  Returns the QHeader which is used on the left side.
*/

QHeader *QTable::verticalHeader() const
{
    return (QHeader*)leftHeader;
}

/*!  If \a b is TRUE, the table grid is shown, else not.
 */

void QTable::setShowGrid( bool b )
{
    sGrid = b;
    viewport()->repaint( FALSE );
}

/*!  Returns wheather the table grid is shown.
 */

bool QTable::showGrid() const
{
    return sGrid;
}

/*!  If \a b is set to TRUE, the columns can be moved by the user,
  else not.
*/

void QTable::setColumnsMovable( bool b )
{
    mCols = b;
}

/*!  Returns wheather the columns can be moved by the user.
 */

bool QTable::columnsMovable() const
{
    return mCols;
}

/*!  If \a b is set to TRUE, the rows can be moved by the user, else
  not.
*/

void QTable::setRowsMovable( bool b )
{
    mRows = b;
}

/*!  Returns wheather the rows can be moved by the user.
 */

bool QTable::rowsMovable() const
{
    return mRows;
}

/*!  Draws the contents of the table on the painter \a p. This
  function is optimized to only draw the relevant cells which are
  inside the clipping rectangle \a cx, \a cy, \a cw, \a ch.

  This function also draws also the indication of the current cell.
*/

void QTable::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    int colfirst = columnAt( cx );
    int collast = columnAt( cx + cw );
    int rowfirst = rowAt( cy );
    int rowlast = rowAt( cy + ch );

    if ( rowfirst == -1 || colfirst == -1 ) {
	paintEmptyArea( p, cx, cy, cw, ch );
	return;
    }

    if ( rowlast == -1 )
	rowlast = rows() - 1;
    if ( collast == -1 )
	collast = columns() - 1;

    // Go through the rows
    for ( int r = rowfirst; r <= rowlast; ++r ) {
	// get row position and height
	int rowp = rowPos( r );
	int rowh = rowHeight( r );
	
	// Go through the columns in the row r
	// if we know from where to where, go through [colfirst, collast],
	// else go through all of them
	for ( int c = colfirst; c <= collast; ++c ) {
	    // get position and width of column c
	    int colp, colw;
	    colp = columnPos( c );
	    colw = columnWidth( c );
	    int oldrp = rowp;
	    int oldrh = rowh;
	
	    QTableItem *item = cellContent( r, c );
	    if ( item &&
		 ( item->columnSpan() > 1 || item->rowSpan() > 1 ) ) {
		bool goon = r == item->row && c == item->col ||
			r == rowfirst && c == item->col ||
			r == item->row && c == colfirst;
		if ( !goon )
		    continue;
		rowp = rowPos( item->row );
		rowh = 0;
		int i;
		for ( i = 0; i < item->rowSpan(); ++i )
		    rowh += rowHeight( i + item->row );
		colp = columnPos( item->col );
		colw = 0;
		for ( i = 0; i < item->columnSpan(); ++i )
		    colw += columnWidth( i + item->col );
	    }
	
	    // Translate painter and draw the cell
	    p->saveWorldMatrix();
	    p->translate( colp, rowp );
	    paintCell( p, r, c, QRect( colp, rowp, colw, rowh ), isSelected( r, c ) );
	    p->restoreWorldMatrix();
	
	    rowp = oldrp;
	    rowh = oldrh;
	}
    }

    // draw indication of current cell
    QRect focusRect = cellGeometry( curRow, curCol );
    p->setPen( QPen( black, 1 ) );
    p->setBrush( NoBrush );
    bool focusEdited = FALSE;
    if ( edMode != NotEditing &&
	 curRow == editRow && curCol == editCol )
	focusEdited = TRUE;
    if ( !focusEdited ) {
	QTableItem *i = cellContent( curRow, curCol );
	focusEdited = ( i &&
			( i->editType() == QTableItem::Always ||
			  ( i->editType() == QTableItem::OnCurrent && curRow == i->row && curCol == i->col ) ) );
    }
    p->drawRect( focusRect.x(), focusRect.y(), focusRect.width() - 1, focusRect.height() - 1 );
    if ( !focusEdited ) {
	p->drawRect( focusRect.x() - 1, focusRect.y() - 1, focusRect.width() + 1, focusRect.height() + 1 );
    } else {
	if ( curRow == rows() - 1 )
	    focusRect.setHeight( focusRect.height() - 1 );
	if ( curCol == columns() - 1 )
	    focusRect.setWidth( focusRect.width() - 1 );
	p->drawRect( focusRect.x() - 2, focusRect.y() - 2, focusRect.width() + 3, focusRect.height() + 3 );
    }

    // Paint empty rects
    paintEmptyArea( p, cx, cy, cw, ch );
}

/*!  Paints the cell at the position \a row, \a col on the painter \a
  p. \a cr describes the corrdinates of the cell in contents
  coordinates. But the painter is already translated so that the cells
  origin is at (0/0).

  If you want to draw a custom cell content reimplement this functions
  and do the custom drwaing here. If you use a custom table item, the
  drawing for this item should be implemented there. See
  QTableItem::paint(). So only implement this function for drawing
  items if you e.g. directly retrieve the data which should be drawn
  from a database and should not be stored in an data structure in the
  table (which would be a custom table item).
*/

void QTable::paintCell( QPainter* p, int row, int col, const QRect &cr, bool selected )
{
    if ( selected &&
	 row == curRow &&
	 col == curCol )
	selected = FALSE;

    int w = cr.width();
    int h = cr.height();
    int x2 = w - 1;
    int y2 = h - 1;

    // Draw cell background
    p->fillRect( 0, 0, w, h, selected ? colorGroup().brush( QColorGroup::Highlight ) : colorGroup().brush( QColorGroup::Base ) );

    if ( sGrid ) {
	// Draw our lines
	QPen pen( p->pen() );
	p->setPen( colorGroup().mid().light() );
	p->drawLine( x2, 0, x2, y2 );
	p->drawLine( 0, y2, x2, y2 );
	p->setPen( pen );
    }

    QTableItem *item = cellContent( row, col );
    if ( item ) {
	p->save();
	item->paint( p, colorGroup(), cr, selected );
	p->restore();
    }
}

/*!  This function fills the rect \a cx, \a cy, \a cw, \a ch with the
  basecolor. This is used by drawContents() to erase or fill unused
  areas.
*/

void QTable::paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch )
{
    // Region of the rect we should draw
    QRegion reg( QRect( cx, cy, cw, ch ) );
    // Subtract the table from it
    reg = reg.subtract( QRect( QPoint( 0, 0 ), tableSize() ) );
    p->save();
    // Set clip region...
    p->setClipRegion( reg );
    // ...and fill background
    p->fillRect( cx, cy, cw, ch, colorGroup().brush( QColorGroup::Base ) );
    p->restore();
}

/*!  Returns the QTableItem which represents the contents of the cell
  \a row, \a col. If the \a row or \a col are out of range <b>or</b>
  there has not been set any contents for this cell, this function
  returns 0.
*/

QTableItem *QTable::cellContent( int row, int col ) const
{
    if ( row < 0 || col < 0 || row > rows() - 1 || col > columns() - 1 )
	return 0;
    return contents[ indexOf( row, col ) ];	// contents array lookup
}

/*!  Sets the contents for the cell \a row, \a col. If there did
  already exist an item for this cell, the old one is deleted.

  This function also repaints the cell.
*/

void QTable::setCellContent( int row, int col, QTableItem *item )
{
    if ( !item )
	return;

    QTableItem *i = contents[ indexOf( row, col ) ];
    if ( i ) {
	if ( i->deref() == 0 )
	    delete i;
    }

    contents.insert( indexOf( row, col ), item ); // contents lookup and assign
    item->row = row;
    item->col = col;
    updateCell( row, col ); // repaint
}

/*!  Clears the cell \p row, \a col. This means it removes the table
  item of it if one exists.
*/

void QTable::clearCell( int row, int col )
{
    QTableItem *i = cellContent( row, col );
    contents.remove( indexOf( row, col ) );
    if ( i && i->deref() == 0 )
	    delete i;
}

/*!  Sets the text for the cell \a row, \a col to \a text. This is a
  convenience function of setCellContent()
*/

void QTable::setCellText( int row, int col, const QString &text )
{
    QTableItem *item = cellContent( row, col );
    if ( item ) {
	item->setText( text );
	updateCell( row, col );
    } else {
	QTableItem *i = new QTableItem( this, text, QPixmap() );
	setCellContent( row, col, i );
    }
}

/*!  Sets the pixmap for the cell \a row, \a col to \a pix. This is a
  convenience function of setCellContent()
*/

void QTable::setCellPixmap( int row, int col, const QPixmap &pix )
{
    QTableItem *item = cellContent( row, col );
    if ( item ) {
	item->setPixmap( pix );
	updateCell( row, col );
    } else {
	QTableItem *i = new QTableItem( this, QString::null, pix );
	setCellContent( row, col, i );
    }
}

/*!  Returns the text which is set for the cell \a row, \a col, or an
  emty string if the cell has no text.
*/

QString QTable::cellText( int row, int col ) const
{
    QTableItem *item = cellContent( row, col );
    if ( item )
	return item->text();
    return QString::null;
}

/*!  Returns the pixmap which is set for the cell \a row, \a col, or a
  null-pixmap if the cell has no pixmap.
*/

QPixmap QTable::cellPixmap( int row, int col ) const
{
    QTableItem *item = cellContent( row, col );
    if ( item )
	return item->pixmap();
    return QPixmap();
}

/*!  Sets the current cell to \a row, \a col.
*/

void QTable::setCurrentCell( int row, int col )
{
    QTableItem *item = cellContent( row, col );
    QTableItem *oldIitem = cellContent( curRow, curCol );
    if ( item && item->rowSpan() > 1 && oldIitem == item && item->row != row ) {
	if ( row > curRow )
	    row = item->row + item->rowSpan();
	else if ( row < curRow )
	    row = QMAX( 0, item->row - 1 );
    }
    if ( item && item->columnSpan() > 1 && oldIitem == item && item->col != col ) {
	if ( col > curCol )
	    col = item->col + item->columnSpan();
	else if ( col < curCol )
	    col = QMAX( 0, item->col - 1 );
    }
    if ( curRow != row || curCol != col ) {
	item = oldIitem;
	if ( item && item->editType() == QTableItem::OnCurrent )
	    endEdit( curRow, curCol, TRUE, item->lastEditor, Editing );
	int oldRow = curRow;
	int oldCol = curCol;
	curRow = row;
	curCol = col;
	updateCell( oldRow, oldCol );
	updateCell( curRow, curCol );
	ensureCellVisible( curRow, curCol );
	emit currentChanged( row, col );
	if ( !isColumnSelected( oldCol ) && !isRowSelected( oldRow ) ) {
	    topHeader->setSectionState( oldCol, QTableHeader::Normal );
	    leftHeader->setSectionState( oldRow, QTableHeader::Normal );
	}
	topHeader->setSectionState( curCol, isColumnSelected( curCol, TRUE ) ? QTableHeader::Selected : QTableHeader::Bold );
	leftHeader->setSectionState( curRow, isRowSelected( curRow, TRUE ) ? QTableHeader::Selected : QTableHeader::Bold );
	item = cellContent( curRow, curCol );
	if ( item && item->editType() == QTableItem::OnCurrent )
	    beginEdit( curRow, curCol, FALSE );
	else if ( item && item->editType() == QTableItem::Always )
	    item->lastEditor->setFocus();
	editRow = editCol = -1;
	edMode = NotEditing;
    }
}

/*!  Scrolls the table so that the cell \a row, \a col is made
  visible.
*/

void QTable::ensureCellVisible( int row, int col )
{
    int cw = columnWidth( col );
    int rh = rowHeight( row );
    ensureVisible( columnPos( col ) + cw / 2, rowPos( row ) + rh / 2, cw / 2, rh / 2 );
}

/*!  Returns wheather the cell \a row, \a col is selected.
 */

bool QTable::isSelected( int row, int col ) const
{
    QListIterator<SelectionRange> it( selections );
    SelectionRange *s;
    while ( ( s = it.current() ) != 0 ) {
	++it;
	if ( s->active &&
	     row >= s->topRow &&
	     row <= s->bottomRow &&
	     col >= s->leftCol &&
	     col <= s->rightCol )
	    return TRUE;
	if ( row == currentRow() && col == currentColumn() )
	    return TRUE;
    }
    return FALSE;
}

/*!  Returns TRUE if the row \a row is selected. If \a full is FALSE,
  TRUE is returned if at least one cell the row is selected, else TRUE
  is only returned if all cells of the row are selected.
*/

bool QTable::isRowSelected( int row, bool full ) const
{
    if ( !full ) {
	QListIterator<SelectionRange> it( selections );
	SelectionRange *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->active &&
		 row >= s->topRow &&
		 row <= s->bottomRow )
	    return TRUE;
	if ( row == currentRow() )
	    return TRUE;
	}
    } else {
	QListIterator<SelectionRange> it( selections );
	SelectionRange *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->active &&
		 row >= s->topRow &&
		 row <= s->bottomRow &&
		 s->leftCol == 0 &&
		 s->rightCol == columns() - 1 )
		return TRUE;
	}
    }
    return FALSE;
}

/*!  Returns TRUE if the columns \a col is selected. If \a full is
  FALSE, TRUE is returned if at least one cell the column is selected,
  else TRUE is only returned if all cells of the columns are selected.
*/

bool QTable::isColumnSelected( int col, bool full ) const
{
    if ( !full ) {
	QListIterator<SelectionRange> it( selections );
	SelectionRange *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->active &&
		 col >= s->leftCol &&
		 col <= s->rightCol )
	    return TRUE;
	if ( col == currentColumn() )
	    return TRUE;
	}
    } else {
	QListIterator<SelectionRange> it( selections );
	SelectionRange *s;
	while ( ( s = it.current() ) != 0 ) {
	    ++it;
	    if ( s->active &&
		 col >= s->leftCol &&
		 col <= s->rightCol &&
		 s->topRow == 0 &&
		 s->bottomRow == rows() - 1 )
		return TRUE;
	}
    }
    return FALSE;
}

/*!  Returns the number of selections.
 */

int QTable::selectionCount() const
{
    return selections.count();
}

/*!  Sets \a topRow, \a leftCol, \a bottomRow and \a rightCol to the
  edges of the selection \a num. If successful TRUE is returned, else
  FALSE.
*/

bool QTable::selection( int num, int &topRow, int &leftCol, int &bottomRow, int &rightCol )
{
    if ( num < 0 || num >= (int)selections.count() ) {
	topRow = leftCol = bottomRow = rightCol = -1;
	return FALSE;
    }

    SelectionRange *s = selections.at( num );
    topRow = s->topRow;
    leftCol = s->leftCol;
    bottomRow = s->bottomRow;
    rightCol = s->rightCol;
    return TRUE;
}

/*!  \reimp
*/

void QTable::contentsMousePressEvent( QMouseEvent* e )
{
    if ( editorWidget && isEditing() )
 	endEdit( editRow, editCol, TRUE, editorWidget, edMode );

    int curRow = rowAt( e->pos().y() );
    int curCol = columnAt( e->pos().x() );
    fixRow( curRow, e->pos().y() );
    fixCol( curCol, e->pos().x() );

    if ( currentSelection && currentSelection->active &&
	 ( currentSelection->anchorCol != curCol || currentSelection->anchorRow != curRow ) )
	setCurrentCell( currentSelection->anchorRow, currentSelection->anchorCol );

    if ( ( e->state() & ShiftButton ) == ShiftButton ) {
	if ( !currentSelection ) {
	    currentSelection = new SelectionRange();
	    selections.append( currentSelection );
	    currentSelection->init( curRow, curCol );
	}
	SelectionRange oldSelection = *currentSelection;
	currentSelection->expandTo( curRow, curCol );
	repaintSelections( &oldSelection, currentSelection );
    } else if ( ( e->state() & ControlButton ) == ControlButton ) {
	currentSelection = new SelectionRange();
	selections.append( currentSelection );
	currentSelection->init( curRow, curCol );
    } else {
	clearSelection();
	currentSelection = new SelectionRange();
	selections.append( currentSelection );
	currentSelection->init( curRow, curCol );
    }
    setCurrentCell( curRow, curCol );
}

/*!  \reimp
*/

void QTable::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    int curRow = rowAt( e->pos().y() );
    int curCol = columnAt( e->pos().x() );
    if ( curRow != -1 && curCol != -1 )
	beginEdit( curRow, curCol, FALSE );
}

/*!  \reimp
*/

void QTable::contentsMouseMoveEvent( QMouseEvent *e )
{
    int curRow = rowAt( e->pos().y() );
    int curCol = columnAt( e->pos().x() );
    fixRow( curRow, e->pos().y() );
    fixCol( curCol, e->pos().x() );

    QPoint pos = mapFromGlobal( e->globalPos() );
    pos -= QPoint( leftHeader->width(), topHeader->height() );
    autoScrollTimer->stop();
    doAutoScroll();
    if ( pos.x() < 0 || pos.x() > visibleWidth() || pos.y() < 0 || pos.y() > visibleHeight() )
	autoScrollTimer->start( 100, TRUE );
}

/*! \internal
 */

void QTable::doAutoScroll()
{
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal( pos );
    pos -= QPoint( leftHeader->width(), topHeader->height() );

    pos += QPoint( contentsX(), contentsY() );

    int curRow = rowAt( pos.y() );
    int curCol = columnAt( pos.x() );
    fixRow( curRow, pos.y() );
    fixCol( curCol, pos.x() );

    ensureCellVisible( curRow, curCol );

    if ( !currentSelection )
	return;

    SelectionRange oldSelection = *currentSelection;
    currentSelection->expandTo( curRow, curCol );
    repaintSelections( &oldSelection, currentSelection );

    if ( pos.x() < 0 || pos.x() > visibleWidth() || pos.y() < 0 || pos.y() > visibleHeight() )
	autoScrollTimer->start( 100, TRUE );

    setCurrentCell( curRow, curCol );
}

/*! \reimp
 */

void QTable::contentsMouseReleaseEvent( QMouseEvent * )
{
    autoScrollTimer->stop();
}

/*!  \reimp
*/

bool QTable::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return QScrollView::eventFilter( o, e );

    switch ( e->type() ) {
    case QEvent::KeyPress: {
	QTableItem *item = cellContent( curRow, curCol );
	if ( isEditing() && editorWidget && o == editorWidget ) {
	    item = cellContent( editRow, editCol );
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Escape ) {
		if ( !item || item->editType() == QTableItem::OnActivate )
		    endEdit( editRow, editCol, FALSE, editorWidget, edMode );
		return TRUE;
	    }

	    if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
		if ( !item || item->editType() == QTableItem::OnActivate )
		    endEdit( editRow, editCol, TRUE, editorWidget, edMode );
		activateNextCell();
		return TRUE;
	    }

	    if ( edMode == Replacing &&
		 ( ke->key() == Key_Up || ke->key() == Key_Prior || ke->key() == Key_Home ||
		   ke->key() == Key_Down || ke->key() == Key_Next || ke->key() == Key_End ||
		   ke->key() == Key_Left || ke->key() == Key_Right ) ) {
		if ( !item || item->editType() == QTableItem::OnActivate )
		    endEdit( editRow, editCol, TRUE, editorWidget, edMode );
		keyPressEvent( ke );
		return TRUE;
	    }
	} else if ( item &&
		    ( ( item->editType() == QTableItem::OnCurrent &&
			item->row == curRow && item->col == curCol ) ||
		      ( item->editType() == QTableItem::Always ) ) && o == item->lastEditor ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ( ke->state() & ControlButton ) == ControlButton ||
		 ( ke->key() != Key_Left && ke->key() != Key_Right && ke->key() != Key_Up &&
		   ke->key() != Key_Down && ke->key() != Key_Prior && ke->key() != Key_Next &&
		   ke->key() != Key_Home && ke->key() != Key_End ) )
		return FALSE;
	    keyPressEvent( (QKeyEvent*)e );
	    return TRUE;
	}
	
	} break;
    case QEvent::FocusOut:
	if ( o == this || o == viewport() )
	    return TRUE;
	if ( isEditing() && editorWidget && o == editorWidget ) {
	    QTableItem *item = cellContent( editRow, editCol );
 	    if ( !item || item->editType() == QTableItem::OnActivate ) {
 		endEdit( editRow, editCol, TRUE, editorWidget, edMode );
		return TRUE;
	    }
	}
	break;
    case QEvent::FocusIn:
 	if ( o == this || o == viewport() )
	    return TRUE;
	break;
    default:
	break;
    }

    return QScrollView::eventFilter( o, e );
}

/*!  \reimp
*/

void QTable::keyPressEvent( QKeyEvent* e )
{
    if ( isEditing() )
	return;
    int curRow = QTable::curRow;
    int curCol = QTable::curCol;
    int oldRow = curRow;
    int oldCol = curCol;

    bool navigationKey = FALSE;
    switch ( e->key() ) {
    case Key_Left:
	curCol = QMAX( 0, curCol - 1 );
	navigationKey = TRUE;
    	break;
    case Key_Right:
	curCol = QMIN( columns() - 1, curCol + 1 );
	navigationKey = TRUE;
	break;
    case Key_Up:
	curRow = QMAX( 0, curRow - 1 );
	navigationKey = TRUE;
	break;
    case Key_Down:
	curRow = QMIN( rows() - 1, curRow + 1 );
	navigationKey = TRUE;
	break;
    case Key_Prior:
    case Key_Next:
    case Key_Home:
    case Key_End:
	clearSelection();
	navigationKey = TRUE;
	break;
    default: // ... or start in-place editing
	if ( e->text()[ 0 ].isPrint() ) {
	    QTableItem *item = cellContent( curRow, curCol );
	    if ( !item || item->editType() == QTableItem::OnActivate ) {
		beginEdit( curRow, curCol, item ? item->isTypeChangeAllowed() : TRUE );
		if ( editorWidget )
		    QApplication::sendEvent( editorWidget, e );
	    }
	}
    }

    if ( navigationKey ) {
	if ( ( e->state() & ShiftButton ) == ShiftButton ) {
	    setCurrentCell( curRow, curCol );
	    if ( !currentSelection ) {
		currentSelection = new SelectionRange();
		selections.append( currentSelection );
		currentSelection->init( oldRow, oldCol );
	    }
	    SelectionRange oldSelection = *currentSelection;
	    currentSelection->expandTo( curRow, curCol );
	    repaintSelections( &oldSelection, currentSelection );
	} else {
	    clearSelection();
	    setCurrentCell( curRow, curCol );
	}	
    } else {
	setCurrentCell( curRow, curCol );
    }
}

/*!  \reimp
*/

void QTable::focusInEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );
}


/*!  \reimp
*/

void QTable::focusOutEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );
}

/*!  \reimp
*/

bool QTable::focusNextPrevChild( bool next )
{
    if ( editorWidget && isEditing() )
 	return TRUE;
    return QScrollView::focusNextPrevChild( next );
}

/*!  \reimp
*/

void QTable::resizeEvent( QResizeEvent *e )
{
    QScrollView::resizeEvent( e );
    updateGeometries();
}

/*!  \reimp
*/

void QTable::showEvent( QShowEvent *e )
{
    QScrollView::showEvent( e );
    QRect r( cellGeometry( rows() - 1, columns() - 1 ) );
    resizeContents( r.right() + 1, r.bottom() + 1 );
    updateGeometries();
}

static bool inUpdateCell = FALSE;

/*!  Repaints the cell \a row, \a col.
*/

void QTable::updateCell( int row, int col )
{
    if ( inUpdateCell || row == -1 || col == -1 )
	return;
    inUpdateCell = TRUE;
    QRect cg = cellGeometry( row, col );
    QRect r( contentsToViewport( QPoint( cg.x() - 2, cg.y() - 2 ) ), QSize( cg.width() + 4, cg.height() + 4 ) );
    QApplication::postEvent( viewport(), new QPaintEvent( r, FALSE ) );
    QTableItem *i = cellContent( row, col );
    if ( i && i->editType() == QTableItem::Always ) {
	beginEdit( row, col, FALSE );
	editRow = editCol = -1;
	edMode = NotEditing;
    }
    if ( row != curRow || col != curCol )
	viewport()->setFocus();
    inUpdateCell = FALSE;
}

/*!  This function is called if the width of the column \a col has
  been changed.
*/

void QTable::columnWidthChanged( int col )
{
    updateContents( columnPos( col ), 0, contentsWidth(), contentsHeight() );
    QSize s( tableSize() );
    int w = contentsWidth();
    resizeContents( s.width(), s.height() );
    if ( contentsWidth() < w )
	repaintContents( s.width(), 0, w - s.width() + 1, contentsHeight(), TRUE );
    if ( editorWidget && isEditing() ) {
	moveChild( editorWidget, columnPos( curCol ) - 1, rowPos( editRow ) - 1 );
	editorWidget->resize( columnWidth( editCol ) + 1, rowHeight( editRow ) + 1 );
    }

    QTableItem *item = 0;
    for ( int j = col; j < columns(); ++j ) {
	for ( int i = 0; i < rows(); ++i ) {
	    item = cellContent( i, j );
	    if ( !item || item->editType() != QTableItem::Always || !item->lastEditor )
		continue;
	    moveChild( item->lastEditor, columnPos( j ) - 1, rowPos( i ) - 1 );
	    item->lastEditor->resize( columnWidth( j ) + 1, rowHeight( i ) + 1 );
	}
    }

    if ( curCol >= col ) {
	item = cellContent( curRow, curCol );
	if ( item && item->editType() == QTableItem::OnCurrent && item->lastEditor ) {
	    moveChild( item->lastEditor, columnPos( curCol ) - 1, rowPos( curRow ) - 1 );
	    item->lastEditor->resize( columnWidth( curCol ) + 1, rowHeight( curRow ) + 1 );
	}
    }

    updateGeometries();
}

/*!  This function is called if the height of the row \a row has
  been changed.
*/

void QTable::rowHeightChanged( int row )
{
    updateContents( 0, rowPos( row ), contentsWidth(), contentsHeight() );
    QSize s( tableSize() );
    int h = contentsHeight();
    resizeContents( s.width(), s.height() );
    if ( contentsHeight() < h )
	repaintContents( 0, contentsHeight(), contentsWidth(), h - s.height() + 1, TRUE );
    if ( editorWidget && isEditing() ) {
	moveChild( editorWidget, columnPos( editCol ) - 1, rowPos( editRow ) - 1 );
	editorWidget->resize( columnWidth( editCol ) + 1, rowHeight( editRow ) + 1 );
    }

    QTableItem *item = 0;
    for ( int j = row; j < rows(); ++j ) {
	for ( int i = 0; i < columns(); ++i ) {
	    item = cellContent( j, i );
	    if ( !item || item->editType() != QTableItem::Always || !item->lastEditor )
		continue;
	    moveChild( item->lastEditor, columnPos( i ) - 1, rowPos( j ) - 1 );
	    item->lastEditor->resize( columnWidth( i ) + 1, rowHeight( j ) + 1 );
	}
    }

    if ( curRow >= row ) {
	item = cellContent( curRow, curCol );
	if ( item && item->editType() == QTableItem::OnCurrent && item->lastEditor ) {
	    moveChild( item->lastEditor, columnPos( curCol ) - 1, rowPos( curRow ) - 1 );
	    item->lastEditor->resize( columnWidth( curCol ) + 1, rowHeight( curRow ) + 1 );
	}
    }

    updateGeometries();
}

/*!  This function is called if the order of the columns has been
  changed. If you want to change the order programmatically, call
  QHeader::moveSection() on the horizontalHeader() or
  verticalHeader().
*/

void QTable::columnIndexChanged( int, int, int )
{
    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
}

/*!  This function is called if the order of the rows has been
  changed. If you want to change the order programmatically, call
  QHeader::moveSection() on the horizontalHeader() or
  verticalHeader().
*/

void QTable::rowIndexChanged( int, int, int )
{
    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
}

/*!  This function is called of the column \c col has been
  clicked. The default implementation sorts this coumns then of
  sorting() is TRUE
*/

void QTable::columnClicked( int col )
{
    if ( !sorting() )
	return;

    if ( col == lastSortCol ) {
	asc = !asc;
    } else {
	lastSortCol = col;
	asc = TRUE;
    }

    sortColumn( lastSortCol, asc );
}

/*!  If \a b is set to TRUE, clicking on the header of a column sorts
  this column.

  \sa sortColumn()
*/

void QTable::setSorting( bool b )
{
    doSort = b;
}

/*!  Returns wheather clicking on a column header sorts the column.

 \sa setSorting()
 */

bool QTable::sorting() const
{
    return doSort;
}

/*!  This function updates the geometries of the left and top header.
*/

void QTable::updateGeometries()
{
    QSize ts = tableSize();
    if ( topHeader->offset() &&
	 ts.width() < topHeader->offset() + topHeader->width() )
	horizontalScrollBar()->setValue( ts.width() - topHeader->width() );
    if ( leftHeader->offset() &&
	 ts.height() < leftHeader->offset() + leftHeader->height() )
	verticalScrollBar()->setValue( ts.height() - leftHeader->height() );

    leftHeader->setGeometry( leftMargin() - 30 + 2, topMargin() + 2,
			     30, visibleHeight() );
    topHeader->setGeometry( leftMargin() + 2, topMargin() + 2 - fontMetrics().height() - 4,
			    visibleWidth(),
			    fontMetrics().height() + 4 );
}

/*!  Returns the width of the column \a col.
*/

int QTable::columnWidth( int col ) const
{
    return topHeader->sectionSize( col );
}

/*!  Returns the height of the row \a row.
 */

int QTable::rowHeight( int row ) const
{
    return leftHeader->sectionSize( row );
}

/*!  Returns the x-position of the column \a col in contents
  coordinates
*/

int QTable::columnPos( int col ) const
{
    return topHeader->sectionPos( col );
}

/*!  Returns the y-position of the row \a row in contents coordinates
*/

int QTable::rowPos( int row ) const
{
    return leftHeader->sectionPos( row );
}

/*!  Returns the column which is at \a pos. \a pos has to be given in
  contents coordinates.
*/

int QTable::columnAt( int pos ) const
{
    return topHeader->sectionAt( pos );
}

/*!  Returns the row which is at \a pos. \a pos has to be given in
  contents coordinates.
*/

int QTable::rowAt( int pos ) const
{
    return leftHeader->sectionAt( pos );
}

/*!  Returns the bounding rect of the cell \a row, \a col in contents
  coordinates.
*/

QRect QTable::cellGeometry( int row, int col ) const
{
    QTableItem *item = cellContent( row, col );
    if ( !item || item->rowSpan() == 1 && item->columnSpan() == 1 )
	return QRect( columnPos( col ), rowPos( row ),
		      columnWidth( col ), rowHeight( row ) );

    while ( row != item->row )
	row--;
    while ( col != item->col )
	col--;

    QRect rect( columnPos( col ), rowPos( row ),
		columnWidth( col ), rowHeight( row ) );
	
    for ( int r = 1; r < item->rowSpan(); ++r )
	    rect.setHeight( rect.height() + rowHeight( r + row ) );
    for ( int c = 1; c < item->columnSpan(); ++c )
	rect.setWidth( rect.width() + columnWidth( c + col ) );

    return rect;
}

/*!  Returns the size of the table (same as the right/bottom edge of
  the last table cell.
*/

QSize QTable::tableSize() const
{
    return QSize( columnPos( columns() - 1 ) + columnWidth( columns() - 1 ),
		  rowPos( rows() - 1 ) + rowHeight( rows() - 1 ) );
}

/*!  Returns the number of rows of the table.
*/

int QTable::rows() const
{
    return leftHeader->count();
}

/*!  Returns the number of columns of the table.
 */

int QTable::columns() const
{
    return topHeader->count();
}

/*!  Sets the number of rows to \a r.
 */

void QTable::setRows( int r )
{
    if ( r > rows() ) {
	clearSelection();
	while ( rows() < r ) {
	    leftHeader->addLabel( QString::number( rows() + 1 ) );
	    leftHeader->resizeSection( rows() - 1, 20 );
	}
    } else {
	qWarning( "decreasing the number of rows is not implemented yet!" );
	return;
    }
    contents.resize( rows() * columns() );
    QRect r2( cellGeometry( rows() - 1, columns() - 1 ) );
    resizeContents( r2.right() + 1, r2.bottom() + 1 );
    updateGeometries();
    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
}

/*!  Sets the number of columns to \a c.
 */

void QTable::setColumns( int c )
{
    if ( c > columns() ) {
	clearSelection();
	while ( columns() < c ) {
	    topHeader->addLabel( QString::number( columns() + 1 ) );
	    topHeader->resizeSection( columns() - 1, 100 );
	}
    } else {
	qWarning( "decreasing the number of columns is not implemented yet!" );
	return;
    }
    contents.resize( rows() * columns() );
    QRect r( cellGeometry( rows() - 1, columns() - 1 ) );
    resizeContents( r.right() + 1, r.bottom() + 1 );
    updateGeometries();
    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
}

/*!  Sets the QValidator which should be used by default for editing
  cells to \a validator.
*/

void QTable::setDefaultValidator( QValidator *validator )
{
    defValidator = validator;
}

/*!  Returns the validator which should be used by default for editing
  table cells.
*/

QValidator *QTable::defaultValidator() const
{
    return defValidator;
}

/*!  This function returns a widget which should be used as editor for
  the cell \a row, \a col. If \a initFromCell is TRUE, the editor is
  used to edit the current content of the cell (so the editor widget
  should be initialized with that content). Otherwise the content of
  this cell will be replaced by a new content.

  The default implementation looks if there exists a QTableItem for
  the cell. If this is the case and \a initFromCell is TRUE or
  QTableItem::isTypeChangeAllowed() of the cell is FALSE, the item of
  that cell is asked to create the editor (using QTableItem::editor()).

  If this is not the case, defaultEditor() is called to get the editor
  for that cell.

  So if you want to create your own editor for certain cells,
  implement your own QTableItem and reimplement
  QTableItem::editor(). If you want to use a different editor than a
  QLineEdit as default editor, reimplement defaultEditor().

  So normally you do not need to reimplement this function.

  The ownership of the editor widget is transferred to the caller.

  Returning 0 here means that the cell is not editable.

  \sa QTableItem::editor()
*/

QWidget *QTable::editor( int row, int col, bool initFromCell ) const
{
    QWidget *e = 0;

    // the current item in the cell should be edited if possible
    QTableItem *i = cellContent( row, col );
    if ( initFromCell || i && !i->isTypeChangeAllowed() ) {
	if ( i ) {
	    e = i->editor();
	    if ( !e || i->editType() == QTableItem::Never )
		return 0;
	    if ( e )
		i->lastEditor = e;
	}
    }

    // no contents in the cell yet, so open the default editor
    if ( !e )
	e = defaultEditor();

    return e;
}

/*! Returns the editor widget which should be used for editing cells
  which have no QTableItem. The default implementation returns a
  QLineEdit which has the defaultValidator() set.

  The ownership of the editor widget is transferred to the caller.

  \sa editor()
*/

QWidget *QTable::defaultEditor() const
{
    QLineEdit *e = new QLineEdit( viewport() );
    e->setFrame( FALSE );
    e->setValidator( defaultValidator() );
    return e;
}

/*!  This function is called to start in-place editing of the cell \a
  row, \a col. If \a replace is TRUE the content of this cell will be
  replaced by the content of the editor later, else the current
  content of that cell (if existing) will be edited by the editor.

  This function calls editor() to get the editor which should be used
  for editing the cell.

  This function is responsible for creating and placing the editor.
*/

void QTable::beginEdit( int row, int col, bool replace )
{
    QTableItem *item = cellContent( row, col );
    if ( item && item->lastEditor )
	return;
    ensureCellVisible( curRow, curCol );
    editorWidget = editor( row, col, !replace );
    if ( !editorWidget )
	return;
    editorWidget->installEventFilter( this );
    QRect cr = cellGeometry( row, col );
    editorWidget->resize( cr.size() + QSize( 1, 1 ) );
    moveChild( editorWidget, cr.x() - 1, cr.y() - 1 );
    editorWidget->show();
    editorWidget->setFocus();
    edMode = replace ? Replacing : Editing;
    editRow = row;
    editCol = col;
    updateCell( row, col );
}

/*!  This function is called if in-place editing of the cell \a row,
  \a col has to be ended. If \a accept is TRUE the content of the \a
  editor has to be transferred to the cell. If mode is \c Replacing
  the current content of that cell should be replaced by the content
  of the editor (this means removing the current QTableItem of the
  cell and creating a new one for the cell), else (if possible) the
  content of the editor should just be set to the QTableItem of this
  cell.

  So, if the cell contents should be replaced or if no QTableItem
  exists for the cell yet, setCellContentFromEditor() is called, else
  QTableItem::setCellContentFromEditor() is called on the QTableItem
  of the cell.

  This function is also responsible for deleting the editor widget.
*/

void QTable::endEdit( int row, int col, bool accept, QWidget *editor, EditMode mode )
{
    editRow = -1;
    editCol = -1;
    if ( !accept ) {
	editor->removeEventFilter( this );
	delete editor;
	editorWidget = 0;
	updateCell( row, col );
	edMode = NotEditing;
	viewport()->setFocus();
	updateCell( row, col );
	return;
    }

    QTableItem *i = cellContent( row, col );
    if ( mode == Replacing && i ) {
	clearCell( row, col );
	i = 0;
    }

    if ( !i )
	setCellContentFromEditor( row, col, editor );
    else
	i->setContentFromEditor( editor );
    editor->removeEventFilter( this );
    delete editor;
    editorWidget = 0;
    viewport()->setFocus();
    edMode = NotEditing;
    updateCell( row, col );
}

/*!  This function is called to set the contents of the cell \a row,
  \a col from the \a editor. If there existed already a QTableItem for
  this cell, this is removed first (see clearCell()).

  Reimplement this if you want to do something different here.

  \sa QTableItem::setContentFromEditor()
*/

void QTable::setCellContentFromEditor( int row, int col, QWidget *editor )
{
    clearCell( row, col );
    if ( editor->inherits( "QLineEdit" ) )
	setCellText( row, col, ( (QLineEdit*)editor )->text() );
}

/*!  Returns wheather the user is just editing a cell or not.
 */

bool QTable::isEditing() const
{
    return edMode != NotEditing;
}

/*!  Returns the current edit mode. NotEditing means the user doesn't
  edit a cell at the moment.
*/

QTable::EditMode QTable::editMode() const
{
    return edMode;
}

/*!  \internal
*/

int QTable::indexOf( int row, int col ) const
{
    return ( row * columns() ) + col; // mapping from 2D table to 1D array
}

/*!  \internal
*/

void QTable::repaintSelections( SelectionRange *oldSelection, SelectionRange *newSelection,
				bool updateVertical, bool updateHorizontal )
{
    if ( *oldSelection == *newSelection )
	return;
    bool optimize1, optimize2;
    QRect old = rangeGeometry( oldSelection->topRow,
			       oldSelection->leftCol,
			       oldSelection->bottomRow,
			       oldSelection->rightCol,
			       optimize1 );

    QRect cur = rangeGeometry( newSelection->topRow,
			       newSelection->leftCol,
			       newSelection->bottomRow,
			       newSelection->rightCol,
			       optimize2 );

    int i;

    if ( !optimize1 || !optimize2 ) {
	QRect rr = cur.unite( old );
	repaintContents( rr, FALSE );
    } else {
	QRegion r1( old );
	QRegion r2( cur );
	QRegion r3 = r1.subtract( r2 );
	QRegion r4 = r2.subtract( r1 );

	for ( i = 0; i < (int)r3.rects().count(); ++i )
	    repaintContents( r3.rects()[ i ], FALSE );
	for ( i = 0; i < (int)r4.rects().count(); ++i )
	    repaintContents( r4.rects()[ i ], FALSE );
    }
    
    if ( updateHorizontal ) {
	for ( i = 0; i <= columns(); ++i ) {
	    if ( !isColumnSelected( i ) )
		topHeader->setSectionState( i, QTableHeader::Normal );
	    else if ( isColumnSelected( i, TRUE ) )
		topHeader->setSectionState( i, QTableHeader::Selected );
	    else
		topHeader->setSectionState( i, QTableHeader::Bold );
	}
    }

    if ( updateVertical ) {
	for ( i = 0; i <= rows(); ++i ) {
	    if ( !isRowSelected( i ) )
		leftHeader->setSectionState( i, QTableHeader::Normal );
	    else if ( isRowSelected( i, TRUE ) )
		leftHeader->setSectionState( i, QTableHeader::Selected );
	    else
		leftHeader->setSectionState( i, QTableHeader::Bold );
	}
    }
}

/*!  Clears all selections.
 */

void QTable::clearSelection()
{
    bool needRepaint = !selections.isEmpty();

    QRect r;
    for ( SelectionRange *s = selections.first(); s; s = selections.next() ) {
	bool b;
	r = r.unite( rangeGeometry( s->topRow,
				    s->leftCol,
				    s->bottomRow,
				    s->rightCol, b ) );
    }

    currentSelection = 0;
    selections.clear();
    if ( needRepaint )
	repaintContents( r, FALSE );

    int i;
    for ( i = 0; i <= columns(); ++i ) {
	if ( !isColumnSelected( i ) && i != curCol )
	    topHeader->setSectionState( i, QTableHeader::Normal );
	else if ( isColumnSelected( i, TRUE ) )
	    topHeader->setSectionState( i, QTableHeader::Selected );
	else
	    topHeader->setSectionState( i, QTableHeader::Bold );
    }

    for ( i = 0; i <= rows(); ++i ) {
	if ( !isRowSelected( i ) && i != curRow )
	    leftHeader->setSectionState( i, QTableHeader::Normal );
	else if ( isRowSelected( i, TRUE ) )
	    leftHeader->setSectionState( i, QTableHeader::Selected );
	else
	    leftHeader->setSectionState( i, QTableHeader::Bold );
    }
}

/*!  \internal
*/

QRect QTable::rangeGeometry( int topRow, int leftCol, int bottomRow, int rightCol, bool &optimize )
{
    optimize = TRUE;
    QRect rect;
    for ( int r = topRow; r <= bottomRow; ++r ) {
	for ( int c = leftCol; c <= rightCol; ++c ) {
	    rect = rect.unite( cellGeometry( r, c ) );
	    QTableItem *i = cellContent( r, c );
	    if ( i && ( i->rowSpan() > 1 || i->columnSpan() > 1 ) )
		optimize = FALSE;
	}
    }
    return rect;
}

/*!  This is called to activate the next cell if in-place editing was
  finished by pressing the Return key.

  If you want a different behaviour then going from top to bottom,
  reimplement this function.
*/

void QTable::activateNextCell()
{
    if ( !currentSelection || !currentSelection->active ) {
	if ( curRow < rows() - 1 )
	    setCurrentCell( curRow + 1, curCol );
	else if ( curCol < columns() - 1 )
	    setCurrentCell( 0, curCol + 1 );
	else
	    setCurrentCell( 0, 0 );	
    } else {
	if ( curRow < currentSelection->bottomRow )
	    setCurrentCell( curRow + 1, curCol );
	else if ( curCol < currentSelection->rightCol )
	    setCurrentCell( currentSelection->topRow, curCol + 1 );
	else
	    setCurrentCell( currentSelection->topRow, currentSelection->leftCol );	
    }

}

/*!  \internal
*/

void QTable::fixRow( int &row, int y )
{
    if ( row == -1 ) {
	if ( y < 0 )
	    row = 0;
	else
	    row = rows() - 1;
    }
}

/*!  \internal
*/

void QTable::fixCol( int &col, int x )
{
    if ( col == -1 ) {
	if ( x < 0 )
	    col = 0;
	else
	    col = columns() - 1;
    }
}

struct SortableTableItem
{
    QTableItem *item;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static int cmpTableItems( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
	return 0;

    SortableTableItem *i1 = (SortableTableItem *)n1;
    SortableTableItem *i2 = (SortableTableItem *)n2;

    return i1->item->key().compare( i2->item->key() );
}

#if defined(Q_C_CALLBACKS)
}
#endif

/*!  Sorts the column \a col in ascending order if \a ascending is
  TRUE, else in descending order.

  In this implementation really only the column is sorted. If you need
  to do more (e.g. moving whole rows instead of only cells when
  re-sorting), you have to reimplement this function and do what you need.

  \internal

  Should we have a moveCell( from, to ) is used and can be
  reimplemented to move whole rows?
*/

void QTable::sortColumn( int col, bool ascending )
{
    int filledRows = 0, i;
    for ( i = 0; i < rows(); ++i ) {
	QTableItem *item = cellContent( i, col );
	if ( item )
	    filledRows++;
    }

    if ( !filledRows )
	return;

    SortableTableItem *items = new SortableTableItem[ filledRows ];
    int j = 0;
    for ( i = 0; i < rows(); ++i ) {
	QTableItem *item = cellContent( i, col );
	if ( !item )
	    continue;
	items[ j++ ].item = item;
    }

    qsort( items, filledRows, sizeof( SortableTableItem ), cmpTableItems );

    for ( i = 0; i < rows(); ++i ) {
	contents.remove( indexOf( i, col ) );
	if ( i < filledRows ) {
	    if ( ascending )
		contents.insert( indexOf( i, col ), items[ i ].item );
	    else
		contents.insert( indexOf( i, col ), items[ filledRows - i - 1 ].item );
	}
    }

    repaintContents( columnPos( col ), 0, columnWidth( col ), contentsHeight(), FALSE );
    delete [] items;
}

/*!  Hides the row \a row.

  \sa showRow()
*/

void QTable::hideRow( int row )
{
    leftHeader->resizeSection( row, 0 );
    leftHeader->setResizeEnabled( FALSE, row );
    rowHeightChanged( row );
}

/*!  Hides the column \a col.

  \sa showCol()
*/

void QTable::hideColumn( int col )
{
    topHeader->resizeSection( col, 0 );
    topHeader->setResizeEnabled( FALSE, col );
    columnWidthChanged( col );
}

/*!  Shows the row \a row.

  \sa hideRow()
*/

void QTable::showRow( int row )
{
    leftHeader->resizeSection( row, 100 );
    leftHeader->setResizeEnabled( TRUE, row );
    rowHeightChanged( row );
}

/*!  Shows the column \a col.

  \sa hideColumn()
*/

void QTable::showColumn( int col )
{
    topHeader->resizeSection( col, 100 );
    topHeader->setResizeEnabled( TRUE, col );
    columnWidthChanged( col );
}

/*! Resizes the column to \a w pixel wide.
 */

void QTable::setColumnWidth( int col, int w )
{
    topHeader->resizeSection( col, w );
    columnWidthChanged( col );
}

/*! Resizes the row to be \a h pixel height.
 */

void QTable::setRowHeight( int row, int h )
{
    leftHeader->resizeSection( row, h );
    rowHeightChanged( row );
}

/*!  Resizes the column \a col to be exactly wide enough so that the
  whole contents is visible.
*/

void QTable::adjustColumn( int col )
{
    int w = 20;
    w = QMAX( w, topHeader->fontMetrics().width( topHeader->label( col ) ) + 10 );
    for ( int i = 0; i < rows(); ++i ) {
	QTableItem *item = cellContent( i, col );
	if ( !item )
	    continue;
	w = QMAX( w, item->sizeHint().width() );
    }
    setColumnWidth( col, w );
}

/*!  Resizes the row \a row to be exactly hight enough so that the
  whole contents is visible.
*/

void QTable::adjustRow( int row )
{
    int h = 20;
    h = QMAX( h, leftHeader->fontMetrics().height() );
    for ( int i = 0; i < columns(); ++i ) {
	QTableItem *item = cellContent( row, i );
	if ( !item )
	    continue;
	h = QMAX( h, item->sizeHint().height() );
    }
    setRowHeight( row, h );
}

/*!  Sets the column \a col to stretchable if \a stretch is TRUE, else
  to non-stretchable. So, if the table widgets gets wider than its
  contents, stretchable columns are stretched so that the contents
  fits exactly into to widget.
*/

void QTable::setColumnStretchable( int col, bool stretch )
{
    topHeader->setSectionStretchable( col, stretch );
}

/*!  Sets the row \a row to stretchable if \a stretch is TRUE, else to
  non-stretchable. So, if the table widgets gets higher than its
  contents, stretchable rows are stretched so that the contents fits
  exactly into to widget.
*/

void QTable::setRowStretchable( int row, bool stretch )
{
    leftHeader->setSectionStretchable( row, stretch );
}

/*! Returns wheather the column \a col is stretchable or not.

  \sa setColumnStretchable()
*/

bool QTable::isColumnStretchable( int col ) const
{
    return topHeader->isSectionStretchable( col );
}

/*! Returns wheather the row \a row is stretchable or not.

  \sa setRowStretchable()
*/

bool QTable::isRowStretchable( int row ) const
{
    return leftHeader->isSectionStretchable( row );
}

/*!  \internal
*/

void QTable::editTypeChanged( QTableItem *i, QTableItem::EditType old )
{
    if ( old == QTableItem::Always )
	endEdit( i->row, i->col, TRUE, i->lastEditor, Editing );
    updateCell( i->row, i->col );
}








void QTable::SelectionRange::init( int row, int col )
{
    anchorCol = leftCol = rightCol = col;
    anchorRow = topRow = bottomRow = row;
    active = FALSE;
}

void QTable::SelectionRange::expandTo( int row, int col )
{
    if ( row < anchorRow ) {
	topRow = row;
	bottomRow = anchorRow;
    } else {
	topRow = anchorRow;
	bottomRow = row;
    }

    if ( col < anchorCol ) {
	leftCol = col;
	rightCol = anchorCol;
    } else {
	leftCol = anchorCol;
	rightCol = col;
    }

    active = topRow != bottomRow || leftCol != rightCol;
}

bool QTable::SelectionRange::operator==( const SelectionRange &s ) const
{
    return ( s.active == active &&
	     s.topRow == topRow &&
	     s.bottomRow == bottomRow &&
	     s.leftCol == leftCol &&
	     s.rightCol == rightCol );
}















QTableHeader::QTableHeader( int i, QTable *t, QWidget *parent, const char *name )
    : QHeader( i, parent, name ), table( t ), caching( FALSE ), numStretchs( 0 )
{
    states.resize( i );
    stretchable.resize( i );
    states.fill( Normal, -1 );
    stretchable.fill( FALSE, -1 );
    mousePressed = FALSE;
    autoScrollTimer = new QTimer( this );
    connect( autoScrollTimer, SIGNAL( timeout() ),
	     this, SLOT( doAutoScroll() ) );
    line1 = new QWidget( table->viewport() );
    line1->hide();
    line1->setBackgroundMode( PaletteText );
    table->addChild( line1 );
    line2 = new QWidget( table->viewport() );
    line2->hide();
    line2->setBackgroundMode( PaletteText );
    table->addChild( line2 );

    connect( this, SIGNAL( sizeChange( int, int, int ) ),
	     this, SLOT( sectionWidthChanged( int, int, int ) ) );
}

void QTableHeader::addLabel( const QString &s )
{
    states.resize( states.count() + 1 );
    states[ (int)states.count() - 1 ] = Normal;
    stretchable.resize( stretchable.count() + 1 );
    stretchable[ (int)stretchable.count() - 1 ] = FALSE;
    QHeader::addLabel( s );
}

void QTableHeader::setSectionState( int s, SectionState state )
{
    if ( s < 0 || s >= (int)states.count() )
	return;
    if ( states[ s ] == state )
	return;

    states[ s ] = state;
    if ( orientation() == Horizontal )
	repaint( sectionPos( s ) - 2 - offset(), 0, sectionSize( s ) + 4, height(), FALSE );
    else
	repaint( 0, sectionPos( s ) - 2 - offset(), width(), sectionSize( s ) + 4, FALSE );
}

QTableHeader::SectionState QTableHeader::sectionState( int s ) const
{
    return states[ s ];
}

void QTableHeader::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.setPen( colorGroup().buttonText() );
    int pos = orientation() == Horizontal
		     ? e->rect().left()
		     : e->rect().top();
    int id = mapToIndex( sectionAt( pos + offset() ) );
    if ( id < 0 )
	if ( pos > 0 )
	    return;
	else
	    id = 0;
    for ( int i = id; i < count(); i++ ) {
	QRect r = sRect( i );
	p.save();
	if ( sectionState( i ) == Bold || sectionState( i ) == Selected ) {
	    QFont f( font() );
	    f.setBold( TRUE );
	    p.setFont( f );
	}	
	paintSection( &p, i, r );
	p.restore();
	if ( orientation() == Horizontal && r. right() >= e->rect().right() ||
	     orientation() == Vertical && r. bottom() >= e->rect().bottom() )
	    return;
    }
}

void QTableHeader::paintSection( QPainter *p, int index, QRect fr )
{
    int section = mapToSection( index );
    if ( section < 0 )
	return;

    if ( sectionState( index ) != Selected ) {
	QHeader::paintSection( p, index, fr );
    } else {
	style().drawBevelButton( p, fr.x(), fr.y(), fr.width(), fr.height(),
				 colorGroup(), TRUE );
	paintSectionLabel( p, index, fr );
    }
}

static int real_pos( const QPoint &p, Qt::Orientation o )
{
    if ( o == Qt::Horizontal )
	return p.x();
    return p.y();
}

void QTableHeader::mousePressEvent( QMouseEvent *e )
{
    QHeader::mousePressEvent( e );
    mousePressed = TRUE;
    pressPos = real_pos( e->pos(), orientation() );
    startPos = -1;
    setCaching( TRUE );
    resizedSection = -1;
    isResizing = cursor().shape() != ArrowCursor;
}

void QTableHeader::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed || cursor().shape() != ArrowCursor ||
	 ( ( e->state() & ControlButton ) == ControlButton && ( orientation() == Horizontal ?
								table->columnsMovable() : table->rowsMovable() ) ) ) {
	QHeader::mouseMoveEvent( e );
	return;
    }

    int p = real_pos( e->pos(), orientation() ) + offset();
    if ( startPos == -1 ) {
	startPos = p;
	if ( ( e->state() & ControlButton ) != ControlButton )
	    table->clearSelection();
	saveStates();
	table->currentSelection = new QTable::SelectionRange();
	table->selections.append( table->currentSelection );
	if ( orientation() == Vertical ) {
	    table->setCurrentCell( sectionAt( p ), 0 );
	    table->currentSelection->init( sectionAt( p ), 0 );
	} else {
	    table->setCurrentCell( 0, sectionAt( p ) );
	    table->currentSelection->init( 0, sectionAt( p ) );
	}
    }
    if ( sectionAt( p ) != -1 )
	endPos = p;
    if ( startPos != -1 ) {
	updateSelections();
	p -= offset();
	if ( orientation() == Horizontal && ( p < 0 || p > width() ) ) {
	    doAutoScroll();
	    autoScrollTimer->start( 100, TRUE );
	} else if ( orientation() == Vertical && ( p < 0 || p > height() ) ) {
	    doAutoScroll();
	    autoScrollTimer->start( 100, TRUE );
	}
    } else {
	QHeader::mouseMoveEvent( e );
    }
}

void QTableHeader::mouseReleaseEvent( QMouseEvent *e )
{
    autoScrollTimer->stop();
    mousePressed = FALSE;
    QHeader::mouseReleaseEvent( e );
    line1->hide();
    line2->hide();
    bool hasCached = resizedSection != -1;
    setCaching( FALSE );
    if ( hasCached ) {
	table->repaintContents( table->contentsX(), table->contentsY(), table->visibleWidth(), table->visibleHeight(), FALSE );
	emit sectionSizeChanged( resizedSection );
    }
}

void QTableHeader::mouseDoubleClickEvent( QMouseEvent *e )
{
    if ( isResizing ) {
	int p = real_pos( e->pos(), orientation() ) + offset();
	int section = sectionAt( p ) - 1;
	if ( orientation() == Horizontal )
	    table->adjustColumn( section );
	else
	    table->adjustRow( section );
    }
}

void QTableHeader::resizeEvent( QResizeEvent *e )
{
    QHeader::resizeEvent( e );
    if ( numStretchs == 0 )
	return;
    if ( orientation() == Horizontal ) {
	if ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) == width() )
	    return;
	int pw = width() - ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) ) - 1;
	pw /= numStretchs;
	for ( int i = 0; i < (int)stretchable.count(); ++i ) {
	    if ( !stretchable[ i ] )
		continue;
	    int nw = sectionSize( i ) + pw;
	    if ( nw >= 20 )
		table->setColumnWidth( i, nw );
	}
    } else {
	if ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) == height() )
	    return;
	int ph = height() - ( sectionPos( count() - 1 ) + sectionSize( count() - 1 ) ) - 1;
	ph /= numStretchs;
	for ( int i = 0; i < (int)stretchable.count(); ++i ) {
	    if ( !stretchable[ i ] )
		continue;
	    int nh = sectionSize( i ) + ph;
	    if ( nh >= 20 )
		table->setRowHeight( i, nh );
	}
    }
}

void QTableHeader::updateSelections()
{
    int a = sectionAt( startPos );
    int b = sectionAt( endPos );
    int start = QMIN( a, b );
    int end = QMAX( a, b );
    for ( int i = 0; i < count(); ++i ) {
	if ( i < start || i > end )
	    setSectionState( i, oldStates[ i ] );
	else
	    setSectionState( i, Selected );
    }

    QTable::SelectionRange oldSelection = *table->currentSelection;
    if ( orientation() == Vertical )
	table->currentSelection->expandTo( b, table->horizontalHeader()->count() - 1 );
    else
	table->currentSelection->expandTo( table->verticalHeader()->count() - 1, b );
    table->repaintSelections( &oldSelection, table->currentSelection,
			      orientation() == Horizontal, orientation() == Vertical );
}

void QTableHeader::saveStates()
{
    oldStates.resize( count() );
    for ( int i = 0; i < count(); ++i )
	oldStates[ i ] = sectionState( i );
}

void QTableHeader::doAutoScroll()
{
    QPoint pos = mapFromGlobal( QCursor::pos() );
    int p = real_pos( pos, orientation() ) + offset();
    if ( sectionAt( p ) != -1 )
	endPos = p;
    if ( orientation() == Horizontal )
	table->ensureVisible( endPos, table->contentsY() );
    else
	table->ensureVisible( table->contentsX(), endPos );
    updateSelections();
    autoScrollTimer->start( 100, TRUE );
}

void QTableHeader::sectionWidthChanged( int col, int, int )
{
    resizedSection = col;
    if ( orientation() == Horizontal ) {
	table->moveChild( line1, QHeader::sectionPos( col ) - 1, table->contentsY() );
	line1->resize( 1, table->visibleHeight() );
	line1->show();
	line1->raise();
	table->moveChild( line2, QHeader::sectionPos( col ) + QHeader::sectionSize( col ) - 1, table->contentsY() );
	line2->resize( 1, table->visibleHeight() );
	line2->show();
	line2->raise();
    } else {	
	table->moveChild( line1, table->contentsX(), QHeader::sectionPos( col ) - 1 );
	line1->resize( table->visibleWidth(), 1 );
	line1->show();
	line1->raise();
	table->moveChild( line2, table->contentsX(), QHeader::sectionPos( col ) + QHeader::sectionSize( col ) - 1 );
	line2->resize( table->visibleWidth(), 1 );
	line2->show();
	line2->raise();
    }
}

int QTableHeader::sectionSize( int section ) const
{
    if ( caching )
	return sectionSizes[ section ];
    return QHeader::sectionSize( section );
}

int QTableHeader::sectionPos( int section ) const
{
    if ( caching )
	return sectionPoses[ section ];
    return QHeader::sectionPos( section );
}

int QTableHeader::sectionAt( int pos ) const
{
    if ( !caching )
	return QHeader::sectionAt( pos );
    int l = 0;
    int r = count() - 1;
    int i = ( (l+r+1) / 2 );
    while ( r - l ) {
	if ( sectionPoses[i] > pos )
	    r = i -1;
	else
	    l = i;
	i = ( (l+r+1) / 2 );
    }
    if ( sectionPoses[i] <= pos && pos <= sectionPoses[i] + sectionSizes[ mapToSection( i ) ] )
	return mapToSection( i );
    return -1;
}

void QTableHeader::setCaching( bool b )
{
    if ( caching == b )
	return;
    caching = b;
    sectionPoses.resize( count() );
    sectionSizes.resize( count() );
    if ( b ) {
	for ( int i = 0; i < count(); ++i ) {
	    sectionSizes[ i ] = QHeader::sectionSize( i );
	    sectionPoses[ i ] = QHeader::sectionPos( i );
	}
    }
}

void QTableHeader::setSectionStretchable( int s, bool b )
{
    if ( stretchable[ s ] == b )
	return;
    stretchable[ s ] = b;
    if ( b )
	numStretchs++;
    else
	numStretchs--;
}

bool QTableHeader::isSectionStretchable( int s ) const
{
    return stretchable[ s ];
}
