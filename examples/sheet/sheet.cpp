/****************************************************************************
** $Id: //depot/qt/main/examples/sheet/sheet.cpp#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qscrollbar.h>

#include "pie.h"
#include "sheet.h"


static const int rows = 5;
static const int cols = 7;


Sheet::Sheet( QWidget *parent, const char *name )
    :QWidget(parent,name)
{
    pie = 0;
    setBackgroundMode( PaletteBase );
    tableView = new MyTableView(rows,cols,this,Tbl_clipCellPainting );

    table = new ParsedArray(rows,cols);

    head = new MyTableLabel( 'A', "Column ", this );
    head->setNumCols(1);
    head->setNumRows(1);
    head->setCellHeight(tableView->cellHeight());
    head->setCellWidth(tableView->cellWidth());

    side = new MyTableLabel(  0 , "Row ", this );
    side->setNumCols(1);
    side->setNumRows(1);
    side->setCellHeight(tableView->cellHeight());
    side->setCellWidth(tableView->cellWidth()/2);

    extraH = head->tHeight();
    extraW = side->tWidth();

    head->move(extraW,0);
    side->move(0,extraH);
    tableView->move(extraW,extraH);

    connect( tableView, SIGNAL(selected(int,int)),
	     this, SLOT(exportText(int,int)) );
    connect( tableView, SIGNAL(newText(int,int,const QString&)),
	     this, SLOT(importText(int,int,const QString&)) );



    horz = new QScrollBar( QScrollBar::Horizontal,this,"scrollBar" );
    horz->resize( tableView->width(), 16 );
    horz->setRange( 0, tableView->numCols() - tableView->numColsVisible() );
    horz->setSteps( 1, tableView->numColsVisible() );

    connect( tableView, SIGNAL(newCol(int)), head, SLOT(setStart(int)));
    connect( tableView, SIGNAL(newCol(int)), this, SLOT(setHorzBar(int)) );
    connect( horz, SIGNAL(valueChanged(int)),
	     tableView, SLOT(scrollHorz(int)));
    connect( horz, SIGNAL(valueChanged(int)),
	     head, SLOT(setStart(int)));
    extraH += horz->height();

    vert = new QScrollBar( QScrollBar::Vertical,this,"scrollBar" );
    vert->resize( 16, tableView->width() );
    vert->setRange( 0, tableView->numRows() - tableView->numRowsVisible() );
    vert->setSteps( 1, tableView->numRowsVisible() );

    connect( tableView, SIGNAL(newRow(int)), side, SLOT(setStart(int)) );
    connect( tableView, SIGNAL(newRow(int)), this, SLOT(setVertBar(int)) );
    connect( vert, SIGNAL(valueChanged(int)),
	     tableView, SLOT(scrollVert(int)));
    connect( vert, SIGNAL(valueChanged(int)),
	     side, SLOT(setStart(int)));
    extraW += vert->width();

}



Sheet::~Sheet()
{
    delete tableView;
    delete side;
    delete head;
    delete table;
}

static int vals[11]; //### not exactly a general solution
static QString strs[11];

void Sheet::showPie()
{

    int i=0;
    for ( int row=0; row < rows; row++ )
	if ( table->type(row,0) == ParsedArray::Number
	     && table->intVal(row,0) > 0 ) {
	    vals[i] = table->intVal(row,0);
	    strs[i] = table->rawText(row,1);
	    i++;
	}
    vals[i]=0;

    if (pie)
	pie->restart(vals,strs);
    else
	pie = new PieView(vals,strs);
    pie ->show();
}
void Sheet::hidePie()
{
    if (pie)
	pie->hide();
}

void Sheet::setHorzBar(int val)
{
    horz->setValue(val);
}

void Sheet::setVertBar(int val)
{
    vert->setValue(val);
}

void Sheet::importText( int row, int col, const QString &s )
{
    table->setText( row, col, s.copy() );
    tableView->showText( row, col, table->calc( row, col ) );
}
void Sheet::exportText( int row, int col )
{
    tableView->setInputText( table->rawText(row,col) );
}
void Sheet::resizeEvent( QResizeEvent * e )
{
    int w = e->size().width() - extraW;
    int h = e->size().height() - extraH;
    int c = w / tableView->cellWidth(); //### TODO: variable width
    int r = h / tableView->cellHeight();
    c = QMIN( c, cols );
    r = QMIN( r, rows );
    
    h = r * tableView->cellHeight();
    w = c * tableView->cellWidth();
    side->setNumRows( r );
    side->resize( side->tWidth(), h );
    head->setNumCols( c );
    head->resize( w, head->height() );

    tableView->resize( w + tableView->extraW+1, h + tableView->extraH+1 );
    QRect cr = tableView->geometry();
    if ( tableView->numCols() <= tableView->numColsVisible() ) {
	horz->hide();
    } else {
	horz->setGeometry( cr.left(), cr.bottom() + 1,
			   cr.width(), horz->height() );
	horz->setRange( 0, tableView->numCols() - tableView->numColsVisible() );
	horz->setSteps( 1, tableView->numColsVisible() );
	horz->show();
    }
    if ( tableView->numRows() <= tableView->numRowsVisible() ) {
	vert->hide();
    } else {
	vert->setGeometry( cr.right() + 1, cr.top(),
			   vert->width(), cr.height() );
	vert->setRange( 0, tableView->numRows() - tableView->numRowsVisible() );
	vert->setSteps( 1, tableView->numRowsVisible() );
	vert->show();
    }
}
