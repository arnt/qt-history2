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
#include <qimage.h>
#include <qpixmap.h>
#include <qstringlist.h>

// Qt logo: static const char *qtlogo_xpm[]
#include "qtlogo.xpm"

// Table size

const int numRows = 100;
const int numCols = 100;

// The program starts here.

int main( int argc, char **argv )
{
    QApplication app( argc, argv );			

    QTable table( numRows, numCols );
    QImage img( qtlogo_xpm );
    QPixmap pix = img.scaleHeight( table.rowHeight(3) );
    table.setPixmap( 3, 2, pix );
    table.setText( 3, 2, "A Pixmap" );

    QStringList comboEntries;
    comboEntries << "one" << "two" << "three" << "four";

    for ( int i = 0; i < numRows; ++i )
	table.setItem( i, 5, new QComboTableItem( &table, comboEntries, 
	               TRUE ) );
    for ( int j = 0; j < numRows; ++j )
	table.setItem( j, 1, new QCheckTableItem( &table, "Check me" ) );

    app.setMainWidget( &table );
    table.show();
    return app.exec();
}
