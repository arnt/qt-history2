#include "databaseapp.h"
#include "dialogs.h"
#include "db.h"

#include <qlayout.h>
#include <qsqltable.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qapplication.h>

//
//  DatabaseWgt class
//

DatabaseWgt::DatabaseWgt( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    init();
}

void DatabaseWgt::init()
{
    hSplitter   = new QSplitter( QSplitter::Horizontal, this );
    QFrame * f1 = new QFrame( hSplitter );
    vSplitter   = new QSplitter( QSplitter::Vertical, hSplitter  );
    QFrame * f2 = new QFrame( vSplitter );
    QFrame * f3 = new QFrame( vSplitter );
    QVBoxLayout * vb1 = new QVBoxLayout( f1 );
    QVBoxLayout * vb2 = new QVBoxLayout( f2 );
    QVBoxLayout * vb3 = new QVBoxLayout( f3 );

    vb1->setMargin( 5 );
    vb2->setMargin( 5 );
    vb3->setMargin( 5 );
    vb1->setSpacing( 5 );
    vb2->setSpacing( 5 );
    vb3->setSpacing( 5 );

    //
    // First area - customer table
    //
    QFont f = font();
    f.setBold( TRUE );

    QLabel * label = new QLabel( f1 );
    label->setText( "Customers" );
    label->setFont( f );
    QFontMetrics fm = label->fontMetrics();

    vb1->addWidget( label );

    customerTable = new QSqlTable( f1 );
    vb1->addWidget( customerTable );

    // customer buttons
    QFrame * buttonFrame = new QFrame( f1 );
    QHBoxLayout * chl = new QHBoxLayout( buttonFrame );
    chl->setSpacing( 2 );

    chl->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				   QSizePolicy::Minimum ) );

    QPushButton * button = new QPushButton( "U&pdate", buttonFrame );
    chl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( updateCustomer() ) );

    button = new QPushButton( "I&nsert", buttonFrame );
    chl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( insertCustomer() ) );

    button = new QPushButton( "D&elete", buttonFrame );
    chl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( deleteCustomer() ) );

    vb1->addWidget( buttonFrame );

    //
    // Second area - customer information
    //
    label = new QLabel( f2 );
    label->setText( "Customer information" );
    label->setFont( f );
    label->resize( fm.width("Customer information" ), label->height() );
    vb2->addWidget( label );

    customerInfo = new QLabel( f2 );
    customerInfo->setText( "Customer info goes here!" );
    customerInfo->resize( fm.width("Customer info goes here!" ),
 		      customerInfo->height() );
    customerInfo->setFont( QFont( "fixed" ) );
    customerInfo->setAutoResize( TRUE );
    customerInfo->setSizePolicy( QSizePolicy( QSizePolicy::Preferred,
					      QSizePolicy::Fixed ) );
    vb2->addWidget( customerInfo );

    //
    // Third area - invoice table
    //
    label = new QLabel( f3 );
    label->setText( "Invoices" );
    label->setFont( f );
    vb3->addWidget( label );

    invoiceTable  = new QSqlTable( f3 );
    connect( invoiceTable, SIGNAL( beginInsert( QSqlRecord* ) ),
	     SLOT( insertingInvoice( QSqlRecord* ) ) );

    vb3->addWidget( invoiceTable );

    // invoice buttons
    buttonFrame = new QFrame( f3 ); // this
    QHBoxLayout * ihl = new QHBoxLayout( buttonFrame );

    vb3->addWidget( buttonFrame );

    ihl->setSpacing( 2 );
    ihl->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, 
				   QSizePolicy::Minimum ) );

    button = new QPushButton( "&Update", buttonFrame );
    ihl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( updateInvoice() ) );

    button = new QPushButton( "&Insert", buttonFrame );
    ihl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( insertInvoice() ) );

    button = new QPushButton( "&Delete", buttonFrame );
    ihl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( deleteInvoice() ) );
}

void DatabaseWgt::resizeEvent( QResizeEvent * )
{
    hSplitter->resize( width(), height() );
}

void DatabaseWgt::insertCustomer()
{
    QSqlCursor * cr = customerTable->cursor();

    GenericDialog dlg( cr->insertBuffer(), GenericDialog::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->insert();
	customerTable->refresh();
    }
}

void DatabaseWgt::updateCustomer()
{
    QSqlCursor * cr = customerTable->cursor();

    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Update, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->update();
	customerTable->refresh();
    }
}

