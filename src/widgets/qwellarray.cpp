/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qwellarray.cpp#22 $
**
** Implementation of QWellArray widget class
**
** Created : 980114
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qwellarray_p.h"

//#include "qlayout.h"
#include "qdrawutil.h"
//#include "qevent.h"
//#include "qobjectlist.h"
#include "qobjectdict.h"

struct QWellArrayData {
    QBrush *brush;
};

// NOT REVISED
/* WARNING, NOT
  \class QWellArray qwellarray_p.h
  \brief ....

  ....

  \ingroup advanced
*/

QWellArray::QWellArray( QWidget *parent, const char * name, bool popup )
    : QTableView( parent, name,
		  popup ? (WStyle_Customize|WStyle_Tool|WStyle_NoBorder) : 0 )
{
    d = 0;
    setFocusPolicy( StrongFocus );
    setBackgroundMode( PaletteButton );
    nCols = 7;
    nRows = 7;
    int w = 24;		// cell width
    int h = 21;		// cell height
    smallStyle = popup;

    if ( popup ) {
	w = h = 18;
	if ( style() == WindowsStyle )
	    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	else
	    setFrameStyle( QFrame::Panel | QFrame::Raised );
	setMargin( 1 );
	setLineWidth( 2 );
    }
    setNumCols( nCols );
    setNumRows( nRows );
    setCellWidth( w );
    setCellHeight( h );
    curCol = 0;
    curRow = 0;
    selCol = -1;
    selRow = -1;

    if ( smallStyle )
	setMouseTracking( TRUE );
    setOffset( 5 , 10 );

    resize( sizeHint() );

}


QSize QWellArray::sizeHint() const
{
    constPolish();
    int f = frameWidth() * 2;
    int w = nCols * cellWidth() + f;
    int h = nRows * cellHeight() + f;
    return QSize( w, h );
}


void QWellArray::paintCell( QPainter* p, int row, int col )
{
    int w = cellWidth( col );			// width of cell in pixels
    int h = cellHeight( row );			// height of cell in pixels
    int b = 1;

    if ( !smallStyle )
	b = 3;

    const QColorGroup & g = colorGroup();
    p->setPen( QPen( black, 0, SolidLine ) );
    if ( !smallStyle && row ==selRow && col == selCol &&
	 style() != MotifStyle ) {
	int n = 2;
	p->drawRect( n, n, w-2*n, h-2*n );
    }


    if ( style() == WindowsStyle ) {
	qDrawWinPanel( p, b, b ,  w - 2*b,  h - 2*b,
		       g, TRUE );
	b += 2;
    } else {
	if ( smallStyle ) {
	    qDrawShadePanel( p, b, b ,  w - 2*b,  h - 2*b,
			     g, TRUE, 2 );
	    b += 2;
	} else {
	    int t = ( row == selRow && col == selCol ) ? 2 : 0;
	    b -= t;
	    qDrawShadePanel( p, b, b ,  w - 2*b,  h - 2*b,
			     g, TRUE, 2 );
	    b += 2 + t;
	}
    }


    if ( (row == curRow) && (col == curCol) ) {
	if ( smallStyle ) {
	    p->setPen ( white );
	    p->drawRect( 1, 1, w-2, h-2 );
	    p->setPen ( black );
	    p->drawRect( 0, 0, w, h );
	    p->drawRect( 2, 2, w-4, h-4 );
	    b = 3;
	} else if ( hasFocus() ) {
	    style().drawFocusRect(p, QRect(0,0,w,h), g );
	}
    }
    drawContents( p, row, col, QRect(b, b, w - 2*b, h - 2*b) );
}

/*!
  Pass-through to QTableView::drawContents() to avoid hiding.
*/
void QWellArray::drawContents( QPainter *p )
{
    QTableView::drawContents(p);
}

/*!
  Reimplement this function to change the contents of the well array.
 */
void QWellArray::drawContents( QPainter *p, int row, int col, const QRect &r )
{

    if ( d ) {
	p->fillRect( r, d->brush[row*nCols+col] );
    } else {
	p->fillRect( r, white );
	p->setPen( black );
	p->drawLine( r.topLeft(), r.bottomRight() );
	p->drawLine( r.topRight(), r.bottomLeft() );
    }
}


/*\reimp
*/
void QWellArray::mousePressEvent( QMouseEvent* e )
{
    // The current cell marker is set to the cell the mouse is pressed
    // in.
    QPoint pos = e->pos();
    setCurrent( findRow( pos.y() ), findCol( pos.x() ) );
}

/*\reimp
*/
void QWellArray::mouseReleaseEvent( QMouseEvent* )
{
    // The current cell marker is set to the cell the mouse is clicked
    // in.
    setSelected( curRow, curCol );
}


