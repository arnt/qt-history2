//
// Qt example: Table
//
// A simple, spreadsheet-like widget, made by inheriting  QTableView.
// 
// File: main.cpp
//
// The main() function, showing the use of the Table widget
// 

#include "table.h"
#include <qpushbutton.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qheader.h>

/*
  Constants
*/

const int numRows = 20;				// Tablesize: number of rows
const int numCols = 20;				// Tablesize: number of columns

/*
  The program starts here. 
*/

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    QWidget w;

    QVBoxLayout l(&w);

    QHeader h( numCols, &w );
    h.setFixedHeight( h.sizeHint().height() );

    l.addWidget( &h );
    Table v( numRows, numCols, &w );
    v.setHeader( &h );

    l.addWidget( &v );

    //    QObject::connect( &b, SIGNAL(clicked()), &v, SLOT(increaseWidth()) );
    /*
      Fill the table with default content: a coordinate string.
    */
    QString s ;
    for( int i = 0; i < numRows; i++ ) {
	for( int j = 0; j < numCols; j++ ) {
	    s.setNum(j);
	    s += ' ';
	    s += 'A' + ( i % 26 );		// Wrap if necessary
	    v.setCellContent( i, j, s );
	}
    }

    a.setMainWidget( &w );
    w.show();

    return a.exec();
}