void DatabaseWgt::deleteCustomer()
{
    QSqlCursor * cr = customerTable->cursor();

    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->del();
	customerTable->refresh();
    }
}

void DatabaseWgt::insertInvoice()
{
    QSqlRecord fl = customerTable->currentFieldSelection();
    if( fl.isEmpty() ) {
	QMessageBox::information( this, "No Customer", "There must be a Customer record for this invoice!" );
	return;
    }
    QSqlCursor * cr = invoiceTable->cursor();
    QSqlRecord * buf = cr->insertBuffer();
    insertingInvoice( buf );

    InvoiceDialog dlg( buf, InvoiceDialog::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->insert();
	invoiceTable->refresh( cr->primaryIndex( TRUE ) );
    }
}
void DatabaseWgt::insertingInvoice( QSqlRecord* buf )
{
    QSqlRecord fl = customerTable->currentFieldSelection();
    buf->setValue( "customerid", fl.field("id")->value() );
}

void DatabaseWgt::updateInvoice()
{
    QSqlRecord r = invoiceTable->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection", "Select an invoice first!" );
	return;
    }
    QSqlCursor  * cr = invoiceTable->cursor();

    InvoiceDialog dlg( cr->updateBuffer(), InvoiceDialog::Update, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->update();
	invoiceTable->refresh( cr->primaryIndex( TRUE ) );
    }
}

void DatabaseWgt::deleteInvoice()
{
    QSqlRecord r = invoiceTable->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection", "Select an invoice first!" );
	return;
    }

    QSqlCursor  * cr = invoiceTable->cursor();
    InvoiceDialog dlg( cr->updateBuffer(), InvoiceDialog::Delete, this );

    if( dlg.exec() == QDialog::Accepted ){
	cr->del();
	invoiceTable->refresh();
    }
}

//
//  DatabaseApp class
//
DatabaseApp::DatabaseApp( QWidget * parent, const char * name )
    : QMainWindow( parent, name )
{
    init();
}

void DatabaseApp::init()
{
    setMinimumSize( 640, 480 );

    d = new DatabaseWgt( this );
    setCentralWidget( d );

    invoiceCr.select( invoiceCr.primaryIndex() );
    customerCr.select( customerCr.primaryIndex() );

    QPopupMenu * p = new QPopupMenu( this );

    // Menus
    p->insertItem( "&Quit", qApp, SLOT( quit() ), CTRL+Key_Q );
    menuBar()->insertItem( "&File", p );
    p = new QPopupMenu( this );
    p->insertItem( "&Create database", this, SLOT( createDB()), CTRL+Key_O );
    p->insertItem( "&Drop database", this, SLOT( dropDB()), CTRL+Key_D );
    menuBar()->insertItem( "&Tools", p );

    // Set up the customer table
    d->customerTable->setConfirmEdits( TRUE );
    d->customerTable->setConfirmCancels( TRUE );
    d->customerTable->setCursor( &customerCr );
    connect( d->customerTable, SIGNAL( currentChanged( const QSqlRecord * ) ),
	     SLOT( updateCustomerInfo( const QSqlRecord * ) ) );

    // Set up the invoice table
    d->invoiceTable->setConfirmEdits( TRUE );
    d->invoiceTable->setConfirmCancels( TRUE );
    d->invoiceTable->setCursor( &invoiceCr );
}

void DatabaseApp::updateCustomerInfo( const QSqlRecord * fields )
{
    static int row = d->customerTable->currentRow();
    QString cap;

    if( row != d->customerTable->currentRow() ){
	// Compile the customer information into a string and display it
	for ( uint i = 0; i < fields->count(); ++i ) {
	    const QSqlField * f  = fields->field(i);
	    if( f->isVisible() )
		cap += f->displayLabel().leftJustify(15) + ": " +
		       f->value().toString().rightJustify(20) + "\n";
	}


	d->customerInfo->setText( cap );
	d->customerInfo->setMinimumSize( 0, d->customerInfo->height() );

	// Only show the invoice(s) for a particular customer
	// Use the customer id to filter the invoice cursor
	invoiceCr.select( "customerid = " +
			  fields->field(0)->value().toString() );
	d->invoiceTable->refresh();
    }
    row = d->customerTable->currentRow();
}

void DatabaseApp::createDB()
{
    create_db();
}

void DatabaseApp::dropDB()
{
    drop_db();
}
