#include "cursors.h"

//
//  CustomerCursor class
//

CustomerCursor::CustomerCursor()
    : QSqlCursor( "customer" )
{
    // Hide the following from the user - will not show up in tables or forms.
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

    // Set display labels - used in tables and forms.
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

void CustomerCursor::primeInsert( QSqlRecord* buf )
{
    /* real-world apps should use a sequence or auto-numbered field */
    QSqlQuery nextId("select max(id)+1 from customer;");
    if ( nextId.next() )
	buf->setValue( "id", nextId.value(0) );
    QSqlCursor::primeInsert( buf );
}


//
//  ProductCursor class
//

ProductCursor::ProductCursor()
    : QSqlCursor( "product" )
{
}

void ProductCursor::primeInsert( QSqlRecord* buf )
{
    /* real-world apps should use a sequence or auto-numbered field */
    QSqlQuery nextId("select max(id)+1 from product;");
    if ( nextId.next() )
	buf->setValue( "id", nextId.value(0).toInt() );
    QSqlCursor::primeInsert( buf );
}


//
//  InvoiceCursor class
//

InvoiceCursor::InvoiceCursor()
    : QSqlCursor( "invoice" )
{
    // Set display labels
    field("num")->setDisplayLabel("Number");
    field("paid")->setDisplayLabel("Paid");
    field("createdate")->setDisplayLabel("Created");
    field("tax")->setDisplayLabel("Tax");
    field("shipping")->setDisplayLabel("Shipping");
    field("total")->setDisplayLabel("Total");

    // Don't show these fields in any tables or forms
    field("customerid")->setVisible( FALSE );
    field("paid")->setVisible( FALSE );
}

void InvoiceCursor::primeInsert( QSqlRecord* buf )
{
    /* real-world apps should use a sequence or auto-numbered field */
    QSqlQuery nextId("select max(id)+1 from invoice;");
    if ( nextId.next() )
	buf->setValue( "id", nextId.value(0).toInt() );
    QSqlCursor::primeInsert( buf );
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

    // add lookup field
    QSqlField productName("productname", 5, QVariant::String );
    productName.setDisplayLabel("Product Name");
    productName.setCalculated( TRUE );
    append( productName );
}

QVariant InvoiceItemCursor::calculateField( uint fieldNumber )
{
    if ( fieldNumber != 5 )
	return QVariant();
    if ( pr.value( "id" ).toInt() != field("productid")->value().toInt() ) {
	pr.setValue( "id",  field("productid")->value().toInt() );
	pr.select( pr.primaryIndex(), pr.primaryIndex() );
	if( pr.next() )
	    return pr.value( "name" );
	else
	    return QVariant( QString::null );
    } else
	return pr.value( "name" );
}

void InvoiceItemCursor::primeInsert( QSqlRecord* buf )
{
    /* real-world apps should use a sequence or auto-numbered field */
    QSqlQuery nextId("select max(id)+1 from invoiceitem;");
    if ( nextId.next() )
	buf->setValue( "id", nextId.value(0).toInt() );
    QSqlCursor::primeInsert( buf );
}

