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
//  DatabaseDlg class
//

DatabaseDlg::DatabaseDlg( QSqlCursor * cr, QSqlRecord* buf, Mode mode, QWidget * parent,
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
    }
    setCaption( caption );

    mForm = new QSqlForm( w, cr, buf, 2, this);
    g->setMargin( 3 );

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
    button->setDefault( TRUE );
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

InvoiceDlg::InvoiceDlg( QSqlCursor * cursor, QSqlRecord* buf, Mode mode, QWidget * parent,
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
    invoiceForm  = new QSqlForm( form, cursor, buf, 2, this);
    invoiceItems = new QSqlTable( this );

    invoiceId = cursor->value("id"); // Save this for later - we need it..
    itemCursor.select( "invoiceid = " + invoiceId.toString() );

    invoiceItems->setCursor( &itemCursor );
    connect( invoiceItems, SIGNAL( currentChanged(const QSqlRecord *) ),
	     SLOT( updateProductTable(const QSqlRecord *) ) );
    connect( invoiceItems, SIGNAL( beginInsert( QSqlRecord* ) ),
	     SLOT( insertingInvoiceItem( QSqlRecord* ) ) );

    productCr = new ProductCursor();
    productCr->select( "id = " + itemCursor.value("productid").toString() );
    //    productCr->next();

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

void InvoiceDlg::insertingInvoiceItem( QSqlRecord* buf )
{
    buf->setValue( "invoiceid", invoiceId );
}

void InvoiceDlg::updateProductTable( const QSqlRecord * )
{
}

void InvoiceDlg::updateInvoiceItem()
{
    QSqlCursor * v = invoiceItems->cursor();

    DatabaseDlg dlg( v, v->updateBuffer(), DatabaseDlg::Delete, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->update();
	invoiceItems->refresh();
    }
}

void InvoiceDlg::insertInvoiceItem()
{
    QSqlCursor * v = invoiceItems->cursor();
    QSqlRecord* buf = v->insertBuffer();
    insertingInvoiceItem( buf );

    DatabaseDlg dlg( v, buf, DatabaseDlg::Insert, this );
    if( dlg.exec() == QDialog::Accepted ){
	v->insert();
	invoiceItems->refresh();
    }
}

void InvoiceDlg::deleteInvoiceItem()
{
    QSqlCursor * v = invoiceItems->cursor();

    DatabaseDlg dlg( v, v->updateBuffer(), DatabaseDlg::Delete, this );
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
