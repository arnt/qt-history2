//
// Qt Example Application: widgets
//
// Demonstrates Qt widgets.
//

#include <qapp.h>

#include <qlistview.h>

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
    
    int i;
    for ( int j = 0; j < 2; j++ ) {
	QListViewItem * root= new QListViewItem( roo, "a", "b", "c", "d" );

	//	root->setOpen(TRUE);

	QListViewItem *hurfu = 0;

	for ( i=35; i<45; i++ ) {
	    QString str;
	    str.sprintf( "%d item", i );
	    QListViewItem* h= new QListViewItem( root, str, "test", "hei", "hopp", 0 );
	    //	    h->setOpen( TRUE );
	    if ( i == 42 )
		hurfu = h;
	}

	QListViewItem *gruff = 0;

	for ( i=4; i>0; i-- ) {
	    QString str;
	    str.sprintf( "Haba %d", i );
	    QListViewItem *h = new QListViewItem( hurfu, "SUBB", (const char*)str, 
						  "urk", "nei", 0 );
	    if ( i == 4 )
		gruff = h;
	}

	new QListViewItem(  gruff, "Dutt", "hepp", "glurk", "niks", 0 );
	// new QCheckListItem( gruff, "Doesn't compile", QCheckListItem::RadioButton );
    }

    new QCheckListItem( lb, "Harsh", QCheckListItem::CheckBox );
    new QCheckListItem( lb, "Angry", QCheckListItem::CheckBox );
    new QCheckListItem( lb, "Vicious", QCheckListItem::CheckBox );
    new QCheckListItem( lb, "Lethal", QCheckListItem::CheckBox );
    //    new QCheckListItem( lb, "Stupid", QCheckListItem::RadioButton );

    QCheckListItem *lit = new QCheckListItem( lb, "One" );
    lit->setOpen( TRUE );
    new QCheckListItem( lit, "Two", QCheckListItem::CheckBox );
    new QCheckListItem( lit, "Three", QCheckListItem::CheckBox );
    QCheckListItem *it = new QCheckListItem( lit, "Four" );
    it->setOpen( TRUE );
    new QCheckListItem( it, "Five", QCheckListItem::RadioButton);
    new QCheckListItem( it, "Six", QCheckListItem::RadioButton);
    new QCheckListItem( it, "Six and a half", QCheckListItem::RadioButton );

    it = new QCheckListItem( lit, "Seven" );
    it->setOpen( TRUE );
    new QCheckListItem( it, "Eight", QCheckListItem::CheckBox );
    new QCheckListItem( it, "Nine", QCheckListItem::CheckBox );
    new QCheckListItem( it, "Ten", QCheckListItem::CheckBox );
    it = new QCheckListItem( it, "Eleven" );
    new QCheckListItem( it, "Twelve", QCheckListItem::RadioButton );
    new QCheckListItem( it, "Thirteen", QCheckListItem::RadioButton );
    new QCheckListItem( it, "Fourteen", QCheckListItem::RadioButton );



#if 0
    QFont f("Helvetica", 24, QFont::Bold);
    lb->setFont( f );              
#endif
    a.setMainWidget( lb );
    lb->show();

    return a.exec();
}
