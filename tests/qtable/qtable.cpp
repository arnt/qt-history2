/****************************************************************************
** $Id: //depot/qt/main/examples/table/table.cpp#3 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qtable.h"
#include <qpainter.h>
#include <qkeycode.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <qwmatrix.h>
#include <qtimer.h>

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
    p->drawText( x, 0, w - x, h, alignment(), txt );
}

/*!  Returns the editor which should be used for editing that
  cell. The default implementation returns 0, which means that the
  deafault editor (QLineEdit) should be used. If you reimplement that
  to use a custom editor widget always create a new widget here as
  parent of table()->viewport(), as the ownership of it is trasferred
  to the caller.
*/

QWidget *QTableItem::editor() const
{
    return 0;
}

/*!  This function is called to set the cell contents from the editor
  \a w.
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
  cells. Also you have of course to repaint the cells which changed
  yourself using updateCell().

  The in-place editing is also made in an abstract way so that it is
  possible to have custom edit widgets for certain cells or types or
  cells. When in-plcae editing is started beginEdit() is called. This
  creates the editor widget for the required cell, places and shows
  it. To create an editor widget this function calls editor() for the
  required cell. Now there exist two different ways to edit a
  cell. Either offer an edit widget to enter a contents which should
  replace the current cell's contents or offer an editor to edit the
  current cell's contents. The edit-mode can be retrieved by the
  editMode() function during editing. Now depending on that mode the
  editor() function either looks if there is a contents to edit (a
  QTableItem exists for that cell) and asks that item for an editor
  (see QTableItem::editor()). If this is not possible, or the item
  returns 0 as editor widget or we are in replace mode, the editor()
  function creates an editor itself. That editor is a QLineEdit and
  defaultValidator() is used as validator for the QLineEdit. Now when
  the user finishes editing endEdit() is called. If the contents of
  the editor should be accepted, and we are in edit mode and the
  edited cell had already a QTableItem,
  QTableItem::setContentFromEditor() is called to set the
  contents. Else setContentFromEditor() is called to create a content
  for the cell or replace the old cell's contents by the editor's one.

  So to customize the in-place editing you can make a custom
  QTableItem and reimplement QTableItem::editor(), also maybe
  reimplement editor(), QTableItem::setContentFromEditor() and
  setContentFromEditor().


*/

/*!  Constructs a table with a range of \a numRows * \a numCols cells.
*/

QTable::QTable( int numRows, int numCols, QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase | WNorthWestGravity ),
      currentSelection( 0 )
{
    setResizePolicy( Manual );
    selections.setAutoDelete( TRUE );

    // Create headers
    leftHeader = new QTableHeader( numRows, this, this );
    leftHeader->setOrientation( Vertical );
    leftHeader->setTracking( TRUE );
    leftHeader->setMovingEnabled( FALSE );
    topHeader = new QTableHeader( numCols, this, this );
    topHeader->setOrientation( Horizontal );
    topHeader->setTracking( TRUE );
    topHeader->setMovingEnabled( FALSE );
    setMargins( 30, fontMetrics().height() + 4, 0, 0 );

    // Initialize headers
    int i = 0;
    for ( i = 0; i < cols(); ++i ) {
	topHeader->setLabel( i, QString::number( i + 1 ) );
	topHeader->resizeSection( i, 100 );
    }
    for ( i = 0; i < rows(); ++i ) {
	leftHeader->setLabel( i, QString::number( i + 1 ) );
	leftHeader->resizeSection( i, 20 );
    }

    // Enable clipper and set background mode
    enableClipper( TRUE );
    viewport()->setBackgroundMode( PaletteBase );

    // Prepare for contents
    contents.resize( numRows * numCols );
    contents.setAutoDelete( TRUE );

    // Connect header, table and scrollbars
    connect( horizontalScrollBar(), SIGNAL( valueChanged( int ) ),
	     topHeader, SLOT( setOffset( int ) ) );
    connect( verticalScrollBar(), SIGNAL( valueChanged( int ) ),
	     leftHeader, SLOT( setOffset( int ) ) );
    connect( topHeader, SIGNAL( sizeChange( int, int, int ) ),
	     this, SLOT( columnWidthChanged( int, int, int ) ) );
    connect( leftHeader, SIGNAL( sizeChange( int, int, int ) ),
	     this, SLOT( rowHeightChanged( int, int, int ) ) );

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
}

