/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include "book.h"

bool createConnections();

int main( int argc, char *argv[] ) 
{
    QApplication app( argc, argv );

    if ( ! createConnections() ) 
	return 1;

    BookForm bookForm;
    app.setMainWidget( &bookForm );
    bookForm.show();

    return app.exec();
}


bool createConnections()
{
    // create the default database connection
    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( "QODBC" );
    defaultDB->setDatabaseName( "books" );
    defaultDB->setUserName( "booksuser" );
    defaultDB->setPassword( "bookspw" );
    defaultDB->setHostName( "bookshost" );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open books database: " + 
		  defaultDB->lastError().driverText() );
	qWarning( defaultDB->lastError().databaseText() );
	return FALSE;
    }

    return TRUE;
}


