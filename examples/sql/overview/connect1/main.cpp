/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include "../connection.h"


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv, FALSE );

    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_SALES_DRIVER );
    if ( ! defaultDB ) {
	qWarning( "Failed to connect to the database driver" );
	return 1;
    }
    defaultDB->setDatabaseName( DB_SALES_DBNAME );
    defaultDB->setUserName( DB_SALES_USER );
    defaultDB->setPassword( DB_SALES_PASSWD );
    defaultDB->setHostName( DB_SALES_HOST );

    if ( defaultDB->open() ) {
	// Database successfully opened; we can now issue SQL commands.
    }

    return 0;
}

