/****************************************************************************
** $Id: //depot/qt/main/examples/table/table.cpp#3 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "table.h"
#include <qpainter.h>
#include <qkeycode.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <qwmatrix.h>

// Qt logo

static const char *qtlogo_xpm[] = {
    "45 36 13 1",
    "  c #000000",
    ". c #999999",
    "X c #333366",
    "o c #6666CC",
    "O c #333333",
    "@ c #666699",
    "# c #000066",
    "$ c #666666",
    "% c #3333CC",
    "& c #000033",
    "* c #9999CC",
    "= c #333399",
    "+ c None",
    "+++++++++++++++++++++++++++++++++++++++++++++",
    "+++++++++++++++.$OOO$.+++++++++++++++++++++++",
    "+++++++++++++$         O.++++++++++++++++++++",
    "+++++++++++.O            $+++++++++++++++++++",
    "++++++++++.    $.++.$     O++++++++++++++++++",
    "+++++++++.   O.+++++++$    O+++++++++++++++++",
    "+++++++++O   ++++++++++$    $++++++++++++++++",
    "++++++++$   .+++++++++++O    .+++++++++++++++",
    "+++++++.   O+++++++++++++    O++++++.++++++++",
    "+++++++$   .+++++++++++++$    .+++.O ++++++++",
    "+++++++    +++++++++++++++    O+++.  ++++++++",
    "++++++.  &Xoooo*++++++++++$    +++.  ++++++++",
    "++++++@=%%%%%%%%%%*+++++++.    .++.  ++++++++",
    "+++**oooooo**++*o%%%%o+++++    $++O  ++++++++",
    "+*****$OOX@oooo*++*%%%%%*++O   $+.   OOO$++++",
    "+.++....$O$+*ooooo*+*o%%%%%O   O+$   $$O.++++",
    "*+++++$$....+++*oooo**+*o%%#   O++O  ++++++**",
    "++++++O  $.....++**oooo**+*X   &o*O  ++++*ooo",
    "++++++$   O++.....++**oooo*X   &%%&  ..*o%%*+",
    "++++++$    ++++.....+++**ooO   $*o&  @oo*++++",
    "++++++.    .++++++.....+++*O   Xo*O  .+++++++",
    "+++++++    O+++++++++......    .++O  ++++++++",
    "+++++++O    +++.$$$.++++++.   O+++O  ++++++++",
    "+++++++.    $$OO    O.++++O   .+++O  ++++++++",
    "++++++++O    .+++.O   $++.   O++++O  ++++++++",
    "++++++++.    O+++++O   $+O   +++++O  ++++++++",
    "+++++++++.    O+++++O   O   .+++++O  .+++++++",
    "++++++++++$     .++++O     .++++.+$  O+.$.+++",
    "+++++++++++.      O$$O    .+++++...      ++++",
    "+++++++++++++$            O+++++$$+.O O$.++++",
    "+++++++++++++++$OO  O$.O   O.++. .+++++++++++",
    "+++++++++++++++++++++++.     OO  .+++++++++++",
    "++++++++++++++++++++++++.       O++++++++++++",
    "+++++++++++++++++++++++++.      .++++++++++++",
    "++++++++++++++++++++++++++.O  O.+++++++++++++",
    "+++++++++++++++++++++++++++++++++++++++++++++"
};

Table::Table( int numRows, int numCols, QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase | WNorthWestGravity )
{
    setResizePolicy( Manual );

    // create headers
    leftHeader = new QHeader( numRows, this );
    leftHeader->setOrientation( Vertical );
    leftHeader->setTracking( TRUE );
    leftHeader->setMovingEnabled( FALSE );
    topHeader = new QHeader( numCols, this );
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

    // preperations for contents
    contents.resize( numRows * numCols );
    contents.setAutoDelete( TRUE );
    QWMatrix wm;
    wm.scale( 0.5, 0.5 );
    QPixmap pix( qtlogo_xpm );
    pix = pix.xForm( wm );
    setCellPixmap( 3, 3, pix );
    setCellText( 3, 3, "A Pixmap" );

    // connect header, table and scrollbars
    connect( horizontalScrollBar(), SIGNAL( valueChanged( int ) ),
	     topHeader, SLOT( setOffset( int ) ) );
    connect( verticalScrollBar(), SIGNAL( valueChanged( int ) ),
	     leftHeader, SLOT( setOffset( int ) ) );
    connect( topHeader, SIGNAL( sizeChange( int, int, int ) ),
	     this, SLOT( columnWidthChanged( int, int, int ) ) );
    connect( leftHeader, SIGNAL( sizeChange( int, int, int ) ),
	     this, SLOT( rowHeightChanged( int, int, int ) ) );

    // init variables
    curRow = curCol = 0;
    editor = 0;

    // initial size
    resize( 640, 480 );
}

Table::~Table()
{
}

/****************************************************************************
  Two drawing functions, 1 which finds out which cells to draw
  and one which actually draws a cell.
  Also one drawing function to darw empty areas.
*****************************************************************************/

