/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/table.cpp#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qpainter.h>
#include <qdrawutil.h>

#include "table.h"

MyTableView::MyTableView( int rows, int cols, QWidget *parent,
			  int flags, const char *name )
    :QTableView(parent,name)
{
    setBackgroundMode( PaletteBase );
    if ( flags < 0 )
	setTableFlags( Tbl_clipCellPainting| Tbl_autoScrollBars |
		       Tbl_snapToGrid| Tbl_cutCells );
    else
	setTableFlags( flags );
    setNumRows(rows);
    setNumCols(cols);
    table = new QString[rows*cols];

    setCellWidth(100);
    setCellHeight(30);

    extraW = width() - viewWidth();
    extraH = height() - viewHeight();

    input = new QLineEdit(this);
    input->setFrame( FALSE );
    input->resize( cellWidth()-2, cellHeight()-2 );
    moveInput(0,0);
    input->setFocus();
    connect( input, SIGNAL(returnPressed()), this, SLOT(nextInput()) );
}


MyTableView::~MyTableView()
{
    delete[] table;
}


void MyTableView::setText( int row, int col, QString s, bool paint )
{
    table[index(row,col)] = s.copy();

    int x,y;
    if ( paint && rowYPos( row, &y ) && colXPos( col, &x )) 
	repaint( x,y, cellWidth(col), cellHeight(row));
}

void MyTableView::placeInput()
{
    // makeVisible( inRow, inCol );
    int x,y;
    if ( colXPos(inCol,&x) && rowYPos(inRow,&y) ) {
	input->move(x+1,y+1);
	input->show();
	if (!input->hasFocus()) 
	    input->setFocus();
    } else
	input->hide();
}

void MyTableView::paintCell( QPainter *p, int row, int col ) 
{
    p->drawLine(0,0, cellWidth(),0);
    p->drawLine(0,0, 0, cellHeight());
    if ( row == numRows()-1 )
	p->drawLine(0,cellHeight()-1, cellWidth(), cellHeight()-1);
    if ( col == numCols()-1 )
	p->drawLine(cellWidth()-1,0,  cellWidth()-1, cellHeight());
    /*
      qDrawShadePanel( p, 0, 0,
      cellWidth(), cellHeight(),
      colorGroup(), FALSE); // raised
    */
    QString str;
    if (table)
	str = table[index(row,col)].copy();
    if ( str.isEmpty() )
	str.sprintf( "Cell %c%d", col+'A', row );
    p->drawText( 1, 1, cellWidth()-2, cellHeight()-2,
		 AlignCenter, str );

    if ( row == inRow && col == inCol )
	placeInput();

}

void MyTableView::setInputText( QString s )
{
    input->setText( s );
}

void MyTableView::nextInput()
{
    int c = inCol;
    int r = ( inRow + 1 ) % numRows();
    if ( !r )
	c = ( inCol + 1 ) % numCols();
    moveInput( r, c );
}

void MyTableView::moveInput( int row, int col )
{
    if ( col < 0 || row < 0 )
	return;
    if ( col == inCol && row == inRow )
	return;

    if ( inRow >= 0 && inCol >= 0 ) {
	QString str = input->text();
	setText( inRow, inCol, str );
	emit newText(inRow, inCol, str );
    }
    inCol = col;
    inRow = row;
    makeVisible( inRow, inCol );
    placeInput();
    emit selected(row,col);
}

void MyTableView::makeVisible( int row, int col )
{
    if ( col < leftCell() ) {
	setLeftCell(col);
	emit newCol(col);
    } else if ( col > lastColVisible() ) {
	int c = leftCell() + col - lastColVisible();
	setLeftCell(c);
	emit newCol(c);
    }

    if ( row < topCell() ) {
	setTopCell(row);
	emit newRow(row);
    } else if ( row > lastRowVisible() ) {
	int r = topCell() + row - lastRowVisible();
	setTopCell(r);
	emit newRow(r);
    } 
}

void MyTableView::mousePressEvent( QMouseEvent * e )
{
    int col = findCol(e->pos().x());
    int row = findRow(e->pos().y());
    moveInput( row, col );
}


void MyTableView::scrollHorz( int col )
{
    setLeftCell( col );
    placeInput();
}

void MyTableView::scrollVert(int row )
{
    setTopCell( row );
    placeInput();
}

MyTableLabel::MyTableLabel( char b, const char *str, 
			    QWidget *parent, const char *name )
    :QTableView( parent, name ),
     base( b ), text( str )
{
    setBackgroundMode( PaletteBase );
    setTableFlags(Tbl_clipCellPainting);
    start = 0;
    if (b)
	text += "%c";
    else
	text += "%d";
}

void MyTableLabel::paintCell( QPainter * p, int row, int col )
{
	QString str;
	str.sprintf( text, base + start + row + col );
	p->drawText( 1, 1, cellWidth()-2, cellHeight()-2,
		     AlignCenter, str );

}
