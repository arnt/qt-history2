/*
    Use this to create: qtableitems.png
*/
#include <qapplication.h>
#include <qtable.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qstringlist.h>

#include "qtlogo.xpm"

const int numRows = 5;
const int numCols = 3;

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QTable table( numRows, numCols );

    QHeader *header = table.horizontalHeader();
    header->setLabel( 0, QObject::tr( "QTableItem" ) );
    header->setLabel( 1, QObject::tr( "QCheckTableItem" ) );
    header->setLabel( 2, QObject::tr( "QComboTableItem" ) );

    int i;
    for ( i = 0; i < numCols; ++i )
	table.setColumnWidth( i, 130 );

    for ( i = 0; i < numRows; ++i )
	table.setText( i, 0, QString( "Item %1" ).arg( i ) );

    QImage img( qtlogo_xpm );
    QPixmap pix = img.scaleHeight( table.rowHeight(3) );
    table.setPixmap( 2, 0, pix );
    table.setText( 2, 0, "Pixmap Item" );

    QStringList comboEntries;
    comboEntries << "One" << "Two" << "Three" << "Four" << "Five";

    for ( i = 0; i < numRows; ++i ){
	QComboTableItem * item = new QComboTableItem( &table, comboEntries, FALSE );
	item->setCurrentItem( i % 5 );
	table.setItem( i, 2, item );
    }
    for ( i = 0; i < numRows; ++i )
	table.setItem( i, 1, new QCheckTableItem( &table, QString( "Check %1" ).arg( i ) ) );
    ((QCheckTableItem*)table.item( 2, 1 ))->setChecked( TRUE );
    ((QCheckTableItem*)table.item( 3, 1 ))->setChecked( TRUE );
    table.setCurrentCell( 4, 0 );

    app.setMainWidget( &table );
    table.setCaption( "Table Items" );
    table.resize( 425, 130 );
    table.show();
    return app.exec();
}