void Table::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
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
    
    // go through the rows
    for ( int r = rowfirst; r <= rowlast; ++r ) {
	// get row position and height
	int rowp = rowPos( r );
	int rowh = rowHeight( r );
	
	// go through the columns in the row r
	// if we know from where to where use this limits (colstart, colend), else go through all of them
	for ( int c = colfirst; c <= collast; ++c ) {
	    // get position and width of column c
	    int colp, colw;
	    colp = columnPos( c );
	    colw = columnWidth( c );

	    //  translate painter ad draw the cell
	    p->saveWorldMatrix();
	    p->translate( colp, rowp );
	    paintCell( p, r, c, QRect( colp, rowp, colw, rowh ) );
	    p->restoreWorldMatrix();
	}
    }

    // paint empty rects
    paintEmptyArea( p, cx, cy, cw, ch );
}

void Table::paintCell( QPainter* p, int row, int col, const QRect &cr )
{
    int w = cr.width();
    int h = cr.height();
    int x2 = w - 1;
    int y2 = h - 1;

    // draw cell background
    p->fillRect( 0, 0, w, h, colorGroup().brush( QColorGroup::Base ) );

    // draw our lines
    QPen pen( p->pen() );
    p->setPen( gray );
    p->drawLine( x2, 0, x2, y2 );
    p->drawLine( 0, y2, x2, y2 );
    p->setPen( pen );

    // if we are the focus cell, draw indication
    if ( row == curRow &&col == curCol ) {
	if ( hasFocus() || viewport()->hasFocus() )
	    p->drawRect( 0, 0, x2, y2 );
    }

    int x = 0;
    QPixmap pix( cellPixmap( row, col ) );
    if ( !pix.isNull() ) {
	p->drawPixmap( 0, ( cr.height() - pix.height() ) / 2, pix );
	x = pix.width() + 2;
    }

    // find out if contents is a number or a string
    bool num;
    bool ok1 = FALSE, ok2 = FALSE;
    QString s( cellText( row, col ) );
    s.toInt( &ok1 );
    s.toDouble( &ok2 );
    num = ok1 || ok2;

    // draw conetnst
    p->drawText( x, 0, w - x, h, ( num ? AlignRight : AlignLeft ) | AlignVCenter, s );
}

void Table::paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch )
{
    // recion of the rect we should draw
    QRegion reg( QRect( cx, cy, cw, ch ) );
    // subtract the table from it
    reg = reg.subtract( QRect( QPoint( 0, 0 ), tableSize() ) );
    p->save();
    // set clip region
    p->setClipRegion( reg );
    // and fill background
    p->fillRect( cx, cy, cw, ch, colorGroup().brush( QColorGroup::Base ) );
    p->restore();
}


/****************************************************************************
  Functions to access and set the cell contents
  plus helper functions.
*****************************************************************************/

TableItem *Table::cellContent( int row, int col ) const
{
    return contents[ indexOf( row, col ) ];	// contents array lookup
}