/*!  Draws the contents of the table on the painter \a p. This
function is optimized to only draw the relevant cells which are inside
the clipping rectangle \a cx, \a cy, \a cw, \a ch.

This function also draws the indication of the current selection which
is the same as the current cell if there is no selection available.
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
	collast = cols() - 1;

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

	    // Translate painter and draw the cell
	    p->saveWorldMatrix();
	    p->translate( colp, rowp );
	    paintCell( p, r, c, QRect( colp, rowp, colw, rowh ), isSelected( r, c ) );
	    p->restoreWorldMatrix();
	}
    }

    // draw indication of current cell
    QRect focusRect = cellGeometry( curRow, curCol );
    p->setPen( QPen( black, 1 ) );
    p->setBrush( NoBrush );
    p->drawRect( focusRect.x() - 2, focusRect.y() - 2, focusRect.width() + 3, focusRect.height() + 3 );
    p->drawRect( focusRect.x(), focusRect.y(), focusRect.width() - 1, focusRect.height() - 1 );

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
QTableItem::paint(). So only implement this function for drawing items
if you e.g. directly retrieve the data which should be drawn from a
database and should not be stored in an data structure in the table
(which would be a custom table item).
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

    // Draw our lines
    QPen pen( p->pen() );
    p->setPen( colorGroup().mid().light() );
    p->drawLine( x2, 0, x2, y2 );
    p->drawLine( 0, y2, x2, y2 );
    p->setPen( pen );

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
    if ( row < 0 || col < 0 || row > rows() - 1 || col > cols() - 1 )
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

    if ( contents[ indexOf( row, col ) ] )
	delete contents[ indexOf( row, col ) ];

    contents.insert( indexOf( row, col ), item ); // contents lookup and assign
    updateCell( row, col ); // repaint
}

void QTable::clearCell( int row, int col )
{
    contents.remove( indexOf( row, col ) );
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
    if ( curRow != row || curCol != col ) {
	int oldRow = curRow;
	int oldCol = curCol;
	curRow = row;
	curCol = col;
	updateCell( oldRow, oldCol );
	updateCell( curRow, curCol );
	ensureCellVisible( curRow, curCol );
	emit currentChanged( row, col );
	if ( !isColSelected( oldCol ) && !isRowSelected( oldRow ) ) {
	    topHeader->setSectionState( oldCol, QTableHeader::Normal );
	    leftHeader->setSectionState( oldRow, QTableHeader::Normal );
	}
	topHeader->setSectionState( curCol, isColSelected( curCol, TRUE ) ? QTableHeader::Selected : QTableHeader::Bold );
	leftHeader->setSectionState( curRow, isRowSelected( curRow, TRUE ) ? QTableHeader::Selected : QTableHeader::Bold );
    }
}

void QTable::ensureCellVisible( int row, int col )
{
    int cw = columnWidth( col );
    int rh = rowHeight( row );
    ensureVisible( columnPos( col ) + cw / 2, rowPos( row ) + rh / 2, cw / 2, rh / 2 );
}

bool QTable::isSelected( int row, int col )
{
    for ( SelectionRange *s = selections.first(); s; s = selections.next() ) {
	if ( s->active &&
	     row >= s->topRow &&
	     row <= s->bottomRow &&
	     col >= s->leftCol &&
	     col <= s->rightCol )
	    return TRUE;
	if ( row == currentRow() && col == currentCol() )
	    return TRUE;
    }
    return FALSE;
}

bool QTable::isRowSelected( int row, bool full )
{
    if ( !full ) {
	for ( SelectionRange *s = selections.first(); s; s = selections.next() ) {
	    if ( s->active &&
		 row >= s->topRow &&
		 row <= s->bottomRow )
	    return TRUE;
	if ( row == currentRow() )
	    return TRUE;
	}
    } else {
	for ( SelectionRange *s = selections.first(); s; s = selections.next() ) {
	    if ( s->active &&
		 row >= s->topRow &&
		 row <= s->bottomRow &&
		 s->leftCol == 0 && 
		 s->rightCol == cols() - 1 )
		return TRUE;
	}
    }
    return FALSE;
}

bool QTable::isColSelected( int col, bool full )
{
    if ( !full ) {
	for ( SelectionRange *s = selections.first(); s; s = selections.next() ) {
	    if ( s->active &&
		 col >= s->leftCol &&
		 col <= s->rightCol )
	    return TRUE;
	if ( col == currentCol() )
	    return TRUE;
	}
    } else {
	for ( SelectionRange *s = selections.first(); s; s = selections.next() ) {
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

/*!  \reimp
*/

