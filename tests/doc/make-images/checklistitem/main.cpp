/*
    Use this to create: qlistviewitems.png
*/
#include <qapplication.h>
#include <qlistview.h>
#include <qpixmap.h>

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    QListView *lv = new QListView( 0 );
    app.setMainWidget( lv );
    lv->setRootIsDecorated( TRUE );
    lv->setSorting( -1 );
    lv->setAllColumnsShowFocus( TRUE );
    lv->addColumn( "Column One" );
    lv->addColumn( "Type" );
    lv->setCaption( "List View" );
    QCheckListItem *cli = new QCheckListItem( lv, "Item One", QCheckListItem::CheckBox );
    cli->setText( 1, "QCheckListItem (CheckBox)" );
    cli = new QCheckListItem( lv, "Item Two", QCheckListItem::CheckBox );
    cli->setText( 1, "QCheckListItem (CheckBox)" );
    cli->setPixmap( 1, QPixmap( "document.xpm" ) );
    cli->setOn( TRUE );
    cli = new QCheckListItem( lv, "Item Three", QCheckListItem::CheckBox );
    cli->setText( 1, "QCheckListItem (CheckBox)" );
    cli->setPixmap( 1, QPixmap( "document.xpm" ) );
    cli->setOn( TRUE );
    QCheckListItem *radioHead = new QCheckListItem( lv, "Item Four" );
    radioHead->setText( 1, "QCheckListItem (Controller)" );
    radioHead->setOpen( TRUE );
    cli = new QCheckListItem( radioHead, "Item Five", QCheckListItem::RadioButton );
    cli->setText( 1, "QCheckListItem (RadioButton)" );
    cli = new QCheckListItem( radioHead, "Item Six", QCheckListItem::RadioButton );
    cli->setText( 1, "QCheckListItem (RadioButton)" );
    cli->setPixmap( 1, QPixmap( "document.xpm" ) );
    cli->setOn( TRUE );
    cli = new QCheckListItem( radioHead, "Item Seven", QCheckListItem::RadioButton );
    cli->setText( 1, "QCheckListItem (RadioButton)" );
    QListViewItem *lvi = new QListViewItem( lv, "Item Eight", "QListViewItem" );
    lvi->setOpen( TRUE );
    lvi = new QListViewItem( lvi, "Item Nine", "QListViewItem" );
    lvi->setPixmap( 0, QPixmap( "book.xpm" ) );
    lvi->setPixmap( 1, QPixmap( "timeedit.png" ) );
    lvi->setOpen( TRUE );
    lvi = new QListViewItem( lvi, "Item Ten", "QListViewItem" );
    lvi->setPixmap( 0, QPixmap( "book.xpm" ) );
    lv->setCurrentItem( lvi );
    lvi->setPixmap( 1, QPixmap( "timeedit.png" ) );
    lvi = new QListViewItem( lv, "Item Eleven", "QListViewItem" );
    lvi->setPixmap( 0, QPixmap( "document.xpm" ) );
    lvi->setPixmap( 1, QPixmap( "timeedit.png" ) );
    lvi = new QListViewItem( lvi, "Item Twelve", "QListViewItem" );
    lv->show();
    return app.exec();
}