void Table::setCellContent( int row, int col, TableItem *item )
{
    if ( contents[ indexOf( row, col ) ] )
	delete contents[ indexOf( row, col ) ];

    contents.insert( indexOf( row, col ), item ); // contents lookup and assign
    updateCell( row, col ); // repaint
}

int Table::indexOf( int row, int col ) const
{
    return ( row * cols() ) + col; // mapping from 2D table to 1D array
}

void Table::setCellText( int row, int col, const QString &text )
{
    TableItem *item = cellContent( row, col );
    if ( item ) {
	item->setText( text );
	updateCell( row, col );
    } else {
	TableItem *i = new TableItem( text, QPixmap() );
	setCellContent( row, col, i );
    }
}

void Table::setCellPixmap( int row, int col, const QPixmap &pix )
{
    TableItem *item = cellContent( row, col );
    if ( item ) {
	item->setPixmap( pix );
	updateCell( row, col );
    } else {
	TableItem *i = new TableItem( QString::null, pix );
	setCellContent( row, col, i );
    }
}

QString Table::cellText( int row, int col ) const
{
    TableItem *item = cellContent( row, col );
    if ( item )
	return item->text();
    return QString::null;
}

QPixmap Table::cellPixmap( int row, int col ) const
{
    TableItem *item = cellContent( row, col );
    if ( item )
	return item->pixmap();
    return QPixmap();
}



/****************************************************************************
  Event handling to react on mouse, key and focus events.
*****************************************************************************/

void Table::contentsMousePressEvent( QMouseEvent* e )
{
    // get rid of editor
    if ( editor )
	editorOk();

    // remember old focus cell
    int oldRow = curRow;
    int oldCol = curCol;

    // get new focus cell
    curRow = rowAt( e->pos().y() );
    curCol = columnAt( e->pos().x() );
    if ( curRow == -1 )
	curRow = oldRow;
    if ( curCol == -1 )
	curCol = oldCol;

    // of we have a new focus cell, repaint
    if ( curRow != oldRow || curCol != oldCol ) {
	updateCell( oldRow, oldCol );
	updateCell( curRow, curCol );
	int cw = columnWidth( curCol );
	int rh = rowHeight( curRow );
	ensureVisible( columnPos( curCol ) + cw / 2, rowPos( curRow ) + rh / 2, cw / 2, rh / 2 );
    }
}

void Table::contentsMouseMoveEvent( QMouseEvent *e )
{
    // do the same as in mouse press
    contentsMousePressEvent( e );
}

void Table::keyPressEvent( QKeyEvent* e )
{
    // if a cell is just editing, do some special stuff
    if ( editor ) {
	if ( e->key() == Key_Escape )
	    ediorCancel();
	else if ( e->key() == Key_Return || e->key() == Key_Enter )
	    editorOk();
	return;
    }

    int oldRow = curRow;
    int oldCol = curCol;

    // navigate in the header...
    switch ( e->key() ) {
    case Key_Left:
	curCol = QMAX( 0, curCol - 1 );
	break;
    case Key_Right:
	curCol = QMIN( cols() - 1, curCol + 1 );
	break;
    case Key_Up:
	curRow = QMAX( 0, curRow - 1 );
	break;
    case Key_Down:
	curRow = QMIN( rows() - 1, curRow + 1 );
	break;
    case Key_Prior:
    case Key_Next:
    case Key_Home:
    case Key_End:
	break;
    default: // ... or start in-place editing
	if ( e->text()[ 0 ].isPrint() ) {
	    editor = new QLineEdit( cellText( curRow, curCol ) + e->text(), viewport() );
	    int d = cellPixmap( curRow, curCol ).width();
	    moveChild( editor, columnPos( curCol ) + 1 + d, rowPos( curRow ) + 1 );
	    editor->resize( columnWidth( curCol ) - 2 - d, rowHeight( curRow ) - 2 );
	    editor->setFrame( FALSE );
	    editor->show();
	    editor->setFocus();
	    connect( editor, SIGNAL( returnPressed() ),
		     this, SLOT( editorOk() ) );
	}	
    }

    // if focus cell changes, repaint
    if ( curRow != oldRow || curCol != oldCol ) {
	updateCell( oldRow, oldCol );
	updateCell( curRow, curCol );
	int cw = columnWidth( curCol );
	int rh = rowHeight( curRow );
	ensureVisible( columnPos( curCol ) + cw / 2, rowPos( curRow ) + rh / 2, cw / 2, rh / 2 );
    }
}