void QTable::contentsMousePressEvent( QMouseEvent* e )
{
    if ( editorWidget && isEditing() )
 	endEdit( editRow, editCol, TRUE, editorWidget );

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
	if ( isEditing() && editorWidget && o == editorWidget ) {
	    QKeyEvent *ke = (QKeyEvent*)e;

	    if ( ke->key() == Key_Escape ) {
		endEdit( editRow, editCol, FALSE, editorWidget );
		return TRUE;
	    }

	    if ( ke->key() == Key_Return || ke->key() == Key_Enter ) {
		endEdit( editRow, editCol, TRUE, editorWidget );
		activateNextCell();
		return TRUE;
	    }

	    if ( edMode == Replacing &&
		 ( ke->key() == Key_Up || ke->key() == Key_Prior || ke->key() == Key_Home ||
		   ke->key() == Key_Down || ke->key() == Key_Next || ke->key() == Key_End ||
		   ke->key() == Key_Left || ke->key() == Key_Right ) ) {
		endEdit( editRow, editCol, TRUE, editorWidget );
		keyPressEvent( ke );
		return TRUE;
	    }
	}
	} break;
    case QEvent::FocusOut:
	if ( o == this || o == viewport() )
	    return TRUE;
	if ( isEditing() && editorWidget && o == editorWidget )
	    endEdit( editRow, editCol, TRUE, editorWidget );
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
	curCol = QMIN( cols() - 1, curCol + 1 );
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
	    beginEdit( curRow, curCol, TRUE );
	    QApplication::sendEvent( editorWidget, e );
	}
    }

    setCurrentCell( curRow, curCol );
    if ( navigationKey ) {
	if ( ( e->state() & ShiftButton ) == ShiftButton ) {
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
	}	
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
    QRect r( cellGeometry( rows() - 1, cols() - 1 ) );
    resizeContents( r.right() + 1, r.bottom() + 1 );
    updateGeometries();
}

/*!  Repaints the cell \a row, \a col.
*/

void QTable::updateCell( int row, int col )
{
    QRect cg = cellGeometry( row, col );
    QRect r( cg.x() - 2, cg.y() - 2, cg.width() + 4, cg.height() + 4 );
    repaintContents( r, FALSE );
}

/*!  This function is called if the width of the column \a col has
  been changed. The second and third parameters should be ignored and
  cellWidth() should be used to get the width of the column.
*/

void QTable::columnWidthChanged( int col, int, int )
{
    updateContents( columnPos( col ), 0, contentsWidth(), contentsHeight() );
    QSize s( tableSize() );
    int w = contentsWidth();
    resizeContents( s.width(), s.height() );
    if ( contentsWidth() < w )
	repaintContents( s.width(), 0, w - s.width() + 1, contentsHeight(), TRUE );
    if ( editorWidget ) {
	moveChild( editorWidget, columnPos( curCol ) + 1, rowPos( curRow ) + 1 );
	editorWidget->resize( columnWidth( curCol ) - 2, rowHeight( curRow ) - 2 );
    }
    updateGeometries();
}

