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
#include <qobjectlist.h>
#include <stdlib.h>

/*!
  \class QTableItem qtable.h

  \brief Implementation of an item for a QTable.

  This item contains the data of a table cell, specifies the editor
  and edit type for that cell, defines some other behaviour and
  provides the API needed for sorting table items. By default it can
  contain a text and pixmaps and offers a QLineEdit for editing.

  By reimplementing paint(), key(), createEditor() and
  setContentFromEditor() you can change this.

  To get rid of an item, just delete it. This does all the required
  actions for remiving it from the table too.
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

QTableItem::QTableItem( QTable *table, EditType et, const QString &t )
    : txt( t ), pix(), t( table ), edType( et ), wordwrap( FALSE ),
      tcha( TRUE ), row( -1 ), col( -1 ), rowspan( 1 ), colspan( 1 )
{
}

/*!  Creates an item for the table \a table with the text \a t and the
  pixmap \a p.
*/

QTableItem::QTableItem( QTable *table, EditType et, const QString &t, const QPixmap &p )
    : txt( t ), pix( p ), t( table ), edType( et ), wordwrap( FALSE ),
      tcha( TRUE ), row( -1 ), col( -1 ), rowspan( 1 ), colspan( 1 )
{
}

/*!  Destructor.
*/

QTableItem::~QTableItem()
{
    table()->takeItem( this );
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
	( (QTableItem*)this )->setContentFromEditor( table()->cellWidget( row, col ) );
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

/*!  Returns the editor which should be used for editing the contents
  of this item. Returning 0 means that this cell is not editable. If
  you reimplement this function to use a custom editor widget always
  create a new widget here as parent of table()->viewport(), as the
  ownership of it is trasferred to the caller.

  \sa QTable::createEditor()
*/

QWidget *QTableItem::createEditor() const
{
    QLineEdit *e = new QLineEdit( table()->viewport() );
    e->setFrame( FALSE );
    e->setText( text() );
    return e;
}

/*!  This function is called to set the cell contents from the editor
  \a w which was use for editing this cell. You might reimplement this
  in subclasses.

  \sa QTable::setContentFromEditor()
*/

void QTableItem::setContentFromEditor( QWidget *w )
{
    if ( w && w->inherits( "QLineEdit" ) )
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

/*!\internal
 */

void QTableItem::updateEditor( int oldRow, int oldCol )
{
    if ( edType != Always )
	return;
    if ( oldRow != -1 && oldCol != -1 )
	table()->clearCellWidget( oldRow, oldCol );
    if ( row != -1 && col != -1 )
	table()->setCellWidget( row, col, createEditor() );
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

void QTableItem::setReplacable( bool b )
{
    tcha = b;
}

/*!  Returns if it is allowed to replace this item if it is set to a
  cell once.

  \sa setReplacable()
*/

bool QTableItem::isReplacable() const
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
    if ( edType == Always && table()->cellWidget( row, col ) )
	return table()->cellWidget( row, col )->sizeHint();

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
    int rrow = row;
    int rcol = col;
    if ( rowspan > 1 || colspan > 1 ) {
	table()->takeItem( this );
	table()->setItem( rrow, rcol, this );
    }

    rowspan = rs;
    colspan = cs;

    for ( int r = 0; r < rowspan; ++r ) {
	for ( int c = 0; c < colspan; ++c ) {
	    if ( r == 0 && c == 0 )
		continue;
	    table()->setItem( r + row, c + col, this );
	    row = rrow;
	    col = rcol;
	}
    }

    table()->updateCell( row, col );
}

/*! Returns the row-span of this item
 */

int QTableItem::rowSpan() const
{
    return rowspan;
}

/*! Returns the col-span of this item
 */

int QTableItem::colSpan() const
{
    return colspan;
}


/*!
  \class QTable qtable.h

  \brief Implementation of a flexible and editable table widget.

  This widget is the implementation of an editable table widget. The
  conecpt is that initially there is no memory allocated for any
  cell. But if a table cell should get a content create a QTableItem
  for that and set it using setItem(). There exist convenience
  functions for setting table text and pixmaps (setText(),
  setPixmap()). These functions just create an item for the
  required cell on demand. To clear a cell use clearCell().

  If you want to draw a custom content for a cell normally you
  implement your own subclass of QTableItem and reimplement the
  QTableItem::paint() method which is then used for drawing the contents.

  If you have your data already in a datastructure in your application
  and do not want to allocate a QTableItem for each cell with a
  contents, you can reimplement QTable::paintCell() and draw there
  directly the contents without the need of any QTableItems. If you do
  this you can't of course use setText() and friends for these
  cells (except you reimplement them and do something different). Also
  you have of course to repaint the cells which changed yourself using
  updateCell().

  The in-place editing is also made in an abstract way so that it is
  possible to have custom edit widgets for certain cells or types or
  cells. First it is generally possible to place widgets in cells. See
  setCellWidget(), clearCellWidget() and cellWidget() for further
  details. For in-place editing these functions plus some additional
  for more specific handling are provided.

  When in-plcae editing is started beginEdit() is called. This
  creates the editor widget for the required cell, places and shows
  it. To create an editor widget this function calls createEditor() for
  the required cell. See the documentation of createEditor() for more
  detailed documentation of that.

  Now there exist two different ways to edit a cell. Either offer an
  edit widget to enter a contents which should replace the current
  cell's contents, or offer an editor to edit the current cell's
  contents. If it shouldn't be possible to replace the contents of a
  cell, but just edit the current content, set a QTableItem for that
  cell and set QTableItem::isReplacable() of that item to FALSE.

  There are also different ways for starting in-place
  editing. Normally if the user starts typing text in-place editing
  (Replacing) for the current cell is started. If the user
  double-clicks on a cell also in-place editing is started for the
  cell he doubleclicked on (Editing).  But it is sometimes required
  that a cell always shows an editor, shows the editor as soon as it
  gets the current cell or that it is not editable at all. This
  edit-type has to be specified in the constructor of a QTableItem.

  Now when the user finishes editing endEdit() is called. Look at
  the documentation of endEdit() for more information on that
  (e.g. how the contents from the editor is transferred to the item
  and how the editor gets destroyed.)

  If you want to make a cell not editable and do not want to waste a
  QTableItem for this cell, reimplement createEditor() and return 0
  there for the cells which should be not editable.

  As mentioned above it is possible to use the QTable without
  QTableItems. The default implementation of QTable's in-place editing
  uses QTableItems. If you want to use your own data structure, you
  have to reimplement createEditor() and setCellContentFromEditor() to
  realize in-place editing without QTableItems. See the documentation
  of these two functions for details about what you need to know for
  reimplementing them.

  QTable also supports all needed selection types like range
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

void QTable::init( int rows, int cols )
{
    mousePressed = FALSE;

    contents.setAutoDelete( TRUE );
    widgets.setAutoDelete( TRUE );

    // Enable clipper and set background mode
    enableClipper( TRUE );
    viewport()->setBackgroundMode( PaletteBase );
    setResizePolicy( Manual );
    selections.setAutoDelete( TRUE );

    // Create headers
    leftHeader = new QTableHeader( rows, this, this );
    leftHeader->setOrientation( Vertical );
    leftHeader->setTracking( TRUE );
    leftHeader->setMovingEnabled( TRUE );
    topHeader = new QTableHeader( cols, this, this );
    topHeader->setOrientation( Horizontal );
    topHeader->setTracking( TRUE );
    topHeader->setMovingEnabled( TRUE );
    setMargins( 30, fontMetrics().height() + 4, 0, 0 );

    // Initialize headers
    int i = 0;
    for ( i = 0; i < numCols(); ++i ) {
	topHeader->setLabel( i, QString::number( i + 1 ) );
	topHeader->resizeSection( i, 100 );
    }
    for ( i = 0; i < numRows(); ++i ) {
	leftHeader->setLabel( i, QString::number( i + 1 ) );
	leftHeader->resizeSection( i, 20 );
    }

    // Prepare for contents
    contents.resize( rows * cols );
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

    installEventFilter( this );

    // Initial size
    resize( 640, 480 );
}

/*!
  Destructor.
*/

QTable::~QTable()
{
    contents.clear();
    widgets.clear();
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
	rowlast = numRows() - 1;
    if ( collast == -1 )
	collast = numCols() - 1;

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
	
	    QTableItem *itm = item( r, c );
	    if ( itm &&
		 ( itm->colSpan() > 1 || itm->rowSpan() > 1 ) ) {
		bool goon = r == itm->row && c == itm->col ||
			r == rowfirst && c == itm->col ||
			r == itm->row && c == colfirst;
		if ( !goon )
		    continue;
		rowp = rowPos( itm->row );
		rowh = 0;
		int i;
		for ( i = 0; i < itm->rowSpan(); ++i )
		    rowh += rowHeight( i + itm->row );
		colp = columnPos( itm->col );
		colw = 0;
		for ( i = 0; i < itm->colSpan(); ++i )
		    colw += columnWidth( i + itm->col );
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
	QTableItem *i = item( curRow, curCol );
	focusEdited = ( i &&
			( i->editType() == QTableItem::Always ||
			  ( i->editType() == QTableItem::OnCurrent && curRow == i->row && curCol == i->col ) ) );
    }
    p->drawRect( focusRect.x(), focusRect.y(), focusRect.width() - 1, focusRect.height() - 1 );
    if ( !focusEdited ) {
	p->drawRect( focusRect.x() - 1, focusRect.y() - 1, focusRect.width() + 1, focusRect.height() + 1 );
    } else {
	if ( curRow == numRows() - 1 )
	    focusRect.setHeight( focusRect.height() - 1 );
	if ( curCol == numCols() - 1 )
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

    QTableItem *itm = item( row, col );
    if ( itm ) {
	p->save();
	itm->paint( p, colorGroup(), cr, selected );
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

QTableItem *QTable::item( int row, int col ) const
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return 0;
    return contents[ indexOf( row, col ) ];	// contents array lookup
}

/*!  Sets the contents for the cell \a row, \a col. If there did
  already exist an item for this cell, the old one is deleted.

  This function also repaints the cell.
*/

void QTable::setItem( int row, int col, QTableItem *item )
{
    if ( !item )
	return;

    int or = item->row;
    int oc = item->col;
    clearCell( row, col );

    contents.insert( indexOf( row, col ), item );
    item->row = row;
    item->col = col;
    updateCell( row, col );
    item->updateEditor( or, oc );
}

/*!  Clears the cell \p row, \a col. This means it removes the table
  item of it if one exists.
*/

void QTable::clearCell( int row, int col )
{
    contents.remove( indexOf( row, col ) );
}

/*!  Sets the text for the cell \a row, \a col to \a text. This is a
  convenience function of setItem()
*/

void QTable::setText( int row, int col, const QString &text )
{
    QTableItem *itm = item( row, col );
    if ( itm ) {
	itm->setText( text );
	updateCell( row, col );
    } else {
	QTableItem *i = new QTableItem( this, QTableItem::OnActivate, text, QPixmap() );
	setItem( row, col, i );
    }
}

/*!  Sets the pixmap for the cell \a row, \a col to \a pix. This is a
  convenience function of setItem()
*/

void QTable::setPixmap( int row, int col, const QPixmap &pix )
{
    QTableItem *itm = item( row, col );
    if ( itm ) {
	itm->setPixmap( pix );
	updateCell( row, col );
    } else {
	QTableItem *i = new QTableItem( this, QTableItem::OnActivate, QString::null, pix );
	setItem( row, col, i );
    }
}

/*!  Returns the text which is set for the cell \a row, \a col, or an
  emty string if the cell has no text.
*/

QString QTable::text( int row, int col ) const
{
    QTableItem *itm = item( row, col );
    if ( itm )
	return itm->text();
    return QString::null;
}

/*!  Returns the pixmap which is set for the cell \a row, \a col, or a
  null-pixmap if the cell has no pixmap.
*/

QPixmap QTable::pixmap( int row, int col ) const
{
    QTableItem *itm = item( row, col );
    if ( itm )
	return itm->pixmap();
    return QPixmap();
}

/*!  Sets the current cell to \a row, \a col.
*/

void QTable::setCurrentCell( int row, int col )
{
    QTableItem *itm = item( row, col );
    QTableItem *oldIitem = item( curRow, curCol );
    if ( itm && itm->rowSpan() > 1 && oldIitem == itm && itm->row != row ) {
	if ( row > curRow )
	    row = itm->row + itm->rowSpan();
	else if ( row < curRow )
	    row = QMAX( 0, itm->row - 1 );
    }
    if ( itm && itm->colSpan() > 1 && oldIitem == itm && itm->col != col ) {
	if ( col > curCol )
	    col = itm->col + itm->colSpan();
	else if ( col < curCol )
	    col = QMAX( 0, itm->col - 1 );
    }
    if ( curRow != row || curCol != col ) {
	itm = oldIitem;
	if ( itm && itm->editType() != QTableItem::Always )
	    endEdit( curRow, curCol, TRUE, FALSE );
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
	itm = item( curRow, curCol );
	
	if ( cellWidget( oldRow, oldCol ) &&
	     cellWidget( oldRow, oldCol )->hasFocus() )
	    viewport()->setFocus();
	
	if ( itm && itm->editType() == QTableItem::OnCurrent ) {
	    if ( beginEdit( curRow, curCol, FALSE ) ) {
		edMode = Editing;
		editRow = row;
		editCol = col;
	    }
	} else if ( itm && itm->editType() == QTableItem::Always ) {
	    if ( cellWidget( itm->row, itm->col ) )
		cellWidget( itm->row, itm->col )->setFocus();
	}
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
		 s->rightCol == numCols() - 1 )
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
		 s->bottomRow == numRows() - 1 )
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
    mousePressed = TRUE;
    if ( isEditing() )
 	endEdit( editRow, editCol, TRUE, edMode == Editing ? FALSE : TRUE );

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
	    currentSelection->init( this->curRow, this->curCol );
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
    if ( curRow != -1 && curCol != -1 ) {
	if ( beginEdit( curRow, curCol, FALSE ) ) {
	    edMode = Editing;
	    editRow = curRow;
	    editCol = curCol;
	}
    }
}

/*!  \reimp
*/

void QTable::contentsMouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
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
    if ( !mousePressed )
	return;
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal( pos );
    pos -= QPoint( leftHeader->width(), topHeader->height() );

    int curRow = this->curRow;
    int curCol = this->curCol;
    if ( pos.y() < 0 )
	curRow--;
    else if ( pos.y() > visibleWidth() )
	curRow++;
    if ( pos.x() < 0 )
	curCol--;
    else if ( pos.x() > visibleWidth() )
	curCol++;
	
    pos += QPoint( contentsX(), contentsY() );
    if ( curRow == this->curRow )
	curRow = rowAt( pos.y() );
    if ( curCol == this->curCol )
	curCol = columnAt( pos.x() );
    pos -= QPoint( contentsX(), contentsY() );

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
    mousePressed = FALSE;
    autoScrollTimer->stop();
}

/*!  \reimp
*/

bool QTable::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e )
	return QScrollView::eventFilter( o, e );

    QWidget *editorWidget = cellWidget( editRow, editCol );
    switch ( e->type() ) {
    case QEvent::KeyPress: {
	QTableItem *itm = item( curRow, curCol );
	
	if ( isEditing() && editorWidget && o == editorWidget ) {
	    itm = item( editRow, editCol );
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Escape ) {
		if ( !itm || itm->editType() == QTableItem::OnActivate )
		    endEdit( editRow, editCol, FALSE, edMode == Editing ? FALSE : TRUE );
		return TRUE;
	    }

	    if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
		if ( !itm || itm->editType() == QTableItem::OnActivate )
		    endEdit( editRow, editCol, TRUE, edMode == Editing ? FALSE : TRUE );
		activateNextCell();
		return TRUE;
	    }

	    if ( ( edMode == Replacing || itm && itm->editType() == QTableItem::OnCurrent ) &&
		 ( ke->key() == Key_Up || ke->key() == Key_Prior || ke->key() == Key_Home ||
		   ke->key() == Key_Down || ke->key() == Key_Next || ke->key() == Key_End ||
		   ke->key() == Key_Left || ke->key() == Key_Right ) ) {
		if ( !itm || itm->editType() == QTableItem::OnActivate )
		    endEdit( editRow, editCol, TRUE, edMode == Editing ? FALSE : TRUE );
		keyPressEvent( ke );
		return TRUE;
	    }
	} else {
	    QObjectList *l = viewport()->queryList( "QWidget" );
	    if ( l && l->find( o ) != -1 ) {
		QKeyEvent *ke = (QKeyEvent*)e;
		if ( ( ke->state() & ControlButton ) == ControlButton ||
		     ( ke->key() != Key_Left && ke->key() != Key_Right && ke->key() != Key_Up &&
		       ke->key() != Key_Down && ke->key() != Key_Prior && ke->key() != Key_Next &&
		       ke->key() != Key_Home && ke->key() != Key_End ) )
		    return FALSE;
		keyPressEvent( (QKeyEvent*)e );
		return TRUE;
	    }
	    delete l;
	}
	
	} break;
    case QEvent::FocusOut:
	if ( o == this || o == viewport() )
	    return TRUE;
	if ( isEditing() && editorWidget && o == editorWidget ) {
	    QTableItem *itm = item( editRow, editCol );
 	    if ( !itm || itm->editType() == QTableItem::OnActivate ) {
 		endEdit( editRow, editCol, TRUE, edMode == Editing ? FALSE : TRUE );
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
    if ( isEditing() && item( editRow, editCol ) &&
	 item( editRow, editCol )->editType() == QTableItem::OnActivate )
	return;

    int curRow = QTable::curRow;
    int curCol = QTable::curCol;
    int oldRow = curRow;
    int oldCol = curCol;

    bool navigationKey = FALSE;
    int r;
    switch ( e->key() ) {
    case Key_Left:
	curCol = QMAX( 0, curCol - 1 );
	navigationKey = TRUE;
    	break;
    case Key_Right:
	curCol = QMIN( numCols() - 1, curCol + 1 );
	navigationKey = TRUE;
	break;
    case Key_Up:
	curRow = QMAX( 0, curRow - 1 );
	navigationKey = TRUE;
	break;
    case Key_Down:
	curRow = QMIN( numRows() - 1, curRow + 1 );
	navigationKey = TRUE;
	break;
    case Key_Prior:
	r = QMAX( 0, rowAt( rowPos( curRow ) - visibleHeight() ) );
	if ( r < curRow )
	    curRow = r;
	navigationKey = TRUE;
	break;
    case Key_Next:
	r = QMIN( numRows() - 1, rowAt( rowPos( curRow ) + visibleHeight() ) );
	if ( r > curRow )
	    curRow = r;
	else
	    curRow = numRows() - 1;
	navigationKey = TRUE;
	break;
    case Key_Home:
	curRow = 0;
	navigationKey = TRUE;
	break;
    case Key_End:
	curRow = numRows() - 1;
	navigationKey = TRUE;
	break;
    default: // ... or start in-place editing
	if ( e->text()[ 0 ].isPrint() ) {
	    QTableItem *itm = item( curRow, curCol );
	    if ( !itm || itm->editType() == QTableItem::OnActivate ) {
		QWidget *w;
		if ( ( w = beginEdit( curRow, curCol, itm ? itm->isReplacable() : TRUE ) ) ) {
		    edMode = ( !itm || itm && itm->isReplacable() ? Replacing : Editing );
		    editRow = curRow;
		    editCol = curCol;
		    QApplication::sendEvent( w, e );
		}
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
    if ( isEditing() )
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
    QRect r( cellGeometry( numRows() - 1, numCols() - 1 ) );
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

    for ( int j = col; j < numCols(); ++j ) {
	for ( int i = 0; i < numRows(); ++i ) {
	    QWidget *w = cellWidget( i, j );
	    if ( !w )
		continue;
	    moveChild( w, columnPos( j ) - 1, rowPos( i ) - 1 );
	    w->resize( columnWidth( j ) + 1, rowHeight( i ) + 1 );
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

    for ( int j = row; j < numRows(); ++j ) {
	for ( int i = 0; i < numCols(); ++i ) {
	    QWidget *w = cellWidget( j, i );
	    if ( !w )
		continue;
	    moveChild( w, columnPos( i ) - 1, rowPos( j ) - 1 );
	    w->resize( columnWidth( i ) + 1, rowHeight( j ) + 1 );
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
    QTableItem *itm = item( row, col );
    if ( !itm || itm->rowSpan() == 1 && itm->colSpan() == 1 )
	return QRect( columnPos( col ), rowPos( row ),
		      columnWidth( col ), rowHeight( row ) );

    while ( row != itm->row )
	row--;
    while ( col != itm->col )
	col--;

    QRect rect( columnPos( col ), rowPos( row ),
		columnWidth( col ), rowHeight( row ) );
	
    for ( int r = 1; r < itm->rowSpan(); ++r )
	    rect.setHeight( rect.height() + rowHeight( r + row ) );
    for ( int c = 1; c < itm->colSpan(); ++c )
	rect.setWidth( rect.width() + columnWidth( c + col ) );

    return rect;
}

/*!  Returns the size of the table (same as the right/bottom edge of
  the last table cell.
*/

QSize QTable::tableSize() const
{
    return QSize( columnPos( numCols() - 1 ) + columnWidth( numCols() - 1 ),
		  rowPos( numRows() - 1 ) + rowHeight( numRows() - 1 ) );
}

/*!  Returns the number of rows of the table.
*/

int QTable::numRows() const
{
    return leftHeader->count();
}

/*!  Returns the number of columns of the table.
 */

int QTable::numCols() const
{
    return topHeader->count();
}

/*!  Sets the number of rows to \a r.
 */

void QTable::setNumRows( int r )
{
    if ( r > numRows() ) {
	clearSelection();
	while ( numRows() < r ) {
	    leftHeader->addLabel( QString::number( numRows() + 1 ) );
	    leftHeader->resizeSection( numRows() - 1, 20 );
	}
    } else {
	qWarning( "decreasing the number of rows is not implemented yet!" );
	return;
    }
    contents.resize( numRows() * numCols() );
    QRect r2( cellGeometry( numRows() - 1, numCols() - 1 ) );
    resizeContents( r2.right() + 1, r2.bottom() + 1 );
    updateGeometries();
    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
}

/*!  Sets the number of columns to \a c.
 */

void QTable::setNumCols( int c )
{
    if ( c > numCols() ) {
	clearSelection();
	while ( numCols() < c ) {
	    topHeader->addLabel( QString::number( numCols() + 1 ) );
	    topHeader->resizeSection( numCols() - 1, 100 );
	}
    } else {
	qWarning( "decreasing the number of columns is not implemented yet!" );
	return;
    }
    contents.resize( numRows() * numCols() );
    QRect r( cellGeometry( numRows() - 1, numCols() - 1 ) );
    resizeContents( r.right() + 1, r.bottom() + 1 );
    updateGeometries();
    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
}

/*!  This function returns a widget which should be used as editor for
  the cell \a row, \a col. If \a initFromCell is TRUE, the editor is
  used to edit the current content of the cell (so the editor widget
  should be initialized with that content). Otherwise the content of
  this cell will be replaced by a new content which the user will
  enter into the widget which this function should create.

  The default implementation looks if there exists a QTableItem for
  the cell. If this is the case and \a initFromCell is TRUE or
  QTableItem::isReplacable() of the item is FALSE, the item of
  that cell is asked to create the editor (using QTableItem::createEditor)).

  If this is not the case, a QLineEdit is created as editor.

  So if you want to create your own editor for certain cells,
  implement your own QTableItem and reimplement
  QTableItem::createEditor(). If you want to use a different editor
  than a QLineEdit as default editor, reimplement this function and
  use a code like

  \code
    QTableItem *i = item( row, col );
    if ( initFromCell || i && !i->isReplacable() )
        return QTable::createEditor( row, col, initFromCell );
    else
        return ...(create your editor)
    \endcode

    So normally you do not need to reimplement this function. But if
    you want e.g. work without QTableItems, you will reimplement this
    function to create the correct editor for the cells.

    The ownership of the editor widget is transferred to the caller.

  Returning 0 here means that the cell is not editable.

  \sa QTableItem::createEditor()
*/

QWidget *QTable::createEditor( int row, int col, bool initFromCell ) const
{
    QWidget *e = 0;

    // the current item in the cell should be edited if possible
    QTableItem *i = item( row, col );
    if ( initFromCell || i && !i->isReplacable() ) {
	if ( i ) {
	    e = i->createEditor();
	    if ( !e || i->editType() == QTableItem::Never )
		return 0;
	}
    }

    // no contents in the cell yet, so open the default editor
    if ( !e ) {
	e = new QLineEdit( viewport() );
	( (QLineEdit*)e )->setFrame( FALSE );
    }

    return e;
}

/*!  This function is called to start in-place editing of the cell \a
  row, \a col. If \a replace is TRUE the content of this cell will be
  replaced by the content of the editor later, else the current
  content of that cell (if existing) will be edited by the editor.

  This function calls createEditor() to get the editor which should be
  used for editing the cell and after that setCellWidget() to set this
  editor as the widget of that cell.

  \sa createEditor(), setCellWidget(), endEdit()
*/

QWidget *QTable::beginEdit( int row, int col, bool replace )
{
    QTableItem *itm = item( row, col );
    if ( itm && cellWidget( itm->row, itm->col ) )
	return 0;
    ensureCellVisible( curRow, curCol );
    QWidget *e = createEditor( row, col, !replace );
    if ( !e )
	return 0;
    setCellWidget( row, col, e );
    e->setFocus();
    updateCell( row, col );
    return e;
}

/*!  This function is called if in-place editing of the cell \a row,
  \a col has to be ended. If \a accept is TRUE the content of the
  editor of this cell has to be transferred to the cell. If \a replace
  is TRUE the current content of that cell should be replaced by the
  content of the editor (this means removing the current QTableItem of
  the cell and creating a new one for the cell), else (if possible)
  the content of the editor should just be set to the existing
  QTableItem of this cell.

  So, if the cell contents should be replaced or if no QTableItem
  exists for the cell yet, setCellContentFromEditor() is called, else
  QTableItem::setContentFromEditor() is called on the QTableItem
  of the cell.

  After that clearCellWidget() is called to get rid of the editor
  widget.

  \sa setCellContentFromEditor(), beginEdit()
*/

void QTable::endEdit( int row, int col, bool accept, bool replace )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor )
	return;

    if ( !accept ) {
	if ( row == editRow && col == editCol ) {
	    editRow = -1;
	    editCol = -1;
	    edMode = NotEditing;
	}
	clearCellWidget( row, col );
    	updateCell( row, col );
	viewport()->setFocus();
	updateCell( row, col );
	return;
    }

    QTableItem *i = item( row, col );
    if ( replace && i ) {
	clearCell( row, col );
	i = 0;
    }

    if ( !i )
	setCellContentFromEditor( row, col );
    else
	i->setContentFromEditor( editor );

    if ( row == editRow && col == editCol ) {
	editRow = -1;
	editCol = -1;
	edMode = NotEditing;
    }

    viewport()->setFocus();
    updateCell( row, col );

    clearCellWidget( row, col );
}

/*!  This function is called to set the contents of the cell \a row,
  \a col from the editor of this cell to this cell. If there existed
  already a QTableItem for this cell, this is removed first (see
  clearCell()).

  If you want to create e.g different QTableItems depending on the
  contents of the editor, you might reimplement this function. Also if
  you want to work without QTableItems, you will reimplement this
  function to set the data which the user entered to your
  datastructure.

  \sa QTableItem::setContentFromEditor()
*/

void QTable::setCellContentFromEditor( int row, int col )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor )
	return;
    clearCell( row, col );
    if ( editor->inherits( "QLineEdit" ) )
	setText( row, col, ( (QLineEdit*)editor )->text() );
}

/*!  Returns wheather the user is just editing a cell, which is not in
  permanent edit mode (see QTableItem::EditType).
*/

bool QTable::isEditing() const
{
    return edMode != NotEditing;
}

/*!  \internal
*/

int QTable::indexOf( int row, int col ) const
{
    return ( row * numCols() ) + col; // mapping from 2D table to 1D array
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
	for ( i = 0; i <= numCols(); ++i ) {
	    if ( !isColumnSelected( i ) )
		topHeader->setSectionState( i, QTableHeader::Normal );
	    else if ( isColumnSelected( i, TRUE ) )
		topHeader->setSectionState( i, QTableHeader::Selected );
	    else
		topHeader->setSectionState( i, QTableHeader::Bold );
	}
    }

    if ( updateVertical ) {
	for ( i = 0; i <= numRows(); ++i ) {
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
    for ( i = 0; i <= numCols(); ++i ) {
	if ( !isColumnSelected( i ) && i != curCol )
	    topHeader->setSectionState( i, QTableHeader::Normal );
	else if ( isColumnSelected( i, TRUE ) )
	    topHeader->setSectionState( i, QTableHeader::Selected );
	else
	    topHeader->setSectionState( i, QTableHeader::Bold );
    }

    for ( i = 0; i <= numRows(); ++i ) {
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
	    QTableItem *i = item( r, c );
	    if ( i && ( i->rowSpan() > 1 || i->colSpan() > 1 ) )
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
	if ( curRow < numRows() - 1 )
	    setCurrentCell( curRow + 1, curCol );
	else if ( curCol < numCols() - 1 )
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
	    row = numRows() - 1;
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
	    col = numCols() - 1;
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
    for ( i = 0; i < numRows(); ++i ) {
	QTableItem *itm = item( i, col );
	if ( itm )
	    filledRows++;
    }

    if ( !filledRows )
	return;

    SortableTableItem *items = new SortableTableItem[ filledRows ];
    int j = 0;
    for ( i = 0; i < numRows(); ++i ) {
	QTableItem *itm = item( i, col );
	if ( !itm )
	    continue;
	items[ j++ ].item = itm;
    }

    qsort( items, filledRows, sizeof( SortableTableItem ), cmpTableItems );

    contents.setAutoDelete( TRUE );
    for ( i = 0; i < numRows(); ++i ) {
	contents.remove( indexOf( i, col ) );
	if ( i < filledRows ) {
	    if ( ascending )
		contents.insert( indexOf( i, col ), items[ i ].item );
	    else
		contents.insert( indexOf( i, col ), items[ filledRows - i - 1 ].item );
	}
    }
    contents.setAutoDelete( FALSE );

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
    for ( int i = 0; i < numRows(); ++i ) {
	QTableItem *itm = item( i, col );
	if ( !itm )
	    continue;
	w = QMAX( w, itm->sizeHint().width() );
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
    for ( int i = 0; i < numCols(); ++i ) {
	QTableItem *itm = item( row, i );
	if ( !itm )
	    continue;
	h = QMAX( h, itm->sizeHint().height() );
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

/*!  Takes the item \a i out of the table. This functions doesn't
  delete it.
*/

void QTable::takeItem( QTableItem *i )
{
    QRect rect = cellGeometry( i->row, i->col );
    if ( !i )
	return;
    contents.setAutoDelete( FALSE );
    for ( int r = 0; r < i->rowSpan(); ++r ) {
	for ( int c = 0; c < i->colSpan(); ++c )
	    clearCell( i->row + r, i->col + c );
    }
    contents.setAutoDelete( TRUE );
    repaintContents( rect, FALSE );
    int or = i->row;
    int oc = i->col;
    i->row = i->col = -1;
    i->updateEditor( or, oc );
}

/*! Sets the widget \a e to the cell \a row, \a col and does all the
 placement and further stuff and takes care about correctly placing
 are resizing it when the cell geometry changes.
*/

void QTable::setCellWidget( int row, int col, QWidget *e )
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return;

    if ( (int)widgets.size() != numRows() * numCols() )
	widgets.resize( numRows() * numCols() );

    QWidget *w = cellWidget( row, col );
    if ( w && row == editRow && col == editCol )
 	endEdit( editRow, editCol, FALSE, edMode == Editing ? FALSE : TRUE );

    e->installEventFilter( this );
    clearCellWidget( row, col );
    widgets.insert( indexOf( row, col ), e );
    QRect cr = cellGeometry( row, col );
    e->resize( cr.size() + QSize( 1, 1 ) );
    moveChild( e, cr.x() - 1, cr.y() - 1 );
    e->show();
    viewport()->setFocus();
}

/*!  Returns the widget which has been set to the cell \a row, \a col
  of 0 if there is no widget.
*/

QWidget *QTable::cellWidget( int row, int col ) const
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return 0;

    if ( (int)widgets.size() != numRows() * numCols() )
	( (QTable*)this )->widgets.resize( numRows() * numCols() );

    return widgets[ indexOf( row, col ) ];
}

/*!  Removes the widget (if there is any) which is set for the cell \a
  row, \a col.
*/

void QTable::clearCellWidget( int row, int col )
{
    if ( row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1 )
	return;

    if ( (int)widgets.size() != numRows() * numCols() )
	widgets.resize( numRows() * numCols() );

    QWidget *w = cellWidget( row, col );
    if ( w )
	w->removeEventFilter( this );
    widgets.remove( indexOf( row, col ) );
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
    if ( hasCached )
	emit sectionSizeChanged( resizedSection );
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
