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
    v_splitter = new QSplitter( QSplitter::Horizontal, this );

    QFrame * f1 = new QFrame( v_splitter );
    h_splitter  = new QSplitter( QSplitter::Vertical, v_splitter  );

    QFrame * f2 = new QFrame( h_splitter );
    QFrame * f3 = new QFrame( h_splitter );
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

    customer_lbl = new QLabel( f1 );
    customer_lbl->setText( "Customers" );
    customer_lbl->setFont( f );
    QFontMetrics fm = customer_lbl->fontMetrics();

    vb1->addWidget( customer_lbl );

    customers = new QSqlTable( f1 );
    vb1->addWidget( customers );

    // customer buttons
    customerBtnFrm = new QFrame( f1 );
    QHBoxLayout * chl = new QHBoxLayout( customerBtnFrm );
    chl->setSpacing( 2 );

    chl->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				   QSizePolicy::Minimum ) );

    QPushButton * p = new QPushButton( "U&pdate", customerBtnFrm );
    chl->addWidget( p );
    connect( p, SIGNAL( clicked() ), this, SLOT( updateCustomer() ) );

    p = new QPushButton( "I&nsert", customerBtnFrm );
    chl->addWidget( p );
    connect( p, SIGNAL( clicked() ), this, SLOT( insertCustomer() ) );

    p = new QPushButton( "D&elete", customerBtnFrm );
    chl->addWidget( p );
    connect( p, SIGNAL( clicked() ), this, SLOT( deleteCustomer() ) );

    vb1->addWidget( customerBtnFrm );

    //
    // Second area - customer information
    //
    customer_inf_lbl = new QLabel( f2 );
    customer_inf_lbl->setText( "Customer information" );
    customer_inf_lbl->setFont( f );
    customer_inf_lbl->resize( fm.width("Customer information" ),
			      customer_inf_lbl->height() );
    vb2->addWidget( customer_inf_lbl );

    customer = new QLabel( f2 );
    customer->setText( "Customer info goes here!" );
    customer->resize( fm.width("Customer info goes here!" ),
 		      customer->height() );
    customer->setFont( QFont( "fixed" ) );
    customer->setAutoResize( TRUE );
    customer->setSizePolicy( QSizePolicy( QSizePolicy::Preferred,
					  QSizePolicy::Fixed ) );
    vb2->addWidget( customer );

    //
    // Third area - invoice table
    //
    invoice_lbl = new QLabel( f3 );
    invoice_lbl->setText( "Invoices" );
    invoice_lbl->setFont( f );
    vb3->addWidget( invoice_lbl );

    invoices  = new QSqlTable( f3 );
    connect( invoices, SIGNAL( beginInsert( QSqlRecord* ) ),
	     SLOT( insertingInvoice( QSqlRecord* ) ) );

    vb3->addWidget( invoices );

    // invoice buttons
    invoiceBtnFrm = new QFrame( f3 ); // this
    QHBoxLayout * ihl = new QHBoxLayout( invoiceBtnFrm );

    vb3->addWidget( invoiceBtnFrm );

    ihl->setSpacing( 2 );
    ihl->addItem( new QSpacerItem( 1, p->height(), QSizePolicy::Expanding ) );

    p = new QPushButton( "&Update", invoiceBtnFrm );
    ihl->addWidget( p );
    connect( p, SIGNAL( clicked() ), this, SLOT( updateInvoice() ) );

    p = new QPushButton( "&Insert", invoiceBtnFrm );
    p->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    ihl->addWidget( p );
    connect( p, SIGNAL( clicked() ), this, SLOT( insertInvoice() ) );

    p = new QPushButton( "&Delete", invoiceBtnFrm );
    p->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    ihl->addWidget( p );
    connect( p, SIGNAL( clicked() ), this, SLOT( deleteInvoice() ) );
}

void DatabaseWgt::resizeEvent( QResizeEvent * )
{
    v_splitter->resize( width(), height() );
}

void DatabaseWgt::insertCustomer()
{
    QSqlCursor * v = customers->cursor();

    DatabaseDlg dlg( v, v->insertBuffer(), DatabaseDlg::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->insert();
	customers->refresh();
    }
}

void DatabaseWgt::updateCustomer()
{
    QSqlCursor  * v = customers->cursor();

    DatabaseDlg dlg( v, v->updateBuffer(), DatabaseDlg::Update, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->update();
	customers->refresh();
    }
}

void DatabaseWgt::deleteCustomer()
{
    QSqlCursor  * v = customers->cursor();

    DatabaseDlg dlg( v, v->updateBuffer(), DatabaseDlg::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->del();
	customers->refresh();
    }
}

void DatabaseWgt::insertInvoice()
{
    QSqlRecord fl = customers->currentFieldSelection();
    if( fl.isEmpty() ) {
	QMessageBox::information( this, "No Customer", "There must be a Customer record for this invoice!" );
	return;
    }
    QSqlCursor * v = invoices->cursor();
    QSqlRecord* buf = v->insertBuffer();
    insertingInvoice( buf );

    InvoiceDlg dlg( v, buf, InvoiceDlg::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->insert();
	invoices->refresh( v->primaryIndex( TRUE ) );
    }
}
void DatabaseWgt::insertingInvoice( QSqlRecord* buf )
{
    QSqlRecord fl = customers->currentFieldSelection();
    buf->setValue( "customerid", fl.field("id")->value() );
}

void DatabaseWgt::updateInvoice()
{
    QSqlRecord r = invoices->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection", "Select an invoice first!" );
	return;
    }
    QSqlCursor  * v = invoices->cursor();

    InvoiceDlg dlg( v, v->updateBuffer(), InvoiceDlg::Update, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->update();
	invoices->refresh( v->primaryIndex( TRUE ) );
    }
}

void DatabaseWgt::deleteInvoice()
{
    QSqlRecord r = invoices->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection", "Select an invoice first!" );
	return;
    }

    QSqlCursor  * v = invoices->cursor();
    InvoiceDlg dlg( v, v->updateBuffer(), InvoiceDlg::Delete, this );

    if( dlg.exec() == QDialog::Accepted ){
	v->del();
	invoices->refresh();
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
    d->customers->setConfirmEdits( TRUE );
    d->customers->setConfirmCancels( TRUE );
    d->customers->setCursor( &customerCr );
    connect( d->customers, SIGNAL( currentChanged( const QSqlRecord * ) ),
	     SLOT( updateCustomerInfo( const QSqlRecord * ) ) );

    // Set up the invoice table
    d->invoices->setConfirmEdits( TRUE );
    d->invoices->setConfirmCancels( TRUE );
    d->invoices->setCursor( &invoiceCr );
}

void DatabaseApp::updateCustomerInfo( const QSqlRecord * fields )
{
    QString cap;

    // Compile the customer information into a string and display it
    for ( uint i = 0; i < fields->count(); ++i ) {
	const QSqlField * f  = fields->field(i);
	if( f->isVisible() )
	    cap += f->displayLabel().leftJustify(15) + ": " +
		   f->value().toString().rightJustify(20);
    }


    d->customer->setText( cap );
    d->customer->setMinimumSize( 0, d->customer->height() );

    // Only show the invoice(s) for a particular customer
    // Use the customer id to filter the invoice cursor
    invoiceCr.select( "customerid = " +
		      fields->field(0)->value().toString() );
    d->invoices->refresh();

}

void DatabaseApp::createDB()
{
    create_db();
}

void DatabaseApp::dropDB()
{
    drop_db();
}
