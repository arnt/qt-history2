#include <qlayout.h>
#include <qabstractlayout.h>
#include <qmultilineedit.h>
#include <qsqltable.h>
#include <qsqlform.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qapplication.h>
#include "databaseapp.h"
#include "db.h"

//
//  CustomerCursor class
//

CustomerCursor::CustomerCursor()
    : QSqlCursor( "customer" )
{
    // Hide the following fields from the user - will not show up in tables or
    // forms.
    field("id")->setVisible( FALSE );
    field("billtoadd1")->setVisible( FALSE );
    field("billtoadd2")->setVisible( FALSE );
    field("billtocity")->setVisible( FALSE );
    field("billtopostalcode")->setVisible( FALSE ); 
    field("billtocountry")->setVisible( FALSE );
    field("shiptoadd1")->setVisible( FALSE );
    field("shiptoadd2")->setVisible( FALSE );
    field("shiptocity")->setVisible( FALSE );
    field("shiptopostalcode")->setVisible( FALSE ); 
    field("shiptocountry")->setVisible( FALSE );    

    // Set display labels for the different fields - used in tables and forms.
    field("name")->setDisplayLabel( "Name" );
    field("add1")->setDisplayLabel( "Address(1)" );
    field("add2")->setDisplayLabel( "Address(2)" );
    field("city")->setDisplayLabel( "City" );
    field("postalcode")->setDisplayLabel( "Zip code" );
    field("country")->setDisplayLabel( "Country" );
   
    field("billtoadd1")->setDisplayLabel( "Bill to Address (1)" );
    field("billtoadd2")->setDisplayLabel( "Bill to Address (2)" );
    field("billtocity")->setDisplayLabel( "Bill to City" );
    field("billtopostalcode")->setDisplayLabel( "Bill to Zip code" );
    field("billtocountry")->setDisplayLabel( "Bill to Country" );

    field("shiptoadd1")->setDisplayLabel( "Ship to Address (1)" );
    field("shiptoadd2")->setDisplayLabel( "Ship to Address(2)" );
    field("shiptocity")->setDisplayLabel( "Ship to City" );
    field("shiptopostalcode")->setDisplayLabel( "Ship to Zip code" );
    field("shiptocountry")->setDisplayLabel( "Ship to Country" );
}

//
//  InvoiceCursor class
//

InvoiceCursor::InvoiceCursor()
    : QSqlCursor( "invoice" )
{
    // Hide the following fields from the user
    field("customerid")->setVisible( FALSE );
    field("paid")->setVisible( FALSE );
    
    // Set display labels
    field("number")->setDisplayLabel("Number");
    field("paid")->setDisplayLabel("Paid");
    field("createdate")->setDisplayLabel("Created");
    field("tax")->setDisplayLabel("Tax");
    field("shipping")->setDisplayLabel("Shipping");
    field("total")->setDisplayLabel("Total");

    // Don't show these fields in any tables or forms
    field("customerid")->setVisible( FALSE );
    field("paid")->setVisible( FALSE );
    
    field("number")->setDisplayLabel("Number");
    field("paid")->setDisplayLabel("Paid");
    field("createdate")->setDisplayLabel("Created");
    field("tax")->setDisplayLabel("Tax");
    field("shipping")->setDisplayLabel("Shipping");
    field("total")->setDisplayLabel("Total");
}

//
//  InvoiceItemCursor class
//

InvoiceItemCursor::InvoiceItemCursor()
    : QSqlCursor( "invoiceitem" )
{
    // Hide fields
    field("id")->setVisible( FALSE );
    field("invoiceid")->setVisible( FALSE );

    // Set display labels
    field("productid")->setDisplayLabel("Product No.");
    field("quantity")->setDisplayLabel("Quantity");
    field("total")->setDisplayLabel("Total");

    QSqlField productName("productname", 6, QVariant::String );
    productName.setDisplayLabel("Product Name");
    productName.setCalculated( TRUE );
    append( productName );
}

