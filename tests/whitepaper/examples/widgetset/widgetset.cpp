/*
  widgetset.cpp
*/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qdial.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qiconview.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>

#include "qtlogo.xpm"

int nextXOffset = 10;
int nextYOffset = 10;

template <class Widget>
void standardLayoutWidgetStuff( Widget *w )
{
    w->setMargin( 6 );
    w->setSpacing( 6 );
    w->move( nextXOffset, nextYOffset );
    w->show();

    nextXOffset += w->width() + 10;
    if ( nextXOffset > 1400 ) {
	nextXOffset = 10;
	nextYOffset += 320;
    }
}

void standardRangeControllStuff( QRangeControl *rc )
{
    rc->setRange( 0, 100 );
    rc->setValue( 64 );
}

void setupLabelAndPushButton()
{
    QHBox *parent = new QHBox;
    QLabel *label = new QLabel( "&A label", parent );
    QPushButton *button = new QPushButton( "&Push button", parent );
    label->setBuddy( button );
    standardLayoutWidgetStuff( parent );
}

void setupButtons()
{
    QHBox *realParent = new QHBox;
    QButtonGroup *parent;
    QRadioButton *radio1, *radio2;
    QCheckBox *check1, *check2;

    // quote
    parent = new QButtonGroup( 2, Qt::Vertical, "QButtonGroup" );
    radio1 = new QRadioButton( "&Radio 1", parent );
    radio2 = new QRadioButton( "R&adio 2", parent );
    radio1->setChecked( TRUE );
    check1 = new QCheckBox( "&Check 1", parent );
    check2 = new QCheckBox( "C&heck 2", parent );
    check2->setChecked( TRUE );
    // endquote

    parent->reparent( realParent, QPoint(0, 0) );
    standardLayoutWidgetStuff( realParent );
}

void setupEdits()
{
    QHBox *parent = new QHBox;
    QVGroupBox *grp = new QVGroupBox( "QGroupBox", parent );

    QDateTimeEdit *dtEdit = new QDateTimeEdit( grp );
    dtEdit->setDateTime( QDateTime(QDate(1905, 5, 17), QTime(3, 14, 16)) );
    dtEdit->dateEdit()->setOrder( QDateEdit::YMD );
    dtEdit->dateEdit()->setSeparator( QChar('-') );
    dtEdit->timeEdit()->setSeparator( QChar(':') );

    (void) new QLineEdit( "QLineEdit", grp );

    QTextEdit *ted = new QTextEdit( grp );
    ted->setText( "<font color=blue><b>QTextEdit</b></font>"
		  "<p>``Everything should always be made as simple as possible,"
		  " but not simpler.''"
		  "<p align=\"right\"><i>Albert Einstein</i>" );

    QComboBox *combo = new QComboBox( TRUE, grp );
    combo->insertItem( "Combobox item 1" );
    combo->insertItem( "Combobox item 2" );
    combo->insertItem( "Combobox item 3" );
    combo->setCurrentText( "Combobox text" );

    standardLayoutWidgetStuff( parent );
}

void setupRangeWidgets()
{
    QGrid *parent = new QGrid( 2, Qt::Horizontal );

    QDial *dial = new QDial( parent );
    dial->setNotchesVisible( TRUE );
    dial->setFixedSize( 60, 60 );
    standardRangeControllStuff( dial );

    QProgressBar *progress = new QProgressBar( parent );
    progress->setProgress( 64 );

    QSpinBox *spin = new QSpinBox( parent );
    standardRangeControllStuff( spin );

    QScrollBar *scroll = new QScrollBar( Qt::Horizontal, parent );
    standardRangeControllStuff( scroll );

    QLCDNumber *lcd = new QLCDNumber( parent );
    lcd->setSegmentStyle( QLCDNumber::Flat );
    lcd->display( 64 );

    QSlider *slider = new QSlider( Qt::Horizontal, parent );
    slider->setTickmarks( QSlider::Below );
    standardRangeControllStuff( slider );

    standardLayoutWidgetStuff( parent );
}

