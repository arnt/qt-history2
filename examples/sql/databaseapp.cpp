#include "databaseapp.h"
#include "dialogs.h"
#include "db.h"

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
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
#include <qfiledialog.h>

//
//  DatabaseFrontEnd class
//

DatabaseFrontEnd::DatabaseFrontEnd( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    init();
}

/*!
  Initialize the widgets that appear in the database front-end.
 */
void DatabaseFrontEnd::init()
{
    QSplitter * vSplitter, * hSplitter;
    QGridLayout* gl = new QGridLayout( this );
    hSplitter   = new QSplitter( QSplitter::Horizontal, this );
    gl->addWidget( hSplitter, 0, 0 );
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
    //    customerInfo->setText( "Customer info goes here!" );
    //    customerInfo->resize( fm.width( customerInfo->text() ),
    // 		      customerInfo->height() );
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

    //
    // Set up the initial customer and invoice tables
    //
    customerCr.select( customerCr.primaryIndex() );
    invoiceCr.select( invoiceCr.primaryIndex() );

    // customer table
    customerTable->setConfirmEdits( TRUE );
    customerTable->setConfirmCancels( TRUE );
    customerTable->setCursor( &customerCr );
    connect( customerTable, SIGNAL( currentChanged( const QSqlRecord * ) ),
	     SLOT( updateCustomerInfo( const QSqlRecord * ) ) );
    QSqlRecord r = customerTable->currentFieldSelection();
    updateCustomerInfo( &r );

    // invoice table
    invoiceTable->setConfirmEdits( TRUE );
    invoiceTable->setConfirmCancels( TRUE );
    invoiceTable->setCursor( &invoiceCr );

}

/*!  When a new customer is selected in the customer table - show
  extended customer info to the user and update the invoice table to
  show the invoices for the currently selected customer.
 */
void DatabaseFrontEnd::updateCustomerInfo( const QSqlRecord * fields )
{
    QString cap;

    // Compile the customer information into a string and display it
    for ( uint i = 0; i < fields->count(); ++i ) {
	const QSqlField * f  = fields->field(i);
	if( f->isVisible() )
	    cap += f->displayLabel().leftJustify(15) + ": " +
		   f->value().toString().rightJustify(20) + "\n";
    }
    customerInfo->setText( cap );
    customerInfo->setMinimumSize( 0, customerInfo->height() );

    // Only show the invoice(s) for a particular customer
    // Use the customer id as a filter
    invoiceCr.select( "customerid = " + fields->value("id").toString() );
    invoiceTable->refresh();
}

/*!  Pops up a data entry form for inserting new customers into the
  database.
 */
void DatabaseFrontEnd::insertCustomer()
{
    QSqlCursor * cr = customerTable->cursor();

    GenericDialog dlg( cr->insertBuffer(), GenericDialog::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->insert();
	customerTable->refresh();
    }
}

/*!  Pops up a data entry form for editing an existing customer. We
  ensure that the fields in the form will contain the values for the
  currently selected customer by using QSqlCursor::updateBuffer().
 */
void DatabaseFrontEnd::updateCustomer()
{
    QSqlCursor * cr = customerTable->cursor();

    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Update, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->update();
	customerTable->refresh();
    }
}

/*!  Pops up a data entry form for deleting an existing customer. The
  fields in the form will contain the current values for the currently
  selected customer.
 */
void DatabaseFrontEnd::deleteCustomer()
{
    QSqlCursor * cr = customerTable->cursor();

    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->del();
	customerTable->refresh();
    }
}

/*!  Pops up a data entry form for inserting a new invoice for a
  customer.
 */
void DatabaseFrontEnd::insertInvoice()
{
    QSqlRecord fl = customerTable->currentFieldSelection();
    if( fl.isEmpty() ) {
	QMessageBox::information( this, "No Customer",
			 "There must be a Customer record for this invoice!" );
	return;
    }
    QSqlCursor * cr = invoiceTable->cursor();
    QSqlRecord * buf = cr->insertBuffer();
    insertingInvoice( buf );

    GenericDialog dlg( buf, GenericDialog::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->insert();
	invoiceTable->refresh( cr->primaryIndex( TRUE ) );
    }
}

/*! Link the new invoice to the currently selected customer.
*/
void DatabaseFrontEnd::insertingInvoice( QSqlRecord* buf )
{
    QSqlRecord fl = customerTable->currentFieldSelection();
    buf->setValue( "customerid", fl.field("id")->value() );
}

