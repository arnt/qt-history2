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
#include "../login.h"

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
    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_BOOKS_DRIVER );
    if ( ! defaultDB ) {
	qWarning( "Failed to connect to driver" );
	return FALSE;
    }
    defaultDB->setDatabaseName( DB_BOOKS );
    defaultDB->setUserName( DB_BOOKS_USER );
    defaultDB->setPassword( DB_BOOKS_PASSWD );
    defaultDB->setHostName( DB_BOOKS_HOST );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open books database: " + 
		  defaultDB->lastError().driverText() );
	qWarning( defaultDB->lastError().databaseText() );
	return FALSE;
    }

    return TRUE;
}


