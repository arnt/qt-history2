/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "main.h"
#include <qdatatable.h>

InvoiceItemCursor::InvoiceItemCursor() :
    QSqlCursor( "invoiceitem" )
{
    // NOOP
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( createConnections() ) {
	InvoiceItemCursor invoiceItemCursor;

	QDataTable *invoiceItemTable = new QDataTable( &invoiceItemCursor );

	app.setMainWidget( invoiceItemTable );

	invoiceItemTable->addColumn( "pricesid", "PriceID" );
	invoiceItemTable->addColumn( "quantity", "Quantity" );
	invoiceItemTable->addColumn( "paiddate", "Paid" );

	invoiceItemTable->refresh();
	invoiceItemTable->show();

	return app.exec();
    }

    return 1;
}