void setupItemWidgets()
{
    QGrid *parent = new QGrid( 2, Qt::Horizontal );

    QIconView *iconView = new QIconView( parent );
    (void) new QIconViewItem( iconView, "Icon 1", QPixmap("AddressBook.png") );
    (void) new QIconViewItem( iconView, "Icon 2", QPixmap("Calibrate.png") );
    (void) new QIconViewItem( iconView, "Icon 3", QPixmap("CityTime.png") );
    QIconViewItem *ivi4 = new QIconViewItem( iconView, "QIconView",
					     QPixmap("Clock.png") );
    iconView->setHScrollBarMode( QScrollView::AlwaysOff ); 
    iconView->setVScrollBarMode( QScrollView::AlwaysOff );

    QListView *listView = new QListView( parent );
    listView->setRootIsDecorated( TRUE );
    listView->setAllColumnsShowFocus( TRUE );
    listView->setResizeMode( QListView::AllColumns );
    listView->addColumn( "Column 1" );
    listView->addColumn( "Column 2" );
    listView->addColumn( "Column 3" );
    QListViewItem *root = new QListViewItem( listView, "QListView" );
    root->setOpen( TRUE );
    QListViewItem *lvi1 = new QListViewItem( root, "Item 1", "One", "Un" );
    lvi1->setPixmap( 0, QPixmap("SettingsIcon.png") );
//    lvi1->setPixmap( 1, QPixmap("language.png") );
    QListViewItem *lvi2 = new QListViewItem( root, "Item 2", "Two",
					     "Deux" );
    lvi2->setPixmap( 0, QPixmap("SettingsIcon.png") );
    lvi2->setOpen( TRUE );
    QListViewItem *lvi3 = new QListViewItem( lvi2, "Item 3", "Three", "Trois" );
    lvi3->setPixmap( 0, QPixmap("trash.png") );
    QListViewItem *lvi4 = new QListViewItem( root, "Item 4", "Four", "Quatre" );
    lvi4->setPixmap( 0, QPixmap("SettingsIcon.png") );
    lvi4->setOpen( TRUE );
    QListViewItem *lvi5 = new QListViewItem( lvi4, "Item 5", "Five", "Cinq" );
    lvi5->setPixmap( 0, QPixmap("trash.png") );
    lvi5->setOpen( TRUE );
    QListViewItem *lvi6 = new QListViewItem( lvi5, "Item 6", "Six", "Six" );
    lvi6->setPixmap( 0, QPixmap("zoom.png") );
    (void) new QListViewItem( lvi6, "Item 7", "Seven", "Sept" );

    QListBox *listBox = new QListBox( parent );
    listBox->insertItem( QPixmap("SettingsIcon.png"), "Item 1" );
    listBox->insertItem( QPixmap("SettingsIcon.png"), "Item 2" );
    listBox->insertItem( QPixmap("trash.png"), "Item 3" );
    listBox->insertItem( QPixmap("SettingsIcon.png"), "QListBox" );

    QTable *table = new QTable( 5, 3, parent );
    table->setShowGrid( TRUE );
    QHeader *header = table->horizontalHeader();
    header->setLabel( 0, "QTableItem", 90 );
    header->setLabel( 1, "QCheckTableItem", 110 );
    header->setLabel( 2, "QComboTableItem", 110 );
    header->setMovingEnabled(TRUE);

    for ( int i = 0; i < 4; i++ ) {
	QImage img( qtlogo_xpm );
	QPixmap pix = img.scaleHeight( table->rowHeight(0) - 2 );
	if ( i == 1 )
	    table->setPixmap( i, 0, pix );
	table->setText( i, 0, QString("Item %1").arg(i + 1) );

	QCheckTableItem *item = new QCheckTableItem(table,
		QString("Check %1").arg(i + 1) );
	if ( i == 2 )
	    item->setChecked( TRUE );
	table->setItem( i, 1, item );
	if ( i == 1 || i == 3 )
	    table->setItem( i, 2, new QComboTableItem(table,
		    QString("Combo %1").arg(i + 1), FALSE) );
    }

    standardLayoutWidgetStuff( parent );

    ivi4->moveBy( 30, -20 );
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    setupLabelAndPushButton();
    setupButtons();
    setupEdits();
    setupRangeWidgets();
    setupItemWidgets();
    QObject::connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );
    return app.exec();
}