void Table::focusInEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );
}


void Table::focusOutEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );
}

bool Table::focusNextPrevChild( bool next )
{
    if ( editor )
	return TRUE;
    return QScrollView::focusNextPrevChild( next );
}

void Table::resizeEvent( QResizeEvent *e )
{
    QScrollView::resizeEvent( e );
    updateGeometries();
}

void Table::showEvent( QShowEvent *e )
{
    QScrollView::showEvent( e );
    QRect r( cellGeometry( rows() - 1, cols() - 1 ) );
    resizeContents( r.right() + 1, r.bottom() + 1 );
    updateGeometries();
}



/****************************************************************************
  Functions to update cells, update geometries, etc.
*****************************************************************************/

void Table::updateCell( int row, int col )
{
    QRect r( cellGeometry( row, col ) );
    updateContents( r );
}

void Table::columnWidthChanged( int col, int, int )
{
    updateContents( columnPos( col ), 0, contentsWidth(), contentsHeight() );
    QSize s( tableSize() );
    int w = contentsWidth();
    resizeContents( s.width(), s.height() );
    if ( contentsWidth() < w )
	repaintContents( s.width(), 0, w - s.width() + 1, contentsHeight(), TRUE );
    if ( editor ) {
	moveChild( editor, columnPos( curCol ) + 1, rowPos( curRow ) + 1 );
	editor->resize( columnWidth( curCol ) - 2, rowHeight( curRow ) - 2 );
    }
    updateGeometries();
}

void Table::rowHeightChanged( int row, int, int )
{
    updateContents( 0, rowPos( row ), contentsWidth(), contentsHeight() );
    QSize s( tableSize() );
    int h = contentsHeight();
    resizeContents( s.width(), s.height() );
    if ( contentsHeight() < h )
	repaintContents( 0, contentsHeight(), contentsWidth(), h - s.height() + 1, TRUE );
    if ( editor ) {
	moveChild( editor, columnPos( curCol ) + 1, rowPos( curRow ) + 1 );
	editor->resize( columnWidth( curCol ) - 2, rowHeight( curRow ) - 2 );
    }
    updateGeometries();
}

void Table::updateGeometries()
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



/****************************************************************************
  Following functions return the position/sizes of table cells. The
  data is stored in the two headers anyway, so simply use it.
*****************************************************************************/

int Table::columnWidth( int col ) const
{
    return topHeader->sectionSize( col );
}

int Table::rowHeight( int row ) const
{
    return leftHeader->sectionSize( row );
}

int Table::columnPos( int col ) const
{
    return topHeader->sectionPos( col );
}

int Table::rowPos( int row ) const
{
    return leftHeader->sectionPos( row );
}

int Table::columnAt( int pos ) const
{
    return topHeader->sectionAt( pos );
}

int Table::rowAt( int pos ) const
{
    return leftHeader->sectionAt( pos );
}

QRect Table::cellGeometry( int row, int col ) const
{
    return QRect( columnPos( col ), rowPos( row ),
		  columnWidth( col ), rowHeight( row ) );
}

QSize Table::tableSize() const
{
    return QSize( columnPos( cols() - 1 ) + columnWidth( cols() - 1 ),
		  rowPos( rows() - 1 ) + rowHeight( rows() - 1 ) );
}

int Table::rows() const
{
    return leftHeader->count();
}

int Table::cols() const
{
    return topHeader->count();
}




/****************************************************************************
  Helper functions for the in-place editor
*****************************************************************************/

void Table::editorOk()
{
    if ( !editor )
	return;
    setCellText( curRow, curCol, editor->text() );
    ediorCancel();
}

void Table::ediorCancel()
{
    if ( !editor )
	return;
    removeChild( editor );
    delete editor;
    viewport()->setFocus();
    editor = 0;
}

