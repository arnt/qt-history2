//
// Qt Example Application: widgets
//
// Demonstrates Qt widgets.
//

#include <qdialog.h>
#include <qmsgbox.h>
#include <qpixmap.h>
#include <qapp.h>

// Standard Qt widgets

#include <qbttngrp.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qframe.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlcdnum.h>
#include <qlined.h>
#include <qlistview.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qscrbar.h>
#include <qdrawutl.h>


#include "lv.moc"

int main( int argc, char **argv )
{
    QApplication  a( argc, argv );

    QListView *lb = new QListView( 0, "Hepp" );
    //lb->setFont( QFont( "Times", 24, 75 ) );
    lb->setColumn( "Name", 100 );
    lb->setColumn( "Size", 50 );
    lb->setColumn( "Type", 50 );
    lb->setColumn( "Date", 100 );
    //    connect( files, SIGNAL(sizeChanged()), SLOT(updateGeometry()) );


    lb->setTreeStepSize( 20 );

    //    QListViewItem * roo= new QListViewItem( lb );

    QListView * roo = lb;
    

    for ( int j = 0; j < 2; j++ ) {
	QListViewItem * root= new QListViewItem( roo, "a", "b", "c", "d" );

	QListViewItem *hurfu = 0;

	for ( int i=45; i>35; i-- ) {
	    QString str;
	    str.sprintf( "%d item", i );
	    QListViewItem* h= new QListViewItem( root, str, "test", "hei", "hopp", 0 );
	    if ( i == 42 )
		hurfu = h;
	}

	QListViewItem *gruff = 0;

	for ( int i=4; i>0; i-- ) {
	    QString str;
	    str.sprintf( "Haba %d", i );
	    QListViewItem *h = new QListViewItem( hurfu, "SUBB", (const char*)str, 
						  "urk", "nei", 0 );
	    if ( i == 4 )
		gruff = h;
	}

	new QListViewItem(  gruff, "Dutt", "hepp", "glurk", "niks", 0 );

    }

    a.setMainWidget( lb );
    lb->show();

    return a.exec();
}