/*!  This function is called if the height of the row \a row has
  been changed. The second and third parameters should be ignored and
  rowHeight() should be used to get the height of the row.
*/

void QTable::rowHeightChanged( int row, int, int )
{
    updateContents( 0, rowPos( row ), contentsWidth(), contentsHeight() );
    QSize s( tableSize() );
    int h = contentsHeight();
    resizeContents( s.width(), s.height() );
    if ( contentsHeight() < h )
	repaintContents( 0, contentsHeight(), contentsWidth(), h - s.height() + 1, TRUE );
    if ( editorWidget ) {
	moveChild( editorWidget, columnPos( curCol ) + 1, rowPos( curRow ) + 1 );
	editorWidget->resize( columnWidth( curCol ) - 2, rowHeight( curRow ) - 2 );
    }
    updateGeometries();
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
    return QRect( columnPos( col ), rowPos( row ),
		  columnWidth( col ), rowHeight( row ) );
}

/*!  Returns the size of the table (same as the right/bottom edge of
  the last table cell.
*/

QSize QTable::tableSize() const
{
    return QSize( columnPos( cols() - 1 ) + columnWidth( cols() - 1 ),
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

int QTable::cols() const
{
    return topHeader->count();
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
  the cell and uses the editor of that (see QTableItem::editor()), if
  \a initFromCell is TRUE, else a default editor which is a QLineEdit
  is created.

  The ownership of the editor widget is transferred to the caller.

  You can reimplement that if you want to create an own editor
  depending on the cell.
*/

QWidget *QTable::editor( int row, int col, bool initFromCell ) const
{
    QWidget *e = 0;

    if ( initFromCell ) {
	QTableItem *i = cellContent( row, col );
	if ( i )
	    e = i->editor();
    }

    if ( !e ) {
	e = new QLineEdit( viewport() );
	( (QLineEdit*)e )->setFrame( FALSE );
	( (QLineEdit*)e )->setValidator( defaultValidator() );
	if ( initFromCell )
	    ( (QLineEdit*)e )->setText( cellText( row, col ) );
    }

    return e;
}

/*!  This function is called to start in-place editing of the cell \a
  row, \a col. If \a replace is TRUE the content of this cell will be
  replaced by the content of the editor, else the current content of
  that cell (if existing) will be edited by the editor.

  This function is responsible for creating and placing the editor.
*/

void QTable::beginEdit( int row, int col, bool replace )
{
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
}

/*!  This function is called if in-place editing of the cell \a row,
  \a col has to be ended. If \a accept is TRUE the content of the \a
  editor has to be transferred to the cell. If editMode() is \c
  Replacing the current content of that cell should be replaced by the
  content of the editor (removing the QTableItem of this cell if one
  exists), else (if possible) the content of the editor should just be
  set to the QTableItem of this cell.

  This function is also responsible for deleting the editor widget.
*/

void QTable::endEdit( int row, int col, bool accept, QWidget *editor )
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
	return;
    }

    QTableItem *i = cellContent( row, col );
    if ( edMode == Replacing && i ) {
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
    updateCell( row, col );
    edMode = NotEditing;
}

/*!  This function is called to set the contents of the cell \a row,
  \a col from the \a editor. If there existed already a contents, this
  is removed first (see clearCell())
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

/*!
  \a internal
*/

int QTable::indexOf( int row, int col ) const
{
    return ( row * cols() ) + col; // mapping from 2D table to 1D array
}

void QTable::repaintSelections( SelectionRange *oldSelection, SelectionRange *newSelection, 
				bool updateVertical, bool updateHorizontal )
{
    QRect old = rangeGeometry( oldSelection->topRow,
			       oldSelection->leftCol,
			       oldSelection->bottomRow,
			       oldSelection->rightCol );

    QRect cur = rangeGeometry( newSelection->topRow,
			       newSelection->leftCol,
			       newSelection->bottomRow,
			       newSelection->rightCol );

    QRect r( old.unite( cur ) );
    repaintContents( r, FALSE );

    int i;

    if ( updateHorizontal ) {
	for ( i = 0; i <= cols(); ++i ) {
	    if ( !isColSelected( i ) )
		topHeader->setSectionState( i, QTableHeader::Normal );
	    else if ( isColSelected( i, TRUE ) )
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

void QTable::clearSelection()
{
    bool needRepaint = !selections.isEmpty();
    
    QRect r;
    for ( SelectionRange *s = selections.first(); s; s = selections.next() ) {
	r = r.unite( rangeGeometry( s->topRow,
				    s->leftCol,
				    s->bottomRow,
				    s->rightCol ) );
    }

    currentSelection = 0;
    selections.clear();
    if ( needRepaint )
	repaintContents( r, FALSE );
    
    int i;
    for ( i = 0; i <= cols(); ++i ) {
	if ( !isColSelected( i ) )
	    topHeader->setSectionState( i, QTableHeader::Normal );
	else if ( isColSelected( i, TRUE ) )
	    topHeader->setSectionState( i, QTableHeader::Selected );
	else
	    topHeader->setSectionState( i, QTableHeader::Bold );
    }

    for ( i = 0; i <= rows(); ++i ) {
	if ( !isRowSelected( i ) )
	    leftHeader->setSectionState( i, QTableHeader::Normal );
	else if ( isRowSelected( i, TRUE ) )
	    leftHeader->setSectionState( i, QTableHeader::Selected );
	else
	    leftHeader->setSectionState( i, QTableHeader::Bold );
    }
}

QRect QTable::rangeGeometry( int topRow, int leftCol, int bottomRow, int rightCol )
{
    int y = rowPos( topRow );
    int x = columnPos( leftCol );
    int y1 = rowPos( bottomRow ) + rowHeight( bottomRow );
    int x1 = columnPos( rightCol ) + columnWidth( rightCol );
    QRect r;
    r.setCoords( x, y, x1, y1 );
    return r;
}

void QTable::activateNextCell()
{
    if ( !currentSelection || !currentSelection->active ) {
	if ( curRow < rows() - 1 )
	    setCurrentCell( curRow + 1, curCol );
	else if ( curCol < cols() - 1 )
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

void QTable::fixRow( int &row, int y )
{
    if ( row == -1 ) {
	if ( y < 0 )
	    row = 0;
	else
	    row = rows() - 1;
    }
}

void QTable::fixCol( int &col, int x )
{
    if ( col == -1 ) {
	if ( x < 0 )
	    col = 0;
	else
	    col = cols() - 1;
    }
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
    : QHeader( i, parent, name ), table( t )
{
    states.resize( i );
    states.fill( Normal, -1 );
    mousePressed = FALSE;
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
}

void QTableHeader::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed || cursor().shape() != ArrowCursor ) {
	QHeader::mouseMoveEvent( e );
	return;
    }
    
    int p = real_pos( e->pos(), orientation() );
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
    endPos = p;
    if ( startPos != -1 )
	updateSelections();
    else
	QHeader::mouseMoveEvent( e );
}

void QTableHeader::mouseReleaseEvent( QMouseEvent *e )
{
    mousePressed = FALSE;
    QHeader::mouseReleaseEvent( e );
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
	table->currentSelection->expandTo( b, count() - 1 );
    else
	table->currentSelection->expandTo( count() - 1, b );
    table->repaintSelections( &oldSelection, table->currentSelection, 
			      orientation() == Horizontal, orientation() == Vertical );
}

void QTableHeader::saveStates()
{
    oldStates.resize( count() );
    for ( int i = 0; i < count(); ++i )
	oldStates[ i ] = sectionState( i );
}
