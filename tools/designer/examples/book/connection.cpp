/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <qsqldatabase.h>
#include "connection.h"

bool createConnections()
{
    // create the default database connection
    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_BOOKS_DRIVER );
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