/*\reimp
*/
void QWellArray::mouseMoveEvent( QMouseEvent* e )
{
    //   The current cell marker is set to the cell the mouse is
    //   clicked in.
    if ( smallStyle ) {
	QPoint pos = e->pos();
	setCurrent( findRow( pos.y() ), findCol( pos.x() ) );
    }
}

/*
  Sets the cell currently having the focus. This is not necessarily
  the same as the currently selected cell.
*/

void QWellArray::setCurrent( int row, int col )
{

    if ( (curRow == row) && (curCol == col) )
	return;

    if ( row < 0 || col < 0 )
	row = col = -1;

    int oldRow = curRow;
    int oldCol = curCol;

    curRow = row;
    curCol = col;

    updateCell( oldRow, oldCol );
    updateCell( curRow, curCol );
}


/*!
  Sets the currently selected cell to \a row, \a col.  If \a row or \a
  col are less than zero, the current cell is unselected.

  Does not set the position of the focus indicator.
*/

void QWellArray::setSelected( int row, int col )
{
    if ( (selRow == row) && (selCol == col) )
	return;

    int oldRow = selRow;
    int oldCol = selCol;

    if ( row < 0 || col < 0 )
	row = col = -1;

    selCol = col;
    selRow = row;

    updateCell( oldRow, oldCol );
    updateCell( selRow, selCol );
    if ( row >= 0 )
	emit selected( row, col );

    if ( isVisible() && parentWidget() && parentWidget()->inherits("QPopupMenu") )
	parentWidget()->close();

}



/*!\reimp
*/
void QWellArray::focusInEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );
}


/*!
  Sets the size of the well array to be \c rows cells by \c cols.
  Resets any brush info set by setCellBrush().

  Must be called by reimplementors.
 */
void QWellArray::setDimension( int rows, int cols )
{
    nRows = rows;
    nCols = cols;
    if ( d ) {
	if ( d->brush )
	    delete[] d->brush;
	delete d;
	d = 0;
    }
    setNumCols( nCols );
    setNumRows( nRows );
}

void QWellArray::setCellBrush( int row, int col, const QBrush &b )
{
    if ( !d ) {
	d = new QWellArrayData;
	d->brush = new QBrush[nRows*nCols];
    }
    if ( row >= 0 && row < nRows && col >= 0 && col < nCols )
	d->brush[row*nCols+col] = b;
#ifdef CHECK_RANGE
    else
	qWarning( "QWellArray::setCellBrush( %d, %d ) out of range", row, col );
#endif
}



/*!
  Returns the brush set for the cell at \a row, \a col. If no brush is set,
  \c NoBrush is returned.
*/

QBrush QWellArray::cellBrush( int row, int col )
{
    if ( d && row >= 0 && row < nRows && col >= 0 && col < nCols )
	return d->brush[row*nCols+col];
    return NoBrush;
}



/*!\reimp
*/

void QWellArray::focusOutEvent( QFocusEvent* )
{
    updateCell( curRow, curCol );
}

/*\reimp
*/
void QWellArray::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() ) {			// Look at the key code
    case Key_Left:				// If 'left arrow'-key,
	if( curCol > 0 ) {			// and cr't not in leftmost col
	    setCurrent( curRow, curCol - 1);	// set cr't to next left column
	    int edge = leftCell();		// find left edge
	    if ( curCol < edge )		// if we have moved off  edge,
		setLeftCell( edge - 1 );	// scroll view to rectify
	}
	break;
    case Key_Right:				// Correspondingly...
	if( curCol < numCols()-1 ) {
	    setCurrent( curRow, curCol + 1);
	    int edge = lastColVisible();
	    if ( curCol >= edge )
		setLeftCell( leftCell() + 1 );
	}
	break;
    case Key_Up:
	if( curRow > 0 ) {
	    setCurrent( curRow - 1, curCol);;
	    int edge = topCell();
	    if ( curRow < edge )
		setTopCell( edge - 1 );
	} else if ( smallStyle )
	    focusNextPrevChild( FALSE );
	break;
    case Key_Down:
	if( curRow < numRows()-1 ) {
	    setCurrent( curRow + 1, curCol);
	    int edge = lastRowVisible();
	    if ( curRow >= edge )
		setTopCell( topCell() + 1 );
	} else if ( smallStyle )
	    focusNextPrevChild( TRUE );
	break;
    case Key_Space:
    case Key_Return:
    case Key_Enter:
	setSelected( curRow, curCol );
	break;
    default:				// If not an interesting key,
	e->ignore();			// we don't accept the event
	return;
    }

}