QVariant InvoiceItemCursor::calculateField( uint fieldNumber )
{
    if( fieldNumber == 6 ){
	QSqlCursor pr("product");
	
	pr["id"] = field("productid")->value().toInt();
	pr.select( pr.primaryIndex(), pr.primaryIndex() );
	if( pr.next() )
	    return pr["name"];
	else
	    return QString::null;
    } 
}

//
//  DatabaseDlg class
//

DatabaseDlg::DatabaseDlg( QSqlCursor * cr, Mode mode, QWidget * parent,
			  const char * name )
    : QDialog( parent, name, TRUE ),
      mMode( mode )
{
    QWidget *     w = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;
    
    mForm = new QSqlForm( w, cr, 2, this);
    g->setMargin( 3 );
    
    QString op, caption;
    if( mMode == Insert ){
	op      = "&Insert";
	caption = "Insert record";
    } else if( mMode == Update ){
	op      = "&Update";
	caption = "Update record";
    } else if( mMode == Delete ){
	op      = "&Delete";
	caption = "Delete record";
    }
    setCaption( caption );
    
    QLabel * label = new QLabel( caption, this );
    QFont f = font();
    f.setBold( TRUE );
    label->setFont( f );
    g->addWidget( label );
    
    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, 
				 QSizePolicy::Minimum ) );

    QPushButton * button = new QPushButton( op, this );
    connect( button, SIGNAL( clicked() ), SLOT( execute() ) );
    h->addWidget( button );

    button = new QPushButton( "&Close", this );
    connect( button, SIGNAL( clicked() ), SLOT( close() ) );
    h->addWidget( button );
    
    g->addWidget( w );
    g->addLayout( h );
}

void DatabaseDlg::close()
{
    reject();
}

void DatabaseDlg::execute()
{
    mForm->writeRecord();
    accept();
}

//
//  InvoiceDlg
//

InvoiceDlg::InvoiceDlg( QSqlCursor * cursor, Mode mode, QWidget * parent,
			  const char * name )
    : QDialog( parent, name, TRUE ),
      mMode( mode )
{
    QString op, caption;
    if( mMode == Insert ){
	op      = "&Insert";
	caption = "Insert record";
    } else if( mMode == Update ){
	op      = "&Update";
	caption = "Update record";
    } else if( mMode == Delete ){
	op      = "&Delete";
	caption = "Delete record";
    }
    setCaption( caption );

    QWidget *     form = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;

    g->setSpacing( 5 );
    g->setMargin( 3 );
    h->setSpacing( 0 );
    h->setMargin( 0 );
    
    // Generate a form based on cursor (should be an InvoiceCursor)
    invoiceForm  = new QSqlForm( form, cursor, 2, this);
    invoiceItems = new QSqlTable( this );

    invoiceId = cursor->value("id"); // Save this for later - we need it..
    itemCursor.select( "invoiceid = " + invoiceId.toString() );

    invoiceItems->setCursor( &itemCursor );
    connect( invoiceItems, SIGNAL( currentChanged(const QSqlRecord *) ),
	     SLOT( updateProductTable(const QSqlRecord *) ) );


    productCr = new QSqlCursor( "product" );
    productCr->setMode( QSqlCursor::Writable );
    productCr->select("id = " + itemCursor.value("productid").toString() );
    productCr->next();

    invoiceItems->refresh();
        
    QFont f = font();
    f.setBold( TRUE );

    QLabel * label = new QLabel( "Invoice", this );
    label->setFont( f );
    g->addWidget( label );
    
    QPushButton * button = new QPushButton( "U&pdate item", this );
    connect( button, SIGNAL( clicked() ), SLOT( updateInvoiceItem() ) );
    h->addWidget( button );

    button = new QPushButton( "In&sert item", this );
    connect( button, SIGNAL( clicked() ), SLOT( insertInvoiceItem() ) );
    h->addWidget( button );

    button = new QPushButton( "Delete i&tem", this );
    connect( button, SIGNAL( clicked() ), SLOT( deleteInvoiceItem() ) );
    h->addWidget( button );
    
    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, 
				 QSizePolicy::Minimum ) );

    button = new QPushButton( op, this );
    connect( button, SIGNAL( clicked() ), SLOT( execute() ) );
    h->addWidget( button );

    button = new QPushButton( "&Close", this );
    connect( button, SIGNAL( clicked() ), SLOT( close() ) );
    h->addWidget( button );

    g->addWidget( form );

    label = new QLabel( "Invoice items", this );
    label->setFont( f );
    
    g->addWidget( label );
    g->addWidget( invoiceItems );
    g->addLayout( h );
}

