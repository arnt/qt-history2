#include "dialogs.h"

#include <qlayout.h>
#include <qabstractlayout.h>
#include <qmultilineedit.h>
#include <qsqltable.h>
#include <qsqlform.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qframe.h>
#include <qsplitter.h>

//
//  GenericDialog class
//

GenericDialog::GenericDialog( QSqlRecord* buf, Mode mode, QWidget * parent,
			      const char * name )
    : QDialog( parent, name, TRUE ),
      mMode( mode )
{
    QWidget *     w = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;

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
	w->setEnabled( FALSE );
    }
    setCaption( caption );

    form = new QSqlForm( w, buf, 2, this);
    g->setMargin( 3 );

    QLabel * label = new QLabel( caption, this );
    QFont f = font();
    f.setBold( TRUE );
    label->setFont( f );
    g->addWidget( label );

    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Minimum ) );

    QPushButton * button = new QPushButton( op, this );
    button->setDefault( TRUE );
    connect( button, SIGNAL( clicked() ), SLOT( execute() ) );
    h->addWidget( button );

    button = new QPushButton( "&Close", this );
    connect( button, SIGNAL( clicked() ), SLOT( close() ) );
    h->addWidget( button );

    g->addWidget( w );
    g->addLayout( h );
}

void GenericDialog::close()
{
    reject();
}

void GenericDialog::execute()
{
    form->writeRecord();
    accept();
}

//
//  InvoiceDialog
//

InvoiceDialog::InvoiceDialog( QSqlRecord * buf,
			      Mode mode, QWidget * parent, const char * name )
    : QDialog( parent, name, TRUE ),
      mMode( mode ),
      cleanupOrphans( FALSE )
{
    QString op, caption;
    QWidget *     form = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;
    if( mMode == Insert ){
	op      = "&Insert";
	caption = "Insert record";
    } else if( mMode == Update ){
	op      = "&Update";
	caption = "Update record";
    } else if( mMode == Delete ){
	op      = "&Delete";
	caption = "Delete record";
	form->setEnabled( FALSE );
    }
    setCaption( caption );

    g->setSpacing( 5 );
    g->setMargin( 3 );
    h->setSpacing( 0 );
    h->setMargin( 0 );

    // Generate a form based on 'buf'
    invoiceForm  = new QSqlForm( form, buf, 2, this);
    invoiceItems = new QSqlTable( this );

    invoiceId = buf->value("id"); // Save this for later - we need it..
    itemCursor.select( "invoiceid = " + invoiceId.toString() );

    invoiceItems->setCursor( &itemCursor );
    connect( invoiceItems, SIGNAL( currentChanged(const QSqlRecord *) ),
	     SLOT( updateProductTable(const QSqlRecord *) ) );
    connect( invoiceItems, SIGNAL( beginInsert( QSqlRecord* ) ),
	     SLOT( insertingInvoiceItem( QSqlRecord* ) ) );

    invoiceItems->refresh();

    QFont f = font();
    f.setBold( TRUE );

    QLabel * label = new QLabel( "Invoice", this );
    label->setFont( f );
    g->addWidget( label );

    QPushButton * button = new QPushButton( "U&pdate item", this );
    connect( button, SIGNAL( clicked() ), SLOT( updateInvoiceItem() ) );
    h->addWidget( button );
    if( mMode == Delete )
	button->setEnabled( FALSE );

    button = new QPushButton( "In&sert item", this );
    connect( button, SIGNAL( clicked() ), SLOT( insertInvoiceItem() ) );
    h->addWidget( button );
    if( mMode == Delete )
	button->setEnabled( FALSE );

    button = new QPushButton( "Delete i&tem", this );
    connect( button, SIGNAL( clicked() ), SLOT( deleteInvoiceItem() ) );
    h->addWidget( button );
    if( mMode == Delete )
	button->setEnabled( FALSE );

    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Minimum ) );

    button = new QPushButton( op, this );
    button->setDefault( TRUE );
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

void InvoiceDialog::insertingInvoiceItem( QSqlRecord* buf )
{
    checkInsertInvoice();
    buf->setValue( "invoiceid", invoiceId );
}

void InvoiceDialog::checkInsertInvoice()
{
    if ( mMode == Insert ) {
	qDebug("inserting dummy invoice");
	QSqlQuery q;
	q.exec( "insert into invoice (id) values (" + invoiceId.toString() + ");" );
	cleanupOrphans = TRUE;
	mMode = Update;
    }
}

void InvoiceDialog::updateProductTable( const QSqlRecord * )
{
}

void InvoiceDialog::updateInvoiceItem()
{
    QSqlRecord r = invoiceItems->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection", "Select an item first!" );
	return;
    }
    
    QSqlCursor * cr = invoiceItems->cursor();

    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->update();
	invoiceItems->refresh();
    }
}

void InvoiceDialog::insertInvoiceItem()
{
    QSqlCursor * cr = invoiceItems->cursor();
    QSqlRecord* buf = cr->insertBuffer();
    insertingInvoiceItem( buf );

    GenericDialog dlg( buf, GenericDialog::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->insert();
	invoiceItems->refresh();
    }
}

void InvoiceDialog::deleteInvoiceItem()
{
    QSqlRecord r = invoiceItems->currentFieldSelection();
    if ( r.isEmpty() ) {
	QMessageBox::information( this, "No Selection", "Select an item first!" );
	return;
    }
    
    QSqlCursor * cr = invoiceItems->cursor();

    GenericDialog dlg( cr->updateBuffer(), GenericDialog::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	cr->del();
	invoiceItems->refresh();
    }
}

void InvoiceDialog::close()
{
    qDebug("void InvoiceDialog::close()");
    /* check for orphaned items */
    if ( cleanupOrphans ) {
	qDebug("cleaning up orphans");
	QSqlQuery q;
	q.exec( "delete from invoiceitems where invoiceid=" + invoiceId.toString() );
	q.exec( "delete from invoice where id=" + invoiceId.toString() );
    }
    qDebug("about to reject");
    reject();
    qDebug("after reject");
}

void InvoiceDialog::execute()
{
    invoiceForm->writeRecord();
    if ( mMode == Delete ) {
	QSqlQuery q;
	q.exec( "delete from invoiceitems where invoiceid=" + invoiceId.toString() );	
    }
    accept();
}
