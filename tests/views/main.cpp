#include <qapplication.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qiconview.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

class View : public QHBox
{
    Q_OBJECT

public:
    View() : QHBox() {

	QButtonGroup *bg = new QButtonGroup( 1, Qt::Horizontal, this );
	bg->setExclusive( TRUE );
	QRadioButton *rb;
	rb = new QRadioButton( "Single Selection", bg );
	rb->setChecked( TRUE );
	connect( rb, SIGNAL( clicked() ),
		 this, SLOT( singleSelection() ) );
	bg->insert( rb );
	rb = new QRadioButton( "Multi Selection", bg );
	connect( rb, SIGNAL( clicked() ),
		 this, SLOT( multiSelection() ) );
	bg->insert( rb );
	rb = new QRadioButton( "Extended Selection", bg );
	connect( rb, SIGNAL( clicked() ),
		 this, SLOT( extendedSelection() ) );
	bg->insert( rb );
	rb = new QRadioButton( "No Selection", bg );
	connect( rb, SIGNAL( clicked() ),
		 this, SLOT( noSelection() ) );
	bg->insert( rb );
	
	QVBox *hb = new QVBox( this );
	lb = new QListBox( hb );
	connect( lb, SIGNAL( doubleClicked( QListBoxItem * ) ),
		 this, SLOT( listboxSelected( QListBoxItem * ) ) );
	connect( lb, SIGNAL( currentChanged( QListBoxItem * ) ),
		 this, SLOT( listboxChanged( QListBoxItem * ) ) );
	connect( lb, SIGNAL( selectionChanged( QListBoxItem * ) ),
		 this, SLOT( listboxSelChanged( QListBoxItem * ) ) );
	lbl1 = new QLabel( "Changed:", hb );
	lbl2 = new QLabel( "Selected:", hb );
	lbl3 = new QLabel( "SelChanged:", hb );
	QPushButton *pb = new QPushButton( "setCurrentItem( 0 )", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( listboxNull() ) );
	pb = new QPushButton( "unselect current", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( listboxUnselect() ) );
	pb = new QPushButton( "Remove Item", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( listboxRemove() ) );
	
	hb = new QVBox( this );
	lv = new QListView( hb );
	lv->addColumn( "First Column" );
	lv->addColumn( "Second Column" );
	lv->addColumn( "Third Column" );
	connect( lv, SIGNAL( doubleClicked( QListViewItem * ) ),
		 this, SLOT( listviewSelected( QListViewItem * ) ) );
	connect( lv, SIGNAL( currentChanged( QListViewItem * ) ),
		 this, SLOT( listviewChanged( QListViewItem * ) ) );
	connect( lv, SIGNAL( selectionChanged( QListViewItem * ) ),
		 this, SLOT( listviewSelChanged( QListViewItem * ) ) );
	lvl1 = new QLabel( "Changed:", hb );
	lvl2 = new QLabel( "Selected:", hb );
	lvl3 = new QLabel( "SelChanged:", hb );
	pb = new QPushButton( "setCurrentItem( 0 )", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( listviewNull() ) );
	pb = new QPushButton( "unselect current", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( listviewUnselect() ) );
	pb = new QPushButton( "Remove Item", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( listviewRemove() ) );

	hb = new QVBox( this );
	iv = new QIconView( hb );
	connect( iv, SIGNAL( doubleClicked( QIconViewItem * ) ),
		 this, SLOT( iconviewSelected( QIconViewItem * ) ) );
	connect( iv, SIGNAL( currentChanged( QIconViewItem * ) ),
		 this, SLOT( iconviewChanged( QIconViewItem * ) ) );
	connect( iv, SIGNAL( selectionChanged( QIconViewItem * ) ),
		 this, SLOT( iconviewSelChanged( QIconViewItem * ) ) );
	ivl1 = new QLabel( "Changed:", hb );
	ivl2 = new QLabel( "Selected:", hb );
	ivl3 = new QLabel( "SelChanged:", hb );
	pb = new QPushButton( "setCurrentItem( 0 )", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( iconviewNull() ) );
	pb = new QPushButton( "unselect current", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( iconviewUnselect() ) );
	pb = new QPushButton( "Remove Item", hb );
	connect( pb, SIGNAL( clicked() ),
		 this, SLOT( iconviewRemove() ) );

	for ( unsigned int i = 0; i < 10; ++i ) {
	    new QListBoxText( lb, tr( "Item %1" ).arg( i ) );
	    new QListViewItem( lv, tr( "Item %1" ).arg( i ), "Col 2", "Col 3" );
	    new QIconViewItem( iv, tr( "Item %1" ).arg( i ) );
	}
    }

private slots:
    void listboxChanged( QListBoxItem *item ) {
	if ( !item )
	    return;
	lbl1->setText( "In Listbox changed: " + item->text() );
    }
    void listviewChanged( QListViewItem *item ) {
	if ( !item )
	    return;
	lvl1->setText( "In Listview changed: " + item->text( 0 ) );
    }
    void iconviewChanged( QIconViewItem *item ) {
	if ( !item )
	    return;
	ivl1->setText( "In Iconview changed: " + item->text() );
    }
    void listboxSelected( QListBoxItem *item ) {
	if ( !item )
	    return;
	lbl2->setText( "In Listbox selected: " + item->text() );
    }
    void listviewSelected( QListViewItem *item ) {
	if ( !item )
	    return;
	lvl2->setText( "In Listview selected: " + item->text( 0 ) );
    }
    void iconviewSelected( QIconViewItem *item ) {
	if ( !item )
	    return;
	ivl2->setText( "In Iconview selected: " + item->text() );
    }
    void listboxSelChanged( QListBoxItem *item ) {
	if ( !item )
	    return;
	lbl3->setText( "In Listbox sel-changed: " + item->text() );
    }
    void listviewSelChanged( QListViewItem *item ) {
	if ( !item )
	    return;
	lvl3->setText( "In Listview sel-chaned: " + item->text( 0 ) );
    }
    void iconviewSelChanged( QIconViewItem *item ) {
	if ( !item )
	    return;
	ivl3->setText( "In Iconview sel-chaned: " + item->text() );
    }
    void listboxNull() {
	lb->setCurrentItem( (QListBoxItem*)0 );
    }
    void listviewNull() {
	lv->setCurrentItem( 0 );
    }
    void iconviewNull() {
	iv->setCurrentItem( 0 );
    }
    void listboxUnselect() {
	if ( lb->currentItem() != -1 )
	    lb->setSelected( lb->item( lb->currentItem() ), FALSE );
    }
    void listviewUnselect() {
	if ( lv->currentItem() )
	    lv->setSelected( lv->currentItem(), FALSE );
    }
    void iconviewUnselect() {
	if ( iv->currentItem() )
	    iv->setSelected( iv->currentItem(), FALSE );
	iv->setCurrentItem( 0 );
    }
    void listboxRemove() {
	if ( lb->currentItem() != -1 )
	    delete lb->item( lb->currentItem() );
    }
    void listviewRemove() {
	if ( lv->currentItem() )
	    delete lv->currentItem();
    }
    void iconviewRemove() {
	if ( iv->currentItem() )
	    delete iv->currentItem();
    }
    void singleSelection() {
	lb->setSelectionMode( QListBox::Single );
	lv->setSelectionMode( QListView::Single );
	iv->setSelectionMode( QIconView::Single );
    }
    void multiSelection() {
	lb->setSelectionMode( QListBox::Multi );
	lv->setSelectionMode( QListView::Multi );
	iv->setSelectionMode( QIconView::Multi );
    }
    void extendedSelection() {
	lb->setSelectionMode( QListBox::Extended );
	lv->setSelectionMode( QListView::Extended );
	iv->setSelectionMode( QIconView::Extended );
    }
    void noSelection() {
	lb->setSelectionMode( QListBox::NoSelection );
	lv->setSelectionMode( QListView::NoSelection );
	iv->setSelectionMode( QIconView::NoSelection );
    }

private:
    QLabel *lbl1, *lbl2, *lvl1, *lvl2, *ivl1, *ivl2, *lbl3, *lvl3, *ivl3;
    QListBox *lb;
    QListView *lv;
    QIconView *iv;

};

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    View v;
    a.setMainWidget( &v );
    v.resize( 900, 480 );
    v.show();

    return a.exec();
}

#include "main.moc"