void InvoiceDlg::updateProductTable( const QSqlRecord * r )
{
}

void InvoiceDlg::updateInvoiceItem()
{
    QSqlCursor * v = invoiceItems->cursor();

    DatabaseDlg dlg( v, DatabaseDlg::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->update();
	invoiceItems->refresh();
    }
}

void InvoiceDlg::insertInvoiceItem()
{
    // What if the table is empty??
    QSqlCursor * v = invoiceItems->cursor();
    v->clearValues();

    // Generate a new unique id for the new customer
    QSqlQuery nextId( "select max(id)+1 from invoiceitem;" );
    if( nextId.next() )
	v->setValue( "id", nextId.value( 0 ) );
    v->setValue( "invoiceid", invoiceId );
    
    DatabaseDlg dlg( v, DatabaseDlg::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->insert();
	invoiceItems->refresh();
    }
}

void InvoiceDlg::deleteInvoiceItem()
{
    QSqlCursor * v = invoiceItems->cursor();

    DatabaseDlg dlg( v, DatabaseDlg::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->del();
	invoiceItems->refresh();
    }
}

void InvoiceDlg::close()
{
    reject();
}

void InvoiceDlg::execute()
{
    invoiceForm->writeRecord();
    accept();
}

//
//  DatabaseWgt class
//

DatabaseWgt::DatabaseWgt( QWidget * parent, const char * name )
    : QFrame( parent, name )
//    : DatabaseAppBase( parent, name )
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
    v->clearValues();

    // Generate a new unique id for the new customer
    QSqlQuery nextId( "select max(id)+1 from customer;" );
    if( nextId.next() )
	v->setValue( "id", nextId.value( 0 ) );
    
    DatabaseDlg dlg( v, DatabaseDlg::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->insert();
	customers->refresh();
    }
}

void DatabaseWgt::updateCustomer()
{
    QSqlCursor  * v = customers->cursor();
    DatabaseDlg dlg( v, DatabaseDlg::Update, this );
    
    if( dlg.exec() == QDialog::Accepted ){
	v->update( v->primaryIndex() );
	customers->refresh();
    }
}

void DatabaseWgt::deleteCustomer()
{
    QSqlCursor  * v = customers->cursor();
    DatabaseDlg dlg( v, DatabaseDlg::Delete, this );

    if( dlg.exec() == QDialog::Accepted ){
	v->del();
	customers->refresh();
    }
}

void DatabaseWgt::insertInvoice()
{
    QSqlCursor * v = invoices->cursor();

    // Generate a new unique id for this item
    QSqlQuery nextId( "select max(id)+1 from invoice;" );
    if( nextId.next() )
	v->setValue( "id", nextId.value(0) );
    
    // Get currently selected customer id from the customer table
    QSqlRecord fl = customers->currentFieldSelection();
    if( !fl.isEmpty() )
	v->setValue( "customerid", fl.field("id")->value() );
    
    InvoiceDlg dlg( v, InvoiceDlg::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->insert();
	invoices->refresh( v->primaryIndex( TRUE ) );
    }
}

void DatabaseWgt::updateInvoice()
{
    QSqlCursor  * v = invoices->cursor();
    
    InvoiceDlg dlg( v, InvoiceDlg::Update, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->update( v->primaryIndex() );
	invoices->refresh( v->primaryIndex( TRUE ) );
    }
}

void DatabaseWgt::deleteInvoice()
{
    QSqlCursor  * v = invoices->cursor();
    InvoiceDlg dlg( v, InvoiceDlg::Delete, this );

    if( dlg.exec() == QDialog::Accepted ){
	v->del( v->primaryIndex() );
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
    
    // Initialize the different widgets
    
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
    // Use the customer id to filter the invoice view
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