/*!  Pops up a data entry form for updating an invoice for a customer.
  We ensure tha the fields in the form will contain the current values
  for the currently selected invoice by using
  QSqlCursor::updateBuffer().
*/
void DatabaseFrontEnd::updateInvoice()
{
    QSqlRecord r = invoiceTable->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection",
				  "Select an invoice first!" );
	return;
    }
    QSqlCursor  * cr = invoiceTable->cursor();

    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Update, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->update();
	invoiceTable->refresh( cr->primaryIndex( TRUE ) );
    }
}

/*!  Pops up a data entry form for deleting an existing invoice. The
  fields in the form will contain the current values for the currently
  selected invoice.
 */
void DatabaseFrontEnd::deleteInvoice()
{
    QSqlRecord r = invoiceTable->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection",
				  "Select an invoice first!" );
	return;
    }

    QSqlCursor  * cr = invoiceTable->cursor();
    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Delete, this );

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
    setCaption( "Database example" );
    DatabaseFrontEnd * frontend = new DatabaseFrontEnd( this );
    setCentralWidget( frontend );

    // Setup menus
    QPopupMenu * menu = new QPopupMenu( this );
    menu->insertItem( "Customer &report..", this, SLOT( customerReport() ), 
		   CTRL+Key_R );    
    menu->insertSeparator();
    menu->insertItem( "&Quit", qApp, SLOT( quit() ), CTRL+Key_Q );
    menuBar()->insertItem( "&File", menu );
    
    menu = new QPopupMenu( this );
    menu->insertItem( "&Create database", this, SLOT( createDatabase()), 
		   CTRL+Key_O );
    menu->insertItem( "&Drop database", this, SLOT( dropDatabase()),
		      CTRL+Key_D );
    menuBar()->insertItem( "&Tools", menu );

    resize( 700, 400 );
    frontend->setMinimumSize( 640, 400 );
}

void DatabaseApp::createDatabase()
{
    create_db();
}

void DatabaseApp::dropDatabase()
{
    drop_db();
}

void DatabaseApp::customerReport()
{
    QString fname = QFileDialog::getSaveFileName( "report.html", "*.html", 
						  this );
    /* report customers and invoices */
    if( !fname.isEmpty() ) {
	QFile output( fname );
	if ( !output.open( IO_WriteOnly ) )
	    return;
	
        QTextStream t( &output );
	t << "<html>";
	t << "<head>";
	t << "<title>Customer Report</title>";
	t << "<body bgcolor=#fff8f8>";
	t << "<h1>Customer Report - Unpaid and Paid Invoices</h1>";

	/* unpaid invoices */
	t << "<h2>Unpaid Invoices</h2>";
	t << "<table border=1 cellpadding=0 cellspacing=0 width=50%>\n";
	QSqlQuery unpaidInvoices;
	t << "<tr><h3>";
	t << "<td>Customer Name</td>";
	t << "<td>Invoices</td>";
	t << "<td>Total</td>\n";
	t << "</tr></h3>";
	unpaidInvoices.exec( "select a.name, count(b.id), sum(b.total) from customer a, invoice b "
			   "where b.customerid=a.id and b.paid=0 group by a.name order by a.name;" );
	while ( unpaidInvoices.next() ) {
	    t << "<tr>";
	    t << "<td>" << unpaidInvoices.value(0).toString().rightJustify(30) << "</td>";
	    t << "<td>" << unpaidInvoices.value(1).toString().rightJustify(30) << "</td>";
	    t << "<td>" << unpaidInvoices.value(2).toString().rightJustify(30) << "</td>\n";
	    t << "</tr>";
	}
	t << "</table>";


	/* paid invoices */
	t << "<h2>Paid Invoices</h2>";
	t << "<table border=1 cellpadding=0 cellspacing=0 width=50%>\n";
	QSqlQuery paidInvoices;
	t << "<tr><h3>";
	t << "<td>Customer Name</td>";
	t << "<td>Invoices</td>";
	t << "<td>Total</td>\n";
	t << "</tr></h3>";
	paidInvoices.exec( "select a.name, count(b.id), sum(b.total) from customer a, invoice b "
			   "where b.customerid=a.id and b.paid=1 group by a.name order by a.name;" );
	while ( paidInvoices.next() ) {
	    t << "<tr>";
	    t << "<td>" << paidInvoices.value(0).toString().rightJustify(30) << "</td>";
	    t << "<td>" << paidInvoices.value(1).toString().rightJustify(30) << "</td>";
	    t << "<td>" << paidInvoices.value(2).toString().rightJustify(30) << "</td>\n";
	    t << "</tr>";
	}
	t << "</table>";

	t << "</body>";
	t << "</html>";

        output.close();
    }
}
