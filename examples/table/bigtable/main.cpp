/****************************************************************************
** $Id: //depot/qt/main/examples/table/main.cpp#2 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qtable.h>

// Table size

const int numRows = 1000000;
const int numCols = 1000000;

class MyTable : public QTable
{
public:
    MyTable( int r, int c ) : QTable( r, c ) {
	items.setAutoDelete( TRUE );
	widgets.setAutoDelete( TRUE );
	setCaption( tr( "This is a big table with 1.000.000x1.000.000 cells..." ) );
	setLeftMargin( fontMetrics().width( "W999999W" ) );
    }

    void resizeData( int ) {}
    QTableItem *item( int r, int c ) const { return items.find( indexOf( r, c ) ); }
    void setItem( int r, int c, QTableItem *i ) { items.replace( indexOf( r, c ), i ); }
    void clearCell( int r, int c ) { items.remove( indexOf( r, c ) ); }
    void insertWidget( int r, int c, QWidget *w ) { widgets.replace( indexOf( r, c ), w );  }
    QWidget *cellWidget( int r, int c ) const { return widgets.find( indexOf( r, c ) ); }
    void clearCellWidget( int r, int c ) { widgets.remove( indexOf( r, c ) ); }

private:
    QIntDict<QTableItem> items;
    QIntDict<QWidget> widgets;

};

// The program starts here.

int main( int argc, char **argv )
{
    QApplication app( argc, argv );			

    MyTable table( numRows, numCols );
    app.setMainWidget( &table );
    table.show();
    return app.exec();
}
